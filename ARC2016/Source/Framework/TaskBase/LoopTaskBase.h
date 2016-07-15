#pragma once

#include "ppltasks.h"
#include "Source/Common.h"
#include "ITaskControlable.h"

namespace ARC2016
{
	namespace Framework
	{
		namespace TaskBase
		{
			class LoopTaskBase : public ITaskControlable
			{
			public:

				LoopTaskBase(std::string taskName, const long cycleMsec);
				virtual ~LoopTaskBase();

				void Start();
				void Stop();
				bool IsRunning();
				TaskResultEnum GetResult();

			protected:

				virtual ARC2016::ResultEnum initialize() = 0;
				virtual ARC2016::ResultEnum taskMain() = 0;
				virtual ARC2016::ResultEnum finalize() = 0;

			private:

				const long			PROC_CYCLE_MSEC;
				const std::string	TASK_NAME;

				bool					m_TaskStop;
				TaskResultEnum			m_ProcResult;
				concurrency::task<int>	m_TaskObject;

				Windows::Foundation::IAsyncOperation<int>^	taskProcedure();
				void doProcedure();

			};
		}
	}
}
