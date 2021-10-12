#!/usr/bin/ksh
# @(#)83    1.1  src/bos/usr/lib/pios/piochpq.sh, cmdpios, bos411, 9428A410j 7/23/93 16:41:15
#
# COMPONENT_NAME: (cmdpios) Printer Backend
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# This script is used by SMIT to change a print queue's characteristics.
# It accepts arguments in a form like the qprt command (eg. -K+) or the
# chvirprt command (eg. -a _K=+).  It  translates the flags passed in
# and generates the appropriate chvirprt command or chquedev command, depending
# on the attribute being changed.
#
# Usage:  piochpq -q QueueName -d QueueDeviceName [ qprtFlags ] [ chvirprtFlags ]
#
# The first instance of '-q' is considered to by the queue name.  The first instance
# of '-d' is considered to be the queue device name.
#
# Two special cases this command handles are for '-a sa=value' and '-a sf=value'.
# The 'sa' and 'sf' attributes result in the /etc/qconfig file being changed
# instead of the virtual printer.
#
# Also, the '-B' flag results in /etc/qconfig being changed.
#
#

# initialize variables
typeset chvirprt_args=""
typeset chquedev_args=""
typeset attr=""
typeset queue=""
typeset qdevice=""
typeset rc="0"
typeset rc2="0"

# parse the command line options
while getopts :a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z:0:1:2:3:4:5:6:7:8:9: option
do 
	case $option in
		a)	if [[ "$OPTARG" = "${OPTARG#*=}" ]]  # check for '=' in arg
			then	
				chvirprt_args="_a='$OPTARG'"
			else
				attr=${OPTARG%%=*}
				value=${OPTARG#*=}
				if [[ "$attr" = "sa" ]]    # align
				then
					chquedev_args="$chquedev_args -a 'align = $value'"
				elif [[ "$attr" = "sf" ]]   # feed
				then
					chquedev_args="$chquedev_args -a 'feed = $value'"
				else  # normal attribute
					chvirprt_args="$chvirprt_args '$OPTARG'"
				fi
			fi
		;;

		d)	if [[ -n $qdevice ]]
			then
				chvirprt_args="$chvirprt_args _d='$OPTARG'"
			else
				qdevice=$OPTARG
			fi
		;;

		q)	if [[ -n $queue ]]
			then
				chvirprt_args="$chvirprt_args _q='$OPTARG'"
			else
				queue=$OPTARG
			fi
		;;

		B)	if [[ "$OPTARG" = "nn" ]]
			then
				chquedev_args=" -a 'header = never' -a 'trailer = never'"
		  	elif [[ "$OPTARG" = "gn" ]]
			then
				chquedev_args=" -a 'header = group' -a 'trailer = never'"
		  	elif [[ "$OPTARG" = "an" ]]
			then
				chquedev_args=" -a 'header = always' -a 'trailer = never'"
		  	elif [[ "$OPTARG" = "ng" ]]
			then
				chquedev_args=" -a 'header = never' -a 'trailer = group'"
		  	elif [[ "$OPTARG" = "gg" ]]
			then
				chquedev_args=" -a 'header = group' -a 'trailer = group'"
		  	elif [[ "$OPTARG" = "ag" ]]
			then
				chquedev_args=" -a 'header = always' -a 'trailer = group'"
		  	elif [[ "$OPTARG" = "na" ]]
			then
				chquedev_args=" -a 'header = never' -a 'trailer = always'"
		  	elif [[ "$OPTARG" = "ga" ]]
			then
				chquedev_args=" -a 'header = group' -a 'trailer = always'"
		  	elif [[ "$OPTARG" = "aa" ]]
			then
				chquedev_args=" -a 'header = always' -a 'trailer = always'"
			fi
		;;

		:)	dspmsg piobe.cat -s 4 15 'piochpq: Missing argument.\n' >&2
		  	dspmsg piobe.cat -s 4 14 'Usage:\tpiochpq -q Queue -d QueueDevice [-a AttributeName=Value | -Flag 'Value']...\n' >&2
			exit 1
		;;

		\?)	dspmsg piobe.cat -s 4 16 'piochpq: Illegal flag. \n' >&2
		  	dspmsg piobe.cat -s 4 14 'Usage:\tpiochpq -q Queue -d QueueDevice [-a AttributeName=Value | -Flag 'Value']...\n' >&2
			exit 1
		;;

		*)	chvirprt_args="$chvirprt_args _$option='$OPTARG'"
		;;

	esac
done

if [[ -n $chvirprt_args ]]
then
	eval /usr/sbin/chvirprt -q "$queue" -d "$qdevice" -a "$chvirprt_args"
	rc=$?
fi

if [[ -n "$chquedev_args" ]]
then
	eval /usr/bin/chquedev -q "$queue" -d "$qdevice" "$chquedev_args"
	rc2=$?
fi

# exit with '2' if chvirprt or chquedev failed
if [[ $rc != 0 || $rc2 != 0 ]]
then
   exit 2
fi

