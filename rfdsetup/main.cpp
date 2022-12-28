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
#define MAX_TRIES			10
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
void UploadData(char* data, std::streamsize len);
void UploadPacket(char* data, int id);
void Reboot();
uint16_t CalculateCRC(char* data, int len);


void Sync();


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

	/**/

	EnterBootloaderMode();

	Wait(CMD_TIMEOUT);

	ShowChipID();

	Wait(CMD_TIMEOUT);

	InitiateUpload();

	Wait(CMD_TIMEOUT);

	for (int i = 0; i < 10; i++)
	{
		Sync();
	}

	return 1;

	for (int i = 0; i < 10; i++)
	{
		std::cout << usb->ReadByte();
	}

	return 1;

	UploadFirmware();

	Wait(CMD_TIMEOUT);

	Reboot();

	Wait(10000);

	EnterATCommandMode();

	Wait(CMD_TIMEOUT);

	ShowFirmwareVersion();

	Wait(CMD_TIMEOUT);

	//return 1;

	/**/

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
		//std::ifstream fw("C:\\tmp\\rfd\\fw.gbl", std::ios::binary | std::ios::ate);
		std::ifstream fw("C:\\tmp\\rfd\\fw_3.38.bin", std::ios::binary | std::ios::ate);
		if (!fw.is_open())
		{
			std::cerr << "Could not open file..." << std::endl;
			return;
		}
	
		std::streamsize len = fw.tellg();
		fw.seekg(0, std::ios::beg);
		char* data = new char[len];
		if (fw.read(data, len))
		{
			std::cout << "Successfully read " << len << " bytes into 'data'..." << std::endl;

			usb->SetBaudRate(115200);
			Wait(100);
			usb->FlushBuffers();
			Wait(100);
			usb->SetReadTimeout(2000);
			Wait(100);
			UploadData(data, len);
			Wait(100);
			usb->SetBaudRate(BAUD_RATE);
			Wait(100);
			usb->WriteByte(0x04);
			Wait(100);
		}
		delete[] data;
	}
	else
	{
		std::cout << "Path not recognized..." << std::endl;
	}
}


void UploadData(char* data, std::streamsize len)
{
	int num_packets = (len % 128 == 0) ? (len / 128) : (len / 128 + 1);
	std::cout << "Number of packets required: " << num_packets << std::endl;
	std::cout << std::endl;

	int read_index = 0;
	for (int i = 0; i < num_packets; i++)
	{
		char chunk[128] = { 0 };
		memset(chunk, 0x26, 128);
		memcpy(chunk, &data[read_index], 128);
		read_index += 128;
		std::cout << "Uploading packet " << (i + 1) << "/" << num_packets << " ";
		UploadPacket(chunk, i + 1);
	}
	std::cout << std::endl;
}


void UploadPacket(char* data, int id)
{
	char packet[133] = { 0 };

	packet[0] = 0x01;
	packet[1] = static_cast<char>(id % 256);
	packet[2] = static_cast<char>(255 - (id % 256));
	memcpy(&packet[3], data, 128);
	uint16_t crc = CalculateCRC(data, 128);
	packet[131] = static_cast<char>(crc >> 8);
	packet[132] = static_cast<char>(crc);

	//for (int i = 0; i < MAX_TRIES; i++)
	bool success = false;
	while (!success)
	{
		usb->FlushBuffers();
		if (usb->Write(packet, 133))
		{
			unsigned char c = 0;
			do
			{
				c = usb->ReadByte();
			} while (c == 67);

			switch (c)
			{
				case 0x06:
				{
					std::cout << "ACK" << std::endl;
					success = true;
				}
				break;
				case 0x15:
				{
					std::cout << "-";
				}
				break;
				case 0x18:
				{
					std::cout << "o";
				}
				break;
				default:
				{
					std::cout << ".";
				}
				break;
			}
		}
	}
}


void Reboot()
{
	// \r\n
	char newline[2] = { 0x0D, 0x0A };
	IssueCommand(newline, 2);
	// BOOTNEW\r\n
	char cmd[9] = { 0x42, 0x4F, 0x4F, 0x54, 0x4E, 0x45, 0x57, 0x0D, 0x0A };
	IssueCommand(cmd, 9);
}


uint16_t CalculateCRC(char* data, int len)
{
	uint16_t result = 0;
	for (int i = 0; i < len; i++)
	{
		result = static_cast<uint16_t>(result >> 8 | static_cast<int>(result) << 8);
		result ^= static_cast<uint16_t>(data[i]);
		result ^= static_cast<uint16_t>((result & 0xFF) >> 4);
		result ^= static_cast<uint16_t>(result << 12);
		result ^= static_cast<uint16_t>((result & 0xFF) << 5);
	}
	return result;
}


void Sync()
{
	usb->FlushBuffers();

	char cmd[2] = { 0x21, 0x20 };
	IssueCommand(cmd, 2);

	if (usb->Write(cmd, 2))
	{
		Wait(ISSUE_CMD_TIMEOUT);

		byte result = usb->ReadByte();

		if (result == 0x12)
		{
			std::cout << "In sync!" << std::endl;
		}
		else
		{
			std::cout << "Not in sync..." << std::endl;
		}
	}
	else
	{
		std::cout << "Could not write to serial port..." << std::endl;
	}
}