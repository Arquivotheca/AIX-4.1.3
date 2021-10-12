#!/bin/sh
# @(#)20        1.12.1.3  src/bldenv/pkgtools/rename/genbff.sh, pkgtools, bos412, GOLDA411a 3/3/93 15:24:22
# COMPONENT_NAME: genbff.sh
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# ENVIRONMENT VARIABLES:
#   $TOP - set by makefiles.            

# Assumptions
#   1. An install build has been performed.
#   2. MKSET has created <set_lppname> directory and copied ship files.
#   3. A subsequent run has changed owners, permissions, etc.
#   4. The apply lists exist in the directory from which this is called.
#   5. The runner of this program has ROOT privileges.
#   6. This program is called in the 'user' or 'share' directory.
#
#  Functions:
#   1. Get the name of the lpp from the lpp_name file.
#   2. Build the names of the apply lists (lpp.option.al)
#   3. Create a backuplist
#      a. First entry is always ./lpp_name
#      b. Next entry could be ./usr/lpp/<lpp_name>/inst_root/lpp_name
#         if there is a root part.
#      c. The next entry is always ./usr/lpp/<lpp_name>/liblpp.a
#	  (or ./usr/share/<lpp_name>/liblpp.a, if share), plus
#	  ./usr/lpp/<lpp_name>/inst_root/liblpp.a if root present.
#      d. The rest is the names of the executables in each apply list.
#   4. Perform a backup or tar, depending on -t flag, using backuplist.

# Initialize flags
tarFlag="n"
quietFlag=""
volFlag="N"
bosFlag="N"

# Parse command line arguments
set -- `getopt "f:l:qvtb" $*`

if [ $# -lt 4 ]; then
    echo "$0: Usage: genbff -f <device> -l <lpp_name> [-q] [-t] [-v]"
    echo "$0: The -f and -l flags are required" 
    exit 1
fi

while [ $1 != "--" ]; do		# Set vars, based on params
    case $1 in
	-f)
	    device=$2;  shift;  shift;;
	-l)
	    lpp=$2; shift; shift;;
	-q)
	    quietFlag="-q";    shift;;
	-t)
	    tarFlag="y"; shift;;
	-v)
	    volFlag="Y"; shift;;
	-b)
	    bosFlag="Y"; shift;;
    esac
done

# Set variable for locating the lpp.lp file
if [ -f $BLDENV/usr/bin/tpath ]; then
   inlppfl=`eval $BLDENV/usr/bin/tpath 2> /dev/null $lpp.lp`
   else
   inlppfl="$lpp.lp"
fi

if [ ! -r $inlppfl ]; then
    echo "$0:  Cannot read $inlppf file."
    exit 2
fi

if [ ! -f lpp_name ]; then
    echo "$0:  Cannot find lpp_name file."
    exit 3
fi

if [ ! -w lpp_name ]; then
    echo "$0:  Cannot write to lpp_name file."
    exit 3
fi

if [ -d root ]; then			# Subdir named 'root'?
    rootFlag="y"			# If so, set root flag.
    else
    rootFlag="n"
fi

curr_dir=`pwd|awk 'BEGIN {FS="/"}; {print $NF}'`  # Get current dir w/o path.
cu_dir=`pwd`				# cu_dir is current path.

if [ $curr_dir = "share" ]; then	# Set set_<lppname> path:
    setPath="$TOP/sets_$lpp"           #   if share, this way
else					# otherwise,
    setPath="$TOP/set_$lpp"		#   not share, this way.
fi

if [ ! -d $setPath ]; then		# Must be able to find set path.
    echo "Cannot find expected set path $setPath."
    exit 4
fi

####### House keeping and directory authentication have been done. ######
#######			REAL PROCESSING STARTS HERE.		   ######

if [ "$bosFlag" = "Y" ]; then		# Start bos lpp

# Set variable name for <lpp>.first

    if [ -f $BLDENV/usr/bin/tpath ]; then
       find_first=`eval $BLDENV/usr/bin/tpath 2> /dev/null $lpp.first`
       else
       find_first="$lpp.first"
    fi

