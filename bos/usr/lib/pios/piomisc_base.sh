#!/usr/bin/ksh
# @(#)90	1.12  src/bos/usr/lib/pios/piomisc_base.sh, cmdpios, bos411, 9438C411a 9/22/94 17:13:53
#
#   COMPONENT_NAME: CMDPIOS
#
#   FUNCTIONS: none
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


###################################################################
#####
# Informal function naming convention:
#    two_dev_*  :  Determine if queue has more than one queue device
#    *_base*    :  Determine if being run on a base vs. extended system
#    is_ext_*   :  Determine if being run on a base vs. extended system
#    *_attach*  :  Determine attachment type
#####
###################################################################

#BASE
###################################################################
###################################################################
# cmd_to_classify for id = "qprt"
# Parameters = queue_name from user (typed or from list)
# Output     = cookedname:queue_name:queue_device:attach_type
#              :input_queue:ps_error_msg

function is_ext_attach_qprt { 
	one=${1##*( |	)}
	one=${one%%*( |	)}
	# If user hit enter without entering queue
	# then use default queue for printing.
	if [[ -z $one ]] 
	then
		one=${LPDEST:-$PRINTER}
		one=${one:-$(/usr/bin/lsallq | /usr/bin/head -1)}
	fi
	q_name=${one%%:*}
	# Keep q_dev == NULL if no colon in user-input
	[[ ${one##*:} != $q_name ]] && q_dev=${one##*:}
	iqueue=$(print $one | /usr/bin/sed -e 's/:/#!:/')
	/usr/bin/enq -P $one -Y 2>/dev/null
	if [[ $? -ne 0 ]]
	then
		print ps_error_ghostName:$q_name:$q_dev::$iqueue:$(/usr/bin/dspmsg -s 18 smit.cat 156 'Print queue %s does not exist.\n' $iqueue)
		exit 0
	fi
	if (( $is_ext ))
	then
		/usr/lib/lpd/pio/etc/piodmgrsu >/dev/null 2>&1
		next=$(/usr/lib/lpd/pio/etc/piolsvp -P $one -n submit_job 2>/dev/null)
		if [[ $? -ne 0 ]]
		then
			print ps_qprt_genericName:$q_name:$q_dev::$iqueue
		else
			q_dev=$(print $next | /usr/bin/cut -f1 -d' ')
			attach_type=$(print $next | /usr/bin/cut -f2 -d' ')
			next=$(print $next | /usr/bin/cut -f3 -d' ')
			if [[ -z "$next" ]]
			then
				print ps_qprt_genericName:$q_name:$q_dev:$attach_type:$iqueue
			else
				print $next:$q_name:$q_dev:$attach_type:$iqueue
			fi
		fi
	else 
		print ps_qprt_genericName:$q_name:$q_dev::$iqueue
	fi  
}

###################################################################
###################################################################
# cmd_to_classify for id = "chpq"
# Parameters  =  queue_name from user (typed or from list)
# Output      =  cookedname:queue_name:queue_device:ps_error_msg

function two_dev_base_chpq { 
	typeset -i num_dev
	/usr/bin/enq -P $1 -Y 2>/dev/null
	if [[ $? -ne 0 ]]
	then
		print ps_error_ghostName:::$(/usr/bin/dspmsg -s 18 smit.cat 156  'Print queue %s does not exist.\n' $(print $1 | /usr/bin/sed -e 's/:/#!:/'))
		exit 0
	fi
	if (( $is_ext ))
	then 
		/usr/lib/lpd/pio/etc/piodmgrsu >/dev/null 2>&1
		[[ $1 != ${1%%:*} ]] && { print ps_chpq_ghostName:${1%%:*}:${1##*:}:
				exit 0; }
		allqdev=$(/usr/bin/lsallqdev -c -q$1)
		num_dev=${allqdev:+$(print "$allqdev" | /usr/bin/wc -l)}
		case ${num_dev:-0} in
		0)	print ps_chpq_genericName:$1::
		;;
		1)	print ps_chpq_ghostName:$allqdev:
		;;
		*)	print ps_chpq_printerName:$1::
		;;
		esac
	else 
		print ps_chpq_genericName:$1:
	fi  
}

###################################################################
###################################################################
# cmd_to_classify for ids that branch to *printerName or *ghostName
# depending on the number of devices on a given queue.
# (eg: qstart, qstop, rmpq, ...)
# Parameters = $1: object_name 
#              $2: queue_name from user (typed or from list)
# Output     = cookedname:queue_name:queue_device:ps_error_msg
function two_devices { 
	typeset -i num_dev

	if (( $is_ext ))
	then 
		/usr/lib/lpd/pio/etc/piodmgrsu >/dev/null 2>&1
	fi

	# If queue is invalid, branch to error dialog
	/usr/bin/enq -P $2 -Y 2>/dev/null
	if [[ $? -ne 0 ]]
	then
		print ps_error_ghostName:::$(/usr/bin/dspmsg -s 18 smit.cat 156 'Print queue %s does not exist.\n' $(print $2 | /usr/bin/sed -e 's/:/#!:/'))
		exit 0
	fi

	# if user enters q:qd then use as fastpath around printerName
	[[ $2 != ${2%%:*} ]] && { print ps_${1}_ghostName:${2%%:*}:${2##*:}
	                exit 0; }

 	# Only go to printerName if there is more than one device
	allqdev=$(/usr/bin/lsallqdev -c -q$2)
	num_dev=${allqdev:+$(print "$allqdev" | /usr/bin/wc -l)}
	case ${num_dev:-0} in
	0)	if [[ $1 = "rmpq" ]]
		then
			print ps_${1}_ghostName:$2:
		else
			print ps_error_ghostName:::$(/usr/bin/dspmsg -s 18 smit.cat 159 'No queue device exists for queue %s.\n' $2)
		fi
			
	;;
	1)	print ps_${1}_ghostName:$allqdev:
	;;
	*)	print ps_${1}_printerName:$2::
	;;
	esac
}

