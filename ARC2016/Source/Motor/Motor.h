#pragma once

#include "../Framework/LoopTaskBase.h"

namespace ARC2016
{
	class Motor : public ARC2016::Framework::TaskBase::LoopTaskBase
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
