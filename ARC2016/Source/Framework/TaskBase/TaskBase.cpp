#include "pch.h"
#include <chrono>
#include <thread>
#include "Source/Common/Common.h"
#include "Source/Framework/StopWatch/StopWatch.h"
#include "Source/Framework/TaskBase/TaskBase.h"

using namespace std;
using namespace concurrency;
using namespace ARC2016::Framework;

TaskBase::TaskBase(string taskName, const long cycleMsec)
 : TASK_NAME(taskName)
 , PROC_CYCLE_MSEC(cycleMsec)
 , m_ProcResult(E_TASK_NOT_RUN)
{
	// nop.
}

TaskBase::~TaskBase()
{
	// nop.
}

void TaskBase::Start()
{
	if (IsRunning() == false)
	{
		m_TaskStop = false;
		m_TaskObject = create_task(taskProcedure());
	}
}

void TaskBase::Stop()
{
	if (IsRunning() == true)
	{
		m_TaskStop = true;
	}
}

TaskBase::TaskResultEnum TaskBase::GetResult()
{
	return m_ProcResult;
}

bool TaskBase::IsRunning()
{
	bool bRet = false;


	if (m_ProcResult == E_TASK_EXECUTING)
	{
		bRet = true;
	}


	return (bRet);
}

Windows::Foundation::IAsyncOperation<int>^ TaskBase::taskProcedure()
{
	return create_async([this]()
	{
		doProcedure();
		return 0;
	});
}

void TaskBase::doProcedure()
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

		// 終了処理は必ず行う
		finalize();

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