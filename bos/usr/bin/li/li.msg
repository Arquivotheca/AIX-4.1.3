$ @(#)70        1.9  src/bos/usr/bin/li/li.msg, cmdfiles, bos411, 9428A410j 8/3/93 16:42:44
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
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
$
$ $ Message numbers 300 - 339 reserved for this file.
$
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate li -- this is the command name.
$ Do not translate %s %c %ld etc.
$
$set MS_LI
$
CANTHAPPEN "li: 0653-300 Internal error. The error number is %d.\n"
$ This is a programming error that you should never see.
$ Call your IBM representatives if you see this message.
$
CANTOPEN "li: 0653-301 Cannot find or open %s.\n"
$ The li command cannot open an archive file named %s.
$
CONFLICT "li: 0653-302 Cannot specify the -d flag with the -R flag.\n"
$ USAGE is diplayed after this message.
$
GRANDTOTAL "total %ld blocks\n"
$
NOMEMORY "li: 0653-304 There is not enough memory available now.\n"
$
TOTALBLOCKS "grand total %ld blocks\n"
$
UNKNOWN1 "li: 0653-306 The -%c flag is not recognized.\n"
$ USAGE is diplayed after this message.
$
UNKNOWN2 "li: 0653-307 -%c<Number> is not a recognized flag.\n"
$ USAGE is diplayed after this message.
$
UNKNOWN3 "li: 0653-308 The -%c%c flag is not recognized.\n"
$ USAGE is diplayed after this message.
$
UNREADABLE "li: 0653-309 Cannot open the directory %s.\n"
$
INODE "Inode "
$ Translators -- This message and the rest of the messages are actully headers.
$ They are formatted in the output of li. Customers may be expecting them
$ to be exactly this way.  If you MUST translate,
$ do not change the width of these messages.
$
$ The file serial number
TPROT "t  prot     "
$ The protections
LINKS " ln "
$ The link count
NODE " node_name/id  "
OWNER "owner    "
RUID "r_uid "
$ The remote user identification number
GROUP "group    "
RGID "r_gid "
$ The remote group identification number
CHARCT " char-ct "
$ The character count
BLOCKS "  blocks "
MODIFIED "   modified   "
ACCESSED "   accessed   "
UPDATED "   updated    "
FILENAME "filename"

NOSORTSPACE "li: 0653-305 There is not enough memory available now for sorting.\n"
$
USAGE "Usage: li [-I[abcfghilmnoprsu]] [-E[abcfghilmnoprsu]]\n\
\t[-O[abcdfpx]] [-S[acmnrsux]] [-R[Number][anqp]]\n\
\t[-Number] [-adfelnsvxL] [File...] [Directory...] \n"
$
NOENTRY "li: 0653-310 File %s was not found.\n"
$
