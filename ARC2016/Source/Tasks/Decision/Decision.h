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

typedef enum _E_COURSE_SECTION_TYPE
{
	E_COURSE_SECTION_FIRST = 0x00,	// ��1�Z�N�V����
	E_COURSE_SECTION_SECOND,		// ��2�Z�N�V�����i��j
	E_COURSE_SECTION_THIRD,			// ��3�Z�N�V����
	E_COURSE_SECTION_FINAL,			// �ŏI�Z�N�V�����@�i��3�Z�N�V�������B�����莞�Ԍo�߂Łj
} E_COURSE_SECTION_TYPE;

#define COURSE_SECTION_FINAL_JUDGE_TIME	(300)					/* ��3�Z�N�V�������B����Final�Z�N�V�������B�Ɣ��肷��܂ł̎��� */

#define RIGHTLEFT_MOVE_COUNT_MAX		(10)					/* �E���܌��m���ɁA��Ԃ�ێ�����ő�� */

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

			// �J����Pos
			char					m_CameraPosition;

			// �S�[������p
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

			// Goal�t���O
			bool					m_Goal;

			ResultEnum				initialize();
			ResultEnum				taskMain();
			ResultEnum				finalize();

			// �n�[�g�r�[�g�֘A
			void					HeartBeatCheck();
			void					SendData();
			void					ReadData();

			// ���씻��֘A
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

			// �E���܌��m��
			int						m_RightMoveFlag;	// �E�܌��m�J�E���^
			int						m_LeftMoveFlag;		// ���܌��m�J�E���^
			int						m_RightMoveCount;	// �E�܉񐔃J�E���^
			int						m_LeftMoveCount;	// ���܉񐔃J�E���^
			long					moveTurnRetention(long moveFlag);

			// �J�����ʒu
			char					m_BeforeCameraPosition;	// ����ʒu�ւ̃R�}���h�͓����Ȃ��悤�ێ����Ă���
			DataSender::E_COMMAND_DATA_SEND_TYPE m_CommandStatus;	// �J�����쓮�R�}���h���s���́A���[�^�ւ̃R�}���h�ŏ����Ԃ��Ȃ��悤�Ɋ�����҂�����

			// ���[�^�[�쓮�R�}���h
			void					setFrontMoveCommand();
			void					setRightMoveCommand();
			void					setLeftMoveCommand();
			void					setBackMoveCommand();
			void					setCameraMoveCommand(char vertical);

			void					delayCommandSend();	// �J�����R�}���h�����s�����ꍇ�́A���[�^�R�}���h�i�[�O��Delay�֐�

		};
	}
}