#! /bin/sh
# @(#)33 1.1 src/bos/usr/lpp/bosinst/scripts/scrip500/sendfssz.sh, bosinst, bos411, 9428A410j 4/3/90 19:28:40
# @(#)dbextract.sh	1.1  com/bosinst/netinst,3.1 11/15/89 08:43:20
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

bosobjname=$1

if [ ! -f "$bosobjname" ]
then
	exit 1
fi

cd /tmp/netinstall/$IPfilename >/dev/null 2>&1 || exit 2
restore -xqf"$bosobjname" ./.fs.size >/dev/null 2>&1 || exit 3
cat /tmp/netinstall/$IPfilename/.fs.size 2>/dev/null || exit 4
exit 0
