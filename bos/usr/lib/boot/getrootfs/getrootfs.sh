#!/bin/ksh
# @(#)10 1.10.1.12 src/bos/usr/lib/boot/getrootfs/getrootfs.sh, bosboot, bos411, 9436B411a 9/2/94 17:10:37
#
# COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Prologue: Shell script to varyon installed rootvg and
#		mount root file systems for maintenance work only.
#
################################################################

# The array "vg_info" contains two types of data.  Even numbered
# elements contain the VGID, and odd numbered elements contain the
# number of physical disks associated with the VG specified by the
# previous array element.

#####################################
create_vg_db() {
# logic: for each physical disk that is in an available state, get the
# volume group identifier for that disk and search the database for that
# vgid.  if not found, add it to the database.  then increment the disk
# count for that volume group, and write disk information to a file for
# each volume group.
# data structures: two array elements in vg_info[] and a file in /tmp for
# each volume group found.  The VGID and number of disks are stored in the
# array, and disk name, size, and location code are stored in the file,
# which is named by the VGID.

# read stdin from a file
exec </tmp/phy_disk.list
while read disk_name state location stuff
do
	[ "$state" != "Available" ] && continue
	vgid=`lqueryvg -p $disk_name -v 2>/dev/null`
	[ $? -ne 0 ] && continue
	typeset -i count=0

	while [ -n "${vg_info[count]}" ]
	do
		# search through the array for the current vgid
		[ "${vg_info[count]}" = ${vgid} ] && break
		((count += 2))
	done

	[ -z "${vg_info[count]}" ] && {	# this is a new entry
		vg_info[count]=${vgid}
		vg_info[count + 1]=0
	}

	# next increment the number of disks associated with this vg
	((vg_info[count + 1] += 1))

	typeset -R16 string1=${disk_name}
	typeset -R6 string2=`bootinfo -s ${disk_name}`
	typeset -R13 string3=${location}
	if ((vg_info[count + 1] % 2))
	then		# the number of disks for this VG is an ODD number
		print -n "${string1}${string2}${string3}" \
			>> /tmp/vg_info_${vg_info[count]}
	else		# the number of disks for this VG is an EVEN number
		print -- "  ${string1}${string2}${string3}" \
			>> /tmp/vg_info_${vg_info[count]}
	fi

done
exec </dev/console
}	# end of create_vg_db()

#####################################
format_vg_files() {
# this function adds a newline character to the vg_info files that describe
# volume groups that have an odd number of physical disks
typeset -i count=0
while [ -n "${vg_info[count]}" ]
do
	if ((vg_info[count + 1] % 2))
	then	# the number of disks for this VG is an ODD number
		print >> /tmp/vg_info_${vg_info[count]}
	fi
	((count += 2))
done
}	# end of format_vg_files()

#####################################
disks_to_lines() {
# In order to reduce the amount of arithmetic needed to create the menus,
# we step through the array and change the element that contains the number
# of physical disks to contain the number of menu lines required.
typeset -i count=1	# modify only the odd numbered elements
while [ -n "${vg_info[count]}" ]
do
	# add 1 for the line that displays the VGID, then add half the
	# number of disks (two per line), and finally, add 1 if the
	# number of disks is odd, e.g. 5 lines to display 9 disks
	# 6 lines to display the VGID and 9 disks
	((vg_info[count] = 1 + vg_info[count]/2 + vg_info[count]%2))
	((count += 2))	# remember, ODD elements only
done
}	# end of disks_to_lines()

