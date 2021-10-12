$ @(#)51	1.5.3.1  src/bos/usr/bin/istat/istat.msg, cmdfs, bos411, 9434A411a 8/19/94 15:05:30  
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len 
$set MS_ISTAT        Define initial set#

$ $ The messages have been assigned a range of 0506-161  
$ Do not translate istat.
$ Do not translate %s, %c, %d, etc.

USAGE               "Usage: istat {File | Inode Device}\n"


INODEONDEV           "Inode %d on device %d/%d\t"

UNALLOC              "Unallocated Inode\n"

DIRECTORY            "Directory\n"

BLOCKDEV             "Block Device\n"

CHARDEV              "Character Device\n"

FILE                 "File\n"

FIFO                 "FIFO\n"
$ Do not translate FIFO.  

TYPEOFILE            "Type %o file\n"

PROTECTION           "Protection: %s\t"

SETUID               "Set UID "
$ Do not translate UID.  

GETGID               "Set GID "
$ Do not translate GID.  

STICKY               "Sticky"
$ Do not translate sticky.  

OWNER                "Owner: %d(%s)\t\t"

GROUP                "Group: %d(%s)\n"

LINKSNLEN            "Link count: %3d\t\tLength %ld bytes\n"

MAJORMINOR           "Major Device %d\t\tMinor Device %d\n"

CTIME                "Last updated:\t%s"

MTIME                "Last modified:\t%s"

ATIME                "Last accessed:\t%s"

BLOCKPTRS	"\nBlock pointers (hexadecimal):\n"

READBIT              "r"

WRITEBIT             "w"

EXECBIT              "x"

BLANKBIT             "-"

CANTSTAT             "istat:  0506-161 Cannot find information about %s"
$ Please leave a space at the end of this message.

INUMBLK		"istat:  0506-162 Cannot get block for inode %d.\n"
$ Do not translate inode.  

CANTRESB        "istat:  0506-163 Cannot read super block on %s.\n"
$ Do not translate superblock.  

NOTJFS         	"istat:  0506-164 %s is not recognized as a JFS file system.\n"
$ Do not translate JFS

NOTSUPJFS     "istat:  0506-165 %s is not a supported JFS file system version.\n"
$ Do not translate JFS

CANTOPEN	"istat:  0506-166 Cannot find or open %s"

CORRUPTJFS  "istat:  0506-167 The %s JFS file system superblock is corrupted.\n"
$ Do not translate JFS


DATE_FMT	     "%a %b %d %T %Y"
$ Format of time output need to be translated
