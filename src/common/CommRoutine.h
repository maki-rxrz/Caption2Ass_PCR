#ifndef _COMM_ROUTINE_
#define _COMM_ROUTINE_

#include <windows.h>
#include <tchar.h>
#include <string>

#define MAX_DEBUG_OUTPUT_LENGTH		2048

#define DBG_PREFIX		_T("TC!") _T(__FUNCTION__) _T(":")

#ifdef _DEBUG
VOID
DbgString(
	IN	LPCTSTR tracemsg,
	...
	);
#else

#define DbgString(x,...)

#endif

std::string GetHalfChar(std::string key);
#endif // #ifndef _COMM_ROUTINE_