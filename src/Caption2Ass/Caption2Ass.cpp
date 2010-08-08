// Caption2SRT.cpp : Defines the entry point for the console application.
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
#include "CaptionDllUtil.h"
#include "ass_header.h"

enum {
	FORMAT_SRT = 1,
	FORMAT_ASS = 2
};

typedef struct _SRT_LINE {
	UINT				index;
	DWORD				startTime;
	DWORD				endTime;
	std::string			str;
} SRT_LINE, *PSRT_LINE;

typedef std::list<PSRT_LINE> SRT_LIST;

VOID _tMyPrintf(IN	LPCTSTR tracemsg, ...);
BOOL ParseCmd(int argc, char**argv);
BOOL FindStartOffset(FILE *fp);
BOOL resync(BYTE *pbPacket, FILE *fp);
long long GetPTS(BYTE *pbPacket);


void DumpAssLine(FILE *fp, SRT_LIST * list, long long PTS)
{
	SRT_LIST::iterator it = list->begin();
	for(int i = 0; it != list->end(); it++, i++) {

		if (i == 0) {
			(*it)->endTime = PTS;

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
			fprintf(fp,"Dialogue: 0,%01d:%02d:%02d.%02d,%01d:%02d:%02d.%02d,Default,,0000,0000,0000,,", sH, sM, sS, sMs, eH, eM, eS, eMs);
		}

		fwrite((*it)->str.c_str(), (*it)->str.size(),1, fp);
		fprintf(fp, "\\N");

		delete (*it);
	}

	if (list->size() > 0)
		fprintf(fp, "\r\n");

	list->clear();
}

