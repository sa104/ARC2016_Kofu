//
// MainPage.xaml.h
// MainPage クラスの宣言。
//

#pragma once

#include "MainPage.g.h"
#include "Source/Framework/Serial/Serial.h"
#include "Source/Tasks/Camera/Camera.h"
#include "Source/Tasks/SensorMonitor/SensorMonitor.h"
#include "Source/Tasks/DataSender/DataSender.h"
#include "Source/Tasks/Decision/Decision.h"

namespace ARC2016
{
	/// <summary>
	/// それ自体で使用できる空白ページまたはフレーム内に移動できる空白ページ。
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:

		// ハードウェア
		Windows::Devices::Gpio::GpioController^						m_GpioController;
		Windows::Devices::Enumeration::DeviceInformationCollection^	m_SerialDeviceCollection;
		Windows::Devices::Enumeration::DeviceInformationCollection^	m_I2cDeviceCollection;

		// カメラ
		int															m_PreviewWidth;
		int															m_PreviewHeight;
		Windows::UI::Xaml::DispatcherTimer^							m_CameraTimer;
		Platform::Agile<Windows::Media::Capture::MediaCapture>		m_MediaCapture;
		long														m_MoveType;
		
		// センサモニター
		ARC2016::Tasks::SensorMonitor*								m_SensorMonitor;
		Windows::UI::Xaml::DispatcherTimer^							m_SensorTimer;

		// データ更新タスク
		Windows::Devices::Enumeration::DeviceInformation^			m_DataSenderSerial;
		ARC2016::Tasks::DataSender*									m_DataSender;

		// 思考タスク
		ARC2016::Tasks::Decision*									m_Decision;

		void InitializeHardware();
		void InitializeCamera();
		void InitializeSensorMonitor();
		void InitializeDataSender();
		void InitializeDecision();

		void ImgCamera_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void btnShutdown_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void btnSensor_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void timer_Camera(Platform::Object^ sender, Platform::Object^ e);
		void timer_SensorMonitor(Platform::Object^ sender, Platform::Object^ e);
	};
}
