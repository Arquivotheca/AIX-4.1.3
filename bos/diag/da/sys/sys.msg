$ @(#)13	1.6  src/bos/diag/da/sys/sys.msg, dasys, bos411, 9428A410j 8/5/93 16:35:38
$
$   COMPONENT_NAME: DASYS
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1993
$   All Rights Reserved
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


$set REGULAR
$quote "
$ 
RTITLE	"TESTING PROCESSOR\n\n\
Please stand by."
$ 
$ This line is displayed during testing while in the non-advanced mode.
$ Leave line spacing as shown.  See general notes on length of title lines.


$set ADVANCED
$quote "
$ 
ATITLE	"TESTING PROCESSOR IN ADVANCED MODE\n\n\
Please stand by."
$ 
$ This line is displayed during testing while in the advanced mode.
$ Leave line spacing as shown.  See general notes on length of title lines.


$set ALOOP
$quote "
$ 
ALTITLE	"TESTING PROCESSOR IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title lines.
$ This line is displayed during advanced mode testing in loop mode.


$set LOOP
$quote "
$ 
LTITLE	"TESTING PROCESSOR\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title lines.
$ This line is displayed during non-advanced mode testing in loop mode.

$set MACHSTOP
$quote "
$ 
MACH_MSG "818999 \
ERROR LOG ANALYSIS INDICATES MEMORY FAILURE\n\n\
The previous service request number 818-%x requires a fully populated\n\
memory card(s) to be replaced."
$ 

$set MACHCHECK
$quote "
$ 
MACHCHK_MSG "818900 \
ERROR LOG ANALYSIS INDICATES A MACHINE CHECK HAS OCCURRED\n\n\
This machine check is most likely to be caused by software."
$ 
