#pragma once

#include "Source/Common/Common.h"
#include "Source/Framework/TaskBase/TaskBase.h"
#include "Source/Framework/Serial/Serial.h"

namespace ARC2016
{
	namespace Tasks
	{
		class DataSender : public ARC2016::Framework::TaskBase
		{
		public :
			DataSender(ARC2016::Framework::Serial* const device);
			virtual ~DataSender();

			enum HeartBeatCommunicationEnum
			{
				HEARTBEAT_SEND_START = 0,
				HEARTBEAT_SEND_END,
			};

			enum HeartBeatStatusEnum
			{
				HEARTBEAT_OFF = 0,
				HEARTBEAT_ON,
			};

			HeartBeatCommunicationEnum m_SendFlag;	// �n�[�g�r�[�g���M�t���O 0:�����M��ԁi���^�C�~���O�Ƀf�[�^���M�j�A1:���M�ς݁j
			HeartBeatStatusEnum m_MyHeartBeat;		// ���Y�p�C��̃n�[�g�r�[�g �X�e�[�^�X
			HeartBeatStatusEnum m_BoardHeartBeat;	// board���̃n�[�g�r�[�g �X�e�[�^�X

			char m_HeartBeatSendBuffer[10];		// ���M�o�b�t�@
			char m_HeartBeatReceiveBuffer[10];	// ��M�o�b�t�@

			char m_MotorMoveSendBuffer[10];
			char m_MotorMoveReceiveBuffer[10];
			bool m_IsMoveSetted;

		protected :

		private  :
			static const int HEARTBEAT_INTERVAL = 30;	// 100ms�P�ʁB�@3�b�Ԋu��ON/OFF�@�P�T�C�N��12�b

			long	heartBeat_SendTimeCount;	// �n�[�g�r�[�g���M�Ԋu�J�E���^

			ARC2016::Framework::Serial*	m_SerialDevice;

			ARC2016::ResultEnum initialize();
			ARC2016::ResultEnum taskMain();
			ARC2016::ResultEnum finalize();

			void sendData(char* sendBuffer);
			void readData(char* receiveBuffer);
		};
	}
}