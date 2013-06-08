//------------------------------------------------------------------------------
// ass_header.cpp
//------------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "ass_header.h"
#include "Caption2Ass_PCR.h"

extern void assHeaderWrite(FILE *fp, ass_setting_t *as)
{
    fprintf(fp, "[Script Info]\r\n");
    fprintf(fp, "; %s\r\n", as->Comment1);
    fprintf(fp, "; %s\r\n", as->Comment2);
    fprintf(fp, "; %s\r\n", as->Comment3);
    fprintf(fp, "%s", ASS_HEADER1);
    fprintf(fp, "PlayResX: %d\r\n", as->PlayResX);
    fprintf(fp, "PlayResY: %d\r\n", as->PlayResY);
    fprintf(fp, "%s", ASS_HEADER2);
    fprintf(fp, "Style: %s,%s,%d,%s\r\n", _T("Default"), as->DefaultFontname, as->DefaultFontsize, as->DefaultStyle);
    fprintf(fp, "Style: %s,%s,%d,%s\r\n", _T("Box"), as->BoxFontname, as->BoxFontsize, as->BoxStyle);
    fprintf(fp, "Style: %s,%s,%d,%s\r\n//\r\n", _T("Rubi"), as->RubiFontname, as->RubiFontsize, as->RubiStyle);
    fprintf(fp, "%s", ASS_HEADER3);

    return;
}
