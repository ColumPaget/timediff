#include "includes.h"
#include "stats.h"
#include "parse.h"
#include "settings.h"

int Started=FALSE;




int InPatternList(const char *Str, const char *List, char **MatchingPattern)
{
    char *Pattern=NULL;
    const char *ptr;
    int result=FALSE;

    ptr=GetToken(List, "\x02", &Pattern, 0);
    while (ptr)
    {
        if (fnmatch(Pattern, Str, 0)==0)
        {
            if (MatchingPattern) *MatchingPattern=CopyStr(*MatchingPattern, Pattern);
            result=TRUE;
            break;
        }
        ptr=GetToken(ptr, "\x02", &Pattern, 0);
    }

    Destroy(Pattern);
    return(result);
}



int ItemIncluded(const char *Line, double secs)
{
//if we specified a minimum seconds, and this item is less than that, then don't include this item
    if (secs < Settings.ClipMin) return(FALSE);
//if we specified a maximum seconds, and this item is greater than that, then don't include
    if  ((Settings.ClipMax > -1) && (secs > Settings.ClipMax)) return(FALSE);
//if we specified we only want to see items that take longer than the warning time, then exclude those that dont
    if ((Settings.Flags & FLAG_WARN_ONLY) && (secs < Settings.WarnTime)) return(FALSE);
    if ( StrValid(Settings.Includes) && (! InPatternList(Line, Settings.Includes, NULL)) ) return(FALSE);
    if ( StrValid(Settings.Excludes) && (InPatternList(Line, Settings.Excludes, NULL)) ) return(FALSE);
    return(TRUE);
}




void ProcessTimedItem(double secs, const char *Line)
{
    const char *prefix="", *postfix="";
    char *Tempstr=NULL;

    if (InPatternList(Line, Settings.StartStrings, NULL))
    {
        //if we are in 'warn only' mode, where we only display items that have a time warning
        //then we want a 'start string' that matches the start of an interaction to be registered as the start time
        //but we don't want it displayed
        if (! (Settings.Flags & FLAG_WARN_ONLY)) printf("%s%10s%s  %s\n", Settings.NormPrefix, " -- ", Settings.NormPostfix, Line);
        Started=TRUE;
    }
    else if (Started)
    {
        if ((Settings.WarnTime > -1) && (secs > Settings.WarnTime))
        {
            StatsInsert("over-warn", secs);
            prefix=Settings.WarnPrefix;
            postfix=Settings.WarnPostfix;
        }
        else
        {
            prefix=Settings.NormPrefix;
            postfix=Settings.NormPostfix;
        }

	if (Settings.Flags & FLAG_NANOSECS) printf("%s%14.6f%s  %s\n", prefix, secs, postfix, Line);
        else printf("%s%10.3f%s  %s\n", prefix, secs, postfix, Line);

        StatsInsert("default", secs);
        if (InPatternList(Line, Settings.Summaries, &Tempstr)) StatsInsert(Tempstr, secs);
        if (InPatternList(Line, Settings.EndStrings, NULL)) Started=FALSE;
    }

    Destroy(Tempstr);
}



void ProcessLine(struct timeval *Prev, struct timeval *Curr, const char *Line)
{
    double secs=0, usec=0;

    if (Prev->tv_sec > 0)
    {
        //calcluate secs as a floating point value with millisecond resolution
        secs=difftime(Curr->tv_sec, Prev->tv_sec);
        if (Curr->tv_usec < Prev->tv_usec) usec=1000000 - Prev->tv_usec + Curr->tv_usec;
        else usec=Curr->tv_usec - Prev->tv_usec;

        secs += usec / 1000000;

    }

    if (ItemIncluded(Line, secs)) ProcessTimedItem(secs, Line);
}





main(int argc, const char *argv[])
{
    STREAM *S;
    char *Tempstr=NULL;
    struct timeval Prev, Curr;

    SettingsInit(argc, argv);
    if (! StrValid(Settings.StartStrings)) Started=TRUE;

    //clear down Prev to be sure it doesn't have nonsense data in it
    memset(&Prev, 0, sizeof(struct timeval));

    //input can be a file, stdin, or even a url
    S=STREAMOpen(Settings.Input, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);
            if (StrValid(Tempstr))
            {
                ParseDateTime(Tempstr, &Curr);
                ProcessLine(&Prev, &Curr, Tempstr);
                memcpy(&Prev, &Curr, sizeof(struct timeval));
            }
            Tempstr=STREAMReadLine(Tempstr, S);
        }
        //Display stats before closing stream,
        //because the stream might be stdin/stdout
        //and older versions of libUseful will close those file descrpitors
        //preventing stats from being displayed
        StatsDisplay();

        STREAMClose(S);

    }

}
