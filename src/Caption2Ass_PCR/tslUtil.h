//------------------------------------------------------------------------------
// tslUtil.h
//------------------------------------------------------------------------------
#ifndef __TSL_UTIL_H__
#define __TSL_UTIL_H__

#include <stdlib.h>

#include "file_reader.h"
#include "packet_types.h"

extern BOOL FindStartOffset(IFileReader *fr);
extern BOOL resync(BYTE *pbPacket, IFileReader *fr);

extern long long GetPTS(BYTE *pbPacket);
extern void parse_PAT(BYTE *pbPacket, USHORT *PMTPid);
extern void parse_PMT(BYTE *pbPacket, USHORT *PCRPid, USHORT *CaptionPid);
extern void parse_Packet_Header(Packet_Header *packet_header, BYTE *pbPacket);

#endif // __TSL_UTIL_H__
