#!/usr/bin/ksh
# @(#)04  1.8  src/bos/etc/rc.powerfail/rc.powerfail.sh, cmdoper, bos41J, 9512A_all 3/20/95 09:54:19
# For safety sync the system before processing any further lines.
/usr/sbin/sync
/usr/sbin/sync
/usr/sbin/sync
#
#
# COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
#
# FUNCTIONS: rc.powerfail 
#
# ORIGINS: 27, 83
#
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994.
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#################################################################################
#
#  This script was created to support DCR 51225.  The purpose of this
#  script is to cleanly shutdown systems released with the 750 Watt power
#  supply.  The purpose of this process in to handle hardware failure
#  conditions by shutting down the system as cleanly as possible.
#
#  Conditions and time allowances as specified by the Power Status :
#
#   Power Status Condition                      Action
#
#          0     Normal State                   no action
#          1     Running on battery backup      10-15 minutes battery
#          2     Programmed Power Off           immediate power off
#          3     Manual Off Button              immediate power off
#          5     Thermal High                   20 seconds before halt
#          6     Internal Fail                  immediate power off
#          7     Power supply overload          immediate power off
#          8     Loss of Primary Power          immediate power off
#          9     Primary power supply fan       20 seconds before halt
#         10     Fan Fail 2                     15 minutes to shutdown
#         11     Fan Fail 3                     15 minutes to shutdown
#         12     Fan Fail 4                     15 minutes to shutdown
#         13     Fan Fail 5                     15 minutes to shutdown
# 
#  This script will only deal with the faults which allow 20 seconds or more 
#  time until the system shutdown or soft shutdown.  This script will perform
#  the following general actions for each of the time periods allowed.
#
#       Time Period	Default Action
#       
#       immediate off	No action
#	20 seconds	sync the system
#	15 minutes	warn the user and halt the system at the end of 15 min.
#	Normal State	No action
#
#  It should be noted that if a 20 second error is received, there is no
#  recovery.  The system will power down in 20 seconds.
#
#  This script is called by default from init if the epow_handler detects an
#  error from the primary power supply.  Depending on the condition of this 
#  error, the epow_handler will send a SIGPWR to init, which will in turn 
#  call the command on the powerfail line of /etc/inittab.  The default command
#  is "sh /etc/rc.powerfail > /dev/console 2>&1"
#
#################################################################################
#
#  EXIT STATUS
#
#  0	The system is currently in a normal condition and we are not going down
#  1	usage problem
#  2    halt -q failed
#  3	shutdown -F failed
#  4    reached the end of rc.powerfail, which should NOT occur.
#  5	hit an undefined state
#
#################################################################################
# This script was modified for fan/power failure handling.
# (CMVC 153726).
# bits 10-13 of  PKSR are also checked using machstat -f
# These bits in the PKSR are new and are defined in a previously
# reserved part of the PKSR. Those bits will always be zero on any
# pre-existing systems.
# The behavior for these bits are the following:
#
#   Power Status 	Action
#
#          0     	fallback to standard behavior
#			interpret bits 0-9
#          1     	warning cooling problem
#          2     	warning power problem
#          3     	10 minutes to shutdown severe cooling problem
#          4     	20 seconds to shutdown very severe cooling problem
#

# The following is a work around for init.  This script/command uses Korn Shell
# specific expressions.  Currently, init call all commands with /bin/sh, which
# may or may not be linked to /bin/ksh.  If it is not, then these steps must be
# taken in order to ensure that rc.powerfail will work as expected.
if [ "$1" != "_!.!.!.!." ]
then
	exec rc.powerfail _!.!.!.!. $*
fi

shift

#
#  send_msg()
#
#  This function will wall a message to all attached terminals.  As a design
#  point, all the messages must have 3 bells and the type of error attached
#  to it.  As a safety percaution, I have added 4 to each message.
#

send_msg()
{
	case $1 in
	
		1) dspmsg -s 1 rc_powerfail.cat 1 \
