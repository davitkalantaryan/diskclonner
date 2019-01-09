// 
// file:		private_header_for_compress_decompress.h 
// created on:	2019 Jan 09
//

#ifndef __private_header_for_compress_decompress_h__
#define __private_header_for_compress_decompress_h__

#include <common_disk_clonner_project_include.h>
#include <stdint.h>
#ifdef _WIN32
#include <WinInet.h>
#else
#endif
#include <stdio.h>

#define MINIMUM_DISK_SIZE_TO_COMPRESS	1024

BEGIN_C_DECLS2

// return 0, continues, non 0 stops
#ifndef typeCompressDecompressClbk_defined
typedef int(*typeCompressDecompressClbk)(const void*buffer, int bufLen, void*userData);
#define typeCompressDecompressClbk_defined
#endif

typedef struct SDiskCompDecompHeader
{
	uint32_t version;
	uint16_t wholeHeaderSizeInBytes;
	uint16_t isAnyPartAvalible;
	int64_t  diskSize;
	//uint64_t driveLayoutStrPointer;
}SDiskCompDecompHeader;

typedef struct SFileOrNetResource {
	uint64_t isUrl : 1;
	union {
		FILE*		file;
		struct { void *d1, *d2; }d;
		struct { HINTERNET session, url; }i;
	}h;
}SFileOrNetResource;

SDiskCompDecompHeader* DZlibCreateHeaderForDiscCompression(uint64_t diskSize, uint32_t diskInfoSize, const DRIVE_LAYOUT_INFORMATION_EX* driveInfo);
SDiskCompDecompHeader* DZlibResizeHeaderForDiscDecompression(SDiskCompDecompHeader** base);
void SortDiscCompressDecompressHeader(SDiskCompDecompHeader* header);
void DestroyHeaderForDiscCompression(SDiskCompDecompHeader* header);

#define DISK_INFO_FROM_ITEM(_item)   (  (DRIVE_LAYOUT_INFORMATION_EX*)( ((char*)(_item))+sizeof(SDiskCompDecompHeader)  )  )


END_C_DECLS2

#endif  // #ifndef __private_header_for_compress_decompress_h__
