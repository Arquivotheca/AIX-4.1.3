#!/bin/ksh
# @(#)56	1.1  src/bos/usr/bin/errlg/samples/corepath.sh, cmderrlg, bos411, 9428A410j 3/9/94 13:02:37

#
# COMPONENT_NAME: (CMDERRLG) Error logging sample
#
# FUNCTIONS: Find a coredump
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990,1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# corepath takes the sequence number of an error log entry for a
# core dump.  It yields the pathname of the core file.

dir=`dirname $0`
PATH=$dir:$PATH
pgm=`basename $0`
log=/tmp/$pgm.log
>$log
if [ -z "$1" -o "$1" = "-?" ]
then	# Help
	echo "Usage:  $pgm <error-sequence-number>" >>$log
	exit 1
fi

seq=$1
mailid=$2
echo $seq $mailid >>$log

tmpf=/tmp/$pgm.$$
tmpf2=/tmp/$pgm.2.$$

# Get a report (errpt) of the entry.  Have LANG=C so we can
# look for specific strings in English.
savelang=$LANG
LANG=C; export LANG
errpt -a -l $seq >$tmpf 
LANG=$savelang

# Get the fs serial number and inode number.
awk "\
  \$0 ~ /^FILE SYSTEM SERIAL NUMBER/ {ser=1;next};
  \$0 ~ /^INODE NUMBER/ {ino=1;next};
  ser == 1 {printf(\"%d \",\$1); ser=2; next};
  ino == 1 {printf(\"%d\",\$1); ino=2; next};
  END {if ((ser == 2) && (ino == 2)) exit 0; else exit 1}" <$tmpf >$tmpf2
if [ $? != 0 ]
then	# Didn't get both fsnum and inode
	echo "Couldn't find all information" >>$log
	rm -f $tmpf $tmpf2
	exit 3
fi

# Get the values from tmpf2
read serno inode <$tmpf2
rm -f $tmpf $tmpf2
echo "Serial $serno, inode $inode" >>$log

# Get the FS name or device for the given serial number.
name=`getvfsname $serno`
[ $? != 0 ] && exit $?

# get the mount point
# This uses the |& feature of ksh which allows a command's output to
# be piped back into the shell's "read -p".
(mount | awk "
  BEGIN {rv=1}
  # Skip nonsense data
  NF < 3 {next}
  # See if there is a remote system name
  {if (index(\$0,\$1) == 1) dev=2;
  else dev=1;
  # dev is the field # of the device,
  # mp for the mountpoint and typ for the type.
  mp = dev+1;
  typ = dev+2;}
  # Now return the mountpoint and type if the devices match.
  \$dev == \"$name\" {printf(\"%s %s\n\",\$mp,\$typ); rv=0; exit rv}
  END {if (rv!=0) print \"unknown unknown\"; exit rv}") |&
read -p dirn dirtype
echo "FS name $name, mount point $dirn, type $dirtype" >>$log

# exit if mount point not found.
[ "$dirn" = "unknown" -a "$dirtype" = "unknown" ] && exit 3

# Exit if mount point is not a jfs
# Comment out the next two lines to search all file systems.
[ "$dirtype" != "jfs" ] && exit 3
findtype="-fstype jfs"

# We got a mount point.  Use "find" to get the directory
# with the corresponding inode in that file system.
coredir=`find $dirn -inum $inode -type d $findtype -print 2>/dev/null`
corefn=$coredir"/core"
if [ -z $coredir -o ! -f $corefn ] 
then	# Not found or file not there
	echo "path = $coredir, file = $corefn" >>$log
	exit 3
fi
msg="The core file associated with error log entry $seq is $coredir/core."

# Send a mail message to $2
echo $msg | mail -s "Core Dump" $mailid

# Put a message in the error log if uncommented.
#errlogger $msg

exit 0