#####################################
create_menus() {
# this function uses the "vg_info" array and the "vg_info" files in /tmp
# to create menu pages.  The menu pages are stored as files in /tmp, and
# are named according to order of creation.  The array "choice_array"
# contains data representing each and every possible menu selection
typeset -ir max_lines=16	# max number of lines per screen
typeset -i lines_free=0		# number of items that will fit on screen
typeset -i page_num=1		# current page number being processed
typeset -i choice_num=1		# the number for each menu selection
typeset -i first_choice=1	# a flag indicating that the first item is ready

screen_file="/tmp/menu_page${page_num}"
lines_free=${max_lines}
while :
do
	counter=1
	current=${counter}

	if [ "${first_choice}" -eq 1 ]
	then	# we are processing the first item for the current screen
		# if it is not the first screen, we need to allow for
		#   the previous menu item
		((page_num > 1)) && ((lines_free -= 1))
		# now determine if there will be a "next" screen
		typeset -i lines_needed=0
		while [ -n "${vg_info[counter]}" ]
		# this loop adds the remaining lines that need to be displayed
		do
			((lines_needed += vg_info[counter]))
			((counter += 2))
		done
		((lines_needed > lines_free)) &&
			# decrement lines_free to allow for "next" menu item
			((lines_free -= 1)) || lines_needed=0

		counter=1
		while [ -n "${vg_info[counter]}" ]
		# this loop searches for largest remaining line count
		do
			((vg_info[counter] > vg_info[current])) &&
					current=${counter}
			((counter += 2))
		done
		# now current is the index of the largest line count
		#   for the volume groups

	else	# we are processing subsequent items for the current screen
		while [ -n "${vg_info[counter]}" ]
		# this loop searches for largest remaining line count that will
		#   fit on the screen
		do
			((vg_info[counter] <= lines_free &&
				vg_info[counter] > vg_info[current])) &&
					current=${counter}
			((counter += 2))
		done

		# check to see if it fits in case there was no
		#   match in previous loop
		((vg_info[current] > lines_free || lines_free <= 1)) && {
			# cannot fit anything else on this screen
			((lines_needed > 0)) && {
				print -- "   ${choice_num})   next page --->" \
						>> ${screen_file}
				choice_array[choice_num]=next
				((choice_num += 1))
			}
			((menu_max[page_num]=choice_num - 1))
			((page_num += 1))
			screen_file="/tmp/menu_page${page_num}"
			lines_free=${max_lines}
			first_choice=1		# set the flag
			continue	# go to loop top and start new screen
		}
	fi

	((lines_free -= vg_info[current]))

	# if the disk count is 0 at this point, all vg's have been processed
	((vg_info[current] == 0)) && {
		((menu_max[page_num]=choice_num - 1))
		break
	}

	((first_choice == 1)) && {
		menu_min[page_num]=${choice_num}
		((page_num > 1)) && {
			print -- "   ${choice_num})   <--- previous page" \
					>> ${screen_file}
			choice_array[choice_num]=previous
			((choice_num += 1))
		}
		first_choice=0		# unset the flag
	}

	print -- "   ${choice_num})   Volume Group ${vg_info[current-1]}" \
				"contains these disks:" >> ${screen_file}
	cat /tmp/vg_info_${vg_info[current-1]} >> ${screen_file}
	cat /tmp/vg_info_${vg_info[current-1]} |
			read choice_array[choice_num] other_stuff
	vg_id[choice_num]=${vg_info[current-1]}

	((choice_num += 1))

	vg_info[current]=0	# set it to 0 to indicate it has been processed
done	# go to loop top and display more items on the same screen
}	# end of create_menus()

#####################################
display_log_vol() {
# this function takes as input a physical disk name and generates a list
# of the logical volumes that are contained on the volume group of which
# the physical disk is a member
divider='---------------------------------------'
divider=' '${divider}${divider}
print -- "\n\n\n\n                           Volume Group Information\n"
print -- "${divider}"
print -- "    Volume Group ID ${vg_id[ans]} includes the following" \
					"logical volumes:\n"
typeset -i lv_count=0
typeset -i line_count=0
lqueryvg -L -p $1 |
while read lvid lv_name other_stuff
do
	((lv_count += 1))
	typeset -R12 lv_string=${lv_name}
	print -n "${lv_string}"
	((lv_count == 6)) && {
		print
		lv_count=0
		((line_count += 1))
	}
done
print -- "\n${divider}"
while ((line_count < 9))
do
	print
	((line_count += 1))
done
print -- "Type the number of your choice and press Enter.\n"
print -- "   1) Access this Volume Group and start a shell"
print -- "   2) Access this Volume Group and start a shell before" \
			"mounting filesystems\n"
print -- "  99) Previous Menu\n"
print -n "    Choice [99]: "
read ans
case ${ans} in
	1 | 2 )	retcode=$ans;;
	* )	retcode=99;;
esac
return $retcode
}	# end of display_log_vol()

#####################################
display_menus() {
screen_file="/tmp/menu_page"
typeset -i page_num=1		# current page number being processed
	while :
	do
		print -- "\n\n\n\n                          " \
				"Access a Root Volume Group\n"
		print -- "Type the number for a volume group to display" \
			"the logical volume information\nand press Enter.\n"
		cat ${screen_file}${page_num}
		wc -l ${screen_file}${page_num} | read lines_used other_stuff
		while ((lines_used < 18))
		do
			print
			((lines_used += 1))
		done
		print -n "   Choice: "
		read ans
		case ${ans} in
			[1-9] | [1-9][0-9] )	;;
			* )	continue;;
		esac
		(( ans < menu_min[page_num] || ans > menu_max[page_num])) &&
			continue
		if [[ "${choice_array[ans]}" = "previous" ]]
		then
			((page_num -= 1))
		elif [[ "${choice_array[ans]}" = "next" ]]
		then
			((page_num += 1))
		else
			target_disk=${choice_array[ans]}
			display_log_vol ${choice_array[ans]}
			retcode=$?
			(( retcode == 1 || retcode == 2 )) && return $retcode
		fi
	done
}	# end of display_menus()

#####################################
error_shell() {
	print -- $1
	print -- Exiting to shell.
	exec /usr/bin/ksh
}

# end of function declarations
#####################################

# set trap to ignore stuff
trap "" 1 2 15

PATH=/usr/sbin:/etc:/usr/bin
LIBPATH=/usr/lib:/lib
ODMDIR=/etc/objrepos
export LIBPATH PATH ODMDIR

# make a list of physical disks
lsdev -Cc disk > /tmp/phy_disk.list

