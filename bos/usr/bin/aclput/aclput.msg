$ @(#)15	1.8  src/bos/usr/bin/aclput/aclput.msg, cmdsdac, bos411, 9428A410j 12/1/93 10:31:58
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
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages for aclput
$ 3002-120 thru 3002-129
$

$set MS_ACLPUT

$quote "

USAGE		"usage:  aclput [-i infile] filename\n"
NOTMP		"3002-120  Cannot create temp file\n"
NOWRITE		"3002-121  Cannot open %s for writing\n"
UNKKEY		"unknown keyword: %s"
UNKATTR		"unknown attribute: %s"
NOUPERM		"permissions for owner not found"
EXPPSET		"expected permission set"
NOGPERM		"permissions for group not found"
NOOPERM		"permissions for others not found"
BADACC		"bad access mode: %s"
ILLFORM		"ill formed type/ID pair"
NOIDENT		"no identifiers"
UNKUSER		"unknown user: %s"
UNKGRP		"unknown group: %s"
UNKID		"unknown ID type: %s"
LINENO		"* line number %d: %s\n"
CANTMALL	"3002-122  Cannot malloc new ACL buffer\n"
TOOBIG		"3002-123  The size of the ACL exceeds the system limit.\n"
$ Do not translate ACL - it is the abbreviation for Access Control List
