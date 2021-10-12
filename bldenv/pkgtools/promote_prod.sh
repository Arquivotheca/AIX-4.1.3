#!/bin/ksh
#
# SCCSID:  @(#)03  1.28  src/bldenv/pkgtools/promote_prod.sh, pkgtools, bos41J, 9519A_all 4/25/95 15:05:50
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: abort
#		entries
#		syntax
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

######################### Function ########################
# syntax function echos command syntax
###########################################################
syntax() {
    echo "\npromote_prod -b build_dir [-p prod_dir] [-s ship_dir]"
    echo "             [-V AIX version] [-T ptfapardef_table] "
    echo "             [-n] ccss_filename(s)\n             [-h | -?]\n"
    echo "FLAGS:   -b     Build directory name"  
    echo "         -p     Production directory name"
    echo "         -s     Ship directory name"
    echo "         -V     AIX version of the ptfs - valid entries are 3 or 4."
    echo "                Defaults to 3 if the -V flag is not used or if"
    echo "                the entry does not begin with a 4"
    echo "         -T     ptfapardef table"
    echo "         -n     NO EXECUTE!  list prerequisites only"
    echo "         -?/-h  Displays promote_prod syntax"
    echo "         ccss   At least one valid ccss file name"
    exit $rc
}


#######################################################################
#  Abort is called when an error occurs after WORK space is allocated
#######################################################################
abort() {
    if [[ "$update_in_progress" = "yes" ]]; then
        echo "\n$cmd: index files for $ccss_filename were not updated although"
        echo "$cmd: this ptf is on prod\n"
        echo "$cmd: ptfapardef.constant, ptfapardef.vary and ptfapardef.ptfpkg"
        echo "$cmd: were not updated for $ccss_filename"
        echo "$cmd: Aborted successfully"
    fi
    if [ "$deletelockfile" = "yes" ]
    then
        rm -f $PROD_DIRNAME/lockfile
    fi
    rm -rf $WORK
    exit 1
}


#######################################################################
#  Begin SHELL logic
#######################################################################
# Initialize variables
execute="yes"
build_dir=""
cmd=`basename $0`
rc=0
start_dir=`pwd`
WORK=""
buildflag="no"
prodflag="no"
shipflag="no"
version=""
prod_dir=""
ship_dir=""  
update_in_progress="no"
ptfapardef_table=""
deletelockfile="no"

# Check for no parameters
if [ $# -eq 0 ]
then
  rc=1
  syntax
fi
cmdline=""

# Validate all options on the command line
set -- `getopt "V:T:s:p:b:nh?" $*`
if [ $? != 0 ]; then
    # no error message necessary as getopt already printed one
    rc=2
    syntax
fi

# Parse the command line
while [ $1 != "--" ]; do                # Set vars, based on params.
    case $1 in
        -b)
            build_dir=$2
            buildflag="yes"
            shift 2;;
        -p)
            prod_dir=$2
            prodflag="yes"
            shift 2;;
        -s)
            ship_dir=$2
            shipflag="yes"
            shift 2;;
        -V)
            version=$2
            shift 2;;
        -T)
            ptfapardef_table=$2
            shift 2;;
        -n)
            execute="no"
            shift;;
        -h)
            syntax;;
    esac
done
shift                                   # Get past the '--' flag.

# clean up the files in /tmp if the shell does not complete.
trap "abort" 1 2 3 15

# check version to determine prod and ship default directories
echo $version | grep "^3" >/dev/null
rc3=$?
echo $version | grep "^4" >/dev/null
rc4=$?
if [ "$version" = "" -o "$rc3" = "0" ]
then
    version=32
elif [ "$rc4" = "0" ]
then
    version=41
else
    echo "$cmd: Invalid version entered with the -V flag."
    echo "$cmd: Defaulted to processing AIX version 3 ptfs."
    version=32
fi

# source in the defaults 
type "$version"_pkg_environment > /dev/null 2>&1
[ $? = 0 ] && . "$version"_pkg_environment 

