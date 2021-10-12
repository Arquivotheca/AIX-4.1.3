#!/bin/ksh
# @(#) 29 1.201 src/bos/usr/lpp/bosinst/bi_main/bi_main.sh, bosinst, bos41J, 9523C_all 6/8/95 15:43:45
# COMPONENT_NAME: (BOSINST) Base Operating System Installation
#
# FUNCTIONS: bosmain
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

# WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNIN:
# In vi, :set tabstop=4 to get everything to line up correctly when editing
# this file.

################################################################################
# The pound signs followed by a number throughout this file refer to the
# modular design flow.
################################################################################

# MIGRATION NOTE:
#
# Each new release of AIX must set the variable CURLEVEL to the current
# level of the release (CURLEVEL is defined in the Initialize function).  The 
# format is: 
#
#                 <version>.<release>.<modification>
#
# For example, if the current release is 4.1.1, CURLEVEL must be set to 4.1.1. 
# Only one character may be used for each field of CURLEVEL. For example, 
# if the current release is 4.1.10, CURLEVEL must be set to 4.1.A. This will 
# ensure that the string comparison operations (used in this shell script) work 
# correctly (e.g., 4.1.A is greater than 4.1.9, but 4.1.10 is not greater 
# than 4.1.9).
#
# Each new release of AIX must also change the file bosinst/blvset/blvset.c 
# to define a string that will uniquely identify the release.
#

################################################################################
# Beginning of function declarations:
#      Functions are declared in olphabetical order.  Please keep it that way.
################################################################################

# NIMCLIENTPATH used to find the nimclient command, which may either be
# in the SPOT or in the RAM filesystem.
NIMCLIENTPATH=""

# Breakpt: Used for debugging purposes only.  
# Allows you to easily set a breakpoint in this script.
# Enter <cntl>-d to exit.
#
function Breakpt
{
exec >/dev/console 2>&1
while read -r cmd?"Enter command>> "
do
        eval "$cmd"
done
}


# BI_Copy_Files:  Copies a list of customization files during a preservation
#   install.  It copies the files specified in "/etc/preserve.list" to
#   /tmp/bos/.
#     syntax:  BI_Copy_Files
#     Always returns 0.
function BI_Copy_Files
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	CWD=`pwd`
	cd $MROOT
# 312
	IM=`bidata -b -g control_flow -f INSTALL_METHOD`
	if [ "$IM" = preserve ]
	then # 300
