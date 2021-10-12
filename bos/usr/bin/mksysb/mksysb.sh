#!/bin/ksh
# @(#)781.21.3.7 src/bos/usr/bin/mksysb/mksysb.sh, cmdbsys, bos41J, 9523C_all 6/9/95 09:11:55
#
# COMPONENT_NAME: (CMDBSYS)
#
# FUNCTIONS: mksysb.sh
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
## [End of PROLOG]
#
# FILE NAME: mksysb
#   NOTE:  savevg command is a "symbolic" link to mksysb command
#
# FILE DESCRIPTION: High-level shell command for backing up the mounted
#   file systems in the volume group.
#   If the -i flag is specified, then the mkszfile/mkvgdata command will
#   be called.
#   If -m is specified, then the mkszfile/mkvgdata command will be called
#   with the -m flag  to create maps of the physical partitions.
#   Mksysb is passed the name of the backup device to use.  Savevg uses
#   the -f <device> parameter to specify the name of the backup device to
#   use and has a default of /dev/rmt0, and is passed the name of the
#   Volume Group to backup.
#   File/Directory Names listed in the /etc/exclude.<$VG> file will be
#   excluded from the backup.
#   Mksysb calls bosboot to generate a "boot" image, bosboot requires
#   working space in /tmp directory, to get amount required issue the
#   following command:  bosboot -qad <tapedevice>.
#
# RETURN VALUE DESCRIPTION:
#                             0         Successful
#                             non-zero  Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED: mkszfile(mkvgdata) must be called prior to
# mksysb(savevg) or the -i or -m flags must be specified, mkszfile(mkvgdata)
# create the VG.data file, which is placed at the beginning  of the
# backup media.
#  Bosboot, mkinsttape, backup commands.
#
# GLOBAL VARIABLES: none
#
################################# cleanup #######################################
#
# NAME: cleanup ()
#
# DESCRIPTION: unset traps, issue error message and exit w/ an error code
#
# INPUT:
#        $1 = exit code
#        $2 = error message if not null
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           exits with a return code of $1
#
# NOTE: This function will not return (i.e., it will exit the entire
#       script with exit value of $1)
#
cleanup ()
{
ec=$1
error=$2

case "$ec"
in
	"$USAGE_MB")    # mksysb usage error
		error="`${dspmsg} -s $MSGSET $MSGCAT 8 '
Usage:\t%s [-i] [-m] [-e] [-b blocks] device\n
-i\tCreate the /image.data file.
-m\tCreate the /image.data file and physical partition maps
-e\tExclude the files/directories
-b blocks\tNumber of 512-byte blocks to write in a single
	 \toutput operation
device\tName of device to receive the backup information.
      \tExample: %s /dev/rmt0\n' $NAME $NAME`" 1>&2 ;;

	"$USAGE_SV" | "$INVALID_VG")    # savevg usage error
		if [ "$ec" = "$INVALID_VG" ]
		then
		${dspmsg} -s $MSGSET $MSGCAT 6 \
		"0512-009 %s: Invalid or missing Volume Group Name.\n" $NAME 1>&2
		fi
		error="`${dspmsg} -s $MSGSET $MSGCAT 9 '
Usage:\t%s [-i] [-m] [-e] [-b blocks] [-f device] vgName\n
-i\tCreate the <vgname>.data file.
-m\tCreate the <vgname>.data file and physical partition maps
-e\tExclude the files/directories listed in /etc/exclude.<vgname>
-b blocks\tNumber of 512-byte blocks to write in a single
	 \toutput operation
-f device\tName of device to receive the backup information.
	 \tDefault is /dev/rmt0
vgName\tName of Volume Group to backup.\n' $NAME`" 1>&2 ;;

	"$CMD_EC" | "$TRAP_EC")
		;;

	*)      ;;
esac

${rm} "$archive_lst" "$mounted_list" > /dev/null 2>&1
if [ $NAME = "mksysb" ]
then
    [ -n "$tapedev" ] && {
	${chdev} -l $tapedev -a block_size=${oldblksize:-1024} \
			>/dev/null 2>&1
	if [ -n "$oldextfm" ]
	then
		${chdev} -l $tapedev -a extfm=$oldextfm \
                       >/dev/null 2>&1
	fi

	${tctl} -f /dev/$tapedev rewind >/dev/null 2>&1
	}
fi

