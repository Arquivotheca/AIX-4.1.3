$ @(#)19        1.8  src/bos/usr/bin/sccs/val.msg, cmdsccs, bos411, 9428A410j 1/26/94 14:03:39
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
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$len 
$ Do not translate val:, SCCS, SID
$ Do not translate %s, %d, %1$s, etc..
$set 12        Define initial set#

MMSMTCH              "val: 1255-144 The value specified by -m does not match the %2$cM%3$c\n\
\tidentification keyword value specified in the SCCS file %1$s.\n"
$ Do not translate -m 
$ %2$cM%3$c represents the identification keyword for module name. 
YMSMTCH              "val: 1255-145 The parameter specified by -y does not match the\n\
\ttext specified by %2$cY%3$c in the SCCS file %1$s.\n"
$ Do not translate -y 
$ %2$cY%3$c represents the identification keyword for module type. 
SIDNOEXST            "val: 1255-146 The specified SID does not exist for the file %s.\n"
$ %s is the file name. 
SIDINVLD             "val: 1255-147 The specified SID is not valid for the file %s.\n"
$ %s is the file name. 
OPNSCCS              "val: 1255-148 Cannot open the file %s or the file is not an SCCS file.\n"
$ %s is the file name. 
CRPTSCCS             "val: 1255-149 %s is a damaged SCCS file.\n"
$ %s is the file name. 
DUPKEYLTR            "val: 1255-150 The specified flag does not exist\n\
\tfor this command or is a duplicate on the command line.\n"
MSGFILARG            "val: 1255-151 Specify a file name or -.\n"
$
VAL_USAGE            "Usage: val [ -s ] [ -r SID ] [ -m Name ] [ -y Type ] File...\n"
