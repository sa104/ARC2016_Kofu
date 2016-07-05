#include "pch.h"
#include "Motor.h"

using namespace ARC2016;

Motor::Motor() : LoopTaskBase("Motor", 1000)
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
	m_Value++;
	return E_RET_NORMAL;
}

ResultEnum Motor::finalize()
{
	return E_RET_NORMAL;
}
