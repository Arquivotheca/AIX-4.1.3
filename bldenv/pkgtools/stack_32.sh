#!/bin/ksh
# @(#)39        1.23  src/bldenv/pkgtools/stack_32.sh, pkgtools, bos412, GOLDA411a 4/4/94 16:24:18
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS:	abort
#		check_size
#		cktape_avail
#		get_type
#		leadvol
#		syntax
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#===============================================================================
#
#   Purpose:
#         This shell creates output stacked tapes. The output
#         tapes contain a newly created Table of Content (TOC)
#         file. The output tape also contains the product image
#         files ( in backup or tar format) that are input
#         via the -l and -c flags.
#
#===============================================================================

leadvol() {
  if [ $volume -lt 10 ]
  then
    volume="0$volume"
  fi
}

#########################  Function ########################
# The 'syntax' function simply echos command syntax, then
# exits the shell.
###########################################################
syntax() {
  echo "$cmd  -l<stack_list> [-c<image_dir>] [-d<device>] [-s<size>] "
  echo "\t[-i<image_type>] [-n<toc_dir>] [-1<boot_image>] [-2<inst_image>]"
  echo "FLAGS:  -l  File name of stack list.  Only one bff/tar file name"
  echo "              or pathname per line."
  echo "        -c  Default directory name containing installable images."
  echo "        -d  Device type for tape device. (default: rmt0)"
  echo "        -s  Maximum size in megs for tape volume."
  echo "            Defaults are: 120 for 1/4 inch tape," 
  echo "                          525 for 7207 tape,"
  echo "                          1000 for 4mm2gb tape,"
  echo "                          2000 for 4mm4gb tape,"
  echo "                          1150 for 8mm tape, and"
  echo "                          2100 for 8mm5gb tape."
  echo "        -n  Name of the directory where you want the TOC to be written."
  echo "              This option will generate a TOC, but not write a tape."
  echo "        -1  Bosboot image. Need to specify directory and"
  echo "              filename of bosboot image. This file will be the first"
  echo "              file of the output tape.  There is no default."

  echo "\nHit any key to continue page:  \b"
  read junk

  echo "\n        -2  Bos install image. Need to specify directory and"
  echo "              filename of bos install image. This file will be the"
  echo "              second file of the output tape. There is no default. "
  echo "        -i  Specifies image type by AIX release for TOC format.  Valid"
  echo "              values are 3.1, 3.2 and 4.1 which indicate type 1,"
  echo "              2, and 3 TOC format, respectively.  Default is type 1."
  echo "\nNotes:  File and Directory names can be absolute or relative"
  echo "        -1 and -2 flag must be used together."
  echo "\nExamples:"
  echo "        stack_32 -l stack_list -c ./ -d rmt1 -s 1000 -n ./"
  echo "        stack_32 -l stack_list -c files -1 boot -2 install\n"
  exit $error_code
}

abort() {
  rm -rf ${WORK} > /dev/null 2>&1
  echo ""
  exit 1
}

cktape_avail() {
  device=`ODMDIR=/etc/objrepos \
	lsdev -C -c tape -l $dev -t $TYPE -S a` > /dev/null 2>&1
  if [ "x$device" = "x" ]; then
    echo "\n$cmd: ERROR: Tape device /dev/$dev defined but not available"
    exit 25
  fi
}

#------------------------------------------------------------------------
# Determine the tape device type.  Set the maximum size value for	|
# that device.  Use 2100000000 for the 5gb tape drive limit so the	|
# shell calculations wil work in Maketoc_32.				|
# Since all tapes are written in block sizes of 512, this actually      |
# reduces the max size to half of the actual tape capacity.  Thus       |
# the max size value for a 2gb tape is 1000meg, etc.  Note that 1/4     |
# inch tapes (types 150mb, 525mb, and ost) are blocked at 512, so       |
# maxsize in this case does reflect the true tape capacity.             |
#------------------------------------------------------------------------
get_type ()
{
	TYPE=`ODMDIR=/etc/objrepos lsdev -C -F type -c tape -l $dev`
	case $TYPE in
            "8mm5gb" ) check_size 2100000000;;
	    "8mm"    ) check_size 1150000000;;
	    "4mm2gb" ) check_size 1000000000;;
	    "4mm4gb" ) check_size 2000000000;;
	    "150mb"  ) check_size 120000000;;
	    "525mb"  ) check_size 525000000;;
	    "ost"    ) check_size 525000000;;
	    *	     )
	    #------------------------------------
	    # Not a recognized device type.	|
	    #------------------------------------

	    echo "\n$cmd:  ERROR:  Device type not recognized.  Valid device\n"
	    echo "\ttypes are 8mm5gb, 8mm, 4mm4gb, 4mm2gb, 150mb, 525mb and ost.\n"
	    exit 24;;
	esac
	return 0
}

