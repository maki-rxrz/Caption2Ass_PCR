#include <Windows.h>
#include <stdio.h>
#include <string>
#include <map>
#include "CaptionDef.h"
#include "ColorDef.h"
#include "ARIB8CharDecode.h"
#include "IniFile.h"
#pragma managed

// mark10als
// ARIBの追加記号 ＆ 追加漢字のテーブル定義
// 実体は、「CaptionMain.cpp」で宣言

#define ARIB_MAX		495
#define ARIB_MAX2		137

typedef struct _GAIJI_TABLE{
	string usARIB8;
	string strChar;
} GAIJI_TABLE;

extern GAIJI_TABLE GaijiTable[];

extern GAIJI_TABLE GaijiTbl2[];

string iniFilename = "cc_DRCS.ini";
string iniFilenameU = "UNICODE_cc_DRCS.ini";
string iniFileARIB = "cc_gaiji.ini";
string iniFileARIBU = "UNICODE_cc_gaiji.ini";
string iniFileARIB2 = "cc_gaiji2.ini";
string iniFileARIB2U = "UNICODE_cc_gaiji2.ini";
// mark10als

BOOL IniFile::ReadIni()
{
	CARIB8CharDecode ARIB8CharDecode;
	string tmpFilename = ARIB8CharDecode.GetAppPath() + "\\Gaiji\\" + iniFilename;
	FILE *fpini = fopen(tmpFilename.c_str(), "rt");
	if (!fpini) {
		return FALSE;
	}
	string tmprl;
	CHAR strSJIS[1024] = {0};
	do
	{
		fgets( strSJIS, 1024, fpini );
		size_t len = strlen( strSJIS );
		strSJIS[len-1]=0;
		tmprl = strSJIS; 
	}
	while(( tmprl != "[外字代用字]") && ( !feof( fpini ) ));
	
	do
	{
		fgets( strSJIS, 1024, fpini );
		size_t len = strlen( strSJIS );
		strSJIS[len-1]=0;
		tmprl = strSJIS; 
		int iPos = tmprl.find_first_of("=");
		if( iPos != string::npos ){
			if(iPos == 32){
				string tmpKey = tmprl.substr( 0, iPos );
				string tmpArg = tmprl.substr( iPos +1 );
				ARIB8CharDecode.Add_dicHash_Char( tmpKey, tmpArg );
			}
		}
	}
	while((tmprl != "[外字出力]") && ( !feof( fpini ) ));
	fclose(fpini);
	return TRUE;
}

