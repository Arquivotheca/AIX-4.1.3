$ @(#)13	1.3.5.1  src/bos/diag/util/u5081/u5081.msg, dsau5081, bos41J, 9513A_all 3/9/95 10:55:53
$
$   COMPONENT_NAME: DSAU5081
$
$   FUNCTIONS: Diagnostic header file.
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
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
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES


$quote "
$set	UDISP_MSGS
$

UDISP_GT	"DISPLAY TEST PATTERNS SERVICE AID"
$
$ See general notes on length of title.

UDISP_STANDBY	"Please stand by."

UDISP_DISPLAYS	"\
Which of the following displays do you want to test?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.

UDISP_PATTERNS	"\
Which of the following patterns do you want to display?\n\n\
Move cursor to selection, then press Enter."
$ Leave line spacing as shown.

UDISP_RED	"The red full screen pattern."
UDISP_GREEN	"The green full screen pattern."
UDISP_BLUE	"The blue full screen pattern."
UDISP_BLACK	"The black full screen pattern."
UDISP_WHITE	"The white full screen pattern."
UDISP_9x9GRID	"The 9x9 grid pattern."
UDISP_9x11GRID	"The 9x11 grid pattern."
UDISP_COLORBAR	"The colorbar pattern."
UDISP_GOTOMAIN	"Return to the Display Selection menu."
UDISP_EXIT	"Return to the Service Aids Selection menu."
$

UDISP_TEST	"\
The pattern you selected will be displayed on the next screen.\n\n\
REMEMBER.......to use 'Cancel' to exit the screen.\n\n\
To display the pattern, press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

UDISP_NDF	"\
The Display Test Patterns Service Aid cannot be run.  There are\n\
either no display adapters located in the system unit to test or\n\
the Diagnostic support needed to run this service aid has not\n\
been loaded.\n\n\
To return to the Service Aid Selection menu, press Enter."
$
$ Leave line spacing as shown.

UDISP_SWE	"\
A software error was encountered that prevents the execution of this \n\
service aid.\n\n\
To return to the Service Aid Selection menu, press Enter."
$
$ Leave line spacing as shown.

UDISP_BOX	"The 50 mm box pattern."
UDISP_FOCUS	"The focus pattern."
$ UDISP_BOX and FOCUS are two new patterns added for Salmon to support 
$ 8517 display.

UDISP_DESCR 	"The display attached to \n\
\t%1$s in location %2$s."
$
$ %1$s is a substitution for the name of the display adapter. %2$s is a 
$ substitution for the location.
$ The \t in this message tabs the name of the display which is obtained from
$ a different catalog file. In translating this text remove and/or reposition
$ the \t as necessary so that any text overflowing the second line is 
$ tabbed when beginning any additional lines.

UDISP_X_WARN	"\
This service aid cannot be run at this time.  Another application,\n\
such as X-windows, is currently using the display subsystem.  To run\n\
this service aid, no other application can be using the graphics\n\
system.\n\n\
To return to the Service Aid Selection menu, press Enter."
$
$ Leave line spacing as shown.

UDISP_NO_SA	"\
There is currently no service aid support for this display adapter.\n\n\
To return to the Service Aid Selection menu, press Enter."
$
$ Leave line spacing as shown.
