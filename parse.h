#ifndef TIMEDEF_PARSE_H
#define TIMEDEF_PARSE_H

#include "includes.h"

const char *ExtractComponentStr(const char *Input, const char *Allowed, char **Extract);
char *ParseDate(char *RetStr, const char *Input);
char *ParseTime(char *RetStr, const char *Input, struct timeval *tv);
int ParseDateTime(const char *Input, struct timeval *tv);


#endif