'Usage: rc.powerfail [-s] [-h] [-t Minutes]\n'
		   break ;;

		3) 
		   wall "`dspmsg -s 1 rc_powerfail.cat 3 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tThe power status is currently %s.\n \
\tThe system may power down immediately.\n \
\tExecute rc.powerfail -h as the root user for more information.\n' \
$state` "
		   break ;;
	
		4)
		   wall "`dspmsg -s 1 rc_powerfail.cat 4 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tThe power status is currently %s.\n \
\tThis indicates that a fan fault has occurred.\n \
\tThe system will shut down in %s minutes unless the problem is\n \
\tresolved before then.  Execute rc.powerfail -h as the root user \n \
\tfor more information.\n' \
$state $time` "
		   break ;;
	
		5)
		   wall "`dspmsg -s 1 rc_powerfail.cat 5 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tThe power status is currently %s.\n \
\tThis indicates that the system is now running on battery backup.\n \
\tThe system will shut down in %s minutes unless the problem is \n \
\tresolved before then.  Execute rc.powerfail -h as the root user \n \
\tfor more information.\n' \
$state $time` "
		   break ;;
	
		6) 
		   wall "`dspmsg -s 1 rc_powerfail.cat 6 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tA call to machstat to validate the signal has failed.\n \
\tThe system will automatically shutdown NOW.\n \
\tExecute rc.powerfail -h as the root user for more information.\n' \
` "
		   break ;;
	
		7)
		   wall "`dspmsg -s 1 rc_powerfail.cat 7 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tA call to machstat to validate the signal has returned \n \
\tan undefined value of %s.  No action will be taken.\n \
\tExecute rc.powerfail -h as the root user for more information.\n' \
$state` "
		   break ;;
	
		8)
		   wall "`dspmsg -s 1 rc_powerfail.cat 8 \
'rc.powerfail: shutdown failed.\n \
\tYou should manually shutdown the system NOW with shutdown -F.\n' \
` "
		   break ;;
	
		9)
		   wall "`dspmsg -s 1 rc_powerfail.cat 9 \
'rc.powerfail: halt failed.\n \
\tYou should manually halt the system NOW with halt -q.\n' \
` "
		   break ;;
	
		10)
		   dspmsg -s 1 rc_powerfail.cat 10 \
'rc.powerfail: 0481-081 The time in minutes given with the -t option\n \
\twas invalid.  System shutdown will occur in 15 minutes.\n'
		   break ;;
	
		11)
		   wall "`dspmsg -s 1 rc_powerfail.cat 11 \
'rc.powerfail: The system will shut down in %s minutes.\n \
\tThe system will shut down because either a fan failure has occurred\n \
\tor the system is running on battery backup power.\n' \
$time ` "
		   break ;;
	
		12)
		   wall "`dspmsg -s 1 rc_powerfail.cat 12 \
'rc.powerfail: The power status has changed to %s.\n \
\tSystem shutdown will occur NOW.\n' \
$state` "
		   break ;;
	
		13)
		   wall "`dspmsg -s 1 rc_powerfail.cat 13 \
'rc.powerfail: The power status has changed to %s.\n \
\tThe system shutdown has been canceled.\n' \
$state` "
		   break ;;
	
		14)
		   wall "`dspmsg -s 1 rc_powerfail.cat 14 \
'rc.powerfail: The power status has changed to %s.\n \
\tThis indicates that another fan has failed or the system is\n \
\tnow running on battery backup.  \n \
\tThe system will shut down as scheduled.\n' \
$state` "
		   break ;;
	
		15)
		   wall "`dspmsg -s 1 rc_powerfail.cat 15 \
'rc.powerfail: The time until system shutdown has expired.\n \
\tThe system will be shutting down NOW.\n' \
` "
		   break ;;
	
		16)
		   wall "`dspmsg -s 1 rc_powerfail.cat 16 \
'rc.powerfail: The time until system shutdown has expired.\n \
\tYou should shut down the system NOW using shutdown -F.\n' \
` "
		   break ;;
	
		17)
		    dspmsg -s 1 rc_powerfail.cat 17 \
