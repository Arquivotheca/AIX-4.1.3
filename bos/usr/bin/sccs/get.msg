$ static char sccsid[] = "@(#)12  1.7  src/bos/usr/bin/sccs/get.msg, cmdsccs, bos411, 9428A410j 7/9/93 12:50:06";
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

$ Do not translate get:
$ Do not translate the paranthesis (ge#) at the end of each line.
$len 
$set 7        Define initial set#

ENOTWITHM       "get: 1255-103 Cannot use the -e flag with the -m flag. (ge3)\n"
$ Do not translate -e, -m
WRITFILEEXST    "get: 1255-104 A writable version of %s exists. (ge4)\n"
ALRDYINC        "get: 1255-105 The delta %s is already included. (ge9)\n"
$ %s is a delta. 
ALRDYEXCLD      "get: 1255-106 The delta %s is already excluded. (ge10)\n"
$ %s is a delta. 
INTRNERR11      "get: 1255-107 There is an internal software error. (ge11)\n"
BNGEDT          "get: 1255-108  Another user is editing %s. (ge17)\n"
WARNMSG         "WARNING: The file %s is being edited. (ge18)\n"
SERNOTOOLRG     "get: 1255-110 The specified serial number does not exist. (ge19)\n"
$ you can do a get -a to specify delta serial number 
$ instead of SID number.  This is not documented in commands reference. 
$ By the way, the serial number overrides the SID number 
ULINES          "%u lines\n"
$ %u is the number of lines. 
BADSIDCALC      "get: 1255-113 There is a bad SID calculated in newsid()."
$ Do not translate this message. The user will never see it. 
NEWDELTA        "new delta %s\n"
$ %s is the new delta. 
INVALIDSID      "The specified SID is not valid.\n\tUse the sact command to \
check the p-file for valid SID numbers.\n"
GET_USAGE "\
Usage: get [-gmnpst -ccutoff -ilist -rSID -xlist -wstring]... [-l -lp]\
 [file]...\n\
Usage: get -e -k [-bst -ccutoff -ilist -rSID -xlist]... [-l -lp] [file]...\n"
