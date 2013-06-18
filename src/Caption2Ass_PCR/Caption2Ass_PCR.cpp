//------------------------------------------------------------------------------
// Caption2Ass_PCR.cpp : Defines the entry point for the console application.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <shlwapi.h>
#include <vector>
#include <list>

#include "CommRoutine.h"
#include "CaptionDllUtil.h"
#include "cmdline.h"
#include "tslutil.h"
#include "Caption2Ass_PCR.h"

#define C2A_SUCCESS         0
#define C2A_FAILURE         1
#define C2A_ERR_DLL         2
#define C2A_ERR_PARAM       3
#define C2A_ERR_MEMORY      4

#define WRAP_AROUND_VALUE           (1LL << 33)
#define WRAP_AROUND_CHECK_VALUE     ((1LL << 32) - 1)
#define PCR_MAXIMUM_INTERVAL        (100)

typedef struct _ASS_COLOR {
    unsigned char   ucR;
    unsigned char   ucG;
    unsigned char   ucB;
    unsigned char   ucAlpha;
} ASS_COLOR;

typedef struct _CAPTION_LINE {
    UINT            index;
    DWORD           startTime;
    DWORD           endTime;
    BYTE            outCharSizeMode;
    ASS_COLOR       outCharColor;
    BOOL            outUnderLine;
    BOOL            outShadow;
    BOOL            outBold;
    BOOL            outItalic;
    BYTE            outFlushMode;
    BYTE            outHLC;     //must ignore low 4bits
    WORD            outCharW;
    WORD            outCharH;
    WORD            outCharHInterval;
    WORD            outCharVInterval;
    WORD            outPosX;
    WORD            outPosY;
    std::string     str;
} CAPTION_LINE, *PCAPTION_LINE;

typedef std::list<PCAPTION_LINE> CAPTION_LIST;

typedef struct {
    // Output handlers
    DWORD       assIndex;       // index for ASS
    DWORD       srtIndex;       // index for SRT
    BOOL        norubi;
    BOOL        srtornament;
    int         sidebar_size;
    BOOL        bUnicode;
    BOOL        bCreateOutput;
    // Timestamp handlers
    long long   startPCR;
    long long   lastPCR;
    long long   lastPTS;
    long long   basePCR;
    long long   basePTS;
    long long   correctTS;
    // File handlers
    FILE       *fpInputTs;
    FILE       *fpTarget1;
    FILE       *fpTarget2;
    FILE       *fpLogFile;
    // Parameter handlers
    CCaption2AssParameter  *param;
    // Control informations
    size_t      string_length;
} app_handler_t;

static int count_UTF8(const unsigned char *string)
{
    int len = 0;

    while (*string) {
        if (string[0] == 0x00)
            break;

        if (string[0] < 0x1f || string[0] == 0x7f) {
            // 制御コード
        } else {
            if (string[0] <= 0x7f)
                ++len; // 1バイト文字
            else if (string[0] <= 0xbf)
                ; // 文字の続き
            else if (string[0] <= 0xdf) {
                ++len; // 2バイト文字
                ++len; // 2バイト文字
                if ((string[0] == 0xc2) && (string[1] == 0xa5))
                    --len; // 2バイト文字
            } else if (string[0] <= 0xef) {
                ++len; // 3バイト文字
                ++len; // 3バイト文字
                if ((string[0] == 0xe2) && (string[1] == 0x80) && (string[2] == 0xbe))
                    --len; // 2バイト文字
                if (string[0] == 0xef) {
                    if (string[1] == 0xbd)
                        if ((string[2] >= 0xa1) && (string[2] == 0xbf))
                            --len; // 2バイト文字
                    if (string[1] == 0xbe)
                        if ((string[2] >= 0x80) && (string[2] == 0x9f))
                            --len; // 2バイト文字
                }
            } else if (string[0] <= 0xf7) {
                ++len; // 4バイト文字
                ++len; // 4バイト文字
            } else if (string[0] <= 0xfb) {
                ++len; // 5バイト文字
                ++len; // 5バイト文字
            } else if (string[0] <= 0xfd) {
                ++len; // 6バイト文字
                ++len; // 6バイト文字
            } else
                ; // 使われていない範囲
        }
        ++string;
    }

    return len;
}

#define HMS(T, h, m, s, ms)             \
do {                                    \
    ms = (int)(T) % 1000;               \
    s  = (int)((T) / 1000) % 60;        \
    m  = (int)((T) / (1000 * 60)) % 60; \
    h  = (int)((T) / (1000 * 60 * 60)); \
} while(0)

