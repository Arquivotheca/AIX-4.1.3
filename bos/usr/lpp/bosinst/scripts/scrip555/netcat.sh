#! /bin/sh
# @(#)59	1.6  src/bos/usr/lpp/bosinst/scripts/scrip555/netcat.sh, bosinst, bos411, 9428A410j 4/30/93 17:58:17
#
#   COMPONENT_NAME: BOSINST
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. `dirname $0`/defaults

if test $# -ne 1
then
	echo "usage: `basename $0` filename" 1>&2
	exit 1
fi

case $1 in
	../* | */../* | */.. )
		echo "$0: filename '$1' contains .. backward reference" 1>&2
		exit 1
		;;
	*)
		if echo `/bin/cat $DB/choices` 2> /dev/null |
			xargs /bin/ls -d 2> /dev/null |
			fgrep $1 > /dev/null
		then
			exec /bin/cat $1
			# no return from an exec!
		fi
		echo "$0: illegal file name '$1'" 1>&2
		exit 1
		;;
esac

exit 0
