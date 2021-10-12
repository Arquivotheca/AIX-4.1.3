#!/bin/bsh
# @(#)97	1.49  src/bos/diag/diags/diag.sh, diagsup, bos41J, 9519A_all 5/8/95 13:32:56
#
# COMPONENT_NAME: CMDDIAG  DIAGNOSTIC SUPERVISOR
#
# FUNCTIONS: Diagnostic invoke script
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

###################################################################
# diag: script to invoke the Diagnostic supervisor/controller.
#

# If in maintenance mode, detach available interfaces before diags
detach_interface()
{
if [ "$DIAG_ENVIRONMENT" = "$EXENV_STD" ]
then
        avail_if=`/usr/sbin/lsdev -C -c if -S available | /usr/bin/awk '{ print $1 }'`
        if [ -n "$avail_if" ]     # there are available interfaces
        then
                for i in $avail_if
                do
                        /usr/sbin/ifconfig $i detach
                done >/dev/null 2>&1
        fi
fi
}


# If in maintenance mode, restore available interfaces
restore_interface()
{
if [ "$DIAG_ENVIRONMENT" = "$EXENV_STD" ]
then
        avail_if=`/usr/sbin/lsdev -C -c if -S available | /usr/bin/awk '{ print $1 }'`
        if [ -n "$avail_if" ]      # there are available interfaces
        then
                for i in $avail_if
                do
                        /usr/sbin/ifconfig $i up
                        /usr/sbin/ifconfig $i down
                done >/dev/null 2>&1
        fi
fi
}


# Check to see if diagnostics is loaded on the hardfile. If not then
# see if there is a CDROM drive present on the machine. If so, then prompt
# the user to see if they want to run diagnostics off a mountable Diagnostics
# CDROM.
check_diag_cdrom()
{

lslpp -l bos.diag.rte	>/dev/null 2>&1
if [ $? -ne 0 ]
then

	CDROM_AVAIL=`/usr/bin/odmget -q"name like cd* AND status = 1 AND chgstatus != 3" CuDv`
	if [ "CDROM_AVAIL$CDROM_AVAIL" = CDROM_AVAIL ]
	then
	# There are no CDROM drives available in the system. Diagnostics
	# need to be installed or run in standalone mode.
		dspmsg -s 2 diagcd.cat 1 \
"\n\nHardware Diagnostics are not installed on your system.\n\
Use the Standalone Diagnostic package or install\n\
diagnostics to perform diagnostic functions.\n\n\n"
		exit 0
	else

	# Diagnostics is not installed. Only allow diag to run if there
	# is there is no -c flag passed to the diag command.

		if [ $NOCONSOLE = 1 ]
		then
			dspmsg -s 2 diagcd.cat 9 \
"\n\nHardware Diagnostics are not installed on your system.\n\
However, they can be run if the Diagnostic CD-ROM is inserted\n\
in the CD-ROM drive, and only if the diag command is issued\n\
without any flags. Please re-enter the diag command with no\n\
flags, if you wish to perform diagnostics.\n\n"
			exit 0
		fi

		dspmsg -s 2 diagcd.cat 2 \
"\n\nHardware Diagnostics are not installed on your system.\n\
However, they can be run if the Diagnostic CD-ROM is inserted\n\
in the CD-ROM drive.\n\n\
Do you want to run diagnostics from the CD-ROM? (Enter 'y' or 'n') : "
		read R
		if [ "$R" = 'y' -o "$R" = 'Y' ]
		then
			create_mount_point
			if [ $? -ne 0 ]
			then
				return 1
			fi
			get_cdrom_list
			dspmsg -s 2 diagcd.cat 12 \
"\n\nIf applicable, unmount the file system from the desired CD-ROM drive.\n\
Remove any existing CD-ROM disc from the CD-ROM drive.\n\
Insert the Diagnostic CD-ROM into the CD-ROM drive, press Enter\n\
when ready or type the name of the CD-ROM drive that you want to use\n\
to run diagnostics ($cdromlist <$defaultdrive> ):\n\
(NOTE: To cancel, enter the <CTRL> 'C'. Default drive is shown between '<>')\n" \
"$cdromlist" "$defaultdrive"
			read R
			if [ "$R" != "" ]
			then
				defaultdrive=$R
			fi
			mount_cdrfs
			if [ $? -ne 0 ]
			then
				return 1
			fi
			check_level
			if [ $? -ne 0 ]
			then
				return 1
			fi
			populate_diagodm
			if [ $? -ne 0 ]
			then
				return 1
			fi
			run_from_cd=1
		else
			exit 0
		fi
	fi
else
	diag_installed=1
fi
}