# 302
		# If the /etc/preserve.list did not come from a diskette.
		# then check the Target system for the file.
		if [ -z "$PRESERVE_ON_DISKETTE" -a -s "$MROOT$PRE_LIST" ]
		then
		      cp $MROOT$PRE_LIST $PRE_LIST
		fi

		grep -v '^#' "$PRE_LIST" | \
			while read FILE_NAME other
			do
			   if [ -n "$FILE_NAME" ]
			   then
				if [ -d "./$FILE_NAME" ]
				then
				   mkdir -p $MROOT/tmp/bos/${FILE_NAME} 2>/dev/null
				else
				   mkdir -p $MROOT/tmp/bos/${FILE_NAME%/*} 2>/dev/null
				   cp ./$FILE_NAME $MROOT/tmp/bos/$FILE_NAME 2>/dev/null
				fi
			   fi
			done
# 328
	# In the migrate path, remove the base AIX files from the existing
	# file systems.
	elif [ "$IM" = migrate ]
	then # 329
		# If booted from tape, move migration files from /usr/lpp/bos to
		# /tmp/bos.  If booted from CDROM or network, unarchive migration
		# files from /usr/lib/bos/liblpp.a and leave them in /usr/lib/bos.
		cd $MROOT/tmp/bos
		case "$BOOTTYPE" in
		"$TAPE" )
			mv /usr/lpp/bos/bos.rte.cfgfiles /usr/lpp/bos/bos.rte.pre_i \
				/usr/lpp/bos/bos.rte.*.rmlist /usr/lpp/bos/incompat.pkgs .
		;;
		"$NETWORK" | "$CDROM" )
			ar x /usr/lpp/bos/liblpp.a bos.rte.cfgfiles \
			     bos.rte.usr.rmlist bos.rte.root.rmlist incompat.pkgs
			# If the pre_i does not exist in the SPOT or is older than the
			# liblpp.a in the SPOT then get the pre_i out of the library
			# This allows an update of bos.rte.pre_i in the spot
		    if [[ -f /usr/lpp/bos/bos.rte.pre_i ]] &&
				[[ /usr/lpp/bos/liblpp.a -ot /usr/lpp/bos/bos.rte.pre_i ]]
		    then
			    cp -p /usr/lpp/bos/bos.rte.pre_i .
		    else
			    rm -f /usr/lpp/bos/bos.rte.pre_i >/dev/null 2>&1
			    ar x /usr/lpp/bos/liblpp.a bos.rte.pre_i
			fi

		;;
		esac
		cd /

        # Save the size attributes of /tmp
		HD3_LVID=`getlvodm -l hd3`
		set -- `getlvodm -c $HD3_LVID`
		bidata -i -m lv_data -f COPIES -e "$6" -c LOGICAL_VOLUME -v hd3
		LANG=C lslv $HD3_LVID | while read hoser
		do
			set -- `echo $hoser`
			case $1 in
			MAX)     
				bidata -i -m lv_data -f MAX_LPS -e "$3" -c LOGICAL_VOLUME -v hd3
				;;
			LPs:)
				bidata -i -m lv_data -f LPs -e "$2" -c LOGICAL_VOLUME -v hd3
				;;
			*)
				;;
			esac
		done
		# Save size of old /tmp in case we need to shrink it back
		set -- `df /dev/hd3 | tail +2`
		bidata -i -m fs_data -f FS_SIZE -e "$2" -c FS_NAME -v /tmp
# 332
		mv $MROOT/tmp/bos/bos.rte.pre_i /
		/bos.rte.pre_i
		rm /bos.rte.pre_i

# 269

		# Redraw the status screen when the pre_i has exitted since it has
		# its own menu.
		echo "$CLEAR\c" >/dev/console
		if [ `/usr/sbin/bootinfo -k` != 3 ]
		then
			/usr/lpp/bosinst/berror -f " " -e 30 -a 1 >/dev/console 2>&1
		else
			/usr/lpp/bosinst/berror -f " " -e 25 -a 1 >/dev/console 2>&1
		fi

		# Restart the status update routine, because pre_i has stopped it.
		if [ -s /../Update_Status.pn ]
		then
			kill -CONT `/usr/bin/cat /../Update_Status.pn`
		fi
	fi

	cd "$CWD"

	return 0
}

# BI_Error:  Error routine called if an arror occurs after the user interface
#     has exited.  It will stop logging, print the error to the screen,  and
#     wait for the user to continue or go into a shell.  If the berror routine
#     exits, logging to the file will begin again.
#     syntax:  BI_Error <funcname> <error_code> <action_code> [ <more_info> ]
#         where funcname is the name of the function being run,
#               error_code is the message number which must be displayed
#               action_code is either 1 or 2.  1=display error and go on,
#                                              2=display error and stop.
#               more_info is extra information about the error.
#     Always returns 0.
function BI_Error
{
	#MAXBERROR is the highest value berror value in the current bos.rte image
	MAXBERROR=38

	trap 'BI_Error "BOS Install" 29 2' INT

	# Stop the updating of the screen while berror is called.
	if [ -s /../Update_Status.pn ]
	then
		kill -STOP `cat /../Update_Status.pn`
	fi

	# If new error conditions had to be added after the ship date, then
	# we have to use the berror in the bosinst image rather than off of
	# the bos.rte image
	if [ $2 -gt $MAXBERROR ]
	then
	   BERROR=/../usr/lpp/bosinst/berror
	else
	   BERROR=/usr/lpp/bosinst/berror
	fi

	# tell NIM about the failure
	if [ "$BOOTTYPE" -eq "$NETWORK" ]
	then
		if [ -n "$4" ]
		then
			$BERROR -f "$1" -e $2 -a 1 -r "$4" 2>/tmp/error
		else
			$BERROR -f "$1" -e $2 -a 1 2>/tmp/error
		fi

		${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
			-a info="$( cat /tmp/error )"

	fi

	if [ "$SETX" -eq 1 ]
	then
		exec 2>/dev/console
		exec >/dev/console
	fi

	# $3 value of 3 is same as 2 except for no-prompt & no-console on NIM
	if [ $3 = 3 ]
	then
	   action_code=2
	else
	   action_code=$3
	fi

	# If ERROR_EXIT is set, run it instead of berror.
	error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
	if [ -n "$error_exit" ]
	then
		exec $error_exit </dev/console >/dev/console 2>/dev/console
	else
		# Logic:  If NIM No-Prompt Install, inform server of error
		#          and die on fatal errors.
		#          Otherwise, Prompt user for an Action.
		if [ "$BOOTTYPE" -eq "$NETWORK" -a "$action_code" = 2 ]
		then
		    P=`bidata -b -g control_flow -f PROMPT`
		    C=`bidata -b -g control_flow -f CONSOLE`
			# Action code of 3 means do everything possible to get system
			# administrator to correct the problem.  This is only possible
			# if there is a console.
			if [ $3 -eq 3 ]
			then
				# If console is not a real device, then there is no choice
				# but to kill the installation.
				if [[ "$C" != /dev/?* ]]
				then
					loopled 0x623
				fi
			else
		    	if [ "$P" != yes ]
		    	then
			    	loopled 0x623
		    	fi
			fi
		fi
		if [ -n "$4" ]
		then
			$BERROR -f "$1" -e $2 -a $action_code -r "$4"
		else
			$BERROR -f "$1" -e $2 -a $action_code
		fi
	fi
	# Redraw the status screen after a fatal error message was displayed.
	if [ "$action_code" -eq 2 ]
	then
	     echo "$CLEAR\c" >/dev/console
	     if [ `/usr/sbin/bootinfo -k` != 3 ]
	     then
		     $BERROR -f " " -e 30 -a 1 >/dev/console 2>&1
	     else
		     $BERROR -f " " -e 25 -a 1 >/dev/console 2>&1
	     fi
	fi

	# Restart the updating of the screen now that processing has resumed.
	if [ -s /../Update_Status.pn ]
	then
		kill -CONT `cat /../Update_Status.pn`
	fi

	return 0
}

# Call_Merge_Methods:
#     During migration install, this calls the routine which merges the saved
#     files from the previous system.
#     During preservation install, this moves the preserved filesystems back
#     to /etc.
#     syntax:  Call_Merge_Methods
#     Always returns 0.
function Call_Merge_Methods
{
	trap 'BI_Error "BOS Install" 29 2' INT
# If this is a migration install, call the first phase of the merge utilities.
	INST_METH=`bidata -b -g control_flow -f INSTALL_METHOD`
	if [ "$INST_METH" = migrate ]
	then
		# If booted from tape, move migration files from /usr/lpp/bos to
		# /tmp/bos.  If booted from CDROM or network, unarchive migration
		# files from /usr/lib/bos/liblpp.a and leave them in /usr/lib/bos.
		cd /tmp/bos
		ar x /usr/lpp/bos/liblpp.a bos.rte.post_i
		cd /

# 351
		/tmp/bos/bos.rte.post_i
		rc="$?"
		case "$rc" in
			# Error in merging the saved and new files.  The running of the
			# system should not be impaired;  BOS install can continue.
			1 ) BI_Error "BOS Install" 33 1 ;;
			# Error in merging the saved and new files.  A successful BOS
			# installation cannot be completed with the new version of the file
			# left in place;  halt BOS install processing with a fatal error.
			2 ) BI_Error "BOS Install" 34 2 ;;
		esac
		rm /tmp/bos/bos.rte.post_i
	elif [ "$INST_METH" = preserve ]
	then
	## cp the preserved files back to the system
		PRE_LIST=/..$PRE_LIST     # Use the list in RAM.
		export PRE_LIST
		grep -v '^#' "$PRE_LIST" | \
			while read FILE_NAME other
			do
			   if [ -n "$FILE_NAME" ]
			   then
				if [ "$FILE_NAME" != "/etc/filesystems" ]
				then
				   if [ -d "/tmp/bos/$FILE_NAME" ]
				   then
					mkdir -p ${FILE_NAME} 2>/dev/null
				   else
					mkdir -p ${FILE_NAME%/*} 2>/dev/null
					cp /tmp/bos/$FILE_NAME  $FILE_NAME 2>/dev/null
				   fi
				fi
			   fi
			done
# 361
# TBD Should merge of filesystems be handled differently.
		if [ "$BOSLEVEL" != 3.1 ]
		then
			cp /tmp/bos/etc/filesystems /etc/ 2>/dev/null
		else
			grep -vp '^/:$' /tmp/bos/etc/filesystems | \
				grep -vp '^/usr:$' >/tmp/filesystems.new
			echo >>/tmp/filesystems.new
			grep -p '^/:$' /etc/filesystems >>/tmp/filesystems.new
			echo >>/tmp/filesystems.new
			grep -p '^/usr:$' /etc/filesystems >>/tmp/filesystems.new
			echo >>/tmp/filesystems.new
			grep -p '^/var:$' /etc/filesystems >>/tmp/filesystems.new
			echo >>/tmp/filesystems.new
			cp /tmp/filesystems.new /etc/ 2>/dev/null
		fi
	fi
	return 0
}

# Clean_Migrated_VPD
#     During 4.1->4.1 migration install, this routine removes
#     the software vital product databases information for the
#	  kernel so that it will always get reinstalled.
#
#     syntax:  Clean_Migrated_VPD
#     Always returns 0.
function Clean_Migrated_VPD
{
	trap 'BI_Error "BOS Install" 29 2' INT
	if [ "$BOSLEVEL" != "3.2" ]
	   then
  			# We should always install the new kernel on a 4.1->4.* migration
  			# Figure out which kernel needs to be deinstalled,
			# clean out the SWVPD information, and remove the 
			# link to /usr/lib/boot/unix
		   	for odmdir in /etc/objrepos /usr/lib/objrepos
		   	do
			    
				UP=0
				# Install either the up or the mp package.
				if [ "`bootinfo -z 2>/dev/null`" != "$UP" ]
				then
					# Deinstall mp package.
					KERNELPKG=bos.rte.mp
				else
					# Deinstall up package.
					KERNELPKG=bos.rte.up
				fi
		   	   # Get lpp_ids for the kernel package
		   	   ODMDIR=$odmdir \
				 odmget -q "name = $KERNELPKG" lpp 2>/dev/null |
				 grep "lpp_id = " | cut -d' ' -f3 > /tmp/dev.deinst.$$

		   	   # Remove the product entry
		   	   ODMDIR=$odmdir odmdelete -o product \
		   	   		 -q "lpp_name = $KERNELPKG" >/dev/null 2>&1 

		   	   # Remove the lpp entry
		   	   ODMDIR=$odmdir odmdelete -o lpp \
		   	   		 -q "name = $KERNELPKG" >/dev/null 2>&1 

		   	   # Remove the history and inventory entries
		   	   cat /tmp/dev.deinst.$$ 2>/dev/null |
		   	   while read lpp_id
		   	   do
		   	      ODMDIR=$odmdir odmdelete -o history \
		   	   		 -q "lpp_id=$lpp_id" >/dev/null 2>&1 
		   	      ODMDIR=$odmdir odmdelete -o inventory \
		   	   		 -q "lpp_id=$lpp_id" >/dev/null 2>&1 
		   	   done
		   	   rm -f /tmp/dev.deinst.$$ 
			done

			# Remove the link so that bos.rte.[um]p can recreate it when it
			# is installed.
			rm -f /usr/lib/boot/unix

		fi
		ODMDIR=/usr/lib/objrepos odmdelete -o inventory \
					-q "loc0=/usr/lpp/bos/migrate" >/dev/null 2>&1
	return 0
}

# Change_Mounts:
#     Unmount hard disk file systems from mount points and mount them over /.
#     syntax:  Change_Mounts
#     Always returns 0.
function Change_Mounts
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 36 'Over mounting /.\n'`"

	# Make the /usr/lib directory in /dev/hd4 a symbolic link to the /usr/lib
	# directory in the RAM file system so mount will work.
	PT=`bidata -i -g image_data -f PRODUCT_TAPE`
	if [ "$PT" = yes ]
	then
		ln -sf /../usr/lib $MROOT/usr/lpp/bos/inst_root/usr/lib
# 348
		if [ "$BOOTTYPE" -eq "$CDROM" -o "$BOOTTYPE" -eq "$NETWORK" ]
		then
			ln -sf /../SPOT $MROOT/usr/lpp/bos/inst_root/SPOT
		fi
	else
		mkdir /foo
		mount $MROOT /foo
		ln -sf /../usr/lib /foo/usr/lib
		umount /foo
		rmdir /foo

# 348
		if [ "$BOOTTYPE" -eq "$NETWORK" ]
		then
			ln -sf /../SPOT $MROOT/SPOT
		fi
	fi

	# Must unmount everything in reverse order, so get the list of mounted file
	# systems and reverse it.
	cd /
	COUNT=`wc -l < /mountlist`
	while [ "$COUNT" -gt 0 ]
	do
		$MROOT/usr/bin/sed -n "${COUNT}p" /mountlist >>/umountlist
		(( COUNT = COUNT - 1 ))
	done

# 349
	# Set PATH and LIBPATH to find only those things in RAM while unmounting
	# all of disks file systems.
	# PATH and LIBPATH will be different, depending on the boot device.
	case "$BOOTTYPE" in
	$NETWORK | $CDROM ) LIBPATH=/usr/ccs/lib:/usr/lib:/SPOT/usr/ccs/lib:/SPOT/usr/lib:
			PATH=/usr/bin:/usr/sbin:/usr/lpp/bosinst:/etc:/SPOT/usr/bin:/SPOT/usr/sbin:/SPOT/usr/lpp/bosinst:
	;;
	$TAPE ) LIBPATH=/usr/ccs/lib:/usr/lib:
			PATH=/usr/bin:/usr/sbin:/usr/lpp/bosinst:/etc:/../usr/lpp/bosinst:
	;;
	esac
	export IFS LIBPATH PATH

	Change_Status 

# 347
	if [ "$PT" = yes ]
	then
		cp /mnt/usr/lpp/bos/inst_root/etc/filesystems /etc
	else
		cp /mnt/etc/filesystems /etc
	fi

	for i in `cat /umountlist`
	do
		umount $i

	done

	Change_Status

	# mount /, /usr, /var, and /tmp
	mount -f -t bootfs
	mount /tmp >/dev/null 2>&1  # Output to /dev/null in case there is no /tmp.

	# start the sync daemon
	[ -x /SPOT/usr/sbin/syncd ] && nohup /SPOT/usr/sbin/syncd 60 > /dev/null 2>&1 &

	# recover NIM files from RAM filesystem
	if [ "$BOOTTYPE" -eq "$NETWORK" ]
	then
		$NIM_BOSINST_RECOVER
	fi

	Change_Status

	# Mount all non-system filesystems in the root vg.
	for fs_name in `bidata -i -g fs_data -f FS_NAME`
	do
		case "$fs_name" in
			/|/usr|/var|/tmp )
				:		# Do nothing, since they are already mounted.
			;;
			* ) # Anything else, mount it.
				fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v $fs_name`
				mount -o log=/dev/hd8 $fs_lv $fs_name
			;;
		esac
	done

	unset MROOT
	export MROOT

	return 0
}

# Change_Status
#     Updates the status indicator to display percentage complete and time
#     elapsed since the user exited the menus.
#     syntax:  Change_Status
#     Always returns 0.
function Change_Status
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$1" != '-f' ]
	then
		(( PERCENT += 4 ))
		BAR=$BAR'*'
		echo $1 >/../wip
		echo $1
	else
		PERCENT=100
	fi
	echo $PERCENT >/../percent

	if [ "$BOOTTYPE" -eq "$NETWORK" ] && [ "$1" != "-f" ]
	then
		if [ -z "$1" ]
		then
			${NIMCLIENTPATH}nimclient -o change -a force=yes \
					-a ignore_lock=yes \
					-a info="BOS install percent complete = $PERCENT"
		else
			${NIMCLIENTPATH}nimclient -o change -a force=yes \
					-a ignore_lock=yes -a info="$1"
		fi
	fi

	if [ "$REALMEM" -le 8192 ]
	then
		Update_Status
	fi

	return 0
}

# Check_Other_Stanzas:
#     The following function will ensure that the non-disk releated stanzas in
#     bosinst.data are valid.
#     syntax:  Check_Other_Stanzas
#     Always returns 0.
function Check_Other_Stanzas
{
	trap 'BI_Error "BOS Install" 29 2' INT

# 210
	ESO=`bidata -b -g control_flow -f EXISTING_SYSTEM_OVERWRITE`
	P=`bidata -b -g control_flow -f PROMPT`
	if [ "$P" = yes ]
	then # 211
# 218
		# Set EXISTING_SYSTEM_OVERWRITE to yes when prompt = yes since the
		# user will be asked to verify which disks to place in the rvg.
		bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes

# 219 # 224 # 225 # 226
		# If INSTALL_METHOD is not migrate, preserve, or overwrite, determine
		# what it should be based on machine configuration and set it.
		IM=`bidata -b -g control_flow -f INSTALL_METHOD`
		if [ "$IM" != migrate -a "$IM" != preserve -a "$IM" != overwrite ]
		then
			MIGRATABLE=
			PRESERVABLE=
			# Open file descriptor 3 for read.
			exec 3< $TARGETVGS

			# The following loop will stop when it finds the first root volume
			# group of level 3.2   If it finds a rvg of level 3.1 or 4.1, it
			# will keep looking to see if a 3.2 rvg exists.
			# 4.1.1->4.1.1 migration is illegal.
			# 4.1->4.1.1 migration is legal. @@@
			while read -u3 VGID INVG LEVEL NAME LOC SIZE BOOTABLE
			do
				if [[ ($LEVEL > "3.2" || "$LEVEL" = "3.2") && "$LEVEL" < "$CURLEVEL" ]]
				then
					MIGRATABLE=TRUE
					break
				else
					PRESERVABLE=TRUE
				fi
			done

			# Close file descriptor 3.
			exec 3<&-

			if [ "$MIGRATABLE" ]
			then
				bidata -b -m control_flow -f INSTALL_METHOD -e migrate
			elif [ "$PRESERVABLE" ]
			then
				bidata -b -m control_flow -f INSTALL_METHOD -e preserve
			else
				bidata -b -m control_flow -f INSTALL_METHOD -e overwrite
			fi
		fi

		# If RUN_STARTUP is neither yes nor no, set it to yes.
		RS=`bidata -b -g control_flow -f RUN_STARTUP`
		if [ "$RS" != yes -a "$RS" != no ]
		then
			bidata -b -m control_flow -f RUN_STARTUP -e yes
		fi

		# If INSTALL_X_IF_ADAPTER is neither yes nor no, set it to yes.
		IX=`bidata -b -g control_flow -f INSTALL_X_IF_ADAPTER`
		if [ "$IX" != yes -a "$IX" != no -a "$IX" != all ]
		then
			bidata -b -m control_flow -f INSTALL_X_IF_ADAPTER -e yes
		fi

		# If any of the locale fields are null, put the value that was there
		# from a previous root volume group.
		PT=`bidata -i -g image_data -f PRODUCT_TAPE`
		# Only change locale if this is not a mksysb install because the
		# values in the previous root volume group may not be applicable.
		if [ "$PT" = yes ]
		then
			# Find the first disk in the first root volume group.
			DN= IN_RVG=
			cat "$TARGETVGS" | while read id IN_RVG level NAME other
				do
					if [ "$IN_RVG" -eq 1 ]
					then
						DN=$NAME
						break
					fi
				done
			# If the first disk in the list of target disks is in a rootvg,
			# then the values for locale previously saved can be placed in
			# locale stanza.
			if [ -n "$DN" ]
			then
				LEVEL=`blvset -d /dev/$DN -g level`
				if [ "$LEVEL" != 3.1 -a "$LEVEL" != 3.2 ]
				then # The level is 4.1 or greater.
					export IFS=":"
					set -- `blvset -d /dev/$DN -g menu`
					export IFS=" 	
"
				else # The level is 3.2 or less.
				# This is not a very reliable way to get the previous locale
				# since it used to be stored as an English text string.  If
				# the text string changes from 3.2 to later releases, this may
				# not match them.  However, this is not fatal, since these
				# values are only to be used as the defaults.
					export IFS="
"
					set -- `blvset -d /dev/$DN -g menu`
					export IFS=" 	
"
					LOCALE=`odmget \
								-q"text_string = '$1' and bosinst_menu = y" CC \
							| grep 'locale = ' | cut -d'"' -f2`
					set -- $LOCALE $LOCALE $LOCALE $LOCALE
				fi

				# By this time, BOSINST_LANG, CULTURAL_CONVENTION, MESSAGES,
				# and KEYBOARD will be set in $1, $2, $3, and $4.  If any of
				# them were already set in the bosinst.data file, don't change
				# the value to what was on the previous rvg.
				if [ -z "`bidata -b -g locale -f BOSINST_LANG`" ]
				then
					bidata -b -m locale -f BOSINST_LANG -e "$1"
				fi
				if [ -z "`bidata -b -g locale -f CULTURAL_CONVENTION`" ]
				then
					bidata -b -m locale -f CULTURAL_CONVENTION -e "$2"
				fi
				if [ -z "`bidata -b -g locale -f MESSAGES`" ]
				then
					bidata -b -m locale -f MESSAGES -e "$3"
				fi
				if [ -z "`bidata -b -g locale -f KEYBOARD`" ]
				then
					bidata -b -m locale -f KEYBOARD -e "$4"
				fi
			fi
		fi

		# If BOSINST_LANG is not set correctly, set it to null.
		BL=`bidata -b -g locale -f BOSINST_LANG`
		BL_OK=`odmget -q"locale=$BL AND bosinst_translated=y" MESSAGES 2>/dev/null`
		if [ "$NOT" "$BL_OK" ]
		then
			bidata -b -m locale -f BOSINST_LANG -e ''
		fi

		# If CULTURAL_CONVENTION is not set correctly, set it to null.
		CC=`bidata -b -g locale -f CULTURAL_CONVENTION`
		CC_OK=`odmget -q"locale=$CC AND bosinst_menu=y" CC 2>/dev/null`
		if [ "$NOT" "$CC_OK" ]
		then
			bidata -b -m locale -f CULTURAL_CONVENTION -e ''
		fi

		# If MESSAGES is not set correctly, set it to null.
		MSG=`bidata -b -g locale -f MESSAGES`
		MSG_OK=`odmget -q"locale=$MSG AND bosinst_menu=y" MESSAGES 2>/dev/null`
		if [ "$NOT" "$MSG_OK" ]
		then
			bidata -b -m locale -f MESSAGES -e ''
		fi

		# If KEYBOARD is not set correctly, set it to null.
		KBD=`bidata -b -g locale -f KEYBOARD`
		KBD_OK=`odmget -q"keyboard_map=$KBD AND bosinst_menu=y" KEYBOARD 2>/dev/null`
		if [ "$NOT" "$KBD_OK" ]
		then
			bidata -b -m  locale -f KEYBOARD -e ''
		fi
	else # 212
		# non-prompt mode.
		# The following loop will determine if there are any volume groups
		# already existing on the disks targeted for the root volume group.
		# If there are and EXISTING_SYSTEM_OVERWRITE = no, BOS install will
		# flag an error since this is the non prompt path.
# 213
		if [ "$ESO" != yes ]
		then
			for D in `bidata -b -g target_disk_data -f LOCATION`
			do
				INVG=`grep " $D " $TARGETVGS | cut -d' ' -f2`
				if [ "$INVG" != 0 ]
				then # 214
					# If ERROR_EXIT is set, run it and then flag that user must
					# be prompted.  If they want to stop before the menus, they
					# may do so with the error exit.
					error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
					if [ -n "$error_exit" ]; then "$error_exit"; fi
# 216
					ERRORTEXT="The bosinst.data file indicated that you do not want to
overwrite an existing system, but disks selected in the
bosinst.data file specify disks which contain data."
					ERRORNUM=22
# 217
					bidata -b -m control_flow -f PROMPT -e yes
					bidata -b -mcontrol_flow -fEXISTING_SYSTEM_OVERWRITE -eyes
				fi # 215
			done
		fi

# 220 # 221 # 222 # 223 # 227
		# If INSTALL_METHOD is not migrate, preserve, or overwrite, determine
		# what it should be based on machine configuration and set it.
		IM=`bidata -b -g control_flow -f INSTALL_METHOD`
		if [ "$IM" != migrate -a "$IM" != preserve -a "$IM" != overwrite ]
		then
			MIGRATABLE=
			PRESERVABLE=
			# Open file descriptor 3 for read.
			exec 3< $TARGETVGS

			# The following loop will stop when it finds the first root volume
			# group of level 3.2 or 4.1.  If it finds a rvg of level 3.1, it
			# will keep looking to see if a 3.2 or 4.1 rvg exists.
			# 4.1.1->4.1.1 migration is illegal.
			# 4.1->4.1.1 migration is legal. @@@
			while read -u3 VGID INVG LEVEL NAME LOC SIZE BOOTABLE
			do
				if [[ ($LEVEL > "3.2" || "$LEVEL" = "3.2") && "$LEVEL" < "$CURLEVEL" ]]
				then
					MIGRATABLE=TRUE
					break
				else
					PRESERVABLE=TRUE
				fi
			done

			# Close file descriptor 3.
			exec 3<&-

			if [ "$MIGRATABLE" ]
			then
				bidata -b -m control_flow -f INSTALL_METHOD -e migrate
			elif [ "$PRESERVABLE" ]
			then
				bidata -b -m control_flow -f INSTALL_METHOD -e preserve
			else
				bidata -b -m control_flow -f INSTALL_METHOD -e overwrite
			fi

			# If ERROR_EXIT is set, run it and then flag that user must
			# be prompted.  If they want to stop before the menus, they
			# may do so with the error exit.
			error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
			if [ -n "$error_exit" ]; then "$error_exit"; fi

			ERRORTEXT="    Invalid install method %s specified in the
    bosinst.data file."
			ERRORARG="$IM"
			ERRORNUM=23
			bidata -b -m control_flow -f PROMPT -e yes

		fi

		# If RUN_STARTUP is neither yes nor no, set it to yes.
		RS=`bidata -b -g control_flow -f RUN_STARTUP`
		if [ "$RS" != yes -a "$RS" != no ]
		then
			# If ERROR_EXIT is set, run it and then flag that user must
			# be prompted.  If they want to stop before the menus, they
			# may do so with the error exit.
			error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
			if [ -n "$error_exit" ]; then "$error_exit"; fi

			bidata -b -m control_flow -f RUN_STARTUP -e yes
			ERRORTEXT="    Invalid startup flag %s specified in the
    bosinst.data file."
			ERRORARG="$RS"
			ERRORNUM=24
			bidata -b -m control_flow -f PROMPT -e yes
		fi

		# If INSTALL_X_IF_ADAPTER is neither yes nor no, set it to yes.
		IX=`bidata -b -g control_flow -f INSTALL_X_IF_ADAPTER`
		if [ "$IX" != yes -a "$IX" != no -a "$IX" != all ]
		then
			# If ERROR_EXIT is set, run it and then flag that user must
			# be prompted.  If they want to stop before the menus, they
			# may do so with the error exit.
			error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
			if [ -n "$error_exit" ]; then "$error_exit"; fi

			bidata -b -m control_flow -f INSTALL_X_IF_ADAPTER -e yes
			ERRORTEXT="Invalid flag (%s) specified in the bosinst.data file
regarding the installation of X11."
			ERRORARG="$IX"
			ERRORNUM=25
			bidata -b -m control_flow -f PROMPT -e yes
		fi

		PT=`bidata -i -g image_data -f PRODUCT_TAPE`
		# If BOSINST_LANG is not set correctly, set it to C locale.
		BL=`bidata -b -g locale -f BOSINST_LANG`
		BL_OK=`odmget -q"locale=$BL AND bosinst_translated=y" MESSAGES 2>/dev/null`
		if [ "$NOT" "$BL_OK" -a "$PT" = yes ]
		then
            if [ -n "$BL" -a "$BL" != C ]
            then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				ERRORTEXT="Invalid BOS install language (%s) specified in the
bosinst.data file."
				ERRORARG="$BL"
				ERRORNUM=26
				bidata -b -m control_flow -f PROMPT -e yes
			fi
			bidata -b -m locale -f BOSINST_LANG -e ''
		fi

		# If CULTURAL_CONVENTION is not set correctly, set it to C locale.
		CC=`bidata -b -g locale -f CULTURAL_CONVENTION`
		CC_OK=`odmget -q"locale=$CC AND bosinst_menu=y" CC 2>/dev/null`
		if [ "$NOT" "$CC_OK" -a "$PT" = yes ]
		then
            if [ -n "$CC" -a "$CC" != C ]
            then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				ERRORTEXT="Invalid cultural convention (%s) specified in the
bosinst.data file."
				ERRORARG="$CC"
				ERRORNUM=27
				bidata -b -m control_flow -f PROMPT -e yes
			fi
			bidata -b -m locale -f CULTURAL_CONVENTION -e ''
		fi

		# If MESSAGES is not set correctly, set it to C locale.
		MSG=`bidata -b -g locale -f MESSAGES`
		MSG_OK=`odmget -q"locale=$MSG AND bosinst_menu=y" MESSAGES 2>/dev/null`
		if [ "$NOT" "$MSG_OK" -a "$PT" = yes ]
		then
            if [ -n "$MSG" -a "$MSG" != C ]
            then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				ERRORTEXT="Invalid message catalogs (%s) specified in the
bosinst.data file."
				ERRORARG="$MSG"
				ERRORNUM=28
				bidata -b -m control_flow -f PROMPT -e yes
			fi
			bidata -b -m locale -f MESSAGES -e ''
		fi

		# If KEYBOARD is not set correctly, set it to C locale.
		KBD=`bidata -b -g locale -f KEYBOARD`
		KBD_OK=`odmget -q"keyboard_map=$KBD AND bosinst_menu=y" KEYBOARD 2>/dev/null`
		if [ "$NOT" "$KBD_OK" -a "$PT" = yes ]
		then
            if [ -n "$KBD" -a "$KBD" != C ]
            then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				ERRORTEXT="Invalid keyboard map (%s) specified in the
bosinst.data file."
				ERRORARG="$KBD"
				ERRORNUM=29
				bidata -b -m control_flow -f PROMPT -e yes
			fi
			bidata -b -m locale -f KEYBOARD -e ''
		fi
	fi

	return 0
}

# Create_Bosinst_Data is called if the bosinst.data file does not exist.  This
# is not a likely situation, but it could happen.  This will create a default
# bosinst.data.
#     Always returns 0.
function Create_Bosinst_Data
{
	trap 'BI_Error "BOS Install" 29 2' INT
# 29
	echo '
control_flow:
    CONSOLE =
    INSTALL_METHOD =
    PROMPT = yes
    EXISTING_SYSTEM_OVERWRITE = no
    INSTALL_X_IF_ADAPTER = yes
    RUN_STARTUP = yes
    RM_INST_ROOTS = no
    ERROR_EXIT =
    CUSTOMIZATION_FILE =

target_disk_data:
    LOCATION =
    SIZE_MB =
    HDISKNAME =

locale:
    BOSINST_LANG =
    CULTURAL_CONVENTION =
    MESSAGES =
    KEYBOARD =' > "$BOSINSTDATA"

	return 0
}

# Display_Status_Screen:
#     Displays install status after user has exited menus.
#     syntax:  Display_Statsu_Screen
#     Always returns 0.
function Display_Status_Screen
{
	trap 'BI_Error "BOS Install" 29 2' INT
	if [ "$1" = '-c' ]
	then
		echo "$CLEAR" >/dev/console
		/usr/lpp/bosinst/berror -f " " -e 24 -a 1 > /dev/console
		kill -9 `cat /../Update_Status.pn`
	else
		START_TIME=$SECONDS
		echo "$CLEAR"
		IM=`bidata -b -g control_flow -f INSTALL_METHOD`
		if [[ $IM = migrate ]]
		then
			/usr/lpp/bosinst/berror -f " " -e 36 -a 1
		
		elif [ `bootinfo -k` != 3 ]
		then
			/usr/lpp/bosinst/berror -f " " -e 30 -a 1
		else
			/usr/lpp/bosinst/berror -f " " -e 25 -a 1
		fi

		echo $PERCENT >/../percent
		echo '' >/../wip

		if [ -z "$NIM_DEBUG"  -a "$REALMEM" -gt 8192 ]
		then

			Update_Status &
			echo $! >/Update_Status.pn

		fi

	fi

	return 0
}

# Extract_Diskette_Data will try to restore a signature file.  If one exists
# and contains the word "data", it will restore the entire diskette.  If the
# image.data file was on that diskette, it will initialize the
# IMAGE_ON_DISKETTE variable.
#     Always returns 0.
function Extract_Diskette_Data
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$BOOTTYPE" -eq "$NETWORK" ]
	then
		${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
			-a info="extract_diskette_data"
	fi

	cd /
# 6
	rm -f ./signature
	restbyname -xqSf /dev/rfd0 ./signature >/dev/null 2>&1
	if [ "$?" -eq 0 -a -s ./signature ]
	then
		read a b <./signature || a=none
		if [ "$a" = data ]
		then
# 7
			# If image.data file is on the diskette, set IMAGE_ON_DISKETTE.
			unset IMAGE_ON_DISKETTE
			restbyname -xqf /dev/rfd0 | grep "$IMAGEDATA" >/dev/null 2>&1
			[ "$?" -eq 0 ] && IMAGE_ON_DISKETTE=TRUE
			# If preserve.list file is on the diskette, set PRESERVE_ON_DISKETTE.
			unset PRESERVE_ON_DISKETTE
			restbyname -Tqf /dev/rfd0 | grep "$PRE_LIST" >/dev/null 2>&1
			[ "$?" -eq 0 ] && PRESERVE_ON_DISKETTE=TRUE
		fi
	else
		if [ -f /usr/bin/dosread ]
		then
			/usr/bin/dosread -S /preload /preload >/dev/null 2>&1
			if [ $? -eq 0 ]
			then 
				/usr/bin/ksh /preload
			fi
		fi
	fi

	return 0
}

# Fill_Target_Stanzas:
#     Ensure that all fields in the target_disk_data stanzas are filled.
#     They may not be if the bosinst.data file had target_disk_data stanzas
#     filled in by the user.
#     syntax:  Fill_Target_STanzas
#     Always returns 0.
function Fill_Target_Stanzas
{
	trap '/usr/bin/ksh' INT

	# Keep a temporary list of disks so can keep track of which ones have
	# been selected.
	lsdev -Ccdisk -S Available -F ':name:' > /tmp/disks

	rm -f /tmp/tdd.add
# 157
	bidata -G | while read TDD_STANZA
	do
		LOC=`echo $TDD_STANZA | cut -d: -f1`
		SIZE=`echo $TDD_STANZA | cut -d: -f2`
		NAME=`echo $TDD_STANZA | cut -d: -f3`

		# Start by checking LOCATION field for this stanza.  If it is set,
		# check to see if there is a disk at that location.  If there is,
		# ensure that the size and disk names are set correctly.  If there is
		# not a disk at that location, flag an error.  If there is no disk at
		# that location, remove the stanza.
# 158
		if [ -n "$LOC" ]
		then # 159
# 161
			IT_IS_THERE=`lsdev -Ccdisk -S Available -F ':location:' | \
				grep ":$LOC:"`
			if [ "$IT_IS_THERE" ]
			then # 162
# 164
				NAME=`bootinfo -o "$LOC"`
				echo "$LOC" `bootinfo -s $NAME` "$NAME" >>/tmp/tdd.add
			else # 163
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

# 165
				ERRORTEXT="An invalid disk (%s) was specified in the
location field of the data file."
				ERRORARG="$LOC"
				ERRORNUM=16
# 166
				bidata -b -m control_flow -f PROMPT -e yes
				bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes

				# Now remove the stanza from the list.
				bidata -d LOCATION -v $LOC
			fi
		else # 160
# 167
			if [ -n "$SIZE" ]
			then # 168
# 170
				if [ "$SIZE" = 'largest' ]
				then # 171
					LARGEST=X
					for DISK in `cut -d: -f2 /tmp/disks`
					do
						CS=`bootinfo -s "$DISK"`
						LS=`bootinfo -s "$LARGEST"`
						# If the size of the current disk is larger than the
						# size of that specified by LARGEST, tag DISK as the
						# largest.  Do this comparison for all disks.
						if [ "$CS" -gt "$LS" ]
						then
							LARGEST=$DISK
						fi
					done
# 173
					# If there are any remaining disks, initialize the
					# target_disk_data stanza for this disk to contain the
					# location, size, and hdisk name for the disk specified by
					# LARGEST.
					if [ "$LARGEST" != X ]
					then # 175
# 179
						echo `bootinfo -o $LARGEST` `bootinfo -s $LARGEST` \
													"$LARGEST" >>/tmp/tdd.add

						# Now take out the disk from the list.
						grep -v ":$LARGEST:" /tmp/disks >/tmp/d
						mv /tmp/d /tmp/disks
					# else There is no else.
						# What this means at this point is that there are no
						# more configured disks.
						# This will not be considered an error since the person
						# making the bosinst.data may not always know how many
						# disks will be on the target machine.  They may put
						# a goodly amount of target_disk_data stanzas with
						# the word "largest" in the SIZE_MB field, and be
						# guaranteed that they get all the disks on the machine.
					fi # 176
				else # 172
# 174
					# The following case ensures that SIZE is a number.
					case "$SIZE" in
						[1-9]*([0-9]) ) # 177
							BEST=
							for DISK in `cut -d: -f2 /tmp/disks`
							do
								CS=`bootinfo -s $DISK`
								# If $BEST is invalid or null, assign a very
								# large number to BS.
								BS=`bootinfo -s "$BEST" || echo $MEG`

								# If the size of the current DISK is bigger than
								# the size specified in SIZE but smaller than
								# the size of the disk pointed to by BEST
								# save current value of DISK into BEST.
								if [ "$CS" -ge "$SIZE" -a "$CS" -lt "$BS" ]
								then
									BEST=$DISK
								fi
							done

							# If there are any remaining disks of the proper
							# size (ie, BEST is not null), initialize the
							# target_disk_data stanza for this disk to contain
							# the location, size, and hdisk name for the disk
							# specified by BEST.
# 182
							if [ -n "$BEST" ]
							then # 183
# 185
								echo `bootinfo -o $BEST` \
										`bootinfo -s $BEST` \
											"$BEST" >>/tmp/tdd.add

								# Now take out the disk from the list.
								grep -v ":$BEST:" /tmp/disks >/tmp/d
								mv /tmp/d /tmp/disks
							else # 184
								# If ERROR_EXIT is set, run it and then flag
								# that user must be prompted.  If they want to
								# stop before the menus, they may do so with
								# the error exit.
								error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
								if [ -n "$error_exit" ]; then "$error_exit"; fi

								# Could not find a disk big enough.
# 186
	 							ERRORTEXT="A disk of size %s was specified in the
bosinst.data file, but there is not a disk of at least
that size on the system."
								ERRORARG="$SIZE"
								ERRORNUM=17
# 187
								bidata -b -m control_flow -f PROMPT -e yes
								bidata -b -m control_flow \
										-f EXISTING_SYSTEM_OVERWRITE -e yes
							fi
						;;
						* ) # 178
							# If ERROR_EXIT is set, run it and then flag
							# that user must be prompted.  If they want to
							# stop before the menus, they may do so with
							# the error exit.
							error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
							if [ -n "$error_exit" ]; then "$error_exit"; fi

# 180
							ERRORTEXT="The size field %s is not numeric."
							ERRORARG="$SIZE"
							ERRORNUM=18
# 181
							bidata -b -m control_flow -f PROMPT -e yes
							bidata -b -m control_flow \
										-f EXISTING_SYSTEM_OVERWRITE -e yes
						;;
					esac
				fi
			else # 169
# 188
				if [ -n "$NAME" ]
				then # 189
					IT_IS_THERE=`lsdev -Ccdisk -S Available -F ':name:' | \
						grep ":$NAME:"`
# 191
					if [ "$IT_IS_THERE" ]
					then # 192
# 194
						echo `bootinfo -o $NAME` \
								`bootinfo -s $NAME` \
									"$NAME" >>/tmp/tdd.add
					else # 193
							# If ERROR_EXIT is set, run it and then flag
							# that user must be prompted.  If they want to
							# stop before the menus, they may do so with
							# the error exit.
							error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
							if [ -n "$error_exit" ]; then "$error_exit"; fi

# 195
							ERRORTEXT="A disk of name %s was not found."
							ERRORARG="$NAME"
							ERRORNUM=19
# 196
							bidata -b -m control_flow -f PROMPT -e yes
							bidata -b -m control_flow \
									-f EXISTING_SYSTEM_OVERWRITE -e yes
					fi
				fi # 190
			fi
		fi
	done

	rm /tmp/disks

# 99
	# If the /tmp/tdd.add file exists, then there must have been at least some
	# valid disks specified in the target_disk_data stanzas.
	# Remove all existing stanzas and replace them with those listed in
	# /tmp/tdd.add.
	# If /tmp/tdd.add does not exist (the else part), then an error must have
	# occurred.  Remove any existing target_disk_data stanzas and call
	# Init_Target_Disks to fill them up with defaults.
	bidata -D			# Removes all target_disk_data stanzas.
	if [ -s /tmp/tdd.add ]
	then
		cat /tmp/tdd.add | while read LOC SIZE NAME
			do
				bidata -a -l $LOC -s $SIZE -n $NAME
			done
	else
		Init_Target_Disks
	fi

# 197
	IM=`bidata -b -g control_flow -f INSTALL_METHOD`
	if [ "$IM" = "preserve" -o "$IM" = "migrate" ]
	then # 198

		# Check to see if all disks specified in target_disk_data are in the
		# same volume group.

		AN_RVG=TRUE
		set -- `bidata -b -g target_disk_data -f HDISKNAME`
		D=$1
		PV_COUNT=`lqueryvg -p$D -c 2>/dev/null`
		VGID1=`lqueryvg -p $D -v 2>/dev/null`
		for D in `bidata -b -g target_disk_data -f HDISKNAME`
		do
			VGID2=`lqueryvg -p $D -v 2>/dev/null`
			if [ "$VGID1" != "$VGID2" ]
			then
				AN_RVG=
				break
			fi
		done

		# If all disks specified are in the same volume group, see if it
		# is a root vg.
		if [ "$AN_RVG" ]
		then
			# For bosinst purposes an RVG is defined as a VG containing the
			# following sets of LVs: (hd2,hd4,hd5,hd8).  Furthermore, all
			# PVs in the VG must be present. The LV hd9var is not required
			# because it only came into being in AIX3.2.
			lvcnt=0
			set -- `bidata -b -g target_disk_data -f HDISKNAME`
			D=$1
			set -- `lqueryvg -p$D -L 2> /dev/null`
			while [ -n "$1" ]
			do
				case "$2"
				in
					hd[2458])	(( lvcnt = ${lvcnt:=0} + 1 ))
								if [ "$2" = hd5 ]
								then
									lvid=$1
								fi
					;;
				esac
				shift 3
			done

			if [ "$lvcnt" -ne 4 ]
			then
				AN_RVG=
			fi
		fi

		# If all the disks specified in the target_disk_data stanzas are
		# part of the same root volume group AND if all disks which belong
		# to the root volume group have been selected, then check to ensure
		# that the disk containing the boot logical volume is first.
		TOTAL_TARGETS=`bidata -G | wc -l`

# 200
		if [ "$AN_RVG" -a "$TOTAL_TARGETS" -eq "$PV_COUNT" ]
		then # 201
# 204
			set -- `bidata -b -g target_disk_data -f HDISKNAME`
			D=$1
			pvid=`lquerylv -L "$lvid" -p$D -d 2>/dev/null | cut -d: -f1`
			# The above will return multiple lines of the same pvid.
			# The below will cut all but the first and place the result in
			# pvid.
			pvid=`echo $pvid | cut -d' ' -f1`
			bootdiskname=`getlvodm -g $pvid 2>/dev/null`
			if [ "$D" != "$bootdiskname" ]
			then # 206
# 208
				# Put the boot disk information first in the stanza list.
				OIFS="$IFS"
				IFS=:
				set -- `bidata -G | grep ":$bootdiskname$"`
				IFS="$OIFS"
				echo $* >/tmp/tdd.add

				# Now delete the boot disk stanza from the database.
				bidata -d HDISKNAME -v $bootdiskname

				# Now append the rest of the disk information to the end of the
				# stanza list.
				OIFS="$IFS"
				IFS=:
				bidata -G | while read LOC SIZE NAME
					do
						echo $LOC $SIZE $NAME
					done >>/tmp/tdd.add
				IFS="$OIFS"

				# Now delete the rest of the disk stanzas from the database
				# and add them again in the proper order.
				bidata -D
				cat /tmp/tdd.add | while read LOC SIZE NAME
					do
						bidata -a -l $LOC -s $SIZE -n $NAME
					done
				rm -f /tmp/tdd.add
			fi # 207
		else # 202
			# If ERROR_EXIT is set, run it and then flag that user must
			# be prompted.  If they want to stop before the menus, they
			# may do so with the error exit.
			error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
			if [ -n "$error_exit" ]; then "$error_exit"; fi

# 203
			ERRORTEXT="Disks specified in bosinst.data do not define a
root volume group."
			ERRORNUM=20
# 205
			bidata -b -m control_flow -f PROMPT -e yes
			bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes

# 139
			# Remove the target_disk_data stanzas and replace them with defaults
			bidata -D
			Init_Target_Disks
		fi

	else # 199
			# else this is an overwrite install.

		if [ "$NOT" "$ERRORTEXT" ]
		then
			# Ensure that a bootable disk comes first.
			BOOTDISK=
			for DISK in `bidata -b -g target_disk_data -f HDISKNAME`
			do
				IS_BOOTABLE=`bootinfo -B "$DISK"`
				if [ "$IS_BOOTABLE" -eq 1 ]
				then
					BOOTDISK=$DISK
					break
				fi
			done
# 386
			if [ -n "$BOOTDISK" ]
			then # 387
				# Swap the bootable stanza with the first stanza.
# 389
				# Put the boot disk information first in the stanza list.
				OIFS="$IFS"
				IFS=:
				set -- `bidata -G | grep ":$BOOTDISK"`
				IFS="$OIFS"
				echo $* >/tmp/tdd.add

				# Now delete the boot disk stanza from the database.
				bidata -d HDISKNAME -v $BOOTDISK

				# Now append the rest of the disk information to the end of the
				# stanza list.
				OIFS="$IFS"
				IFS=:
				bidata -G | while read LOC SIZE NAME
					do
						echo $LOC $SIZE $NAME
					done >>/tmp/tdd.add
				IFS="$OIFS"

				# Now delete the rest of the disk stanzas from the database
				# and add them again in the proper order.
				bidata -D
				cat /tmp/tdd.add | while read LOC SIZE NAME
					do
						bidata -a -l $LOC -s $SIZE -n $NAME
					done
			else # 388
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

# 390
				ERRORTEXT="The bosinst.data file does not specify any bootable disks."
				ERRORNUM=21
				bidata -b -m control_flow -f PROMPT -e yes
				bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
# 142
				# Remove the target_disk_data stanzas and replace them with
				# defaults.
				bidata -D
				Init_Target_Disks
			fi
		fi
	fi
}

# Finish:
#     Prompt to turn key, if necessary, Displays copyright and reboots.
#     syntax:  Finish
#     Does not return.
function Finish
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	Change_Status -f

	# if NIM_LOG is set, then /var/adm/ras is overmounted and no copies needed
	if [ -z "$NIM_LOG" ]
	then
#		mkdir -p /var/adm/ras >/dev/null 2>&1
		cat /../var/adm/ras/bi.log /var/adm/ras/bi.log | \
			alog -t bosinst -q -s 16384
		rm -f /var/adm/ras/bi.log
		cp /../var/adm/ras/BosMenus.log /var/adm/ras 2>/dev/null
		cp /../bosinst.data /var/adm/ras 2>/dev/null
		cp /../image.data /var/adm/ras 2>/dev/null
	fi
   
	C=`bidata -b -g control_flow -f CONSOLE`
	PLATFORM=`bootinfo -T`
	KEYPOS=`bootinfo -k`

	# It is important that NO bootinfo commands or other commands which
	# expect the RAM devices or the RAM ODM to be reflected on the disk
	# get run after this point on a migration, because the /dev entries 
	# from the original system are put back in place, leaving the RAM
	# devices and the disk devices out of sync.  Errors could occur if
	# boot commands are attempted to run without the RAM devices.

	# If this was a 4.1->4.* migration, we need to put the saved devices
	# back in place as late as possible in the installation
	IM=`bidata -b -g control_flow -f INSTALL_METHOD`
	if [ "$IM" = migrate  -a "$BOSLEVEL" != 3.2 ]
	then
		# Wipe out the devices copied down from the RAM filesystem
		rm -rf /dev >/dev/null 2>&1
		# Recover the original devices
		mkdir /dev
		chown root.system /dev
		chmod 775 /dev
		(cd /dev && restore -xqf /../dev.disk.bff >/dev/null 2>&1)
		# Remove any old trace devices 
		if [ "$BOSLEVEL" = "4.1" ]
		then
			rm -f /dev/systrace /dev/systrace[1-7] /dev/systrctl /dev/systrctl[1-7]
		fi
	fi

	if [ "$KEYPOS" -ne 3  -o "$DEVINSTALL_FAIL" = true ]
	then
		/usr/lib/methods/showled 0xA58
		echo "$CLEAR"
		/usr/lpp/bosinst/berror -f " " -e 26 -a 1

		/usr/lpp/bosinst/berror -f " " -e 27 -a 1
		if [ -n "$DEVINSTALL_FAIL" ]
		then
			dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 67 'An error occurred while migrating packages.\nSome packages have not been installed.\nPlease see /var/adm/ras/devinst.log for details or perform an overwrite \nor preservation install.'
		fi
		# Flush the input buffer before displaying this screen so that any input
		# entered before this will not be read at this time.
		/usr/lpp/bosinst/bi_io -f
		read ans
	fi

	cd /tmp
	ar x /usr/lpp/bos/liblpp.a
	echo $CLEAR
# 381
	cat *copyright*
	rm -f `ar t /usr/lpp/bos/liblpp.a`
	rm -f /liblpp.a /lpp_name /SPOT /../Update_Status.pn

	if [ "$BOOTTYPE" -eq "$NETWORK" ]
	then
		${NIMCLIENTPATH}nimclient -R success
		${NIMCLIENTPATH}nimclient -S shutdown
	fi

	sync;sync;sync
# 382
	bootlist; rc="$?"
	# If setable bootlist not supported and no console, then shutdown
	# else reboot
    if [ "$rc" -ne 0 -a "$C" = none  ]
	then
		shutdown -F
	else
		# If setable bootlist not supported then prompt for media removal
		# before rebooting
        if [ "$rc" -ne 0 -a "$BOOTTYPE" -ne "$NETWORK" ] 
		then
			if [ "$BOOTTYPE" -eq "$CDROM" ]
			then
            	/../SPOT/usr/bin/cdeject /dev/cd0
			fi
			echo "$CLEAR"
			/usr/lpp/bosinst/berror -f " " -e 26 -a 1
			dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 11 35 '\n        1. Remove the installation media.\n        2. Press the ENTER key to restart (reboot) the system.\n\n\n\n\n\n'

			# Flush the input buffer before displaying this screen so that any input
			# entered before this will not be read at this time.
			/usr/lpp/bosinst/bi_io -f
			read ans
		fi
		reboot -q
    fi
}

# Get_Data_Files will extract bosinst.data and image.data if we booted from
# CDROM or network.  If we booted from tape, these files have already been
# extracted.
#     Always returns 0.
function Get_Data_Files
{
	trap 'BI_Error "BOS Install" 29 2' INT
# 11
	case "$BOOTTYPE" in
	$CDROM ) # 12
		# Booted from CDROM.  /dev/cd? will be mounted over /SPOT.
		# Copy /SPOT/usr/lpp/bosinst/bosinst.template and
		# /SPOT/usr/lpp/bosinst/image.template to /.
        if [ -f /SPOT/usr/turbo/image.template ] ; then
                BOS_FORMAT=turbo
                cp /SPOT/usr/lpp/bosinst/bosinst.template $BOSINSTDATA 2>/dev/null
                cp /SPOT/usr/turbo/image.template "$IMAGEDATA"
        else
# 21
            cp /SPOT/usr/lpp/bosinst/bosinst.template $BOSINSTDATA 2>/dev/null
            cp  /SPOT/usr/lpp/bosinst/image.template $IMAGEDATA 2>/dev/null
        fi
	;;

	$NETWORK ) # 13
		${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
			-a info="extract_data_files"
		 # Booted from Network
# 14
		if [ "$NIM_BOS_IMAGE" -a "$NIM_BOS_FORMAT" ]
		then # 15
# 17
			case "$NIM_BOS_FORMAT" in
			rte )
				# Must have dot (.) on front of name so can extract from bff.
				restbyname -xqSf $NIM_BOS_IMAGE ."$BOSINSTDATA" ."$IMAGEDATA"
			;;
			mksysb )
				# Must have dot (.) on front of name so can extract from bff.
				restbyname -xqdSf $NIM_BOS_IMAGE ."$BOSINSTDATA" ."$IMAGEDATA" \
					./tmp/vgdata
			;;
			spot )
				cp /SPOT/usr/lpp/bosinst/bosinst.template "$BOSINSTDATA"
				cp /SPOT/usr/lpp/bosinst/image.template "$IMAGEDATA"
			;;
            turbo )
                BOS_FORMAT=turbo
                cp /SPOT/usr/lpp/bosinst/bosinst.template "$BOSINSTDATA"
                cp /SPOT/usr/turbo/image.template "$IMAGEDATA"
            ;;
			esac

		fi # 16

		# If there is a valid file specified by NIM_BOSINST_DATA, copy it to
		# bosinst.data. If there is NIM_IMAGE_DATA, copy it to image.data.
# 18
		if [ -s "$NIM_BOSINST_DATA" ]
		then # 19
# 22
			cp "$NIM_BOSINST_DATA" "$BOSINSTDATA"
		fi # 20

		if [ -s "$NIM_IMAGE_DATA" ]
		then
			cp "$NIM_IMAGE_DATA" "$IMAGEDATA"
		fi 
	;;
	esac

	# Copy the "preserve.list" file in the CD or NIM to /etc.
	cp /SPOT/usr/lpp/bos/inst_root/etc/preserve.list "$PRE_LIST" >/dev/null 2>&1

	return 0
}

# Get_Locale_Packages:  Determine names of packages to be isntalled to support
#     the selected locales.
#     syntax:  Get_Locale_Packages
#     Always returns 0.
function Get_Locale_Packages
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
	# Based on the variables CULTURAL_CONVENTION, MESSAGES, and KEYBOARD,
	# build a list of packages to install.  Get the package information from
	# the CC, MESSAGES, and KEYBOARD ODM databases.
	CC=`bidata -b -g locale -f CULTURAL_CONVENTION`
	MSG=`bidata -b -g locale -f MESSAGES`
	KBD=`bidata -b -g locale -f KEYBOARD`
	{ odmget -qlocale="$CC" CC 2>/dev/null; \
                odmget -qlocale="$MSG" MESSAGES 2>/dev/null; \
                odmget -qkeyboard_map="$KBD" KEYBOARD 2>/dev/null; } | \
		grep package | cut -d'"' -f2 | tr ' ' '\n' | tr '\t' '\n' | sort -u \
			>>/../tmp/device.pkgs
}

# Get_User_Input:
#     Gathers data from the user if in prompt mode, and validates the data
#     which was specified both from the user and from the bosinst.data file.
#     Runs pretest on selected disks.
#     syntax:  Get_User_Input
#     Always returns 0.
function Get_User_Input
{
	trap 'BI_Error "BOS Install" 29 2' INT

	status=`bidata -S`
    if [ "$status" -ne 0 ]
    then
        # If ERROR_EXIT is set, run it and then flag that user must
        # be prompted.  If they want to stop before the menus, they
        # may do so with the error exit.
        error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
        if [ -n "$error_exit" ]; then "$error_exit"; fi
        bidata -b -m control_flow -f PROMPT -e yes

        case "$status" in
        1 ) ERRORTEXT="Duplicate lv_data stanzas specified in the image.data
file.  The installation cannot continue because data
may be lost."
			FATAL=TRUE
            ERRORNUM=32 ;;
        2|3 ) ERRORTEXT="Duplicate fs_data stanzas specified in the image.data
file.  The installation cannot continue because data
may be lost."
			FATAL=TRUE
            ERRORNUM=33 ;;
        4 ) ERRORTEXT="Duplicate target_disk_data stanzas specified in the
bosinst.data file."
            ERRORNUM=34 ;;
        esac
    fi

	# Kill the Animate program that has been running since the console was set.
	kill -9 `cat /tmp/Animate.pn`
	rm /tmp/Animate.pn

	# If the user has typed 000 prior to this point, change to prompt mode.
	# If user typed 111 then invoke the shell for testing
	# If user typed 333 then force off turbo install mode (if on)
	/usr/lpp/bosinst/bi_io -t
	rc="$?"
	case "$rc" in
		0) bidata -b -m control_flow -f PROMPT -e yes ;;
		1) PS1='TEST SHELL >' /usr/bin/sh ;;
		3)	if [ "$BOS_FORMAT" = turbo ]
			then
				BOS_FORMAT=overflow
			fi 
			;;
	esac

	# If prompt = yes, unset all stanza variables and call menus.  Want to
	# unset the variables because the menus will be reading the same info
	# into its internal structures.  We do not want to malloc ram twice for
	# the same information.  When menus returns, reread the bosinst.data
	# and image.data files into structures to be used for the remainder of the
	# installation.
	NOTDONE=TRUE
	while [ "$NOTDONE" ]
	do
# 229
		P=`bidata -b -g control_flow -f PROMPT`
		if [ "$P" = yes ]
		then # 230
			/usr/lib/methods/showled 0xA48
# 232
			export KEYBOARD_CLASS=`/usr/lpp/bosinst/bi_io -k`

			# Tell data daemon to write out stanzas and exit.
			bidata -w -x

		{ BosMenus $CURLEVEL $BOSMENU_LANG; echo $? >/tmp/rc; } | tee /var/adm/ras/BosMenus.log
			rc=`cat /tmp/rc 2>/dev/null`
# 234
			if [ "$rc" -eq 1 ]
			then # 235
			# Remove a set of files which are no longer needed since this is
			# the point of no return.
				rm -fr /usr/lpp /usr/lib/nls /var/adm /etc/objrepos/FONT* \
						/bosinst.data /image.data /etc/objrepos/CC* \
						/etc/objrepos/KEYBOARD* /etc/objrepos/MESSAGES* \
						/usr/lib/microcode /usr/lib/drivers \
						/usr/lib/methods >/dev/null 2>&1
# 237
				cp /usr/lib/boot/getrootfs /etc
				/etc/init -c "chmod /etc/getrootfs 0777" >/dev/null 2>&1
				exec /etc/getrootfs
			elif [ "$rc" -eq 3 -o "$rc" -eq 4 ]
			then
				# If return code = 3, then they booted from one media and chose
				# to install a mksysb from another.  At this point, we must
				# re-exec bi_main so it can extract the data files and do the
				# error checking just as if it started after booting from the
				# mksysb tape.
				LANG=`grep '^[ 	]*BOSINST_LANG' $BOSINSTDATA`
				LANG=`echo ${LANG#*=}`
				BOSMENU_LANG="$LANG"
				IFS=' 	'
				set | while read ackgag
				do
					var=${ackgag%=*}
					if [ "$var" != SECONDS -a "$var" != RANDOM -a "$var" != var -a "$var" != IFS -a "$var" != LIBPATH -a "$var" != PATH -a "$var" != ODMDIR -a "$var" != ODMPATH -a "$var" != ERRNO -a "$var" != OPTIND -a "$var" != LINENO -a "$var" != OPTARG -a "$var" != PS1 -a "$var" != PS2 -a "$var" != PS3 -a "$var" != PS4 -a "$var" != PS5 -a -n "$var" -a "$var" != LANG -a "$var" != BOSMENU_LANG ]
					then
						unset ${var%=*} >/dev/null 2>/dev/null
					fi
				done

				exec /usr/lpp/bosinst/bi_main
			elif [ "$rc" -ne 0 ]
			then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				BOSMENU_LANG=`grep '^[  ]*BOSINST_LANG' $BOSINSTDATA`
				BOSMENU_LANG=`echo ${BOSMENU_LANG#*=}`
				ERRORTEXT="Encountered an unrecoverable error."
				ERRORNUM=30
				continue
			else # 236

			/usr/lpp/bosinst/datadaemon &
			# Wait to ensure the datadaemon reads in all the data before bidata
			# waits on the pipe.
			sleep 1
			BOSMENU_LANG="`bidata -b -g locale -f BOSINST_LANG`"
			fi
		else # 231
			# Else prompt = no.
			# Tell data daemon to write out stanzas, but do not exit.
			bidata -w
		fi

		# If a fatal error occurred (invalid/missing image.data file), we
		# cannot allow them to leave the menus loop.
		if [ "$FATAL" ]
		then
			bidata -b -m control_flow -f PROMPT -e yes
			continue
		fi

# 238
	       # Set LANG variable based on the BOSINST_LANG flag.
		export LANG=`bidata -b -g locale -f BOSINST_LANG`

		if [ "$ESERVER" -eq 0 ]
		then
			MODELTYPE=`bootinfo -M`
			rc="$?"
			if [ "$rc" -eq 102 -o "$rc" -eq 103 ] 
			then
				echo "$CLEAR\c" >/dev/console
				dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 78 \
'\n\n THIS OPERATING SYSTEM PRODUCT IS NOT SUPPORTED ON THIS\n\
HARDWARE CONFIGURATION.\n\
Contact your Product or Service Representative and report\n\
the model code shown below to assist in obtaining the correct operating\n\
system product for this hardware configuration.\n\
MODEL CODE: %s\n\n\
*** SYSTEM HALTED ***\n\n' $MODELTYPE >/dev/console
			loopled 0xA99
			fi
		fi

		Display_Status_Screen

# 273
		# Set the physical partition size.  Use the value from
		# the image.data file if it is not blank and is compatible
		# with the target disks.  
		PPSZ=`bidata -i -g vg_data -f PPSIZE`
		Set_PP_Size

# 239
		IM=`bidata -b -g control_flow -f INSTALL_METHOD`
		if [ "$IM" = 'overwrite' ]
		then # 240
# 242 # 243 # 244 # 245 # 246 # 247 # 248 # 249 # 250 # 251
			PT=`bidata -i -g image_data -f PRODUCT_TAPE`
			SHR=`bidata -i -g logical_volume_policy -f SHRINK`
			if [ "$PT" = no -a "$SHR" = yes ]
			then
				CheckSize -s
				rc="$?"
			else
				CheckSize
				rc="$?"
			fi
# 252
			if [ "$rc" -eq 0 ]
			then # 253
				NOTDONE=
			else # 254
				if [ "$BOS_FORMAT" = turbo ]
				then
					BOS_FORMAT=overflow
				else
					# If ERROR_EXIT is set, run it and then flag that user must
					# be prompted.  If they want to stop before the menus, they
					# may do so with the error exit.
					error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
					if [ -n "$error_exit" ]; then "$error_exit"; fi

# 255 # 256
					ERRORTEXT="Not enough disk space selected to contain the
operating system."
					ERRORNUM=31
					bidata -b -m control_flow -f PROMPT -e yes
					if [ -s /Update_Status.pn ]
					then
						kill -9 `cat /Update_Status.pn` >/dev/null 2>&1
					fi
				fi
			fi
		else # 241
# 257
			if [ "$IM" = preserve ]
			then # 258
# 260 # 261
			    CheckSize -p
			    rc="$?"
# 262
			    if [ "$rc" -eq 0 ]
			    then # 263
					NOTDONE=
			    else # 264
                    if [ "$BOS_FORMAT" = turbo ]
                    then
                        BOS_FORMAT=overflow
                    else
						# If ERROR_EXIT is set, run it and then flag that user must
						# be prompted.  If they want to stop before the menus, they
						# may do so with the error exit.
						error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
						if [ -n "$error_exit" ]; then "$error_exit"; fi

# 265 # 266
						ERRORTEXT="Not enough disk space selected to contain the operating system."
						ERRORNUM=31
						bidata -b -m control_flow -f PROMPT -e yes
						if [ -s /Update_Status.pn ]
						then
							kill -9 `cat /Update_Status.pn` >/dev/null 2>&1
						fi
			   		fi
				fi
			else
			   NOTDONE=
			fi # 259
		fi
		if [ "$BOS_FORMAT" = overflow ]
		then
			BOS_FORMAT=nonturbo
			# Tell data daemon to write out stanzas and exit.
			bidata -w -x
			# Use base image.template instead of turbo image.template
			cp  /SPOT/usr/lpp/bosinst/image.template $IMAGEDATA 2>/dev/null
			# Start the daemon which manages BOSINSTDATA and IMAGEDATA.
			/usr/lpp/bosinst/datadaemon &
			# Wait to ensure the datadaemon reads in all the data
			# before bidata waits on the pipe.
			sleep 1
			Set_PP_Size
			NOTDONE=
		fi	
# There is no test here for space on a migration install, since we know from
# 3.2 to 4.1 there will be enough and from 4.1 to 4.1, there should be enough.
# But this test must be done when doing migration from 4.1 to 4.2.

# 267
# 268
		# Run diagnostic pretests on disks to be in the root vg.
		if [ -x /usr/lpp/diagnostics/bin/diagdskpt ]
		then
			FAILED_DIAG=
			for i in `bidata -b -g target_disk_data -f HDISKNAME`
			do
				/usr/lpp/diagnostics/bin/diagdskpt $i
				rc="$?"
				if [ "$rc" -ne 0 ]
				then
					# Reset NOTDONE flag so it will show menus.
					NOTDONE=TRUE
					FAILED_DIAG="$FAILED_DIAG $i"
				fi
			done
			if [ "$FAILED_DIAG" ]
			then
				ERRORTEXT="The following disks failed the preliminary
diagnostic tests:
%s."
				ERRORARG="$FAILED_DIAG"
				ERRORNUM=35
				bidata -b -m control_flow -f PROMPT -e yes
				if [ -s /Update_Status.pn ]
				then
					kill -9 `cat /Update_Status.pn` >/dev/null 2>&1
				fi
			fi
		fi
	done

	/usr/lib/methods/showled 0xA46

# 228
	# Set LANG variable based on the BOSINST_LANG flag.
	export LANG=`bidata -b -g locale -f BOSINST_LANG`

	rm -f /tmp/Get_RVG_Disks.pn

	return 0
}

# Image_Data_Exists:  Check for existence of image.data.  If it is not
# there, if we did not boot over the network, flag errors.
#     syntax:  Image_Data_Exists
#     Always returns 0.
function Image_Data_Exists
{
	trap 'BI_Error "BOS Install" 29 2' INT
# 62
	if [ ! -s "$IMAGEDATA" ]
	then # 63
		# then there is no image.data file.
# 66
		# If we booted over the network and NIM_BOS_IMAGE is set, then we are
		# okay, since the user will choose the image to install.  If
		# NIM_BOS_IMAGE is not set, flag an error.
		# If we booted from tape, see if we can get the image.data file from
		# the fourth tape mark.  If that fails, flag an error.
		# If we booted from cdrom, flag an error.
		if [ "$BOOTTYPE" -eq "$NETWORK" ]
		then # 69
# 72
			if [ "$NOT" "$NIM_BOS_IMAGE" ]
			then # 77
# 85
				ERRORTEXT="    Missing image.template file.  The network install
    server is not set up correctly."
				ERRORNUM=4
				FATAL=TRUE
			fi # 76
		else # 70
# 73
			if [ "$BOOTTYPE" -eq "$TAPE" ]
			then # 78
				cd /
				tctl -f /dev/$BOOTDEV.1 rewind || /usr/lib/methods/showled 0xA43
				tctl -f /dev/$BOOTDEV.1 fsf 3 || /usr/lib/methods/showled 0xA43
				restbyname -xqSf /dev/$BOOTDEV.1 .$IMAGEDATA >/dev/null 2>&1
# 80
				if [ "$NOT" -s "$IMAGEDATA" ]
				then # 82
					ERRORTEXT="    Missing image.data file.  The tape does not
    contain a valid install image."
					ERRORNUM=5
					FATAL=TRUE
				fi # 81
			else # 79
					ERRORTEXT="    Missing image.data file.  The CDROM does not
    contain a valid install image."
					ERRORNUM=6
					FATAL=TRUE
			fi
		fi

	fi
	return 0
}

# Init_Target_Disks:
#     If there are no values for the target disks specified in the bosinst.data
#     file, initialize the bosinst.data file to contain either the previous rvg
#     or first disk listed by lsdev.
#     syntax:  Init_Target_Disks
#     Always returns 0.
function Init_Target_Disks
{
	trap 'BI_Error "BOS Install" 29 2' INT

	/usr/lib/methods/showled 0xA44
	# Get_RVG_Disks (which creates the $TARGETVGS file) was put in the
	# background.  Now, before trying to access $TARGETVGS, wait for it to
	# finish.
	if [ -f /tmp/Get_RVG_Disks.pn ]
	then
		wait `cat /tmp/Get_RVG_Disks.pn`
	fi

	/usr/lib/methods/showled 0xA46
# 98
	ANY_DEFINED=
	ANY_DEFINED=`lsdev -Ccdisk -S Defined -F'name'`
	if [ "$ANY_DEFINED" ]
	then
		ERRORTEXT="The following disks did not configure properly:
%s."
		ERRORARG=$ANY_DEFINED
		ERRORNUM=32
		bidata -b -m control_flow -f PROMPT -e yes
		bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
	fi

	unset RVG_EXISTS
	# Open file descriptor 3 for read.
	exec 3< $TARGETVGS
	# The following loop will stop when it finds the first root volume group,
	# leaving the VGID, INVG, LEVEL, NAME, LOCATION, SIZE and BOOTABLE
	# variables set.
	while read -u3 VGID INVG LEVEL NAME LOC SIZE BOOTABLE
	do
		if [ "$INVG" -eq "$ROOTVG" ]
		then
			RVG_EXISTS=TRUE
			break
		fi
	done

	# Close file descriptor 3.
	exec 3<&-

# If an RVG exists, put all disks which are in it into the target_disk_data
# structure.  Otherwise, put the first disk returned by lsdev into the
# target_disk_data structure.
# 38
	if [ "$RVG_EXISTS" ]
	then # 39
		# Determine if target_disk_data stanzas were null in bosinst.data.
		# If all LOCATION, SIZE_MB, and HDISKNAME fields are null, then no
		# target_disk_data was specified in bosinst.data.
		LOC=`bidata -b -g target_disk_data -f LOCATION`; LOC=`echo $LOC`
		SZ=`bidata -b -g target_disk_data -f SIZE_MB`; SZ=`echo $SZ`
		HD=`bidata -b -g target_disk_data -f HDISKNAME`; HD=`echo $HD`
# 41
		if [ -z "$LOC" -a -z "$SZ" -a -z "$HD" ]
		then # 43
# 47
			# If no target_disk_data info in bosinst.data, create
			# target_disk_data stazas for each disk in the rvg.  This is done
			# by adding stanzas to the list kept by the datadaemon.

			# Now add a target_disk_data stanza for each disk in rvg.
			# VGID was set in above loop.  Don't need to worry about putting
			# bootable disk first here since this was done when TARGETVGS
			# was created.
			grep "^$VGID" "$TARGETVGS" | \
				while read VGID INVG LEVEL NAME LOC SIZE BOOTABLE
				do
					bidata -a -l $LOC -s $SIZE -n $NAME
				done

			# Set NULL_TARGET to true to indicate that the target_disk_data
			# stanzas in the bosinst.data file were null.  If NULL_TARGET is
			# true, later procedures will know that the values in the
			# target_disk_data stanzas are defaults and can be overridden.
			NULL_TARGET=TRUE
		fi # 44
# 53
		unset MIGRATABLE
		# Open file descriptor 3 for read.
		exec 3< $TARGETVGS
		# The following loop will stop when it finds the first root volume
		# group of level 3.2 or 4.1  @@@
		while read -u3 VGID INVG LEVEL NAME LOC SIZE BOOTABLE
		do
			if [[ ($LEVEL > "3.2" || "$LEVEL" = "3.2") && "$LEVEL" < "$CURLEVEL" ]]
			then
				MIGRATABLE=TRUE
				break
			fi
		done

		# Close file descriptor 3.
		exec 3<&-

		IM=`bidata -b -g control_flow -f INSTALL_METHOD`
		# If any of the previous rootvgs are not of level 3.1,
		# and if INSTALL_METHOD is null, initialize it to migrate.
		if [ "$MIGRATABLE" ]
		then # 55
# 57
			# If INSTALL_METHOD is null in bosinst.data, set it to "migrate".
			if [ "$NOT" "$IM" ]
			then
				bidata -b -m control_flow -f INSTALL_METHOD -e migrate
			fi
		else # 56
# 58
			# If INSTALL_METHOD is null in bosinst.data, set it to "preserve".
			# Otherwise, if it equals migrate, change it to preserve, flag an
			# error, and set PROMPT to yes.
			if [ "$NOT" "$IM" ]
			then # 303
# 392
				bidata -b -m control_flow -f INSTALL_METHOD -e preserve
# 307
			elif [ "$IM" = migrate ]
			then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

# 393
				bidata -b -m control_flow -f INSTALL_METHOD -e preserve
# 395
				ERRORTEXT="The bosinst.data file specified doing a migration
install, but there is no existing root volume group
of level 3.2 or greater."
				ERRORNUM=1
# 397
				bidata -b -m control_flow -f PROMPT -e yes
				bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
			fi
		fi
	else # 40
		# Determine if target_disk_data stanzas were null in bosinst.data.
		# If all LOCATION, SIZE_MB, and HDISKNAME fields are null, then no
		# target_disk_data was specified in bosinst.data.
		LOC=`bidata -b -g target_disk_data -f LOCATION`; LOC=`echo $LOC`
		SZ=`bidata -b -g target_disk_data -f SIZE_MB`; SZ=`echo $SZ`
		HD=`bidata -b -g target_disk_data -f HDISKNAME`; HD=`echo $HD`
# 42
		if [ -z "$LOC" -a -z "$SZ" -a -z "$HD" ]
		then # 45
# 48
			PLATFORM=`bootinfo -T`
			SAVED_ID=0

			lsdev -Ccdisk -S Available | while read NAME stuff LOC rest
			do
				if [ "`bootinfo -B $NAME`" -eq 1 ]
				then
					if [ "$PLATFORM" = rspc ]
					then
						# extract the scsi id from the location code,
						# which is assumed
						# to be in a format like ??-??-??-?,?
						# this gets everything after the last dash
						ID_WITH_COMMA=${LOC##*-}
						# this gets everything before the first comma
						CURRENT_ID=${ID_WITH_COMMA%%,*}
						if [[ $CURRENT_ID -ge $SAVED_ID ]]; then
							SAVED_ID=$CURRENT_ID
							SAVED_NAME=$NAME
							SAVED_LOC=$LOC
						fi
					else
						SIZE=`bootinfo -s $NAME`
						bidata -a -l $LOC -s $SIZE -n $NAME
						break
					fi
				fi
			done
			if [ "$PLATFORM" = rspc ]
			then
				SIZE=`bootinfo -s $SAVED_NAME`
				bidata -a -l $SAVED_LOC -s $SIZE -n $SAVED_NAME
			fi

			# Set NULL_TARGET to true to indicate that the target_disk_data
			# stanzas in the bosinst.data file were null.  If NULL_TARGET is
			# true, later procedures will know that the values in the
			# target_disk_data stanzas are defaults and can be overridden.
			NULL_TARGET=TRUE
		fi # 46
# 54
		# If INSTALL_METHOD is null in bosinst.data, set it to "overwrite".
		# If it is not null and does not equal overwrite, flag an error, set
		# PROMPT to yes, and set INSTALL_METHOD to overwrite.
		IM=`bidata -b -g control_flow -f INSTALL_METHOD`
		if [ "$NOT" "$IM" ]
		then # 37
# 147
			bidata -b -m control_flow -f INSTALL_METHOD -e overwrite
# 145 # 209
		elif [ "$IM" != overwrite ]
		then # 391
			# If ERROR_EXIT is set, run it and then flag that user must
			# be prompted.  If they want to stop before the menus, they
			# may do so with the error exit.
			error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
			if [ -n "$error_exit" ]; then "$error_exit"; fi

# 394
			bidata -b -m control_flow -f INSTALL_METHOD -e overwrite
# 396
			ERRORTEXT="The bosinst.data file specified doing either a migration
or a preservation install, but there is no existing root
volume group."
			ERRORNUM=2
# 398
			bidata -b -m control_flow -f PROMPT -e yes
			bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
		fi # 343
	fi

	return 0
}

# Initialize:
#     Does some basic environment initialization.
#     Always returns 0.
function Initialize
{
	trap 'BI_Error "BOS Install" 29 2' INT
# 33

	/usr/lib/methods/showled 0xA46
	umask 002
	# Set values for variables.  These variables will be used in a way
	# similar to the way "#define" values are used in C.
	BOSINSTDATA=/bosinst.data
	CDROM=3
	CURLEVEL=4.1.1
	IMAGEDATA=/image.data
	MEG=1048576
	MROOT=/mnt
	NETWORK=5
	NOT=!
	NOVG=0
	NOVGID=0000000000000000
	ODMDIR=/etc/objrepos
	OTHERVG=2
	PRE_LIST=/etc/preserve.list
	ROOTVG=1
	PERCENT=0
	TIME=0
	FATAL=
	BAR=|
	TAPE=4
	TAPEBLKSZ=/tapeblksz
	TAPEDEVICE=/tapedevice
	TARGETVGS=/tmp/targetvgs
	ESERVER=1
    BOS_FORMAT=nonturbo
	# Export the above variables.
	export BOSINSTDATA CDROM IMAGEDATA MEG MROOT NETWORK NOT NOVG NOVGID
	export OTHERVG ROOTVG TAPE TAPEBLKSZ TARGETVGS ODMDIR TAPEDEVICE
	export TIME PERCENT BAR FATAL PRE_LIST CURLEVEL ESERVER BOS_FORMAT

	# Set the phase variable
	export SYSCFG_PHASE=BOSINST

    # find out how much memory we have
	REALMEM=`bootinfo -r`
	export REALMEM

	if [ ! -s "$TAPEDEVICE" ]
	then
		# Put a set -x here for debugging.  There is nothing to the screen at
		# this point, so the only way to see this is through enter_dbg.
		set -x
	fi

	# Set boot type and boot device name.
	unset BOOTDEV BOOTTYPE
	if [ -s "$TAPEDEVICE" ]
	then
		BOOTTYPE=$TAPE
		BOOTDEV=`/usr/bin/cat $TAPEDEVICE`
		BOOTDEV=${BOOTDEV#/dev/}
	else
		BOOTTYPE=`/usr/sbin/bootinfo -t || loopled 0xA41`
		BOOTDEV=`/usr/sbin/bootinfo -b || loopled 0xA41`
		export BOSMENU_LANG=
        
	fi
	export BOOTTYPE BOOTDEV
	set +x

	# Initialize white space to be "space, tab, newline."
	IFS=" 	
"

	# PATH and LIBPATH will be different, depending on the boot device.
	case "$BOOTTYPE" in
	$NETWORK | $CDROM ) LIBPATH=/../usr/lib:/../lib:/SPOT/usr/ccs/lib:/SPOT/usr/lib:/usr/lib:/lib:/../SPOT/usr/ccs/lib:/../SPOT/usr/lib:
			PATH=/../usr/bin:/../usr/sbin:/../bin:/../etc:/SPOT/usr/bin:/SPOT/usr/sbin:/SPOT/usr/lpp/bosinst:/usr/bin:/usr/sbin:/bin:/etc:/../SPOT/usr/bin:/../SPOT/usr/sbin:/../SPOT/usr/lpp/bosinst:

			if [ ! -d /sbin/helpers ]
			then
				rm -fr /sbin/helpers
				ln -sf /SPOT/usr/lpp/bos/inst_root/sbin/helpers /sbin
			fi
			# Create links to the jfs compression utilities
			ln -sf /SPOT/usr/lpp/bos/inst_root/sbin/comp.kext /sbin
			ln -sf /SPOT/usr/lpp/bos/inst_root/sbin/comp.uext /sbin
	;;
	$TAPE ) LIBPATH=/usr/ccs/lib:/usr/lib:/lib:/../usr/ccs/lib:/../usr/lib:/lib:
			PATH=/usr/bin:/usr/sbin:/usr/lpp/bosinst:/bin:/etc:/../usr/bin:/../usr/sbin:/../usr/lpp/bosinst:/../bin:/../etc:
	;;
	esac
	export IFS LIBPATH PATH

	# Create /var/adm/ras if it does not exist.
	mkdir -p /var/adm/ras >/dev/null 2>&1

	# Remove data and temporary files if they exist.  These files will exist
	# if bi_main has been re-execed.
	rm -f $IMAGEDATA $BOSINSTDATA 

	# If BOOTTYPE is CDROM, make sure all tape devices start out with a block
	# size 512.
	if [ "$BOOTTYPE" -eq "$CDROM" ]
	then
		lsdev -Cc tape | while read tapename dummy
			do
				chdev -l $tapename -a block_size=512
			done
	fi

	# Ensure that environment variables to be used later are clear.
	unset ALL_MATCH ERRORTEXT ERRORNUM IMAGE_ON_DISKETTE NULL_TARGET RVG_EXISTS
	unset MIGRATABLE TOTAL_TARGETS TOTAL_SOURCES TOTAL_VG_DATA TOTAL_LV_DATA
	unset TOTAL_FS_DATA LEVEL PRESERVE_ON_DISKETTE
	export ALL_MATCH ERRORTEXT ERRORNUM ERRORARG IMAGE_ON_DISKETTE NULL_TARGET RVG_EXISTS
	export MIGRATABLE TOTAL_TARGETS TOTAL_SOURCES TOTAL_VG_DATA
	export TOTAL_LV_DATA TOTAL_FS_DATA PRESERVE_ON_DISKETTE

	export SETX=0
	return 0
}

# Initialize_Disk_Environment:
#     Set up environment so that the files just restored can be used and the
#     disk environment can be accessed instead of the RAM environment.
#     syntax:  Initialize_Disk_Environment
#     Always returns 0.
function Initialize_Disk_Environment
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 37 'Initializing disk environment.\n'`"

# 35
	# PATH and LIBPATH will be different, depending on the boot device.
	case "$BOOTTYPE" in
	$NETWORK | $CDROM ) LIBPATH=/SPOT/usr/ccs/lib:/SPOT/usr/lib:$MROOT/usr/ccs/lib:$MROOT/usr/lib:
			PATH=/SPOT/usr/bin:/SPOT/usr/sbin:/SPOT/usr/lpp/bosinst:$MROOT/usr/bin:$MROOT/usr/sbin:$MROOT/usr/lpp/bosinst:
	;;
	$TAPE ) LIBPATH=/usr/lib:$MROOT/usr/ccs/lib:$MROOT/usr/lib:
			PATH=/usr/bin:/usr/sbin:/usr/lpp/bosinst:$MROOT/usr/bin:$MROOT/usr/sbin:$MROOT/usr/lpp/bosinst:
	;;
	esac
	export PATH LIBPATH

	PT=`bidata -i -g image_data -f PRODUCT_TAPE`
	if [ "$PT" = no ]
	then
		# Remove device special files as they were when the backup was made.
# 341
		rm -rf /mnt/dev/*
# 342
		# Preserve device ODM as it was before the backup so it can be merged
		# into the current ODM later.
		# The hard disk /dev/hd3 is mounted over /tmp at this point, so we
		# are actually copying the old device ODM to the hard disk /tmp.
		mkdir -p /tmp/bos/objrepos.inst 2>/dev/null
	    ODMPATH=/etc/objrepos:/usr/lib/objrepos
	    export ODMPATH
	    /usr/lpp/bosinst/rda -e -p $MROOT/etc/objrepos -d /tmp/bos/objrepos.inst/CuDv.sav -a /tmp/bos/objrepos.inst/CuAt.sav
		rc="$?"

		# Save value of previous dump device.
		OLDODMPATH="$ODMPATH"
		export ODMPATH=$MROOT/etc/objrepos:$MROOT/usr/lib/objrepos
		dumpdev=`odmget -q'attribute=tprimary' SWservAt | grep value | cut -d'"' -f2`
		echo $dumpdev > /tmp/bos/dumpdev

		# Unset ODMPATH since it cannot be used reliably if the old odm*
		# commands are being used.
		export ODMPATH=
	fi

	if [ "$PT" = yes ]
	then
		ETC=usr/lpp/bos/inst_root/etc
		DEV=usr/lpp/bos/inst_root/dev
	else
		ETC=etc
		DEV=dev
	fi

	# Copy RAM /dev to hard disk so can access the devices once the RAM / has
	# been overmounted with the one on the disk.
	ln -sf /mnt/usr/bin/pwd /usr/bin/pwd >/dev/null 2>&1
	IM=`bidata -b -g control_flow -f INSTALL_METHOD`
    # Just remove the RAM devices in the case of a 4.x->4.x migration install 
	if [ "$IM" != migrate ] || [ "$BOSLEVEL" = "3.2" ]
	then
	    rm $MROOT/$DEV/*
	else
		# For a migration install with compatible devices, save off the 
		# existing /dev entries from the disk which have the same name as
		# those from the RAM filesystem.  Use the RAM devices until after the
		# bosboot (which must use the RAM ODM) to keep everything in sync.

		(cd $MROOT/$DEV && ls | backbyname -irf /dev.disk.bff 2>/dev/null)
	    rm $MROOT/$DEV/*

		# This tells bosboot to use the RAM ODM instead of the disk ODM 
		echo "Migration installation requires RAM ODM" > /BOOTFS_COOKIE
	fi
	(cd /dev && ls | backbyname -irf - 2>/dev/null) |
		(cd $MROOT/$DEV && restbyname -xqf - >/dev/null 2>&1)

	rc="$?"
	echo ''
	if [ "$rc" -ne 0 ]
	then
		/usr/lib/methods/showled 0xA59
		BI_Error "BOS Install" 15 2
	fi
	rm -f $MROOT/$DEV/ram*

	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 38 'Copying Cu* to disk.\n'`"

	# Copy volume group information to disk.
	(cd /etc && find ./vg -print | backbyname -irf - 2>/dev/null) |
		(cd $MROOT/$ETC && restbyname -xqf - >/dev/null 2>&1)
	rc="$?"
	if [ "$rc" -ne 0 ]
	then
		/usr/lib/methods/showled 0xA59
		BI_Error "BOS Install" 16 2
	fi

	# Copy Customized device object classes to disk.
	cp /etc/objrepos/Cu* $MROOT/$ETC/objrepos; rc="$?"
	if [ "$rc" -ne 0 ]
	then
		/usr/lib/methods/showled 0xA59
		BI_Error -f"BOS Install" 17 2
	fi

# 345
	# copy any special console definition file to disk for disk boot support
	if [ -f /etc/consdef ]
	then
		cp /etc/consdef $MROOT/$ETC
	fi
# 346
	cp /mnt/usr/sbin/slibclean /etc
	/etc/slibclean

	return 0
}

# Initialize_Log:
#     Initialize_Log will set the SETX variable if the user wants debugging
#     turned on.
#     Always returns 0.
function Initialize_Log
{
	trap 'BI_Error "BOS Install" 29 2' INT
	unset SETX
	if [ "`bootinfo -k`" -eq 1 ]
	then
		/etc/methods/showled 0xeee
		sleep 3
		if [ "`bootinfo -k`" -eq 2 ]
		then
			/etc/methods/showled 0xddd
			sleep 3
			if [ "`bootinfo -k`" -eq 3 ]
			then
	    		export SETX=1
			fi
		fi
	fi
	/etc/methods/showled 0xfff
	return 0
}

# Install_Updates:
#     Installs mandatory gold updates for bos.rte.
#     syntax:  Install_Updates
#     Returns the return code of the installp.
function Install_Updates
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	case "$BOOTTYPE"
	in
		$TAPE ) INSTALL_DEVICE=/dev/$BOOTDEV.1 ;;
		$NETWORK | $CDROM) INSTALL_DEVICE=/SPOT/usr/sys/inst.images ;;
	esac

	# BOSINSTENV variable is set to TRUE so that the bosrun shell script
	# which gets called by installp will not execute bosboot.
	BOSINSTENV=TRUE
	export BOSINSTENV
	ret=0
	rm -f /tmp/ret
	installp -acqNXgId $INSTALL_DEVICE bos.rte 2>&1 || echo $? >/tmp/ret | \
					/../usr/bin/alog -s 16384 -q -f /var/adm/ras/installp.log
	ret=`cat /tmp/ret`
	while [ -f /tmp/_spam_3426 ]
	do
		rm -f /tmp/_spam_3426
		rm -f /tmp/ret
		installp -acqNXgId $INSTALL_DEVICE bos.rte 2>&1 || echo $? >/tmp/ret | \
					/../usr/bin/alog -s 16384 -q -f /var/adm/ras/installp.log
		ret=`cat /tmp/ret`
	done

	unset BOSINSTENV
	return $ret
}

# Log:
#     If SETX is not set, Log will run the command and send standard out to
#     the alog command which will log it to /var/adm/ras/bi.log.  If SETX
#     is set, Log will run the command and send standard out to the screen.
#     Always returns 0.
function Log
{
	rm -f /tmp/badret
	if [ "$SETX" -ne 1 ]
	then
		$1 >>/var/adm/ras/bi.log 2>>/var/adm/ras/bi.log
	else
		$1
	fi
}

# loopled:
#    Takes as input an LED number and loops forever.  This is used for a fatal
#    error.
function loopled {

	# tell NIM about the failure
	if [ "$BOOTTYPE" -eq "$NETWORK" ]
	then
		${NIMCLIENTPATH}nimclient -R failure
	fi

	/etc/methods/showled $1   # show a LED code
    while :     # loop forever
    do
        :
    done
}

# Make_Sys_FS
#     Make the specified file system based on the image.data file.
#     syntax:  Make_Sys_FS <fsname>
#         where <fsname> is the name of the file system to create.
#     Always returns 0.
function Make_Sys_FS
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
	fs_name=$1

	fs_size=`bidata -i -g fs_data -f FS_SIZE -c FS_NAME -v $fs_name`
	fs_min_size=`bidata -i -g fs_data -f FS_MIN_SIZE -c FS_NAME -v $fs_name`
	fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v $fs_name`
	fs_fs=`bidata -i -g fs_data -f FS_FS -c FS_NAME -v $fs_name`
	fs_nbpi=`bidata -i -g fs_data -f FS_NBPI -c FS_NAME -v $fs_name`
	fs_compress=`bidata -i -g fs_data -f FS_COMPRESS -c FS_NAME -v $fs_name`

	# If any of the variables containing the parameters are non-null, set
	# a variable indicating that the specified flag must be given to mkfs.
	VFLAG='-V jfs'
	shrink=`bidata -i -g logical_volume_policy -f SHRINK`
	SFLAG=
	if [ "$shrink" = yes ]
	then
		if [ -n "$fs_min_size" ]
		then
			SFLAG="-s $fs_min_size"
		fi
	else
		if [ -n "$fs_size" ]
		then
			SFLAG="-s $fs_size"
		fi
	fi
	OFLAG=
	if [ -n "$fs_fs" ]
	then
		OFLAG="-o frag=$fs_fs"
	fi
	if [ -n "$fs_nbpi" ]
	then
		if [ -n "$OFLAG" ]
		then
			OFLAG="$OFLAG,nbpi=$fs_nbpi"
		else
			OFLAG="-o nbpi=$fs_nbpi"
		fi
	fi
	if [ -n "$fs_compress" ]
	then
		if [ -n "$OFLAG" ]
		then
			OFLAG="$OFLAG,compress=$fs_compress"
		else
			OFLAG="-o compress=$fs_compress"
		fi
	fi

	echo y | mkfs $VFLAG $OFLAG $fs_lv
	rc="$?"
	if [ "$rc" -ne 0 ]
	then
		BI_Error "BOS Install" 10 2 "$fs_name"
	fi

	return 0
}

# Make_Sys_LV
#     Make the specified logical volume based on the image.data file.
#     syntax:  Make_Sys_LV <lv>
#         where <lv> is the name of the logical volume to create.
#     Always returns 0.
function Make_Sys_LV
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
	exact_fit=no
	if [ "$1" = "-m" ]
	then
		exact_fit=yes
		shift
	fi

	LV=$1

	# Get all parameters which will be used to create this logical volume.
	PPS=`bidata -i -g vg_data -f PPSIZE -c VGNAME -v rootvg`
	volume_group=`bidata -i -g lv_data -f VOLUME_GROUP -c LOGICAL_VOLUME -v $LV`
	disk_list=`bidata -i -g lv_data -f LV_SOURCE_DISK_LIST -c LOGICAL_VOLUME -v $LV`
	type=`bidata -i -g lv_data -f TYPE -c LOGICAL_VOLUME -v $LV`
	max_lps=`bidata -i -g lv_data -f MAX_LPS -c LOGICAL_VOLUME -v $LV`
	copies=`bidata -i -g lv_data -f COPIES -c LOGICAL_VOLUME -v $LV`
	lps=`bidata -i -g lv_data -f LPs -c LOGICAL_VOLUME -v $LV`
	lvppsz=`bidata -i -g lv_data -f PP_SIZE -c LOGICAL_VOLUME -v $LV`
	bb_policy=`bidata -i -g lv_data -f BB_POLICY -c LOGICAL_VOLUME -v $LV`
	inter_policy=`bidata -i -g lv_data -f INTER_POLICY -c LOGICAL_VOLUME -v $LV`
	intra_policy=`bidata -i -g lv_data -f INTRA_POLICY -c LOGICAL_VOLUME -v $LV`
	write_verify=`bidata -i -g lv_data -f WRITE_VERIFY -c LOGICAL_VOLUME -v $LV`
	upper_bound=`bidata -i -g lv_data -f UPPER_BOUND -c LOGICAL_VOLUME -v $LV`
	sched_policy=`bidata -i -g lv_data -f SCHED_POLICY -c LOGICAL_VOLUME -v $LV`
	relocatable=`bidata -i -g lv_data -f RELOCATABLE -c LOGICAL_VOLUME -v $LV`
	label=`bidata -i -g lv_data -f LABEL -c LOGICAL_VOLUME -v $LV`
	mwc=`bidata -i -g lv_data -f MIRROR_WRITE_CONSISTENCY -c LOGICAL_VOLUME -v $LV`
	lv_separate=`bidata -i -g lv_data -f LV_SEPARATE_PV -c LOGICAL_VOLUME -v $LV`
	map_file=`bidata -i -g lv_data -f MAPFILE -c LOGICAL_VOLUME -v $LV`

	# Some values must be converted so that mklv will take them as parameters.
	case "$bb_policy" in
		relocatable     ) bb_policy=y ;;
		non-relocatable ) bb_policy=n ;;
		*               ) bb_policy= ;;
	esac

	case "$sched_policy" in
		parallel   ) sched_policy=p ;;
		sequential ) sched_policy=s ;;
		*          ) sched_policy= ;;
	esac

	case "$inter_policy" in
		minimum ) inter_policy=m ;;
		maximun ) inter_policy=x ;;
		*       ) inter_policy= ;;
	esac

	case "$intra_policy" in
		middle         ) intra_policy=m ;;
		center         ) intra_policy=c ;;
		edge           ) intra_policy=e ;;
		'inner middle' ) intra_policy=im ;;
		'inner edge'   ) intra_policy=ie ;;
		*              ) intra_policy= ;;
	esac

	case "$relocatable" in
		yes ) relocatable=y ;;
		no  ) relocatable=n ;;
		*   ) relocatable= ;;
	esac

	case "$mwc" in
		on  ) mwc=y ;;
		off ) mwc=n ;;
		*   ) mwc= ;;
	esac

	case "$write_verify" in
		on  ) write_verify=y ;;
		off ) write_verify=n ;;
		*   ) write_verify= ;;
	esac

	case "$lv_separate" in
		yes ) lv_separate=y ;;
		no  ) lv_separate=n ;;
		*   ) lv_separate= ;;
	esac

	# adjust LPs if necessary
	if [ "$PPS" -ne "$lvppsz" ]
	then
	    (( mb = lvppsz * lps ))
	    (( lps = mb / PPS ))
		if [ `expr "$mb" % "$PPS"` -ne 0 ]
		then
			(( lps += 1 ))
		fi

	    # If LPs now exceeds MAX_LPS, adjust MAX_LPS
	    if [ -n "$max_lps" -a "$max_lps" -lt "$lps" ]
	    then
			max_lps=$lps
	    fi
	fi

	# If any of the variables containing the parameters are non-null, set
	# a variable indicating that the specified flag must be given to mklv.
	YFLAG=
	YFLAG="-y $LV"
	TFLAG=
	if [ -n "$type" ]
	then
		TFLAG="-t $type"
	fi
	XFLAG=
	if [ -n "$max_lps" ]
	then
		XFLAG="-x $max_lps"
	fi
	CFLAG=
	if [ -n "$copies" ]
	then
		CFLAG="-c $copies"
	fi
	BFLAG=
	if [ -n "$bb_policy" ]
	then
		BFLAG="-b $bb_policy"
	fi
	EFLAG=
	if [ -n "$inter_policy" ]
	then
		EFLAG="-e $inter_policy"
	fi
	RFLAG=
	if [ -n "$relocatable" ]
	then
		RFLAG="-r $relocatable"
	fi
	DFLAG=
	if [ -n "$sched_policy" ]
	then
		DFLAG="-d $sched_policy"
	fi
	UFLAG=
	if [ -n "$upper_bound" ]
	then
		UFLAG="-u $upper_bound"
	fi
	VFLAG=
	if [ -n "$write_verify" ]
	then
		VFLAG="-v $write_verify"
	fi
	AFLAG=
	if [ -n "$intra_policy" ]
	then
		AFLAG="-a $intra_policy"
	fi
	LFLAG=
	if [ -n "$label" ]
	then
		LFLAG="-L $label"
	fi
	WFLAG=
	if [ -n "$mwc" ]
	then
		WFLAG="-w $mwc"
	fi
	SFLAG=
	if [ -n "$lv_separate" ]
	then
		SFLAG="-s $lv_separate"
	fi
	MFLAG=
	if [ -s "$map_file" -a "$exact_fit" = yes ]
	then
		MFLAG="-m $map_file"
		# Cannot use -a, -c, -e, -s, and -u flags with the -m flag.
		AFLAG= CFLAG= EFLAG= SFLAG= UFLAG=
	elif [ ! -s "$map_file" -a "$exact_fit" = yes ]
	then
		BI_Error "BOS Install" 8 2
	fi

	# First try the mklv by specifying list of disks.
	mklv $AFLAG $BFLAG $CFLAG $DFLAG $EFLAG $LFLAG $MFLAG $RFLAG $SFLAG $TFLAG $UFLAG $VFLAG $WFLAG $XFLAG $YFLAG $volume_group $lps $disk_list
	rc="$?"

	# If the above mklv failed and this is not to be an exact fit, try the mklv
	# again without the list of disks.
	if [ "$rc" -ne 0 -a "$exact_fit" != yes ]
	then
		mklv $AFLAG $BFLAG $CFLAG $DFLAG $EFLAG $LFLAG $RFLAG $SFLAG $TFLAG $UFLAG $VFLAG $WFLAG $XFLAG $YFLAG $volume_group $lps
		rc="$?"
	fi

	# If mklv failed, call error routine.
	if [ "$rc" -ne 0 ]
	then
		BI_Error "BOS Install" 9 2 "$LV"
	fi

	return 0
}

# Make_Sys_VG:
#     Make the specified volume group based on the image.data file.
#     syntax:  Make_Sys_VG <vg> <disk1> <disk2> <disk3> ...
#         where <vg> is the name of the volume group to create
#               <disk1> <disk2> <disk3> ... are the names of the disks to
#                                           contain the root volume group.
#     Always returns 0.
function Make_Sys_VG
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
	VG=$1
	shift
	DISKS=$*

	PPS=`bidata -i -g vg_data -f PPSIZE -c VGNAME -v $VG`
	TOTAL_TARGETS=`bidata -G | wc -l`
	# If TOTAL_TARGETS is null, put the number 1 in there.
	TOTAL_TARGETS=${TOTAL_TARGETS:-1}
	# Add 7 to TOTAL_TARGETS to ensure that the lvm will allow at least 8
	# disks to be put into the rvg.
	(( MAXPVS = TOTAL_TARGETS + 7 ))

	# if this is an 8 meg machine, turn off the datadaemon
	# durning mkvg. Reduce maxpvs by 4.
	if [ "$REALMEM" -le 8192 ]
	then
		bidata -wx
		(( MAXPVS = MAXPVS - 4 ))
	fi

	mkvg -f -y $VG -s $PPS -d $MAXPVS $DISKS; rc=$?
	if [ "$REALMEM" -le 8192 ]
	then
		/usr/lpp/bosinst/datadaemon &
	fi
	if [ "$rc" -ne 0 ]
	then
		BI_Error "BOS Install" 11 2
	fi

	return 0
}

# Mount_Rootfs: Used for mounting root filesystems for new product installs
# Always Returns 0
#
function Mount_Rootfs
{
    trap 'BI_Error "BOS Install" 29 2' INT
    Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 52 'Mounting file systems.\n'`"
    logdev=/dev/hd8
    mkdir -p $MROOT/usr
    fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /usr`
    mount -o log=$logdev $fs_lv $MROOT/usr
    echo "$MROOT/usr" >> /mountlist

    mkdir -p $MROOT/usr/lpp/bos/inst_root
    fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /`
    mount -o log=$logdev $fs_lv $MROOT/usr/lpp/bos/inst_root
    echo "$MROOT/usr/lpp/bos/inst_root" >> /mountlist

    mkdir -p $MROOT/usr/lpp/bos/inst_root/var
    fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /var`
    mount -o log=$logdev $fs_lv $MROOT/usr/lpp/bos/inst_root/var
    echo "$MROOT/usr/lpp/bos/inst_root/var" >> /mountlist

    mkdir -p $MROOT/usr/lpp/bos/inst_root/tmp
    if [ -n "$TMP_LV"  ]
    then
        mount -o log=$logdev $TMP_LV $MROOT/usr/lpp/bos/inst_root/tmp
        echo "$MROOT/usr/lpp/bos/inst_root/tmp" >> /mountlist
    fi
    if [ -n "$HOME_THERE" ]
    then
        mkdir -p $MROOT/usr/lpp/bos/inst_root/home
        mount -o log=$logdev $HOME_LV $MROOT/usr/lpp/bos/inst_root/home
        echo "$MROOT/usr/lpp/bos/inst_root/home" >> /mountlist
    fi
    IM=`bidata -b -g control_flow -f INSTALL_METHOD`
    if [ "$IM" = overwrite ] ; then
        # Now mount file systems other than /, /home, /tmp, /usr, or /var.
        for fs in `bidata -i -g fs_data -f FS_NAME`
        do
            case "$fs" in
                !(/|/home|/tmp|/usr|/var) )
                    fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v $fs`

                    # If the following matches /usr/, mount the fs over
                    # /usr.  If the following doesn't match, mount it over
                    # /usr/lpp/bos/inst_root.
                    if [ "${fs%${fs#/usr/}}" = /usr/ ]
                    then
                        mkdir -p $MROOT$fs 2>/dev/null
                        mount -o log=$logdev $fs_lv $MROOT$fs
                        echo "$MROOT$fs" >> /mountlist
                    else
                        mkdir -p $MROOT/usr/lpp/bos/inst_root$fs 2>/dev/null
                        mount -o log=$logdev \
                                    $fs_lv $MROOT/usr/lpp/bos/inst_root$fs
                        echo "$MROOT/usr/lpp/bos/inst_root$fs" >>/mountlist
                    fi
                ;;
            esac
        done
    fi
    return 0
}


# Other_Initialization:  if they are still null, initialize all other variables
# in the "control_flow:" stanza which have not yet been initialized.
#     syntax:  Other_Initialization
#     Always returns 0.
function Other_Initialization
{
	trap 'BI_Error "BOS Install" 29 2' INT

	# Link locale databases to /etc.
	ln -sf /usr/lib/objrepos/CC /usr/lib/objrepos/CC.vc /usr/lib/objrepos/FONT /usr/lib/objrepos/FONT.vc /usr/lib/objrepos/KEYBOARD /usr/lib/objrepos/KEYBOARD.vc /usr/lib/objrepos/MESSAGES /usr/lib/objrepos/MESSAGES.vc /etc/objrepos/

# 59
	# If PROMPT is null in bosinst.data, set it to yes.
	P=`bidata -b -g control_flow -f PROMPT`
	if [ "$NOT" "$P" ]
	then
		bidata -b -m control_flow -f PROMPT -e yes
	fi

# 60
	# If INSTALL_X_IF_ADAPTER is null in bosinst.data, set it to yes.
	X=`bidata -b -g control_flow -f INSTALL_X_IF_ADAPTER`
	if [ "$NOT" "$X" ]
	then
		bidata -b -m control_flow -f INSTALL_X_IF_ADAPTER -e yes
	fi

# 61
	# If RUN_STARTUP is null in bosinst.data, set it to yes.
	X=`bidata -b -g control_flow -f RUN_STARTUP`
	if [ "$NOT" "$X" ]
	then
		bidata -b -m control_flow -f RUN_STARTUP -e yes
	fi

# 51
	# If RM_INST_ROOTS is null in bosinst.data, set it to no.
	X=`bidata -b -g control_flow -f RM_INST_ROOTS`
	if [ "$NOT" "$X" ]
	then
		bidata -b -m control_flow -f RM_INST_ROOTS -e no
	fi

	if [ "$NOT" -f /usr/sbin/umount ]
	then
		ln /usr/sbin/mount /usr/sbin/umount
	fi

	return 0
}

# Post_Install:
#     Miscellaneous post-install procedures.  These items include but are not
#     limited to the following:  call merge methods, set 'remove inst
#     roots' flag, install mandatory updates, install device support, initialize
#     dump and paging, run rda, import previously existing volume groups on
#     preserve or migrate, install locales, install X if appropriate, call
#     nimclient if appropriate, set up startup in /etc/firstboot if
#     appropriate, install either the mp or up piece, create boot image.
#     On Product Installs, set the Installed Date of the bos.rte.xx filesets
#     to the current system date/time.
#
#     syntax:  Post_Install
#     Always returns 0.
function Post_Install
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
	PT=`bidata -i -g image_data -f PRODUCT_TAPE`
	IM=`bidata -b -g control_flow -f INSTALL_METHOD`
	RIR=`bidata -b -g control_flow -f RM_INST_ROOTS`
	IXIA=`bidata -b -g control_flow -f INSTALL_X_IF_ADAPTER`
	RUN_ST=`bidata -b -g control_flow -f RUN_STARTUP`
	CC=`bidata -b -g locale -f CULTURAL_CONVENTION`
	MSG=`bidata -b -g locale -f MESSAGES`
	KBD=`bidata -b -g locale -f KEYBOARD`
	BI_LANG=`bidata -b -g locale -f BOSINST_LANG`
	PLATFORM=`bootinfo -T`

	export ODMPATH=/etc/objrepos:/usr/lib/objrepos

	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 39 'Merging.\n'`"

	# Tell installp what the method is
	if [ "$IM" = migrate ]
	then
		INUBOSTYPE=0
	else
		INUBOSTYPE=1
	fi
    export INUBOSTYPE

	# If this is a product install, set the installed time of the
	#  bos.rte.<all> filesets to the current system date/time.
	if [ "$PT" = yes ]
	then
	    date -u +"%Y %j %T" | sed "s/:/ /g" | read yr days hours min sec

	    (( yrs = $yr - 1970 ))

	    # figure out the leap days prior to this year.
	    (( lpdays = ( $yrs + 1 ) / 4 ))

	    (( seconds = ( ( ( $yrs * 365 + $lpdays + $days - 1 ) * 24 + $hours ) * 60 + \
		 $min ) * 60 + $sec ))

	    echo "history:\n\ttime = $seconds"  > /tmp/rte_time_set

	    ODMDIR=/etc/objrepos odmchange -o history -q time=0 /tmp/rte_time_set

	    ODMDIR=/usr/lib/objrepos odmchange -o history -q time=0 /tmp/rte_time_set

	    rm /tmp/rte_time_set
	fi

	if [ "$IM" = migrate -o "$IM" = preserve ]
	then
		# If this is a migration from 3.2 and if they had the default
		# dump device from 3.2 (8MB hd7), then remove the old dump device
		if [ "$IM" = migrate -a "$BOSLEVEL" = "3.2" ]
		then
			cat /tmp/bos/dumpdev | grep -q "/dev/hd7$"
			if [ $? = 0 ]
			then
			    LANG=C lslv hd7 | 
						awk '$3 == "PPs:" {PPS=$4}
							$1 == "TYPE:" {type=$2}
							END {if (PPS=2 && type == "sysdump") exit 0
						 	exit 1}'
				if [ $? -eq 0 ]
				then
					rmlv -f hd7 >/dev/null 2>&1 
					# Removing the following file forces installation of the
					# default dump device.
					rm -f /tmp/bos/dumpdev
				fi
			fi

		fi

		Call_Merge_Methods
		if [ "$IM" = migrate ]
		then
		   Clean_Migrated_VPD
		   rm -rf /usr/lpp/bos/migrate
		fi
	fi


# 353
	# If this is a product install, everything updates, devices, locale, X11,
	# and up/mp package, as necessary.
	if [ "$PT" = yes ]
	then
# 350

		odmdelete -o PdAt -q "attribute=TCB_STATE" > /dev/null 2>&1
		installp -C >/dev/null 2>&1

		if [ "$RIR" = yes ]
		then
			/usr/lib/instl/inurid -r
		fi

		if [ "$BOOTTYPE" -eq "$NETWORK" ]
		then
			${NIMCLIENTPATH}nimclient -o change -a force=yes \
				-a ignore_lock=yes \
				-a info="installing_mp/up"
		fi
		UP=0
		# Install either the up or the mp package.
		if [ "`bootinfo -z 2>/dev/null`" != "$UP" ]
		then
			# Install mp package.
			echo bos.rte.mp >>/../tmp/device.pkgs
		else
			# Install up package.
			echo bos.rte.up >>/../tmp/device.pkgs
		fi
		# Install diagnostics if supported
		if [ "`bootinfo -T`" = rspc ] ; then
			bootinfo -M >/dev/null
			if [ "$?" -eq 101 -o "$?" -eq 103 ] ; then
				echo bos.diag >>/../tmp/device.pkgs
			fi
		else
			echo bos.diag >>/../tmp/device.pkgs
		fi

# 354
		# Append base bundle on end of /../tmp/device.pkgs.  Delete the line
		# if CC is C, else replace key word
		# "[lang]" with name of primary cultural convention and key word
		# "[msg]" with name of primary messages locale.
		if [ "$CC" = C ]
		then 
			CHANGE_CC='-e "/\[lang\]/d"'
		else
			CHANGE_CC='-e "s/\[lang\]/$CC/g"'
	    fi

		if [ "$MSG" = C ]
		then
			CHANGE_MSG='-e "/\[msg\]/d"'
		else
			CHANGE_MSG='-e "s/\[msg\]/$MSG/g"'
		fi

		# If install type is not full and is not eserver,
		# then remove login licensing
		# fileset from auto install bundle
		INSTALL_TYPE="`bidata -b -g control_flow -f INSTALL_TYPE`"
		if [ -z "$INSTALL_TYPE" -o "$INSTALL_TYPE" = full -o "$INSTALL_TYPE" = eserver ]
		then
			CHANGE_LIC=
		else
			CHANGE_LIC='-e "/loginlic/d"'
		fi

# 368 # 371
		BNDDIR=/usr/sys/inst.data/sys_bundles
		cp -p $BNDDIR/BOS.autoi /../tmp
		eval sed $CHANGE_CC $CHANGE_MSG $CHANGE_LIC \
				$BNDDIR/BOS.autoi >/../tmp/BOS.autoi
		cat /../tmp/BOS.autoi >>/../tmp/device.pkgs
		mv /../tmp/BOS.autoi $BNDDIR/BOS.autoi

                # append updates to device.pkgs
                case "$BOOTTYPE"
                in
                        $TAPE )
                                UPDATES_FROM=/dev/$BOOTDEV.1
                                ;;
                        $NETWORK | $CDROM )
                                UPDATES_FROM=/SPOT/usr/sys/inst.images
                                ;;
                esac
                installp -q -L -d "$UPDATES_FROM"|awk '{FS=":"}
                        {PKG=$1}
                        {TYPE=$5}
                        {if (PKG == "bos" && TYPE != "I") print $2, $3}
                        END {}' >>/../tmp/device.pkgs

		# save a copy of device.pkgs
		cp /../tmp/device.pkgs /../tmp/min.pkgs

		# Add in any migration-only packages if this is a migration
		if [ "$IM" = migrate -a -f /../tmp/mig.pkgs ]
		then
		   cat /../tmp/mig.pkgs >>/../tmp/device.pkgs
		fi

# 368 # 371
		# If an lft has been chosen as the console, then there is a
		# graphics display attached.  If the INSTALL_X_IF_ADAPTER flag is
		# also set, install X11.rte, the sysmgt GUI, and supporting
		# graphics software.
		# Replace key word "[lang]" with name of primary cultural
		# convention and key word "[msg]" with name of primary
		# messages locale if its not C.
		cp -p $BNDDIR/GOS.autoi /../tmp
		eval sed  $CHANGE_CC $CHANGE_MSG \
					$BNDDIR/GOS.autoi  >/../tmp/GOS.autoi
		mv /../tmp/GOS.autoi $BNDDIR/GOS.autoi
		cp -p $BNDDIR/ASCII.autoi /../tmp
		eval sed $CHANGE_CC $CHANGE_MSG \
			$BNDDIR/ASCII.autoi >/../tmp/ASCII.autoi
		mv /../tmp/ASCII.autoi $BNDDIR/ASCII.autoi
		if [ "`bi_io -l </dev/console 2>/dev/null`" -eq 1 -a "$IXIA" = yes -o "$IXIA" = all ]
		then # 369 # 372
# 374
			cat $BNDDIR/GOS.autoi >>/../tmp/device.pkgs
			>/../tmp/gos.pkgs
		else
			# Install the ASCII bundle if GOS.autoi not installed.
			# Append ascii bundle on end of /../tmp/device.pkgs.
			#link GOS.autoi to bundle name
			cat $BNDDIR/ASCII.autoi >>/../tmp/device.pkgs
			ln -f $BNDDIR/GOS.autoi $BNDDIR/Graphics-Startup.bnd
		fi # 370 # 373

		if [ "$RUN_ST" = yes -a "$BOOTTYPE" -ne "$NETWORK" ] &&
		   [ "$IM" != migrate -o "$BOSLEVEL" = "3.2" ]
		then
# 376
			mkitab -i cron \
			"install_assist:2:wait:/usr/sbin/install_assist </dev/console >/dev/console 2>&1"
		fi

		Change_Status
		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 40 'Installing all packages.\n'`"

		Get_Locale_Packages

		/usr/lib/methods/showled 0xA54
		# Redraw the screen when installing optional packages so user can see
		# progress, errors, prompts for next volumes, and so we can meet our
		# legal requirements of displaying all copyrights.
		if [ -s /../Update_Status.pn ]
		then
			kill -STOP `cat /../Update_Status.pn`
		fi
		echo "$CLEAR\c" >/dev/console

		tcb="`bidata -b -g control_flow -f TCB`"
		if [ "$tcb" = yes ]
		then
			# add the flag to odm

			echo "PdAt:" > /tmp/tcb.add
			echo "    attribute = \"TCB_STATE\"" >> /tmp/tcb.add
			echo "    deflt = \"tcb_enabled\"" >> /tmp/tcb.add

		else
			echo "PdAt:" > /tmp/tcb.add
			echo "    attribute = \"TCB_STATE\"" >> /tmp/tcb.add
			echo "    deflt = \"tcb_disabled\"" >> /tmp/tcb.add
		fi
		odmadd /tmp/tcb.add

		tcb="`bidata -b -g control_flow -f TCB`"
		if [ "$tcb" = yes ]
		then
			Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 41 '
Initializing trusted computing base.\n'`"

			tcbck -y ALL
		fi

		# If a supplemental disk was specified as the target disk, the file
		# /../tmp/device.supp will exist and contain the name(s) of the device
		# package(s) that support the supplemental disk.
		if [ -s /../tmp/device.supp ]
		then
			# Check to ensure the diskette is still in the drive.  If it is not
			# prompt the user to insert it.
			ITS_IN_THERE=
			until [ "$ITS_IN_THERE" ]
			do
				rm -f ./signature
				restbyname -xqSf /dev/rfd0 ./signature >/dev/null 2>&1
				if [ "$?" -eq 0 -a -s ./signature ]
				then
					read a b <./signature || a=none
					if [ "$a" = target ]
					then
						# If this is the supplemental diskette, set ITS_IN_THERE
						ITS_IN_THERE=TRUE
					fi
				fi

				# If diskette is not in the drive, prompt them.
				if [ "$NOT" "$ITS_IN_THERE" ]
				then
					echo "$CLEAR" >/dev/console
					dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 60 \
"\tSupport for Disks not known to Base Operating System Installation\n\n\
 Device Support must be supplied for disks not known to Base Operating\n\
 System Installation.\n\n\n\
 Insert the diskette containing the device support into the diskette drive\n\
 /dev/fd0, and press Enter." >/dev/console
					read answer
				fi
			done
			devinstall -b -d /dev/rfd0 -f /../tmp/device.supp >/dev/console 2>&1
		fi
        BUNDLES="`bidata -b -g control_flow -f BUNDLES`"
        if [ -z "$NIM_LOG" ]
        then
            mkdir -p /var/adm/ras >/dev/null 2>&1
            cp /../bosinst.data /var/adm/ras 2>/dev/null
        fi
		# Now install all the rest of the packages from the boot media.
		# Rename installp.summary
		mv -f /var/adm/sw/installp.summary /var/adm/sw/installp.sav 2>/dev/null
		DEVINSTALL_OK=false
		while [ $DEVINSTALL_OK = false ]
		do
		  rm -f $BNDDIR/Migration.bnd 2>/dev/null
		  DEVINSTALL_FAIL=
		  case "$BOOTTYPE"
		  in
			$TAPE )
                /usr/lib/instl/automsg -d /dev/$BOOTDEV.1 >>/../tmp/device.pkgs
				devinstall -b -d /dev/$BOOTDEV.1 \
							-f /../tmp/device.pkgs >/dev/console 2>&1
				if [ ! -f /var/adm/sw/installp.summary ]
				then
					# This can happen if there's not enough space.  Try
					# again with less packages - drop migration packages
					# (if any) first, then drop gos packages
					export DEVINSTALL_FAIL=true
					if [ -f /../tmp/mig.pkgs ]
					then
						cp /../tmp/mig.pkgs $BNDDIR/Migration.bnd
						# Add back in the gos base (if graphics) and try again
						if [ -f /../tmp/gos.pkgs ]
						then
							cat $BNDDIR/GOS.autoi /../tmp/min.pkgs > /../tmp/device.pkgs
							devinstall -b -d /dev/$BOOTDEV.1 \
								-f /../tmp/device.pkgs >/dev/console 2>&1
						fi
					fi

					if [ ! -f /var/adm/sw/installp.summary ]
					then
						ln -f $BNDDIR/GOS.autoi $BNDDIR/Graphics-Startup.bnd
						devinstall -b -d /dev/$BOOTDEV.1 \
								-f /../tmp/min.pkgs >/dev/console 2>&1
					fi
				fi

				if [ -f /var/adm/sw/installp.summary ]
				then
				  cat /var/adm/sw/installp.summary >> /var/adm/sw/installp.sav 2>/dev/null
                  if [ -n "$BUNDLES" ]
                  then
					cat $BUNDLES > /tmp/bundles 2>/dev/null
                    /usr/lib/instl/automsg -d /dev/$BOOTDEV.1 >>/tmp/bundles
                    devinstall -b -d /dev/$BOOTDEV.1 -f /tmp/bundles >/dev/console 2>&1
					cat /var/adm/sw/installp.summary >> /var/adm/sw/installp.sav 2>/dev/null
                    rm /tmp/bundles
                  fi
				  DEVINSTALL_OK=true
				else
				  sync
				  BI_Error "BOS Install" 39 3
				fi

				;;
			$NETWORK | $CDROM)
               	/usr/lib/instl/automsg -d /SPOT/usr/sys/inst.images  >>/../tmp/device.pkgs
				devinstall -b -d /SPOT/usr/sys/inst.images \
							-f /../tmp/device.pkgs >/dev/console 2>&1
				if [ ! -f /var/adm/sw/installp.summary ]
				then
					# This can happen if there's not enough space.  Try
					# again with less packages - drop migration packages
					# (if any) first, then drop gos packages
					export DEVINSTALL_FAIL=true
					if [ -f /../tmp/mig.pkgs ]
					then
						cp /../tmp/mig.pkgs $BNDDIR/Migration.bnd
						# Add back in the gos base (if graphics) and try again
						if [ -f /../tmp/gos.pkgs ]
						then
							cat $BNDDIR/GOS.autoi /../tmp/min.pkgs > /../tmp/device.pkgs
							devinstall -b -d /SPOT/usr/sys/inst.images \
								-f /../tmp/device.pkgs >/dev/console 2>&1
						fi
					fi

					if [ ! -f /var/adm/sw/installp.summary ]
					then
						ln -f $BNDDIR/GOS.autoi $BNDDIR/Graphics-Startup.bnd
						devinstall -b -d /SPOT/usr/sys/inst.images \
								-f /../tmp/min.pkgs >/dev/console 2>&1
					fi
				fi

				if [ -f /var/adm/sw/installp.summary ]
				then
					if [ "$BOS_FORMAT" = turbo ]
					then
						# if turbo install, run cfgmgr to ensure any necessary
				    	# kernel extensions are loaded that some post_i of an
				    	# install package may have a dependancy on.
						unset DEV_PKGNAME
						cfgmgr -p2  >/dev/null 2>&1
					fi
					DEVINSTALL_OK=true
					cat /var/adm/sw/installp.summary >> /var/adm/sw/installp.sav 2>/dev/null
                	if [ -n "$BUNDLES" ]
                	then
						cat $BUNDLES > /tmp/bundles 2>/dev/null
                    	/usr/lib/instl/automsg -d /SPOT/usr/sys/inst.images  >>/tmp/bundles
                    	devinstall -b -d /SPOT/usr/sys/inst.images -f /tmp/bundles >/dev/console 2>&1
						cat /var/adm/sw/installp.summary >> /var/adm/sw/installp.sav 2>/dev/null
                    	rm /tmp/bundles
                	fi
					$NIM_BOSINST_RECOVER
				else
				  sync
				  BI_Error "BOS Install" 39 3
				fi
				;;
		  esac
		done

		mv /var/adm/sw/installp.sav /var/adm/sw/installp.summary
        ln -f $BNDDIR/App-Dev.def $BNDDIR/App-Dev.bnd
        ln -f $BNDDIR/DCE-Client.def $BNDDIR/DCE-Client.bnd
        if [ -z "$INSTALL_TYPE" -o "$INSTALL_TYPE" = full -o "$INSTALL_TYPE" = eserver ]
        then
            ln -f $BNDDIR/Client.def $BNDDIR/Client.bnd
            ln -f $BNDDIR/Server.def $BNDDIR/Server.bnd
        fi
        if [ "$INSTALL_TYPE" = personal -a -f "$BNDDIR/Pers-Prod.bnd" ]
        then
            mv -f $BNDDIR/Pers-Prod.bnd $BNDDIR/Pers-Prod.def
        fi

		# Now perform a root sync if a SPOT was used as the BOS image
		if [ "$NIM_BOS_FORMAT" = spot ]
		then
			${NIMCLIENTPATH}nimclient -o change -a force=yes \
				-a ignore_lock=yes \
				-a info="performing root sync"
			installp -XOr -e /var/adm/ras/nim.installp  all >/dev/console 2>&1
			UP=0
			# Force a link of /unix to the mp/up kernel.
			if [ "`bootinfo -z 2>/dev/null`" != "$UP" ]
			then
			    # Link to mp Kernel.
			    ln -fs /usr/lib/boot/unix_mp /unix  >/dev/null 2>&1
			    ln -fs /usr/lib/boot/unix_mp /usr/lib/boot/unix >/dev/null 2>&1
			else
			    # Link to up Kernel.
			    ln -fs /usr/lib/boot/unix_up /unix >/dev/null 2>&1
			    ln -fs /usr/lib/boot/unix_up /usr/lib/boot/unix >/dev/null 2>&1
			fi
			$NIM_BOSINST_RECOVER
		fi

		sync;sync;sync
		unset DEV_PKGNAME
		mv -f /var/adm/sw/installp.summary /var/adm/sw/installp.sav
		case "$BOOTTYPE"
		in
			$TAPE )
				cfgmgr -i /dev/$BOOTDEV.1 >/dev/console 2>&1
				;;
			$NETWORK | $CDROM)
				cfgmgr -p2 -i /SPOT/usr/sys/inst.images >/dev/console 2>&1
				;;
		esac
		cat /var/adm/sw/installp.summary >> /var/adm/sw/installp.sav 2>/dev/null
		mv /var/adm/sw/installp.sav /var/adm/sw/installp.summary

		# Place install method and install device in __assistinfo.
		echo "INSTALL_METHOD=$IM" >/var/adm/sw/__assistinfo
		if [ "$BOOTTYPE" -ne "$NETWORK" ]
		then
			echo "INSTALL_DEVICE=/dev/$BOOTDEV" >>/var/adm/sw/__assistinfo
		fi

		/usr/lib/methods/showled 0xA46
		# Redraw the status screen.
		echo "$CLEAR\c" >/dev/console
		if [ `bootinfo -k` != 3 ]
		then
			/usr/lpp/bosinst/berror -f " " -e 30 -a 1 >/dev/console 2>&1
		else
			/usr/lpp/bosinst/berror -f " " -e 25 -a 1 >/dev/console 2>&1
		fi

		Change_Status

		# Restart the status update routine.
		if [ -s /../Update_Status.pn ]
		then
			kill -CONT `cat /../Update_Status.pn`
		fi

		Set_Primary_Locale

		# For entry server, allow up to 16 fixed licenses
		if [ "$ESERVER" -eq 0 ]
		then
			chlicense -u16 2>/dev/null
		fi

    else                # Not a product tape
        UP=0
        # Link to either the up or the mp kernel.
        if [ "`bootinfo -z 2>/dev/null`" != "$UP" ]
        then
            # Link to mp kernel.
            ln -fs /usr/lib/boot/unix_mp /unix >/dev/null 2>&1
            ln -fs /usr/lib/boot/unix_mp /usr/lib/boot/unix >/dev/null 2>&1
        else
            # Link to up kernel.
            ln -fs /usr/lib/boot/unix_up /unix >/dev/null 2>&1
            ln -fs /usr/lib/boot/unix_up /usr/lib/boot/unix >/dev/null 2>&1
        fi

		tcb="`bidata -b -g control_flow -f TCB`"
		if [ "$tcb" = yes ]
		then
			Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 41 '
Initializing trusted computing base.\n'`"

			tcbck -y ALL
		fi

        if [ "$BOOTTYPE" -eq "$CDROM" -o "$BOOTTYPE" -eq "$NETWORK" ]
        then
			mv -f /var/adm/sw/installp.summary /var/adm/sw/installp.sav
            $NIM_BOSINST_RECOVER
            cfgmgr -p2 -i /SPOT/usr/sys/inst.images >/dev/console 2>&1
			cat /var/adm/sw/installp.summary >> /var/adm/sw/installp.sav 2>/dev/null
			mv /var/adm/sw/installp.sav /var/adm/sw/installp.summary
        fi
		Change_Status
		Change_Status
		Change_Status
		Change_Status
		Change_Status
		Change_Status
	fi

	tcb="`bidata -b -g control_flow -f TCB`"
	if [ "$tcb" = yes ]
	then
		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 41 'Initializing trusted computing base.\n'`"

		# Update sysck.cfg with dynamically created special files.
		/usr/lpp/bos/mksysck

	fi

	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 42 'Initializing dump device.\n'`"

	# Initialize dump device.
	DUMP_DEV=`cat /tmp/bos/dumpdev 2>/dev/null`
# 355
	if [ "$NOT" "$DUMP_DEV" ]
	then # 357
		# If DUMP_DEV is not already set, set it to the first paging device.
		# NOTE:  If the following is ever run in the Bourne shell, it will
		#        yield different results.  In the Korn shell, DUMP_DEV
		#        remain set after the while loop, but it would not stay set
		#        if this same thing were done in the Bourne shell.
# 359
		getlvodm -L rootvg | while read lvname lvid
			do
				if [ "`getlvodm -y $lvid`" = paging ]
				then
					DUMP_DEV=/dev/$lvname
					break
				fi
			done
	fi # 358
	sysdumpdev -Pp $DUMP_DEV

	sysdumpdev -z > /dev/null 2>&1   ## Reset Dump Device

# 360
	# Initialize /etc/swapspaces with all paging devices in the rootvg.
	psnames=`lsvg -l rootvg | sed '1,2'd | grep paging | cut -f1 -d' '`
	cp /etc/swapspaces /tmp/swapspaces.tmp
	sed -n "/*/p" /tmp/swapspaces.tmp > /etc/swapspaces
	rm /tmp/swapspaces.tmp
	for ps in $psnames
	do
		chps -a y $ps
	done

# 362
	# Recover device attributes.
	if [ -x "/tmp/bos/objrepos.inst" -a -d "/tmp/bos/objrepos.inst" ]
	then
		if [ "$BOOTTYPE" -eq "$NETWORK" ]
		then
			${NIMCLIENTPATH}nimclient -o change -a force=yes \
				-a ignore_lock=yes \
				-a info="recover_device_attributes"
		fi
	    # If this is a mksysb install, specify to rda to relax the
	    # restrictions such that the device doesn't have to be in the same
	    # slot in order to have its attributes changed.
	    Rflag=
        if [ "$PT" = no ]
        then
            Rflag=-R
			RDAOUTPUT=">/dev/null 2>&1"
            #change the odm keyboard maps
            set -- `grep 'swkb_path = ' /tmp/bos/objrepos.inst/CuAt.sav`
            if [ "$?" -eq 0 ]
            then
                 chkbd $3
            fi
        else
			RDAOUTPUT=">/dev/null 2>&1"
        fi

		# Don't run rda on a 4.x->4.x+ migration since Cu* is preserved
		if [ "$IM" != migrate ] || [ "$BOSLEVEL" = "3.2" ]
		then
	    	# Put entries in /etc/firstboot to reconfigure device attributes.
	    	# Run it twice because some devices have prereqs on other devices
	    	# ans we want to make sure that the prereqed devices get configured
	    	# before the ones that need them.
	    	echo "
/usr/lpp/bosinst/rda $Rflag -d /tmp/bos/objrepos.inst/CuDv.sav -a /tmp/bos/objrepos.inst/CuAt.sav -s /tmp/reconfig1 $RDAOUTPUT
/usr/lpp/bosinst/rda $Rflag -d /tmp/bos/objrepos.inst/CuDv.sav -a /tmp/bos/objrepos.inst/CuAt.sav -s /tmp/reconfig2 $RDAOUTPUT
/usr/lpp/bosinst/rda $Rflag -d /tmp/bos/objrepos.inst/CuDv.sav -a /tmp/bos/objrepos.inst/CuAt.sav -s /tmp/reconfig3 $RDAOUTPUT
cfgmgr
	    	" >> /etc/firstboot
		fi

		# Preserve printer info
		[[ $BOSLEVEL = 3.1 || $BOSLEVEL = 3.2 ]] && 
			/usr/lpp/bosinst/modprtdevs \
			    -d /tmp/bos/objrepos.inst/CuDv.sav \
				-a /tmp/bos/objrepos.inst/CuAt.sav

	    # If this is a mksysb install, and the Tape Block size is
	    # less than 512 then set the Tape Block Size to its original
	    # value. (Block sizes less than 512 not supported).
		if [ "$PT" = no -a "$BOOTTYPE" -eq "$TAPE" -a  -n "$TBLOCKSZ" ]
		then
			if [ "$TBLOCKSZ" -lt 512 ]
			then
				echo "
				/etc/methods/ucfgdevice -l "$BOOTDEV"
				/etc/methods/chggen -l $BOOTDEV -a block_size=$TBLOCKSZ
				`lsdev -Cl$BOOTDEV -rConfigure` -l $BOOTDEV
				" >> /etc/firstboot
			fi
		fi
	fi

# 363
	# If this is a preservation or migration, import previously existing
	# volume groups.
	if [ \( "$IM" = preserve -o "$IM" = migrate \) \
						-a -s /tmp/bos/oldvgs/.oldvgs ]
	then
		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 43 'Importing other volume groups.\n'`"

		hdisk=
		while read vgname val
		do
			# find name of disk with vgid=val
			while read vgid a b name c
			do
				if [ "$vgid" = "$val" ]
				then 
					hdisk=$name
					break;
				fi
			done < /..$TARGETVGS
			if [ -z "$hdisk" ]
			then
				break
			fi
			# Recovering volume group information for $vgname onto disk $hdisk
			berror -f"$vgname" -e31  -a1 -r "$hdisk"
			importvg -y $vgname $hdisk 2>/dev/null
			rc="$?"

			# If import failed, there may not be enough space in /.  If
			# importvg failed and there is less than 2 Meg in /,
			# increase / by a partition.
			FREE=`df -k / | tail -1 | awk '{ print $3 }'`
			if [ "$rc" -ne 0 -a "$FREE" -lt 2048 ]
			then
				chfs -a size=+4096 /
				importvg -y $vgname $hdisk 2>/dev/null
				rc="$?"
			fi
			if [ "$rc" -ne 0 ]
			then
				# Could not import volume group $vgname.  Continuing
				# post-install processing.
				berror -f"$vgname" -e32  -a1
			fi
		done < /tmp/bos/oldvgs/.oldvgs
	fi

    if [ "$BOOTTYPE" -eq "$NETWORK" ]
    then
        # Somewhere after the second nimclient -R success below, the SPOT will
        # no longer be available, so we have to copy the nimclient command to
        # the RAM filesystem to be used from here on in.

        NIMCLIENTPATH="/../"
        cp -p `whence nimclient` ${NIMCLIENTPATH}nimclient
    fi

	# Clear flag so installp will do preinstallation cleanup
	INUBOSTYPE=0
	# Call NIM-specific utility.
	if [ "$BOOTTYPE" -eq "$NETWORK" -a -n "$NIM_CUSTOM" ]
	then
		# report success for BOS install phase 1
		${NIMCLIENTPATH}nimclient -R success

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 44 'Network Install Manager customization.\n'`"

# 375
		$NIM_CUSTOM

		# report success for BOS install phase 2
		${NIMCLIENTPATH}nimclient -R success
	fi

# 380
	# Delete installation configurations from ODM databases.
	/usr/bin/odmdelete -quniquetype=runtime -o PdAt >/dev/null 2>&1
	/usr/bin/odmdelete -qattribute=block_size -o CuAt >/dev/null 2>&1

	# Remove all unnecessary files in /tmp before creating boot image.
	if [ "$PT" = yes ]
	then
		cd /tmp
		for i in *
		do
			if [ "$i" != lost+found -a "$i" != bos ]
			then
				rm -rf $i
			fi
		done
		cd /
	fi

	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 45 'Creating boot image.\n'`"

	# Initialize boot logical volume for subsequent IPL's.
	# Create boot file system from proto file.  The backup part of the clone
	# install guarantees us that there will be enough free space in /tmp before
	# running the bosboot command, so we do not want to delete all files under
	# /tmp.
# 377
	BLVDISK=`lslv -l hd5 | grep hdisk | head -1 | cut -d' ' -f1`
	# Export BLVDISK for manufacturing preload scripts.
	export BLVDISK

	if [ "$BOOTTYPE" -eq "$NETWORK" ]
	then
		${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
				-a info="bosboot"
	fi
	bosboot -d /dev/$BLVDISK -a; rc="$?"
	if [ "$rc" -ne 0 ]
	then
		/usr/lib/methods/showled 0xA61
		BI_Error "BOS Install" 18 2
	fi
	# Check to see if this system supports setable bootlist
	# If bootlist without parameters returns 0 then it is supported
	#
	bootlist; rc="$?"
	if [ "$rc" -eq 0 ]
	then
		bootlist -m normal $BLVDISK
		# Service mode bootlist not supported on rspc models
		if [ "$PLATFORM" != rspc ]
		then
			bootlist -m service fd rmt cd $BLVDISK badisk scdisk
		fi
	fi
	# if migration, blow away and recreate /tmp if /tmp grew during
	# the migration
	#
	# if migration from a compatible level (as in 4.1.0->4.1.1), recover the
	# device information
	#
	if [ "$IM" = migrate ]
	then
		sync # Just in case something goes wrong in this area
		#
		# Recover the device information saved off at the beginning of
		# the installation
		if [ "$BOSLEVEL" != 3.2 ]
		then
			# Make sure the customized data is saved off correctly
			# Recover the saved customized data from the save directory
			mv /tmp/bos/etc/objrepos/Cu* /etc/objrepos
			if [ "$BOSLEVEL" = "4.1" ]
			then
				ODMDIR=/etc/objrepos odmdelete -q"value1=trcdd" -o CuDvDr >/dev/null 2>&1 
				sync
			fi
			savebase -d /dev/$BLVDISK
			# The original devices will be moved back in place in Finish
		fi
		#
		#Determine if we need to shrink /tmp to its previous size
		oldtmpfs_size=`bidata -i -g fs_data -f FS_SIZE -c FS_NAME -v /tmp`
		OLDIFS=$IFS
		IFS=":"
		set -- `LANG=C lsjfs /dev/hd3 | tail +2`
		IFS=$OLDIFS
		# Check to see if we increased the size of /tmp
		if [ $oldtmpfs_size -lt $6 ]
		then
		    set -- `LANG=C du -s /tmp/bos 2>/dev/null`
			# Make sure that we have at least part of one partition available
			(( configsize=$1+1 ))
			# Copy the old files out into /usr if there's any space left
			find /tmp/bos /tmp/lost+found -print | backup -irf - 2>/dev/null |
										compress -cf > /usr/tmpbos.$$.Z
			if [ $? = 0 ]
			then
				# See if we need to recreate the fs larger than it was 
				if [ $configsize -gt $oldtmpfs_size ]
				then
					bidata -i -m fs_data -f FS_MIN_SIZE -e "$configsize" -c FS_NAME -v /tmp
					Shrink_FS /dev/hd3
					# Set FS_SIZE to altered FS_MIN_SIZE
					FS_MIN=`bidata -i -g fs_data -f FS_MIN_SIZE -c FS_NAME -v /tmp`
					bidata -i -m fs_data -f FS_SIZE -e "$FS_MIN" -c FS_NAME -v /tmp
				fi

				# Destroy, recreate, and remount /tmp
				umount /tmp
				if [ $? -eq 0 ]
				then
					rmlv -f hd3 >/dev/null 2>&1
					Make_Sys_LV hd3
					Make_Sys_FS /tmp
					mount /tmp
					chmod 1777 /tmp
					chown bin:bin /tmp
					savebase -d /dev/$BLVDISK

					# Replace the config files previously saved
					zcat /usr/tmpbos.$$ | restore -xqf - >/dev/null 2>&1
				fi
			fi
			rm -f /usr/tmpbos.$$.Z
		fi

	fi

# 52
# Get a number for this.
	# Save settings and mark rootvg as level 4.1.1.
	echo "$BI_LANG:$CC:$MSG:$KBD" | blvset -d /dev/"$BLVDISK" -p menu

	#sync the disk(s)
	sync

	return 0
}

# Prepare_Target_Disks:
#     Create the root volume group.  For overwrite, this means a completely new
#     root volume group.  For preservation, this means deleting hd4, hd2, and
#     hd9var and recreating them.  For migration, this means removing bos.obj
#     from the existing rvg and replacing it with bos.rte.
#     syntax:  Prepare_Target_Disks
#     Always returns 0.
function Prepare_Target_Disks
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 46 'Preparing target disks.\n'`"


	# Determine parameter to give to lslpp based on the level of AIX
	# installed on this root volume group.
	set -- `bidata -b -g target_disk_data -f HDISKNAME`
	FIRSTDISK=$1
	BOSLEVEL=`/usr/lpp/bosinst/blvset -g level -d /dev/$FIRSTDISK`

	if [ -z "$BOSLEVEL" ]
	then
		BOSLEVEL=3.1		
	fi
	export BOSLEVEL

	IM=`bidata -b -g control_flow -f INSTALL_METHOD`

	if [ "$IM" = overwrite ]
	then # 270
# 272
		for hdname in `bidata -b -g target_disk_data -f HDISKNAME`
		do
			/etc/methods/chgdisk -l $hdname -a pv=yes
			rc="$?"
			if [ "$rc" -gt 1 ]
			then
				/usr/lib/methods/showled 0xA47
				# Tell user that we could not change the pvid.
				BI_Error "BOS Install" 1 2 "$hdname"
			fi
		done

# 275
		# Create root volume group based on parameters in image.data.
		/usr/lib/methods/showled 0xA50
		Make_Sys_VG rootvg `bidata -b -g target_disk_data -f HDISKNAME`
		/usr/lib/methods/showled 0xA46

		# Set flag specifying whether or not to use maps in creation of
		# logical volumes.
# 135
		PT=`bidata -i -g image_data -f PRODUCT_TAPE`
		EXFT=`bidata -i -g logical_volume_policy -f EXACT_FIT`
		if [ "$PT" = no -a "$EXFT" = yes ]
		then
			MAPFLAG=-m
		else
			MAPFLAG=
		fi

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 47 'Making paging logical volumes.\n'`"

		# First, create paging logical volumes so paging can be swapped on.
# 138
		for lv in `bidata -i -g lv_data -f LOGICAL_VOLUME -c TYPE -v paging`
		do
			VG=`bidata -i -g lv_data -f VOLUME_GROUP -c LOGICAL_VOLUME -v $lv`
			if [ "$VG" = rootvg ]
			then
				Make_Sys_LV $MAPFLAG $lv
# 282
				swapon /dev/$lv
				# since paging is now turned on, fake out realmem
				if [ "$REALMEM" -le 8192 ]
				then

					REALMEM=8193
					Update_Status &
					echo $! >/Update_Status.pn
				fi
			fi
		done

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 48 'Making boot logical volume.\n'`"

		# Now create boot logical volume to ensure it is at beginning of the
		# specified disk.  If a disk name is not specified in the image.data
		# file for hd5, put hd5 on the first disk on the target_disk_data list.
# 87
		disks=`bidata -i -g lv_data -f LV_SOURCE_DISK_LIST -c LOGICAL_VOLUME -v hd5`
		if [ "$NOT" "$disks" ]
		then
			set -- `bidata -b -g target_disk_data -f HDISKNAME`
			FIRSTDISK=$1
			bidata -i -m lv_data -f LV_SOURCE_DISK_LIST -e $FIRSTDISK -c LOGICAL_VOLUME -v hd5
		fi
# 88
		Make_Sys_LV $MAPFLAG hd5

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 49 'Making logical volumes.\n'`"
		# Create non-paging and non-blv logical volumes only since paging lvs
		# were already created.  We can go ahead and create all jfs and
		# non-jfs logical volumes (except paging) since the number of logical
		# partitions has already been adjusted in the SHRINK case (see
		# Get_User_Input).
# 276 # 280 # 281
		for lv in `bidata -i -g lv_data -f LOGICAL_VOLUME -c VOLUME_GROUP -v rootvg`
		do
			IS_PAGING=`bidata -i -g lv_data -f TYPE -c LOGICAL_VOLUME -v $lv`
			if [ "$IS_PAGING" != paging -a "$lv" != hd5 ]
			then
				Make_Sys_LV $MAPFLAG $lv
			fi
		done

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 50 'Forming the jfs log.\n'`"
# 283
		echo y | logform /dev/hd8 ; rc="$?"
		if [ "$rc" -ne 0 ]
		then
			/usr/lib/methods/showled 0xA49
			# Could not form the log.  Cannot continue.
			BI_Error "BOS Install" 2 2
		fi

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 51 'Making file systems.\n'`"

# 287 # 288
		for FS in `bidata -i -g fs_data -f FS_NAME`
		do
            if [ "$FS" != "/usr" -o "$BOS_FORMAT" != turbo ] ; then
                Make_Sys_FS $FS
            fi
		done

		# Mount file systems.  Keep a list of mount points so they can later
		# be unmounted.
# 289
		PT=`bidata -i -g image_data -f PRODUCT_TAPE`
		if [ "$PT" = yes ]
		then # 290
# 292
            HOME_THERE=TRUE
            HOME_LV=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /home`
            TMP_LV=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /tmp`
            if [ "$BOS_FORMAT" != turbo ] ; then
                Mount_Rootfs
            fi

 		else # 291
            Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 52 'Mounting file systems.\n'`"
			cd /
# 297 # 299
			# Expecting datadaemon to have the list of mount points sorted.
			for I in `bidata -i -g fs_data -f FS_NAME`
			do
				logdev=/dev/hd8
				# If the directory does not exist, create it.
				if [ "$NOT" -d "$MROOT$I" ]
				then
					mkdir -p $MROOT$I
				fi

				fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v $I`
				mount -o log=$logdev $fs_lv $MROOT$I
				echo "$MROOT$I" >>/mountlist
			done
		fi
	else # 271
	# else it is a preservation or migration install.

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 53 'Importing root volume group.\n'`"
# 304
		# Import and varyon the root volume group.
		importvg -y rootvg "$FIRSTDISK"

# 305
		SWAPPED_ON=
		# Find as many paging devices as possible and swap them on.
		# NOTE:  If the following is ever run in the Bourne shell, it will
		#        yield different results.  In the Korn shell, SWAPPED_ON
		#        remain set after the while loop, but it would not stay set
		#        if this same thing were done in the Bourne shell.
		getlvodm -L rootvg | while read lvname lvid
		do
			if [ "`getlvodm -y $lvid`" = paging ]
			then
				swapon /dev/$lvname && SWAPPED_ON=TRUE
				# since paging is now turned on, fake out realmem
				if [ "$REALMEM" -le 8192 ]
				then
					REALMEM=8193
					Update_Status &
					echo $! >/Update_Status.pn
				fi
			fi
		done

		if [ "$NOT" "$SWAPPED_ON" ]
		then
			/usr/lib/methods/showled 0xA51
			# No paging spaces were found.  Cannot continue.
			set -- `bidata -b -g target_disk_data -f HDISKNAME`
			DISKS=$*
			BI_Error "BOS Install" 3 2 "$DISKS"
		fi

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 54 'Replaying the log.\n'`"

# 306
		# Replay the file system log.
		echo y | /usr/sbin/logredo /dev/hd8; rc="$?"
		if [ "$rc" -ne 0 ]
		then
			/usr/lib/methods/showled 0xA49
			# Could not replay the log.  Cannot continue.
			BI_Error "BOS Install" 4 2
		fi

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 55 'Preserving old data.\n'`"

# 308
		mount -o log=/dev/hd8 /dev/hd4 $MROOT
		mount -o log=/dev/hd8 /dev/hd2 $MROOT/usr
		HD9_THERE="`lqueryvg -p $FIRSTDISK -L | grep hd9var 2>/dev/null`"
		if [ "$HD9_THERE" ]
		then
			 mount -o log=/dev/hd8 /dev/hd9var $MROOT/var
		fi

		# Find out what logical volume device is associated with /tmp and mount
		# it.  The device does not have to be hd3, but it does have to be in
		# the rootvg.
		# NOTE:  If the following is ever run in the Bourne shell, it will
		#        yield different results.  In the Korn shell, TMP_LV
		#        remain set after the while loop, but it would not stay set
		#        if this same thing were done in the Bourne shell.
		grep -p '^/tmp:$' $MROOT/etc/filesystems | while read ATTR OP VALUE
		do
			if [ "$ATTR" = "dev" ]
			then
				mount -o log=/dev/hd8 $VALUE $MROOT/tmp
				TMP_LV=$VALUE
				break
			fi
		done

# 323
		# If a /home exists in /etc/filesystems, see if the associated logical
		# volume is in the root volumg group.
		HOME_THERE=`grep '^/home:$' $MROOT/etc/filesystems`
		if [ "$HOME_THERE" ]
		then
			# NOTE:  If the following is ever run in the Bourne shell, it will
			#        yield different results.  In the Korn shell, HOME_LV
			#        remain set after the while loop, but it would not stay set
			#        if this same thing were done in the Bourne shell.
			grep -p '^/home:$' $MROOT/etc/filesystems | while read ATTR OP VALUE
				do
					if [ "$ATTR" = "dev" ]
					then
						HOME_LV=$VALUE
						break
					fi
				done
			# TBD  Make sure the HOME_LV is in rootvg.  Otherwise, unset
			# HOME_THERE.
		fi

		OLDODMDIR=$ODMDIR
		OLDPATH=$PATH
		OLDLIBPATH=$LIBPATH
		ODMDIR=$MROOT/etc/objrepos
		PATH=/usr/sbin:/usr/bin:/usr/lpp/bosinst:$MROOT/usr/sbin:$MROOT/usr/bin:$MROOT/bin:$MROOT/etc:
		LIBPATH=/usr/ccs/lib:/usr/lib:$MROOT/usr/lib:$MROOT/lib
		export PATH LIBPATH ODMDIR

		# Clean out tmp so we can save things there.
		rm -fr $MROOT/tmp/*

		# Save the non-rootvg volume groups that existed on this machine so
		# they can be imported later.
# 309
		mkdir -p $MROOT/tmp/bos/oldvgs
		vgname=
		odmget -qPdDvLn=logical_volume/vgsubclass/vgtype CuDv | \
			grep name | cut -d'"' -f2 |\
		while read vgname
		do
			if [ "$vgname" != rootvg ]
			then
				vgid=`odmget -q"name=$vgname AND attribute=vgserial_id" CuAt | \
					grep value | cut -d'"' -f2`
				echo $vgname $vgid >> $MROOT/tmp/bos/oldvgs/.oldvgs
			fi
		done

# 310
		# Save value of previous dump device.
		if [ "$BOSLEVEL" = "3.2"  -o "$BOSLEVEL" = "3.1" ]
		then
			dumpdev=`odmget -q'name = sys0 AND attribute = tprimary' CuAt | grep value | cut -d'"' -f2`
			if [ -z "$dumpdev" ]
			then
				ODMDIR=$MROOT/usr/lib/objrepos dumpdev=`odmget -q'uniquetype = sys/node/sys AND attribute = primary' PdAt | grep deflt | cut -d'"' -f2`
			fi
		else
			# 4.1 +, look in SWservAt
			dumpdev=`odmget -q'attribute=tprimary' SWservAt | grep value | cut -d'"' -f2`
		fi
		echo $dumpdev > $MROOT/tmp/bos/dumpdev


		if [ "$BOSLEVEL" = 3.2 -o "$BOSLEVEL" = 3.1 \
										-a ! -s $MROOT/tmp/bos/dumpdev ]
		then
			HD7_THERE="`lqueryvg -p $FIRSTDISK -L | grep hd7 2>/dev/null`"
			if [ "$HD7_THERE" ]
			then
				echo /dev/hd7 >$MROOT/tmp/bos/dumpdev
			fi
		fi

		ODMDIR=$OLDODMDIR
		export ODMDIR

# 311
		# Preserve existing device customization.
		mkdir -p $MROOT/tmp/bos/objrepos.inst >/dev/null 2>&1
	    ODMDIR=/etc/objrepos
	   	export ODMDIR
	   	/usr/lpp/bosinst/rda -e -p $MROOT/etc/objrepos -d $MROOT/tmp/bos/objrepos.inst/CuDv.sav -a $MROOT/tmp/bos/objrepos.inst/CuAt.sav
		rc="$?"

		if [ "$rc" -ne 0 ]
		then
			# Could not preserve previous customized device information.
			# Display a message and continue.
			BI_Error "BOS Install" 5 1
		fi

		# save printer info
		/usr/lpp/bosinst/getprtpkgs -o $MROOT/etc/objrepos \
			  -p /../tmp/device.pkgs

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 56 'Copying old files\n'`"
		# In preservation install, copy a list of files from the old system to
		# be preserved on the the new one.
		# files and remove the BOS files.
		# In migration install, call the pre_i routine which will save config
		BI_Copy_Files


		if [ "$IM" = preserve ]
		then # 325
# 326
# Must use df which is in the RAM file system so we can be sure of the units
# which it outputs.  In 3.2 and 3.1, df used to output 1024-byte blocks,
# in 4.1, it outputs 512-byte blocks.
			# Filter df output through grep to get rid of df header.
			set -- `/usr/bin/df -k $MROOT/tmp | grep /`
			FREE=$3

			# Ensure that there is enough free space in /tmp to make a
			# boot image.
			if (( "$FREE" < ( 7 * 1024 ) ))
			then
				/usr/lib/methods/showled 0xA53
				BI_Error "BOS Install" 6 2
			fi
		fi

		cd /

		PATH=$OLDPATH
		LIBPATH=$OLDLIBPATH
		export PATH LIBPATH
# 316
		# tmp and var may or may not have been mounted, so send output of those
		# umount to /dev/null.
		umount $MROOT/var >/dev/null 2>&1
		umount $MROOT/tmp
		umount $MROOT/usr
		umount /dev/hd4

		Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 57 'Creating logical volumes.\n'`"

# 324
		if [ "$IM" = preserve ]
		then # 325
		# Save the number of copies which hd2, hd4, and hd9var had so that they
		# can be created again with the same number.
# 52
			HD2_LVID=`getlvodm -l hd2`
			set -- `getlvodm -c $HD2_LVID`
			bidata -i -m lv_data -f COPIES -e "$6" -c LOGICAL_VOLUME -v hd2

			HD4_LVID=`getlvodm -l hd4`
			set -- `getlvodm -c $HD4_LVID`
			bidata -i -m lv_data -f COPIES -e "$6" -c LOGICAL_VOLUME -v hd4

			if [ "$HD9_THERE" ]
			then
				HD9_LVID=`getlvodm -l hd9var`
				set -- `getlvodm -c $HD9_LVID`
				bidata -i -m lv_data -f COPIES -e "$6" -c LOGICAL_VOLUME -v hd9var
			fi

# 313
		# Remove /var, /usr, and / in reverse order of which they were created
		# so that they will get recreated in the correct order.  This is so
		# that when they get recreated, they have the correct lvid's.
		# We can tell what order they were created in by looking at the lvids.
			RVG_ID=`getlvodm -v rootvg 2>/dev/null`
			count=0
			LV_LIST[0]=`getlvodm -l hd2 2>/dev/null | cut -d. -f2`
			(( count = count + 1 ))
			LV_LIST[1]=`getlvodm -l hd4 2>/dev/null | cut -d. -f2`
			(( count = count + 1 ))
			# If hd9var is an lv, include it in the list of things to remove.
			if [ "$HD9_THERE" ]
			then
				LV_LIST[2]=`getlvodm -l hd9var 2>/dev/null | cut -d. -f2`
				(( count = count + 1 ))
			fi

		# Make sure the logical volume which was last created comes first in
		# the lv list.  The one created last has the largest lv id.
			i=0
			while [ "$i" -lt "$count" ]
			do
				j=$i
				while [ "$j" -lt "$count" ]
				do
					if [ "${LV_LIST[j]}" -gt "${LV_LIST[i]}" ]
					then
						temp=${LV_LIST[i]}
						LV_LIST[i]=${LV_LIST[j]}
						LV_LIST[j]=$temp
					fi
					(( j = j + 1 ))
				done
				(( i = i + 1 ))
			done

		# Now remove hd2 (/usr), hd4 (/), and hd9var (/var) in reverse order
		# from which they were created.
			i=0
			while [ "$i" -lt "$count" ]
			do
				LV_LIST[i]=`getlvodm -e "$RVG_ID.${LV_LIST[i]}"`
				rmlv -f "${LV_LIST[i]}" >/dev/null 2>&1; rc="$?"
				if [ "$rc" -ne 0 ]
				then
					/usr/lib/methods/showled 0xA55
					# Could not remove the specified logical volume.
					# Cannot continue.
					BI_Error "BOS Install" 7 2 "${LV_LIST[i]}"
				fi
				(( i = i + 1 ))
			done

		# If hd9var was not there, put it in the list now so it can be created
		# later.
			if [ "$NOT" "$HD9_THERE" ]
			then
				LV_LIST[2]=hd9var
				(( count = count + 1 ))
			fi
# 314
		# Now create hd4, hd2, and hd9var in the reverse order from which they
		# were deleted..
			(( i = count - 1 ))
			while [ "$i" -ge 0 ]
			do
				Make_Sys_LV "${LV_LIST[i]}"
				(( i = i - 1 ))
			done

# 315
			for FS in / /usr /var
			do
            	if [ "$FS" != "/usr" -o "$BOS_FORMAT" != turbo ] ; then
                	Make_Sys_FS $FS
            	fi
			done
		fi # 327

# 317
        if [ "$BOS_FORMAT" != turbo ] ; then
            Mount_Rootfs
        fi
    fi
    Change_Status

    return 0
}

# Restore_Bosinst
#     If we booted from tape, position the tape and restore second tape image
#     to get access to BOS install programs and data files.  The bosinst.data
#     and image.data files are contained in that image.  If any of this fails,
#     we have not yet configured the display, so we must show an LED.
#     Always returns 0.
function Restore_Bosinst
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ ! -s "$TAPEDEVICE" ]
	then
		# Put a set -x here for debugging.  There is nothing to the screen at
		# this point, so the only way to see this is through enter_dbg.
		set -x
	fi

	# Position tape.
	tctl -f /dev/$BOOTDEV rewind || loopled 0xA43
	tctl -f /dev/$BOOTDEV.1 fsf 1 || loopled 0xA43

# 5
	# Restore bosinst image.
	if [ ! -s "$TAPEDEVICE" ]
	then
		restbyname -xqSf /dev/$BOOTDEV || loopled 0xA43
	else
		restbyname -xqSf /dev/$BOOTDEV ./bosinst.data ./image.data .$TAPEBLKSZ >/dev/null
	fi

	set +x

# 34
	return 0
}

# Restore_System:
#     Restore bos.rte or mksysb image.  From a SPOT, this will cpio the image.
#     syntax:  Restore_System
#     Always returns 0.
function Restore_System
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 59 'Restoring base operating system.\n'`"

	PT=`bidata -i -g image_data -f PRODUCT_TAPE`

	# Restore image.
	case "$BOOTTYPE" in
		$CDROM )
            if [ "$BOS_FORMAT" = turbo ] ; then
                Turbo_Restore
            else
                if [ "$PT" = yes ]
                then
                    mount $MROOT/usr/lpp/bos/inst_root/tmp /tmp
                else
                    mount $MROOT/tmp /tmp
                fi
                echo /tmp >>/mountlist
                cd $MROOT/
                restbyname -xqf/SPOT/usr/sys/inst.images/bos >/dev/null 2>/dev/console
            fi
			rc="$?"
			if [ "$rc" -ne 0 ]
			then
				/usr/lib/methods/showled 0xA57
				# Restore of AIX Base Operating System Failed.
				BI_Error "BOS Install" 13 2
			fi
		;;
		$TAPE )
# 339
            if [ "$PT" = yes ]
            then
                mount $MROOT/usr/lpp/bos/inst_root/tmp /tmp
            else
                mount $MROOT/tmp /tmp
            fi
            echo /tmp >>/mountlist
            cd $MROOT/
			# If booted from tape, change the tape block size to be that when
			# the install image (bos.rte or mksysb) was backed up.
			RESTORE_BFLAG=""  ## set -b parm to null
			if [ -s "$TAPEBLKSZ" ]
			then
				# Change tape block size, this will cause tape to rewind.
				 set -- `cat $TAPEBLKSZ`
				 TBLOCKSZ=$1
				 if [ "$2" != "NONE" ]
				 then       ## set -b option
				     RESTORE_BFLAG="-b "$2
				 fi
				 # BLock Sizes less than 512 are not supported
				 # so leave the tape blocked at 512
				 if [ $TBLOCKSZ -gt 512 ]
				 then
				    /etc/methods/ucfgdevice -l "$BOOTDEV"
				    /etc/methods/chggen -l $BOOTDEV -a block_size=$TBLOCKSZ
				    `lsdev -Cl$BOOTDEV -rConfigure` -l $BOOTDEV
				 fi
			else
				 # The TAPEBLKSZ file only exists in mksysb
				 #  images, so for product tapes set the
				 #  blocking factor to 64.  (-C is same as -b)
				 RESTORE_BFLAG="-b 64"
			fi

# 340
			tctl -f /dev/$BOOTDEV.1 rewind
			tctl -f /dev/$BOOTDEV.1 fsf 3
			if [ "$PT" = yes ]
			then
				restbyname -xqf/dev/$BOOTDEV.1 $RESTORE_BFLAG -h -S >/dev/null 2>/dev/console ; rc="$?"
			else
				restbyname -xqf/dev/$BOOTDEV.1 $RESTORE_BFLAG >/dev/null 2>/dev/console ; rc="$?"
			fi
			if [ "$rc" -ne 0 ]
			then
				/usr/lib/methods/showled 0xA57
				# Restore of AIX Base Operating System Failed.
				BI_Error "BOS Install" 14 2 "$BOOTDEV"
			fi
		;;
		$NETWORK )
			case "$NIM_BOS_FORMAT" in
			rte|mksysb )
                if [ "$PT" = yes ]
                then
                    mount $MROOT/usr/lpp/bos/inst_root/tmp /tmp
                else
                    mount $MROOT/tmp /tmp
                fi
                echo /tmp >>/mountlist
                cd $MROOT/
				if [ "$PT" = yes ]
				then
					restbyname -xqf$NIM_BOS_IMAGE -h -S >/dev/null 2>/dev/console ; rc="$?"
				else
					restbyname -xqf$NIM_BOS_IMAGE -S >/dev/null 2>/dev/console ; rc="$?"
				fi
			;;
			spot )
                if [ "$PT" = yes ]
                then
                    mount $MROOT/usr/lpp/bos/inst_root/tmp /tmp
                else
                    mount $MROOT/tmp /tmp
                fi
                echo /tmp >>/mountlist
                cd $MROOT/
				cd "$NIM_BOS_IMAGE" || rc="$?"
				mount

				find ./usr -print | grep -v usr/sys/inst.images | \
					cpio -padu $MROOT >/dev/null 2>/dev/console
				rc="$?"
				
				# Since we greped out usr/sys/inst.images, we need
				# to mke the directory.
				mkdir -p $MROOT/usr/sys/inst.images

# Mksysb from spot not allowed.
#				if [ "$PT" = "no" ]
#				then
#					cd $MROOT/usr/lpp/bos/inst_root
#					find . -print | cpio -pmduv $MROOT >/dev/null
#				fi
			;;
            turbo )
                Turbo_Restore
            ;;
			esac
			if [ "$rc" -ne 0 ]
			then
				/usr/lib/methods/showled 0xA57
				# Restore of AIX Base Operating System Failed.
				BI_Error "BOS Install" 14 2 "$NIM_BOS_IMAGE"
			fi
		;;
	esac

	# Remove /dev/tty to accommodate TCB
	rm -f /dev/tty

	cd /

	return 0
}

# Run_Customization:
#     If it exists, run customization script.
#     syntax:  Run_Customization
#     Always returns 0.
function Run_Customization
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
	Display_Status_Screen -c

	CF=`bidata -b -g control_flow -f CUSTOMIZATION_FILE`
	if [ -z "$CF" ]
	then
		CF=`bidata -i -g post_install_data -f BOSINST_FILE`
	fi

	cd /

	# If the CF variable is set and it exists in the RAM file system,
	# copy it to the hard disk.
	if [ -f "/../$CF" ]
	then
		mkdir -p `dirname $CF` >/dev/null 2>&1
		cp /../$CF $CF
	fi

	if [ -f "$CF" ]
	then
		if [ "$BOOTTYPE" -eq "$NETWORK" ]
		then
			${NIMCLIENTPATH}nimclient -o change -a force=yes \
					-a ignore_lock=yes \
					-a info="BOS install customization"
		fi
		ksh $CF
	fi

	return 0
}

# If CONSOLE is set in bosinst.data and if PROMPT = no (non-prompt mode),
# update the CuAt with the value of CONSOLE.  Only do this in non-prompt
# mode because if CONSOLE is set to a device that does not have a display
# attached, the user who should be prompted has no way of changing their
# input device.  Then it will call cfgcon, which will do the work of
# determining whether or not to prompt the user to pick the console.  If the
# value which was placed in CuAt for the console attribute is valid, the user
# will not be prompted.  However, if cfgcon determines that the value is
# either invalid or not there, it will prompt the user for the system
# console (console finder).
#     Always returns 0.
function Set_Console
{
	trap 'BI_Error "BOS Install" 29 2' INT
	if [ "$BOOTTYPE" -eq "$NETWORK" ]
	then
		${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
			-a info="setting_console"
	fi

# 26
	C="`bidata -b -g control_flow -f CONSOLE`"
	P="`bidata -b -g control_flow -f PROMPT`"
	if [ "$C" -a "$P" = no ]
	then # 27
# 30
		# on NIM install, disable login on console if its a file
		if [[ "$BOOTTYPE" -eq "$NETWORK"  ]] && [[ "$C" != /dev/?* ]]
		then
			chcons -a login=disable $C
		else
			if [[ $C = none ]]
			then
				chcons -a login=disable /dev/null
			else
				chcons $C
			fi
		fi
	fi # 28

# 31
	/usr/lib/methods/cfgcon || loopled 0xA45

	unset C P

	# If /dev/tty does not exist, link it to /dev/console.  This is necessary
	# for restbyname (multivolume support) to work properly.
	# This link is removed at the end of Restore_System
	# to accommodate TCB
	ln -f /dev/console /dev/tty >/dev/null 2>&1

	CLEAR=`/usr/lpp/bosinst/bi_io -c < /dev/console`
	if [ -z "$CLEAR" ]
	then
	    # 24 newlines - can't use '\n' because BosMenus can't 
	    # interpret them
	    CLEAR="






















	"
	fi
	export CLEAR


	return 0
}

# Set_PP_Size:
#	  Determine the physical partition size for the root volume
#	  group.
#     syntax:  Set_PP_Size
#     Always returns 0.
function Set_PP_Size
{
	trap 'BI_Error "BOS Install" 29 2' INT
	# If install method is overwrite, set the physical partition size
	# based on the value required by the largest target disk. 
	IM=`bidata -b -g control_flow -f INSTALL_METHOD`
	if [ "$IM" = "overwrite" ]
	then
		PPSZ_VG=0
		for hdname in `bidata -b -g target_disk_data -f HDISKNAME`
		do
			# Determine largest physical partition size required
			# by the target disks.
			PPSZ_PV=`bootinfo -P 0 -s $hdname`
			if [ "PPSZ_PV" -gt "$PPSZ_VG" ]
			then
				PPSZ_VG="$PPSZ_PV"
			fi
		done
		# If requested physical partition size is null or
		# smaller than the largest value required by a target disk,
		# override it.
		[ -z "$PPSZ" -o "$PPSZ_VG" -gt "$PPSZ" ] && PPSZ="$PPSZ_VG"
	else
		# else install method is either migrate or preserve, so a root vg
		# already exists.  See what the size of the PPs are.
		set -- `bidata -b -g target_disk_data -f HDISKNAME`
		D=$1
		PPSZ=`lqueryvg -p$D -s`
		# Convert PPSZ to number of meg.
		(( PPSZ = ( 2 << ( PPSZ - 1 ) ) / 1048576 ))
	fi
	bidata -i -m vg_data -f PPSIZE -e $PPSZ -c VGNAME -v rootvg

	return 0
}

# Set_Primary_Locale:  invokes support for selected cultural convention,
#   keyboard map, and message catalogs.
#     syntax:  Set_Primary_Locale
#     Always returns 0.
function Set_Primary_Locale
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	# Based on the variables CULTURAL_CONVENTION, MESSAGES, and KEYBOARD,
	# build a list of packages to install.  Get the package information from
	# the CC, MESSAGES, and KEYBOARD ODM databases.
	CC=`bidata -b -g locale -f CULTURAL_CONVENTION`
	MSG=`bidata -b -g locale -f MESSAGES`
	KBD=`bidata -b -g locale -f KEYBOARD`

	# If packages did not install correctly, return.
	PACKAGES=`{ odmget -qlocale="$CC" CC 2>/dev/null; \
                odmget -qlocale="$MSG" MESSAGES 2>/dev/null; \
                odmget -qkeyboard_map="$KBD" KEYBOARD 2>/dev/null; } | \
		grep package | cut -d'"' -f2 | tr ' ' '\n' | tr '\t' '\n' | sort -u`

	VAR_LIST=
	for pkg in $PACKAGES
	do
		STATUS=`fgrep :"$pkg": /var/adm/sw/installp.summary | cut -d: -f1`
		for status in $STATUS
		do
			if [ "$status" != 0 ]
			then
				VAR_LIST=bad;
			fi
		done
	done
	if [ -n "$VAR_LIST" ]
	then
		dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 66 \
		"Some ILS packages did not install.  See /var/adm/ras/devinst.log\n\
		for details"
	fi

	VAR_LIST=
	# Set variables in /etc/environment.
	VAR_LIST=`odmget -qlocale=$MSG MESSAGES | grep variables | cut -d'"' -f2 | cut -d'=' -f2`

	if [ -z "$VAR_LIST" -o "$VAR_LIST" = "$CC" ]
	then
		chlang -M $CC
    else
		chlang -m $VAR_LIST $CC
	fi

	# Now get variables in KEYBOARD data base.
	VAR_LIST=`odmget -qkeyboard_map=$KBD KEYBOARD | grep variables | cut -d'"' -f2`

	# Get the proper font.
	CODESET=`odmget -q"locale = $CC" CC | grep codeset | cut -d'"' -f2`
	FONT_CMD=
	if [ "$CODESET" ]
	then
		FONT_CMD=`odmget -qcodeset="$CODESET" FONT | grep font_cmd | cut -d'"' -f2`
	fi

	# Get command to run to set proper keyboard map.
	RUN_KBD_CMD=`odmget -q"keyboard_map = $KBD" KEYBOARD | \
					grep keyboard_cmd | cut -d'"' -f2`

# 366
	# If there is an lft running, change the keyboard map and font.
	havelft=`lsdev -C -l lft0 -S Available`
	if [ -n "$havelft" -a "$CODESET" ]
	then
		$FONT_CMD
		: # Put a colon here in case $FONT_CMD is null.
	fi
	if [ -n "$havelft" -a "$RUN_KBD_CMD" ]
	then
		$RUN_KBD_CMD
		: # Put a colon here in case $RUN_KBD_CMD is null.
	fi

# 367
	for varline in $VAR_LIST
	do
		var=`echo $varline | cut -f1 -d'='`
		grep -v "^${var}=" /etc/environment >/etc/environment.new
		echo $varline >>/etc/environment.new
		cp /etc/environment.new /etc/environment
	done
	rm -f /etc/environment.new

	return 0
}

# Shrink_FS:
#	  Set up a file system to be shrunk when recreated
#	  FS_MIN_SIZE has to have been set before this function is called
#     
#     syntax:  Shrink_FS logical_volume
#     Returns 0
function Shrink_FS
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi

	LV=$1
	FS_MIN=`bidata -i -g fs_data -f FS_MIN_SIZE -c FS_LV -v $LV`
	# First ensure that FS_MIN is not null.
	FS_MIN=${FS_MIN:=0}
	# Calculate size in number of physical partitions needed based on
	# FS_MIN_SIZE for this file system.  1/2048 = 512 / 1Meg.
	(( S = FS_MIN % 2048 ))
	if [ "$S" -ne 0 -o "$FS_MIN" -eq 0 ]
	then
		(( FS_MIN = ( FS_MIN / 2048 ) + 1 ))
	else
		(( FS_MIN = FS_MIN / 2048 ))
	fi

	PPSZ=`bidata -i -g vg_data -f PPSIZE -c VGNAME -v rootvg`
	# If PPSZ will not go into FS_MIN evenly, round up to
	# the next Physical Partition boundary.
	(( R = FS_MIN % PPSZ ))
	if [ "$R" -ne 0 ]
	then
		(( NUMLPS = ( FS_MIN / PPSZ ) + 1 ))
	else
		(( NUMLPS = FS_MIN / PPSZ ))
	fi

	# Change number of logical partitions to be only that necessary
	# to contain the files.
	bidata -i -m lv_data -f LPs -e $NUMLPS \
							-c LOGICAL_VOLUME -v ${LV#/dev/*}

	# Round out FS_MIN_SIZE to fill the logical volume.
	(( FS_MIN = NUMLPS * PPSZ * 2048 ))

	# Change FS_SIZE in the fs_data stanza to the value of FS_MIN_SIZE.
	bidata -i -m fs_data -f FS_MIN_SIZE -e $FS_MIN -c FS_LV -v $LV

	return 0
}

# Shrink_It:
#     For logical volumes which contain jfs file systems, this will change the
#     number of logical partitions to be no more than that necessary to contain
#     the file system.
#     syntax:  Shrink_It
#     Always returns 0.
function Shrink_It
{
	trap 'BI_Error "BOS Install" 29 2' INT

	if [ "$SETX" -eq 1 ]
	then
		set -x
	fi
# 277 # 278 # 279 # 284 # 285 # 286
	# If it has been specified to shrink the lvs containing jfs file systems
	# to the size of the lv so as to eliminate free space, set LPs field
	# in lv_data stanzas to be correct size.  Also, change FS_SIZE to be
	# FS_MIN_SIZE.
	SHR=`bidata -i -g logical_volume_policy -f SHRINK`
	PT=`bidata -i -g image_data -f PRODUCT_TAPE`
	if [ "$SHR" = yes -a "$PT" = no ]
	then
		for LV in `bidata -i -g fs_data -f FS_LV`
		do
			Shrink_FS $LV
		done
	fi

	return 0
}

# 
function Turbo_Restore
{
    trap 'BI_Error "BOS Install" 29 2' INT
    Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 82 'Turbo Installing.\n'`"
	uncompress -c /SPOT/usr/turbo/hd2.Z | dd of=/dev/rhd2 bs=1024k  &
	DDPID=$!
	if [ -s /../Update_Status.pn ]
	then
		kill -STOP `cat /../Update_Status.pn`
		Update_Status -i
	fi
	sleep 5
	echo "$CLEAR\c" >/dev/console
        turbocr </SPOT/usr/turbo/copyrights >/dev/console 
	# Redraw the status screen.
	echo "$CLEAR\c" >/dev/console
	if [ `bootinfo -k` != 3 ]
	then
		/usr/lpp/bosinst/berror -f " " -e 30 -a 1 >/dev/console 2>&1
	else
		/usr/lpp/bosinst/berror -f " " -e 25 -a 1 >/dev/console 2>&1
	fi
	# Restart the status update routine.
	if [ -s /../Update_Status.pn ]
	then
		kill -CONT `cat /../Update_Status.pn`
	else
		Update_Status -i
	fi
	wait $DDPID
    if [ "$?" -ne 0 ] ; then
        # Restore of BOS Operating System Failed
        BI_Error "BOS Install" 13 2
    fi
    cp /SPOT/usr/turbo/filesystems /etc
    sync ; sync
    Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 83 'Checking file systems.\n'`"
    chfs -m /usr -a LogName=/dev/hd8 /dev/hd2
    fsck -n /dev/hd2 </dev/null
    if [ "$?" -ne 0 ] ; then
        BI_Error "BOS Install" 10 2 "/usr"
    fi
# Temporarily mount the root, var & tmp filesystems and cpio from inst_root
    Change_Status "`dspmsg /../usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 84 'Installing root file systems\n'`"
    logdev=/dev/hd8
    mkdir -p $MROOT/usr
    fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /usr`
    mount -o log=$logdev $fs_lv $MROOT/usr
    mkdir /tmpm
    fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /`
    mount -o log=$logdev $fs_lv /tmpm
    mkdir -p tmpm/var
    fs_lv=`bidata -i -g fs_data -f FS_LV -c FS_NAME -v /var`
    mount -o log=$logdev $fs_lv /tmpm/var
    IM=`bidata -b -g control_flow -f INSTALL_METHOD`
    if [ "$IM" = overwrite ] ; then
        if [ -n "$HOME_THERE" ] ; then
            mkdir -p tmpm/home
            mount -o log=$logdev $HOME_LV /tmpm/home
        fi
    fi
    if [ -n "$TMP_LV"  ]
    then
        mkdir -p tmpm/tmp
        mount -o log=$logdev $TMP_LV /tmpm/tmp
    fi
    cd $MROOT/usr/lpp/bos/inst_root
    find . -print | cpio -padu /tmpm >/dev/null 2>&1
    if [ "$?" -ne 0 ] ; then
        # Restore of BOS Operating System Failed
        BI_Error "BOS Install" 13 2
    fi
    if [ "$IM" = overwrite  -a -n "$HOME_THERE" ] ; then
        umount /tmpm/home
    fi
    if [ -n "$TMP_LV"  ]
    then
        umount /tmpm/tmp
    fi
    umount /tmpm/var
    umount /tmpm
    rm -fr $MROOT/usr/lpp/bos/inst_root/*
    cd /
    umount $MROOT/usr
    Mount_Rootfs
    if [ "$PT" = yes ]
    then
        mount $MROOT/usr/lpp/bos/inst_root/tmp /tmp
    else
        mount $MROOT/tmp /tmp
    fi
    echo /tmp >>/mountlist
    rc="$?"
    cd $MROOT/
    return 0
}


# Update_Status runs in the background and updates the screen every minute.
# Optional parameter -i will cause an immediate update of the status line
# and return
function Update_Status
{
	while :
	do
		read PERCENT < /../percent
		read WORK_IN_PROGRESS < /../wip
		(( TIME = ( $SECONDS - START_TIME ) / 60 ))
		STATUS_LINE="          $PERCENT               $TIME      $WORK_IN_PROGRESS"
        # echo 80 blanks
		echo "                                                                               \c" > /dev/console
		echo "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\c" > /dev/console
		echo "$STATUS_LINE\c" >/dev/console

		len=${#STATUS_LINE}
		while (( len > 0 ))
		do
			echo "\b\c"
			(( len = len - 1 ))
		done > /dev/console

		if [ "$REALMEM" -le 8192 -o "$1" = '-i' ]
		then
			break
		fi

		if [ -f /../usr/bin/sleep ]
		then
			/../usr/bin/sleep 20
		fi
	done
}

# Verify_Image_Parameters:
#     Verifies that the stanzas in the image.data file are correct with
#     regars to size, placement, and number of logical volumes and available
#     disks.
#     syntax:  Verify_Image_Parameters
#     Always returns 0.
function Verify_Image_Parameters
{
	trap 'BI_Error "BOS Install" 29 2' INT

	# If they went down the maintenance path for a mksysb install, we cannot
	# use maps in creating the logical volumes.
	if [ -f "$TAPEDEVICE" ]
	then
		bidata -i -m logical_volume_policy -f EXACT_FIT -e no
		bidata -i -m lv_data -f MAPFILE -e " "
	fi
# 65
	ML=`bidata -i -g ils_data -f LANG`
	if [ -n "$ML" ]
	then # 67
# 71
		# If LANG variable specified in image.data is valid, and BOS
		# install language specified in bosinst.data is null, set
		# BOSINST_LANG to LANG variable as specified in image.data.
		IL_OK=`odmget -q"locale=$ML AND bosinst_translated=y" MESSAGES 2>/dev/null`
		BL=`bidata -b -g locale -f BOSINST_LANG`
		if [ -n "$IL_OK" -a -z "$BL" ]
		then # 74
# 84
			bidata -b -m locale -f BOSINST_LANG -e $ML
		fi # 75

	fi # 68

# 52
	# Validate that a vg_data for rootvg exists.
	RVG_STANZA_EXISTS=`bidata -i -g vg_data -f VGNAME`
	if [ "$RVG_STANZA_EXISTS" != rootvg ]
	then
		# If ERROR_EXIT is set, run it and then flag that user must
		# be prompted.  If they want to stop before the menus, they
		# may do so with the error exit.
		error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
		if [ -n "$error_exit" ]; then "$error_exit"; fi

		ERRORTEXT="    The image.data file contains no vg_data stanza
    for rootvg.  The installation cannot continue."
		ERRORNUM=3
		FATAL=TRUE
		bidata -b -m control_flow -f PROMPT -e yes
		bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
	fi

# 91

# 153
	# First see if the image.data file contains a paging device.
	ANY_PAGELVS=
	ANY_PAGELVS=`bidata -i -g lv_data -f LOGICAL_VOLUME -c TYPE -v paging`

	# Then see if all necessary logical volumes are specified in the rootvg.
	# Namely, hd2, hd4, hd5, hd8, hd9var.
	ALL_LVS=TRUE
	for I in hd2 hd4 hd5 hd8 hd9var
	do
		THERE=`bidata -i -g lv_data -f LOGICAL_VOLUME -c LOGICAL_VOLUME -v $I`
		if [ "$NOT" "$THERE" ]
		then
			ALL_LVS=
			break
		fi
	done

# 154
	if [ "$NOT" "$ANY_PAGELVS" -o "$NOT" "$ALL_LVS" ]
	then
		# If ERRORTEXT is not already set, set it now.  This is so that the
		# first error that occurs will be displayed.  We don't just return,
		# because as much processing should be done on the image.data file as
		# possible.
		if [ "$NOT" "$ERRORTEXT" ]
		then
			# If ERROR_EXIT is set, run it and then flag that user must
			# be prompted.  If they want to stop before the menus, they
			# may do so with the error exit.
			error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
			if [ -n "$error_exit" ]; then "$error_exit"; fi

			ERRORTEXT='image.data has invalid logical volume data.
Can not continue.'
			ERRORNUM=7
			FATAL=TRUE
		fi
		bidata -b -m control_flow -f PROMPT -e yes
		bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
	fi

	# Check to ensure that hd2, hd4, and hd9var are associated with the
	# /usr, /, and /var file systems, respectively.
# 155
	ALL_SYS_FSS=TRUE
	for I in hd2 hd4 hd9var
	do
		THERE=`bidata -i -g fs_data -f FS_NAME -c FS_LV -v /dev/$I`
		THERE=`echo $THERE`
		case "$I" in
			hd2    )	if [ "$THERE" != /usr ]
						then
							ALL_SYS_FSS=
							break
						fi
				;;
			hd4    )	if [ "$THERE" != / ]
						then
							ALL_SYS_FSS=
							break
						fi
				;;
			hd9var )
						if [ "$THERE" != /var ]
						then
							ALL_SYS_FSS=
							break
						fi
				;;
		esac
	done

	ALL_FSS_HAVE_LVS=TRUE
	for I in `bidata -i -g fs_data -f FS_LV`
	do
		lv=`echo $I | cut -d'/' -f3`
		THERE=`bidata -i -g lv_data -f LOGICAL_VOLUME -c LOGICAL_VOLUME -v $lv`

		if [ "$NOT" "$THERE" ]
		then
			ALL_FSS_HAVE_LVS=
			break
		fi
	done

# 156
	if [ "$NOT" "$ALL_SYS_FSS" -o "$NOT" "$ALL_FSS_HAVE_LVS" ]
	then
		# If ERRORTEXT is not already set, set it now.  This is so that the
		# first error that occurs will be displayed.  We don't just return,
		# because as much processing should be done on the image.data file as
		# possible.
		if [ "$NOT" "$ERRORTEXT" ]
		then
			# If ERROR_EXIT is set, run it and then flag that user must
			# be prompted.  If they want to stop before the menus, they
			# may do so with the error exit.
			error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
			if [ -n "$error_exit" ]; then "$error_exit"; fi

			ERRORTEXT='image.data has invalid file system data.
Can not continue.'
			ERRORNUM=8
			FATAL=TRUE
		fi
		bidata -b -m control_flow -f PROMPT -e yes
		bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
	fi

# 92
	PT=`bidata -i -g image_data -f PRODUCT_TAPE`
	if [ "$PT" = "no" ]
	then # 94
		# Set INSTALL_METHOD to "overwrite", since all mksysb installs are
		# overwrite.
# 100
		bidata -b -m control_flow -f INSTALL_METHOD -e overwrite

# 101
		# If booted over network, and NIM_BOS_FORMAT is set but
		# equals "spot", this is an error, since you cannot do a mksysb over
		# network from a SPOT.
		if [ -n "$NIM_BOS_FORMAT" -a "$NIM_BOS_FORMAT" = spot ]
		then
			# If ERRORTEXT is not already set, set it now.  This is so that the
			# first error that occurs will be displayed.  We don't just return,
			# because as much processing should be done on the image.data file
			# as possible.
			if [ "$NOT" "$ERRORTEXT" ]
			then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				ERRORTEXT='The data file indicated that a mksysb install be done
from a SPOT.  This is an illegal operation.'
				ERRORNUM=9
			fi
			bidata -b -m control_flow -f PROMPT -e yes
			bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
		fi

# 102
		SHR=`bidata -i -g logical_volume_policy -f SHRINK`
		if [ "$SHR" = "yes" ]
		then # 103
# 105
			# Add up non-jfs lvs.  Then add to that the number specified in
			# fs_data stanzas which specify the least amount of space required
			# tor the file system to fit.  Then add total disk size and see
			# if there is enough total disk space to hold the root volume
			# group, based on total size required to contain rootvg.


			# Get all logical volumes in the rootvg and sum the logical
			# partitions needed for those which are not jfs file systems.
			SIZE_NON_JFS=0
			for lv in `bidata -i -g lv_data -f LOGICAL_VOLUME -c VOLUME_GROUP -v rootvg`
			do
				name=`bidata -i -g fs_data -f FS_LV -c FS_LV -v /dev/$lv`
				# If the name is null, that means that there was no jfs
				# file system stanza associated with this logical volume.
				# If there is no file system add it up.
				if [ ! "$name" ]
				then
					copies=`bidata -i -g lv_data -f COPIES -c LOGICAL_VOLUME -v $lv`
					SOURCE_PPSIZE=`bidata -i -g lv_data -f PP_SIZE -c LOGICAL_VOLUME -v $lv`
					NUM_LPs=`bidata -i -g lv_data -f LPs -c LOGICAL_VOLUME -v $lv`
					(( SIZE_NON_JFS = SIZE_NON_JFS + ( NUM_LPs * SOURCE_PPSIZE * copies ) ))
				fi
			done

			# Gather total size needed for file systems if free space is
			# omitted (ie, shrink it down to the size specified in FS_MIN_SIZE).
			TOTAL_LV_SIZE_MB=0
			for fs_lv in `bidata -i -g fs_data -f FS_LV`
			do
				FS_MIN=`bidata -i -g fs_data -fFS_MIN_SIZE -c FS_LV -v "$fs_lv"`
				fs_lv=${fs_lv##*/}
				copies=`bidata -i -g lv_data -fCOPIES -c LOGICAL_VOLUME -v $fs_lv`

				# PP size is in megabytes, FS_MIN is in 512 byte blocks.
				# Must convert FS_MIN to megabytes before doing arithmetic.
				# so can have a common base.  1/2048 = 512 / 1Meg.

				# First ensure that FS_MIN is not null.
				FS_MIN=${FS_MIN:=0}
				# Then see if FS_MIN is evenly divisible by 2048.  If it is not,
				# divide by 2048 and add 1 to round up to the nearest megabyte.
				# If it is, just divide by 2048.
				(( S = FS_MIN % 2048 ))
				if [ "$S" -ne 0 -o "$FS_MIN" -eq 0 ]
				then
					(( FS_MIN = ( FS_MIN / 2048 ) + 1 ))
				else
					(( FS_MIN = FS_MIN / 2048 ))
				fi

				# Now, sum the adjusted FS_MIN values.
				(( TOTAL_LV_SIZE_MB = TOTAL_LV_SIZE_MB + ( FS_MIN * copies) ))
			done

			# Add total needed for file systems plus total needed for non jfs
			# logical volumes
			(( TOTAL_LV_SIZE_MB = TOTAL_LV_SIZE_MB + SIZE_NON_JFS ))
		else # 104
