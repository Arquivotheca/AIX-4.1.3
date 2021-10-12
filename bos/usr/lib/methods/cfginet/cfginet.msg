$ @(#)01	1.13  src/bos/usr/lib/methods/cfginet/cfginet.msg, cmdnet, bos411, 9428A410j 6/15/90 22:28:28
$
$ Messages for CFGINET.c
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
$ Message numbers have been assigned a range of 189-205.
$ Do not translate cfginet.
$set MS_CFGINET
BADCFG "0821-189 cfginet: The sysconfig (SYS_CFGKMOD) system call failed.  "
$ Do not translate SYS_CFGKMOD
BADLOAD	"0821-190 cfginet: The sysconfig (SYS_SINGLELOAD) system call failed.  "
$ Do not translate SYS_SINGLELOAD
BADSTATE "0821-191 cfginet: Expected status = DEFINED. Got status = %s.\n"
$ Do not translate DEFINED.
GETCUAT "0821-193 cfginet: Cannot get attribute record.\n"
GETCUDV "0821-194 cfginet: Cannot get records from CuDv.\n"
$ Do not translate CuDv.
GETPDDV "0821-195 cfginet: Cannot get records from PdDv.\n"
$ Do not translate PdDv.
NOTONE "0821-196 cfginet: Expected one inet0 instance, retrieved %d.\n"
$ Do not translate inet0
NOTONEINET "0821-197 cfginet: expected one INET record, retrieved %d.\n"
$ Do not translate INET
OPENCUDV "0821-198 cfginet: Cannot open CuDv.\n"
$ Do not translate CuDv.
BADQUERY "0821-200 cfginet: error querying kernel extension "
USAGE1 "\nusage:\t%s\n"
USAGE2 "\t%s  -h\n"
