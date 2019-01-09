//
// file:		main_clonner_test.cpp
// created on:	2019 Jan 7
//
// docs:
//		https://www.installsetupconfig.com/win32programming/windowsdiskapis2_3.html  
//		https://github.com/pbatard/rufus/blob/master/src/drive.c  
//

#define BURN_DISK

#include <disk_clone_by_zipping_and_partition_using_zip.h>
#include <zlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#define MAKE_FINAL_REPORT(...)	printf(__VA_ARGS__);fflush(stdout)
#else
#define MAKE_FINAL_REPORT(...)
#define _getch()
#endif


#define		_devicename			"\\\\.\\PhysicalDrive1"
#define		DEST_FILE_NAME		"D:\\image.zlib1"
#define		WEB_URL				"https://desycloud.desy.de/index.php/s/xBwZiLk6CftawAF/download?path=%2Fdisk_images&files=image.zlib1"

#ifdef BURN_DISK

BEGIN_C_DECLS2
HINSTANCE hMainInstance;
HWND hMainDialog = NULL;
END_C_DECLS2

int main()
{
	int nReturn = DecompressFromPathOrUrlAndPrepareDisk(_devicename,DEST_FILE_NAME);

	hMainInstance = GetModuleHandle(NULL);

	MAKE_FINAL_REPORT("Press any key to exit");
	_getch();
	printf("\n");
	return nReturn;
}
#else   // #ifdef BURN_DISK

int main()
{

	int nReturn = CompressDiskToPathOrUrl(_devicename,DEST_FILE_NAME,Z_DEFAULT_COMPRESSION);

	printf("compression returned: %d\n", nReturn);

	//nReturn = 0;
//returnPoint:
	MAKE_FINAL_REPORT("Press any key to exit");
	_getch();
	printf("\n");
	return nReturn;
}


#endif  // #ifdef BURN_DISK
