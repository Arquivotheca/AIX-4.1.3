#!/bin/sh
  SCCSID="@(#)21  1.5  src/bldenv/pkgtools/rename/distoc.sh, pkgtools, bos412, GOLDA411a 6/24/93 08:38:25"
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: abort
#		addblk
#		doit
#		func1
#		match
#		quiet_abort
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
#       Called from disstk to create toc from lpp_name files
#
#   Syntax:
#       distoc imagedir stacklist TOC stkwork
#
#   Flags:  Ref. usage message
#
#   Change history:
#      05/14/91 created from v3.1 utility
#	6/10/91 Modified to use ./$$ as work area, not /tmp--Defect 20434
#	6/10/91 Modified to make $fsize always numeric     --Defect 21259
#	6/28/91 Modified to detect blank lines in stack list
#	7/02/91 Removed redundant error checking
#	7/31/91 Modified to use work directory for lpp_name files
#	7/31/91 Added quiet_abort to handle while read loop exiting
#	7/31/91 Added $cmd to ERROR messages
#
#========================================================================

# Function: abort is called when an un-recoverable error has been detected
abort() {
  exit 1
}

# Function: quiet_abort is called when a 'while read' loop aborts
quiet_abort() {
  exit 1
}

#########################################################################
# func1 creates a file with data about the TOC and all the bff or tar files
# the file contains the following data:
# x=filename
# blk_count=size of file in 512 block
# skcount= the number of blocks to skip from the beginning of the output media
#          when writing using the dd command
#          this number will be used when writing to a temporary file
# disk_id= the diskette volume
# off_set=the block number on the diskette where the file begins
#          this number will be used when writing to diskette from a temp file
# fsize=the actual file size without any adjustments
#########################################################################
func1() {
  cd $work
  #maximum number of 512 blocks in a 2.0 MB diskette
  blk_maxsize=2879
  #this give the actual file size
    fsize=`li -Ic -En $x`
    fsize=`expr $fsize + 0`
    fsize_new=$fsize
    # echo "Size of $x: $fsize "

    difs=`expr $fsize % $BLKSZ`
    #if there is a remainder it means the file is not blocked in 512 blocks
    if [ "$difs" -ne 0 ]; then
      #adjust file size to 512 block
      fsize_new=`expr $fsize + $BLKSZ - $difs`
    fi
    #Divide by 512 to compute number of 512 blocks in file
    blk_count=`expr $fsize_new / $BLKSZ`
    #write to sizefile
    #filename block_count seek_count disk_volume offset filesize
    echo " $x $blk_count $skcount $disk_id $off_set $fsize" >> $SIZEFILE
    skcount=`expr $blk_count + $skcount`
    #compute total number of blocks used
    fsize_tot=`expr $fsize_tot + $blk_count`
    #compute the diskette volume the file will be written to and adjust by 1
    disk_id=`expr $fsize_tot / $blk_maxsize`
    disk_id=`expr $disk_id + 1`
    echo "$disk_id" > $LAST
    #the remainder will be the offset (or block where file starts)
    off_set=`expr $fsize_tot % $blk_maxsize`
}

#########################################################################
# doit function parses input lines read from lpp_name file.
#       And puts a marker on the first line of each lpp_name file.
#       This marker is a place holder that will be replace by the
#       volume, block number and size of file for every bff and or tar
#       file that will be written to diskette. This was done to keep
#       the size of the TOC file from increasing after the actual data
#       was written to the TOC file.
#########################################################################
doit() {
          #vvv:bbbbb:ssssssss
    markid=!!!!!!!!!!!!!!!!!!
    out=""
    if [ "$first_line" = "yes" ]; then
      #### The first record needs to have  volume:count:size info.
      #### All other records remain the same.
       out="$markid $stuff"
       first_line=no
    else
       out="$stuff"
    fi
    echo "$out" >> $TMPTOC
}

# Add blank lines to end of TOC.
# This is needed so volume number, blocksize offset, and file size
# can be entered later without making the size of TOC larger.
# Just wanted to make sure the file size of TOC was not increased,
# since the TOC is the first file entry in the diskette. If the TOC
# size increased, the TOC file would be truncated when written to
# diskette because the size of TOC was already computed by
# padded with blanks at the end and with the first record mark with
# exclamation for a place holder.
addblk() {
  blk_line_cnt=10
  blk_stuff=
  while [ $blk_line_cnt -gt 0 ]; do
    out="$blk_stuff"
    echo "$out" >> $TMPTOC
    blk_line_cnt=`expr $blk_line_cnt - 1`
  done

}

