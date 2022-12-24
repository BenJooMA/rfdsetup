#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>
#include <fstream>


#include "SerialConnection.h"


#define BAUD_RATE			57600
#define CMD_TIMEOUT			250
#define ISSUE_CMD_TIMEOUT	250
#define MAX_READ_LEN		1024
#define	AIR_SPEED			125
#define MAX_WINDOW			20


void Wait(unsigned int milliseconds);


void IssueCommand(char* cmd, int len);


void EnterATCommandMode();
void ShowFirmwareVersion();
void ShowCurrentParams();
void ShowCurrentGPIOState();
void SetAirSpeed();
void SetMaxWindow();
void SetNetID(uint8_t id);
void SetAllGPIOPinsToZero();


void SaveSettings();


void EnterBootloaderMode();
void ShowChipID();
void InitiateUpload();
void UploadFirmware();


auto usb = std::make_shared<SerialConnection>(BAUD_RATE);


int main(int argc, char** argv)
{
	std::cout << "Connect air-side module / power-cycle if already connected..." << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	char tmp = 0;
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

	Wait(CMD_TIMEOUT);

	EnterATCommandMode();

	Wait(CMD_TIMEOUT);

	ShowFirmwareVersion();

	Wait(CMD_TIMEOUT);

	EnterBootloaderMode();

	Wait(CMD_TIMEOUT);

	ShowChipID();

	Wait(CMD_TIMEOUT);

	InitiateUpload();

	Wait(CMD_TIMEOUT);

	UploadFirmware();

	return 1;

	ShowCurrentParams();

	Wait(CMD_TIMEOUT);

	std::cout << "Setting 'Air Speed' to " << AIR_SPEED << std::endl;
	SetAirSpeed();

	Wait(CMD_TIMEOUT);

	std::cout << "Setting 'Max Window' to " << MAX_WINDOW << std::endl;
	SetMaxWindow();

	Wait(CMD_TIMEOUT);

	std::cout << "Enter Net ID (0-255): " << std::endl;
	std::string net_id_str = "";
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

	std::cout << "Setting 'Net ID' to " << static_cast<int>(net_id) << std::endl;
	SetNetID(net_id);

	Wait(CMD_TIMEOUT);

	ShowCurrentGPIOState();

	Wait(CMD_TIMEOUT);

	std::cout << "Setting state of each GPIO pins to 0" << std::endl;
	SetAllGPIOPinsToZero();

	Wait(CMD_TIMEOUT);

	SaveSettings();

	Wait(CMD_TIMEOUT);

	ShowCurrentGPIOState();

	Wait(CMD_TIMEOUT);

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
		Wait(ISSUE_CMD_TIMEOUT);

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


void SetAllGPIOPinsToZero()
{
	// ATPO=0\r\n
	char cmd_0[8] = { 0x41, 0x54, 0x50, 0x4F, 0x3D, 0x30, 0x0D, 0x0A };
	IssueCommand(cmd_0, 8);

	// ATPO=1\r\n
	char cmd_1[8] = { 0x41, 0x54, 0x50, 0x4F, 0x3D, 0x31, 0x0D, 0x0A };
	IssueCommand(cmd_1, 8);

	// ATPO=2\r\n
	char cmd_2[8] = { 0x41, 0x54, 0x50, 0x4F, 0x3D, 0x32, 0x0D, 0x0A };
	IssueCommand(cmd_2, 8);

	// ATPO=3\r\n
	char cmd_3[8] = { 0x41, 0x54, 0x50, 0x4F, 0x3D, 0x33, 0x0D, 0x0A };
	IssueCommand(cmd_3, 8);

	// ATPO=4\r\n
	char cmd_4[8] = { 0x41, 0x54, 0x50, 0x4F, 0x3D, 0x34, 0x0D, 0x0A };
	IssueCommand(cmd_4, 8);

	// ATPO=5\r\n
	char cmd_5[8] = { 0x41, 0x54, 0x50, 0x4F, 0x3D, 0x35, 0x0D, 0x0A };
	IssueCommand(cmd_5, 8);
}


void SaveSettings()
{
	// AT&W\r\n
	char cmd[6] = { 0x41, 0x54, 0x26, 0x57, 0x0D, 0x0A };
	IssueCommand(cmd, 6);
	std::cout << "Settings saved..." << std::endl;
}


void EnterBootloaderMode()
{
	// AT&UPDATE\r\n
	char cmd[11] = { 0x41, 0x54, 0x26, 0x55, 0x50, 0x44, 0x41, 0x54, 0x45, 0x0D, 0x0A };
	IssueCommand(cmd, 11);
}


void ShowChipID()
{
	// CHIPID\r
	char cmd[7] = { 0x43, 0x48, 0x49, 0x50, 0x49, 0x44, 0x0D };
	IssueCommand(cmd, 7);
}


void InitiateUpload()
{
	// UPLOAD\r
	char cmd[7] = { 0x55, 0x50, 0x4C, 0x4F, 0x41, 0x44, 0x0D };
	IssueCommand(cmd, 7);
}


void UploadFirmware()
{
	std::cout << "Enter firmware path or 'd' for using default path: " << std::endl;
	std::string path_str = "";
	std::cin >> path_str;
	if ((path_str == "d") || (path_str == "D"))
	{
		std::ifstream fw("C:\\tmp\\rfd\\fw.gbl", std::ios::binary | std::ios::ate);
		if (!fw.is_open())
		{
			std::cerr << "Could not open file..." << std::endl;
			return;
		}

		std::streamsize sz = fw.tellg();
		fw.seekg(0, std::ios::beg);
		char* data = new char[sz];
		if (fw.read(data, sz))
		{
			std::cout << "Successfully read " << sz << " bytes into 'data'..." << std::endl;
		}
		delete[] data;
	}
	else
	{
		std::cout << "Path not recognized..." << std::endl;
	}
}