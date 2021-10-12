#!/bin/ksh
# @(#)081.7 src/bos/usr/bin/restvg/restvg.sh, cmdbsys, bos41B, 412_41B_sync 12/22/94 09:47:45
#
# COMPONENT_NAME: (CMDBSYS)
#
# FUNCTIONS: restvg.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
## [End of PROLOG]

#################
#
# FILE NAME: restvg
#
# FILE DESCRIPTION: High-level shell command for restoring a volume group.
#   The restvg command will create the Volume Group, Logical Volumes and
#   Filesystems as specified in the <vgname>.data file from the backup
#   image (-f <device/file> default is /dev/rmt0).
#   If the value "EXACT_FIT" is set to "yes" and the LV Maps are included
#   in the backup image. restvg will create the Logical Volumes with the
#   specified map files.
#   The Exact_Fit will be overridden by the "SHRINK" parameter, either by
#   the -s flag or by the "SHRINK = yes" value in the <vgname>.data file.
#   The user will be prompted before the actual creation of the VG begins,
#   unless the -q flag is used.
#   The target disks are taken for the <vgname>.data file or may be
#   specified on the command line.  The target disks MUST not belong to
#   any volume group.
#   Only User volume groups may be restored by the restvg command.

#
# RETURN VALUE DESCRIPTION:
#                             0         Successful
#                             non-zero  Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED: mkvg, mklv, mkfs commands
#
# GLOBAL VARIABLES: none
#

################################# cleanup #######################################
#
# NAME: cleanup ()
#
# DESCRIPTION: Unset traps, issue error message and exit w/ an error code
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
#       script with an exit value of $1)
#
cleanup ()
{
ec=$1
error=$2

case "$ec"
in
	"$USAGE_EC" )    # restvg usage error
		error="`${dspmsg} -s $MSGSET $MSGCAT 1 '
Usage:  %s [-q] [-s] [-b blocks] [-f device] diskname...\n
-q\tdo not prompt before creating the Volume Group
-s\tCreate the Logical Volumes to the Minimum Size as specified
  \tin the vgname.data file.
-b blocks\tNumber of 512-byte blocks to read in a single
	 \tinput operation
-f device\tName of device to restore the information from.
	 \tDefault is /dev/rmt0
diskname..\tlist of target disk devices to overwrite the disks
	 \tspecfied in the vgname.data file.\n\n' $NAME`" 1>&2 ;;

	"$EXIT_EC" | "$TRAP_EC")
		;;

	*)      ;;
esac

[ -n "$error" ] && echo "\n${error}\n"
trap '' 1 2 15
exit "$ec"
}
####################################################
getoptions ()
{
    # Parse command line arguments
    set -- `${getopt} b:f:qs? $*`     # restvg options
    [ $? -ne 0 ] && cleanup "$USAGE_EC" ""

    while [ $1 != -- ]            ## Parse flag arguements
    do
	case $1 in
		"-b") BOPT="-b $2" ;;  ## get number of blocks
		"-q") PROMPT="no";;   ## Do Not prompt before Creating VG
		"-s") SHRINK="yes";;  ## Set Shrink = yes
		"-f") DEVICE=$2; shift ;;  ## get backup device
		"-?") cleanup "$USAGE_EC" "";;  ## Display usage message
	esac
	shift
    done

    set `${getopt} $*`     #Get list of target disks
    TDisks=$*   ## Get Target Disks from CmdLine
    [ -n "$TDisks" ] && REMAP=yes   ## Will need to remap the hdisk names
	## Check each target disk to make sure it does not belong to a VG
    validate_disks
}

#####  Get Stanza Data from data file

function get_stanza_data
{
    STANZA=$1    ## Stanza Name
    ATTR="$2"    ## Field Name=
    DSK=$3       ##      Unique Field Name Value (may be null)
    RECORD=$4    ## Number of Record within the stanza to return

    ${grep} -p $STANZA: $IMAGE_DATA | ${awk} ' BEGIN { RS = "" ;  FS = "\n" }
	     /'$ATTR'= '$DSK'/ {  print $'$RECORD' } ' |  \
	     ${awk} '{ for (i = 2; i <= NF; i++)
		       { print $i } }'
    return 0
}

