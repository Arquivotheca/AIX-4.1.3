$ @(#)83	1.9  src/bos/usr/bin/pax/pax.msg, cmdarch, bos411, 9428A410j 12/1/93 12:15:56
$
$  COMPONENT_NAME: (CMDARCH) archive files
$
$  FUNCTIONS: pax
$ 
$  ORIGINS: 18, 27
$ 
$  This module contains IBM CONFIDENTIAL code. -- (IBM
$  Confidential Restricted when combined with the aggregated
$  modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1991, 1993
$  All Rights Reserved
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 Release 1.0
$
$ Message numbers 600-699 are reserved for this file.

$
$ RCSfile Revision (OSF) Date
$
$ Messages for the pax archiving program
$

$quote "
$set MS_PAX

$
$ Generally when a message has a '%s : ' in the front of it, the
$ name of the program (pax/tar/cpio) will be inserted there.
$

NOMEM		"0511-600 There is a memory allocation problem."
NOTDIR		"0511-601 The destination directory is not a directory."
BLOCKS		"%ld Blocks\n"
$
$ 	messages for the file append.c
$
APPEND_CORRUPT	"0511-603 The archive file appears to be corrupt."
APPEND_BACK "tape backspace error"
APPEND_BACK2	"0511-604 A backspace error occurred during the append operation."
$
$ 	messages for the file buffer.c
$
BUF_PRESERVE	"0511-605 An error occurred attempting to preserve the owner and group ids."
BUF_EOF		"0511-606 An unexpected end-of-file has occurred."
BUF_OVERFLOW	"0511-607 An overflow condition exists in the write buffer."
BUF_BLKSIZE	"0511-608 The blocksize value is not valid."
BUF_IOBUF	"0511-609 Cannot allocate memory for the input/output buffer."
BUF_BUFSKIP	"0511-610 An overflow condition has occurred within the write buffer."
BUF_BUF_READ	"0511-611 An overflow condition exists in the read buffer."
BUF_CORRUPT	"0511-612 The archive file appears to be corrupt."
BUF_WRITE	"0511-613 A write error has occurred."
BUF_INPUT	"0511-614 The archive is empty."
BUF_AVAIL	"0511-615 An overflow condition exists within the write buffer."
$
$ 	messages for the file charmap.c
$
$ NOTICE TO TRANSLATORS:
$ The CM_ messages should not be translated since the charmap stuff with
$ the -e option has been removed from Posix1003.2 Draft 12.  That means
$ these messages cannot be seen by any user using the pax command.  This
$ may change but as of 06-24-91 this option is out of pax.
$
CM_LONG		"0511-616 The charmap converted filename has become too long."
CM_NOTFOUND	"Could not find charmap entry"
CM_SYNTAX	"0511-617 Syntax error in charmap - skipping"
$ This message is followed by the CM_LINE message 
CM_DECL		"0511-618 Error in charmap declarations - skipping"
$ This message is followed by the CM_LINE message 
CM_RANGE	"0511-619 Error in charmap range - skipping"
$ This message is followed by the CM_LINE message 
CM_ENCODE	"0511-620 Error in charmap encoding - skipping"
$ This message is followed by the CM_LINE message 
CM_ADDRANGE	"0511-621 Error adding charmap range"
$ This message is followed by the CM_LINE message 
CM_ADDENTRY	"0511-622 Error adding charmap entry"
$ This message is followed by the CM_LINE message 
CM_LINE		"line %d"
$ This message follows the CM_SYNTAX,CM_DECL,CM_RANGE, and CM_ENCODE
$ messages and tells the line number that is being skipped in the 
$ charmap file.
CM_NOMEM	"0511-623 Unable to allocate enough memory to construct charmap."
$
$ 	messages for the file cpio.c
$
CPIO_USAGE1	"Usage: %s -o[Bacv]\n"
CPIO_USAGE2	"       %s -i[Bcdmrtuvf] [pattern...]\n"
CPIO_USAGE3	"       %s -p[adlmruv] directory\n"
$
$ 	messages for the file create.c
$
CR_LONG		"0511-624 The name is longer than 255 bytes."
CR_LINKLONG	"0511-673 The linkname is longer than 100 bytes."
$
$ 	messages for the file extract.c
$
EX_CORRUPT	"0511-625 The archive file appears to be corrupt."
EX_NOTAR	"0511-626 An invalid file header has been read."
$	This message is followed by EX_SKIP
EX_SKIP		"         Skipping to the next file..."
$	This message is preceded by EX_NOTAR
EX_UNREG	"0511-627 The archive format is not recognizable."
EX_BADMAG	"0511-628 The magic number is not recognizable."
EX_MAY		"0511-629 The file may be corrupt."
EX_RESYNC	"The extraction appears to have resynchronized."
EX_CONT		"Continuing"
EX_SYM		"0511-630 A symbolic link is corrupted."
EX_OWN		"0511-631 An error occurred attempting to restore the owner and group ids."
EX_HDR		"0511-632 The swapped header is corrupted."
EX_PLEN1	"0511-633 The swapped pathname length is bad."
EX_PATH		"0511-634 The swapped pathname is corrupted."
EX_BADPATH	"0511-635 The swapped pathname is bad."
EX_ASCII	"0511-636 A cpio ASCII header is corrupted."
EX_BADHDR	"0511-637 A cpio ASCII header is bad."
EX_PLEN2	"0511-638 An cpio ASCII pathname length is bad."
EX_APATH	"0511-639 An cpio ASCII pathname is corrupted."
EX_BADAPATH	"0511-640 An cpio ASCII pathname is bad."
EX_BHDR		"0511-642 A cpio binary header is corrupted."
EX_PLEN3	"0511-643 A cpio binary pathname length is bad."
EX_BPATH	"0511-644 A cpio binary pathname is corrupted."
EX_BADP		"0511-645 A cpio binary pathname is bad."
$
$ 	messages for the file fileio.c
$
FIO_EXISTS	"The file exists and will not be overwritten."
FIO_SAME	"The file is the same."
FIO_NEWER	"The file exists and is newer."
FIO_DIR		"Directories are not being created."
FIO_UKNOWN	"0511-646 The filetype is unknown."
$
$ 	messages for the file link.c
$
LK_UNSEEN	"0511-647 A link exists on the archive but was not processed."
$
$ 	messages for the file list.c
$
LS_SYM		"0511-648 Unable to read a symbolic link"
$	This message is followed by the strerror message string for the errno
LS_LINK		" is linked to %s"
$	%s is the name that the file is linked to
LS_SYMLINK	" is a symbolic link to %s"
$	%s is the name that the symbolic link points to 
LS_XSYM		"x %1$s is a symbolic link to %2$s\n"
$	%1$s is replaced by the extracted file name
$	%2$s is replaced by the filename that the extracted file name points to
LS_LINK2	"%1$s is linked to %2$s\n"
$	%1$s is replaced by the extracted file name
$	%2$s is replaced by the filename that the extracted file name is linked to
LS_SUM		"x %s, %ld bytes, %d tape blocks\n"
$	%s is replaced by the file size in bytes
$	%d is replaced by the number of blocks required to store the file 
LS_READ		"0511-649 Unable to read a symbolic link"
$	This message is followed by the strerror message string for the errno
LS_ASYM		"a %1$s is a symbolic link to %2$s\n"
$	%1$s is replaced by the extracted file name
$	%2$s is replaced by the filename that the extracted file name points to
LS_LINK3	"is a link to %s\n"
$	%s is replaced by the name of the link
LS_SUM2		"? %s %ld blocks\n"
$	%s is replaced by the filename
$	%d is replaced by the number of blocks required to store the file 
$
$ 	messages for the file namelist.c
$
NL_MEM		"0511-650 Unable to allocate enough memory for the filename list."
NL_DIR		"0511-651 An error occurred attempting to open the directory."
NL_FOUND	"%1$s: %2$s was not found in the archive.\n"
$	%1$s is replaced by the name of the command(ie, pax,cpio,or tar)
$	%2$s is replaced by the filename which was not found
NL_LONG		"0511-652 The name is too long."
$
$ 	messages for the file options.c
$
OPT_NOVAL	"0511-668 The keyword specified requires a value."
OPT_NOSUCH	"0511-669 The keyword specified is not valid."
OPT_SIZE	"0511-670 The size value specified is not valid."
$
$ 	messages for the file pathname.c
$
PN_TOOMANY	 "0511-653 The pathname contains too many elements."
$
$ 	messages for the file pax.c
$
PAX_BADAOPT	"0511-674 The -f archive option must be specified with the -a option.\n"
PAX_BSIZE	"0511-654 An invalid block size was specified with the -b option."
PAX_PRIV	"0511-655 An invalid file characteristic was specified with the -p option."
PAX_CHARMAP	"0511-656 An error occurred trying to read the charmap file."
PAX_FORMAT	"0511-657 The archive format specified is different from the existing archive."
PAX_TYPE	"0511-658 Unable to determine the archive format."
PAX_OVER	"0511-672 The value specified exceeds the maximum file size."
PAX_CPIO	"CPIO format archive\n"
PAX_TAR		"USTAR format archive\n"
PAX_BADVAL	"0511-659 The value specified is not recognizable."
PAX_U1 "Usage: %s [-cdnv] [-f archive] [-s replstr] [pattern...]\n"
PAX_U2 "       %s -r [-cdiknuvy] [-f archive] [-o options] [-p string] [-s replstr]\n              [pattern...]\n"
PAX_U3 "       %s -w [-dituvyX] [-b blocking] [[-a] -f archive] [-o options] [-s replstr]\n              [-x format] [pathname...]\n"
PAX_U4 "       %s -r -w [-diklntuvyX] [-p string] [-s replstr] [pathname...] directory\n"
PAX_NOTDIR	"0511-667 The destination directory is not a directory."
PAX_DSREQ	"0511-671 The 'datastr_size' option is required\n\
         when the 'datastream' option is specified."
$	Do not translate 'datastr_size' or 'datastream'.

$
$       messages for the file replace.c
$
RP_NOADD	"0511-660 A replacement string was not added"
$	This message is followed by a colon and then one of
$	the RP_SPACE,RP_BADRE, RP_TRLR, RP_BADDEL or regerror() messages.
RP_SPACE	"Unable to allocate enough memory."
$	This message is preceded by the RP_NOADD message.
RP_MOD		"Invalid RE modifier"
$	This message is preceded by the RP_NOADD message.
RP_TRLR		"0511-661 An invalid trailing option was specified with the -s option."
$	This message is preceded by the RP_NOADD message.
RP_BADDEL	"Bad delimeters"
$	This message is preceded by the RP_NOADD message.
RP_ADD		"add %s ? "
$	%s is replaced by the filename to add
RP_EXTRACT	"extract %s ? "
$	%s is replaced by the filename to extract
RP_PASS		"pass %s ? "
$	%s is replaced by the filename to copy
RP_RENAME	"rename %s? "
$	%s is replaced by the filename to rename
RP_BADREF	"An invalid backreference was specified."
RP_STRLONG  "0511-662 The new filename exceeded the path name limit.\n"
$	This message is followed by RP_NOSUB
RP_NOSUB    "         There was no substitution done on %s\n"
$	%s is replaced by the filename
$
$       messages for the file tar.c
$
TAR_MODE  "0511-663 %s: Exactly one of the c, t, r, u or x options must be specified.\n"
TAR_BSIZE	"Tar: blocksize = %d\n"
$	%d is the blocksize(an integer)
TAR_OPT		"0511-664 %s: unknown option %c\n"
$	%s is replaced by "tar"
$	%c is replaced by the invalid option
TAR_ARG		"0511-665 %s: %c argument missing\n"
$	%s is replaced by "tar"
$	%c is replaced by the invalid option
TAR_USAGE1	"Usage: %s -c[bfvw] device block filename..\n"
TAR_USAGE2	"       %s -r[bvw] device block [filename...]\n"
TAR_USAGE3	"       %s -t[vf] device\n"
TAR_USAGE4	"       %s -u[bvw] device block [filename...]\n"
TAR_USAGE5	"       %s -x[flmovw] device [filename...]\n"
$	%s is replaced by "tar"
$
$       messages for the file ttyio.c
$
TTY_UNVAIL	"0511-666 Unable to prompt user: /dev/tty is unavailable."
TTY_ABORT	"Aborted"
TTY_CONT	"Continuing"
$
$ Make sure the two strings quoted here are the same as TTY_GO and
$ TTY_QUIT, as the program will use those two messages to do a
$ strcmp on.
$
$ message should not be more than 200 bytes, 
$ answers can't be more than 20 bytes
$
TTY_PROMPT	"%s: Ready for volume %u\n%s: Type \"go\" when ready to proceed (or \"quit\" to abort): \07"
TTY_GO		"go"
TTY_QUIT	"quit"
$
$       messages for the file warn.c
$
WARN_OFF	"%s: [offset "
$	%s is replaced by the command name

