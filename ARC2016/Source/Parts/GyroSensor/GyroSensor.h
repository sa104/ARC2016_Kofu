#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include "Source/Common/Common.h"

namespace ARC2016
{
	namespace Parts
	{
		class GyroSensor
		{
		public :

			typedef struct
			{
				double AngleX;
				double AngleY;
				double AccelX;
				double AccelY;
				double AccelZ;
			} GyroDataStr;

			GyroSensor(Windows::Devices::I2c::I2cDevice^ i2c);
			virtual ~GyroSensor();

			virtual ResultEnum Initialize();
			virtual ResultEnum Get(GyroDataStr& data);

		protected :

			GyroDataStr	m_Value;

		private :

			Windows::Devices::I2c::I2cDevice^	m_I2cDevice;

			ResultEnum readData(const byte address, byte& value);

		};

		class GyroSensorDummy : public GyroSensor
		{
		public :

			GyroSensorDummy(Windows::Devices::I2c::I2cDevice^ i2c);
			virtual ~GyroSensorDummy();

			virtual ResultEnum Initialize();
			virtual ResultEnum Get(GyroDataStr& data);

		protected:

		private:

		};
	}
}