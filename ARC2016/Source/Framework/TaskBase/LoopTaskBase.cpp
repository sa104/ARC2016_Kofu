#include "pch.h"
#include <chrono>
#include <thread>
#include "Source/Common.h"
#include "Source/Framework/StopWatch/StopWatch.h"
#include "LoopTaskBase.h"

using namespace std;
using namespace concurrency;
using namespace ARC2016::Framework::TaskBase;
using namespace ARC2016::Framework::Measure;

LoopTaskBase::LoopTaskBase(string taskName, const long cycleMsec)
 : TASK_NAME(taskName)
 , PROC_CYCLE_MSEC(cycleMsec)
 , m_ProcResult(E_TASK_NOT_RUN)
{
	// nop.
}

LoopTaskBase::~LoopTaskBase()
{
	// nop.
}

void LoopTaskBase::Start()
{
	if (IsRunning() == false)
	{
		m_TaskStop = false;
		m_TaskObject = create_task(taskProcedure());
	}
}

void LoopTaskBase::Stop()
{
	if (IsRunning() == true)
	{
		m_TaskStop = true;
		m_TaskObject.then([this](int){});
	}
}

LoopTaskBase::TaskResultEnum LoopTaskBase::GetResult()
{
	return m_ProcResult;
}

bool LoopTaskBase::IsRunning()
{
	bool bRet = false;


	if (m_ProcResult == E_TASK_EXECUTING)
	{
		bRet = true;
	}


	return (bRet);
}

Windows::Foundation::IAsyncOperation<int>^ LoopTaskBase::taskProcedure()
{
	return create_async([this]()
	{
		doProcedure();
		return 0;
	});
}

void LoopTaskBase::doProcedure()
{
	ResultEnum	result = E_RET_ABNORMAL;
	long		lSpan = 0;
	long		lWaitTime = 0;
	StopWatch	watch;


	// タスク動作中
	m_ProcResult = E_TASK_EXECUTING;

	// 初期処理
	result = initialize();
	if (result != E_RET_NORMAL)
	{
		m_ProcResult = E_TASK_ABNORMAL_END;
		goto FINISH;
	}

	// メインループ
	while (m_TaskStop == false)
	{
		watch.Start();
		result = taskMain();
		watch.Stop();

		if (result != E_RET_NORMAL)
		{
			m_ProcResult = E_TASK_ABNORMAL_END;
			goto FINISH;
		}
		
		lWaitTime = PROC_CYCLE_MSEC - lSpan;
		if (lWaitTime <= 0)
		{
			lWaitTime = 1;
		}

		this_thread::sleep_for(std::chrono::milliseconds(lWaitTime));
	}

	// 終了処理
	result = finalize();
	if (result != E_RET_NORMAL)
	{
		m_ProcResult = E_TASK_ABNORMAL_END;
		goto FINISH;
	}

	m_ProcResult = E_TASK_NORMAL_END;


FINISH:
	return;
}