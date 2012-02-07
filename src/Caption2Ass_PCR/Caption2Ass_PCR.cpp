// Caption2Ass_PCR.cpp : Defines the entry point for the console application.
//
#pragma warning(disable: 4996 4995)
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <shlwapi.h>
//#include <strsafe.h>
#include <vector>
#include <list>
// mark10als
#include "CommRoutine.h"
// mark10als
#include "CaptionDllUtil.h"
#include "ass_header.h"

enum {
	FORMAT_SRT = 1,
// mark10als
//	FORMAT_ASS = 2
	FORMAT_ASS = 2,
	FORMAT_TAW = 3,
	FORMAT_DUAL = 4
// mark10als
};

// mark10als
typedef struct _ASS_COLOR{
	unsigned char ucR;
	unsigned char ucG;
	unsigned char ucB;
} ASS_COLOR;
// mark10als
typedef struct _SRT_LINE {
	UINT				index;
	DWORD				startTime;
	DWORD				endTime;
// mark10als
	BYTE				outCharSizeMode;
	ASS_COLOR			outCharColor;
	WORD				outCharW;
	WORD				outCharH;
	WORD				outCharHInterval;
	WORD				outCharVInterval;
	WORD				outPosX;
	WORD				outPosY;
	BOOL				outornament;
// mark10als
	std::string			str;
} SRT_LINE, *PSRT_LINE;

typedef std::list<PSRT_LINE> SRT_LIST;

VOID _tMyPrintf(IN	LPCTSTR tracemsg, ...);
BOOL ParseCmd(int argc, char**argv);
void IniFileRead(char *passType);
void assHeaderWrite(FILE *fp);
BOOL FindStartOffset(FILE *fp);
BOOL resync(BYTE *pbPacket, FILE *fp);
long long GetPTS(BYTE *pbPacket);
// mark10als
long delayTime = 0;
BOOL bLogMode = FALSE;
BOOL bUnicode = FALSE;
BOOL bsrtornament = FALSE;
TCHAR *pTargetFileName2 = NULL;
TCHAR *pLogFileName = NULL;
extern long assSWF0offset = 0;
extern long assSWF5offset = 0;
extern long assSWF7offset = 0;
extern long assSWF9offset = 0;
extern long assSWF11offset = 0;
extern TCHAR *passType = NULL;
extern TCHAR *passComment1 = NULL;
extern TCHAR *passComment2 = NULL;
extern TCHAR *passComment3 = NULL;
extern long assPlayResX = 0;
extern long assPlayResY = 0;
extern TCHAR *passDefaultFontname = NULL;
extern long assDefaultFontsize = 0;
extern TCHAR *passRubiFontname = NULL;
extern long assRubiFontsize = 0;
extern TCHAR *passDefaultStyle = NULL;
extern TCHAR *passRubiStyle = NULL;
// mark10als

