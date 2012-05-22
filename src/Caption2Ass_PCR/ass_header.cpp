//ass_header.cpp
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include"ass_header.h"

extern TCHAR *passComment1;
extern TCHAR *passComment2;
extern TCHAR *passComment3;
extern long assPlayResX;
extern long assPlayResY;
extern TCHAR *passDefaultFontname;
extern long assDefaultFontsize;
extern TCHAR *passDefaultStyle;
extern TCHAR *passBoxFontname;
extern long assBoxFontsize;
extern TCHAR *passBoxStyle;
extern TCHAR *passRubiFontname;
extern long assRubiFontsize;
extern TCHAR *passRubiStyle;

void assHeaderWrite(FILE *fp)
{
	fprintf(fp, "[Script Info]\r\n");
	fprintf(fp, "; %s\r\n", passComment1);
	fprintf(fp, "; %s\r\n", passComment2);
	fprintf(fp, "; %s\r\n", passComment3);
	fprintf(fp, "%s", ASS_HEADER1);
	fprintf(fp, "PlayResX: %d\r\n", assPlayResX);
	fprintf(fp, "PlayResY: %d\r\n", assPlayResY);
	fprintf(fp, "%s", ASS_HEADER2);
	fprintf(fp, "Style: %s,%s,%d,%s\r\n", _T("Default"), passDefaultFontname, assDefaultFontsize, passDefaultStyle);
	fprintf(fp, "Style: %s,%s,%d,%s\r\n", _T("Box"), passBoxFontname, assBoxFontsize, passBoxStyle);
	fprintf(fp, "Style: %s,%s,%d,%s\r\n//\r\n", _T("Rubi"), passRubiFontname, assRubiFontsize, passRubiStyle);
	fprintf(fp, "%s", ASS_HEADER3);

	return;
}
