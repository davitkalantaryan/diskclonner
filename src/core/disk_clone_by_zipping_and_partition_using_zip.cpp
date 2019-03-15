//
// file:		disk_clone_by_zipping_and_partition_using_zip.cpp
// created on:	2019 Jan 09
//


#define _TEST_BY_RUFUS

#define KEEP_C_NOMINMAX

#include <common_disk_clonner_project_include.h>

BEGIN_C_DECLS2

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#endif
#include <zlib.h>
#include "../include/private_header_for_compress_decompress.h"

#define DEF_CHUNK_SIZE				16384  // 2^14
#ifndef DRIVE_ACCESS_TIMEOUT_NEW
#define DRIVE_ACCESS_TIMEOUT_NEW	10000
#endif
#define DRIVE_ACCESS_SLEEP_TIME		100

BOOL CreatePartition(HANDLE hDrive, int partition_style, int file_system, BOOL mbr_uefi_marker, uint8_t extra_partitions);

typedef struct SUserDataForDrive
{
	SDiskCompDecompHeader* dch;
	int64_t	driveSize, alreadyReadBytes;
#ifdef _WIN32
	HANDLE	driveHandle;
#else
#endif
}SUserDataForDrive;

static int CallbackForDecompressToDrive(const void*a_buffer, int a_bufLen, void*a_userData);
static int CallbackForCompressToFileOrUrl(const void*a_buffer, int a_bufLen, void*a_userData);
static int DecompressFromFileOrWebToCallback(
	z_stream* a_strm,
	const SFileOrNetResource* a_source,
	void* a_in, int a_inBufferSize,
	void* a_out, int a_outBufferSize,
	typeCompressDecompressClbk a_clbk, void* a_userData);

INCLUDE_LIB("zlib.lib")
INCLUDE_LIB("Wininet.lib")

DISABLE_WARNING(4996)


int ZlibCompressBufferToCallback(
	z_stream* a_strm, int a_flush,
	void* a_out, int a_outBufferSize,
	typeCompressDecompressClbk a_clbk, void* a_userData);
int ZlibDecompressBufferToCallback(
	z_stream* a_strm,
	void* a_out, int a_outBufferSize,
	typeCompressDecompressClbk a_clbk, void* a_userData);
int CompressFromHandleToCallbackRawEx(
	z_stream* a_strm,
	HANDLE a_source,
	typeCompressDecompressClbk a_callback, void* a_userData,
	void* a_in, int a_inBufferSize,
	void* a_out, int a_outBufferSize,
	int a_nZlibFlush, int64_t a_llnOffset, int64_t a_llnReadSize);

int DecompressFromPathOrUrlAndPrepareDisk(const char* a_driveName, const char* a_cpcPathOrUrl)
{
	int nReturn = -;
	int nZstrInited = 0;
	SUserDataForDrive aUserData;
	SFileOrNetResource aResource = { 0 };
	DISK_GEOMETRY_EX drvGeo;
	DWORD dwReturned;
	int isDataInited = 0;
	BOOL isOk;
	z_stream strm;
	unsigned char in[DEF_CHUNK_SIZE];
	unsigned char out[DEF_CHUNK_SIZE];
	HANDLE hDrive = CreateFileA(a_driveName, GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hDrive == INVALID_HANDLE_VALUE) { goto returnPoint; }

	isOk = DeviceIoControl(hDrive,IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,NULL,0,&drvGeo,sizeof(drvGeo),&dwReturned, NULL);//_PARTITION_INFORMATION
	if (!isOk) { goto returnPoint; }

	if((strncmp(a_cpcPathOrUrl,"http",4)==0)||(strncmp(a_cpcPathOrUrl,"ftp",3)==0)){
		aResource.isUrl = 1;
		aResource.h.i.session = InternetOpenA("DesyCloud", NULL, NULL, NULL, NULL);
		if (!aResource.h.i.session) { goto returnPoint; }

		aResource.h.i.url = InternetOpenUrlA(
			aResource.h.i.session, a_cpcPathOrUrl,
			NULL, 0,
			INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE,
			NULL);
		if (!aResource.h.i.url) { goto returnPoint; }
	}
	else{
		aResource.isUrl = 0;
		aResource.h.file = fopen(a_cpcPathOrUrl,"rb");
		if(!aResource.h.file){goto returnPoint;}
	}

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	nReturn = inflateInit(&strm);
	if (nReturn != Z_OK) { goto returnPoint; }
	nZstrInited = 1;

	aUserData.dch = DZlibCreateHeaderForDiscCompression(0, 0, NULL); isDataInited = 1;
	aUserData.alreadyReadBytes = 0;
	aUserData.driveHandle = hDrive;
	aUserData.driveSize = drvGeo.DiskSize.QuadPart;
	nReturn=DecompressFromFileOrWebToCallback(&strm,&aResource,in, DEF_CHUNK_SIZE,out, DEF_CHUNK_SIZE,CallbackForDecompressToDrive,&aUserData);

returnPoint:
	if(aResource.h.d.d1){
		if(aResource.isUrl){
			if(aResource.h.i.url){InternetCloseHandle(aResource.h.i.url);}
			if(aResource.h.i.session){InternetCloseHandle(aResource.h.i.session);}
		}
		else{
			fclose(aResource.h.file);
		}
	}
	if(isDataInited){ DestroyHeaderForDiscCompression(aUserData.dch);}
	if(nZstrInited){(void)inflateEnd(&strm);}
	if(hDrive != INVALID_HANDLE_VALUE) { CloseHandle(hDrive); }
	return nReturn;
}