'rc.powerfail: \
This command is used to handle power losses or fan failures. \
There are several different states that the system can be in when \
the signal SIGPWR is received by init.  The action taken will be \
determined by the value of the power status.  The following table \
shows the values of the power status and action taken. \n \
Power \
Status\tIndication \
------\t---------------------------------------------------------------- \
  0\tSystem is running normally, there is no action taken. \
  1\tSystem is running on battery power, shutdown in 15 minutes. \
  2\tProgrammed power off, no action from this command. \
  3\tManual off button depressed, no action from this command. \
  4\tUndefined status, no action for this state. \
  5\tThermal high has been detected, system power off in 20 seconds. \
  6\tInternal power fail, immediate system power off. \
  7\tPower supply overload, immediate system power off. \
  8\tLoss of primary power without battery backup, \
\t\timmediate system power off. \
  9\tLoss of primary power fan, system power off in 20 seconds. \
 10\tLoss of secondary fan, system shutdown in 15 minutes. \
 11\tLoss of secondary fan, system shutdown in 15 minutes. \
 12\tLoss of secondary fan, system shutdown in 15 minutes. \
 13\tLoss of secondary fan, system shutdown in 15 minutes. \
 14\tUndefined status, no action for this state. \
 15\tUndefined status, no action for this state. \
255\tERROR with the machstat command, system shutdown starts immediately.\n' 
		   break ;;

		18)
		   wall "`dspmsg -s 1 rc_powerfail.cat 18 \
'rc.powerfail: An internal error has occurred in the rc.powerfail\n \
\tcommand.  If you have received messages that the system is shutting \n \
\tdown, you should execute shutdown -F immediately.\n \
\tYou should also call your Service Representative\n \
\tand report this problem.\n' \
` "
		   break ;;

		19)
		   wall "`dspmsg -s 1 rc_powerfail.cat 19 \
'rc.powerfail: An internal error has occurred in the machstat command.\n \
\tThe system will be shut down NOW.\n \
\tYou should also call your Service Representative\n \
\tand report this problem.\n' \
` "
		break ;;

		20)
		   wall "`dspmsg -s 2 rc_powerfail.cat 8 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system is now operating with cooling problem.\n' \
` "
		   break ;;

		21)
		   wall "`dspmsg -s 2 rc_powerfail.cat 2 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system is now operating without backup power.\n' \
` "
		   break ;;
	
		22)
		   wall "`dspmsg -s 2 rc_powerfail.cat 3 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system has no backup cooling .\n \
\tThe system will shut down in %s minutes unless the problem is\n \
\tresolved before then.  Execute rc.powerfail -h as the root user \n \
\tfor more information.\n' \
$time` "
		   break ;;

		23)
		   wall "`dspmsg -s 2 rc_powerfail.cat 4 \
'rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system has severe cooling fault.\n \
\tThe system  will shut down NOW.\n' \
` "
		   break ;;

		24)
		   wall "`dspmsg -s 2 rc_powerfail.cat 5 \
'rc.powerfail: The power status has changed.\n \
\tThis indicates a very severe fan fault.\n \
\tThe system will shut down as scheduled.\n' \
` "
		   break ;;

		25)
		   wall "`dspmsg -s 2 rc_powerfail.cat 6 \
'rc.powerfail: The power status has changed.\n \
\tSystem shutdown will occur NOW.\n' \
$state` "
		   break ;;

		26)
		   wall "`dspmsg -s 2 rc_powerfail.cat 7 \
'rc.powerfail: The power status has changed.\n \
\tThe system shutdown has been canceled.\n' \
` "
		   break ;;

		*) dspmsg -s 1 rc_powerfail.cat 2 \
'rc.powerfail: send_msg error/n'
		   break ;;
	
	esac  # end of case through the possible messages to be sent

}  # end of send_msg()

#
#  get_all_status()
#
#  This function will call machstat -f  to check 10-13 bits of pksr register
#  if return value = 0 , fallback on standard behavior
#  else set the environment to the proper failure
#

