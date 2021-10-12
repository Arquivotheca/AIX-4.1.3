$ @(#)55	1.5  src/bos/usr/bin/units/units.msg, cmdmisc, bos411, 9428A410j 1/22/93 11:17:37
$
$ COMPONENT_NAME: (CMDMISC) miscellaneous commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set MS_UNITS
$quote "
NOTABLE	 "units: 0702-005 Table, %s, was not found.\n"
UHAVE	 "Please enter the unit you wish to translate from: "
UNDOVFLOW	 "units: 0702-006 underflow or overflow.\n"
$ The number is too big or too small.
UWANT	 "Please enter the unit you wish to translate to: "
CONFO	 "conformability\n"
$ This tells the user that the two units cannot be converted. i.e.
$ meters to seconds.  units then gives an answer using the base
$ units in its table.
CANTREC	 "units: 0702-007 Cannot recognize %S\n"
DUMP	 "%d units; %d bytes\n\n"
REDEF 	 "units: 0702-008 Redefinition %S\n"
