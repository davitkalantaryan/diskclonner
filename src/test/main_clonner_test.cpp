//
// file:		main_clonner_test.cpp
// created on:	2019 Jan 7
//

#ifdef _MSC_VER
#pragma comment (lib,"zlib.lib")
#pragma comment(lib,"Wininet.lib")
#pragma warning(disable:4996)
#endif

#if 0
#include <windows.h>
#include <stdio.h>

void DisplayVolumePaths(
	__in PWCHAR VolumeName
)
{
	DWORD  CharCount = MAX_PATH + 1;
	PWCHAR Names = NULL;
	PWCHAR NameIdx = NULL;
	BOOL   Success = FALSE;

	for (;;)
	{
		//
		//  Allocate a buffer to hold the paths.
		Names = (PWCHAR) new BYTE[CharCount * sizeof(WCHAR)];

		if (!Names)
		{
			//
			//  If memory can't be allocated, return.
			return;
		}

		//
		//  Obtain all of the paths
		//  for this volume.
		Success = GetVolumePathNamesForVolumeNameW(
			VolumeName, Names, CharCount, &CharCount
		);

		if (Success)
		{
			break;
		}

		if (GetLastError() != ERROR_MORE_DATA)
		{
			break;
		}

		//
		//  Try again with the
		//  new suggested size.
		delete[] Names;
		Names = NULL;
	}

	if (Success)
	{
		//
		//  Display the various paths.
		for (NameIdx = Names;
			NameIdx[0] != L'\0';
			NameIdx += wcslen(NameIdx) + 1)
		{
			wprintf(L"  %s", NameIdx);
		}
		wprintf(L"\n");
	}

	if (Names != NULL)
	{
		delete[] Names;
		Names = NULL;
	}

	return;
}

void __cdecl wmain(void)
{
	DWORD  CharCount = 0;
	WCHAR  DeviceName[MAX_PATH] = L"";
	DWORD  Error = ERROR_SUCCESS;
	HANDLE FindHandle = INVALID_HANDLE_VALUE;
	BOOL   Found = FALSE;
	size_t Index = 0;
	BOOL   Success = FALSE;
	WCHAR  VolumeName[MAX_PATH] = L"";

	//
	//  Enumerate all volumes in the system.
	FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

	if (FindHandle == INVALID_HANDLE_VALUE)
	{
		Error = GetLastError();
		wprintf(L"FindFirstVolumeW failed with error code %d\n", Error);
		return;
	}

	for (;;)
	{
		//
		//  Skip the \\?\ prefix and remove the trailing backslash.
		Index = wcslen(VolumeName) - 1;

		if (VolumeName[0] != L'\\' ||
			VolumeName[1] != L'\\' ||
			VolumeName[2] != L'?' ||
			VolumeName[3] != L'\\' ||
			VolumeName[Index] != L'\\')
		{
			Error = ERROR_BAD_PATHNAME;
			wprintf(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
			break;
		}

		//
		//  QueryDosDeviceW does not allow a trailing backslash,
		//  so temporarily remove it.
		VolumeName[Index] = L'\0';

		CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName));

		VolumeName[Index] = L'\\';

		if (CharCount == 0)
		{
			Error = GetLastError();
			wprintf(L"QueryDosDeviceW failed with error code %d\n", Error);
			break;
		}

		wprintf(L"\nFound a device:\n %s", DeviceName);
		wprintf(L"\nVolume name: %s", VolumeName);
		wprintf(L"\nPaths:");
		DisplayVolumePaths(VolumeName);

		//
		//  Move on to the next volume.
		Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

		if (!Success)
		{
			Error = GetLastError();

			if (Error != ERROR_NO_MORE_FILES)
			{
				wprintf(L"FindNextVolumeW failed with error code %d\n", Error);
				break;
			}

			//
			//  Finished iterating
			//  through all the volumes.
			Error = ERROR_SUCCESS;
			break;
		}
	}

	FindVolumeClose(FindHandle);
	FindHandle = INVALID_HANDLE_VALUE;

	return;
}

#endif


#if 1

#include "zlib_compression_routines.h"
#include "zlib_decompress_routines.h"
#include <stdio.h>
#include <iostream>


#if 0

static void PrintSize(int64_t a_llnSize);

int main()
{
	ULARGE_INTEGER liFreeBytesAvailableToCaller, liTotalNumberOfBytes, lipTotalNumberOfFreeBytes;
	union {DRIVE_LAYOUT_INFORMATION_EX i;char b[8192];}dli;
	DISK_GEOMETRY_EX dg;
	STORAGE_PROPERTY_QUERY spq = { StorageDeviceProperty, PropertyStandardQuery };
	STORAGE_DEVICE_DESCRIPTOR sdd;
	HANDLE hDrive;
	DWORD dwReturned;
	BOOL bSuccs;
	char _devicename[] = "\\\\.\\PhysicalDriveX";
	char cLast('0' + 10);
	char cDrv;

#if 0
	hDrive = CreateFileA("\\Device\\HarddiskVolume1", GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDrive != INVALID_HANDLE_VALUE) { CloseHandle(hDrive); }
	bSuccs = GetDiskFreeSpaceExA("Volume{7ae24770-0000-0000-0000-100000000000}", &liFreeBytesAvailableToCaller, &liTotalNumberOfBytes, &lipTotalNumberOfFreeBytes);
	if (!bSuccs) {
		printf(", GetDiskFreeSpaceExA:lastError=%d", GetLastError());
	}
#endif

	for (cDrv = '0'; cDrv < cLast; ++cDrv) {
		_devicename[17] = cDrv;
		hDrive = CreateFileA(_devicename, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		printf("hDrive%c=%p\t", cDrv, hDrive);
		if (hDrive == INVALID_HANDLE_VALUE) { printf("\n"); continue; }
		bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &dg, sizeof(dg), &dwReturned, NULL);
		if (bSuccs) {
			PrintSize(dg.DiskSize.QuadPart);
		}
		else {
			printf(", GET_DRIVE_GEOMETRY:lastError=%d", GetLastError());
		}
		bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &dli, sizeof(dli), &dwReturned, NULL);
		if (!bSuccs) {
			printf(", GET_DRIVE_LAYOUT:lastError=%d",GetLastError());
		}
		bSuccs = DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY,&spq, sizeof(spq), &sdd, sizeof(sdd), &dwReturned, NULL);
		if (bSuccs) {
			printf(", %s", sdd.RemovableMedia?"removable":"not_removable");
		}
		else {
			printf(", STORAGE_QUERY_PROPERTY:lastError=%d", GetLastError());
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

#endif



#if 1


#define BURN_DISK


#define		NUMBER_REG_IN_SEC	512
//#define		_devicename			TEXT( "\\\\.\\PhysicalDrive0")
//#define		_devicename			TEXT( "\\\\.\\E:")
#define		_devicename			"\\\\.\\PhysicalDrive1"


#ifdef BURN_DISK

#define  WEB_URL	"https://desycloud.desy.de/index.php/s/xBwZiLk6CftawAF/download?path=%2Fdisk_images&files=image.zlib1"

int main()
{
	int nReturn(-1);
	DWORD dwReturned = 0;
	BOOL bSuccs;
	DISK_GEOMETRY_EX dg;
	//PARTITION_INFORMATION dg;
	HANDLE hDrive = CreateFile(_devicename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

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

	ZlibCompressDiskRaw(_devicename,fpOut,Z_DEFAULT_COMPRESSION);

	nReturn = 0;
returnPoint:
	if(fpOut){fclose(fpOut);}
	return nReturn;
}


#endif  // #ifdef BURN_DISK


#endif  // #if 0


#endif
