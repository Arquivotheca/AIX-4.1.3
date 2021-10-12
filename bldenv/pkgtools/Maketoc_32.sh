#!/bin/ksh
# @(#)40        1.24  src/bldenv/pkgtools/Maketoc_32.sh, pkgtools, bos412, GOLDA411a 3/22/94 10:38:42
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS:  abort
#		checksize
#		dobosboot
#		dolppimage
#		leadcount
#		leadvolume
#		
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
#       Called from stack_32 to create toc from lpp_name files
#
#   Syntax:
#       Maketoc imagedir stacklist TOC log < y | n > maxsize stkwork
#
#===============================================================================
############################################
# Abort exits the shell with an error code #
############################################
abort() {
  exit 1
}

leadcount() {
  if [ $count -lt 10 ]
  then
    count="0$count"
  fi
  if [ $count -lt 100 ]
  then
    count="0$count"
  fi
}

leadvolume() {
  if [ $volume -lt 10 ]
  then
    volume="0$volume"
  fi
}

######################################################################
# Check size of image - if it's not blocked at 31744, tell user, and #
# recompute image size, rounding up to nearest multiple of 31744.    #
######################################################################
checksize() {
  fsize=`expr $fsize + 0`                       # Remove blanks from the fsize
  fsize_string=$fsize
  difs=`expr $fsize % $BLOCK_SIZE`              # Get remainder
  if [ $difs != 0 ]; then                       # Is file multiple of BLOCK_SIZE
    fsize=`expr $fsize + $BLOCK_SIZE - $difs`   # If not, raise it to next
    adj_string="(adjusted to $fsize)"
  else
    adj_string=""
  fi
  volume_size=`expr $volume_size + $fsize`
  echo $volume_size > $WORK/volume_size
}


dobosboot() {
  for x in $BOSBOOT $BOSINSTAL; do
    # The -L flag of 'ls' follows any symbolic link(s) to get the actual filesize
    # as opposed to just the link itself.
    fsize=`ls -lL $x | awk '{print $5}'`
    checksize
    echo "Size of $x:        $fsize_string   $adj_string"
  done
}


