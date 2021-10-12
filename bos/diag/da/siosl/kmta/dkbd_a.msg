$ @(#)72	1.4  src/bos/diag/da/siosl/kmta/dkbd_a.msg, dakbd, bos411, 9428A410j 12/11/92 15:01:24
$
$   COMPONENT_NAME: DAKBD
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1990,1992
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
$set DA_TITLE

KDB_TITLE "TESTING KEYBOARD CONNECTOR"
$
$ See general notes on length of title.

KDB_TITLE_A "TESTING KEYBOARD CONNECTOR IN ADVANCED MODE"
$
$ See general notes on length of title.

KDB_STND_BY "Please stand by."

$set DKB_INLOOP

DKB_INLOOP_TITLE "TESTING KEYBOARD CONNECTOR IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ The previous lines are used to inform the user on the number of loops tested.
$ Leave line spacing as shown.  See general notes on length of title.