# Obtain a list of all available CDROM drive in the system

get_cdrom_list()
{

	/usr/bin/odmget -q"name like cd* AND status = 1 AND chgstatus != 3"\
		 CuDv >/tmp/diag.cdromlist
	cdromlist=`awk -F'"' ' /name/ {print $2} ' /tmp/diag.cdromlist`
	numberofcds=`awk -F'"' ' /name/ { count += 1 } END { print count} ' \
		/tmp/diag.cdromlist`
	rm -rf /tmp/diag.cdromlist
	if [ "$numberofcds" != "1" ]
	then
		defaultdrive=`echo $cdromlist | cut -d ' ' -f 1`
	else
		defaultdrive=$cdromlist
		cdromlist=""
	fi

}

# Create a temporary directory to mount the cdrfs, in order to populate
# the diagnostic ODM object class.

create_mount_point()
{

	FREE=`df -k /tmp | tail -1 | awk '{ print $3 }'`
	if [ "$FREE" -lt 2048 ]
	then
		space_needed=$FREE+2048
		dspmsg -s 2 diagcd.cat 5 \
"\nThere is not enough  file system space present in /tmp.\n\
Please increase the file system space to $space_needed (1024-byte blocks),\n\
and re-run diagnostics.\n\n" "$space_needed"
		return 1
	fi

	if [ -d $CDRFS_DIR ]
	then
		dspmsg -s 2 diagcd.cat 13 \
"\n\n\The directory '/tmp/DIAGCDRFS' exists. This directory needs to be\n\
removed before diagnostics can be run from the mounted CD-ROM drive.\n\
Please ensure that no other 'diag' session is in progress.\n"
		return 1
	fi

	mkdir $CDRFS_DIR
	if [ $? -ne 0 ]
	then
		return 1
	fi
	CDROMDIR_CREATED=1
	mkdir $DIAG_OBJREPOS
	if [ $? -ne 0 ]
	then
		return 1
	fi
	mkdir $DIAG_DATA
	if [ $? -ne 0 ]
	then
		return 1
	fi
	mkdir $DIAG_DATA/data
	if [ $? -ne 0 ]
	then
		return 1
	fi
}

# Mount the CDROM file system

mount_cdrfs()
{
	dspmsg -s 2 diagcd.cat 4 \
"\n\nMounting CD-ROM File System....\n"
	CDRFS_MOUNTED=1
	mount -o ro -v cdrfs /dev/$defaultdrive $CDRFS_DIR
	if [ $? -ne 0 ]
	then
		CDRFS_MOUNTED=0
		return 1
	fi
}

# Check level of Operating System against level in CDROM

