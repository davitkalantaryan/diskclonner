// 
// file:		disk_clone_by_zipping_and_partition_using_zip.h
// created on:	2019 Jan 09
//

#ifndef __disk_clone_by_zipping_and_partition_using_zip_h__
#define __disk_clone_by_zipping_and_partition_using_zip_h__

#include <common_disk_clonner_project_include.h>

BEGIN_C_DECLS2

COMP_DECOMP_API int CompressDiskToPathOrUrl(const char* a_driveName, const char* a_cpcPathOrUrl, int a_nCompressionLeel);
COMP_DECOMP_API int DecompressFromPathOrUrlAndPrepareDisk(const char* a_driveName, const char* a_cpcPathOrUrl);

END_C_DECLS2


#endif  // #ifndef __disk_clone_by_zipping_and_partition_using_zip_h__
