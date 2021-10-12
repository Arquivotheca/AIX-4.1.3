# @(#) 35 1.4 src/bos/usr/lpp/bosinst/Get_RVG_Disks/Get_RVG_Disks.sh, bosinst, bos411, 9440E411c 10/11/94 15:38:51
#
#   COMPONENT_NAME: BOSINST
#
#   FUNCTIONS: Blv_Disk_Test
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

# Blv_Disk_Test
#	Return 0 if the named disk contains the blv.
#	Return 1 if the named disk does not contain the blv
#
function Blv_Disk_Test
{
	lvid=$1
	name=$2
	bootable=$3
	#
	# Else check the ODM to see if the
	# boot lv is on disk $name
	#
	pvid=`lquerylv -L "$lvid" -p$name -d 2>/dev/null | cut -d: -f1`
	# The above will return multiple lines of the same pvid.
	# The below will cut all but the first and place the result in pvid.
	pvid=`echo $pvid | cut -d' ' -f1`
	bootdiskname=`getlvodm -g $pvid 2>/dev/null`
	[ "$bootdiskname" = "$name" ] && return 0 || return 1
}

#
# Get_RVG_Disks:	Create a data base of disks.
#       Disks are one per line.  The file created is $TARGETVGS.
#
#		The format of this file is as follows:
#
#		VGID RVG LEVEL HDISKNAME LOCATION SIZE [y|n]
#
#		where:
#		- VGID is either a valid VGID for which the listed disks
#		are members or a string of 16 zeros if the listed disks
#		are not members of any volume group.
#
#		- RVG = {0 | 1 | 2}, 0=disk is not part of a vg,
#                            1=disk is part of a rootvg,
#                            2=disk is part of a non-rootvg vg
#
#       - LEVEL is the level of AIX which is installed if the disk
#         is part of an RVG.  If it is not, this field will contain "0.0".
#
#       - HDISKNAME is the name of the disk
#
#		- LOCATION is the location code of the disk
#
#		- SIZE is the size of the disk
#
#		- [y|n] a bootable disk flag
#		y if the disk is bootable (however this is currently defined).
#		n if the disk is not bootable.
#
#		- The following disks will have 0000000000000000 as the vgid:
#			- All disks that do not belong to any VG
#			- All disks that belong to an  RVG but fail one
#			or both of the following:
#				- the requsite set of logical volumes
#				is not located.
#				- all physical disks in the RVG are not
#				available.
#

