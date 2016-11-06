#include "pch.h"
#include "Source/Framework/Serial/Serial.h"

using namespace ARC2016;
using namespace ARC2016::Framework;
using namespace concurrency;
using namespace Platform;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;

Serial::Serial(Windows::Devices::Enumeration::DeviceInformation^ device)
 : m_Device(device)
 , m_InitializeComplete(false)
{
	// nop.
}

Serial::~Serial()
{
	if (m_Reader != nullptr)
	{
		delete m_Reader;
		m_Reader = nullptr;
	}

	if (m_Writer != nullptr)
	{
		delete m_Writer;
		m_Writer = nullptr;
	}
}

void Serial::Initialize()
{
	create_task(SerialDevice::FromIdAsync(m_Device->Id)).then([this](SerialDevice^ serial)
	{
		if (serial != nullptr)
		{
			m_SerialPort = serial;

			TimeSpan t;
			t.Duration = 10000L;

			m_SerialPort->WriteTimeout = t;
			m_SerialPort->ReadTimeout = t;
			m_SerialPort->BaudRate = 19200;
			m_SerialPort->Parity = SerialParity::None;
			m_SerialPort->StopBits = SerialStopBitCount::One;
			m_SerialPort->DataBits = 8;
			m_SerialPort->Handshake = SerialHandshake::None;
		
			m_Reader = ref new DataReader(m_SerialPort->InputStream);
			m_Reader->InputStreamOptions = InputStreamOptions::Partial;

			m_Writer = ref new DataWriter(m_SerialPort->OutputStream);

			m_InitializeComplete = true;
		}
	});
}

ResultEnum Serial::Send(char* const pBuffer, const long lSize)
{
	ResultEnum eRet = E_RET_ABNORMAL;

	if (m_InitializeComplete == false)
	{
		return E_RET_NORMAL;
	}

	unsigned char* p = (unsigned char *)pBuffer;
	m_Writer->WriteBytes(ArrayReference<unsigned char>(p, lSize));

	task<size_t> asyncTask = create_task(m_Writer->StoreAsync());
	try
	{
		asyncTask.wait();
		if (asyncTask.get() < 0)
		{
			eRet = E_RET_ABNORMAL;
		}
		else
		{
			eRet = E_RET_NORMAL;
		}
	}
	catch (...)
	{
		eRet = E_RET_ABNORMAL;
	}


	return (eRet);
}

ResultEnum Serial::Send(unsigned char* const pBuffer, const long lSize)
{
	ResultEnum eRet = E_RET_ABNORMAL;

	if (m_InitializeComplete == false)
	{
		return E_RET_NORMAL;
	}

	unsigned char* p = pBuffer;
	m_Writer->WriteBytes(ArrayReference<unsigned char>(p, lSize));

	task<size_t> asyncTask = create_task(m_Writer->StoreAsync());
	try
	{
		asyncTask.wait();
		if (asyncTask.get() < 0)
		{
			eRet = E_RET_ABNORMAL;
		}
		else
		{
			eRet = E_RET_NORMAL;
		}
	}
	catch (...)
	{
		eRet = E_RET_ABNORMAL;
	}


	return (eRet);
}

ResultEnum Serial::Receive(char* const pBuffer, const long lSize)
{
	ResultEnum	eRet = E_RET_ABNORMAL;

	if (m_InitializeComplete == false)
	{
		return E_RET_NORMAL;
	}

	task<unsigned int> asyncTask = create_task(m_Reader->LoadAsync(lSize));
	asyncTask.wait();
	if (asyncTask.get() < 0)
	{
		eRet = E_RET_ABNORMAL;
	}
	else
	{
		unsigned char* p = (unsigned char *)pBuffer;
		m_Reader->ReadBytes(ArrayReference<unsigned char>(p, lSize));

		eRet = E_RET_NORMAL;
	}


	return (eRet);
}


ResultEnum Serial::Receive(unsigned char* const pBuffer, const long lSize)
{
	ResultEnum	eRet = E_RET_ABNORMAL;

	if (m_InitializeComplete == false)
	{
		return E_RET_NORMAL;
	}

	task<unsigned int> asyncTask = create_task(m_Reader->LoadAsync(lSize));
	asyncTask.wait();
	if (asyncTask.get() < 0)
	{
		eRet = E_RET_ABNORMAL;
	}
	else
	{
		unsigned char* p = pBuffer;
		m_Reader->ReadBytes(ArrayReference<unsigned char>(p, lSize));

		eRet = E_RET_NORMAL;
	}


	return (eRet);
}