check_level()
{
	version=`uname -v`
	release=`uname -r`
	if [ -f $CDRFS_DIR/.version -a -f $CDRFS_DIR/usr/lpp/diagnostics/bin/dctrl ]
	then
		read cdversion cdrelease fixlevel < $CDRFS_DIR/.version
		if [ "$version" != "$cdversion" ]
		then
			dspmsg -s 2 diagcd.cat 6 \
"\n\nThe Diagnostics on the CD-ROM are not compatible with the Operating\n\
System level. Run the Standalone Diagnostic Package from CD-ROM.\n\n\n"
			return 1
		fi

		# If release does not match, it may still be OK to run. Only
		# display warning message.

		if [ "$release" != "$cdrelease" ]
		then
			dspmsg -s 2 diagcd.cat 10 \
"\n\nThe release number on the CD-ROM does not match the Operating\n\
System release number. If the diagnostic cannot be started, \n\
run the Standalone Diagnostic Package from CD-ROM.\n\n\n"
			sleep 2 # To allow message to be seen by user.
		fi
		return 0
	else
		dspmsg -s 2 diagcd.cat 11 \
"\n\nThe CD-ROM is not a Diagnostic CD-ROM.\n\n"
		return 1
	fi
}

# Populate the ODM databse under the /tmp directory with
# diagnostics object class.

populate_diagodm()
{

	dspmsg -s 2 diagcd.cat 8 \
			"\nPopulating ODM data base......\n"
	ODMDIR=$CDRFS_DIR/usr/lpp/diagnostics/obj \
		/usr/bin/odmshow CDiagDev DSMOptions DAVars \
		DSMenu DSMOptions FRUB FRUs MenuGoal PDiagAtt \
		PDiagDev TMInput > $DIAG_OBJREPOS/diagodm.cre

	ODMDIR=$DIAG_OBJREPOS /usr/bin/odmcreate -c  \
		$DIAG_OBJREPOS/diagodm.cre >/dev/null 2>&1
	if [ $? -ne 0 ]
	then
		return 1
	fi

	# Due to binary compatibility, an odmshow of PdDv and PdAt,
	# will not give a .cre file that allow creation of PdDv.vc
	# and PdAt.vc. Therefore, these object classes need to be
	# copied over to the new objrepos directory, then deleted.

	cp /etc/objrepos/PdDv $DIAG_OBJREPOS/
	cp /etc/objrepos/PdDv.vc $DIAG_OBJREPOS/
	cp /etc/objrepos/PdAt $DIAG_OBJREPOS/
	cp /etc/objrepos/PdAt.vc $DIAG_OBJREPOS/
	cp /etc/objrepos/PdCn $DIAG_OBJREPOS/
	if [ $DIAG_OBJREPOS != "/etc/objrepos" ]
	then
		ODMDIR=$DIAG_OBJREPOS /usr/bin/odmdelete -o PdDv >/dev/null 2>&1
		ODMDIR=$DIAG_OBJREPOS /usr/bin/odmdelete -o PdAt >/dev/null 2>&1
		ODMDIR=$DIAG_OBJREPOS /usr/bin/odmdelete -o PdCn >/dev/null 2>&1

		ODMDIR=$DIAG_OBJREPOS /usr/bin/odmadd \
			$CDRFS_DIR/diagodm.add >/dev/null 2>&1
		ODMDIR=$DIAG_OBJREPOS /usr/bin/odmadd \
			$CDRFS_DIR/async.add >/dev/null 2>&1
	fi
	LIBPATH=$CDRFS_DIR:/lib:/usr/lib \
		$CDRFS_DIR/usr/lpp/diagnostics/bin/mergeodm \
		/etc/objrepos $DIAG_OBJREPOS
	ODM_MERGED=1
	return 0
}


# Clean up routine

cleanup()
{

	if [ $CDRFS_MOUNTED -eq 1 ]
	then
		unmount /dev/$defaultdrive	>/dev/null 2>&1
	fi
	if [ $ODM_MERGED -eq 1 ]
	then
		if [ $DIAG_OBJREPOS != "/etc/objrepos" ]
		then
	 		for i in CDiagDev DSMOptions DAVars DSMenu \
		 		FRUB FRUs MenuGoal PDiagAtt PDiagDev \
			 	TMInput PdDv PdAt PdCn
			 do
				 ODMDIR=$DIAG_OBJREPOS \
					 /usr/bin/odmdrop -o $i	>/dev/null 2>&1
			done
		fi
	fi
	rm -rf $DIAG_OBJREPOS	>/dev/null 2>&1
	rm -rf $DIAG_DATA	>/dev/null 2>&1
	if [ $CDROMDIR_CREATED -eq 1 ]
	then
		rm -rf $CDRFS_DIR	>/dev/null 2>&1
	fi

}

