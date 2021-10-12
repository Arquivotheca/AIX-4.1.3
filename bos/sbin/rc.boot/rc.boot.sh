# @(#)62  1.58.2.71  src/bos/sbin/rc.boot/rc.boot.sh, bosboot, bos41J, 9520A_all 5/16/95 11:53:05
#
# COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
#
# FUNCTIONS: rc.boot.sh
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

# local defines
ODMSTRNG="attribute=keylock and value=service"

# define local functions

loopled () {
	${SHOWLED} $1	# show a LED code
	while :		# loop forever
	do
		:
	done
# End of loopled function.
}

fd_invoker () {
#
# check for the existence of a diskette in the drive that the system booted
# from and attempt to identify the diskette by reading the signature file.
# If a parameter is passed in, use that as the name of the diskette to look
# for.  Otherwise, read the startup script from the diskette and execute the
# startup script.  When the signature file indicates "last_diskette", stop
# processing and return without restoring any other files.  This function
# loops until one of the following conditions is true:
#	a file named /tmp/last_diskette exists
#	the named diskette ($1, aka $NAME) is processed
#	the "last_diskette" signature is read.
# NOTE: the display and kbd must be available to process a named diskette!
#

${SHOWLED} 0xA07
[ -n "$1" ] && NAME=$1 || unset NAME
init -c "unlink /tmp/last_diskette" >/dev/null

until [ -r /tmp/last_diskette ]
do
	init -c "unlink ./signature" >/dev/null
	init -c "unlink ./startup" >/dev/null
	restbyname -xqf - ./signature >/dev/null 2>&1 <${FD_DEVICE}
	[ "$?" -ne 0 ] && {
		[ -n "${NAME}" ] && {
			# a display device and keyboard MUST be available
			# if ${NAME} is non-null
			echo "Insert ${NAME} Diskette and press Enter" \
					>/dev/console
			read stuff
		}
		continue
	}

	[ -s ./signature ] && read a b <./signature || a=none

	# At this point, the signature file exists, or the variable "a" = none.

	if [ -n "${NAME}" ]		# NAME is defined
	then
		if [ "${NAME}" != "$a" ]	# signature does NOT match
		then
			${SHOWLED} 0xA03
			echo "Wrong diskette, expecting ${NAME} Diskette" \
					>/dev/console
			echo "Insert ${NAME} Diskette and press Enter" \
					>/dev/console
			read stuff
			continue
		else		# the signature matches
			${SHOWLED} 0x513
			echo "Restoring ${NAME} files from diskette." \
					>/dev/console
			restbyname -xqf${FD_DEVICE} ./dsktfiles_list \
					>/dev/null 2>&1
			[ -r ./dsktfiles_list ] && {
				restbyname -qx `cat ./dsktfiles_list` \
					-f${FD_DEVICE} >/dev/null 2>&1
				init -c "unlink ./dsktfiles_list" >/dev/null
			}
			${SHOWLED} 0xA00
			break
		fi
	else		# NAME is undefined

		[ "$a" = 'last_diskette' ] && {
			# the signature on this diskette is "last_diskette", so
			# break from the loop without any further processing
			>/tmp/$a
			break
		}

		[ "$a" != 'none' -a ! -r "/tmp/$a" ] && {
			# signature is not "none" and this diskette has not yet
			# been processed
			${SHOWLED} 0xA09
			restbyname -xqf${FD_DEVICE} ./startup >/dev/null 2>&1
			[ "$?" -eq 0 ] && {
				[ -s ./startup ] && {
					. ./startup
					init -c "unlink ./startup" >/dev/null
				}
				>/tmp/$a
			}
			${SHOWLED} 0xA07
		}
	fi
done
return 0
# End of fd_invoker function.
}

