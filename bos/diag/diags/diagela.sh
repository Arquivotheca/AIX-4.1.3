#!/bin/ksh
# @(#)34	1.18  src/bos/diag/diags/diagela.sh, diagsup, bos41B, 9504A 12/20/94 08:56:14
#
# COMPONENT_NAME: CMDDIAG  DIAGNOSTIC ERROR LOG ANALYSIS
#
# FUNCTIONS: Invoke individual device Error Log Analysis when a Hardware
#		error is placed in the System Error Log.
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                   


###################################################################
# diagela: script executed by error notification daemon
#
# usage() function provides command syntax to the user

usage()
{
	echo "$cmd {ENABLE | DISABLE [-t <device> [ <device> ]]}"
	exit 0
}

# Mail a message to root reporting the failure discovered after running
# Error Log Analysis.

mail_msg()
{

# See if the system has any customer installed notification program.
# Any PDiagAtt stanza in the following form, will indicate that the
# notification program will be used instead of the mail message:
# PDiagAtt:
#       DType = ""
#       DSClass = ""
#       attribute = "diag_notify"
#       value = "/full_path_name/my_notification_program"
#       rep = "s"
#
# For example, if customer has a program called
# /usr/lpp/process_all_errors then the stanza will be:
# PDiagAtt:
#	DType = ""
#	DSClass = ""
#	attribute = "diag_notify"
#	rep = "s"

        if [ $1 ]
        then
  	    diag_notify=`odmget -q"attribute=diag_notify and DType=$TYPE and \
		DSClass=$DSCLASS and DClass=$DCLASS" PDiagAtt`
	    if [ "diag_notify$diag_notify" = diag_notify ]
	    then
		diag_notify=`odmget -q"attribute=diag_notify" PDiagAtt`
	    fi
	    if [ "diag_notify$diag_notify" != diag_notify ]
	    then
		notify_program=`odmget -q"attribute=diag_notify" PDiagAtt | \
			grep "value" | cut -f2 -d"\""`
		prog=`echo $notify_program | cut -f1 -d" "`
		if [ -x $prog ]
		then
			eval $notify_program
			return
		fi
	    fi
	fi

# Mail a message to root reporting the failure discovered after running
# Error Log Analysis.

	/usr/sbin/lsgroup -c -a users system | /usr/bin/grep system: \
		| /usr/bin/sed "s/system://" | \
		/usr/bin/awk -F, '{cnt=1; {while (cnt<=NF) \
		{print "mail -s diagela",$cnt, \
		" < /tmp/diagela.msg"; cnt++;}}}' > /tmp/diagela.mail
	chmod +x /tmp/diagela.mail
	/tmp/diagela.mail 1>/dev/null 2>&1
	rm /tmp/diagela.mail 1>/dev/null 2>&1

}

# Check to see if we have already notified root about the problem within 
# a 24 hour period.

check_notification()
{

	OK_TO_NOTIFY=0
	current_dir=`pwd`
	cd $DIAGDATADIR
	if [ -f $1 ]
	then
	# File exists, see if it's been changed/created within the last 
	# 24 hour.
		X=`find $1 ! -ctime 1 -print`
		if [ "X$X" = X ]
		then
		# The file is not older than 24hour, i.e, we have notified root
			OK_TO_NOTIFY=0
		else
			OK_TO_NOTIFY=1
			rm -f $1
			>$1
		fi
	else
	# File does not exist, must be the first time this resource has
	# a problem. Create the file and notify root.
		>$1
		OK_TO_NOTIFY=1
	fi
	cd $current_dir
	return $OK_TO_NOTIFY

}

# Check to see if an srn is reported in the diagnostic report file
# to make sure the the return code from diag is not caused by
# things like no paging space. 1 is returned if an srn is found,
# otherwise 0 is returned.

check_srn()
{
	led=`odmget -q"type=$TYPE and subclass=$DSCLASS and class=$DCLASS" \
		PdDv | grep -e "led" | awk '{ printf "%x", $3}'`
	if [ "$led" != "870" ]
	then
		SRN=`cat /tmp/diagela.msg | grep -e "$led-" | \
			cut -f1 -d":"`
	else
		SRN=`cat /tmp/diagela.msg | \
		       grep -e "[0-9,A-F][0-9,A-F][0-9,A-F][0-9,A-F]:" | \
		      cut -f1 -d":"`
        fi
	if [ "SRN$SRN" = SRN ]
	then
		return 0 # no srn found
	else
		return 1 # found srn
	fi

}

