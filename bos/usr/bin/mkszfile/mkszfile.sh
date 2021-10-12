#!/bin/ksh
# @(#)801.40 src/bos/usr/bin/mkszfile/mkszfile.sh, cmdbsys, bos41J, 9512A_all 3/17/95 11:00:36
#
# COMPONENT_NAME: (CMDBSYS)
#
# FUNCTIONS: mkszfile.sh
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
## [End of PROLOG]

# FILE NAME: mkszfile
#   NOTE: mkvgdata command is a "symbolic" link to mkszfile command.
#
# FILE DESCRIPTION: High-level shell command for creating the Volume
#   Group(VG) Data file. This file captures the Volume Group
#   characteristics; the Physical Volumes (and sizes) in the VG, all the
#   characteristics of the non-jfs Logical Volumes and MOUNTED
#   Filesystems (jfs Logical Volumes) in the VG.  This will exclude NFS,
#   CD-ROM, and other "special" file systems.
#   If the Volume Group is the rootvg, then the System LANG environment
#   variable is also captured.
#   Specifying the "-m" flag, will cause mkszfile to generate "maps" of
#   the physical layout of the physical partitions used by each Logical
#   Volume.
#   Mkszfile will determine if there is sufficient work space available
#   in the file systems for the image data file and logical volume map
#   file(s) and also (for rootvg) will determine if enough work space is
#   available in /tmp to generate a Boot image.
#   For user Volume Groups, the image data file and map file(s) are all
#   created in the /tmp filesystem, in Directory /tmp/vgdata/<vgname>/
#   For the rootvg Volume Group, the image data file is placed in the
#   / filesystem and map file(s) are created in the /tmp/vgdata/rootvg/
#   directory.
#
# INPUT FLAGS:
#        -m  Generate Physical Partition Location Data
#        VGname  Name of VG to generate data on (mkvgdata only)
#        NOTE: the -f flag for the mkszfile command is no longer used, for
#              compatibility to existing scripts, the -f flag will be
#              excepted from the commandline but no action will be taken
#              and no error will be returned.
#
# OUTPUT:
#        Error messages (Standard Error)
#        Image Data file (/image.data for rootvg or
#                   /tmp/vgdata/<vgname>/<vgname>.data for user VGs)
#        Map Files in /tmp/vgdata/<vgname>/<LVname>.map
#
# RETURN VALUE DESCRIPTION:
#                             0         Successful
#                             non-zero  Unsuccessful
#
# EXTERNAL PROCEDURES CALLED: lsvg, lslv, lsjfs, df, bootinfo, bosboot
#
# GLOBAL VARIABLES: none
#
########################## Function Declarations #########################

############################## cleanup #####################################
#
# NAME: cleanup()
#
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
# RETURN VALUE DESCRIPTION: exits with a return code of $1
# NOTE: This function will not return (i.e., it will exit the entire
#       script with exit value of $1)
##########
cleanup ()
{
ec=$1
error=$2

case "$ec"
in
   "$USAGE_SZ")    # mkszfile usage error
	   error="`${dspmsg} -s $MSGSET $MSGCAT 4 '
Usage:  %s [-m]\n
-m\tGenerate Physical Partition maps\n\n' $NAME`" 1>&2 ;;

   "$USAGE_VG" | "$INVALID_VG")    # mkvgdata usage error
	   if [ "$ec" = "$INVALID_VG" ]
	   then
	   ${dspmsg} -s 1 $MSGCAT 6 \
	   "0512-009 %s: Invalid or missing Volume Group Name.\n\n" $NAME 1>&2
	   fi
	   error="`${dspmsg} -s $MSGSET $MSGCAT 5 '
Usage:  %s [-m] vgname\n
-m\tGenerate Physical Partition maps
vgname\tName of volume group to backup.\n\n' $NAME`" 1>&2 ;;

   "$FILE_EC")    # Error creating a file or directory
	   error="`${dspmsg} -s $MSGSET $MSGCAT 22  \
		'0512-022 %s: An Error occurred creating the Image Data File or
