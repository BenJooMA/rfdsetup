#include "SerialConnection.h"


SerialConnection::SerialConnection(int baudrate)
	:
	m_Path(""),
	m_Baudrate(baudrate),
	m_Port(nullptr)
{

}


SerialConnection::SerialConnection(const std::string& path, int baudrate)
	:
	m_Path(path),
	m_Baudrate(baudrate),
	m_Port(nullptr)
{

}


SerialConnection::~SerialConnection()
{
	
}


bool SerialConnection::FindNextOpenPort(int& first_port_to_check)
{
	bool success = false;

	std::string port_str = "\\\\.\\COM";
	for (int i = first_port_to_check; i <= MAX_PORT_NUM; i++)
	{
		std::string port_num_str = port_str + std::to_string(i);

		//std::cout << "Checking port " << port_num_str << "..." << std::endl;

		HANDLE port = CreateFileA(port_num_str.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 0);

		if (port == INVALID_HANDLE_VALUE)
		{
			//std::cout << "Could not open port " << port_num_str << std::endl;
			continue;
		}
		else
		{
			std::cout << port_num_str << " successfully opened" << std::endl;
			success = true;
			m_Port = port;
			m_Path = port_num_str;
			first_port_to_check = i;


			DCB serialParams = { 0 };
			serialParams.DCBlength = sizeof(serialParams);

			GetCommState(m_Port, &serialParams);
			serialParams.BaudRate = m_Baudrate;
			serialParams.ByteSize = 8;
			serialParams.StopBits = ONESTOPBIT;
			serialParams.Parity = NOPARITY;
			if (SetCommState(m_Port, &serialParams) == 0)
			{
				std::cerr << "Failed to configure port: " << GetLastError() << std::endl;
				return false;
			}

			COMMTIMEOUTS timeout = { 0 };
			timeout.ReadTotalTimeoutConstant = 500;
			timeout.WriteTotalTimeoutConstant = 500;
			if (SetCommTimeouts(m_Port, &timeout) == 0)
			{
				std::cerr << "Failed to set time-out parameters: " << GetLastError() << std::endl;
				return false;
			}

			break;
		}
	}

	return success;
}


