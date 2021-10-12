$ @(#)35        1.2.1.5  src/bos/diag/da/gem/dgraph.msg, dagem, bos411, 9428A410j 3/22/94 16:11:05
$
$   COMPONENT_NAME: DAGEM
$
$   FUNCTIONS: paragraph
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
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
$ END OF GENERAL NOTES
 
$quote "
$set GEM_NOTIFY
$

GEM_TITLE "TESTING GRAPHICS SUBSYSTEM ADAPTER"
$
$ See general notes on length of title.

GEM_ADV   "TESTING GRAPHICS SUBSYSTEM ADAPTER IN ADVANCED MODE"
$
$ See general notes on length of title.

GEM_STDBY "Please stand by."
SAY_YES   " YES "
SAY_NO    " NO"

DGEM_RGBQ  "This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
ENSURE.........that the top section of the monitor is red.\n\
ENSURE.........that the middle section of the monitor is green.\n\
ENSURE.........that the bottom section of the monitor is blue.\n\n\
Is the monitor screen being displayed as described above?\n\n\
Move cursor to selection, then press Enter."
$
DGEM_HAVEDISP	"\
Is there a monitor attached to the display adapter currently being tested?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.

DGEM_REDSCREEN	"\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the text being displayed.\n\n\
Is the color of this text red?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DGEM_REDCUR "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the background being displayed.\n\n\
Is the color of this background red?"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DGEM_GREENSCREEN "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the text being displayed.\n\n\
Is the color of this text green?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DGEM_GREENCUR "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the background being displayed.\n\n\
Is the color of this background green?"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DGEM_BLUESCREEN "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the text being displayed.\n\n\
Is the color of this text blue?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DGEM_BLUECUR "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the background being displayed.\n\n\
Is the color of this background blue?"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DGEM_PDPQ  "This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
WITHOUT exiting diagnostics or powering off the system unit:\n\n\
RUN............the problem determination procedures that came\n\
               with the display.\n\
ANSWER.........the question below.\n\n\
Did the problem determination procedures reveal a problem?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DGEM_RP	"%1$x \
%2$s\n\
FOLLOW.........the procedures for reporting problems with the display."
$
$ %1$x is a six digit number that will appear as a menu number.
$ %2$s is a title message.  Be sure and leave these two substitutions
$ exactly as they are shown.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

TAURUS_PRESENT "\
Is there a  GTO 7235 cabled to the system unit? \n\n"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

TAURUS_POWER "\
Is the GTO 7235 powered on? \n\n"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

RGB_7_SEC "\
This is a test of the display attached to the\n\
graphics adapter currently being tested.\n\n\
A three color pattern will be displayed on the next screen\n\
for 7 seconds.\n\n\
ENSURE.........that the top section of the monitor is red.\n\
ENSURE.........that the middle section of the monitor is green.\n\
ENSURE.........that the bottom section of the monitor is blue.\n\n\
To display the pattern, press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

WAS_IT_RGB "\
This is a test of the display attached to the\n\
graphics adapter currently being tested.\n\n\
WAS............the top section of the monitor red,\n\
WAS............the middle section of the monitor green, and\n\
WAS............the bottom section of the monitor blue?\n\n\
Move cursor to selection, then press Enter."

$set GEM_IN_LOOP
GEM_LOOP_MSG "TESTING GRAPHICS SUBSYSTEM ADAPTER IN ADVANCED MODE\n\n\
Testing multiple times. \n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ See general notes on length of title.