/* Compress from file source to file dest until EOF on source.
def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
allocated for processing, Z_STREAM_ERROR if an invalid compression
level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
version of the library linked do not match, or Z_ERRNO if there is
an error reading or writing the files. */
int CompressDiskToPathOrUrl(const char* a_driveName, const char* a_cpcPathOrUrl,int a_nCompressionLeel)
{
	int nReturn=Z_ERRNO;
	int nZstrInited = 0;
	SFileOrNetResource aResource = {0};
	PARTITION_INFORMATION_EX* pPartitions;
	SDiskCompDecompHeader* pHeader= NULLPTR2;
	HANDLE hDrive = INVALID_HANDLE_VALUE;
	union { DRIVE_LAYOUT_INFORMATION_EX i; char b[8192]; }dli;
	int64_t llnDiskSize, llnNextRead=0, llnStart, llnEnd;
	uint32_t i,unNumberOfPartitions;
	z_stream strm;
	unsigned char in[DEF_CHUNK_SIZE];
	unsigned char out[DEF_CHUNK_SIZE];
	int nZlibFlush;

	aResource.h.d = {NULLPTR2,NULLPTR2};

#ifdef _WIN32

	DISK_GEOMETRY_EX dg2;
	DWORD dwReturned;
	BOOL bSuccs;
	
	hDrive = CreateFileA(a_driveName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hDrive == INVALID_HANDLE_VALUE) { return -2; }

	bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &dg2, sizeof(dg2), &dwReturned, NULL);
	if (!bSuccs) { goto returnPoint;}
	llnDiskSize = dg2.DiskSize.QuadPart;

	bSuccs = DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &dli, sizeof(dli), &dwReturned, NULL);
	if (!bSuccs) {goto returnPoint;}

