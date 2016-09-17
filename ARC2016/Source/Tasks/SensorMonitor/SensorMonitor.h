#pragma once

#include "Source/Framework/TaskBase/TaskBase.h"
#include "Source/Parts/DistanseSensor/DistanceSensor.h"
#include "Source/Parts/GyroSensor/GyroSensor.h"

namespace ARC2016
{
	namespace Tasks
	{
		class SensorMonitor : public ARC2016::Framework::TaskBase
		{
		public :

			SensorMonitor(std::vector<ARC2016::Parts::DistanceSensor*> distanceDev, std::vector<ARC2016::Parts::GyroSensor *> gyroDev);
			virtual ~SensorMonitor();
			void GetDistanceSensorValue(std::vector<long>& value);
			void GetGyroSensorValue(std::vector<ARC2016::Parts::GyroSensor::GyroDataStr>& value);

		protected :

		private :

			std::vector<ARC2016::Parts::DistanceSensor*>			m_DistanceSensor;
			std::vector<ARC2016::Parts::GyroSensor *>				m_GyroSensor;
			std::vector<long>										m_DistanceValue;
			std::vector<ARC2016::Parts::GyroSensor::GyroDataStr>	m_GyroValue;

			ResultEnum initialize();
			ResultEnum taskMain();
			ResultEnum finalize();

			void readDistanceSensor();
			void readGyroSensor();

		};

	}
}