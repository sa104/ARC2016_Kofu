#include "pch.h"
#include <Robuffer.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/cap_winrt.hpp>
#include "Source/Tasks/Camera/Camera.h"

using namespace ARC2016;
using namespace concurrency;
using namespace cv;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml::Media::Imaging;

WriteableBitmap^ ARC2016::ConvertProc(VideoFrame^ source, const bool isDisplay)
{
	// SoftwareBitmap -> WriteableBitmap
	SoftwareBitmap^ softBp = source->SoftwareBitmap;
	WriteableBitmap^ wb = ref new WriteableBitmap(softBp->PixelWidth, softBp->PixelHeight);
	softBp->CopyToBuffer(wb->PixelBuffer);

	// WriteableBitmap -> Mat
	IBuffer^ buffer = wb->PixelBuffer;
	auto reader = DataReader::FromBuffer(buffer);
	unsigned int length = buffer->Length;
	BYTE *extracted = new BYTE[length];
	reader->ReadBytes(ArrayReference<BYTE>(extracted, length));
	Mat img_image(wb->PixelHeight, wb->PixelWidth, CV_8UC4, extracted);

	// グレースケール変換
	Mat img_gray(wb->PixelHeight, wb->PixelWidth, CV_8UC4);
	cvtColor(img_image, img_gray, CV_BGRA2GRAY);

	// ライン検知



	// Get access to the pixels
	WriteableBitmap^ bp = ref new WriteableBitmap(img_image.cols, img_image.rows);

	if (isDisplay == true)
	{
		IBuffer^ buffer2 = bp->PixelBuffer;
		length = buffer2->Length;
		unsigned char* dstPixels;

		// Obtain IBufferByteAccess
		ComPtr<IBufferByteAccess> pBufferByteAccess;
		ComPtr<IInspectable> pBuffer((IInspectable*)buffer2);
		pBuffer.As(&pBufferByteAccess);

		// Get pointer to pixel bytes
		pBufferByteAccess->Buffer(&dstPixels);

		long size = img_image.step.buf[1] * img_image.cols * img_image.rows;
		memcpy(dstPixels, img_image.data, size);
	}


	delete[] extracted;

	return bp;
}
