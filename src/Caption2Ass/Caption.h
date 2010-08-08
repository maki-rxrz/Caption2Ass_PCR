#ifndef __CAPTION_H__
#define __CAPTION_H__

#include "CaptionDef.h"

typedef DWORD (WINAPI* InitializeCP)();

typedef DWORD (WINAPI* UnInitializeCP)();

typedef DWORD (WINAPI* AddTSPacketCP)(BYTE* pbPacket);

typedef DWORD (WINAPI* ClearCP)();

typedef DWORD (WINAPI* GetTagInfoCP)(LANG_TAG_INFO_DLL** ppList, DWORD* pdwListCount);

typedef DWORD (WINAPI* GetCaptionDataCP)(unsigned char ucLangTag, CAPTION_DATA_DLL** ppList, DWORD* pdwListCount);

#endif