$ @(#)60	1.4  src/bos/diag/da/memory/memory.msg, damemory, bos411, 9428A410j 1/15/93 09:36:51
$  
$   COMPONENT_NAME: DAMEMORY
$  
$   FUNCTIONS: 
$  
$   ORIGINS: 27
$  
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$  
$   (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1993
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

  
$set MEM_TEST_NOTEST
$quote "
$ 
MEM_TEST_TITLE "TESTING MEMORY\n\n\
%1$d blocks (16K bytes per block) of memory (%2$d%%) have been\n\
determined to be defective.\n\n\
Do you wish to ignore the problem?"
$
$ K stands for kilo (as in kilobytes) and means 1024.
$ %1$d is the number of block that are found to be defective.
$ %2$d%% shows the percentage of memory that is defective.  Be sure that this
$ string is replaced exactly as it is shown.  The double percent signs '%%'
$ are necessary to produce a percent sign in the sentence.  It will read
$ "2 blocks (16K bytes per block) of memory (5%) were found defective."
$ Leave line spacing as shown.  See general notes on length of title line.


MEM_TEST_YES "YES"
$ This option is shown when a YES answer is possible.

MEM_TEST_NO  "NO"
$ This option is shown when a NO answer is possible.

MEM_TEST_ACTION "Move cursor to selection, then press Enter."
$ This message is shown when a multiple selection list is presented.


$set REGULAR
$quote "
$
RTITLE	"TESTING MEMORY\n\n\
Please stand by."
$ 
$ This line is displayed during testing while in the non-advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set ADVANCED
$quote "
$ 
ATITLE	"TESTING MEMORY IN ADVANCED MODE\n\n\
Please stand by."
$ 
$ This line is displayed during testing while in the advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set ALOOP
$quote "
$ 
ALTITLE	"TESTING MEMORY IN ADVANCED MODE\n\n\
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
LTITLE	"TESTING MEMORY\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$ 
$ This line is displayed during non-advanced mode testing in loop mode.
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.
