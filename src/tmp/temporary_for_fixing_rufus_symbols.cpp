// 
// file:		temporary_for_fixing_rufus_symbols.cpp 
// created on:	2019 Jan 09 
//

#ifndef UNICODE
#define UNICODE
#endif

#include <common_disk_clonner_project_include.h>
#include <stdio.h>
#include <stdarg.h>
//#include <rufus.h>
#include <stdint.h>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Ole32.lib")
#pragma warning(disable:4996)

BEGIN_C_DECLS2

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#endif

#ifndef MSG_LEN
#define MSG_LEN 1024
#endif

#ifndef WRITE_TIMEOUT
#define WRITE_TIMEOUT 100
#endif

#ifndef FAC
#define FAC(f)                         (f<<16)
#endif

#ifndef MAX_LIBRARY_HANDLES
#define         MAX_LIBRARY_HANDLES 32
#endif

int bt;
DWORD FormatStatus = 0;
HMODULE  OpenedLibrariesHandle[MAX_LIBRARY_HANDLES];
uint16_t OpenedLibrariesHandleSize=0;

void _uprintf(const char *a_format, ...)
{
	va_list argList;
	va_start(argList, a_format);
	vprintf(a_format, argList);
	va_end(argList);
}

#ifndef uprintf
#define uprintf(...) _uprintf(__VA_ARGS__)
#endif


const char *WindowsErrorString(void)
{
	static char vcErrorBuffer[1024];
#ifdef _WIN32
	DWORD dwChars, dwErr=GetLastError();

	dwChars = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErr,
		0,
		vcErrorBuffer,
		1023,
		NULL);

	if(dwChars>0){vcErrorBuffer[dwChars]=0;}
	else{vcErrorBuffer[0]=0;}
#else
	vcErrorBuffer[0] = 0;
#endif
	return vcErrorBuffer;
}


void PrintStatusInfo(BOOL info, BOOL debug, unsigned int duration, int msg_id, ...)
{
#if 0
	char *format = NULL, buf[MSG_LEN];
	char *msg_hi = szMessage[info?MSG_INFO:MSG_STATUS][MSG_HIGH_PRI];
	char *msg_lo = szMessage[info?MSG_INFO:MSG_STATUS][MSG_LOW_PRI];
	char *msg_cur = (duration > 0)?msg_lo:msg_hi;
	va_list args;

	if (msg_id < 0) {
		// A negative msg_id clears the message
		msg_hi[0] = 0;
		OutputMessage(info, msg_hi);
		return;
	}

	if ((msg_id < MSG_000) || (msg_id >= MSG_MAX)) {
		uprintf("PrintStatusInfo: invalid MSG_ID\n");
		return;
	}

	// We need to keep track of where szStatusMessage should point to so that ellipses work
	if (!info)
		szStatusMessage = szMessage[MSG_STATUS][(duration > 0)?MSG_LOW_PRI:MSG_HIGH_PRI];

	if ((msg_id >= MSG_000) && (msg_id < MSG_MAX))
		format = msg_table[msg_id - MSG_000];
	if (format == NULL) {
		safe_sprintf(msg_hi, MSG_LEN, "MSG_%03d UNTRANSLATED", msg_id - MSG_000);
		uprintf(msg_hi);
		OutputMessage(info, msg_hi);
		return;
	}

	va_start(args, msg_id);
	safe_vsnprintf(msg_cur, MSG_LEN, format, args);
	va_end(args);
	msg_cur[MSG_LEN-1] = '\0';

	if ((duration != 0) || (!bStatusTimerArmed))
		OutputMessage(info, msg_cur);

	if (duration != 0) {
		SetTimer(hMainDialog, (info)?TID_MESSAGE_INFO:TID_MESSAGE_STATUS, duration, PrintMessageTimeout);
		bStatusTimerArmed = TRUE;
	}

	// Because we want the log messages in English, we go through the VA business once more, but this time with default_msg_table
	if (debug) {
		if ((msg_id >= MSG_000) && (msg_id < MSG_MAX))
			format = default_msg_table[msg_id - MSG_000];
		if (format == NULL) {
			safe_sprintf(buf, sizeof(szStatusMessage), "(default) MSG_%03d UNTRANSLATED", msg_id - MSG_000);
			return;
		}
		va_start(args, msg_id);
		safe_vsnprintf(buf, MSG_LEN, format, args);
		va_end(args);
		buf[MSG_LEN-1] = '\0';
		uprintf(buf);
	}
#endif
}


char* SizeToHumanReadable(uint64_t a_size, BOOL copy_to_log, BOOL fake_units)
{
	static char	str_size[1024];

	_snprintf(str_size,1023,"%I64u",a_size);
	return str_size;
}


unsigned char* GetResource(HMODULE module, char* name, char* type, const char* desc, DWORD* len, BOOL duplicate)
{
	HGLOBAL res_handle;
	HRSRC res;
	unsigned char* p = NULL;

	res = FindResourceA(module, name, type);
	if (res == NULL) {
		uprintf("Could not locate resource '%s': %s\n", desc, WindowsErrorString());
		goto out;
	}
	res_handle = LoadResource(module, res);
	if (res_handle == NULL) {
		uprintf("Could not load resource '%s': %s\n", desc, WindowsErrorString());
		goto out;
	}
	*len = SizeofResource(module, res);

	if (duplicate) {
		p = (unsigned char*)malloc(*len);
		if (p == NULL) {
			uprintf("Coult not allocate resource '%s'\n", desc);
			goto out;
		}
		memcpy(p, LockResource(res_handle), *len);
	}
	else {
		p = (unsigned char*)LockResource(res_handle);
	}

out:
	return p;
}


