#include "pch.h"
#include <Robuffer.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
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

S_LINE_INFO	gsLineInfo = { 0x00 };
S_MAP		gsMapData = { 0x00 };

Mat             m_Image_Original;
Mat             m_Image_Gray;
Mat             m_Image_Binary;
Mat             m_Image_Red;

S_LINE_INFO     m_LineInfo;
S_MAP           m_MapData;
long            m_BinaryThresh;
long            m_DisplayView;
long            m_CornerLineWidth = 110;
long            m_MaxDeviation[3] = {120, 140, 150};
long            m_CornerIndex = 8;
long            m_OnCount;
long            m_OnCountLineMax;
long            m_ContinuityOnCount;

float           m_CameraVerticalAngle;
float           m_CameraVerticalAngleMax;
MoveTypeEnum    m_BeforeMoveType;
bool            m_GoalDetect;

MoveTypeEnum	m_MoveType;

enum
{
	E_IMAGE_ORIGINAL = 0x00,
	E_IMAGE_GRAY,
	E_IMAGE_BINARY,
} E_IMAGE_LIST;

WriteableBitmap^ ARC2016::ConvertProc(VideoFrame^ source, const bool isDisplay)
{
	S_LINE_INFO     *sRet = NULL;
	S_LINE_INFO     sData = { 0x00 };
	long            lWidthIndex = 0;
	long            lLineIndex = 0;
	long            lLineOffset = (SCREEN_HEIGHT / C_DATALINE_MAX);
	long            lOnCount = 0;
	long            lLineCount = 0;
	long            lLineStart = 0;
	long            lLineEnd = 0;
	long            lLineWidth = 0;
	long            lCenterGravity = 0;
	bool            bLineFind = false;
	bool            bLine = false;
	bool            bCorner = false;
	long            lMaxDeviation = 0;
	long            lMaxDeviationMinus = 0;
	long            lCornerIndex = 0;
	long            lBeforeIndex = 0;
	long            lContinuity = 0;
	long            lStopContinuity = 0;
	bool            bStraight = false;
	long            lCornerLine = 0;
	CvPoint         cvPointStart = { 0x00 };
	CvPoint         cvPointEnd = { 0x00 };
	CvScalar        cvColor = Scalar(256, 0, 0);
	CvScalar        cvColor2 = Scalar(256, 256, 0);
	CvFont          cvFont = { 0x00 };
	char            cText[32] = { 0x00 };
	int             iRedData = 0;
	int             iBlueData = 0;
	int             iGreenData = 0;
	long			lBinaryThresh = 0;

	MoveTypeEnum eMoveType = MOVE_PATTERN_MIN;
	S_MAP*          sMap = NULL;
	IplImage        sImage;

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
	cv::Mat m_Image_Original(wb->PixelHeight, wb->PixelWidth, CV_8UC4, extracted);

	// �O���[�X�P�[���ϊ�
	cv::Mat m_Image_Gray(wb->PixelHeight, wb->PixelWidth, CV_8UC4);
	cvtColor(m_Image_Original, m_Image_Gray, CV_BGRA2GRAY);

	// ��l��
	cv::Mat m_Image_Binary(wb->PixelHeight, wb->PixelWidth, CV_8UC4);
	threshold(m_Image_Gray, m_Image_Binary, m_BinaryThresh, 255, CV_THRESH_BINARY);

	sImage = m_Image_Binary;
	sMap = getMapData(&sImage);
	if (sMap == NULL)
	{
		goto FINISH;
	}

	/********************* DEBUG INFO *********************/
	cvInitFont(&cvFont, CV_FONT_HERSHEY_SIMPLEX, 1, 1);
	cvPointStart = Point(C_LINE_ORIGIN, 0);
	cvPointEnd = Point(C_LINE_ORIGIN, SCREEN_HEIGHT);
	line(m_Image_Original, cvPointStart, cvPointEnd, cvColor);
	/********************* DEBUG INFO *********************/

	/********************* DEBUG INFO *********************/
	for (long deviationCounter = 0; deviationCounter<3; deviationCounter++)
	{
		cvPointStart = Point(160 + m_MaxDeviation[deviationCounter], 0);
		cvPointEnd = Point(160 + m_MaxDeviation[deviationCounter], sMap->lHeight - 1);
		line(m_Image_Original, cvPointStart, cvPointEnd, cvColor);
	}
	for (long deviationCounter = 0; deviationCounter<3; deviationCounter++)
	{
		cvPointStart = Point(160 - m_MaxDeviation[deviationCounter], 0);
		cvPointEnd = Point(160 - m_MaxDeviation[deviationCounter], sMap->lHeight - 1);
		line(m_Image_Original, cvPointStart, cvPointEnd, cvColor);
	}
	/********************* DEBUG INFO *********************/
	m_OnCount = 0;
	/* ������ */
	for (lLineIndex = 0; lLineIndex < C_DATALINE_MAX; lLineIndex++)
	{
		long	lOffset = (lLineIndex + 1)*lLineOffset;
		if (lLineIndex == (C_DATALINE_MAX - 1))
		{
			lOffset--;
		}
		memcpy((void*)sData.sLineData[lLineIndex].cLineData, (void*)&sMap->ucMap[lOffset][0], sizeof(unsigned char)*sMap->lWidth);
		sData.sLineData[lLineIndex].lMapLineNo = lOffset;
		sData.sLineData[lLineIndex].lLineThreshold = C_LINETHRESHOLD;

		/********************* DEBUG INFO *********************/
		cvPointStart = Point(0, sData.sLineData[lLineIndex].lMapLineNo);
		cvPointEnd = Point(sMap->lWidth - 1, sData.sLineData[lLineIndex].lMapLineNo);
		line(m_Image_Original, cvPointStart, cvPointEnd, cvColor);
		/********************* DEBUG INFO *********************/
		lOnCount = 0;
		lLineCount = 0;
		bLineFind = false;
		lLineStart = 0;
		lLineEnd = 0;
		lLineWidth = 0;
		lCenterGravity = 0;

		for (lWidthIndex = 0; lWidthIndex < SCREEN_WIDTH; lWidthIndex++)
		{
			if (1 == sData.sLineData[lLineIndex].cLineData[lWidthIndex])
			{
				lOnCount++;
				m_OnCount++;
				if (C_LINETHRESHOLD == lOnCount)
				{
					/* ���C�������o */
					lLineCount++;
					bLineFind = true;
					lLineStart = lWidthIndex;
				}
			}
			else
			{
				if (true == bLineFind)
				{
					/* ���C���̐؂�ڂ����o */
					lLineEnd = lWidthIndex;
					lLineWidth = (lLineEnd - lLineStart) + 1;
					lCenterGravity = lLineStart + (lLineWidth / 2);
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lStartPos = lLineStart;							/* ���C���J�n�ʒu */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lEndPos = lLineEnd;								/* ���C���I���ʒu */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lLineWidth = lLineWidth;							/* ���C���� */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterGravity = lCenterGravity;					/* ���C�������ʒu */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation = lCenterGravity - C_LINE_ORIGIN;	/* ���S����̃Y���� */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lMapLineNo = sData.sLineData[lLineIndex].lMapLineNo;
					sData.sLineData[lLineIndex].bLine = true;

					if (m_CornerLineWidth <= lLineWidth)
					{
						/* �R�[�i�[�����o */
						sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_CORNER;
						sData.sLineData[lLineIndex].eLineType = E_LINE_CORNER;
						sData.sLineData[lLineIndex].bCorner = true;
						sData.sLineData[lLineIndex].lCornerNum++;

						if (false == sData.sFind[E_FIND_TOP_CORNER].bFind)
						{
							/* ��ԍŏ��Ɍ��o����Corner�����擾 */
							sData.sFind[E_FIND_TOP_CORNER].bFind = true;
							sData.sFind[E_FIND_TOP_CORNER].lDataLineNo = lLineIndex;
							sData.sFind[E_FIND_TOP_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
							sData.sFind[E_FIND_TOP_CORNER].sLineType = E_LINE_CORNER;
							sData.sFind[E_FIND_TOP_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
						}
						sData.sFind[E_FIND_BACK_CORNER].bFind = true;
						sData.sFind[E_FIND_BACK_CORNER].lDataLineNo = lLineIndex;
						sData.sFind[E_FIND_BACK_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
						sData.sFind[E_FIND_BACK_CORNER].sLineType = E_LINE_CORNER;
						sData.sFind[E_FIND_BACK_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
						/* �R�[�i�[�����o�����̂ō��܂ł̍ő�Y���ʂ������� */
						sData.lMaxDeviation = 0;
						sData.lMaxDeviationMinus = 0;
						if (C_CORNER_MAX>sData.lCornerCount)
						{
							sData.sCorner[sData.lCornerCount].lLineNo = lLineIndex;
							sData.lCornerCount++;
						}

						if (sData.sLineData[lLineIndex].lMapLineNo - 1 >= C_CAMERA_LOWANGLE_THRESHOLD)
						{
							if (m_CameraVerticalAngle <= m_CameraVerticalAngleMax)
							{
								cvColor2 = Scalar(0, 0, 256);
								cvPointStart = Point(20, 30);
								char            cCameraText[50] = { 0x00 };
								m_CameraVerticalAngle = m_CameraVerticalAngle + 1.0f;
								snprintf(cCameraText, sizeof(cCameraText), "Camera Down %1.1f", m_CameraVerticalAngle);
								putText(m_Image_Original, cCameraText, cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
							}
						}
					}
					else
					{
						/* ���C�� */
						sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_LINE;
						sData.sLineData[lLineIndex].eLineType = E_LINE_LINE;
						sData.sLineData[lLineIndex].lLineNum++;

						if (false == sData.sFind[E_FIND_TOP_LINE].bFind)
						{
							/* ��ԍŏ��Ɍ��o����Corner�����擾 */
							sData.sFind[E_FIND_TOP_LINE].bFind = true;
							sData.sFind[E_FIND_TOP_LINE].lDataLineNo = lLineIndex;
							sData.sFind[E_FIND_TOP_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
							sData.sFind[E_FIND_TOP_LINE].sLineType = E_LINE_LINE;
							sData.sFind[E_FIND_TOP_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
						}
						sData.sFind[E_FIND_BACK_LINE].bFind = true;
						sData.sFind[E_FIND_BACK_LINE].lDataLineNo = lLineIndex;
						sData.sFind[E_FIND_BACK_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
						sData.sFind[E_FIND_BACK_LINE].sLineType = E_LINE_LINE;
						sData.sFind[E_FIND_BACK_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];

						/* �ő�Y���ʂ�ێ����� */
						if (0 <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
						{
							if (sData.lMaxDeviation <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
							{
								sData.lMaxDeviation = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
							}
						}
						else
						{
							if (sData.lMaxDeviationMinus >= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
							{
								sData.lMaxDeviationMinus = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
							}
						}
					}
					sData.bLine = true;
					sData.sLineData[lLineIndex].lLineCount++;
					/********************* DEBUG INFO *********************/
					cvPointStart = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo - 1);
					cvPointEnd = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo + 1);
					if (E_LINE_CORNER == sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount - 1].eLineType)
					{
						cvColor2 = Scalar(0, 256, 0);
					}
					else
					{
						cvColor2 = Scalar(256, 256, 0);
					}
					line(m_Image_Original, cvPointStart, cvPointEnd, cvColor2, 3, CV_AA);
					/********************* DEBUG INFO *********************/
				}
				lOnCount = 0;
				bLineFind = false;
				lLineStart = 0;
				lLineEnd = 0;
				lLineWidth = 0;
				lCenterGravity = 0;
			}
		}
		if (true == bLineFind)
		{
			/* ���C���̐؂�ڂ����o */
			if (SCREEN_WIDTH <= lWidthIndex)
			{
				lWidthIndex = (SCREEN_WIDTH - 1);
			}
			lLineEnd = lWidthIndex;
			lLineWidth = (lLineEnd - lLineStart) + 1;
			lCenterGravity = lLineStart + (lLineWidth / 2);
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lStartPos = lLineStart;							/* ���C���J�n�ʒu */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lEndPos = lLineEnd;								/* ���C���I���ʒu */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lLineWidth = lLineWidth;							/* ���C���� */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterGravity = lCenterGravity;					/* ���C�������ʒu */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation = lCenterGravity - C_LINE_ORIGIN;	/* ���S����̃Y���� */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lMapLineNo = sData.sLineData[lLineIndex].lMapLineNo;
			sData.sLineData[lLineIndex].bLine = true;

			if (m_CornerLineWidth <= lLineWidth)
			{
				/* �R�[�i�[�����o */
				sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_CORNER;
				sData.sLineData[lLineIndex].eLineType = E_LINE_CORNER;
				sData.sLineData[lLineIndex].bCorner = true;
				sData.sLineData[lLineIndex].lCornerNum++;

				if (false == sData.sFind[E_FIND_TOP_CORNER].bFind)
				{
					/* ��ԍŏ��Ɍ��o����Corner�����擾 */
					sData.sFind[E_FIND_TOP_CORNER].bFind = true;
					sData.sFind[E_FIND_TOP_CORNER].lDataLineNo = lLineIndex;
					sData.sFind[E_FIND_TOP_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
					sData.sFind[E_FIND_TOP_CORNER].sLineType = E_LINE_CORNER;
					sData.sFind[E_FIND_TOP_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
				}
				sData.sFind[E_FIND_BACK_CORNER].bFind = true;
				sData.sFind[E_FIND_BACK_CORNER].lDataLineNo = lLineIndex;
				sData.sFind[E_FIND_BACK_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
				sData.sFind[E_FIND_BACK_CORNER].sLineType = E_LINE_CORNER;
				sData.sFind[E_FIND_BACK_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];

				/* �R�[�i�[�����o�����̂ō��܂ł̍ő�Y���ʂ������� */
				sData.lMaxDeviation = 0;
				sData.lMaxDeviationMinus = 0;
				if (C_CORNER_MAX>sData.lCornerCount)
				{
					sData.sCorner[sData.lCornerCount].lLineNo = lLineIndex;
					sData.lCornerCount++;
				}

				if (sData.sLineData[lLineIndex].lMapLineNo - 1 >= C_CAMERA_LOWANGLE_THRESHOLD)
				{
					if (m_CameraVerticalAngle <= m_CameraVerticalAngleMax)
					{
						cvColor2 = Scalar(0, 0, 256);
						cvPointStart = Point(20, 30);
						char            cCameraText[50] = { 0x00 };
						m_CameraVerticalAngle = m_CameraVerticalAngle + 1.0f;
						snprintf(cCameraText, sizeof(cCameraText), "Camera Down %1.1f", m_CameraVerticalAngle);
						putText(m_Image_Original, cCameraText, cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
					}
				}
			}
			else
			{
				/* ���C�� */
				sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_LINE;
				sData.sLineData[lLineIndex].eLineType = E_LINE_LINE;
				sData.sLineData[lLineIndex].lLineNum++;

				if (false == sData.sFind[E_FIND_TOP_LINE].bFind)
				{
					/* ��ԍŏ��Ɍ��o����Corner�����擾 */
					sData.sFind[E_FIND_TOP_LINE].bFind = true;
					sData.sFind[E_FIND_TOP_LINE].lDataLineNo = lLineIndex;
					sData.sFind[E_FIND_TOP_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
					sData.sFind[E_FIND_TOP_LINE].sLineType = E_LINE_LINE;
					sData.sFind[E_FIND_TOP_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
				}
				sData.sFind[E_FIND_BACK_LINE].bFind = true;
				sData.sFind[E_FIND_BACK_LINE].lDataLineNo = lLineIndex;
				sData.sFind[E_FIND_BACK_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
				sData.sFind[E_FIND_BACK_LINE].sLineType = E_LINE_LINE;
				sData.sFind[E_FIND_BACK_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];

				/* �ő�Y���ʂ�ێ����� */
				if (0 <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
				{
					if (sData.lMaxDeviation <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
					{
						sData.lMaxDeviation = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
					}
				}
				else
				{
					if (sData.lMaxDeviationMinus >= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
					{
						sData.lMaxDeviationMinus = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
					}
				}
			}
			sData.bLine = true;
			sData.sLineData[lLineIndex].lLineCount++;

			/********************* DEBUG INFO *********************/
			cvPointStart = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo - 1);
			cvPointEnd = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo + 1);
			if (E_LINE_CORNER == sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount - 1].eLineType)
			{
				cvColor2 = Scalar(0, 256, 0);
			}
			else
			{
				cvColor2 = Scalar(256, 256, 0);
			}
			line(m_Image_Original, cvPointStart, cvPointEnd, cvColor2, 3, CV_AA);
			/********************* DEBUG INFO *********************/
		}
	}

	E_CORNER_TYPE eCornerType = E_CORNER_NONE;

	//if ((100  > m_OnCount) ||
	//	(2000 < m_OnCount))
	//{
	//	eMoveType = PATTERN_MISSING;
	//	sData.bLine = false;
	//}
	if ((true == sData.bLine) && (0 != sData.lCornerCount))
	{
		if (1 == sData.lCornerCount)
		{
			/* 1�R�[�i�[ */
			if (m_CornerIndex <= sData.sCorner[0].lLineNo)
			{
				long linecount = 0;
				for (long lineindex = sData.sCorner[sData.lCornerCount - 1].lLineNo - 2; lineindex >= 0; lineindex--)
				{
					linecount += (0<sData.sLineData[lineindex].lLineNum) ? 1 : 0;
				}
				if (linecount >= 2)
				{
					/* ���i�Ƃ��Ĕ��f���� */
					bStraight = true;
				}
				/* �R�[�i�[�쓮����߁A�␳�쓮�݂̂ŃR�[�i�[�����؂� */
				else
				{
					eCornerType = GetCornerType(sData, 0);
					if (E_CORNER_RIGHT == eCornerType)
					{
#if 0
						//                        eMoveType = PATTERN_TURN_RIGHT_90;
#endif
						eMoveType = PATTERN_FRONT_RIGHT_90;
					}
					else
					{
#if 0
						//                        eMoveType = PATTERN_TURN_LEFT_90;
#endif
						eMoveType = PATTERN_FRONT_LEFT_90;
					}
				}
			}
			else
			{
				bStraight = true;
			}
		}
		else
		{
			/* �����R�[�i�[ */
			for (lCornerIndex = 0; lCornerIndex < sData.lCornerCount; lCornerIndex++)
			{
				if (0 != lCornerIndex)
				{
					/* ���O��Line���R�[�i�[�����o���Ă��邩�H */
					if (sData.sCorner[lCornerIndex - 1].lLineNo == (sData.sCorner[lCornerIndex].lLineNo - 1))
					{
						/* ���o */
						lContinuity++;
						lCornerLine = sData.sCorner[lCornerIndex].lLineNo;
					}
					else
					{
						/* �����o */
						lStopContinuity++;
					}
				}
			}
			if ((0 != lContinuity) &&
				(0 == lStopContinuity))
			{
				/* �P�ƃR�[�i�[ */
				if (m_CornerIndex <= sData.sCorner[sData.lCornerCount - 1].lLineNo)
				{
					long linecount = 0;
					for (long lineindex = sData.sCorner[sData.lCornerCount - 1].lLineNo - 2; lineindex >= 0; lineindex--)
					{
						linecount += (0<sData.sLineData[lineindex].lLineNum) ? 1 : 0;
					}
					if (linecount >= 2)
					{
						/* ���i�Ƃ��Ĕ��f���� */
						bStraight = true;
					}
					else
					{
						eCornerType = GetCornerType(sData, sData.lCornerCount - 1);
						if (E_CORNER_RIGHT == eCornerType)
						{
#if 0
							//                            eMoveType = PATTERN_TURN_RIGHT_90;
#endif
							eMoveType = PATTERN_FRONT_RIGHT_90;
						}
						else
						{
#if 0
							//                            eMoveType = PATTERN_TURN_LEFT_90;
#endif
							eMoveType = PATTERN_FRONT_LEFT_90;
						}
					}
				}
				else
				{
					bStraight = true;
				}
			}
			else
			{
				/* �Ɨ����������̃R�[�i�[������ */
				bStraight = true;
			}
		}
	}

	long	lFrontLineNo = 0;
	long	lFrontCornerNo = 0;
	long	lLineCountIndex = 0;
	long	lTopBackDiff = 0;
	bool	bLineCheck = false;

	if (((true == sData.bLine) && (0 == sData.lCornerCount)) ||
		(true == bStraight))
	{
		if (0 == sData.lCornerCount)
		{
			/* �R�[�i�[�Ȃ� */
			bLineCheck = true;
		}
		if ((true == sData.sFind[E_FIND_TOP_LINE].bFind) &&
			(true == sData.sFind[E_FIND_TOP_CORNER].bFind))
		{
			if (sData.sFind[E_FIND_TOP_LINE].lDataLineNo >= sData.sFind[E_FIND_TOP_CORNER].lDataLineNo)
			{
				/* TOP�R�[�i�[�����TOP���C�������� */
				bLineCheck = true;
			}
			else if (sData.sFind[E_FIND_TOP_LINE].lDataLineNo < sData.sFind[E_FIND_TOP_CORNER].lDataLineNo)
			{
				/* TOP�R�[�i�[����ԏ� */
				sData.sFind[E_FIND_TOP_LINE] = sData.sFind[E_FIND_TOP_CORNER];
				bLineCheck = true;
			}
			else if (sData.sFind[E_FIND_BACK_LINE].lDataLineNo < sData.sFind[E_FIND_BACK_CORNER].lDataLineNo)
			{
				/* BACK���C����艺��BACK�R�[�i�[���� */
				sData.sFind[E_FIND_BACK_LINE] = sData.sFind[E_FIND_BACK_CORNER];
				bLineCheck = true;
			}
			else if (sData.sFind[E_FIND_BACK_LINE].lDataLineNo >= sData.sFind[E_FIND_BACK_CORNER].lDataLineNo)
			{
				/* BACK���C������ԉ� */
				bLineCheck = true;
			}
		}
		else if ((false == sData.sFind[E_FIND_TOP_LINE].bFind) &&
			(true == sData.sFind[E_FIND_TOP_CORNER].bFind))
		{
			/* �R�[�i�[�����Ȃ� */
			sData.sFind[E_FIND_TOP_LINE] = sData.sFind[E_FIND_TOP_CORNER];
			sData.sFind[E_FIND_BACK_LINE] = sData.sFind[E_FIND_BACK_CORNER];
			bLineCheck = true;
		}
		else
		{

		}

		if (true == bLineCheck)
		{
			if (m_MaxDeviation[0] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
			{
				if ((m_MaxDeviation[0] * 1.5) >= labs(lTopBackDiff))
				{
					eMoveType = PATTERN_FRONT;
				}
				else
				{
					lTopBackDiff = sData.sFind[E_FIND_TOP_LINE].sLine.lStartPos - sData.sFind[E_FIND_BACK_LINE].sLine.lStartPos;
					if (0 < sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)
					{
						if ((m_MaxDeviation[0] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[1] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_RIGHT_15;
						}

						if ((m_MaxDeviation[1] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[2] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_RIGHT_30;
						}
						else
						{
							eMoveType = PATTERN_FRONT_RIGHT_45;
						}
					}
					else
					{
						if ((m_MaxDeviation[0] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[1] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_LEFT_15;
						}
						else if ((m_MaxDeviation[1] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[2] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_LEFT_30;
						}
						else
						{
							eMoveType = PATTERN_FRONT_LEFT_45;
						}
					}
				}
			}
			else
			{
				if (0 < sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)
				{
					if ((m_MaxDeviation[0] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[1] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_RIGHT_15;
					}
					else if ((m_MaxDeviation[1] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[2] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_RIGHT_30;
					}
					else
					{
						eMoveType = PATTERN_FRONT_RIGHT_45;
					}
				}
				else
				{
					if ((m_MaxDeviation[0] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[1] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_LEFT_15;
					}
					else if ((m_MaxDeviation[1] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[2] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_LEFT_30;
					}
					else
					{
						eMoveType = PATTERN_FRONT_LEFT_45;
					}
				}
			}
			cvPointStart = Point(sData.sFind[E_FIND_TOP_LINE].sLine.lStartPos, sData.sFind[E_FIND_TOP_LINE].sLine.lMapLineNo);
			cvPointEnd = Point(sData.sFind[E_FIND_BACK_LINE].sLine.lStartPos, sData.sFind[E_FIND_BACK_LINE].sLine.lMapLineNo);
			cvColor2 = Scalar(256, 0, 256);
			line(m_Image_Original, cvPointStart, cvPointEnd, cvColor2, 2, CV_AA);
		}
		else
		{

		}
	}

	if (eMoveType == PATTERN_STOP)
	{
		eMoveType = PATTERN_MISSING;
	}

	if ((eMoveType != PATTERN_MISSING) && (eMoveType != PATTERN_STOP))
	{
		m_BeforeMoveType = eMoveType;
	}

	cvColor2 = Scalar(0, 0, 256);
	cvPointStart = Point(20, 20);
	switch (eMoveType)
	{
	case PATTERN_FRONT:
	case PATTERN_FRONT_LEFT_15:
	case PATTERN_FRONT_LEFT_30:
	case PATTERN_FRONT_LEFT_45:
	case PATTERN_FRONT_RIGHT_15:
	case PATTERN_FRONT_RIGHT_30:
	case PATTERN_FRONT_RIGHT_45:
		putText(m_Image_Original, "^^^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_TURN_LEFT_90:
	case PATTERN_FRONT_LEFT_90:
		putText(m_Image_Original, "Corner[LEFT]", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_TURN_RIGHT_90:
	case PATTERN_FRONT_RIGHT_90:
		putText(m_Image_Original, "Corner[RIGHT]", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;

	//case PATTERN_FRONT_LEFT_15:
	//	putText(m_Image_Original, "^^^<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_LEFT_30:
	//	putText(m_Image_Original, "^^^<<<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_LEFT_45:
	//	putText(m_Image_Original, "^^^<<<<<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_RIGHT_15:
	//	putText(m_Image_Original, "^^^>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_RIGHT_30:
	//	putText(m_Image_Original, "^^^>>>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_RIGHT_45:
	//	putText(m_Image_Original, "^^^>>>>>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_LEFT_15:
	//	putText(m_Image_Original, "^^^<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_LEFT_30:
	//	putText(m_Image_Original, "^^^<<<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_LEFT_45:
	//	putText(m_Image_Original, "^^^<<<<<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_RIGHT_15:
	//	putText(m_Image_Original, "^^^>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_RIGHT_30:
	//	putText(m_Image_Original, "^^^>>>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	//case PATTERN_FRONT_RIGHT_45:
	//	putText(m_Image_Original, "^^^>>>>>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
	//	break;
	default:
		putText(m_Image_Original, " ", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;

	}
	char cBuf[16] = { 0x00 };
	cvPointStart.y = 50;
	snprintf(cBuf, sizeof(cBuf), "%ld", m_OnCount);
	putText(m_Image_Original, cBuf, cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2);

	//if (isGoalLine() == true)
	//{
	//	cvPointStart.x = 20;
	//	cvPointStart.y = 120;
	//	putText(m_Image_Original, "G O A L !!!!", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2);
	//	eMoveType = PATTERN_PUT;
	//}

	m_LineInfo = sData;
	m_MoveType = eMoveType;

FINISH:

	// Get access to the pixels
	WriteableBitmap^ bp = ref new WriteableBitmap(m_Image_Original.cols, m_Image_Original.rows);

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
		
		long size = m_Image_Original.step.buf[1] * m_Image_Original.cols * m_Image_Original.rows;
		memcpy(dstPixels, m_Image_Original.data, size);
	}


	delete[] extracted;

	return bp;
}


void ARC2016::SetCornerLineWidth(long lCornerLineWidth)
{
	m_CornerLineWidth = lCornerLineWidth;
}

void ARC2016::SetMaxDeviation1(long lMaxDeviation)
{
	m_MaxDeviation[0] = lMaxDeviation;
}
void ARC2016::SetMaxDeviation2(long lMaxDeviation)
{
	m_MaxDeviation[1] = lMaxDeviation;
}
void ARC2016::SetMaxDeviation3(long lMaxDeviation)
{
	m_MaxDeviation[2] = lMaxDeviation;
}

void ARC2016::SetCornerIndex(long lCornerIndex)
{
	m_CornerIndex = lCornerIndex;
}

long ARC2016::GetCornerLineWidth()
{
	return (m_CornerLineWidth);
}

long ARC2016::GetMaxDeviation1()
{
	return (m_MaxDeviation[0]);
}

long ARC2016::GetMaxDeviation2()
{
	return (m_MaxDeviation[1]);
}

long ARC2016::GetMaxDeviation3()
{
	return (m_MaxDeviation[2]);
}

long ARC2016::GetCornerIndex()
{
	return (m_CornerIndex);
}

void ARC2016::ChangeBinaryThresh(const long lThresh)
{
	m_BinaryThresh = lThresh;
	return;
}

long ARC2016::GetMoveType()
{
	long ret = (long)m_MoveType;
	return(ret);
}

void ARC2016::ChangeConvertView()
{
	if (VIEW_MAX <= m_DisplayView)
	{
		m_DisplayView = VIEW_MIN;
	}
	else
	{
		m_DisplayView++;
	}
}

long ARC2016::CaptureToDirection(IplImage* pOriginal, MoveTypeEnum& eMoveType)
{
	long            lRet = -1;
	S_MAP*          spMap = NULL;
	S_LINE_INFO*    spLineInfo = NULL;
	IplImage*       pGray;
	IplImage*       pBinary;


	pGray = cvCreateImage(cvGetSize(pOriginal), pOriginal->depth, 1);

	/* �O���[�X�P�[���ϊ� */
	cvCvtColor(pOriginal, pGray, CV_BGR2GRAY);
	pBinary = cvCreateImage(cvGetSize(pGray), pGray->depth, pGray->nChannels);

	/* ������ */
	cvSmooth(pGray, pGray, CV_GAUSSIAN, 5);

	/* 2�l�� */
	cvThreshold(pGray, pBinary, m_BinaryThresh, 255, CV_THRESH_BINARY);

	spMap = getMapData(pBinary);
	spLineInfo = getLineData(spMap, eMoveType);
	if (NULL == spLineInfo)
	{
		goto FINISH;
	}

	Sleep(10);

	lRet = 0;


FINISH:
	return (lRet);
}

long ARC2016::CaptureToDirection(Mat& sOriginal, Mat& sConvert, MoveTypeEnum& eMoveType)
{
	long            lRet = -1;
	S_MAP*          spMap = NULL;
	S_LINE_INFO*    spLineInfo = NULL;
	IplImage        sImage;

	// �I���W�i����ێ�
	m_Image_Original = sOriginal;


	// �O���[�X�P�[���ϊ�
	cvtColor(m_Image_Original, m_Image_Gray, CV_BGR2GRAY);
	if (m_Image_Gray.empty() == true)
	{
		goto FINISH;
	}

	// ��l��
	threshold(m_Image_Gray, m_Image_Binary, m_BinaryThresh, 255, CV_THRESH_BINARY);
	if (m_Image_Binary.empty() == true)
	{
		goto FINISH;
	}

	// �ϊ����ʂ��i�[
	//    sConvert = m_Image_Binary;

	// ���C�����m
	sImage = m_Image_Binary;
	spMap = getMapData(&sImage);
	spLineInfo = getLineData(spMap, eMoveType);
	if (NULL == spLineInfo)
	{
		goto FINISH;
	}

	// ��͌��ʂ�ϊ����ʂ�Mat�ɔ��f
	switch (m_DisplayView)
	{
	case ORIGINAL_LINE_VIEW:
		sConvert = m_Image_Original;
		break;

	case THRESH_VIEW:
		sConvert = m_Image_Binary;
		break;

	case RED_VIEW:
		sConvert = m_Image_Red;
		break;

	default:
		sConvert = m_Image_Original;
		m_DisplayView = ORIGINAL_LINE_VIEW;
		break;
	}

	lRet = 0;


FINISH:
	return (lRet);
}

float ARC2016::GetVerticalMotorAngle(void)
{
	float ret = m_CameraVerticalAngle;

	return (ret);
}

void ARC2016::SetVerticalMotorAngleMax(float angle)
{
	m_CameraVerticalAngleMax = angle;
	return;
}

Mat ARC2016::GetOriginal()
{
	return (m_Image_Original);
}

Mat ARC2016::GetGray()
{
	return (m_Image_Gray);
}

Mat ARC2016::GetBinary()
{
	return (m_Image_Binary);
}

Mat ARC2016::GetRed()
{
	return (m_Image_Red);
}

bool ARC2016::isGoalLine()
{
	bool    bRet = false;


	if (m_GoalDetect == true)
	{
		bRet = true;
	}
	else
	{
		bRet = false;
	}


	return (bRet);
}

S_MAP* ARC2016::getMapData(IplImage* sImage)
{
	S_MAP*      sRet = NULL;
	int         iHeight = 0;
	int         iWidth = 0;
	long        lWidthIndex = 0;
	long        lHeightIndex = 0;
	long        lDetectCount = 0;
	Mat         hsvImage;


	m_MapData.lHeight = (SCREEN_HEIGHT * 0.75);
	m_MapData.lWidth = SCREEN_WIDTH;

	//// HSV �n�ɕϊ�
	//cvtColor(m_Image_Original, hsvImage, CV_BGR2HSV);

	iHeight = m_Image_Original.rows;
	iWidth = m_Image_Original.cols;
	//Mat redImage = Mat(Size(iWidth, iHeight), CV_8UC1);

	for (lHeightIndex = 0; lHeightIndex < sImage->height; lHeightIndex++)
	{
		for (lWidthIndex = 0; lWidthIndex < sImage->width; lWidthIndex++)
		{
			m_MapData.ucMap[lHeightIndex][lWidthIndex] = ((unsigned char)sImage->imageData[sImage->widthStep * lHeightIndex + lWidthIndex * 1] == 0) ? 1 : 0;

			//// �Ԍ��o
			//unsigned char hue = hsvImage.at<Vec3b>(lHeightIndex, lWidthIndex)[0];
			//unsigned char sat = hsvImage.at<Vec3b>(lHeightIndex, lWidthIndex)[1];
			//unsigned char val = hsvImage.at<Vec3b>(lHeightIndex, lWidthIndex)[2];

			//if ((hue < 8 || hue > 168) && (sat > 100))
			//{
			//	lDetectCount++;
			//	redImage.at<unsigned char>(lHeightIndex, lWidthIndex) = 0;
			//}
			//else
			//{
			//	redImage.at<unsigned char>(lHeightIndex, lWidthIndex) = 255;
			//}
		}
	}

	//m_Image_Red = redImage;

	//if (10000 <= lDetectCount)
	//{
	//	m_GoalDetect = true;
	//}

	sRet = (S_MAP*)&m_MapData;


FINISH:
	return (sRet);
}

S_LINE_INFO* ARC2016::getLineData(S_MAP* sMap, MoveTypeEnum& eMoveType)
{
	S_LINE_INFO     *sRet = NULL;
	S_LINE_INFO     sData = { 0x00 };
	long            lWidthIndex = 0;
	long            lLineIndex = 0;
	long            lLineOffset = (SCREEN_HEIGHT / C_DATALINE_MAX);
	long            lOnCount = 0;
	long            lLineCount = 0;
	long            lLineStart = 0;
	long            lLineEnd = 0;
	long            lLineWidth = 0;
	long            lCenterGravity = 0;
	bool            bLineFind = false;
	bool            bLine = false;
	bool            bCorner = false;
	long            lMaxDeviation = 0;
	long            lMaxDeviationMinus = 0;
	long            lCornerIndex = 0;
	long            lBeforeIndex = 0;
	long            lContinuity = 0;
	long            lStopContinuity = 0;
	bool            bStraight = false;
	long            lCornerLine = 0;
	CvPoint         cvPointStart = { 0x00 };
	CvPoint         cvPointEnd = { 0x00 };
	CvScalar        cvColor = Scalar(256, 0, 0);
	CvScalar        cvColor2 = Scalar(256, 256, 0);
	CvFont          cvFont = { 0x00 };
	char            cText[32] = { 0x00 };
	int             iRedData = 0;
	int             iBlueData = 0;
	int             iGreenData = 0;

	/********************* DEBUG INFO *********************/
	cvInitFont(&cvFont, CV_FONT_HERSHEY_SIMPLEX, 1, 1);
	cvPointStart = Point(C_LINE_ORIGIN, 0);
	cvPointEnd = Point(C_LINE_ORIGIN, SCREEN_HEIGHT);
	line(m_Image_Original, cvPointStart, cvPointEnd, cvColor, 2, CV_AA);
	/********************* DEBUG INFO *********************/

	if (NULL == sMap)
	{
		return(sRet);
	}

	/********************* DEBUG INFO *********************/
	for (long deviationCounter = 0; deviationCounter<3; deviationCounter++)
	{
		cvPointStart = Point(160 + m_MaxDeviation[deviationCounter], 0);
		cvPointEnd = Point(160 + m_MaxDeviation[deviationCounter], sMap->lHeight - 1);
		line(m_Image_Original, cvPointStart, cvPointEnd, cvColor);
	}
	for (long deviationCounter = 0; deviationCounter<3; deviationCounter++)
	{
		cvPointStart = Point(160 - m_MaxDeviation[deviationCounter], 0);
		cvPointEnd = Point(160 - m_MaxDeviation[deviationCounter], sMap->lHeight - 1);
		line(m_Image_Original, cvPointStart, cvPointEnd, cvColor, 2, CV_AA);
	}
	/********************* DEBUG INFO *********************/
	m_OnCount = 0;
	/* ������ */
	for (lLineIndex = 0; lLineIndex < C_DATALINE_MAX; lLineIndex++)
		{
		long	lOffset = (lLineIndex + 1)*lLineOffset;
		if (lLineIndex == (C_DATALINE_MAX - 1))
		{
			lOffset--;
		}
		memcpy((void*)sData.sLineData[lLineIndex].cLineData, (void*)&sMap->ucMap[lOffset][0], sizeof(unsigned char)*sMap->lWidth);
		sData.sLineData[lLineIndex].lMapLineNo = lOffset;
		sData.sLineData[lLineIndex].lLineThreshold = C_LINETHRESHOLD;

		/********************* DEBUG INFO *********************/
		cvPointStart = Point(0, sData.sLineData[lLineIndex].lMapLineNo);
		cvPointEnd = Point(sMap->lWidth - 1, sData.sLineData[lLineIndex].lMapLineNo);
		line(m_Image_Original, cvPointStart, cvPointEnd, cvColor, 2, CV_AA);
		/********************* DEBUG INFO *********************/
		lOnCount = 0;
		lLineCount = 0;
		bLineFind = false;
		lLineStart = 0;
		lLineEnd = 0;
		lLineWidth = 0;
		lCenterGravity = 0;

		for (lWidthIndex = 0; lWidthIndex < SCREEN_WIDTH; lWidthIndex++)
		{
			if (1 == sData.sLineData[lLineIndex].cLineData[lWidthIndex])
			{
				lOnCount++;
				m_OnCount++;
				if (C_LINETHRESHOLD == lOnCount)
				{
					/* ���C�������o */
					lLineCount++;
					bLineFind = true;
					lLineStart = lWidthIndex;
				}
			}
			else
			{
				if (true == bLineFind)
				{
					/* ���C���̐؂�ڂ����o */
					lLineEnd = lWidthIndex;
					lLineWidth = (lLineEnd - lLineStart) + 1;
					lCenterGravity = lLineStart + (lLineWidth / 2);
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lStartPos = lLineStart;							/* ���C���J�n�ʒu */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lEndPos = lLineEnd;								/* ���C���I���ʒu */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lLineWidth = lLineWidth;							/* ���C���� */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterGravity = lCenterGravity;					/* ���C�������ʒu */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation = lCenterGravity - C_LINE_ORIGIN;	/* ���S����̃Y���� */
					sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lMapLineNo = sData.sLineData[lLineIndex].lMapLineNo;
					sData.sLineData[lLineIndex].bLine = true;

					if (m_CornerLineWidth <= lLineWidth)
					{
						/* �R�[�i�[�����o */
						sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_CORNER;
						sData.sLineData[lLineIndex].eLineType = E_LINE_CORNER;
						sData.sLineData[lLineIndex].bCorner = true;
						sData.sLineData[lLineIndex].lCornerNum++;

						if (false == sData.sFind[E_FIND_TOP_CORNER].bFind)
						{
							/* ��ԍŏ��Ɍ��o����Corner�����擾 */
							sData.sFind[E_FIND_TOP_CORNER].bFind = true;
							sData.sFind[E_FIND_TOP_CORNER].lDataLineNo = lLineIndex;
							sData.sFind[E_FIND_TOP_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
							sData.sFind[E_FIND_TOP_CORNER].sLineType = E_LINE_CORNER;
							sData.sFind[E_FIND_TOP_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
						}
						sData.sFind[E_FIND_BACK_CORNER].bFind = true;
						sData.sFind[E_FIND_BACK_CORNER].lDataLineNo = lLineIndex;
						sData.sFind[E_FIND_BACK_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
						sData.sFind[E_FIND_BACK_CORNER].sLineType = E_LINE_CORNER;
						sData.sFind[E_FIND_BACK_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
						/* �R�[�i�[�����o�����̂ō��܂ł̍ő�Y���ʂ������� */
						sData.lMaxDeviation = 0;
						sData.lMaxDeviationMinus = 0;
						if (C_CORNER_MAX>sData.lCornerCount)
						{
							sData.sCorner[sData.lCornerCount].lLineNo = lLineIndex;
							sData.lCornerCount++;
						}

						if (sData.sLineData[lLineIndex].lMapLineNo - 1 >= C_CAMERA_LOWANGLE_THRESHOLD)
						{
							if (m_CameraVerticalAngle <= m_CameraVerticalAngleMax)
							{
								cvColor2 = Scalar(0, 0, 256);
								cvPointStart = Point(20, 30);
								char            cCameraText[50] = { 0x00 };
								m_CameraVerticalAngle = m_CameraVerticalAngle + 1.0f;
								snprintf(cCameraText,sizeof(cCameraText), "Camera Down %1.1f", m_CameraVerticalAngle);
								putText(m_Image_Original, cCameraText, cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
							}
						}
					}
					else
					{
						/* ���C�� */
						sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_LINE;
						sData.sLineData[lLineIndex].eLineType = E_LINE_LINE;
						sData.sLineData[lLineIndex].lLineNum++;

						if (false == sData.sFind[E_FIND_TOP_LINE].bFind)
						{
							/* ��ԍŏ��Ɍ��o����Corner�����擾 */
							sData.sFind[E_FIND_TOP_LINE].bFind = true;
							sData.sFind[E_FIND_TOP_LINE].lDataLineNo = lLineIndex;
							sData.sFind[E_FIND_TOP_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
							sData.sFind[E_FIND_TOP_LINE].sLineType = E_LINE_LINE;
							sData.sFind[E_FIND_TOP_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
						}
						sData.sFind[E_FIND_BACK_LINE].bFind = true;
						sData.sFind[E_FIND_BACK_LINE].lDataLineNo = lLineIndex;
						sData.sFind[E_FIND_BACK_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
						sData.sFind[E_FIND_BACK_LINE].sLineType = E_LINE_LINE;
						sData.sFind[E_FIND_BACK_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];

						/* �ő�Y���ʂ�ێ����� */
						if (0 <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
						{
							if (sData.lMaxDeviation <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
							{
								sData.lMaxDeviation = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
							}
						}
						else
						{
							if (sData.lMaxDeviationMinus >= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
							{
								sData.lMaxDeviationMinus = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
							}
						}
					}
					sData.bLine = true;
					sData.sLineData[lLineIndex].lLineCount++;
					/********************* DEBUG INFO *********************/
					cvPointStart = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo - 1);
					cvPointEnd = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo + 1);
					if (E_LINE_CORNER == sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount - 1].eLineType)
					{
						cvColor2 = Scalar(0, 256, 0);
					}
					else
					{
						cvColor2 = Scalar(256, 256, 0);
					}
					line(m_Image_Original, cvPointStart, cvPointEnd, cvColor2, 3, CV_AA);
					/********************* DEBUG INFO *********************/
				}
				lOnCount = 0;
				bLineFind = false;
				lLineStart = 0;
				lLineEnd = 0;
				lLineWidth = 0;
				lCenterGravity = 0;
			}
		}
		if (true == bLineFind)
		{
			/* ���C���̐؂�ڂ����o */
			if (SCREEN_WIDTH <= lWidthIndex)
			{
				lWidthIndex = (SCREEN_WIDTH - 1);
			}
			lLineEnd = lWidthIndex;
			lLineWidth = (lLineEnd - lLineStart) + 1;
			lCenterGravity = lLineStart + (lLineWidth / 2);
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lStartPos = lLineStart;							/* ���C���J�n�ʒu */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lEndPos = lLineEnd;								/* ���C���I���ʒu */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lLineWidth = lLineWidth;							/* ���C���� */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterGravity = lCenterGravity;					/* ���C�������ʒu */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation = lCenterGravity - C_LINE_ORIGIN;	/* ���S����̃Y���� */
			sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lMapLineNo = sData.sLineData[lLineIndex].lMapLineNo;
			sData.sLineData[lLineIndex].bLine = true;

			if (m_CornerLineWidth <= lLineWidth)
			{
				/* �R�[�i�[�����o */
				sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_CORNER;
				sData.sLineData[lLineIndex].eLineType = E_LINE_CORNER;
				sData.sLineData[lLineIndex].bCorner = true;
				sData.sLineData[lLineIndex].lCornerNum++;

				if (false == sData.sFind[E_FIND_TOP_CORNER].bFind)
				{
					/* ��ԍŏ��Ɍ��o����Corner�����擾 */
					sData.sFind[E_FIND_TOP_CORNER].bFind = true;
					sData.sFind[E_FIND_TOP_CORNER].lDataLineNo = lLineIndex;
					sData.sFind[E_FIND_TOP_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
					sData.sFind[E_FIND_TOP_CORNER].sLineType = E_LINE_CORNER;
					sData.sFind[E_FIND_TOP_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
				}
				sData.sFind[E_FIND_BACK_CORNER].bFind = true;
				sData.sFind[E_FIND_BACK_CORNER].lDataLineNo = lLineIndex;
				sData.sFind[E_FIND_BACK_CORNER].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
				sData.sFind[E_FIND_BACK_CORNER].sLineType = E_LINE_CORNER;
				sData.sFind[E_FIND_BACK_CORNER].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];

				/* �R�[�i�[�����o�����̂ō��܂ł̍ő�Y���ʂ������� */
				sData.lMaxDeviation = 0;
				sData.lMaxDeviationMinus = 0;
				if (C_CORNER_MAX>sData.lCornerCount)
				{
					sData.sCorner[sData.lCornerCount].lLineNo = lLineIndex;
					sData.lCornerCount++;
				}

				if (sData.sLineData[lLineIndex].lMapLineNo - 1 >= C_CAMERA_LOWANGLE_THRESHOLD)
				{
					if (m_CameraVerticalAngle <= m_CameraVerticalAngleMax)
					{
						cvColor2 = Scalar(0, 0, 256);
						cvPointStart = Point(20, 30);
						char            cCameraText[50] = { 0x00 };
						m_CameraVerticalAngle = m_CameraVerticalAngle + 1.0f;
						snprintf(cCameraText,sizeof(cCameraText), "Camera Down %1.1f", m_CameraVerticalAngle);
						putText(m_Image_Original, cCameraText, cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
					}
				}
			}
			else
			{
				/* ���C�� */
				sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].eLineType = E_LINE_LINE;
				sData.sLineData[lLineIndex].eLineType = E_LINE_LINE;
				sData.sLineData[lLineIndex].lLineNum++;

				if (false == sData.sFind[E_FIND_TOP_LINE].bFind)
				{
					/* ��ԍŏ��Ɍ��o����Corner�����擾 */
					sData.sFind[E_FIND_TOP_LINE].bFind = true;
					sData.sFind[E_FIND_TOP_LINE].lDataLineNo = lLineIndex;
					sData.sFind[E_FIND_TOP_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
					sData.sFind[E_FIND_TOP_LINE].sLineType = E_LINE_LINE;
					sData.sFind[E_FIND_TOP_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];
				}
				sData.sFind[E_FIND_BACK_LINE].bFind = true;
				sData.sFind[E_FIND_BACK_LINE].lDataLineNo = lLineIndex;
				sData.sFind[E_FIND_BACK_LINE].lLineCountNo = sData.sLineData[lLineIndex].lLineCount;
				sData.sFind[E_FIND_BACK_LINE].sLineType = E_LINE_LINE;
				sData.sFind[E_FIND_BACK_LINE].sLine = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount];

				/* �ő�Y���ʂ�ێ����� */
				if (0 <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
				{
					if (sData.lMaxDeviation <= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
					{
						sData.lMaxDeviation = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
					}
				}
				else
				{
					if (sData.lMaxDeviationMinus >= sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation)
					{
						sData.lMaxDeviationMinus = sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount].lCenterDeviation;
					}
				}
			}
			sData.bLine = true;
			sData.sLineData[lLineIndex].lLineCount++;

			/********************* DEBUG INFO *********************/
			cvPointStart = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo - 1);
			cvPointEnd = Point(lCenterGravity, sData.sLineData[lLineIndex].lMapLineNo + 1);
			if (E_LINE_CORNER == sData.sLineData[lLineIndex].sLine[sData.sLineData[lLineIndex].lLineCount - 1].eLineType)
			{
				cvColor2 = Scalar(0, 256, 0);
			}
			else
			{
				cvColor2 = Scalar(256, 256, 0);
			}
			line(m_Image_Original, cvPointStart, cvPointEnd, cvColor2, 3, CV_AA);
			/********************* DEBUG INFO *********************/
		}
	}

	E_CORNER_TYPE eCornerType = E_CORNER_NONE;

	if ((100  > m_OnCount) ||
		(2000 < m_OnCount))
	{
		eMoveType = PATTERN_MISSING;
		sData.bLine = false;
	}
	if ((true == sData.bLine) && (0 != sData.lCornerCount))
	{
		if (1 == sData.lCornerCount)
		{
			/* 1�R�[�i�[ */
			if (m_CornerIndex <= sData.sCorner[0].lLineNo)
			{
				long linecount = 0;
				for (long lineindex = sData.sCorner[sData.lCornerCount - 1].lLineNo - 2; lineindex >= 0; lineindex--)
				{
					linecount += (0<sData.sLineData[lineindex].lLineNum) ? 1 : 0;
				}
				if (linecount >= 2)
				{
					/* ���i�Ƃ��Ĕ��f���� */
					bStraight = true;
				}
				/* �R�[�i�[�쓮����߁A�␳�쓮�݂̂ŃR�[�i�[�����؂� */
				else
				{
					eCornerType = GetCornerType(sData, 0);
					if (E_CORNER_RIGHT == eCornerType)
					{
#if 0
						//                        eMoveType = PATTERN_TURN_RIGHT_90;
#endif
						eMoveType = PATTERN_FRONT_RIGHT_90;
					}
					else
					{
#if 0
						//                        eMoveType = PATTERN_TURN_LEFT_90;
#endif
						eMoveType = PATTERN_FRONT_LEFT_90;
					}
				}
			}
			else
			{
				bStraight = true;
			}
		}
		else
		{
			/* �����R�[�i�[ */
			for (lCornerIndex = 0; lCornerIndex < sData.lCornerCount; lCornerIndex++)
			{
				if (0 != lCornerIndex)
				{
					/* ���O��Line���R�[�i�[�����o���Ă��邩�H */
					if (sData.sCorner[lCornerIndex - 1].lLineNo == (sData.sCorner[lCornerIndex].lLineNo - 1))
					{
						/* ���o */
						lContinuity++;
						lCornerLine = sData.sCorner[lCornerIndex].lLineNo;
					}
					else
					{
						/* �����o */
						lStopContinuity++;
					}
				}
			}
			if ((0 != lContinuity) &&
				(0 == lStopContinuity))
			{
				/* �P�ƃR�[�i�[ */
				if (m_CornerIndex <= sData.sCorner[sData.lCornerCount - 1].lLineNo)
				{
					long linecount = 0;
					for (long lineindex = sData.sCorner[sData.lCornerCount - 1].lLineNo - 2; lineindex >= 0; lineindex--)
					{
						linecount += (0<sData.sLineData[lineindex].lLineNum) ? 1 : 0;
					}
					if (linecount >= 2)
					{
						/* ���i�Ƃ��Ĕ��f���� */
						bStraight = true;
					}
					else
					{
						eCornerType = GetCornerType(sData, sData.lCornerCount - 1);
						if (E_CORNER_RIGHT == eCornerType)
						{
#if 0
							//                            eMoveType = PATTERN_TURN_RIGHT_90;
#endif
							eMoveType = PATTERN_FRONT_RIGHT_90;
						}
						else
						{
#if 0
							//                            eMoveType = PATTERN_TURN_LEFT_90;
#endif
							eMoveType = PATTERN_FRONT_LEFT_90;
						}
					}
				}
				else
				{
					bStraight = true;
				}
			}
			else
			{
				/* �Ɨ����������̃R�[�i�[������ */
				bStraight = true;
			}
		}
	}

	long	lFrontLineNo = 0;
	long	lFrontCornerNo = 0;
	long	lLineCountIndex = 0;
	long	lTopBackDiff = 0;
	bool	bLineCheck = false;

	if (((true == sData.bLine) && (0 == sData.lCornerCount)) ||
		(true == bStraight))
	{
		if (0 == sData.lCornerCount)
		{
			/* �R�[�i�[�Ȃ� */
			bLineCheck = true;
		}
		if ((true == sData.sFind[E_FIND_TOP_LINE].bFind) &&
			(true == sData.sFind[E_FIND_TOP_CORNER].bFind))
		{
			if (sData.sFind[E_FIND_TOP_LINE].lDataLineNo >= sData.sFind[E_FIND_TOP_CORNER].lDataLineNo)
			{
				/* TOP�R�[�i�[�����TOP���C�������� */
				bLineCheck = true;
			}
			else if (sData.sFind[E_FIND_TOP_LINE].lDataLineNo < sData.sFind[E_FIND_TOP_CORNER].lDataLineNo)
			{
				/* TOP�R�[�i�[����ԏ� */
				sData.sFind[E_FIND_TOP_LINE] = sData.sFind[E_FIND_TOP_CORNER];
				bLineCheck = true;
			}
			else if (sData.sFind[E_FIND_BACK_LINE].lDataLineNo < sData.sFind[E_FIND_BACK_CORNER].lDataLineNo)
			{
				/* BACK���C����艺��BACK�R�[�i�[���� */
				sData.sFind[E_FIND_BACK_LINE] = sData.sFind[E_FIND_BACK_CORNER];
				bLineCheck = true;
			}
			else if (sData.sFind[E_FIND_BACK_LINE].lDataLineNo >= sData.sFind[E_FIND_BACK_CORNER].lDataLineNo)
			{
				/* BACK���C������ԉ� */
				bLineCheck = true;
			}
		}
		else if ((false == sData.sFind[E_FIND_TOP_LINE].bFind) &&
			(true == sData.sFind[E_FIND_TOP_CORNER].bFind))
		{
			/* �R�[�i�[�����Ȃ� */
			sData.sFind[E_FIND_TOP_LINE] = sData.sFind[E_FIND_TOP_CORNER];
			sData.sFind[E_FIND_BACK_LINE] = sData.sFind[E_FIND_BACK_CORNER];
			bLineCheck = true;
		}
		else
		{

		}

		if (true == bLineCheck)
		{
			if (m_MaxDeviation[0] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
			{
				if ((m_MaxDeviation[0] * 1.5) >= labs(lTopBackDiff))
				{
					eMoveType = PATTERN_FRONT;
				}
				else
				{
					lTopBackDiff = sData.sFind[E_FIND_TOP_LINE].sLine.lStartPos - sData.sFind[E_FIND_BACK_LINE].sLine.lStartPos;
					if (0 < sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)
					{
						if ((m_MaxDeviation[0] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[1] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_RIGHT_15;
						}

						if ((m_MaxDeviation[1] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[2] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_RIGHT_30;
						}
						else
						{
							eMoveType = PATTERN_FRONT_RIGHT_45;
						}
					}
					else
					{
						if ((m_MaxDeviation[0] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[1] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_LEFT_15;
						}
						else if ((m_MaxDeviation[1] * 1.5) < labs(lTopBackDiff)
							&& (m_MaxDeviation[2] * 1.5) >= labs(lTopBackDiff))
						{
							eMoveType = PATTERN_FRONT_LEFT_30;
						}
						else
						{
							eMoveType = PATTERN_FRONT_LEFT_45;
						}
					}
				}
			}
			else
			{
				if (0 < sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)
				{
					if ((m_MaxDeviation[0] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[1] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_RIGHT_15;
					}
					else if ((m_MaxDeviation[1] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[2] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_RIGHT_30;
					}
					else
					{
						eMoveType = PATTERN_FRONT_RIGHT_45;
					}
				}
				else
				{
					if ((m_MaxDeviation[0] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[1] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_LEFT_15;
					}
					else if ((m_MaxDeviation[1] < labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation))
						&& (m_MaxDeviation[2] >= labs(sData.sFind[E_FIND_TOP_LINE].sLine.lCenterDeviation)))
					{
						eMoveType = PATTERN_FRONT_LEFT_30;
					}
					else
					{
						eMoveType = PATTERN_FRONT_LEFT_45;
					}
				}
			}
			cvPointStart = Point(sData.sFind[E_FIND_TOP_LINE].sLine.lStartPos, sData.sFind[E_FIND_TOP_LINE].sLine.lMapLineNo);
			cvPointEnd = Point(sData.sFind[E_FIND_BACK_LINE].sLine.lStartPos, sData.sFind[E_FIND_BACK_LINE].sLine.lMapLineNo);
			cvColor2 = Scalar(256, 0, 256);
			line(m_Image_Original, cvPointStart, cvPointEnd, cvColor2, 2, CV_AA);
		}
		else
		{

		}
	}

	if (eMoveType == PATTERN_STOP)
	{
		eMoveType = PATTERN_MISSING;
	}

	if ((eMoveType != PATTERN_MISSING) && (eMoveType != PATTERN_STOP))
	{
		m_BeforeMoveType = eMoveType;
	}

	cvColor2 = Scalar(0, 0, 256);
	cvPointStart = Point(20, 20);
	switch (eMoveType)
	{
	case PATTERN_FRONT:
		putText(m_Image_Original, "^^^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_TURN_LEFT_90:
		putText(m_Image_Original, "Corner[LEFT]", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_TURN_RIGHT_90:
		putText(m_Image_Original, "Corner[RIGHT]", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_FRONT_LEFT_15:
		putText(m_Image_Original, "^^^<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_FRONT_LEFT_30:
		putText(m_Image_Original, "^^^<<<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_FRONT_LEFT_45:
		putText(m_Image_Original, "^^^<<<<<<^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_FRONT_RIGHT_15:
		putText(m_Image_Original, "^^^>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_FRONT_RIGHT_30:
		putText(m_Image_Original, "^^^>>>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	case PATTERN_FRONT_RIGHT_45:
		putText(m_Image_Original, "^^^>>>>>>^^^", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;
	default:
		putText(m_Image_Original, "unknown.", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);
		break;

	}
	char cBuf[16] = { 0x00 };
	cvPointStart.y = 50;
	snprintf(cBuf, sizeof(cBuf), "%ld", m_OnCount);
	putText(m_Image_Original, cBuf, cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2, 2, CV_AA);

	//if (isGoalLine() == true)
	//{
	//	cvPointStart.x = 20;
	//	cvPointStart.y = 120;
	//	putText(m_Image_Original, "G O A L !!!!", cvPointStart, FONT_HERSHEY_SIMPLEX, 1.2, cvColor2);
	//	eMoveType = PATTERN_PUT;
	//}

	m_LineInfo = sData;
	sRet = (S_LINE_INFO*)&m_LineInfo;

	return (sRet);
}

E_CORNER_TYPE ARC2016::GetCornerType(S_LINE_INFO &sLineInfo, const long lCornerIndex)
{
	E_CORNER_TYPE	eRet = E_CORNER_NONE;
	long			lLineIndex = 0;
	long			lWidthIndex = 0;
	long			lLeftCount = 0;
	long			lRightCount = 0;
	long			lOffCountL = 0;
	long			lOffCountR = 0;
	long			lLineNo = -1;
	long			lLineCountIndex = 0;
	long			lLineCountIndex2 = 0;
	long			lLineCount = 0;

	/* �R�[�i�[�����o������������艺�̑��������m�F */
#if 0
	//    lLineIndex = sLineInfo.sCorner[lCornerIndex].lLineNo +1;
	//    for (lLineIndex; lLineIndex < C_DATALINE_MAX; lLineIndex++ )
#endif
	for (lLineIndex = sLineInfo.sCorner[lCornerIndex].lLineNo + 1; lLineIndex < C_DATALINE_MAX; lLineIndex++)
	{
		switch (sLineInfo.sLineData[lLineIndex].eLineType)
		{
		case E_LINE_LINE:
			/* ���C�� */
			/* ��������̕������C���m�F */
			if (1<sLineInfo.sLineData[lLineIndex].lLineCount)
			{
				if (false == sLineInfo.sLineData[lLineIndex].bCorner)
				{
					/* �S�ă��C�� */
					for (lWidthIndex = 0; lWidthIndex < sLineInfo.sLineData[lLineIndex].sLine[0].lStartPos; lWidthIndex++)
					{
						if (0 == sLineInfo.sLineData[lLineNo].cLineData[lWidthIndex])
						{
							lOffCountL++;
						}
					}
					for (lWidthIndex = sLineInfo.sLineData[lLineIndex].sLine[sLineInfo.sLineData[lLineIndex].lLineCount - 1].lStartPos; lWidthIndex > SCREEN_WIDTH; lWidthIndex--)
					{
						if (0 == sLineInfo.sLineData[lLineNo].cLineData[lWidthIndex])
						{
							lOffCountR++;
						}
					}
					if (lOffCountL>lOffCountR)
					{
						eRet = E_CORNER_RIGHT;
					}
					else
					{
						eRet = E_CORNER_LEFT;
					}
					goto FINISH;
				}
				else
				{
					/* �R�[�i�[�܂� */
					for (lLineCountIndex = 0; lLineCountIndex < sLineInfo.sLineData[lLineIndex].lLineCount; lLineCountIndex++)
					{
						if (E_LINE_CORNER == sLineInfo.sLineData[lLineIndex].sLine[lLineCountIndex].eLineType)
						{
							break;
						}
					}
					for (lLineCountIndex2 = 0; lLineCountIndex2 < sLineInfo.sLineData[lLineIndex].lLineCount; lLineCountIndex2++)
					{
						if (E_LINE_LINE == sLineInfo.sLineData[lLineIndex].sLine[lLineCountIndex2].eLineType)
						{
							break;
						}
					}
					if ((sLineInfo.sLineData[lLineIndex].lLineCount > lLineCountIndex) &&
						(sLineInfo.sLineData[lLineIndex].lLineCount > lLineCountIndex2))
					{
						/* ���C���J�n�ʒu�ƃR�[�i�[���S�ʒu���獶�E�𔻒f���� */
						if (sLineInfo.sLineData[lLineIndex].sLine[lLineCountIndex2].lStartPos < sLineInfo.sLineData[lLineIndex].sLine[lLineCountIndex2].lCenterGravity)
						{
							/* �E�R�[�i�[ */
							eRet = E_CORNER_RIGHT;
						}
						else
						{
							/* ���R�[�i�[ */
							eRet = E_CORNER_LEFT;
						}
						goto FINISH;
					}
				}
			}

			if (sLineInfo.sLineData[sLineInfo.sCorner[lCornerIndex].lLineNo].sLine[0].lCenterGravity <= sLineInfo.sLineData[lLineIndex].sLine[0].lStartPos)
			{
				/* ���R�[�i�[ */
				eRet = E_CORNER_LEFT;
			}
			else
			{
				/* �E�R�[�i�[ */
				eRet = E_CORNER_RIGHT;
			}
			goto FINISH;
			break;
		case E_LINE_CORNER:
		case E_LINE_NONE:
			break;
		default:
			break;
		}
	}
	/* ���̑������Ń��C�������o�ł��Ȃ��ꍇ�́A��̑��������烉�C�����m�F���� */
#if 0
	//    lLineIndex = sLineInfo.sCorner[lCornerIndex].lLineNo -2;
	//    for (lLineIndex; lLineIndex >= 0; lLineIndex-- )
#endif
	for (lLineIndex = sLineInfo.sCorner[lCornerIndex].lLineNo - 2; lLineIndex >= 0; lLineIndex--)
	{
		switch (sLineInfo.sLineData[lLineIndex].eLineType)
		{
		case E_LINE_LINE:
			/* ���C�� */
			/* �R�[�i�[�̒��S�ʒu�Ɖ��̑������Ō��o�������C���̊J�n�ʒu����R�[�i�[�����𔻒� */
			if (sLineInfo.sLineData[sLineInfo.sCorner[lCornerIndex].lLineNo].sLine[0].lCenterGravity <= sLineInfo.sLineData[lLineIndex].sLine[0].lStartPos)
			{
				/* �E�R�[�i�[ */
				eRet = E_CORNER_RIGHT;
			}
			else
			{
				/* ���R�[�i�[ */
				eRet = E_CORNER_LEFT;
			}
			goto FINISH;
			break;
		case E_LINE_CORNER:
		case E_LINE_NONE:
			break;
		default:
			break;
		}
	}
	/* �������A���E��ON�̊����Ō��߂� */
	for (lLineIndex = (C_DATALINE_MAX / 2); lLineIndex < C_DATALINE_MAX; lLineIndex++)
	{
		for (lWidthIndex = 0; lWidthIndex < SCREEN_WIDTH; lWidthIndex++)
		{
			if (1 == sLineInfo.sLineData[lLineIndex].cLineData[lWidthIndex])
			{
				if (lWidthIndex >= (SCREEN_WIDTH / 2))
				{
					lRightCount++;
				}
				else
				{
					lLeftCount++;
				}
			}
		}
	}
	for (lLineIndex = C_DATALINE_MAX - 1; lLineIndex >= 0; lLineIndex--)
	{
		if ((sLineInfo.sLineData[lLineIndex].bLine == true) ||
			(sLineInfo.sLineData[lLineIndex].bCorner == true))
		{
			lLineNo = lLineIndex;
		}
	}
	for (lWidthIndex = 0; lWidthIndex < (SCREEN_WIDTH / 2); lWidthIndex++)
	{
		if (0 == sLineInfo.sLineData[lLineNo].cLineData[lWidthIndex])
		{
			lOffCountL++;
		}
	}
	for (lWidthIndex = (SCREEN_WIDTH - 1); lWidthIndex >(SCREEN_WIDTH / 2); lWidthIndex--)
	{
		if (0 == sLineInfo.sLineData[lLineNo].cLineData[lWidthIndex])
		{
			lOffCountR++;
		}
	}
	if (lLeftCount <= lRightCount)
	{
		/* �E�R�[�i�[ */
		if (lOffCountL>lOffCountR)
		{
			eRet = E_CORNER_RIGHT;
		}
		else
		{
			eRet = E_CORNER_LEFT;
		}
	}
	else
	{
		/* ���R�[�i�[ */
		if (lOffCountL <= lOffCountR)
		{
			eRet = E_CORNER_LEFT;
		}
		else
		{
			eRet = E_CORNER_RIGHT;
		}
	}
FINISH:
	return (eRet);
}