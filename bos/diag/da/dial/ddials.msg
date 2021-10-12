$ @(#)03        1.2.2.1  src/bos/diag/da/dial/ddials.msg, dadial, bos41J, 9523A_all 5/30/95 23:37:17
$
$   COMPONENT_NAME: DADIAL
$
$   FUNCTIONS:
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1995
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
$ The Dials are a series of 8 dial knobs that are arranged in two columns
$ on a flat rectangular holder.  This holder can lie flat on a surface, or it
$ can be set to stand up by attaching a backing plate to it.  The dials are
$ used in graphics applications.  They can be used to control the orientation
$ of an object displayed on the screen, or similar graphics functions.  See
$ a brochure for the device if any further assistance is needed.
$
$ END OF GENERAL NOTES


$quote "
$set DIAL_GENERIC

DIAL_TITLE "TESTING DIALS"
$
$ See general notes on length of title.

DIAL_TITLE_A "TESTING DIALS IN ADVANCED MODE"
$
$ See general notes on length of title.

DIAL_YES "YES"
$ This option is shown when a YES answer is possible.

DIAL_NO  "NO"
$ This option is shown when a NO answer is possible.

DIAL_ACTION "Move cursor to selection, then press Enter."
DIAL_PRS_ENT "When finished, press Enter."
DIAL_STND_BY "Please stand by."
DIAL_INST_PRS_ENT "To start the test, press Enter."

$set DIAL_WRP_NUM

DIAL_WRP_TITLE "TESTING DIALS\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ Leave line spacing as shown.  See general notes on length of title.

DIAL_WRP_TITLE_A "TESTING DIALS IN ADVANCED MODE\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ Leave line spacing as shown.  See general notes on length of title.

$set DIAL_RMV_ATTACHEMENT

DIAL_UNPLUG_CBL "\
UNPLUG.........the Graphics Input Cable, if attached, from the Dials device."
DIAL_PLUG_WRP   "\
PLUG...........the wrap plug (Part Number 22F9689) into the end of\n\
               the Graphics Input Cable."
$
$ See general notes on expanding ACTION lines.

$set DIAL_RECONNECT

DIAL_UNPLUG_WRP "\
UNPLUG.........the wrap plug from the Graphics Input Cable."
$
$ See general notes on expanding ACTION lines.

DIAL_PLUG_CBL "\
PLUG...........the Graphics Input Cable, if removed, into the Dials device."
$
$ See general notes on expanding ACTION lines.

$set DIAL_ORD

DIAL_ORD_INST "\
The following test requires each dial knob to be turned\n\
until the word 'ROTATED' appears next to the dial number.\n\
The word 'CLOCKWISE' or 'COUNTERCLOCKWISE' will appear\n\
at the bottom of the screen to indicate the direction\n\
the knob was turned."

DIAL_ORD_EXIT "\
TURN...........each dial knob until the word 'ROTATED' appears.\n\
OBSERVE........the direction of the turn indicated at the\n\
               bottom of the screen.\n\n\
If a dial knob is turned completely and the word 'ROTATED'\n\
does not appear or if the direction indicated is not correct,\n\
then use 'Exit' to end the test."
$
$ NOTE:  THIS MESSAGE CAN ONLY BE A MAXIMUM OF 7 LINES LONG.  Do not exceed
$ this seven line limit.

DIAL_ORD_EXITA "\
TURN...........each dial knob until the word 'ROTATED' appears.\n\
OBSERVE........the direction of the turn indicated at the\n\
               bottom of the screen.\n\n\
When finished, press Enter."
$
$ NOTE:  THIS MESSAGE CAN ONLY BE A MAXIMUM OF 7 LINES LONG.  Do not exceed
$ this seven line limit.

DIAL_ORD_QUEST "TESTING DIALS\n\n\
Did the word 'ROTATED' appear when the knob was turned and\n\
was the direction indicated correct?"
$

DIAL_ORD_QUESTA "TESTING DIALS IN ADVANCED MODE\n\n\
Did the word 'ROTATED' appear when the knob was turned and\n\
was the direction indicated correct?"
$

$set DIAL_RND

DIAL_RND_TITLE "TESTING DIALS\n\n\
Did the status of each dial change when it was turned?"
$
$ Leave line spacing as shown.  See general notes on length of title.

DIAL_RND_TITLE_A "TESTING DIALS IN ADVANCED MODE\n\n\
Did the status of each dial change when it was turned?"
$
$ Leave line spacing as shown.  See general notes on length of title.

DIAL_RND_INST "\
The following test checks for suspected problems occuring\n\
on specific dial knobs.  Turning each knob clockwise or\n\
counterclockwise will cause a + on the screen to move next\n\
to the dial number.  Turn the dial with the suspected problem\n\
and verify that the + moves in the same direction as the knob."

DIAL_RND_EXIT "\
TURN...........each knob with a suspected problem.\n\
OBSERVE........the + on the screen next to the dial number.\n\n\
If turning any dial knob does not cause the + to move on the screen\n\
in the correct direction, then use 'Exit' to end the test.\n\n\
When finished, press Enter."
$
$ NOTE:  THIS MESSAGE CAN ONLY BE A MAXIMUM OF 7 LINES LONG.  Do not exceed
$ this seven line limit.

DIAL_RND_EXITA "\
TURN...........each knob with a suspected problem.\n\
OBSERVE........the + on the screen next to the dial number.\n\n\
When finished, press Enter."
$
$ NOTE:  THIS MESSAGE CAN ONLY BE A MAXIMUM OF 7 LINES LONG.  Do not exceed
$ this seven line limit.

DIAL_RND_QUEST "TESTING DIALS\n\n\
Did the + next to each knob tested move in the same direction\n\
as the knob was turned?"
$

DIAL_RND_QUESTA "TESTING DIALS IN ADVANCED MODE\n\n\
Did the + next to each knob tested move in the same direction\n\
as the knob was turned?"
$

$set DIAL_INLOOP
DIAL_INLOOP_TITLE "TESTING DIALS IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title.
$ DO NOT TRANSLATE the next line, it's a set id used in the software.

$set DIAL_MISC
DIAL_NAME "Dial #%d -"
$
$ This message, when combined with DIAL_ROT, can be no longer than 22
$ characters.  Translate both messages accordingly.
$ The %d is the number [1..8] of the dial being tested.

DIAL_ROT "ROTATED"
$
$ This message, when combined with DIAL_NAME, can be no longer than 22
$ characters.  Translate both messages accordingly.

DIAL_CLK     "   CLOCKWISE    "
$
$ This message must take up the same number of characters as the DIAL_CNT_CLK
$ message.  Be sure to fill this message out with blanks if necessary.
$ This message can be no longer than 50 characters.

DIAL_CNT_CLK "COUNTERCLOCKWISE"
$
$ This message must take up the same number of characters as the DIAL_CLK
$ message.  Be sure to fill this message out with blanks if necessary.
$ This message can be no longer than 50 characters.

DIAL_BLANK " "
$ This message is blank.  Do not translate.

DIAL_PLUS "+"
$ This character is used as the cursor on the display to describe the
$ movement of the mouse.  Be sure that this character is the same
$ character that the user is told to watch in the MSE_STRM_MOVE
$ message elsewhere in this file.

$set DIAL_FRUS
$ ---------------------------------------------------------------
$ Reason code set used by device type "Dials"

DIAL_CBL "Dials cable needs replacement."
DIAL_COM_FAILED "Dials communication line failed."
DIAL_RESET_LINE "Dials selective reset line failed."
DIAL_RESET_FAILED "Dials reset failed."
DIAL_BAT_CODE "Dials basic assurance test failed."
DIAL_LCL_WRAP "Dials local wrap test failed."
DIAL_FAILED "Dials unit failed."
DIAL_POW "Device external power supply failure."
ASYNC_DIAL_CBL "Cable from Serial Port to Dial Unit Failed."
SER_PORT_FAILED "Serial port that Dial is connected to failed."
DIAL_CONFIG_FAIL "Device configuration fails."
DIAL_SOFTWARE_ERROR "Software."
DIAL_OPEN_ERROR "Device driver indicates a hardware failure."
DIAL_IOPLANAR "I/O Planar."
DIAL_CABLE "Cable"

$set DEVICE
$ ---------------------------------------------------------------

LPP_DEVICE_CANNOT_UNCONFIGURED "929032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s          Dials"


LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "929033 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state.\n\n\
The resource is\n\
- %1$s             %2$s          Graphics Input Adapter"


LPP_DEVICE_CANNOT_SET_TO_DEFINE "929034 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state.\n\n\
The resource is\n\
- %1$s             %2$s          Graphics Input Adapter"

DIAG_DEVICE_CANNOT_CONFIGURED "929035 \
THE DIAGNOSTICS DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s          Dials"


DEVICE_DOES_NOT_EXIST "929036 \
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, the because the system indicated that \n\
the resource does not exist.\n\n\
The resource is\n\
- %1$s             %2$s          Dials"

DIAG_DEVICE_CANNOT_UNCONFIGURED "929037 \
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Dials"


LPP_DEVICE_CANNOT_CONFIGURED "929038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Dials"


LPFK_DEVICE_CANNOT_CONFIGURED "929038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s                      Lighted Program Function Keys"

GIO_DEVICE_CANNOT_CONFIGURED "929038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s                      Graphics Input Adapter"

LPFK_DEVICE_CANNOT_UNCONFIGURED "929039 \
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Lighted Program Function Keys"

$

GIO_DEVICE_CANNOT_UNCONFIGURED "929039 \
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Graphics Input Adapter"

$


NEW_LPP_DEVICE_CANNOT_UNCONFIGURED "929032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s          Dials"
$


TTY_DEVICE_CANNOT_UNCONFIGURED "929032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, no testing was performed because the\n\
system indicated that the following resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s          Asynchronous Terminal"
$


SA_DEVICE_CANNOT_UNCONFIGURED "929032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, no testing was performed because the\n\
system indicated that the following resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s          Standard I/O Serial Port"
$