## Check the number of Target disks
##    if less than the number of source disks then return error....
function chk_num_disks
{
   CNT=`echo $VGDisks | ${awk} '{print NF}'`
   TCNT=`echo $TDisks | ${awk} '{print NF}'`
   [ $CNT -gt $TCNT ] &&  return 1
   return 0
}

## Check the sizes of the Target disks
##    if less than the size of the source disks then exit.....
function chk_disk_sizes
{
   CNT=0
   for dk in $VGDisks
   do
      CNT=`${expr} $CNT + 1`
      vg_dsk_size=`get_stanza_data "source_disk_data" "HDISKNAME"  $dk 3`
      tdk=`echo $TDisks | ${awk} '{ print $'$CNT' }'`
      T_DSK_Size=`${bootinfo} -s $tdk`
      [ $T_DSK_Size -lt $vg_dsk_size ] && return 1
   done
   return 0
}

## map Disks - map a list of disks ($TDisks) to the
## Disks listed in the Data  file ($VGDisks)
##    Requires that $TDisks list be equal or larger than $VGDisks
##    Will return re-mapped disk list in $mlist

function map_disks
{
   LIST=$*
   mlist=""
   for VGDK in $LIST
   do
       ACNT=`echo $VGDisks | ${awk} ' BEGIN { RS = " " }
			    /'$VGDK'/ { print NR }'`
       mlist="$mlist `echo $TDisks | ${awk} '{ print $'$ACNT' }'`"
   done
 return 0
}

## check each disk listed in $TDisks to assure that:
##  - the disk exists
##  - the disk does not belong to a Volume Group
##  this function will not return on error, but will exit

function validate_disks
{
    for td in $TDisks
    do
       JNK=`${bootinfo} -s $td`
       [ $? -ne 0 ] && cleanup "$EXIT_EC" \
       "`${dspmsg} -s $MSGSET $MSGCAT 16 \
	'0512-036 %s: Target Disk %s is invalid.  Restore of Volume Group canceled.' $NAME $td`"
       ${getlvodm} -j $td >/dev/null 2>&1
       [ $? -eq 0 ] && cleanup "$EXIT_EC" \
       "`${dspmsg} -s $MSGSET $MSGCAT 17 \
	'0512-037 %s: Target Disk %s Already belongs to a Volume Group.  Restore of Volume Group canceled.' $NAME $td`"
    done
}
######  end of function declarations ###############
##########  start of main routine ##################

	#
	# set up environment
	#
PATH=/usr/bin:/usr/sbin:/sbin:/etc: ; export PATH
NAME=`/usr/bin/basename $0`
MSGCAT=mksysb.cat        # message catalog name
MSGSET=3                # message set number
USAGE_EC=1              # exit code for usage error
EXIT_EC=2               # exit code
TRAP_EC=4               # exit code for trap
DEVICE="/dev/rmt0"      # initialize backup device to rmt0
BOPT=                   # # Blocks for the B option to restore
PROMPT="yes"             ## prompt before Creating VG
TDisks=                 ## initialize target disk list to NULL
SHRINK=                 ## Set Shrink = null
Exact_Fit="no"          ## Set Exact fit = null (do not use LV maps)
REMAP=                  ## Remap hdisk names
VGDATA_DIR=/tmp/vgdata    # VG data Directory
VGDATA_LIST=$VGDATA_DIR/vgdata.files # file listing vg.data files in archive
TMP_FILESYSTEMS=/tmp/filesystems.$$

##    Full-Path to commands used
awk=/usr/bin/awk
cat=/usr/bin/cat
cp=/usr/bin/cp
dspmsg=/usr/bin/dspmsg
expr=/usr/bin/expr
getopt=/usr/bin/getopt
grep=/usr/bin/grep
ls=/usr/bin/ls
mkdir=/usr/bin/mkdir
mount=/usr/sbin/mount
rm=/usr/bin/rm
bootinfo=/usr/sbin/bootinfo
getlvodm=/usr/sbin/getlvodm
restore=/usr/sbin/restore
mkvg=/usr/sbin/mkvg
varyonvg=/usr/sbin/varyonvg
sed=/usr/bin/sed
mv=/usr/bin/mv
mklv=/usr/sbin/mklv
mkfs=/usr/sbin/mkfs


	#
	# Trap on exit/interrupt/break to clean up
	#

