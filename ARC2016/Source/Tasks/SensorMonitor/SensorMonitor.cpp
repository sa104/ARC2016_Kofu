#include "pch.h"
#include "Source/Tasks/SensorMonitor/SensorMonitor.h"

using namespace ARC2016;
using namespace ARC2016::Parts;
using namespace ARC2016::Tasks;
using namespace concurrency;
using namespace Windows::Storage;

SensorMonitor::SensorMonitor(std::vector<DistanceSensor*> distanceDev, std::vector<GyroSensor *> gyroDev)
: TaskBase("SensorMonitor", 100)
{
	for (auto itr = distanceDev.begin(); itr != distanceDev.end(); ++itr)
	{
		m_DistanceSensor.push_back(*itr);
		m_DistanceValue.push_back(0);
		m_DistanceValue.push_back(0);

		m_DistanceData.push_back(m_DistanceValue);
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

		m_GyroData.push_back(m_GyroValue);

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
	StorageFolder^ folder = ApplicationData::Current->LocalFolder;
	StorageFile^ file;

	task<StorageFile^> asyncTask = create_task(folder->CreateFileAsync("SensorLog.txt", CreationCollisionOption::ReplaceExisting));
	try
	{
		asyncTask.wait();
		file = asyncTask.get();
	}
	catch (...)
	{
		/* nop */
	}
	
	Streams::IRandomAccessStream^ stream;
	task<Streams::IRandomAccessStream^> asyncOpenTask = create_task(file->OpenAsync(FileAccessMode::ReadWrite));
	try
	{
		asyncOpenTask.wait();
		stream = (Streams::IRandomAccessStream^)asyncOpenTask.get();
	}
	catch (...)
	{
		/* nop */
	}

	try
	{
		Streams::IOutputStream^ outputStream = (Streams::IOutputStream^)stream->GetOutputStreamAt(0);
		try
		{
			Streams::DataWriter^ dataWriter = ref new Streams::DataWriter(outputStream);

			std::vector<std::vector<ARC2016::Parts::GyroSensor::GyroDataStr>>::iterator itGyro = m_GyroData.begin();
			while (itGyro != m_GyroData.end())
			{
				std::vector<ARC2016::Parts::GyroSensor::GyroDataStr> data = *itGyro;

				wchar_t info[100] = { 0 };
				wchar_t formatStr[64] = L"Gyro  accelX=%s, accelY=%s, accelZ=%s, angleX=%lf, angleY=%lf \n";

				swprintf(info, size_t(100), formatStr,
					data[0].AccelX.ToString(), data[0].AccelY.ToString(), data[0].AccelZ.ToString(),
					data[0].AngleX.ToString(), data[0].AngleY.ToString());

				Platform::String^ str = ref new Platform::String(info);
				dataWriter->WriteString(str);

				++itGyro;
			}

			std::vector<std::vector<long>>::iterator itDis = m_DistanceData.begin();
			while (itDis != m_DistanceData.end())
			{
				std::vector<long> data = *itDis;
				wchar_t info[100] = { 0 };
				wchar_t formatStr[64] = L"Distance  Data1=%s, Data2=%s \n";

				swprintf(info, size_t(100), formatStr, data[0].ToString(), data[1].ToString());

				Platform::String^ str = ref new Platform::String(info);
				dataWriter->WriteString(str);

				++itDis;
			}

			task<size_t> asyncWriteTask = create_task(dataWriter->StoreAsync());
			try
			{
				asyncWriteTask.wait();
				if (asyncWriteTask.get() < 0)
				{
					/* ファイル書き込み失敗 */
				}
				else
				{
					/* ファイル書き込み成功 */
				}
			}
			catch (...)
			{
				/* nop */
			}

		}
		catch (...)
		{

		}
	}
	catch (...)
	{

	}

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
	m_DistanceData.push_back(valueList);
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
	m_GyroData.push_back(valueList);
}
