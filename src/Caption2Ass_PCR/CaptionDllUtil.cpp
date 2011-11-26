
#include <windows.h>
#include <tchar.h>
#include "CaptionDllUtil.h"
#include "CaptionDef.h"
extern BOOL bChangedDll;

CCaptionDllUtil::CCaptionDllUtil(void)
{
	m_hModule = NULL;
}

CCaptionDllUtil::~CCaptionDllUtil(void)
{
	UnLoadDll();
}

BOOL CCaptionDllUtil::LoadDll(void)
{
	if ( m_hModule != NULL ) {
		return FALSE;
	}

	pfnInitializeCP = NULL;
	pfnUnInitializeCP = NULL;
	pfnAddTSPacketCP = NULL;
	pfnClearCP = NULL;
	pfnGetTagInfoCP = NULL;
	pfnGetCaptionDataCP = NULL;


	BOOL bRet = TRUE;

	m_hModule = ::LoadLibrary(_T("Caption.dll"));

	if ( m_hModule == NULL ){
		//AfxMessageBox( L"Caption.dll ÇÃÉçÅ[ÉhÇ…é∏îsÇµÇ‹ÇµÇΩ");
		return FALSE;
	}

	pfnInitializeCP = ( InitializeCP ) ::GetProcAddress( m_hModule , "InitializeCP");
	if ( !pfnInitializeCP ){
		bRet = FALSE;
		goto ERR_END;
	}
	pfnUnInitializeCP = ( UnInitializeCP ) ::GetProcAddress( m_hModule , "UnInitializeCP");
	if ( !pfnUnInitializeCP ){
		bRet = FALSE;
		goto ERR_END;
	}
	pfnAddTSPacketCP = ( AddTSPacketCP ) ::GetProcAddress( m_hModule , "AddTSPacketCP");
	if ( !pfnAddTSPacketCP ){
		bRet = FALSE;
		goto ERR_END;
	}
	pfnClearCP = ( ClearCP ) ::GetProcAddress( m_hModule , "ClearCP");
	if ( !pfnClearCP ){
		bRet = FALSE;
		goto ERR_END;
	}
	pfnGetTagInfoCP = ( GetTagInfoCP ) ::GetProcAddress( m_hModule , "GetTagInfoCP");
	if ( !pfnGetTagInfoCP ){
		bRet = FALSE;
		goto ERR_END;
	}
	pfnGetCaptionDataCP = ( GetCaptionDataCP ) ::GetProcAddress( m_hModule , "GetCaptionDataCP");
	if ( !pfnGetCaptionDataCP ){
		bRet = FALSE;
		goto ERR_END;
	}


ERR_END:
	if ( bRet == FALSE ){
		::FreeLibrary( m_hModule );
		m_hModule=NULL;
		//AfxMessageBox( L"GetProcAddress Ç…é∏îsÇµÇ‹ÇµÇΩ");
	}
	return bRet;
}

BOOL CCaptionDllUtil::UnLoadDll(void)
{
	if ( m_hModule != NULL ){
		pfnUnInitializeCP();
		::FreeLibrary( m_hModule );
	}
	m_hModule=NULL;
	return TRUE;
}

// mark10als
BOOL CCaptionDllUtil::CheckUNICODE()
{
	pfnInitializeUNICODE = NULL;
	if ( m_hModule == NULL ){
		if ( LoadDll() == FALSE ){
			return FALSE;
		}
	}
	pfnInitializeUNICODE = ( InitializeUNICODECP ) ::GetProcAddress( m_hModule , "InitializeUNICODE");
	if ( !pfnInitializeUNICODE ){
		return FALSE;
	}
	return TRUE;
}
DWORD CCaptionDllUtil::InitializeUNICODE()
{
	return pfnInitializeUNICODE();
}

// mark10als
DWORD CCaptionDllUtil::Initialize()
{
// mark10als
//	if ( LoadDll() == FALSE ){
//		return ERR_INIT;
//	}
// mark10als
	return pfnInitializeCP();
}

DWORD CCaptionDllUtil::UnInitialize()
{
	if ( m_hModule == NULL ){
		return ERR_INIT;
	}
	DWORD dwRet = pfnUnInitializeCP();
	UnLoadDll();
	return dwRet;
}

DWORD CCaptionDllUtil::AddTSPacket(
	BYTE* pbPacket
	)
{
	if ( m_hModule == NULL ){
		return ERR_INIT;
	}
	return pfnAddTSPacketCP(pbPacket);
}

DWORD CCaptionDllUtil::Clear()
{
	if ( m_hModule == NULL ){
		return ERR_INIT;
	}

	return pfnClearCP();
}

DWORD CCaptionDllUtil::GetTagInfo(
	LANG_TAG_INFO_DLL** ppList,
	DWORD* pdwListCount
	)
{
	if ( m_hModule == NULL ){
		return ERR_INIT;
	}

	return pfnGetTagInfoCP(ppList,pdwListCount);
}

