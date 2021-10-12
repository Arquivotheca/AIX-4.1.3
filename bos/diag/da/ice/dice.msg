$ @(#)50	1.1  src/bos/diag/da/ice/dice.msg, daice, bos41J, 9513A_all 3/22/95 12:18:18
$
$   COMPONENT_NAME: DAICE
$
$   FUNCTIONS: 
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1995
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

$set ICE_SRN_MSG
$quote "
$ This set should always be the first set.  Do NOT move this set or put other
$ message sets before this one.

ICE_101 "Error log analysis indicated a hardware failure." 
ICE_102 "Adapter Test failed."
ICE_103 "Display Test failed."
DISPLAY "Attached Display"



$set ICE_MSG
$quote "

ADVANCED_MODE_MENU "\nIN ADVANCED MODE"
$ Leave line spacing as shown.  See general notes on length of title line.

CUSTOMER_MODE_MENU " "
$ This is a blank message because there is no additional information for this
$ mode.
$ Leave line spacing as shown.  See general notes on length of title line.

LOOPMODE_MODE_MENU " "
$ This is a blank message because there is no additional information for this
$ mode.
$ Leave line spacing as shown.  See general notes on length of title line.

TITLE   "\
TESTING %1$s %2$s IN LOCATION %3$s %4$s\n\
"
$ %1$s is the type of adapter.  This is gotten from devices.cat.
$ %2$s is the adapter that is tested.
$ %3$s is the location code of the device as described in the general notes
$ %4$s is whether this is replaced with "IN ADVANCED MODE" if this is advanced
$       mode.
$ Leave line spacing as shown.  See general notes on length of title line.

SEL_OPTION "Move cursor to selection, then press Enter "
$ Leave line spacing as shown.  See general notes on length of title line.

FINISHED "\
When finished, press Enter."

YES_OPTION "YES"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

NO_OPTION "NO"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

DISP_PD  "This is a test of the display attached to the\n\
display adapter currently being tested.\n\n\
WITHOUT exiting diagnostics or powering off the system unit:\n\n\
EXECUTE........the problem determination procedures that\n\
               came with the display.\n\
Did the problem determination procedures reveal a problem?\n\n\
Move cursor to selection, then press Enter."
$
$ See general notes on expanding ACTION lines if necessary.
$ Leave line spacing as shown.

HAVEDISP	"\
Is there a display attached to the graphics adapter currently being tested?\n\n\
Move cursor to selection, then press Enter.\n\n\
NOTE:  If you choose YES, you will be asked to identify the COLORS of\n\
several screens.  If you do not wish to run any of the color display tests,\n\
choose NO."

$
$ Leave line spacing as shown.


OBSERVATION "\
%1$s\n\n\
Move cursor to selection, then press Enter."
$
$ %1$s is for the color of the screen which is used.(e.g. REDSCRN,GREENSCRN,etc.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
$ Do not translate the digits 1

REDSCRN "\
Was the color of the screen red?"

BLUESCRN "\
Was the color of the screen blue?"

GREENSCRN "\
Was the color of the screen green?"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

SWITCHES	"\
A problem has been detected that may be caused by the incorrect\n\
switch settings on the adapter.  Are the switches on the adapter\n\
set correctly?\n\n\
Move cursor to selection, then press Enter.\n\n\
NOTE:  If you do not know if the switches are set correctly, choose NO."

SWITCH_GOAL "942008 \
%1$s ADAPTER SWITCHES SET INCORRECTLY\n\n\
The switches on the %2$s adapter may not\n\
be set correctly. Check that the switches are in the proper\n\
position or alternatively, remove the cable from the back of\n\
the adapter and re-run Diagnostics.\n\n\
The switches should be set to a supported value to run Diagnostics\n\
on this adapter."
$
$ %1$s is the description of the resource 
$ %2$s is the description of the resource 
$ Leave line spacing as shown.  See general notes on length of title.
$ Leave it exactly as shown.

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
