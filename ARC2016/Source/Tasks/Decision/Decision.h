#pragma once

#include "Source/Common/Common.h"
#include "Source/Framework/TaskBase/TaskBase.h"
#include "Source/Framework/Serial/Serial.h"
#include "Source/Tasks/Camera/Camera.h"
#include "Source/Tasks/DataSender/DataSender.h"
#include "Source/Tasks/SensorMonitor/SensorMonitor.h"
#include "Source/Parts/GyroSensor/GyroSensor.h"

typedef enum _E_DISTANCE_DATA_ORDER_TYPE
{
	E_DISTANCE_ORDER_FRONT = 0x00,
	E_DISTANCE_ORDER_RIGHT,
	E_DISTANCE_ORDER_LEFT,
} E_DISTANCE_DATA_ORDER_TYPE;

typedef enum _E_DISTANCE_JUDGE_TYPE
{
	E_DISTANCE_JUDGE_NONE = 0x00,	// 壁無し
	E_DISTANCE_JUDGE_ALL,			// 3方向  壁有
	E_DISTANCE_JUDGE_FRONT_RIGHT,	// 前・右 壁有
	E_DISTANCE_JUDGE_FRONT_LEFT,	// 前・左 壁有
	E_DISTANCE_JUDGE_RIGHT_LEFT,	// 右・左 壁有
	E_DISTANCE_JUDGE_FRONT,			// 前     壁有
	E_DISTANCE_JUDGE_RIGHT,			// 右     壁有
	E_DISTANCE_JUDGE_LEFT,			// 左     壁有
} E_DISTANCE_JUDGE_TYPE;

typedef enum _E_GYRO_MODE_TYPE
{
	E_GYRO_MODE_UNKNOWN = 0x00,
	E_GYRO_MODE_FLAT,
	E_GYRO_MODE_SLOPE,
} E_GYRO_MODE_TYPE;

namespace ARC2016
{
	namespace Tasks
	{
		class Decision : public ARC2016::Framework::TaskBase
		{
		public:
			Decision(SensorMonitor* sensMonitor, DataSender* dataSender);
			virtual ~Decision();



		protected:

		private:

			SensorMonitor*			m_SensorMonitor;
			DataSender*				m_DataSender;

			int						m_SendFlag;
			int						m_MyHeartBeatFlag;
			int						m_MiconHeartBeatFlag;
			int						m_HeartBeatInterval;

			// 仮
			long					m_FlontDistanceJudgementValue;
			long					m_RightDistanceJudgementValue;
			long					m_LeftDistanceJudgementValue;
			double					m_SlopeJudgementValue;

			ResultEnum				initialize();
			ResultEnum				taskMain();
			ResultEnum				finalize();

			// ハートビート関連
			void					HeartBeatCheck();
			void					SendData();
			void					ReadData();

			// 動作判定関連
			long					ConversionLineTraceFlag(long flag);
			void				    MoveTypeDecision();
			long					DistanceCheck(long typeLineTrace);
			E_DISTANCE_JUDGE_TYPE	DistanceSensorCheck(std::vector<long> data);
			E_GYRO_MODE_TYPE		GyroCheck();

		};
	}
}