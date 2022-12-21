#include <memory>
#include <thread>
#include <chrono>


#include "SerialConnection.h"


void Wait(unsigned int seconds);


void EnterATCommandMode();
void ShowFirmwareVersion();


auto usb = std::make_shared<SerialConnection>(57600);


int main(int argc, char** argv)
{
	int port_to_check = 0;
	if (!usb->FindNextOpenPort(port_to_check))
	{
		return 1;
	}

	Wait(1);

	EnterATCommandMode();

	Wait(1);

	ShowFirmwareVersion();

	Wait(1);

	if (!usb->Disconnect())
	{
		std::cerr << "Failed to disconnect serial port..." << std::endl;
		return 1;
	}

	return 0;
}


void Wait(unsigned int seconds)
{
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
}


void EnterATCommandMode()
{
	char cmd[3] = { 0x2B, 0x2B, 0x2B };
	if (usb->Write(cmd, 3))
	{
		Wait(1);

		char buff[4] = { 0 };
		int bytes_read = usb->Read(buff, 4);
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


void ShowFirmwareVersion()
{
	char cmd[5] = { 0x41, 0x54, 0x49, 0x0D, 0x0A };
	if (usb->Write(cmd, 5))
	{
		Wait(1);

		char buff[32] = { 0 };
		int bytes_read = usb->Read(buff, 32);
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