// 
// file:		common_disk_clonner_project_include.h
// created on:	2019 Jan 09
// 

#ifndef __common_disk_clonner_project_include_h__
#define __common_disk_clonner_project_include_h__

// these should be removed from here
#define COMP_DECOMP_API
#define COMP_DECOMP_VAR

#ifdef __cplusplus
#define NULLPTR2	nullptr
#define STATIC_CAST2(_variable,_typeToCast)	( static_cast<_typeToCast>(_variable) )
#ifndef KEEP_C_NOMINMAX
#define NOMINMAX   // use std::min   / std::max
#endif
#define EXTERN_C2		extern "C"
#define BEGIN_C_DECLS2	extern "C"{
#define END_C_DECLS2	}
#else		// #ifdef __cplusplus
#define NULLPTR2	NULL
#define STATIC_CAST2(_variable,_typeToCast)	( (_typeToCast)(_variable) )
#define EXTERN_C2 
#define BEGIN_C_DECLS2  
#define END_C_DECLS2 
#endif		// #ifdef __cplusplus

#ifdef _MSC_VER
#define INCLUDE_LIB(__libName)			__pragma (comment (lib,__libName))
#define DISABLE_WARNING(__warningCode)	__pragma (warning (disable:__warningCode))
#define DLL_PUBLIC2		__declspec(dllexport)
#define DLL_PRIVATE2 
#define DLL_IMPORT2		__declspec(dllimport)
#elif defined(__CYGWIN__)
#define INCLUDE_LIB(__libName) 
#define DISABLE_WARNING(__warningCode)
#define DLL_PUBLIC2		__attribute__(dllexport)
#define DLL_PRIVATE2 
#define DLL_IMPORT2 
#elif defined(__GNUC__) || defined(__clang__)
#define INCLUDE_LIB(__libName) 
#define DISABLE_WARNING(__warningCode)
#define DLL_PUBLIC2		__attribute__((visibility("default")))
#define DLL_PRIVATE2	__attribute__((visibility("hidden")))
#define DLL_IMPORT2 
#elif defined(__MINGW64__) || defined(__MINGW32__)
#define INCLUDE_LIB(__libName) 
#define DISABLE_WARNING(__warningCode)
#define DLL_PUBLIC2		__declspec(dllexport)
#define DLL_PRIVATE2 
#define DLL_IMPORT2		__declspec(dllimport)
#elif defined(__SUNPRO_CC)
#define INCLUDE_LIB(__libName) 
#define DISABLE_WARNING(__warningCode)
#define DLL_PUBLIC2 
#define DLL_PRIVATE2	__hidden
#define DLL_IMPORT2	
#else
// no guarantii for other compilers
#define INCLUDE_LIB(__libName) 
#define DISABLE_WARNING(__warningCode)
#define DLL_PUBLIC2 
#define DLL_PRIVATE2 
#define DLL_IMPORT2	
#endif

// corresponding includes during using these macroses should be included
#ifdef _WIN32
#define SleepMs(_timeMs)		Sleep((_timeMs))
#define SleepMsIntr(_timeMs)	SleepEx((_timeMs),TRUE)
#else
#define SleepMs(_timeMs)		do{sleep((_timeMs)/1000);usleep((_timeMs)%1000);}while(0)  // here nanosleep should be used
#define SleepMsIntr(_timeMs)	do{sleep((_timeMs)/1000);usleep((_timeMs)%1000);}while(0)
#endif


#endif  // #ifndef __common_disk_clonner_project_include_h__