the Physical Partition Map Files. Make sure you have write permission
to the /tmp and / directories.\n\n' $NAME`" ;;

   *)      ;;      # Error Message Passed in, in $2

esac

echo "\n${error}\n"
trap '' 1 2 15
exit "$ec"
}

###########################  getoptions  ##################################
#
# NAME: getoptions()
#
# DESCRIPTION: parse the commandline arguements, check prerequsites: usage
#
# INPUT:
#        $* = the commandline arguements
#
# OUTPUT:
#        sets variables FORCE and MAPS
#
# RETURN VALUE DESCRIPTION:
#        exits on error or
#        returns successful to program
##########
getoptions()
{
    if [ $NAME = "mkszfile" ]
    then
       set -- `${getopt} fm $*`       # mkszfile options
    else
       set -- `${getopt} m $*`        # mkvgdata options
    fi
    if [ $? -ne 0 ]
    then
	 cleanup "$USAGE_EC" ""
    fi

    while [ $1 != -- ]         ## Parse flag arguements
    do
	    case $1 in
		    "-f")   ;;       ## No-op, no error - continue
		    "-m")   MAPS="yes"  ;;
		     *) cleanup "$USAGE_EC" "";;
	    esac
	    shift
    done

    if [ $NAME = "mkszfile" ]
    then
	 VG="rootvg"
    else
	 set `${getopt} $*`     #Get VG Name of VG to Backup.
	 VG=$1
	 [ "$VG" = "" ]  && cleanup "$INVALID_VG" ""  # valid VG name?
	 ${lsvg} $VG  >/dev/null  2>&1
	 [ $? -ne 0 ] &&  cleanup "$INVALID_VG" ""
     fi
}

###########################  mk_mapfile  ##################################
#
# NAME: mk_mapfile()
#
# DESCRIPTION: Creates the map files
#
# INPUT:
#        $1 = lv_name (i.e. hd1)
#
# OUTPUT:
#        map files created for the lv in the $VG_DIR directory
#        map file name is $LV_NAME.map
#
# RETURN VALUE DESCRIPTION:
#        returns sucessful

mk_mapfile()
{
      LV_NAME=$1
      ${rm} -f $VG_DIR/$LV_NAME.map     # remove any old map data
      set -- `LC_MESSAGES=C ${lslv} -m $LV_NAME | ${grep} "0001"`
      CP1=$2          # 1st copy of LV
      CP2=$4          # 2nd copy of LV (mirror)
      CP3=$6          # 3rd copy of LV (2nd mirror)
      [ -n "$CP1" ] && LC_MESSAGES=C ${lslv} -m $LV_NAME | ${tail} +3l | ${awk} '{print $3 ":" $2}' > $VG_DIR/$LV_NAME.map
      [ -n "$CP2" ] && LC_MESSAGES=C ${lslv} -m $LV_NAME | ${tail} +3l | ${awk} '{print $5 ":" $4}' >> $VG_DIR/$LV_NAME.map
      [ -n "$CP3" ] && LC_MESSAGES=C ${lslv} -m $LV_NAME | ${tail} +3l | ${awk} '{print $7 ":" $6}' >> $VG_DIR/$LV_NAME.map
}
########################## Function Declarations #########################

################################# main ########################################
#
# set up environment
#
PATH=/usr/bin:/usr/sbin:        # export PATH
NAME=`/usr/bin/basename $0`       # Command invoked  mkszfile or mkvgdata
MSGCAT=mksysb.cat        # message catalog name
MSGSET=2
USAGE_SZ=1              # mkszfile usage err exit code
USAGE_VG=2              # mkvgdata usage err exit code
USAGE_EC=$USAGE_SZ      # set mkszfile usage error
[ $NAME = "mkvgdata" ] && USAGE_EC=$USAGE_VG    # set mkvgdata usage error

NO_SPACE_EC=3          # No Space err exit code
TRAP_EC=4              # Trap/Interrupt Encountered err exit code
INVALID_VG=5           # invalid VG specified for mkvgdata
FILE_EC=6              # Error creating a file or dir