# make sure the most recent version of the pkg_environment script is being used
if [ "$PROD_DIRNAME_PATTERN" = "" ]
then
    echo "$cmd: The most recent version of the ${version}_pkg_environment "
    echo "$cmd:  script is not being used.  Please extract the latest     "
    echo "$cmd:  version out of CMVC and modify as necessary.  New        "
    echo "$cmd:  environment variables have been added.                   "
    rc=3
    exit $rc
fi

# if the prod directory was not set on the command line, then set it to the 
# environment variable
if [ "$prod_dir" = "" ]
then
    prod_dir=$PROD_DIRNAME
fi

# if the ship directory was not set on the command line, then set it to the 
# environment variable
if [ "$ship_dir" = "" ]
then
    ship_dir=$SHIP_DIRNAME
fi
 
# Check build directory 
if [ "$build_dir" = "" ]; then
   echo "$cmd: The build directory is required"
   rc=3
else
   if [ ! -d "$build_dir" ]; then
      echo "$cmd: The build directory $build_dir is invalid"
      rc=3
   else
      if [ ! -w "$build_dir" ]; then
         echo "$cmd: build directory $build_dir must be writable"
         rc=3
      else
         # make directory absolute and check index file
         cd $build_dir
         build_dir=`pwd`
	 if [ ! -f "$build_dir"/index ]; then
            if [ "$version" != "32" ]
            then
               # if the -T flag was not specified, then give an error
               if [ "$ptfapardef_table" = "" ]
               then
                  echo "$cmd: cannot create the index file for version 4"
                  echo "$cmd: ptfs without specifying the ptfapardef table"
                  echo "$cmd: with the -T flag"
                  rc=3
               else
	          # create a new index file in the build directory
                  gen_ptf_index -o $build_dir/index -T $ptfapardef_table $build_dir > /dev/null
               fi
            else 
               # create a new index file in the build directory
               gen_ptf_index -o $build_dir/index $build_dir > /dev/null
            fi
	    if [ $? != 0 ]; then
	       echo "$cmd: error occurred with gen_ptf_index making index"
	       abort
            fi
         fi
         cd $start_dir
      fi
   fi
fi
   
# Check production directory 
if [ ! -d "$prod_dir" ]; then
   echo "$cmd: The production directory $prod_dir is invalid"
   rc=3
else
   if [ ! -w "$prod_dir" ]; then
      echo "$cmd: production directory $prod_dir must be writable"
      rc=3
   else
      # make directory absolute and check index file
      cd $prod_dir > /dev/null
      prod_dir=`pwd`
      if [ ! -w "index" ]; then
         echo "$cmd: index in $prod_dir must be writable"
         rc=3
      fi  
      cd $start_dir
   fi
fi

# Check ship directory ( needed only on no_execute ) 
if [ ! -d "$ship_dir" ]; then
   echo "$cmd: The ship directory $ship_dir is invalid"
   rc=3
else
   if [ ! -r "$ship_dir" ]; then
      echo "$cmd: The ship directory $ship_dir must be readable"
      rc=3
   else
      # make directory absolute and check index file
      cd $ship_dir
      ship_dir=`pwd`
      if [ ! -r "index" ]; then
         echo "$cmd: index in $ship_dir must be readable"
         rc=3
      fi  
      cd $start_dir
   fi
fi

# Check for unique directory names  
if [ "$build_dir" = "$ship_dir" ]; then
   echo "$cmd:  The build and ship directories are the same"
   rc=3
fi
if [ "$build_dir" = "$prod_dir" ]; then
   echo "$cmd:  The build and production directories are the same"
   rc=3
fi
if [ "$prod_dir" = "$ship_dir" ]; then
   echo "$cmd:  The production and ship directories are the same"
   rc=3
fi

# check to make sure that the prod and ship directories are the same release
print "$prod_dir $ship_dir" | grep "/320/" | grep "/4.1/" 
if [ $? = 0 ]
then
   echo "$cmd: the prod and ship directories cannot be different releases"
fi