#else   // #ifdef _WIN32
#endif  // #ifdef _WIN32

	if(llnDiskSize<MINIMUM_DISK_SIZE_TO_COMPRESS){goto returnPoint;}

	if((strncmp(a_cpcPathOrUrl,"http",4)==0)||(strncmp(a_cpcPathOrUrl,"ftp",3)==0)){
		aResource.isUrl = 1;
		aResource.h.i.session = InternetOpenA("DesyCloud", NULL, NULL, NULL, NULL);
		if (!aResource.h.i.session) { goto returnPoint; }

		aResource.h.i.url = InternetOpenUrlA(
			aResource.h.i.session, a_cpcPathOrUrl,
			NULL, 0,
			INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE,
			NULL);
		if (!aResource.h.i.url) { goto returnPoint; }
	}
	else{
		aResource.isUrl = 0;
		aResource.h.file = fopen(a_cpcPathOrUrl,"wb");
		if(!aResource.h.file){goto returnPoint;}
	}

	unNumberOfPartitions = dli.i.PartitionCount;
	pHeader=DZlibCreateHeaderForDiscCompression(llnDiskSize,dwReturned,&dli.i);
	if (!pHeader) { goto returnPoint; }

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	nReturn = deflateInit(&strm, a_nCompressionLeel);
	if (nReturn != Z_OK){goto returnPoint;}
	nZstrInited = 1;

	strm.avail_in = pHeader->wholeHeaderSizeInBytes;
	strm.next_in = (Bytef*)pHeader;
	//if (ZlibCompressBufferToFile(&strm,0,out, DEF_CHUNK_SIZE, a_dest) == Z_STREAM_ERROR) { goto returnPoint;}
	if(ZlibCompressBufferToCallback(&strm,0,out,DEF_CHUNK_SIZE,&CallbackForCompressToFileOrUrl,&aResource)==Z_STREAM_ERROR){goto returnPoint;}

	SortDiscCompressDecompressHeader(pHeader);

	if(pHeader->isAnyPartAvalible){
		pPartitions = DISK_INFO_FROM_ITEM(pHeader)->PartitionEntry;
		nReturn=CompressFromHandleToCallbackRawEx(&strm,hDrive,&CallbackForCompressToFileOrUrl,&aResource,in,DEF_CHUNK_SIZE,out,DEF_CHUNK_SIZE,Z_PARTIAL_FLUSH,0,MINIMUM_DISK_SIZE_TO_COMPRESS);
		llnNextRead = MINIMUM_DISK_SIZE_TO_COMPRESS;
		for (i = 1; (nReturn != Z_STREAM_ERROR) && (i < unNumberOfPartitions); ++i) {
			llnStart = max(pPartitions[i].StartingOffset.QuadPart, llnNextRead);
			llnEnd = pPartitions[i].StartingOffset.QuadPart + pPartitions[i].PartitionLength.QuadPart;
			if (llnEnd > llnStart) {
				nZlibFlush = pPartitions[i].RewritePartition?Z_FINISH:Z_PARTIAL_FLUSH;
				nReturn=CompressFromHandleToCallbackRawEx(&strm,hDrive,&CallbackForCompressToFileOrUrl,&aResource,in,DEF_CHUNK_SIZE,out,DEF_CHUNK_SIZE,nZlibFlush,llnStart,llnEnd-llnStart);
				llnNextRead = llnEnd;
			}
		}  // for (i = 1; (nReturn != Z_STREAM_ERROR) && (i < unNumberOfPartitions); ++i) {
	}
	else{nReturn=CompressFromHandleToCallbackRawEx(&strm,hDrive,&CallbackForCompressToFileOrUrl,&aResource,in,DEF_CHUNK_SIZE,out,DEF_CHUNK_SIZE,Z_FINISH,0,MINIMUM_DISK_SIZE_TO_COMPRESS);}

returnPoint:
	if(aResource.h.d.d1){
		if(aResource.isUrl){
			if(aResource.h.i.url){InternetCloseHandle(aResource.h.i.url);}
			if(aResource.h.i.session){InternetCloseHandle(aResource.h.i.session);}
		}
		else{
			fclose(aResource.h.file);
		}
	}
	if(nZstrInited){(void)deflateEnd(&strm);}
	if(hDrive!= INVALID_HANDLE_VALUE){CloseHandle(hDrive);}
	if(pHeader){DestroyHeaderForDiscCompression(pHeader);}
	return nReturn;
}


/* Compress from file source to file dest until EOF on source.
def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
allocated for processing, Z_STREAM_ERROR if an invalid compression
level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
version of the library linked do not match, or Z_ERRNO if there is
an error reading or writing the files. */
int CompressFromHandleToCallbackRawEx(
	z_stream* a_strm,
	HANDLE a_source, 
	typeCompressDecompressClbk a_callback, void* a_userData,
	void* a_in, int a_inBufferSize,
	void* a_out, int a_outBufferSize,
	int a_nZlibFlush, int64_t a_llnOffset, int64_t a_llnReadSize)
{
	DWORD dwBytesRead;
	BOOL bRet;
	int ret=Z_OK;
	int inBufferSize;
	int64_t llnReadedTotal(0);

#ifdef _WIN32
	LONG lDistToMoveLow = (LONG)(a_llnOffset & 0xffffffff);
	LONG lDistToMoveHigh = (LONG)((a_llnOffset>>32) & 0xffffffff);
	if(SetFilePointer(a_source, lDistToMoveLow,&lDistToMoveHigh, FILE_BEGIN)== INVALID_SET_FILE_POINTER){return -1;}
#else
#endif

	/* compress until end of file */
	do {
		inBufferSize = min(a_inBufferSize,(int)(a_llnReadSize-llnReadedTotal));
		bRet=ReadFile(a_source,a_in,inBufferSize,&dwBytesRead,NULL);
		if(!bRet){return Z_ERRNO;}
		a_strm->avail_in = dwBytesRead;
		a_strm->next_in = (Bytef*)a_in;
		llnReadedTotal += dwBytesRead;
		if(dwBytesRead&&(llnReadedTotal<a_llnReadSize)){
			ret=ZlibCompressBufferToCallback(a_strm,Z_NO_FLUSH,a_out,a_outBufferSize,a_callback,a_userData);
		}
		else{
			ret=ZlibCompressBufferToCallback(a_strm,a_nZlibFlush,a_out,a_outBufferSize,a_callback,a_userData);
			break;
		}

	} while (ret!=Z_STREAM_ERROR);
	//if(a_nFlushInTheEnd){assert(ret == Z_STREAM_END);}        /* stream will be complete */
	
	return Z_OK;
}


