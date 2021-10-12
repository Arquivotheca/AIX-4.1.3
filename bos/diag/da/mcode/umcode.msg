$ @(#)57        1.4.1.5  src/bos/diag/da/mcode/umcode.msg, damcode, bos411, 9428A410j 3/25/94 14:19:01
$
$   COMPONENT_NAME: DAMCODE
$
$   FUNCTIONS: messages
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ 
$ GENERAL NOTES
$
$ Do not translate %d, %s, or \t in any messages.  They
$   are used for word or number substitution and noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc, 
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ The location of a resource is in the form of xx-xx-xx-x where x is an 
$ alpha-numeric character.  The location is not translatable.  It is
$ an alpha-numeric descriptor of where the resource can be found.
$
$ These comments concern the TITLE LINES at the top of the diagnostic screen.
$   The title must be in all capital letters.  The first line 
$   of the title cannot be longer than 70 characters starting from
$   column 1.  If the line is greater than 70, it may be continued on
$   the next line.  Leave line spacing as shown:  one blank line after
$   the last title line.  For example:
$
$ *****
$ TESTING PORT 12 OF THE 16-PORT ASYNCRONOUS ADAPTER IN PLANAR SLOT 2
$ IN ADVANCED MODE
$
$ Please stand by.
$ *****
$
$ These comments concern the user ACTIONS in all capital letters.
$   If translations require the creation of new lines, begin the new lines
$   in the column immediately following the row of periods.
$   For example:
$
$ *****
$ ACTION.........the one line of translation might need more than one line, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$ END OF GENERAL NOTES


$quote "
$set MCODE
$

TITLE "MICROCODE DOWNLOAD SERVICE AID"
$
$ Title line cannot be more than 70 characters on the first line.

Select "Move cursor to selection, then press Enter.\n"
$ This message is shown when a mulitple selection list is presented.

YES_OPTION "YES"
$ This option is shown when a YES answer is possible.

NO_OPTION "NO"
$ This option is shown when a NO answer is possible.

STANDBY_DL "Please stand by.  The microcode is being downloaded."

MOTORUP "Please stand by.  The disk drive motor is being started."

INSERT "\
INSERT.........the microcode diskette into the diskette drive.\n\n\
When finished, press Enter."
$
$ Leave line spacing between sentences as shown.

ERR_ODM "The microcode download cannot be performed.\n\n\
A software error has occurred.  The system's configuration\n\
data could not be accessed.\n\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

ERR_HW "The microcode download cannot be performed.\n\n\
A hardware error has occurred.  The device selected\n\
could not be accessed.\n\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

ERR_SW "The microcode download cannot be performed.\n\n\
A software error has occurred.\n\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

SELECT_LEVEL "MICROCODE DOWNLOAD SERVICE AID\n\n\
Select the microcode level to download to the drive.\n"

LEVEL1 "Download the latest level of microcode"
$
$ This choice allows the user to download the most recent microcode level.

LEVEL2 "Download the previous level of microcode"
$
$ This choice allows the user to return back to a previous level
$ of microcode in case a problem occurred with the disk drive after
$ the latest level of microcode was downloaded.

REMOVE_TAPE "Please remove tape from the drive, if one is inserted.\n\n\
When finished, press Enter."

MICRO_DISKETTE "\
Do you have the microcode on a diskette."

SELECT_DEVICE "MICROCODE DOWNLOAD SERVICE AID\n\n\
Select the device to receive the microcode download."
$
$ Title line cannot be more than 70 characters on the first line.
$ Leave line spacing between sentences as shown.

N_INCOMPAT "The microcode download could not be performed.\n\n\
The Load ID of the device you selected does not match the\n\
Load ID of the microcode.\n\n\
To continue, press Enter."
$
$ The microcode Load ID is an identifier that ensures that the microcode
$ downloaded is compatible with the drive.
$ Leave line spacing between sentences as shown.

N_FINISHED "The microcode download has completed successfully.\n\n\
Once you have finished using this service aid you should perform\n\
the system's shutdown procedure and then load diagnostics to test\n\
the device or devices on which the microcode has been downloaded.\n\n\
To continue, press Enter."
$
$ Leave line spacing between sentences as shown.

STANDBY_READ "Please stand by.  The microcode is being read from the media."

ERR_NO_FILE "The microcode download cannot be performed.\n\n\
An error occurred while attempting to access the microcode file.\n\
This is an indication of one of the following conditions: \n\n\
1. The latest level of microcode is not present.\n\
2. The previous level of microcode is not present.\n\
3. An error occured while attempting to read the microcode file.\n\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

ERR_CHECKSUM "The microcode download cannot be performed.\n\n\
A checksum error occurred while downloading the microcode.\n\
The microcode file is not useable.\n\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

ERR_NO_DEVICES "The microcode download cannot be performed.\n\n\
The system's configuration indicates that there are no device types\n\
configured that are designed to accept a microcode download.\n\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

ERR_READING_FILE "The microcode download cannot be performed.\n\n\
An error occurred attempting to read the source of the\n\
microcode file.\n\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

ERR_DEVICE_BUSY "The microcode download cannot be performed.\n\n\
The device you chose is not available to accept a microcode\n\
download.  To perform a microcode download on this device you must\n\
use this service aid from the removable media package.\n\
\n\
To return to the Service Aids Selection menu, press Enter."
$
$ This message is used when the microcode download is to a drive that
$ is in use because the diagnostics or AIX is using the drive.  In order
$ to download this microcode, the diagnostic must be run from a removable
$ media package.
$ Leave line spacing between sentences as shown.

ERR_AT_LEVEL "The microcode download cannot be performed.\n\n\
The microcode on the device chosen is already at the\n\
correct level.\n\n\
To continue, press Enter."
$
$ This message is displayed when the disk drive that was selected to
$ have the microcode downloaded was already at the correct level.
$ Leave line spacing as shown.

NO_CURR_LEVEL "MICROCODE DOWNLOAD SERVICE AID\n\n\
The current level of microcode is not present on the microcode\n\
media.  This microcode is necessary if you desire to return to the\n\
current level of microcode after downloading a new level of microcode.\n\n\
Do you want to continue with the microcode download?"
$
$ This message is presented after a user selects a drive to download 
$ microcode to and it is determined that the current level of microcode
$ is not on the microcode diskette.  It requires a YES or NO reply.

MCODE_BOOT "Boot Block microcode"
$ Microcode type name. This microcode used on IPL time.

MCODE_BLOCK "Functional microcode"
$ Microcode type name. This microcode used when the system is up and running.

MCODE_JUMPER "Do you have a jumper installed on the card?"
$ Ask the user if the boot block microcode jumper is installed on the adapter.

DEVICE "Device at location %1$-16.16s "

MCODE_JUMPER_REQ "Downloading the boot block microcode requires the jumper\n\
to be installed on the adapter.\n\n\n\
After the jumper has been installed, run this service aid again.\n\n\n\
When finished, press Enter."
$ This message is presented after the user selects a boot block microcode,
$ which requires a jumper to be installed on the adapter.

NO_DOSDIR "\
The microcode download cannot be performed.\n\n\
The command \"dosdir\" or \"dosread\" is not found on the system.\n\n\
To perform the download of the microcode run the non-disk based\n\
diagnostics which were shipped with the machine. These must be version\n\
2.4 or higher.\n\n\
When finished, press Enter."

UNLOAD_FAILED "\
A software error occured while attempting to unload diagnostics\n\
kernel extension."

TAPE_IN_DRIVE "The drive indicates that there is a tape in the drive.\n\
Remove the tape from the drive before attempting to download microcode.\n\
\n\
To continue, press Enter."
$

ERR_ADAPTER_BUSY "The microcode download cannot be performed.\n\n\
The adapter you chose is not available to accept a microcode download.\n\
To perform a microcode download on this adapter, you must load Diagnostics\n\
from a device that does not use this adapter.\n\
\n\
To return to the Service Aids Selection menu, press Enter."
$
$ Leave line spacing between sentences as shown.

