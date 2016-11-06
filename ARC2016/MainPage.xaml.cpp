//
// MainPage.xaml.cpp
// MainPage クラスの実装。
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Source/Tasks/Camera/Camera.h"

using namespace ARC2016;
using namespace ARC2016::Framework;
using namespace ARC2016::Parts;
using namespace ARC2016::Tasks;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Gpio;
using namespace Windows::Devices::I2c;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media;
using namespace Windows::Media::Capture;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage::Streams;
using namespace Windows::System;

// 空白ページのアイテム テンプレートについては、http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 を参照してください

MainPage::MainPage()
 : m_GpioController(nullptr)
 , m_SerialDeviceCollection(nullptr)
 , m_I2cDeviceCollection(nullptr)
 , m_CameraTimer(nullptr)
 , m_MediaCapture(nullptr)
 , m_SensorMonitor(nullptr)
 , m_SensorTimer(nullptr)
 , m_DataSenderSerial(nullptr)
 , m_DataSender(nullptr)
 , m_MoveType(0)
{
	InitializeComponent();
	InitializeHardware();
	InitializeSensorMonitor();
}

void ARC2016::MainPage::InitializeHardware()
{
	// GPIO
	m_GpioController = GpioController::GetDefault();

	// I2C
	Platform::String^ i2cAqs = I2cDevice::GetDeviceSelector();
	create_task(DeviceInformation::FindAllAsync(i2cAqs)).then([this](DeviceInformationCollection^ i2cDeviceCollection)
	{
		m_I2cDeviceCollection = i2cDeviceCollection;
	});

}

void ARC2016::MainPage::InitializeCamera()
{
	m_MediaCapture = ref new MediaCapture();
	
	create_task(m_MediaCapture->InitializeAsync()).then([this](void)
	{
		ImgCamera->Source = m_MediaCapture.Get();
		create_task(m_MediaCapture->StartPreviewAsync()).then([this](void)
		{
			auto previewProperty = static_cast<MediaProperties::VideoEncodingProperties^>(m_MediaCapture->VideoDeviceController->GetMediaStreamProperties(MediaStreamType::VideoPreview));
			previewProperty->Width = SCREEN_WIDTH;
			previewProperty->Height = SCREEN_HEIGHT;
			m_PreviewWidth = (int)previewProperty->Width;
			m_PreviewHeight = (int)previewProperty->Height;

			m_CameraTimer = ref new DispatcherTimer();
			TimeSpan t;
			t.Duration = 200;
			m_CameraTimer->Interval = t;

			m_CameraTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &ARC2016::MainPage::timer_Camera);
			m_CameraTimer->Start();
		});
	});
}