static int DecompressFromFileOrWebToCallback(
	z_stream* a_strm,
	const SFileOrNetResource* a_source,
	void* a_in, int a_inBufferSize,
	void* a_out, int a_outBufferSize,
	typeCompressDecompressClbk a_clbk, void* a_userData)
{
	int retInf;
	DWORD dwRead;

	/* decompress until deflate stream ends or end of file */
	do {
		if(a_source->isUrl){
			if(!InternetReadFile(a_source->h.i.url,a_in,a_inBufferSize,&dwRead)){return Z_ERRNO;}
		}
		else{
			dwRead=STATIC_CAST2(fread(a_in,1,a_inBufferSize,a_source->h.file),DWORD);
			if(ferror(a_source->h.file)){return Z_ERRNO;}
		}
		a_strm->avail_in = dwRead;
		if (a_strm->avail_in == 0){break;} // we are done
		a_strm->next_in = (Bytef*)a_in;

		retInf=ZlibDecompressBufferToCallback(a_strm,a_out,a_outBufferSize,a_clbk,a_userData);
		if(retInf<0){return retInf;}

	} while (retInf != Z_STREAM_END);

	return 0;

}



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
static int CallbackForCompressToFileOrUrl(const void*a_buffer, int a_bufLen, void*a_userData)
{
	SFileOrNetResource* pCompData = (SFileOrNetResource*)a_userData;
	int nTotalWritten = 0, isOk;
	DWORD dwWritten;  // for non windows DWOER should be defined

	if(pCompData->isUrl){
		
		do {
			isOk=InternetWriteFile(pCompData->h.i.url, a_buffer, a_bufLen,&dwWritten);
			nTotalWritten += dwWritten;
		} while ((nTotalWritten < a_bufLen) && isOk);
	}
	else{
		do {
			dwWritten= STATIC_CAST2(fwrite(a_buffer, 1, a_bufLen,pCompData->h.file),DWORD);
			isOk=(~ferror(pCompData->h.file));
			nTotalWritten += dwWritten;
		} while ((nTotalWritten < a_bufLen) && isOk);
	}

	return isOk ? 0: Z_ERRNO;
}


#ifdef _TEST_BY_RUFUS
#include "rufus.h"
#include "drive.h"
extern RUFUS_DRIVE_INFO SelectedDrive;
#endif


