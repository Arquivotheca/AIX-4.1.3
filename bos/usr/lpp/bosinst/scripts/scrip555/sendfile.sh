#! /bin/sh
# @(#)67 1.2 src/bos/usr/lpp/bosinst/scripts/scrip555/sendfile.sh, bosinst, bos411, 9428A410j 8/16/91 15:55:12
#
# COMPONENT_NAME: (BOSINST) Base Operating System Installation
#
# FUNCTIONS: dbextract
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1991
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

if [ ! -r "$bosobjname" ]
then
	exit 1
fi

cd /tmp/netinstall/$IPfilename >/dev/null 2>&1 || exit 2
{ dd if="$bosobjname" bs=512 conv=sync count=4 2>/dev/null; echo '\0'; } \
	| tar -xf - -b1 ./.fs.size > /dev/null 2>&1 || exit 3
cat /tmp/netinstall/$IPfilename/.fs.size 2>/dev/null || exit 4
exit 0
