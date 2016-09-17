#pragma once

#include "ppltasks.h"
#include "Source/Common/Common.h"

namespace ARC2016
{
	namespace Framework
	{
		class TaskBase
		{
		public:

			typedef enum
			{
				E_TASK_NOT_RUN = 0,
				E_TASK_EXECUTING,
				E_TASK_NORMAL_END,
				E_TASK_ABNORMAL_END,
			} TaskResultEnum;

			TaskBase(std::string taskName, const long cycleMsec);
			virtual ~TaskBase();

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