#############################################################
## Start of shell logic.  First, check number of parms, give
## abort if not enough.
#############################################################
if [ $# -lt 5 ]; then
  abort
fi

cmd="distoc"

# first octal characters needed to identify a file that has been backup via
# the backup command.
# Need this pattern to distinguish between a file that has been backup using
# backup or tar command.
backup_pattern="0000000  004400"

# distoc $CUR $STAKLST $TOC $WORK $WORK/stack.list

# Name of directory containing installable image directory
SOURCE=$1

# Full path name and filename of stack list file
RIOSPACK=$2

# Working TOC in temporary directory
TOC=$3

# Work directory
WORK=$4

# Working copy of stack list in temporary directory
STCK=$5

TMPTOC=$WORK/tmptoc
SIZEFILE=$WORK/sizefile
LAST=$WORK/last
BLKSZ=512

cd $SOURCE

# Read file that contain stack list (bff and or tar file names)
cat $RIOSPACK | while read a b; do
  i=`echo $a`
  # Check to make sure that the file name is not null
  if [ "x$i" = "x" ]; then
    echo "\n$cmd: ERROR: There is a blank line in your stack list file"
    echo "          Please remove it"
    abort
  else
    if [ -f $i ]; then
      cd $WORK
      echo "$i"
      # Remove lpp_name from temporary working directory
      rm -f ./lpp_name > /dev/null 2>&1
      #To distinguish how the file was backuped, check pattern to see
      #if it matchs backup pattern
      # if a match (return code from grep = 0 )
      # than extract file using restore command
      # if not a match assume file was backup using tar command
      # use tar to extract file
      od $SOURCE/$i | head -1 | grep "^$backup_pattern" > /dev/null
      if [ $? -eq 0 ]; then
        restore -xqf $SOURCE/$i ./lpp_name >/dev/null 2>&1
        if [ $? -ne 0 ]; then
	  echo "\n$cmd: ERROR: Could not extract lpp_name file from $SOURCE/$i"
	  abort
        fi
      else
        tar -xf $SOURCE/$i ./lpp_name > /dev/null 2>&1
        if [ $? -ne 0 ]; then
	  echo "\n$cmd: ERROR: Could not extract lpp_name file from $SOURCE/$i"
	  abort
        fi
      fi
      # if lpp_name file not found restore or tar failed
      if [ ! -r lpp_name ]; then
        echo "\n$cmd: ERROR: Could not extract lpp_name file from $SOURCE/$i"
        abort
      fi
      # Process files in stack list filename
      # Keep track of size of files being written to tape
      x=${SOURCE}/$i
      echo "$i" >> ${STCK}
      first_line=yes
      cat ./lpp_name | while read stuff; do
        doit
      done
      # Check return code from while loop (when while loop was invoked 
      # a new shell program was started
      if [ $? != 0 ]; then
        quiet_abort
      fi
      cd $SOURCE
    else
      echo "\n$cmd: ERROR: BFF $i not found in image directory $SOURCE "
      echo "         Check image directory and/or stack list file"
      abort
    fi
  fi
  cd $SOURCE
done
# Check return code from while loop (when while loop was invoked 
# a new shell program was started
if [ $? -ne 0 ]; then
  quiet_abort
fi

# Add blank lines to end of toc
addblk

# Compute size and offset information for first file which will be the TOC
first_size=yes
if [ "$first_size" = "yes" ]; then
  skcount=0
  fsize_tot=1
  off_set=1
  disk_id=1
  cat $TMPTOC > $VOLUME_TOC		# Putting TMPTOC in sizefile in not right.
  x=$VOLUME_TOC			# Exported value from disstk.
  func1
  first_size=no
fi

# Compute size and offset information for rest of the image files
cd $SOURCE
cat $RIOSPACK | while read a b; do
  i=`echo $a`
  x=${SOURCE}/$i
  func1
done
if [ $? != 0 ]; then
  quiet_abort
fi

# Replace all record containing the special place holder and replace
# with the disk_id, offset and filesize information
cd $WORK

# awk_count= the record number to be read by the awk command
# need to set awk_count=2 to skip over the first record in SIZEFILE
# which was the TOC data that was already processed
awk_count=2

# Read all lines of temporary TOC file
cat $TMPTOC | while read junk; do
  out=$junk
  # if line has exclamation sign in column one replace marked data with data
  # from SIZEFILE
  if expr "$junk" : "!" > /dev/null 2>&1; then
    #set id to fourth,fifth and sixth field of 'sizefile' record.
    # fourth field = disk_id; fifth field = offset; sixth field = file size.
    id=`awk "NR==  $awk_count"'{print$4 ":" $5 ":" $6}' $SIZEFILE`
    awk_count=`expr $awk_count + 1`
    set $out; shift; out="$id $*"	# Replace !!!! with $id value.
   fi
  echo "$out" >> $TOC
done
if [ $? != 0 ]; then
  quiet_abort
fi

cd $SOURCE
exit 0
