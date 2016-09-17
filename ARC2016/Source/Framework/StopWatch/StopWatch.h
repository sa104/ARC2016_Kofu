#pragma once

#include <chrono>

namespace ARC2016
{
	namespace Framework
	{
		class StopWatch
		{
		public:

			StopWatch();
			virtual ~StopWatch();

			void Start();
			void Stop();
			bool IsRunning();
			long GetStoppedTime();
			long GetLapTime();

		protected :

		private :

			bool									m_Running;
			std::chrono::system_clock::time_point	m_StartTime;
			std::chrono::system_clock::time_point	m_EndTime;

		};
	}
}

