#include "pch.h"
#include "Source/Tasks/Decision/Decision.h"

using namespace ARC2016;
using namespace ARC2016::Framework;
using namespace ARC2016::Parts;
using namespace ARC2016::Tasks;
using namespace Windows::Foundation;
using namespace Windows::System;

Decision::Decision(SensorMonitor* sensMonitor, DataSender* dataSender)
 : TaskBase("Decision", 100)
 , m_SendFlag(0)
 , m_MyHeartBeatFlag(0)
 , m_HeartBeatInterval(0)
 , m_CourseSection(E_COURSE_SECTION_FIRST)
 , m_GyroStatus(E_GYRO_MODE_FLAT)
 , m_FinalSectionJudgeCount(0)
 , m_FrontDistanceJudge(false)
 , m_RightDistanceJudge(false)
 , m_LeftDistanceJudge(false)
 , m_Goal(false)
{
	m_SensorMonitor = sensMonitor;
	m_DataSender = dataSender;
}

Decision::~Decision()
{
}

ResultEnum Decision::initialize()
{
	return E_RET_NORMAL;
}

ResultEnum Decision::taskMain()
{
	// 処理順は後で考える

	// マイコンとのハートビート監視
	HeartBeatCheck();

	// カメラ & センサ情報による動作判定及び指示
	MoveTypeDecision();

	return E_RET_NORMAL;
}

ResultEnum Decision::finalize()
{
	return E_RET_NORMAL;
}


void Decision::HeartBeatCheck()
{
	SendData();
	// ReadData();
}

void Decision::SendData()
{
	// if (m_SendFlag == 0)
	// {
		if (m_HeartBeatInterval >= 10)
		{
			// ラズパイ側のハートビート信号を反転させる
			m_MyHeartBeatFlag ^= 1;

			// 送信データ作成
			// 送信及びフラグ更新はDataSenderタスクに任せる
			char sendBuffer[10];

			sendBuffer[0] = 0;
			sendBuffer[1] = 0;
			sendBuffer[2] = 0;
			sendBuffer[3] = 0;		// どこかにm_MyHeartBeatFlagを入れる
			sendBuffer[4] = 0;
			sendBuffer[5] = 0;
			sendBuffer[6] = 0;
			sendBuffer[7] = 0;
			sendBuffer[8] = 0;
			sendBuffer[9] = 0;

			memcpy(m_DataSender->m_HeartBeatSendBuffer, sendBuffer, sizeof(sendBuffer));
			m_SendFlag = 1;
		}
		else
		{
			// ラズパイ側受信後のインターバル時間到達待ち, もしくは回線空き待ち
			m_HeartBeatInterval++;
		}
	//}
}

// 差し当たり使わない
void Decision::ReadData()
{
	if (m_SendFlag == 1)
	{
		char reciveBuffer[10];	/* 受信コマンドサイズに合わせる必要あり */

		memcpy(reciveBuffer, m_DataSender->m_HeartBeatReceiveBuffer, sizeof(reciveBuffer));

		if (reciveBuffer[0] == 0)
		{
			// 受信データがないので何もしない
		}
		else
		{
			// 受信データ確認
			if (reciveBuffer[0] != m_MiconHeartBeatFlag)
			{
				// 値確認は必要？

				// マイコン側のハートビートデータが変動していれば、ハートビート更新通知とみなす
				m_MiconHeartBeatFlag = reciveBuffer[0];

				// 送信フラグを0に戻し、ラズパイからのハートビート送信が行われるようにする
				m_SendFlag = 0;
				m_HeartBeatInterval = 0;
			}

			// 電源OFF I/O 確認　※別の通信かも
			if (reciveBuffer[0] == 1111 )
			{
				// 電源がOFFればよいので、ここでやっちゃう？
				TimeSpan t;
				t.Duration = 0;
				ShutdownKind kind = ShutdownKind::Shutdown;
				ShutdownManager::BeginShutdown(kind, t);
			}

			// 今回の受信データ初期化
			memset(m_DataSender->m_HeartBeatReceiveBuffer, 0, sizeof(m_DataSender->m_HeartBeatReceiveBuffer));
		}
	}
}

