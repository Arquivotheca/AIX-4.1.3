$ @(#)31	1.1  src/bos/usr/ccs/lib/libcurses/infocmp.msg, libcurses, bos411, 9428A410j 9/3/93 15:09:11
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
$  (C) COPYRIGHT International Business Machines Corp. 1993
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.
$len 
$set MS_INFOCMP        Define initial set#

$ This catalog uses messages in the range 1375-400 -- 1375-499.

$ Global comment to translators: Do not translate 'infocmp'.

ADDING "Adding in terminal type '%s'.\n"
$ %s is a terminal name.

BOOLCAP "There are %d boolean capabilities.\n"

BYHAND "Must change this entry manually."

CHGSEEN "%1$s: %2$d:%3$d, Changed=%4$d, Seen=%5$d.\n"

CHGSEEN2 "%1$s: '%2$s':'%3$s', Changed=%4$d, Seen=%5$d.\n"

COMP1 "Comparing %1$s (TERMINFO=%2$s) to %3$s (TERMINFO=%4$s).\n"
$ %1$s and %3$s are terminal names.  Do not translate '(TERMINFO=%2$s)'.

COMP2 "Comparing %1$s (TERMINFO=%2$s) to %3$s.\n"
$ %1$s and %3$s are terminal names.  Do not translate '(TERMINFO=%2$s)'.

COMP3 "Comparing %1$s to %2$s (TERMINFO=%3$s).\n"
$ %1$s and %2$s are terminal names.  Do not translate '(TERMINFO=%3$s)'.

COMP4 "Comparing %1$s to %2$s.\n"
$ %1$s and %2$s are terminal names.

COMPBOOL "    Comparing booleans.\n"
$ This message should have 4 spaces at the beginning.

COMPNUMS "    Comparing numbers.\n"
$ This message should have 4 spaces at the beginning.

COMPSTR "    Comparing strings.\n"
$ This message should have 4 spaces at the beginning.

LOOKAT "Looking at '%s'.\n"
$ %s is a capability name.

NOTADD "infocmp: 1375-400 'use=%s' did not add anything to the description.\n"
$ %s is a terminal name.

NUMCAP "There are %d numeric capabilities.\n"

NUMHAS1 "    %1$s: Terminal entry '%2$s' has a value of %3$d,\n\
    while terminal entry '%4$s' has a value of %5$d.\n"
NUMHAS2 "    %1$s: Terminal entry '%2$s' has a value of '%3$s',\n\
    while terminal entry '%4$s' has a value of '%5$s'.\n"
$ %1$s is a capability name.  %2$s and %4$s are terminal names.
$ In 'NUMHAS1', %3$d and %5$d are numerical capability values.
$ In 'NUMHAS2', %3$s and %5$s are strings capability values.
$ These messages should both have 4 spaces at the beginning of each line.
$ One of these messages follows the 'ORDERDEP' message.

ORDERDEP "infocmp: 1375-401 use= order dependency found:\n"
$ Do not translate 'use='.
$ This message is followed by the 'NUMHAS1' or the 'NUMHAS2' message.

PADMOVE "infocmp: 1375-402 Padding information was moved to the end.\n"

PADREM "infocmp: 1375-403 Mandatory padding was removed.\n"

SETTING "Setting TERMINFO=%s.\n"
$ Do not translate TERMINFO.  %s is a path name.

SETUP "Setting up terminal type '%s'.\n"
$ %s is a terminal name.

SPACE "infocmp: 1375-404 Malloc is out of space!\n"
$ Do not translate 'Malloc'.

STRCAP "There are %d string capabilities.\n"

TWOTERM "infocmp: 1375-405 Must have at least two terminal names\n\
for a comparison to be done.\n"

USAGE01 "usage: infocmp [-ducnILC1Vv] [-s d|i|l|c] [-A dir] [-B dir] term-names ...\n"
USAGE02 "\t-d\tPrint differences (the default for more than 1 term-name).\n"
USAGE03 "\t-u\tProduce relative description.\n"
USAGE04 "\t-c\tPrint common entries.\n"
USAGE05 "\t-n\tPrint capabilities that are not in either terminal.\n"
USAGE06 "\t-I\tPrint terminfo entries (the default for 1 term-name).\n"
USAGE07 "\t-C\tPrint termcap entries.\n"
USAGE08 "\t-L\tPrint long C variable names.\n"
USAGE09 "\t-1\tSingle column output.\n"
USAGE10 "\t-V\tPrint program version.\n"
USAGE11 "\t-v\tVerbose debugging output.\n"
USAGE12 "\t-s\tChange sort order.\n"
USAGE13 "\t-A\tSet $TERMINFO for first term-name.\n"
USAGE14 "\t-B\tSet $TERMINFO for other term-names.\n"
$ Do not translate: 'term-name', 'terminfo', 'termcap', 'C', or '$TERMINFO'.

VERSION "infocmp: infocmp.c is at version %s.\n"
$ Do not transtale 'infocmp.c'.