static int CallbackForDecompressToDrive(const void*a_buffer, int a_bufLen, void*a_userData)
{
	static const int64_t scnHeaderSize(sizeof(SDiskCompDecompHeader));
	BOOL bRet;
	DWORD dwWrited;
	SUserDataForDrive* pUserData = (SUserDataForDrive*)a_userData;

	if(pUserData->alreadyReadBytes<scnHeaderSize){
		char* pcHeader = (char*)pUserData->dch;
		int64_t nSizeToCopy = scnHeaderSize - pUserData->alreadyReadBytes;
		
		nSizeToCopy = min(nSizeToCopy,(int64_t)a_bufLen);
		memcpy(pcHeader+(size_t)pUserData->alreadyReadBytes,a_buffer,(size_t)nSizeToCopy);
		a_buffer = ((const char*)a_buffer)+ ((size_t)nSizeToCopy);
		a_bufLen -= ((int)nSizeToCopy);
		pUserData->alreadyReadBytes += nSizeToCopy;

		if(  pUserData->alreadyReadBytes >= scnHeaderSize  ){

			if (!DZlibResizeHeaderForDiscDecompression(&pUserData->dch)) {return -4;}
		}
	}  // if(pUserData->alreadyReadBytes<8){

	if (  (pUserData->alreadyReadBytes >= scnHeaderSize)&&(pUserData->alreadyReadBytes < (int64_t)pUserData->dch->wholeHeaderSizeInBytes)) {
		int64_t nSizeToCopy = (int64_t)pUserData->dch->wholeHeaderSizeInBytes - pUserData->alreadyReadBytes;
		char* pcHeader = (char*)pUserData->dch;
		
		nSizeToCopy = min(nSizeToCopy, (int64_t)a_bufLen);
		memcpy(pcHeader + (size_t)pUserData->alreadyReadBytes, a_buffer, (size_t)nSizeToCopy);
		a_buffer = ((const char*)a_buffer) + ((size_t)nSizeToCopy);
		a_bufLen -= ((int)nSizeToCopy);
		pUserData->alreadyReadBytes += nSizeToCopy;

		if (pUserData->alreadyReadBytes >= (int64_t)pUserData->dch->wholeHeaderSizeInBytes) {
			DRIVE_LAYOUT_INFORMATION_EX* pDiskInfo = DISK_INFO_FROM_ITEM(pUserData->dch);
			uint64_t EndTime;
			CREATE_DISK aDiskStr;
			DISK_GEOMETRY_EX drvGeo;
			DWORD dwReturned, dwInpLen = (DWORD)(pUserData->dch->wholeHeaderSizeInBytes - sizeof(SDiskCompDecompHeader));
			BOOL isOk ;
			//BOOL bSuccs = DeviceIoControl(pUserData->driveHandle,IOCTL_DISK_SET_DRIVE_LAYOUT_EX,DISK_INFO_FROM_ITEM(pUserData->dch),dwInpLen,NULL,0, &dwReturned, NULL);

			DeviceIoControl(pUserData->driveHandle,FSCTL_ALLOW_EXTENDED_DASD_IO,NULL,0,NULL,0,&dwReturned,NULL);

			EndTime = GetTickCount64() + DRIVE_ACCESS_TIMEOUT_NEW;
			isOk = FALSE;
			do {
				if(DeviceIoControl(pUserData->driveHandle,FSCTL_LOCK_VOLUME,NULL,0,NULL,0,&dwReturned,NULL)){isOk = TRUE;break;}
				SleepMsIntr(DRIVE_ACCESS_SLEEP_TIME);
			} while (GetTickCount64() < EndTime);
			if (!isOk) {
				int nReturnLoc = GetLastError();
				return -nReturnLoc;
			}

			isOk = DeviceIoControl(pUserData->driveHandle,IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,NULL,0,&drvGeo,sizeof(drvGeo),&dwReturned, NULL);//_PARTITION_INFORMATION
			if (!isOk) { return -1; }

			aDiskStr.PartitionStyle = (PARTITION_STYLE)DISK_INFO_FROM_ITEM(pUserData->dch)->PartitionStyle;
			aDiskStr.Gpt.DiskId = DISK_INFO_FROM_ITEM(pUserData->dch)->Gpt.DiskId;
			aDiskStr.Gpt.MaxPartitionCount = DISK_INFO_FROM_ITEM(pUserData->dch)->Gpt.MaxPartitionCount;

#ifdef _TEST_BY_RUFUS
			SelectedDrive.DiskSize = drvGeo.DiskSize.QuadPart;
			SelectedDrive.DeviceNumber = 1;
			SelectedDrive.SectorsPerTrack = drvGeo.Geometry.SectorsPerTrack;
			SelectedDrive.SectorSize = drvGeo.Geometry.SectorsPerTrack;
			SelectedDrive.FirstDataSector = 0; // DK todo:
			SelectedDrive.MediaType = drvGeo.Geometry.MediaType;
			CreatePartition(pUserData->driveHandle, aDiskStr.PartitionStyle, FS_FAT32, 0, 0);
#endif

			isOk = DeviceIoControl(pUserData->driveHandle, IOCTL_DISK_CREATE_DISK, &aDiskStr, sizeof(aDiskStr), NULL, 0, &dwReturned, NULL);
			if (!isOk) {
				int nReturnLoc=GetLastError();
				return -nReturnLoc; 
			}

			isOk = DeviceIoControl(pUserData->driveHandle,IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &dwReturned, NULL);
			if (!isOk) {
				int nReturnLoc=GetLastError();
				return -nReturnLoc; 
			}

			isOk = DeviceIoControl(pUserData->driveHandle,IOCTL_DISK_SET_DRIVE_LAYOUT_EX, pDiskInfo,dwInpLen,NULL,0, &dwReturned, NULL);
			if (!isOk) {
				int nReturnLoc = GetLastError();
				return -nReturnLoc;
			}

			isOk = DeviceIoControl(pUserData->driveHandle, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &dwReturned, NULL);
			if (!isOk) {
				int nReturnLoc = GetLastError();
				return -nReturnLoc;
			}
			SortDiscCompressDecompressHeader(pUserData->dch);
		}
	}  // if (  (pUserData->alreadyReadBytes >= scnHeaderSize)&&(pUserData->alreadyReadBytes < (int64_t)pUserData->dch->wholeHeaderSizeInBytes)) {

	bRet=WriteFile(pUserData->driveHandle,a_buffer,a_bufLen,&dwWrited,NULL);
	pUserData->alreadyReadBytes += a_bufLen;
	return bRet?0:-1;
}