# Check to see if the device is supported by diagnostics and if
# it is ok to run the test.

ok_to_run()
{

	# first see if the device is missing. If it is don't run
	missing=`odmget -q"name=$1 and chgstatus=3" CuDv`
	if [ "missing$missing" != missing ]
	then
		return 0
	fi
	# find PdDvLn field from CuDv
	UNIQUETYPE=`odmget -q"name=$1" CuDv | grep PdDvLn | cut -f2 -d"\""`
	# find type from PdDv
	TYPE=`echo $UNIQUETYPE | cut -f3 -d"/"`
	DSCLASS=`echo $UNIQUETYPE | cut -f2 -d"/"`
	DCLASS=`echo $UNIQUETYPE | cut -f1 -d"/"`
      	if [ "TYPE$TYPE" != TYPE -a "DSCLASS$DSCLASS" != DSCLASS \
			-a "DCLASS$DCLASS" != DCLASS ]
   	then
	# find DaName from PDiagDev
	    DANAME=`odmget -q"DType=$TYPE and DSClass=$DSCLASS and \
			DClass=$DCLASS" PDiagDev | grep -v EnclDaName | \
			grep DaName | cut -f2 -d"\""`
	    # Resource is supported by diags. If the name is not null
	    # check to make sure the DA program is present.
	    if [ "DANAME$DANAME" != DANAME ]
	    then
		# see if da name starts with '*'
		asterisk=`echo $DANAME | cut -c 1`
		if [ "$asterisk" = "*" ]
		then
			return 1
		fi
		if [ -x $DADIR/$DANAME ]
		then
			return 1
		fi
	    fi
	fi
	return 0
}

# Execute the Diagnostic Supervisor with the
# resource name and the no console flag

do_test()
{
	# Message and set number from udiagmon.cat
	set_num=4
	msg1=1
	msg2=2
	$DIAGBINDIR/dctrl -c -d $1 1>/dev/null 2>&1
	rc=$?
   	if [ $rc -eq 1 ]
    	then
	        $DIAGBINDIR/diagrpt -o > /tmp/diagela.msg
		check_srn
		if [ $? -eq 1 ]
		then
			console_msg=`dspmsg -s $set_num udiagmon.cat $msg1 \
'Periodic Diagnostics has detected error(s) that may require your attention.'`
			cons=`/usr/sbin/lscons`
			if [ "$cons" != "NULL" ]
			then
				echo "`date` $1 :  $console_msg " \
					1>/dev/console 2>/dev/null
			fi
			mail_msg diag_notify $1 $SRN $TYPE
		fi
	else
		if [ $rc -eq 4 ]
		then
		# Notify user that the test cannot be performed.

			notest_msg=`dspmsg -s $set_num udiagmon.cat $msg2 \
			"Unable to test resource $1 because another diagnostic session was in progress." "$1"`
			echo $notest_msg > /tmp/diagela.msg
			mail_msg
		fi
	fi
}

# Execute the controller to do Error Log Analysis

do_ela()
{

	# Message and set number in dcda.cat. Currently using message
	# from SCSI disk 
	elaset_num=32
	elamsg_num=21
	
	$DIAGBINDIR/dctrl -e -c -d $1 1>/dev/null 2>&1
	rc=$?
	if [ $rc -eq 1 ]
	then
       	    $DIAGBINDIR/diagrpt -o > /tmp/diagela.msg
	    check_srn
	    if [ $? -eq 1 ]
	    then
	    	check_notification $1
	    	if [ $? = 1 ]
		then
	            console_msg=`dspmsg -s $elaset_num dcda.cat $elamsg_num \
'Error Log Analysis has detected error(s) that may require your attention.'`
		    cons=`/usr/sbin/lscons`
		    if [ "$cons" != "NULL" ]
		    then
		   	 echo "`date` $1 :  $console_msg " \
				>/dev/console 2>/dev/null
		    fi
		    mail_msg diag_notify $1 $SRN $TYPE
		fi
	     fi
	 fi
}
#-------------- MAIN starts here ---------------------------

