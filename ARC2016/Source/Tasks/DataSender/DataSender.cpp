#include "pch.h"
#include "Source/Tasks/DataSender/DataSender.h"

using namespace ARC2016;
using namespace ARC2016::Framework;
using namespace ARC2016::Tasks;

DataSender::DataSender(Serial* device)
 : TaskBase("DataSender", 100)
 , m_SerialDevice(device)
{
	// nop.
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
