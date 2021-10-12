$ @(#)98	1.7.1.4  src/bos/usr/bin/df/df.msg, cmdfs, bos411, 9428A410j 1/11/94 17:12:14
$ 
$  COMPONENT_NAME: (CMDFS) commands that deal with the file system
$ 
$  FUNCTIONS: df messages
$ 
$  ORIGINS: 27
$ 
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$   
$quote "
$ define quote character.
$len
$set MS_DF
$ Define initial set #
$
$  Message numbers have been assigned a range of 001-009 
$  Do not translate df:, %s 
$       The following 11 messages are column labels. DO NOT CHANGE THE WIDTH
$       of these messages if possible. 

FILSYS               "Filesystem   "
$ after translation, pad with trailing spaces so that it's at least 13 
$	chars wide

MOUNT                "Mounted on        "
$ after translation, pad with trailing spaces so that it is 18 chars wide


BLK1024		     "1024-blocks"
$ after translation, pad with leading spaces so that it's at least 9 chars wide

BLK512		     "512-blocks"
$ after translation, pad with leading spaces so that it's at least 9 chars wide

AVAIL		     "Available"
$ after translation, pad with leading spaces so that it's at least 9 chars wide

CAPACITY	     "Capacity"
$ after translation, pad with leading spaces so that it's at least 4 chars wide

USED                 "     Used"
$ after translation, pad with leading spaces so that it's at least 9 chars wide

FREE                 "     Free"
$ after translation, pad with leading spaces so that it's at least 9 chars wide

FREES                "    Free*"
$ after translation, pad with leading spaces so that it's at least 9 chars wide

pUSED                "%Used"
$ after translation, pad with leading spaces so that it's at least 4 chars wide

IUSED                "   Iused"
$ after translation, pad with leading spaces so that it's at least 8 chars wide

IFREE                "   Ifree"
$ after translation, pad with leading spaces so that it's at least 8 chars wide

pIUSED               "%Iused"
$ after translation, pad with leading spaces so that it's at least 4 chars wide

SMO                  "Mounted on"
$ after translation, pad with leading spaces so that it's at least 10
$	 chars wide

USAGE                "Usage: df  [-P] | [-IMitv] [-k] [-s] [filesystem ...] [file ...]\n"

NOMNTTBL             "df: 0506-001 Cannot get mount table information.\n"

MALLOC               "df: 0506-002 There is not enough memory available now.\n"

NOFS                 "df: 0506-003 Cannot find or open file system %s.\n"

NOFIND               "df: 0506-004 Cannot find %s in /etc/filesystems.\n"
$  Do not translate /etc/filesystems.

