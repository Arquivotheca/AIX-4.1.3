$ @(#)05	1.13.2.1  src/bos/usr/lib/methods/chginet/chginet.msg, cmdnet, bos411, 9428A410j 10/4/90 15:40:38
$
$ Messages for CHGINET.c
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
$ Message numbers have been assigned a range of 206-221.
$ Do not translate chginet
$set MS_CHGINET

BADADDCUAT "0821-206 chginet: Cannot add customized attribute %s.\n"
BADADDROUTE "0821-207 chginet: Cannot add route record to CuAt.\n"
$ Do not translate CuAt.
GETCUDV "0821-208 chginet: Cannot get records from CuDv for inet0.\n"
$ Do not translate inet0.
GETGATEWAY "0821-209 chginet: Cannot get attribute record for gateway.\n"
GETHOSTNAME "0821-210 chginet: Cannot get hostname attribute record from CuAt.\n"
$ Do not translate CuAt, hostname.
GETROUTE "0821-211 chginet: Cannot get route attribute from CuAt.\n"
$ Do not translate CuAt.
NOTONE "0821-212 chginet: Expected one inet0 instance, retrieved %d from CuDv.\n"
$ Do not translate inet0 CuDv
OPENCUAT "0821-213 chginet: Cannot open CuAt.\n"
$ Do not translate CuAt
BADRTDEL "0821-216 chginet: Cannot delete route (%s) from CuAt.\n"
$ Do not translate CuAt
CANTADDGATEWAY  "0821-217 chginet: Route add gateway command failed.\n"
USAGE1 "\nusage:\t%s  [-d] [-a attr=val ...] [-D]\n"
GETBOOTOPTN "0821-218 chginet: Cannot get bootup_option attribute record from CuAt.\n"
