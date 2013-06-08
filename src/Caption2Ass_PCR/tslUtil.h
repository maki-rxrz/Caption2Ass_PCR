//------------------------------------------------------------------------------
// tslUtil.h
//------------------------------------------------------------------------------
#ifndef __TSL_UTIL_H__
#define __TSL_UTIL_H__

#include <stdlib.h>

#include "packet_types.h"

extern BOOL FindStartOffset(FILE *);
extern BOOL resync(BYTE *, FILE *);

extern void parse_PAT(BYTE *, USHORT *);
extern void parse_PMT(BYTE *, USHORT *, USHORT *);
extern long long GetPTS(BYTE *);
extern void parse_Packet_Header(Packet_Header *, BYTE *);

#endif // __TSL_UTIL_H__
