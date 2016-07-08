//
// MainPage.xaml.cpp
// MainPage クラスの実装。
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Source/Camera/Camera.h"

using namespace ARC2016;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media;
using namespace Windows::Media::Capture;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;


// 空白ページのアイテム テンプレートについては、http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 を参照してください

MainPage::MainPage()
{
	InitializeComponent();
}

void ARC2016::MainPage::ImgCamera_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_MediaCapture = ref new MediaCapture();
	
	task<void> asyncTask = create_task(m_MediaCapture->InitializeAsync());
	asyncTask.then([this](void)
	{
		ImgCamera->Source = m_MediaCapture.Get();
		task<void> asyncTask2 = create_task(m_MediaCapture->StartPreviewAsync());
		asyncTask2.then([this](void)
		{
			auto previewProperty = static_cast<MediaProperties::VideoEncodingProperties^>(m_MediaCapture->VideoDeviceController->GetMediaStreamProperties(MediaStreamType::VideoPreview));
			m_PreviewWidth = (int)previewProperty->Width;
			m_PreviewHeight = (int)previewProperty->Height;

			m_CameraTimer = ref new DispatcherTimer();
			m_CameraTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &ARC2016::MainPage::timer_Tick);

			TimeSpan t;
			t.Duration = 10;
			m_CameraTimer->Interval = t;
			m_CameraTimer->Start();
		});
	});
}

void ARC2016::MainPage::timer_Tick(Platform::Object^ sender, Platform::Object^ e)
{
	m_CameraTimer->Stop();

	VideoFrame^ videoFrame = ref new Windows::Media::VideoFrame(BitmapPixelFormat::Bgra8, m_PreviewWidth, m_PreviewHeight);
	task<VideoFrame^> asyncTask = create_task(m_MediaCapture->GetPreviewFrameAsync(videoFrame));
	asyncTask.then([this](VideoFrame^ currentFrame)
	{
		ImgCapture->Source = ConvertProc(currentFrame);
		m_CameraTimer->Start();
	});
}
