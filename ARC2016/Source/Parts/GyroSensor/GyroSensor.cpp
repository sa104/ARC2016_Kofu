#include "pch.h"
#include "Source/Parts/GyroSensor/GyroSensor.h"

using namespace ARC2016;
using namespace ARC2016::Parts;
using namespace concurrency;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::I2c;

GyroSensor::GyroSensor(I2cDevice^ i2c)
 : m_I2cDevice(i2c)
{
	// nop.
}

GyroSensor::~GyroSensor()
{
	// nop.
}

ResultEnum GyroSensor::Initialize()
{
	ResultEnum	retVal = E_RET_ABNORMAL;


	Platform::Array<byte>^	powerControl = ref new Platform::Array<byte>(2);
	Platform::Array<byte>^	adcControl = ref new Platform::Array<byte>(2);

	powerControl[0] = 0x20;		// ACCEL_REG_CONTROL1
	powerControl[1] = 0x27;

	adcControl[0] = 0x1F;		// ACCEL_REG_TEMP_CFG
	adcControl[1] = 0x80;

	try
	{
		m_I2cDevice->Write(powerControl);
		m_I2cDevice->Write(adcControl);
	}
	catch (...)
	{
		return E_RET_ABNORMAL;
	}

	retVal = E_RET_NORMAL;


	return retVal;
}

ResultEnum GyroSensor::Get(GyroDataStr& data)
{
	const int ACCEL_RES = 4095;								// The ADXL345 has 10 bit resolution giving 1024 unique values
	const int ACCEL_DYN_RANGE_G = 4;						// The ADXL345 had a total dynamic range of 8G, since we're configuring it to +-2G
	const int UNITS_PER_G = ACCEL_RES / ACCEL_DYN_RANGE_G;  // Ratio of raw int values to G units

	ResultEnum	retVal = E_RET_ABNORMAL;
	int			rawX = 0;
	int			rawY = 0;
	int			rawZ = 0;
	double		accelX = 0.0;
	double		accelY = 0.0;
	double		accelZ = 0.0;
	byte		rawData[2] = { 0 };


	// データ取得
	readData(0x28, rawData[0]);		// ACCEL_REG_X_L
	readData(0x29, rawData[1]);		// ACCEL_REG_X_H
	rawX = rawData[1];
	rawX <<= 8;
	rawX += rawData[0];
	rawX >>= 4;

	readData(0x2A, rawData[0]);		// ACCEL_REG_Y_L
	readData(0x2B, rawData[1]);		// ACCEL_REG_Y_H
	rawY = rawData[1];
	rawY <<= 8;
	rawY += rawData[0];
	rawY >>= 4;

	readData(0x2C, rawData[0]);		// ACCEL_REG_Z_L
	readData(0x2D, rawData[1]);		// ACCEL_REG_Z_H
	rawZ = rawData[1];
	rawZ <<= 8;
	rawZ += rawData[0];
	rawZ >>= 4;

	// In order to get the raw 16-bit data values, we need to concatenate two 8-bit bytes from the I2C read for each axis.
	// We accomplish this by using the BitConverter class.
	// Convert raw values to G's
	accelX = (double)rawX / UNITS_PER_G;
	accelY = (double)rawY / UNITS_PER_G;
	accelZ = (double)rawZ / UNITS_PER_G;

	// 加速度 (Low Pass Filter)
	//m_Value.AccelX = accelX * 0.05 + m_Value.AccelX * 0.95;
	//m_Value.AccelY = accelY * 0.05 + m_Value.AccelY * 0.95;
	//m_Value.AccelZ = accelZ * 0.05 + m_Value.AccelZ * 0.95;
	m_Value.AccelX = accelX;
	m_Value.AccelY = accelY;
	m_Value.AccelZ = accelZ;

	// 角度
	double rawSum = sqrt((rawX * rawX) + (rawY * rawY) + (rawZ * rawZ));

	if (rawSum != 0.0)
	{
		m_Value.AngleX = 90 - acos(rawX / rawSum) / M_PI * 180;
		m_Value.AngleY = 90 - acos(rawY / rawSum) / M_PI * 180;
	}
	else
	{
		m_Value.AngleX = 0.0;
		m_Value.AngleY = 0.0;
	}

	data = m_Value;

	retVal = E_RET_NORMAL;

	return retVal;
}

ResultEnum GyroSensor::readData(const byte address, byte& value)
{
	Platform::Array<byte>^ command = ref new Platform::Array<byte>(1);
	Platform::Array<byte>^ data = ref new Platform::Array<byte>(1);

	command[0] = address;

	try
	{
		m_I2cDevice->WriteRead(command, data);
	}
	catch (...)
	{
		return E_RET_ABNORMAL;
	}

	value = data[0];

	return E_RET_NORMAL;
}

//############################################
// For Simulation
//############################################
GyroSensorDummy::GyroSensorDummy(Windows::Devices::I2c::I2cDevice^ i2c)
 : GyroSensor(i2c)
{
	m_Value.AccelX = 1.0;
	m_Value.AccelY = 2.0;
	m_Value.AccelZ = 3.0;
	m_Value.AngleX = 4.0;
	m_Value.AngleY = 5.0;
}

GyroSensorDummy::~GyroSensorDummy()
{
	// nop.
}

ResultEnum GyroSensorDummy::Initialize()
{
	return E_RET_NORMAL;
}

ResultEnum GyroSensorDummy::Get(GyroDataStr& data)
{
	m_Value.AccelX += 0.1;
	m_Value.AccelY += 0.1;
	m_Value.AccelZ += 0.1;
	m_Value.AngleX += 0.1;
	m_Value.AngleY += 0.1;

	data = m_Value;

	return E_RET_NORMAL;
}