DWORD CCaptionDllUtil::GetCaptionData(
	unsigned char ucLangTag, 
	CAPTION_DATA_DLL** ppList, 
	DWORD* pdwListCount
)
{
	if ( m_hModule == NULL ){
		return ERR_INIT;
	}

	return pfnGetCaptionDataCP(ucLangTag, ppList, pdwListCount);
}

DWORD CCaptionDllUtil::GetTagInfo(vector<LANG_TAG_INFO>* pList)
{
	if ( m_hModule == NULL ){
		return ERR_INIT;
	}

	LANG_TAG_INFO_DLL* pListDll;
	DWORD dwListCount;

	DWORD dwRet = pfnGetTagInfoCP(&pListDll,&dwListCount);
	if ( dwRet == TRUE ){
		for( DWORD i=0; i<dwListCount; i++ ){
			LANG_TAG_INFO Item;
			Item.ucLangTag=pListDll[i].ucLangTag;
			Item.ucDMF = pListDll[i].ucDMF;
			Item.ucDC = pListDll[i].ucDC;
			memcpy(Item.szISOLangCode, pListDll[i].szISOLangCode, 4 );
			Item.ucFormat = pListDll[i].ucFormat;
			Item.ucTCS = pListDll[i].ucTCS;
			Item.ucRollupMode = pListDll[i].ucRollupMode;
			pList->push_back(Item);
		
		}
	}
	return dwRet;
}

DWORD CCaptionDllUtil::GetCaptionData(unsigned char ucLangTag, vector<CAPTION_DATA>* pList)
{
	if ( m_hModule == NULL ){
		return ERR_INIT;
	}
	CAPTION_DATA_DLL* pListDll;
	DWORD dwListCount = 0;

	DWORD dwRet = pfnGetCaptionDataCP(ucLangTag, &pListDll, &dwListCount);
	if ( dwRet == TRUE ){
		for( DWORD i=0; i<dwListCount; i++ ){
			CAPTION_DATA Item;
			Item.bClear = pListDll[i].bClear;
			Item.wSWFMode = pListDll[i].wSWFMode;
			Item.wClientX = pListDll[i].wClientX;
			Item.wClientY = pListDll[i].wClientY;
			Item.wClientW = pListDll[i].wClientW;
			Item.wClientH = pListDll[i].wClientH;
			Item.wPosX = pListDll[i].wPosX;
			Item.wPosY = pListDll[i].wPosY;
			Item.dwWaitTime = pListDll[i].dwWaitTime;
			for( DWORD j=0; j<pListDll[i].dwListCount; j++ ){
				CAPTION_CHAR_DATA ItemChar;
				ItemChar.strDecode = pListDll[i].pstCharList[j].pszDecode;
				ItemChar.emCharSizeMode = (STRING_SIZE)pListDll[i].pstCharList[j].wCharSizeMode;
				ItemChar.stCharColor.ucAlpha = pListDll[i].pstCharList[j].stCharColor.ucAlpha;
				ItemChar.stCharColor.ucB = pListDll[i].pstCharList[j].stCharColor.ucB;
				ItemChar.stCharColor.ucG = pListDll[i].pstCharList[j].stCharColor.ucG;
				ItemChar.stCharColor.ucR = pListDll[i].pstCharList[j].stCharColor.ucR;
				ItemChar.stBackColor.ucAlpha = pListDll[i].pstCharList[j].stBackColor.ucAlpha;
				ItemChar.stBackColor.ucB = pListDll[i].pstCharList[j].stBackColor.ucB;
				ItemChar.stBackColor.ucG = pListDll[i].pstCharList[j].stBackColor.ucG;
				ItemChar.stBackColor.ucR = pListDll[i].pstCharList[j].stBackColor.ucR;
				ItemChar.stRasterColor.ucAlpha = pListDll[i].pstCharList[j].stRasterColor.ucAlpha;
				ItemChar.stRasterColor.ucB = pListDll[i].pstCharList[j].stRasterColor.ucB;
				ItemChar.stRasterColor.ucG = pListDll[i].pstCharList[j].stRasterColor.ucG;
				ItemChar.stRasterColor.ucR = pListDll[i].pstCharList[j].stRasterColor.ucR;
				ItemChar.bUnderLine = pListDll[i].pstCharList[j].bUnderLine;
				ItemChar.bShadow = pListDll[i].pstCharList[j].bShadow;
				ItemChar.bBold = pListDll[i].pstCharList[j].bBold;
				ItemChar.bItalic = pListDll[i].pstCharList[j].bItalic;
				ItemChar.bFlushMode = pListDll[i].pstCharList[j].bFlushMode;
				ItemChar.wCharW = pListDll[i].pstCharList[j].wCharW;
				ItemChar.wCharH = pListDll[i].pstCharList[j].wCharH;
				ItemChar.wCharHInterval = pListDll[i].pstCharList[j].wCharHInterval;
				ItemChar.wCharVInterval = pListDll[i].pstCharList[j].wCharVInterval;
				Item.CharList.push_back(ItemChar);
			}
			pList->push_back(Item);
		}
	}
	return dwRet;
}
