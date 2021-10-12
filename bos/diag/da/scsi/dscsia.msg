$ @(#)92	1.6.1.5  src/bos/diag/da/scsi/dscsia.msg, dascsi, bos411, 9428A410j 3/11/94 16:10:30
$
$   COMPONENT_NAME: DASCSI
$
$   FUNCTIONS: messages
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1992
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
$ These comments concern the techincal terms used in the messages.
$   SCSI:  is an acronym for "Small Computer Systems Interface."  SCSI is
$      a technical term.  Do not translate unless an equivalent tern is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character. The location is not translatable. It is 
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL MESSAGES


$quote "
$set DSCSIA_SET1
$quote "
$
WE_R_TESTING " "
$
$ This panel is displayed when the test starts.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

WE_R_TESTING_INLM " "
$
$ This panel is displayed when the test starts in loop mode.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

WE_R_ADVANCED " "
$
$ This panel is displayed when the test starts in advanced mode.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

LOOPMODE_STATUS " "
$
$ This panel is displayed in problem determination AND advanced mode
$ while in loop mode.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ SCSI is a technical term.  See general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

UNPLUG_PLUG "TESTING SCSI DEVICE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
The following test requires these steps to be performed:\n"
UPC   "\
UNPLUG.........the external SCSI data cable, if attached, from\n\
               the SCSI adapter card in location %1$s."
$
$ This panel is displayed in problem determination AND advanced
$ when entering loopmode.
$ %1$s is the name of the SCSI device.
$ %2$s and %3$s are the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ See general notes on how to expand ACTION lines if necessary.
   
PIEST "\
PLUG...........the external SCSI terminator into the SCSI adapter card\n\
	       in location %1$s."
$
$ This panel is displayed in problem determination AND advanced
$ when entering loopmode.
$ %1$s is the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ See general notes on how to expand ACTION lines if necessary.

REPLUG_DEV "TESTING SCSI DEVICE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
The following test requires these steps to be performed:\n"
UNPLUG_TERM "\
UNPLUG.........the external SCSI terminator from the SCSI adapter card in\n\
	       location %1$s."
$
$ This panel is displayed in problem determination AND advanced mode
$ when exiting loopmode.
$ %1$s is the name of the SCSI device.
$ %2$s and %3$s are the location of the SCSI device.
$ SCSI is a technical term.  See general notes. 
$ A SCSI terminator is a device the plugs into the end of the cable
$ to indicate that there are no more devices on the cable.
$ See general notes on how to expand ACTION lines if necessary.

REPLUG      "\
PLUG...........the external SCSI data cable, if it was removed,\n\
               into the SCSI adapter card in location %1$s."
$
$ This panel is displayed in problem determination AND advanced mode
$ when exiting loopmode.
$ %1$s is the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ See general notes on how to expand ACTION lines if necessary.

YES_OPTION "YES"
$ This option is shown when a YES answer is possible.

NO_OPTION  "NO"
$ This option is shown when a NO answer is possible.

CHOOSE_AN_OPTION  "Move cursor to selection, then press Enter.\n\n"
$ This message is shown when a multiple selection list is presented.

PRESS_ENTER "\nWhen finished, press Enter.\n"
$ This message is shown when a menu requires an action from the user.

EXT_TERMINATOR "TESTING SCSI DEVICE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
The following test requires an external SCSI terminator.\n\n\
NOTE:  If you do not have an external SCSI terminator,\n\
       the test for this device will stop.\n\n\
Do you have an external SCSI terminator?"
$
$ A SCSI terminator is a device the plugs into the end of the cable
$ to indicate that there are no more devices on the cable.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

INT_TERMINATOR "TESTING SCSI DEVICE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
The following test requires an internal SCSI terminator.\n\n\
Do you have an internal SCSI terminator?"
$
$ A SCSI terminator is a device the plugs into the end of the cable
$ to indicate that there are no more devices on the cable.
$ %1$s is the name of the SCSI device.
$ %2$s is the location of the SCSI device.
$ Leave line spacing as shown.  See general notes on length of title line.

UNPLUG_PLUG_INT "TESTING SCSI DEVICE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
The following test requires these steps to be performed:\n"
UPCI   "\
UNPLUG.........the internal SCSI data cable, if attached, from\n\
               the SCSI adapter card in location %1$s."
$
$ This panel is displayed in problem determination AND advanced
$ when entering loopmode.
$ %1$s is the name of the SCSI device.
$ %2$s and %3$s are the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ See general notes on how to expand ACTION lines if necessary.
   
PIEST1 "\
PLUG...........the internal SCSI terminator into the SCSI adapter card\n\
	       in location %1$s."
$
$ This panel is displayed in problem determination AND advanced
$ when entering loopmode.
$ %1$s is the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ See general notes on how to expand ACTION lines if necessary.

REPLUG_INT_DEV "TESTING SCSI DEVICE ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
The following test requires these steps to be performed:\n"
UNPLUG_INT_TERM "\
UNPLUG.........the internal SCSI terminator from the SCSI adapter card in\n\
	       location %3$s."
$
$ This panel is displayed in problem determination AND advanced mode
$ when exiting loopmode.
$ %1$s is the name of the SCSI device.
$ %2$s and %3$s are the location of the SCSI device.
$ SCSI is a technical term.  See general notes. 
$ A SCSI terminator is a device the plugs into the end of the cable
$ to indicate that there are no more devices on the cable.
$ See general notes on how to expand ACTION lines if necessary.

REPLUG_INT      "\
PLUG...........the internal SCSI data cable, if it was removed,\n\
               into the SCSI adapter card in location %1$s."
$
$ This panel is displayed in problem determination AND advanced mode
$ when exiting loopmode.
$ %1$s is the location of the SCSI device.
$ SCSI is a technical term.  See general notes.
$ See general notes on how to expand ACTION lines if necessary.

SCSI_ADAPTER "SCSI DEVICE ADAPTER"
$
$ This message is used for title line substitution.
$ SCSI is a technical term.  See general notes.
$ See general notes on how to expand ACTION lines if necessary.

PTC_MENU_GOAL "%1$x \
SCSI ADAPTER HAS A TEMPORARY PROBLEM\n\n\
The diagnostics application for the SCSI adapter has detected a temporary\n\
problem with the SCSI adapter. This problem can be corrected by powering\n\
the system off and leaving the system off for a minimum of 5 minutes.\n\
\n\
If the problem presists after the system is powered on, report SRN %2$s\n\
which was displayed prior to this message.\n\
\n\
When this problem occurs frequently posssible causes are:\n\
\n\
- Plugging or unplugging the SCSI cable while the system is powered on.\n\
\n\
- A defective SCSI cable. \n\
\n\
- A device attached to the SCSI bus."

FUSE_MENU_GOAL "869010 \
SCSI ADAPTER FUSE IS BLOWN\n\n\
The diagnostics application for the SCSI adapter has detected a blown fuse\n\
on the SCSI adapter.\n\
\n\
The primary cause of a blown fuse is plugging or unplugging the SCSI cable\n\
while the system is powered on. Other probable causes of a blown fuse are:\n\
\n\
- A defective SCSI cable. \n\
\n\
- A device attached to the SCSI bus."


SCSI1_FUSE_MENU_GOAL "%1$x \
ADDITIONAL SERVICE INFORMATION FOR SCSI ADAPTER IN LOCATION %2$s\n\n\
A blown fuse on the SCSI adapter has been detected.\n\
Some probable causes for a blown fuse are listed below.\n\n\n\
PROBABLE CAUSE........A SCSI-2 Differential device may be attached\n\
                      to the SCSI Single-Ended adapter.\n\
CORRECTIVE ACTION.....Remove any SCSI-2 Differential device from this SCSI\n\
\t\t      Single-Ended adapter. This SCSI Single-Ended adapter\n\
		      only supports SCSI Single-Ended devices.\n\n\
PROBABLE CAUSE........Plugging or unplugging a cable or terminator\n\
                      while the system is powered on (hot plugging).\n\
CORRECTIVE ACTION.....Hot plugging is not supported.\n\n\
PROBABLE CAUSE........A defective SCSI cable or terminator.\n\
CORRECTIVE ACTION.....Inspect all SCSI cables and terminators for damage.\n\
                      Replace any suspect SCSI cables or terminators.\n\
                      Ensure all cables and terminators are compatible\n\
                      with this SCSI adapter.\n\n\
PROBABLE CAUSE........Any device attached to this SCSI adapter may\n\
                      cause this type of problem.\n\
CORRECTIVE ACTION.....Refer to the Service Hints section of the Common\n\
                      Diagnostics and Service Guide for more information."

SCSI2_DE_MENU_GOAL "%1$x \
ADDITIONAL SERVICE INFORMATION FOR SCSI ADAPTER IN LOCATION %2$s\n\
A temporary problem with the SCSI-2 adapter has been detected.\n\
Some probable causes for this problem are listed below.\n\n\
PROBABLE CAUSE........A SCSI Single-Ended device may be attached\n\
                      to the SCSI-2 Differential adapter.\n\
CORRECTIVE ACTION.....Remove any SCSI Single-Ended device from this SCSI-2\n\
		      Differential adapter. This SCSI-2 Differential adapter\n\
		      only supports SCSI-2 Differential devices.\n\n\
PROBABLE CAUSE........A defective SCSI cable or terminator.\n\
CORRECTIVE ACTION.....Inspect all SCSI cables and terminators for damage.\n\
                      Replace any suspect SCSI cables or terminators.\n\
                      Ensure all cables and terminators are compatible\n\
                      with this SCSI-2 Differential adapter.\n\n\
PROBABLE CAUSE........Any device attached to this SCSI adapter may\n\
                      cause this type of problem.\n\
CORRECTIVE ACTION.....Refer to the Service Hints section of the Common\n\
                      Diagnostics and Service Guide for more information.\n\n\
PROBABLE CAUSE........PTC (Positive Temperature Coefficient) resistor\n\
                      has tripped.\n\
CORRECTIVE ACTION.....Power down the system for a minimum of 5\n\
                      minutes to allow the PTC to return to normal\n\
                      operating temperature. If this problem persists,\n\
                      use the SRN number (%3$s) which was displayed on\n\
                      the previous screen."


SCSI1_SE_MENU_GOAL "%1$x \
ADDITIONAL SERVICE INFORMATION FOR SCSI ADAPTER IN LOCATION %2$s\n\
A temporary problem with the SCSI adapter has been detected.\n\
Some probable causes for this problem are listed below.\n\n\
PROBABLE CAUSE........A SCSI-2 Differential device may be attached\n\
                      to the SCSI Single-Ended adapter.\n\
CORRECTIVE ACTION.....Remove any SCSI-2 Differential device from this SCSI\n\
		      Single-Ended adapter. This SCSI Single-Ended adapter\n\
		      only supports SCSI Single Ended devices.\n\n\
PROBABLE CAUSE........Plugging or unplugging a cable or terminator\n\
                      while the system is powered on (hot plugging).\n\
CORRECTIVE ACTION.....Hot plugging is not supported.\n\n\
PROBABLE CAUSE........A defective SCSI cable or terminator.\n\
CORRECTIVE ACTION.....Inspect all SCSI cables and terminators for damage.\n\
                      Replace any suspect SCSI cables or terminators.\n\
                      Ensure all cables and terminators are compatible\n\
                      with this SCSI adapter.\n\n\
PROBABLE CAUSE........Any device attached to this SCSI adapter may\n\
                      cause this type of problem.\n\
CORRECTIVE ACTION.....Refer to the Service Hints section of the Common\n\
                      Diagnostics and Service Guide for more information.\n\
PROBABLE CAUSE........PTC (Positive Temperature Coefficient) resistor\n\
                      has tripped.\n\
CORRECTIVE ACTION.....Power down the system for a minimum of 5\n\
                      minutes to allow the PTC to return to normal\n\
                      operating temperature. If this problem persists,\n\
                      use the SRN number (%3$s) which was displayed on\n\
                      the previous screen."

SCSI2_SE_MENU_GOAL "%1$x \
ADDITIONAL SERVICE INFORMATION FOR SCSI ADAPTER IN LOCATION %2$s\n\
A temporary problem with the SCSI-2 adapter has been detected.\n\
Some probable causes for this problem are listed below.\n\n\
PROBABLE CAUSE........A SCSI-2 Differential device may be attached\n\
		      to the SCSI-2 Single-Ended adapter.\n\
CORRECTIVE ACTION.....Remove any SCSI-2 Differential device from this SCSI-2\n\
		      Single-Ended adapter. This SCSI-2 Single-Ended adapter\n\
		      only supports Single Ended devices.\n\n\
PROBABLE CAUSE........Plugging or unplugging a cable or terminator\n\
                      while the system is powered on (hot plugging).\n\
CORRECTIVE ACTION.....Hot plugging is not supported.\n\n\
PROBABLE CAUSE........A defective SCSI cable or terminator.\n\
CORRECTIVE ACTION.....Inspect all SCSI cables and terminators for damage.\n\
                      Replace any suspect SCSI cables or terminators.\n\
                      Ensure all cables and terminators are compatible\n\
                      with this SCSI adapter.\n\n\
PROBABLE CAUSE........Any device attached to this SCSI adapter may\n\
                      cause this type of problem.\n\
CORRECTIVE ACTION.....Refer to the Service Hints section of the Common\n\
                      Diagnostics and Service Guide for more information.\n\
PROBABLE CAUSE........PTC (Positive Temperature Coefficient) resistor\n\
                      has tripped.\n\
CORRECTIVE ACTION.....Power down the system for a minimum of 5\n\
                      minutes to allow the PTC to return to normal\n\
                      operating temperature. If this problem persists,\n\
                      use the SRN number (%3$s) which was displayed on\n\
                      the previous screen."
