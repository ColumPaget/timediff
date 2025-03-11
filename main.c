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



#define ITEM_NORMAL   0
#define ITEM_START   -1
#define ITEM_END     -2
#define ITEM_EXCLUDE -3

int ProcessTimedItem(struct timeval *When, double Secs, const char *Line, char **Output)
{
    const char *prefix="", *postfix="";
    char *Tempstr=NULL;
    int RetVal=ITEM_NORMAL;

    //if a start is declared, and we have not encountered a start-line, or
    //are between a start-line and an end-line, then ignore this line
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

        if (Settings.Flags & FLAG_NANOSECS) *Output=FormatStr(*Output, "%s%14.6f%s  %s\n", prefix, Secs, postfix, Line);
        else *Output=FormatStr(*Output, "%s%10.3f%s  %s\n", prefix, Secs, postfix, Line);

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




double ProcessLine(struct timeval *Prev, struct timeval *Curr, const char *Line, char **Output)
{
    double secs=0;
    int result;

    secs=CalcTimediff(Prev, Curr);

    //if it's a 'Start' line, then we won't be calculating a difference between it
    //and the previous line. Also, if a start is declared, then we will ignore lines
    //before a start, or between a start and an end if an end is declared.
    if (InPatternList(Line, Settings.StartStrings, NULL))
    {
        //if we are in 'warn only' mode, where we only display items that have a time warning
        //then we want a 'start string' that matches the start of an interaction to be registered as the start time
        //but we don't want it displayed
        if (! (Settings.Flags & FLAG_WARN_ONLY)) *Output=FormatStr(*Output, "%s%10s%s  %s\n", Settings.NormPrefix, " -- ", Settings.NormPostfix, Line);
        Started=TRUE;
        secs=ITEM_START;
    }
    else
    {
        if (ItemIncluded(Line, secs))
        {
            result=ProcessTimedItem(Curr, secs, Line, Output);
            if (result != ITEM_NORMAL) secs=(double) result;
        }
        else secs=ITEM_EXCLUDE;
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


char *HandleStartEndBlock(char *RetStr, double secs, char *Line, struct timeval *PrevTime, struct timeval *CurrTime)
{
//if we aren't using start or end strings, then we will just output the current line
//and PrevTime becomes the CurrTime. We don't use the OutputBuffer provided by RetStr
    if ( (! StrValid(Settings.StartStrings)) && (! StrValid(Settings.EndStrings)) )
    {
        memcpy(PrevTime, CurrTime, sizeof(struct timeval));
        if (StrValid(Line)) printf("%s\n", Line);
        return(CopyStr(RetStr, ""));
    }

    if (secs == ITEM_START)
    {
// on a start we always start a new output block,
// and set PrevTime to CurrTime
        memcpy(PrevTime, CurrTime, sizeof(struct timeval));
        RetStr=CopyStr(RetStr, Line);
    }
    else if (secs == ITEM_END)
    {
//on an end we output the always buffered block of lines
        RetStr=CatStr(RetStr, Line);
        printf("%s\n", RetStr);
        RetStr=CopyStr(RetStr, "");
    }
    else if (secs != ITEM_EXCLUDE)
    {
        RetStr=CatStr(RetStr, Line);

//if we are measuring time from a start line, then we don't update PrevTime on every line
        if  (! (Settings.Flags & FLAG_FROM_START)) memcpy(PrevTime, CurrTime, sizeof(struct timeval));
        if (! StrValid(Settings.EndStrings))
        {
            printf("%s\n", RetStr);
            RetStr=CopyStr(RetStr, "");
        }
    }

    return(RetStr);
}


int main(int argc, const char *argv[])
{
    STREAM *S;
    char *Tempstr=NULL, *InputLine=NULL, *Output=NULL;
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
        InputLine=TimediffReadLine(InputLine, S);
        while (InputLine)
        {
            if (StrValid(InputLine))
            {
                ParseDateTime(InputLine, &Curr);
                secs=ProcessLine(&Prev, &Curr, InputLine, &Tempstr);
                //actual printing of output happens in HandleStartEndBlock
                Output=HandleStartEndBlock(Output, secs, Tempstr, &Prev, &Curr);
            }
            InputLine=TimediffReadLine(InputLine, S);
        }
        //Display stats before closing stream,
        //because the stream might be stdin/stdout
        //and older versions of libUseful will close those file descrpitors
        //preventing stats from being displayed
        StatsDisplay();

        STREAMClose(S);
    }

    Destroy(InputLine);
    Destroy(Tempstr);
    Destroy(Output);

    return(0);
}