DWORD assIndex = 1; // index for ASS
void DumpAssLine(FILE *fp, SRT_LIST * list, long long PTS)
{
	SRT_LIST::iterator it = list->begin();
	for(int i = 0; it != list->end(); it++, i++) {

// mark10als
//		if (i == 0) {
// mark10als
			(*it)->endTime = (DWORD)PTS;

			unsigned short sH, sM, sS, sMs, eH, eM, eS, eMs;

			sMs = (int)(*it)->startTime % 1000;
			sS = (int)((*it)->startTime / 1000) % 60;
			sM = (int)((*it)->startTime / (1000 * 60)) % 60;
			sH = (int)((*it)->startTime / (1000 * 60 *60));

			eMs = (int)(*it)->endTime % 1000;
			eS = (int)((*it)->endTime / 1000) % 60;
			eM = (int)((*it)->endTime / (1000 * 60)) % 60;
			eH = (int)((*it)->endTime / (1000 * 60 *60));
			sMs /= 10;
			eMs /= 10;
// mark10als
//			fprintf(fp,"Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Default,,0000,0000,0000,,", sH, sM, sS, sMs, eH, eM, eS, eMs);
			if ((*it)->outCharSizeMode == STR_SMALL) {
				fprintf(fp,"Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Rubi,,0000,0000,0000,,{\\pos(%d,%d)", sH, sM, sS, sMs, eH, eM, eS, eMs, (*it)->outPosX, (*it)->outPosY);
			} else {
				fprintf(fp,"Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Default,,0000,0000,0000,,{\\pos(%d,%d)", sH, sM, sS, sMs, eH, eM, eS, eMs, (*it)->outPosX, (*it)->outPosY);
			}
			if ((*it)->outCharColor.ucR != 0xff || (*it)->outCharColor.ucG != 0xff || (*it)->outCharColor.ucB != 0xff ) {
				fprintf(fp,"\\c&H%06x&", (*it)->outCharColor);
			}
			if ((bUnicode) && (((*it)->outCharH + (*it)->outCharVInterval) != 60)) {
				int iFontSize = 0;
			//	if ((*it)->outCharSizeMode == STR_SMALL) {
			//		iFontSize = (assRubiFontsize * ((*it)->outCharH + (*it)->outCharVInterval)) / 60;
			//	} else {
			//		iFontSize = (assDefaultFontsize * ((*it)->outCharH + (*it)->outCharVInterval)) / 60;
			//	}
				iFontSize = (100 * ((*it)->outCharH + (*it)->outCharVInterval)) / 60;
				fprintf(fp,"\\fscy%d", iFontSize);
			}
			fprintf(fp,"}");
//		}
// mark10als

		fwrite((*it)->str.c_str(), (*it)->str.size(),1, fp);
		fprintf(fp, "\\N");
// mark10als
		fprintf(fp, "\r\n");
// mark10als

//		delete (*it);
	}

// mark10als
	if (list->size() > 0)
		assIndex++;
//		fprintf(fp, "\r\n");
// mark10als

//	list->clear();
}

DWORD srtIndex = 1; // index for SRT
void DumpSrtLine(FILE *fp, SRT_LIST * list, long long PTS)
{
	BOOL bNoSRT = TRUE;
	SRT_LIST::iterator it = list->begin();
	for(int i = 0; it != list->end(); it++, i++) {

		if (i == 0) {
			(*it)->endTime = (DWORD)PTS;

			unsigned short sH, sM, sS, sMs, eH, eM, eS, eMs;

			sMs = (int)(*it)->startTime % 1000;
			sS = (int)((*it)->startTime / 1000) % 60;
			sM = (int)((*it)->startTime / (1000 * 60)) % 60;
			sH = (int)((*it)->startTime / (1000 * 60 *60));

			eMs = (int)(*it)->endTime % 1000;
			eS = (int)((*it)->endTime / 1000) % 60;
			eM = (int)((*it)->endTime / (1000 * 60)) % 60;
			eH = (int)((*it)->endTime / (1000 * 60 *60));

			fprintf(fp,"%d\r\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\r\n", srtIndex, sH, sM, sS, sMs, eH, eM, eS, eMs);
		}

// mark10als
		// ‚Ó‚è‚ª‚È Skip
		if ((*it)->outCharSizeMode == STR_SMALL)
			continue;
		bNoSRT = FALSE;
		if ((*it)->outornament) {
			if ((*it)->outCharColor.ucR != 0xff || (*it)->outCharColor.ucG != 0xff || (*it)->outCharColor.ucB != 0xff ) {
				fprintf(fp,"<font color=\"#%02x%02x%02x\">", (*it)->outCharColor.ucR, (*it)->outCharColor.ucG, (*it)->outCharColor.ucB);
			}
		}
		fwrite((*it)->str.c_str(), (*it)->str.size(),1, fp);
		if ((*it)->outornament) {
			if ((*it)->outCharColor.ucR != 0xff || (*it)->outCharColor.ucG != 0xff || (*it)->outCharColor.ucB != 0xff ) {
				fprintf(fp,"</font>");
			}
		}
// mark10als
		fprintf(fp, "\r\n");

//		delete (*it);
	}

	if (list->size() > 0) {
		if (bNoSRT){
			fprintf(fp, "\r\n");
		}
		fprintf(fp, "\r\n");
		srtIndex++;
	}

//	list->clear();
}

#include "packet_types.h"

