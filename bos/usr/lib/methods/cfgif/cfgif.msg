$ @(#)99	1.6.1.6  src/bos/usr/lib/methods/cfgif/cfgif.msg, cmdnet, bos411, 9428A410j 4/9/90 13:42:57
$
$ Messages for CFGIF.c 
$
$ COMPONENT_NAME:  CMDNET   Network Configuration  
$
$ ORIGINS: 27 
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1989. 
$ All Rights Reserved 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$ Messages for cfgif
$
$ Do not translate cfgif, %s, %d, %c, %m, %1$s, %2$s, etc... .
$
$ len 
$quote "         define quote character.
$
$ Message numbers have been assigned a range of 001-013.
$
$set MS_CFGIF        Define initial set#

GETCUAT "0821-001 cfgif: Getting records from CuAt.\n"
$ Do not translate CuAt.
GETCUDV "0821-002 cfgif: Getting records from CuDv.\n"
$ Do not translate CuDv.
NOCFG "0821-003 cfgif: No interfaces were configured.\n"
NOCUDV "0821-004 cfgif: There is no customized device record for %s.\n"
OPENCUDV "0821-006 cfgif: Cannot open CuDv.\n"
$ Do not translate CuDv.
IFCFGFAIL "0821-007 cfgif: ifconfig command failed.\n\t\
The status of\"%s\" Interface in the current running system is uncertain.\n"
$ Do not translate ifconfig
USAGE1 "\nusage:\t%s [-l Interface] ...\n"
USAGE2 "\t%s -h\n"