trap "cleanup $TRAP_EC \"`${dspmsg} -s 1 $MSGCAT 4 '0512-007 %s: Abnormal program termination.' $NAME`"\"  1 2 15

getoptions $*    # Parse the commandline arguements

${rm} -fr $VGDATA_DIR
cd /  >/dev/null 2>&1
	 # get data file and maps (if available)
${restore} -xqf $DEVICE $BOPT .$VGDATA_LIST >/dev/null 2>&1
[ ! -r $VGDATA_LIST ] && cleanup "$EXIT_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 2 \
	'0512-025 %s: The image data file does not exist.  Restore of Volume Group canceled.' $NAME`"
${restore} -xqf $DEVICE $BOPT  `${cat} $VGDATA_LIST` >/dev/null 2>&1
${rm} -f .$VGDATA_LIST
VG=`${ls} $VGDATA_DIR`         # Get Volume group name
VG_DIR=$VGDATA_DIR/$VG         # vg data Directory
IMAGE_DATA=$VG_DIR/$VG.data    # vg Image.data file
VG_FILESYSTEMS=$VG_DIR/filesystems  # A copy of /etc/filesystems
	#
	# check prerequsites:
	#
if [ -z $VG ]                  ## No volume group name found
then                             ## exit error
    cleanup "$EXIT_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 2 \
	'0512-025 %s: The image data file does not exist.  Restore of Volume Group canceled.' $NAME`"
fi

if [  $VG = "rootvg" ]
then                             ## exit error
    cleanup "$EXIT_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 3 \
	'0512-026 %s: The %s command cannot restore the %s Volume Group.  Restore of Volume Group canceled.' $NAME $NAME $VG`"
fi

[ ! -s $IMAGE_DATA ]  &&  cleanup "$EXIT_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 2 \
	'0512-025 %s: The image data file does not exist.  Restore of Volume Group canceled.' $NAME`"

################ Get LV Allocation Policy ##########################

if [ -z "$SHRINK" ]       # Shrink not specified on cmdline
then
    SHRINK=`get_stanza_data "logical_volume_policy" "SHRINK" "" 2`
fi
[ "$SHRINK" = "no" ] &&  \
      Exact_Fit=`get_stanza_data "logical_volume_policy" "EXACT_FIT" "" 3`

################ mkvg - Volume Group ##########################
VGDisks=`get_stanza_data "vg_data" "VGNAME" $VG 5`
if [ -z "$TDisks" ]              ## Get target disks for vg data file
then
    REMAP="no"                # No need to remap disk names
    TDisks=$VGDisks
fi
Psize=`get_stanza_data "vg_data" "VGNAME" $VG 3`

################ PROMPT Before Continuing #####################
  ${dspmsg} -s $MSGSET $MSGCAT 4 "
  Will create the Volume Group:\t%s
  Target Disks:\t" $VG   1>&2
  echo "$TDisks"
  ${dspmsg} -s $MSGSET $MSGCAT 15 \
"  Allocation Policy:
  \tShrink Filesystems:\t%s
  \tPreserve Physical Partitions for each Logical Volume:\t%s\n\n"  \
   $SHRINK $Exact_Fit   1>&2
if [ "$PROMPT" = "yes" ]
then
  ${dspmsg} -s $MSGSET $MSGCAT 5 "
Enter \"y\" to continue: "  1>&2
read resp
[ "$resp" != "y" -a "$resp" != "Y" ] && cleanup 0
fi
################ Prceed with Creating the Volume Group#########
if  [ "$Exact_Fit" = "yes" ]
then
    if [ "$REMAP" = "yes" ]    ## Target disks were specified on cmdline
    then
	chk_num_disks
	[ $? -ne 0 ] && cleanup "$EXIT_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 6 \
	'0512-027 %s: Need to specify more Disks, need at least %s.  Restore of Volume Group canceled.' $NAME $CNT`"
    fi
    chk_disk_sizes
    [ $? -ne 0 ] && cleanup "$EXIT_EC" \
    "`${dspmsg} -s $MSGSET $MSGCAT 7 \
    '0512-028 %s: One or more Disks are of the wrong size.  Restore of Volume Group canceled.' $NAME`"