USHORT PMTPid = 0;
USHORT CaptionPid = 0;
USHORT PCRPid = 0;
DWORD format = FORMAT_ASS;
TCHAR *pFileName = NULL;
TCHAR *pTargetFileName = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	long long startPCR = 0;
	long long lastPCR = 0;
	long long lastPTS = 0;
// mark10als
	long long basePCR = 0;
	long long basePTS = 0;
	BOOL bFirstPTS = true;
	long long offsetPCR = 0;
	int workCharSizeMode = 0;
	unsigned char workucB = 0;
	unsigned char workucG = 0;
	unsigned char workucR = 0;
	int workCharW = 0;
	int workCharH = 0;
	int workCharHInterval = 0;
	int workCharVInterval = 0;
	int workPosX = 0;
	int workPosY = 0;
	WORD wLastSWFMode = 999;
	int amariPosX = 0;
	int amariPosY = 0;
	int offsetPosX = 0;
	int offsetPosY = 0;
	float ratioX = 2;
	float ratioY = 2;
	FILE *fp3;
	FILE *fp4;
	passType = new TCHAR[256];
	memset(passType, 0, sizeof(TCHAR) * 256);
	passComment1 = new TCHAR[256];
	memset(passComment1, 0, sizeof(TCHAR) * 256);
	passComment2 = new TCHAR[256];
	memset(passComment2, 0, sizeof(TCHAR) * 256);
	passComment3 = new TCHAR[256];
	memset(passComment3, 0, sizeof(TCHAR) * 256);
	passDefaultFontname = new TCHAR[256];
	memset(passDefaultFontname, 0, sizeof(TCHAR) * 256);
	passRubiFontname = new TCHAR[256];
	memset(passRubiFontname, 0, sizeof(TCHAR) * 256);
// mark10als

	SRT_LIST srtList;
	
	BOOL bPrintPMT = TRUE;

//#ifdef _DEBUG
// mark10als
//	argc = 2;
//	argv[1] = _T("C:\\Users\\YourName\\Videos\\sample.ts");
// mark10als
//#endif

	system("cls");

	/*
	 * Parse arguments
	 */

	if (!ParseCmd(argc, argv)) {
		return 1;
	}

	// Initialize Caption Utility.
	CCaptionDllUtil capUtil;

// mark10als
	if (!capUtil.CheckUNICODE() || (format == FORMAT_TAW)) {
		if (capUtil.Initialize() != NO_ERR) {
			_tMyPrintf(_T("Load Caption.dll failed\r\n"));
			return 1;
		}
		bUnicode = FALSE;
	} else {
		if (capUtil.InitializeUNICODE() != NO_ERR) {
			_tMyPrintf(_T("Load Caption.dll failed\r\n"));
			return 1;
		}
		bUnicode = TRUE;
	}
// mark10als
	
	// Initialize ASS filename.
	int result = 1;
	if (pTargetFileName) {
		TCHAR *pExt = PathFindExtension(pTargetFileName);
		result = _tcsicmp(pExt, _T(".ts"));
	}
	if ((!pTargetFileName) || ( result == 0 )) {
		pTargetFileName = new TCHAR[MAX_PATH];
		memset(pTargetFileName, 0, sizeof(TCHAR) * MAX_PATH);

		_tcscat(pTargetFileName, pFileName);

	//	TCHAR *pExt = PathFindExtension(pTargetFileName);

// mark10als
//		if (format == FORMAT_ASS)
//			_tcscpy(pExt, _T(".ass"));
//		else if (format == FORMAT_SRT)
//			_tcscpy(pExt, _T(".srt"));
	}
	if ( (format == FORMAT_ASS) || (format == FORMAT_DUAL) ) {
		TCHAR *pExt = PathFindExtension(pTargetFileName);
		_tcscpy(pExt, _T(".ass"));
	} else {
		TCHAR *pExt = PathFindExtension(pTargetFileName);
		_tcscpy(pExt, _T(".srt"));
// mark10als
	}
// mark10als
	if (format == FORMAT_DUAL) {
		pTargetFileName2 = new TCHAR[MAX_PATH];
		memset(pTargetFileName2, 0, sizeof(TCHAR) * MAX_PATH);
		_tcscat(pTargetFileName2, pTargetFileName);
		TCHAR *pExt = PathFindExtension(pTargetFileName2);
		_tcscpy(pExt, _T(".srt"));
	}
