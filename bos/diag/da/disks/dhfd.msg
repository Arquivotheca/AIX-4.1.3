$ @(#)48	1.7  src/bos/diag/da/disks/dhfd.msg, dadisks, bos411, 9428A410j 1/5/93 14:04:29
$
$   COMPONENT_NAME: DADISKS
$
$   FUNCTIONS: 
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
$ These comments concern the technical terms used in the messages.
$   SCSI:  is an acronym for "Small Computer Systems Interface."  SCSI is
$      a technical term.  Do not translate unless an equivalent term is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character. The location is not translatable. It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES


$quote "
$set DHFD_SET1
$

$quote "
WE_R_TESTING "TESTING DIRECT BUS ATTACHED DISK DRIVE %1$s\n\
IN LOCATION %2$s\n\n\
This test will take approximately 6 minutes to complete.\n\n\
Please stand by."
$
$ This panel is displayed when the test starts.
$ MB stands for megabyte and is 1024 x 1024, or 1,048,576 bytes.
$ %1$s is the name of the direct attached device.
$ %2$s is the location of the direct attached device.
$ Leave line spacing as shown.  See general notes on length of title.

WE_R_ADVANCED "TESTING DIRECT BUS ATTACHED DISK DRIVE %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE\n\n\
This test will take approximately 6 minutes to complete.\n\n\
Please stand by."
$
$ This panel is displayed when the test starts in advanced mode.
$ MB stands for megabyte and is 1024 x 1024, or 1,048,576 bytes.
$ %1$s is the name of the direct attached device.
$ %2$s is the location of the direct attached device.
$ Leave line spacing as shown.  See general notes on length of title.

LOOPMODE_STATUS "TESTING DIRECT BUS ATTACHED DISK DRIVE %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
Each pass will take approximately 6 minutes to complete.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This panel is displayed in advanced mode and the user has chosen to
$ test the device in loop mode. It displays test status.
$ MB stands for megabyte and is 1024 x 1024, or 1,048,576 bytes.
$ %1$s is the name of the direct attached device.
$ %2$s is the location of the direct attached device.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title.

ELA_MENUGOAL "\
Error log analysis indicates a potential problem with the\n\
Direct Attached Disk Drive %1$s. In order to preserve the data\n\
on the disk drive, it is recommended that a back-up of the\n\
Direct Attached Disk Drive %2$s be done at this time.\n\n\
To continue, press Enter.\n"
$
$ This panel is displayed as a menu goal, if error log analysis of the
$ data found in the error log indicates that the disk drive could
$ have a potentially fatal error, it tells the user to back up the drive.
$ %1$s is the name of the direct attached device.
$ %2$s is the same as %1$s.
$ Leave line spacing as shown.

CERTIFY_PROMPT "TESTING DIRECT BUS ATTACHED DISK DRIVE %1$s\n\
IN LOCATION %2$s\n\n\
No trouble was found. Media verification can be done by running\n\
the certify procedure. This will allow you to determine\n\
the number of non-recoverable and soft data errors.\n\
The certify procedure may take up 3 minutes to complete.\n\
Do you want to certify the disk?\n"
$
$ This panel is displayed when all the tests are run.
$ %1$s is the name of the direct attached device.
$ %2$s is the location of the direct attached device.
$ Leave line spacing as shown.  See general notes on length of title.

DM_YES "YES"
$
$ This option is shown when a yes answer is possible.

DM_NO "NO"
$
$ This option is shown when a no answer is possible.

CHOOSE_AN_OPTION "Move cursor to selection, then press Enter.\n"
$
$ This message is shown when a multiple selection list is presented
 
CERTIFYING_DISK "TESTING DIRECT BUS ATTACHED DISK DRIVE %1$s\n\
IN LOCATION %2$s\n\n\
The Certify Operation is in progress.\n\n\
Please stand by.\n"
$
$ This message is shown when user choose to certify the disk.
$ %1$s is the name of the direct attached device.
$ %2$s is the location of the direct attached device.
$ Leave line spacing as shown.  See general notes on length of title.

CERTIFY_ABORT "TESTING DIRECT BUS ATTACHED DISK DRIVE %1$s\n\
IN LOCATION %2$s\n\n\
The Certify process has failed.\n\
Replace or reformat the disk drive.\n\n\
To continue, press Enter."
$
$ This message is shown when the certify aborted.
$ %1$s is the name of the direct attached device.
$ %2$s is the location of the direct attached device.
$ Leave line spacing as shown.  See general notes on length of title.

CERTIFY_COMPLETED "TESTING DIRECT BUS ATTACHED DISK DRIVE %1$s\n\
IN LOCATION %2$s\n\n\
The Certify operation has completed.\n\n\
To continue, press Enter."
$
$ This message is shown when the certify has completed.
$ %1$s is the name of the direct attached device.
$ %2$s is the location of the direct attached device.
$ Leave line spacing as shown.  See general notes on length of title.

