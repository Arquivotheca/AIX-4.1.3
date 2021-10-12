# @(#)18  1.19.1.2  src/bos/usr/lib/boot/network/rc.dd_boot.sh, cmdnim, bos41B, 412_41B_sync 12/7/94 09:47:02
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: make_dataless
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
#
# This script is to be run as a "dot" script from within rc.boot only.
# Function: diskless/dataless configuration

nim_status() {
	# update the "info" attribute on the NIM master
	/usr/sbin/nimclient -o change -a force=yes -a ignore_lock=yes -a info="$*"
}

fatal_error() {
	typeset led=${1}
	shift
	nim_status "${*:-fatal network boot error}"
	loopled ${led}
}

DEFAULT_DISK=hdisk0	# the default physical disk to use for local paging

make_dataless() {
${NIM_DEBUG}

	nim_status "creating a local paging device"
	${SHOWLED} 0x616
	[ -z "${DTLS_VG_DISK}" ] && DTLS_VG_DISK=${DEFAULT_DISK}
	# clear the ipl record on this disk
	mkboot -c -d /dev/${DTLS_VG_DISK}
	VGNAME=$(mkvg -f -d 8 ${DTLS_VG_DISK})
	[ $? != 0 ] && fatal_error 0x617 "mkvg failed on ${DTLS_VG_DISK}"
	varyonvg -n ${VGNAME}
	[ $? != 0 ] && fatal_error 0x617 "varyonvg failed for ${VGNAME}"
	syncvg -v ${VGNAME} &
	syncvg1_pid=$!
	cp /mnt/etc/swapspaces /etc
	[ $? != 0 ] && fatal_error 0x617 "copy of swapspaces failed"
	# if no page space is specified, default to the amount of real memory
	[ -z "${DTLS_PAGING_SIZE}" ] && \
		((DTLS_PAGING_SIZE=$(bootinfo -r)/4096))
	SWAP_DEV=$(mkps -s${DTLS_PAGING_SIZE} -n -a ${VGNAME})
	[ $? != 0 ] && fatal_error 0x617 "mkps failed on ${VGNAME}"
	cp /etc/swapspaces /mnt/etc/swapspaces
	[ $? != 0 ] && fatal_error 0x617 "restore copy of swapspaces failed"

	# initialize primary dump device
	ln -s /mnt/etc/objrepos/SWservAt* /etc/objrepos
	sysdumpdev -Pp /dev/$SWAP_DEV
	[ $? != 0 ] && fatal_error 0x617 "sysdumpdev failed"

	[ -n "${DTLS_LOCAL_FS}" ] && {
	# if this variable is not null, then we need to create /home
	# and/or /tmp.  Remember that the NFS /usr filesystem is mounted
	# over /usr, but that the NFS root filesystem is mounted over /mnt.
	# That is why we must copy /etc/filesystem to the ram filesystem
	# and then back.  This will only work for filesystems that already
	# have the mountpoints in the NFS root filesystem.
		nim_status "creating a local /home and/or /tmp"
		cp /mnt/etc/filesystems /etc
		cp /mnt/sbin/helpers/v3fshelper /sbin/helpers
		for fs in ${DTLS_LOCAL_FS}
		do
			case ${fs} in
				home)	FS_SIZE=8192;;
				tmp)	FS_SIZE=16384;;
				*)	continue;;
			esac
		crfs -v jfs -m /${fs} -g ${VGNAME} -A yes -p rw \
				-a size=${FS_SIZE}
		done
		cp /etc/filesystems /mnt/etc
	}

	# Copy LVM information to the hardfile
	cd /
	find /etc/vg -print | cpio -updmv /mnt
	[ $? != 0 ] && fatal_error 0x617 "failed to copy lvm info to hardfile"
	# Make sure we save base cust info
	[[ -s /etc/basecust ]] && cp /etc/basecust /mnt/etc/basecust

	# inform NIM that this has succeeded
	/usr/sbin/nimclient -R success
}

#
# -----------------------  PHASE ONE
#
${NIM_DEBUG}

case "$PHASE" in
	1)
	# update the Mstate
	ln -s /SPOT/usr/sbin/nimclient /usr/sbin/nimclient
	ln -s /SPOT/usr/bin/chmod /usr/bin/chmod
	ln -s /SPOT/usr/bin/alog /usr/bin/alog
	/usr/sbin/nimclient -S booting

