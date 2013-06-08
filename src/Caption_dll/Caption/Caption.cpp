//------------------------------------------------------------------------------
// Caption.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//------------------------------------------------------------------------------

#include <windows.h>

#include "CaptionDef.h"
#include "ColorDef.h"
#include "ARIB8CharDecode.h"
#include "CaptionMain.h"
#include "Caption.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    return TRUE;
}

CCaptionMain* g_sys = NULL;

//DLLの初期化
//戻り値：エラーコード
DWORD WINAPI InitializeCP()
{
    if (g_sys != NULL) {
        return ERR_INIT;
    }
    BOOL bUNICODE = FALSE;
    g_sys = new CCaptionMain(bUNICODE);
    return NO_ERR;
}

//DLLの初期化 UNICODE対応
//戻り値：エラーコード
DWORD WINAPI InitializeUNICODE()
{
    if (g_sys != NULL) {
        return ERR_INIT;
    }
    BOOL bUNICODE = TRUE;
    g_sys = new CCaptionMain(bUNICODE);
    return NO_ERR;
}

//DLLの開放
//戻り値：エラーコード
DWORD WINAPI UnInitializeCP()
{
    if (g_sys != NULL) {
        delete g_sys;
        g_sys = NULL;
    }
    return NO_ERR;
}

DWORD WINAPI AddTSPacketCP(BYTE* pbPacket)
{
    if (g_sys == NULL) {
        return ERR_NOT_INIT;
    }
    return g_sys->AddTSPacket(pbPacket);
}

DWORD WINAPI ClearCP()
{
    if (g_sys == NULL) {
        return ERR_NOT_INIT;
    }
    return g_sys->Clear();
}

DWORD WINAPI GetTagInfoCP(LANG_TAG_INFO_DLL** ppList, DWORD* pdwListCount)
{
    if (g_sys == NULL) {
        return ERR_NOT_INIT;
    }
    return g_sys->GetTagInfo(ppList, pdwListCount);
}

DWORD WINAPI GetCaptionDataCP(unsigned char ucLangTag, CAPTION_DATA_DLL** ppList, DWORD* pdwListCount)
{
    if (g_sys == NULL) {
        return ERR_NOT_INIT;
    }
    return g_sys->GetCaptionData(ucLangTag, ppList, pdwListCount);
}