void Decision::MoveTypeDecision()
{	
	// ライントレース結果取得
	long moveFlag = GetMoveType();
	long moveLineTrace = ConversionLineTraceFlag(moveFlag);

	long moveIntegration = moveLineTrace;
	m_GyroStatus = E_GYRO_MODE_FLAT;

	if (m_SensorMonitor != nullptr)
	{
		// ジャイロセンサ状況確認
		m_GyroStatus = GyroCheck();

		// 距離センサ状況とライントレース結果により動作決定
		long moveIntegration = DistanceCheck(moveLineTrace);

		// 第3セクションからFinalセクションと判定するまでのカウント
		if (m_CourseSection == E_COURSE_SECTION_THIRD)
		{
			m_FinalSectionJudgeCount++;
		}
	}

	// Motorへ指示
	switch (moveIntegration)
	{
		case E_MOVE_STRAIGHT:
			setFrontMoveCommand();
			break;
		case E_MOVE_TURNLEFT:
			setLeftMoveCommand();
			break;
		case E_MOVE_TURNRIGHT:
			setRightMoveCommand();
			break;
	}

}

long Decision::ConversionLineTraceFlag(long flag)
{
	long retVal = E_MOVE_STRAIGHT;

	switch (flag)
	{
	case PATTERN_FRONT:
		retVal = E_MOVE_STRAIGHT;
		break;
	case PATTERN_FRONT_LEFT_15:              // 前進＆左 15°
	case PATTERN_FRONT_LEFT_30:              // 前進＆左 30°
	case PATTERN_FRONT_LEFT_45:              // 前進＆左 45°
	case PATTERN_FRONT_LEFT_90:              // 前進＆左 90°
	case PATTERN_TURN_LEFT_ONCE:             // 左旋回 単体 (15°)
	case PATTERN_TURN_LEFT_45:               // 左旋回 45°
	case PATTERN_TURN_LEFT_90:               // 左旋回 90°
		retVal = E_MOVE_TURNLEFT;
		break;
	case PATTERN_FRONT_RIGHT_15:             // 前進＆右 15°
	case PATTERN_FRONT_RIGHT_30:             // 前進＆右 30°
	case PATTERN_FRONT_RIGHT_45:             // 前進＆右 45°
	case PATTERN_FRONT_RIGHT_90:             // 前進＆右 90°
	case PATTERN_TURN_RIGHT_ONCE:            // 右旋回 単体 (15°)
	case PATTERN_TURN_RIGHT_45:              // 右旋回 45°
	case PATTERN_TURN_RIGHT_90:              // 右旋回 90°
		retVal = E_MOVE_TURNRIGHT;
		break;
	case PATTERN_STOP:
	case PATTERN_BACK:
	case PATTERN_PUT:
	case PATTERN_MISSING:
		retVal = E_MOVE_UNKNOWN;
		break;

	default:
		break;
	}

	return retVal;
}

long Decision::DistanceCheck(long typeLineTrace)
{
	long ret = typeLineTrace;
	std::vector<long> distanceValue;

	// 各距離データ取得
	m_SensorMonitor->GetDistanceSensorValue(distanceValue);

	// 距離データから周辺状況判定
	E_DISTANCE_JUDGE_TYPE judge = DistanceSensorCheck(distanceValue);

	switch (judge)
	{
	case E_DISTANCE_JUDGE_ALL:
		ret = typeLineTrace;
		break;
	case E_DISTANCE_JUDGE_FRONT_RIGHT:
		ret = E_MOVE_LEFT;
		break;
	case E_DISTANCE_JUDGE_FRONT_LEFT:
		ret = E_MOVE_RIGHT;
		break;
	case E_DISTANCE_JUDGE_RIGHT_LEFT:
		ret = typeLineTrace;
		break;
	case E_DISTANCE_JUDGE_FRONT:
		if (typeLineTrace != E_MOVE_STRAIGHT)
		{
			ret = typeLineTrace;
		}
		else
		{
			// 指針がないので、取り敢えずコースを考え右に曲がることにする
			ret = E_MOVE_RIGHT;
		}
		break;
	case E_DISTANCE_JUDGE_RIGHT:
		ret = E_MOVE_LEFT;
		break;
	case E_DISTANCE_JUDGE_LEFT:
		ret = E_MOVE_RIGHT;
		break;
	case E_DISTANCE_JUDGE_NONE:
		ret = typeLineTrace;
		break;

	default:
		break;
	}
	
	return (ret);
}

