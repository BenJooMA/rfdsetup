#include <memory>
#include <thread>
#include <chrono>
#include <cstdint>
#include <fstream>


#include "SerialConnection.h"


#define BAUD_RATE			57600
#define PACKET_SIZE			128
#define CMD_TIMEOUT			250
#define ISSUE_CMD_TIMEOUT	1000
#define MAX_READ_LEN		1024
#define MAX_TRIES			10
#define	AIR_SPEED			125
#define MAX_WINDOW			20


static const uint16_t crc16tab[256] = {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};


static const char hex_table[16] = {	'0', '1', '2', '3', '4', '5', '6', '7',
									'8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };


int SerialConnectAvailable();
bool SerialConnectSpecific(int port_no);
bool SerialDisconnect();


bool GroundSideBasicSetup();
bool AirSideBasicSetup();
bool FirmwareAndBasicParamsSetup();
bool AirSidePinSetup();
bool GroundSidePinSetup();
bool AirSidePinDefaults();


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
void MirrorGPIOPins();
void SetDefaultGPIOPinValues();
void ResetDefaults();
void SaveSettings();
void EnterBootloaderMode();
void ShowChipID();
void InitiateUpload();
void UploadFirmware();
void UploadData(unsigned char* data, int len);
void ExportData(unsigned char* data, int len);
void ExportPacket(std::ofstream& output, unsigned char* data, int id);
void UploadPacket(unsigned char* data, int id);
void Reboot();


uint16_t CalculateCRC(unsigned char* data, int len);
std::string ToHex(uint8_t in, char delimiter = ' ');


static int g_ground_side_port = -1;
static int g_air_side_port = -1;
auto usb = std::make_shared<SerialConnection>(BAUD_RATE);


int main(int argc, char** argv)
{
	if (!AirSideBasicSetup())
	{
		return 1;
	}

	if (!GroundSideBasicSetup())
	{
		return 1;
	}

	if (!AirSidePinSetup())
	{
		return 1;
	}

	if (!GroundSidePinSetup())
	{
		return 1;
	}

	if (!AirSidePinDefaults())
	{
		return 1;
	}

	return 0;
}


bool GroundSideBasicSetup()
{
	std::cout << "Connect ground-side module (killbox) / power-cycle if already connected..." << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	char tmp = 0;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return false;
	}

	Wait(3000);

	if ((g_ground_side_port = SerialConnectAvailable()) < 0)
	{
		return false;
	}

	if (!FirmwareAndBasicParamsSetup())
	{
		return false;
	}

	if (!SerialDisconnect())
	{
		return false;
	}

	std::cout << "Disconnect ground-side module" << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	tmp = 0;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return false;
	}

	return true;
}


bool AirSideBasicSetup()
{
	std::cout << "Connect air-side module / power-cycle if already connected..." << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	char tmp = 0;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return false;
	}

	Wait(3000);

	if ((g_air_side_port = SerialConnectAvailable()) < 0)
	{
		return false;
	}

	if (!FirmwareAndBasicParamsSetup())
	{
		return false;
	}

	if (!SerialDisconnect())
	{
		return false;
	}

	std::cout << "Disconnect air-side module" << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	tmp = 0;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return false;
	}

	return true;
}


bool FirmwareAndBasicParamsSetup()
{
	Wait(CMD_TIMEOUT);

	EnterATCommandMode();

	Wait(CMD_TIMEOUT);

	ShowFirmwareVersion();

	Wait(CMD_TIMEOUT);

	EnterBootloaderMode();

	Wait(CMD_TIMEOUT);

	UploadFirmware();

	Wait(CMD_TIMEOUT);

	Reboot();

	Wait(5000);

	EnterATCommandMode();

	Wait(CMD_TIMEOUT);

	ShowFirmwareVersion();

	Wait(CMD_TIMEOUT);

	ResetDefaults();

	Wait(CMD_TIMEOUT);

	SaveSettings();

	Wait(CMD_TIMEOUT);

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
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
	if ((temp_id >= 0) && (temp_id <= 255))
	{
		net_id = static_cast<uint8_t>(temp_id);
	}
	else
	{
		std::cerr << "Net ID entered is out of range! Valid range is 0-255." << std::endl;
		return false;
	}

	std::cout << "Setting 'Net ID' to " << static_cast<int>(net_id) << std::endl;
	SetNetID(net_id);

	Wait(CMD_TIMEOUT);

	return true;
}


