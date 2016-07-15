#pragma once

#include "Source/Framework/LoopTaskBase.h"
#include "Source/Serial/Serial.h"

namespace ARC2016
{
	class Motor : public ARC2016::Framework::TaskBase::LoopTaskBase
	{
	public:

		Motor(Serial* const device);
		virtual ~Motor();

		long GetValue();

	protected:


	private:

		const int ANGLE_MAX = 135;
		const int ANGLE_MIN = -135;
		const int CMD_MAX = 11500;
		const int CMD_MIN = 3500;

		long			m_Value;
		Serial*			m_SerialDevice;

		ARC2016::ResultEnum initialize();
		ARC2016::ResultEnum taskMain();
		ARC2016::ResultEnum finalize();

	};
}
