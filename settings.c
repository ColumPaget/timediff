#include "settings.h"
#include "help.h"

TSettings Settings;

static double SettingsParseDuration(const char *Str)
{
    double val;
    char *ptr;

    val=strtod(Str, &ptr);
    if (ptr)
    {
        switch (*ptr)
        {
        case 'c':
            val=val/100.0;
            break;
        case 'm':
            val=val/1000.0;
            break;
        }
    }

    return(val);
}


void SettingsInit(int argc, const char *argv[])
{
    CMDLINE *Cmd;
    const char *item;

    memset(&Settings, 0, sizeof(TSettings));
    Settings.Input=CopyStr(Settings.Input, "-");

    if (isatty(1))
    {
        Settings.WarnPrefix=CopyStr(Settings.WarnPrefix, "\x1b[1m");
        Settings.WarnPostfix=CopyStr(Settings.WarnPostfix, "\x1b[m");
        Settings.NormPrefix=CopyStr(Settings.NormPrefix, "");
        Settings.NormPostfix=CopyStr(Settings.NormPostfix, "");
    }
    else
    {
        Settings.WarnPrefix=CopyStr(Settings.WarnPrefix, "");
        Settings.WarnPostfix=CopyStr(Settings.WarnPostfix, "!");
        Settings.NormPrefix=CopyStr(Settings.NormPrefix, "");
        Settings.NormPostfix=CopyStr(Settings.NormPostfix, " ");
    }

    Settings.WarnTime=-1;
    Settings.ClipMin=-1;
    Settings.ClipMax=-1;

    Cmd=CommandLineParserCreate(argc, argv);
    item=CommandLineNext(Cmd);
    while (item)
    {
        if (strcmp(item, "-min")==0) Settings.ClipMin=SettingsParseDuration(CommandLineNext(Cmd));
        else if (strcmp(item, "-max")==0) Settings.ClipMax=SettingsParseDuration(CommandLineNext(Cmd));
        else if (strcmp(item, "-indent")==0) Settings.Flags |= FLAG_INDENT_CONTINUES;
        else if (strcmp(item, "-I")==0) Settings.Flags |= FLAG_INDENT_CONTINUES;
        else if (strcmp(item, "-multiline")==0) Settings.Flags |= FLAG_NONUM_CONTINUES;
        else if (strcmp(item, "-M")==0) Settings.Flags |= FLAG_NONUM_CONTINUES;
        else if (strcmp(item, "-from-start")==0) Settings.Flags |= FLAG_FROM_START;
        else if (strcmp(item, "-fs")==0) Settings.Flags |= FLAG_FROM_START;
        else if (strcmp(item, "-F")==0) Settings.Flags |= FLAG_FROM_START;
        else if (strcmp(item, "-w")==0) Settings.WarnTime=SettingsParseDuration(CommandLineNext(Cmd));
        else if (strcmp(item, "-W")==0) Settings.Flags=FLAG_WARN_ONLY;
        else if (strcmp(item, "-s")==0) Settings.StartStrings=MCatStr(Settings.StartStrings, CommandLineNext(Cmd), "\x02", NULL);
        else if (strcmp(item, "-e")==0) Settings.EndStrings=MCatStr(Settings.EndStrings, CommandLineNext(Cmd), "\x02", NULL);
        else if (strcmp(item, "-i")==0) Settings.Includes=MCatStr(Settings.Includes, CommandLineNext(Cmd), "\x02", NULL);
        else if (strcmp(item, "-x")==0) Settings.Excludes=MCatStr(Settings.Excludes, CommandLineNext(Cmd), "\x02", NULL);
        else if (strcmp(item, "-S")==0) Settings.Summaries=MCatStr(Settings.Summaries, CommandLineNext(Cmd), "\x02", NULL);
        else if (strcmp(item, "-?")==0) Help();
        else if (strcmp(item, "-help")==0) Help();
        else if (strcmp(item, "--help")==0) Help();
        else if (strcmp(item, "-version")==0) Version();
        else if (strcmp(item, "--version")==0) Version();
        else Settings.Input=CopyStr(Settings.Input, item);
        item=CommandLineNext(Cmd);
    }

}

