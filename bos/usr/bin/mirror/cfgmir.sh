# @(#)81        1.1  src/bos/usr/bin/mirror/cfgmir.sh, cmdmirror, bos412, 9445C412a 11/4/94 09:24:56
#
# COMPONENT_NAME:  CMDMIRROR: Console mirroring
#
# NAME:        cfgmir.sh
#
# FUNCTIONS: Active or desactive the mirroring during the boot in "service" mode 
#
# SYNTAX:        cfgmir ON|OFF
#
# CALLED BY:     srvboot
#
# ORIGINS: 83
#
#  LEVEL 1, 5 Years Bull Confidential Information
#

CFG_FILE=/tmp/mir_old_cfg	# file name to save variables values

case $1 in 
  ON)
	# save state and configure device attach to port s2 
	PARENT_S2=`odmget -q "connwhere=s2a AND PdDvLn LIKE adapter/sio/*" CuDv|grep name|cut -f 2 -d\"`
	if [ "$PARENT_S2" = "" ] 
	then
		exit
	fi
	PARENT_S2_STATUS=`lsdev -C -l $PARENT_S2 -F status`
	if [ "$PARENT_S2_STATUS" = "Defined" ] 
	then
		mkdev -l $PARENT_S2 >/dev/null
	fi
	TTY_S2=`odmget -q "parent=$PARENT_S2" CuDv|grep name|cut -f 2 -d\"`
	if [ "$TTY_S2" = "" ]
	then
		mkdev -c tty -s rs232 -t tty -p $PARENT_S2 -w s2 >/dev/null
		TTY_S2=`odmget -q "parent=$PARENT_S2" CuDv|grep name|cut -f 2 -d\"`
		TTY_S2_STATUS=""
	else
		TTY_S2_STATUS=`lsdev -C -l $TTY_S2 -F status`
		if [ "$TTY_S2_STATUS" = "Defined" ]
		then
			mkdev -l $TTY_S2 >/dev/null
		fi
	fi

	# start the mirror daemon if it exists
	[ -x /usr/sbin/mirrord ] && /usr/sbin/mirrord mir_modem

	# save variables for mirroring OFF 
	echo >$CFG_FILE		# reset the CFG_FILE
	echo "PARENT_S2 $PARENT_S2" >>$CFG_FILE
	echo "TTY_S2 $TTY_S2" >>$CFG_FILE
	echo "PARENT_S2_STATUS $PARENT_S2_STATUS" >>$CFG_FILE
	echo "TTY_S2_STATUS $TTY_S2_STATUS" >>$CFG_FILE

	;;
  OFF)
	# kill the mirror daemon 
	MIR_PID=`ps -ef | grep mirrord | grep -v grep | cut -c10-16`
	if [ "$MIR_PID" != "" ] 
	then
		kill -15 $MIR_PID

		# wait (with timeout) the end of mirrord
		for i in 1 2 3 4 5 6
		do
			if ps -ef | grep mirrord | grep -v grep >/dev/null
			then
				sleep 1
			else
				break;
			fi
		done
	fi

	# restore variables values 
	if [ ! -r $CFG_FILE ]
	then
		exit
	fi
	PARENT_S2=` awk '/PARENT_S2 / { print $2 }' $CFG_FILE `
	TTY_S2=` awk '/TTY_S2 / { print $2 }' $CFG_FILE `
	PARENT_S2_STATUS=` awk '/PARENT_S2_STATUS/ { print $2 }' $CFG_FILE `
	TTY_S2_STATUS=` awk '/TTY_S2_STATUS/ { print $2 }' $CFG_FILE `
	rm -f $CFG_FILE

	# restore TTY_s2 status before the mirroring

	if [ "$TTY_S2" != "" ]
	then
		if [ "$TTY_S2_STATUS" = "" ] 
		then
			rmdev -l $TTY_S2 -d >/dev/null
		else
			if [ "$TTY_S2_STATUS" = "Defined" ]
			then
				rmdev -l $TTY_S2 >/dev/null
			fi
		fi
	fi
	
	# restore PARENT_S2 status before the mirroring
	if [ "$PARENT_S2" != "" ]
	then
		if [ "$PARENT_S2_STATUS" = "Defined" ]
		then
			rmdev -l $PARENT_S2 >/dev/null
		fi
	fi
	;;
esac