# 106
			# Sum logical partitions times physical partition size to get total
			# size needed in megabytes.
			TOTAL_LV_SIZE_MB=0
			for lv in `bidata -i -g lv_data -f LOGICAL_VOLUME -c VGNAME -v rootvg`
			do
				num_pps=`bidata -i -g lv_data -f LPs -c LOGICAL_VOLUME -v $lv`
				pp_size=`bidata -i -g lv_data -f PP_SIZE -c LOGICAL_VOLUME -v $lv`
				copies=`bidata -i -g lv_data -f COPIES -c LOGICAL_VOLUME -v $lv`
				(( TOTAL_LV_SIZE_MB = TOTAL_LV_SIZE_MB + (num_pps * pp_size * copies) ))
			done
		fi

# 107
		# Add total disk size and see if there is enough total disk space
		# to hold the root volume group, based on TOTAL_LV_SIZE_MB.
		TOTAL_DISK_SIZE=0
		ONE_BOOTABLE=
		for DISK in `lsdev -Ccdisk -S Avaliable -F "name"`
		do
			if [ `bootinfo -B $DISK` -eq 1 ]
			then
				ONE_BOOTABLE=TRUE
			fi
			SZ=`bootinfo -s $DISK`
			if [ -n "$SZ" ]
			then
				(( TOTAL_DISK_SIZE = TOTAL_DISK_SIZE + SZ ))
			fi
		done

		if [ "$NOT" "$ONE_BOOTABLE" ]
		then
			# If ERRORTEXT is not already set, set it now.  This is so that the
			# first error that occurs will be displayed.  We don't just return,
			# because as much processing should be done on the image.data file
			# as possible.
			if [ "$NOT" "$ERRORTEXT" ]
			then
				# If ERROR_EXIT is set, run it and then flag that user must
				# be prompted.  If they want to stop before the menus, they
				# may do so with the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				ERRORTEXT="There are no disks on this system which can be booted."
				ERRORNUM=10
			fi
			bidata -b -m control_flow -f PROMPT -e yes
			bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
		elif [ "$TOTAL_LV_SIZE_MB" -le "$TOTAL_DISK_SIZE" ]
		then # 108
			# then there is enough disk space to contain the rootvg.
			# If there were no disks already specified in the bosinst.data
			# file and if EXIXTING_SYSTEM_OVERWRITE is no, pick only disks
			# which have no volume group on them to be in the rvg.
			# If there were no disks already specified in the bosinst.data file
			# and if EXISTING_SYSTEM_OVERWRITE is yes, first pick the old
			# rootvg(s) and see if there is enough space there to hold the
			# mksysb image.  If not, add to that the disks with no vg and see
			# if there is enough space there to hold the mksysb image.  If not
			# pick disks from the remaining disks until the target rootvg will
			# contain the mksysb image.
