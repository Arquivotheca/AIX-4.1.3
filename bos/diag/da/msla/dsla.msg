$ @(#)16        1.5.1.1  src/bos/diag/da/msla/dsla.msg, damsla, bos411, 9433A411a 8/15/94 14:05:32
$
$   COMPONENT_NAME: DAMSLA
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1992
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

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
$set MSLA_INLOOP
$

MSLA_INLOOP_TITLE " "
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.

MSLA_INLOOP_EMPTY " "
$
$ This is a blank message.  DO NOT TRANSLATE.


$set MSL_TITLES
$

MSL_NOTIFY " "

MSL_ADV " "

MSL_SBY  " "

MSL_NAME "5085/86/88 ATTACHMENT ADAPTER"


