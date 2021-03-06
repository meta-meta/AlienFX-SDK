#include "stdafx.h"
#include <Setupapi.h>
#include <codecvt>
#include <locale>
#include "AlienFX_SDK.h"  
#include <iostream>
extern "C" {
#include <hidclass.h>
#include <hidsdi.h>
}

#define ALIENFX_DEVICE_RESET 0x06
#define ALIENFX_READY 0x10
#define ALIENFX_BUSY 0x11
#define ALIENFX_UNKOWN_COMMAND 0x12

namespace AlienFX_SDK
{
	bool isInitialized = false;
	HANDLE devHandle;
	int length = 9;

	//Use this method to scan for devices that uses same vid
	int Functions::AlienFXInitialize(int vid)
	{
		int pid = -1;
		GUID guid;
		bool flag = false;

		HidD_GetHidGuid(&guid);
		HDEVINFO hDevInfo = SetupDiGetClassDevsA(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (hDevInfo == INVALID_HANDLE_VALUE)
		{
			//std::cout << "Couldn't get guid";
			return false;
		}
		unsigned int dw = 0;
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

		unsigned int lastError = 0;
		while (!flag)
		{
			deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, dw, &deviceInterfaceData))
			{
				lastError = GetLastError();
				return pid;
			}
			dw++;
			DWORD dwRequiredSize = 0;
			if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, NULL, 0, &dwRequiredSize, NULL))
			{
				//std::cout << "Getting the needed buffer size failed";
				return pid;
			}
			//std::cout << "Required size is " << dwRequiredSize << std::endl;
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				//std::cout << "Last error is not ERROR_INSUFFICIENT_BUFFER";
				return pid;
			}
			std::unique_ptr<SP_DEVICE_INTERFACE_DETAIL_DATA> deviceInterfaceDetailData((SP_DEVICE_INTERFACE_DETAIL_DATA*)new char[dwRequiredSize]);
			deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, deviceInterfaceDetailData.get(), dwRequiredSize, NULL, NULL))
			{
				std::wstring devicePath = deviceInterfaceDetailData->DevicePath;
				//OutputDebugString(devicePath.c_str());
				devHandle = CreateFile(devicePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

				if (devHandle != INVALID_HANDLE_VALUE)
				{
					std::unique_ptr<HIDD_ATTRIBUTES> attributes(new HIDD_ATTRIBUTES);
					attributes->Size = sizeof(HIDD_ATTRIBUTES);
					if (HidD_GetAttributes(devHandle, attributes.get()))
					{

						if (attributes->VendorID == vid)
						{
							length = attributes->Size;
							pid = attributes->ProductID;
							flag = true;
						}
					}


				}
			}
		}
		//OutputDebugString(flag);
		return pid;
	}

	//Use this method for general devices
	bool Functions::AlienFXInitialize(int vid, int pid)
	{
		GUID guid;
		bool flag = false;

		HidD_GetHidGuid(&guid);
		HDEVINFO hDevInfo = SetupDiGetClassDevsA(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (hDevInfo == INVALID_HANDLE_VALUE)
		{
			//std::cout << "Couldn't get guid";
			return false;
		}
		unsigned int dw = 0;
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

		unsigned int lastError = 0;
		while (!flag)
		{
			deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, dw, &deviceInterfaceData))
			{
				lastError = GetLastError();
				return false;
			}
			dw++;
			DWORD dwRequiredSize = 0;
			if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, NULL, 0, &dwRequiredSize, NULL))
			{
				//std::cout << "Getting the needed buffer size failed";
				return false;
			}
			//std::cout << "Required size is " << dwRequiredSize << std::endl;
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				//std::cout << "Last error is not ERROR_INSUFFICIENT_BUFFER";
				return false;
			}
			std::unique_ptr<SP_DEVICE_INTERFACE_DETAIL_DATA> deviceInterfaceDetailData((SP_DEVICE_INTERFACE_DETAIL_DATA*)new char[dwRequiredSize]);
			deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, deviceInterfaceDetailData.get(), dwRequiredSize, NULL, NULL))
			{
				std::wstring devicePath = deviceInterfaceDetailData->DevicePath;
				//OutputDebugString(devicePath.c_str());
				devHandle = CreateFile(devicePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

				if (devHandle != INVALID_HANDLE_VALUE)
				{
					std::unique_ptr<HIDD_ATTRIBUTES> attributes(new HIDD_ATTRIBUTES);
					attributes->Size = sizeof(HIDD_ATTRIBUTES);
					if (HidD_GetAttributes(devHandle, attributes.get()))
					{

						if (((attributes->VendorID == vid) && (attributes->ProductID == pid)))
						{

							flag = true;
						}
					}

				}
			}
		}
		//OutputDebugString(flag);
		return flag;
	}

	int GetByteLength()
	{
		return length;

	}



	bool Functions::Reset(bool status)
	{
		size_t BytesWritten;
		byte Buffer[] = { 0x02 ,0x07 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
		if(status)
		Buffer[2] = 0x04;
		else
			Buffer[2] = 0x03;
		return DeviceIoControl(devHandle, IOCTL_HID_SET_OUTPUT_REPORT, Buffer, length, NULL, 0, (DWORD*)&BytesWritten, NULL);
	}

	bool Functions::UpdateColors()
	{
		size_t BytesWritten;
		byte Buffer[] = { 0x02 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,0x00 ,0x00 ,0x00 };
		return DeviceIoControl(devHandle, IOCTL_HID_SET_OUTPUT_REPORT, Buffer, length, NULL, 0, (DWORD*)&BytesWritten, NULL);
	}

	void Loop()
	{
		size_t BytesWritten;
		byte Buffer[] = { 0x02 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,0x00 ,0x00 ,0x00 };
		DeviceIoControl(devHandle, IOCTL_HID_SET_OUTPUT_REPORT, Buffer, length, NULL, 0, (DWORD*)&BytesWritten, NULL);
	}


	int Power = 13;
	int Macro = 0;
	int leftZone = 0x8;
	int leftMiddleZone = 0x4;
	int rightZone = 0x1;
	int rightMiddleZone = 0x2;
	int AlienFrontLogo = 0x40;
	int AlienBackLogo = 0x20;
	int LeftPanelTop = 0x1000;
	int LeftPanelBottom = 0x400;
	int RightPanelTop = 0x2000;
	int RightPanelBottom = 0x800;
	int TouchPad = 0x80;


	bool Functions::SetColor(int index, int r, int g, int b)
	{
		int location;
		switch (index)
		{
		case 1: location = leftZone; break;
		case 2: location = leftMiddleZone; break;
		case 3: location = rightZone; break;
		case 4: location = rightMiddleZone; break;
		case 5: location = Macro; break;
		case 6: location = AlienFrontLogo; break;
		case 7: location = LeftPanelTop; break;
		case 8: location = LeftPanelBottom; break;
		case 9: location = RightPanelTop; break;
		case 10: location = RightPanelBottom; break;
	
		case 12: location = AlienBackLogo; break;
		case 11: location = Power; break; 
		case 13: location = TouchPad; break;
		default: location = AlienFrontLogo; break;
		}
		size_t BytesWritten;
		byte Buffer[] = { 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,0x00 ,0x00 ,0x00 };
		Buffer[0] = 0x02;
		Buffer[1] = 0x03;
		Buffer[2] = index;
		Buffer[3] = (location & 0xFF0000) >> 16;
		Buffer[4] = (location & 0x00FF00) >> 8;
		Buffer[5] = (location & 0x0000FF);
		Buffer[6] = r;
		Buffer[7] = g;
		Buffer[8] = b;
		
		if (index == 5)
		{
			Buffer[1] = 0x83;
			Buffer[2] = (byte)index;
			Buffer[3] = 00;
			Buffer[4] = 00;
			Buffer[5] = 00;
		}
		
		 if (index == 11)
		{
			Buffer[1] = 0x01;
			Buffer[2] = (byte)index;
			Buffer[3] = 00;
			Buffer[4] = 01;
			Buffer[5] = 00;
		}
		bool val = DeviceIoControl(devHandle, IOCTL_HID_SET_OUTPUT_REPORT, Buffer, length, NULL, 0, (DWORD*)&BytesWritten, NULL);
		Loop();
		return val;
	}

	int ReadStatus;

	byte AlienfxGetDeviceStatus()
	{
		size_t BytesWritten;
		byte Buffer[] = { 0x02 ,0x06 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00};
		DeviceIoControl(devHandle, IOCTL_HID_SET_OUTPUT_REPORT, Buffer, length, NULL, 0, (DWORD*)&BytesWritten, NULL);
		
			Buffer[0] = 0x01;
			DeviceIoControl(devHandle, IOCTL_HID_GET_INPUT_REPORT, NULL, 0, Buffer, length, (DWORD*)&BytesWritten, NULL);
	
			if (Buffer[0] == 0x01)
				return 0x06;
			return Buffer[0];
	}

	byte AlienfxWaitForReady()
	{
		byte status = AlienfxGetDeviceStatus();
		for (int i = 0; i < 5 && status != ALIENFX_READY; i++)
		{
			if (status == ALIENFX_DEVICE_RESET)
				return status;
			Sleep(2);
			status = AlienfxGetDeviceStatus();
		}
		return status;
	}

	byte AlienfxWaitForBusy()
	{

		byte status = AlienfxGetDeviceStatus();
		for (int i = 0; i < 5 && status != ALIENFX_BUSY; i++)
		{
			if (status == ALIENFX_DEVICE_RESET)
				return status;
			Sleep(2);
			status = AlienfxGetDeviceStatus();
		}
		return status;
	}

	bool Functions::IsDeviceReady()
	{
		int status = AlienfxWaitForBusy();

		if (status == ALIENFX_DEVICE_RESET)
		{
			Sleep(1000);
			
			return false;
			//AlienfxReinit();

		}
		else if (status != ALIENFX_BUSY)
		{
			Sleep(50);
		}
		Reset(0x04);
		
		status = AlienfxWaitForReady();
		if (status == 6)
		{
			Sleep(1000);
			//AlienfxReinit();
			
			return false;
		}
		else if (status != ALIENFX_READY)
		{
			if (status == ALIENFX_BUSY)
			{
				Reset(0x04);
			
				status = AlienfxWaitForReady();
				if (status == ALIENFX_DEVICE_RESET)
				{
					Sleep(1000);
					//AlienfxReinit();
					return false;
				}
			}
			else
			{
				Sleep(50);
				
				return false;
			}
		}
		return true;
	}

	bool Functions::AlienFXClose()
	{
		bool result;
		if (devHandle != NULL)
		{
			result = CloseHandle(devHandle);
		}
		return result;
	}

	int GetError()
	{
		return GetLastError();
	}
}
