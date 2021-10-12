$ @(#)88	1.10  src/bos/usr/bin/split/split.msg, cmdfiles, bos411, 9428A410j 1/11/93 17:41:55
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
$ $ Message numbers 680 - 689 reserved for this file.
$
$set MS_SPLIT

USAGE "Usage: split [-l Line_Count] [-a Suffix_Length] [File [Prefix]]\n\
   or: split -b Number[k|m] [-a Suffix_Length] [File [Prefix]]\n"
INPOPNER "split: 0653-680 Cannot open input file.\n"
OUTNMSIZ "split: 0653-681 Output file cannot exceed %d characters.\n"
$ %d is the maximum number of characters in the output file.
ABRTSPLT "split: 0653-682 Split is terminated. Exceeded the maximum of %d output files.\n"
$ %d is the maximum number of output files.
INVALNUMB "split: 0653-683 Number of bytes specified is not valid.\n."
INVALNUML "split: 0653-684 Number of lines specified is not valid.\n."
INVALMULT "split: 0653-685 Multiplier specified is not valid.\n"
