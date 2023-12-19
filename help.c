#include "help.h"

void Help()
{
    printf("timediff reads log lines starting with a timestamp. It calculates the time difference between lines and prints them back out with that time difference in floating-point seconds printed at the start. If <file> is ommitted then input is read from stdin. All options that take a 'seconds' argument are floating point, so milliseconds can be expressed as fractions of a second. e.g. -w 0.5 for 500 milliseconds\n\n");
    printf("timediff <options> <file>\n\n");
    printf("options:\n");
    printf("  -min <seconds>        ignore/discard items with time difference less than <seconds>\n");
    printf("  -max <seconds>        ignore/discard items with time difference greater than <seconds>\n");
    printf("  -w <seconds>          'warn time' - highlight items with time difference greater than <seconds>\n");
    printf("  -W                    only display lines that exceed 'warn time' (use in combination with -w)\n");
    printf("  -s <pattern>          add a shell-style fnmatch pattern that marks a 'start' of a block of events (use -s multiple times to add multiple patterns)\n");
    printf("  -e <pattern>          add a shell-style fnmatch pattern that marks a 'end' of a block of events (use -e multiple times to add multiple patterns)\n");
    printf("  -i <pattern>          add a shell-style fnmatch pattern that selects lines to be INcluded in output\n");
    printf("  -x <pattern>          add a shell-style fnmatch pattern that selects lines to be EXcluded in output\n");
    printf("  -S <pattern>          add a shell-style fnmatch pattern that selects lines to have their own statistics summary\n");
    printf("  -?                    this help\n");
    printf("  -help                 this help\n");
    printf("  --help                this help\n");
    printf("  -version              print program version\n");
    printf("  --version             print program version\n");

    exit(0);
}


void Version()
{
printf("timediff: %s\n", PACKAGE_VERSION);
printf("Copyright (C) 2023 Colum Paget.\n");
printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
printf("This is free software: you are free to change and redistribute it.\n");
printf("There is NO WARRANTY, to the extent permitted by law.\n");
printf("\nWritten by Colum Paget.\n");
exit(0);
}


