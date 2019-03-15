// 
// file:			main_set_volumes_info_test.cpp
// created on:		2019 Mar 15
//

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <thread>

#define	_devicenameInp			"\\\\.\\PhysicalDrive0"
#define	_devicenameOut			"\\\\.\\PhysicalDrive1"

int main()
{
	int nReturn(-1);
	//DISK_GEOMETRY_EX drvGeoInp, drvGeoOut;
	DWORD dwReturned;
	BOOL isOk;
	HANDLE hDriveInp = INVALID_HANDLE_VALUE;
	HANDLE hDriveOut = INVALID_HANDLE_VALUE;
	union { DRIVE_LAYOUT_INFORMATION_EX i; char b[8192]; }dli;

	hDriveInp = CreateFileA(_devicenameInp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDriveInp == INVALID_HANDLE_VALUE) { goto returnPoint; }

	hDriveOut = CreateFileA(_devicenameOut, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDriveOut == INVALID_HANDLE_VALUE) { goto returnPoint; }

	isOk = DeviceIoControl(hDriveInp, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &dli, sizeof(dli), &dwReturned, NULL);
	if (!isOk) { goto returnPoint; }

	isOk = DeviceIoControl(hDriveOut, IOCTL_DISK_SET_DRIVE_LAYOUT_EX, &dli, dwReturned, NULL, 0, &dwReturned, NULL);
	if (!isOk) {goto returnPoint;}

	isOk = DeviceIoControl(hDriveOut, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &dwReturned, NULL);
	if (!isOk) {goto returnPoint;}

	nReturn = 0;
returnPoint:
	if(nReturn){
		nReturn = GetLastError();
	}
	return nReturn;
}