config_network() {
#
# Function: configure network
#

	${NIM_DEBUG}
	ifconfig lo0 inet 127.0.0.1 up
	ifconfig $LDEV inet $CLIENT_IPADDR up $SUBMASK
	[ "$?" -ne 0 ] && return 1

	# set up route tables
	[ "$BOOT_GATE_IP" ] && route -v add $BOOT_SERV_IP $BOOT_GATE_IP
	[ -n "${ROUTES}" ] && {
		for route_args in $ROUTES
		do
			OIFS="$IFS"; IFS=':'
			set -- $route_args
			IFS="$OIFS"
			# verify that there are 3 arguments
			[ $# -ne 3 ] && continue
			route -v add -net $1 -netmask $2 $3 || return 2
		done
	}

	return 0
	# End of config_network function.
}

pdev_to_ldev() {
#
# Function: convert physical device name to logical device name
#
	case "$PHY_BOOT_DEV" in
		ent*)	[ "$E802" -eq 1 ] && \
				LDEV=et${PHY_BOOT_DEV##*([!0-9])} || \
				LDEV=en${PHY_BOOT_DEV##*([!0-9])};;
		fddi*)	LDEV=fi${PHY_BOOT_DEV##*([!0-9])};;
		tok*)	LDEV=tr${PHY_BOOT_DEV##*([!0-9])};;
		*)	# unknown network boot device
			loopled 0x605;;
	esac
}

# End of rc.boot functions.

# Set environment
HOME=/
LIBPATH=/usr/lib:/lib
ODMDIR=/etc/objrepos
PATH=/usr/sbin:/etc:/usr/bin
SHOWLED=/usr/lib/methods/showled
SYSCFG_PHASE=BOOT
export HOME LIBPATH ODMDIR PATH SHOWLED SYSCFG_PHASE

# Get config boot phase argument
[ "$#" -ne 1 ] && loopled 0xA08
PHASE=$1

# Truncate shared libc to save RAM disk space.
[ "$PHASE" -eq 1 ] && >/usr/lib/libc.a

# Start the boot process for a particular device.
BOOTYPE=`bootinfo -t`
[ "$?" -ne 0 ] && loopled 0xA06

[ -z "$BOOTYPE" ] && BOOTYPE=1

case "$BOOTYPE" in

	1)	# Disk boot

		unset config_network fd_invoker pdev_to_ldev

		case "$PHASE" in

			1)	# Phase 1 boot - disk

			echo "\n\n_______________________________________"\
				"_____________________________________" \
				"\nrc.boot: starting disk boot process" \
				>/tmp/boot_log
			ln /usr/lib/lib* /lib

			chramfs -t	# expand RAMFS
			# Call restore base
			echo "rc.boot: executing \"restbase\"" \
				>>/tmp/boot_log
			restbase
			rc=$?
			[ $rc -eq 1 ] && ${SHOWLED} 0x548 #fatal error
			[ $rc -eq 2 ] && >/no_sbase	#non-fatal error

			${SHOWLED} 0x510
			# Call config manager phase 1
			echo "rc.boot: executing \"cfgmgr -f -v\"" \
				>>/tmp/boot_log
			cfgmgr -f -v
			dvc=`bootinfo -b`
			echo "rc.boot: boot device is $dvc" \
				>>/tmp/boot_log
			ln /dev/r$dvc /dev/ipldevice
			${SHOWLED} 0x511
			exit 0
			;;


			2)	# Phase 2 boot - disk

			${SHOWLED} 0x551
			# Bring up the root volume group
			echo "rc.boot: executing \"ipl_varyon -v\"" \
				>>/tmp/boot_log
			ipl_varyon -v
			rc=$?
			case $rc in
				0 )     ;;  # do nothing
				7 | 8 ) loopled 0x554;;
				4 | 9 ) loopled 0x556;;
				* )     loopled 0x552;;
			esac

			ln /usr/bin/sh /usr/bin/bsh
			ln /usr/sbin/mount /etc/umount

			${SHOWLED} 0x517
			echo "rc.boot: executing \"fsck -fp /dev/hd4\"" \
				>>/tmp/boot_log
			fsck -fp /dev/hd4
			[ "$?" -ne 0 ] && loopled 0x555

			echo "rc.boot: executing \"mount /dev/hd4 /mnt\"" \
				>>/tmp/boot_log
			mount /dev/hd4 /mnt
			[ "$?" -ne 0 ] && loopled 0x557

			mount /mnt/etc/filesystems /etc/filesystems
			LIBPATH=/lib:/usr/lib
			export LIBPATH
			ln -s /mnt/sbin/comp* /sbin

			# Mount /usr
			echo "rc.boot: executing \"fsck -fp /usr\"" \
				>>/tmp/boot_log
			fsck -fp /usr
			echo "rc.boot: executing \"mount /usr\"" \
				>>/tmp/boot_log
			mount /usr
			[ "$?" -ne 0 ] && loopled 0x518
			echo "The \"date\" command is now available: " \
				"`date`" >>/tmp/boot_log

			umount /etc/filesystems
			# now we have cp from the mounted /usr
			cp /mnt/etc/filesystems /etc/filesystems

			# Mount /var for copycore
			echo "rc.boot: executing \"fsck -fp var\"" \
				>>/tmp/boot_log
			fsck -fp /var
			echo "rc.boot: executing \"mount /var\"" \
				>>/tmp/boot_log
			mount /var
			[ $? -ne 0 ] && loopled 0x518
			# retrieve dump
			echo "rc.boot: executing \"copycore\"" \
				>>/tmp/boot_log
			copycore
			umount /var

			# Do not try to put up dump menu on 8MB system
			[ -f /needcopydump ] && [ `bootinfo -r` -le 8192 ] \
				&& rm -f /needcopydump

			# Start paging if no dump
			[ ! -f /needcopydump ] && swapon /dev/hd6

			# Error Recovery if customized data is zero
			[ -f /no_sbase ] && {
			echo "rc.boot: executing savebase recovery procedures" \
				>>/tmp/boot_log
				X=`ODMDIR=/mnt/etc/objrepos odmshow CuDv |\
					fgrep population`
				count=`echo $X | cut -f2 -d' '`
				[ $count -ne 0 ] && {
					/usr/sbin/savebase -o /mnt/etc/objrepos
					[ $? -ne 0 ] && loopled 0x546
					mount /var	# so that reboot can log
					echo "savebase recovery reboot" \
						>>/tmp/boot_log
					cat /tmp/boot_log | alog -q -t boot
					reboot
				}
			}

			# allow service if there is a dump or
			#  need diag/maintenance
			KEYPOS=`odmget -q"$ODMSTRNG" CuAt`
			if [ -n "$KEYPOS" -o -f /needcopydump ]
			then
			echo "rc.boot: initiating service procedures" \
				>>/tmp/boot_log
				export KEYPOS
				/usr/lib/boot/srvboot
				[ $? -ne 0 ] && {
					if [ `bootinfo -T` != "rspc" ]; then
						# hang at 549 when not rspc.
						# they might want the dump image
						loopled 0x549
					fi
					# else it is an rspc system, and there's
					# no LED for us to indicate that the
					# dump image could not be retrieved.
					# log it and move along
					echo "\nrc.boot: NOTICE: the dump image could not be saved." >>/tmp/boot_log
					echo "Increase the size of the dump copy directory.\n" >>/tmp/boot_log
					/usr/bin/sysdumpdev -z
				}
			fi

			# Recover /dev directory from any maintenance work.
			if [ -d /mnt/dev.org ]
			then
				rm -fr /mnt/dev
				mvdir /mnt/dev.org /mnt/dev
			fi

			# Copy LVM information to the hardfile
			cd /
			find /etc/vg -print | cpio -updmv /mnt

			# Copy special files to the hardfile
			/usr/lib/boot/mergedev

			# Copy ram disk repository customized data to disk
			cp /etc/objrepos/Cu* /mnt/etc/objrepos

			#
			# If this is the first reboot since an update,
			# we need to move the new odm commands
			# and library into the proper place.
			#
			if [ -f /mnt/etc/rc.update ]
			then
				/mnt/etc/rc.update
				rm /mnt/etc/rc.update
			fi

			# Permanent mounts
			# need absolute pathname for umount command because
			# the umount command is hashed to /usr/sbin/umount
			echo "rc.boot: unmounting temporary mounts" \
				>>/tmp/boot_log
			/etc/umount /usr
			umount /dev/hd4
			${SHOWLED} 0x517
			echo "rc.boot: run time mount of / and /usr" \
				>>/tmp/boot_log
			mount -f /
			[ "$?" -ne 0 ] && loopled 0x518

			# reset SHOWLED just in case the next mount fails,
			# then need to look in the RAM fs for the command
			SHOWLED=/../usr/lib/methods/showled
			export SHOWLED
			/../usr/sbin/mount /usr
			[ "$?" -ne 0 ] && loopled 0x518

			LIBPATH=/usr/lib:/lib
			SHOWLED=/usr/lib/methods/showled
			export LIBPATH SHOWLED
			echo "rc.boot: run time mount of /var" \
				>>/../tmp/boot_log
			mount /var
			[ "$?" -ne 0 ] && loopled 0x518

			cat /../tmp/boot_log | alog -q -t boot
			${SHOWLED} 0x553
			exit 0
			# Exit to kernel newroot
			;;


			3)	# Phase 3 inittab finish boot - disk

			echo "rc.boot: run time mount of /tmp" \
				| alog -q -t boot
			if [ -r /etc/filesystems ]
			then
				fsck -fp /tmp
				mount /tmp
			else
				fsck -fp /dev/hd3
				mount -o log=/dev/hd8 /dev/hd3 /tmp
			fi

			# Volume group sync.
			syncvg -v rootvg &

			# fall through to bottom - phase 3 common code
			;;

			*)	exit 0 ;;

		esac
		;;

	2)	# Diskette boot

		unset config_network pdev_to_ldev

		case "$PHASE" in

			1)	# Phase 1 boot - diskette

			${SHOWLED} 0x510
			# Run config manager for phase 1 boot
			cfgmgr -f -v
			${SHOWLED} 0x511

			# Remove methods
			for i in /usr/lib/methods/cfgfda \
				/usr/lib/methods/cfgfdd \
				/usr/lib/drivers/fd
			do
				init -c "unlink $i" >/dev/null
			done

			# Create the shell script version of "cat"
			#
			#	syntax: cat file1 file2 ...
			echo 'for i in $*