void ARC2016::MainPage::InitializeSensorMonitor()
{
	// 起動済み
	if (m_SensorMonitor != nullptr)
	{
		return;
	}

	// 未オープン
	if ((m_GpioController == nullptr) || (m_I2cDeviceCollection == nullptr))
	{
		return;
	}

	bool isSimulation = (bool)chkSensorSimulation->IsChecked->Value;

	// 距離センサ：I2C
	auto i2cDistanceSensorSettings = ref new I2cConnectionSettings(0x40);
	i2cDistanceSensorSettings->BusSpeed = I2cBusSpeed::FastMode;
	create_task(I2cDevice::FromIdAsync(m_I2cDeviceCollection->GetAt(0)->Id, i2cDistanceSensorSettings)).then([this, isSimulation](I2cDevice^ distanceSensorDevice)
	{
		// ジャイロセンサ：I2C
		auto i2cGyroSensorSettings = ref new I2cConnectionSettings(0x6B);
		i2cGyroSensorSettings->BusSpeed = I2cBusSpeed::FastMode;
		create_task(I2cDevice::FromIdAsync(m_I2cDeviceCollection->GetAt(0)->Id, i2cGyroSensorSettings)).then([this, isSimulation, distanceSensorDevice](I2cDevice^ gyroSensorDevice)
		{
			if (distanceSensorDevice == nullptr)
			{
				return;
			}

			if (gyroSensorDevice == nullptr)
			{
				return;
			}

			// GPIO
			GpioPin^ sensor1Gpio = m_GpioController->OpenPin(18);
			GpioPin^ sensor2Gpio = m_GpioController->OpenPin(17);
			sensor1Gpio->SetDriveMode(GpioPinDriveMode::Output);
			sensor2Gpio->SetDriveMode(GpioPinDriveMode::Output);

			// インスタンス生成
			DistanceSensor*	pDistanceSensor1 = nullptr;
			DistanceSensor*	pDistanceSensor2 = nullptr;
			GyroSensor* pGyroSensor = nullptr;
			if (isSimulation == true)
			{
				pDistanceSensor1 = new DistanceSensorDummy(distanceSensorDevice, sensor1Gpio);
				pDistanceSensor2 = new DistanceSensorDummy(distanceSensorDevice, sensor2Gpio);
				pGyroSensor = new GyroSensorDummy(gyroSensorDevice);
			}
			else
			{
				pDistanceSensor1 = new DistanceSensor(distanceSensorDevice, sensor1Gpio);
				pDistanceSensor2 = new DistanceSensor(distanceSensorDevice, sensor2Gpio);
				pGyroSensor = new GyroSensor(gyroSensorDevice);
			}

			if ((pDistanceSensor1 == nullptr) || (pDistanceSensor2 == nullptr) || (pGyroSensor == nullptr))
			{
				if (pDistanceSensor1 != nullptr)
				{
					delete pDistanceSensor1;
				}
				if (pDistanceSensor2 != nullptr)
				{
					delete pDistanceSensor2;
				}
				if (pGyroSensor != nullptr)
				{
					delete pGyroSensor;
				}

				return;
			}

			// ジャイロセンサ―初期化
			if (pGyroSensor->Initialize() != E_RET_NORMAL)
			{
				delete pDistanceSensor1;
				delete pDistanceSensor2;
				delete pGyroSensor;
				return;
			}

			// 距離センサリスト作成
			std::vector<DistanceSensor *> distanceSensorList;
			distanceSensorList.push_back(pDistanceSensor1);
			distanceSensorList.push_back(pDistanceSensor2);

			// ジャイロセンサリスト作成
			std::vector<GyroSensor *> gyroSensorList;
			gyroSensorList.push_back(pGyroSensor);

			// センサモニタータスク生成
			SensorMonitor* pMonitor = new SensorMonitor(distanceSensorList, gyroSensorList);
			if (pMonitor == nullptr)
			{
				delete pDistanceSensor1;
				delete pDistanceSensor2;
				delete pGyroSensor;
				return;
			}

			// センサモニタータスク起動
			m_SensorMonitor = pMonitor;
			m_SensorMonitor->Start();
		});
	});
}

void ARC2016::MainPage::InitializeDataSender()
{
	if (m_SerialDeviceCollection == nullptr)
	{
		goto FINISH;
	}

	m_DataSender = nullptr;
	for (unsigned int i = 0; i < m_SerialDeviceCollection->Size; i++)
	{
		DeviceInformation^ info = m_SerialDeviceCollection->GetAt(i);
		std::wstring id(info->Id->Data());
		if (id.find(L"FTDI") != std::wstring::npos)
		{
			if ((m_DataSender == nullptr) &&
				(info != nullptr))
			{ 
				m_DataSenderSerial = info;
				Serial* dev = new Serial(m_DataSenderSerial);
				dev->Initialize();
				m_DataSender = new DataSender(dev);
			}

			if (m_DataSender->IsRunning() == false)
			{
				m_DataSender->Start();
			}
			break;
		}
	}

FINISH:
	return;
}

void ARC2016::MainPage::InitializeDecision()
{
	bool isSimulation = (bool)chkSensorSimulation->IsChecked->Value;

	if (m_DataSender == nullptr
	||	m_Decision != nullptr)
	{
		goto FINISH;
	}

	m_Decision = new Decision(m_SensorMonitor, m_DataSender);
	m_Decision->Start();

FINISH:
	return;
}