# 36
	#
	# zero out target file
	#
	>$TARGETVGS
	#
	# start writing/reading to/from the pipe
	#
	lsdev -Cc disk -S Available -F "name location"  | while read  name location rest
	do
		# Get a vgid
		vgid=`lqueryvg -p$name -v 2>/dev/null`
		vgid=${vgid:-"$NOVGID"}

		# Get the number of PVs in this VG
		pvcount=`lqueryvg -p$name -c 2>/dev/null`
		pvcount=${pvcount:-0}

		# Get the size of the disk
		size=`bootinfo -s "$name"`
		size=${size:-0}

		# Get the bootable disk flag
		# bootinfo -B writes a non-zero numeric character to stdio if
		# the disk is bootable
		bootable=`bootinfo -B $name 2>/dev/null`
		if [ "$bootable" -eq 1 ]; then bootable=y; else bootable=n; fi

		# Get the RVG tag defined above
		blv=1		# flag existence of boot LV as false
					#
		bootdisk=1	# bootdisk will be set (in the following
					# loop) to 0 if this is the disk that
					# contains the boot LV
		lvcnt=0
		#
		# For bosinst purposes an RVG is defined as a VG containing
		# the following sets of LVs: (hd2,hd4,hd5,hd8).
		# Futhermore, all PVs in the VG must be present. The LV
		# hd9var is not required because it only came into being
		# in AIX3.2.
		#
		set -- `lqueryvg -p$name -L 2> /dev/null`
		while [ -n "$1" ]
		do
			case "$2"
			in
				hd[248])	lvcnt=`expr ${lvcnt:=0} + 1` ;;

				hd5)		blv=0
						# Is this the PV that actually contains
						# the boot LV ?.
						# bootdiskname will contain the name of the disk
						# containing the boot lv.  It will be set in
						# Blv_Disk_Test.
						unset bootdiskname
						Blv_Disk_Test $1 $name $bootable
						bootdisk=${?:-1} ;;
			esac
			shift 3
		done
		#
		# If there is no vg id on this disk, tag it as no vg.
		# If the above loop found a blv, the boot disk, and all required
		# logical volumes, mark this vg as a rootvg.
		# If neither of those criteria are met, there is a volume group on the
		# set of disks, but it is not an RVG.  Mark it as a non-RVG volume
		# group.
		# 

		# If we found everything in the rvg but the bootdiskname, then
		# consider this to be a non vg.
		if [ "$blv" -eq 0 -a "$lvcnt" -eq 3 -a -z "$bootdiskname" ]
		then
			vgid="$NOVGID"
		fi

		if [ "$vgid" = "$NOVGID" ]
		then
			rvg="$NOVG"
			level=0.0
		elif [ "$blv" -eq 0 -a "$lvcnt" -eq 3 ]
		then
			rvg="$ROOTVG"
			level=`blvset -d /dev/$bootdiskname -g level`
			# Due to the fact that the 3.2 installation sometimes wrote the
			# install defaults and level in the wrong place on the disk, we
			# will conduct this kludge to ensure that the level retreived was
			# really 3.1.  A level of 3.1 means that either it was really 3.1
			# or blvset could not find anything so it defaulted to 3.1.
			if [ "$level" = "3.1" ]
			then
				# If hd9var exists, it is really a 3.2 machine.
				if lqueryvg -p /dev/$bootdiskname -L | grep hd9var >&- 2>&-
				then
					level=3.2
				fi
			fi
		else
			rvg="$OTHERVG"
			level=0.0
		fi
		#
		# Assemble all data collected above into variables
		# to be assembled into a single list after this
		# loop exits.

		# Keep a list of the VG ID and the rvg flag.
		# One list per vgid.
		eval [ -z "\"\$vginfo$vgid\"" ] && \
			eval vginfo$vgid="\"$vgid $rvg $level\""

		# Keep the location, the size, and the bootable flag of each disk.
		# Two separate lists are maintained. One list
		# contains the boot LV the other contains all
		# other PVs per VG.
		if [ "$bootdisk" -eq 0 ]
		then
			eval [ -z "\"\$boot$vgid\"" ] && \
				eval boot$vgid="\"$name $location $size $bootable\""
		else
			eval disks$vgid="\$disks$vgid\" $name $location $size $bootable\""
		fi

		# keep the count of disks in this vg
		eval [ -z "\"\$pvcnt$vgid\"" ] && eval pvcnt$vgid="\"$pvcount\""

		# keep a list of unique vgids for future reference
		echo $list_ofids | grep "$vgid" >/dev/null 2>&1
		rc="$?"
		[ "$rc" -ne 0 ] && list_ofids="$list_ofids $vgid"
	done
	unset vgid pvcount size bootable blv bootdisk lvcnt rvg
	#
	# Process the lists collected above. Create the TARGETVGS file in
	# the format described above
	#
	for i in $list_ofids
	do
		count=0
		eval pvcount=\$pvcnt$i
		eval unset pvcnt$i
		eval set -- \$vginfo$i

		# if the above "while" loop "thought" this list represented an
		# RVG then double check the number of disks
		rvg="$2"
		if [ "$rvg" -eq "$ROOTVG" ]
		then
			# undefine all positional parameters
			shift $#
			# count the number of disks
			eval set -- \$disks$i
			count=`expr $# \/ 4`
			# if there is a boot lv for this rvgid (ie $i)
			# then add anotehr disk to the count
			eval [ -n "\"\$boot$i\"" ] && count=`expr ${count:-0} + 1`
		fi

		# If this is a root volume group list and
		# pvcount != count then we can not classify
		# the current list of disks as belonging
		# to an RVG. Thus add the current disks to
		# the list of disks that do not belong to any VG.
		if [ "$rvg" -eq "$ROOTVG" -a "$count" -ne "$pvcount" ]
		then
			eval disks$NOVGID="\$disks$NOVGID\" \$boot$i \$disks$i\""
		elif [ "$i" != "$NOVGID" ]
		then
			# Else if this is not the $NOVGID list then we can
			# put the list in the target file
			eval set -- \`echo "\$boot$i \$disks$i"\`
			while [ "$#" -gt 0 ]
			do
				eval echo \$vginfo$i $1 $2 $3 $4
				shift 4
			done >> $TARGETVGS
			eval unset disks$i
		fi
		# don't need boot$i or vginfo$i anymore
		eval unset boot$i
		eval unset vginfo$i
	done

		# if the NOVGID list exists then place it in the file of disks
		eval novglist=\"\$disks$NOVGID\"
		if [ -n "$novglist" ]
		then
			eval set -- \`echo "\$disks$NOVGID"\`
			while [ "$#" -gt 0 ]
			do
				echo "$NOVGID $NOVG 0.0 $1 $2 $3 $4"
				shift 4
			done >> "$TARGETVGS"
			eval unset disks$NOVGID
		fi

	unset list_ofids

	[ -s "$TARGETVGS" ] && rc=0 || rc=1
	return $rc
