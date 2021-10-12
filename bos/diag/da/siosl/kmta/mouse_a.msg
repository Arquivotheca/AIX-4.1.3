$ @(#)78	1.6  src/bos/diag/da/siosl/kmta/mouse_a.msg, damouse, bos411, 9428A410j 12/9/93 17:30:16
$
$   COMPONENT_NAME: DAMOUSE
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1990,1991,1992
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
$ END OF GENERAL NOTES


$quote "

$set MSE_GENERIC

MSE_DA_TITLE "TESTING MOUSE CONNECTOR"
$
$ See general notes on length of title.

MSE_DA_TITLE_A "TESTING MOUSE CONNECTOR IN ADVANCED MODE"
$
$ See general notes on length of title.

MSE_STND_BY "Please stand by."
$ Diagnostics application is in process.

MSE_PRS_ENTER "When finished, press Enter."
$ when the user is done testing must press to continue diagnostics

$set MSE_RMV_ATTACHEMENT

MSE_UNPLUG_CBL "\
UNPLUG.........the cable, if attached, from the mouse connector."
$
$ Disconnect the cable from the port and to insert a wrap plug,
$ See general notes on expanding ACTION lines is necessary.

DEVICE_CANNOT_UNCONFIGURE "823010 \
THE DEVICE CANNOT BE UNCONFIGURED\n\
Diagnostics cannot unconfigure the device attached to the adapter\n\
under test.\n\n\
The resource is\n\
- %1$s		%2$s		Mouse"
$

DEVICE_CANNOT_CONFIGURE "823011 \
THE DEVICE CANNOT BE CONFIGURED\n\
Diagnostics cannot configure the device attached to the adapter\n\
under test.\n\n\
The resource is\n\
- %1$s	        %2$s		Mouse" 
$


$set MSE_RECONNECT

MSE_PLUG_CBL "\
PLUG...........the mouse cable, if removed, into the mouse connector."
$
$ Reconnect the cable to the adapter port if removed.
$ See general notes on expanding ACTION lines is necessary.

$set MSE_INLOOP

MSE_INLOOP_TITLE "TESTING MOUSE CONNECTOR IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Display menu title, number of test completed trough the loop and
$ numer of errors found during testings.
$ Leave line spacing as shown.  See general notes on length of title.

$set MSE_ASK_USR

MSE_IS_A_MSE "\
Is there a mouse attached to the system?\n\n\
Move cursor to selection, then press Enter."
$ Ask the user if a mouse is present.
$ See general notes on expanding ACTION lines if necessary.

MSE_1 "YES"

MSE_2 "NO"

