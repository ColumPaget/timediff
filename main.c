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



#define ITEM_NORMAL 0
#define ITEM_START  -1
#define ITEM_END    -2

int ProcessTimedItem(struct timeval *When, double Secs, const char *Line)
{
    const char *prefix="", *postfix="";
    char *Tempstr=NULL;
    int RetVal=ITEM_NORMAL;

    if (Started)
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
        if (InPatternList(Line, Settings.EndStrings, NULL))
        {
            Started=FALSE;
            RetVal=ITEM_END;
        }
    }

    Destroy(Tempstr);

    return(RetVal);
}




double ProcessLine(struct timeval *Prev, struct timeval *Curr, const char *Line)
{
    double secs=0;
    int result;

    secs=CalcTimediff(Prev, Curr);
    if (InPatternList(Line, Settings.StartStrings, NULL))
    {
        //if we are in 'warn only' mode, where we only display items that have a time warning
        //then we want a 'start string' that matches the start of an interaction to be registered as the start time
        //but we don't want it displayed
        if (! (Settings.Flags & FLAG_WARN_ONLY)) printf("%s%10s%s  %s\n", Settings.NormPrefix, " -- ", Settings.NormPostfix, Line);
        Started=TRUE;
        secs=ITEM_START;
    }
    else
    {
        if (ItemIncluded(Line, secs))
        {
            result=ProcessTimedItem(Curr, secs, Line);
            if (result != ITEM_NORMAL) secs=(double) result;
        }
    }

    return(secs);
}



int TimediffNextLineIsContinuation(int result, char PeekChar)
{
    if (result < 1) return(FALSE);
    if ((Settings.Flags & FLAG_INDENT_CONTINUES) && isspace(PeekChar)) return(TRUE);
    if ((Settings.Flags & FLAG_NONUM_CONTINUES) && (! isdigit(PeekChar))) return(TRUE);

    return(FALSE);
}



char *TimediffReadLine(char *RetStr, STREAM *S)
{
    char *Tempstr=NULL;
    char inchar;
    int result;

    RetStr=STREAMReadLine(RetStr, S);
    if (RetStr)
    {
        StripLeadingWhitespace(RetStr);
        StripTrailingWhitespace(RetStr);

        result=STREAMPeekBytes(S, &inchar, 1);
        while (TimediffNextLineIsContinuation(result, inchar))
        {
            Tempstr=STREAMReadLine(Tempstr, S);
            StripLeadingWhitespace(Tempstr);
            StripTrailingWhitespace(Tempstr);
            RetStr=CatStr(RetStr, Tempstr);
            result=STREAMPeekBytes(S, &inchar, 1);
        }
    }

    Destroy(Tempstr);
    return(RetStr);
}



int main(int argc, const char *argv[])
{
    STREAM *S;
    char *Tempstr=NULL;
    struct timeval Prev, Curr;
    double secs;

    SettingsInit(argc, argv);
    if (! StrValid(Settings.StartStrings)) Started=TRUE;

    //clear down Prev to be sure it doesn't have nonsense data in it
    memset(&Prev, 0, sizeof(struct timeval));

    //input can be a file, stdin, or even a url
    S=STREAMOpen(Settings.Input, "r");
    if (S)
    {
        Tempstr=TimediffReadLine(Tempstr, S);
        while (Tempstr)
        {
            if (StrValid(Tempstr))
            {
                ParseDateTime(Tempstr, &Curr);
                secs=ProcessLine(&Prev, &Curr, Tempstr);

                if  (Settings.Flags & FLAG_FROM_START)
                {
                    if (secs == ITEM_START) memcpy(&Prev, &Curr, sizeof(struct timeval));
                }
                else memcpy(&Prev, &Curr, sizeof(struct timeval));
            }
            Tempstr=TimediffReadLine(Tempstr, S);
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
