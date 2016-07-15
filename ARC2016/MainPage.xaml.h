//
// MainPage.xaml.h
// MainPage クラスの宣言。
//

#pragma once

#include "MainPage.g.h"
#include "Source/Motor/Motor.h"

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

		Platform::Agile<Windows::Media::Capture::MediaCapture>		m_MediaCapture;
		Windows::UI::Xaml::DispatcherTimer^							m_CameraTimer;
		int															m_PreviewWidth;
		int															m_PreviewHeight;
		Windows::Devices::Enumeration::DeviceInformationCollection^	m_DeviceCollection;
		Windows::Devices::Enumeration::DeviceInformation^			m_MotorSerial;
		Windows::Devices::Enumeration::DeviceInformation^			m_PowerBoardSerial;

		ARC2016::Motor*												m_Motor;



		void timer_Tick(Platform::Object^ sender, Platform::Object^ e);
		void ImgCamera_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void InitializeSerial();

	};
}
