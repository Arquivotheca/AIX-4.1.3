$ @(#)21	1.10  src/bos/usr/lib/methods/ucfgif/ucfgif.msg, cmdnet, bos411, 9428A410j 6/15/90 22:30:12
$
$ Messages for UCFGIF.c 
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
$quote "         define quote character.
$ len 
$
$ Message numbers have been assigned a range of 149-156.
$
$set MS_UCFGIF        Define initial set#

FREECUDV "0821-149 ucfgif: Freeing the CuDv list.\n"
$ Do not translate CuDv
GETCUDV "0821-150 ucfgif: Cannot get records from CuDv.\n"
$ Do not translate CuDv
OPENCUDV "0821-151 ucfgif: Cannot open CuDv.\n"
$ Do not translate CuDv
USAGE1 "\nusage:\t%s [-l interface ] ...\n"
