#include "common.h"
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




void ProcessTimedItem(struct timeval *When, double Secs, const char *Line)
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
        if ((Settings.WarnTime > -1) && (Secs > Settings.WarnTime))
        {
            StatsInsert("over-warn", When, Secs);
            prefix=Settings.WarnPrefix;
            postfix=Settings.WarnPostfix;
        }
        else
        {
            prefix=Settings.NormPrefix;
            postfix=Settings.NormPostfix;
        }

        if (Settings.Flags & FLAG_NANOSECS) printf("%s%14.6f%s  %s\n", prefix, Secs, postfix, Line);
        else printf("%s%10.3f%s  %s\n", prefix, Secs, postfix, Line);

        StatsInsert("default", When, Secs);

        if (InPatternList(Line, Settings.Summaries, &Tempstr)) StatsInsert(Tempstr, When, Secs);
        if (InPatternList(Line, Settings.EndStrings, NULL)) Started=FALSE;
    }

    Destroy(Tempstr);
}




double ProcessLine(struct timeval *Prev, struct timeval *Curr, const char *Line)
{
    double secs=0;

    secs=CalcTimediff(Prev, Curr);
    if (ItemIncluded(Line, secs)) ProcessTimedItem(Curr, secs, Line);

    return(secs);
}



int main(int argc, const char *argv[])
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

    return(0);
}