do
	OIFS="$IFS"
	IFS="
"
	while read j
	do
		echo "$j"
	done < $i
	IFS="$OIFS"
done' > /usr/bin/cat
			init -c "chmod /usr/bin/cat 0777" >/dev/null

			# Process diskettes
			FD_DEVICE=/dev/`bootinfo -b`
			fd_invoker

			exit 0
			;;


			2)	# Phase 2 boot - diskette

			FD_DEVICE=/dev/`bootinfo -b`

			${SHOWLED} 0xFFF
			init -c "unlink /signature" >/dev/null
			shift $#
			exec /usr/bin/sh
			exit 0
			;;

			*)	exit 0 ;;

		esac
		;;


	3)	# CDROM boot

		unset config_network pdev_to_ldev
		case "$PHASE" in

			1)	# Phase 1 boot - CDROM
			${SHOWLED} 0x510
			# Run config manager first phase
			cfgmgr -f -v

			# create /etc/filesystems
			>/etc/filesystems
			${SHOWLED} 0x517
			mount -v cdrfs -o ro /dev/`bootinfo -b` /SPOT
			[ "$?" -ne 0 ] && loopled 0x518
			${SHOWLED} 0x512
			/SPOT/usr/bin/rm -r /etc/init /usr/bin \
				/usr/lib/drivers/* /usr/lib/methods/* /usr/sbin

			/SPOT/usr/bin/ln -s /SPOT/usr/* /usr
			/SPOT/usr/bin/ln -s /SPOT/usr/lib/boot/ssh /etc/init

			# since /usr/lib already existed before the above
			# ln command, we now need to populate that dir
			ln -s /SPOT/usr/lib/* /usr/lib

			# /usr/lib/drivers, methods, and microcode are
			# directories, NOT links, so we need to populate them
			ln -s /SPOT/usr/lib/drivers/* /usr/lib/drivers
			ln -s /SPOT/usr/lib/methods/* /usr/lib/methods
			ln -s /SPOT/usr/lib/microcode/* /usr/lib/microcode

			ln -s /usr/lib/objrepos/* /etc/objrepos

			# Enable device package name generation
			DEV_PKGNAME=ALL
			export DEV_PKGNAME

			strload -f /dev/null
			# Run config manager for remaining devices
			${SHOWLED} 0x510
			cfgmgr -p3 -v
			${SHOWLED} 0x511

			exit 0
			;;

			2)	# Phase 2 boot - CDROM

			rm -f /sbin/rc.boot

			unset fd_invoker loopled
			shift $#

			${SHOWLED} 0x513
			exec /usr/lpp/bosinst/bi_main
			exit 0
			;;

			*)	exit 0 ;;

		esac
		;;

	4)	# Tape boot

		unset config_network pdev_to_ldev
		case "$PHASE" in

			1)	# Phase 1 boot - tape

			${SHOWLED} 0x510
			strload -f /dev/null
			# Run config manager first phase
			cfgmgr -f -v

			# Enable device package name generation
			DEV_PKGNAME=ALL
			export DEV_PKGNAME

			# Run config manager second phase
			cfgmgr -p3 -v

			${SHOWLED} 0x512
			# Ensure that all tape drives are
			# configured to block size 512.
			lsdev -Cc tape | while read tapename dummy
			do
				/usr/lib/methods/ucfgdevice -l $tapename
				/usr/lib/methods/chggen -l $tapename \
					-a block_size=512
				`lsdev -Cl$tapename -rConfigure` -l $tapename
			done

			/usr/lib/methods/instdbcln

			# Cleanup
			for i in /usr/lib/drivers/*/* /usr/lib/drivers/* \
				/usr/lib/methods/cfglft \
				/usr/lib/methods/instdbcln \
				/usr/lib/methods/startlft \
				/usr/lib/methods/starttty \
				/usr/lib/microcode/* \
				/usr/lib/nls/loc/C.lftkeymap /usr/lpp/fonts/* \
				/usr/sbin/strload
			do
				init -c "unlink $i" >/dev/null
			done

			# Flatten Predefined databases.
			odmget PdAt >/tmp/Pd.objects 2>/dev/null
			odmdelete -o PdAt > /dev/null 2>&1
			odmget PdCn >>/tmp/Pd.objects 2>/dev/null
			odmdelete -o PdCn > /dev/null 2>&1
			odmget PdDv >>/tmp/Pd.objects 2>/dev/null
			odmdelete -o PdDv > /dev/null 2>&1

			odmadd /tmp/Pd.objects
			init -c "unlink /tmp/Pd.objects" >/dev/null

			exit 0
			;;


			2)	# Phase 2 boot - tape

			init -c "unlink /sbin/rc.boot" >/dev/null
			unset fd_invoker loopled
			shift $#
			${SHOWLED} 0x513

			exec /usr/lpp/bosinst/bi_main
			exit 0
			;;

			*)	exit 0 ;;

		esac
		;;


	5)	# Network boot

		unset NIM_DEBUG		# make sure this is null if not used
		# export NIM_DEBUG='set -x'
		unset fd_invoker
		case "$PHASE" in

			1)	# Phase 1 boot - network
			${NIM_DEBUG}
			${SHOWLED} 0x600

			# free ramfs space by truncating libs.a
			>/usr/lib/libs.a

			# Read IPL control block for network addresses
			set -- `bootinfo -c`
			CLIENT_IPADDR=$1
			BOOT_SERV_IP=$2
			BOOT_GATE_IP=$3
			E802=$6
			BOOTFILE=$7
			VEND=$8

			set +x
			# Set subnet mask and gateway from bootp vendor field.
			OIFS=$IFS
			IFS="."
			set -- $VEND
			while [ "$6" ]
			do
				if [ "$2" = 4 ]
				then
					if [ "$1" = 1 ]		# Subnet mask
					then
						SUBMASK=$3.$4.$5.$6
						shift 5
					elif [ "$1" = 3 ]	# Gateway
					then
						BOOT_GATE_IP=$3.$4.$5.$6
						shift 5
					fi
				fi
				shift
			done
			IFS=$OIFS
			${NIM_DEBUG}
			[ -n "$SUBMASK" ] && SUBMASK="netmask $SUBMASK"

			[ "$BOOT_GATE_IP" = 0 -o \
				"$BOOT_GATE_IP" = "$BOOT_SERV_IP" ] &&
							unset BOOT_GATE_IP

			# configure the network boot device and parent devices
			cfgmgr -fv

			# Get boot device
			PHY_BOOT_DEV=`bootinfo -b`

			# call the function to convert physical to logical name
			pdev_to_ldev

			# set up env variable for local name resolution
			export NSORDER=local
			# configure network - NOTE:  we assume that the
			#	"ROUTES" env variable is undefined at this
			#	point.  It may be defined for subsequent calls.
			${SHOWLED} 0x606
			config_network
			[ $? -ne 0 ] && loopled 0x607

			CLIENT_INFO_FILE=${BOOTFILE}.info
			# tftp read client miniroot mount point file
			${SHOWLED} 0x608
			until tftp -g /SPOT/niminfo \
					$BOOT_SERV_IP $CLIENT_INFO_FILE image
			do
				${SHOWLED} 0xfff
				init -c "unlink /SPOT/niminfo"
				${SHOWLED} 0x608
			done

			# dot execute the /SPOT/niminfo file to load variables
			if [ -s "/SPOT/niminfo" ]
			then
				. /SPOT/niminfo
			else
				loopled 0x609
			fi

			# create /etc/hosts for the RAM filesystem
			[ -n "${NIM_HOSTS}" ] && {
				for host_lines in $NIM_HOSTS
				do
					OIFS="$IFS"; IFS=':'
					set -- $host_lines
					IFS="$OIFS"
					echo $* >> /etc/hosts
				done
			}

			# set up route tables
			[ -n "${ROUTES}" ] && {
				for route_args in $ROUTES
				do
					OIFS="$IFS"; IFS=':'
					set -- $route_args
					IFS="$OIFS"
					# verify that there are 3 arguments
					[ $# -ne 3 ] && continue
					route -v add -net $1 -netmask $2 $3 ||
						loopled 0x613
				done
			}

			> /etc/filesystems

			# NFS mount the spot
			${SHOWLED} 0x610
			mount -r $SPOT /SPOT/usr
			[ "$?" -ne 0 ] && loopled 0x611
			${SHOWLED} 0x612
			cp /SPOT/usr/lib/boot/network/$RC_CONFIG /etc
			RC_CONFIG=/etc/$RC_CONFIG
			. $RC_CONFIG
			exit 0
			;;

			2)	# Phase 2 boot - network

			${NIM_DEBUG}
			export NSORDER=local
			PHY_BOOT_DEV=`bootinfo -b`
			. /SPOT/niminfo
			RC_CONFIG=/etc/$RC_CONFIG
			. $RC_CONFIG
			exit 0
			# Exit to kernel newroot
			;;

			3)	# Phase 3 inittab finish boot - network

			# Start the portmapper
			${NIM_DEBUG}
			/usr/sbin/portmap &
			/usr/sbin/rpc.statd &
			/usr/sbin/rpc.lockd -t1 -g0 &

			# fall through to bottom - phase 3 common code
			;;

			*)	exit 0 ;;

		esac
		;;

	*)
		loopled 0xA06
		;;

esac

unset SYSCFG_PHASE

echo "rc.boot: checking free space in /tmp" | alog -q -t boot
# this removes enough files to create 1024 K free space in /tmp
# if that much space is already free, it does nothing
/usr/lib/boot/bootutil -d /tmp -s 1024 | alog -t boot

# Load the streams modules
echo "rc.boot: executing \"strload\"" | alog -t boot
strload

sysdumpdev -q

# Run the configuration manager
echo "rc.boot: executing \"cfgmgr\"" | alog -t boot
/usr/bin/rm /.bootsequence >/dev/null 2>&1
KEYPOS=`odmget -q"$ODMSTRNG" CuAt`
if [ -n "$KEYPOS" ]
then
	cfgmgr -p3 -v 2>&1 | alog -t boot	# key is in service position
	/usr/lib/methods/cfgcon
	# this call to swcons disables controlling tty
	swcons -c
	set +x
else
	cfgmgr -p2 -v 2>&1 | alog -t boot	# key is in normal position
	/usr/lib/methods/cfgcon
	# this call to swcons disables controlling tty
	swcons -c
	set +x
	#
	# If desktop to be started in inittab, then start graphical boot
	#
	RLEVEL=`/usr/bin/cat /etc/.init.state 2>/dev/null`
	DTRUN=`/usr/bin/grep "^dt:" /etc/inittab | /usr/bin/cut -d: -f2`
	if [ "$?" -eq 0 -a -s "/etc/rc.dt" -a -s "/usr/bin/X11/aixconsole" \
		-a "$RLEVEL" != "S" ]
	then
		RUNINFO=`/usr/bin/grep ":initdefault:" /etc/inittab \
			| /usr/bin/cut -d: -f2`
		if [ "$DTRUN" = "$RUNINFO" ] ; then
			>/.bootsequence
			/etc/rc.dt boot | alog -t boot -q
			/usr/bin/sleep 8
		fi
	fi
	#
	# End of Graphical Boot  Startup
	#
fi

sysdumpdev -q

# Save base customize
# (this must be done AFTER system console setup)

echo "\nSaving Base Customize Data to boot disk" | alog -t boot
# savebase for disk booted systems
[ "$BOOTYPE" -eq 1 ] && savebase
# savebase for net boot systems
[ "$BOOTYPE" -eq 5 ] && savebase -d /etc/basecust

# Remove unavailable ttys from inittab
# also remove ttyx if it is serving as the console
/usr/lib/methods/cleantty
sync; sync

echo "Starting the sync daemon" | alog -t boot
nohup /usr/sbin/syncd 60 > /dev/null 2>&1 &

if [ -x /usr/lib/errdemon ]
then
	echo "Starting the error daemon" | alog -t boot
	/usr/lib/errdemon >/tmp/errdemon.$$ 2>&1
	if [ $? -ne 0 ]
	then
		cat /tmp/errdemon.$$ | alog -t boot
		/usr/bin/rm -f /tmp/errdemon.$$
		echo "Starting the errdemon with the system default" \
			"log file, /var/adm/ras/errlog." | alog -t boot
		/usr/lib/errdemon -i /var/adm/ras/errlog
	fi
fi

${SHOWLED} 0xFFF

# If shutdown did not end gracefully
rm -f /etc/nologin

# If locks remain.
rm -f /etc/locks/*

# start the mirrod if it exists
[ -x /usr/sbin/mirrord ] && /usr/sbin/mirrord mir_modem

# Configuration check, if it exists
[ -x /usr/lib/methods/cfgchk ] && /usr/lib/methods/cfgchk | alog -t boot

# if this is not an rspc machine, or one that supports diagnostic stuff
# then do diagnostic stuff

diag_ok=1			# Ok to run diagnostics
if [ `bootinfo -T` = "rspc" ]; then
	# This is only valid on this type of machine
	bootinfo -M > /dev/null
	rc=$?
	if [ $rc -ne 101 -a $rc -ne 103 ]; then
		diag_ok=0	# diagnostics not supported
	fi
fi

if [ $diag_ok = 1 ]; then
	# Setup Service boot and display message for missing devices.
	if [ -n "$KEYPOS" ]; then
		# this call to swcons enables controlling tty
		swcons +c
		DIAGD=/usr/lpp/diagnostics/bin
		[ -s $DIAGD/diagpt ] && \
			$DIAGD/diagpt </dev/console >/dev/console 2>&1
		[ -s $DIAGD/diagipl ] && \
			$DIAGD/diagipl </dev/console >/dev/console 2>&1
		# diagnostics exits by doing a shutdown, so we should
		#	never get back to this point
	else
		DEVICES=`odmget -qchgstatus=3 CuDv`
		if [ -n "$DEVICES" ]
		then
			echo "A device that was previously detected could" \
				"not be found.\nRun \"diag -a\" to update" \
				"the system configuration." | alog -t boot
		fi
	fi
fi

echo "System initialization completed." | alog -t boot

exit 0