#------------------------------------------------------------------------
# If -s flag set make sure it is not higher than maximum media limit.	|
# Otherwise set maxsize to media limit.					|
#------------------------------------------------------------------------

check_size ()
{
	[ -z "$sizeFlag" ] && { maxsize=$1; return; }

	if [ $maxsize -gt $1 ]
	then
	    echo "\n$cmd:  ERROR:  The value specified with the -s option\n"
	    echo "\tmust be less than or equal to the maximum size of $1\n"
	    echo "\tfor a type $TYPE tape.\n\n\t$cmd Terminating."
	    exit 18
	fi
}

###########################################################
# START OF SHELL LOGIC.
# First, check number of parameters on command line.
###########################################################
set -u

error_code=0
cmd=`basename $0`
if [ $# -lt 1 ]; then
  error_code=1
  syntax
fi
cmdline="$*"

############################################################
# Parse command line parameters
############################################################
set -- `getopt "bl:c:d:s:n:i:1:2:" $*`
if [ $? != 0 ]; then
  echo "\n$cmd: ERROR: Problem in command line: $cmdline"
  error_code=2
  syntax
fi

##############################
# Initialize variables
##############################
TOC_DATE=`date +"%m""%d""%H""%M""%S""%y"`       # Constant date stamp
WORK_DATE=`date +"%y""%m""%d""%H""%M""%S"`      # Work directory date stamp
image_dir=`pwd`         # Installable image directory
CURRENT_DIR=`pwd`       # Current directory
STACK_LIST=""           # Stack list file name
bootable=no             # Bootable is no
gotsize=no              # Defaults to program specified maximum tape size
toc_format=""
################################
writetape=yes           # Defaults to write to tape
dev=rmt0                # Default device
DEV=/dev/rmt0           # Set device driver 
sizeFlag=""		# -s flag set
TYPE=""                 # Tape device type (either 1/4 inch, 4mm or 8mm)
b1=no                   # -1 specified flag
b2=no                   # -2 specified flag
BOSBOOT=""              # initialize filename to null
BOSINSTAL=""            # initialize filename to null
backup_pattern="0000000  004400"        # Pattern that distinguishes between
                        # bff and tar files
BLOCK_SIZE=31744        # Streaming tape block size
export BLOCK_SIZE backup_pattern

while [ "$1" != "--" ]; do
  case $1 in
    "-d")
       shift
       dev=`basename $1`
       DEV=/dev/$dev
       ;;
    "-n")
       shift
       writetape=no
       TOC_DIR=$1
       if [ ! -d $TOC_DIR ]; then
         echo "\n$cmd: ERROR: Can't find TOC directory $TOC_DIR"
         error_code=3
       else
         cd $TOC_DIR
         TOC_DIR=`pwd`
         if [ ! -w $TOC_DIR ]; then
           echo "\n$cmd: ERROR: TOC directory: $TOC_DIR not writable"
           error_code=4
         fi
       fi
       cd $CURRENT_DIR
       ;;
    "-b")
       echo "NOTICE: stack_32 no longer uses the '-b' flag.  To leave bos"
       echo "off of the tape, take bos and/or bos.obj out of your stacklist."
       syntax
       ;;
    "-s")
       # Maxsize of 2300 was causing errors because of the integer size
       # in Maketoc_32, so the maxsize was changed to 2100
       # the breaking point seems to be 2147.
       shift
       if [ $1 -gt 0 -a $1 -lt 2100 ]; then
	 sizeFlag=yes
	 maxsize="${1}000000"
       else
         echo "\n$cmd: ERROR: Maximum tape size should be less than 2100 megs."
         echo "       Defaults are 120 for 1/4 inch tape, 525 for 7207 tape,"
         echo "       2000 for 4mm4gb tape, 1000 for 4mm2gb tape, 1150 for 8mm tapes,"
         echo "       and 2100 for 8mm5gb tape." 
         error_code=5
       fi
       ;;
    "-l")

       shift
       # dirname gives all except the last part of the path name
       # that is all but the last /
       # set STACK_LIST to full path directory and filename using dirname cmd
       stk_dir=`dirname $1`
       if [ ! -d $stk_dir ]; then
         echo "\n$cmd: ERROR: Can't find stack file directory $stk_dir"
         error_code=6
       else
         cd `dirname $1`
         stk_dir=`pwd`
         stk_name=`basename $1`
         STACK_LIST=$stk_dir/$stk_name
         if [ ! -f $STACK_LIST ]; then
           echo "\n$cmd: ERROR: Can't find stack list $STACK_LIST"
           error_code=7
         else
           if [ ! -r $STACK_LIST ]; then
             echo "\n$cmd: ERROR: Can't read stack list $STACK_LIST"
             error_code=7
           fi
         fi
       fi
       cd $CURRENT_DIR
       ;;
    "-c")
       # Set image directory to full path directory
       shift
       image_dir=$1
       if [ ! -d $image_dir ]; then
         echo "\n$cmd: ERROR: Can't find installable image directory $image_dir"
         error_code=8
       else
         cd $image_dir
         image_dir=`pwd`
       fi
       cd $CURRENT_DIR
       ;;

    "-i")
	shift
	[ "$1" = "4.1" ] && toc_format=3
	[ "$1" = "3.2" ] && toc_format=2
	[ "$1" = "3.1" ] && toc_format=1
	if [ -z "$toc_format" ] && [ "$cmd" != "stack41" ]
	then
	   echo "\n$cmd:  WARNING:  invalid value specified for -i option."
	   echo "\tDefaulting to type 1 (3.1) toc.  Processing will continue."
	   toc_format=1
	fi
	;;

    "-1")
       # set BOSBOOT parameter.
       # Will later be resolved relative to image_dir (-c)
       # also set the b1 flag which will be used later to make sure
       # that flag 1 and 2 were used together
       shift
       BOSBOOT="$1"
       b1=yes
       ;;
    "-2")
       # set BOSINSTAL parameter
       # Will later be resolved relative to image_dir (-c)
       # also set the b2 flag which will be used later to make sure
       # that flag 1 and 2 were used together
       shift
       BOSINSTAL="$1"
       b2=yes
       ;;
  esac
  shift