get_all_status()
{
	# save the current state of the system
	prev_state1=$state1

	# get the current state 
	/usr/sbin/machstat -f
	state1=$?

	# change the state to its true value
	error=`expr $state1 % 16`
	state1=`expr $state1 / 16`

	# see if there has been any change
	if [ $state1 = $prev_state1 -a $state1 -ne 0 ]
	then
		return 		# no need to continue in the function
	fi

	# else the state has changed 
	#
	# There are a certain number of minutes until the system goes down.

	# if the error is anything other than 0 then there was an error 
	# in the call or return from machstat.
	if [ $error -ne 0 ]
	then
		send_msg 19

		# shutdown the system
		/usr/sbin/shutdown -F

		# if we get back to this point, then there was an error
		send_msg 8

		exit 3
	else
		case $state1 in

		0)	# fallback to standard behavior
			stop_warning
			get_status
			break ;;

                # if this change in state indicates that we have gone from bad
                # to a good state, then lets inform the user that we are not
                # going down at all.
		1)
			send_msg 26
			send_msg 20
			stop_warning
			sendwarning=true
		   	warning_nb=1
			send_warning
			exit 0 #no reason to continue
			break ;;
		2)
			send_msg 26
			send_msg 21
			stop_warning
			sendwarning=true
		   	warning_nb=2
			send_warning
			exit 0 #no reason to continue
			break ;;
		3)
			send_msg 24
			stop_warning
			break ;;
		4)
	                # let the user know that we are going down NOW and why
			send_msg 25
			stop_warning
			
			/usr/sbin/sync
			/usr/sbin/sync
			/usr/sbin/halt -q
		 	# if we get this far, then there was an error with halt
			send_msg 9
			exit 2;;
		*)
			# the value returned was a multiple of 16, but was not
			# a valid value.  This is considered a machstat error and the
			# system will begin to shutdown now.

			# inform the user to the change in state 
			send_msg 25

			# shutdown the system
			/usr/sbin/shutdown -F

			# if we get back to this point, then there was an error
			send_msg 8

			exit 3 ;;
		
		esac
	fi  # end the if-else machstat error.

	return
}

#
#  get_status()
#
#  This function will call machstat -p and set the environment to the proper
#  failure.
#

get_status()
{
	# save the current state of the system
	if [ $get_state -eq 1 ]
	then
	prev_state=$state
	fi

	# get the current state
	/usr/sbin/machstat -p
	state=$?

	# change the state to its true value
	error=`expr $state % 16`
	state=`expr $state / 16`

	# see if there has been any change
	if [  $get_state -eq 1 ]
	then
	if [ $state = $prev_state ]
	then
		return 		# no need to continue in the function
	fi
	fi
	# else the state has changed 
	#
	# To even call this routine we are in what is called a "bad" state. 
	# There are a certain number of minutes until the system goes down.

	# if the error is anything other than 0 then there was an error 
	# in the call or return from machstat.
	if [ $error -ne 0 ]
	then
		send_msg 19

		# shutdown the system
		/usr/sbin/shutdown -F

		# if we get back to this point, then there was an error
		send_msg 8

		exit 3

	else

		case $state in

                # if this change in state indicates that we have gone from bad
                # to a good state, then lets inform the user that we are not
                # going down at all.

			0) send_msg 13
			   exit 0 ;;

                # cases where we have gone from "bad" to "worse" and have < 20 
		# seconds left warn the user and exit

			5 | 9| 2 | 3 | 6 | 7 | 8)
	                   # let the user know that we are going down NOW and why
	                   send_msg 12

	                   /usr/sbin/sync
	                   /usr/sbin/sync
			   /usr/sbin/halt -q

			   # if we get this far, then there was an error with halt

			   send_msg 9

	                   exit 2 ;;

                # if this change in state indicates just another fan fail then
                # inform the users and continue on. A "bad" to "bad" change

			1 | 10 | 11 | 12 | 13)
			    send_msg 14
			    break ;;

                # if this change in state indicates that we have gone from bad
                # to an undefined state, then assume the previous state.

			4 | 14 | 15)
				# sync the system just in case
			    /usr/sbin/sync 
			    /usr/sbin/sync
		   	    state=$prev_state
			    break ;;

		# else the value returned was a multiple of 16, but was not
		# a valid value.  This is considered a machstat error and the
		# system will begin to shutdown now.

			*)
			   # inform the user to the change in state 
			   send_msg 12

			   # shutdown the system
			   /usr/sbin/shutdown -F

			   # if we get back to this point, then there was an error
			   send_msg 8

			   exit 3 ;;

		esac  # end of case through the machstat output
	fi  # end the if-else machstat error.

	return

}  # end of get_status()

