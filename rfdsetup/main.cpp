#include <memory>
#include <thread>
#include <chrono>


#include "SerialConnection.h"


void Wait(unsigned int milliseconds);


void EnterATCommandMode();
void ShowFirmwareVersion();
void ShowCurrentParams();
void ShowCurrentGPIOState();


auto usb = std::make_shared<SerialConnection>(57600);


int main(int argc, char** argv)
{
	std::cout << "Connect air-side module - or power-cycle if already connected - and press ENTER when done..." << std::endl;
	char tmp;
	std::cin >> tmp;

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


void EnterATCommandMode()
{
	char cmd[3] = { 0x2B, 0x2B, 0x2B };
	if (usb->Write(cmd, 3))
	{
		Wait(1000);

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
		Wait(1000);

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


void ShowCurrentParams()
{
	char cmd[6] = { 0x41, 0x54, 0x49, 0x35, 0x0D, 0x0A };
	if (usb->Write(cmd, 6))
	{
		Wait(1000);

		char buff[1024] = { 0 };
		int bytes_read = usb->Read(buff, 1024);
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


void ShowCurrentGPIOState()
{
	char cmd[6] = { 0x41, 0x54, 0x50, 0x50, 0x0D, 0x0A };
	if (usb->Write(cmd, 6))
	{
		Wait(1000);

		char buff[256] = { 0 };
		int bytes_read = usb->Read(buff, 256);
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