#ifndef TIMEDIFF_SETTINGS_H
#define TIMEDIFF_SETTINGS_H

#include "includes.h"

#define FLAG_WARN_ONLY        1
#define FLAG_NANOSECS         2
#define FLAG_INDENT_CONTINUES 4
#define FLAG_NONUM_CONTINUES  8
#define FLAG_FROM_START      16

typedef struct
{
    int Flags;
    char *Input;
    double WarnTime;
    double ClipMin;
    double ClipMax;
    char *WarnPrefix;
    char *WarnPostfix;
    char *NormPrefix;
    char *NormPostfix;
    char *StartStrings;
    char *EndStrings;
    char *Includes;
    char *Excludes;
    char *Summaries;
} TSettings;


extern TSettings Settings;

void SettingsInit(int argc, const char *argv[]);

#endif