DWORD srtIndex = 1; // index for SRT
void DumpSrtLine(FILE *fp, SRT_LIST * list, long long PTS)
{
	SRT_LIST::iterator it = list->begin();
	for(int i = 0; it != list->end(); it++, i++) {

		if (i == 0) {
			(*it)->endTime = PTS;

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

		fwrite((*it)->str.c_str(), (*it)->str.size(),1, fp);
		fprintf(fp, "\r\n");

		delete (*it);
	}

	if (list->size() > 0) {
		fprintf(fp, "\r\n");
		srtIndex++;
	}

	list->clear();
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

	SRT_LIST srtList;
	
	BOOL bPrintPMT = TRUE;

#ifdef _DEBUG
	argc = 2;
	argv[1] = _T("C:\\Users\\YourName\\Videos\\sample.ts");
#endif

	system("cls");

	/*
	 * Parse arguments
	 */

	if (!ParseCmd(argc, argv)) {
		return 1;
	}

	// Initialize Caption Utility.
	CCaptionDllUtil capUtil;

	if (capUtil.Initialize() != NO_ERR) {
		_tMyPrintf(_T("Load Caption.dll failed\r\n"));
		return 1;
	}
	
	// Initialize ASS filename.
	if (!pTargetFileName) {
		pTargetFileName = new TCHAR[MAX_PATH];
		memset(pTargetFileName, 0, sizeof(TCHAR) * MAX_PATH);

		_tcscat(pTargetFileName, pFileName);

		TCHAR *pExt = PathFindExtension(pTargetFileName);

		if (format == FORMAT_ASS)
			_tcscpy(pExt, _T(".ass"));
		else if (format == FORMAT_SRT)
			_tcscpy(pExt, _T(".srt"));
	}

	_tMyPrintf(_T("[Source] %s\r\n\r\n"), pFileName);
	_tMyPrintf(_T("[Target] %s\r\n\r\n"), pTargetFileName);
	if (format == FORMAT_SRT) {
		_tMyPrintf(_T("[Format] %s\r\n\r\n"), _T("srt"));
	}
	else if (format == FORMAT_ASS) {
		_tMyPrintf(_T("[Format] %s\r\n\r\n"), _T("ass"));
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
		_tMyPrintf(_T("Open .ASS File: %s failed\r\n"), pTargetFileName);
		goto EXIT;
	}

#ifdef _DEBUG
	_tcscat(pTargetFileName, _T(".log"));
	FILE *fp3 = _tfopen(pTargetFileName, _T("wb"));

#endif

	// Writes UTF-8 Tag
	unsigned char tag[] = {0xEF, 0xBB, 0xBF};
	fwrite(tag, 3, 1, fp2);

	// Writes .ass header & template style
	if (format == FORMAT_ASS)
		fprintf(fp2, "%s", ASS_HEADER);

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
			void parse_PMT(BYTE *pbPacket);

			parse_PMT(&pbPacket[0]);

			continue; // next packet
		}

		long long PCR = 0;

		if (packet.PID == PCRPid) {
			/*     90kHz           27kHz
			 *  +--------+-------+-------+
			 *  | 33 bits| 6 bits| 9 bits|
			 *  +--------+-------+-------+
			 */

			PCR =(	(DWORD)pbPacket[6] << 25) | 
					((DWORD)pbPacket[7] << 17) | 
					((DWORD)pbPacket[8] << 9) | 
					((DWORD)pbPacket[9] << 1) | 
					((DWORD)pbPacket[10] / 128) ;

			PCR = PCR / 90;

			if (startPCR == 0) {
				startPCR = PCR;
				lastPCR = PCR;
			}
			else if (startPCR > 0) {
				if (startPCR > PCR)
					PCR += startPCR;

				else if (PCR > startPCR && (PCR - startPCR) < 500)
					lastPCR = PCR;
			}

			continue; // next packet
		}

		if (CaptionPid != 0 && packet.PID == CaptionPid) {
			long long PTS = 0;
			static __int64 lastStamp =0;

			// Get Caption PTS.
			if (packet.PayloadStartFlag) {
				PTS = GetPTS(pbPacket);
				if (!PTS) {
					PTS = lastPCR;
				}
				if ((abs(PTS - startPCR) - lastStamp) > 1800000) {
					startPCR = PTS - lastStamp;
				}

				unsigned short sH, sM, sS, sMs;

				sMs = (int)(PTS - startPCR) % 1000;
				sS = (int)((PTS - startPCR) / 1000) % 60;
				sM = (int)((PTS - startPCR) / (1000 * 60)) % 60;
				sH = (int)((PTS - startPCR) / (1000 * 60 *60));

				lastStamp = (PTS - startPCR);
				_tMyPrintf(_T("Caption Time: %01d:%02d:%02d.%03d\r\n"), sH, sM, sS, sMs);
			}
			else {
				PTS = GetPTS(pbPacket);
				if (!PTS) {
					PTS = lastPCR;
				}
				if ((abs(PTS - startPCR) - lastStamp) > 1800000) {
					startPCR = PTS - lastStamp;
				}
				PTS = lastPTS;
			}

			lastPTS = PTS;

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

						continue;
					}
					else {

						std::vector<CAPTION_CHAR_DATA>::iterator it2 = it->CharList.begin();
						for(;it2 != it->CharList.end(); it2++) {
							// ‚Ó‚è‚ª‚È Skip
							if (it2->emCharSizeMode == STR_SMALL)
								continue;
#ifdef _DEBUG
							if (fp3)
								fprintf(fp3, "%s\r\n", it2->strDecode.c_str());
#endif

							WCHAR str[1024] = {0};
							CHAR strUTF8_2[1024] = {0};
							
							// CP 932 to UTF-8
							MultiByteToWideChar(932, 0, it2->strDecode.c_str(), -1, str, 1024);
							WideCharToMultiByte(CP_UTF8, 0, str, -1, strUTF8_2, 1024, NULL, NULL);

							strcat(strUTF8, strUTF8_2);
						}

						PSRT_LINE pSrtLine = new SRT_LINE();
						pSrtLine->index = 0;	//useless
						pSrtLine->startTime = PTS - startPCR;
						pSrtLine->endTime = 0;
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

#ifdef _DEBUG
	if (fp3)
		fclose(fp3);
#endif
	return 0;
}