static void DumpAssLine(FILE *fp, CAPTION_LIST *list, DWORD endTime, app_handler_t *app)
{
    CAPTION_LIST::iterator it = list->begin();
    for (int i = 0; it != list->end(); it++, i++) {
        (*it)->endTime = endTime;

        unsigned short sH, sM, sS, sMs, eH, eM, eS, eMs;
        HMS((*it)->startTime, sH, sM, sS, sMs);
        HMS((*it)->endTime, eH, eM, eS, eMs);
        sMs /= 10;
        eMs /= 10;

        if (((*it)->outCharSizeMode != STR_SMALL) && ((*it)->outHLC == HLC_box)) {
            int iHankaku;
            unsigned char usTmpUTF8[STRING_BUFFER_SIZE] = { 0 };
            memcpy_s(usTmpUTF8, STRING_BUFFER_SIZE, (*it)->str.c_str(), (*it)->str.size());
            iHankaku = count_UTF8(usTmpUTF8);
            int iBoxPosX = (*it)->outPosX + (iHankaku * (((*it)->outCharW + (*it)->outCharHInterval) / 4)) - ((*it)->outCharHInterval / 4);
            int iBoxPosY = (*it)->outPosY + ((*it)->outCharVInterval / 2);
            int iBoxScaleX = (iHankaku + 1) * 50;
            int iBoxScaleY = 100 * ((*it)->outCharH + (*it)->outCharVInterval) / (*it)->outCharH;
            fprintf(fp, "Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Box,,0000,0000,0000,,{\\pos(%d,%d)\\fscx%d\\fscy%d\\3c&H%06x&}",
                    sH, sM, sS, sMs, eH, eM, eS, eMs, iBoxPosX, iBoxPosY, iBoxScaleX, iBoxScaleY, (*it)->outCharColor);
            static const unsigned char utf8box[] = { 0xE2, 0x96, 0xA0 };
            fwrite(utf8box, 3, 1, fp);
            fprintf(fp, "\r\n");
        }
        if (((*it)->outCharSizeMode != STR_SMALL) && ((*it)->outHLC == HLC_draw)) {
            int iHankaku;
            unsigned char usTmpUTF8[STRING_BUFFER_SIZE] = { 0 };
            memcpy_s(usTmpUTF8, STRING_BUFFER_SIZE, (*it)->str.c_str(), (*it)->str.size());
            iHankaku = count_UTF8(usTmpUTF8);
            int iBoxPosX = (*it)->outPosX + (iHankaku * (((*it)->outCharW + (*it)->outCharHInterval) / 4));
            int iBoxPosY = (*it)->outPosY + ((*it)->outCharVInterval / 4);
            int iBoxScaleX = iHankaku * 55;
            int iBoxScaleY = 100;   //*((*it)->outCharH + (*it)->outCharVInterval) / (*it)->outCharH;
            fprintf(fp, "Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Box,,0000,0000,0000,,{\\pos(%d,%d)\\3c&H%06x&\\p1}m 0 0 l %d 0 %d %d 0 %d{\\p0}\r\n",
                    sH, sM, sS, sMs, eH, eM, eS, eMs, iBoxPosX, iBoxPosY, (*it)->outCharColor, iBoxScaleX, iBoxScaleX, iBoxScaleY, iBoxScaleY);
        }
        if ((*it)->outCharSizeMode == STR_SMALL)
            fprintf(fp, "Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Rubi,,0000,0000,0000,,{\\pos(%d,%d)",
                    sH, sM, sS, sMs, eH, eM, eS, eMs, (*it)->outPosX, (*it)->outPosY);
        else
            fprintf(fp, "Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Default,,0000,0000,0000,,{\\pos(%d,%d)",
                    sH, sM, sS, sMs, eH, eM, eS, eMs, (*it)->outPosX, (*it)->outPosY);

        if ((*it)->outCharColor.ucR != 0xff || (*it)->outCharColor.ucG != 0xff || (*it)->outCharColor.ucB != 0xff)
            fprintf(fp, "\\c&H%06x&", (*it)->outCharColor);
        if ((*it)->outUnderLine)
            fprintf(fp, "\\u1");
        if ((*it)->outBold)
            fprintf(fp, "\\b1");
        if ((*it)->outItalic)
            fprintf(fp, "\\i1");
        fprintf(fp, "}");

        if (((*it)->outCharSizeMode == STR_SMALL) && (app->norubi)) {
            fprintf(fp, "\\N");
        } else {
            if (((*it)->outCharSizeMode != STR_SMALL) && ((*it)->outHLC == HLC_kigou))
                fprintf(fp, "[");
            fwrite((*it)->str.c_str(), (*it)->str.size(), 1, fp);
            if (((*it)->outCharSizeMode != STR_SMALL) && ((*it)->outHLC == HLC_kigou))
                fprintf(fp, "]");
            fprintf(fp, "\\N");
        }
        fprintf(fp, "\r\n");
    }

    if (list->size() > 0)
        ++(app->assIndex);
}

