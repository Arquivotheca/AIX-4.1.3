$ @(#)78	1.3  src/bos/usr/ccs/bin/fsplit/fsplit.msg, cmdprog, bos411, 9428A410j 6/15/90 22:51:10
$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$len
$quote "         define quote character.

$set MS_fsplit        Define initial set#
$ The message numbers have been assigned a range of 075 - 099
$ Do not translate fsplit: , %s, %1$s, %2$s, etc.. 

M_MSG_3              "fsplit: 1286-075 Cannot open the input file %s.\n"
M_MSG_5              "fsplit: 1286-076 Cannot find %s\n"
M_MSG_8              "fsplit: 1286-077 File %1$s already exists. Writing to %2$s\n"
M_MSG_10             "Usage:  fsplit [-e SubprogramUnit] ... [File] \n"
M_MSG_11             "fsplit: 1286-078 Cannot generate more file names.\n"
M_MSG_12             "fsplit: 1286-079 Line truncated to %d characters.\n"
