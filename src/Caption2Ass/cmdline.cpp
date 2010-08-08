
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>



VOID _tMyPrintf(IN	LPCTSTR tracemsg, ...);
enum {
	FORMAT_SRT = 1,
	FORMAT_ASS = 2
};

extern DWORD format;
extern TCHAR *pFileName;
extern TCHAR *pTargetFileName;
extern USHORT PMTPid;

BOOL ParseCmd(int argc, char **argv)
{
	if (argc < 2) {
ERROR_PARAM:
		_tMyPrintf(_T("Caption2Ass.exe [OPTIONS] source.ts [target filename]\r\n\r\n"));
		_tMyPrintf(_T("-PMT_PID PID  PID is HEX value. Ex: -PMT_PID 1f2\r\n"));
		_tMyPrintf(_T("-format {srt|ass}. Ex: -format srt\r\n"));

		return FALSE;
	}
	for (int i = 1; i< argc; i++) {
		if (_tcsicmp(argv[i], _T("-PMT_PID")) == 0) {
			i++;
			if (i > argc)
				goto ERROR_PARAM;

			if (_stscanf_s(argv[i], _T("%x"), &PMTPid) <= 0)
				goto ERROR_PARAM;

			continue;
		}
		else if (_tcsicmp(argv[i], _T("-format")) == 0) {
			i++;
			if (i > argc)
				goto ERROR_PARAM;

			if (_tcsicmp(argv[i], _T("srt")) == 0) {
				format = FORMAT_SRT;
			}
			else if (_tcsicmp(argv[i], _T("ass")) == 0) {
				format = FORMAT_ASS;
			}
			else
				goto ERROR_PARAM;

			continue;
		}

		if (!pFileName) {
			pFileName = argv[i];
			continue;
		}

		if (!pTargetFileName) {
			pTargetFileName = argv[i];
			continue;
		}
	}
	return TRUE;
}


VOID _tMyPrintf(
	IN	LPCTSTR tracemsg,
	...
	)
{
	TCHAR buf[1024] = {0};
	HRESULT ret;

	__try {
		va_list ptr;
		va_start(ptr,tracemsg);

		ret = StringCchVPrintf(
			buf,
			2048,
			tracemsg,
			ptr
			);

		if (ret == S_OK) {
			DWORD ws;
			WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buf, _tcsclen(buf), &ws, NULL);

		}
	}
	__finally {
	}

	return;
}
