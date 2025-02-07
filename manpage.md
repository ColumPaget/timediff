title: timediff
mansection: 1
date: 2025/02/07


NAME
====

timediff - read lines with a leading timestamp, and print out the time difference between them


DESCRIPTION
===========

timediff reads log lines starting with a timestamp. It calculates the time difference between lines and prints them back out with that time difference in floating-point seconds printed at the start of line. 



SYNOPSIS
========

     timediff <options> <file>


OPTIONS
=======

-min <seconds>        
: ignore/discard items with time difference less than <seconds>
-max <seconds>        
: ignore/discard items with time difference greater than <seconds>
-w <seconds>          
: 'warn time' - highlight items with time difference greater than <seconds>
-W                    
: only display lines that exceed 'warn time' (use in combination with -w)
-indent               
: treat indented lines (lines starting with withspace) as continuations of the previous line
-I                    
: treat indented lines (lines starting with withspace) as continuations of the previous line
-multiline            
: treat any line not starting with a digit as continuation of the previous line
-M                    
: treat any line not starting with a digit as continuation of the previous line
-from-start           
: measure time from 'start' not from previous item
-fs                   
: measure time from 'start' not from previous item
-F                    
: measure time from 'start' not from previous item
-s <pattern>          
: add a shell-style fnmatch pattern that marks a 'start' of a block of events (use -s multiple times to add multiple patterns)
-e <pattern>          
: add a shell-style fnmatch pattern that marks a 'end' of a block of events (use -e multiple times to add multiple patterns)
-i <pattern>          
: add a shell-style fnmatch pattern that selects lines to be INcluded in output
-x <pattern>          
: add a shell-style fnmatch pattern that selects lines to be EXcluded in output
-S <pattern>          
: add a shell-style fnmatch pattern that selects lines to have their own statistics summary
-?                    
: this help
-help                 
: this help
--help                
: this help
-version              
: print program version
--version             
: print program version


All options that take a 'seconds' argument are floating point, so milliseconds can be expressed as fractions of a second. e.g. '-w 0.5' for 500 milliseconds. Alternatively the suffixes 'm' or 'ms' for milliseconds and 'c' or 'cs' can be used, e.g. '-min 200ms'.


TIMESTAMP FORMATS
=================

Timestamps are expected to be at the start of the line. 

timediff can handle the formats:

```
YYYY?mm?dd?HH:MM:SS?mmm
HH:MM:SS?mmm YYYY?mm?dd
bbb dd HH:MM:SS?mmm
SSSS.mmm 
```

where '?' is any character, 'YYYY' is the year 'mm' is the month number, 'bbb' is the month short name (e.g. "Oct") 'dd' is the day number, 'HH' is the hour, 'MM' is the minutes, 'SS' is the seconds and 'mmm' is milliseconds. 

Note how the ':' character is used to distinguish the time component of a date-time from the date. 

The milliseconds component is optional, and if it's six digits instead of three, then timediff will parse it as microseconds. The 'SSSS.mmm' format is 'kernel/dmesg' style, where number of seconds since some date, either the unix epoch or boot-up, are used to indicate the time.

timediff can also handle these formats if they're wrapped in characters like [] <> or {}

example formats that have been seen to work:

```
2023-12-05T10:30:14.182 Connection from 192.168.8.1
2023/12/05 10:30:14.182 Connection from 192.168.8.1
2023/12/05 10:30:14.182543 Connection from 192.168.8.1
2023/12/05 10:30:14 Connection from 192.168.8.1
Dec 05 10:30:14 Connection from 192.168.8.1
10:30:14.182543 Connection from 192.168.8.1
[1000.234762] Starting up
```


timediff has been seen to work with the dmesg, syslog, strace and tcpdump log formats.


WARN TIME
=========

The '-w' option allows the specification of a 'warn time'. Any events that take longer than this time will be highlighted in the output of timediff. The '-W' option can be used in combination with '-w' to ONLY display those events longer than the warn time. 


INCLUDING AND EXCLUDING LINES
=============================

The '-i' and '-x' options allow specifying a shell-style pattern that can be used to either include or exclude lines from consideration. e.g.:

```
-i "*SCAN*"
-x "*DEVICE*STATUS*"
```

if 'include' patterns are specified, then ONLY lines matching those patterns are processed. 
if 'exclude' patterns are specified then any lines matching them are dropped from processing.

Multiple '-i' and '-x' options can be supplied on the command-line.

The '-min' and '-max' options can also be used to exclude lines based on their time difference, e.g. '-max 0.1' would only show lines with timediffs less than or equal to 100ms.


START AND END LINES
=======================

The '-s' and '-e' options allow the specification of a 'start' and 'end' pattern. e.g.:

```
-s "*NEWITEM*"
-e "*ITEM_GONE*"
```

Lines matching these patterns are treated as the start and end of a block of events. '-s' can be used independantly of '-e', but '-e' causes lines after it to be excluded from processing until a 'start line' (set with '-s') is seen.

This is intended for automation processes where events are connected to the processing of a physical object, with long gaps between such objects arriving and being processed. In these situations we don't consider the gaps between objects being processed to be problematic delays, but timediff will report them as such. Specifying a start time allows us to say "the period we are interested in starts here". This is useful in logs where each batch of events starts with a particular event message. There may be a long delay before the start event, and we don't want to register that delay. In some cases there may be events between the end of a block of events and the 'start' event. If we are uninterested in these events we can specify an 'end' pattern too, and now only events between the start and end events will be paid attention to.

The '-from-start' (or '-fs' or '-F') option causes timediff to measure time from a 'start line' rather than from the previous line.  



CONTINUATION LINES
==================

Some logs (notably tcpdump used with '-A' or '-X') produce output that is more than one line, and timediff struggles to parse this. The '-indent' and '-multiline' options offer a solution. '-indent' (or '-I') will concatanate lines that start with whitespace into one line (so this is useful to parse 'tcpdump -X'). Alternatively '-multiline' (or '-M') will concatanate any lines that do not start with a number (as the datetime format is expected to use numbers) (so this works for 'tcpdump -A'). Unfortuantely the '-multiline' option is not compatible with date formats with a month name at the start.


SUMMARY LINES
=============

At the end of output, a batch of 'summaries' are printed. These lines represent the minimum, average and maximum times for a logged event. The 'default' summary applies to all eventss, but the '-S' option can be used to add a patterns and generate summaries for lines/events matching that pattern.
