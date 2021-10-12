$ @(#)28	1.9  src/bos/usr/lib/methods/udefinet/udefinet.msg, cmdnet, bos411, 9428A410j 6/15/90 22:30:42
$
$ Messages for UDEFINET.c
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
$
$ Do not translate udefinet.
$
$ Message numbers have been assigned a range of 133-148.
$
$set MS_UDEFINET        Define initial set#

CUATDEL "0821-133 udefinet: Cannot delete from CuAt.\n"
$ Do not translate CuAt.
CUDEPDEL "0821-134 udefinet: Cannot delete from CuDep.\n"
$ Do not translate CuDep.
CUDVDEL "0821-135 udefinet: Cannot delete inet0 from CuDv.\n"
$ Do not translate inet, CuDv.
FREECUAT "0821-136 udefinet: Cannot free CuDv list.\n"
$ Do not translate CuDv.
GETCUDV "0821-137 udefinet: Cannot get records from CuDv.\n"
$ Do not translate CuDv.
GETINET "0821-138 udefinet: Expected one inet0 instance, retrieved %d.\n"
$ Do not translate inet0.
INETSTAT "0821-139 udefinet: The inet0 must not be in the STOPPED or AVAILABLE state.\n"
$ Do not translate inet0.
OPENCUDP "0821-140 udefinet: Cannot open CuDep.\n"
$ Do not translate CuDep.
OPENCUAT "0821-141 udefinet: Cannot open CuAt.\n"
$ Do not translate CuAt.
OPENCUDV "0821-142 udefinet: Cannot open CuDv.\n"
$ Do not translate CuDv.
USAGE1 "\nusage:\t%s \n"