[ -n "$error" ] && echo "\n${error}\n"
trap '' 1 2 15
exit "$ec"
}
####################################################
getoptions ()
{
    # Parse command line arguments
    if [ $NAME = "mksysb" ]
    then
	set -- `${getopt} b:eim? $*`       # mksysb options
    else
	set -- `${getopt} b:ef:im? $*`     # savevg options
    fi
    [ $? -ne 0 ] && cleanup "$USAGE_EC" ""

    while [ $1 != -- ]            ## Parse flag arguements
    do
	case $1 in
		"-b") BOPT="$2"; BFLAG="-b $BOPT" ;;  ## get -b parameter
		"-i") IMAGE="y" ;;        ## Create image.data file
		"-m") IMAGE="y"; MAPS="-m";;   ## Create Maps data
		"-e") EX="yes" ;; ## use exclude file
		"-f") DEVICE=$2 ;;  ## get backup device for savevg
		"-?") cleanup "$USAGE_EC" "";;  ## Display usage message
	esac
	shift
    done

    set `${getopt} $*`     #Get Device/Volume Group Name
    if [ $NAME = "mksysb" ]
    then
	 DEVICE=$1
	 VG="rootvg"
    else
	 VG=$1
	 [ "$DEVICE" = "" ] && DEVICE="/dev/rmt0"
	 [ "$VG" = "" ]  && cleanup "$INVALID_VG" ""
	 ${lsvg} $VG  >/dev/null  2>&1
	 [ $? -ne 0 ] &&  cleanup "$INVALID_VG" ""
    fi

    if [ "$DEVICE" = "" ]
    then
	cleanup "$USAGE_EC" ""
    fi

    [ `${expr} "$DEVICE" : "[/]"` -eq 0 ] && cleanup "$CMD_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 3 \
	'0512-006 %s: Backup device or file name must start with a '/'.' $NAME`"

    EXCL_DEV=""           # exclude archive file but not device (ie. rmt0)
    [ `${dirname} $DEVICE` != "/dev" ] && EXCL_DEV="|^.$DEVICE"

}
####################################################
make_bootape ()
{
device=$1

#Check to make sure we can write on the tape.
2>/dev/null >$device
rc="$?"
[ "$rc" -ne 0 ] && cleanup "$CMD_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 17 \
	'0512-0017 %s: Cannot write to the device %s.\n\t\t Either write protected or in use.' $NAME $device `"

${tctl} -f $device retension

	#
	# put the bosboot image on the tape
	#
if [ "$NAME" = "mksysb" -a "$RSTYPE" != "rspc" ]
then
    KERNEL=
    if [ -x /usr/lib/boot/unix_mp -a -x /usr/lib/methods/cfgsys_p ]
    then
        KERNEL="-k /usr/lib/boot/unix_mp"
        if [ -x /usr/lib/boot/unix_up -a -x /usr/lib/methods/cfgsys ]
        then
                cat >/usr/lib/boot/protoext/tape.proto.ext.base.com.upmp <<ENDOFDATA
cfgsys     ---- 777 0 0 /usr/lib/methods/cfgsys
cfgsys_p   ---- 777 0 0 /usr/lib/methods/cfgsys_p
ENDOFDATA
        fi
    fi

    ${bosboot} -d$device -a $KERNEL
    rc="$?"
    rm -f /usr/lib/boot/protoext/tape.proto.ext.base.com.upmp
    [ "$rc" -ne 0 ] && cleanup "$CMD_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 18 \
	'0512-016 %s: Attempt to create a bootable tape failed:\n\tbosboot -d %s -a  failed with return code %s.' $NAME $device $rc`"
else   
	#
	# put a "dummy" bosboot image on the tape
	#
    echo "Dummy Boot Image" | ${dd} of=$device bs=512 conv=sync

fi
	#
	# put the bosinst image on the tape
	#
${mkinsttape} $device
rc=$?
[ "$rc" -ne 0 ] && cleanup "$CMD_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 19 \
	'0512-021 %s: Attempt to create a bootable tape failed:\n\tmkinsttape%s failed with return code %s.' $NAME $device $rc`"
	#
	# place a dummy toc on the tape
	#
echo "Dummy tape TOC" | ${dd} of=$device bs=512 conv=sync

[ "$NAME" = "mksysb" ] && BOOTABLE=1              # tape is bootable
}
#################### main ##########################

 #
 # set up environment
 #
PATH=/usr/bin:/usr/sbin:/sbin:/etc: ; export PATH
NAME=`/usr/bin/basename $0`
MSGCAT=mksysb.cat       # message catalog name
MSGSET=1                # message set number
USAGE_MB=1              # mksysb usage err exit code
USAGE_SV=2              # savevg usage err exit code
USAGE_EC=$USAGE_MB      # set mksysb usage error
[ "$NAME" = "savevg" ] && USAGE_EC=$USAGE_SV    # set mkvgdata usage error
CMD_EC=3                # exit code prior to mounting any file systems
TRAP_EC=4               # exit code for trap

