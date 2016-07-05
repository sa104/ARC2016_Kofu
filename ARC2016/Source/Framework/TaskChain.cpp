#include "pch.h"
#include "Source/Common.h"
#include "TaskChain.h"

using namespace std;
using namespace ARC2016::Framework::TaskBase;

ChainTaskOnce::ChainTaskOnce()
 : m_ProcResult(E_TASK_NOT_RUN)
{
	// nop.
}

ChainTaskOnce::~ChainTaskOnce()
{
	// nop.
}

void ChainTaskOnce::Start()
{
	TaskResultEnum	result = E_TASK_ABNORMAL_END;

	m_ProcResult = E_TASK_EXECUTING;

	if (taskMain() != E_RET_NORMAL)
	{
		goto FINISH;
	}

FINISH :

	m_ProcResult = result;
	return;
}

bool ChainTaskOnce::IsRunning()
{
	bool	bRet = false;

	if (m_ProcResult == E_TASK_EXECUTING)
	{
		bRet = true;
	}


	return (bRet);
}

ChainTaskOnce::TaskResultEnum ChainTaskOnce::GetResult()
{
	return m_ProcResult;
}