ONEMEG=1048576         # One Meg to use for calculations
ONE_K=1024             # 1 Kbytes
FOUR_K=4096            # 4 Kbytes - Min size of file
BLK_SZ=512             # Block Size used for calculations
MIN_FILE_BLKS=8        # Minimum Num. of blks allocated per file
MAPS="no"              # Do not generate Maps (Default)

##    Full-Path to commands used
awk=/usr/bin/awk
cp=/usr/bin/cp
date=/usr/bin/date
df=/usr/bin/df
dspmsg=/usr/bin/dspmsg
expr=/usr/bin/expr
fsdb=/usr/sbin/fsdb
getopt=/usr/bin/getopt
grep=/usr/bin/grep
mkdir=/usr/bin/mkdir
rm=/usr/bin/rm
tail=/usr/bin/tail
wc=/usr/bin/wc
uname=/usr/bin/uname
bootinfo=/usr/sbin/bootinfo
bosboot=/usr/sbin/bosboot
lqueryvg=/usr/sbin/lqueryvg
lsjfs=/usr/sbin/lsjfs
lslv=/usr/sbin/lslv
lsvg=/usr/sbin/lsvg
mount=/usr/sbin/mount

BLK_SZ2=`${expr} $ONEMEG \/ $BLK_SZ`

	#
	# Trap on exit/interrupt/break to clean up
	#
trap "cleanup $TRAP_EC \"`${dspmsg} -s 1 $MSGCAT 4 '0512-007 %s: Abnormal program termination.' $NAME`"\"  1 2 15

getoptions $*               # parse commandline arguements

VGDATA_DIR=/tmp/vgdata      # VG data Directory
VG_DIR=$VGDATA_DIR/$VG      # vg data Directory
if [ $VG = "rootvg" ]
then
    VG_DATA=/image.data     # Rootvg image data file
else
    VG_DATA=$VG_DIR/$VG.data    # User vg image data file
fi
ROOT_LV=`LC_MESSAGES=C ${df} / | ${tail} +2l | ${awk} '{ if ( length($7) == 1 ) { gsub ("/dev/", "", $1); print $1 } }' `
TMP_LV=`LC_MESSAGES=C ${df} /tmp | ${tail} +2l | grep /dev | ${awk} '{gsub ("/dev/", "", $1); print $1}' `
FREETMPBLKS=`LC_MESSAGES=C ${df} /dev/$TMP_LV | ${tail} +2l | ${awk} '{print $3}'`
PPBYTES=`${lqueryvg} -p$VG -s | ${awk} '{print 2 ^ $1}'` # Get Partition Size
PPSIZE=`${expr} $PPBYTES \/ $ONEMEG`   # Get Partition Size in MBytes
###################################################################
#
#     ESTIMATE size of the VG.DATA file (in num of Partitions)
#
### Estimate of bytes needed for "source_disk_data" stanzas
SP_HDISK=`${lqueryvg} -p $VG -c | ${awk} '{print $1 * 160}'` # about 160 bytes
						       ##  per hdisk
SP_LV=`${lqueryvg} -p $VG -n | ${awk} '{print $1 * 570}'`  # about 570 bytes
						     ## per Logical Volume
### Estimate of bytes needed for "fs_data:" stanzas
SP_FS=`LC_MESSAGES=C ${lsvg} -l rootvg | ${awk} '$2 ~/jfs/ { print $1 } ' |  # about 100 bytes
		 ${wc} -l | ${awk} '{print $1 * 100}'`       ## per filesystem

### Add up all stanza values for estimate of # bytes for the image.data
	     # about 550 bytes for all other "Single Entry" stanzas
IMAGE_DATA_SZ=`echo 550 | ${awk} '{print $1 + '$SP_HDISK' + '$SP_LV' + '$SP_FS'}'`

