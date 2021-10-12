#!/bin/sh
  SCCSID="@(#)20  1.5  src/bldenv/pkgtools/rename/disstk.sh, pkgtools, bos412, GOLDA411a 6/24/93 08:38:21"
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: abort
#		func1
#		quiet_abort
#		syntax
#		track
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
#========================================================================
#
#   Purpose:
#         This shell creates output stacked diskettes. The output
#         diskettes contain a newly created Table of Content (TOC)
#         file. The output diskette also contain the product image
#         files ( in backup or tar format) that were inputted
#         via the -l and -c flags.
#
#   Diskette Format:
#     1. Each diskette must have a header file beginning in block 1. The format
#        of the header is :
#        bytes 0-3 binary integer with value of 3609823512= x'D7298918'
#        bytes 4-15 date and time
#        bytes 16-19 binary integer volume number (1st diskett is x'00000001')
#        bytes 20-511 binary zeros
#     2. The TOC file will begin in block 2. The TOC file has a TOC header
#        in the form of Volume Date Format_Of_code.  (for 3.2 Format_Of_Code=2).
#        The TOC also has an entry for each product image which specifies
#        the file position in the diskette. The file position data is formatted
#        volume:beginning block:size of file.
#     3. Following the TOC each of the product image files are written to
#        successive diskette blocks.
#
#   Syntax:
#       disstk  <-l> <-c> [-d] [-n]
#
#   Flags:  Ref. usage message
#
#   Known limitations:
#	All files to be written to diskettes are first written to one huge file.
#	Depending on the amount of /tmp space that is allocated, the program may
#	run out of space.  It should be noted that this algorithm was used to
#	speed up the diskette production process.  Writing files to diskette
#	using a block size of 512 took approximately 12-15 minutes.  The current
#	version takes approximately 1-2 minutes.
#
#   Change history:
#	5/14/91 created from v3.1 utility
#       6/10/91 Modified to use better temp file scheme    --Defect 19882
#	7/02/91 Use a constant date/time stamp
#	7/02/91 Performance change: increase size of block size on dd
#	7/10/91 Modified to delete files when program bombs
#	7/11/91 Modified /tmp/toc.x algorithm
#	7/12/91 Modified /tmp/stkwork.x algorithm
#	7/31/91 Removed log file & replaced it with the WORK directory
#	7/31/91 Removed the toc file from /tmp
#	7/31/91 Added checks to the end of while-read loops for exiting
#	7/31/91 Changed the WORK directory name to include a date stamp
#	7/31/91 Added $cmd to ERROR messages
#	8/01/91 Changed to one exit point for errors on command line
#	9/13/91 Added directory parameter to -n option
#
#========================================================================
#########################  Function ########################
# The 'syntax' function simply echos command syntax, then
# exits the shell.
###########################################################
syntax() {
  echo "\n$SCCSID"
  echo "\n$cmd  <-l> <-c> [-d] [-n]"
  echo "FLAGS: -l  Name of stack list that contains bff or tar file names to be"
  echo "           written to diskette."
  echo "       -c  Directory name containing installable images."
  echo "       -d  Device name for diskett device (default is fd0)."
  echo "       -n  Name of the directory where you want the TOC to be written."
  echo "             This option will generate a TOC, but not write a diskette."
  echo "Note:  File and directory names can be absolute or relative.\n"

  # Remove temporary files and directories
  rm -rf $WORK > /dev/null 2>&1
  exit $error_code
}

abort() {
  # remove temporary files and directories
  rm -rf $WORK > /dev/null 2>&1
  echo ""
  exit 1
}

quiet_abort() {
  exit 1
}

########################
# START OF SHELL LOGIC #
########################
set -u

########################
# Initialize variables #
########################
IMAGE_DIR=""		# Image directory
STACK_LIST=""		# Stack list
toc_format=2		# AIX V3.2 media format
writetape=yes		# Default write to media
dev=fd0			# Default device name
DEV=/dev/fd0		# Full path of device
BLKSZ=512		# Block size for diskette media
count_max=2879		# Maximum number of 512 blocks in a 2.0 MB diskette
block_max=2880		# Maximum block size
TOC_DATE=`date +"%m""%d""%H""%M""%S""%y"`	# TOC constant date stamp
WORK_DATE=`date +"%y""%m""%d""%H""%M""%S"`	# WORK date stamp
CURRENT_DIR=`pwd`	# Current directory

# Use a directory name variable for the tmp work directory
WORK=/tmp/disstk_work.$$.$WORK_DATE
rm -rf $WORK > /dev/null 2>&1
if [ -d $WORK ]; then
  echo "\n$cmd: ERROR: The temporary work directory $WORK could not be removed"
  abort
fi