create_vg_db

rm -f /tmp/phy_disk.list

# Cannot continue if there are no volume groups at all
[ -z "${vg_info[0]}" ] && \
	error_shell "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nNo volume groups were found on any configured disks."

format_vg_files

disks_to_lines

create_menus

rm -f /tmp/vg_info*

display_menus
retcode=$?
[[ "${retcode}" = "1" ]] && SHELL_BEFORE_MOUNT=false
[[ "${retcode}" = "2" ]] && SHELL_BEFORE_MOUNT=true

rm -f /tmp/menu_page*

# determine what type of boot device was used
BOOTDEV_TYPE=$(bootinfo -t)

echo Importing Volume Group...

importvg -f -y rootvg ${target_disk}
[ "$?" -eq 1 ] && error_shell "The \"importvg\" command did not succeed."

# get the name of the boot logical volume
odmget -q "attribute=type and value=boot" CuAt | fgrep name |
			cut -f2 -d \" | read bootdev

# get the serial number of the boot logical volume
boot_lvid=`getlvodm -l $bootdev`

# get the physical volume identifier for the boot disk
lquerylv -L $boot_lvid -r |cut -f1 -d ' ' |read pvid

# get the name of the disk that contains the boot logical volume
bootdisk=`odmget -q "attribute=pvid and value like ${pvid}*" CuAt |
					fgrep name|cut -f2 -d \"`

# Link boot disk to /dev/ipldevice
ln /dev/${bootdisk} /dev/ipldevice >/dev/null 2>&1

# get the name of a paging device
odmget -q "attribute=type and value=paging" CuAt | fgrep name |
			cut -f2 -d \" | read pagedev

# activate paging
[ -n "$pagedev" ] && swapon /dev/$pagedev

cp /usr/sbin/mount /etc
ln /etc/mount /etc/umount

rm /lib
mkdir /lib
[ "${BOOTDEV_TYPE}" -eq 5 ] &&		# booted from network?
		cp /SPOT/usr/ccs/lib/libc.a.min /lib/libc.a ||	# yes
		ln /usr/lib/lib* /lib >/dev/null 2>&1	# no

# get the filesystem logical volume names
rootlv=`odmget -q "attribute=label and value=/" CuAt |
					fgrep name |cut -f2 -d \"`
usrlv=`odmget -q "attribute=label and value=/usr" CuAt |
					fgrep name |cut -f2 -d \"`

[ -z "$rootlv" -o -z "$usrlv" ] &&
		error_shell "Could not find \"/\" and/or \"/usr\" filesystems."

echo "Checking the / filesystem."
fsck -fp /dev/$rootlv
echo "Checking the /usr filesystem."
fsck -fp /dev/$usrlv

[[ "${SHELL_BEFORE_MOUNT}" = "true" ]] && {
	print -- "Exit from this shell to continue the process of accessing" \
		"the root\nvolume group."
	/usr/bin/ksh
}

mount -f /dev/$rootlv /mnt

# create symlinks for jfs compression
ln -s /mnt/sbin/comp* /sbin >/dev/null 2>&1

# if cdrom or tape boot, then mount /usr to access the commands.
# Otherwise, access the commands in the NFS filesystem
[ "${BOOTDEV_TYPE}" -ne 5 ] && mount /dev/$usrlv /usr
# if cdrom or tape boot, then we are now using commands and libraries
# from the hard disk based filesystem

# Save logical volume manager data.
find /etc/vg -print | cpio -updmv /mnt >/dev/null 2>&1

# Save normal boot devices.  The rc.boot script will recover the
# original /dev during the next normal (disk) boot.

echo "Saving special files and device configuration information."
rm -fr /mnt/dev.org >/dev/null 2>&1
mvdir /mnt/dev /mnt/dev.org
mkdir /mnt/dev

# Copy RAM devices to regular file system
/usr/lib/boot/mergedev >/dev/null 2>&1

# leave a flag for the bosboot command
echo "this is used by the bosboot command" > /BOOTFS_COOKIE

if [ "${BOOTDEV_TYPE}" -ne 5 ]		# booted from cdrom or tape?
then
	LIBPATH=/lib /etc/umount /dev/$usrlv
fi
umount /dev/$rootlv

echo "Mounting the / filesystem."
mount -f /dev/$rootlv /
echo "Mounting the /usr filesystem."
if [ "${BOOTDEV_TYPE}" -eq 4 ]		# booted from tape?
then
	LIBPATH=/../lib /../etc/mount /dev/$usrlv /usr
else
	LIBPATH=/../lib /../SPOT/usr/sbin/mount /dev/$usrlv /usr
fi
echo "Checking and mounting the /tmp filesystem."
fsck -fp /tmp
mount /tmp
echo "Checking and mounting the /var filesystem."
fsck -fp /var
mount /var

# write buffers out to disk since syncd is not running
sync;sync;sync

echo "Files systems mounted for maintenance work."

# put this shell in a loop to prevent hitting simple shell if they exit
while :
do
	/usr/bin/ksh
done
