#pragma once

namespace ARC2016
{
	class ITaskControlable abstract
	{
	public :

		typedef enum
		{
			E_TASK_NOT_RUN = 0,
			E_TASK_EXECUTING,
			E_TASK_NORMAL_END,
			E_TASK_ABNORMAL_END,
		} TaskResultEnum;

		virtual void Start() = 0;
		virtual bool IsRunning() = 0;
		virtual TaskResultEnum GetResult() = 0;

	};
};
