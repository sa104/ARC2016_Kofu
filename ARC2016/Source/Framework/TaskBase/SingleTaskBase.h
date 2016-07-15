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
			class SingleTaskBase : public ITaskControlable
			{
			public:

				SingleTaskBase();
				virtual ~SingleTaskBase();

				void Start();
				bool IsRunning();
				TaskResultEnum GetResult();

			protected:

				virtual ARC2016::ResultEnum initialize() = 0;
				virtual ARC2016::ResultEnum taskMain() = 0;
				virtual ARC2016::ResultEnum finalize() = 0;

			private:

				concurrency::task<int>	m_TaskObject;
				TaskResultEnum			m_ProcResult;

				Windows::Foundation::IAsyncOperation<int>^ taskProcedure();
				void doProcedure();

			};
		}
	}
}