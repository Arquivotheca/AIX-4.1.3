#! /usr/bin/ksh
# @(#)97  1.13  src/bos/usr/lib/boot/srvboot/srvboot.sh, bosboot, bos41J, 9520A_all 5/16/95 07:04:34
#
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS: srvboot.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: srvboot.sh
#
# FUNCTION:
#	srvboot.sh configures a defined console device.
#	If there is a dump which needs to be copied out,
#	it brings up a copydumpmenu.  If it is in service
#	mode boot, it displays a diagnostic function menu
#	with options to choose diagnostics or maintenance
#	functions.  Upon exit, it unconfigures the console,
#	and proceeds to boot. If console configuration fails,
#	it will hang if there is a dump. Otherwise, it
#	defaults to diagnostics boot (if diagnostics is
#	installed) or single user mode ( if diagnostics is
#	not installed).
#
# EXECUTION ENVIRONMENT:
#
#	CALLED BY:	rc.boot
#
#	SYNTAX:		srvboot
#
#	RETURN VALUE:
#		returns 0 always
#
#
#
###########################################################################
#
# SET EXECUTION ENVIRONMENT

# setup default locale
[ -s /mnt/etc/environment ] && {
. /mnt/etc/environment
export LANG LOCPATH NLSPATH
}

SHOWLED=${SHOWLED:-/usr/lib/methods/showled}
PATH=${PATH:-.:/usr/sbin:/etc:/usr/bin}
MESSAGE="/usr/bin/dspmsg -s 10 srvboot.cat"
mach_type=`bootinfo -T`

# ODM search string for key position
QSTR="name=sys0 and attribute=keylock"
# console device or display adapter name
CONSDEV=
#

# Function config_console
config_console () {
#
# configures or unconfigure console device if defined in ODM
# Note: If console is not defined in ODM, cfgcon will fail.
#
case $1 in
    ON ) # configure the console
	/usr/sbin/strload -f /dev/null
	CONSDEV=`/usr/lib/methods/cfgcon -c` # configure console and keyboard
	return "$?"
	;;
    OFF ) # unconfigure the console
	/usr/lib/methods/cfgcon -u # set console to null
	;;
esac
} # End of config_console

# Function: set_odmkeylock
set_odmkeylock () {
#
# Sets keylock attribute to correct value
#
     X=`odmget -q"attribute=keylock and value='$1'" CuAt`
     [ -n "$X" ] && return 0   # already set correctly
     if [ "$1" = "normal" ]
     then
	odmget -q"$QSTR" CuAt  | \
	sed 's/service/normal/' | \
	odmchange -o CuAt -q"$QSTR"
     else
	odmget -q"$QSTR" CuAt  | \
	sed 's/normal/service/' | \
	odmchange -o CuAt -q"$QSTR"
     fi
     return $?
} # End of set_odmkeylock

