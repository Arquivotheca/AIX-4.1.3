#!/bin/ksh
# @(#)92	1.1  src/bos/usr/bin/errlg/odm/logsymptom.sh, cmderrlg, bos41B, 9504A 12/13/94 13:57:35

#
# COMPONENT_NAME: (CMDERRLG) Error logging sample
#
# FUNCTIONS: Find a coredump
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Name:  Exit
#
# Function:  Exit the script
#
Exit() {
	# unmount remove dump directory if present.
	if [ -n "$mountdir" ]
	then	# It's there
		umount -f $mountdir >/dev/null 2>&1
		rmdir $mountdir 2>/dev/null
	fi
	# Remove temp file if specified.
	[ -n "$tmpf" ] && rm -f $tmpf
	exit $1
}

#
# Name:  main (main body)
#
# Function:
#   This shell first gets control when a DUMP_STATS error is logged.
#   It then puts the output of sysdumpdev -L into a file for later use.
#
#   It then gets control later in the boot process, after NFS has started.
#   It uses the saved sysdumpdev -L output to find the dump, if possible.
#   If found, it runs crash's "symptom" subcommand to generate a
#   symptom string in the error log.
#   It will always delete the sysdumpdev -L output.
#
# Input:
#   If a parameter is specified, this is the first invocation of
#   this script i.e., the error notify method.


# Execution path
PATH=/usr/bin:/etc:/usr/sbin:/usr/ucb:/sbin; export PATH

pgm=`basename $0`

# Use C for text parsing
LANG=C; export LANG

# Log to alog_type of dumpsymp
alogparms="-t dumpsymp"
tmpf=/tmp/sysdumpdev-L

# mountdir is only set if we have a remote dump
mountdir=

# See if notify method?
if [ $# -ne 0 ]
then	# Error notify method.
	# Log this event
	echo "------ $pgm $*" `date` | alog $alogparms

	# Get info for recent dump.
	sysdumpdev -L >$tmpf
	exit 0
	# Note:  The Exit (cap e) function is NOT used.
	# The real work is done later, see below.
fi

# This is the subsequent execution (from inittab)
# Just exit if $tmpf isn't there.
[ -s $tmpf ] || Exit 0

# Log this event
echo "------ $pgm" `date` | alog $alogparms

# Get the dump status
dumpstat=`fgrep 'Dump status' $tmpf`
dumpstat=`echo $dumpstat | sed 's/.* \(.*$\)/\1/'`
# exit if status not 0=good or -2=0C4.
if [ "$dumpstat" = '0' -o "$dumpstat" = '-2' ] 
then	# Dump's ok
	echo "Status:$dumpstat" | alog $alogparms

else	# unusable.
	echo "Dump not usable, Status:$dumpstat" | alog $alogparms
	Exit 2
fi

# See if dump was copied.
dumpname=`fgrep 'Dump copy filename' $tmpf`
if [ -n "$dumpname" ]
then	# The dump was copied to a file.
	echo "$dumpname" | alog $alogparms
	# Get the actual filename if present.
	dumpname=`echo $dumpname | sed 's/.* \(.*$\)/\1/'`
	if [ -n "$dumpname" -a -r "$dumpname" ]
	then	# We've got the filename and can read it.
		continue

	else	# Dump not copied.
		echo "Copy file not usable." | alog $alogparms
		Exit 1
	fi

else	# Try to get dump from the device.
	# Get the dump device name.
	dumpdev=`fgrep 'Device name' $tmpf`
	dumpdev=`echo $dumpdev | sed 's/.* \(.*$\)/\1/'`
	if [ -z "$dumpdev" ]
	then	# dump device not found.
		echo "Dump device not found." | alog $alogparms
		Exit 1
	fi
	echo "Device:$dumpdev" | alog $alogparms

	# basename needed for ODM queries.
	dumpname=`basename $dumpdev`

	# See if dump device is a logical volume
	attr=`odmget -q "name = $dumpname and attribute = 'lvserial_id'" CuAt 2>/dev/null`
	if [ -n "$attr" ]
	then	# It's an LV
		dumpname=$dumpdev

	else	# Not a logical volume.
		# Check for remote dump.
		echo $dumpdev | fgrep ':' >/dev/null
		if [ $? -ne 0 ]
		then	# Not remote either.
			echo "Can't use device." | alog $alogparms
			Exit 1
		fi
		host_dir=`dirname $dumpdev`  # Looks like host:dir
		hostfile=`basename $dumpdev`
		mountdir=/tmp/mnt.$$
		# make a temporary mount point
		mkdir $mountdir
		# Try the mount
		mount $host_dir $mountdir 2>/dev/null
		if [ $? -ne 0 ]
		then	# It didn't mount
			echo "mount $host_dir $mountdir failed." | alog $alogparms
			Exit 3
		fi
		# It's mounted up
		dumpname=$mountdir"/"$hostfile
	fi
fi

# $dumpname is either a copy filename, an LV pathname or a remote filename.

# Generate a symptom string.
# Put it in the error log (symptom -e).
echo "symptom -e" | crash $dumpname /unix 2>&1 | alog $alogparms
echo | alog $alogparms

Exit 0
