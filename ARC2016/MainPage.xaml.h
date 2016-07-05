//
// MainPage.xaml.h
// MainPage クラスの宣言。
//

#pragma once

#include "MainPage.g.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/cap_winrt.hpp>

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

		bool													m_MediaInitialized;
		bool													m_CaptureDisplaied;
		Platform::Agile<Windows::Media::Capture::MediaCapture>	m_MediaCapture;
		Windows::UI::Xaml::DispatcherTimer^						m_CameraTimer;

		void timer_Tick(Platform::Object^ sender, Platform::Object^ e);
		void ImgCamera_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
