#pragma once

#include "Source/Common/Common.h"

namespace ARC2016
{
	namespace Parts
	{
		class DistanceSensor
		{
		public :
			DistanceSensor(Windows::Devices::I2c::I2cDevice^ i2c, Windows::Devices::Gpio::GpioPin^ gpio);
			virtual ~DistanceSensor();

			virtual ResultEnum Get(long& distance);
			virtual void Enable();
			virtual void Disable();

		protected :

		private :

			Windows::Devices::Gpio::GpioPin^	m_GpioPin;
			Windows::Devices::I2c::I2cDevice^	m_I2cDevice;

		};

		class DistanceSensorDummy : public DistanceSensor
		{
		public :
			DistanceSensorDummy(Windows::Devices::I2c::I2cDevice^ i2c, Windows::Devices::Gpio::GpioPin^ gpio);
			virtual ~DistanceSensorDummy();

			ResultEnum Get(long& distance);
			void Enable();
			void Disable();

		protected :

		private :

			long	m_Value;

		};
	}
}