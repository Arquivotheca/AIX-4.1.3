$ @(#)38	1.2.1.3  src/bos/usr/ccs/lib/libcurses/tic.msg, terminfo, bos411, 9428A410j 9/3/93 14:48:36
$
$  COMPONENT_NAME: LIBCURSES
$
$  FUNCTIONS:  Message Catalog
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.
$len 
$set MS_TIC        Define initial set#

$ This catalog uses messages in the range 1375-100 -- 1375-199.

$ Note to translators: do not translate "tic" or "terminfo".

$ The following messages with the prefix of "M_MSG_" are for the old version
$ of tic found in /usr/aix3/tic.  Once that command is removed, these messages
$ can be deleted.

M_MSG_1 "Usage: tic [-vnumber] [files...]\n"
M_MSG_2 "Terminfo entry too long:\n"
M_MSG_4 "Terminfo entry too long:\n"
M_MSG_6 "Infinite use= loop '%s'\n"
M_MSG_7 "Cannot find term %s\n"
M_MSG_9 "Terminfo entry empty:\n"
M_MSG_10 "create '%s'\n"
M_MSG_11 "bool cap %s code %s val %d\n"
M_MSG_12 "num cap %s code %s val %d\n"
M_MSG_13 "str %s code %s val %s\n"
M_MSG_14 "str %s code %s val NULL\n"
M_MSG_15 "link '%s' '%s'\n"
M_MSG_16 "mkdir %s"
M_MSG_18 "Command '%s' failed\n"
M_MSG_19 "%s: Terminal name too short\n"
M_MSG_20 "unlink %s\n"
M_MSG_21 "Out of memory, try again.\n"

$ The following messages are for the new version of tic in /usr/bin/tic.

ACCESS "tic: 1375-100 %s is non-existant or permission denied.\n"
$ %s is a filename.

BACKSPC "1375-101 Backspaced off beginning of line.\n"

BADFIRST "1375-102 '%s': Bad first terminal name.\n"
$ %s is a terminal name.

BADTERM "1375-103 '%s': Bad terminal name found in list.\n"
$ %s is a terminal name.

BADTERM2 "1375-104 %s: Bad terminal name.\n"
$ %s is a terminal name.

BADTYPE "1375-105 Wrong type used for capability '%s'\n"
$ %s is a capability name.

CANTLINK "1375-106 Cannot link %1$s to %2$s.\n"
$ %1$s and %2$s are filenames.

CANTOPEN "tic: 1375-107 Cannot open %s.\n"
$ %s is a filename.

CANTOPN2 "1375-108 Cannot open %s/%s\n"
$ %s/%s is a filename.

CORRUPT "*** Possibly corrupted terminfo file. ***\n"

DIRSTAT "1375-109 mkdir %s returned bad status.\n"
$ %s is a directory name.  Do not translate 'mkdir'.

DIRPERM "tic: 1375-110 %s/%s: Permission denied.\n"
$ %s/%s is a directory name.

ERRABORT "tic: Line %1$d in terminal '%2$s':\n"
$ %1$d is a line number and %2$s is a terminal name.

FILES "tic: 1375-111 Too many file names.\n"

FILETERM "1375-112 File does not start with terminal names in column one.\n"

ILLCHAR "1375-113 Illegal character - '%c'.\n"

ILLCTRL "1375-114 Illegal control character - '^%c'.\n"

ILLESC "1375-115 Illegal character in escape sequence - '\\%c'.\n"

ILLTERM "tic: 1375-116 Line %1$d: Illegal terminal name - '%2$s'.\n"
ILLTERM2 "Terminal names must start with a letter or digit.\n"
$ %1$d is a line number and %2$s is a terminal name.
$ Both of these messages are printed together in the order shown.

LINKERR1 "tic: 1375-117 There is an error in following use-links."
LINKERR2 "\tThere is a loop in the links or they reference non-existant terminals.\n"
LINKERR3 "\tThe following is a list of the entries involved:\n\n"
$ Do not translate 'use-links'.
$ All three of these messages are printed together in the order shown.

LONGSTR "1375-118 Very long string found.  Missing comma?\n"

MULTDEF "1375-119 '%s' defined in more than one entry.\n"
MULTDEF2 "Entry being used is '%s'.\n"
$ Both of these messages are printed together in the order shown.
$ %s is a terminal name in both messages.

NEWLINE "1375-120 A newline was found in the middle of a terminal name.\n"

NOCOMMA "1375-121 Missing comma.\n"

NOCOMMA2 "1375-122 Missing comma?\n"

NOMEM "1375-123 Not enough memory for use_list element.\n"
$ Do not translate 'use_list'.

NONUM "1375-124 Missing numeric value.\n"

NOTDIR "tic: 1375-125 %s is not a directory.\n"
$ %s is a directory name.

NOTDIR2 "tic: 1375-126 %s/%s: Not a directory.\n"
$ %s/%s is a directory name.

NULLSTR "1375-127 NULL string value.\n"

ONESYN "1375-128 At least one synonym should begin with a letter.\n"

OOMEM "1375-129 Out of memory.\n"

PREMEOF "1375-130 Premature EOF.\n"

PREMEOF2 "1375-131 Premature EOF.  Missing comma?\n"

REREAD "1375-132 Error in re-reading compiled file %s.\n"
$ %s is a filename.

SYSABORT "tic: PROGRAM ERROR: Line %1$d in terminal '%2$s':\n"
$ %1$d is a line number and %2$s is a terminal name.

TERMSYN "1375-133 Terminal name '%s' synonym for itself.\n"
$ %s is a terminal name.

TOKSEEK "1375-134 Terminal names not found after a seek.\n"
$ Do not translate 'seek'.

TOOLONG "1375-135 '%s': Terminal name too long.\n"
$ %s is a terminal name.

TOOSHORT "1375-136 '%s': Terminal name too short.\n"
$ %s is a terminal name.

UNKCAP "1375-137 Capability is not recognized: '%s'.\n"
$ %s is a capability name.

UNKOPT "tic: 1375-138 Option is not valid.\n"

UNKTOK "1375-139 Token type is not recognized.\n"

USAGE "Usage: tic [-v[n]] [-c] source-file\n"

WARNING "tic: Warning near line %1$d in terminal '%2$s':\n"
$ %1$d is a line number and %2$s is a terminal name.

WRITERR "1375-140 Error in writing %s/%s.\n"
$ %s/%s is a filename.

