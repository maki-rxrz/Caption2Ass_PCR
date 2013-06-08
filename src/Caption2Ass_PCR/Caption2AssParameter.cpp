//------------------------------------------------------------------------------
// Caption2AssParameter.cpp
//------------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "CaptionDef.h"
#include "Caption2Ass_PCR.h"

#define CLI_PARAMETER_STRING_NUMS   (5)
#define ASS_SETTING_STRING_NUMS     (9)
#define PARAMETER_STRING_NUMS       (CLI_PARAMETER_STRING_NUMS + ASS_SETTING_STRING_NUMS)

CCaption2AssParameter::CCaption2AssParameter(void)
 : string_length(MAX_PATH)
{
    Initialize();
}

CCaption2AssParameter::CCaption2AssParameter(size_t _string_length)
 : string_length(_string_length)
{
    Initialize();
}

CCaption2AssParameter::~CCaption2AssParameter(void)
{
    Free();
}

void CCaption2AssParameter::Initialize(void)
{
    pid_information_t *pi = &pid_information;
    cli_parameter_t   *cp = &cli_parameter;
    ass_setting_t     *as = &ass_setting;

    // Initialize
    memset(pi, 0, sizeof(pid_information_t));
    memset(cp, 0, sizeof(cli_parameter_t));
    memset(as, 0, sizeof(ass_setting_t));

    // Setup default settings.
    cp->format  = FORMAT_ASS;
    cp->HLCmode = HLC_kigou;
    if (string_length < MAX_PATH)
        string_length = MAX_PATH;
}

int CCaption2AssParameter::Allocate(size_t _string_length)
{
    cli_parameter_t *cp = &cli_parameter;
    ass_setting_t   *as = &ass_setting;

    if (string_length < _string_length)
        string_length = _string_length;

    // Allocate string buffers.
    TCHAR **string_list[PARAMETER_STRING_NUMS + 1] = {
        // cli_parameter_t
        &(cp->ass_type),
        &(cp->FileName),
        &(cp->TargetFileName1),
        &(cp->TargetFileName2),
        &(cp->LogFileName),
        // ass_setting_t
        &(as->Comment1),
        &(as->Comment2),
        &(as->Comment3),
        &(as->DefaultFontname),
        &(as->DefaultStyle),
        &(as->BoxFontname),
        &(as->BoxStyle),
        &(as->RubiFontname),
        &(as->RubiStyle),
        // last(null)
        NULL
    };
    for (int i = 0; i < PARAMETER_STRING_NUMS; i++) {
        *(string_list[i]) = new TCHAR[string_length];
        if (!*(string_list[i]))
            goto fail;
        memset(*(string_list[i]), 0, sizeof(TCHAR) * string_length);
    }

    return 0;

fail:
    Free();
    return -1;
}

void CCaption2AssParameter::Free(void)
{
    cli_parameter_t *cp = &cli_parameter;
    ass_setting_t   *as = &ass_setting;

    // Free string buffers.
    TCHAR **string_list[PARAMETER_STRING_NUMS + 1] = {
        // cli_parameter_t
        &(cp->ass_type),
        &(cp->FileName),
        &(cp->TargetFileName1),
        &(cp->TargetFileName2),
        &(cp->LogFileName),
        // ass_setting_t
        &(as->Comment1),
        &(as->Comment2),
        &(as->Comment3),
        &(as->DefaultFontname),
        &(as->DefaultStyle),
        &(as->BoxFontname),
        &(as->BoxStyle),
        &(as->RubiFontname),
        &(as->RubiStyle),
        // last(null)
        NULL
    };
    for (int i = 0; i < PARAMETER_STRING_NUMS; i++)
        if (*(string_list[i]))
            SAFE_DELETE_ARRAY(*(string_list[i]));
}