static void DumpSrtLine(FILE *fp, CAPTION_LIST *list, DWORD endTime, app_handler_t *app)
{
    BOOL bNoSRT = TRUE;
    CAPTION_LIST::iterator it = list->begin();
    for (int i = 0; it != list->end(); it++, i++) {

        if (i == 0) {
            (*it)->endTime = endTime;

            unsigned short sH, sM, sS, sMs, eH, eM, eS, eMs;
            HMS((*it)->startTime, sH, sM, sS, sMs);
            HMS((*it)->endTime, eH, eM, eS, eMs);

            fprintf(fp, "%d\r\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\r\n", app->srtIndex, sH, sM, sS, sMs, eH, eM, eS, eMs);
        }

        // ふりがな Skip
        if ((*it)->outCharSizeMode == STR_SMALL)
            continue;
        bNoSRT = FALSE;
        if (app->srtornament) {
            if ((*it)->outItalic)
                fprintf(fp, "<i>");
            if ((*it)->outBold)
                fprintf(fp, "<b>");
            if ((*it)->outUnderLine)
                fprintf(fp, "<u>");
            if ((*it)->outCharColor.ucR != 0xff || (*it)->outCharColor.ucG != 0xff || (*it)->outCharColor.ucB != 0xff)
                fprintf(fp, "<font color=\"#%02x%02x%02x\">", (*it)->outCharColor.ucR, (*it)->outCharColor.ucG, (*it)->outCharColor.ucB);
        }
        if ((*it)->outHLC != 0)
            fprintf(fp, "[");
        fwrite((*it)->str.c_str(), (*it)->str.size(), 1, fp);
        if ((*it)->outHLC != 0)
            fprintf(fp, "]");
        if (app->srtornament) {
            if ((*it)->outCharColor.ucR != 0xff || (*it)->outCharColor.ucG != 0xff || (*it)->outCharColor.ucB != 0xff)
                fprintf(fp, "</font>");
            if ((*it)->outUnderLine)
                fprintf(fp, "</u>");
            if ((*it)->outBold)
                fprintf(fp, "</b>");
            if ((*it)->outItalic)
                fprintf(fp, "</i>");
        }
        fprintf(fp, "\r\n");
    }

    if (list->size() > 0) {
        if (bNoSRT)
            fprintf(fp, "\r\n");
        fprintf(fp, "\r\n");
        ++(app->srtIndex);
    }
}

static void clear_caption_list(CAPTION_LIST *list)
{
    if (list->empty())
        return;
    for(std::list<PCAPTION_LINE>::iterator it = list->begin(); it != list->end(); ++it) {
        delete *it;
    }
    list->clear();
}