# Check if any ccss's were given
if [ $# -eq 0 ]; then
   echo "$cmd: at least one ccss_filename is required"
   rc=3
fi

# Give syntax if error was found above
if [ "$rc" != "0" ]; then
   syntax
fi 

# determine the df command format
os_version=`uname -v`
if [ "$os_version" = "4" ]
then
    DF_CMD="df -k"
else
    DF_CMD="df"
fi

# Create temporary work space with unique directory name 
# using process id and current date and time
if [ "$BLDTMP" = "" ]; then
   BLDTMP="/tmp"
fi
WORK=$BLDTMP/${cmd}_work.$$
rm -rf $WORK
mkdir $WORK
if [ $? != 0 ]; then
   echo "$cmd: Could not make work directory $WORK"
   echo "$cmd: Process terminated!"      
   abort
fi
chmod 777 $WORK


if [ "$buildflag" = "yes" ]; then
   cmdline="$cmdline -b $build_dir"
fi
if [ "$prodflag" = "yes" ]; then
   cmdline="$cmdline -p $prod_dir"
fi
if [ "$shipflag" = "yes" ]; then
   cmdline="$cmdline -s $ship_dir"
fi

cmdline="$cmdline $*"

# process ccss filenames ( now that directories are verified )
ccss_filenames=""
for ccss_filename in $*; do
   # strip any path given from each filename
   ccss_filename=`basename $ccss_filename`

   # add .ptf to each filename if not already there
   if expr "$ccss_filename" : "^.*\.ptf$" >/dev/null 2>&1 ; then
      ccss_filename="${ccss_filename}"
   else
      ccss_filename="${ccss_filename}.ptf"
   fi

   # check if file exists 
   if [ ! -f "$build_dir/$ccss_filename" ]; then
      echo "$cmd: input ptf $ccss_filename not found in $build_dir"
      echo "$cmd: Process terminated!"
      abort
   fi

   # check if file exists on ship or prod servers
   if [ -d "$ship_dir" ]; then
      if [ -f "$ship_dir/$ccss_filename" ]; then
         echo "$cmd: WARNING: PTF $ccss_filename is in $ship_dir"         
         echo "$cmd: Do you want to continue this process? <y or n> \c"
         read answer
         # if the answer is not yes and file descriptor 0 is a tty
         if [ "$answer" != "Y" -a "$answer" != "y" -a -t 0 ]; then
            echo "$cmd: Process terminated!"
            abort
         fi
      fi
   fi
   if [ -d "$prod_dir" ]; then
      if [ -f "$prod_dir/$ccss_filename" ]; then
         echo "$cmd: WARNING: PTF $ccss_filename is in $prod_dir"
         echo "$cmd: Do you want to continue this process? <y or n> \c"
         read answer
         # if the answer is not yes and file descriptor 0 is a tty
         if [ "$answer" != "Y" -a "$answer" != "y" -a -t 0 ]; then
            echo "$cmd: Process terminated!"
            abort
         fi
      fi
   fi

   ccss_filenames="$ccss_filenames $ccss_filename"
done

# Eliminate duplicates from ccss_filenames
> $WORK/ccsss
for ccss in $ccss_filenames; do
   grep "$ccss" $WORK/ccsss > /dev/null
   if [ $? = 0 ]; then
      echo "$cmd: Warning: $ccss was found on the command line twice"
      echo "$cmd: Process continuing"
   else
      echo "$ccss" >> $WORK/ccsss
   fi
done                          # awk zaps the \n's 
ccss_filenames=`cat $WORK/ccsss | awk '{printf("%s ",$0)}' `

# Default is always updating ptfapardef.constant and ptfapardef.vary files
updateConst="Y"

# determine if the prod directory matches the prod dirname for the version
echo "${prod_dir}/" | grep "${PROD_DIRNAME_PATTERN}/" > /dev/null 2>&1
rc1=$?

# determine if the build directory matches the prod dirname siblings
# for the version
echo "${build_dir}/" | grep "${PROD_DIRNAME_PATTERN}/" > /dev/null 2>&1
rc2=$?

# if the target directory is not a prod directory or the source directory is
# a sibling of the target directory, we can assume we don't need to update
# the ptfapardef.constant and ptfapardef.vary files
if [ $rc1 != 0 -o $rc2 = 0 ]; then
    updateConst="N"
fi

# However, if any of the PTFs have NO info in ptfapardef.constant then we
# DO need to update.
for ccss in $ccss_filenames; do
   ptf=`echo $ccss | sed -e 's/\.ptf$//'`
   grep "^$ptf" $PROD_DIRNAME/ptfapardef.constant > /dev/null 2>&1    
   if [ $? -ne 0 ]; then
      updateConst="Y"
      break
   fi
done

if [ $updateConst = "Y" ]; then
    # Target directory is a prod directory and the source directory
    # is not a sibling of the target directory  -or-  one or more of 
    # the PTFs have no entry in ptfapardef.constant

    # Check the ptfapardef.master file was entered on the command line
    if [ -f "$ptfapardef_table" ]; then
        ptfapardef=$ptfapardef_table
    else
        ptfapardef="/selfix/HISTORY/ptfapardef.master"
        if [ ! -f "$ptfapardef" ]; then
            if [ -z "$TOP" ]; then
                echo "TOP variable not set; Please enter your TOP value for" 
                echo "ptfapardef.master file: \c"
                read TOP
                export TOP
            fi
            ptfapardef="$TOP/HISTORY/ptfapardef.master"
            if [ ! -f "$ptfapardef" ]; then
                updateConst="N"
                # The ptfapardef.constant and ptfapardef.vary should be
                # updated, but cannot find ptfapardef.master file
                # Give the error message and exit the program
                echo "$cmd: Cannot find the ptfapardef.master file under"
                if [ $TOP/HISTORY != "/selfix/HISTORY" ]; then
                    echo "$cmd: either /selfix/HISTORY or $TOP/HISTORY"
                else
                    echo "$cmd: /selfix/HISTORY"
                fi
                echo "$cmd: Process terminated!"      
                abort
            fi
        fi
    fi
fi

if [[ "$updateConst" = "Y" ]]
then
   i=1
   while (($i<=6)) 
   do
       if [ ! -f "$PROD_DIRNAME"/lockfile ]; then
          touch $PROD_DIRNAME/lockfile
          deletelockfile="yes"
          break
       else
          echo "$cmd: Cannot gain access to update ptfapardef.constant and"
          echo "$cmd: ptfapardef.vary files at this moment."
          echo "$cmd: Sleep 20 seconds and try again." 
          sleep 20
          ((i=$i+1))
      fi
   done
   if [ ! -f "$PROD_DIRNAME"/lockfile ]; then
      echo "\n$cmd: Time out to access to ptfapardef.constant and"
      echo "$cmd: ptfapardef.vary files."
      echo "$cmd: Process terminated!"              
      echo "$cmd: Try promote_prod later!"
      abort
   fi 
fi

# Check target directory space
echo $prod_dir | grep "^/afs" > /dev/null 2>&1
if [ $? -eq 0 ]; then
   total_space=`fs lq $prod_dir | awk ' NR > 1 {print $2} '`
   used_space=`fs lq $prod_dir | awk ' NR > 1 {print $3} '`
   ((unused_space=$total_space-$used_space))
   ((unused_bytes=$unused_space*1024))
else
   $DF_CMD > $WORK/outfile
   cat $WORK/outfile | awk ' NR > 1 {print $3 " " $7}' > $WORK/outfile1
   pdir=$prod_dir
   found=0
   while [ $found = 0 ]
   do
       grep "$pdir$" $WORK/outfile1 > $WORK/outfile2
       if [ $? = 0 ]; then
          found=1
          unused_space=`cat $WORK/outfile2 | awk '{print $1}'`
          (( unused_bytes=$unused_space*1024))
       else
          pdir=`dirname $pdir`
       fi
   done
fi
# Calculate total size needs for all ptfs with their index entries
total_size=0
> $WORK/index
exitFlag="N"
for ccss in $ccss_filenames; do
    ptf_size=`ls -lL $build_dir/$ccss | awk '{print $5}'`
    ((total_size=$total_size+$ptf_size))
    ccss_name=`expr "$ccss" : "^\(.*\)\.ptf$" `
    grep "^[^:]*:$ccss_name:" $build_dir/index > $WORK/ptf_index
    if [ -s $WORK/ptf_index ]; then
       cat $WORK/ptf_index >> $WORK/index
    else
       echo "\n$cmd: No entry for PTF $ccss_name in the index file"
       echo "$cmd: under $build_dir directory."
       exitFlag="Y"
    fi
done
if [ "$exitFlag" = "Y" ]; then
   echo "$cmd: Please run gen_ptf_index command to generate the correct index"
   echo "$cmd: file under $build_dir directory."
   echo "$cmd: Ptfs have not been promoted."
   echo "$cmd: Process terminated!"
   abort 
else
   index_size=`ls -lL $WORK/ptf_index | awk '{print $5}'`
   ((total_size=$total_size+$index_size))
   if [[ $total_size -gt $unused_bytes ]]
   then
      echo "$cmd: The space of $prod_dir is not enough"
      echo "$cmd: for all ptfs with their index entrie"
      echo "$cmd: Please check with your system administrator to increase"
      echo "$cmd: the filesystem space"
      echo "$cmd: Process terminated!"
      abort
   fi
fi 

# Check working directory space
echo $WORK | grep "^/afs" > /dev/null 2>&1
if [ $? -eq 0 ]; then
   total_space=`fs lq $WORK | awk ' NR >1 {print $2}'`
   used_space=`fs lq $WORK | awk ' NR >1 {print $3}'`
   ((unused_space=$total_space-$used_space))
   ((unused_bytes=$unused_space*1024))
else
   $DF_CMD > $WORK/spacefile
   cat $WORK/spacefile | awk 'NR >1{print $3 " " $7}' > $WORK/spacefile1
   wdir=$WORK
   found=0
   while [ $found -eq 0 ]
   do
      grep "$wdir$" $WORK/spacefile1 > $WORK/spacefile2
      if [ $? -eq 0 ]; then
    	found=1
	unused_space=`cat $WORK/spacefile2 | awk '{print $1}'`
	((unused_bytes=$unused_space*1024))
      else
	wdir=`dirname $wdir`
      fi
   done
fi

# Check to see if the files are to be promoted
if [ "$execute" = "no" ]; then
   prereq_list -s$ship_dir -p$prod_dir -b$build_dir $ccss_filenames
   echo "\n$cmd: FINISHED. No files promoted to $prod_dir" 
else
   # Move the files from build to prod 
   for ccss_filename in $ccss_filenames; do
      cp -p $build_dir/$ccss_filename $prod_dir
      if [ $? != 0 ]; then
         echo -n "$cmd: error occurred moving $ccss_filename"
         echo " from $build_dir to $prod_dir"
         echo "$cmd: Process terminated!"
         abort
      fi
      if [ "$updateConst" = "Y" ]; then
         # calculate the size of the ptfapardef.constant, .vary and .ptfpkg
         const_size=`ls -lL $PROD_DIRNAME/ptfapardef.constant | awk '{print $5}'`
         vary_size=`ls -lL $PROD_DIRNAME/ptfapardef.vary | awk '{print $5}'`
         ptfpkg_size=`ls -lL $PROD_DIRNAME/ptfapardef.ptfpkg | awk '{print $5}'`
         ((total_size=$const_size+$const_size+$vary_size+$vary_size+ptfpkg_size+ptfpkg_size+5000))
         if [ $total_size -gt $unused_bytes ]; then
             echo "$cmd: The space of $BLDTMP is not enough"
             echo "$cmd: for two copies of the ptfapardef.constant,"
             echo "$cmd: ptfapardef.vary, and ptfapardef.ptfpkg files."
             echo "$cmd: Please increase space for $BLDTMP or"
             echo "$cmd: locate $BLDTMP in a different place by"
             echo "$cmd: means of the BLDTMP environment variable"
             echo "$cmd: Process terminated!"
             abort
         fi

         update_in_progress="yes"
         ptf=`echo $ccss_filename | sed -e 's/\.ptf$//'`
         grep "^$ptf" "$ptfapardef" > /dev/null 2>&1
         if [ $? -eq 0 ]; then
            grep "^$ptf" $PROD_DIRNAME/ptfapardef.constant > /dev/null 2>&1    
            if [ $? -eq 0 ]; then
               # remove any existing entries in the ptfapardef.constant
               # file for the current ptf
               while true;do
                  grep -v "^$ptf" $PROD_DIRNAME/ptfapardef.constant \
                       > $WORK/ptfapardef.constant.$$
                  [[ $? -eq 0 ]] && break
                  echo "\n$cmd: WARNING: Could not remove existing $ptf "
                  echo "$cmd: WARNING: from $PROD_DIRNAME/ptfapardef.constant"
                  echo "$cmd: WARNING: trying again !"
                  sleep 60
               done
            else
               # if no entries exist in the ptfapardef.constant
               # file for the current ptf, cp the file to .$$ so that
               # promote_prod completes successfully
               while true; do
                  cp $PROD_DIRNAME/ptfapardef.constant $WORK/ptfapardef.constant.$$  \
                     2> /dev/null
                  [[ $? -eq 0 ]] && break
                  echo "\n$cmd: WARNING: Could not copy ptfapardef.constant to"
                  echo "$cmd: WARNING: ptfapardef.constant.$$...trying again"
                  sleep 60
               done
            fi

            grep "^$ptf" $PROD_DIRNAME/ptfapardef.vary > /dev/null 2>&1    
            if [ $? -eq 0 ]; then
               # remove any existing entries in the ptfapardef.vary
               # file for the current ptf
               while true; do
	          grep -v "^$ptf" $PROD_DIRNAME/ptfapardef.vary \
                       > $WORK/ptfapardef.vary.$$
                  [[ $? -eq 0 ]] && break
		  echo "\n$cmd: WARNING: Could not remove existing $ptf"
                  echo "$cmd: WARNING: from $PROD_DIRNAME/ptfapardef.vary"
                  echo "$cmd: WARNING: trying again!"
                  sleep 60
               done
            else
               # if no entries exist in the ptfapardef.vary
               # file for the current ptf, cp the file to .$$ so that
               # promote_prod completes successfully
               while true; do
                  cp $PROD_DIRNAME/ptfapardef.vary $WORK/ptfapardef.vary.$$ \
                     2> /dev/null 
                  [[ $? -eq 0 ]] && break
                  echo "\n$cmd: WARNING: Could not copy ptfapardef.vary to"
                  echo "$cmd: WARNING: ptfapardef.vary.$$...trying again"
                  sleep 60
               done
            fi

            grep "^$ptf" $PROD_DIRNAME/ptfapardef.ptfpkg > /dev/null 2>&1    
            if [ $? -eq 0 ]; then
               # remove any existing entries in the ptfapardef.ptfpkg
               # file for the current ptf
               while true;do
                  grep -v "^$ptf" $PROD_DIRNAME/ptfapardef.ptfpkg \
                       > $WORK/ptfapardef.ptfpkg.$$
                  [[ $? -eq 0 ]] && break
                  echo "\n$cmd: WARNING: Could not remove existing $ptf "
                  echo "$cmd: WARNING: from $PROD_DIRNAME/ptfapardef.ptfpkg"
                  echo "$cmd: WARNING: trying again !"
                  sleep 60
               done
            else
               # if no entries exist in the ptfapardef.ptfpkg
               # file for the current ptf, cp the file to .$$ so that
               # promote_prod completes successfully
               while true; do
                  cp $PROD_DIRNAME/ptfapardef.ptfpkg $WORK/ptfapardef.ptfpkg.$$  \
                     2> /dev/null
                  [[ $? -eq 0 ]] && break
                  echo "\n$cmd: WARNING: Could not copy ptfapardef.ptfpkg to"
                  echo "$cms: WARNING: ptfapardef.ptfpkg.$$...trying again"
                  sleep 60
               done
            fi

            # add the ptf's entries from ptfapardef.master to both the
            # ptfapardef.constant.$$ and ptfapardef.vary.$$
	    while true; do
               grep "^$ptf" "$ptfapardef" >> $WORK/ptfapardef.constant.$$
               [[ $? -eq 0 ]] && break
	       echo "$cmd: WARNING: Could not add $ptf information to"
               echo "$cmd: WARNING: ptfapardef.constant...trying again!"
               sleep 60
            done
            grep "^$ptf" "$ptfapardef" |
            awk -F"|" '{print $1 " " $2 " " $3}' |
            while read Ptf apar defect
            do
                while true; do
                   echo "$Ptf|$apar|$defect|||||||||||||" \
                         >> $WORK/ptfapardef.vary.$$
                   [[ $? -eq 0 ]] && break
                   echo "\n$cmd: WARNING: Could not add $Ptf information to"
                   echo "$cmd: WARNING: ptfapardef.vary...trying again!"
                   sleep 60
                done
            done
             
            # add the ptf's entries from the wk_ptf_pkg file to the 
            # ptfapardef.ptfpkg file

            # get the fileset name for this ptf from the ptfoptions file
            fileset=`grep "^$ptf" $TOP/HISTORY/ptfoptions | cut -f2 -d" "`
            if [ -n "$fileset" ]
            then
               # replace the . with / in the fileset name
               filesetpath=`echo $fileset | sed -e 's:\.:/:g'`

               # add the information in the wk_ptf_pkf file to the 
               # ptfapardef.ptfpkg file
               if [ -f $TOP/UPDATE/$filesetpath/$ptf/wk_ptf_pkg ]
               then
                  while true; do
                     cat $TOP/UPDATE/$filesetpath/$ptf/wk_ptf_pkg >> $WORK/ptfapardef.ptfpkg.$$
                     [[ $? -eq 0 ]] && break
	             echo "$cmd: WARNING: Could not add $ptf information to"
                     echo "$cmd: WARNING: ptfapardef.ptfpkg...trying again!"
                     sleep 60
                  done
               else
                  echo "\n$cmd: WARNING: The file $TOP/UPDATE/$filesetpath/$ptf/wk_ptf_pkg does not exist"
                  echo "\n$cmd: WARNING: The $PROD_DIRNAME/ptfapardef.ptfpkg file was not updated for ptf $ptf"
               fi
            else
               echo "\n$cmd: WARNING: The $ptf was not found in the file $TOP/HISTORY/ptfoptions"
               echo "\n$cmd: WARNING: The $PROD_DIRNAME/ptfapardef.ptfpkg file was not updated for ptf $ptf"
            fi
            
            # handle cntl-c when updating ptfapardef.constant, .vary, or .ptfpkg
            (trap 'echo "cannot ctrl c during the updating of the ptfapardef files"' 3
            update_in_progress="restore"
            # copy the new .constant, .vary, and .ptfpkg files back to 
            # PROD_DIRNAME
	    while true; do
   	       cp $WORK/ptfapardef.constant.$$ \
                  $PROD_DIRNAME/ptfapardef.constant
               diff $WORK/ptfapardef.constant.$$ \
                  $PROD_DIRNAME/ptfapardef.constant >/dev/null 2>&1
               [[ $? -eq 0 ]] && break
	       echo "\n$cmd: WARNING: $WORK/ptfapardef.constant and "
               echo "$cmd: WARNING: $PROD_DIRNAME/ptfapardef.constant are not"
               echo "$cmd: WARNING: the same...trying cp again!"
               sleep 60
            done
            while true; do               
               cp $WORK/ptfapardef.vary.$$ $PROD_DIRNAME/ptfapardef.vary
               diff $WORK/ptfapardef.vary.$$ $PROD_DIRNAME/ptfapardef.vary \
                    >/dev/null 2>&1
               [[ $? -eq 0 ]] && break
	       echo "\n$cmd: WARNING: $WORK/ptfapardef.vary and "
               echo "$cmd: WARNING: $PROD_DIRNAME/ptfapardef.vary are not the "
               echo "$cmd: WARNING: same...trying cp again!"
               sleep 60
            done
	    while true; do
   	       cp $WORK/ptfapardef.ptfpkg.$$ \
                  $PROD_DIRNAME/ptfapardef.ptfpkg
               diff $WORK/ptfapardef.ptfpkg.$$ \
                  $PROD_DIRNAME/ptfapardef.ptfpkg >/dev/null 2>&1
               [[ $? -eq 0 ]] && break
	       echo "\n$cmd: WARNING: $WORK/ptfapardef.ptfpkg and "
               echo "$cmd: WARNING: $PROD_DIRNAME/ptfapardef.ptfpkg are not"
               echo "$cmd: WARNING: the same...trying cp again!"
               sleep 60
            done
            )  # end of trap on control c
          else
            echo "\n$cmd: PTF $ptf not found in the $ptfapardef file"
	    echo "$cmd: Did not update ptfapardef.constant, ptfapardef.vary,"
	    echo "$cmd: ptfapardef.ptfpkg files for PTF $ptf"
            echo "Continue to process next PTF!"
         fi
         update_in_progress="no"
         rm -f $WORK/ptfapardef.constant.$$ \
               $WORK/ptfapardef.vary.$$ \
               $WORK/ptfapardef.ptfpkg.$$
      fi

      ccss_name=`expr "$ccss_filename" : "^\(.*\)\.ptf$" `
      # Update index file on target directory
      grep -v "^[^:]*:$ccss_name:" $prod_dir/index > $WORK/prod_index
      mv $WORK/prod_index $prod_dir/index > /dev/null 2>&1
      grep "^[^:]*:$ccss_name:" $build_dir/index >> $prod_dir/index
      if [ $? -eq 0 ]; then
         grep -v "^[^:]*:$ccss_name:" $build_dir/index > $WORK/build.temp 
         if [ -s $WORK/build.temp ]; then
            mv $WORK/build.temp $build_dir/index  
            if [ $? -eq 0 ]; then
               rm -f $build_dir/$ccss_filename
               if [ $? -ne 0 ]; then
                  echo "$cmd: Cannot remove ptf $ccss_name from $build_dir"
                  echo "$cmd: Process terminated!"
                  abort
               fi
            else
               echo "$cmd: Fail to remove index entries for $ccss_name on"
               echo "$cmd: $build_dir directory"
               echo "$cmd: PTF $ccss_name is on $prod_dir and $build_dir"
               echo "$cmd: and the index file on $prod_dir has been updated"
               echo "$cmd: Please recover this problem"      
               echo "$cmd: Process terminated!"
               abort
            fi
         else
	    if [ ! -f $WORK/build.temp ]; then
               echo "$cmd: Fail to remove index entries for $ccss_name on"
               echo "$cmd: $build_dir directory"
               echo "$cmd: PTF $ccss_name is on $prod_dir and $build_dir"
               echo "$cmd: and the index file on $prod_dir has been updated"
               echo "$cmd: Please recover this problem"      
               echo "$cmd: Process terminated!"
               abort
            else 
               mv $WORK/build.temp $build_dir/index  
               if [ $? -eq 0 ]; then
                  rm -f $build_dir/$ccss_filename
                  if [ $? -ne 0 ]; then
                     echo "$cmd: Cannot remove ptf $ccss_name from $build_dir"
                     echo "$cmd: Process terminated!"
                     abort
                  fi
               else
                  echo "$cmd: Fail to remove index entries for $ccss_name on"
                  echo "$cmd: $build_dir directory"
                  echo "$cmd: PTF $ccss_name is on $prod_dir and $build_dir"
                  echo "$cmd: and the index file on $prod_dir has been updated"
                  echo "$cmd: Please recover this problem"      
                  echo "$cmd: Process terminated!"
                  abort
               fi
            fi
         fi
      else
         echo "$cmd: Fail to update index file on $prod_dir for $ccss_name"
         echo "$cmd: But, the ptf is on $prod_dir and $build_dir"
         echo "$cmd: Please recover the correct index file by running gen_ptf_index command"
         echo "$cmd: Process terminated!"
         abort 
      fi
   done

   if [ "$updateConst" = "Y" -a -f $PROD_DIRNAME/lockfile ]; then
      rm -f $PROD_DIRNAME/lockfile
   fi

   # Log all the promote information
   echo "`date` `hostname` `whoami` $cmd $cmdline"  >> $LOG_FILE

   if [ ! -s "$build_dir"/index ]; then
      rm -f "$build_dir"/index
   fi       

   echo "\n$cmd: FINISHED. ALL files successfully promoted" 
fi


# Remove temporary files and directories
rm -rf $WORK 
   
# Graceful exit
sync
exit 0

# END OF promote_prod # end of promote_prod #

