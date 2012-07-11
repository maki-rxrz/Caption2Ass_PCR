#ifndef __CAPTION_DEF_H__
#define __CAPTION_DEF_H__

typedef struct _CLUT_DAT_DLL{
	unsigned char ucR;
	unsigned char ucG;
	unsigned char ucB;
	unsigned char ucAlpha;
} CLUT_DAT_DLL;

typedef struct _CAPTION_CHAR_DATA_DLL{
	char* pszDecode;
	DWORD wCharSizeMode;

	CLUT_DAT_DLL stCharColor;
	CLUT_DAT_DLL stBackColor;
	CLUT_DAT_DLL stRasterColor;

	BOOL bUnderLine;
	BOOL bShadow;
	BOOL bBold;
	BOOL bItalic;
	BYTE bFlushMode;
	BYTE bHLC; //must ignore low 4bits

	WORD wCharW;
	WORD wCharH;
	WORD wCharHInterval;
	WORD wCharVInterval;
} CAPTION_CHAR_DATA_DLL;

typedef struct _CAPTION_DATA_DLL{
	BOOL bClear;
	WORD wSWFMode;
	WORD wClientX;
	WORD wClientY;
	WORD wClientW;
	WORD wClientH;
	WORD wPosX;
	WORD wPosY;
	DWORD dwListCount;
	CAPTION_CHAR_DATA_DLL* pstCharList;
	DWORD dwWaitTime;
} CAPTION_DATA_DLL;

typedef struct _LANG_TAG_INFO_DLL{
	unsigned char ucLangTag;
	unsigned char ucDMF;
	unsigned char ucDC;
	char szISOLangCode[4];
	unsigned char ucFormat;
	unsigned char ucTCS;
	unsigned char ucRollupMode;
}LANG_TAG_INFO_DLL;
/*
typedef struct _DRCS_PATTERN_DLL{
	DWORD dwDRCCode;
	DWORD dwUCS;
	WORD wGradation;
	WORD wReserved; //zero cleared
	DWORD dwReserved; //zero cleared
	BITMAPINFOHEADER bmiHeader;
	const BYTE* pbBitmap;
}DRCS_PATTERN_DLL;
*/
#endif
