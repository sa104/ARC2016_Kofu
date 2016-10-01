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
	}
	m_SendFlag = HEARTBEAT_SEND_START;
	m_MyHeartBeat = HEARTBEAT_OFF;
	m_BoardHeartBeat = HEARTBEAT_OFF;
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
	// 差し当たりHEARTBEATだけ考える
	sendData();
	readData();





	// データ収集


	// 送信



	//char buffer[10] = { 0 };
	//buffer[0] = 1;
	//buffer[1] = 2;
	//buffer[2] = 3;
	//buffer[3] = 4;
	//buffer[4] = 5;
	//buffer[5] = 6;
	//buffer[6] = 7;
	//buffer[7] = 8;
	//buffer[8] = 9;
	//buffer[9] = 10;

	//m_SerialDevice->Send(&buffer[0], sizeof(buffer));


	//	char recvBuffer[10] = {0};
	//	m_SerialDevice->Receive(&recvBuffer[0], sizeof(recvBuffer));



	return E_RET_NORMAL;
}

ResultEnum DataSender::finalize()
{
	return E_RET_NORMAL;
}

void DataSender::sendData()
{
	// 未送信状態 且つ 送信データがある場合のみ送信実施
	if (m_SendFlag == HEARTBEAT_SEND_START && m_HeartBeatSendBuffer[0] != 0)
	{
		// データ送信
		m_SerialDevice->Send(&m_HeartBeatSendBuffer[0], sizeof(m_HeartBeatSendBuffer));

		// 送信したのでデータクリア
		for (int count = 0; count < 10; ++count)

		{
			m_HeartBeatSendBuffer[count] = 0;
		}

		m_SendFlag = HEARTBEAT_SEND_END;
	}
}

void DataSender::readData()
{
	if (m_SendFlag == HEARTBEAT_SEND_END)
	{
		// 受信前にバッファクリア
		for (int count = 0; count < 10; ++count)
		{
			m_HeartBeatReceiveBuffer[count] = 0;
		}

		// データ受信
		m_SerialDevice->Receive(&m_HeartBeatReceiveBuffer[0], sizeof(m_HeartBeatReceiveBuffer));
	}
}