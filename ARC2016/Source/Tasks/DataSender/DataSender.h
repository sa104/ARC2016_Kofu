#pragma once

#include "Source/Common/Common.h"
#include "Source/Framework/TaskBase/TaskBase.h"
#include "Source/Framework/Serial/Serial.h"

namespace ARC2016
{
	namespace Tasks
	{
		class DataSender : public ARC2016::Framework::TaskBase
		{
		public :
			DataSender(ARC2016::Framework::Serial* const device);
			virtual ~DataSender();

		protected :

		private  :

			ARC2016::Framework::Serial*	m_SerialDevice;

			ARC2016::ResultEnum initialize();
			ARC2016::ResultEnum taskMain();
			ARC2016::ResultEnum finalize();
		};
	}
}