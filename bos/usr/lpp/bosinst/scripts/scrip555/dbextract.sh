#!/bin/sh
# @(#)54	1.5  src/bos/usr/lpp/bosinst/scripts/scrip555/dbextract.sh, bosinst, bos411, 9428A410j 4/30/93 10:53:43
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
. `dirname $0`/defaults

leader="\t"
patterns_only=no
if test X"$1" = X-p
then
	patterns_only=yes
	leader=""
	shift
fi

if test $# -ne 1
then
	echo "usage: $0 [-p] IPaddr" 1>&2
	exit 1
fi

descrfile=$DB/$1
if test ! -f $descrfile
then
	exit 1
fi

if test $patterns_only = no
then
	echo '
if test $# -ne 1
then
	echo "usage: $0 recname" 1>&2
	exit 1
fi
case $1 in
	'
fi

/lib/cpp -P -I`dirname $0`/../db < $descrfile |

sed '
	$a\
eod
	/^[ 	]*$/d
' |

while read in_recname in_rectype in_recvalue
do
	if test X"$in_recname" = Xeod
	then
		echo "$leader\t$endofpattern"
		break
	fi

	if test X"$endofpattern" = X -a X"$in_recname" = X-
	then
		echo "echo $0: first recordname is -, should be explicit" 1>&2
		exit 1
	fi

	if test X"$in_recname" != X-
	then
		echo "$leader\t$endofpattern"
		echo "$leader$in_recname)"
		endofpattern=";;"
	fi

	recvalue="$in_recvalue"

	case $in_rectype in
	value)
		etc=
		;;
	filename)
		if test -f $recvalue
		then
			etc="file `ls -Ll $recvalue | awk '{print $5}'`"
		fi
		;;
	script)
		etc=
		recvalue="`eval $in_recvalue`"
		;;
	runtime)
		etc=
		serverIP=`hostname`
		serverIP=`host $serverIP | awk '{print $3}'`
		recvalue='`ninst '$serverIP' '"$recvalue"'`'
		;;
	*)
		echo "echo $0: unknown rectype: '$in_rectype'" 1>&2
		exit 1
		;;
	esac

	if test X"$recvalue" = X -a X"$etc" = X
	then
		echo "$leader\t:"
	else
		echo "$leader\techo $recvalue $etc"
	fi
done

if test $patterns_only = no
then
	echo "$leader*)
$leader\t:
$leader\t;;
esac"

fi