// mark10als
BOOL IniFile::ReadIniARIB()
{
	// ARIBの追加記号 「ARIB STD-B24」第一編 第２部の表7-10
	CARIB8CharDecode ARIB8CharDecode;
	string tmpFilename = ARIB8CharDecode.GetAppPath() + "\\Gaiji\\" + iniFileARIB;
	FILE *fpini = fopen(tmpFilename.c_str(), "rt");
	if (!fpini) {
		return FALSE;
	}
	string tmprl;
	CHAR strSJIS[1024] = {0};
	do
	{
		fgets( strSJIS, 1024, fpini );
		size_t len = strlen( strSJIS );
		strSJIS[len-1]=0;
		tmprl = strSJIS; 
	}
	while(( tmprl != "[ARIB外字代用字]") && ( !feof( fpini ) ));
	int iGaijiCtr = 0;
	do
	{
		fgets( strSJIS, 1024, fpini );
		size_t len = strlen( strSJIS );
		strSJIS[len-1]=0;
		tmprl = strSJIS; 
		int iPos = tmprl.find_first_of("=");
		if( iPos != string::npos ){
			if((iPos == 4) && (iGaijiCtr < ARIB_MAX)){
				GaijiTable[iGaijiCtr].usARIB8 = tmprl.substr( 0, iPos );
				GaijiTable[iGaijiCtr].strChar = tmprl.substr( iPos +1 );
				iGaijiCtr += 1;
			}
		}
	}
	while(( tmprl != "[ARIB外字出力]") && ( !feof( fpini ) ));
	fclose(fpini);

	// ARIBの追加漢字 「ARIB STD-B24」第一編 第２部の表7-11
	tmpFilename = ARIB8CharDecode.GetAppPath() + "\\Gaiji\\" + iniFileARIB2;
	fpini = fopen(tmpFilename.c_str(), "rt");
	if (!fpini) {
		return FALSE;
	}
//	string tmprl;
//	CHAR strSJIS[1024] = {0};
	do
	{
		fgets( strSJIS, 1024, fpini );
		size_t len = strlen( strSJIS );
		strSJIS[len-1]=0;
		tmprl = strSJIS; 
	}
	while(( tmprl != "[ARIB外字2代用字]") && ( !feof( fpini ) ));
	int iGaijiCtr2 = 0;
	do
	{
		fgets( strSJIS, 1024, fpini );
		size_t len = strlen( strSJIS );
		strSJIS[len-1]=0;
		tmprl = strSJIS; 
		int iPos = tmprl.find_first_of("=");
		if( iPos != string::npos ){
			if((iPos == 3) && (iGaijiCtr2 < ARIB_MAX2)){
				GaijiTbl2[iGaijiCtr2].usARIB8 = tmprl.substr( 0, iPos );
				GaijiTbl2[iGaijiCtr2].strChar = tmprl.substr( iPos +1 );
				iGaijiCtr2 += 1;
			}
		}
	}
	while(( tmprl != "[ARIB外字2出力]") && ( !feof( fpini ) ));
	fclose(fpini);
	return TRUE;
}
BOOL IniFile::ReadIniUNICODE()
{
	CARIB8CharDecode ARIB8CharDecode;
	string tmpFilename = ARIB8CharDecode.GetAppPath() + "\\Gaiji\\" + iniFilenameU;
	FILE *fpini = fopen(tmpFilename.c_str(), "rb");
	if (!fpini) {
		return FALSE;
	}
	// BOM判定
	unsigned char utf16bom[2];
	utf16bom[0] = (char)fgetc( fpini );
	utf16bom[1] = (char)fgetc( fpini );
	if (utf16bom[0] == 0xFF && utf16bom[1] == 0xFE) {
	} else {
		int result = fseek( fpini, 0, SEEK_SET);
		if( result ){
			return FALSE;
		}
	}
	string tmprl;
	WCHAR str[1024] = {0};
	CHAR strUTF8[1024] = {0};
	CHAR strSJIS[1024] = {0};
	string tmpUTF8;
	do
	{
		fgetws( str, 1024, fpini );
		size_t len = wcslen( str );
		str[len-2]=0;
		WideCharToMultiByte(CP_THREAD_ACP, 0, str, -1, strSJIS, 1024, NULL, NULL);
		tmprl = strSJIS; 
	}
	while(( tmprl != "[外字代用字]") && ( !feof( fpini ) ));
	
	do
	{
		fgetws( str, 1024, fpini );
		size_t len = wcslen( str );
		str[len-2]=0;
		WideCharToMultiByte(CP_THREAD_ACP, 0, str, -1, strSJIS, 1024, NULL, NULL);
		tmprl = strSJIS; 
		WideCharToMultiByte(CP_UTF8, 0, str, -1, strUTF8, 1024, NULL, NULL);
		tmpUTF8 = strUTF8;
		int iPos = tmpUTF8.find_first_of("=");
		if( iPos != string::npos ){
			if(iPos == 32){
				string tmpKey = tmpUTF8.substr( 0, iPos );
				string tmpArg = tmpUTF8.substr( iPos +1 );
				ARIB8CharDecode.Add_dicHash_Char( tmpKey,  tmpArg);
			}
		}
	}
	while((tmprl != "[外字出力]") && ( !feof( fpini ) ));
	fclose(fpini);
	return TRUE;
}

	// mark10als
