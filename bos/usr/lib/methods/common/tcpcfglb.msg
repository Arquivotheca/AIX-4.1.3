$ @(#)19	1.11  src/bos/usr/lib/methods/common/tcpcfglb.msg, cmdnet, bos411, 9428A410j 6/15/90 22:30:01
$
$ Messages for TCPCFGLB.c 
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
$ Do not translate tcpcfglb.
$
$ Message numbers have been assigned a range of 102 - 117
$
$set MS_TCPCFGLB        Define initial set#

BADPATH "0821-102 : Cannot set the default path to %s.\n"
EXECFAIL "0821-103 : The command %s failed.\n"
FORKFAIL "0821-104 : Cannot start process %2$s.\n\tThe error number is %1$d.\n"
GETCUAT_ERR "0821-105 : Cannot get records from CuAt.\n"
$ Do not translate CuAt, tcp_getattr.
GETCUDV_ERR "0821-106 : Cannot get records from CuDv.\n"
$ Do not translate CuDv, tcp_getattr.
GETPDAT_ERR "0821-107 : Cannot get records from PdAt.\n"
$ Do not translate PdAt, tcp_getattr.
INITFAIL "0821-108 : The database initialization failed.\n"
NOLOCK "0821-109 : Another process owns the lock on the configuration database.\n"
NOMEM "0821-110 : There is not enough memory available for new attribute.\n"
$ Do not translate tcp_getattr.
NOPDAT_OBJ_ERR "0821-111 : no attributes found in PdAt for the criteria\n%s\n"
$ Do not translate.
