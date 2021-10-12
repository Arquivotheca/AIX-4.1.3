$ @(#)12	1.1.2.7  src/bos/diag/util/umkdskt/umkdskt.msg, dsaumkdskt, bos411, 9428A410j 3/2/94 15:02:26
$
$   COMPONENT_NAME: DSAUMKDSKT
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1991,1994
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
$       This file contains the corresponding messages for the menus and
$	errors generated from the UMKDSKT Utility.
$
$ END OF GENERAL NOTES

$quote "
$set MKDSKT_SET

MKDSKT_TITLE "DIAGNOSTIC PACKAGE UTILITY SERVICE AID"
$
$ See GENERAL NOTES on titles

$
$	Menu 0x802021
$
MKDSKT_REMOVE	"REMOVE.........the diskette, if any, from the diskette drive."

MKDSKT_INSERT	"INSERT.........a %d MB diskette into the diskette drive."

MKDSKT_INSERT_CONS	"INSERT.........a blank formatted diskette into the diskette drive."
$ This menu is used for the consdef diskette

MKDSKT_ENTER	"When finished, press Enter."

MKDSKT_STANDBY	"Please stand by."

$
$	menu 0x802026
$
MKDSKT_CONSDEF "This menu is used to create a console configuration file that allows\n\
a terminal attached to any RS232 or RS422 adapter to be selected as\n\
a console device.  The default device is a RS232 tty attached to the\n\
first standard serial port(S1).\n\n\
If the default tty console selection is desired, then press\n\
COMMIT to continue.  Otherswise, modify the fields as required,\n\
then press COMMIT to continue."


MKDSKT_TCONSDEF	"MODIFY CONSOLE CONFIGURATION FILE"
MKDSKT_MODIFY	"Modify the fields as required."
MKDSKT_CONN	"Connection type"
MKDSKT_LOC	"Location"
MKDSKT_BAUD	"Speed (baud rate)"
MKDSKT_BITS	"Bits per character"
MKDSKT_SBITS	"Stop bits"
MKDSKT_XON	"Flow discipline"
MKDSKT_PARITY	"Parity"
MKDSKT_TERM	"TERM environment variable"

$
$	menu 0x802027
$
MKDSKT_REFRESH	"This menu is used to set the refresh rate used by the standalone diagnostic\n\
package to the refresh rate used by the display.  If the display's\n\
refresh rate is 77 Hz, then set the refresh rate below to 77.  The\n\
default refresh rate is 60 Hz.\n\n\
If the default refresh rate is desired then press COMMIT to continue.\n\
Else modify the required field below, then press COMMIT to continue." 

$
$	menu 0x802029
$
MKDSKT_CFG_NO_CHG "The Configuration Diskette was not created.  Since the default selections\n\
were taken, the Configuration Diskette is not needed."

$
$	menu 0x802025
$
MKDSKT_CFG_OK "\
The Configuration Diskette was written successfully.  To use this\n\
diskette when running diagnostics from the standalone diagnostic\n\
package, insert the diskette into the diskette drive before IPL'ing\n\
the system."


MKDSKT_RREFRESH	"Refresh rate"


$
$  New messages
MKDSKT_FORMAT_1MB "Format a 1 MB Diskette ( 720 KB formatted capacity )"

MKDSKT_FORMAT_2MB "Format a 2 MB Diskette ( 1.44 MB formatted capacity )"

MKDSKT_FORMAT_4MB "Format a 4 MB Diskette ( 2.88 MB formatted capacity )"

MKDSKT_CONFIG "Create a Standalone Diagnostic Package Configuration Diskette"

MKDSKT_PATCH "Create a Standalone Diagnostic Package Patch Diskette"

MKDSKT_SELECT_DISKETTE	"Move the cursor to selection, then press enter."

MKDSKT_FORMATTING	"Formatting the diskette".

MKDSKT_NO_DRIVE "A %s MB diskette drive is required to run this service\n\
aid.  This service aid could not find the device.  If this device is\n\
present on the system, run diagnostics to determine the problem.\n\n\
To continue, press Enter."
$

MKDSKT_FORMAT_COMPLETE	"The format of this diskette has successfully completed."

MKDSKT_FORMAT_BAD	"The format did not complete successfully.  this could be\n\
caused by a faulty diskette or a defective drive.  Run diagnostics\n\
on this diskette drive to assure that it is functioning properly."

MKDSKT_ENTER2 "Press Enter to continue."

MKDSKT_CFG_BAD "An error occurred while trying to create the configuration diskette."

