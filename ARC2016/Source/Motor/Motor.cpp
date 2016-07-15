#include "pch.h"
#include "Motor.h"

using namespace ARC2016;


Motor::Motor(Serial* const device)
 : LoopTaskBase("Motor", 30)
 , m_SerialDevice(device)
{
	// nop.
}

Motor::~Motor()
{
	// nop.
}

long Motor::GetValue()
{
	return m_Value;
}

ResultEnum Motor::initialize()
{
	m_Value = 0;
	return E_RET_NORMAL;
}

ResultEnum Motor::taskMain()
{
	char buffer[10] = {0};
	buffer[0] = 1;
	buffer[1] = 2;
	buffer[2] = 3;
	buffer[3] = 4;
	buffer[4] = 5;
	buffer[5] = 6;
	buffer[6] = 7;
	buffer[7] = 8;
	buffer[8] = 9;
	buffer[9] = 10;

	m_SerialDevice->Send(&buffer[0], sizeof(buffer));


//	char recvBuffer[10] = {0};
//	m_SerialDevice->Receive(&recvBuffer[0], sizeof(recvBuffer));

	

	return E_RET_NORMAL;
}

ResultEnum Motor::finalize()
{
	return E_RET_NORMAL;
}
