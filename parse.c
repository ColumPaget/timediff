#include "parse.h"
#include "settings.h"


int IsMonthName(const char *ptr)
{
    if (strncasecmp(ptr, "jan",3)==0) return(1);
    if (strncasecmp(ptr, "feb",3)==0) return(2);
    if (strncasecmp(ptr, "mar",3)==0) return(3);
    if (strncasecmp(ptr, "apr",3)==0) return(4);
    if (strncasecmp(ptr, "may",3)==0) return(5);
    if (strncasecmp(ptr, "jun",3)==0) return(6);
    if (strncasecmp(ptr, "jul",3)==0) return(7);
    if (strncasecmp(ptr, "aug",3)==0) return(8);
    if (strncasecmp(ptr, "sep",3)==0) return(9);
    if (strncasecmp(ptr, "oct",3)==0) return(10);
    if (strncasecmp(ptr, "nov",3)==0) return(11);
    if (strncasecmp(ptr, "dec",3)==0) return(12);

    return(0);
}


const char *ExtractComponentStr(const char *Input, const char *Allowed, char **Extract)
{
    const char *ptr;

    for (ptr=Input; *ptr !='\0'; ptr++)
    {
        if (! strchr(Allowed, *ptr)) break;
    }

    *Extract=CopyStrLen(*Extract, Input, ptr-Input);

    return(ptr);
}


char *ParseDate(char *RetStr, const char *Input)
{
    const char *ptr;
    char *Token=NULL, *Year=NULL, *Month=NULL, *Day=NULL;

    RetStr=CopyStr(RetStr, "");
    if (! StrValid(Input)) return(RetStr);

    ptr=ExtractComponentStr(Input, "01234567890", &Token);
    Year=CopyStr(Year, Token);
    ptr=ExtractComponentStr(ptr+1, "01234567890", &Token);
    Month=CopyStr(Month, Token);
    ptr=ExtractComponentStr(ptr+1, "01234567890", &Token);
    Day=CopyStr(Day, Token);

    //if day is longer than 2 characters, then it must be year, so we reverse everything
    if (StrLen(Day) > 2) RetStr=MCopyStr(RetStr, Day, "-", Month, "-", Year, NULL);
    else RetStr=MCopyStr(RetStr, Year, "-", Month, "-", Day, NULL);

    Destroy(Token);
    Destroy(Year);
    Destroy(Month);
    Destroy(Day);

    return(RetStr);
}


char *ParseTime(char *RetStr, const char *Input, struct timeval *tv)
{
    const char *ptr;
    char *Token=NULL;

    RetStr=CopyStr(RetStr, "");

    ptr=ExtractComponentStr(Input, "01234567890", &Token);
    RetStr=MCatStr(RetStr, Token, ":", NULL);
    ptr=ExtractComponentStr(ptr+1, "01234567890", &Token);
    RetStr=MCatStr(RetStr, Token, ":", NULL);
    ptr=ExtractComponentStr(ptr+1, "01234567890", &Token);
    RetStr=CatStr(RetStr, Token);
    if (StrValid(ptr))
    {
        ptr=ExtractComponentStr(ptr+1, "01234567890", &Token);
        if (StrLen(Token)==6) 
	{
	tv->tv_usec=atoi(Token);
	Settings.Flags |= FLAG_NANOSECS;
	}
        else tv->tv_usec=atoi(Token) * 1000;
    }

    Destroy(Token);

    return(RetStr);
}


int ParseNumericDateTime(const char *Input, struct timeval *tv)
{
    const char *ptr;
    char *Tempstr=NULL, *Time=NULL, *Date=NULL;

    if (StrLen(Input) < 10) return(FALSE);

    ptr=Input;
    while ( (*ptr !='\0') && (! isdigit(*ptr)) ) ptr++;

    //the list of characters are VALID characters in the component
    ptr=ExtractComponentStr(ptr, "/-_:.01234567890", &Tempstr);
    if (strchr(Tempstr, ':')) Time=ParseTime(Time, Tempstr, tv);
    else Date=ParseDate(Date, Tempstr);

    ptr=ExtractComponentStr(ptr+1, "/-_:.01234567890", &Tempstr);
    if (strchr(Tempstr, ':')) Time=ParseTime(Time, Tempstr, tv);
    else Date=ParseDate(Date, Tempstr);

    if (! StrValid(Date)) Date=CopyStr(Date, GetDateStr("%Y-%m-%d", NULL));
    Tempstr=MCopyStr(Tempstr, Date, "T", Time, NULL);

    tv->tv_sec=DateStrToSecs("%Y-%m-%dT%H:%M:%S",Tempstr,  NULL);

    Destroy(Tempstr);
    Destroy(Date);
    Destroy(Time);
    return(TRUE);
}


int ParseUnixLogDateTime(const char *Input, struct timeval *tv)
{
    int val;
    char *Token=NULL, *Tempstr=NULL;
    const char *ptr;


    ptr=GetToken(Input, "\\S", &Token, 0);
    val=IsMonthName(Token);
    ptr=GetToken(ptr, "\\S", &Token, 0);

    Tempstr=FormatStr(Tempstr, "%s-%02d-%02dT", GetDateStr("%Y", NULL), val, atoi(Token));
    Token=ParseTime(Token, ptr, tv);
    Tempstr=CatStr(Tempstr, Token);

    tv->tv_sec=DateStrToSecs("%Y-%m-%dT%H:%M:%S",Tempstr,  NULL);

    Destroy(Tempstr);
    Destroy(Token);
    return(TRUE);
}

int ParseSecondsDateTime(const char *Input, struct timeval *tv)
{
uint64_t val;
char *ptr=NULL;

//printf("PS: %s\n", Input);

tv->tv_sec=strtol(Input, &ptr, 10);
ptr_incr((const char **) &ptr, 1);
tv->tv_usec=strtol(ptr, NULL, 10);
return(TRUE);
}


int ParseDateTime(const char *Input, struct timeval *tv)
{
    const char *start, *end;
    char *Tempstr=NULL;
    int result=0;

    start=Input;

    //some logfiles put date in a [ ] wrapper
    if (strchr("[<{", *start)) start++;
    while (isspace(*start)) start++;

    end=ExtractComponentStr(start, ".01234567890", &Tempstr);
    if ( (StrLen(Tempstr) > 2) && (strchr(" ]>}", *end)) ) result=ParseSecondsDateTime(start, tv);
    else if (IsMonthName(start)) result=ParseUnixLogDateTime(start, tv);
    else result=ParseNumericDateTime(start, tv);

    Destroy(Tempstr);
    return(result);
}