bool AirSidePinSetup()
{
	std::cout << "Connect air-side module / power-cycle if already connected..." << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	char tmp = 0;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return false;
	}

	Wait(3000);

	if (!SerialConnectSpecific(g_air_side_port))
	{
		return false;
	}

	Wait(CMD_TIMEOUT);

	EnterATCommandMode();

	Wait(CMD_TIMEOUT);

	ShowFirmwareVersion();

	Wait(CMD_TIMEOUT);

	ShowCurrentGPIOState();

	Wait(CMD_TIMEOUT);

	std::cout << "Setting state of each GPIO pins to 0..." << std::endl;
	SetAllGPIOPinsToZero();

	Wait(CMD_TIMEOUT);

	SaveSettings();

	Wait(CMD_TIMEOUT);

	ShowCurrentGPIOState();

	Wait(CMD_TIMEOUT);

	if (!SerialDisconnect())
	{
		return false;
	}

	return true;
}


bool GroundSidePinSetup()
{
	std::cout << "Connect ground-side module (killbox), do NOT unplug air-side module" << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	char tmp = 0;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return false;
	}

	Wait(3000);

	if (!SerialConnectSpecific(g_ground_side_port))
	{
		return false;
	}

	Wait(CMD_TIMEOUT);

	EnterATCommandMode();

	Wait(CMD_TIMEOUT);

	std::cout << "Make sure both BOTTOM and TOP switches are set to KILL..." << std::endl;
	std::cout << "Enter 'c' to continue or any other key to quit and press ENTER..." << std::endl;
	tmp = 0;
	std::cin >> tmp;

	if (!((tmp == 'c') || (tmp == 'C')))
	{
		std::cout << "Quitting..." << std::endl;
		return false;
	}

	Wait(CMD_TIMEOUT);

	ShowCurrentGPIOState();

	Wait(CMD_TIMEOUT);

	std::cout << "Mirroring GPIO pins..." << std::endl;
	MirrorGPIOPins();

	Wait(CMD_TIMEOUT);

	SaveSettings();

	Wait(CMD_TIMEOUT);

	ShowCurrentGPIOState();

	Wait(CMD_TIMEOUT);

	if (!SerialDisconnect())
	{
		return false;
	}

	return true;
}


bool AirSidePinDefaults()
{
	Wait(1000);

	if (!SerialConnectSpecific(g_air_side_port))
	{
		return false;
	}

	Wait(CMD_TIMEOUT);

	EnterATCommandMode();

	Wait(CMD_TIMEOUT);

	std::cout << "Setting default air-side GPIO pin state..." << std::endl;
	SetDefaultGPIOPinValues();

	Wait(CMD_TIMEOUT);

	SaveSettings();

	Wait(CMD_TIMEOUT);

	ShowCurrentGPIOState();

	Wait(CMD_TIMEOUT);

	if (!SerialDisconnect())
	{
		return false;
	}

	return true;
}


int SerialConnectAvailable()
{
	int port_to_check = 0;
	if (!usb->FindNextOpenPort(port_to_check))
	{
		return -1;
	}
	return port_to_check;
}


bool SerialConnectSpecific(int port_no)
{
	if (!usb->FindOpenPort(port_no))
	{
		return false;
	}
	return true;
}


bool SerialDisconnect()
{
	if (!usb->Disconnect())
	{
		std::cerr << "Failed to disconnect serial port..." << std::endl;
		return false;
	}
	return true;
}


void Wait(unsigned int milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}