EXENV_STD=2
EXENV_CONC=4
DIAG_ENVIRONMENT=$EXENV_CONC
DIAG_IPL_SOURCE=0
DIAG_KEY_POS=0

# Check for /etc/.init.state file.  If present, get contents.
if test -s "/etc/.init.state"
then
	MODE=`cat /etc/.init.state`
	case $MODE in
		s|S)    DIAG_ENVIRONMENT=$EXENV_STD;;
		*)      DIAG_ENVIRONMENT=$EXENV_CONC;;
	esac
fi

# Now set up basic environment variables
DIAGNOSTICS=/usr/lpp/diagnostics
SHOWLEDS=0
PATH=$PATH:`pwd`
# Diagnostic Applications path
DADIR=$DIAGNOSTICS/da
DIAGX_SLIH_DIR=$DIAGNOSTICS/slih
DIAGBINDIR=$DIAGNOSTICS/bin

# Diagnostic data directory - diagnostic conclusion files, etc
DIAGDATA=/etc/lpp/diagnostics
DIAGDATADIR=$DIAGDATA/data

# Search path for the catalog files
if [ "X$LANG" = X -o "$LANG" = C ]; then
	LANG=en_US
fi
UNIQUETYPE=""
TYPE=""
DSCLASS=""
CLASS=""
DANAME=""
SRN=""
ODMDIR=/etc/objrepos
NLSPATH=/usr/lpp/diagnostics/catalog/default/%N:/usr/lib/nls/msg/%L/%N:$NLSPATH
export NLSPATH DADIR LANG ODMDIR SHOWLEDS DIAGX_SLIH_DIR DIAGDATADIR PATH
export DIAG_ENVIRONMENT DIAGNOSTICS DIAG_KEY_POS DIAGBINDIR DIAGDATA

cmd=`basename $0`
if [ "$1" = "" ]
then
	usage
fi

FLAGS=`echo $1 | /usr/bin/cut -c 1`
# Switch on the argument passed in to the script
if [ "$FLAGS" != "-" ]
then
	case	$1 in
		"ENABLE")
			# Add stanzas to errnotify object class
			# no-op if stanza already exist
			Y=`odmget -q"en_name=diagela" errnotify`
			if [ "Y$Y" = Y ]
			then
			# Enable error notification for all Permanent Hardware errors
			    echo 'errnotify:
				en_pid = 0
				en_name = "diagela"
				en_persistenceflg = 1
				en_label = ""
				en_crcid = 0
				en_class = "H"
				en_type = "PERM"
				en_alertflg = ""
				en_resource = ""
				en_rtype = ""
				en_rclass = ""
				en_method = "/usr/lpp/diagnostics/bin/diagela $6"
			    ' | odmadd 1>/dev/null 2>&1
			fi
			exit 0
			;;
		"DISABLE")
			# Remove stanzas from errnotify object class
			odmdelete -q"en_name=diagela" -o errnotify 1>/dev/null 2>&1
			exit 0
			;;
		*)
		    # if logging of symptom strings is desired,
		    # export the environment variable DIAG_REPORT
		    # and set it to 1, before calling do_ela.
		    ok_to_run $1
		    rc=$?
		    if [ $rc = 1 ]
		    then
			do_ela $1
		    fi
		    ;;
	esac

# -t flag is passed in

else
	cmdline="$*"
	set -- `getopt "t:" $*`
	if [ $? -ne 0 ]
	then
		echo "ERROR in command line '$cmdline':\n"
		usage
	fi
	while [ "$1" != "--" ]
	do
		case $1 in
			"-t")
				shift;
				resource_list="$*"
				for i in $resource_list
				do
					if [ $i != "--" ]
					then
						resources=$resources"$i "
					fi
				done
				shift;
				;;
			*)
				usage
				;;
		esac
	done
	# Now test each resource in the resources list.
	for device in $resources
	do
		ok_to_run $device
		rc=$?
		if [ $rc = 1 ]
		then
			do_test $device
		fi
	done
fi
rm /tmp/diagela.msg 1>/dev/null 2>&1
exit 0
