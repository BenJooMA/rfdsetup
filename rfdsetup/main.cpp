#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>


#include "SerialConnection.h"


#define MAX_READ_LEN	1024
#define	AIR_SPEED		125
#define MAX_WINDOW		20


void Wait(unsigned int milliseconds);


void IssueCommand(char* cmd, int len);


void EnterATCommandMode();
void ShowFirmwareVersion();
void ShowCurrentParams();
void ShowCurrentGPIOState();
void SetAirSpeed();
void SetMaxWindow();
void SetNetID(uint8_t id);


void SaveSettings();


auto usb = std::make_shared<SerialConnection>(57600);


int main(int argc, char** argv)
{
	std::cout << "Connect air-side module / power-cycle if already connected..." << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	char tmp;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return 1;
	}

	Wait(3000);

	int port_to_check = 0;
	if (!usb->FindNextOpenPort(port_to_check))
	{
		return 1;
	}

	Wait(1000);

	EnterATCommandMode();

	Wait(1000);

	ShowFirmwareVersion();

	Wait(1000);

	ShowCurrentParams();

	Wait(1000);

	std::cout << "Setting 'Air Speed' to " << AIR_SPEED << std::endl;
	SetAirSpeed();

	Wait(1000);

	std::cout << "Setting 'Max Window' to " << MAX_WINDOW << std::endl;
	SetMaxWindow();

	Wait(1000);

	std::cout << "Enter Net ID (0-255):" << std::endl;
	std::string net_id_str;
	std::cin >> net_id_str;
	uint8_t net_id = 255;
	int temp_id = -1;
	try
	{
		temp_id = std::stoi(net_id_str);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	if ((temp_id >= 0) && (temp_id <= 255))
	{
		net_id = static_cast<uint8_t>(temp_id);
	}
	else
	{
		std::cerr << "Net ID entered is out of range! Valid range is 0-255." << std::endl;
		return 1;
	}

	std::cout << "Setting 'Net ID' to " << std::to_string(static_cast<int>(net_id)) << std::endl;
	SetNetID(net_id);

	Wait(1000);

	SaveSettings();

	Wait(1000);

	ShowCurrentGPIOState();

	Wait(1000);

	if (!usb->Disconnect())
	{
		std::cerr << "Failed to disconnect serial port..." << std::endl;
		return 1;
	}

	return 0;
}


void Wait(unsigned int seconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(seconds));
}


void IssueCommand(char* cmd, int len)
{
	if (usb->Write(cmd, len))
	{
		Wait(1000);

		char buff[MAX_READ_LEN] = { 0 };
		int bytes_read = usb->Read(buff, MAX_READ_LEN);
		if (bytes_read > 0)
		{
			std::cout << std::string(buff) << std::endl;
		}
		else
		{
			std::cout << "Bytes read: " << bytes_read << std::endl;
		}
	}
	else
	{
		std::cout << "Could not write to serial port..." << std::endl;
	}
}


void EnterATCommandMode()
{
	// +++
	char cmd[3] = { 0x2B, 0x2B, 0x2B };
	IssueCommand(cmd, 3);
}


void ShowFirmwareVersion()
{
	// ATI\r\n
	char cmd[5] = { 0x41, 0x54, 0x49, 0x0D, 0x0A };
	IssueCommand(cmd, 5);
}


void ShowCurrentParams()
{
	// ATI5\r\n
	char cmd[6] = { 0x41, 0x54, 0x49, 0x35, 0x0D, 0x0A };
	IssueCommand(cmd, 6);
}


void ShowCurrentGPIOState()
{
	// ATPP\r\n
	char cmd[6] = { 0x41, 0x54, 0x50, 0x50, 0x0D, 0x0A };
	IssueCommand(cmd, 6);
}


void SetAirSpeed()
{
	// ATS2=125\r\n
	char cmd[10] = { 0x41, 0x54, 0x53, 0x32, 0x3D, 0x31, 0x32, 0x35, 0x0D, 0x0A };
	IssueCommand(cmd, 10);
}


void SetMaxWindow()
{
	// ATS14=20\r\n
	char cmd[10] = { 0x41, 0x54, 0x53, 0x31, 0x34, 0x3D, 0x32, 0x30, 0x0D, 0x0A };
	IssueCommand(cmd, 10);
}


void SetNetID(uint8_t id)
{
	// ATS3=XXX\r\n
	std::string cmd_str = "ATS3=" + std::to_string(id);
	std::size_t sz = cmd_str.size() + 1;
	if (sz >= 7)
	{
		char* cmd = new char[sz];
		memset(cmd, 0, sz);
		int index = 0;
		for (const auto& ch : cmd_str)
		{
			if (ch != '\0')
			{
				cmd[index++] = ch;
			}
		}
		cmd[index++] = '\r';
		cmd[index++] = '\n';
		IssueCommand(cmd, static_cast<int>(sz));
		delete[] cmd;
	}
}


void SaveSettings()
{
	// AT&W\r\n
	char cmd[6] = { 0x41, 0x54, 0x26, 0x57, 0x0D, 0x0A };
	IssueCommand(cmd, 6);
	std::cout << "Settings saved..." << std::endl;
}