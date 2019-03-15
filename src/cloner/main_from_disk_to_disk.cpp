// 
// file:			main_from_disk_to_disk.cpp
// created on:		2019 Mar 03
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

#define MEMORY_OP_SIZE			4096
#define MASK_32_BIT				0x00000000ffffffff
#define STATUS_SHOW_PERIOD_MS	2000

static void ClonnerThreadFunction(void);
static void CtrlThreadFunction(void);
static void PrintInfoThreadFunction(void);  // In this case this will be called from main
static void NTAPI APCFunction(ULONG_PTR) {}

static int	s_nReturnByClonner = 0;
static int	s_nShouldClonnerWork = 0;
static int	s_nIsClonningActive = 0;
static int	s_nKeyboardPressed = 0;
static uint64_t s_ullnReaded, s_ullnTotalToRead;
static HANDLE	s_ctrlThreadHandle = (HANDLE)0;

int main()
{
	s_nShouldClonnerWork = s_nIsClonningActive = 1;

	::std::thread threadClonner = ::std::thread(ClonnerThreadFunction);
	::std::thread threadControl = ::std::thread(CtrlThreadFunction);
	
	s_ctrlThreadHandle = threadControl.native_handle();
	PrintInfoThreadFunction();
	threadControl.join();
	threadClonner.join();

	return s_nReturnByClonner;
}


static void ClonnerThreadFunction(void)
{
	int nReturn(-1);
	DISK_GEOMETRY_EX drvGeoInp, drvGeoOut;
	DWORD dwReturned;
	BOOL isOk;
	HANDLE hDriveInp = INVALID_HANDLE_VALUE;
	HANDLE hDriveOut = INVALID_HANDLE_VALUE;
	char vcBuffer[MEMORY_OP_SIZE];

	hDriveInp = CreateFileA(_devicenameInp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDriveInp == INVALID_HANDLE_VALUE) { goto returnPoint; }

	hDriveOut = CreateFileA(_devicenameOut, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDriveOut == INVALID_HANDLE_VALUE) { goto returnPoint; }

	isOk = DeviceIoControl(hDriveInp, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &drvGeoInp, sizeof(DISK_GEOMETRY_EX), &dwReturned, NULL);//_PARTITION_INFORMATION
	if (!isOk) { goto returnPoint; }

	isOk = DeviceIoControl(hDriveOut, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &drvGeoOut, sizeof(DISK_GEOMETRY_EX), &dwReturned, NULL);//_PARTITION_INFORMATION
	if (!isOk) { goto returnPoint; }

	if ((drvGeoInp.DiskSize.HighPart > drvGeoOut.DiskSize.HighPart) || ((drvGeoInp.DiskSize.HighPart == drvGeoOut.DiskSize.HighPart) && (drvGeoInp.DiskSize.LowPart > drvGeoOut.DiskSize.LowPart))) {
		goto returnPoint;
	}

	s_ullnTotalToRead = drvGeoInp.DiskSize.HighPart;
	s_ullnTotalToRead <<= 32;
	s_ullnTotalToRead += drvGeoInp.DiskSize.LowPart;
	s_ullnTotalToRead = (s_ullnTotalToRead / MEMORY_OP_SIZE)*MEMORY_OP_SIZE;

	for (s_ullnReaded = 0; s_nShouldClonnerWork && ReadFile(hDriveInp, vcBuffer, MEMORY_OP_SIZE, &dwReturned, NULL) && (s_ullnReaded < s_ullnTotalToRead); s_ullnReaded += dwReturned) {
		if (!WriteFile(hDriveOut, vcBuffer, dwReturned, NULL, NULL)) { break; }
	}

	nReturn = 0;
returnPoint:
	s_nReturnByClonner = nReturn?GetLastError():0;
	if (hDriveOut != INVALID_HANDLE_VALUE) { CloseHandle(hDriveOut); }
	if (hDriveInp != INVALID_HANDLE_VALUE) { CloseHandle(hDriveInp); }
	s_nIsClonningActive = 0;
}


static void CtrlThreadFunction(void)
{
	do{
		_getch();
		s_nKeyboardPressed = 1;
		while(s_nShouldClonnerWork && s_nKeyboardPressed){
			SleepEx(INFINITE,TRUE);
		}
	} while (s_nShouldClonnerWork);
}


static void PrintInfoThreadFunction(void)
{
	int i,nIteration(0),nLastLinePrinted(0);
	char cReason;
	const char vcTickTack[2] = { '\\','/' };
	while(s_nShouldClonnerWork && s_nIsClonningActive){
		for (i = 0; i < nLastLinePrinted; ++i) {
			printf("\b");
		}
		nLastLinePrinted = printf("progress = %lf %c  ", 100. * double(s_ullnReaded) / double(s_ullnTotalToRead), vcTickTack[(++nIteration) % 2]);
		SleepEx(STATUS_SHOW_PERIOD_MS, TRUE);
		while(s_nKeyboardPressed){
			printf("\nWhich action should be done? s/c (stop/continue) ");
			fflush(stdout);
			cReason=_getch();
			if(cReason=='s'){
				s_nShouldClonnerWork = 0;
				s_nKeyboardPressed = 0;
				printf("\n");
				nLastLinePrinted = 0;
				QueueUserAPC(APCFunction,s_ctrlThreadHandle,NULL);
			}
			else if(cReason == 'c'){
				s_nKeyboardPressed = 0;
				printf("\n");
				nLastLinePrinted = 0;
				QueueUserAPC(APCFunction, s_ctrlThreadHandle, NULL);
			}
		}  // while(s_nKeyboardPressed){
		
	}  // while(s_nIsClonningActive){

	s_nShouldClonnerWork = 0;
	s_nKeyboardPressed = 0;
	printf("\nClonning done! Error returned by thread (non 0 is systemError): %d.\n",s_nReturnByClonner);
	printf("Press any key to exit ");
	fflush(stdout);
	s_nKeyboardPressed = 0;
	QueueUserAPC(APCFunction, s_ctrlThreadHandle, NULL);
}
