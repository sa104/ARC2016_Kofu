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
	E_DISTANCE_JUDGE_NONE = 0x00,	// �ǖ���
	E_DISTANCE_JUDGE_ALL,			// 3����  �ǗL
	E_DISTANCE_JUDGE_FRONT_RIGHT,	// �O�E�E �ǗL
	E_DISTANCE_JUDGE_FRONT_LEFT,	// �O�E�� �ǗL
	E_DISTANCE_JUDGE_RIGHT_LEFT,	// �E�E�� �ǗL
	E_DISTANCE_JUDGE_FRONT,			// �O     �ǗL
	E_DISTANCE_JUDGE_RIGHT,			// �E     �ǗL
	E_DISTANCE_JUDGE_LEFT,			// ��     �ǗL
} E_DISTANCE_JUDGE_TYPE;

typedef enum _E_GYRO_MODE_TYPE
{
	E_GYRO_MODE_UNKNOWN = 0x00,
	E_GYRO_MODE_FLAT,
	E_GYRO_MODE_SLOPE,
} E_GYRO_MODE_TYPE;

// buffer[0]
#define BUFFER1_TOP_FRAME				(0xFF)

// buffer[1]
#define BUFFER2_SECOND_FRAME			(0x3)

// buffer[2]
#define	BUFFER3_SERVO_POSITION_SET		(2)						/* �S�T�[�{�ʒu�ݒ� */
#define BUFFER3_CAMERA_POSITION_SET		(3)						/* �J�����ʒu�ݒ� */
#define BUFFER3_ARM_POSITION_SET		(4)						/* �A�[���ʒu�ݒ� */
#define BUFFER3_MOTION_PLAY				(10)					/* ���[�V�����Đ� */
#define BUFFER3_MOTION_STOP				(11)					/* ���[�V������~ */
#define BUFFER3_SERVO_OFF				(100)					/* �T�[�{OFF */
#define BUFFER3_STATUS_SEND_REQUEST		(254)					/* �X�e�[�^�X���M�v�� */

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

			// �Z���T�f�[�^�����Ԃ𔻒肷��ׂ̏����ݒ�l
			long					m_FrontDistanceJudgementValue;
			long					m_RightDistanceJudgementValue;
			long					m_LeftDistanceJudgementValue;
			double					m_SlopeJudgementValue;


		protected:

		private:

			SensorMonitor*			m_SensorMonitor;
			DataSender*				m_DataSender;

			int						m_SendFlag;
			int						m_MyHeartBeatFlag;
			int						m_MiconHeartBeatFlag;
			int						m_HeartBeatInterval;

			ResultEnum				initialize();
			ResultEnum				taskMain();
			ResultEnum				finalize();

			// �n�[�g�r�[�g�֘A
			void					HeartBeatCheck();
			void					SendData();
			void					ReadData();

			// ���씻��֘A
			long					ConversionLineTraceFlag(long flag);
			void				    MoveTypeDecision();
			long					DistanceCheck(long typeLineTrace);
			E_DISTANCE_JUDGE_TYPE	DistanceSensorCheck(std::vector<long> data);
			E_GYRO_MODE_TYPE		GyroCheck();

			// ���[�^�[�쓮�R�}���h
			void					setFrontMoveCommand();
			void					setRightMoveCommand();
			void					setLeftMoveCommand();

		};
	}
}