#### Convert IMAGE_DATA_SZ to 512 byte blks (rounded up)
  ADD_ONE=0
  REM=`${expr} $IMAGE_DATA_SZ % $BLK_SZ` && ADD_ONE=1
  IMAGE_DATA_SZ=`${expr} $IMAGE_DATA_SZ \/ $BLK_SZ`
  IMAGE_DATA_SZ=`${expr} $IMAGE_DATA_SZ \+ $ADD_ONE`
  REM=`${expr} $IMAGE_DATA_SZ % $MIN_FILE_BLKS` &&  \
			  REM=`${expr} $MIN_FILE_BLKS \- $REM`
  IMAGE_DATA_SZ=`${expr} $IMAGE_DATA_SZ \+ $REM`

if [ $VG = "rootvg" ]  # Check for space for /image.data file
then
      FREEROOTBLKS=`LC_MESSAGES=C ${df} / | ${tail} +2l | ${awk} '{ if ( length($7) == 1 ) {print $3} }'`
      if [ "$FREEROOTBLKS" -lt "$IMAGE_DATA_SZ" ]
      then
		KBYTES=`${expr} $IMAGE_DATA_SZ \/ 2 ` #Convert to KBytes
		cleanup "$NO_SPACE_EC" \
		"`${dspmsg} -s $MSGSET $MSGCAT 18 \
		'0512-018 %s: The backup could not continue because the %s Kbytes
of free work space on the / filesystem that are necessary
were not found.  Before you reissue this command, you
should extend the / filesystem (chfs) or free up space to provide a
total of %s Kbytes of free work space in the / filesystem.\n\n' $NAME $KBYTES $KBYTES`"
      fi

#     Get space needed for bosboot, required at mksysb and bosinstall time
      BOOT_BLKS=`LC_MESSAGES=C ${bosboot} -qad /dev/ipldevice | ${tail} +3l | ${awk} '{print $2 * 2}'`
      RSTYPE=`${bootinfo}  -T`
      if [ "$NAME" = "mkszfile" -a "$RSTYPE" != "rspc" ]
      then
	    TAPE_BLKS=`LC_MESSAGES=C ${bosboot} -qad /dev/rmt0 | ${tail} +3l | ${awk} '{print $2 * 2}'`
	    [ "$TAPE_BLKS" -gt "$BOOT_BLKS" ] && BOOT_BLKS=$TAPE_BLKS
      fi
      MUSTHAVE_TMP_BLKS=`${expr} $IMAGE_DATA_SZ \+ $BOOT_BLKS `
else
      MUSTHAVE_TMP_BLKS=$IMAGE_DATA_SZ
fi

################################################################
#     ESTIMATE size needed for the MAP files
#
if [ "$MAPS" = "yes" ]      # calculate space needed for the maps
then
    MAP_BLKS=`LC_MESSAGES=C ${lsvg} -l $VG | ${tail} +3l | \
${grep} -v "pmhibernation" | ${awk} 'BEGIN{x=0} {
		    y = $4 * 12
		    if ( y <= '$FOUR_K' )
		       x++
		    else
		    {   x += (y % '$FOUR_K') ?  \
				 (int(y / '$FOUR_K') + 1) : \
				 int(y / '$FOUR_K')
		     }
		   }
		   END{ print (x * '$MIN_FILE_BLKS')}'`
    MUSTHAVE_TMP_BLKS=`${expr} $MUSTHAVE_TMP_BLKS \+ $MAP_BLKS `
fi

##############################################
#
#      ###   CHECK IF ENOUGH FREE SPACE EXISTS IN /tmp    ######
#

if [ $FREETMPBLKS -lt $MUSTHAVE_TMP_BLKS ]
then
     KBYTES=`${expr} $MUSTHAVE_TMP_BLKS \/ 2 ` #Convert to KBytes

     cleanup "$NO_SPACE_EC" \
     "`${dspmsg} -s $MSGSET $MSGCAT 23 \
     '0512-023 %s: The backup could not continue because the %s KBytes
of free work space on the /tmp filesystem that are necessary
were not found.  Before you reissue this command, you
should extend the /tmp filesystem (chfs) or free up space to provide a
total of %s Kbytes of free work space in the /tmp filesystem.\n\n' $NAME $KBYTES $KBYTES`"

