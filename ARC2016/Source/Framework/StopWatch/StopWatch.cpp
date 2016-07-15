#include "pch.h"
#include "StopWatch.h"

using namespace ARC2016;
using namespace ARC2016::Framework::Measure;
using namespace std::chrono;

StopWatch::StopWatch() : m_Running(false)
{
	// nop.
}

StopWatch::~StopWatch()
{
	// nop.
}

void StopWatch::Start()
{
	if (IsRunning() == false)
	{
		m_Running = true;
		m_StartTime = system_clock::now();
	}
}

void StopWatch::Stop()
{
	if (IsRunning() == true)
	{
		m_EndTime = system_clock::now();
		m_Running = false;
	}
}

bool StopWatch::IsRunning()
{
	return (m_Running);
}

long StopWatch::GetStoppedTime()
{
	long	lRet = 0;
	
	auto span = m_EndTime - m_StartTime;
	lRet = static_cast<long>(duration_cast<milliseconds>(span).count());
	
	return (lRet);
}

long StopWatch::GetLapTime()
{
	long	lRet = 0;

	system_clock::time_point nowTime = system_clock::now();

	auto span = nowTime - m_StartTime;
	lRet = static_cast<long>(duration_cast<milliseconds>(span).count());

	return (lRet);
}
