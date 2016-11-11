#pragma once

#include "Source/Common/Common.h"
#include "Source/Framework/TaskBase/TaskBase.h"
#include "Source/Framework/Serial/Serial.h"
#include "Source/Tasks/Camera/Camera.h"
#include "Source/Tasks/DataSender/DataSender.h"
#include "Source/Tasks/SensorMonitor/SensorMonitor.h"
#include "Source/Parts/GyroSensor/GyroSensor.h"

typedef enum _E_DISTANCE_DATA_ORDER_TYPE
{
	E_DISTANCE_ORDER_FRONT = 0x00,
	E_DISTANCE_ORDER_RIGHT,
	E_DISTANCE_ORDER_LEFT,
} E_DISTANCE_DATA_ORDER_TYPE;

typedef enum _E_DISTANCE_JUDGE_TYPE
{
	E_DISTANCE_JUDGE_NONE = 0x00,	// 壁無し
	E_DISTANCE_JUDGE_ALL,			// 3方向  壁有
	E_DISTANCE_JUDGE_FRONT_RIGHT,	// 前・右 壁有
	E_DISTANCE_JUDGE_FRONT_LEFT,	// 前・左 壁有
	E_DISTANCE_JUDGE_RIGHT_LEFT,	// 右・左 壁有
	E_DISTANCE_JUDGE_FRONT,			// 前     壁有
	E_DISTANCE_JUDGE_RIGHT,			// 右     壁有
	E_DISTANCE_JUDGE_LEFT,			// 左     壁有
} E_DISTANCE_JUDGE_TYPE;

typedef enum _E_GYRO_MODE_TYPE
{
	E_GYRO_MODE_UNKNOWN = 0x00,
	E_GYRO_MODE_FLAT,
	E_GYRO_MODE_SLOPE,
} E_GYRO_MODE_TYPE;

typedef enum _E_COURSE_SECTION_TYPE
{
	E_COURSE_SECTION_FIRST = 0x00,	// 第1セクション
	E_COURSE_SECTION_SECOND,		// 第2セクション（坂）
	E_COURSE_SECTION_THIRD,			// 第3セクション
	E_COURSE_SECTION_FINAL,			// 最終セクション　（第3セクション到達から一定時間経過で）
} E_COURSE_SECTION_TYPE;

#define COURSE_SECTION_FINAL_JUDGE_TIME	(300)					/* 第3セクション到達からFinalセクション到達と判定するまでの時間 */

#define RIGHTLEFT_MOVE_COUNT_MAX		(10)					/* 右左折検知時に、状態を保持する最大回数 */

// buffer[0]
#define BUFFER1_TOP_FRAME				(0xFF)

// buffer[1]
#define BUFFER2_SECOND_FRAME			(0x3)

// buffer[2]
#define	BUFFER3_SERVO_POSITION_SET		(2)						/* 全サーボ位置設定 */
#define BUFFER3_CAMERA_POSITION_SET		(3)						/* カメラ位置設定 */
#define BUFFER3_ARM_POSITION_SET		(4)						/* アーム位置設定 */
#define BUFFER3_MOTION_PLAY				(10)					/* モーション再生 */
#define BUFFER3_MOTION_STOP				(11)					/* モーション停止 */
#define BUFFER3_SERVO_OFF				(100)					/* サーボOFF */
#define BUFFER3_STATUS_SEND_REQUEST		(254)					/* ステータス送信要求 */

// buffer[3]
#define BUFFER4_DIRECTION_FRONT			(0)
#define BUFFER4_DIRECTION_BACK			(1)

#define LEFT_STRIDE(x)					(x*12)
#define RIGHT_STRIDE(x)				(x*12)

namespace ARC2016
{
	namespace Tasks
	{
		class Decision : public ARC2016::Framework::TaskBase
		{
		public:
			Decision(SensorMonitor* sensMonitor, DataSender* dataSender);
			virtual ~Decision();

			// センサデータから状態を判定する為の条件設定値
			long					m_FrontDistanceJudgementValue;
			long					m_RightDistanceJudgementValue;
			long					m_LeftDistanceJudgementValue;
			double					m_SlopeJudgementValue;

			// カメラPos
			char					m_CameraPosition;

			// ゴール判定用
			long					m_FrontDistanceGoalJudgementValue;
			long					m_RightDistanceGoalJudgementValue;
			long					m_LeftDistanceGoalJudgementValue;


		protected:

		private:

			SensorMonitor*			m_SensorMonitor;
			DataSender*				m_DataSender;

			int						m_SendFlag;
			int						m_MyHeartBeatFlag;
			int						m_MiconHeartBeatFlag;
			int						m_HeartBeatInterval;

			// Goalフラグ
			bool					m_Goal;

			ResultEnum				initialize();
			ResultEnum				taskMain();
			ResultEnum				finalize();

			// ハートビート関連
			void					HeartBeatCheck();
			void					SendData();
			void					ReadData();

			// 動作判定関連
			E_COURSE_SECTION_TYPE	m_CourseSection;
			E_GYRO_MODE_TYPE		m_GyroStatus;
			int						m_FinalSectionJudgeCount;
			bool					m_FrontDistanceJudge;
			bool					m_RightDistanceJudge;
			bool					m_LeftDistanceJudge;
			long					ConversionLineTraceFlag(long flag);
			void				    MoveTypeDecision();
			long					DistanceCheck(long typeLineTrace);
			E_DISTANCE_JUDGE_TYPE	DistanceSensorCheck(std::vector<long> data);
			bool					WallCheck(long judgeValue, long data);
			E_GYRO_MODE_TYPE		GyroCheck();

			// 右左折検知時
			int						m_RightMoveFlag;	// 右折検知カウンタ
			int						m_LeftMoveFlag;		// 左折検知カウンタ
			int						m_RightMoveCount;	// 右折回数カウンタ
			int						m_LeftMoveCount;	// 左折回数カウンタ
			long					moveTurnRetention(long moveFlag);

			// カメラ位置
			char					m_BeforeCameraPosition;	// 同一位置へのコマンドは投げないよう保持しておく
			DataSender::E_COMMAND_DATA_SEND_TYPE m_CommandStatus;	// カメラ駆動コマンド実行時は、モータへのコマンドで書きつぶさないように完了を待たせる

			// モーター駆動コマンド
			void					setFrontMoveCommand();
			void					setRightMoveCommand();
			void					setLeftMoveCommand();
			void					setBackMoveCommand();
			void					setCameraMoveCommand(char vertical);

			void					delayCommandSend();	// カメラコマンドを実行した場合の、モータコマンド格納前のDelay関数

		};
	}
}