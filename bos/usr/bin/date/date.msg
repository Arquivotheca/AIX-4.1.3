$ @(#)58    1.10.1.4  src/bos/usr/bin/date/date.msg, cmdstat, bos411, 9428A410j 11/22/93 11:21:18
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 71
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ 
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.1
$
$ 
$ RCSfile: date.msg,v  Revision: 1.5  (OSF) Date: 90/10/07 16:23:10 

$
$ messages for command date
$

$ Message numbers 0551-001 through 0551-006 and 0511-400 through 0511-449
$ are reserved for this command.

$quote "
$set MS_DATE

INV_FIELD   "date: 0551-001 %c is not a recognized field descriptor.\n"
$ Displayed when a unsupported field descriptor was specified.

BAD_PERM    "date: 0551-003 You must be root to change the date.\n"
$ Do not translate root

NOPORTS "date: 0551-004 All ports are in use.\n"

WRONG_ACK "date: 0551-005 Wrong acknowledgement received from time server: %s\n"

CANTREACH "date: 0551-006 Cannot reach the time server; time set locally.\n"

USAGE_ROOT1 "Usage: date [-n][-u] [mmddHHMM[yy]] [+\"Field Descriptors\"]\n"
$ Do not translate date and [mmddHHMM[yy]]

USAGE_ROOT2 "Usage: date [-n][-u] [mmddHHMM[.SSyy]] [+\"Field Descriptors\"]\n"
$ Do not translate date and [mmddHHMM[.SSyy]]


USAGE       "Usage: date [-u] [+\"Field Descriptors\"]\n"
$ Do not translate date

EPRIV       "date: 0551-400 Insufficient privileges\n"

NEED_PLUS   "date: 0551-401 Field descriptor must begin with a + (plus sign).\n"
$ Displayed when command line argument string contains % character.

INV_CHAR    "date: 0551-402 Invalid character in date/time specification.\n"
$ Displayed when date/time specification includes non-numeric characters. 

INV_FORM    "date: 0551-403 Format of date/time specification is not valid.\n"
$ Displayed when date/time specification is not in correct format.

INV_RANGE   "date: 0551-404 Specified value of date/time is out of range.\n"
$ Displayed when specified value of date/time is out of range.

MULTDATE "date: 0511-405 Do not specify more than one date/time parameter.\n"
$ Displayed when more than one date/time is specified.