# respawn function, called when an interrupt is detected

respawn_function()
{
	RESPAWNFLAG=0
	cleanup
	exit 0
}


########### Main starts here ##################
# Defaults are Concurrent mode and no stand alone 
EXENV_STD=2
EXENV_CONC=4
DIAG_ENVIRONMENT=$EXENV_CONC
DIAG_REPORT=1
NOCONSOLE=0
DIAG_KEY_POS=0
CDRFS_MOUNTED=0
CDROMDIR_CREATED=0
ODM_MERGED=0
IPLSOURCE_DISK=0
IPLSOURCE_CDROM=1
IPLSOURCE_TAPE=2
IPLSOURCE_LAN=3
saved_status=0
run_from_cd=0
numberofcds=0
defaultdrive=""
cdromlist=""
diag_installed=0
CURRENT_DATE=`date +"%y""%m""%d""%H""%M""%S"`      # Work directory date stamp
CDRFS_DIR=/tmp/DIAGCDRFS
DIAG_OBJREPOS=/tmp/DIAGOBJREPOS.$CURRENT_DATE
DIAG_DATA=/tmp/DIAGDATA.$CURRENT_DATE

# Trap signal INT, QUIT and TERM

trap respawn_function 2 3 15
# Check for /etc/.init.state file.  If present, get contents.
if test -s "/etc/.init.state"
then
        MODE=`cat /etc/.init.state`
        case $MODE in
                s|S)    DIAG_ENVIRONMENT=$EXENV_STD;;
                *)      DIAG_ENVIRONMENT=$EXENV_CONC;;
        esac
fi

# Check to see if diagnostics can be run

pcmodel=`bootinfo -T`
if [ "$pcmodel" = "rspc" ]
then
	bootinfo -M >/dev/null 2>&1
	if [ "$?" -ne 101 -a "$?" -ne 103 ]
	then
	    dspmsg -s 4 diagcd.cat 1 \
		"\n\nThe 'diag' command is not supported on this system.\n\n"
	    exit 0
	fi
										fi

# Check for the boot type
BOOTYPE=`bootinfo -t`
case $BOOTYPE in
          1)    DIAG_IPL_SOURCE=$IPLSOURCE_DISK;;
          3)    DIAG_IPL_SOURCE=$IPLSOURCE_CDROM;;
          4)    DIAG_IPL_SOURCE=$IPLSOURCE_TAPE;;
          5)    DIAG_IPL_SOURCE=$IPLSOURCE_LAN;;
          *)    ;;
esac
export DIAG_IPL_SOURCE
# Parse command line option to see if the no console flag is passed in

cmdline="$*"
set -- `getopt "acCAlesd:r:" $*`

while [ "$1" != "--" ]
do
	case $1 in
		"-c")
		    NOCONSOLE=1
		    ;;
	esac
	shift
done

# Now determine if diagnostics is installed or if there is a CDROM present

check_diag_cdrom $cmdline
if [ $? -ne 0 ]
then
	cleanup
	exit 1
fi
if [ $diag_installed -eq 1 ]
then
	# Now set up basic environment variables
	: ${DIAGNOSTICS:=/usr/lpp/diagnostics}
	: ${DIAGX_SLIH_DIR:=/usr/lpp/diagnostics/slih}

	# Diagnostic data directory - diagnostic conclusion files, etc
	: ${DIAGDATA:=/etc/lpp/diagnostics}
	: ${DIAGDATADIR:=$DIAGDATA/data}

	# Object class repository directory
	ODMDIR=/etc/objrepos
	export ODMDIR
	# Search path for the shared libraries
	: ${LIBPATH:=/lib:/usr/lib}
	NLSPATH=/usr/lib/nls/msg/%L/%N:/usr/lib/nls/msg/prime/%N:$DIAGNOSTICS/catalog/default/%N:$NLSPATH
