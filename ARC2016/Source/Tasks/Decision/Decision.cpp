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
	// �������͌�ōl����

	// �}�C�R���Ƃ̃n�[�g�r�[�g�Ď�
	HeartBeatCheck();

	// �J���� & �Z���T���ɂ�铮�씻��y�юw��
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
			// ���Y�p�C���̃n�[�g�r�[�g�M���𔽓]������
			m_MyHeartBeatFlag ^= 1;

			// ���M�f�[�^�쐬
			// ���M�y�уt���O�X�V��DataSender�^�X�N�ɔC����
			char sendBuffer[10];

			sendBuffer[0] = 0;
			sendBuffer[1] = 0;
			sendBuffer[2] = 0;
			sendBuffer[3] = 0;		// �ǂ�����m_MyHeartBeatFlag������
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
			// ���Y�p�C����M��̃C���^�[�o�����ԓ��B�҂�, �������͉���󂫑҂�
			m_HeartBeatInterval++;
		}
	//}
}

// ����������g��Ȃ�
void Decision::ReadData()
{
	if (m_SendFlag == 1)
	{
		char reciveBuffer[10];	/* ��M�R�}���h�T�C�Y�ɍ��킹��K�v���� */

		memcpy(reciveBuffer, m_DataSender->m_HeartBeatReceiveBuffer, sizeof(reciveBuffer));

		if (reciveBuffer[0] == 0)
		{
			// ��M�f�[�^���Ȃ��̂ŉ������Ȃ�
		}
		else
		{
			// ��M�f�[�^�m�F
			if (reciveBuffer[0] != m_MiconHeartBeatFlag)
			{
				// �l�m�F�͕K�v�H

				// �}�C�R�����̃n�[�g�r�[�g�f�[�^���ϓ����Ă���΁A�n�[�g�r�[�g�X�V�ʒm�Ƃ݂Ȃ�
				m_MiconHeartBeatFlag = reciveBuffer[0];

				// ���M�t���O��0�ɖ߂��A���Y�p�C����̃n�[�g�r�[�g���M���s����悤�ɂ���
				m_SendFlag = 0;
				m_HeartBeatInterval = 0;
			}

			// �d��OFF I/O �m�F�@���ʂ̒ʐM����
			if (reciveBuffer[0] == 1111 )
			{
				// �d����OFF��΂悢�̂ŁA�����ł�����Ⴄ�H
				TimeSpan t;
				t.Duration = 0;
				ShutdownKind kind = ShutdownKind::Shutdown;
				ShutdownManager::BeginShutdown(kind, t);
			}

			// ����̎�M�f�[�^������
			memset(m_DataSender->m_HeartBeatReceiveBuffer, 0, sizeof(m_DataSender->m_HeartBeatReceiveBuffer));
		}
	}
}

