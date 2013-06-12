//------------------------------------------------------------------------------
// cmdline.cpp
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <strsafe.h>

#include "cmdline.h"
#include "CaptionDef.h"

static void usage(void)
{
    _tMyPrintf(_T("Caption2Ass_PCR.exe [OPTIONS] source.ts [target filename]\r\n\r\n"));
    _tMyPrintf(_T("-PMT_PID PID  PID is HEX value. Ex: -PMT_PID 1f2\r\n"));
    _tMyPrintf(_T("-format {srt|ass|taw|dual}. Ex: -format srt\r\n"));
    _tMyPrintf(_T("-delay TIME   TIME is mili-sec. Ex: -delay 500\r\n"));
    _tMyPrintf(_T("-asstype TYPE . Ex: -asstype Default\r\n"));
    _tMyPrintf(_T("-norubi. not-out RUBI to ass-file\r\n"));
    _tMyPrintf(_T("-srtornament. set ornament to srt-file\r\n"));
    _tMyPrintf(_T("-hlc {kigou|box|draw}. Ex: -hlc kigou\r\n"));
    _tMyPrintf(_T("-log. make log-file\r\n"));
    _tMyPrintf(_T("-detect_length LENGTH. Ex: -detect_length 100\r\n"));
}

extern int ParseCmd(int argc, TCHAR **argv, CCaption2AssParameter *param)
{
    if (argc < 2) {
ERROR_PARAM:
        usage();
        return -1;
    }

    // Refer parameters.
    pid_information_t *pi = param->get_pid_information();
    cli_parameter_t   *cp = param->get_cli_parameter();
    size_t string_length  = param->string_length;

    // Set up the default value.
    cp->detectLength = 300 * 10000;
    _tcscpy_s(cp->ass_type, string_length, _T("Default"));

    // Parse args.
    for (int i = 1; i< argc; i++) {
        if (_tcsicmp(argv[i], _T("-PMT_PID")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            if (_stscanf_s(argv[i], _T("%x"), &(pi->PMTPid)) <= 0)
                goto ERROR_PARAM;
        } else if (_tcsicmp(argv[i], _T("-format")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            if (_tcsicmp(argv[i], _T("srt")) == 0)
                cp->format = FORMAT_SRT;
            else if (_tcsicmp(argv[i], _T("ass")) == 0)
                cp->format = FORMAT_ASS;
            else if (_tcsicmp(argv[i], _T("taw")) == 0)
                cp->format = FORMAT_TAW;
            else if (_tcsicmp(argv[i], _T("dual")) == 0)
                cp->format = FORMAT_DUAL;
            else
                goto ERROR_PARAM;
        } else if (_tcsicmp(argv[i], _T("-i")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            _tcscpy_s(cp->FileName, string_length, argv[i]);
            
        } else if (_tcsicmp(argv[i], _T("-o")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            _tcscpy_s(cp->TargetFileName1, string_length, argv[i]);
        } else if (_tcsicmp(argv[i], _T("-delay")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            if (_stscanf_s(argv[i], _T("%d"), &(cp->DelayTime)) <= 0)
                goto ERROR_PARAM;
        } else if (_tcsicmp(argv[i], _T("-asstype")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            _tcscpy_s(cp->ass_type, string_length, argv[i]);
        } else if (_tcsicmp(argv[i], _T("-log")) == 0) {
            cp->LogMode = TRUE;
        } else if (_tcsicmp(argv[i], _T("-srtornament")) == 0) {
            cp->srtornament = TRUE;
        } else if (_tcsicmp(argv[i], _T("-hlc")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            if (_tcsicmp(argv[i], _T("kigou")) == 0)
                cp->HLCmode = HLC_kigou;
            else if (_tcsicmp(argv[i], _T("box")) == 0)
                cp->HLCmode = HLC_box;
            else if (_tcsicmp(argv[i], _T("draw")) == 0)
                cp->HLCmode = HLC_draw;
            else
                goto ERROR_PARAM;
        } else if (_tcsicmp(argv[i], _T("-norubi")) == 0) {
            cp->norubi = TRUE;
        } else if (_tcsicmp(argv[i], _T("-detect_length")) == 0) {
            i++;
            if (i > argc)
                goto ERROR_PARAM;
            if (_stscanf_s(argv[i], _T("%d"), &cp->detectLength) < 0)
                goto ERROR_PARAM;
            cp->detectLength *= 10000;
        } else if (_tcsicmp(cp->FileName, _T("")) == 0) {
            _tcscpy_s(cp->FileName, string_length, argv[i]);
        } else if (_tcsicmp(cp->TargetFileName1, _T("")) == 0) {
            _tcscpy_s(cp->TargetFileName1, string_length, argv[i]);
        } else if (_tcsicmp(cp->TargetFileName2, _T("")) == 0) {
            _tcscpy_s(cp->TargetFileName2, string_length, argv[i]);
        }
    }
    return 0;
}

extern void _tMyPrintf(IN  LPCTSTR tracemsg, ...)
{
    TCHAR buf[MAX_PATH + 2048] = { 0 };
    HRESULT ret;

    __try {
        va_list ptr;
        va_start(ptr, tracemsg);

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
