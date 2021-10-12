$ @(#)92	1.10.2.3  src/bos/sbin/helpers/v3fshelpers/op_namei.msg, cmdfs, bos411, 9428A410j 11/15/93 09:18:20
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the filesystem
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set MS_OP_NAMEI

$ NOTE:	The message numbers 0506-101 through 0506-105 are in ff.msg. Also,
$ NOTE:	the message numbers 0506-106 through 0506-120 and 0506-621 through
$ NOTE:  0506-640 are reserved for this program.


ADDDIRB		"adddirb: nDb = %d\n"
$ Do not translate

ALLOCNEWBUF	"storestring: allocate new buffer.\n"
$ Do not translate the word "storestring"

BADINONUM	"0506-106 %d is not a valid inode number.\n"

BADSUPER	"0506-107 The superblock is not valid.\n"

BADVALUE	"0506-108 %s is not a valid value.  Values for option %c must not be zero.\n"

BADVERS		"0506-631 JFS filesystem, invalid version number.\n"

CANTGI		"0506-626 Cannot get inode for inode map.\n"

CANTREAD	"0506-629 Cannot read superblock.\n"

CORRUPTSUPER	"0506-630 Corrupt superblock.\n"


DIRBLKEXCEED	"the directory data block list"
$ This is appended to LIMITEXCEEDED.  Do not add a newline.

HAVEWANT	"%u bytes of memory are available.  Program needs are estimated to be %d.\n"

HOOK		"?"
$ This is used to display an unknown value.

INODEISZERO	"0506-627 The inode is zero.\n"

INODESTATS	"current inode = %d, name = `%s', parent = `%d'\n"

INUMBLK		"0506-628 Cannot get block for inode %d.\n"

SEEKFAIL	"0506-109 The seek call failed for block %d. errno is %d.\n"
$ Do not translate errno

READFAIL	"0506-110 The read call failed for block %d. errno is %d.\n"
$ Do not translate errno

LESSMEMMAYFAIL	"0506-111 The program is approaching memory limits.\n"

LIMITEXCEEDED	"0506-112 The program limit of %d has been exceeded for %s.\n"

LINKEXCEED	"the linked file list"
$ This is appended to LIMITEXCEEDED.  Do not add a newline.

MALLOCFAILS	"0506-113 The malloc(%d) call failed.\n"

MEMSTAT1	"     Dd:   %5d (%5d bytes)   Link: %5d (%5d bytes) \n"

MEMSTAT2	"     Ireq: %5d (%5d bytes)   Db:   %5d (%5d bytes) \n"

NAMEISDOTS	"xdname: `%s' is dots.\n"

NODBMEM		"0506-114 There is not enough memory available for the Db area.\n"

NODDMEM		"0506-115 There is not enough memory available for the Dd area.\n"

NOINPUT		"0506-116 Cannot get information about %s.\n"

NOIREQMEM	"0506-117 There is not enough memory available for the Ireq area.\n"

NOLINKMEM	"0506-118 There is not enough memory available for the Link area.\n"

NOMEMGIVEUP	"0506-119 There is not enough memory available. Giving up.\n"

NOMEMORY	"0506-120 There is not enough memory available.\n"

NONESELECTED	"No files were selected.\n"

NOTSPECIAL	"0506-621 Input device %s is not a special file.\n"

NULLTOSTORSTR	"0506-622 A NULL pointer was passed to storestring.\n"
$ Do not translate NULL or storestring.

NUMLINKS	"%d link names were detected\n"

NUMSELECTED	"%d files were selected\n"

OPENFAILED	"0506-623 Cannot find or open %s.\n"

PARTRESOLVE	"NOTE! %d pathnames were only partially resolved.\n"

PATHEXCEED	"path components"
$ This is appended to LIMITEXCEEDED.  Do not add a newline.

READDIRS	"readdirs(): blk = %d, j = %d, cnt = %d, tot = %d\n"
$ Do not translate

READINFO	"i: %3d  sz: %5d  ln: %2d\n"

READSUPER	"reading superblock\n"

SAVEINOD	"saveinod: nireq = %d\n"

SELECTEXCEED	"the selected file list"
$ This is appended to LIMITEXCEEDED.  Do not add a newline.

SHORTBLK	"short block, j=0\n"

STATFAIL	"0506-625 Cannot get information about %s.\n"

STATS		"nLink=%d  nIreq=%d  nDb=%d\n"
$ Do not translate

SUPERSTATS	"lstblk:%d  bsize:%d inopb:%d\n"
$ Do not translate

UNKNOWNERR	"??errno"
$ String representing an unknown error

UNKNOWNSUPER	"0506-632 Unknown filesystem type.\n"

UNRESOLVED	"??unresolved??"
$ String representing a portion of a path that could not be unresolved

XDNAME		"calling xdname: inum = %d,  name = `%s', len = %d\n"

XDNAMEADDING	"xdname: adding: inode %d, name = %s, parent %d\n"
