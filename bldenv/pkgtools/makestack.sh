#!/bin/ksh
# @(#)26	1.11  src/bldenv/pkgtools/makestack.sh, pkgtools, bos412, GOLDA411a 9/28/94 11:33:50
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: 
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
#  FILE_NAME: makestack     
#========================================================================
#
#   Purpose: 
#       To provide a front-end to "stack_32" tool for creating stacked
#       tapes.  ("stack_32" can also be used directly if the user
#       prefers.)  "makestack" simply fills in some of the flag fields to
#       make it easier to create a simple bootable stacked tape.  This
#       shell also allows the path to images, and stacklist to be 
#       specified via environment variables.
#
#    Description:
#       If called with no flags, "makestack" will cause a bootable stacked
#       tape to be created using /dev/rmt0 as the tape drive, and the current
#       directory as the location of the images and stack.list (unless the
#       environment variables STAKLST and CUR are used to specify some other
#       directory).
#
#   Flags:  Refer to syntax function below 
#
#   Change history:
#      5/21/91 created 
#      7/16/91 added -m flag support
#      10/1/91 changed to go with stack_32 and Maketoc_32 changes
#              regarding how "current" directory is handled
#      10/7/91 made it able to handle suffixes on bosboot.tape** and
#              bosinst.tape** names
#      10/8/91 don't ck for bos*tape* if -B flag specified
#     07/15/92 add -h option
#     09/27/93 add -1 and -2 option
#     09/28/94 changed algorithm for bosboot.tape and bosinst.tape when
#              -1 and -2 options are not specified
#
#========================================================================
#
#===============================  Function ==============================
# the 'syntax' function simply echos command syntax, then exits the shell
#========================================================================
syntax() {
  echo "$cmd [-l staklst] [-c imgdir] [-d rmt?] [-n dir] [-B] [-h]"
  echo "\t[-i<image_type>] [-1 bosboot_image] [-2 bosinst_image]"
  echo ""
  echo "FLAGS:   -l  Filename of stack list file.  Defaults to file named"
  echo "             stack.list in current directory. Can also be set via"
  echo "             environment variable STAKLST."
  echo "         -c  Directory name containing installable images. Defaults to"
  echo "             current directory.  Can also be specified by env var CUR."
  echo "             Note: Filename and Directory can be full path name or"
  echo "                   relative path name."
  echo "         -d  Device type for tape device. Default is rmt0."
  echo "             Example: -d rmt5  (do not enter /dev/rmt5)"

  echo "Hit any key to continue page:  \b"
  read junk

  echo "         -i  Specifies image type by AIX release for TOC format.  Valid"
  echo "             values are 3.1, 3.2 and 4.1 which indicate type 1, 2 and"
  echo "             3 TOC formats, respectively.  Default is type 1."
  echo "         -n  Will cause generation of TOC, but will not write a tape."
  echo "             Takes as an argument the name of the directory for the"
  echo "             TOC to be written in."
  echo "         -B  Will generate a tape without the bosboot/bosinst images."
  echo "             (i.e, will generate a NON-bootable tape)"
  echo "             ....Another way to make a non-bootable tape is to NOT"
  echo "             have bosboot* or bosinst* in the '-c' directory."
  echo "         -h  List help message"
  echo "         -1  Bosboot image. Defaults to bosboot.tape or most recent"
  echo "             version of bosboot.tape*."
  echo "         -2  Bosinst image. Defaults to bosinst.tape or most recent"
  echo "             version of bosinst.tape*."
exit
}

#========================================================================
#                         First Things First    
#========================================================================
PATH=$PATH:`pwd`:
export PATH
cmd=`basename $0`
cmdline="$*"

#========================================================================
# Check to see if the stack list is being specified by an env variable.
# If it's not, then, make it default to "stack.list" in pwd.
#   $STAKLST
#      The pathname (full or relative) of the 'stack.list' file.
#      The 'stack.list' file is a list, in the required install sequence,
#      of all the package entities to be put on a stacked tape.
#========================================================================
if [ "x$STAKLST" = "x" ]
then
   STAKLST=`pwd`/stack.list
fi

#========================================================================
#                          Evaluate Flags        
#========================================================================
#  Look at flags: If -B, then need to call "stack_32" without -1/-2 flags.
#    If -l or -c specified, then need to override values in $CUR/$STAKLST.
#    Other flags should be passed on to "stack_32" as is.  
#========================================================================
set -- `getopt "l:c:d:n:Bbhi:1:2:" $*`
if [ $? -ne 0 ]
then
  echo "ERROR in command line '$cmdline':\n"
  syntax
fi
 
#========================================================================
# Have to set this so can use it in figuring out bos*.tape** names...
# Don't need it for call to stack_32 anymore.
#========================================================================
if [ "x$CUR" = "x" ]
then
	CUR=`pwd`
fi