dolppimage() {
    # Check to make sure that the file name is not null
    if [ "x$a" = "x" ]; then
      echo "\n$cmd: ERROR: There is a blank line in your stack list file"
      echo "                   Please remove it"
      abort
    fi

    #
    # check if path specified on stacklist entry
    #
    echo $a | grep '^/' > /dev/null
    if [ $? -ne 0 ]; then
        #
        # add default path for relative entries
        #
        i="$image_dir/$a"
    else
        #
        # do nothing for absolute filenames
        #
        i="$a"
    fi

    if [ ! -f $i ]; then
      echo $i | grep "^/afs/austin" > /dev/null 2>&1
      if [ $? -eq 0 ]; then
        echo "\n$cmd: ERROR: image $i not found"
        echo "                   The file is on the AFS, this could be the"
        echo "                   AFS server problem. Please check!"
      else 
        echo "\n$cmd: ERROR: image $i  not found"
        echo "                   Check image directory and/or stack list file"
      fi
      abort
    fi

    # To distinguish how the file was backed-up, check pattern to see
    # if it matchs backup pattern
    backup_pattern="0000000  004400 065752"
    od $i | head -1 | grep "^$backup_pattern" > /dev/null
    if [ $? -eq 0 ]; then

        ##This is a temporary fix for a restore command problem that truncates
        ## filenames after 100 characters
        ## Referece Defects 41487 and 41362
        ## Use wc to count the number of character in full path
        ## image directory and file name
      wc_cnt=`echo $i | wc -c`
        ## if wc is greater than 100 copy file to temp directory since
        ## restore command truncates file names after 100 characters
      if [ $wc_cnt -gt 100 ]
      then
        clone_file=`basename $i`
         ## create symbolic link for filename gt 100 into work directory
         ## and verify copy was successful
        ln -s $i $clone_file
        if [ $? -ne 0 ]; then
          echo "\n$cmd: ERROR: creating a symbolic link in the work directory\c"
          echo " $WORK that points to the file $i"
          abort
        fi
        ## Extract files using restore command
        tocentry=$WORK/tocent.$$
        rm -f $tocentry
        #$tooldir/gen_toc_entry -b $clone_file -t $tocentry
        gen_toc_entry -b $clone_file -t $tocentry
        ##Remove clone file from working directory
        rm -f $clone_file
      else
        # Extract files using restore command
        tocentry=$WORK/tocent.$$
        rm -f $tocentry
        #$tooldir/gen_toc_entry -b $i -t $tocentry
        gen_toc_entry -b $i -t $tocentry
      fi
    else
      # If not a match assume file was backed up using tar command
      tocentry=$WORK/tocent.$$
      rm -f $tocentry
      #$tooldir/gen_toc_entry -r $i -t $tocentry
      gen_toc_entry -r $i -t $tocentry
    fi

    # Check to see if the $lpp_name file was created
    if [ ! -f $tocentry ]; then
      echo "\n$cmd: ERROR: Problem extracting $lpp_name file from \c"
      echo "$i"
      abort
    fi
################################################################
## Find out if this is bos.obj.  If it is,
## then DON'T put it into TOC.  (For AIX 4.1, bos.obj
## is called bos.rte and SHOULD be included in the TOC.)
################################################################

    bosname=`head -2 $tocentry | tail -1 | cut -d" " -f1`
    if [ "x$bosname" = "xbos.obj" ]
    then
      ###################################################
      ######  Added below stuff to differentiate between
      ######   bos.obj and the updates for bos.obj
      ######   Tom Krueger    1/17/91
      ###################################################
      mojo=`awk 'NR == 1 {print $3;exit}' < $tocentry`
      if [ "$mojo" = "I" -o "$mojo" = "T" ];
      then
	 isbos=yes
      else
	 isbos=no
      fi

    else
      isbos=no
    fi
    # Process files in stack list filename
    # Keep track of size of files being written to tape
    # The -L flag of 'ls' follows any symbolic link(s) to get the actual 
    # file size, as opposed to just the link itself.
    x=$i
    fsize=`ls -lL $x | awk '{print $5}'`

    checksize
    if [ $fsize -gt $maxsize ]; then
      difs=`expr $fsize % 1000000`                    # Get remainder
      new_max=`expr $fsize + 1000000 - $difs`
      echo "\n$cmd: ERROR: Maximum size of media must be at least $new_max."
      abort
    fi
    if [ $volume_size -gt $maxsize ]; then
      volume_size=`expr $volume_size - $fsize`
      volume_size=`expr $volume_size + 0`
      echo "Total size of images on volume $volume is $volume_size bytes\n"
      volume_size=`expr $fsize + 0`
      volume=`expr $volume + 1`
      leadvolume
      echo $volume > $WORK/volume
      echo $volume_size > $WORK/volume_size
      count=4
      leadcount
    fi

    # Put the file name into the current volume's stack list
    echo "Size of $x:      $fsize_string   $adj_string"
    echo "$i" >> ${WORK_STACK_LIST}.$volume

    # The first record needs to have  volume:count info.
    # All other records remain the same.
    if [ "$isbos" = "no" ]
    then
      #############################################
      ## following line replaces a lot of logic that
      ## is now in gen_toc_entry.
      #############################################
      echo "$volume:$count:$fsize `cat $tocentry`" >> $TOC
      if [ $? -ne 0 ]; then
        echo "\n$cmd: ERROR: Problem while appending toc fragment to toc"
        abort
      fi
      count=`expr $count + 1`
      leadcount
    else
      count=`expr $count + 1`
      leadcount
    fi
}
#########################
# Start of shell logic. #
#########################
if [ $# -lt 7 ]; then
  abort
fi

# Initialize variables
cmd="Maketoc_32"
tooldir=`type Maketoc_32 | cut -d" " -f3`
tooldir=`dirname $tooldir`

# Make sure volume_size is numeric
volume_size=0

# Initialize fsize
fsize=0

# $image_dir $STACK_LIST $TOC $CURRENT $bootable $maxsize $WORK_STACK_LIST $toc_format
# Name of directory containing installable image directory
image_dir=$1

# Full path name and filename of stack list file
STACK_LIST=$2

# Working TOC in temporary directory
TOC=$3

# Work directory
WORK=`dirname $3`

# Current directory
CURRENT_DIR=$4

# Set bootable flag
bootable=$5

# Max size of tape
maxsize=$6

# Working copy of stack list in temporary directory
WORK_STACK_LIST=$7

# Format of TOC file where:
#	1=AIX v3.1
#	2=AIX v3.2
#	3=AIX v4.1
toc_format=$8

########################################################################

# Open the volume file
volume=1
leadvolume
echo $volume > $WORK/volume

# If bootable get size of file1 and file2
# Get file size - must accumulate file sizes per volume so we
# don't try to put more on a volume than will fit the tape.
count=4
leadcount
if [ "$bootable" = "yes" ]
then
  dobosboot
fi

# For each file in the image directory, extract the bff and build the toc
cd $WORK
lpp_name="./lpp_name"
cat $STACK_LIST | while read a garbage; do
  dolppimage
done
# Check return code from while loop (when while loop was invoked, a
# New shell program was started)
if [ $? -ne 0 ]; then
  abort
fi

# Get the volume_size from the temporary file
volume_size=`awk '{print $1}' $WORK/volume_size`
volume=`awk '{print $1}' $WORK/volume`
echo "Total size of images on volume $volume is $volume_size bytes\n"

# Graceful exit
cd $CURRENT_DIR
exit 0

