#pragma once

#include "Source/Common/Common.h"
#include <Robuffer.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/cap_winrt.hpp>



using namespace ARC2016;
using namespace concurrency;
using namespace cv;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Media;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml::Media::Imaging;

#define BLOCK (10)

#define C_LINETHRESHOLD			(15)
#define C_DATALINE_MAX			(12)
#define C_LINECOUNT_MAX			(10)
#define C_DATALINE_VERTICAL_MAX	(5)
#define C_LINE_ORIGIN			(SCREEN_WIDTH/2)
#define	C_LINE_WIDTH			(80)
#define C_LINE_MAXDAVIATION		(140)
#define C_CORNER_MAX			(4)
#define C_CAMERA_LOWANGLE_THRESHOLD (120)


// ����p�^�[��
typedef enum _MoveTypeEnum
{
	MOVE_PATTERN_MIN = 0,
	PATTERN_STOP = MOVE_PATTERN_MIN,    //  0: ��~
	PATTERN_FRONT,                      //  1: �O�i
	PATTERN_BACK,                       //  2: ���
	PATTERN_TURN_LEFT_ONCE,             //  3: ������ �P�� (15��)
	PATTERN_TURN_LEFT_45,               //  4: ������ 45��
	PATTERN_TURN_LEFT_90,               //  5: ������ 90��
	PATTERN_TURN_RIGHT_ONCE,            //  6: �E���� �P�� (15��)
	PATTERN_TURN_RIGHT_45,              //  7: �E���� 45��
	PATTERN_TURN_RIGHT_90,              //  8: �E���� 90��
	PATTERN_FRONT_LEFT_15,              //  9: �O�i���� 15��
	PATTERN_FRONT_LEFT_30,              // 10: �O�i���� 30��
	PATTERN_FRONT_LEFT_45,              // 11: �O�i���� 45��
	PATTERN_FRONT_LEFT_90,              // 12: �O�i���� 90��
	PATTERN_FRONT_RIGHT_15,             // 13: �O�i���E 15��
	PATTERN_FRONT_RIGHT_30,             // 14: �O�i���E 30��
	PATTERN_FRONT_RIGHT_45,             // 15: �O�i���E 45��
	PATTERN_FRONT_RIGHT_90,             // 16: �O�i���E 90��
	PATTERN_MISSING,                    // 17: ���C���������Ă���
	PATTERN_PUT,                        // 18: ���Y�i��u��
	MOVE_PATTERN_MAX,
} MoveTypeEnum;

typedef enum _E_MOVE_TYPE
{
	E_MOVE_UNKNOWN = 0x00,
	E_MOVE_STRAIGHT,
	E_MOVE_RIGHT,
	E_MOVE_LEFT,
	E_MOVE_TURNRIGHT,
	E_MOVE_TURNLEFT,
} E_MOVE_TYPE;

typedef enum _E_CORNER_TYPE
{
	E_CORNER_NONE = 0x00,
	E_CORNER_RIGHT,
	E_CORNER_LEFT,
	E_CORNER_UNKNOWN,
} E_CORNER_TYPE;

typedef enum _E_LINE_TYPE
{
	E_LINE_NONE = 0x00,
	E_LINE_LINE,
	E_LINE_CORNER,
} E_LINE_TYPE;

typedef enum _E_FIND_TYPE
{
	E_FIND_TOP_LINE = 0x00,
	E_FIND_TOP_CORNER,
	E_FIND_BACK_LINE,
	E_FIND_BACK_CORNER,
	E_FIND_MAX,
} E_FIND_TYPE;

typedef struct _S_MAP
{
	unsigned char	ucMap[SCREEN_HEIGHT][SCREEN_WIDTH];
	long			lWidth;
	long			lHeight;
} S_MAP;

typedef struct _S_LINE
{
	E_LINE_TYPE		eLineType;
	long			lStartPos;
	long			lEndPos;
	long			lCenterDeviation;
	long			lCenterGravity;
	long			lLineWidth;
	long			lMapLineNo;
} S_LINE;

typedef struct _S_LINE_DATA
{
	unsigned char	cLineData[SCREEN_WIDTH];
	S_LINE			sLine[C_LINECOUNT_MAX];
	long			lLineCount;
	long			lMapLineNo;
	long			lLineOrigin;
	long			lLineThreshold;
	long			lLineNum;
	long			lCornerNum;
	bool			bLine;
	bool			bCorner;
	E_LINE_TYPE		eLineType;
} S_LINE_DATA;

typedef struct _S_CORNER_INFO
{
	long			lLineNo;
	E_CORNER_TYPE	eCornerType;
} S_CORNER_INFO;

typedef struct _S_DATAFIND
{
	S_LINE		sLine;
	E_LINE_TYPE	sLineType;
	long		lDataLineNo;
	long		lLineCountNo;
	bool		bFind;
} S_DATAFIND;

typedef struct
{
	S_LINE_DATA		sLineData[C_DATALINE_MAX];
	S_CORNER_INFO	sCorner[C_CORNER_MAX];
	S_DATAFIND		sFind[E_FIND_MAX];
	long			lCornerCount;
	long			lMaxDeviation;
	long			lMaxDeviationMinus;
	bool			bLine;
} S_LINE_INFO;

enum
{
	VIEW_MIN = 0,
	ORIGINAL_LINE_VIEW = VIEW_MIN,
	THRESH_VIEW,
	RED_VIEW,
	VIEW_MAX = RED_VIEW,
};

namespace ARC2016
{
	Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ ConvertProc(Windows::Media::VideoFrame^ source, const bool isDisplay);

	void    ChangeBinaryThresh(const long lThresh);
	void    ChangeConvertView();
	long    CaptureToDirection(IplImage* pOriginal, MoveTypeEnum& eMoveType);
	long    CaptureToDirection(Mat& sOriginal, Mat& sConvert, MoveTypeEnum& eMoveType);
	float   GetVerticalMotorAngle(void);
	void    SetVerticalMotorAngleMax(float angle);
	Mat     GetOriginal();
	Mat     GetGray();
	Mat     GetBinary();
	Mat     GetRed();
	void    SetCornerLineWidth(long lCornerLineWidth);
	void    SetMaxDeviation1(long lMaxDeviation);
	void    SetMaxDeviation2(long lMaxDeviation);
	void    SetMaxDeviation3(long lMaxDeviation);
	void    SetCornerIndex(long lCornerIndex);
	long    GetCornerLineWidth();
	long    GetMaxDeviation1();
	long    GetMaxDeviation2();
	long    GetMaxDeviation3();
	long    GetCornerIndex();
	long	GetMoveType();

	S_MAP* getMapData(IplImage* sImage);
	S_LINE_INFO* getLineData(S_MAP* sMap, MoveTypeEnum& eMoveType);
	E_CORNER_TYPE GetCornerType(S_LINE_INFO &sLineInfo, const long lCornerIndex);
	bool isGoalLine();
}