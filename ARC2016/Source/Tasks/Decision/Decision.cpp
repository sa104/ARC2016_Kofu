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
	ReadData();
}

void Decision::SendData()
{
	if (m_SendFlag == 0)
	{
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
		}
		else
		{
			// ���Y�p�C����M��̃C���^�[�o�����ԓ��B�҂�
			m_HeartBeatInterval++;
		}
	}
}

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

	// �����Z���T�󋵂ƃ��C���g���[�X���ʂɂ�蓮�쌈��
	long moveIntegration = DistanceCheck(moveLineTrace);

	// �W���C���Z���T�󋵊m�F
	E_GYRO_MODE_TYPE gyroFlag = GyroCheck();

	// Motor�֎w��
	switch (moveLineTrace)
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
	bool frontFlag = false;
	bool rightFlag = false;
	bool leftFlag = false;

	// �e�����f�[�^��臒l�`�F�b�N�i�ߕӂɕǂ����邩�ǂ����j
	if (data[E_DISTANCE_ORDER_FRONT] <= m_FlontDistanceJudgementValue)
	{
		frontFlag = true;
	}
	if (data[E_DISTANCE_ORDER_RIGHT] <= m_RightDistanceJudgementValue)
	{
		rightFlag = true;
	}
	if (data[E_DISTANCE_ORDER_LEFT] <= m_LeftDistanceJudgementValue)
	{
		leftFlag = true;
	}

	// �󋵔���
	if (frontFlag == true)
	{
		if (rightFlag == true)
		{
			if (leftFlag == true)
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
		else if (leftFlag == true)
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
	else if (rightFlag == true)
	{
		if (leftFlag == true)
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
	else if (leftFlag == true)
	{
		// ���̂ݕ�
		ret = E_DISTANCE_JUDGE_LEFT;
	}

	return ret;
}

E_GYRO_MODE_TYPE Decision::GyroCheck()
{
	E_GYRO_MODE_TYPE ret = E_GYRO_MODE_FLAT;
	double nowSlopeDegree = 0;

	// �W���C���Z���T�̓ǂݒl�擾
	std::vector<ARC2016::Parts::GyroSensor::GyroDataStr> gyroValue;
	m_SensorMonitor->GetGyroSensorValue(gyroValue);

	// ���ȏ�
	nowSlopeDegree = fabs(gyroValue[0].AngleY) / 90;

	// �e��Ԃ�臒l�`�F�b�N�i�⓹���ǂ����j
	if (nowSlopeDegree > m_SlopeJudgementValue)
	{
		// �⓹
		ret = E_GYRO_MODE_SLOPE;
	}
	else
	{
		// �ʏ�
		ret = E_GYRO_MODE_FLAT;
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
