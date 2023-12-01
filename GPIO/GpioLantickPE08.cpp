//---------------------------------------------------------------------------


#pragma hdrstop

#include "GpioLantickPE08.h"
#include "PhoneLocal.h"
#include "Device.h"
#include "Utils.h"
#include <algorithm>
#include <stdio.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#include "Log.h"


static const unsigned char request[] = {
	15,	// SOF
	100,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	115	// CRC
	};
std::string GpioLantick::keepAliveBuf(reinterpret_cast<const char*>(request), sizeof(request));
//std::string GpioLantick::initBuf = "";
std::string GpioLantick::initBuf(reinterpret_cast<const char*>(request), sizeof(request));

std::string varStateName;
std::string varConnName;

GpioLantick::GpioLantick(Device &device):
	device(device),
	connected(false),
	connectedInitialized(false),
	receivedCount(0)
{
	std::string path = Utils::GetDllPath();
	std::string dllName = Utils::ExtractFileNameWithoutExtension(path);
	varStateName = dllName;
	#pragma warn -8091	// incorrectly issued by BDS2006
	std::transform(varStateName.begin(), varStateName.end(), varStateName.begin(), tolower);
	varConnName = varStateName;
	varStateName += "State";
	varConnName += "Conn";
}

int GpioLantick::OnDeviceBufRx(unsigned char byte)
{
#if 0
	LOG("RX %s", cmd.c_str());
	const char *ptr = cmd.c_str();
	if (StartsWith(ptr, "SETVARIABLE ")) {
		SetVariable("TEST", "TESTVAL");
	} else if (StartsWith(ptr, "CLEARVARIABLE ")) {
		ClearVariable("TEST");
	}
#endif

	rxBuf[receivedCount] = byte;
	receivedCount++;
	if (receivedCount >= 16)
	{
		device.ResetWaitForReply();
		unsigned int diValue = 0;
		LOG("RX buf: %02X %02X %02X %02X  %02X %02X %02X %02X\n",
			(unsigned char)rxBuf[1],
			(unsigned char)rxBuf[3],
			(unsigned char)rxBuf[5],
			(unsigned char)rxBuf[7],
			(unsigned char)rxBuf[9],
			(unsigned char)rxBuf[11],
			(unsigned char)rxBuf[13],
			(unsigned char)rxBuf[15]
			);
		for (int i=0; i<8; i++)
		{
			if (rxBuf[i*2 + 1])
			{
				diValue |= (1<<i);
			}
		}
		receivedCount = 0;
		char str[16];
		sprintf(str, "%d", diValue);
		SetVariable(varStateName.c_str(), str);
	}
	return 0;
}

void GpioLantick::OnPoll(void)
{
	if (connected != device.isConnected() || !connectedInitialized)
	{
		connected = device.isConnected();
		connectedInitialized = true;
		if (connected)
		{
			SetVariable(varConnName.c_str(), "1");
		}
		else
		{
			SetVariable(varConnName.c_str(), "0");
			ClearVariable(varStateName.c_str());
			receivedCount = 0;			
		}
	}
}
