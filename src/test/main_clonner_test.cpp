//
// file:		main_clonner_test.cpp
// created on:	2019 Jan 7
//

#ifdef _MSC_VER
#pragma comment (lib,"zlib.lib")
#pragma comment(lib,"Wininet.lib")
#pragma warning(disable:4996)
#endif


#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <stdint.h>

static void PrintSize(int64_t a_llnSize);

int main()
{
	union {DRIVE_LAYOUT_INFORMATION_EX i;char b[8192];}dli;
	PARTITION_INFORMATION_EX pi;
	DISK_GEOMETRY_EX dg;
	//STORAGE_PROPERTY_QUERY spq;
	STORAGE_PROPERTY_QUERY spq = { StorageDeviceProperty, PropertyStandardQuery };
	STORAGE_DEVICE_DESCRIPTOR sdd;
	HANDLE hDrive;
	DWORD dwReturned, dwLogicalDrives;
	BOOL bSuccs;
	char _devicename[] = "\\\\.\\PhysicalDriveX";
	char logicalDrive[] = "\\\\.\\A:";
	char cLast('0' + 10);
	//wchar_t vcVolumeName[128];
	char vcFileSyst2[128];
	char cDrv;
	DWORD bitWs, i;

#if 1
	DWORD dwVolumeSerialNumber, dwpMaximumComponentLength, dwFileSystemFlags;
	char vcVolumeName[128];
	char vcVolumeNameBuf[128];
	HANDLE hFirstVolume = FindFirstVolumeA(vcVolumeName, 127);
	if (hFirstVolume != INVALID_HANDLE_VALUE) {
		do {
			bSuccs = GetVolumeInformationA(vcVolumeName, vcVolumeNameBuf, 127, &dwVolumeSerialNumber, &dwpMaximumComponentLength, &dwFileSystemFlags, vcFileSyst2, 127);
			printf("volume:  %s\t", vcVolumeName);
			if (bSuccs) {
				printf("name:\"%s\", fileSy:\"%s\"", vcVolumeNameBuf, vcFileSyst2);
			}
			printf("\n");
#if 0
			hDrive = CreateFileA(vcVolume, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			printf("hDrive[%s]=%p\t", vcVolume, hDrive);
			if (hDrive == INVALID_HANDLE_VALUE) { printf("\n"); continue; }
			bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0, &pi, sizeof(pi), &dwReturned, NULL);
			if (bSuccs) {
				PrintSize(pi.PartitionLength.QuadPart);
			}
			printf("\n");
			CloseHandle(hDrive);
#endif
		} while (FindNextVolumeA(hFirstVolume, vcVolumeName, 127));
		FindVolumeClose(hFirstVolume);
		printf("\n\n");
	}
#endif

	for (cDrv = '0'; cDrv < cLast; ++cDrv) {
		_devicename[17] = cDrv;
		hDrive = CreateFileA(_devicename, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		printf("hDrive%c=%p\t", cDrv, hDrive);
		if (hDrive == INVALID_HANDLE_VALUE) { printf("\n"); continue; }
		//GetVolumeInformationByHandleW(hDrive, vcVolume, 127, NULL, NULL, NULL, vcFileSyst, 127);
		bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &dg, sizeof(dg), &dwReturned, NULL);
		if (bSuccs) {
			PrintSize(dg.DiskSize.QuadPart);
		}
		else {
			printf(", lastError=%d", GetLastError());
		}
		bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0, &pi, sizeof(pi), &dwReturned, NULL);
		if (bSuccs) {
			//PrintSize(pi.PartitionLength.QuadPart);
		}
		else {
			printf(", lastError=%d", GetLastError());
		}
		bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &dli, sizeof(dli), &dwReturned, NULL);
		if (bSuccs) {
			//PrintSize(pi.dli.PartitionLength.QuadPart);
		}
		else {
			printf(", lastError=%d",GetLastError());
		}
		bSuccs = DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY,&spq, sizeof(spq), &sdd, sizeof(sdd), &dwReturned, NULL);
		if (bSuccs) {
			//PrintSize(pi.dli.PartitionLength.QuadPart);
		}
		else {
			printf(", lastError=%d", GetLastError());
		}
		printf("\n");
		CloseHandle(hDrive);
	}

	dwLogicalDrives = GetLogicalDrives();
	printf("\nlogicalDrives=%x\n\n", dwLogicalDrives);

	for (i = 0, bitWs = 1; i < 32; ++i, bitWs <<= 1) {
		if (!(bitWs&dwLogicalDrives)) { continue; }
		logicalDrive[4] = 'A' + char(i);
		hDrive = CreateFileA(logicalDrive, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		printf("hDrive[%s]=%p\t", logicalDrive, hDrive);
		if (hDrive == INVALID_HANDLE_VALUE) { printf("\n"); continue; }
		//GetVolumeInformationByHandleW(hDrive, vcVolume, 127, NULL, NULL, NULL, vcFileSyst, 127);
		//bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0, &pi, sizeof(pi), &dwReturned, NULL);//IOCTL_DISK_GET_DRIVE_LAYOUT
		bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL, 0, &pi, sizeof(pi), &dwReturned, NULL);
		if (bSuccs) {
			//PrintSize(pi.m_pi.PartitionLength.QuadPart);
		}
		printf("\n");
		CloseHandle(hDrive);
	}

	return 0;
}


