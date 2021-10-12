$ @(#)93        1.1.2.1  src/bos/diag/da/lpfk/dlpfk.msg, dalpfk, bos41J, 9523A_all 5/30/95 23:40:51
$
$   COMPONENT_NAME: DALPFK
$
$   FUNCTIONS: None
$
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
$ Toggle is an action of the key that changes for each press.  For instance
$  pressing a key once will turn a light on; pressing it again will turn it
$  off; etc.
$
$ END OF GENERAL NOTES


$quote "
$set LPF_GENERIC

LPF_TITLE "TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK)"
$
$ See general notes on length of title line.

LPF_TITLE_A "TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK) IN ADVANCED MODE"
$
$ Display diagnostics title.
$ See general notes on length of title line.

LPF_YES "YES"
$ This option is shown when a YES answer is possible.

LPF_NO  "NO"
$ This option is shown when a NO answer is possible.

LPF_ACTION "Move cursor to selection, then press Enter."
LPF_PRS_ENT "When finished, press Enter."
LPF_INST_PRS_ENT "To start the test, press Enter "

LPF_STND_BY "Please stand by."
$ Diagnostics is in process.

$set LPF_WRP_NUM

LPF_WRP_TITLE "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK)\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ Ask the user for a wrap plug, upon user's respond diagnostics takes certain
$ actions.
$ Leave line spacing as shown.  See general notes on length of title line.

LPF_WRP_TITLE_A "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK) IN ADVANCED MODE\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ Ask the user for a wrap plug.
$ Leave line spacing as shown.  See general notes on length of title line.

$set LPF_RMV_ATTACHEMENT

LPF_UNPLUG_CBL "\
UNPLUG.........the cable, if attached, from the LPFK."
$
$ Leave line spacing as shown.  See general notes on expanding ACTIONS.

LPF_PLUG_WRP "\
PLUG...........the wrap plug (Part Number 22F9689) into the end\n\
               of the LPFK cable."
$
$ Action must be taken by the user to remove the cable from the LPFK
$ device and to insert a wrap plug at the end of the cable.
$ Leave line spacing as shown.  See general notes on expanding ACTIONS.

$set LPF_RECONNECT

LPF_UNPLUG_WRP "\
UNPLUG.........the wrap plug from the LPFK cable."
$
$ Leave line spacing as shown.  See general notes on expanding ACTIONS.

LPF_PLUG_CBL "\
PLUG...........the cable, if removed, into the LPFK."
$
$ The user must remove the wrap plug from the LPFK cable and reconnect
$ the cable to LPFK device if removed.
$ Leave line spacing as shown.  See general notes on expanding ACTIONS.

$set LPF_LIGHT_ON

LPF_LIGHT_TITLE "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK)\n\n\
Are all the lights on the LPFK ON?"
$
$ Ask the user if the light are lit on the LPFKs.
$ Leave line spacing as shown.  See general notes on length of title line.

LPF_LIGHT_TITLE_A "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK) IN ADVANCED MODE\n\n\
Are all the lights on the LPFK device ON?"
$
$ Ask the user if the light are lit on the LPFKs.
$ Leave line spacing as shown.  See general notes on length of title line.

$set LPF_LIGHT_OFF

LPF_UNLIT_TITLE "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK)\n\n\
Are all the lights on the LPFK OFF?"
$
$ Ask the user if the light are unlit on the LPFKs.
$ Leave line spacing as shown.  See general notes on length of title line.

LPF_UNLIT_TITLE_A "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK) IN ADVANCED MODE\n\n\
Are all the lights on the LPFK OFF?"
$
$ Ask the user if the light are unlit on the LPFKs.
$ Leave line spacing as shown.  See general notes on length of title line.

$set LPF_LIGHT_ORD

LPF_ORD_INST "\
The following test will cycle through all of the keys on\n\
the LPFK, lighting them one at a time.  As each key becomes\n\
illuminated, press and release the key to turn it off.\n\n\
PRESS..........each key as it becomes illuminated."
$
$ The user must press each lit key on the LPFK in order to report exact
$ problem.
$ Leave line spacing as shown.  See general notes on expanding ACTIONS.

LPF_ORD_EXIT "\
NOTE : If pressing any illuminated key does not cause its light to turn OFF,\n\
then use 'Exit' to end the test."

$set LPF_LIGHT_RND

LPF_RND_TITLE "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK)\n\n\
Did each key toggle ON and OFF when pressed?"

LPF_RND_TITLE_A "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK) IN ADVANCED MODE\n\n\
Did each key toggle ON and OFF when pressed?"

LPF_RND_INST_E "\
The following test checks for suspected problems occuring\n\
on specific keys.  Pressing a key will cause its light\n\
to turn ON or OFF.  Repeatedly press and then release\n\
each specific problem key and verify that the status\n\
of the light changes with each press of the key.\n\n\
To start the test, press Enter."
$
$ Leave line spacing as shown.

LPF_RND_INST "\
The following test checks for problems occuring on specific keys.\n\n\
PRESS..........repeatedly each key with a suspected problem.\n\
OBSERVE........the light on the key and verify that it toggles\n\
               between OFF and ON."
$
$ Leave line spacing as shown.

LPF_RND_EXIT "\
If pressing any key does not cause its light to toggle ON and OFF,\n\
then use 'Exit' to end the test.\n\n\
When finished, press Enter."

$set LPF_INLOOP

LPF_INLOOP_TITLE "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK) IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ Display menu title, number of times the loop passing through and number of
$ errors found while it's running.
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

$set HFT_ORD_OK

HFT_ORD_A_OK_TITLE "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK) IN ADVANCED MODE\n\n\
Was there a problem with the previous test?"
$
HFT_ORD_OK_TITLE "\
TESTING LIGHTED PROGRAM FUNCTION KEYBOARD (LPFK)\n\n\
Was there a problem with the previous test?"
$

LPP_DEVICE_CANNOT_UNCONFIGURED "930032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s          Lighted Program Function Keyboard"


LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "930033 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state.\n\n\
The resource is\n\
- %1$s             %2$s          Lighted Program Function Keyboard"


LPP_DEVICE_CANNOT_SET_TO_DEFINE "930034 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state.\n\n\
The resource is\n\
- %1$s             %2$s          Lighted Program Function Keyboard"

DIAG_DEVICE_CANNOT_CONFIGURED "930035 \
THE DIAGNOSTICS DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s          Lighted Program Function Keyboard"


DEVICE_DOES_NOT_EXIST "930036 \
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, the because the system indicated that \n\
the resource does not exist.\n\n\
The resource is\n\
- %1$s             %2$s          Lighted Program Function Keyboard"

DIAG_DEVICE_CANNOT_UNCONFIGURED "930037 \
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Lighted Program Function Keyboard"


LPP_DEVICE_CANNOT_CONFIGURED "930038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Lighted Program Function Keyboard"


DIALS_LPP_DEVICE_CANNOT_CONFIGURED "930038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s                      Dials"

GIO_DEVICE_CANNOT_CONFIGURED "930038 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s                      Graphics Input Adapter"


TTY_DEVICE_CANNOT_UNCONFIGURED "930032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, no testing was performed because the\n\
system indicated that the following resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s          Asynchronous Terminal"
$


SA_DEVICE_CANNOT_UNCONFIGURED "930032 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, no testing was performed because the\n\
system indicated that the following resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s          Standard I/O Serial Port"
$


$set 11
$ ---------------------------------------------------------------
$ Reason code set used by device type "LPF Keys"
$ LPFK stands for Lighted Program Function Keyboard.

LPF_CBL "LPFK cable needs replacement."
LPFK_COM_FAILED "LPFK communication line failed."
LPFK_RESET_LINE "LPFK selective reset line failed."
LPFK_RESET_FAILED "LPFK reset failed."
LPFK_BAT_CODE "LPFK basic assurance test failed."
LPFK_LCL_WRAP "LPFK local wrap test failed."
LPFK_FAILED "LPFK unit failed."
LPFK_POW "Device external power supply failure."
ASYNC_LPFK_CBL "Cable from Serial Port to LPFK Unit Failed."
SERIAL_PORT_FAILED "Serial port that LPFK is connected to failed."
LPFK_CONFIG_FAIL "Device configuration fails."
LPFK_SOFTWARE_ERROR "Software."
LPFK_OPEN_ERROR "Device driver indicates a hardware failure."
LPFK_IOPLANAR "I/O Planar."
LPFK_CABLE "Cable"