# Create the work directory
mkdir $WORK >/dev/null 2>&1
if [ $? != 0 ]; then
  echo "\n$cmd: ERROR: The temporary work directory $WORK could not be created"
  abort
fi

# chmod added to make sure that the directory can be overwritten in the future
chmod 777 $WORK

# Temporary header file
HDR=${WORK}/hdr

# Temporary file that contains the toc and all the bff and/or tar file data
DISK=${WORK}/disk

# Temporary toc file does not contain toc header record
WORK_TOC=${WORK}/toc

# Contains the list of files enter by user with the -l flag
STK=${WORK}/stack.list

# SIZEFILE has data needed to write to diskette
SIZEFILE=$WORK/sizefile

# Has last diskette volume number
LAST=$WORK/last

# Use a file name variable for toc
VOLUME_TOC=$WORK/volume_toc
export VOLUME_TOC                             # Do this for func1() in distoc.

####################
# Start processing #
####################
# Check number of parameters on command line.
error_code=0
cmd=`basename $0`
if [ $# -lt 1 ]; then
  error_code=1
  syntax
fi
cmdline="$*"

# Parse command line parameters
set -- `getopt "l:c:d:n:" $*`
if [ $? -ne 0 ]; then
  echo "\n$cmd: ERROR: in command line: $cmdline"
  error_code=2
  syntax
fi

# Parse the command line
while [ "$1" != "--" ]; do
  case $1 in
    "-d")
       shift
       dev="$1"
       # The lsdev command displays information about devices
       # So search for the device name the user inputted
       # If device name is found then make sure that the device
       # type is a diskette
       # If device name and type are found search to make sure
       # diskette is available
       # Set DEV to /dev/diskette.name
       # Here is a sample output of lsdev -C -c diskette:
       #    fd0 Available 00-00-0D-00 Diskette Drive
       lsdev -C -c diskette | grep $dev  > /dev/null
       if [ $? -ne 0 ]; then
         echo "\n$cmd: ERROR: Wrong diskette device name $dev"
         error_code=3
       fi
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
    "-l")
       shift
       # Set STACK_LIST to full path directory and filename using dirname cmd
       stk_dir=`dirname $1`
       if [ ! -d $stk_dir ]; then
         echo "\n$cmd: ERROR: Can't find stack file directory $stk_dir"
         STACK_LIST='x'		# Added to remove STACK_LIST error message
         error_code=4
       else
         cd `dirname $1`
         stk_dir=`pwd`
         stk_name=`basename $1`
         STACK_LIST=$stk_dir/$stk_name
         if [ ! -f $STACK_LIST ]; then
           echo "\n$cmd: ERROR: Can't find stack list $STACK_LIST"
           error_code=5
         fi
       fi
       cd $CURRENT_DIR
       ;;
    "-c")
       # set IMAGE_DIR to full path directory
       shift
       IMAGE_DIR=$1
       if [ ! -d $IMAGE_DIR ]; then
         echo "\n$cmd: ERROR: Can't find installable image directory $IMAGE_DIR"
         error_code=6
       else
         cd $IMAGE_DIR
         IMAGE_DIR=`pwd`
       fi
       cd $CURRENT_DIR
       ;;
  esac
  shift
done

# Shift past the last option (--)
shift

