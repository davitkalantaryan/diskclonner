// 
// file:			main_any_test.cpp
// created on:		2019 Mar 15
//

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <iostream>

#define	_devicename			"\\\\.\\PhysicalDrive0"

int main()
{
	int nReturn(-1);
	//DISK_GEOMETRY_EX drvGeoInp, drvGeoOut;
	DWORD dwReturned;
	DWORD pi; 
	BOOL isOk;
	HANDLE hDrive = INVALID_HANDLE_VALUE;
	union { DRIVE_LAYOUT_INFORMATION_EX i; char b[8192]; }dli;

	hDrive = CreateFileA(_devicename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDrive == INVALID_HANDLE_VALUE) { goto returnPoint; }

	isOk = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &dli, sizeof(dli), &dwReturned, NULL);
	if (!isOk) { goto returnPoint; }

	::std::cout << "NumberOfPartitions=" << dli.i.PartitionCount << ", PartitionStyle: " << dli.i.PartitionStyle << ::std::endl;

	for(pi=0;pi<dli.i.PartitionCount;++pi){
		::std::cout 
			<< "Partition " << dli.i.PartitionEntry[pi].PartitionNumber << ::std::endl
			<< "\tPartitionStyle  :" << dli.i.PartitionEntry[pi].PartitionStyle << ::std::endl
			<< "\tPartitionOffset :" << ((uint64_t)dli.i.PartitionEntry[pi].StartingOffset.HighPart<<32)+((uint64_t)dli.i.PartitionEntry[pi].StartingOffset.LowPart) << ::std::endl
			<< "\tPartitionLength :" << ((uint64_t)dli.i.PartitionEntry[pi].PartitionLength.HighPart<<32)+((uint64_t)dli.i.PartitionEntry[pi].PartitionLength.LowPart) << ::std::endl
			<< ::std::endl;
	}

	nReturn = 0;
returnPoint:
	if(nReturn){
		nReturn = GetLastError();
	}
	if (hDrive != INVALID_HANDLE_VALUE) { CloseHandle(hDrive); }
	return nReturn;
}
