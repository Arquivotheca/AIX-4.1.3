#!/usr/bin/ksh
# @(#)05        1.4  src/bos/usr/lib/pios/piomkjetd.sh, cmdpios, bos41J, 9512A_all 3/16/95 15:42:50
#
#   COMPONENT_NAME: CMDPIOS
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#EXT
###################################################################
###################################################################
# cmd_to_exec for id = "ps_rmpq_CmdHdr_jetdirect"
# Parameters = $1: remove_queue ( = q:qd )
# Output     = (none)

function rmpq_jetdirect { 
	q_name=${1%%:*}; q_device=${1##*:}
        if [[ -z $q_device ]]
        then
                # if no devices, just remove the queue.
                # (first nuke the jobs so the queue can be removed.)
                /usr/bin/enq -P$q_name -X >/dev/null 2>&1
                sleep 2;  # Allow time for jobs to die.
                /usr/bin/rmque -q $q_name
                exit $?
        fi
	# Check for existing jobs on queue
	if [[ -n $(qchk -P$1 | awk 'NR == 3 { print $4 }; NR == 4 { print $2; exit 0 }') ]] 
	then 
		dspmsg -s 18 smit.cat 157  'Print queue cannot be removed until no jobs remain.\n'
		exit 1
	fi 

	# Remove the pseudo-device.
	# if dev is NULL or FALSE then there's 
	# no psuedo-device to remove
	dev=$(lsquedev -q $q_name -d $q_device | awk '{if ($1 == "file") { print $3 } }')
	if [ -n $dev ] && [ $dev != "FALSE" ]
	then
		# Only remove if this is last queue using device
		if [[ $(egrep -c "^[ 	]*file[ 	]*=[ 	]*${dev}[ 	]*$" /etc/qconfig) -eq 1 ]]
		then
			# Remove pseudo-device
			pseudo_dev_name=${dev##*/}
			/usr/lib/lpd/pio/etc/piomgpdev -p ${pseudo_dev_name%%#*} -R -t $attach_type
		fi 
	fi 

	rmquedev -q $q_name -d $q_device 

	rmvirprt -q $q_name -d $q_device

	# if there are no devices left on queue, remove the queue
	if [[ $(lsallqdev -q $q_name | wc -l) -eq 0 ]]
	then 
		rmque -q $q_name 
	fi 
}

###################################################################
###################################################################
###################################################################
# cmd_to_exec for id = "ps_mkbootp_sys"
# Output     = (none)

function mkpq_jetdirect { 

	bootptab=/etc/bootptab
	hwadd=""		# JetDirect Card's hardware address
	ipadd=""		# The ip address assigned to the printer
	gateway=""		# The gateway's ip address 
	submask=""		# The subnet mask 
	network=""		# The network interface
	cmdstring=""		# String used to pass along arguments

	# Capture the arguments we need here and pass all others along
	while getopts A:D:Q:P:a:c:d:e:g:h:m:n:p:q:r:s:t:v:w: var
	do
		case $var in
		h) host=$OPTARG
		   device=hp@$host ;;
		e) hwadd=$OPTARG ;;
		g) gateway=$OPTARG ;;
		m) submask=$OPTARG ;;
		n) network=$OPTARG ;;
		*) cmdstring="$cmdstring -$var $OPTARG" ;;
		esac
	done

	# Get the ip address using the hostname
	ipadd=`/usr/bin/host $host 2>/dev/null`
	if [ $? -ne 0 ]
	then
		/usr/bin/dspmsg -s 1 piosmit.cat 113 'Host name %s does not exist.  A valid host name is required!\n' $host
		exit 1
	fi
	ipadd=`print -r - $ipadd | /usr/bin/cut -f3 -d" "`
	ipadd=${ipadd%,}

	# First check to see if the pseudo device exists.  If not, create it.
	/usr/lib/lpd/pio/etc/piomgpdev -p $device -t $attach_type -D -a desc=$attach_type >/dev/null 2>&1
	if [ $? -ne 0 ]
	then
		/usr/lib/lpd/pio/etc/piomgpdev -p $device -t $attach_type -A -a desc=$attach_type 
	fi

	# If the hardware address has been given, create the bootptab entry
	if [[ -n "$hwadd" ]]
	then
		if [[ -w $bootptab ]]
		then
			# Check to see if there is already a bootptab entry
			if /usr/bin/fgrep $host: $bootptab >/dev/null
			then
			/usr/bin/dspmsg -s 1 piosmit.cat 107  'WARNING: There is already an entry for %1$s in %2$s!\n' $host $bootptab
			else
				print -r - $host:hn:ht=$network:ha=$hwadd:ip=$ipadd:sm=$submask:gw=$gateway: >> $bootptab
			fi
		else
			/usr/bin/dspmsg -s 1 piosmit.cat 108 '%s does not exist or does not have write permission!\n' $bootptab
		fi
	fi
			
	# Create the printer queue(s) 
	/usr/lib/lpd/pio/etc/piomkpq -A $attach_type -d $device $cmdstring -c "file = /var/spool/lpd/pio/@local/dev/$device#$attach_type" -c "header = never" -c "trailer = never" -c "access = both" -c "backend = /usr/lib/lpd/pio/etc/piojetd $host"
}

###################################################################

# Main program 
{ 
attach_type=hpJetDirect
fname=$1;
shift;
functions $fname >/dev/null || { 
		/usr/bin/dspmsg -s 1 piosmit.cat 106 'Usage: piomkjetd <function-name> [arg1] [arg2] ...'
		exit 1 
	}
$fname "$@"
}