BOOTABLE=0              # true if a bootable image is made
DEVICE=                 # initialize backup device to NULL
TAPEBLKSZ=/tapeblksz    # file to save tape block size
BOPT="NONE"             # # Blocks for the B option to backup
BFLAG=
oldblksize=
tapedev=
IMAGE=
MAPS=
EX=
EXCLUDE=

##    Full-Path to commands used
awk=/usr/bin/awk
backup=/usr/sbin/backup
bootinfo=/usr/sbin/bootinfo
bosboot=/usr/sbin/bosboot
cat=/usr/bin/cat
chdev=/usr/sbin/chdev
cp=/usr/bin/cp
cut=/usr/bin/cut
dd=/usr/bin/dd
dirname=/usr/bin/dirname
dspmsg=/usr/bin/dspmsg
egrep=/usr/bin/egrep
expr=/usr/bin/expr
find=/usr/bin/find
getopt=/usr/bin/getopt
grep=/usr/bin/grep
lqueryvg=/usr/sbin/lqueryvg
ls=/usr/bin/ls
lsattr=/usr/sbin/lsattr
lsvg=/usr/sbin/lsvg
mkinsttape=/usr/sbin/mkinsttape
mkszfile=/usr/bin/mkszfile
mkvgdata=/usr/bin/mkvgdata
mount=/usr/sbin/mount
rm=/usr/bin/rm
sed=/usr/bin/sed
tctl=/usr/bin/tctl

getoptions $*    # Parse the commandline arguements

RSTYPE=`${bootinfo} -T`  ## Get System Type

BOSINST_DATA=/bosinst.data         # Bos Install Options Data file
BOSINST_VARFL=/var/adm/ras/bosinst.data  # Bos Install Data file
BOSINST_TMPLT=/usr/lpp/bosinst/bosinst.template  # Bos Install Template file
VGDATA_DIR=/tmp/vgdata             # VG data Directory
VG_DIR=$VGDATA_DIR/$VG             # vg data Directory
if [ "$VG" = "rootvg" ]
then
    IMAGE_DATA="/image.data"       # rootvg Image.data file
    EX_BOSINST="|^.$BOSINST_DATA"
else
    IMAGE_DATA=$VG_DIR/$VG.data    # vg Image.data file
    VGDATA_LIST=$VGDATA_DIR/vgdata.files # file listing vg.data files in archive
    VG_FILESYSTEMS=$VG_DIR/filesystems  # A copy of /etc/filesystems
    EX_BOSINST=""
fi

EXCLUDE_DATA=/etc/exclude.$VG  # File containing list of files
[ ! -s "$EXCLUDE_DATA" ] && EX=		# and/or directories to exclude

tmp_file=".archive.list.$$"
tmp_list=".MNT.list.$$"
archive_lst="/tmp/$tmp_file"
mounted_list="/tmp/$tmp_list"

	#
	# Trap on exit/interrupt/break to clean up
	#

trap "cleanup $TRAP_EC \"`${dspmsg} -s $MSGSET $MSGCAT 4 '0512-007 %s: Abnormal program termination.' $NAME`"\"   1 2 15

	#
	# check prerequsites:
	#       usage;
	#       $IMAGE_DATA file exists or is created (-i flag);
	#       $BOSINST_DATA file exists;
	#       the absolute path was used in $device.
	#

if [ "$VG" = "rootvg" ]
then
     [ ! -s $BOSINST_DATA ]  &&  ${cp} $BOSINST_VARFL $BOSINST_DATA 2> /dev/null
     [ ! -s $BOSINST_DATA ]  &&  ${cp} $BOSINST_TMPLT $BOSINST_DATA 2> /dev/null
     [ ! -s $BOSINST_DATA ]  &&  cleanup "$CMD_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 1 \
	'0512-004 %s: The %s file does not exist.  Backup canceled.' $NAME $BOSINST_DATA`"
fi

if [ "$IMAGE" = "y" ]        # Create image_data file
then
	if [ "$NAME" = "mksysb" ]
	then
	     MKDATA_CMD="mkszfile"
	     ${mkszfile} $MAPS    ## call mkszfile
	else
	     MKDATA_CMD="mkvgdata"
	     ${mkvgdata} $MAPS $VG    ## call mkvgdata
	fi
	[ $? != 0 ] && cleanup "$CMD_EC"  \
	"`${dspmsg} -s $MSGSET $MSGCAT 5 \
	'0512-008 %s: The %s command failed.  Backup canceled.' $NAME $MKDATA_CMD`"
fi

