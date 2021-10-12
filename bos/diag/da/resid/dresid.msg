$ @(#)70	1.1  src/bos/diag/da/resid/dresid.msg, daresid, bos41J, 9520A_all 5/15/95 17:28:14
$  
$   COMPONENT_NAME: DARESID
$  
$   FUNCTIONS: 
$  
$   ORIGINS: 27
$  
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$  
$   (C) COPYRIGHT International Business Machines Corp. 1995
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

$set DRESID_SRN
$quote "
$ This set should always be the first set.  Do NOT move this set or put other
$ message sets before this one.

RR_PLANAR "Planar Tested bad."
R_SIMM_ERROR "Memory Tested bad."


$set REGULAR
$quote "
$
RTITLE	"ANALYZING MEMORY ERRORS FOUND DURING POST\n\n\
Please stand by."
$ 
$ This line is displayed during testing while in the non-advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set ADVANCED
$quote "
$ 
ATITLE	"ANALYZING MEMORY ERRORS FOUND DURING POST IN ADVANCED MODE\n\n\
Please stand by."
$ 
$ This line is displayed during testing while in the advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set ALOOP
$quote "
$ 
ALTITLE	"ANALYZING MEMORY ERRORS FOUND DURING POST IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This line is displayed during advanced mode testing in loop mode.
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.


$set LOOP
$quote "
$ 
LTITLE	"ANALYZING MEMORY ERRORS FOUND DURING POST\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$ 
$ This line is displayed during non-advanced mode testing in loop mode.
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.
