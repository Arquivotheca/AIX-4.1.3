$ @(#)50	1.7.2.8  src/bos/usr/bin/grep/grep.msg, cmdscan, bos411, 9428A410j 5/6/94 15:41:23
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.
$len
$set MS_GREP        Define initial set#

PARG                 "%s: 0652-032 Do not use the -p flag more than 20 times.\n"
USAGE "Usage: %s [-E|-F] [-c|-l|-q] [-insvxbhwy] [-p[parasep]] -e pattern_list...\n\
\t[-f pattern_file...] [file...]\n\
Usage: %s [-E|-F] [-c|-l|-q] [-insvxbhwy] [-p[parasep]] [-e pattern_list...]\n\
\t-f pattern_file... [file...]\n\
Usage: %s [-E|-F] [-c|-l|-q] [-insvxbhwy] [-p[parasep]] pattern_list [file...]\n"
$ $ Check the usage statement.
OPERR                "%s: 0652-033 Cannot open %s.\n"
OVFLO                "%s: 0652-034 The maximum paragraph length is 10,000 bytes.\n"
REERR                "%s: 0652-035 The specified pattern is not valid.\n"

$ 0652-036 was not checked as being unique.

OVERFLO              "%s: 0652-036 The number of patterns (including those in -f files) must not exceed 400.\n"

STDIN	"(standard input)"

$ These usage forms are for "grep -E" when called as egrep and "grep -F"
$   when called as fgrep.

EUSAGE "Usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] -e pattern_list...\n\
\t[-f pattern_file...] [file...]\n\
Usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] [-e pattern_list...]\n\
\t-f pattern_file... [file...]\n\
Usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] pattern_list [file...]\n"

FUSAGE "Usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] -e pattern_list...\n\
\t[-f pattern_file...] [file...]\n\
Usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] [-e pattern_list...]\n\
\t-f pattern_file... [file...]\n\
Usage: %s [-hisvwxy] [[-bn]|[-c|-l|-q]] [-p[parasep]] pattern_list [file...]\n"

BANDSTDIN	"$s: 0652-225 The -b flag may not be used with stdin or pipes.\n"

LINELENERR	"%s: 0652-226 Maximum line length of %d exceeded.\n"
FMODE		"%s: 0652-227 %s must be a regular or fifo file.\n"
NOPAT		"%s: 0652-228 Specify at least one pattern in %s.\n"
