#include "pch.h"
#include "Source/Tasks/DataSender/DataSender.h"

using namespace ARC2016;
using namespace ARC2016::Framework;
using namespace ARC2016::Tasks;

DataSender::DataSender(Serial* device)
 : TaskBase("DataSender", 100)
 , m_SerialDevice(device)
{
	for (int i = 0; i < 10; i++)
	{
		m_HeartBeatSendBuffer[i] = 0;
		m_MotorMoveSendBuffer[i] = 0;
	}
	m_SendFlag = HEARTBEAT_SEND_START;
	m_MyHeartBeat = HEARTBEAT_OFF;
	m_BoardHeartBeat = HEARTBEAT_OFF;
	m_IsMoveSetted = false;
}

DataSender::~DataSender()
{
	if (m_SerialDevice != nullptr)
	{
		delete m_SerialDevice;
		m_SerialDevice = nullptr;
	}
}

ResultEnum DataSender::initialize()
{
	return E_RET_NORMAL;
}

ResultEnum DataSender::taskMain()
{
	sendData(&m_HeartBeatSendBuffer[0]);
	readData(&m_HeartBeatReceiveBuffer[0]);

	sendData(&m_MotorMoveSendBuffer[0]);
	readData(&m_MotorMoveReceiveBuffer[0]);

	return E_RET_NORMAL;
}

ResultEnum DataSender::finalize()
{
	return E_RET_NORMAL;
}

void DataSender::sendData(char* sendBuffer)
{
	if (sendBuffer[0] != 0)
	{
		// データ送信
		m_SerialDevice->Send(&sendBuffer[0], sizeof(sendBuffer));

		// 送信したのでデータクリア
		for (int count = 0; count < sizeof(sendBuffer); ++count)

		{
			m_HeartBeatSendBuffer[count] = 0;
		}
	}
}

void DataSender::readData(char* receiveBuffer)
{
	// 受信前にバッファクリア
	for (int count = 0; count < sizeof(receiveBuffer); ++count)
	{
		receiveBuffer[count] = 0;
	}

	// データ受信
	m_SerialDevice->Receive(&receiveBuffer[0], sizeof(receiveBuffer));
}