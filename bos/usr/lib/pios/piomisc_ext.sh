#!/usr/bin/ksh
# @(#)91        1.6  src/bos/usr/lib/pios/piomisc_ext.sh, cmdpios, bos411, 9428A410j 5/4/94 16:59:00
#
#   COMPONENT_NAME: CMDPIOS
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#EXT
###################################################################
###################################################################
# cmd_to_exec for id = "ps_rmpq_CmdHdr_local"
# Parameters = $1: remove_queue ( = q:qd )
# Output     = (none)

function rmpq_local { 
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
	if [[ -n $(/usr/bin/enq -eq -P$1 | /usr/bin/awk 'NR == 3 { print $4 }; NR == 4 { print $2; exit 0 }') ]] 
	then 
		/usr/bin/dspmsg -s 18 smit.cat 157  'Print queue cannot be removed until no jobs remain.\n'
		exit 1
	fi 
	/usr/bin/rmquedev -q $q_name -d $q_device 
	/usr/sbin/rmvirprt -q $q_name -d $q_device
	# if there are no devices left on queue, remove the queue
	if [[ $(/usr/bin/lsallqdev -q $q_name | /usr/bin/wc -l) -eq 0 ]]
	then 
		/usr/bin/rmque -q $q_name 
	fi 
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_rmpq_CmdHdr_local_last"
# Parameters = $1: remove_queue           ( = q:qd )
#              $2: device                 (Pathname to phys. device) 
#              $3: remove_physical_device (yes/no)
# Output     = (none)

function rmpq_local_last { 
	q_name=${1%%:*}; q_device=${1##*:}; device=${2##*/}
	if [[ -z $q_device ]]
	then
		# if no devices, just remove the queue.
		# (first nuke the jobs so the queue can be removed.)
		/usr/bin/enq -P$q_name -X >/dev/null 2>&1
		sleep 2;  # Allow time for jobs to die.
		/usr/bin/rmque -q $q_name
		exit $?
	fi
	if [[ -n $(/usr/bin/enq -eq -P$1 | /usr/bin/awk 'NR == 3 { print $4 }; NR == 4 { print $2; exit 0 }') ]] 
	then 
		/usr/bin/dspmsg -s 18 smit.cat 157  'Print queue cannot be removed until no jobs remain.\n'
		exit 1
	fi 
	/usr/bin/rmquedev -q $q_name -d $q_device 
	/usr/sbin/rmvirprt -q $q_name -d $q_device
	# if there are no devices left on queue, remove the queue
	if [[ $(/usr/bin/lsallqdev -q $q_name | /usr/bin/wc -l) -eq 0 ]]
	then 
		/usr/bin/rmque -q $q_name 
	fi 
	if [[ "$3" = "no" ]]
	then
		/usr/sbin/rmdev -l $device -d >/dev/null
	fi
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_rmpq_CmdHdr_other"
# Parameters = $1: remove_queue ( = q:qd )
#              $2: attach_type  (xstation | remote.nfs | *)
# Output     = (none)

function rmpq_other { 
	q_name=${1%%:*}; q_device=${1##*:}
	# Check for existing jobs on queue
	if [[ -n $(/usr/bin/enq -eq -P$1 | /usr/bin/awk 'NR == 3 { print $4 }; NR == 4 { print $2; exit 0 }') ]] 
	then 
		/usr/bin/dspmsg -s 18 smit.cat 157  'Print queue cannot be removed until no jobs remain.\n'
		exit 1
	fi 

	# If this is Xstation | remote.local attachment, 
	# remove the pseudo-device.
	if [[ "$2" = "xstation" ]]
	then
		pseudo_dev_type=$2
		# if dev is NULL or FALSE then there's 
		# no psuedo-device to remove
		dev=$(/usr/bin/lsquedev -q $q_name -d $q_device | /usr/bin/awk '{if ($1 == "file") { print $3 } }')
		[[ -n $dev ]] && [[ $dev != "FALSE" ]]
		if [[ $? -eq 0 ]]
		then
			# Only remove if this is last queue using device
			if [[ $(/usr/bin/egrep -c "^[ 	]*file[ 	]*=[ 	]*${dev}[ 	]*$" /etc/qconfig) -eq 1 ]]
			then
				# Remove pseudo-device
				pseudo_dev_name=${dev##*/}
				/usr/lib/lpd/pio/etc/piomgpdev -p ${pseudo_dev_name%%#*} -R -t $pseudo_dev_type
			fi 
		fi 
	fi 

	/usr/bin/rmquedev -q $q_name -d $q_device 

	# get hostname before removing virtual printer if this is NFS mounted
	[[ "$2" = "remote.nfs" ]] && hostname=$(print $(/usr/sbin/lsvirprt -q $q_name -d $q_device -f '%5$s' -a zH))
	/usr/sbin/rmvirprt -q $q_name -d $q_device

	# if there are no devices left on queue, remove the queue
	if [[ $(/usr/bin/lsallqdev -q $q_name | /usr/bin/wc -l) -eq 0 ]]
	then 
		# if NFS mounted, remove mount before removing queue
		if [[ "$2" = "remote.nfs" ]]
		then
			# Unmount if last reference to hostname
			# /usr/bin/grep all custom files for remote.nfs, then 
			#    /usr/bin/grep all those files for zH = hostname 
			list=$(/usr/bin/grep -l remote.nfs /var/spool/lpd/pio/@local/custom/*:* 2>/dev/null)
			if [[ -n $list && $(/usr/bin/egrep -c ":zH:[^:]*:$hostname[ 	]*$" $list) -eq 0 ]]
			then
				# If unmount fails, just leave it there
				/usr/sbin/rmnfsmnt -f /var/spool/lpd/pio/$hostname -B >/dev/null 2>&1 
				/usr/bin/rmdir /var/spool/lpd/pio/$hostname >/dev/null 2>&1 
			fi
		fi 
		/usr/bin/rmque -q $q_name 
	fi 
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_mkpq_attach_remoteCmdHdr"
# Parameters          = queue_name remote_host_name remote_queue_name 
#                       remote_host_type 
# Optional Parameters = remote_printer_description remote_printer_device 
#                       mount_remote_directory 
# Output              = (none)

function mkpq_remote_ext { 
	typeset		piodir=/var/spool/lpd/pio
	typeset		mount_flg=""
	typeset -i	is_mntd=0
	typeset -i	is_fsad=0
	typeset		mknfs_flgs=""

	while getopts "q:h:t:r:d:m:n:" var
	do
		case $var in
		q)	queue_name="$OPTARG";;
		h)	# All NFS queues mount a directory named after the 
			# fully qualified hostname.  If host fails, TCP/IP
			# may not be installed (use what user typed).
			mhost=$(LANG=C host $OPTARG 2>/dev/null || print $OPTARG)
			mhost=$(print "$mhost" | /usr/bin/cut -d' ' -f1)
			host="-a\"host = $mhost\""
			queue_device="@${mhost%%.*}";;
		t)	sfilter="-a\"s_statfilter = /usr/lib/lpd/${OPTARG}short\""
			lfilter="-a\"l_statfilter = /usr/lib/lpd/${OPTARG}long\""
			qtype=$OPTARG;;
		r)	remote_queue="-a\"rq = $OPTARG\"";;
		d)	descr="mL=\"$OPTARG\"";;
		m)	mount_flg="yes";
			qtype=aix;;
		n)	rem_prt_device="zE=$OPTARG";; 
		esac
	done
	
	new_mount=""
	if [[ -n $mount_flg ]]
	then
		# Only mount if not already mounted; only add to
		# /etc/filesystems if not already added.
		/usr/sbin/mount | /usr/bin/egrep -q "$mhost[ 	]+$piodir/@local[ 	]+" &&
		   let is_mntd=1
		/usr/sbin/lsnfsmnt | /usr/bin/egrep -q "$piodir/@local[ 	]+$mhost[ 	]+" &&
		   let is_fsad=1
		if (( is_mntd == 0 && is_fsad == 0 ))
		then
		   mknfs_flgs="-B"
		elif (( is_mntd == 1 ))
		then
		   mknfs_flgs="-I"
		elif (( is_fsad == 1 ))
		then
		   mknfs_flgs="-N"
		fi
		if (( is_mntd == 0 || is_fsad == 0 ))
		then
			new_mount=yes		
			[[ -d $piodir/$mhost ]] || mkdir $piodir/$mhost
			/usr/sbin/mknfsmnt -f $piodir/$mhost -d $piodir/@local -h $mhost -t ro $mknfs_flgs -A -w fg 
			if [[ $? -eq 0 ]]
			then
				/usr/bin/dspmsg -s 18 smit.cat 160 'Server attribute directory mounted.\n'
			else 
				/usr/sbin/rmnfsmnt -f $piodir/$mhost -I
				/usr/bin/rmdir $piodir/$mhost
				new_mount=""
				/usr/bin/dspmsg -s 1 piosmit.cat 104 'mknfsmnt failed.\nUnable to create queue.'
				exit 1
			fi
		fi
	fi

	eval /usr/bin/mkque -q $queue_name -a\"up = TRUE\" $host $sfilter $lfilter $remote_queue || 
		{
		[[ "$new_mount" = "yes" ]] && { /usr/sbin/rmnfsmnt -f $piodir/$mhost -B; /usr/bin/rmdir $piodir/$mhost; }	
		/usr/bin/dspmsg -s 18 smit.cat 161 'mkque failed.\n'
		exit 1
		}

	/usr/bin/mkquedev -q $queue_name -d $queue_device -a"backend = /usr/lib/lpd/rembak" ||
		{
		[[ "$new_mount" = "yes" ]] && { /usr/sbin/rmnfsmnt -f $piodir/$mhost -B; /usr/bin/rmdir $piodir/$mhost; }	
		/usr/bin/rmque -q $queue_name 2>&1 >/dev/null
		/usr/bin/dspmsg -s 18 smit.cat 162 'mkquedev failed.\n'
		exit 1
		}

	/usr/sbin/mkvirprt -q $queue_name -d $queue_device -n $queue_device -t remote -s remote -A remote ||
		{
		[[ "$new_mount" = "yes" ]] && { /usr/sbin/rmnfsmnt -f $piodir/$mhost -B; /usr/bin/rmdir $piodir/$mhost; }	
		/usr/bin/rmquedev -q $queue_name -d $queue_device 2>&1 >/dev/null
		/usr/bin/rmque -q $queue_name 2>&1 >/dev/null
		/usr/bin/dspmsg -s 18 smit.cat 163 'mkvirprt failed.\n'
		exit 1
		}

	case $qtype in
	aix)	if [[ -n $mount_flg ]]
		then
			eval /usr/sbin/chvirprt -q $queue_name -d $queue_device -a zH=$mhost $rem_prt_device zA=remote.nfs $descr
		else
			eval /usr/sbin/chvirprt -q $queue_name -d $queue_device -a zH=$mhost $descr
		fi
	;;
	bsd | att | aixv2 | *) eval /usr/sbin/chvirprt -q $queue_name -d $queue_device -a zH=$mhost $descr
	;;
	esac
	if [[ $? -ne 0 ]] 
	then
		/usr/sbin/rmvirprt -q $queue_name -d $queue_device 2>&1 >/dev/null
		/usr/bin/rmquedev -q $queue_name -d $queue_device 2>&1 >/dev/null
		/usr/bin/rmque -q $queue_name 2>&1 >/dev/null
		/usr/bin/dspmsg -s 18 smit.cat 172 'chvirprt failed.\n'
		exit 1
	fi
	/usr/bin/dspmsg -s 18 smit.cat 164 "Added print queue '%s'.\n" $queue_name
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_mkpq_remote_localCmdHdrOpt"
# Parameters          = piomkpq parameters + TYPE of remote server and 
#                       DESCRIPTION of remote printer
# Output              = (none)

function mkpq_remote_local { 
	pioetc=/usr/lib/lpd/pio/etc
	queue_names=""
	while getopts ":p:D:q:b:c:1:2:3:4:5:" option
	do
		case $option in
		1)   mhost=$(LANG=C host $OPTARG 2>/dev/null || print $OPTARG)
		     mhost=$(print "$mhost" | /usr/bin/cut -d' ' -f1)
		     parms="$parms -b \"host = $mhost\"" 
		     queue_device="@${mhost%%.*}"; 
		     host="$mhost";;
		2)   parms="$parms -b\"s_statfilter = /usr/lib/lpd/${OPTARG}short\" -b\"l_statfilter = /usr/lib/lpd/${OPTARG}long\"";
		     filter_name="/usr/lib/lpd/${OPTARG}short";;
		3)   descr="mL=\"$OPTARG\"";;
		4)   if [[ $OPTARG = "yes" ]]
		     then
		     	format_on_server="!"
		     else
		     	format_on_server="+"
		     fi
		     ;;
		5)   parms="$parms -b \"rq = $OPTARG\"";
		     remote_queue=$OPTARG;;
		p | D)   parms="$parms -$option $OPTARG";;
		q)   parms="$parms -$option $OPTARG"; 
		     queue_names="$queue_names $OPTARG";;
		c | *)   parms="$parms -$option $(print \"$OPTARG\" | /usr/bin/sed -e 's/=/ = /')";;
		esac
	done

	parms="$parms -c \"header = never\" -c \"trailer = never\" -c \"access = both\""
	backend="$pioetc/piorlfb -f $format_on_server"

	eval $pioetc/piomkpq -A remote.local "$parms" -d $queue_device -c "\"backend = $backend"\" >/dev/null || exit 1

	for qnm in $queue_names
	do
	   if [[ -n $descr ]] 
	   then
		eval /usr/sbin/chvirprt -q $qnm -d $queue_device -a $descr
		if [[ $? -ne 0 ]]
		then
		   /usr/bin/dspmsg -s 18 smit.cat 172 'chvirprt failed.\n'
		   for rmqnm in $queue_names
		   do
			/usr/sbin/rmvirprt -q $rmqnm -d $pdevname 2>&1 >/dev/null
			/usr/bin/rmquedev -q $rmqnm -d $pdevname 2>&1 >/dev/null
			/usr/bin/rmque -q $rmqnm 2>&1 >/dev/null
		   done
		   exit 1
		fi
	   fi
	done
	for qnm in $queue_names
	do
	   /usr/bin/dspmsg -s 18 smit.cat 164 "Added print queue '%s'.\n" $qnm
	done
}

###################################################################
###################################################################

# Main program 
{ 
fname=$1;
shift;
functions $fname >/dev/null || { 
		/usr/bin/dspmsg -s 1 piosmit.cat 85 'Usage: piomisc_ext <function-name> [arg1] [arg2] ...'
		exit 1 
	}
$fname "$@"
}

