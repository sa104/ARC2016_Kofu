//
// MainPage.xaml.cpp
// MainPage クラスの実装。
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <Robuffer.h>

using namespace ARC2016;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Devices::Enumeration;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace Windows::Media;
using namespace Windows::Media::Capture;
using namespace Microsoft::WRL;
using namespace Windows::Storage::Streams;

// 空白ページのアイテム テンプレートについては、http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409 を参照してください

MainPage::MainPage()
//: m_MediaInitialized(false)
//, m_CaptureDisplaied(true)
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
			m_MediaInitialized = true;
		});
	});

	m_CameraTimer = ref new DispatcherTimer();
	m_CameraTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &ARC2016::MainPage::timer_Tick);

	TimeSpan t;
	t.Duration = 10;
	m_CameraTimer->Interval = t;
	m_CameraTimer->Start();
}

void ARC2016::MainPage::timer_Tick(Platform::Object^ sender, Platform::Object^ e)
{
	if (m_MediaInitialized == false)
	{
		return;
	}

	if (m_CaptureDisplaied == false)
	{
		return;
	}

	m_CaptureDisplaied = false;
	auto previewProperty = static_cast<MediaProperties::VideoEncodingProperties^>(m_MediaCapture->VideoDeviceController->GetMediaStreamProperties(MediaStreamType::VideoPreview));
	Windows::Media::VideoFrame^ videoFrame = ref new Windows::Media::VideoFrame(Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8, (int)previewProperty->Height, (int)previewProperty->Width);

	task<Windows::Media::VideoFrame^> asyncTask = create_task(m_MediaCapture->GetPreviewFrameAsync(videoFrame));
	asyncTask.then([this](Windows::Media::VideoFrame^ currentFrame)
	{
		// Bitmap -> Mat
		Windows::Graphics::Imaging::SoftwareBitmap^ softBp = currentFrame->SoftwareBitmap;
		WriteableBitmap^ wb = ref new WriteableBitmap(softBp->PixelWidth, softBp->PixelHeight);
		softBp->CopyToBuffer(wb->PixelBuffer);
		Windows::Storage::Streams::IBuffer^ buffer = wb->PixelBuffer;
		auto reader = ::Windows::Storage::Streams::DataReader::FromBuffer(buffer);
		BYTE *extracted = new BYTE[buffer->Length];
		reader->ReadBytes(Platform::ArrayReference<BYTE>(extracted, buffer->Length));
		cv::Mat img_image(wb->PixelHeight, wb->PixelWidth, CV_8UC4, extracted);

		// グレースケール変換
		cv::Mat img_gray(wb->PixelHeight, wb->PixelWidth, CV_8UC4);
		cv::cvtColor(img_image, img_gray, CV_BGRA2GRAY);

		// 二値化
		cv::Mat img_thresh(wb->PixelHeight, wb->PixelWidth, CV_8UC4);
		cv::threshold(img_gray, img_thresh, 60, 255, CV_THRESH_BINARY);

		// Get access to the pixels
		WriteableBitmap^ bp = ref new WriteableBitmap(img_thresh.cols, img_thresh.rows);
		Windows::Storage::Streams::IBuffer^ buffer2 = bp->PixelBuffer;
		unsigned char* dstPixels;

		// Obtain IBufferByteAccess
		ComPtr<IBufferByteAccess> pBufferByteAccess;
		ComPtr<IInspectable> pBuffer((IInspectable*)buffer2);
		pBuffer.As(&pBufferByteAccess);

		// Get pointer to pixel bytes
		pBufferByteAccess->Buffer(&dstPixels);
		memcpy(dstPixels, img_thresh.data, img_thresh.step.buf[1] * img_thresh.cols * img_thresh.rows);

		delete[] extracted;

		// Set the bitmap to the Image element
		ImgCapture->Source = bp;
		m_CaptureDisplaied = true;

	});
}
