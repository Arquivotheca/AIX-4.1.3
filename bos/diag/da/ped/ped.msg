$ @(#)09	1.9.1.2  src/bos/diag/da/ped/ped.msg, daped, bos41J, 9519A_all 5/4/95 11:16:04
$
$   COMPONENT_NAME: DAPED
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1991, 1995
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

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
$set	DPED_MSGS
$

DPED_C8B   "TESTING POWER Gt4 8-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s"
$
$ See general notes on length of title.

DPED_C24B "TESTING POWER Gt4 24-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s"
$
$ See general notes on length of title.

DPED_C8E   "TESTING POWER Gt4x 8-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s"
$
$ See general notes on length of title.

DPED_C24E "TESTING POWER Gt4x 24-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s"
$
$ See general notes on length of title.

DPED_A8B "TESTING POWER Gt4 8-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ See general notes on length of title.

DPED_A24B "TESTING POWER Gt4 24-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$

DPED_A8E "TESTING POWER Gt4x 8-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ See general notes on length of title.

DPED_A24E "TESTING POWER Gt4x 24-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ See general notes on length of title.

DPED_L8B "TESTING POWER Gt4 8-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ See general notes on length of title.

DPED_L24B "TESTING POWER Gt4 24-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ See general notes on length of title.

DPED_L8E "TESTING POWER Gt4x 8-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ See general notes on length of title.

DPED_L24E "TESTING POWER Gt4x 24-BIT FEATURE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ See general notes on length of title.

DPED_STANDBY_OLD ""

DPED_BLANKSC	"\nThis test may cause the screen to go blank for a while."

DPED_NOTOUCH  "\nDo not press any key during this test."

DPED_EXT_YES	"YES"
$ This option is shown when a YES answer is possible.

DPED_EXT_NO	"NO"
$ This option is shown when a NO answer is possible.

DPED_PDPQ  "This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
WITHOUT exiting diagnostics or powering off the system unit:\n\n\
EXECUTE........the problem determination procedures that\n\
               came with the display.\n\
ANSWER.........the question below.\n\n\
Did the problem determination procedures reveal a problem?\n\n\
Move cursor to selection, then press Enter."
$
$ See general notes on expanding ACTION lines if necessary.
$ Leave line spacing as shown.

DPED_RP	"FOLLOW.........the procedures for reporting problems with the display.\n\n\
Press Enter to continue."
$
$ See general notes on expanding ACTION lines if necessary.
$ %1$x is a six digit menu number that is displayed on the screen.
$ %2$s is a title message.  Be sure and leave these two substitutions 
$ exactly as they are shown.

DPED_HAVEDISP	"\
Is there a monitor attached to the display adapter currently being tested?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.

DPED_BLUECUR "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the background being displayed.\n\n\
Is the color of this background blue?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
$ Do not translate the digits 1 and 2 in this message.

DPED_REDCUR "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the background being displayed.\n\n\
Is the color of this background red?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
$ Do not translate the digits 1 and 2 in this message.

DPED_GREENCUR "\
This is a test of the monitor attached to the\n\
display adapter currently being tested.\n\n\
OBSERVE........the color of the background being displayed.\n\n\
Is the color of this background green?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

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

DPED_STANDBY	"Please stand by.\n\n\
This test may take up to 10 minutes."

$set NO_RCM
$quote "
$ 
NO_RCM_TITLE "878555 \
POWER Gt4 PROCESSOR OPERATIONAL MICROCODE IS MISSING\n\n\
The POWER Gt4 PROCESSOR operational microcode is either\n\
missing or not accessible.\n\n\
This microcode is necessary in order to use the POWER Gt4 PROCESSOR card\n\
in normal system operations."
$
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the number 878100 at the beginning of the message.
$ Leave it exactly as shown.

$set NO_DIAG
$quote "
$
NO_DIAG_TITLE "878666 \
POWER Gt4 PROCESSOR DIAGNOSTIC MICROCODE IS MISSING\n\n\
The POWER Gt4 PROCESSOR diagnostic microcode is either\n\
missing or not accessible.\n\n\
This microcode is necessary in order to test the POWER Gt4 PROCESSOR card."
$
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the number 878200 at the beginning of the message.
$ Leave it exactly as shown.

$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
