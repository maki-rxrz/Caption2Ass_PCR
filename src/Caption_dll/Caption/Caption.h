#ifndef __CAPTION_H__
#define __CAPTION_H__

#include "CaptionDef.h"

#ifdef __cplusplus
extern "C" {
#endif

//DLLの初期化
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI InitializeCP();

//DLLの開放
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI UnInitializeCP();

//188バイトTS1パケット
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI AddTSPacketCP(BYTE* pbPacket);

//内部データクリア
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI ClearCP();

//字幕情報取得
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI GetTagInfoCP(LANG_TAG_INFO_DLL** ppList, DWORD* pdwListCount);

//字幕データ本文取得
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI GetCaptionDataCP(unsigned char ucLangTag, CAPTION_DATA_DLL** ppList, DWORD* pdwListCount);

#ifdef __cplusplus
}
#endif

#endif