###################################################################
###################################################################
# cmd_to_classify for id = "ps_rmpq_ghostName"
# Parameters = queue_name queue_device
# Output     = cookedname:attach_type:device

function is_ext_rmpq_ghostName { 
	if (( $is_ext ))
	then 
		# Check if virtual printer exists. 
		# If so, are fields properly defined?
		str=$(/usr/lib/lpd/pio/etc/piolsvp -P $1:$2 -n remove_queue 2>/dev/null)
		if [[ $? -ne 0 ]]
		then
			# No virtual printer, treat as generic
			print ps_rmpq_ghostName_generic:generic:
			exit 0
		fi
		next=$(print $str | /usr/bin/cut -f3 -d' ')
		if [[ -z "$next" ]]
		then
			# SMIT panel not identified for remove_queue.
			# Treat as generic.
			print ps_rmpq_ghostName_generic:generic:
			exit 0
		fi
		# Branch based on attachment type 
		attach=$(print $str | /usr/bin/cut -f2 -d' ')
		case $attach in
		"")  # Generic attachment 
		     print ps_rmpq_ghostName_generic:generic:
		;;
		local)  # Determine if last queue using device.
	                # device=physical device (printer - not queue device)
	                device=$(/usr/bin/lsquedev -q $1 -d $2 | /usr/bin/awk '{if ($1 == "file") {print $3; exit 0 } }')
			case "$device" in
			"" | FALSE) print $next:local:$device
			;;
			*)  case $(/usr/bin/egrep -c "^[ 	]*file[ 	]*=[ 	]*${device}[ 	]*$" /etc/qconfig) in
		            "" | 1)  # This is last device.
			             print $next'_last':local:$device
			    ;;
			    *)  print $next:local:$device
			    ;;
			    esac
			;;
			esac
		;;
		*)  print $next:$attach:
		;;
		esac
	else 
	   # Must be base SMIT
	   print ps_rmpq_ghostName_generic:generic:
	fi 
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_rmpq_CmdHdr_generic"
# Parameters = remove_queue ( = q:qd )
# Output     = (none)

function rmpq_generic { 
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
	if [[ -z $(/usr/bin/lsallqdev -q $q_name) ]] 
	then 
		/usr/bin/rmque -q $q_name 
	fi 
}