else    ### check for total DASD (Will data fit in specified disks)
    if [ "$SHRINK" = "yes" ]
    then
	PARTS="LV_MIN_LPS="
    else
	PARTS="LPs="
    fi
    partitions_needed=`${grep} $PARTS $IMAGE_DATA | ${awk} '
		     { x = x + $2}
		     END { print x } '`

    NDASD=`${expr} $partitions_needed  \* $Psize`   ## DASD Needed
    TDASD=0
    for td in $TDisks
    do
	  DSKDASD=`${bootinfo} -s $td`
	  TDASD=`${expr} $TDASD \+ $DSKDASD`
    done
    [ $TDASD -lt $NDASD ] && cleanup "$EXIT_EC" \
	"`${dspmsg} -s $MSGSET $MSGCAT 8 \
	'0512-029 %s: There is not enough Room in the specified disks for the volume group.  Restore of Volume Group canceled.' $NAME`"
    if [ "$REMAP" = "yes" ]
    then
	chk_num_disks
	[ $? -ne 0 ]  && REMAP="no"
    fi
fi

################ mkvg - Volume Group ##########################
#####  Get rest of data for mkvg command

TMPV=`get_stanza_data "vg_data" "VGNAME" $VG 4`
if [ "$TMPV" = "yes" ]
then
    Varyon=""
else
    Varyon="-n "
fi
   ## Check each target disk to make sure it does not belong to a VG
validate_disks
     ## Create the volume Group
${mkvg} -f -y $VG -s $Psize $Varyon $TDisks >/dev/null 2>&1
    [ $? -ne 0 ] && cleanup "$EXIT_EC" \
    "`${dspmsg} -s $MSGSET $MSGCAT 10 \
    '0512-031 %s: Unable to create the Volume Group %s.  Restore of Volume Group canceled.' $NAME $VG`"

${varyonvg} $VG  >/dev/null 2>&1    ## Varyon the Volume Group
    [ $? -ne 0 ] && cleanup "$EXIT_EC" \
    "`${dspmsg} -s $MSGSET $MSGCAT 10 \
    '0512-030 %s: Unable to varyon the Volume Group %s.  Restore of Volume Group canceled.' $NAME $VG`"


################ Volume Group created ##########################

################ remap the map files  ##########################
if [ "$Exact_Fit" = "yes" ]
then
 MAPDATA=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  "" 27`

 SED_STR=
 for hd in $VGDisks
 do
   map_disks $hd
   typeset -u UP=$mlist
   XUP=`echo $UP | ${awk} '{ printf("%-s", $1)}'`
   SED_STR="$SED_STR -e s/$hd/$XUP/ "
 done
 for mapfile in $MAPDATA         ## Remap the map files
 do
     ${sed} $SED_STR -e "s/HDISK/hdisk/" $mapfile > $VGDATA_DIR/new.map
     ${mv} $VGDATA_DIR/new.map $mapfile
 done
fi

################ mklv - Logical Volume ##########################
#####  Get data for mklv command
# get list of all logical volumes in image data file
LVS=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  "" 5`