# 110
			if [ "$NULL_TARGET" ]
			then # 114
				ESO=`bidata -b -g control_flow -f EXISTING_SYSTEM_OVERWRITE`
# 116
				if [ "$ESO" = no ]
				then
					# This will be using only those disks which have no vgid.
					# Although we checked for bootable disks before, this
					# search will be narrowed to bootable disks which have no
					# vgid.

					FREE_VGS=`grep "^$NOVGID" $TARGETVGS`
					if [ -z "$FREE_VGS" ]
					then
						# If ERRORTEXT is not already set, set it now.  This
						# is so that the first error that occurs will be
						# displayed.  We don't just return, because as much
						# processing should be done on the image.data file
						# as possible.
						if [ "$NOT" "$ERRORTEXT" ]
						then
							# If ERROR_EXIT is set, run it and then flag
							# that user must be prompted.  If they want to
							# stop before the menus, they may do so with
							# the error exit.
							error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
							if [ -n "$error_exit" ]; then "$error_exit"; fi

							ERRORTEXT="You chose to install only onto disks which are not
contained in a volume group, but there are not enough
of those disks to contain the mksysb image."
							ERRORNUM=11
						fi
						bidata -b -m control_flow -f PROMPT -e yes
						bidata -b -m control_flow \
									-f EXISTING_SYSTEM_OVERWRITE -e yes
					fi

					rm -f /tmp/available
					ONE_BOOTABLE=
					# NOTE:  The following 'grep piped to while' will produce
					#    different results if run in the Bourne shell.
					grep "^$NOVGID" $TARGETVGS | while read a b c d e f BOOTABLE
					do
						if [ "$BOOTABLE" = y ]
						then
							ONE_BOOTABLE=TRUE
							echo "$a $b $c $d $e $f $BOOTABLE" >> /tmp/available
						fi
					done

					# If there was at least one bootable disk with no vgid,
					# put the non-bootable ones in the available list and start
					# selecting defaults.
					if [ "$ONE_BOOTABLE" ]
					then
						# Now put any non-bootable disks with no vg into the
						# available list.
						# NOTE:  The following 'grep piped to while' will
						# produce different results if run in the Bourne shell.
						grep "^$NOVGID" $TARGETVGS | \
							while read a b c d e f BOOTABLE
							do
								if [ "$BOOTABLE" = n ]
								then
									echo "$a $b $c $d $e $f $BOOTABLE" >> /tmp/available
								fi
							done

						# Since this specifies to not overwrite existing data,
						# pick only disks not in a vg to conatin the mksysb
						# image.
						rm -f /tmp/tdd.add
						TOTAL_DISK_SIZE=0
						# NOTE:  The following 'cat piped to while' will produce
						#    different results if run in the Bourne shell.
						cat /tmp/available | \
							while read VGID rvg lvl DISK LOC SIZE b
							do
								if [ "$VGID" = "$NOVGID" ]
								then
									(( TOTAL_DISK_SIZE = \
													TOTAL_DISK_SIZE + SIZE ))
									echo "$LOC $SIZE $DISK" >> /tmp/tdd.add

									# If enough disks have been selected, get
									# out of loop.
									if [ "$TOTAL_DISK_SIZE" -ge \
														"$TOTAL_LV_SIZE_MB"  ]
									then
										break
									fi
								fi
							done
						# If there were not enough empty disks to contain the
						# mksysb image, flag an error.
						if [ "$TOTAL_DISK_SIZE" -lt "$TOTAL_LV_SIZE_MB"  ]
						then
							# If ERRORTEXT is not already set, set it now.  This
							# is so that the first error that occurs will be
							# displayed.  We don't just return, because as much
							# processing should be done on the image.data file
							# as possible.
							if [ "$NOT" "$ERRORTEXT" ]
							then
								# If ERROR_EXIT is set, run it and then flag
								# that user must be prompted.  If they want to
								# stop before the menus, they may do so with
								# the error exit.
								error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
								if [ -n "$error_exit" ]; then "$error_exit"; fi

								ERRORTEXT="You chose to install only onto disks which are not