static void PrintSize(int64_t a_llnSize)
{
#define NEXT_LEVEL_DEN	1024
	int64_t llnDenom(NEXT_LEVEL_DEN), llnSize = a_llnSize / NEXT_LEVEL_DEN;
	if (!llnSize) {
		std::cout << a_llnSize << " B";
		return;
	}

	llnSize /= NEXT_LEVEL_DEN;
	if (!llnSize) {
		std::cout << double(a_llnSize) / double(llnDenom) << " KB";
		return;
	}

	llnDenom *= NEXT_LEVEL_DEN;
	llnSize /= NEXT_LEVEL_DEN;
	if (!llnSize) {
		std::cout << double(a_llnSize) / double(llnDenom) << " MB";
		return;
	}


	llnDenom *= NEXT_LEVEL_DEN;
	llnSize /= NEXT_LEVEL_DEN;
	if (!llnSize) {
		std::cout << double(a_llnSize) / double(llnDenom) << " GB";
		return;
	}

	llnDenom *= NEXT_LEVEL_DEN;
	std::cout << double(a_llnSize) / double(llnDenom) << " TB";

}


#if 0

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <psapi.h>
#include <strsafe.h>

#define BUFSIZE 512

BOOL GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		_tprintf(TEXT("Cannot map a file with a length of zero.\n"));
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0,
		1,
		NULL);

	if (hFileMap)
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem)
		{
			if (GetMappedFileName(GetCurrentProcess(),
				pMem,
				pszFilename,
				MAX_PATH))
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[BUFSIZE];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(BUFSIZE - 1, szTemp))
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH)
							{
								bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0
									&& *(pszFilename + uNameLen) == _T('\\');

								if (bFound)
								{
									// Reconstruct pszFilename using szTempFile
									// Replace device path with DOS path
									TCHAR szTempFile[MAX_PATH];
									StringCchPrintf(szTempFile,
										MAX_PATH,
										TEXT("%s%s"),
										szDrive,
										pszFilename + uNameLen);
									StringCchCopyN(pszFilename, MAX_PATH + 1, szTempFile, _tcslen(szTempFile));
				}
			  }
			}

						// Go to the next NULL character.
						while (*p++);
		  } while (!bFound && *p); // end of string
		}
	  }
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
	}

		CloseHandle(hFileMap);
  }
	_tprintf(TEXT("File name is %s\n"), pszFilename);
	return(bSuccess);
}

int _tmain(int argc, TCHAR *argv[])
{
	HANDLE hFile;

	if (argc != 2)
	{
		_tprintf(TEXT("This sample takes a file name as a parameter.\n"));
		return 0;
	}
	hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("CreateFile failed with %d\n"), GetLastError());
		return 0;
	}
	GetFileNameFromHandle(hFile);
}


#endif



#if 0

#ifndef CINTERFACE
#define CINTERFACE
#endif

#define BURN_DISK

#include "zlib_compression_routines.h"
#include "zlib_decompress_routines.h"

#include <Windows.h>
#include <stdio.h>

#define		NUMBER_REG_IN_SEC	512
//#define		_devicename			TEXT( "\\\\.\\PhysicalDrive0")
#define		_devicename			TEXT( "\\\\.\\E:")


#ifdef BURN_DISK

#define  WEB_URL	"https://desycloud.desy.de/index.php/s/xBwZiLk6CftawAF/download?path=%2Fdisk_images&files=image.zlib1"

int main()
{
	int nReturn(-1);
	DWORD dwReturned = 0;
	BOOL bSuccs;
	DISK_GEOMETRY_EX dg;
	//PARTITION_INFORMATION dg;
	HANDLE hDrive = CreateFile(_devicename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if(hDrive == INVALID_HANDLE_VALUE){goto returnPoint;}

	bSuccs=DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &dg, sizeof(dg),&dwReturned, NULL);//_PARTITION_INFORMATION
	//bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0, &dg, sizeof(dg), &dwReturned, NULL);
	//bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_SET_PARTITION_INFO, NULL, 0, &dg, sizeof(dg), &dwReturned, NULL);
	if(!bSuccs){goto returnPoint;}
	//bSuccs = IOCTL_DISK_GET_DRIVE_GEOMETRY_EX;

	SetFilePointer(hDrive, 0, 0, FILE_BEGIN);

	ZlibBurnImageFromWeb(WEB_URL,hDrive,dg.DiskSize.QuadPart);

	nReturn = 0;
returnPoint:
	return nReturn;
}
#else   // #ifdef BURN_DISK

#define DEST_FILE_NAME		"D:\\image.zlib1"

int main()
{
	FILE* fpOut = NULL;
	int nReturn(-1);

	fpOut = fopen(DEST_FILE_NAME, "wb");
	if(!fpOut){goto returnPoint;}

	ZlibCompressDriveRaw(_devicename,fpOut,Z_DEFAULT_COMPRESSION);

	nReturn = 0;
returnPoint:
	if(fpOut){fclose(fpOut);}
	return nReturn;
}


#endif  // #ifdef BURN_DISK


#endif  // #if 0
