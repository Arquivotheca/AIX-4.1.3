$ static char sccsid[] = "@(#)06  1.8  src/bos/usr/bin/sccs/delta.msg, cmdsccs, bos411, 9428A410j 7/9/93 12:49:53";
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$len 
$ Do not translate %s, %d, etcc...
$ Do not translate delta:, SID, MR
$ Do not translate the parenthesis (de#) at the end of each line.
$set 6        Define initial set#

MISSRKYLTR    "delta: 1255-086 Specify an SID with the -r flag. (de1)\n"
$ do not translate -r
NAMORSID      "delta: 1255-087 The specified SID or your user name is not\n\
\tin the p-file. (de2)\n"
$ Do not translate p-file  
BADDIFF       "delta: 1255-089 The diff program failed when SCCS tried to \n\
\tcreate the delta. (de4)\n"
$ Do not translate diff
INTRNERR5     "delta: 1255-090 There is an internal software error. (de5)\n"
INTRNERR6     "delta: 1255-091 There is an internal software error. (de6)\n"
INTRNERR7     "delta: 1255-092 There is an internal software error. (de7)\n"
RETRYSEG      "WARNING: The system is trying %1$s again\n\
\twith a segmentation size of %2$d. (de13).\n"
$ %s is the name of the program that delta uses to compare the files. 
$ %d is the segmentation size. 
AMBIG         "delta: 1255-095 Specify the -r flag when there is more than one outstanding delta. (de15)\n"
STDINSPCD     "delta: 1255-096 Specify the -y or the -m flag when using \n\
\tstandard input. (de16)\n"
$ Do not translate -y or -m
ILLDAT        "delta: 1255-097 Cannot use the SOH character in the\n\
\tfirst position of line %1$d of file %2$s. (de14)\n"
$ %1$d is the line number. 
$ %2$s is the file name. 
INSM          "%u inserted\n"
$ %u is the number of lines. 
DELM          "%u deleted\n"
$ %u is the number of lines. 
UNC           "%u unchanged\n"
$ %u is the number of lines. 
AUTONDELTA    "AUTO NULL DELTA\n"
$ Do not translate. The user does not see this message. 
MCOMMENTS     "comments"
$ Note to translators: translate the above word exactly as it is. 
EXLINEMAX     "delta: 1255-098 maximum line length of %d bytes exceeded.\n"
$ %d is LINE_MAX - 2
DELTA_USAGE "Usage: delta [-rSID -s -n -glist -p] [-m[mrlist]]\
 [-y[comment]] files\n"
