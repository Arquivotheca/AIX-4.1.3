$ @(#)99	1.3  src/bos/diag/da/wga/dwga.msg, dawga, bos411, 9428A410j 2/24/94 13:56:21
$ 
$
$   COMPONENT_NAME: DAWGA
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1993,1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
$set	DWGA_MSGS
$

DWGA "TESTING POWER Gt1x GRAPHICS ADAPTER %1$s IN LOCATION %2$s"
$
$ See general notes on length of title.

DWGA_A "TESTING POWER Gt1x GRAPHICS ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ See general notes on length of title.

DWGA_L "TESTING POWER Gt1x GRAPHICS ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ See general notes on length of title.

DWGA_STANDBY	"Please stand by."

DWGA_EXT_YES	"YES"
$ This option is shown when a YES answer is possible.

DWGA_EXT_NO	"NO"
$ This option is shown when a NO answer is possible.

DWGA_PDPQ  "This is a test of the display attached to the\n\
display adapter currently being tested.\n\n\
WITHOUT exiting diagnostics or powering off the system unit:\n\n\
EXECUTE........the problem determination procedures that\n\
               came with the display.\n\
Did the problem determination procedures reveal a problem?\n\n\
Move cursor to selection, then press Enter."
$
$ See general notes on expanding ACTION lines if necessary.
$ Leave line spacing as shown.

DWGA_HAVEDISP	"\
Is there a display attached to the graphics adapter currently being tested?\n\n\
Move cursor to selection, then press Enter.\n\n\
NOTE:  If you choose YES, you will be asked to identify the COLORS of\n\
several screens.  If you do not wish to run any of the color display tests,\n\
choose NO."
$
$ Leave line spacing as shown.

DWGA_BLUECUR "\
This is a test of the display attached to the\n\
graphics adapter currently being tested.\n\n\
OBSERVE........the background color of the display that is connected\n\
	       to the device under test.\n\n\
Is the color of this background blue?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
$ Do not translate the digits 1 and 2 in this message.

DWGA_REDCUR "\
This is a test of the display attached to the\n\
graphics adapter currently being tested.\n\n\
OBSERVE........the background color of the display that is connected\n\
	       to the device under test.\n\n\
Is the color of this background red?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
$ Do not translate the digits 1 and 2 in this message.

DWGA_GREENCUR "\
This is a test of the display attached to the\n\
graphics adapter currently being tested.\n\n\
OBSERVE........the background color of the display that is connected\n\
	       to the device under test.\n\n\
Is the color of this background green?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

R_BASE "Adapter Test failed."
R_DISPLAY "Display Test failed."
F_CABLE "Internal Cable"
F_MON "Attached Display"
BLANK " "

$ Reason code and Field Replacable Unit (FRU) message to be displayed in FRU callout
$ Used to be in dcda.msg file

F3_EXIT  "\
This is a test of the display attached to the\n\
graphics adapter currently being tested.\n\n\
A %s color pattern will be displayed on the next screen\n\
for 7 seconds.\n\n\
To display the pattern, press Enter."
$
$ %s is a color.  Be sure and leave this substitution
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

RED "red"
$
$ red is the color red

GREEN "green"
$
$ green is the color green

BLUE "blue"
$
$ blue is the color green

COLOR_SCREEN "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
Was the color of the screen %s?\n\n\
Move cursor to selection, then press Enter."
$
$ %s is a color.  Be sure and leave this substitution
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
