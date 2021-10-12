$ @(#)87	1.10  src/bos/usr/bin/sort/sort.msg, cmdfiles, bos411, 9428A410j 11/5/92 15:57:11
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 18, 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 Release 1.0
$
$ RCSfile: sort.msg,v  Revision: 1.4  (OSF) Date: 90/10/07 17:03:42 
$

$quote "
$
$ Message set MS_SORT is obsolete. ALL messages in this set have been replaced
$ by messages in set SORTMSG2 below.
$
$ Note to NLS translator: don't add a new line character to
$ the end of any of these that do not already have one.
$

$set SORTMSG2
USAGE2 "Usage:   sort\t[-Abcdfimnru] [-T Directory] [-t Character] [-o File]\n\
\t\t[-y[Kilobytes]] [-z Recordsize] [-k Keydefinition]...\n\
\t\t[[+Position1][-Position2]]... [File]...\n"
$ Translate these words in the "Usage" message:
$  "Usage", "Directory", "Character", and "File" : Translate as usual.
$  "Keydefinition": Definition of a sort key field.
$  "Position": Position of a sort key field in a line.
$  "Recordsize": Maximum size of a record (1 line of text) during a sort.
PATH2 "0653-646 The path name is too long: %s\n"
KEYS2 "0653-647 Cannot use more than 10 sort keys.\n"
CHECK2 "0653-648 Specify only one input file when using the -c flag.\n"
ALLOC2 "0653-649 There is not enough memory available to perform the sort.\n"
$ Do not translate sort
LOCATE2 "0653-650 Cannot locate the temporary directory or /tmp.\n"
MALLOC2 "0653-651 There is not enough memory available to perform the merge.\n"
TOOLONG2 "0653-652 A line of the input file contains more than %d characters.\n"
CREATE2 "0653-654 Cannot create %s\n"
OPEN2 "0653-655 Cannot open %s\n"
$ %s is a file name. The sort command cannot open the file.
EREAD2 "0653-656 The read system call failed. Cannot read from %s\n"
$ Do not translate "read" in "read system call". %s is a file name.
STDIN2 "standard input"
$ This is the standard input name. It may be substituted into message EREAD2 above.
EWRITE2 "0653-657 A write error occurred while sorting.\n"
EWRITE3 "0653-657 A write error occurred while merging.\n"
NULLS2 "0653-658 Cannot process the specified file. It contains null characters.\n"
DISORDER2 "This line is out of order: %s\n"
NUNIQUE2 "Duplicate line: %s\n"
NEWLINE4 "Warning: A newline character was added to the end of the input.\n"
NEWLINE5 "Warning: A newline character was added to the end of file %s.\n"
