//------------------------------------------------------------------------------
// Caption2AssParameter.h
//------------------------------------------------------------------------------
#ifndef __CAPTION2ASS_PARAMETER_H__
#define __CAPTION2ASS_PARAMETER_H__

#include <tchar.h>

typedef struct {
    USHORT      PMTPid;
    USHORT      CaptionPid;
    USHORT      PCRPid;
} pid_information_t;

typedef struct {
    DWORD       format;
    long        DelayTime;
    BOOL        keepInterval;
    BYTE        HLCmode;
    BOOL        srtornament;
    BOOL        norubi;
    BOOL        LogMode;
    DWORD       detectLength;
    TCHAR      *ass_type;
    TCHAR      *FileName;
    TCHAR      *TargetFileName1;
    TCHAR      *TargetFileName2;
    TCHAR      *LogFileName;
} cli_parameter_t;

typedef struct {
    long        SWF0offset;
    long        SWF5offset;
    long        SWF7offset;
    long        SWF9offset;
    long        SWF11offset;
    TCHAR      *Comment1;
    TCHAR      *Comment2;
    TCHAR      *Comment3;
    long        PlayResX;
    long        PlayResY;
    TCHAR      *DefaultFontname;
    long        DefaultFontsize;
    TCHAR      *DefaultStyle;
    TCHAR      *BoxFontname;
    long        BoxFontsize;
    TCHAR      *BoxStyle;
    TCHAR      *RubiFontname;
    long        RubiFontsize;
    TCHAR      *RubiStyle;
} ass_setting_t;

class CCaption2AssParameter
{
public:
    size_t              string_length;

protected:
    pid_information_t   pid_information;
    cli_parameter_t     cli_parameter;
    ass_setting_t       ass_setting;

protected:
    void Initialize(void);
    void Free(void);

public:
    CCaption2AssParameter(void);
    CCaption2AssParameter(size_t _string_length);
    ~CCaption2AssParameter(void);

    int Allocate(size_t string_length = 0);

    pid_information_t *get_pid_information(void){ return &pid_information; }
    cli_parameter_t *get_cli_parameter(void){ return &cli_parameter; }
    ass_setting_t *get_ass_setting(void){ return &ass_setting; }
};

#endif // __CAPTION2ASS_PARAMETER_H__