# If no args were specified after the flags, exit
if [ $# -ne 0 ]; then
  echo "\n$cmd: ERROR in command line: $cmdline"
  error_code=7
fi

# IMAGE_DIR was not set by using the -c flag
if [ "x$IMAGE_DIR" = "x" ]; then
  echo "\n$cmd: ERROR: Must have directory containing the installable images"
  echo "               Use the -c flag to enter directory"
  error_code=8
fi

# STACK_LIST was not set by using the -l flag
if [ "x$STACK_LIST" = "x" ]; then
  echo "\n$cmd: ERROR: Must have the stack list file"
  echo "               Use the the -l flag to specify stack list file"
  error_code=9
fi

# Check for errors in the command line
if [ $error_code != 0 ]; then
  syntax
fi

echo "\nMaking a stacked diskette of the following description:\n"
echo "  DISKETTE DEVICE: ... $DEV"
echo "  IMAGE DIRECTORY: ... $IMAGE_DIR"
echo "  STACK LIST FILE: ... $STACK_LIST"

# Call distoc who will build the TOC
echo "\nBuilding Table-of-Contents file\n"
distoc $IMAGE_DIR $STACK_LIST $WORK_TOC $WORK $WORK/stack.list
#Check return code from distoc and exit if distoc did not return a zero
if [ $? -ne 0 ]; then
  abort
fi

# Change to the image directory
cd $IMAGE_DIR

########################################################################
# Format of TOC specifies that first line contain volume # in          #
# first character, the date created and a # for version of toc header. #
########################################################################
volume=1
echo "$volume $TOC_DATE $toc_format" > $VOLUME_TOC
if [ $? != 0 ]; then
  echo "\n$cmd: ERROR: Unable to create table of contents $VOLUME_TOC"
  abort
fi
cat $WORK_TOC >> $VOLUME_TOC

if [ "$writetape"  = "yes" ]; then
  skcount=0
  echo "\nWriting image files to temporary blocked file"
  # Read filename and 512 block count size from the SIZEFILE
  # This data was computed in distoc by the func1 routine
  cat $SIZEFILE | while read fil blkcnt junk; do
    # Write the toc and all the bff and or tar images to one huge file using dd
    # dd command options used:
    # if=input file of=output file count=copy only this number of records
    # seek=seek to the nth number of records before writing bs=block size
    # conv=sync pad every input record to specified block size
    echo "\n`basename $fil`"
    dd if=$fil of=$DISK count=$blkcnt seek=$skcount bs=$BLKSZ conv=sync
    if [ $? != 0 ]; then
      echo "\n$cmd: ERROR: dd could not write $fil to temporary work space"
      abort
    fi
    # increment the seek count by the 512 block count size
    skcount=`expr $skcount + $blkcnt`
  done
  if [ $? != 0 ]; then
    quiet_abort
  fi
fi

skipcnt=0
if [ "$writetape"  = "yes" ]; then
  # awk gets the first field from record
  # That number is the largest disk volume number
  last_volume_num=`tail -1 $LAST | awk '{print$1}'`

  # Tell the user how many diskettes he will need
  echo "\nYou will need $last_volume_num diskettes for this stack set\n"
  echo "WRITING to diskette takes about 1 to 2 MINUTES each\n"

  # Process each diskette
  while [ $volume -le $last_volume_num ]; do
    # Each diskette must have a header file with this format:
    # Bytes 0-3 binary integer with value of 3609823512= x'D7298918'
    # Bytes 4-15 date and time
    # Bytes 16-19 binary integer volume number (1st diskett is x'00000001')
    # Bytes 20-511 binary zeros
    # Build diskett header file
    # This echo with a leading zero for each octal number will echo the required
    # Binary integer value
    echo  "\0327\051\0211\030\c" > $HDR
    echo "$TOC_DATE\c" >> $HDR
    # This will generate three leading binary zeros for the volume number
    echo "\00\00\00\c" >>$HDR
    # This stmt will generate a binary integer for the value of volume
    oct_volume=`echo "$volume" | awk '{printf "%o", $1}'`
    echo "\0$oct_volume\c" >>$HDR
    # Write header record using dd by using count and conv=sync byte 20-511
    # will have binary zeros
    echo "\nWriting header file onto temporary file"
    dd if=$HDR of=$WORK/temp_file count=1 bs=$BLKSZ conv=sync > /dev/null
    if [ $? -ne 0 ]; then
      echo "\n$cmd: ERROR: dd could not write $WORK/temp_file"
      abort
    fi
    # Write from huge temporary file to diskette size temporary file
    # seek=1 skips over the header record
    echo "\nWriting from huge file to diskette size file"
    dd if=$DISK of=$WORK/temp_file count=$count_max seek=1 bs=$BLKSZ skip=$skipcnt conv=sync > /dev/null
    if [ $? -ne 0 ]; then
      echo "\n$cmd: ERROR: dd could not write $WORK/temp_file"
      abort
    fi

    # Prompt for a diskette mount.
    echo "\07\nMount diskette volume $volume in $DEV and press enter"
    read answer

    # dd diskette sized file out to a diskette
    # bs equals one entire track (improves original performance)
    echo "\nWriting diskette size file to diskette"
    dd if=$WORK/temp_file of=$DEV bs=18432
    if [ $? -ne 0 ]; then
      echo "\n$cmd: ERROR: dd could not write $WORK/temp_file to diskette"
      abort
    fi
    volume=`expr $volume + 1`
    skipcnt=`expr $skipcnt + $count_max`
  done
fi


if [ "$writetape" = "yes" ]; then
  echo "\n$cmd SUCCESSFUL!  Please remove diskette from drive: /dev/$dev\n"
else
  echo "\n$cmd SUCCESSFUL!  NO diskette was written"
  cp $VOLUME_TOC $TOC_DIR/toc.$WORK_DATE > /dev/null 2>&1
  if [ $? != 0 ]; then
    echo "\n$cmd: ERROR: Unable to write the toc file to $TOC_DIR"
    abort
  fi
  VOLUME_NAME=`basename $VOLUME_TOC`
  echo "\nNOTE: TOC file: $VOLUME_NAME.$WORK_DATE is now in $TOC_DIR\n"
fi

# remove temporary files and directories
rm -rf ${WORK} > /dev/null 2>&1

exit 0
# END OF SHELL