newcmd=""
boot=yes
lflag=no
cflag=no
image_type=""

#------------------------------------------------------------------------
# If we were invoked as makestack_41 use a 4.1 image type in stack_32.	|
# Set image_type so we will ignore -i from command line if it's there.	|
#------------------------------------------------------------------------
if [ "$cmd" = "makestack41" ]
then
	newcmd="-i 4.1"
	image_type=4.1
fi

while [ "$1" != "--" ]
do
  case $1 in
    "-B")
	boot=no
	;;
    "-b")
        echo "\nNOTICE: the makestack command has been changed - you no longer"
        echo "need to use the '-b' flag to indicate NO BOS - simply don't put"
        echo "bos or bos.obj in your stacklist file."
        sleep 2
        syntax
        ;;
    "-l")
	shift
	STAKLST="$1"
	lflag=yes
    	;;
    "-c")
	newcmd="$newcmd $1"
	shift
	newcmd="$newcmd $1"
	cflag=yes
	CUR="$1"
    	;;
     "-d") 
	newcmd="$newcmd $1"
	shift
	newcmd="$newcmd $1"
	;;
     "-n") 
	newcmd="$newcmd $1"
	shift
	newcmd="$newcmd $1"
	;;
    "-i")
	if [ -z "$image_type" ]
	then
	    newcmd="$newcmd $1"
	    shift
	    newcmd="$newcmd $1"
	fi
	;;
     "-h")
        syntax
        ;;
     "-1")
        shift
        BOSBOOT="$1"
        ;;
     "-2")
        shift
        BOSINSTAL="$1"
        ;;
    esac
  shift
done 

#=======================================================================
# If a -c flag was NOT specified, then, look to see if the CUR environment 
# variable was set.  If it was, then, must send it's value to stack_32.
#=======================================================================
if [ x$cflag != "xyes" ]
then 
  if [ "x$CUR" != "x" ]
  then
    newcmd="$newcmd -c $CUR"
  fi
fi

#=======================================================================
# Regardless of how STAKLST was set, must now make sure file exists.
#=======================================================================
if [ ! -f $STAKLST ]
then
  echo "$cmd: stack.list file ($STAKLST) was not found." 
  echo "         Use -l option, or set environment var STAKLST.\n"
  syntax
fi

#========================================================================
# Must find full name of bosinst.tape and bosboot.tape images
#========================================================================
if [ x$boot = "xyes" ]
then
  # if the bosboot image was not specified on the command line
  if [ -z "$BOSBOOT" ]
  then 
    fn=`ls $CUR/bosboot.tape 2>/dev/null`
    # if there is no file by the name bosboot.tape
    if [ "x$fn" = "x" ]
    then
      # get the latest version of the bosboot.tape* image
      fn=`ls -t $CUR/bosboot.tape* 2>/dev/null | head -1`
    fi
  else
    fn=$BOSBOOT
  fi
  bosbootname=`basename $fn 2>/dev/null`
  if [ "x$bosbootname" = "x" ]
  then
    echo "Error: bosboot* image not found." 
    echo "Do you want to make a non-bootable tape? (y/n - default 'y')"
    read ans
    case "$ans" in
      n* | N*)
        echo "Either make sure bosboot<.tape> is in directory with rest of"
        echo "	the images, or call stack_32 directly.\n"
        syntax
        ;;
      *)
        boot=no
        break
        ;;
    esac
  fi

  # if the bosinst image was not specified on the command line
  if [ -z "$BOSINSTAL" ]
  then 
    fn=`ls $CUR/bosinst.tape 2>/dev/null`
    # if there is no file by the name bosinst.tape
    if [ "x$fn" = "x" ]
    then
      # get the latest version of the bosinst.tape* image
      fn=`ls -t $CUR/bosinst.tape* 2>/dev/null | head -1`
    fi
  else
    fn=$BOSINSTAL
  fi
  bosinstname=`basename $fn 2>/dev/null`
  if [ "x$bosinstname" = "x" ]
  then
    echo "Error: bosinst* image not found." 
    echo "Do you want to make a non-bootable tape? (y/n - default 'y')"
    read ans
    case "$ans" in
      n* | N*)
        echo "Either make sure bosinst<.tape> is in directory with rest of"
        echo "	the images, or call stack_32 directly.\n"
        syntax
        ;;
      *)
        boot=no
        break
        ;;
    esac
  fi
fi

#========================================================================
#                            Call "stack_32"     
#========================================================================
if [ x$boot = "xyes" ]
then
  echo "stack_32 -l $STAKLST -1 $bosbootname -2 $bosinstname $newcmd"
  stack_32 -l $STAKLST -1 $bosbootname -2 $bosinstname $newcmd
else    
  echo "stack_32 -l $STAKLST $newcmd"
  stack_32 -l $STAKLST $newcmd
fi
exit 0