// mark10als

	_tMyPrintf(_T("[Source] %s\r\n"), pFileName);
	printf("[Source] %s\r\n", pFileName);
	_tMyPrintf(_T("[Target] %s\r\n"), pTargetFileName);
	printf("[Target] %s\r\n", pTargetFileName);
	if (format == FORMAT_SRT) {
		_tMyPrintf(_T("[Format] %s\r\n"), _T("srt"));
	}
	else if (format == FORMAT_ASS) {
		_tMyPrintf(_T("[Format] %s\r\n"), _T("ass"));
	}
	else if (format == FORMAT_TAW) {
		_tMyPrintf(_T("[Format] %s\r\n"), _T("srt for TAW"));
		bsrtornament = FALSE;
	}
	else if (format == FORMAT_DUAL) {
		_tMyPrintf(_T("[Format] %s\r\n"), _T("ass & srt"));
	}

	// Open TS File
	FILE *fp = _tfopen(pFileName, _T("rb"));
	if (!fp) {
		_tMyPrintf(_T("Open TS File: %s failed\r\n"), pFileName);
		goto EXIT;
	}

	// Open ASS File
	FILE *fp2 = _tfopen(pTargetFileName, _T("wb"));
	if (!fp2) {
		_tMyPrintf(_T("Open Target File: %s failed\r\n"), pTargetFileName);
		goto EXIT;
	}
// mark10als
//	FILE *fp4 = NULL;
	if (format == FORMAT_DUAL) {
		fp4 = _tfopen(pTargetFileName2, _T("wb"));
		if (!fp4) {
			_tMyPrintf(_T("Open Target File: %s failed\r\n"), pTargetFileName2);
			goto EXIT;
		}
	}
// mark10als

//#ifdef _DEBUG
//	_tcscat(pTargetFileName, _T(".log"));
//	BOOL bLogMode = FALSE;

	if (bLogMode) {
		pLogFileName = new TCHAR[MAX_PATH];
		memset(pLogFileName, 0, sizeof(TCHAR) * MAX_PATH);
		_tcscat(pLogFileName, pTargetFileName);
		TCHAR *pExt = PathFindExtension(pLogFileName);
		_tcscpy(pExt, _T("_Caption.log"));
		fp3 = _tfopen(pLogFileName, _T("wb"));
		if (!fp3) {
			_tMyPrintf(_T("Open Log File: %s failed\r\n"), pLogFileName);
			goto EXIT;
		}
	} else {
		fp3 = FALSE;
	}

//#endif

// mark10als
	// Writes UTF-8 Tag
//	unsigned char tag[] = {0xEF, 0xBB, 0xBF};
//	fwrite(tag, 3, 1, fp2);

	// Writes .ass header & template style
//	if (format == FORMAT_ASS)
//		fprintf(fp2, "%s", ASS_HEADER);

	if ((format == FORMAT_ASS) || (format == FORMAT_DUAL)) {
		IniFileRead(passType);
	}
	if (format == FORMAT_SRT) {
		unsigned char tag[] = {0xEF, 0xBB, 0xBF};
		fwrite(tag, 3, 1, fp2);
	}
	else if (format == FORMAT_ASS) {
		unsigned char tag[] = {0xEF, 0xBB, 0xBF};
		fwrite(tag, 3, 1, fp2);
//		fprintf(fp2, "%s", ASS_HEADER);
		assHeaderWrite(fp2);
	}
	else if (format == FORMAT_DUAL) {
		unsigned char tag[] = {0xEF, 0xBB, 0xBF};
		fwrite(tag, 3, 1, fp2);
//		fprintf(fp2, "%s", ASS_HEADER);
		assHeaderWrite(fp2);
		fwrite(tag, 3, 1, fp4);
	}
	if ((fp3) && (bUnicode)) {
		unsigned char tag[] = {0xEF, 0xBB, 0xBF};
		fwrite(tag, 3, 1, fp3);
	}
