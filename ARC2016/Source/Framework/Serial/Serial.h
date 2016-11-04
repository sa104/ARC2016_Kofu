#pragma once

#include "Source/Common/Common.h"

namespace ARC2016
{
	namespace Framework
	{
		class Serial
		{
		public:

			Serial(Windows::Devices::Enumeration::DeviceInformation^ device);
			virtual ~Serial();

			void Initialize();
			ResultEnum Send(char* const pBuffer, const long lSize);
			ResultEnum Send(unsigned char* const pBuffer, const long lSize);
			ResultEnum Receive(char* const pBuffer, const long lSize);
			ResultEnum Receive(unsigned char* const pBuffer, const long lSize);

			bool	m_InitializeComplete;

		protected:

		private:

			Windows::Devices::Enumeration::DeviceInformation^		m_Device;
			Windows::Devices::SerialCommunication::SerialDevice^	m_SerialPort;
			Windows::Storage::Streams::DataReader^					m_Reader;
			Windows::Storage::Streams::DataWriter^					m_Writer;

		};
	}
};
