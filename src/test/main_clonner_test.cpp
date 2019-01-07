//
// file:		main_clonner_test.cpp
// created on:	2019 Jan 7
//

#ifndef CINTERFACE
#define CINTERFACE
#endif

#include <Windows.h>
#include <stdio.h>

#define		NUMBER_REG_IN_SEC	512
//#define		_devicename			TEXT( "\\\\.\\PhysicalDrive0")
#define		_devicename			TEXT( "\\\\.\\E:")


int main()
{
	DISK_GEOMETRY_EX dg;
	DWORD dwReturned = 0;
	BOOL bSuccs;
	int nReturn(-1);
	//DWORD dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	HANDLE hDrive = CreateFile(_devicename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
	if(hDrive == INVALID_HANDLE_VALUE){goto returnPoint;}

	SetFilePointer(hDrive,0,0,FILE_BEGIN);

	bSuccs=DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &dg, sizeof(dg),
		&dwReturned, NULL);

	//bSuccs=GetDiskFreeSpace("E:\\",&dwSectorsPerCluster,&dwBytesPerSector,&dwNumberOfFreeClusters,&dwTotalNumberOfClusters);
#if 0
	printf(
		"GetDiskFreeSpace returned:%d\nSectorsPerCluster=%d\n"
		"BytesPerSector=%d\n"
		"NumberOfFreeClusters=%d\n"
		"TotalNumberOfClusters=%d\n",
		bSuccs,dwSectorsPerCluster,dwBytesPerSector,dwNumberOfFreeClusters,dwTotalNumberOfClusters);
#endif
	if(!bSuccs){goto returnPoint;}

	nReturn = 0;
returnPoint:
	if(hDrive!= INVALID_HANDLE_VALUE){CloseHandle(hDrive);}
	return nReturn;
}