contained in a volume group, but there are not enough
of those disks to contain the mksysb image."
								ERRORNUM=11
							fi
							bidata -b -m control_flow -f PROMPT -e yes
							bidata -b -m control_flow \
									-f EXISTING_SYSTEM_OVERWRITE -e yes
						fi
					else
						if [ "$NOT" "$ERRORTEXT" ]
						then
							# If ERROR_EXIT is set, run it and then flag
							# that user must be prompted.  If they want to
							# stop before the menus, they may do so with
							# the error exit.
							error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
							if [ -n "$error_exit" ]; then "$error_exit"; fi

							ERRORTEXT="You chose to install only onto disks which are not
contained in a volume group, but there are none which
can be booted."
							ERRORNUM=12
						fi
						bidata -b -m control_flow -f PROMPT -e yes
						bidata -b -m control_flow \
									-f EXISTING_SYSTEM_OVERWRITE -e yes
					fi
				else
				# Even though it is all right to overwrite any of the disks,
				# select disks carefully such that disks in non-rvg volume
				# groups are the last to be destroyed.

					# This will be set if we have any bootable disks.
					ONE_BOOTABLE=

					rm -f /tmp/tdd.add
					TOTAL_DISK_SIZE=0
					cp "$TARGETVGS" /tmp/tmpvgs.file

					# Keep getting disks from existing root volume groups until
					# we have selected enough disk space or there are no more
					# root volume groups.
					FIRST_RVG_VGID=TRUE
					while [ "$TOTAL_DISK_SIZE" -lt "$TOTAL_LV_SIZE_MB"  -a \
														-n "$FIRST_RVG_VGID" ]
					do
						# Open file descriptor 3 for read.
						exec 3< /tmp/tmpvgs.file

						# Get the VGID of the first rootvg.
						FIRST_RVG_VGID=
						while read -u3 VGID RVG lvl DISK LOC SIZE b
						do
							if [ "$RVG" = 1 ]
							then
								ONE_BOOTABLE=TRUE
								FIRST_RVG_VGID="$VGID"
								break
							fi
						done

						# Close file descriptor 3.
						exec 3<&-

						# Select the disks which were in the previous rvg.
						# If there are no rvgs, FIRST_RVG_VGID will be blank,
						# an the following loop will be skipped.
						if [ -n "$FIRST_RVG_VGID" ]
						then
							grep "^$FIRST_RVG_VGID" /tmp/tmpvgs.file | \
							while read vgid rvg lvl DISK LOC SIZE b
							do
								(( TOTAL_DISK_SIZE = TOTAL_DISK_SIZE + SIZE ))
								echo "$LOC" "$SIZE" "$DISK" >> /tmp/tdd.add
							done
							# Take out used disks from tmpvgs.file.
							grep -v "^$FIRST_RVG_VGID" /tmp/tmpvgs.file>/tmp/foo
							mv /tmp/foo /tmp/tmpvgs.file
						fi
					done

					# If not enough disks were selected in the above steps,
					# now pick the nonvg disks.
					if [ "$TOTAL_DISK_SIZE" -lt "$TOTAL_LV_SIZE_MB"  ]
					then
						# Since we already picked rootvg disks and did not get
						# enough space, select from $TARGETVGS the nonvg disks
						# and ensure that the bootable ones are first.

						# If ONE_BOOTABLE is set, then there was a previous rvg
						# and we don't need to worry about putting the bootable
						# nonrvg disks first, since we already have a bootable
						# disk selected.  If it is not set, then reorder the
						# list of nonrvg disks to make the bootable ones first.
						if [ "$ONE_BOOTABLE" ]
						then
							grep "^$NOVGID" $TARGETVGS >/tmp/available
						else
							# NOTE:  The following 'grep piped to while' will
							# produce different results if run in the Bourne
							# shell.
							grep "^$NOVGID" $TARGETVGS | \
								while read a b c d e f BOOTABLE
								do
									if [ "$BOOTABLE" = y ]
									then
										ONE_BOOTABLE=TRUE
										echo "$a $b $c $d $e $f $BOOTABLE"
									fi
								done > /tmp/available

					# If there was at least one bootable disk with no vgid,
					# put the non-bootable ones in the available list and start
					# selecting defaults.
						# Now put any non-bootable disks with no vg into the
						# available list.
							if [ "$ONE_BOOTABLE" ]
							then
								grep "^$NOVGID" $TARGETVGS | \
									while read a b c d e f BOOTABLE
									do
										if [ "$BOOTABLE" = n ]
										then
											echo "$a $b $c $d $e $f $BOOTABLE"
										fi
									done >> /tmp/available
							fi
						fi

						# /tmp/available may be null if ONE_BOOTABLE is not set
						# and there was not one found.  In that case, the
						# following while will not execute.
						cat /tmp/available | \
						while read vgid rvg lvl DISK LOC SIZE b
						do
							(( TOTAL_DISK_SIZE = TOTAL_DISK_SIZE + SIZE ))
							echo "$LOC" "$SIZE" "$DISK" >> /tmp/tdd.add

							# Take out used disks from tmpvgs.file.
							grep -v " $LOC " /tmp/tmpvgs.file >/tmp/foo
							mv /tmp/foo /tmp/tmpvgs.file

							# If enough disks have been selected, get out of
							# loop.
							if [ "$TOTAL_DISK_SIZE" -ge "$TOTAL_LV_SIZE_MB" ]
							then
								break
							fi
						done

					fi

					# If STILL not enough disks were selected in the above
					# steps, pick the nonrvg disks which are in volume groups.
					if [ "$TOTAL_DISK_SIZE" -lt "$TOTAL_LV_SIZE_MB"  ]
					then
						if [ "$ONE_BOOTABLE" ]
						then
							cp /tmp/tmpvgs.file /tmp/available
						else
							# NOTE:  The following 'grep piped to while' will
							# produce different results if run in the Bourne
							# shell.
							cat /tmp/tmpvgs.file | \
								while read a b c d e f BOOTABLE
								do
									if [ "$BOOTABLE" = y ]
									then
										ONE_BOOTABLE=TRUE
										echo "$a $b $c $d $e $f $BOOTABLE"
									fi
								done > /tmp/available

						# Now put any non-bootable disks into the
						# available list.
						# NOTE:  The following 'grep piped to while' will
						# produce different results if run in the Bourne shell.
							if [ "$ONE_BOOTABLE" ]
							then
								cat /tmp/tmpvgs.file | \
									while read a b c d e f BOOTABLE
									do
										if [ "$BOOTABLE" = n ]
										then
											echo "$a $b $c $d $e $f $BOOTABLE"
										fi
									done >> /tmp/available
							fi
						fi

						cat /tmp/available | \
						while read VGID rvg lvl DISK LOC SIZE b
						do
							(( TOTAL_DISK_SIZE = TOTAL_DISK_SIZE + SIZE ))
							echo "$LOC $SIZE $DISK" >> /tmp/tdd.add

							# If enough disks have been selected, get out of
							# loop.
							if [ "$TOTAL_DISK_SIZE" -ge "$TOTAL_LV_SIZE_MB" ]
							then
								break
							fi
						done
					fi
				fi

				rm -f /tmp/tmpvgs.file /tmp/available

				# Add new target_disk_data stanzas.
				if [ -s /tmp/tdd.add ]
				then
					# Remove existing target_disk_data stanzas.
					bidata -D

					cat /tmp/tdd.add | while read LOC SIZE DISK
						do
							bidata -a -l "$LOC" -s "$SIZE" -n "$DISK"
						done
					rm /tmp/tdd.add
				fi

			fi # 115
		else # 109
			# then there is not enough disk space to contain the rootvg.