bool SerialConnection::FindOpenPort(int port_no)
{
	std::string port_str = "\\\\.\\COM";
	std::string port_num_str = port_str + std::to_string(port_no);

	HANDLE port = CreateFileA(port_num_str.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);

	if (port == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	std::cout << "Port " << port_num_str << " successfully opened" << std::endl;
	m_Port = port;
	m_Path = port_num_str;


	DCB serialParams = { 0 };
	serialParams.DCBlength = sizeof(serialParams);

	GetCommState(m_Port, &serialParams);
	serialParams.BaudRate = m_Baudrate;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
	serialParams.Parity = NOPARITY;
	if (SetCommState(m_Port, &serialParams) == 0)
	{
		std::cerr << "Failed to configure port: " << GetLastError() << std::endl;
		return false;
	}

	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadTotalTimeoutConstant = 500;
	timeout.WriteTotalTimeoutConstant = 500;
	if (SetCommTimeouts(m_Port, &timeout) == 0)
	{
		std::cerr << "Failed to set time-out parameters: " << GetLastError() << std::endl;
		return false;
	}

	return true;
}


bool SerialConnection::Initialise()
{
	try
	{
		std::cout << "Opening port: " << m_Path << std::endl;

		m_Port = CreateFileA(m_Path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 0);

		if (m_Port == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Could not open port: " << m_Path << " - Error code: " << GetLastError() << std::endl;
			return false;
		}
		else
		{
			std::cout << "Port " << m_Path << " successfully opened" << std::endl;
		}

		DCB serialParams = { 0 };
		serialParams.DCBlength = sizeof(serialParams);

		GetCommState(m_Port, &serialParams);
		serialParams.BaudRate = m_Baudrate;
		serialParams.ByteSize = 8;
		serialParams.StopBits = ONESTOPBIT;
		serialParams.Parity = NOPARITY;
		if (SetCommState(m_Port, &serialParams) == 0)
		{
			std::cerr << "Failed to configure port: " << GetLastError() << std::endl;
			return false;
		}

		COMMTIMEOUTS timeout = { 0 };
		timeout.ReadIntervalTimeout = 50;
		timeout.ReadTotalTimeoutConstant = 50;
		timeout.ReadTotalTimeoutMultiplier = 50;
		timeout.WriteTotalTimeoutConstant = 50;
		timeout.WriteTotalTimeoutMultiplier = 10;
		if (SetCommTimeouts(m_Port, &timeout) == 0)
		{
			std::cerr << "Failed to set time-out parameters: " << GetLastError() << std::endl;
			return false;
		}

	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}

	return true;
}


bool SerialConnection::SetReadTimeout(int value)
{
	COMMTIMEOUTS timeout = { 0 };

	if (GetCommTimeouts(m_Port, &timeout) == 0)
	{
		std::cerr << "Failed to get time-out parameters: " << GetLastError() << std::endl;
		return false;
	}

	timeout.ReadIntervalTimeout = value;

	if (SetCommTimeouts(m_Port, &timeout) == 0)
	{
		std::cerr << "Failed to set time-out parameters: " << GetLastError() << std::endl;
		return false;
	}

	return true;
}


int SerialConnection::GetReadTimeout()
{
	COMMTIMEOUTS timeout = { 0 };

	if (GetCommTimeouts(m_Port, &timeout) == 0)
	{
		std::cerr << "Failed to get time-out parameters: " << GetLastError() << std::endl;
		return 0;
	}

	return (int)timeout.ReadIntervalTimeout;
}


bool SerialConnection::SetBaudRate(int baudrate)
{
	DCB serialParams = { 0 };
	serialParams.DCBlength = sizeof(serialParams);

	GetCommState(m_Port, &serialParams);
	serialParams.BaudRate = baudrate;
	if (SetCommState(m_Port, &serialParams) == 0)
	{
		std::cerr << "Failed to configure port: " << GetLastError() << std::endl;
		return false;
	}

	return true;
}


bool SerialConnection::FlushBuffers()
{
	return FlushFileBuffers(m_Port);
}


bool SerialConnection::CancelSyncIo()
{
	return CancelSynchronousIo(m_Port);
}


byte SerialConnection::ReadByte()
{
	byte c;
	DWORD bytes_received = 0;
	bool result = false;
	result = ReadFile(m_Port, &c, 1, &bytes_received, NULL);
	if (bytes_received != 1 || !result)
	{
		return 0;
	}
	return c;
}


int SerialConnection::Read(char* buffer, int count)
{
	DWORD bytes_received = 0;
	bool success = ReadFile(m_Port, buffer, count, &bytes_received, NULL);
	if (success)
	{
		return (int)bytes_received;
	}

	return 0;
}


bool SerialConnection::WriteByte(char byte)
{
	DWORD bytes_written = 0;

	return WriteFile(m_Port, &byte, 1, &bytes_written, NULL);
}


bool SerialConnection::Write(const std::string& msg)
{
	const char* buffer = msg.c_str();

	DWORD bytes_written = 0;

	bool success = WriteFile(m_Port, buffer, msg.size(), &bytes_written, NULL);

	if (success)
	{
		std::cout << "Bytes written: " << bytes_written << std::endl;
	}

	return success;
}


bool SerialConnection::Write(const char* buffer, int count)
{
	bool success = false;

	DWORD bytes_written = 0;
	success = WriteFile(m_Port, buffer, count, &bytes_written, NULL);

	return success;
}


bool SerialConnection::Disconnect()
{
	std::cout << "Disconnecting " << m_Path << "...";
	bool close_success = CloseHandle(m_Port);
	std::cout << "..." << m_Path << " disconnected." << std::endl;
	return close_success;
}