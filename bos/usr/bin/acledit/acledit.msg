$ @(#)13	1.8  src/bos/usr/bin/acledit/acledit.msg, cmdsdac, bos411, 9428A410j 2/2/94 13:20:37
$
$ COMPONENT_NAME:  (CMDSDAC) security: access control
$
$ FUNCTIONS:  acledit
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages numbers reserved for acledit: 3002-100 thru 3002-109
$

$set MS_ACLEDIT

$quote "

USAGE	  "Usage:  %s filename\n"
NOED	  "3002-100  %s: EDITOR environment variable not set\n"
NOTMP	  "3002-101  %s: Cannot create temporary file\n"
NOACC	  "3002-102  %s: Cannot access %s\n"
APPLY	  "Should the modified ACL be applied? (%s) or (%s) "
NOEXEC	  "3002-103  Cannot execute %s\n"
EDNOTABS  "3002-104  %s: EDITOR environment variable must be full pathname\n"
YESSTR    "yes"
NOSTR     "no"

$ NOTE TO TRANSLATORS:  Do not translate either "EDITOR" or "ACL"