static int output_caption(app_handler_t *app, CCaptionDllUtil *capUtil, CAPTION_LIST *capList, long long PTS)
{
    int workCharSizeMode = 0;
    unsigned char workucB = 0;
    unsigned char workucG = 0;
    unsigned char workucR = 0;
    BOOL workUnderLine;
    BOOL workShadow;
    BOOL workBold;
    BOOL workItalic;
    BYTE workFlushMode;
    BYTE workHLC; //must ignore low 4bits
    int workCharW = 0;
    int workCharH = 0;
    int workCharHInterval = 0;
    int workCharVInterval = 0;
    int workPosX = 0;
    int workPosY = 0;
    WORD wLastSWFMode = 999;
    int offsetPosX = 0;
    int offsetPosY = 0;
    float ratioX = 2;
    float ratioY = 2;

    // Prepare the handlers.
//  pid_information_t *pi = app->param->get_pid_information();
    cli_parameter_t   *cp = app->param->get_cli_parameter();
    ass_setting_t     *as = app->param->get_ass_setting();

    // Output
    std::vector<CAPTION_DATA> Captions;
    int ret = capUtil->GetCaptionData(0, &Captions);

    std::vector<CAPTION_DATA>::iterator it = Captions.begin();
    for (; it != Captions.end(); it++) {
        CHAR strUTF8[STRING_BUFFER_SIZE] = { 0 };

        if (it->bClear) {
            // 字幕のスキップをチェック
            if ((PTS + it->dwWaitTime) <= app->startPCR) {
                _tMyPrintf(_T("%d Caption skip\r\n"), capList->size());
                if (app->fpLogFile)
                    fprintf(app->fpLogFile, "%d Caption skip\r\n", capList->size());
                clear_caption_list(capList);
                continue;
            }
            app->bCreateOutput = TRUE;
            DWORD endTime = (DWORD)((PTS + it->dwWaitTime) - app->startPCR);
            if (cp->format == FORMAT_ASS)
                DumpAssLine(app->fpTarget1, capList, endTime, app);
            else if (cp->format == FORMAT_SRT)
                DumpSrtLine(app->fpTarget1, capList, endTime, app);
            else if (cp->format == FORMAT_TAW)
                DumpSrtLine(app->fpTarget1, capList, endTime, app);
            else if (cp->format == FORMAT_DUAL) {
                DumpAssLine(app->fpTarget1, capList, endTime, app);
                DumpSrtLine(app->fpTarget2, capList, endTime, app);
            }
            clear_caption_list(capList);

            continue;
        } else {

            std::vector<CAPTION_CHAR_DATA>::iterator it2 = it->CharList.begin();

            if (app->fpLogFile) {
                fprintf(app->fpLogFile, "SWFMode    : %4d\r\n", it->wSWFMode);
                fprintf(app->fpLogFile, "Client X:Y : %4d\t%4d\r\n", it->wClientX, it->wClientY);
                fprintf(app->fpLogFile, "Client W:H : %4d\t%4d\r\n", it->wClientW, it->wClientH);
                fprintf(app->fpLogFile, "Pos    X:Y : %4d\t%4d\r\n", it->wPosX, it->wPosY);
            }

            if (it->wSWFMode != wLastSWFMode) {
                wLastSWFMode = it->wSWFMode;
                static const struct {
                    int x;
                    int y;
                } resolution[4] = {
                    {1920, 1080}, { 720,  480}, {1280,  720}, { 960,  540}
                };
                int index = (wLastSWFMode ==  5) ? 0
                          : (wLastSWFMode ==  9) ? 1
                          : (wLastSWFMode == 11) ? 2
                          :                        3;
                ratioX = (float)(as->PlayResX) / (float)(resolution[index].x);
                ratioY = (float)(as->PlayResY) / (float)(resolution[index].y);
            }
            if (app->bUnicode) {
                if ((it->wPosX < 2000) || (it->wPosY < 2000)) {
                    offsetPosX = it->wClientX;
                    offsetPosY = it->wClientY;
                } else {
                    offsetPosX = 0;
                    offsetPosY = 0;
                    it->wPosX -= 2000;
                    it->wPosY -= 2000;
                }
            }

            for (; it2 != it->CharList.end(); it2++) {
                workCharSizeMode = it2->emCharSizeMode;
                workucR = it2->stCharColor.ucR;
                workucG = it2->stCharColor.ucG;
                workucB = it2->stCharColor.ucB;
                workUnderLine = it2->bUnderLine;
                workShadow = it2->bShadow;
                workBold = it2->bBold;
                workItalic = it2->bItalic;
                workFlushMode = it2->bFlushMode;
                workHLC = (it2->bHLC != 0) ? cp->HLCmode : it2->bHLC;
                workCharW = it2->wCharW;
                workCharH = it2->wCharH;
                workCharHInterval = it2->wCharHInterval;
                workCharVInterval = it2->wCharVInterval;
                // Calculate offsetPos[X/Y].
                if (!(app->bUnicode)) {
                    int amariPosX = 0;
                    int amariPosY = 0;
                    if (wLastSWFMode == 9) {
                        amariPosX = it->wPosX % 18;
                        amariPosY = it->wPosY % 15;
                    } else {
                        amariPosX = it->wPosX % ((workCharW + workCharHInterval) / 2);
                        amariPosY = it->wPosY % ((workCharH + workCharVInterval) / 2);
                    }
                    if ((amariPosX == 0) || (amariPosY == 0)) {
                        offsetPosX = it->wClientX;
                        offsetPosY = it->wClientY +10;
                    } else {
                        offsetPosX = 0;
                        offsetPosY = 0;
                    }
                }
                // Calculate workPos[X/Y].
                int   y_swf_offset = 0;
                float x_ratio      = ratioX;
                float y_ratio      = ratioY;
                switch (wLastSWFMode) {
                case 0:
                    y_swf_offset = as->SWF0offset;
                    break;
                case 5:
                    y_swf_offset = as->SWF5offset /* - 0 */;
                    break;
                case 7:
                    y_swf_offset = as->SWF7offset /* + 0 */;
                    break;
                case 9:
                    y_swf_offset = as->SWF9offset + ((app->bUnicode) ? 0 : -50);
                    break;
                case 11:
                    y_swf_offset = as->SWF11offset /* - 0 */;
                    break;
                default:
                    x_ratio = y_ratio = 1.0;
                    break;
                }
                workPosX = (int)((float)(it->wPosX + offsetPosX               ) * x_ratio);
                workPosY = (int)((float)(it->wPosY + offsetPosY + y_swf_offset) * y_ratio);
                // Correction for workPosX.
                workPosX = (workPosX > app->sidebar_size) ? workPosX - app->sidebar_size : 0;

                // ふりがな Skip
                // ふりがな Skip は 出力時に
                if ((it2->emCharSizeMode == STR_SMALL) && (!(app->bUnicode)))
                    workPosY += (int)(10 * ratioY);
                if ((it2->emCharSizeMode == STR_MEDIUM) && (!(app->bUnicode)))
                    // 全角 -> 半角
                    it2->strDecode = GetHalfChar(it2->strDecode);

                if (app->fpLogFile) {
                    if (it2->bUnderLine)
                        fprintf(app->fpLogFile, "UnderLine : on\r\n");
                    if (it2->bBold)
                        fprintf(app->fpLogFile, "Bold : on\r\n");
                    if (it2->bItalic)
                        fprintf(app->fpLogFile, "Italic : on\r\n");
                    if (it2->bHLC != 0)
                        fprintf(app->fpLogFile, "HLC : on\r\n");
                    fprintf(app->fpLogFile, "Color : %#.X   ", it2->stCharColor);
                    fprintf(app->fpLogFile, "Char M,W,H,HI,VI : %4d, %4d, %4d, %4d, %4d   ",
                            it2->emCharSizeMode, it2->wCharW, it2->wCharH, it2->wCharHInterval, it2->wCharVInterval);
                    fprintf(app->fpLogFile, "%s\r\n", it2->strDecode.c_str());
                }

                WCHAR str[STRING_BUFFER_SIZE] = { 0 };
                CHAR strUTF8_2[STRING_BUFFER_SIZE] = { 0 };

                if ((cp->format == FORMAT_TAW) || (app->bUnicode))
                    strcat_s(strUTF8, STRING_BUFFER_SIZE, it2->strDecode.c_str());
                else {
                    // CP 932 to UTF-8
                    MultiByteToWideChar(932, 0, it2->strDecode.c_str(), -1, str, STRING_BUFFER_SIZE);
                    WideCharToMultiByte(CP_UTF8, 0, str, -1, strUTF8_2, STRING_BUFFER_SIZE, NULL, NULL);

                    strcat_s(strUTF8, STRING_BUFFER_SIZE, strUTF8_2);
                }
            }

            PCAPTION_LINE pCapLine = new CAPTION_LINE();
            if (!pCapLine)
                goto ERR_EXIT;
            pCapLine->str = strUTF8;
            if (pCapLine->str == "") {
                delete pCapLine;
                continue;
            }
            pCapLine->index                = 0;     //useless
            pCapLine->startTime            = (PTS > app->startPCR) ? (DWORD)(PTS - app->startPCR) : 0;
            pCapLine->endTime              = 0;
            pCapLine->outCharSizeMode      = workCharSizeMode;
            pCapLine->outCharColor.ucAlpha = 0x00;
            pCapLine->outCharColor.ucR     = workucR;
            pCapLine->outCharColor.ucG     = workucG;
            pCapLine->outCharColor.ucB     = workucB;
            pCapLine->outUnderLine         = workUnderLine;
            pCapLine->outShadow            = workShadow;
            pCapLine->outBold              = workBold;
            pCapLine->outItalic            = workItalic;
            pCapLine->outFlushMode         = workFlushMode;
            pCapLine->outHLC               = workHLC;
            pCapLine->outCharW             = (WORD)(workCharW * ratioX);
            pCapLine->outCharH             = (WORD)(workCharH * ratioY);
            pCapLine->outCharHInterval     = (WORD)(workCharHInterval * ratioX);
            pCapLine->outCharVInterval     = (WORD)(workCharVInterval * ratioY);
            pCapLine->outPosX              = workPosX;
            pCapLine->outPosY              = workPosY;

            capList->push_back(pCapLine);
        }

    }
    return 0;

ERR_EXIT:
    return -1;
}