// mark10als

	if (!FindStartOffset(fp)) {
		_tMyPrintf(_T("Invalid TS File.\r\n"));
		Sleep(2000);
		goto EXIT;
	}

	BYTE pbPacket[188*2+4] = {0};
	
	// Main loop
	while (fread(pbPacket, 188, 1, fp) == 1) {
		Packet_Header packet;
		void parse_Packet_Header(Packet_Header *packet_header, BYTE *pbPacket);

		parse_Packet_Header(&packet, &pbPacket[0]);

		if (packet.Sync != 'G') {
			if (!resync(pbPacket, fp)) {
				_tMyPrintf(_T("Invalid TS File.\r\n"));
				Sleep(2000);
				goto EXIT;
			}
			continue;
		}

		if (packet.TsErr)
			continue;

		// PAT
		if (packet.PID == 0 && (PMTPid == 0 || bPrintPMT)) {
			void parse_PAT(BYTE *pbPacket);

			parse_PAT(&pbPacket[0]);
			bPrintPMT = FALSE;

			continue; // next packet
		}

		// PMT
		if (PMTPid != 0 && packet.PID == PMTPid) {
// mark10als
			if(0x2b == (pbPacket[5] << 4) + ((pbPacket[6] & 0xf0) >> 4)){
			} else {
				continue; // next packet
			}
// mark10als
			void parse_PMT(BYTE *pbPacket);

			parse_PMT(&pbPacket[0]);
// mark10als
			if (fp3) {
				if (lastPTS == 0) {
					fprintf(fp3, "PMT, PCR, Caption : %04x, %04x, %04x\r\n", PMTPid, PCRPid, CaptionPid);
				}
			}
// mark10als

			continue; // next packet
		}

		long long PCR = 0;

// mark10als
//		if (packet.PID == PCRPid) {
		if (PCRPid != 0 && packet.PID == PCRPid) {
			DWORD bADP = (((DWORD)pbPacket[3] & 0x30) >> 4);
			if (!(bADP & 0x2)) {
				continue; // next packet
			}
			DWORD bAF = (DWORD)pbPacket[5];
			if (!(bAF & 0x10)) {
				continue; // next packet
			}
// mark10als
			/*     90kHz           27kHz
			 *  +--------+-------+-------+
			 *  | 33 bits| 6 bits| 9 bits|
			 *  +--------+-------+-------+
			 */

// mod
//			PCR =(	(DWORD)pbPacket[6] << 25) | 
			PCR =(	(long long)pbPacket[6] << 25) | 
// mod
					((DWORD)pbPacket[7] << 17) | 
					((DWORD)pbPacket[8] << 9) | 
					((DWORD)pbPacket[9] << 1) | 
					((DWORD)pbPacket[10] / 128) ;

			PCR = PCR / 90;

// mark10als
//			if (startPCR == 0) {
//				startPCR = PCR;
//				lastPCR = PCR;
//			}
//			else if (startPCR > 0) {
//				if (startPCR > PCR)
//					PCR += startPCR;

//				else if (PCR > startPCR && (PCR - startPCR) < 500)
//					lastPCR = PCR;
//			}
//#ifdef _DEBUG
			if (fp3) {
				if (lastPTS == 0) {
					fprintf(fp3, "PCR, startPCR, lastPCR, basePCR : %11lld, %11lld, %11lld, %11lld\r\n", PCR, startPCR, lastPCR, basePCR);
				}
			}
//#endif
			if (startPCR == 0) {
				startPCR = PCR - delayTime;
				lastPCR = PCR;
			}
			if ( (PCR > lastPCR) && ((PCR - lastPCR) > 60000) ) {
				startPCR = PCR - delayTime;
				lastPCR = PCR;
			}
			if (PCR < lastPCR) {
				if (fp3) {
					fprintf(fp3, "====== PCR less than lastPCR ======\r\n");
					fprintf(fp3, "PCR, startPCR, lastPCR, basePCR : %11lld, %11lld, %11lld, %11lld\r\n", PCR, startPCR, lastPCR, basePCR);
				}
// mod
//				basePCR = basePCR + lastPCR;
				basePCR = basePCR + (0x1FFFFFFFF / 90);
// mod
				lastPCR = PCR;
			} else {
				lastPCR = PCR;
			}
// mark10als

			continue; // next packet
		}

		if (CaptionPid != 0 && packet.PID == CaptionPid) {
			long long PTS = 0;
			static __int64 lastStamp =0;

			// Get Caption PTS.
			if (packet.PayloadStartFlag) {
				PTS = GetPTS(pbPacket);
// mark10als
//				if (!PTS) {
//					PTS = lastPCR;
//				}
//				if ((abs(PTS - startPCR) - lastStamp) > 1800000) {
//					startPCR = PTS - lastStamp;
//				}
//				PTS = lastPTS;
//	BOOL bFirstPTS = true;
				if (fp3) {
					fprintf(fp3, "PTS, lastPTS, basePTS, startPCR : %11lld, %11lld, %11lld, %11lld    ", PTS, lastPTS, basePTS, startPCR);
				}
// mod
//				if ((PTS > 0) && (lastPTS == 0)) {
//					startPCR = lastPCR - startPCR;
//					startPCR = PTS - startPCR;
				if ((PTS > 0) && (lastPTS == 0) && (PTS < lastPCR)) {
					startPCR = lastPCR - (0x1FFFFFFFF / 90);
// mod
				}
				if (PTS == 0) {
					PTS = lastPTS;
				}
				if (PTS < lastPTS) {
// mod
//					basePTS = basePTS + lastPTS;
					basePTS = basePTS + (0x1FFFFFFFF / 90);
// mod
					lastPTS = PTS;
				} else {
					lastPTS = PTS;
				}
				PTS = PTS + basePTS;
				if ((PTS - startPCR) <= 0) {
					PTS = startPCR;
				}
// mark10als

				unsigned short sH, sM, sS, sMs;

				sMs = (int)(PTS - startPCR) % 1000;
				sS = (int)((PTS - startPCR) / 1000) % 60;
				sM = (int)((PTS - startPCR) / (1000 * 60)) % 60;
				sH = (int)((PTS - startPCR) / (1000 * 60 *60));

				lastStamp = (PTS - startPCR);
				_tMyPrintf(_T("Caption Time: %01d:%02d:%02d.%03d\r\n"), sH, sM, sS, sMs);
// mark10als
//#ifdef _DEBUG
				if (fp3) {
					fprintf(fp3, "1st Caption Time: %01d:%02d:%02d.%03d\r\n", sH, sM, sS, sMs);
				}
//#endif
// mark10als
			}
			else {
				PTS = GetPTS(pbPacket);
// mark10als
//				if (!PTS) {
//					PTS = lastPCR;
//				}
//				if ((abs(PTS - startPCR) - lastStamp) > 1800000) {
//					startPCR = PTS - lastStamp;
//				}
//				PTS = lastPTS;
				if (fp3) {
					fprintf(fp3, "PTS, lastPTS, basePTS, startPCR : %11lld, %11lld, %11lld, %11lld    ", PTS, lastPTS, basePTS, startPCR);
				}
				if (!PTS) {
					PTS = lastPTS;
				}
				PTS = PTS + basePTS;
// mark10als
// mark10als
//#ifdef _DEBUG
				unsigned short sH, sM, sS, sMs;

				sMs = (int)(PTS - startPCR) % 1000;
				sS = (int)((PTS - startPCR) / 1000) % 60;
				sM = (int)((PTS - startPCR) / (1000 * 60)) % 60;
				sH = (int)((PTS - startPCR) / (1000 * 60 *60));

				if (fp3) {
					fprintf(fp3, "2nd Caption Time: %01d:%02d:%02d.%03d\r\n", sH, sM, sS, sMs);
				}
//#endif
// mark10als
			}

// mark10als
//			lastPTS = PTS;
// mark10als

			// parse caption;

			int ret = capUtil.AddTSPacket(pbPacket);

			if (ret == CHANGE_VERSION) {
				std::vector<LANG_TAG_INFO> tagInfoList;
				ret = capUtil.GetTagInfo(&tagInfoList);
			}
			else if (ret == NO_ERR_CAPTION) {

				std::vector<CAPTION_DATA> Captions;
				ret = capUtil.GetCaptionData(0, &Captions);

				std::vector<CAPTION_DATA>::iterator it = Captions.begin();
				for(;it != Captions.end(); it++) {
					CHAR strUTF8[1024] = {0};

					if (it->bClear) {
						if (format == FORMAT_ASS)
							DumpAssLine(fp2, &srtList, (PTS + it->dwWaitTime) - startPCR);
						else if (format == FORMAT_SRT)
							DumpSrtLine(fp2, &srtList, (PTS + it->dwWaitTime) - startPCR);
// mark10als
						else if (format == FORMAT_TAW)
							DumpSrtLine(fp2, &srtList, (PTS + it->dwWaitTime) - startPCR);
						else if (format == FORMAT_DUAL) {
							DumpAssLine(fp2, &srtList, (PTS + it->dwWaitTime) - startPCR);
							DumpSrtLine(fp4, &srtList, (PTS + it->dwWaitTime) - startPCR);
						}
						srtList.clear();
// mark10als

						continue;
					}
					else {

						std::vector<CAPTION_CHAR_DATA>::iterator it2 = it->CharList.begin();
// mark10als
//#ifdef _DEBUG
						if (fp3) {
							fprintf(fp3, "SWFMode    : %4d\r\n", it->wSWFMode);
							fprintf(fp3, "Client X:Y : %4d\t%4d\r\n", it->wClientX, it->wClientY);
							fprintf(fp3, "Client W:H : %4d\t%4d\r\n", it->wClientW, it->wClientH);
							fprintf(fp3, "Pos    X:Y : %4d\t%4d\r\n", it->wPosX, it->wPosY);
						}
//#endif
						if (it->wSWFMode != wLastSWFMode) {
							wLastSWFMode = it->wSWFMode;
							if (wLastSWFMode == 5) {
								ratioX = (float)(assPlayResX) / (float)(1920);
								ratioY = (float)(assPlayResY) / (float)(1080);
							}
							else if (wLastSWFMode == 9) {
								ratioX = (float)(assPlayResX) / (float)(720);
								ratioY = (float)(assPlayResY) / (float)(480);
							}
							else if (wLastSWFMode == 11) {
								ratioX = (float)(assPlayResX) / (float)(1280);
								ratioY = (float)(assPlayResY) / (float)(720);
							}
							else {
								ratioX = (float)(assPlayResX) / (float)(960);
								ratioY = (float)(assPlayResY) / (float)(540);
							}
						}
						if (bUnicode) {
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

// mark10als
						for(;it2 != it->CharList.end(); it2++) {
// mark10als
							workCharSizeMode = it2->emCharSizeMode;
							workucR = it2->stCharColor.ucR;
							workucG = it2->stCharColor.ucG;
							workucB = it2->stCharColor.ucB;
							workCharW = it2->wCharW;
							workCharH = it2->wCharH;
							workCharHInterval = it2->wCharHInterval;
							workCharVInterval = it2->wCharVInterval;
							if (!bUnicode) {
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
							if (wLastSWFMode == 0) {
								workPosX = (int)((float)( it->wPosX + offsetPosX ) * ratioX);
								workPosY = (int)((float)( it->wPosY + offsetPosY + assSWF0offset ) * ratioY);
							} else if (wLastSWFMode == 5) {
								workPosX = (int)((float)( it->wPosX + offsetPosX ) * ratioX);
								workPosY = (int)((float)( it->wPosY + offsetPosY - 0 + assSWF5offset ) * ratioY);
							} else if (wLastSWFMode == 7) {
								workPosX = (int)((float)( it->wPosX + offsetPosX ) * ratioX);
								workPosY = (int)((float)( it->wPosY + offsetPosY +0 + assSWF7offset ) * ratioY);
							} else if (wLastSWFMode == 9) {
								workPosX = (int)((float)( it->wPosX + offsetPosX ) * ratioX);
								if (bUnicode) {
									workPosY = (int)((float)( it->wPosY + offsetPosY + assSWF9offset ) * ratioY);
								} else {
									workPosY = (int)((float)( it->wPosY + offsetPosY -50 + assSWF9offset ) * ratioY);
								}
							} else if (wLastSWFMode == 11) {
								workPosX = (int)((float)( it->wPosX + offsetPosX ) * ratioX);
								workPosY = (int)((float)( it->wPosY + offsetPosY - 0 + assSWF11offset ) * ratioY);
							} else {
								workPosX = it->wPosX + offsetPosX;
								workPosY = it->wPosY + offsetPosY;
							}
// mark10als
// mark10als
							// ‚Ó‚è‚ª‚È Skip
//							if (it2->emCharSizeMode == STR_SMALL)
//								continue;
							// ‚Ó‚è‚ª‚È Skip ‚Í o—ÍŽž‚É
							if ((it2->emCharSizeMode == STR_SMALL) &&  (!bUnicode)) {
								workPosY += (int)(10 * ratioY);
							}
							if (( it2->emCharSizeMode == STR_MEDIUM ) &&  (!bUnicode)){
								// ‘SŠp -> ”¼Šp
								it2->strDecode = GetHalfChar(it2->strDecode);
							}
// mark10als
//#ifdef _DEBUG
// mark10als
//							if (fp3)
//								fprintf(fp3, "%s\r\n", it2->strDecode.c_str());
							if (fp3) {
								fprintf(fp3, "Color : %#.X   ", it2->stCharColor);
								fprintf(fp3, "Char M,W,H,HI,VI : %4d, %4d, %4d, %4d, %4d   ", it2->emCharSizeMode ,it2->wCharW, it2->wCharH,  it2->wCharHInterval,  it2->wCharVInterval);
								fprintf(fp3, "%s\r\n", it2->strDecode.c_str());
							}
// mark10als
//#endif

							WCHAR str[1024] = {0};
							CHAR strUTF8_2[1024] = {0};
							
// mark10als
//							// CP 932 to UTF-8
//							MultiByteToWideChar(932, 0, it2->strDecode.c_str(), -1, str, 1024);
//							WideCharToMultiByte(CP_UTF8, 0, str, -1, strUTF8_2, 1024, NULL, NULL);

//							strcat(strUTF8, strUTF8_2);
							if ((format == FORMAT_TAW) || (bUnicode)) {
								strcat(strUTF8, it2->strDecode.c_str());
							} else {
								// CP 932 to UTF-8
								MultiByteToWideChar(932, 0, it2->strDecode.c_str(), -1, str, 1024);
								WideCharToMultiByte(CP_UTF8, 0, str, -1, strUTF8_2, 1024, NULL, NULL);

								strcat(strUTF8, strUTF8_2);
							}
// mark10als
						}

						PSRT_LINE pSrtLine = new SRT_LINE();
						pSrtLine->index = 0;	//useless
						pSrtLine->startTime = (DWORD)(PTS - startPCR);
						pSrtLine->endTime = 0;
// mark10als
						pSrtLine->outCharSizeMode = workCharSizeMode;
						pSrtLine->outCharColor.ucR = workucR;
						pSrtLine->outCharColor.ucG = workucG;
						pSrtLine->outCharColor.ucB = workucB;
						pSrtLine->outCharW = workCharW;
						pSrtLine->outCharH = workCharH;
						pSrtLine->outCharHInterval = workCharHInterval;
						pSrtLine->outCharVInterval = workCharVInterval;
						pSrtLine->outPosX = workPosX;
						pSrtLine->outPosY = workPosY;
						pSrtLine->outornament = bsrtornament;
// mark10als
						pSrtLine->str = strUTF8;
						if (pSrtLine->str == ""){
							delete pSrtLine;
							continue;
						}

						srtList.push_back(pSrtLine);
					} //if (it->bClear)

				} // for(;it != Captions.end(); it++)

			} //else if (ret == NO_ERR_CAPTION)

		} //if (CaptionPid != 0 && usPID == CaptionPid)

	} //while(fread(pbPacket, 188, 1, fp) == 1)

EXIT:
	if (fp)
		fclose(fp);

	if (fp2)
		fclose(fp2);

// mark10als
	if (format == FORMAT_DUAL) {
		if (fp4)
			fclose(fp4);
	}
	if ((assIndex == 1) && (srtIndex == 1)){
		Sleep(2000);
		remove( pTargetFileName );
		if (format == FORMAT_DUAL) {
			remove( pTargetFileName2 );
		}
	}
// mark10als

//#ifdef _DEBUG
	if (bLogMode) {
		if (fp3)
			fclose(fp3);
	}
//#endif
	//	Sleep(20000);
	return 0;
}
