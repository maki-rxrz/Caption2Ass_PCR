//------------------------------------------------------------------------------
//IniFile.cpp
//------------------------------------------------------------------------------
//iniファイル操作

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "IniFile.h"
#include "CaptionDef.h"
#include "Caption2Ass_PCR.h"

//iniファイルパスを取得
static void GetPrivateProfilePath(TCHAR *pIniFilePath)
{
    TCHAR wkPath[_MAX_PATH];
    TCHAR wkDrive[_MAX_DRIVE];
    TCHAR wkDir[_MAX_DIR];
    TCHAR wkFileName[_MAX_FNAME];
    TCHAR wkExt[_MAX_EXT];
    DWORD dwRet;

    //初期化
    memset(wkPath, 0x00, sizeof(wkPath));
    memset(wkDrive, 0x00, sizeof(wkDrive));
    memset(wkDir, 0x00, sizeof(wkDir));
    memset(wkExt, 0x00, sizeof(wkExt));
    dwRet = 0;

    //実行中のプロセスのフルパス名を取得する
    dwRet = GetModuleFileName(NULL, wkPath, sizeof(wkPath));
    if (dwRet == 0) {
    //エラー処理など(省略)
    }

    //フルパス名を分割する
    _splitpath_s(wkPath, wkDrive, _MAX_DRIVE, wkDir, _MAX_DIR, wkFileName, _MAX_FNAME, wkExt, _MAX_EXT);

    _tcscat_s(pIniFilePath, _MAX_DRIVE, wkDrive);
    _tcscat_s(pIniFilePath, _MAX_DIR  , wkDir);
    _tcscat_s(pIniFilePath, _MAX_FNAME, wkFileName);
    _tcscat_s(pIniFilePath, _MAX_EXT  , _T(".ini"));
    return;
}

//PrivateProfileファイルから読み込む
extern int IniFileRead(TCHAR *ass_type, ass_setting_t *as)
{
    int iStrLen = 256;

    //iniファイルパス取得
    TCHAR pIniFilePath[_MAX_PATH];
    memset(pIniFilePath, 0x00, sizeof(pIniFilePath));

    GetPrivateProfilePath(pIniFilePath);
    // Open ini File
    FILE *fp = NULL;
    if (_tfopen_s(&fp, pIniFilePath, _T("r")) || !fp) {
        if (_tfopen_s(&fp, pIniFilePath, _T("wb")) || !fp)
            return -1;
        fprintf(fp, "%s", DEFAULT_INI);
    }
    fclose(fp);
    // Caption offset of SWF-Mode
    as->SWF0offset=GetPrivateProfileInt(_T("SWFModeOffset"),_T("SWF0offset"),0,pIniFilePath);
    as->SWF5offset=GetPrivateProfileInt(_T("SWFModeOffset"),_T("SWF5offset"),0,pIniFilePath);
    as->SWF7offset=GetPrivateProfileInt(_T("SWFModeOffset"),_T("SWF7offset"),0,pIniFilePath);
    as->SWF9offset=GetPrivateProfileInt(_T("SWFModeOffset"),_T("SWF9offset"),0,pIniFilePath);
    as->SWF11offset=GetPrivateProfileInt(_T("SWFModeOffset"),_T("SWF11offset"),0,pIniFilePath);

    // ass header infomation
    TCHAR *tmpBuff;
    tmpBuff = new TCHAR[iStrLen];
    memset(tmpBuff, 0, sizeof(TCHAR) * iStrLen);
    WCHAR str[1024] = {0};

    GetPrivateProfileString(ass_type,_T("Comment1"),NULL,tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->Comment1, 1024, NULL, NULL);
    GetPrivateProfileString(ass_type,_T("Comment2"),NULL,tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->Comment2, 1024, NULL, NULL);
    GetPrivateProfileString(ass_type,_T("Comment3"),NULL,tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->Comment3, 1024, NULL, NULL);
    as->PlayResX=GetPrivateProfileInt(ass_type,_T("PlayResX"),1920,pIniFilePath);
    as->PlayResY=GetPrivateProfileInt(ass_type,_T("PlayResY"),1080,pIniFilePath);

    GetPrivateProfileString(ass_type,_T("DefaultFontname"),_T("MS UI Gothic"),tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->DefaultFontname, 1024, NULL, NULL);
    as->DefaultFontsize=GetPrivateProfileInt(ass_type,_T("DefaultFontsize"),90,pIniFilePath);
    GetPrivateProfileString(ass_type,_T("DefaultStyle"),_T("&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,15,0,1,2,2,1,10,10,10,0"),tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->DefaultStyle, 1024, NULL, NULL);

    GetPrivateProfileString(ass_type,_T("BoxFontname"),_T("MS UI Gothic"),tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->BoxFontname, 1024, NULL, NULL);
    as->BoxFontsize=GetPrivateProfileInt(ass_type,_T("BoxFontsize"),90,pIniFilePath);
    GetPrivateProfileString(ass_type,_T("BoxStyle"),_T("&HFFFFFFFF,&H000000FF,&H00FFFFFF,&H00FFFFFF,0,0,0,0,100,100,0,0,1,2,2,2,10,10,10,0"),tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->BoxStyle, 1024, NULL, NULL);

    GetPrivateProfileString(ass_type,_T("RubiFontname"),_T("MS UI Gothic"),tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->RubiFontname, 1024, NULL, NULL);
    as->RubiFontsize=GetPrivateProfileInt(ass_type,_T("RubiFontsize"),50,pIniFilePath);
    GetPrivateProfileString(ass_type,_T("RubiStyle"),_T("&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,2,1,10,10,10,0"),tmpBuff,iStrLen,pIniFilePath);
    MultiByteToWideChar(932, 0, tmpBuff, -1, str, 1024);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, as->RubiStyle, 1024, NULL, NULL);

    SAFE_DELETE_ARRAY(tmpBuff);

    return 0;
}