static int prepare_app_handler(int argc, _TCHAR **argv, app_handler_t *app)
{
    size_t string_length = MAX_PATH;

    // Check max of string length.
    for (int i = 0; i < argc; i++) {
        size_t length = _tcslen(argv[i]) + 1 + 20;  // +20: It's a margin for append the suffix.
        if (length > string_length)
            string_length = length;
    }

    // Create parameter handler.
    CCaption2AssParameter *param = new CCaption2AssParameter(string_length);
    if (!param) {
        _tMyPrintf(_T("Failed to allocate the paramter handler.\r\n"));
        return -1;
    }
    if (param->Allocate()) {
        _tMyPrintf(_T("Failed to allocate the buffers for output.\r\n"));
        SAFE_DELETE(param);
        return -1;
    }

    // Setup.
    app->startPCR      = TIMESTAMP_INVALID_VALUE;
    app->lastPCR       = TIMESTAMP_INVALID_VALUE;
    app->lastPTS       = TIMESTAMP_INVALID_VALUE;
    app->string_length = string_length;
    app->param         = param;
    return 0;
}

int _tmain(int argc, _TCHAR *argv[])
{
    int             result = C2A_SUCCESS;
    CCaptionDllUtil capUtil;
    CAPTION_LIST    capList;
    app_handler_t   app = { 0 };

#ifdef _DEBUG
//  argc    = 5;
//  argv[1] = _T("-log");
//  argv[2] = _T("-format");
//  argv[3] = _T("dual");
//  argv[4] = _T("C:\\Users\\YourName\\Videos\\sample.ts");
#endif

    // Prepare the handlers.
    if (prepare_app_handler(argc, argv, &app)) {
        result = C2A_ERR_MEMORY;
        goto EXIT;
    }
    pid_information_t *pi = app.param->get_pid_information();
    cli_parameter_t   *cp = app.param->get_cli_parameter();
    ass_setting_t     *as = app.param->get_ass_setting();

    // Parse arguments.
    if (ParseCmd(argc, argv, app.param)) {
        result = C2A_ERR_PARAM;
        goto EXIT;
    }
    app.norubi      = cp->norubi;
    app.srtornament = (cp->format == FORMAT_TAW) ? FALSE : cp->srtornament;

    // Initialize Caption Utility.
    if (!capUtil.CheckUNICODE() || (cp->format == FORMAT_TAW)) {
        if (capUtil.Initialize() != NO_ERR) {
            _tMyPrintf(_T("Load Caption.dll failed\r\n"));
            result = C2A_ERR_DLL;
            goto EXIT;
        }
        app.bUnicode = FALSE;
    } else {
        if (capUtil.InitializeUNICODE() != NO_ERR) {
            _tMyPrintf(_T("Load Caption.dll failed\r\n"));
            result = C2A_ERR_DLL;
            goto EXIT;
        }
        app.bUnicode = TRUE;
    }

    // Initialize ASS/SRT filename.
    int not_specified = _tcsicmp(cp->TargetFileName1, _T("")) == 0;
    if (!not_specified) {
        TCHAR *pExt = PathFindExtension(cp->TargetFileName1);
        if (_tcsicmp(pExt, _T(".ts")) == 0)
            not_specified = 1;
    }
    if (not_specified)
        _tcscpy_s(cp->TargetFileName1, app.string_length, cp->FileName);

    if ((cp->format == FORMAT_ASS) || (cp->format == FORMAT_DUAL)) {
        TCHAR *pExt = PathFindExtension(cp->TargetFileName1);
        _tcscpy_s(pExt, 5, _T(".ass"));
    } else {
        TCHAR *pExt = PathFindExtension(cp->TargetFileName1);
        _tcscpy_s(pExt, 5, _T(".srt"));
    }
    if (cp->format == FORMAT_DUAL) {
        _tcscpy_s(cp->TargetFileName2, app.string_length, cp->TargetFileName1);
        TCHAR *pExt = PathFindExtension(cp->TargetFileName2);
        _tcscpy_s(pExt, 5, _T(".srt"));
    }

    static const TCHAR *format_name[FORMAT_MAX] = {
        _T(""),
        _T("srt"),
        _T("ass"),
        _T("srt for TAW"),
        _T("ass & srt")
    };
    _tMyPrintf(_T("[Source] %s\r\n"), cp->FileName);
    _tMyPrintf(_T("[Target] %s\r\n"), cp->TargetFileName1);
    _tMyPrintf(_T("[Format] %s\r\n"), format_name[cp->format]);

    // Open TS File.
    if (_tfopen_s(&(app.fpInputTs), cp->FileName, _T("rb")) || !(app.fpInputTs)) {
        _tMyPrintf(_T("Open TS File: %s failed\r\n"), cp->FileName);
        result = C2A_ERR_PARAM;
        goto EXIT;
    }

    // Open ASS/SRT File.
    if (_tfopen_s(&(app.fpTarget1), cp->TargetFileName1, _T("wb")) || !(app.fpTarget1)) {
        _tMyPrintf(_T("Open Target File: %s failed\r\n"), cp->TargetFileName1);
        result = C2A_FAILURE;
        goto EXIT;
    }
    app.assIndex = app.srtIndex = 1;
    if (cp->format == FORMAT_DUAL) {
        if (_tfopen_s(&(app.fpTarget2), cp->TargetFileName2, _T("wb")) || !(app.fpTarget2)) {
            _tMyPrintf(_T("Open Target File: %s failed\r\n"), cp->TargetFileName2);
            result = C2A_FAILURE;
            goto EXIT;
        }
    }

    if (cp->LogMode) {
        // Open Log File.
        if (_tcsicmp(cp->LogFileName, _T("")) == 0) {
            _tcscpy_s(cp->LogFileName, app.string_length, cp->TargetFileName1);
            TCHAR *pExt = PathFindExtension(cp->LogFileName);
            _tcscpy_s(pExt, 13, _T("_Caption.log"));
        }
        if (_tfopen_s(&(app.fpLogFile), cp->LogFileName, _T("wb")) || !(app.fpLogFile)) {
            _tMyPrintf(_T("Open Log File: %s failed\r\n"), cp->LogFileName);
            result = C2A_FAILURE;
            goto EXIT;
        }
    }

    // Read ini settings for ASS.
    if ((cp->format == FORMAT_ASS) || (cp->format == FORMAT_DUAL)) {
        if (IniFileRead(cp->ass_type, as)) {
            result = C2A_FAILURE;
            goto EXIT;
        }
        if ((as->PlayResX * 3) == (as->PlayResY * 4)) {
            app.sidebar_size = (((as->PlayResY * 16) / 9) - as->PlayResX) / 2;
            as->PlayResX = (as->PlayResY * 16) / 9;
        }
    }

    // Output header.
    static const unsigned char utf8_bom[3] = { 0xEF, 0xBB, 0xBF };
    if (cp->format == FORMAT_SRT)
        fwrite(utf8_bom, 3, 1, app.fpTarget1);
    else if (cp->format == FORMAT_ASS) {
        fwrite(utf8_bom, 3, 1, app.fpTarget1);
        assHeaderWrite(app.fpTarget1, as);
    } else if (cp->format == FORMAT_DUAL) {
        fwrite(utf8_bom, 3, 1, app.fpTarget1);
        assHeaderWrite(app.fpTarget1, as);
        fwrite(utf8_bom, 3, 1, app.fpTarget2);
    }
    if ((app.fpLogFile) && (app.bUnicode))
        fwrite(utf8_bom, 3, 1, app.fpLogFile);

    if (!FindStartOffset(app.fpInputTs)) {
        _tMyPrintf(_T("Invalid TS File.\r\n"));
        Sleep(2000);
        result = C2A_FAILURE;
        goto EXIT;
    }

    BOOL bPrintPMT = TRUE;
    BYTE pbPacket[188 * 2 + 4] = { 0 };
    DWORD packetCount = 0;

    // Main loop
    while (fread(pbPacket, 188, 1, app.fpInputTs) == 1) {
        packetCount++;
        if (cp->detectLength > 0) {
            if (packetCount > cp->detectLength && !(app.bCreateOutput)) {
                _tMyPrintf(_T("Programe has deteced %dw packets, but can't find caption. Now it exits.\r\n"), packetCount / 10000);
                break;
            }
        }
        if (app.fpLogFile) {
            if (packetCount < 100000) {
                if ((packetCount % 10000) == 0)
                    fprintf(app.fpLogFile, "Process  %dw packets.\r\n", packetCount / 10000);
            } else if (packetCount < 1000000) {
                if ((packetCount % 100000) == 0)
                    fprintf(app.fpLogFile, "Process  %dw packets.\r\n", packetCount / 10000);
            } else if (packetCount < 10000000) {
                if ((packetCount % 1000000) == 0)
                    fprintf(app.fpLogFile, "Process  %dw packets.\r\n", packetCount / 10000);
            } else {
                if ((packetCount % 10000000) == 0)
                    fprintf(app.fpLogFile, "Process  %dw packets.\r\n", packetCount / 10000);
            }
        }

        Packet_Header packet;
        parse_Packet_Header(&packet, &pbPacket[0]);

        if (packet.Sync != 'G') {
            if (!resync(pbPacket, app.fpInputTs)) {
                _tMyPrintf(_T("Invalid TS File.\r\n"));
                Sleep(2000);
                result = C2A_FAILURE;
                goto EXIT;
            }
            continue;
        }

        if (packet.TsErr)
            continue;

        // PAT
        if (packet.PID == 0 && (pi->PMTPid == 0 || bPrintPMT)) {
            parse_PAT(&pbPacket[0], &(pi->PMTPid));
            bPrintPMT = FALSE;

            continue; // next packet
        }

        // PMT
        if (pi->PMTPid != 0 && packet.PID == pi->PMTPid) {
            if (0x2b == (pbPacket[5] << 4) + ((pbPacket[6] & 0xf0) >> 4)) {
                ;
            } else
                continue; // next packet

            parse_PMT(&pbPacket[0], &(pi->PCRPid), &(pi->CaptionPid));

            if (app.fpLogFile)
                if (app.lastPTS == TIMESTAMP_INVALID_VALUE)
                    fprintf(app.fpLogFile, "PMT, PCR, Caption : %04x, %04x, %04x\r\n", pi->PMTPid, pi->PCRPid, pi->CaptionPid);

            continue; // next packet
        }

        // PCR
        if (pi->PCRPid != 0 && packet.PID == pi->PCRPid) {
            DWORD bADP = (((DWORD)pbPacket[3] & 0x30) >> 4);
            if (!(bADP & 0x2))
                continue; // next packet

            DWORD bAF = (DWORD)pbPacket[5];
            if (!(bAF & 0x10))
                continue; // next packet

            // Get PCR.
            /*     90kHz           27MHz
             *  +--------+-------+-------+
             *  | 33 bits| 6 bits| 9 bits|
             *  +--------+-------+-------+
             */
            long long PCR, PCR_base, PCR_ext;
            PCR_base = ((long long)pbPacket[ 6] << 25)
                     | ((long long)pbPacket[ 7] << 17)
                     | ((long long)pbPacket[ 8] <<  9)
                     | ((long long)pbPacket[ 9] <<  1)
                     | ((long long)pbPacket[10] >>  7);
            PCR_ext = ((long long)(pbPacket[10] & 0x01) << 8)
                    |  (long long) pbPacket[11];
            PCR = (PCR_base * 300 + PCR_ext) / 27000;

            if (app.fpLogFile)
                if (app.lastPTS == TIMESTAMP_INVALID_VALUE)
                    fprintf(app.fpLogFile, "PCR, startPCR, lastPCR, basePCR : %11lld, %11lld, %11lld, %11lld\r\n",
                            PCR, app.startPCR, app.lastPCR, app.basePCR);

            // Check startPCR.
            if (app.startPCR == TIMESTAMP_INVALID_VALUE) {
                app.startPCR = PCR;
                app.correctTS = cp->DelayTime;
            } else {
                long long checkTS = 0;
                // Check wrap-around.
                if (PCR < app.lastPCR) {
                    if (app.fpLogFile) {
                        fprintf(app.fpLogFile, "====== PCR less than lastPCR ======\r\n");
                        fprintf(app.fpLogFile, "PCR, startPCR, lastPCR, basePCR : %11lld, %11lld, %11lld, %11lld\r\n",
                                PCR, app.startPCR, app.lastPCR, app.basePCR);
                    }
                    app.basePCR += WRAP_AROUND_VALUE / 90;
                    checkTS = WRAP_AROUND_VALUE / 90;
                }
                // Check drop packet. (This is even if the CM cut.)
                checkTS += PCR;
                if (checkTS > app.lastPCR) {
                    checkTS -= app.lastPCR;
                    if (!(cp->keepInterval) && (checkTS > PCR_MAXIMUM_INTERVAL))
                        app.correctTS -= checkTS - (PCR_MAXIMUM_INTERVAL >> 2);
                }
            }

            // Update lastPCR.
            app.lastPCR = PCR;

            continue; // next packet
        }

        // Caption
        if (pi->CaptionPid != 0 && packet.PID == pi->CaptionPid) {

            long long PTS = 0;

            if (packet.PayloadStartFlag) {
#if 0
                // FIXME: Check PTS flag in PES Header.
                // [example]
                //if (!(packet.pts_flag))
                //    continue;
#endif

                // Get Caption PTS.
                PTS = GetPTS(pbPacket);
                if (app.fpLogFile)
                    fprintf(app.fpLogFile, "PTS, lastPTS, basePTS, startPCR : %11lld, %11lld, %11lld, %11lld    ",
                            PTS, app.lastPTS, app.basePTS, app.startPCR);

                // Check skip.
                if (PTS == TIMESTAMP_INVALID_VALUE || app.startPCR == TIMESTAMP_INVALID_VALUE) {
                    fprintf(app.fpLogFile, "Skip 1st caption\r\n");
                    continue;
                }

                // Check wrap-around.
                // [case]
                //   lastPCR:  Detection on the 1st packet.             [1st PCR  >>> w-around >>> 1st PTS]
                //   lastPTS:  Detection on the packet of 2nd or later. [prev PTS >>> w-around >>> now PTS]
                long long checkTS = (app.lastPTS == TIMESTAMP_INVALID_VALUE) ? app.lastPCR : app.lastPTS;
                if ((PTS < checkTS) && ((checkTS - PTS) > (WRAP_AROUND_CHECK_VALUE / 90)))
                    app.basePTS += WRAP_AROUND_VALUE / 90;

                // Update lastPTS.
                app.lastPTS = PTS;

            } else {
                if (app.fpLogFile)
                    fprintf(app.fpLogFile, "PTS, lastPTS, basePTS, startPCR : %11lld, %11lld, %11lld, %11lld    ",
                            PTS, app.lastPTS, app.basePTS, app.startPCR);

                // Check skip.
                if (app.lastPTS == TIMESTAMP_INVALID_VALUE || app.startPCR == TIMESTAMP_INVALID_VALUE) {
                    fprintf(app.fpLogFile, "Skip 2nd caption\r\n");
                    continue;
                }

                // Get Caption PTS from 1st caption.
                PTS = app.lastPTS;
            }

            // Correct PTS for output.
            PTS += app.basePTS + app.correctTS;

            unsigned short sH, sM, sS, sMs;
            HMS((PTS > app.startPCR) ? (PTS - app.startPCR) : 0, sH, sM, sS, sMs);

            if (packet.PayloadStartFlag)
                _tMyPrintf(_T("Caption Time: %01d:%02d:%02d.%03d\r\n"), sH, sM, sS, sMs);
            if (app.fpLogFile)
                fprintf(app.fpLogFile, "%s Caption Time: %01d:%02d:%02d.%03d\r\n",
                        ((packet.PayloadStartFlag) ? "1st" : "2nd"), sH, sM, sS, sMs);

            // Parse caption.
            int ret = capUtil.AddTSPacket(pbPacket);
            if (ret == CHANGE_VERSION) {
                std::vector<LANG_TAG_INFO> tagInfoList;
                ret = capUtil.GetTagInfo(&tagInfoList);
            } else if (ret == NO_ERR_CAPTION)
                if (output_caption(&app, &capUtil, &capList, PTS)) {
                    result = C2A_ERR_MEMORY;
                    goto EXIT;
                }
        }

    }

EXIT:
    clear_caption_list(&capList);

    if (app.fpInputTs)
        fclose(app.fpInputTs);
    if (app.fpTarget1)
        fclose(app.fpTarget1);
    if (app.fpTarget2)
        fclose(app.fpTarget2);
    if (app.fpLogFile)
        fclose(app.fpLogFile);

    if ((app.assIndex == 1) && (app.srtIndex == 1)) {
        Sleep(2000);
        remove(cp->TargetFileName1);
        if (cp->format == FORMAT_DUAL)
            remove(cp->TargetFileName2);
    }

    SAFE_DELETE(app.param);

    return result;
}
