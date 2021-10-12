$ @(#)23	1.13  src/bos/usr/lib/methods/ucfginet/ucfginet.msg, cmdnet, bos411, 9428A410j 6/15/90 22:30:22
$
$ Messages for UCFGINET.c
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
$quote "
$ len 
$ Message numbers have been assigned a range of 256-265.
$set MS_UCFGINET        Define initial set#

BADUNCFG "0821-256 ucfginet: The sysconfig (SYS_CFGKMOD) system call failed.  "
$ Do not translate SYS_CFGKMOD.
BADUNLOAD "0821-257 ucfginet: The sysconfig (SYS_KULOAD) system call failed.  "
$ Do not translate SYS_KULOAD.
GETCUDV "0821-258 ucfginet: Cannot get records from CuDv.\n"
$ Do not translate CuDv.
BADQUERY "0821-259 ucfginet: error querying kernel extension "
$ Do not translate kmid, netinet, ifconfig.
OPENCUDV "0821-260 ucfginet: Cannot open CuDv.\n"
$ Do not translate CuDv.
USAGE1 "\nUsage:\t%s\n"