done

# shift past the --
shift

# Additional arguments on the command line
if [ $# != 0 ]; then
  echo "\n$cmd: ERROR: Additional arguments on command line: $*"
  error_code=13
fi

# Default to type 1 toc format if -i not specified on command line.
[ -z "$toc_format" ] && toc_format=1

#
# check if absolute path specified on bosboot entry
#
if [ x != x$BOSBOOT ]; then
   echo $BOSBOOT | grep '^/' > /dev/null
   if [ $? -ne 0 ]; then
       #
       # add default path for relative entries
       #
       BOSBOOT="$image_dir/$BOSBOOT"
   fi

   if [  ! -f $BOSBOOT -o ! -r $BOSBOOT ]; then
     echo "\n$cmd: ERROR: Can't read bosboot image $BOSBOOT"
     echo "                   Check image directory and/or bosboot file"
     error_code=10
   fi
fi

#
# check if absolute path specified on bosinstall entry
#
if [ x != x$BOSINSTAL ]; then
    echo $BOSINSTAL | grep '^/' > /dev/null
    if [ $? -ne 0 ]; then
        #
        # add default path for relative entries
        #
        BOSINSTAL="$image_dir/$BOSINSTAL"
    fi

    if [ ! -f $BOSINSTAL -o ! -r $BOSINSTAL ]; then
      echo "\n$cmd: ERROR: Can't read bosinstall image $BOSINSTAL"
      echo "                   Check image directory and/or bosinstall file"
      error_code=11
    fi
fi

# Make sure flag -1 and -2 were used together
if [ "$b1" = "yes" -a "$b2" = "yes" ]; then
  # set flags that specify that the tape should be bootable
  # that is that files will be written to file 1 and 2 of the output tape
  bootable=yes
  export BOSBOOT
  export BOSINSTAL
else
  if [ "$b1" = "yes" -o "$b2" = "yes" ]; then
    echo "\n$cmd: ERROR: FLAG -1 and -2 must be used together"
    error_code=14
  fi
fi

# Determine if device name and type are found
# Here is a sample output of lsdev -C -c tape:
# rmt0 Available 00-08-60-00 2.3 8mm tape drive
device=`ODMDIR=/etc/objrepos \
	lsdev -C -c tape -l $dev` > /dev/null 2>&1
if [ "x$device" = "x" ]; then
  echo "\n$cmd: ERROR: Invalid tape device: $dev"
  error_code=15
else
    get_type
    cktape_avail

  # Check the block size of device.  It must be 512.
  tapebs=`ODMDIR=/etc/objrepos \
	lsattr -E -a block_size -l $dev | cut -d' ' -f2`
  if [ "$tapebs" != "512" ]; then
      echo "\n$cmd: ERROR: Tape block size on tape device must be set to 512"
      echo "       Use 'smit tape' to set block size to 512 or use"
      echo "       chdev -l $dev -a block_size=512"
      error_code=21
  fi
fi

# Image directory was set by using the -c flag
if [ "x$image_dir" = "x" ]; then
  echo "\n$cmd: ERROR: Must have directory containing the installable images"
  echo "       Use the -c flag to enter directory"
  error_code=22
fi

# STACK_LIST was set by using the -l flag
if [ "x$STACK_LIST" = "x" ]; then
  echo "\n$cmd: ERROR: Must have the stack list file"
  echo "       Use the the -l flag to specify stack list file"
  error_code=23
fi

#---------------------------------------------------------------------------
# If we were invoked as stack41 then always use a 4.1 (type 3) toc format. |
#---------------------------------------------------------------------------
[ "$cmd" = "stack41" ] && toc_format=3

# Check for errors in the command line
if [ $error_code != 0 ]; then
  syntax
fi

if [ "$TYPE" = "525mb" -o "$TYPE" = "ost" ]; then
   DEV="$DEV".1
else
   DEV="$DEV".5
fi
echo "\nMaking a stacked tape of the following description:\n"
[ "$TYPE" = "150mb" ] && echo "  TAPE TYPE: ......... 1/4 inch"
[ "$TYPE" = "8mm5gb" ] && echo "  TAPE TYPE: ......... 8mm5gb"
[ "$TYPE" = "8mm" ] && echo "  TAPE TYPE: ......... 8mm"
[ "$TYPE" = "4mm4gb" ] && echo "  TAPE TYPE: ......... 4mm4gb"
[ "$TYPE" = "4mm2gb" ] && echo "  TAPE TYPE: ......... 4mm2gb"
[ "$TYPE" = "525mb" ] && echo "  TAPE TYPE: ......... 525 mb tape"
[ "$TYPE" = "ost" ] && echo "  TAPE TYPE: ........  7207 tape"

# [ "$toc_format" = "3" ] && echo "  MEDIA FORMAT: ...... AIX v4.1"
# [ "$toc_format" = "2" ] && echo "  MEDIA FORMAT: ...... AIX v3.2"
if [ "$toc_format" = "1" ]
then
  [ "$toc_format" = "1" ] && echo "  MEDIA FORMAT: ...... AIX v3.1"
else
  echo "  Defaulting to MEDIA FORMAT: ...... AIX v3.1"
  echo "\t\t\tuntil Boulder supports additional formats."
  toc_format=1
fi

echo "  TAPE DEVICE: ....... /dev/$dev"
echo "  TAPE SIZE: ......... $maxsize"
echo "  BOOTABLE: .......... $bootable"
echo "  DEFAULT IMAGE DIRECTORY:  $image_dir"
echo "  STACK LIST FILE: ... $STACK_LIST"

# Use a directory name variable for the tmp work directory
WORK=/tmp/stack_32_work.$$.$WORK_DATE
rm -f $WORK
if [ -d $WORK ]; then
  echo "\n$cmd: ERROR: The temporary work directory $WORK could not be removed"
  abort
fi

# Create the work directory
mkdir $WORK
if [ $? != 0 ]; then
  echo "\n$cmd: ERROR: The temporary work directory $WORK could not be created"
  abort
fi

# chmod added to make sure that the directory can be overwritten in the future
chmod 777 $WORK

VOLUME_TOC=$WORK/toc
WORK_TOC=$WORK/work_toc
WORK_STACK_LIST=$WORK/stack_list

# Prompt for a tape mount.
if [ "$writetape" = "yes" ]; then
  echo "\07\nMount a tape in /dev/$dev and press enter"
  read gorp
  touch $WORK/tmpfile
  dd if=$WORK/tmpfile of=$DEV 2>/dev/null
  while [ $? -ne 0 ]; do
    echo "Could not write to device /dev/$dev."
    echo "\07\nCheck that a writable tape is mounted and press enter"
    read gorp
    dd if=$WORK/tmpfile of=$DEV 2>/dev/null
  done
  rm $WORK/tmpfile >/dev/null 2>&1
  tctl -f$DEV rewind > /dev/null 2>&1&
  rwd=$!
fi

# Build the WORK_TOC
echo "\nBuilding Table-of-Contents file:\n"
Maketoc_32 $image_dir $STACK_LIST $WORK_TOC $CURRENT_DIR $bootable $maxsize $WORK_STACK_LIST $toc_format
# Check return code from Maketoc_32
if [ $? != 0 ]; then
  abort
fi

cd $image_dir
volume=1
leadvol
# WORK_STACK_LIST.volume contains a copy of the stack list created by Maketoc_32
while [ -f ${WORK_STACK_LIST}.${volume} ]; do
  #########################################
  ## Format of TOC specifies that first line contain volume # in
  ## first character, the date created and a # for version of toc header.
  ## TOC for all volumes is same but for 1st line.
  #########################################

  # Put the header information into the new toc
  echo "$volume $TOC_DATE $toc_format" > $VOLUME_TOC
  cat $WORK_TOC >> $VOLUME_TOC

  # chmod added to make sure that the toc can be overwritten in the future
  chmod 777 $VOLUME_TOC

  if [ "$writetape" = "yes" ]; then
    stuff=`ps -p$rwd | tail -1 | cut -f4 -d" "`
    if [ "x$stuff" != "xPID" ]
    then
      echo "Wait for rewind to complete..."
      wait
    else
      echo "Rewind has completed..."
    fi
    # Add the bootable files, or place-holders
    if [ "$bootable" = "yes" -a $volume -eq 1 ]; then
      for x in $BOSBOOT $BOSINSTAL; do
        echo "\n`basename $x`"
        dd if=$x of=$DEV bs=$BLOCK_SIZE conv=sync
        if [ $? != 0 ]; then
          echo "\n$cmd: ERROR: Problem writing boot files to tape using dd"
          abort
        fi
      done
    else
      # Must have placeholders for instupdt to process correctly.
      for x in Reserved_file_1 Reserved_file_2; do
        echo "\n$x"
        echo "This is a reserved file" | dd of=$DEV bs=$BLOCK_SIZE conv=sync
        if [ $? != 0 ]; then
          echo "\n$cmd: ERROR: Problem writing reserved files to tape using dd"
          abort
        fi
      done
    fi

    # Add the table of contents
    echo "\nTable of Contents"
    dd if=$VOLUME_TOC of=$DEV bs=512 conv=sync
    if [ $? != 0 ]; then
      echo "\n$cmd: ERROR: Problem writing TOC file to tape using dd"
      abort
    fi

    # Get filename for each bff file in stack list
    for fil in `cat ${WORK_STACK_LIST}.$volume`; do
      echo "\n`basename $fil`"
      dd if=$fil of=$DEV bs=$BLOCK_SIZE conv=sync
      if [ $? != 0 ]; then
        echo "\n$cmd: ERROR: Problem writing $fil file to tape using dd"
        abort
      fi
    done

    # After last file on tape, write an extra tape mark.
    tctl -f$DEV weof
    # Then rewind the tape.
    tctl -f$DEV rewind
  fi

  # Increment the number of volumes
  volume=`expr $volume + 1`
  leadvol

  # Get the next stack_list
  if [ -f ${WORK_STACK_LIST}.$volume ]; then
    if [ "$writetape" = "yes" ]; then
      echo "\07\nMount volume $volume in $DEV and press enter"
      read gorp
      tctl -f$DEV rewind
    fi
  fi
done

# Graceful exit
if [ "$writetape" = "yes" ]; then
  echo "\n$cmd SUCCESSFUL!  Please remove tape from drive: /dev/$dev"
  echo "\nTo get a copy of the table of contents you can use the command:"
  echo "     dd if=/dev/$dev of=table_of_contents fskip=2"
else
  echo "\n$cmd SUCCESSFUL!  NO Tape was written"
  cp $VOLUME_TOC $TOC_DIR/toc.$WORK_DATE > /dev/null 2>&1
  if [ $? != 0 ]; then
    echo "\n$cmd: ERROR: Unable to write the toc file to $TOC_DIR"
    abort
  fi
  VOLUME_NAME=`basename $VOLUME_TOC`
  echo "\nNOTE: TOC file: $VOLUME_NAME.$WORK_DATE is now in $TOC_DIR"
fi
echo ""
cd $CURRENT_DIR
rm -rf ${WORK} > /dev/null 2>&1
exit 0