else
	if [ $run_from_cd -eq 1 ]
	then
		: ${DIAGNOSTICS:=$CDRFS_DIR/usr/lpp/diagnostics}
		: ${DIAGX_SLIH_DIR:=$DIAGNOSTICS/slih}

		# IPL from hard file, but run diag via CD.
		DIAG_SOURCE=1
		# Object class repository directory
		ODMPATH=$DIAG_OBJREPOS:$ODMDIR
		unset ODMDIR
		export ODMPATH
		# Search path for the shared libraries
		LIBPATH=$CDRFS_DIR:/lib:/usr/lib
		MCPATH=$CDRFS_DIR/usr/lib/microcode:/usr/lib/microcode:/etc/microcode
		export MCPATH CDRFS_DIR
		ALT_DIAG_DIR=/etc/lpp/diagnostics
		export ALT_DIAG_DIR
		DIAGDATA=$DIAG_DATA
		DIAGDATADIR=$DIAGDATA/data
		NLSPATH=/usr/lib/nls/msg/%L/%N:$DIAGNOSTICS/catalog/default/%N:$ALT_DIAG_DIR/catalog/%L/%N:$ALT_DIAG_DIR/catalog/default/%N:$ALT_DIAG_DIR/catalog/En_US/%N:$NLSPATH
	else
		exit 1
	fi
fi
# Diagnostic Applications path
: ${DADIR:=$DIAGNOSTICS/da}
: ${DIAG_UTILDIR:=$DIAGNOSTICS/bin}



# Search path for the catalog files
if [ "X$LANG" = X -o "$LANG" = C ]; then
      	LANG=en_US
fi

		
NOTTY=0
TRM=`/usr/bin/tty`
if [ $? -ne 0 ]
then
	NOTTY=1
fi
case $TRM in
        /dev/con* ) : ${TERM:=dumb};;
        /dev/tty* ) : ${TERM:=dumb};;
        /dev/lft* ) : ${TERM:=lft};;
        * ) : ${TERM:=dumb};;
esac


# What diagnostics will check is to see if the X server is running.
# If it is, then regardless of the terminal used (i.e aixterm,
# tty, lft) the X_DIAG environment should be set to 1 in order 
# for the display adapters, keyboard, and mouse to not show in
# the device test list. This is done by finding the X server pid
# using ps, then use awk to find the record whose 4th field contains 
# '.' or a number of '/' and terminating with X
# Below is a sample output from the above ps command:
#  root  4576      1 /usr/sbin/uprintfd 
#  root  4630   4306 /usr/lpp/info/bin/infod 
#  root  5096   4306 /usr/sbin/syslogd 
#  root  5348   4060 /usr/lpp/X11/bin/X -D /usr/lib/X11/rgb -T -force :0 -au
#  root  5378   4306 /usr/sbin/portmap 
#

X_DIAG=0
XPID=`/usr/bin/ps -ef -o ruser,pid,ppid=parent,args | awk '$4~/^.*\/X$/||$4~/X$/{print $2}'`
if [ -n "$XPID" ]
then
 	X_DIAG=1
fi

#  Export all of the environment variables
export TERM DIAGNOSTICS DIAGDATADIR DIAGDATA DIAG_REPORT
export NLSPATH DADIR LANG DIAGX_SLIH_DIR DIAG_UTILDIR
export DIAG_ENVIRONMENT DIAG_KEY_POS DIAG_SOURCE
export LIBPATH X_DIAG

# If in maintenance mode, detach available interfaces before diags
detach_interface

# Execute the Diagnostic Supervisor with any parameters passed on
$DIAGNOSTICS/bin/diags $cmdline
saved_status=$?

# If in maintenance mode, restore available interfaces
restore_interface

cleanup

exit $saved_status