###################################################################
###################################################################
# cmd_to_exec for id = "mkitab_lpd"
# Parameters = $1: now | boot | both  (start when?)
#              $2: trace | no         (trace lpd to syslog?)
#              $3: yes | no           (extended option - export @local?)
# Output     = (none)

function server_start_when { 
	# The server directory is always exported BOTH.  This allows 
	# for continued operation after the server recovers from a crash. 
	case "$2" in
	trace)  /usr/bin/chssys -s lpd -a '-l' >/dev/null
	;;
	no)  /usr/bin/chssys -s lpd -a '' >/dev/null
	;;
	esac

	case $1 in
	now) /usr/bin/startsrc -s lpd 
	;;
	boot) /usr/sbin/mkitab 'lpd:2:once:/usr/bin/startsrc -s lpd'
	;;
	both) /usr/bin/startsrc -s lpd; /usr/sbin/mkitab 'lpd:2:once:/usr/bin/startsrc -s lpd'
	;;
	esac
	if [[ "$3" = "yes" && -x /usr/sbin/rmnfsexp && -d /var/spool/lpd/pio/@local ]]
	then
		/usr/sbin/rmnfsexp -d /var/spool/lpd/pio/@local -I >/dev/null
		/usr/sbin/mknfsexp -d /var/spool/lpd/pio/@local -t ro -B >/dev/null
	fi
}

###################################################################
###################################################################
# cmd_to_exec for id = "rmitab_lpd"
# Parameters = $1: now | boot | both  (stop when?)
# Output     = (none)

function server_stop_when { 
	# The server directory is unexported in sync with when the lpd is
	# stopped.  However, if stopping lpd now but not at system restart
	# then don't unexport (so that users can continue to see the SMIT
	# panels in the mounted server attribute directory).
	case $1 in 
	now)	/usr/bin/stopsrc -s lpd
	;;
	boot)	/usr/sbin/rmitab 'lpd'
		[[ -x /usr/sbin/rmnfsexp && -d /var/spool/lpd/pio/@local ]] &&
		/usr/sbin/rmnfsexp -d /var/spool/lpd/pio/@local -I >/dev/null
	;;
	both)	/usr/bin/stopsrc -s lpd
		/usr/sbin/rmitab 'lpd'
		[[ -x /usr/sbin/rmnfsexp && -d /var/spool/lpd/pio/@local ]] &&
		/usr/sbin/rmnfsexp -d /var/spool/lpd/pio/@local -B >/dev/null
	;;
	esac
}

###################################################################
###################################################################
# cmd_to_classify for id = "pq_chquedev"
# Parameters = queue_name from user (typed or from list)
# Output     = cookedname:queue_name:queue_device:ps_error_msg

function pq_chquedev_two_dev {
	typeset -i num_dev
	/usr/bin/enq -P $1 -Y 2>/dev/null
	if [[ $? -ne 0 ]]
	then
		print ps_error_ghostName:::$(/usr/bin/dspmsg -s 18 smit.cat 158 'Queue %s does not exist.\n' $(print $1 | /usr/bin/sed -e 's/:/#!:/'))
		exit 0
	fi

	# if user enters q:qd then use as fastpath around printerName
	[[ $1 != ${1%%:*} ]] && { print chquedevghostName:${1%%:*}:${1##*:}:
	                exit 0; }

	allqdev=$(/usr/bin/lsallqdev -c -q$1)
	num_dev=${allqdev:+$(print "$allqdev" | /usr/bin/wc -l)}
	case ${num_dev:-0} in
	0)	print ps_error_ghostName:::$(/usr/bin/dspmsg -s 18 smit.cat 159 'No queue device exists for queue %s.\n' $1)
	;;
	1)	print chquedevghostName:$allqdev:
	;;
	*)	print chquedevNameHdr:$1::
	;;
	esac
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_mkpq_attach_remoteCmdHdr"
# Parameters          = queue_name remote_host_name remote_queue_name 
#                       remote_host_type 
# Output              = (none)

