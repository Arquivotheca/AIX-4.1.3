$ @(#)98        1.12.2.12  src/bos/diag/dutil/dutil.msg, dutil, bos411, 9428A410j 2/28/94 13:40:06
$
$   COMPONENT_NAME: DUTIL
$
$   FUNCTIONS: Diagnostic message file.
$
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
$       This file contains the corresponding messages for the menus and
$       errors generated from the Diagnostic Utility.
$
$ END OF GENERAL NOTES


$set MSET1
$ Leave two blank spaces at the beginning of every line after the first.
$quote "

MTITLE   "SERVICE AIDS SELECTION"
$
$ See general notes on length of title.

MLASTLINE "Move cursor to selection, then press Enter."

$set MSET2
SERVICE_HINTS "\
Service Hints\n\
  This selection displays service hints and errata information about the\n\
  maintenance package."

DISPLAY_RESULTS "\
Display Previous Diagnostic Results\n\
  This selection displays the results of previous diagnostic runs."

PROD_TOP "\
Product Topology\n\
  This selection updates the system Product Topology information. It should\n\
  be run whenever the system is installed, or when adapters or parts are\n\
  added, removed or replaced."

DSENSE "\
Hardware Error Report Service Aid\n\
  This selection provides a tool for viewing the hardware error log\n\
  and performing error log analysis."

DISPLAY_VPD "\
Display or Change Configuration or Vital Product Data (VPD)\n\
  This selection displays or changes the Configuration or VPD."

DISK_MEDIA "\
Disk Media\n\
  This selection provides the format disk and certify disk service aids."

DISKETTE_MEDIA "\
Diskette Media\n\
  This selection provides a tool for checking out a diskette."

TEST_PATTERN "\
Display Test Patterns\n\
  This selection displays patterns required to adjust all supported\n\
  displays."

LAN "\
Local Area Network Service Aid\n\
  This service aid is used to exercise the Local Area Network\n\
  communication adapters -- Token-Ring, Ethernet and Fiber\n\
  Distributed Data Interface (FDDI)."

ETHERNET "\
Service Aids For Use With Ethernet\n\
  (Ethernet is a Trademark of Xerox Corp.)\n\
  This selection provides a tool for diagnosing Ethernet problems."

DISPLAY_LIST "\
Display or Change Diagnostic Test List\n\
  This selection displays the resources tested by Diagnostics and allows\n\
  the resources to be added or deleted from the Diagnostic Test List."

SCSI_BUS "\
SCSI Bus\n\
  This selection provides a tool for diagnosing SCSI bus problems."

DIAG_PACKAGE "\
Diagnostic Package Utility Service Aid\n\
  This selection provides a tool for formatting diskettes, and\n\
  creating customized configuration diskettes for the standalone\n\
  diagnostic package."

DISK_UPDATE "\
Disk Based Diagnostic Update Service Aid\n\
  This selection provides a tool for updating disk based diagnostics\n\
  from diskette or tape."

MICROCODE_DOWN "\
Microcode Download\n\
  This selection provides a tool for downloading disk controller\n\
  microcode from a diskette."

TRACE "\
Trace Service Aid\n\
  This selection provides a tool for doing Trace Dump on the Serial Disk\n\
  subsystem."

BOOTLIST "\
Display/Alter Bootlist\n\
  This selection allows the list of boot devices to be displayed, altered,\n\
  or erased."

PERIODIC_DIAG "\
Periodic Diagnostics Service Aid\n\
  This selection provides a tool for periodic testing of hardware\n\
  resources and enabling or disabling Automatic Error Log Analysis."

$ All of the above messages have been reordered due to a customer requirement. 
$ The text is the same in most cases as those in the previous release. Also, the
$ MSG names have been renamed to simplify the ordering. 


$set MSET3
DIALS_LPFKEYS "\
Dials and Lpfkeys Configuration Service Aid\n\
  This selection provides a tool for configuring dials/lpfkeys to the\n\
  native serial ports."

AIX_SHELL "\
AIX Shell Prompt\n\
  This selection provides a Bourne Shell for the root user.\n\
  The root password must be entered to get to the shell prompt.\n\
  When finished, enter 'exit' to return to the Service Aids."

$set ESET
$quote "
ERROR1 "Internal error occured in %1$s.  Cannot initialize ODM."
ERROR2 "Cannot run %s."
ERROR3 "Cannot start process %s."
ERROR4 "Object class %s error."
ERROR5 "Usage: %s -m[...]"


$set DSKSET
$quote "
$
DIAGSA_END      "\
To continue, press Enter."
$

DIAGSA_REMOVE   "REMOVE.........the diskette, if any, from the diskette drive."
DIAGSA_INSERT   "INSERT.........%s into the diskette drive."
DIAGSA_ENTER    "When finished, press Enter."
DIAGSA_CANCEL   "\n\NOTE: press Cancel, if you don't want to read the diskette."
$
$       Menu Insert Diskette
$

DIAGSA_STANDBY  "Processing the data from the diskette.\n\n\
Please stand by."
$
$       Menu Standby
$

DIAGSA_RD_ROR   "\
An error occurred while reading the data from the diskette.\n\n\
Do you want to read the diskette again?\n\n\
Move cursor to selection, then press Enter."
$
$       menu Error
$

DIAGSA_NO       "NO"
DIAGSA_YES      "YES"
$
$ The following messages are new messages created to fix a consistency
$ problem with the way the words diagnostic diskette are being displayed.
$ From now on, anytime the words diagnostic diskette are used, the first
$ character of the word diagnostic must be capitalized.
$ Do not translate the following messages, as they are used only in the
$ Diagnostic diskette package.
$
MSG_DSKT_31     "\
Processing %s.\n\n\
Please stand by."
MSG_DSKT_32     "\
Bad diskette.  Insert a new %s.\n\n\
To continue, press Enter."
MSG_DSKT_33     "\
Incorrect diskette.  Insert %s.\n\n\
To continue, press Enter."
MSG_DSKT_35     "Supplemental Diagnostic diskette"
MSG_RESTORE_FILES "Restoring file(s) from tape device %s\n\n\
Please stand by."
MSG_RESTORE_ERROR "\
An error was received during the restore operation.\n\n\
The following service procedures are recommended.\n\
CLEAN..........the tape drive read/write head and tape path.  For\n\
	       instructions on cleaning, refer to the Setup and \n\
	       Operator Guide for this tape drive.\n\
\n\
OBTAIN.........a new diagnostic tape.\n\
	       NOTE: An old or worn out diagnostic tape\n\
	             may cause media errors.\n\
\n\
RUN............Advanced Diagnostics, to ensure this drive is \n\
	       functioning properly.\n\n\
Press Enter to continue."

DIAGSA_SUPP     "\
This Service Aid requires that all Supplemental Diagnostic diskettes be\n\
processed.\n\
Do you have any Supplemental Diagnostic diskettes that have\n\
not been processed?\n\n\
Move cursor to selection, then press Enter."

DIAGSA_MORE_DSKT     "\
Do you have any additional Supplemental Diagnostic diskettes\n\
that have not been processed?\n\n\
Move cursor to selection, then press Enter."