#
# send_warning()
#
# This function add a crontab entry to warm backup or cooling problem
# every 12 hours
#
send_warning()
{
crontab -l > /tmp/crontab$$ 2>/dev/null
grep "rc.powerfail" /tmp/crontab$$>/dev/null
if [ $? -ne 0 ]
then
	case $warning_nb in
	1)
		echo "0 00,12 * * * wall%rc.powerfail:1:WARNING!!! The system is now operating with cooling problem.">>/tmp/crontab$$
	break ;;
	2)
		echo "0 00,12 * * * wall%rc.powerfail:2:WARNING!!! The system is now operating without backup power.">>/tmp/crontab$$
	break ;;
	esac

crontab /tmp/crontab$$
rm /tmp/crontab$$
return
fi
}

#
# check_warning()
#
# This function removes the crontab entry for the warning messages
#
check_warning()
{
crontab -l > /tmp/crontab$$.sav 2>/dev/null
if [ $? -eq 0 ]
then
	grep $warning_msg /tmp/crontab$$.sav>/dev/null
	if [ $? -eq 0 ]
	then
		crontab -r
		if [ $? -eq 0 ]
		then
			cat /tmp/crontab$$.sav | grep -v $warning_msg >/tmp/crontab$$
			crontab /tmp/crontab$$
			rm /tmp/crontab$$
		fi
	fi
fi
rm /tmp/crontab$$.sav
return
}

#
# stop_warning()
#
# This function sets warning_msg to the proper value in order
# to remove the warning message from crontab
#
stop_warning()
{
case $state1 in
		   0 | 3 | 4 )
			# stop the warning messages
			warning_msg="rc.powerfail"
		   break ;;
		   1)
			# stop only the backup power warning
			warning_msg="rc.powerfail:2"
		   break ;;
		   2)
			# stop only the backup cooling warning
			warning_msg="rc.powerfail:1"
		   break ;;
		   *) 
		   break ;;
esac
check_warning
return
}

#
# check_status()
#
# This function check first bits 10-13 in PKSR using machstat -f command
#
check_status()
{
# check first 10-13 bits in pksr
/usr/sbin/machstat -f
state1=$?

# change the state to its true value
error=`expr $state1 % 16`
state1=`expr $state1 / 16`

if [ $error -ne 0 ]
then
	machstat_error=true
else
	case $state1 in
		0)	# check bits 0-4 of pksr register
		   further_check=true
		   stop_warning
		   break;;

		4)
			# send a message letting them know the situation
		   send_msg 23
		   stop_warning

			# halt the system immediatly
		   /usr/sbin/sync 
		   /usr/sbin/sync 
		   /usr/sbin/halt -q

			# if we get this far, then there was an error
		   send_msg 9
		   exit 2

		   ;;

		# we are in a non-critical situation from this point on


		1)	# warning cooling problem
		   send_msg 20
		   stop_warning
			# sync the system just in case
		   /usr/sbin/sync ; /usr/sbin/sync
		   sendwarning=true
		   warning_nb=1
		   send_warning
		   break ;;

		2)	# warning power problem
		   send_msg 21
		   stop_warning
		        # sync the system just in case
		   /usr/sbin/sync ; /usr/sbin/sync
		   sendwarning=true
		   warning_nb=2
		   send_warning
		   break ;;
	
		3)	# severe cooling problem
		   stop_warning
		   severe_cooling=true
		   break ;;

		*)	# ERROR machstat did not return a value between 
			# 0-15 hex
		   machstat_error=true
		   break ;;

	esac  # end of case through the initial machstat output
fi

return

}

 
#################################################################################
#										#
# 					MAIN					#
#										#
#################################################################################

# At this point in the program, time IS money, so we are attempting to determine
# the problem or problems as quickly as possible so that we can take down the
# system if/as neccessary.

# save the arguments and get the processing  done quickly for the immediate cases.

