$ @(#)47	1.10  src/bos/usr/bin/newform/newform.msg, cmdfiles, bos411, 9428A410j 3/18/93 18:12:13
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 450 - 464 reserved for this file.
$
$set MS_NEWFORM
$ Do not translate newform

M_NOPEN "newform: 0653-451 Cannot open %s.\n"

M_USAGE "Usage: newform [-f] [-s] [-a[Number]] [-b[Number]] [-c[Character]]\n\
\t\t[-e[Number]] [-i[Tabulation] [-l[Number]] [-o[Tabulation]]\n\
\t\t[-p[Number]] [File...]\n"

M_CWIDE "newform: 0653-452 The character specified on the -c flag is not supported.\n"

M_TWIDE "newform: 0653-453 Cannot split a multibyte display character.\n"

M_2LONG "newform: 0653-454 A line in the file is longer than %d bytes.\n"

M_INDIR "newform: 0653-455 The tabulation specification contains an error.\n\
\tThe tabulation specification format is \t-a\t-a2\n\
\t-c\t-c2\t-c3\t-f\t-p\t-s\t-u\n\
\t--\t--File\t-Number\tNumber,..,Number\n"

M_TSPEC "newform: 0653-456 The tabulation specification is not correct.\n\
\tThe tabulation specification format is \t-a\t-a2\n\
\t-c\t-c2\t-c3\t-f\t-p\t-s\t-u\n\
\t--\t--File\t-Number\tNumber,..,Number\n"

M_FORMT "newform: 0653-457 The file is not in a format supported by the -s flag.\r\n"
M_TOOLARGE "newform: 0653-458 Specify a number less than %d.\n"
$ Note: %d is a number.