E_DISTANCE_JUDGE_TYPE Decision::DistanceSensorCheck(std::vector<long> data)
{
	E_DISTANCE_JUDGE_TYPE ret = E_DISTANCE_JUDGE_NONE;

	// 各距離データの閾値チェック（近辺に壁があるかどうか）
	// 条件設定が0の場合は距離センサ情報を無視する
	m_FrontDistanceJudge = WallCheck(m_FrontDistanceJudgementValue, data[E_DISTANCE_ORDER_FRONT]);
	m_RightDistanceJudge = WallCheck(m_RightDistanceJudgementValue, data[E_DISTANCE_ORDER_RIGHT]);
	m_LeftDistanceJudge = WallCheck(m_LeftDistanceJudgementValue, data[E_DISTANCE_ORDER_LEFT]);

	// 最終セクションの場合はゴール判定も行う
	if (m_CourseSection == E_COURSE_SECTION_FINAL)
	{
		bool frontJudge = WallCheck(m_FrontDistanceGoalJudgementValue, data[E_DISTANCE_ORDER_FRONT]);
		bool rightJudge = WallCheck(m_RightDistanceGoalJudgementValue, data[E_DISTANCE_ORDER_RIGHT]);
		bool leftJudge = WallCheck(m_LeftDistanceGoalJudgementValue, data[E_DISTANCE_ORDER_LEFT]);
		
		if (frontJudge == true && rightJudge == true && leftJudge == true)
		{
			m_Goal = true;
		}
	}

	// 状況判定
	if (m_FrontDistanceJudge == true)
	{
		if (m_RightDistanceJudge == true)
		{
			if (m_LeftDistanceJudge == true)
			{
				// 3方向 壁有
				ret = E_DISTANCE_JUDGE_ALL;
			}
			else
			{
				// 前・右　壁有
				ret = E_DISTANCE_JUDGE_FRONT_RIGHT;
			}
		}
		else if (m_LeftDistanceJudge == true)
		{
			// 前・左 壁有
			ret = E_DISTANCE_JUDGE_FRONT_LEFT;
		}
		else
		{
			// 前のみ壁
			ret = E_DISTANCE_JUDGE_FRONT;
		}
	}
	else if (m_RightDistanceJudge == true)
	{
		if (m_LeftDistanceJudge == true)
		{
			// 左右 壁有
			ret = E_DISTANCE_JUDGE_RIGHT_LEFT;
		}
		else
		{
			// 右のみ壁
			ret = E_DISTANCE_JUDGE_RIGHT;
		}
	}
	else if (m_LeftDistanceJudge == true)
	{
		// 左のみ壁
		ret = E_DISTANCE_JUDGE_LEFT;
	}

	return ret;
}

bool Decision::WallCheck(long judgeValue, long data)
{
	bool retVal = false;

	if (judgeValue != 0 && data <= judgeValue)
	{
		retVal = true;
	}

	return retVal;
}

