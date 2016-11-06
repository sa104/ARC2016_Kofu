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

			typedef enum _E_COMMAND_DATA_SEND_TYPE
			{
				E_COMMAND_DATA_NONE = 0x00,
				E_COMMAND_DATA_NOT_SEND,
				E_COMMAND_DATA_SEND,
			} E_COMMAND_DATA_SEND_TYPE;

			void SetCommandSendStatus(E_COMMAND_DATA_SEND_TYPE flag);
			E_COMMAND_DATA_SEND_TYPE CommandSendStatusCheck();			// Decisionタスクへ送信状況を通知するための関数

			HeartBeatCommunicationEnum m_SendFlag;	// ハートビート送信フラグ 0:未送信状態（次タイミングにデータ送信）、1:送信済み）
			HeartBeatStatusEnum m_MyHeartBeat;		// ラズパイ川のハートビート ステータス
			HeartBeatStatusEnum m_BoardHeartBeat;	// board側のハートビート ステータス

			unsigned char m_HeartBeatSendBuffer[10];		// 送信バッファ
			unsigned char m_HeartBeatReceiveBuffer[10];	// 受信バッファ

			unsigned char m_MotorMoveSendBuffer[10];
			unsigned char m_MotorMoveReceiveBuffer[10];
			unsigned char m_MotorSendedBuffer[10];
			bool m_IsMoveSetted;

		protected :

		private  :
			static const int HEARTBEAT_INTERVAL = 30;	// 100ms単位。　3秒間隔でON/OFF　１サイクル12秒

			long	heartBeat_SendTimeCount;	// ハートビート送信間隔カウンタ
			
			E_COMMAND_DATA_SEND_TYPE	sendStatus;		// コマンド送信ステータス

			ARC2016::Framework::Serial*	m_SerialDevice;

			ARC2016::ResultEnum initialize();
			ARC2016::ResultEnum taskMain();
			ARC2016::ResultEnum finalize();

			ARC2016::ResultEnum sendData(unsigned char *sendBuffer, const long size);
			ARC2016::ResultEnum readData(unsigned char *receiveBuffer, const long size);
		};
	}
}