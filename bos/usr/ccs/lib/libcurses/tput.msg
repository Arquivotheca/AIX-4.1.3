$ @(#)08	1.1  src/bos/usr/ccs/lib/libcurses/tput.msg, libcurses, bos411, 9428A410j 9/3/93 15:14:26
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
$set MS_TPUT        Define initial set#

$ This catalog uses messages in the range 1375-200 -- 1375-299.

$ Do not translate 'tput'.

CANTOPEN "tput: 1375-200 Cannot open initialization file %s.\n"

NOINFO "tput: 1375-201 No terminfo database.\n"
$ Do not translate 'terminfo'.

NOVALUE "tput: 1375-202 No value for $TERM and -T was not specified.\n"
$ Do not translate '$TERM'.

UNKCAP "tput: 1375-203 The terminfo capability '%s' is not recognized.\n"
$ Do not translate 'terminfo'.

UNKTERM "tput: 1375-204 The terminal \"%s\" is not recognized.\n"

UNREAD "tput: 1375-205 Terminal descriptor \"%s\" is not readable.\n"

USAGE "Usage: \"tput [-S] [-T terminal-type] capability [tparm argument...]\"\n"
$ Do not translate 'tparm'.