for lv in $LVS
do           ## For each Logical Volume
  LVDisks=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 3`   # Physical Volumes
  LVTYPE=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 7`    # -t LV Type
  MAXLP=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 8`     # -x Maximun Number of Logical Partitions
  COPY=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 9`      # -c Number of Copies
  if [ "$SHRINK" = "yes" ]
  then
     LPS=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 28`   # number of Logical Partitions (Minimum)
  else
     LPS=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 10`   # number of Logical Partitions
  fi
  INTER=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 12`    # -e Inter Policy
  INTRA=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 13`    # -a  Intra-Policy
  MIRROR=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 15`   # -w Mirror Writes
  STRICT=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 16`   # -s Strict Allocation Policy
  VERIFY=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 19`   # -v Write-Verify State
  SCH=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 21`      # -d Scheduling Policy
  BADBLK=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 23`   # -b Badblock Policy
  RELOCATE=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 24` # -r Relocation Flag
  UPPER=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 25`    # -u Upper bound
  LABEL=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 26`    # -L Label
  if [ "$Exact_Fit" = "yes" ]
  then
  MAPF=`get_stanza_data "lv_data" "LOGICAL_VOLUME"  $lv 27`      # -m Map File
  fi

  # Some values must be converted so that mklv will take them as parameters.

  AFLAG=""
  case "$INTRA" in
	  middle         ) AFLAG="-a m" ;;
	  center         ) AFLAG="-a c" ;;
	  edge           ) AFLAG="-a e" ;;
	  'inner middle' ) AFLAG="-a im" ;;
	  'inner edge'   ) AFLAG="-a ie" ;;
	  *              ) AFLAG=""      ;;
  esac

  BFLAG=""
  case "$BADBLK" in
	  relocatable     ) BFLAG="-b y" ;;
	  non-relocatable ) BFLAG="-b n" ;;
	  *               ) BFLAG=""  ;;
  esac

  DFLAG=""
  case "$SCH" in
	  parallel   ) DFLAG="-d p" ;;
	  sequential ) DFLAG="-d s" ;;
	  *          ) DFLAG=""   ;;
  esac

  EFLAG=""
  case "$INTER" in
	  minimum ) EFLAG="-e m" ;;
	  maximun ) EFLAG="-e x" ;;
	  *       ) EFLAG=""     ;;
  esac

  RFLAG=""
  case "$RELOCATE" in
	  yes ) RFLAG="-r y" ;;
	  no  ) RFLAG="-r n" ;;
	  *   ) RFLAG=""     ;;
  esac

  SFLAG=""
  case "$STRICT" in
	  yes ) SFLAG="-s y" ;;
	  no  ) SFLAG="-s n" ;;
	  *   ) SFLAG="" ;;
  esac

  VFLAG=""
  case "$VERIFY" in
	  on  ) VFLAG="-v y" ;;
	  off ) VFLAG="-v n" ;;
	  *   ) VFLAG="" ;;
  esac

  WFLAG=""
  case "$MIRROR" in
	  on  ) WFLAG="-w y" ;;
	  off ) WFLAG="-w n" ;;
	  *   ) WFLAG=""  ;;
  esac

  MFLAG=""
  if [ -s "$MAPF" -a "$Exact_Fit" = yes ]
  then
	  MFLAG="-m $MAPF"
  elif [ ! -s "$MAPF" -a "$Exact_Fit" = yes ]
  then
     cleanup "$EXIT_EC" \
    "`${dspmsg} -s $MSGSET $MSGCAT 11 \
    '0512-032 %s: The map file for the Logical Volume %s does not exist or is not readable. Restore of Volume Group canceled.' $NAME $lv`"
  fi

  CFLAG=""; [ -n "$COPY" ]   && CFLAG="-c $COPY"
  LFLAG=""; [ -n "$LABEL" ]  && LFLAG="-L $LABEL"
  TFLAG=""; [ -n "$LVTYPE" ] && TFLAG="-t $LVTYPE"
  UFLAG=""; [ -n "$UPPER" ]  && UFLAG="-u $UPPER"
  XFLAG=""; [ -n "$MAXLP" ]  && XFLAG="-x $MAXLP"

  YFLAG="-y $lv"
  if [ "$REMAP" = "yes" ]
  then
	map_disks $LVDisks
	LVDisks="$mlist"
  else
	[ -n "$TDisks" ] && LVDisks="$TDisks"
  fi
  if [ "$Exact_Fit" != "yes" ]
  then
	  # First try the mklv by specifying list of disks.
      ${mklv} $AFLAG $BFLAG $CFLAG $DFLAG $EFLAG $LFLAG $MFLAG $RFLAG  \
	$SFLAG $TFLAG $UFLAG $VFLAG $WFLAG $XFLAG $YFLAG $VG $LPS $LVDisks >/dev/null 2>&1
      rc="$?"
      # If mklv failed, try the mklv again without the list of disks.
      if [ $rc -ne 0 ]
      then
	    ${mklv} $AFLAG $BFLAG $CFLAG $DFLAG $EFLAG $LFLAG $RFLAG \
	       $SFLAG $TFLAG $UFLAG $VFLAG $WFLAG $XFLAG $YFLAG $VG $LPS >/dev/null 2>&1
       rc="$?"
      fi
  else      ##  Exact fit was specified
      ${mklv} $BFLAG $DFLAG $LFLAG $MFLAG $RFLAG  \
	 $TFLAG $VFLAG $WFLAG $XFLAG $YFLAG $VG $LPS $LVDisks >/dev/null 2>&1
      rc="$?"
  fi

  # If mklv failed, call error routine.
  if [ $rc -ne 0 ]
  then
     cleanup "$EXIT_EC" \
    "`${dspmsg} -s $MSGSET $MSGCAT 12 \
    '0512-033 %s: Failed creating the Logical Volume %s.  Restore of Volume Group canceled.' $NAME $lv`"
  fi

