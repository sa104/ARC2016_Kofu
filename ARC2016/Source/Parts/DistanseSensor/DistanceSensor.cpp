#include "pch.h"
#include "ppltasks.h"
#include "Source/Parts/DistanseSensor/DistanceSensor.h"

using namespace ARC2016;
using namespace ARC2016::Parts;
using namespace concurrency;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Gpio;
using namespace Windows::Devices::I2c;

DistanceSensor::DistanceSensor(I2cDevice^ i2c, GpioPin^ gpio)
 : m_I2cDevice(i2c)
 , m_GpioPin(gpio)
{
	// nop.
}

DistanceSensor::~DistanceSensor()
{
	Disable();
}

ResultEnum DistanceSensor::Get(long& distance)
{
	ResultEnum				retVal = E_RET_ABNORMAL;
	unsigned char			valueL = 0;
	unsigned char			valueH = 0;
	Platform::Array<byte>^	command = ref new Platform::Array<byte>(1);
	Platform::Array<byte>^	data = ref new Platform::Array<byte>(2);


	// ‰Šú‰»
	distance = LONG_MAX;

	if ((m_GpioPin == nullptr) || (m_I2cDevice == nullptr))
	{
		goto FINISH;
	}

	// Read from the accelerometer.
	// We call WriteRead() so we first write the address of the X - Axis I2C register, then read all 3 axes
	command[0] = 0x5E;

	try
	{
		m_I2cDevice->WriteRead(command, data);
	}
	catch (...)
	{
		distance = -1;
		return E_RET_NORMAL;
	}

	valueH = data[0];
	valueL = data[1];

	// Convert raw values to G's
	distance  = valueH;
	distance *= 16;
	distance += valueL;
	distance = distance / 16 / 4;

	retVal = E_RET_NORMAL;


FINISH :
	
	return retVal;
}

void DistanceSensor::Enable()
{
	m_GpioPin->Write(GpioPinValue::High);
}

void DistanceSensor::Disable()
{
	m_GpioPin->Write(GpioPinValue::Low);
}

//############################################
// For Simulation
//############################################
DistanceSensorDummy::DistanceSensorDummy(Windows::Devices::I2c::I2cDevice^ i2c, Windows::Devices::Gpio::GpioPin^ gpio)
 : DistanceSensor(i2c, gpio)
 , m_Value(0)
{
	// nop.
}

DistanceSensorDummy::~DistanceSensorDummy()
{
	// nop.
}

ResultEnum DistanceSensorDummy::Get(long& distance)
{
	distance = m_Value;
	m_Value++;
	return E_RET_NORMAL;
}

void DistanceSensorDummy::Enable()
{
	// nop.
}

void DistanceSensorDummy::Disable()
{
	// nop.
}