# 117
			# If ERRORTEXT is not already set, set it now.  This is so that
			# the first error that occurs will be displayed.  We don't just
			# return, because as much processing should be done on the
			# image.data file as possible.
			if [ "$NOT" "$ERRORTEXT" ]
			then
				# If ERROR_EXIT is set, run it and then flag
				# that user must be prompted.  If they want to
				# stop before the menus, they may do so with
				# the error exit.
				error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
				if [ -n "$error_exit" ]; then "$error_exit"; fi

				ERRORTEXT="The data file did not specify enough disk space to
contain the operating system."
				ERRORNUM=13
# 118
				bidata -b -m control_flow -f PROMPT -e yes
				bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
			fi
		fi

		# Now see if all disks in the source_disk_data stanzas match those
		# currently on the machine.  If they do, make them the defaults.
# 119
		# The following will set ALL_MATCH to non-null if there are values in
		# the source_disk_data stanzas.  If there are no values in the
		# source_disk_data stanzas, ALL_MATCH will be null and when it is
		# tested later, this function will determine that the data in the
		# source_disk_data stanzas do not match the disks on the machine.
# 120
		ALL_MATCH=`bidata -i -g source_disk_data -f LOCATION`
		if [ "$ALL_MATCH" ]
		then
			for LOC in `bidata -i -g source_disk_data -f LOCATION`
			do
				TNAME=`bootinfo -o "$LOC"`
				TSIZE=`bootinfo -s "$TNAME"`

				SNAME=`bidata -i -g source_disk_data -f HDISKNAME -c LOCATION -v "$LOC"`
				SSIZE=`bidata -i -g source_disk_data -f SIZE_MB -c LOCATION -v "$LOC"`

				# If either TNAME and SNAME or TSIZE and SSIZE are not the same,
				# then there is not a disk of the specified location, name, and
				# size on the machine.