void IssueCommand(char* cmd, int len)
{
	usb->FlushBuffers();

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


void MirrorGPIOPins()
{
	// ATPM=0\r\n
	char cmd_0[8] = { 0x41, 0x54, 0x50, 0x4D, 0x3D, 0x30, 0x0D, 0x0A };
	IssueCommand(cmd_0, 8);

	// ATPM=1\r\n
	char cmd_1[8] = { 0x41, 0x54, 0x50, 0x4D, 0x3D, 0x31, 0x0D, 0x0A };
	IssueCommand(cmd_1, 8);

	// ATPM=2\r\n
	char cmd_2[8] = { 0x41, 0x54, 0x50, 0x4D, 0x3D, 0x32, 0x0D, 0x0A };
	IssueCommand(cmd_2, 8);

	// ATPM=3\r\n
	char cmd_3[8] = { 0x41, 0x54, 0x50, 0x4D, 0x3D, 0x33, 0x0D, 0x0A };
	IssueCommand(cmd_3, 8);

	// ATPM=4\r\n
	char cmd_4[8] = { 0x41, 0x54, 0x50, 0x4D, 0x3D, 0x34, 0x0D, 0x0A };
	IssueCommand(cmd_4, 8);

	// ATPM=5\r\n
	char cmd_5[8] = { 0x41, 0x54, 0x50, 0x4D, 0x3D, 0x35, 0x0D, 0x0A };
	IssueCommand(cmd_5, 8);
}


void SetDefaultGPIOPinValues()
{
	// ATPC=0,0\r\n
	char cmd_0[10] = { 0x41, 0x54, 0x50, 0x43, 0x3D, 0x30, 0x2C, 0x30, 0x0D, 0x0A };
	IssueCommand(cmd_0, 10);

	// ATPC=1,0\r\n
	char cmd_1[10] = { 0x41, 0x54, 0x50, 0x43, 0x3D, 0x31, 0x2C, 0x30, 0x0D, 0x0A };
	IssueCommand(cmd_1, 10);

	// ATPC=2,1\r\n
	char cmd_2[10] = { 0x41, 0x54, 0x50, 0x43, 0x3D, 0x32, 0x2C, 0x31, 0x0D, 0x0A };
	IssueCommand(cmd_2, 10);

	// ATPC=3,0\r\n
	char cmd_3[10] = { 0x41, 0x54, 0x50, 0x43, 0x3D, 0x33, 0x2C, 0x30, 0x0D, 0x0A };
	IssueCommand(cmd_3, 10);

	// ATPC=4,1\r\n
	char cmd_4[10] = { 0x41, 0x54, 0x50, 0x43, 0x3D, 0x34, 0x2C, 0x31, 0x0D, 0x0A };
	IssueCommand(cmd_4, 10);

	// ATPC=5,0\r\n
	char cmd_5[10] = { 0x41, 0x54, 0x50, 0x43, 0x3D, 0x35, 0x2C, 0x30, 0x0D, 0x0A };
	IssueCommand(cmd_5, 10);
}


void ResetDefaults()
{
	// AT&F\r\n
	char cmd[6] = { 0x41, 0x54, 0x26, 0x46, 0x0D, 0x0A };
	IssueCommand(cmd, 6);
	std::cout << "Setting all parameters to default..." << std::endl;
	std::cout << std::string(30, '.');
	for (int i = 0; i < 30; i++)
	{
		std::cout << "\r" << std::string(i + 1, '#');

		Wait(1000);
	}
	std::cout << std::endl;
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
	char cmd[8] = { 0x43, 0x48, 0x49, 0x50, 0x49, 0x44, 0x0D, 0x0A };
	IssueCommand(cmd, 8);
}


void InitiateUpload()
{
	// UPLOAD\r
	char cmd[8] = { 0x55, 0x50, 0x4C, 0x4F, 0x41, 0x44, 0x0D, 0x0A };
	IssueCommand(cmd, 8);
}


void UploadFirmware()
{
	std::cout << "Enter firmware path or 'd' for using default path: " << std::endl;
	std::string path_str = "";
	std::cin >> path_str;
	if ((path_str == "d") || (path_str == "D"))
	{
		std::ifstream fw("C:\\tmp\\rfd\\fw_3_54.gbl", std::ios::binary | std::ios::ate);
		//std::ifstream fw("C:\\tmp\\rfd\\fw_4_03.gbl", std::ios::binary | std::ios::ate);
		if (!fw.is_open())
		{
			std::cerr << "Could not open file..." << std::endl;
			return;
		}

		InitiateUpload();

		Wait(CMD_TIMEOUT);

		std::cout << usb->ReadByte() << std::endl;

		Wait(CMD_TIMEOUT);
	
		std::streamsize len = fw.tellg();
		fw.seekg(0, std::ios::beg);
		unsigned char* data = new unsigned char[len];
		if (fw.read((char*)data, len))
		{
			std::cout << "Successfully read " << len << " bytes into 'data'..." << std::endl;

			//usb->SetBaudRate(115200);
			//Wait(100);
			usb->FlushBuffers();
			Wait(100);
			usb->SetReadTimeout(2000);
			Wait(100);
			UploadData(data, static_cast<int>(len));
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


void UploadData(unsigned char* data, int len)
{
	int num_packets = (len % PACKET_SIZE == 0) ? (len / PACKET_SIZE) : (len / PACKET_SIZE + 1);
	std::cout << "Number of packets required: " << num_packets << std::endl;
	std::cout << std::endl;

	int read_index = 0;
	for (int i = 0; i < num_packets; i++)
	{
		unsigned char chunk[PACKET_SIZE] = { 0 };
		memset(chunk, 0x26, PACKET_SIZE);
		memcpy(chunk, &data[read_index], PACKET_SIZE);
		read_index += PACKET_SIZE;

		int percentage = static_cast<int>(static_cast<float>(i + 1) / static_cast<float>(num_packets) * 100.0f);
		std::cout << "\rUploading firmware - " << percentage << "%";

		UploadPacket(chunk, i + 1);
	}
	std::cout << std::endl;
}


void ExportData(unsigned char* data, int len)
{
	int num_packets = (len % PACKET_SIZE == 0) ? (len / PACKET_SIZE) : (len / PACKET_SIZE + 1);
	std::cout << "Number of packets required: " << num_packets << std::endl;
	std::cout << std::endl;

	std::ofstream hex_output("fw_hex_outbut.txt");
	if (!hex_output.is_open())
	{
		return;
	}

	int read_index = 0;
	for (int i = 0; i < num_packets; i++)
	{
		unsigned char chunk[PACKET_SIZE] = { 0 };
		memset(chunk, 0x26, PACKET_SIZE);
		memcpy(chunk, &data[read_index], PACKET_SIZE);
		read_index += PACKET_SIZE;

		int percentage = static_cast<int>(static_cast<float>(i + 1) / static_cast<float>(num_packets) * 100.0f);
		std::cout << "\rUploading firmware - " << percentage << "%";

		ExportPacket(hex_output, chunk, i + 1);
	}
	std::cout << std::endl;

	hex_output.close();
}


void ExportPacket(std::ofstream& output, unsigned char* data, int id)
{
	char packet[PACKET_SIZE + 5] = { 0 };

	packet[0] = 0x01;
	packet[1] = static_cast<unsigned char>(id % 256);
	packet[2] = static_cast<unsigned char>(255 - (id % 256));
	memcpy(&packet[3], data, PACKET_SIZE);
	uint16_t crc = CalculateCRC(data, PACKET_SIZE);
	packet[PACKET_SIZE + 3] = static_cast<unsigned char>(crc >> 8) & 0xFF;
	packet[PACKET_SIZE + 4] = static_cast<unsigned char>(crc) & 0xFF;

	for (int i = 0; i < (PACKET_SIZE + 5); i++)
	{
		output << ToHex(packet[i]);
	}
	output << std::endl;
}


void UploadPacket(unsigned char* data, int id)
{
	char packet[PACKET_SIZE + 5] = { 0 };

	packet[0] = 0x01;
	packet[1] = static_cast<unsigned char>(id % 256);
	packet[2] = static_cast<unsigned char>(255 - (id % 256));
	memcpy(&packet[3], data, PACKET_SIZE);
	uint16_t crc = CalculateCRC(data, PACKET_SIZE);
	packet[PACKET_SIZE + 3] = static_cast<unsigned char>(crc >> 8) & 0xFF;
	packet[PACKET_SIZE + 4] = static_cast<unsigned char>(crc) & 0xFF;

	bool success = false;
	while (!success)
	{
		usb->FlushBuffers();
		if (usb->Write(packet, (PACKET_SIZE + 5)))
		{
			Wait(100);
			success = (usb->ReadByte() == 0x06);
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


uint16_t CalculateCRC(unsigned char* data, int len)
{
	uint16_t crc = 0;
	for (int i = 0; i < len; i++)
	{
		crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *data++) & 0x00FF];
	}
	return crc;
}


std::string ToHex(uint8_t in, char delimiter)
{
	int num = static_cast<int>(in);

	std::string out = "";

	out += hex_table[num / 16];
	out += hex_table[num % 16];
	out += delimiter;

	return out;
}