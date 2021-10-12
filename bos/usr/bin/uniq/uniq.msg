$ @(#)25        1.8  src/bos/usr/bin/uniq/uniq.msg, cmdfiles, bos411, 9428A410j 12/10/92 13:13:00
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
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ RCSfile: uniq.msg,v  Revision: 1.4  (OSF) Date: 90/10/07 17:17:37 
$
$
$quote "
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate uniq -- this is the command name.
$set MS_UNIQ
$
OPERR "uniq: 0653-721 Cannot open file %s.\n"
$ %s is the input file name.
CRERR "uniq: 0653-722 Cannot create file %s.\n"
$ %s is the output file name.
USAGE "Usage: uniq [-c | -d | -u] [-f Fields] [-s Chars] [-Fields] [+Chars] [InFile [OutFile]]\n"
$ Fields  is a number of fields.  Chars  is a number of characters.
$ InFile  is an input file name.  OutFile is an output file name.
BADNUM "uniq: 0653-720 Specify a numeric value for option %s.\n"
$ %s is an option that requires a numeric value.
$ This invocation of uniq had a non-numeric string following %s.
BADOPT "uniq: 0653-723 Too many File operands.\n"
$ The uniq command accepts 0, 1, or 2 file operands.
$ This invocation of uniq had 3 or more file operands. 
NONTEXT "uniq: 0653-724 %s is not a text file.\n"
SAMEFIL "uniq: 0653-725 Input and output files can not be the same.\n"
OUTOFRANGE "uniq: 0653-726 Number is out of range for option %c.  The range of the number parameter is from 0 to 2048.\n"
