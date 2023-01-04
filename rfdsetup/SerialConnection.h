#pragma once


#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>


#define MAX_PORT_NUM	40 // 25


using byte = unsigned char;


class SerialConnection
{
public:

	SerialConnection(int baudrate);
	SerialConnection(const std::string& path, int baudrate);
	~SerialConnection();

	bool FindNextOpenPort(int& first_port_to_check);
	bool FindOpenPort(int port_no);

	bool Initialise();

	bool SetReadTimeout(int timeout);
	int GetReadTimeout();

	bool SetBaudRate(int baudrate);

	bool FlushBuffers();
	bool CancelSyncIo();

	byte ReadByte();
	int Read(char* buffer, int count);

	bool WriteByte(char byte);
	bool Write(const std::string& msg);
	bool Write(const char* buffer, int count);

	bool Disconnect();

private:

	std::string		m_Path;
	int				m_Baudrate;
	HANDLE			m_Port;

};