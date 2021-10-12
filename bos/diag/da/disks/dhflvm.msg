$ @(#)53	1.5  src/bos/diag/da/disks/dhflvm.msg, dadisks, bos411, 9428A410j 3/13/91 10:15:13
$ $ *
$ $ * FUNCTIONS:   This file contains the English language messages for the
$ $ *              fixed disk mount/unmount.
$ $ *
$ $ * ORIGINS: 27
$ $ *
$ $ * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ $ * combined with the aggregated modules for this product)
$ $ *                  SOURCE MATERIALS
$ $ * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
$ $ * All Rights Reserved
$ $ *
$ $ * US Government Users Restricted Rights - Use, duplication or
$ $ * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ $ *
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
$ END OF GENERAL NOTES


$quote "
$set SET_LVSBUSY

MSG_LVSBUSY_T "TESTING %1$s IN LOCATION %2$s\n\n\
The disk drive you have chosen to test is currently being used and\n\
cannot be tested.  It can be made available by unmounting the\n\
designated file systems.\n\n\
NOTE:  After being tested, the disk drive will be restored to\n\
       its current state."
$
$ Leave line spacing as shown.  Additional lines may be added if necessary.

MSG_LVSBUSY_2 "Do not unmount any file systems."

MSG_LVSBUSY_3 "Unmount the following file systems:\n%1$s"
$
$ %1$s is the list of names of file systems that can be unmounted.

MSG_LVSBUSY_E "Select one of the following."

$set	SET_UNMOUNT
MSG_UNMNT_T	"TESTING %1$s IN LOCATION %2$s"
$
$ See general notes on length of title line.

MSG_UNMNT_2	"Cannot unmount %1$s from %2$s.\n\n\
%3$s is still in use and must be made available\n\
before this option can be run."
$
$ %1$s is the name of the file system.
$ %2$s is the name of the file system mounted on by %1$s.
$ %3$s is the same as %2$s.
$ Leave line spacing as shown.

MSG_UNMNT_E	"To continue, press Enter."

$set 	SET_MOUNT
MSG_MNT_T	"TESTING %1$s IN LOCATION %2$s"
$
$ See general notes on length of title line.

MSG_MNT_2	"Cannot restore mount %1$s on %2$s."
$
$ %1$s is the name of the file system.
$ %2$s is the name of the file system mounted on by %1$s.

MSG_MNT_E	"To continue, press Enter."

$set	SET_CPVERR
MSG_CPVERR_T	"TESTING %1$s IN LOCATION %2$s"
$
$ See general notes on length of title line.

MSG_CPVERR_2	"Please make a note of the information below.\n\n"

MSG_CPVERR_3	""
$
$ This is a blank message.

MSG_CPVERR_E	"To continue, press Enter."

$set	SET_STATUS1
MSG_STATUS1_A	"Checking for mounted file systems.\n\n\
Please stand by."
$
$ Leave line spacing as shown.

$set	SET_STATUS7
MSG_STATUS7_A "The file systems for the physical volume %1$s are\n\
being mounted again.\n\n\
Please stand by."
$
$ %1$s is the name of the file system.
$ Leave line spacing as shown.