function mkpq_remote_base { 
	while getopts q:h:t:r:d:m:n: var
	do
		case $var in
		q)	queue_name="$OPTARG";;
		h)	mhost=$(LANG=C /usr/bin/host $OPTARG | /usr/bin/cut -d' ' -f1)
			host="-a\"host = $mhost\""
			queue_device="@${mhost%%[\.]*}";;
		t)	sfilter="-a\"s_statfilter = /usr/lib/lpd/${OPTARG}short\""
			lfilter="-a\"l_statfilter = /usr/lib/lpd/${OPTARG}long\""
			qtype=$OPTARG;;
		r)	remote_queue="-a\"rq = $OPTARG\"";;
		esac
	done

	eval /usr/bin/mkque -q $queue_name -a\"up = TRUE\" $host $sfilter $lfilter $remote_queue || 
		{
		/usr/bin/dspmsg -s 18 smit.cat 161 'mkque failed.\n'
		exit 1
		}
	/usr/bin/mkquedev -q $queue_name -d $queue_device -a"backend = /usr/lib/lpd/rembak" ||
		{
		/usr/bin/rmque -q $queue_name 2>&1 >/dev/null
		/usr/bin/dspmsg -s 18 smit.cat 162 'mkquedev failed.\n'
		exit 1
		}
	/usr/bin/dspmsg -s 18 smit.cat 164 "Added print queue '%s'.\n" $queue_name
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_mkpq_attach_user-definedCmdHdr"
# Parameters          = queue_name queue_device backend 
# Optional Parameters = ... many ...
# Output              = (none)

function mkpq_other { 
	default_queue=""
	while getopts q:d:b:u:Di:a:h:r:s:l:f:c:e:t:g:z: var
	do
		case $var in
		q)	queue_name="$OPTARG";;
		d)	queue_device="$OPTARG";;
		b)	backend="-a\"backend = $OPTARG\"";;
		u)	activate="-a\"up = $OPTARG\"";;
		D)	default_queue="-D";;
		i)	discipline="-a\"discipline = $OPTARG\"";;
		a)	acctfile="-a\"acctfile = $OPTARG\"";;
		h)	host="-a\"host = $(LANG=C /usr/bin/host $OPTARG | /usr/bin/cut -d' ' -f1)\"";;
		r)	remote_queue="-a\"rq = $OPTARG\"";;
		s)	stype="-a\"s_statfilter = $OPTARG\"";;
		l)	ltype="-a\"l_statfilter = $OPTARG\"";;
		f)	file="-a\"file = $OPTARG\"";;
		c)	access="-a\"access = $OPTARG\"";;
		e)	header="-a\"header = $OPTARG\"";;
		t)	trailer="-a\"trailer = $OPTARG\"";;
		g)	align="-a\"align = $OPTARG\"";;
		z)	feed="-a\"feed = $OPTARG\"";;
		esac
	done

	eval /usr/bin/mkque -q $queue_name $activate $default_queue $discipline $acctfile $host $stype $ltype $remote_queue || 
		{
		/usr/bin/dspmsg -s 18 smit.cat 161  'mkque failed.\n'
		exit 1
		}
	/usr/bin/dspmsg -s 18 smit.cat 165 "Added queue '%s'.\n" $queue_name

	eval /usr/bin/mkquedev -q $queue_name -d $queue_device $file $backend $access $header $trailer $align $feed ||
		{
		/usr/bin/rmque -q $queue_name 2>&1 >/dev/null
		/usr/bin/dspmsg -s 18 smit.cat 162  'mkquedev failed.\n'
		exit 1
		}
	/usr/bin/dspmsg -s 18 smit.cat 166 "Added queue device '%s'.\n" $queue_device
}

###################################################################
###################################################################
# cmd_to_exec for id = "ps_mkprinterCmdHdr"
# Parameters          = queue_name queue_device backend 
# Optional Parameters = ... many ...
# Output              = (none)