done    ## Create LV

################ Logical Volumes Created ##########################

################ mkfs - Filesystems  ##########################
#####  Get data for mkfs command
FSLIST=`get_stanza_data "fs_data" "FS_NAME"  "" 5`   # Get list of Filesystems

for lfs in $FSLIST
do
  fs=`echo $lfs | ${awk} '{ gsub(/\//, "\\\/", $1) ; print $0 " " }'`
  FSNAME=`get_stanza_data "fs_data" "FS_LV"  $fs 2`       # Filesystem name
  if [ "$SHRINK" != "yes" ]
  then                                                    #
       FSSIZE=`get_stanza_data "fs_data" "FS_LV"  $fs 3`  # -a Size Attribute
  else                                                    #   specified as
       FSSIZE=`get_stanza_data "fs_data" "FS_LV"  $fs 4`  #   number of 512 blks
  fi
  FSLV=`get_stanza_data "fs_data" "FS_LV"  $fs 5`        # -d Device Name
  FSFRAG=`get_stanza_data "fs_data" "FS_LV"  $fs 6`      # -a Frag Attribute
  FSNBPI=`get_stanza_data "fs_data" "FS_LV"  $fs 7`      # -a NBPI Attribute
  FSCOMPRS=`get_stanza_data "fs_data" "FS_LV"  $fs 8`    # -a Compress Attribute

  # Some values must be converted so that mkfs will take them as parameters.

## Save /etc/filesystems file temporarily

${cp} /etc/filesystems $TMP_FILESYSTEMS
{ echo ;
${grep} -p $FSNAME: $VG_FILESYSTEMS ; } >> /etc/filesystems

  VFLAG="-V jfs"        # Create JFS Filesystems only
  SFLAG=""; [ -n "$FSSIZE" ]   && SFLAG="-s $FSSIZE"
  OFLAG=""
  [ -n "$FSFRAG" ] && OFLAG="-o frag=$FSFRAG"
  [ -n "$OFLAG" -a -n "$FSNBPI" ] && OFLAG="$OFLAG,nbpi=$FSNBPI"
  [ -z "$OFLAG" -a -n "$FSNBPI" ] && OFLAG="-o nbpi=$FSNBPI"
  [ -n "$OFLAG" -a -n "$FSCOMPRS" ] && OFLAG="$OFLAG,compress=$FSCOMPRS"
  [ -z "$OFLAG" -a -n "$FSCOMPRS" ] && OFLAG="-o compress=$FSCOMPRS"

  echo "y" | ${mkfs} $VFLAG $SFLAG $OFLAG $FSLV >/dev/null 2>&1

  if [ $? -ne 0 ]
  then
      ${mv} $TMP_FILESYSTEMS /etc/filesystems       # Restore /etc/filesystems
      cleanup "$EXIT_EC" \
    "`${dspmsg} -s $MSGSET $MSGCAT 13 \
    '0512-034 %s: Failed creating the Filesystem %s.  Restore of Volume Group canceled.' $NAME $FSNAME`"
  fi

done

############### Mount the newly created Filesystems  #####################

# Get list of Filesystems (sorted)
MNTLIST=`get_stanza_data "fs_data" "FS_NAME"  "" 2 | sort`

for fs in $MNTLIST
do
  [ ! -d $fs ] &&  ${mkdir} -p $fs >/dev/null 2>&1   # create directory
  [ -d $fs ] &&  ${mount} $fs >/dev/null 2>&1    # Mount filesystem
  [ $? -ne 0 ] && cleanup "$EXIT_EC" \
    "`${dspmsg} -s $MSGSET $MSGCAT 14 \
    '0512-035 %s: Failed Mounting the Filesystem %s.  Restore of Volume Group canceled.' $NAME $fs`"
done

################ Filesystems Created And Mounted ##########################

### Restore the Files from the media
${restore} -xvqf $DEVICE $BOPT

### Get any post exec file to exec
POST_EXEC=`get_stanza_data "post_restvg" "RESTVG_FILE"  "" 2`
$POST_EXEC         ## execute any file specified

cleanup 0
