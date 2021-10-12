$ @(#)99	1.4  src/bos/diag/da/swmono/dskym.msg, daswmono, bos411, 9428A410j 3/11/94 10:16:42
$
$   COMPONENT_NAME: DASWMONO
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 
$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %07X, or \t in any messages.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ These comments concern the TITLE LINES at the top the diagnostic screen.
$   The title must be in all capital letters. The first line
$   of the title cannot be longer than 70 characters starting from 
$   column 1.  If the line is greater than 70, it may be continued on
$   the next line.  Leave line spacing as shown:  one blank line after
$   the last title line.  For example:
$
$ *****
$ TESTING PORT 12 OF THE 16-PORT ASYNCHRONOUS ADAPTER IN PLANAR SLOT 2
$ IN ADVANCED MODE
$
$ Please stand by.
$ *****
$
$ These comments concern the user ACTIONS in all caps.
$   If translations require the creation of new lines, begin the
$   new lines in the column immediately following the row of periods. 
$   For example:
$
$ *****
$ ACTION.........one line of English might require several when translated, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$ END OF GENERAL NOTES


$quote "
$set	DSKY_MSGS
$
DSKY_CM		"TESTING GRAYSCALE GRAPHICS DISPLAY ADAPTER"
$
$ See general notes on length of title.

DSKY_AM		"TESTING GRAYSCALE GRAPHICS DISPLAY ADAPTER IN ADVANCED MODE"
$
$ See general notes on length of title.

DSKY_CCM	"TESTING COLOR GRAPHICS DISPLAY ADAPTER"
$
$ See general notes on length of title.

DSKY_CAM	"TESTING COLOR GRAPHICS DISPLAY ADAPTER IN ADVANCED MODE"
$
$ See general notes on length of title.

DSKY_STANDBY	"Please stand by."
DSKY_SAYYES	"YES"
$ This option is shown when a YES answer is possible.

DSKY_SAYNO	"NO"
$ This option is shown when a YES answer is possible.

DSKY_LOOP	"\
TESTING GRAYSCALE GRAPHICS DISPLAY ADAPTER IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the test.
$ See general notes on length of title.

DSKY_CLOOP	"\
TESTING COLOR GRAPHICS DISPLAY ADAPTER IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%6d\tpasses completed.\n\
%6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the test.
$ See general notes on length of title.

DSKY_RGBQ  "This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
ENSURE.........that the top section of the monitor is red.\n\
ENSURE.........that the middle section of the monitor is green.\n\
ENSURE.........that the bottom section of the monitor is blue.\n\n\
Is the monitor screen being displayed as described above?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DSKY_PDPQ  "This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
WITHOUT exiting diagnostics or powering off the system unit:\n\n\
RUN............the problem determination procedures that came\n\
               with the display.\n\
ANSWER.........the question below. \n\n\
Did the problem determination procedures reveal a problem?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DSKY_RP	"%1$x \
%2$s\n\
FOLLOW.........the procedures for reporting problems with the display."
$
$ %1$x is a six digit number that will appear as a menu number.
$ %2$s is a title message.  Be sure and leave these two substitutions
$ exactly as they are shown.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DSKY_HAVEDISP	"\
Is there a monitor attached to the display adapter currently being tested?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.
