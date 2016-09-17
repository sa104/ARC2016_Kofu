#include "pch.h"
#include "Source/Tasks/SensorMonitor/SensorMonitor.h"

using namespace ARC2016;
using namespace ARC2016::Parts;
using namespace ARC2016::Tasks;

SensorMonitor::SensorMonitor(std::vector<DistanceSensor*> distanceDev, std::vector<GyroSensor *> gyroDev)
: TaskBase("SensorMonitor", 100)
{
	for (auto itr = distanceDev.begin(); itr != distanceDev.end(); ++itr)
	{
		m_DistanceSensor.push_back(*itr);
		m_DistanceValue.push_back(0);
	}

	for (auto itr = gyroDev.begin(); itr != gyroDev.end(); ++itr)
	{
		m_GyroSensor.push_back(*itr);
		
		GyroSensor::GyroDataStr	value;
		value.AccelX = 0.0;
		value.AccelY = 0.0;
		value.AccelZ = 0.0;
		value.AngleX = 0.0;
		value.AngleY = 0.0;

		m_GyroValue.push_back(value);
	}
}

SensorMonitor::~SensorMonitor()
{
	for (auto itr = m_DistanceSensor.begin(); itr != m_DistanceSensor.end(); ++itr)
	{
		DistanceSensor* dev = *itr;
		delete dev;
	}

	for (auto itr = m_GyroSensor.begin(); itr != m_GyroSensor.end(); ++itr)
	{
		GyroSensor* dev = *itr;
		delete dev;
	}
}

void SensorMonitor::GetDistanceSensorValue(std::vector<long>& value)
{
	value = m_DistanceValue;
}

void SensorMonitor::GetGyroSensorValue(std::vector<ARC2016::Parts::GyroSensor::GyroDataStr>& value)
{
	value = m_GyroValue;
}

ResultEnum SensorMonitor::initialize()
{
	// 1 回目のデータ取得
	ResultEnum retVal = taskMain();
	return retVal;
}

ResultEnum SensorMonitor::taskMain()
{
	readDistanceSensor();
	readGyroSensor();

	return E_RET_NORMAL;
}

ResultEnum SensorMonitor::finalize()
{
	return E_RET_NORMAL;
}

void SensorMonitor::readDistanceSensor()
{
	long				value = 0;
	std::vector<long>	valueList;


	for (auto itr = m_DistanceSensor.begin(); itr != m_DistanceSensor.end(); ++itr)
	{
		DistanceSensor* dev = *itr;

		// 有効切り替え
		dev->Enable();

		// 有効に切り替えた後の待ち時間 (50msec)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		// データ取得
		if (dev->Get(value) != E_RET_NORMAL)
		{
			return;
		}

		valueList.push_back(value);

		// 無効切り替え
		dev->Disable();

		// 無効に切り替えた後の待ち時間 (50msec)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	m_DistanceValue = valueList;
}

void SensorMonitor::readGyroSensor()
{
	GyroSensor::GyroDataStr					value = { 0.0 };
	std::vector<GyroSensor::GyroDataStr>	valueList;


	for (auto itr = m_GyroSensor.begin(); itr != m_GyroSensor.end(); ++itr)
	{
		GyroSensor* dev = *itr;

		// データ取得
		if (dev->Get(value) != E_RET_NORMAL)
		{
			return;
		}

		valueList.push_back(value);
	}

	m_GyroValue = valueList;
}