/****************************************************************************************************************************************/
/****** commpon part *********************************************************************************************************************/
/****************************************************************************************************************************************/
SDiskCompDecompHeader* DZlibCreateHeaderForDiscCompression(uint64_t a_diskSize, uint32_t a_diskInfoSize, const DRIVE_LAYOUT_INFORMATION_EX* a_driveInfo)
{
	uint16_t unWholeHeaderSizeInBytes = (uint16_t)a_diskInfoSize+sizeof(SDiskCompDecompHeader);
	SDiskCompDecompHeader* pDiskCompDecompHeader = (SDiskCompDecompHeader*)malloc(unWholeHeaderSizeInBytes);

	if (!pDiskCompDecompHeader) { return pDiskCompDecompHeader; }

	//pDiskCompDecompHeader->version = ZLIB_COMPR_DECOMPR_VERSION;
	pDiskCompDecompHeader->version = 1;
	pDiskCompDecompHeader->wholeHeaderSizeInBytes = unWholeHeaderSizeInBytes;
	pDiskCompDecompHeader->isAnyPartAvalible = 0;
	pDiskCompDecompHeader->diskSize = a_diskSize;

	if(a_diskInfoSize){memcpy(DISK_INFO_FROM_ITEM(pDiskCompDecompHeader),a_driveInfo, a_diskInfoSize);}
	return pDiskCompDecompHeader;
}


SDiskCompDecompHeader* DZlibResizeHeaderForDiscDecompression(SDiskCompDecompHeader** a_initial)
{
	SDiskCompDecompHeader* pDiskCompDecompHeader = (SDiskCompDecompHeader*)realloc(*a_initial, (*a_initial)->wholeHeaderSizeInBytes);
	if(pDiskCompDecompHeader){*a_initial= pDiskCompDecompHeader;}
	return pDiskCompDecompHeader;
}


static int CompareStat(const void *a_arg1, const void *a_arg2)
{
	return (int)(   ((PARTITION_INFORMATION_EX*)a_arg1)->StartingOffset.QuadPart - ((PARTITION_INFORMATION_EX*)a_arg2)->StartingOffset.QuadPart   );
}


void SortDiscCompressDecompressHeader(SDiskCompDecompHeader* a_header)
{
	DRIVE_LAYOUT_INFORMATION_EX* pInfo = DISK_INFO_FROM_ITEM(a_header);
		
	if(pInfo->PartitionCount){
		PARTITION_INFORMATION_EX* pMax;
		PARTITION_INFORMATION_EX* pPartitions = pInfo->PartitionEntry;
		int64_t llnMax, llnNew;
		DWORD i;

		qsort(pPartitions, pInfo->PartitionCount, sizeof(PARTITION_INFORMATION_EX), &CompareStat);
		
		a_header->isAnyPartAvalible = 0;
		pMax = NULL;
		llnMax = MINIMUM_DISK_SIZE_TO_COMPRESS;
		
		for (i = 0; i < pInfo->PartitionCount;++i){
			llnNew = pPartitions[i].StartingOffset.QuadPart + pPartitions[i].PartitionLength.QuadPart;
			if(llnNew>llnMax){
				if(pMax){pMax->RewritePartition = 0;}
				else{a_header->isAnyPartAvalible=1;}
				pPartitions[i].RewritePartition = 1;
				pMax = &pPartitions[i];
				llnMax = llnNew;
			}
			else{pPartitions[i].RewritePartition = 0;}
		}
	}

}


void DestroyHeaderForDiscCompression(SDiskCompDecompHeader* a_header)
{
	free(a_header);
}


/****************************************************************************************************************************************/
/****** commpon part *********************************************************************************************************************/
/****************************************************************************************************************************************/

END_C_DECLS2