E_GYRO_MODE_TYPE Decision::GyroCheck()
{
	E_GYRO_MODE_TYPE ret = E_GYRO_MODE_FLAT;

	// ジャイロセンサの読み値取得
	std::vector<ARC2016::Parts::GyroSensor::GyroDataStr> gyroValue;
	m_SensorMonitor->GetGyroSensorValue(gyroValue);

	// 各状態の閾値チェック（坂道かどうか）
	if (gyroValue[0].AngleY > m_SlopeJudgementValue)
	{
		// 坂道
		ret = E_GYRO_MODE_SLOPE;
	}
	else
	{
		// 通常
		ret = E_GYRO_MODE_FLAT;
	}

	// 走行中のセクション判定
	if (m_CourseSection == E_COURSE_SECTION_FIRST && ret == E_GYRO_MODE_SLOPE)
	{
		// 第2セクションに入ったと判定
		m_CourseSection = E_COURSE_SECTION_SECOND;
	}
	else if (m_CourseSection == E_COURSE_SECTION_SECOND && ret == E_GYRO_MODE_FLAT)
	{
		// 第3セクションに入ったと判定
		m_CourseSection = E_COURSE_SECTION_THIRD;
	}
	else if (m_CourseSection == E_COURSE_SECTION_THIRD && m_FinalSectionJudgeCount >= COURSE_SECTION_FINAL_JUDGE_TIME)
	{
		// 第3セクション到達後、一定時間経過によりFinalセクションと判定
		m_CourseSection = E_COURSE_SECTION_FINAL;
		m_FinalSectionJudgeCount = 0;
	}

	return (ret);
}

void Decision::setFrontMoveCommand()
{
	char sendBuffer[10] = { 0 };

	// 先頭フレーム
	sendBuffer[0] = BUFFER1_TOP_FRAME;
	sendBuffer[1] = BUFFER2_SECOND_FRAME;

	// 指定コマンド

	// モーション再生
	sendBuffer[2] = BUFFER3_MOTION_PLAY;
	// 前進
	sendBuffer[3] = BUFFER4_DIRECTION_FRONT;

	// 左足 歩幅
	sendBuffer[4] = LEFT_STRIDE(10);

	// 右足 歩幅
	sendBuffer[5] = RIGHT_STRIDE(10);

	// 高さ
	sendBuffer[6] = 80;

	// 傾斜
	sendBuffer[7] = 0;

	// 速度
	sendBuffer[8] = 255;

	memcpy(m_DataSender->m_MotorMoveSendBuffer, sendBuffer, sizeof(sendBuffer));

	return;
}

void Decision::setRightMoveCommand()
{
	char sendBuffer[10] = { 0 };

	// 先頭フレーム
	sendBuffer[0] = BUFFER1_TOP_FRAME;
	sendBuffer[1] = BUFFER2_SECOND_FRAME;

	// 指定コマンド

	// モーション再生
	sendBuffer[2] = BUFFER3_MOTION_PLAY;
	// 前進
	sendBuffer[3] = BUFFER4_DIRECTION_FRONT;

	// 左足 歩幅
	sendBuffer[4] = LEFT_STRIDE(10);

	// 右足 歩幅
	sendBuffer[5] = RIGHT_STRIDE(1);

	// 高さ
	sendBuffer[6] = 80;

	// 傾斜
	sendBuffer[7] = 0;

	// 速度
	sendBuffer[8] = 255;

	memcpy(m_DataSender->m_MotorMoveSendBuffer, sendBuffer, sizeof(sendBuffer));

	return;
}

void Decision::setLeftMoveCommand()
{
	char sendBuffer[10] = { 0 };

	// 先頭フレーム
	sendBuffer[0] = BUFFER1_TOP_FRAME;
	sendBuffer[1] = BUFFER2_SECOND_FRAME;

	// 指定コマンド

	// モーション再生
	sendBuffer[2] = BUFFER3_MOTION_PLAY;
	// 前進
	sendBuffer[3] = BUFFER4_DIRECTION_FRONT;

	// 左足 歩幅
	sendBuffer[4] = LEFT_STRIDE(1);

	// 右足 歩幅
	sendBuffer[5] = RIGHT_STRIDE(10);

	// 高さ
	sendBuffer[6] = 80;

	// 傾斜
	sendBuffer[7] = 0;

	// 速度
	sendBuffer[8] = 255;

	memcpy(m_DataSender->m_MotorMoveSendBuffer, sendBuffer, sizeof(sendBuffer));

	return;
}
