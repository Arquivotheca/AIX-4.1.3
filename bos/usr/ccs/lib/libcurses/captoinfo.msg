$ @(#)37	1.2.1.2  src/bos/usr/ccs/lib/libcurses/captoinfo.msg, terminfo, bos411, 9428A410j 11/18/93 10:52:53
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
$set MS_CAPTOINFO        Define initial set#

$ This catalog uses messages in the range 1375-000 -- 1375-099.

$ Note to translators: do not translate "captoinfo", "termcap", "terminfo",
$ or "TERM".

$ The following messages with the prefix of "M_MSG_" are for the old version
$ of captoinfo found in /usr/aix3/captoinfo.  Once that command is removed,
$ these messages can be deleted.

M_MSG_1 "termcap string '%c%c' commented out.\n"
M_MSG_2 "%s: TERM=%s: commented out code '%.2s' is unknown.\n"
M_MSG_3 "looking at termcap string '%.2s'.\n"
M_MSG_4 "%s: TERM=%s: unknown type given for the termcap code '%.2s'.\n"
M_MSG_5 "type of '%.2s' is %s.\n"
M_MSG_6 "%s: TERM=%s: wrong type given for the boolean termcap code '%.2s'.\n"
M_MSG_7 "%s: TERM=%s: wrong type given for the numeric termcap code '%.2s'.\n"
M_MSG_8 "%s: TERM=%s: wrong type given for the string termcap code '%.2s'.\n"
M_MSG_9 "%s: TERM=%s: the %s termcap code '%.2s' is not a valid name.\n"
M_MSG_10 "%s: TERM=%s: tgetent failed with return code %d (%s).\n"
M_MSG_11 "non-existent or invalid entry"
M_MSG_12 "cannot open $TERMCAP"
M_MSG_13 "unknown reason"
M_MSG_24 "%s: TERM=%s: cap %s (info %s) is NULL: REMOVED\n"
M_MSG_33 "%s: TERM=%s: cap %s (no terminfo name) is NULL: REMOVED\n"
M_MSG_34 "%s: obsolete 2 character name '%2.2s' removed.\n"
M_MSG_35 "\tsynonyms are: '%s'\n"
M_MSG_36 "looking for termcap value of %s.\n"
M_MSG_37 "\tvalue is:"
M_MSG_39 "termcap name '%s' not found in ostrcodes.\n"
M_MSG_40 "\tvalue is:"
M_MSG_42 "%s: TERM=%s: termcap name '%s' not found.\n"
M_MSG_43 "search looking for %s.\n"
M_MSG_45 "returning %d.\n"
M_MSG_46 "looking for terminfo value of %s.\n"
M_MSG_47 "\tvalue is:"
M_MSG_49 "terminfo name '%s' not found.\n"
M_MSG_50 "changing value for %s to %d.\n"
M_MSG_51 "value was: %d.\n"
M_MSG_52 "%s: TERM=%s: the boolean name '%s' was not found!\n"
M_MSG_53 "changing value for %s to %d.\n"
M_MSG_54 "value was: %d.\n"
M_MSG_55 "%s: TERM=%s: the numeric name '%s' was not found!\n"
M_MSG_56 "changing value for %s to "
M_MSG_58 "value was:"
M_MSG_60 "%s: TERM=%s: the string name '%s' was not found!\n"
M_MSG_63 "working on termcap 'ko' string.\n"
M_MSG_64 "key termcap name is '"
M_MSG_66 "looking at termcap name %s.\n"
M_MSG_67 "%s: TERM=%s: a function key for '%s' was specified with the value "
M_MSG_68 ", but it already has the value '"
M_MSG_70 "%s: TERM=%s: the unknown termcap name '%s' was\n"
M_MSG_71 "specified in the 'ko' termcap capability.\n"
M_MSG_73 "working on termcap ma string.\n"
M_MSG_74 "key value is '"
M_MSG_76 "the vi key is '"
M_MSG_78 "looking at vi character '"
M_MSG_80 "%s: TERM=%s: the vi character '"
M_MSG_81 "' (info '%s') has the value '"
M_MSG_82 "', but 'ma' gives '"
M_MSG_84 "%s: the unknown vi key '"
M_MSG_85 "' was\n"
M_MSG_86 "specified in the 'ma' termcap capability.\n"
M_MSG_87 "assigning defaults.\n"
M_MSG_88 "%s: TERM=%s: Warning: termcap sg and ug had different values (%d<->%d).\n"
M_MSG_89 "%s needs copying, was:"
M_MSG_91 "%s: TERM=%s: Warning: the string produced for '%s' may be inefficient.\n"
M_MSG_92 "It should be looked at by hand.\n"
M_MSG_93 "and has become:"
M_MSG_96 "%s: Null term_name given.\n"
M_MSG_97 "changing cap to info, TERM=%s.\n"
M_MSG_98 "use= found, %s uses %s.\n"
M_MSG_99 "reading from /etc/termcap\n"
M_MSG_100 "current directory name=%s.\n"
M_MSG_103 "setting the environment for %s.\n"
M_MSG_104 "setting the environment for TERM=%s.\n"
M_MSG_106 "extracting name from '%s'.\n"
M_MSG_109 "returning %s.\n"
M_MSG_110 "returning NULL.\n"
M_MSG_111 "reading from %s.\n"
M_MSG_112 "%s: cannot open %s for reading.\n"
M_MSG_114 "usage: %s [-1Vv] [-w width] [filename ...]\n"
M_MSG_115 "\t-1\tsingle column output\n"
M_MSG_116 "\t-v\tverbose debugging output\n"
M_MSG_117 "\t-V\tprint program version\n"
M_MSG_119 "tgetent(): TERM=%s: Termcap entry is too long.\n"
M_MSG_120 "tgetent(): TERM=%s: Termcap entry too long\n"
M_MSG_121 "tnchktc(): TERM=%s: Bad termcap entry\n"
M_MSG_122 "tnchktc(): TERM=%s: Infinite tc= loop\n"
M_MSG_123 "tnchktc(): TERM=%s: Termcap entry too long\n"
M_MSG_131 "Terminal type %s\n"
M_MSG_133 "flags\n"
M_MSG_155 "\nnumbers\n"
M_MSG_204 "%s: WARNING: termcap entry is too long!\n"
M_MSG_205 "end of strings\n"

$ The following messages are for /usr/bin/captoinfo.

ASGNDEF "Assigning defaults.\n"

BADCAP "1375-000 Bad termcap entry.\n"

BADTYPE "1375-001 The termcap code '%2$.2s' should be of type '%1$s'.\n"
$ %2$.2s is a termcap capability.
$ %1$s is a termcap capability type as given below. (messages labeled TYPE_xxx).
$ Note that '2$' precedes '1$' in this message.

BECOME "The string has become: %s.\n"
$ %s is a capability value.

CANTOPEN "captoinfo: 1375-002 Cannot open %s for reading.\n"
$ %s is a filename.

CHNGCAP "Changing termcap entry to terminfo entry, TERM=%s.\n"
$ %s is a terminal name.  Do not translate 'TERM'.

CHVAL "Changing value for '%1$s' to %2$d.\n"
$ %1$s is a terminfo capability.
$ %2$d is a numeric value.

CHVAL2 "Changing value for '%1$s' to %2$s.\n"
$ %1$s is a terminfo capability.
$ %2$s is a string value.

COMMENT "The termcap string '%c%c' was commented out.\n"
$ %c%c is a termcap capability.

COMMENT2 "1375-003 The commented out code '%2$.2s' is unknown.\n"
$ %1$s is a terminal name.
$ %2$.2s is a termcap capability.

CURDIR "The current directory is %s.\n"
$ %s is a directory name.

EXTRNAME "Extracting name from '%s'.\n"
$ %s is the current line being read.

FUNCKEY "1375-004 A function key for '%1$s' was specified with the value\n\
'%2$s', but it already has the value '%3$s'.\n"
$ %1$s is a termcap capability.
$ %2$s and %3$s are termcap capability strings.

INEFF "1375-005 Warning: The string produced for '%s' may be inefficient.\n\
It should be checked manually.\n"
$ %s is a capability name.

INFLOOP "1375-006 Infinite 'tc=' loop.\n"
$ Do not translate 'tc='.

INVALID "1375-007 The termcap code '%.2s' is not a valid name.\n"
$ %.2s is a termcap capability.

INVALID_ARG1 "Invalid argument '%s'.\n"
$ %s is an argument given as width, by the user.

INVALID_ARG2 "Number out of range '%s'.\n"
$ %s is an argument given as width, by the user.

KEYNAME "Key termcap name is '%s'.\n"
$ %s is a termcap capability.

KEYVAL "The key value is '%s'.\n"
$ %s is a termcap capability.

KEYVI "The vi key is '%s'.\n"
$ %s is a termcap capability. Do not translate 'vi'.

KOSTR "Working on termcap 'ko' string.\n"
$ Do not translate 'ko'.

LOOKAT "Looking at termcap string '%.2s'.\n"
$ %.2s is a termcap capability.

LOOKAT2 "Looking at termcap name '%.2s'.\n"
$ %.2s is a termcap capability.

LOOKFOR "Looking for the value of termcap code '%s'.\n"
$ %s is a termcap capability.

LOOKFOR2 "Looking for the value of terminfo code '%s'.\n"
$ %s is a terminfo capability.

LOOKVI "Looking at vi character '%s'.\n"
$ %s is a character such as 'a' or '^G'.
$ Do not translate 'vi'.

MASTR "Working on termcap 'ma' string.\n"
$ Do not translate 'ma'.

NEEDCOPY "%1$s needs copying, was: %2$s.\n"
$ %1$s is a capability name.
$ %2$s is a capability value.

NOTFND "termcap name '%s' was not found in ostrcodes.\n"
$ %s is a termcap capability.  Do not translate 'ostrcodes'.

NOTFND2 "termcap name '%s' was not found.\n"
$ %s is a termcap capability.

NOTFND3 "1375-008 terminfo name '%s' was not found.\n"
$ %s is a terminfo capability.

NULLREM "1375-009 The termcap code '%1$s' corresponding to\n\
terminfo code '%2$s' is NULL and will be removed.\n"
$ %1$s is a termcap capability.
$ %2$s is a terminfo capability.

NULLREM2 "1375-010 The termcap code '%s', which has no corresponding\n\
terminfo code, is NULL and will be removed.\n"
$ %s is a termcap capability.

NULLTERM "captoinfo: Null term_name given.\n"

OBSOL "captoinfo: Obsolete 2 character name '%2.2s' will be removed.\n"
$ %2.2s is a capability name.
OBSOL2 "\tSynonyms are: '%s'.\n"
$ %s is a list of capability synonyms.  This messages follows OBSOL.

READFROM "Reading from %s.\n"
$ %s is a filename.

RETURN "Returning %d.\n"
$ %d is a return value.

RETURN2 "Returning %s.\n"
$ %s is a return value.

SEARCH "Searching for %s.\n"
$ %s is a terminfo capability.

SETENVI "Setting the environment for %s.\n"
$ %s is a string of the form 'TERMCAP=xxxxx'.

SETENVI2 "Setting the environment for TERM=%s.\n"
$ %s is a terminal name.

TGFAIL "1375-011 tgetent failed with a return code of %1$d\n\
The reason given was \"%2$s\".\n"
$ %1$d is a return value.
$ %2$s is a reason for failure. (messages labeled TG_xxx).

TG_INV "Non-existent or invalid entry"
TG_OPN "Cannot open $TERMCAP"
TG_UNK "Unknown reason"
$ These messages are used in conjuction with TGFAIL.

TOOLONG "1375-012 Termcap entry is too long.\n"

TYPEOF "The type of termcap string '%1$.2s' is '%2$s'.\n"
$ %1$.2s is a termcap capability.
$ %2$s is a termcap capability type as given below. (messages labeled TYPE_xxx).

TYPE_BOO "boolean"
TYPE_NUM "numeric"
TYPE_STR "string"
TYPE_CAN "canceled"
TYPE_UNK "(unknown type)"
$ These messages are used in conjuction with BADTYPE and TYPEOF.

UNKCAP "The unknown termcap name '%s' was\n\
specified in the 'ko' termcap capability.\n"
$ %s is a capability name.  Do not translate 'ko'.

UNKTYPE "1375-013 Unknown type given for the termcap code '%.2s'.\n"
$ %.2s is a termcap capability.

UNKVI "captoinfo: 1375-014 The unknown vi key '%s' was\n\
specified in the 'ma' termcap capability.\n"
$ Do not translate 'vi' or 'ma'.
$ %s is a character such as 'a' or '^G'.

USAGE1 "Usage: captoinfo [-1Vv] [-w width] [filename ...]\n"
USAGE2 "\t-1\tSingle column output.\n"
USAGE3 "\t-v\tVerbose debugging output.\n"
USAGE4 "\t-V\tPrint program version.\n"

USEFOUND "The 'use=' specifier was found.  %1$s uses %2$s.\n"
$ %1$s and %2$s are terminal names.  Do not translate 'use='.

VALUE "\tThe value is: %s.\n"
$ %s is the value of a termcap or terminfo capability.

VALWAS "The value was: %d.\n"
$ %d is the value of a terminfo capability.

VALWAS2 "The value was: %d.\n"
$ %d is the value of a terminfo capability.

VERSION "captoinfo: captoinfo.c is at version %s\n"

VICHAR "1375-015 The vi character '%1$s' (terminfo '%2$s') has the\n\
value '%3$s', but 'ma' gives '%4$s'.\n"
$ Do not translate 'vi'.
$ %1$s is a character such as 'a' or '^G'.
$ %2$s is a terminfo capability.
$ %3$s and %4$ are terminfo capability strings.

WARNSGUG "1375-016 Warning: termcap 'sg' and 'ug' had different values (%d<->%d).\n"
$ Do not translate 'sg' or 'ug'.