orig_args=$*

# set defaults

integer time=15		# default time to system shutdown when on battery backup
shutdown=true		# default for system shutdown when on battery backup true

# initialize internal variables

normal=false		# the system is running normally or we should not be here
on_battery=false	# is the system running on battery backup power
fan_fail=false		# has a non-primary fan failed
machstat_error=false	# did machstat fail
undefined=false		# did machstat return an undefined state?
integer counter=0	# initialize the seconds counter for battery backup states
max_time=5		# maximum amount of time before the first message

severe_cooling=false	# has the system severe cooling problem?
sendwarning=false	# send warnings every 12 hours ?
further_check=false	# fallback to standard behavior: run also machstat -p?
get_state=0		# machstat -p already run ?

# check the Power Status : bits 10-13 first
check_status
if [ "$further_check" = "true" ]
then
get_state=1
/usr/sbin/machstat -p
state=$?

# NOTE : machstat was changed to return a factor of 16 instead of the
# real value so that we could more easily discover errors.  The number
# will be divided by 16 before presenting it to the user.

# change the state to its true value
error=`expr $state % 16`
state=`expr $state / 16`

# if the error is anything other than 0 then there was an error 
# in the call or return from machstat.
if [ $error -ne 0 ]
then
	machstat_error=true
else
	case $state in

		0)	# Normal running mode
		   /usr/sbin/sync ; /usr/sbin/sync
		   normal=true
		   break ;;

		   # These are the immediate or 4msec cases.
		   #   5 - Thermal high
		   #   9 - Loss of Primary power supply fan
		   #
		   #   2 - Programmed power off
		   #   3 - Manual off button
		   #   6 - Internal fail
		   #   7 - Power supply overload
		   #   8 - Loss of primary power without battery backup
		5 | 9 | 2 | 3 | 6 | 7 | 8)

			# send a message letting them know the situation
		   send_msg 3

			# halt the system immediatly
		   /usr/sbin/sync 
		   /usr/sbin/sync 
		   /usr/sbin/halt -q

			# if we get this far, then there was an error
		   send_msg 9
		   exit 2

		   ;;

		# we are in a non-critical situation from this point on

		1)	# On battery backup power
		   on_battery=true
		   break ;;

		10 | 11 | 12 | 13)	# Secondary fan fail or faults
		    fan_fail=true
		    break ;;

		4 | 14 | 15)	# undefined states
		    undefined=true
			# sync the system just in case
		    /usr/sbin/sync ; /usr/sbin/sync ; /usr/sbin/sync 
		    break ;;

		*)	# ERROR machstat did not return a value between 
			# 0-15 hex
		   machstat_error=true
		   break ;;

	esac  # end of case through the initial machstat output
fi  # end else it was not an error.
fi # end if further_check=true

# From this point on, we are only dealing with fan fails, on battery 
# machstat error conditions, undefined states or running in a normal state.

# get the options and process accordingly

while getopts hst: opt
do
	case $opt in
		h) send_msg 17		# send the help message
		   if [ $machstat_error = false -a \
			$fan_fail = false -a \
			$on_battery = false -a \
			$severe_cooling = false ]
		   then
			exit 0 
		   fi
		   ;;

		s) shutdown=false	# change shutdown on powerfail to false
		   ;;

		t) tmp_time=$OPTARG	# change the time to shutdown
		   case $tmp_time in

			+([0-9])) time=$tmp_time
				  ;;

			*) send_msg 10
			   send_msg 1
			   time=15
			   ;;
		   esac
		   ;;

		*) send_msg 1		# invalid option 
		   if [ $machstat_error = false -a \
			$fan_fail = false -a \
			$on_battery = false -a \
			$severe_cooling = false ]
		   then
			exit 1 
		   fi
		   ;;

	esac  # end of case for the options
done  # end of while-do through the options

# send a message giving the state of the system
if [ "$fan_fail" = "true" ]	# wall message for fan fail or fault
then
	send_msg 4
fi

if [ "$on_battery" = "true" ]	# wall message for system on battery
then
	send_msg 5
fi