BOOL IniFile::ReadIniARIBUNICODE()
{
	// ARIBの追加記号 「ARIB STD-B24」第一編 第２部の表7-10
	CARIB8CharDecode ARIB8CharDecode;
	string tmpFilename = ARIB8CharDecode.GetAppPath() + "\\Gaiji\\" + iniFileARIBU;
	FILE *fpini = fopen(tmpFilename.c_str(), "rb");
	if (!fpini) {
		return FALSE;
	}
	// BOM判定
	unsigned char utf16bom[2];
	utf16bom[0] = (char)fgetc( fpini );
	utf16bom[1] = (char)fgetc( fpini );
	if (utf16bom[0] == 0xFF && utf16bom[1] == 0xFE) {
	} else {
		int result = fseek( fpini, 0, SEEK_SET);
		if( result ){
			return FALSE;
		}
	}
	string tmprl;
	WCHAR str[1024] = {0};
	CHAR strUTF8[1024] = {0};
	CHAR strSJIS[1024] = {0};
	string tmpUTF8;
	do
	{
		fgetws( str, 1024, fpini );
		size_t len = wcslen( str );
		str[len-2]=0;
		WideCharToMultiByte(CP_THREAD_ACP, 0, str, -1, strSJIS, 1024, NULL, NULL);
		tmprl = strSJIS; 
	}
	while(( tmprl != "[ARIB外字代用字]") && ( !feof( fpini ) ));
	int iGaijiCtr = 0;
	do
	{
		fgetws( str, 1024, fpini );
		size_t len = wcslen( str );
		str[len-2]=0;
		WideCharToMultiByte(CP_THREAD_ACP, 0, str, -1, strSJIS, 1024, NULL, NULL);
		tmprl = strSJIS; 
		WideCharToMultiByte(CP_UTF8, 0, str, -1, strUTF8, 1024, NULL, NULL);
		tmpUTF8 = strUTF8;
		int iPos = tmpUTF8.find_first_of("=");
		if( iPos != string::npos ){
			if((iPos == 4) && (iGaijiCtr < ARIB_MAX)){
				GaijiTable[iGaijiCtr].usARIB8 = tmpUTF8.substr( 0, iPos );
				GaijiTable[iGaijiCtr].strChar = tmpUTF8.substr( iPos +1 );
				iGaijiCtr += 1;
			}
		}
	}
	while(( tmprl != "[ARIB外字出力]") && ( !feof( fpini ) ));
	fclose(fpini);

	// ARIBの追加漢字 「ARIB STD-B24」第一編 第２部の表7-11
	tmpFilename = ARIB8CharDecode.GetAppPath() + "\\Gaiji\\" + iniFileARIB2U;
	fpini = fopen(tmpFilename.c_str(), "rb");
	if (!fpini) {
		return FALSE;
	}
	// BOM判定
//	unsigned char utf16bom[2];
	utf16bom[0] = (char)fgetc( fpini );
	utf16bom[1] = (char)fgetc( fpini );
	if (utf16bom[0] == 0xFF && utf16bom[1] == 0xFE) {
	} else {
		int result = fseek( fpini, 0, SEEK_SET);
		if( result ){
			return FALSE;
		}
	}
//	wstring tmprl;
//	WCHAR str[1024] = {0};
//	CHAR strUTF8[1024] = {0};
//	string tmpUTF8;
	do
	{
		fgetws( str, 1024, fpini );
		size_t len;
		len = wcslen( str );
		str[len-2]=0;
		WideCharToMultiByte(CP_THREAD_ACP, 0, str, -1, strSJIS, 1024, NULL, NULL);
		tmprl = strSJIS; 
	}
	while(( tmprl != "[ARIB外字2代用字]") && ( !feof( fpini ) ));
	int iGaijiCtr2 = 0;
	do
	{
		fgetws( str, 1024, fpini );
		size_t len;
		len = wcslen( str );
		str[len-2]=0;
		WideCharToMultiByte(CP_THREAD_ACP, 0, str, -1, strSJIS, 1024, NULL, NULL);
		tmprl = strSJIS; 
		WideCharToMultiByte(CP_UTF8, 0, str, -1, strUTF8, 1024, NULL, NULL);
		tmpUTF8 = strUTF8;
		int iPos = tmpUTF8.find_first_of("=");
		if( iPos != string::npos ){
			if((iPos == 3) && (iGaijiCtr2 < ARIB_MAX2)){
				GaijiTbl2[iGaijiCtr2].usARIB8 = tmpUTF8.substr( 0, iPos );
				GaijiTbl2[iGaijiCtr2].strChar = tmpUTF8.substr( iPos +1 );
				iGaijiCtr2 += 1;
			}
		}
	}
	while(( tmprl != "[ARIB外字2出力]") && ( !feof( fpini ) ));
	fclose(fpini);
	return TRUE;
}