[ ! -s $IMAGE_DATA ]  &&  cleanup "$CMD_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 1 \
	'0512-004 %s: The %s file does not exist.  Backup canceled.' $NAME $IMAGE_DATA`"

if [ -r $VG_DIR/*.map ]      ## Gets any maps found
then
    MAP_DATA=`${ls} -1 $VG_DIR/*map | ${awk} '{print $1}'`
fi

case "$DEVICE"
in
   #
   # Fix the block size if $DEVICE is a tape device
   #
   /dev/rmt[0-9]*)
       if [ "$VG" = "rootvg" ]
       then
	  # see if a low or high density tape device was specified
	  # (eg rmt0.1)
	  density="`${expr} $DEVICE : \
		  "/dev/rmt[0-9]*\.\([0-9]*\)"`"
	  #
	  # strip /dev/ from device name and
	  # get the base name (eg translate:
	  # /dev/rmt0.2 to rmt0)
	  #
	  tapedev="`${expr} $DEVICE : \
		  "/dev/\(rmt[0-9]*\)[\.]*[0-9]*"`"
	  #
	  # change device to 512 tape block size
	  #
	  CMD="${lsattr} -E -O -a block_size"
	  oldblksize=`LC_MESSAGES=C $CMD -l $tapedev 2> /dev/null | ${grep} -v block_size`
	  if [ "$?" -eq 0 ]
	  then
	      oldblksize=${oldblksize:-512}
	      echo "$oldblksize $BOPT" > $TAPEBLKSZ
	      ${chdev} -l $tapedev -a block_size=512 >/dev/null 2>&1
              # Force use of extended file marks
              if [ $oldblksize -gt 512 ]
              then
                      CMD2="${lsattr} -E -O -a extfm"
                      oldextfm=`LC_MESSAGES=C $CMD2 -l $tapedev \
                              2> /dev/null | ${grep} -v extfm`
                      if [ "$?" -eq 0 ]
                      then
                              oldextfm=${oldextfm:-no}
                              ${chdev} -l $tapedev -a extfm=yes >/dev/null 2>&1
                      else
                              oldextfm=
                      fi
              fi

	      #
	      # Use the same density device driver as
	      # the user specified, but since we are now
	      # making bootable tapes, the tape drive
	      # must be no-rewind. Thus, rmt(n).1, rmt(n).2,
	      # and rmt(n).3 will be mapped to rmt(n).1 and
	      # rmt(n).4 and above map to rmt(n).5.
	      #
	      [ "${density:-1}" -lt 4 ] && density=1 || density=5
	      TDEVICE="/dev/${tapedev}.${density}"
	      make_bootape $TDEVICE
	      # reset tape device to tape block size preset by user.
	      if [ $oldblksize -gt 512 ]
	      then
		   ${chdev} -l $tapedev -a block_size=${oldblksize:-512} \
				>/dev/null 2>&1

                   # Reset extfm to value preset by user
                   if [ -n "$oldextfm" ]
                   then
                           ${chdev} -l $tapedev -a extfm=$oldextfm \
                                   >/dev/null 2>&1
                   fi

		   ${tctl} -f $TDEVICE rewind   # wait for tape to rewind
		   ${tctl} -f $TDEVICE fsf 3
		   tapedev=
	      fi
	      ${rm} -f $TAPEBLKSZ
	  else
	      tapedev=
	  fi    # end of test for tape device
       fi    # end of test for rootvg Volume Group
   ;;        # end of Tape Case

   #
   # make sure someone's not backing up over /dev/hdisk*!
   # granted, this is a very incomplete check, but it could
   # catch some people before they wipe their world out
   #
   *hdisk* )
	   ${lqueryvg} -p $DEVICE > /dev/null 2>&1 && \
	   cleanup "$CMD_EC" "`${dspmsg} -s $MSGSET $MSGCAT 10 \
	   '0512-015 %s: %s appears to be a member of a volume group.\n\tBackup canceled.' $NAME $DEVICE`"
   ;;       # end of hdisk case

   *) ;;    # all other cases
esac

[ "$NAME" = "mksysb" -a "$BOOTABLE" -eq 0 ]  && echo  "`${dspmsg} -s $MSGSET $MSGCAT 21 \
	'0512-039 %s: WARNING: %s does not appear to be a tape device and\n\twill NOT have a bootable image.' $NAME $DEVICE`"

  #
  # make a list of filessystems to be backed up
  #

for i in `${grep} "FS_NAME=" $IMAGE_DATA | ${awk} 'BEGIN{FS="="} {print $2}'`
do
	rvg_list="$i $rvg_list"
done

cd / > /dev/null 2>&1
	# Put the bosint.data, image.data and Map files in front of tape
if  [ $VG != "rootvg" ]
then
   {  echo .$IMAGE_DATA
      echo .$VG_FILESYSTEMS
      for m in $MAP_DATA
      do
	  echo .$m
      done
   } > "$VGDATA_LIST"
fi
	#
	# make a list of files to backup using
	# the list of filesystems just created
	#
	# DO NOT remove the '.' on the '.$i' in the find command. The list
	# of files need to be relative to the current directory (/)
	#

{  if [ $VG = "rootvg" ]
   then                       # Save bosinst.data file for rootvg
      echo .$BOSINST_DATA
   else                       # save othe vg specific files for other vgs
      echo .$VGDATA_LIST
      echo .$VG_FILESYSTEMS
   fi
 echo .$IMAGE_DATA
 for m in $MAP_DATA
 do
    echo .$m
 done

for i in $rvg_list
do
  if [ -n "$EX" ]
  then                     ## use exclude file
      ${find} .$i ! -name "$tmp_file" ! -type s -fstype jfs -xdev -print |  \
      ${egrep} -v "^.$IMAGE_DATA|^.$VGDATA_DIR$EX_BOSINST$EXCL_DEV" | \
      ${egrep} -v -f $EXCLUDE_DATA 2> /dev/null
  else
      ${find} .$i ! -name "$tmp_file" ! -type s -fstype jfs -xdev -print |  \
      ${egrep} -v "^.$IMAGE_DATA|^.$VGDATA_DIR$EX_BOSINST$EXCL_DEV"  2> /dev/null
  fi
done ; } > "$archive_lst"
rc=$?

[ "$rc" -ne 0 ] &&  cleanup "$CMD_EC" \
	   "`${dspmsg} -s $MSGSET $MSGCAT 7 \
	   '0512-010 %s: Creation of backup list failed' $NAME`"

# Add the remote file system mount points to the list.
# if the node field is not blank then the "mounted over" is the third field
LC_MESSAGES=C remote_mnts=`${mount} |  \
		 ${awk} '(NR>2){
		 if (/^ / && $3!="jfs"){print $2}
		 if (!/^ / && $4!="jfs"){print $3}}'`

> "$mounted_list"
for i in $remote_mnts
do
      ${egrep} -x .`${dirname} $i` "$archive_lst" > /dev/null 2>&1
      [ "$?" -eq 0 ] && echo .$i >> "$mounted_list"
done
[ -s "$mounted_list" ] && cat "$mounted_list" >> "$archive_lst"


	#
	# Backup the files in the list
	#

echo
${dspmsg} -s $MSGSET $MSGCAT 30 'Backing up...'

# As a progress indicator print dots every 15 seconds during 
# the system backup
while :; do
	sleep 15
	echo ".\c"
done &

PID=$!
   
cd / > /dev/null 2>&1
${cat} "$archive_lst" | ${backup}  -iqpf"$DEVICE" $BFLAG
rc=$?

kill $PID
echo

	#
	# Backup completed
	#

if [ "$rc" -gt 1 ]  
then
	echo "`${dspmsg} -s $MSGSET $MSGCAT 2 \
	'0512-005 %s: Backup Completed.\n\tThe backup command completed with errors.\n\tThe messages displayed on Standard Error contained additional\n\tinformation.' $NAME`"
	rc="$CMD_EC"
elif [ "$rc" -eq 1 ]  
then
	echo  "`${dspmsg} -s $MSGSET $MSGCAT 31 \
	'0512-003 %s may not have been able to archive some files.\nThe messages displayed on the Standard Error contained additional\ninformation.\n' $NAME`"
	rc=0
else
	echo  "`${dspmsg} -s $MSGSET $MSGCAT 20 \
	'0512-038 %s: Backup Completed Successfully.' $NAME`"
	rc=0
fi
[ "$NAME" = "mksysb" -a "$BOOTABLE" -eq 0 ]  && echo  "`${dspmsg} -s $MSGSET $MSGCAT 22 \
	'0512-040 %s: WARNING: %s does not appear to be a tape device and\n\tdoes NOT have a bootable image.' $NAME $DEVICE`"

[ "$NAME" = "mksysb" -a "$BOOTABLE" -eq 1 -a "$RSTYPE" = "rspc" ] &&  \
      echo  "`${dspmsg} -s $MSGSET $MSGCAT 25 \
       '0512-043 %s: WARNING: This system does not support booting\n\tfrom a tape device; therefore, the mksysb tape does not contain a\n\tbootable image.  To install the mksysb image you will need to boot\n\tyour system from a bootable media.\n' $NAME`"


cleanup "$rc" ""