function mkprinter { 
	while getopts q:d:b:f:a:h:t:l:e: var
	do
		case $var in
		q)	queue_name="$OPTARG";;
		d)	queue_device="$OPTARG";;
		b)	backend="-a\"backend = $OPTARG\"";;
		f)	file="-a\"file = $OPTARG\"";;
		a)	access="-a\"access = $OPTARG\"";;
		h)	header="-a\"header = $OPTARG\"";;
		t)	trailer="-a\"trailer = $OPTARG\"";;
		l)	align="-a\"align = $OPTARG\"";;
		e)	feed="-a\"feed = $OPTARG\"";;
		esac
	done

	eval /usr/bin/mkquedev -q $queue_name -d $queue_device $file $backend $access $header $trailer $align $feed ||
		{
		/usr/bin/dspmsg -s 18 smit.cat 162  'mkquedev failed.\n'
		exit 1
		}
	/usr/bin/dspmsg -s 18 smit.cat 166 "Added queue device '%s'.\n" $queue_device
}

###################################################################
###################################################################
# Display a list of all job numbers across all queues.
# Parameters: none
# Output:     queue  job#  file  owner

function list_job_number {
	/usr/bin/enq -Ae 2>/dev/null | /usr/bin/awk '
		$1 ~ /:$/ { next } # Skip error messages
		{ line = substr($0,1,8) substr($0,25,33) }
		NR == 1 || NR == 2 { printf("# %s\n",line); next; }
		{
			que=substr(line,1,7);
			if ( que != "       ") oldque = que
			if (substr(line,9,3) == "") next;
			printf("  %s %s\n",oldque,substr(line,9))
		}'
}

###################################################################
###################################################################
# Display a list of all job owners across all queues.
# Parameters: none
# Output:     queue  owner  job#  file

function list_job_owner {
    /usr/bin/enq -Ae 2>/dev/null | /usr/bin/awk '
	$1 ~ /:$/ { next } # Skip error messages
	{
	    line = substr($0,1,8) substr($0,25,33)
	    line = substr(line,1,8) substr(line,32,10) " " substr(line,9,23)
	}
	NR == 1 || NR == 2 { printf("# %s\n",line); next; }
	{
		que=substr(line,1,7);
		if ( que != "       ") oldque = que
		if (substr(line,20,3) == "") next;
		printf("  %s %s\n",oldque,substr(line,9))
	}'
}

###################################################################
###################################################################
# Display a list of all job numbers across all queues w/queue status.
# Parameters: -P [qname | *]    (* for all queues)
# Output:     qchk output listing only queues with jobs

function get_job_status {
	[[ $2 = "*" ]] && { flg="-A"; shift 2; } # -P must be first parameter
	qchk_out=$(/usr/bin/enq -eq $flg "$@"); rc=$?; 
	[[ $rc -ne 0 ]] && exit $rc
	print "$qchk_out" | /usr/bin/awk '
	NR <= 2 { past_hdr = 0; print; next }
	NR > 2 { 
		flg = 0; 
		qname = substr($0,1,7); qdev = substr($0,9,5);
		if (qname != "       ") { flg = 1; old_qname = qname; }
		if (qdev  !=   "     ") { flg = 1; old_qdev  = qdev;  }
		if (flg != 0) { 
			que[NR] = old_qname SUBSEP old_qdev;
			line[NR] = $0;
		} # endif
		if (substr($0,25,3) == "") next; # Dont print if no job number
		for (i in que) {
			# Print queue status for this job
			if (match(que[i],old_qname)) {
				print line[i];
			} 
			delete que[i];
		} # endfor
		if (!flg)
		   print $0;
		past_hdr = 1;
	} # end NR > 2
	END { if (past_hdr != 1) exit 3; }' # end /usr/bin/awk
	rc=$?
	if [[ $rc -eq 3 ]] then 
		/usr/bin/dspmsg -s 18 smit.cat 66 'No jobs found that meet the specified criteria.\n'
		rc=0
	fi
	exit $rc
}

###################################################################
###################################################################

# Main program 
{ 
fname=$1;
shift;
functions $fname >/dev/null || { 
		/usr/bin/dspmsg -s 18 smit.cat 167 'Usage: piomisc_base <function-name> [arg1] [arg2] ...'
		exit 1 
		}
[[ -x /usr/lib/lpd/pio/etc/piolsvp ]] && export is_ext=1
$fname "$@"
}