# Function: func_menu
func_menu() {
#
# Display Function Menu and read selected options
#

${SHOWLED} 0xFFF	# leds off

$MESSAGE 100 '

DIAGNOSTIC OPERATING INSTRUCTIONS

POWERstation and POWERserver
LICENSED MATERIAL and LICENSED INTERNAL CODE - PROPERTY OF IBM.
(C) COPYRIGHTS BY IBM AND BY OTHERS 1982,1991-1994.
ALL RIGHTS RESERVED.

'

$MESSAGE 102 'These programs allow you to enter diagnostics, service aids, single
user mode, or low-level maintenance mode.  The diagnostics should be
used whenever problems with the system occur which have not been
corrected by any software application procedures available.

In general, the diagnostics will run automatically.  However, sometimes
you will be required to select options, inform the system when to
continue, do simple tasks, and exchange diskettes.

Several keys are used to control the diagnostics:
- The Enter key continues the procedure or performs an action.
- The Backspace key allows keying errors to be corrected.
- The cursor keys are used to select an option.

To continue, press Enter.
'

read ANS

while :
do
if [ $mach_type != "rspc" ]; then

$MESSAGE 104 '
FUNCTION SELECTION

1. Diagnostic Routines
    This selection will test the machine hardware. A problem will be
    indicated by a SRN (Service Request Number).
2. Service Aids
    This selection will look at the machine configuration, exercise
    external interfaces, format media, look at past diagnostic
    results, control what resources are tested, check out media, etc.
3. Advanced Diagnostic Routines
    This selection will normally be used by the service representative.
4. System Exerciser
    This selection will test resources running in an overlap mode.
5. Single User Mode
    The system will enter single-user mode for software maintenance.
\n\n\n\n\n\n\n\nTo make a selection, type the number and press Enter [1]:'

else

$MESSAGE 112 '
FUNCTION SELECTION

1. Diagnostic Routines
    This selection will test the machine hardware. A problem will be
    indicated by a SRN (Service Request Number).
2. Service Aids
    This selection will look at the machine configuration, exercise
    external interfaces, format media, look at past diagnostic
    results, control what resources are tested, check out media, etc.
3. Advanced Diagnostic Routines
    This selection will normally be used by the service representative.
4. Single User Mode
    The system will enter single-user mode for software maintenance.
\n\n\n\n\n\n\n\n\n\nTo make a selection, type the number and press Enter [1]:'
fi

	read ANS

	# check if return is entered
	[ -z "$ANS" ] && ANS=1

	# If selection 4 is made on a RSPC system, set it to a 5 for the case
	# statement below. (Single User Mode)
	# The System Exerciser is not supported on RSPC systems.
	if [ $mach_type = "rspc" -a $ANS = 4 ]; then
		ANS=5
	fi

	case "$ANS" in
	 1|2|3|4 ) # Diagnostics functions selected
	    if [ ! -s /usr/lpp/diagnostics/bin/diagpt  ]
	    then
		$MESSAGE 106 '\nDiagnostics not installed.  Press Enter to continue.'
		read ANS
		continue
	    else
		echo '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n'
		$MESSAGE 110 'The system will now continue the boot process.  Please wait...'
		echo $ANS > /mnt/etc/lpp/diagnostics/data/fastdiag
		[ $? -eq 0 ] && {
			set_odmkeylock service
			/etc/init -c "telinit x" > /dev/null
		} || { # if echo failed, go into single user mode
			set_odmkeylock normal
			/etc/init -c "telinit s" > /dev/null
		}
		return 0
	    fi
	    ;;
	5) # Single User option selected
	    echo '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n'
	    $MESSAGE 110 'The system will now continue the boot process.  Please wait...'
	    set_odmkeylock normal
	    /etc/init -c "telinit s" > /dev/null
	    return 0
	    ;;
	41) # Low-level Maintenance selected.  This function is provided
	    # without support and entirely at the user's risk.
	   echo '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n'
	   $MESSAGE 108 '\t\t\t\tWARNING!\n\nAny type of system use in this environment is NOT recommended\nand NOT supported!  Proceed at your own risk.\n\nExit from this shell to return to the main menu.\n\n'
	   trap 1 2 15
	   /bin/bsh
	   trap '' 1 2 15
	    ;;
	*)	;;		# bad answer - try again
	esac
done
}  #End of func_menu

#
#  Main
#
trap '' 1 2 15
config_console  ON   #Configure console
if [ $? -eq 0 ]
then
	# console configured
	[ -f /needcopydump ] &&  { # Check for dump
		[ -x /usr/sbin/cfgmir ] && /usr/sbin/cfgmir ON
		copydumpmenu -b
		[ -x /usr/sbin/cfgmir ] && /usr/sbin/cfgmir OFF
		swapon /dev/hd6		# Turn paging on
	}

	diag_ok=1			# Ok to run diagnostics
	if [ $mach_type = "rspc" ]; then
		# This is only valid on this type of machine
		bootinfo -M > /dev/null
		rc=$?
		if [ $rc -ne 101 -a $rc -ne 103 ]; then
			diag_ok=0	# diagnostics not supported
		fi
	fi

	[ -n "$KEYPOS" -a $diag_ok = 1 ] && {		# Check key position
		# run pretests here
		[ -s /usr/lpp/diagnostics/bin/diagpt ] && {
			# set up links for diagnostic databases
			for i in PDiagDev PDiagDev.vc PDiagAtt PDiagAtt.vc
			do
				ln -s /usr/lib/objrepos/$i /etc/objrepos/$i
			done
			for i in CDiagDev TMInput MenuGoal DAVars FRUB FRUs
			do
				cp  /mnt/etc/objrepos/$i /etc/objrepos/$i
			done
			[[ $CONSDEV != tty* ]] && {
				# start rcm for graphics adapters only
				${SHOWLED} 0x594
				/usr/lib/methods/startrcm > /dev/null
				/usr/lib/methods/cfgrcm -l rcm0 > /dev/null
			}
			${SHOWLED} 0xFFF

			# run pretest on console and native keyboard if present
			/usr/lpp/diagnostics/bin/diagpt -d $CONSDEV
			[[ $CONSDEV != tty* ]] && {
				# Bring rcm back down to defined.
				rmdev -l rcm0 > /dev/null
			}
		}

		[ -x /usr/sbin/cfgmir ] && /usr/sbin/cfgmir ON
		func_menu	# Bring up menu
		[ -x /usr/sbin/cfgmir ] && /usr/sbin/cfgmir OFF
	}
	config_console OFF   # Unconfig console
else
	# console fail to configure
	[ -f /needcopydump ] && exit 1

	# Goto single user mode if diagnostics is not installed
	[ ! -s /usr/lpp/diagnostics/bin/diagpt ] && {
		set_odmkeylock normal
		/etc/init -c "telinit s" > /dev/null
	}
fi
exit 0