void ARC2016::MainPage::ImgCamera_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	InitializeCamera();

	// 画面表示更新タイマ起動
	m_SensorTimer = ref new DispatcherTimer();
	m_SensorTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &ARC2016::MainPage::timer_SensorMonitor);

	TimeSpan t;
	t.Duration = 10;
	m_SensorTimer->Interval = t;
	m_SensorTimer->Start();
}

void ARC2016::MainPage::btnShutdown_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// シャットダウン
	TimeSpan t;
	t.Duration = 0;
	Windows::System::ShutdownKind kind = ShutdownKind::Shutdown;
	Windows::System::ShutdownManager::BeginShutdown(kind, t);
}

void ARC2016::MainPage::btnSensor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	InitializeSensorMonitor();
}

void ARC2016::MainPage::timer_Camera(Platform::Object^ sender, Platform::Object^ e)
{
	m_CameraTimer->Stop();

	VideoFrame^ videoFrame = ref new VideoFrame(BitmapPixelFormat::Bgra8, m_PreviewWidth, m_PreviewHeight);
	task<VideoFrame^> asyncTask = create_task(m_MediaCapture->GetPreviewFrameAsync(videoFrame));
	asyncTask.then([this](VideoFrame^ currentFrame)
	{
		// Platform::Stringへ直接代入
		long lData = 0;
		Platform::String^ str1 = txtBinaryThreshold->Text;
		if (str1 != "")
		{ 
			// String⇒wstring
			std::wstring    ws1(str1->Data());

			lData = std::stol(ws1);
		}

		ChangeBinaryThresh(lData);		

		bool isChecked = (bool)ChkDisplay->IsChecked->Value;
		if (isChecked == true)
		{
			ImgCapture->Source = ConvertProc(currentFrame, true);
		}
		else
		{
			ConvertProc(currentFrame, false);
		}
		m_MoveType = GetMoveType();

		m_CameraTimer->Start();
	});
}

void ARC2016::MainPage::timer_SensorMonitor(Platform::Object^ sender, Platform::Object^ e)
{
	if (m_SensorMonitor != nullptr)
	{
		std::vector<long>	distanceValue;
		m_SensorMonitor->GetDistanceSensorValue(distanceValue);

		std::vector<GyroSensor::GyroDataStr> gyroValue;
		m_SensorMonitor->GetGyroSensorValue(gyroValue);

		txtDistance1->Text = distanceValue[0].ToString();
		txtDistance2->Text = distanceValue[1].ToString();

		txtAccelX->Text = gyroValue[0].AccelX.ToString();
		txtAccelY->Text = gyroValue[0].AccelY.ToString();
		txtAccelZ->Text = gyroValue[0].AccelZ.ToString();
		txtAngleX->Text = gyroValue[0].AngleX.ToString();
		txtAngleY->Text = gyroValue[0].AngleY.ToString();
	}

	if (m_SerialDeviceCollection == nullptr)
	{
		// Serial
		Platform::String ^serialAqs = SerialDevice::GetDeviceSelector();
		create_task(DeviceInformation::FindAllAsync(serialAqs)).then([this](DeviceInformationCollection ^serialDeviceCollection)
		{
			m_SerialDeviceCollection = serialDeviceCollection;
		});
	}

	InitializeDataSender();
	InitializeDecision();

	// 強制出力デバッグ
	//if (m_DataSender != nullptr)
	//{
	//	unsigned char sendBuffer[10] = { 0 };

	//	sendBuffer[0] = BUFFER1_TOP_FRAME;
	//	sendBuffer[1] = BUFFER2_SECOND_FRAME;
	//	sendBuffer[2] = BUFFER3_MOTION_PLAY;
	//	sendBuffer[3] = BUFFER4_DIRECTION_FRONT;
	//	sendBuffer[4] = 36;
	//	sendBuffer[5] = 36;
	//	sendBuffer[6] = 85;
	//	sendBuffer[7] = 0;
	//	sendBuffer[8] = 3;
	//	memcpy(m_DataSender->m_MotorMoveSendBuffer, sendBuffer, sizeof(sendBuffer));
	//}


}
