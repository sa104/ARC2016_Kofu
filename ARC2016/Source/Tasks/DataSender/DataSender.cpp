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
		m_MotorSendedBuffer[i] = 0;
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
	ResultEnum result = E_RET_ABNORMAL;

	if (m_SerialDevice->m_InitializeComplete == false)
	{
		goto FINISH;
	}

	//sendData(&m_HeartBeatSendBuffer[0], sizeof(m_HeartBeatSendBuffer));

	result = sendData(&m_MotorMoveSendBuffer[0], sizeof(m_MotorMoveSendBuffer));
	if (result == E_RET_NORMAL)
	{
		//readData(&m_MotorMoveReceiveBuffer[0], 1);
	}

FINISH:

	return E_RET_NORMAL;
}

ResultEnum DataSender::finalize()
{
	return E_RET_NORMAL;
}

ResultEnum DataSender::sendData(unsigned char *sendBuffer, const long size)
{
	ResultEnum retVal = E_RET_ABNORMAL;
	ResultEnum result = E_RET_ABNORMAL;

	
	if ((sendBuffer[0] != 0) && (memcmp(m_MotorSendedBuffer, sendBuffer, 10) != 0))
	{
		// データ送信
		result = m_SerialDevice->Send(&sendBuffer[0], size);
		if (result == E_RET_NORMAL)
		{
			memcpy(m_MotorSendedBuffer, sendBuffer, 10);

			// 送信したのでデータクリア
			for (int count = 0; count < size; ++count)

			{
				sendBuffer[count] = 0;
			}
		}
		else
		{
			retVal = result;
			goto FINISH;
		}
	}

	retVal = E_RET_NORMAL;

FINISH:
	return retVal;
}

ResultEnum DataSender::readData(unsigned char *receiveBuffer, const long size)
{
	ResultEnum retVal = E_RET_ABNORMAL;

	// 受信前にバッファクリア
	for (int count = 0; count < size; ++count)
	{
		receiveBuffer[count] = 0;
	}

	// データ受信
	retVal = m_SerialDevice->Receive(&receiveBuffer[0], size);

	return retVal;
}