if [ "$machstat_error" = "true" ]	# wall message for error from machstat
then
	send_msg 6
fi

if [ "$severe_cooling" = "true" ]	# wall message for severe cooling problem, 10 minutes before shutdown
then
	time=10
	send_msg 22
fi

if [ "$undefined" = "true" ]	# wall message for undefined returned
then
	send_msg 7
	exit 5			# there is no reason to continue
fi

if [ "$normal" = "true" ]		# wall message for all other states
then
	send_msg 3
	exit 0			# there is no reason to continue
fi

if [ "$sendwarning" = "true" ]	
then
	exit 0			# there is no reason to continue
fi

# for situations where requested, shutdown the system NOW.

if [ $time = 0 -o "$machstat_error" = "true" ]
then			# the user specified immediate shutdown or an error was
			# received from machstat.
	/usr/sbin/shutdown -F

	send_msg 8	# We should not get this far.  Shutdown should have taken
			# the system down immediately.
	exit 3

fi  # end if time=0

# From this point on, we are now dealing only with fan fails and on battery 
# situations where we are going to shutdown the system in some specified amount 
# of time.
# 
# The default time to shutdown is 15 minutes, but the user may have changed this
# with the -t option.
#
# Every 5 seconds we will be checking the power status looking for changes.  The
# power status is not checked in the last 60 seconds of the cycle.
#
# Every 5 minutes up to the last 5 minutes a message will be sent to the users 
# with the time remaining until shutdown.
#
# Every minute in the last 5 minutes a message will be sent to all users
# indicating the time remaining until shutdown.

# set the loop exit values
if [ $time = 1 ]
then
	stop_loop=1
else
	stop_loop=0
fi

counter=0
while [ $time -gt 0 ]		# while the count is greater than 0 minutes
do
	# sleep 5 seconds 
	sleep 5

	# check the power status again.  If we come back from the call then there
	# has been no significant change and the loop should continue, otherwise
	# the system will soon be automatically shutting down anyway.

	get_all_status

	# exit the loop if we are in our last minute

	if [ $counter = 0 -a $stop_loop = 1 ]
	then
		send_msg 11
		break
	fi

	# set/reset the counter if neccessary
        if [ $counter = 0 ]
        then

                send_msg 11

		# reset the max_time if under 5 minutes

                if [ $time -le $max_time ]
                then
                        max_time=1
                fi

		# if we are in the last minutes set stop_loop to exit after
		# this cycle.
		
                if [ $time = 2 ]
                then
                        time=1
                        counter=10	# set to 10 to account for extra 5 sec
					# in the beginning of the loop and the
					# 5 sec of assumed overhead
                        stop_loop=1
                else
			# figure how many cycle we still have 

	                c1=`expr $time - $max_time`

			# if there are less than 10 but more than 5 minutes left
			# then cycle the remaining minutes.

                        if [ $c1 -lt $max_time ]
                        then
                                time=`expr $time - $c1`
                                counter=`expr $c1 \* 11`
                        else
				# else there are either less than 5 or more than
				# 10 minutes to cycle.
                                time=$c1
                                counter=`expr $max_time \* 11`
                        fi

                fi  # end if-else time=2

        fi  # end if counter = 0

	# decrement the counter by one since we have slept the required 5 seconds.
        counter=`expr $counter - 1`


done  # end while time is greater than 5 minutes

# We now have exactly 1 minute left until shutdown or the battery power dies.

# sleep the remaining time
sleep 60

#
# UNCOMMENT THE NEXT LINE TO CHECK THE POWER STATUS ONE MORE TIME
#
#get_status

# if the user did not specify to NOT shutdown (-s)
if [ "$shutdown" = "true" ]
then

	# tell the user that we are shutting down now
	send_msg 15

	# shutdown the system NOW
	/usr/sbin/shutdown -F

	# if we come back, then there was an error and the user should do the 
	# shutdown manually
	send_msg 8
	exit 3

else		# the user specified no shutdown
	
	# Tell the user to shutdown now.
	send_msg 16

	# exit normally
	exit 0

fi   # end if-else shutdown=true

# If we get this far, then there is a problem.

send_msg 18

exit 4
