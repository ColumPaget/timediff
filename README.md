AUTHOR
======

timediff is (C) 2023 Colum Paget. It is released under the Gnu Public License so you may do anything with them that the GPL allows.

Email: colums.projects@gmail.com

DISCLAIMER
==========

This is free software. It comes with no guarentees and I take no responsiblity if it makes your computer explode or opens a portal to the demon dimensions, or does anything at all, or doesn't.


LICENSE
=======

timediff is released under the GPLv3.


SYNOPSIS
========

timediff is a utility that reads timestamped lines from a file or from stdin, and calculates the time difference between each line, and displays the line with this time difference prepended. It is mostly intended for investigating process timings in industrial automation logs, where questions like "what is the longest time for data lookup" or "what is the slowest step in the process" are important. It can also parse and create timings for strace output.


INSTALL
=======


```
./configure
make
make install
```

an installation prefix can be set using `--prefix` as usual. All that's installed is the executable 'timediff', so you can also just copy that by hand to wherever you want it to be.



USAGE
=====

```
timediff <options> [file]
```

If 'file' is ommitted, input will be read from stdin.


OPTIONS
=======

```
  -min <seconds>        ignore/discard items with time difference less than <seconds>
  -max <seconds>        ignore/discard items with time difference greater than <seconds>
  -w <seconds>          'warn time' - highlight items with time difference greater than <seconds>
  -s <pattern>          add a shell-style fnmatch pattern that marks a 'start' of a block of events (use -s multiple times to add multiple patterns)
  -e <pattern>          add a shell-style fnmatch pattern that marks a 'end' of a block of events (use -e multiple times to add multiple patterns)
  -i <pattern>          add a shell-style fnmatch pattern that selects lines to be INcluded in output
  -x <pattern>          add a shell-style fnmatch pattern that selects lines to be EXcluded in output
  -S <pattern>          add a shell-style fnmatch pattern that selects lines to have their own statistics summary
  -W                    only display lines that exceed 'warn time' (use in combination with -w)
  -?                    print program help
  -help                 print program help
  --help                print program help
  -version              print program version info
  --version             print program version info
```

All options that take a 'seconds' argument are floating point, so milliseconds can be expressed as fractions of a second. e.g. -w 0.5 for 500 milliseconds


TIMESTAMP FORMATS
=================

Timestamps are expected to be at the start of the line, and timediff can handle the formats `YYYY?mm?dd?HH:MM:SS?mmm` `HH:MM:SS?mmm YYYY?mm?dd` `bbb dd HH:MM:SS?mmm` where `?` is any character, `YYYY` is the year `mm` is the month number, `bbb` is the month short name (e.g. "Oct") `dd` is the day number, `HH` is the hour, `MM` is the minutes, `SS` is the seconds and `mmm` is milliseconds. The milliseconds component is optional, and if it's six digits instead of three, then timediff will parse it as nanoseconds.

example formats that have been seen to work:

```
2023-12-05T10:30:14.182 Connection from 192.168.8.1
2023/12/05 10:30:14.182 Connection from 192.168.8.1
2023/12/05 10:30:14.182543 Connection from 192.168.8.1
2023/12/05 10:30:14 Connection from 192.168.8.1
Dec 05 10:30:14 Connection from 192.168.8.1
10:30:14.182543 Connection from 192.168.8.1
```


WARN TIME
=========

The `-w` option allows the specification of a 'warn time'. Any events that take longer than this time will be highlighted in the output of timediff. The `-W` option can be used in combination with `-w` to ONLY display those events longer than the warn time. 


INCLUDING AND EXCLUDING LINES
=============================

The `-i` and `-x` options allow specifying a shell-style pattern that can be used to either include or exclude lines from consideration. e.g.:

-i "*SCAN*"
-x "*DEVICE*STATUS*"

if 'include' patterns are specified, then ONLY lines matching those patterns are processed. 
if 'exclude' patterns are specified then any lines matching them are dropped from processing.

Multiple `-i` and `-x` options can be supplied on the command-line.


'START' AND 'END' LINES
=======================

The `-s` and `-e` options allow the specification of a 'start' and 'end' pattern. Lines matching these patterns are treated as the start and end of a block of events. This is intended for automation processes where events are connected to the processing of a physical object, with long gaps between such objects arriving and being processed. In these situations we don't consider the gaps between objects being processed to be problematic delays, but timediff will report them as such. Specifying a start time allows us to say "the period we are interested in starts here". This is useful in logs where each batch of events starts with a particular event message. There may be a long delay before the start event, and we don't want to register that delay. In some cases there may be events between the end of a block of events and the 'start' event. If we are uninterested in these events we can specify an 'end' pattern too, and now only events between the start and end events will be paid attention to.


SUMMARY LINES
=============

At the end of output, a batch of 'summaries' are printed. These lines represent the minimum, average and maximum times for a logged event. The 'default' summary applies to all eventss, but the `-S` option can be used to add a patterns and generate summaries for lines/events matching that pattern.
