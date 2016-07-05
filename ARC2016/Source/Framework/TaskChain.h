#pragma once

#include <list>
#include "Source/Common.h"
#include "ITaskControlable.h"

namespace ARC2016
{
	namespace Framework
	{
		namespace TaskBase
		{
			class ChainTaskOnce : public ITaskControlable
			{
			public:

				ChainTaskOnce();
				virtual ~ChainTaskOnce();

				void Start();
				bool IsRunning();
				TaskResultEnum GetResult();

			protected:

				virtual ARC2016::ResultEnum taskMain() = 0;

			private:

				TaskResultEnum	m_ProcResult;
			};

			class TaskChain
			{
			public:

				TaskChain operator+(TaskChain* p);
				TaskChain operator+=(TaskChain* p);

			protected:


			private:

				std::list<ChainTaskOnce*>	m_TaskList;
				long						m_ExecuteIndex;


			};


		}
	}
}