DWORD GetResourceSize(HMODULE module, char* name, char* type, const char* desc)
{
	DWORD len = 0;
	return (GetResource(module, name, type, desc, &len, FALSE) == NULL) ? 0 : len;
}


/*
 * Parse a file (ANSI or UTF-8 or UTF-16) and return the data for the 'index'th occurrence of 'token'
 * The returned string is UTF-8 and MUST be freed by the caller
 */
char* get_token_data_file_indexed(const char* token, const char* filename, int index)
{
	return NULL;
#if 0
	int i = 0;
	wchar_t *wtoken = NULL, *wdata= NULL, *wfilename = NULL;
	wchar_t buf[1024];
	FILE* fd = NULL;
	char *ret = NULL;

	if ((filename == NULL) || (token == NULL))
		return NULL;
	if ((filename[0] == 0) || (token[0] == 0))
		return NULL;

	wfilename = utf8_to_wchar(filename);
	if (wfilename == NULL) {
		uprintf(conversion_error, filename);
		goto out;
	}
	wtoken = utf8_to_wchar(token);
	if (wfilename == NULL) {
		uprintf(conversion_error, token);
		goto out;
	}
	fd = _wfopen(wfilename, L"r, ccs=UNICODE");
	if (fd == NULL) goto out;

	// Process individual lines. NUL is always appended.
	// Ideally, we'd check that our buffer fits the line
	while (fgetws(buf, ARRAYSIZE(buf), fd) != NULL) {
		wdata = get_token_data_line(wtoken, buf);
		if ((wdata != NULL) && (++i == index)) {
			ret = wchar_to_utf8(wdata);
			break;
		}
	}

out:
	if (fd != NULL)
		fclose(fd);
	safe_free(wfilename);
	safe_free(wtoken);
	return ret;
#endif
}

BOOL large_drive = FALSE;


// A WriteFile() equivalent, with up to nNumRetries write attempts on error.
BOOL WriteFileWithRetry(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten, DWORD nNumRetries)
{
	DWORD nTry;
	BOOL readFilePointer;
	LARGE_INTEGER liFilePointer, liZero = { { 0,0 } };

	// Need to get the current file pointer in case we need to retry
	readFilePointer = SetFilePointerEx(hFile, liZero, &liFilePointer, FILE_CURRENT);
	if (!readFilePointer)
		uprintf("Warning: Could not read file pointer: %s", WindowsErrorString());

	if (nNumRetries == 0)
		nNumRetries = 1;
	for (nTry = 1; nTry <= nNumRetries; nTry++) {
		// Need to rewind our file position on retry - if we can't even do that, just give up
		if ((nTry > 1) && (!SetFilePointerEx(hFile, liFilePointer, NULL, FILE_BEGIN))) {
			uprintf("Could not set file pointer - Aborting");
			break;
		}
		if (WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL)) {
			if (nNumberOfBytesToWrite == *lpNumberOfBytesWritten)
				return TRUE;
			// Some large drives return 0, even though all the data was written - See github #787 */
			if (large_drive && (*lpNumberOfBytesWritten == 0)) {
				uprintf("Warning: Possible short write");
				return TRUE;
			}
			uprintf("Wrote %d bytes but requested %d", *lpNumberOfBytesWritten, nNumberOfBytesToWrite);
		} else {
			uprintf("Write error [0x%08X]", GetLastError());
		}
		// If we can't reposition for the next run, just abort
		if (!readFilePointer)
			break;
		if (nTry < nNumRetries) {
			uprintf("Retrying in %d seconds...", WRITE_TIMEOUT / 1000);
			Sleep(WRITE_TIMEOUT);
		}
	}
	if (SCODE_CODE(GetLastError()) == ERROR_SUCCESS)
		SetLastError(ERROR_SEVERITY_ERROR|FAC(FACILITY_STORAGE)|ERROR_WRITE_FAULT);
	return FALSE;
}


static BYTE access_mask;

/**
 * Search all the processes and list the ones that have a specific handle open.
 *
 * \param HandleName The name of the handle to look for.
 * \param dwTimeOut The maximum amounf of time (ms) that may be spent searching
 * \param bPartialMatch Whether partial matches should be allowed.
 * \param bIgnoreSelf Whether the current process should be listed.
 * \param bQuiet Prints minimal output.
 *
 * \return a byte containing the cummulated access rights (f----xwr) from all the handles found
 *         with bit 7 ('f') also set if at least one process was found.
 */
BYTE SearchProcess(char* HandleName, DWORD dwTimeOut, BOOL bPartialMatch, BOOL bIgnoreSelf, BOOL bQuiet)
{
	HANDLE handle=0;
	DWORD res = 0;

	char* _HandleName = HandleName;
	BOOL _bPartialMatch = bPartialMatch;
	BOOL _bIgnoreSelf = bIgnoreSelf;
	BOOL _bQuiet = bQuiet;
	access_mask = 0;

#ifdef SearchProcess_implement

	handle = CreateThread(NULL, 0, SearchProcessThread, NULL, 0, NULL);
	if (handle == NULL) {
		uprintf("Warning: Unable to create conflicting process search thread");
		return 0x00;
	}
	res = WaitForSingleObjectWithMessages(handle, dwTimeOut);
	if (res == WAIT_TIMEOUT) {
		// Timeout - kill the thread
		TerminateThread(handle, 0);
		uprintf("Warning: Conflicting process search failed to complete due to timeout");
	} else if (res != WAIT_OBJECT_0) {
		TerminateThread(handle, 0);
		uprintf("Warning: Failed to wait for conflicting process search thread: %s", WindowsErrorString());
	}
#else
#endif
	return access_mask;
}


END_C_DECLS2