# 121
				if [ "$TNAME" != "$SNAME" -o "$TSIZE" != "$SSIZE" ]
				then # 123
# 124
					ALL_MATCH=
# 126
					break
				fi # 122 # 125
			done
		fi

		# If EXISTING_SYSTEM_OVERWRITE is yes, then set ALL_CLEAR to TRUE
		# since it doesn't matter where the rootvg goes.
		if [ "$ESO" = yes ]
		then
			ALL_CLEAR=TRUE
		else
			# See if all source_disk_data disks have no vgid.
			# ALL_CLEAR will be null if there are no entries in source_disk_data
			# and will thus evaluate to false in a test.
			ALL_CLEAR=`bidata -i -g source_disk_data -f LOCATION`
			if [ "$ALL_CLEAR" ]
			then
				for LOC in `bidata -i -g source_disk_data -f LOCATION`
				do
					grep " $LOC " $TARGETVGS | read a VGSTATUS c d e f g
					if [ "$VGSTATUS" -ne 0 ]
					then
						ALL_CLEAR=
						break
					fi
				done
			fi
		fi

		# Since this is a mksysb install, if all disks specified in the
		# source_disk_data stanzas match, and either the image.data file
		# specifies exact fit or the bosinst.data file does not contain
		# entries for target_disk_data stanzas, then put the values of
		# source_disk_data into the target_disk_data stanzas.