# If there is a file called <lpp_name>.first then put the contents of
# that file as the first entry in the backup list. bos is probably the
# only one to do this and it would be for a tar image.

    if [ -f "$find_first" ]; then	# If file found, copy
	cp  $find_first  backup_list	# contents to backup_list
	echo "./lpp_name" >> backup_list   # then add lppname.
	else				# If not found, then start
	  echo "./lpp_name" > backup_list  # backup_list with lppname.
    fi
    
# Need to do a find on setdir and then grep through the output for ./usr
# This will put all files into the backup list that start with "./usr"

    curdir=`pwd` 
    cd $setPath				# cd to dir where files are.
    find . -print >/tmp/bcklst$$
    cd $curdir
    grep "./usr" /tmp/bcklst$$ >> backup_list
    rm -f /tmp/bcklst$$

#    cat $inlppfl | awk '$1 ~ /^'$lpp'.*/ {print $1}' > /tmp/wrk1.$$

#    cat /tmp/wrk1.$$ | while read opt; do    # Read each option from wrk1,
#	al=$opt.al                      #   construct apply list file name,
#	cat $al | grep "^./usr" >> backup_list  #   and append apply list to
#    done                                #   backup list.

#    rm /tmp/wrk1.$$ 			#  clean up 

# This section will process a share in addition to a usr and usr/root.
# This is a special requirement for bos and is done if the bosFlag is set.

#    if [ -f $BLDENV/usr/bin/tpath ]; then  # Set variable for share
#       inlppfl=`eval $BLDENV/usr/bin/tpath 2> /dev/null share/$lpp.lp`
#       else				# <lpp>.lp file.
#       inlppfl="share/$lpp.lp"
#    fi
#    cat $inlppfl | awk '$1 ~ /^'$lpp'.*/ {print $1}' > /tmp/wrk1.$$
#    cat /tmp/wrk1.$$ | while read opt; do	# Read each option from wrk1,
#    al=share/$opt.al			#   construct apply list file name,
#    cat $al | grep "^./usr" >> backup_list      #   and append share apply
#    done				#   list to the backup list.
#    rm /tmp/wrk1.$$

fi					# End bos lpp

# If the bosflag is not set, process as a normal lpp

if [ "$bosFlag" = "N" ]; then		# Start normal lpp

# Put liblpp.a into the backup_list according to whether this is a
# "usr", "usr/root" or "share" part.

    echo "./lpp_name" > backup_list		# Start with lpp_name

    if [ $curr_dir = "share" ]; then	# If share, only put share in list.
	echo "./usr/share/lpp/$lpp/liblpp.a" >> backup_list
	else				# It is a user or user/root, put
	    echo "./usr/lpp/$lpp/liblpp.a" >> backup_list # in the user part.
    fi

    if [ $rootFlag = "y" ]; then	# If root, put root lpp_name in list.
	    echo "./usr/lpp/$lpp/inst_root/lpp_name" >> backup_list
	    echo "./usr/lpp/$lpp/inst_root/liblpp.a" >> backup_list 
    fi

######### Put option names into wrk1 from input style lpp name file ######

    cat $inlppfl | awk '$1 ~ /^'$lpp'.*/ {print $1}' > /tmp/wrk1.$$

    if [ "$bosFlag" = "N" ]; then
	cat /tmp/wrk1.$$ | while read opt; do   # Read each option from wrk1,
	    al=$opt.al                      #   construct apply list file name,
	    cat $al >> backup_list          #   and append apply list to
	done                                #   backup list.
    fi

fi					# End normal lpp

rm -f /tmp/wrk1.$$

cd $setPath				# cd to dir where files are.

# If this lpp has "R5" in it, call genpro to strip all of the "R5" stuff.
cnt=`echo $lpp | grep -c "R5"`             
if [ "$cnt" != "0" ]; then
   $BLDENV/usr/bin/genpro -p$cu_dir
fi

if [ $tarFlag = "y" ]; then		# Use tar
    tar -L $cu_dir/backup_list -cdl $quietFlag -f $device
    else				# Use backup
	cat $cu_dir/backup_list \
        | backup -iv -b62 $quietFlag -f$device -p -e"lpp_name" 2>&1 \
 	| tee $cu_dir/back_track
fi

exit 0
