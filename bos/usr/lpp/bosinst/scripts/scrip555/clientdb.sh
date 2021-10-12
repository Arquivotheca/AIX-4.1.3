#! /bin/sh
# @(#)32 1.2 src/bos/usr/lpp/bosinst/scripts/scrip555/clientdb.sh, bosinst, bos411, 9428A410j 6/15/90 19:48:45
#
# COMPONENT_NAME: (BOSINST) Base Operating System Installation
#
# FUNCTIONS: dbextract
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. `dirname $0`/defaults		# load defaults

IPfilename=`echo $cIPaddr |
	awk -F. '
		{
			printf("%03d%03d%03d%03d\n",$1,$2,$3,$4)
		}
	'
`

set "$@"
if test "$1" = "netdone"
then
	cat /tmp/netinstall/$IPfilename/clientdb
	rm -fr /tmp/netinstall/$IPfilename
else
	. /tmp/netinstall/$IPfilename/clientdb
fi
