//------------------------------------------------------------------------------
// Caption2Ass_PCR.h
//------------------------------------------------------------------------------
#ifndef __CAPTION2ASS_PCR_H__
#define __CAPTION2ASS_PCR_H__

#include <tchar.h>

#include "Caption2AssParameter.h"

extern void assHeaderWrite(FILE *, ass_setting_t *);
extern int IniFileRead(TCHAR *, ass_setting_t *);

enum {
    FORMAT_INVALID = 0,
    FORMAT_SRT     = 1,
    FORMAT_ASS     = 2,
    FORMAT_TAW     = 3,
    FORMAT_DUAL    = 4
};

enum {
    HLC_INVALID = 0,
    HLC_kigou   = 1,
    HLC_box     = 2,
    HLC_draw    = 3
};

#endif // __CAPTION2ASS_PCR_H__
