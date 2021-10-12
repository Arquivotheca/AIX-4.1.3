$ @(#)46	1.10.1.1  src/bos/usr/bin/dosdir/dosdir.msg, cmdpcdos, bos411, 9430C411a 7/21/94 16:31:11
$
$
$ COMPONENT_NAME: CMDPCDOS  routines to read dos floppies
$
$ FUNCTIONS:  message file for command portion of dos utilities
$
$ ORIGINS: 10,27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_DOSDIR

AIXWRITE "dosread: 0467-001 There is an error in writing to the file system."
$ The perror message follows this message.

ASSUMING "The default beginning for the File Allocation Table\n\
\tis sector %d.\n"
$ %d is a sector.

CANTFINDFILE "Cannot locate file %s.\n\
\tCheck path name.\n"

CANTOPEN "Cannot open %s.\n\
\tCheck path name and permissions.\n"

CLOSE_FAIL "doswrite: 0467-002 There is a DOS close failure on %s.\n\
\tUse the dosread command to verify that the data exists on the disk.\n"
$ %s is the file on the DOS disk.
$ Do not translate doswrite or dosread.

CLUSTERS "\n  Clusters: "
$ This command lists clusters attached to the file. The cluster numbers are
$ printed from the disk.

CLUSTSIZ "The cluster size is %d sectors.\n"
$ %d is the cluster size in sectors.
$ This message follows the DISKPART message.

DELETED "%s is deleted\n."
$ %s is a file.

DEL_USAGE "Usage: dosdel [-v] [-DDevice] DOSFile\n\
\tDeletes a file from a DOS disk.\n"
$ Do not translate dosdel, AIX, or DOS.

DIRSIZE "The maximum directory size is %d entries.\n"
$ %d is the number of entries.

DIR_USAGE "Usage: dosdir [-adltv] [-DDevice] [DOSFile]\n\
\tLists all DOS files on the disk.\n"
$ Do not translate dosdir, -adltv, -D, or DOS.

DISKDATA "Disk data: %1$d bytes per sector, %2$d sectors per track, %3$d heads\n"
$ %1$d is the number of bytes. %2$d is the number of sectors. %3$d is number
$ of heads.

DISKPART "Disk partition size: %1$d sectors, %2$d reserved, %3$d hidden\n"
$ %1$d is the number of sectors. %2$d is the number of reserved sectors.
$ %3$d is the number of hidden sectors.

DISKTYPE "Cannot identify the disk type.\n\
\tThe File Allocation Table number is %d.\n\
\tUse a DOS disk.\n"
$ Do not translate DOS.
$ %d is the File Allocation Table descriptor.

DOSCANTOPEN "doswrite: 0467-003 Cannot open file %1$s.\n\
\tThe system call error code number is %2$d."
$ %1$s is the file. %2$d is the error code number.

EIGHTSECTOR ", eight sector disk.\n"

FATSIZE "File Allocation Table size: %1$d sectors.\n\
\tFile Allocation Table count: %2$d.\n"
$ %1$d is the number of sectors.
$ %2$d is the FAT count number.

FREESPACE "There are %d bytes of free space.\n"
$ %d is the number of bytes.

INITFAIL "The DOS init library routine failed.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate DOS. Do not translate init.

INVFILESYS "The DOS file system is not valid or is not accessible.\n\
\tUse a DOS disk.\n"

NINESECTOR ", nine sector disk.\n"

NOTDELETED "dosdel: 0467-004 %s is not deleted"
$ %s is a file.

ONESIDED "The disk is a one-sided"

RD_USAGE "Usage: dosread [-av] [-DDevice] DOSFile [UNIXFile]\n\
\tReads a DOS file from a disk to a UNIX file system.\n"
$ Do not translate dosread, -av, -D, UNIX, or DOS.

SPECIFY "doswrite: 0467-005 Specify a PC-DOS file.\n"
$ Do not translate PC-DOS.

TWOSIDED "The disk is a two-sided"

WRT_FAIL "doswrite: 0467-006 There is a DOS write failure on %s.\n\
\tThe disk may be damaged.\n\
\tTry another disk or\n\
\tuse local problem reporting procedures.\n"
$ %s is the DOS file name the user is trying to write to.

WR_USAGE "Usage: doswrite [-av] [-DDevice] UNIXFile DOSFile\n\
\tMoves UNIX files.\n"
$ Do not translate doswrite, -av, -D, UNIX, or DOS.

MONTH "Jan"
$ This message shows the month the file was last updated. It is the abbreviation
$ for January. The translation can have only 3 characters.

MONTH1 "Feb"
$ This message shows the month the file was last updated. It is the abbreviation
$ for February. The translation can have only 3 characters.

MONTH2 "Mar"
$ This message shows the month the file was last updated. It is the abbreviation
$ for March. The translation can have only 3 characters.

MONTH3 "Apr"
$ This message shows the month the file was last updated. It is the abbreviation
$ for April. The translation can have only 3 characters.

MONTH4 "May"
$ This message shows the month the file was last updated. The translation can
$ have only 3 characters.

MONTH5 "Jun"
$ This message shows the month the file was last updated. It is the abbreviation
$ for June. The translation can have only 3 characters.

MONTH6 "Jul"
$ This message shows the month the file was last updated. It is the abbreviation
$ for July. The translation can have only 3 characters.

MONTH7 "Aug"
$ This message shows the month the file was last updated. It is the abbreviation
$ for August. The translation can have only 3 characters.

MONTH8 "Sep"
$ This message shows the month the file was last updated. It is the abbreviation
$ for September. The translation can have only 3 characters.

MONTH9 "Oct"
$ This message shows the month the file was last updated. It is the abbreviation
$ for October. The translation can have only 3 characters.

MONTH10 "Nov"
$ This message shows the month the file was last updated. It is the abbreviation
$ for November. The translation can have only 3 characters.

MONTH11 "Dec"
$ This message shows the month the file was last updated. It is the abbreviaton
$ for December. The translation can have only 3 characters.

CANTWRITEDIR "doswrite: 0467-007 Cannot make directory entry %1$s.\n\
\tThe system call error code number is %2$d.\n"
$ %1$s is the file. %2$d is the error code number.

ABORT		"Abort, Retry, Ignore?"
A_RESP		"Aa"
R_RESP		"Rr"
I_RESP		"Ii"
$ The letters listed in A_RESP, R_RESP, and I_RESP are the possible responses
$ to the three options listed in the ABORT message.  Thus, 'R' and 'r' are both
$ possible responses corresponding to the word "Retry" in the ABORT message.
$ These responses must be single-byte characters.  A possible solution for
$ multi-byte languages would be to use messages similar to:
$
$ ABORT		"1) <mb-string1>, 2) <mb-string2>, 3) <mb-string3>?"
$ A_RESP	"1"
$ R_RESP	"2"
$ I_RESP	"3"

FATALERR	"XXX FATAL ERROR: line %1$d, file %2$s: "
$ %1$d is a line number.   %2$s is a file name.
$ This message is followed by an error message from the perror() subroutine.

INSDISK		"Insert diskette for drive %s and strike\nany key when ready\n"
$ %s is the name of a device such as "/dev/fd0".

WRT_FAIL1	"Insufficient disk space.\n"