void Decision::MoveTypeDecision()
{	
	// ���C���g���[�X���ʎ擾
	long moveFlag = GetMoveType();
	long moveLineTrace = ConversionLineTraceFlag(moveFlag);

	long moveIntegration = moveLineTrace;
	m_GyroStatus = E_GYRO_MODE_FLAT;

	if (m_SensorMonitor != nullptr)
	{
		// �W���C���Z���T�󋵊m�F
		m_GyroStatus = GyroCheck();

		// �����Z���T�󋵂ƃ��C���g���[�X���ʂɂ�蓮�쌈��
		long moveIntegration = DistanceCheck(moveLineTrace);

		// ��3�Z�N�V��������Final�Z�N�V�����Ɣ��肷��܂ł̃J�E���g
		if (m_CourseSection == E_COURSE_SECTION_THIRD)
		{
			m_FinalSectionJudgeCount++;
		}
	}

	// Motor�֎w��
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
	case PATTERN_FRONT_LEFT_15:              // �O�i���� 15��
	case PATTERN_FRONT_LEFT_30:              // �O�i���� 30��
	case PATTERN_FRONT_LEFT_45:              // �O�i���� 45��
	case PATTERN_FRONT_LEFT_90:              // �O�i���� 90��
	case PATTERN_TURN_LEFT_ONCE:             // ������ �P�� (15��)
	case PATTERN_TURN_LEFT_45:               // ������ 45��
	case PATTERN_TURN_LEFT_90:               // ������ 90��
		retVal = E_MOVE_TURNLEFT;
		break;
	case PATTERN_FRONT_RIGHT_15:             // �O�i���E 15��
	case PATTERN_FRONT_RIGHT_30:             // �O�i���E 30��
	case PATTERN_FRONT_RIGHT_45:             // �O�i���E 45��
	case PATTERN_FRONT_RIGHT_90:             // �O�i���E 90��
	case PATTERN_TURN_RIGHT_ONCE:            // �E���� �P�� (15��)
	case PATTERN_TURN_RIGHT_45:              // �E���� 45��
	case PATTERN_TURN_RIGHT_90:              // �E���� 90��
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

	// �e�����f�[�^�擾
	m_SensorMonitor->GetDistanceSensorValue(distanceValue);

	// �����f�[�^������ӏ󋵔���
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
			// �w�j���Ȃ��̂ŁA��芸�����R�[�X���l���E�ɋȂ��邱�Ƃɂ���
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

	// �e�����f�[�^��臒l�`�F�b�N�i�ߕӂɕǂ����邩�ǂ����j
	// �����ݒ肪0�̏ꍇ�͋����Z���T���𖳎�����
	m_FrontDistanceJudge = WallCheck(m_FrontDistanceJudgementValue, data[E_DISTANCE_ORDER_FRONT]);
	m_RightDistanceJudge = WallCheck(m_RightDistanceJudgementValue, data[E_DISTANCE_ORDER_RIGHT]);
	m_LeftDistanceJudge = WallCheck(m_LeftDistanceJudgementValue, data[E_DISTANCE_ORDER_LEFT]);

	// �ŏI�Z�N�V�����̏ꍇ�̓S�[��������s��
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

	// �󋵔���
	if (m_FrontDistanceJudge == true)
	{
		if (m_RightDistanceJudge == true)
		{
			if (m_LeftDistanceJudge == true)
			{
				// 3���� �ǗL
				ret = E_DISTANCE_JUDGE_ALL;
			}
			else
			{
				// �O�E�E�@�ǗL
				ret = E_DISTANCE_JUDGE_FRONT_RIGHT;
			}
		}
		else if (m_LeftDistanceJudge == true)
		{
			// �O�E�� �ǗL
			ret = E_DISTANCE_JUDGE_FRONT_LEFT;
		}
		else
		{
			// �O�̂ݕ�
			ret = E_DISTANCE_JUDGE_FRONT;
		}
	}
	else if (m_RightDistanceJudge == true)
	{
		if (m_LeftDistanceJudge == true)
		{
			// ���E �ǗL
			ret = E_DISTANCE_JUDGE_RIGHT_LEFT;
		}
		else
		{
			// �E�̂ݕ�
			ret = E_DISTANCE_JUDGE_RIGHT;
		}
	}
	else if (m_LeftDistanceJudge == true)
	{
		// ���̂ݕ�
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

	// �W���C���Z���T�̓ǂݒl�擾
	std::vector<ARC2016::Parts::GyroSensor::GyroDataStr> gyroValue;
	m_SensorMonitor->GetGyroSensorValue(gyroValue);

	// �e��Ԃ�臒l�`�F�b�N�i�⓹���ǂ����j
	if (gyroValue[0].AngleY > m_SlopeJudgementValue)
	{
		// �⓹
		ret = E_GYRO_MODE_SLOPE;
	}
	else
	{
		// �ʏ�
		ret = E_GYRO_MODE_FLAT;
	}

	// ���s���̃Z�N�V��������
	if (m_CourseSection == E_COURSE_SECTION_FIRST && ret == E_GYRO_MODE_SLOPE)
	{
		// ��2�Z�N�V�����ɓ������Ɣ���
		m_CourseSection = E_COURSE_SECTION_SECOND;
	}
	else if (m_CourseSection == E_COURSE_SECTION_SECOND && ret == E_GYRO_MODE_FLAT)
	{
		// ��3�Z�N�V�����ɓ������Ɣ���
		m_CourseSection = E_COURSE_SECTION_THIRD;
	}
	else if (m_CourseSection == E_COURSE_SECTION_THIRD && m_FinalSectionJudgeCount >= COURSE_SECTION_FINAL_JUDGE_TIME)
	{
		// ��3�Z�N�V�������B��A��莞�Ԍo�߂ɂ��Final�Z�N�V�����Ɣ���
		m_CourseSection = E_COURSE_SECTION_FINAL;
		m_FinalSectionJudgeCount = 0;
	}

	return (ret);
}

void Decision::setFrontMoveCommand()
{
	char sendBuffer[10] = { 0 };

	// �擪�t���[��
	sendBuffer[0] = BUFFER1_TOP_FRAME;
	sendBuffer[1] = BUFFER2_SECOND_FRAME;

	// �w��R�}���h

	// ���[�V�����Đ�
	sendBuffer[2] = BUFFER3_MOTION_PLAY;
	// �O�i
	sendBuffer[3] = BUFFER4_DIRECTION_FRONT;

	// ���� ����
	sendBuffer[4] = LEFT_STRIDE(10);

	// �E�� ����
	sendBuffer[5] = RIGHT_STRIDE(10);

	// ����
	sendBuffer[6] = 80;

	// �X��
	sendBuffer[7] = 0;

	// ���x
	sendBuffer[8] = 255;

	memcpy(m_DataSender->m_MotorMoveSendBuffer, sendBuffer, sizeof(sendBuffer));

	return;
}

void Decision::setRightMoveCommand()
{
	char sendBuffer[10] = { 0 };

	// �擪�t���[��
	sendBuffer[0] = BUFFER1_TOP_FRAME;
	sendBuffer[1] = BUFFER2_SECOND_FRAME;

	// �w��R�}���h

	// ���[�V�����Đ�
	sendBuffer[2] = BUFFER3_MOTION_PLAY;
	// �O�i
	sendBuffer[3] = BUFFER4_DIRECTION_FRONT;

	// ���� ����
	sendBuffer[4] = LEFT_STRIDE(10);

	// �E�� ����
	sendBuffer[5] = RIGHT_STRIDE(1);

	// ����
	sendBuffer[6] = 80;

	// �X��
	sendBuffer[7] = 0;

	// ���x
	sendBuffer[8] = 255;

	memcpy(m_DataSender->m_MotorMoveSendBuffer, sendBuffer, sizeof(sendBuffer));

	return;
}

void Decision::setLeftMoveCommand()
{
	char sendBuffer[10] = { 0 };

	// �擪�t���[��
	sendBuffer[0] = BUFFER1_TOP_FRAME;
	sendBuffer[1] = BUFFER2_SECOND_FRAME;

	// �w��R�}���h

	// ���[�V�����Đ�
	sendBuffer[2] = BUFFER3_MOTION_PLAY;
	// �O�i
	sendBuffer[3] = BUFFER4_DIRECTION_FRONT;

	// ���� ����
	sendBuffer[4] = LEFT_STRIDE(1);

	// �E�� ����
	sendBuffer[5] = RIGHT_STRIDE(10);

	// ����
	sendBuffer[6] = 80;

	// �X��
	sendBuffer[7] = 0;

	// ���x
	sendBuffer[8] = 255;

	memcpy(m_DataSender->m_MotorMoveSendBuffer, sendBuffer, sizeof(sendBuffer));

	return;
}