# 127
		EXFT=`bidata -i -g logical_volume_policy -f EXACT_FIT`
		if [ "$ALL_MATCH" -a "$ALL_CLEAR" -a \( "$EXFT" = "yes" -o -n "$NULL_TARGET" \) ]
		then # 128
# 130
			# Remove all target_disk_data stanzas from the bosinst.data file.
			# Then use the information in all of the source_disk_data stanzas to
			# create new target_disk_data stanzas.
			for LOC in `bidata -i -g source_disk_data -f LOCATION`
			do
				DNAME=`bidata -i -g source_disk_data -f HDISKNAME -c LOCATION -v $LOC`
				SIZE=`bidata -i -g source_disk_data -f SIZE_MB -c LOCATION -v $LOC`

				echo "$LOC" "$SIZE" "$DNAME" >> /tmp/tdd.add
			done

			# Remove existing target_disk_data stanzas.
			bidata -D

			# Now add the stanza placed in /tmp/tdd.add.
			cat /tmp/tdd.add | while read LOC SIZE DISK
				do
					bidata -a -l "$LOC" -s "$SIZE" -n "$DISK"
				done

			rm /tmp/tdd.add
		fi # 129

# 131
		if [ "$EXFT" = "yes" ]
		then # 132
			if [ "$NOT" "$ALL_MATCH" ]
			then # 137
# 143
				# If ERRORTEXT is not already set, set it now.  This is so that
				# the first error that occurs will be displayed.  We don't just
				# return, because as much processing should be done on the
				# image.data file as possible.
				if [ "$NOT" "$ERRORTEXT" ]
				then
					# If ERROR_EXIT is set, run it and then flag that user must
					# be prompted.  If they want to stop before the menus, they
					# may do so with the error exit.
					error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
					if [ -n "$error_exit" ]; then "$error_exit"; fi

					ERRORTEXT="The data file indicated that all logical volumes should
be created exactly as they were before, but the disks
are not the same as they were on the source system."
					ERRORNUM=14
				fi
# 145
				bidata -i -m logical_volume_policy -f EXACT_FIT -e no
# 146
				bidata -b -m control_flow -f PROMPT -e yes
			fi # 136

			if [ "$NOT" "$ALL_CLEAR" ]
			then # 137
# 143
				# If ERRORTEXT is not already set, set it now.  This is so that
				# the first error that occurs will be displayed.  We don't just
				# return, because as much processing should be done on the
				# image.data file as possible.
				if [ "$NOT" "$ERRORTEXT" ]
				then
					# If ERROR_EXIT is set, run it and then flag that user must
					# be prompted.  If they want to stop before the menus, they
					# may do so with the error exit.
					error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
					if [ -n "$error_exit" ]; then "$error_exit"; fi

					ERRORTEXT="You chose to install only onto disks which are not
contained in a volume group, but there are not enough
of those disks to contain the mksysb image."
					ERRORNUM=11
				fi
# 145
				bidata -i -m logical_volume_policy -f EXACT_FIT -e no
# 146
				bidata -b -m control_flow -f PROMPT -e yes
			fi # 136

		# EXACT_FIT and SHRINK are mutually exclusive.  The following test
		# is to ensure that the two do not conflict.
# 148
			SHR=`bidata -i -g logical_volume_policy -f SHRINK`
			if [ "$SHR" = "yes" ]
			then # 149
# 151
				# If ERRORTEXT is not already set, set it now.  This is so that
				# the first error that occurs will be displayed.  We don't just
				# return, because as much processing should be done on the
				# image.data file as possible.
				if [ "$NOT" "$ERRORTEXT" ]
				then
					# If ERROR_EXIT is set, run it and then flag that user must
					# be prompted.  If they want to stop before the menus, they
					# may do so with the error exit.
					error_exit=`bidata -b -g control_flow -f ERROR_EXIT`
					if [ -n "$error_exit" ]; then "$error_exit"; fi

					ERRORTEXT="The shrink and exact fit operations cannot be
specified together in the same install."
					ERRORNUM=15
				fi
# 98
				bidata -i -m logical_volume_policy -f SHRINK -e no
# 152
				bidata -b -m control_flow -f PROMPT -e yes
				bidata -b -m control_flow -f EXISTING_SYSTEM_OVERWRITE -e yes
			fi # 150
		fi # 133
	fi # 93
	return 0
}

# Screen put up in non-prompted mode which tells the user that they can type
# zero and get screens if they want.
#     Always returns 0.
function Zero_Screen
{
	trap 'BI_Error "BOS Install" 29 2' INT
	PT=`bidata -b -g control_flow -f PROMPT`
	if [ "$PT" = no ]
	then
		echo "$CLEAR\c"
		echo '000\c'
	fi

	Animate '\|/-' &

	echo $! >/tmp/Animate.pn

	return 0
}

################################################################################
# Main
################################################################################

if [ -n "$NIM_DEBUG" ]
then
	set -x
	for i in $(typeset +f)
	do
		typeset -ft $i
	done
fi

if [ ! -s ./bi_main.debug ] ; then
	restbyname -xqf - ./bi_main.debug ./startup >/dev/null 2>&1 </dev/rfd0
	[ -s ./bi_main.debug ] && read a b <./bi_main.debug || a=none
	# At this point, the signature file exists, or the variable "a" = none.
	if [ "$a" = "bi_main.debug" ]
	then
		if [ -s ./startup ] ; then
			./startup
		else
			/usr/lib/methods/cfgcon && exec /usr/bin/ksh
		fi
	fi
fi

# 1
# Set trap on control C, since, in the RAM environment, a control C will leave
# the system in an indeterminate state.
trap 'BI_Error "BOS Install" 29 2' INT

cd /

# Set global variables, copy stripped libs and things where they need to be,
# etc.
Initialize

# tell NIM that we've begun executing
[ "$BOOTTYPE" -eq "$NETWORK" ] && ${NIMCLIENTPATH}nimclient -R success

# If we booted from tape, restore the BOS install programs and data files from
# the second tape mark.  These programs and data files include the bosinst.data
# and the image.data files.
# 2
/usr/lib/methods/showled 0xA40
if [ "$BOOTTYPE" -eq "$TAPE" ]
then # 3
	Restore_Bosinst
else # 4 # 9
# Get bosinst.data and image.data if CDROM or network was booted.  Since these
# files were restored in the Restore_Bosinst step above, this will be bypassed
# if the boot device was a tape.
# Also get the /etc/preserve.list from the CDROM or Network.
# 8
	Get_Data_Files
fi # 10

# Create TARGETVGS file which will contain a list of disks on the machine.
# Put it in the background and then continue
if [ "$BOOTTYPE" -eq "$NETWORK" ]
then
	${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
			-a info="query_disks"
fi
# If the machine only has 8 Meg, run Get_RVG_Disks in the foreground to ensure
# only one thing is vieing for memory at a time.
rm -f /tmp/Get_RVG_Disks.pn
if [ `bootinfo -r` -le 8192 ]
then
	/usr/lpp/bosinst/Get_RVG_Disks
else
	/usr/lpp/bosinst/Get_RVG_Disks &
	echo $! >/tmp/Get_RVG_Disks.pn
fi
#
# Check the INSTALL_TYPE of the default bosinst.data file to determine if it is
# an entry server install type. If it is entry server ESERVER will be 0 otherwise
# ESERVER will be non-zero. The bosinst.data file checked here is either the one on
# the product media, or the one from the backup tape if a restore of a mksysb is being
# performed. -- This check must be done before allowing another bosinst.data file 
# from diskette to replace the default one. 
#
if [ -s "$BOSINSTDATA" ] ; then
	grep -E "^(	| )*INSTALL_TYPE" $BOSINSTDATA | grep "eserver"
	ESERVER="$?"
fi

# If there is a diskette in /dev/fd0 which contains data files, extract them.
# Also, set IMAGE_ON_DISKETTE flag to indicate whether or not the image.data
# file came from the diskette.
# Also, set PRESERVE_ON_DISKETTE flag to indicate whether or not the
# /etc/preserve.list file came from the diskette.
/usr/lib/methods/showled 0xA42
Extract_Diskette_Data

# 23
# If bosinst.data does not exist, create it.
if [ "$NOT" -s "$BOSINSTDATA" ]
then # 24
	Create_Bosinst_Data
fi # 25
# Force off turbo install if image file extracted from diskette
if [ -n "$IMAGE_ON_DISKETTE" ] ; then
    BOS_FORMAT=nonturbo
fi
# Ensure that it image.data file exists prior to calling datadaemon.
Image_Data_Exists

# Start the daemon which manages BOSINSTDATA and IMAGEDATA.
/usr/lpp/bosinst/datadaemon &
# Wait to ensure the datadaemon reads in all the data before bidata waits on
# the pipe.
sleep 1

# If CONSOLE is set in bosinst.data and if PROMPT = no (non-prompt mode),
# update the CuAt with the value of CONSOLE.  Only do this in non-prompt
# mode because if CONSOLE is set to a device that does not have a display
# attached, the user who should be prompted has no way of changing their
# input device.
Set_Console

# If there were errors in Image_Data_Exists, set prompt to yes.
if [ "$ERRORTEXT" ]
then
	bidata -b -m control_flow -f PROMPT -e yes
fi

# 95
# Display screen on non-prompt so user will know they can type 000 to get
# prompt mode.
Zero_Screen

# If there are no values for the target disks specified in the bosinst.data
# file, initialize the bosinst.data file to contain either the previous rvg
# or first disk listed by lsdev.
if [ "$BOOTTYPE" -eq "$NETWORK" ]
then
	${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
			-a info="initialization"
fi
Init_Target_Disks

# If they are still null, initialize all other variables in the "control_flow:"
# stanza which have not yet been initialized.
Other_Initialization

# The following function will ensure that the non-disk related stanzas in
# bosinst.data are valid.
Check_Other_Stanzas

# Verifying entries image.data file.
if [ "$BOOTTYPE" -eq "$NETWORK" ]
then
	${NIMCLIENTPATH}nimclient -o change -a force=yes -a ignore_lock=yes \
		-a info="verifying_data_files"
fi
Verify_Image_Parameters

# Ensure that all fields in the target_disk_data stanzas are filled.
# They may not be if the bosinst.data file had target_disk_data stanzas
# filled in by the user.
Fill_Target_Stanzas

# Gather data from the user if in prompt mode, and validate the data which
# was specified both from the user and from the bosinst.data file.
Get_User_Input

# If migration was selected, Trusted Computing Base requested, or not a 
# graphical install, then turn off turbo mode 
#
tcb="`bidata -b -g control_flow -f TCB`"
IXIA=`bidata -b -g control_flow -f INSTALL_X_IF_ADAPTER`
IM=`bidata -b -g control_flow -f INSTALL_METHOD`
if [ "$BOS_FORMAT" = turbo ] ; then
	if [ "$IM" = migrate -o "$IXIA" = no -o "$tcb" = yes  -o "`bi_io -l </dev/console 2>/dev/null`" -ne 1 -a "$IXIA" != all ]
	then
		# Tell data daemon to write out stanzas and exit.
		bidata -w -x
		# Restore the non-turbo product image.template for bi_data to process
		cp  /SPOT/usr/lpp/bosinst/image.template $IMAGEDATA 2>/dev/null
		BOS_FORMAT=nonturbo
		# Start the daemon which manages BOSINSTDATA and IMAGEDATA.
		/usr/lpp/bosinst/datadaemon &
		# Wait to ensure the datadaemon reads in all the data before
		# bidata waits on the pipe.
		sleep 1
		Set_PP_Size
	fi
fi

# Start Logging
Initialize_Log

if [ "$SETX" -eq 1 ]
then
    set -x
fi

# Create file systems the minimum size necessary if this is a mksysb install
# and the user has selected to do so.
Log Shrink_It

# Create the root volume group.  For overwrite, this means a completely new
# root volume group.  For preservation, this means deleting hd4, hd2, and
# hd9var and recreating them.  For migration, this means removing bos.obj
# from the existing rvg.
Log Prepare_Target_Disks


# Restore bos.rte or mksysb image.
/usr/lib/methods/showled 0xA54
Log Restore_System

# Set up environment so that the files just restored can be used and the disk
# environment can be accessed instead of the RAM environment.
/usr/lib/methods/showled 0xA52
Log Initialize_Disk_Environment

# Unmount hard disk file systems from mount points and mount them over /.
Log Change_Mounts
/usr/lib/methods/showled 0xA46

# Run post install procedures.
Log Post_Install

# Execute customization file, if it exists.
/usr/lib/methods/showled 0xA56
Log Run_Customization
/usr/lib/methods/showled 0xA46

# Display copyright and reboot.
Finish
