$ @(#)13	1.11  src/bos/usr/lib/methods/stpinet/stpinet.msg, cmdnet, bos411, 9428A410j 6/15/90 22:29:34
$
$ Messages for STPINET.c 
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
$ Do not translate stpinet.
$
$ Message numbers have been assigned a range of 078 - 088
$
$set MS_STPINET        Define initial set#

FREECUDV "0821-078 stpinet: Freeing the CuDv list.\n"
$ Do not translate CuDv.
GETCUDV "0821-079 stpinet: Cannot get records from CuDv.\n"
$ Do not translate CuDv.
NOTCFGD "0821-080 stpinet: TCP/IP is not configured yet.\n"
$ Do not translate TCP/IP
NOWARNING "/bin/echo 'Warning - TCP/IP will stop immediately!' | wall"
$ Do not translate TCP/IP.
$ Do not translate /bin/echo.
$ Do not translate | wall
OPENCUAT "0821-081 stpinet: Cannot open CuAt.\n"
$ Do not translate CuAt.
OPENCUDV "0821-082 stpinet: Cannot open CuDv.\n"
$ Do not translate CuDv.
TIMEWARN "/bin/echo 'Warning - TCP/IP will stop in %d minute' | wall"
$ Do not translate TCP/IP.
$ Do not translate /bin/echo.
$ Do not translate | wall.
USAGE1 "\nusage:\t%s [-l interface] ... [-t minutes]\n"
