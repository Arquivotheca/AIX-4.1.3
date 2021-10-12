$ @(#)70	1.7  src/bos/diag/da/mouse/mouse.msg, damouse, bos411, 9428A410j 3/4/94 16:36:19
$
$   COMPONENT_NAME: DAMOUSE
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1990,1994
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

MSE_TITLE "TESTING 3-BUTTON MOUSE DEVICE"
$
$ Diagnostics menu title.
$ See general notes on length of title.

MSE_TITLE_A "TESTING 3-BUTTON MOUSE DEVICE IN ADVANCED MODE"
$
$ Diagnostics menu title in advanced mode.
$ See general notes on length of title.

MSE_YES "YES"
$ This option is shown when a YES answer is possible.

MSE_NO  "NO"
$ This option is shown when a NO answer is possible.

MSE_ACTION "Move cursor to selection, then press Enter"
$ Users instruction on how to make a selection.

MSE_EXIT "When finished, press Enter."
$ Tell the user to press the Enter key after testing is completed.

MSE_STND_BY "Please stand by."

DEVICE_CANNOT_UNCONFIGURE "%1$s \
THE DEVICE CANNOT BE UNCONFIGURED\n\
Diagnostics cannot unconfigure the device under test.\n\n\
The resource is\n\
- %2$s          %3$s            Mouse"
$

$set MSE_BUTTONS_TEST

MSE_BUT_PRESS "\
This test requires each mouse button to be pressed and released.\n\
As each mouse button is pressed and released, the corresponding\n\
rectangle on the display will change from OFF to ON.\n\n\
PRESS..........each button on the mouse device several times.\n\
OBSERVE........the corresponding rectangle on the display\n\
               and verify the status change from OFF to ON."
$
$ NOTE:  THIS MESSAGE CAN TAKE UP ONLY 7 LINES.  Do not exceed this
$ seven line limit.
$ Instruct the user to press and release each button on the mouse.
$ Instruct the user to observe the screen status of each button.
$ Leave line spacing as shown.  See general notes on expanding ACTIONS.

$set MSE_STREAM

MSE_STRM_MOVE "\
This test requires the mouse to be moved in all directions.\n\
As the mouse is moved, the cursor on the display should move\n\
in the same manner.\n\n\
MOVE...........the mouse on a smooth, hard surface in all directions.\n\
OBSERVE........the corresponding movement of the + on the display."
$
$ NOTE:  THIS MESSAGE CAN TAKE UP ONLY 7 LINES.  Do not exceed this
$ seven line limit.
$ The '+' symbol used in the message above can be changed to any 
$ character that is accessible in the translating language.  If this
$ character is changed, make sure that the MSE_PLUS message is changed
$ to be the equivalent character.
$ Requirement of the test is to move the device in all directions and to observe
$ the cursor movement on the screen.
$ Leave line spacing as shown.  See general notes on expanding ACTIONS.

$set MSE_RESPONSE

MSE_RESP_TITLE "TESTING 3-BUTTON MOUSE DEVICE\n\n\
Did each rectangle change from OFF to ON when the corresponding\n\
mouse button was pressed and released?"
$
$ Ask the user to select his/her observation from pressing down the buttons.
$ Leave line spacing as shown.  See general notes on length of title.

MSE_RESP_TITLE_A "TESTING 3-BUTTON MOUSE DEVICE IN ADVANCED MODE\n\n\
Did each rectangle change from OFF to ON when the corresponding\n\
mouse button was pressed and released?"
$
$ Ask the user to select his/her observation from pressing down the buttons.
$ Leave line spacing as shown.  See general notes on length of title.

MSE_RESP_O_T "TESTING 3-BUTTON MOUSE DEVICE\n\n\
Did the mouse work correctly?"
$
$ Ask the use if the mouse worked as expected.
$ Leave line spacing as shown.  See general notes on length of title.

MSE_RESP_O_T_A "TESTING 3-BUTTON MOUSE DEVICE IN ADVANCED MODE\n\n\
Did the mouse work correctly?"
$
$ Ask the use if the mouse worked as expected in advance mode.
$ Leave line spacing as shown.  See general notes on length of title.

$set MSE_INLOOP

MSE_INLOOP_TITLE "TESTING 3-BUTTON MOUSE IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$ Leave line spacing as shown.  See general notes on length of title.

$set MSE_CLEAN

MSE_CLEAN_ACT "\
Refer to the 'Cleaning the Mouse' section of the\n\
System Unit Operator Guide for instructions on cleaning the mouse."
$
$ Determine the proper name of the manual for each translating country.

MSE_CLEAN_INST "\
CLEAN..........the mouse as directed in the System Unit Operator Guide."
$
$ Determine the proper name of the manual for each translating country.

$set MSE_WINDOW

MSE_ON  "ON "
MSE_OFF "OFF"
$ Previous two lines they use the word toggle ON/OFF  

MSE_LEFT "L"
$ Indicate Left button of the mouse.  Translate accordingly.

MSE_RIGHT "R"
$ Indicate Right button of the mouse.  Translate accordingly.

MSE_MIDDLE "M"
$ Indicate Middle button of the mouse.  Translate accordingly.

MSE_BLANK " "
$ This message is blank.  Do not translate.

MSE_PLUS "+"
$ This character is used as the cursor on the display to describe the 
$ movement of the mouse.  Be sure that this character is the same 
$ character that the user is told to watch in the MSE_STRM_MOVE
$ message elsewhere in this file.

$ generic messages for either 2 or 3-button mouse

$set MSE_NEWTITLE

MSE_TITLE_C "TESTING MOUSE DEVICE"
$
$ Diagnostics menu title.
$ See general notes on length of title.

MSE_TITLE_D "TESTING MOUSE DEVICE IN ADVANCED MODE"
$
$ Diagnostics menu title in advanced mode.
$ See general notes on length of title.

$set MSE_RETURN

MSE_RETN_TITLE "TESTING MOUSE DEVICE\n\n\
Did each rectangle change from OFF to ON when the corresponding\n\
mouse button was pressed and released?"
$
$ Ask the user to select his/her observation from pressing down the buttons.
$ Leave line spacing as shown.  See general notes on length of title.

MSE_RETN_TITLE_A "TESTING MOUSE DEVICE IN ADVANCED MODE\n\n\
Did each rectangle change from OFF to ON when the corresponding\n\
mouse button was pressed and released?"
$
$ Ask the user to select his/her observation from pressing down the buttons.
$ Leave line spacing as shown.  See general notes on length of title.

MSE_RETN_O_T "TESTING MOUSE DEVICE\n\n\
Did the mouse work correctly?"
$
$ Ask the use if the mouse worked as expected.
$ Leave line spacing as shown.  See general notes on length of title.

MSE_RETN_O_T_A "TESTING MOUSE DEVICE IN ADVANCED MODE\n\n\
Did the mouse work correctly?"
$
$ Ask the use if the mouse worked as expected in advance mode.
$ Leave line spacing as shown.  See general notes on length of title.

$set MSE_LOOP

MSE_LOOP_TITLE "TESTING MOUSE IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$ Leave line spacing as shown.  See general notes on length of title.

