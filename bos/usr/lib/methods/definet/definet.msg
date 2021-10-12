$ @(#)09	1.11  src/bos/usr/lib/methods/definet/definet.msg, cmdnet, bos411, 9428A410j 6/15/90 22:29:19
$
$ Messages for DEFINET.c 
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
$ Do not translate definet.
$
$ Message numbers have been assigned a range of 038-053.
$
$set MS_DEFINET        Define initial set#

BADADD "0821-038 definet: Cannot add a customized inet record.\n"
$ Do not translate inet.
BADCLASS "0821-039 definet: The class specified must be tcpip.\n"
$ Do not translate tcpip
GETCUDV "0821-041 definet: Cannot get records from CuDv.\n"
$ Do not translate CuDv
GETPDDV "0821-042 definet: Cannot get records from PdDv.\n"
$ Do not translate PdDv
OPENCUDP "0821-043 definet: Cannot open CuDep.\n"
$ Do not translate CuDep
OPENCUDV "0821-044 definet: Cannot open CuDv.\n"
$ Do not translate CuDv
OPENPDAT "0821-045 definet: Cannot open PdAt.\n"
$ Do not translate PdAt
OPENPDCN "0821-046 definet: Cannot open PdCn.\n"
$ Do not translate PdCn
OPENPDDV "0821-047 definet: Cannot open PdDv.\n"
$ Do not translate PdDv
USAGE1 "\nusage:\t%s [-c class]\n"
USAGE2 "\t%s -h\n"