fi   ### End of TMP space check

########## We GOT enough  space to continue

${rm} -fr $VGDATA_DIR              ## Clean any old data files
${mkdir} $VGDATA_DIR
[ "$?" != 0 ] && cleanup "$FILE_EC" ""
${mkdir} $VG_DIR
[ "$?" != 0 ] && cleanup "$FILE_EC" ""
[ $VG != "rootvg" ] && ${cp} /etc/filesystems $VG_DIR

###########  Save Estimated Minimum  /tmp sizes

TMP_MIN_FS_SZ=`LC_MESSAGES=C ${df} -IM /dev/$TMP_LV | ${tail} +2l | ${awk} '{print $4 + '$MUSTHAVE_TMP_BLKS'}'`
ROOT_MIN_FS_SZ=`LC_MESSAGES=C ${df} -IM /dev/$ROOT_LV | ${tail} +2l | ${awk} '{print $4 + '$IMAGE_DATA_SZ'}'`

################### Start Processing image.data file    #################
#For each line, print command output in file image.data
{
#####   SYSTEM INFO
	print "image_data:"
	print "\tIMAGE_TYPE= bff"
	print "\tDATE_TIME= "`${date}`
	print "\tUNAME_INFO= "`${uname} -a`
	if [ $VG = "rootvg" ]
	then
	     print "\tPRODUCT_TAPE= no"
	     ## generate list of vgs
	     for vg in `${lsvg} -o | ${grep} -v "$VG"`  #For all active VGs
	     do
		  USERVGS="$USERVGS$vg "   # Concatenate vg to list
	     done
	     print "\tUSERVG_LIST= $USERVGS"
	fi
	print ""
	print "logical_volume_policy:"
	print "\tSHRINK= no"
	print "\tEXACT_FIT= "$MAPS
	print ""
	if [ $VG = "rootvg" ]
	then
	    print "ils_data:"
	    print "\tLANG= "`echo` $LANG
	    print ""
	fi

#####   BEGIN VG DATA               ##############
	# generate list of hdisks
	for hd in `LC_MESSAGES=C ${lsvg} -p $VG | ${tail} +3l | ${awk} '{print $1}'`
	do
	     HDISKLIST="$HDISKLIST$hd "
	done

	print "##Command used for vg_data; ${lsvg}\n" # list command used
	# Get lsvg output in a "language independent" format
	COL2=`LC_MESSAGES=C ${lsvg} $VG |    \
	${awk} 'BEGIN{FS=":"} {print $3}' | ${awk} 'BEGIN{ORS=":"}{print $1}'`
	print "vg_data:"  #Cut & Paste follows for data fields.
	print "\tVGNAME= "$VG
	print "\tPPSIZE= "`echo $COL2 |${awk} 'BEGIN{FS=":"} {print $2}'`
	print "\tVARYON= "`echo $COL2 |${awk} 'BEGIN{FS=":"} {print $9}'`
	print "\tVG_SOURCE_DISK_LIST= $HDISKLIST"
	print ""
######  END VG DATA               #################

#####   START HARD DISK DATA.
	print "##Command used for source_disk_data; ${bootinfo}\n" # list command used

	for h in $HDISKLIST  #hard disk data.
	do
	print "source_disk_data:"   # Cut & Paste follows for fields.
		print "\tLOCATION= "`LC_MESSAGES=C ${bootinfo} -o $h`
		print "\tSIZE_MB= "`LC_MESSAGES=C ${bootinfo} -s $h`
		print "\tHDISKNAME= $h"
		print ""
	done  # with hard disk data.
#####   END HARD DISK DATA.

######  BEGIN LV_DATA              ###############
# only non-jfs LVs and mounted jfs LVs (filesystems)

	LVLIST=`LC_MESSAGES=C ${lsvg} -l $VG | ${tail} +3l |${awk} '{print $1}'`
	MTFSLIST=`LC_MESSAGES=C ${mount} | ${awk} '$1 ~ /^\/dev\// {gsub ("/dev/", "", $1); print $1} '`
	FSLIST=

	print "##Command used for lv_data; ${lslv}\n" # list command used
	for LV in $LVLIST
	do
	    # Get lslv output in a "language independent" format
	    COL1=`LC_MESSAGES=C ${lslv} $LV | ${awk} 'BEGIN{FS=":"} {print $2}' |  \
			     ${awk} 'BEGIN{ORS=":"}{print $1}'`
	    COL2=`LC_MESSAGES=C ${lslv} $LV | ${awk} 'BEGIN{FS=":"} {print $3}' |  \
			     ${awk} 'BEGIN{ORS=":"}{print $1}'`
	    LV_TYP=`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $4}'`
	    if [ "$LV_TYP" = "pmhibernation" ]    # If LV for Power Mgmt, skip.
	    then
		continue
	    fi
	    MNT_PT=`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $11}'`
	    NOFS=0     ## reset NO FileSystem in LV flag.
	    if [ "$LV_TYP" = "jfs" ]    # Is LV a jfs Filesystem ?
	    then
		JNK=`echo $MTFSLIST | ${grep} $LV`   ## LV is type jfs.
		if [ $? -ne 0 ]   ## Is the filesystem mounted?
		then
			    #  LV Not mounted OR has no Filesystem
			    # If LV has a Filesystem then skip it
			    #  otherwise save the LV attributes.
		     echo "q" | ${fsdb} /dev/$LV > /dev/null 2>&1
		     [ "$?" -eq 0 ] && continue
		else
		     if [ "$MNT_PT" = "N/A" ] #Yes, is Fs defined in /etc/filesystems
		     then              ### No, print warning msg
			 echo $LV |  \
			 ${awk} '{system ( "'${dspmsg}' -s '$MSGSET' '$MSGCAT' 20 \"0512-020 %s: The logical volume %s does not have an entry in\n/etc/filesystems which is needed to accurately define the mount point.\nAs a result, it will not be backed up.  If you want it to be included\nin the backup, add a stanza for it in the /etc/filesystems file and\nreissue this command before running mksysb.\n\n\" '$NAME' " $1 " 1>&2" )}'
			 continue
		     else             ### yes, it is mounted
			 FSLIST="$FSLIST $LV"    ## add to FS list
			 NOFS=1     ## set NO FileSystem in LV flag.
		     fi
		fi
	    fi
	    print "lv_data:"
	    print "\tVOLUME_GROUP= $VG"
	    L_S_DISK=""
	    for S_DISK in `LC_MESSAGES=C ${lslv} -l $LV | ${tail} +3l | ${awk} '{print $1}'`
	    do
		    L_S_DISK="$L_S_DISK$S_DISK "
	    done
	    print "\tLV_SOURCE_DISK_LIST= $L_S_DISK"
	    print "\tLV_IDENTIFIER= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $ 2}'`
	    print "\tLOGICAL_VOLUME= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $1}'`
	    print "\tVG_STAT= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $3}'`
	    print "\tTYPE= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $4}'`
	    print "\tMAX_LPS= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $5}'`
	    print "\tCOPIES= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $6}'`
	    NUM_LPs=`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $7}'`
	    print "\tLPs= $NUM_LPs"
	    print "\tSTALE_PPs= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $8}'`
	    print "\tINTER_POLICY= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $9}'`
	    print "\tINTRA_POLICY= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $10}'`
	    [ "$MNT_PT" = "N/A" ] && MNT_PT=""
	    print "\tMOUNT_POINT= $MNT_PT"
	    print "\tMIRROR_WRITE_CONSISTENCY= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $12}'`
	    print "\tLV_SEPARATE_PV= "`echo $COL1 | ${awk} 'BEGIN{FS=":"} {print $13}'`
	    print "\tPERMISSION= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $2}'`
	    print "\tLV_STATE= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $3}'`
	    print "\tWRITE_VERIFY= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $4}'`
	    print "\tPP_SIZE= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $5}'`
	    print "\tSCHED_POLICY= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $6}'`
	    print "\tPP= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $7}'`
	    print "\tBB_POLICY= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $8}'`
	    print "\tRELOCATABLE= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $9}'`
	    print "\tUPPER_BOUND= "`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $10}'`
	    LBL=`echo $COL2 | ${awk} 'BEGIN{FS=":"} {print $11}'`
	    [ "$LBL" = "None" ] && LBL=""
	    print "\tLABEL= $LBL"
	    if [ "$MAPS" = "yes" ]
	    then
		    mk_mapfile $LV
		    print "\tMAPFILE= $VG_DIR/$LV.map"
	    else
		    print "\tMAPFILE="
	    fi
	    if [ "$NOFS" -eq 1 ]  ## Does LV have a filesystem defined.
	    then
		 if [ $LV = $TMP_LV -o $LV = $ROOT_LV ]
		 then
		    if [ $LV = $TMP_LV ]
		    then
			NUMLP=$TMP_MIN_FS_SZ
		    else
			NUMLP=$ROOT_MIN_FS_SZ
		    fi
		  else
		     NUMLP=`LC_MESSAGES=C ${df} -IM /dev/$LV | ${tail} +2l | ${awk} '{print $4}'`
		  fi
		  # LV size in MB
		  REM=`${expr} $NUMLP \% $BLK_SZ2`
		  NUMLP=`${expr} $NUMLP \/ $BLK_SZ2`
		  [ $REM -ne 0 ] && NUMLP=`${expr} $NUMLP \+ 1`
		  # LV size in partitions
		  LV_MIN_LPS=`${expr} $NUMLP \/ $PPSIZE`
		  REM=`${expr} $NUMLP \% $PPSIZE` && \
			 LV_MIN_LPS=`${expr} $LV_MIN_LPS \+ 1`

		  print "\tLV_MIN_LPS= $LV_MIN_LPS"
	     else
		  print "\tLV_MIN_LPS= $NUM_LPs"
	     fi
	    print ""
	done # With lv_data
#######    END LV DATA          #################

#######    START FS DATA         #################

	print "##Commands used for fs_data; ${df} and ${lsjfs}\n" # list command
	for fs in $FSLIST
	do
	    print "fs_data:"
	    set -- `LC_MESSAGES=C ${df} -IM /dev/$fs | ${tail} +2l`
	    print "\tFS_NAME= $2"
	    FS_SIZE=$3
	    FS_MIN_SIZE=$4
	    if [ $fs = $TMP_LV -o $fs = $ROOT_LV ]
	    then
	       if [ $fs = $TMP_LV ]
	       then
		    FS_MIN_SIZE=$TMP_MIN_FS_SZ
	       else
		    FS_MIN_SIZE=$ROOT_MIN_FS_SZ
	       fi
	    fi
	    print "\tFS_SIZE= $FS_SIZE"
	    print "\tFS_MIN_SIZE= $FS_MIN_SIZE"
	    print "\tFS_LV= $1"
	    FSEXT=`LC_MESSAGES=C ${lsjfs} /dev/$fs |${tail} +2l`
	    print "\tFS_FS= "`echo $FSEXT | ${awk} 'BEGIN{FS=":"} {print $13}'`
	    print "\tFS_NBPI= "`echo $FSEXT | ${awk} 'BEGIN{FS=":"} {print $14}'`
	    print "\tFS_COMPRESS= "`echo $FSEXT | ${awk} 'BEGIN{FS=":"} {print $15}'`
	    print ""
	done # With fs data
#####    END FS DATA           #################

	if [ $VG = "rootvg" ]
	then
	    print "post_install_data:"
	    print "\tBOSINST_FILE= "
	else
	    print "post_restvg:"
	    print "\tRESTVG_FILE= "
	fi
}  > $VG_DATA
[ "$?" != 0 ] && cleanup "$FILE_EC" ""

### reset trap and exit
trap '' 1 2 15
exit 0





