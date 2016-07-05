#pragma once

#include "../Framework/TaskBase.h"

namespace ARC2016
{
	class Motor : public TaskBase
	{
	public:

		Motor();
		virtual ~Motor();

		long GetValue();

	protected:


	private:

		long	m_Value;

		ARC2016::ResultEnum initialize();
		ARC2016::ResultEnum taskMain();
		ARC2016::ResultEnum finalize();

	};
}
