#!/bin/ksh
# @(#)61	1.1  src/bldenv/pkgtools/makediagboot.sh, pkgtools, bos412, GOLDA411a 5/6/94 10:57:16
# COMPONENT_NAME: PKGTOOLS
#
# FUNCTIONS: Make diagnostic boot media
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------
# Purpose:
# 	Provide a way to build diagnostic boot media.
#-----------------------------------------------------------------
#
# usage() function provides command syntax to the user
usage()
{
	echo "$cmd [-c imgdir] [-d rmt?] [-l image_list] [-V] [-h]"
	echo ""
	echo "    where:"
	echo "          -c Directory name containing the boot image"
	echo "             as well as other images."
	echo "          -d Name of tape device to write to. Default is rmt0."
	echo "             Example: -d rmt5 (do not enter /dev/rmt5)"
	echo "          -l File containing the list of images to copy"
	echo "          -V Displays source version information."
	echo "          -h List help message"
	echo ""
	echo "    Build a bootable diagnostics tape."
	echo "    The boot image file name can be set via the environment"
	echo "    variable DIAGBOOT_IMAGE."
exit 0
}

# Print error message
p_error()
{
	echo "\n$cmd:  ERROR:  An error has been received writing the media."
	echo "\tDo not use the media to boot"
	exit 1
}

# respawn function, called when an interrupt is detected

respawn_function()
{
	RESPAWN_FLAG=0
	cleanup
	exit 0
}

# Cleanup function

cleanup()
{
	rm $WORK >/dev/null 2>&1

}

# Figure out device type

get_type ()
{
	ODMDIR=/etc/objrepos lsdev -C -c tape -l $lname -t "8mm5gb" | \
		grep "$lname" > /dev/null 2>&1
	[ $? -eq 0 ] && { TYPE="8mm5gb"; return 0; }

	ODMDIR=/etc/objrepos lsdev -C -c tape -l $lname -t "8mm" | \
		grep "$lname" > /dev/null 2>&1
	[ $? -eq 0 ] && { TYPE="8mm"; return 0; }

	ODMDIR=/etc/objrepos lsdev -C -c tape -l $lname -t "150mb" | \
		grep "$lname" > /dev/null 2>&1
	[ $? -eq 0 ] && { TYPE="150mb"; return 0; }

	ODMDIR=/etc/objrepos lsdev -C -c tape -l $lname -t "525mb" | \
		grep "$lname" > /dev/null 2>&1
	[ $? -eq 0 ] && { TYPE="525mb"; return 0; }

	ODMDIR=/etc/objrepos lsdev -C -c tape -l $lname -t "4mm2gb" | \
		grep "$lname" > /dev/null 2>&1
	[ $? -eq 0 ] && { TYPE="4mm2gb"; return 0; }

	ODMDIR=/etc/objrepos lsdev -C -c tape -l $lname -t "4mm4gb" | \
		grep "$lname" > /dev/null 2>&1
	[ $? -eq 0 ] && { TYPE="4mm4gb"; return 0; }

	ODMDIR=/etc/objrepos lsdev -C -c tape -l $lname -t "ost" | \
		grep "$lname" > /dev/null 2>&1
	[ $? -eq 0 ] && { TYPE="ost"; return 0; }

	#------------------------------------------------
	# Not a recognized device type.			|
	#------------------------------------------------

	echo "\n$cmd:  ERROR:  Device type not recognized.  Valid\n"
	echo "\tdevice types are 4mm2gb, 4mm4gb, 8mm5gb, 8mm, 150mb,\n"
	echo "\t525mb and ost.\n"
	cleanup
	exit 1
}
# Main Program begins here
PATH=$PATH:`pwd`:
export PATH
cmd=`basename $0`
cmdline="$*"
imagedir=`pwd`
device="/dev/rmt0"
lname="rmt0"
media_type="tape"
block_size=31744
image_list=""
TYPE=""	# Tape type
DEV=""
trap respawn_function 2 3 15
: ${DIAGBOOT_IMAGE:=diagboot.tape}
# evaluate flags passed in
set -- `getopt "c:d:l:Vh" $*`
if [ $? -ne 0 ]
then
	echo "ERROR in command line '$cmdline':\n"
	usage
fi

while [ "$1" != "--" ]
do
	case $1 in
		"-c")
		    shift
		    imagedir="$1"
		    ;;
		"-d")
		    shift
		    lname=$1
		    device="/dev/$1"
		    ;;
		"-l")
		    shift
		    image_list=`cat $1`
		    ;;
		"-V")
		    # display SCCS status
		    SCCS_CHECKEDIN="@(#)"
		    SCCS_VERSIONSTR="1.1 - 94/05/06 %U"
		    sccs_threechar=`echo $SCCS_CHECKEDIN | cut -c3-3`
	            if [ "q$sccs_threechar" != "q#" ]
		    then
			 echo "$cmd: not checked in."
		    else
			 echo "$cmd: version $SCCS_VERSIONSTR."
		    fi
		    exit 0
		    ;;

		"-h")
		    usage
		    ;;
	esac
	shift
done

if [ "$image_list" = "" ]
then
	echo "The list of images to be copied is empty"
	usage
fi

get_type

# make sure that the tape block size is set to 512

current_tapebs=`ODMDIR=/etc/objrepos lsattr -E -a block_size -l $lname | cut -d' ' -f2`
if [ "$current_tapebs" != "512" ]
then
    echo "\n$cmd:  ERROR:  Tape block size needs to be set to 512."
    echo "\tCurrent block size is set to $current_tapebs"
    exit 1
fi
# Prompt for a tape mount.
WORK_DATE=`date +"%y""%m""%d""%H""%M""%S"`      # Work directory date stamp
WORK=/tmp/makediagboot.$WORK_DATE
echo "\07\nMount a tape in $device and press enter"
read gorp

touch $WORK
dd if=$WORK of=$device 2>/dev/null
while [ $? -ne 0 ]; do
    echo "Could not write to device $device."
    echo "\07\nCheck that a writable tape is mounted and press enter"
    read gorp
    dd if=$WORK of=$device 2>/dev/null
done
rm $WORK >/dev/null 2>&1
if [ "$TYPE" = "525mb" -o "$TYPE" = "ost" ]
then
	DEV="$device".1
else
	DEV="$device".5
fi
echo "Copying bootimage..."
dd if=$imagedir/$DIAGBOOT_IMAGE of=$DEV bs=$block_size conv=sync
if [ $? -ne 0 ]
then
	p_error
fi
   
for i in $image_list
do
   echo "Copying $i..."
   dd if=$imagedir/$i of=$DEV bs=$block_size conv=sync
   if [ $? -ne 0  ]
   then
	p_error
   fi
   if [ "$i" = "tape1.image" -o "$i" = "tape3.image" ]
   then
	tctl -f$DEV weof 1
   	if [ $? -ne 0  ]
	then
		p_error
   	fi
   fi
done
tctl -f$device rewind
exit 0

