$ @(#)15	1.10  src/bos/usr/lib/methods/sttinet/sttinet.msg, cmdnet, bos411, 9428A410j 6/15/90 22:29:44
$
$ Messages for STTINET.c  
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
$ len 
$quote "         define quote character.
$
$ Do not translate sttinet.
$
$ Message numbers have been assigned a range of 089 - 101
$
$set MS_STTINET        Define initial set#

FREECUDV "0821-089 sttinet: Freeing the CuDv list.\n"
$ Do not translate CuDv. 
GETCUDV "0821-090 sttinet: Cannot get records from CuDv.\n"
$ Do not translate CuDv. 
IFNOTCFG "0821-091 sttinet: %s is not configured.\n"
NOTDEFINED "0821-092 sttinet: TCP/IP is not configured.\n"
$ Do not translate TCP/IP. 
NOTONEINET "0821-093 sttinet: Expected one inet0 instance, retrieved %d.\n"
OPENCUAT "0821-094 sttinet: Cannot open CuAt.\n"
$ Do not translate CuAt. 
OPENCUDV "0821-095 sttinet: Cannot open CuDv.\n"
$ Do not translate CuDv.
USAGE1 "\nusage:\t%s [-l interface ...]\n"