set +x
	/SPOT/usr/bin/rm /lib /sbin/helpers/v3fshelper /usr/bin/tftp
	cp /SPOT/usr/sbin/rmdev /usr/sbin
	cp /SPOT/usr/bin/rm /usr/bin
	/SPOT/usr/bin/mkdir -p /lib/netsvc
	# we removed /lib, which was a symlink to /usr/lib, and replaced it
	# with a real directory.  now we populate that real directory with
	# hard links to the files in /usr/lib.  this enables us to access the
	# libraries in the RAM filesystem when /usr is overmounted.  This
	# is a good thing because we do not want to have multiple copies
	# of the same library loaded into memory during boot!
	ln /usr/lib/* /lib
	ln /usr/lib/netsvc/* /lib/netsvc
${NIM_DEBUG}
	${SHOWLED} 0x610
	mount ${ROOT} /mnt
	[ $? -ne 0 ] && \
		fatal_error 0x611 "unable to NFS mount the root directory at ${ROOT}"
	${SHOWLED} 0x612

	# copy the niminfo file into the root
	cp /SPOT/niminfo /mnt/etc/niminfo

	if [ -s /mnt/etc/basecust ]
	then
		cp /mnt/etc/basecust /etc
		cp /SPOT/usr/lib/boot/restbase /usr/sbin
		cp /SPOT/usr/bin/uncompress /usr/bin
 		SIBLING_DEVS=$(/SPOT/usr/bin/odmget \
			-q "name like ${PHY_BOOT_DEV%%*([0-9])}*" CuDv |
			/SPOT/usr/bin/fgrep name |/SPOT/usr/bin/cut -f2 -d \")
	fi
	ln /usr/sbin/mount /usr/sbin/umount
	ln /usr/sbin/mount /etc/umount
	nim_status "network boot phase 1"
	umount allr

	# unconfigure network services and devices and let cfgmgr do it the
	# right way.  this also resets device information to what it was
	# before the system was shutdown if /etc/basecust is non-null
	ifconfig ${LDEV} down
	ifconfig ${LDEV} detach
	ifconfig lo0 down

	# SIBLING_DEVS is defined only if /mnt/etc/basecust exists.  If basecust
	# does not exist, then we do not need to remove the boot device ODM data
 	for dev in ${SIBLING_DEVS}; do
 		rmdev -dl ${dev}
 	done

	rm -f /usr/sbin/rmdev
	route -f

	if [ -s /etc/basecust ]
	then
		restbase -o /etc/basecust
		rm -f /etc/basecust /usr/bin/restbase /usr/bin/uncompress
	fi

	# run config manager to configure only the network boot devices and
	# their parents.  we are limited to only those devices because the
	# boot RAM filesystem contains a subset of device configuration files
	cfgmgr -f -v

	# check the physical boot device name again, in case the basecust data
	# added an entry that conflicts with what was there before basecust
	PHY_BOOT_DEV=`bootinfo -b`
	pdev_to_ldev

	${SHOWLED} 0x606
	config_network
	case $? in
		1) fatal_error 0x607 "unable to configure the primary network interface";;
		2) fatal_error 0x613 "unable to add routes";;
	esac
	rm -fr /usr/lib/drivers /usr/lib/methods/!(showled) /usr/lib/microcode \
		/usr/sbin/cfgmgr /usr/sbin/ifconfig /usr/sbin/route
${NIM_DEBUG}

	LIBPATH=/lib; export LIBPATH

	${SHOWLED} 0x610
	mount -r ${SPOT} /usr
	[ $? -ne 0 ] && \
		fatal_error 0x611 "unable to NFS mount the SPOT at location ${SPOT}"
	strload -f /dev/null
	# run cfgmgr to configure remaining devices now that we have the
	# device support in /usr from the remotely mounted filesystem
	cfgmgr -f -v

	mount ${ROOT} /mnt
	[ $? -ne 0 ] && \
		fatal_error 0x611 "unable to NFS mount root directory at location ${ROOT}"

	# configure paging - local or NFS network

	# the grep searches for lines that do not have an asterisk as the
	# first character and contain "/dev".  the sed deletes the lines that
	# contain "swapnfs" and strips away all characters preceding the "/dev"
	local_swap=$(grep "^[^*].*/dev" /mnt/etc/swapspaces | \
		sed -e "/.*\/dev\/swapnfs.*/d; s/.*\(\/dev.*\)/\1/")

	# Start local paging
	if [ -n "$local_swap" ]
	then
		nim_status "configuring local paging device"
		for paging in $local_swap
		do
			${SHOWLED} 0x614
			# get the volume group name
			name=${paging##*/}
			vgname=$(odmget -qname=$name CuDv | \
				sed -n "s/.*parent *= *\"\(.*\)\"/\1/gp")
			if [ -z "$vgname" ]
			then
				${SHOWLED} 0x615
				continue
			fi

			# make the /dev entries which are found
			# in the client's root
			dev_vg=$(ls -l /mnt/dev/$vgname 2>/dev/null)
			if [ -n "$dev_vg" ]
			then
				if [ ! -c /dev/$vgname ]
				then
				# volume group not present yet
				# activate it

					# major number is field 5
					# minor is field 6
					set -- $dev_vg

					# remove the "," after
					# the major number
					major=$(echo $5 | sed -e "s/,//g")

					# make the /dev entry
					mknod /dev/$vgname c $major $6

					# varyon the volume group
					varyonvg -fn $vgname || ${SHOWLED} 0x615
					syncvg -v $vgname &
					syncvg2_pid=$!
				fi

				# make the paging device entry
				dev_ps=$(ls -l /mnt$paging)
				if [ -n "$dev_ps" ]
				then
					# major number is field 5
					# minor is field 6
					set -- $dev_ps

					# remove the "," after
					# the major number
					major=$(echo $5 | sed -e "s/,//g")

					# make the /dev entry
					mknod $paging b $major $6

					# check for local dump image
					# if present, upload it to the server
					BS=8192
					dump_info=/tmp/dump.info
					dump_mnt=/tmp/dump
					dump_file=${dump_mnt}/dump

					# is there a dump image to upload?
					sysdumpdev -z >${dump_info} 2>&1
					if [[ -n "${DUMP}" ]] && [[ -s ${dump_info} ]]
					then
						# dump info in this format:
						#		<total num bytes> <dump device>
						set -- $( cat ${dump_info} )
						num_bytes=${1}
						dump_device=${2}

						# how many blocks?
						(( num_blocks=num_bytes/BS ))
						(( (num_bytes%BS) > 0 )) && (( num_blocks=num_blocks+1 ))

						# mount the remote dump directory
						mkdir ${dump_mnt}
						if mount ${DUMP} ${dump_mnt}
						then
							# upload the dump image
							dd if=${dump_device} of=${dump_file} bs=${BS} \
								count=${num_blocks}

							unmount ${dump_mnt}
						fi
					fi

					# swapon the paging device
					swapon $paging || ${SHOWLED} 0x615
				else
					${SHOWLED} 0x615
				fi
			else
				${SHOWLED} 0x615
			fi

		done

		# Copy LVM information to the hardfile
		cd /
		find /etc/vg -print | cpio -updmv /mnt

	fi # local_swap

	# if the dataless conversion flag is set, call a function to do it
	[ -n "${NIM_MK_DATALESS}" ] && make_dataless

	remote_swap=$(grep "^[^*]remdev" /mnt/etc/swapspaces | \
		sed -e "s/.*= *\(.*\)/\1/")

	# Start NFS remote paging
	# configure the remote paging devices
	DEFAULT_IFS=$IFS
	for i in $remote_swap
	do
		nim_status "configuring remote paging device"
		${SHOWLED} 0x618
		IFS=':'
		set -- $i
		IFS=$DEFAULT_IFS
		SWAPHOST=$1
		SWAPFILE=$2
		SWAPDEV=${SWAPFILE##*/}

		if [ -z "$(odmget -qname=$SWAPDEV CuDv)" ]
		then
		# device doesn't exist - create it
			cat <<- EOF > /tmp/swapnfs
			CuDv:
			name = $SWAPDEV
			status = 0
			chgstatus = 1
			PdDvLn = swap/nfs/paging
			EOF

			odmadd /tmp/swapnfs

			# change the paging attributes
			/usr/lib/methods/chggen -l $SWAPDEV -a \
			"swapfilename=$SWAPFILE hostname=$SWAPHOST" || \
			${SHOWLED} 0x619
		fi

		/usr/lib/methods/cfgswpnfs -l $SWAPDEV || ${SHOWLED} 0x619
		swapon /dev/$SWAPDEV && RC=0 || ${SHOWLED} 0x619
	done # remote_swap

	${SHOWLED} 0x620

	# Copy special files to the hardfile
	/usr/lib/boot/mergedev

	# Copy ram disk repository customized data to disk
	cp /etc/objrepos/Cu* /mnt/etc/objrepos

	# copy the client's /etc/filesystems to the RAM filesystem
	cp /mnt/etc/filesystems /mnt/etc/hosts /etc

	# this prevents crashes if no paging was started.  Before paging
	# has been activated, every exec unloads libraries from memory.
	# After paging has been started, the libraries in memory are
	# not unloaded.  Therefore, in the event that paging does not
	# start, this copy of the minimum libs.a is needed for the shell.
	# NOTE: this cp fails if paging has been enabled, but that's ok.
	# it works if paging didn't start, and enables the boot to continue
	cp /usr/lib/libs.a.min /lib/libs.a

	# unmount all of the remote filesystems
	nim_status "network boot phase 2"

	# ensure we can access usr/lib after we mount the root.
	/usr/bin/ln -fs /../usr/lib /mnt/usr/lib

	wait $syncvg1_pid $syncvg2_pid

	/etc/umount -f allr
	;;

#
# -----------------------  PHASE TWO
#
	2)

	${SHOWLED} 0x610
	# change SHOWLED just in case the mount of / succeeds and the mount
	# of /usr fails because /usr will be empty
	SHOWLED=/../usr/lib/methods/showled; export SHOWLED
	mount -f -t dd_boot
	[ $? -ne 0 ] && fatal_error 0x611 "unable to mount -f -t dd_boot"

	slibclean	# clean up unused shared libraries
	SHOWLED=/usr/lib/methods/showled; export SHOWLED

	# set the client's hostname.  this makes NFS services happy.
	hostname ${NIM_HOSTNAME}

	${SHOWLED} 0x553

	nim_status "network boot phase 3"
	;;
esac
