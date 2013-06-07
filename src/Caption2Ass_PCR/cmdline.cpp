//------------------------------------------------------------------------------
// cmdline.cpp
//------------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

VOID _tMyPrintf(IN  LPCTSTR tracemsg, ...);
enum {
    FORMAT_SRT  = 1,
    FORMAT_ASS  = 2,
    FORMAT_TAW  = 3,
    FORMAT_DUAL = 4
};

enum {
    HLC_kigou = 1,
    HLC_box   = 2,
    HLC_draw  = 3
};

extern DWORD format;
extern TCHAR *pFileName;
extern TCHAR *pTargetFileName;
extern USHORT PMTPid;

extern long delayTime;
extern BOOL bLogMode;
extern BOOL bsrtornament;
extern BOOL bnorubi;
extern TCHAR *passType;
extern DWORD detectLength;
extern BYTE HLCmode;

BOOL ParseCmd(int argc, char **argv)
{
    if (argc < 2) {
ERROR_PARAM:
        _tMyPrintf(_T("Caption2Ass.exe [OPTIONS] source.ts [target filename]\r\n\r\n"));
        _tMyPrintf(_T("-PMT_PID PID  PID is HEX value. Ex: -PMT_PID 1f2\r\n"));
        _tMyPrintf(_T("-format {srt|ass|taw|dual}. Ex: -format srt\r\n"));
        _tMyPrintf(_T("-delay TIME   TIME is mili-sec. Ex: -delay 500\r\n"));
        _tMyPrintf(_T("-asstype TYPE . Ex: -asstype Default\r\n"));
        _tMyPrintf(_T("-norubi. not-out RUBI to ass-file\r\n"));
        _tMyPrintf(_T("-srtornament. set ornament to srt-file\r\n"));
        _tMyPrintf(_T("-hlc {kigou|box|draw}. Ex: -hlc kigou\r\n"));
        _tMyPrintf(_T("-log. make log-file\r\n"));
        _tMyPrintf(_T("-detect_length LENGTH. Ex: -detect_length 100\r\n"));

        return FALSE;
    }
    _tcscpy_s(passType, 256, _T("Default"));
    bLogMode = FALSE;
    bsrtornament = FALSE;
    bnorubi = FALSE;
    for (int i = 1; i< argc; i++) {
        if (_tcsicmp(argv[i], _T("-PMT_PID")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;

            if (_stscanf_s(argv[i], _T("%x"), &PMTPid) <= 0)
                goto ERROR_PARAM;

            continue;
        } else if (_tcsicmp(argv[i], _T("-format")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;

            if (_tcsicmp(argv[i], _T("srt")) == 0)
                format = FORMAT_SRT;
            else if (_tcsicmp(argv[i], _T("ass")) == 0)
                format = FORMAT_ASS;
            else if (_tcsicmp(argv[i], _T("taw")) == 0)
                format = FORMAT_TAW;
            else if (_tcsicmp(argv[i], _T("dual")) == 0)
                format = FORMAT_DUAL;
            else
                goto ERROR_PARAM;

            continue;
        } else if (_tcsicmp(argv[i], _T("-i")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;

            pFileName = argv[i];
            continue;
        } else if (_tcsicmp(argv[i], _T("-o")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;

            pTargetFileName = argv[i];
            continue;
        } else if (_tcsicmp(argv[i], _T("-delay")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;

            if (_stscanf_s(argv[i], _T("%d"), &delayTime) <= 0)
                goto ERROR_PARAM;

            continue;
        } else if (_tcsicmp(argv[i], _T("-asstype")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;

            passType = argv[i];
            continue;
        } else if (_tcsicmp(argv[i], _T("-log")) == 0) {
            bLogMode = TRUE;
            continue;
        } else if (_tcsicmp(argv[i], _T("-srtornament")) == 0) {
            bsrtornament = TRUE;
            continue;
        } else if (_tcsicmp(argv[i], _T("-hlc")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;

            if (_tcsicmp(argv[i], _T("kigou")) == 0)
                HLCmode = HLC_kigou;
            else if (_tcsicmp(argv[i], _T("box")) == 0)
                HLCmode = HLC_box;
            else if (_tcsicmp(argv[i], _T("draw")) == 0)
                HLCmode = HLC_draw;
            else
                goto ERROR_PARAM;

            continue;
        } else if (_tcsicmp(argv[i], _T("-norubi")) == 0) {
            bnorubi = TRUE;
            continue;
        } else if(_tcsicmp(argv[i], _T("-detect_length")) == 0) {
            i++;
            if(i > argc)
                goto ERROR_PARAM;

            if(_stscanf_s(argv[i], _T("%d"), &detectLength) < 0)
                goto ERROR_PARAM;
            detectLength *= 10000;
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
    IN  LPCTSTR tracemsg,
    ...
    )
{
    TCHAR buf[MAX_PATH + 2048] = {0};
    HRESULT ret;

    __try {
        va_list ptr;
        va_start(ptr,tracemsg);

        ret = StringCchVPrintf(
            buf,
            MAX_PATH + 2048,
            tracemsg,
            ptr
            );

        if (ret == S_OK) {
            DWORD ws;
            WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buf, (DWORD)_tcslen(buf), &ws, NULL);

        }
    }
    __finally {
    }

    return;
}
