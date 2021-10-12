#!/bin/ksh
# @(#)79        1.21  src/bldenv/pkgtools/gendiskette.sh, pkgtools, bos41B, 9506B 1/25/95 15:39:01
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS:  abort
#               check_cmdline_parms
#               quiet_abort
#               recovery_from_interrupt
#               resolv_ptf_paths_excluding_prereqs
#               resolv_ptf_paths_including_prereqs
#               save_var_to_file
#               syntax
#
#   ORIGINS: 27
#
#   IBM INTERNAL USE ONLY
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#=======================================================================
#
#  Purpose:
#       This shell creates stacked output diskette from the specified
#       list of PTF files.  The order of search on the servers is:
#       build, production, then ship.
#
#  Syntax: see syntax function below.
#
#=======================================================================

#########################  Function ####################################
# The syntax function simply echoes command syntax, then exits the shell.
########################################################################
syntax() {
  echo "\n$cmd {-l stack_file | -D PTF_number} [-b build_dir] [-p prod_dir]"
  echo "       [-s ship_dir] [-f device] [-n dir_name] [-M maint_level]"
  echo "       [-V AIX_version] [-T ptfapardef_table] [-e] [-r] [-t] [-k]"
  echo "       [-A] [-v] [-x] [-N] [-B] [-h] [-?]\n"
  echo "FLAGS: -l  File name of stack list.  Only one PTF file name \c"
  echo " per line.\n             Default: all prereqs are included."
  echo "       -D  File name or number of a single PTF file."
  echo "             Example: U00001 or U00001.ptf"
  echo "       -A  Include only PTFs for the APAR numbers listed in the stack"
  echo "           list file (specified by the -l flag).  For version 4 PTFs"
  echo "           only.  By default, any existing prereqs are also included."
  echo "       -b  Build directory."
  echo "       -p  Production directory."
  echo "       -s  Ship directory."
  echo "       -f  Device name.  Default: fd0"
  echo "       -e  Exclude PTF prereqs (overrides default)."
  echo "            PTF files will be installed in the order specified."
  echo "       -n  Directory_name where you want the TOC to be written."
  echo "            This option will generate a TOC, but not write a diskette."
  echo "       -M  Current maintenance level on target system.  All PTFs in "
  echo "            this and earlier maintenance levels are excluded."
  echo "HIT <ENTER> TO RECEIVE THE REST OF MESSAGE"
  read
  echo "       -r  Restart the command after errors occur while writing to "
  echo "            diskette. This flag can not be used with other flags."
  echo "       -w  Directory_name where you want the temporary files generated"
  echo "            by this program to be written.  The default is /tmp."
  echo "       -v  Do not verify the diskette."
  echo "       -k  Keeps the files used to generate the diskettes."
  echo "       -t  There can be duplicate PTF's on BUILD_SVR and PROD or"
  echo "            SHIP server."
  echo "       -x  Exclude superseded images."
  echo "       -N  Do not include fixes from Problem Resolution file."
  echo "           (overrides default)"
  echo "       -B  Set both Ship directory and Product directory;"
  echo "           Default is setting Ship directory only."
  echo "       -V  Target AIX version for the diskette. "
  echo "           Default is 3 (AIX version 3)." 
  echo "       -T  The mapping table for PTF to vrmf information per fileset."
  echo "           (i.e. the ptfapardef file)  This flag must be specified when"
  echo "           both the -b and -V options are used, IF the -V option"  
  echo "           specifies AIX version 4."
  echo "    -h/-?  Prints this help screen."
  echo "\nNotes:   File and Directory names can be absolute or relative."
  echo "           Files output to diskette will not be in stack format"
  echo "           Flag -t requires -b"
  echo "           Default values for product directory and ship directory are"
  echo "           taken from 41_pkg_environment if AIX version 4 is specified."
  echo "\nExamples:"
  echo "        gendiskette -l stack_list -f fd1"
  echo "        gendiskette -D U00001 -f fd2"
  echo "        gendiskette -l ./stack_list -b build -f fd0 -e"
  echo "        gendiskette -D U00001 -w /test "
  echo "        gendiskette -r\n"

  rm -rf $WORK
  if [ $error_code != 0 ]; then
    echo "\ngendiskette FAILED!\n"
  fi
  exit $error_code
}

#########################  Function ####################################
# The 'abort' function terminates the shell and removes work files.    #
########################################################################
abort() {
  rm -rf $WORK
  echo "\ngendiskette FAILED!\n"
  exit 1
}

#########################  Function ####################################
# The quiet_abort function terminates the shell in while read loop.    #
########################################################################
quiet_abort() {
  exit 1
}


#########################  Function ###########################################
# Function to verify required files, directories, and access privileges
###############################################################################
check_cmdline_parms() {
    # Check to see if -l or -D were used
    if [ "x$l_or_D" = "x" ]; then
      echo "\n$cmd: ERROR: At least one file parameter (-l or -D) is required."
      error_code=12
    fi

    # Check production directory
    if [ "$ship_prod" = "yes" ]; then
	if [ "x$prod_dir" = "x" ]; then
	    echo "\n$cmd: ERROR: Specify the production directory with the"
	    echo "\t-p option or the PROD_DIRNAME environment variable.\n"
            error_code=14
	else
	    if [ ! -d $prod_dir ]; then
		echo "\n$cmd: ERROR: The production directory $prod_dir is invalid"
		echo "\tUse the -p option or the PROD_DIRNAME environment variable\n"
		echo "\tto specify the production directory."
		error_code=15
	    else
		if [ ! -r $prod_dir ]; then
		    echo "\n$cmd: ERROR: Could not read the production directory $prod_dir"
		    error_code=16
		else
		    # Set prod_dir to absolute path
		    cd $prod_dir
		    prod_dir=`pwd`
		    cd $CURRENT_DIR
		fi
          fi
       fi
    fi 

    # Check ship directory
    if [ "x$ship_dir" = "x" ]; then
	echo "\n$cmd: ERROR: Specify the ship directory with the"
	echo "\t-s option or the SHIP_DIRNAME environment variable.\n"
	error_code=17
    else
      if [ ! -d $ship_dir ]; then
	echo "\n$cmd: ERROR: The ship directory $ship_dir is invalid"
	echo "\tUse the -s option or the SHIP_DIRNAME environment variable\n"
	echo "\tto specify the ship directory."
        error_code=18
      else
        if [ ! -r $ship_dir ]; then
          echo "\n$cmd: ERROR: Could not read the ship directory $ship_dir"
          error_code=19
        else
          # Set ship_dir to absolute path
          cd $ship_dir
          ship_dir=`pwd`
          cd $CURRENT_DIR
        fi
      fi
    fi

    # Check for unique directory names
    if [ "x$build_dir" != "x" ]; then
      if [ $build_dir = $ship_dir ]; then
        echo "\n$cmd: ERROR: The build and ship directories are the same"
        error_code=20
      fi
      if [ "x$prod_dir" != "x" ]; then
         if [ $build_dir = $prod_dir ]; then
            echo "\n$cmd: ERROR: The build and production directories are the same"
            error_code=21
         fi
      fi 
    fi
    if [ "x$prod_dir" != "x" ]; then
       if [ $prod_dir = $ship_dir ]; then
          echo "\n$cmd: ERROR: The production and ship directories are the same"
          error_code=22
       fi
    fi

    #
    # Check that prod and ship index files are readable
    #
    if [ "x$prod_dir" != "x"  -a "$ship_prod" = "yes" ]; then
       if [ ! -f $prod_dir/index  -o  ! -r $prod_dir/index ]; then
          echo "\n$cmd: ERROR: Unable to read file '$prod_dir/index'"
          error_code=22
       fi
    fi
    if [ ! -f $ship_dir/index  -o  ! -r $ship_dir/index ]; then
       echo "\n$cmd: ERROR: Unable to read file '$ship_dir/index'"
       error_code=22
    fi

    # test to make sure flag -t and -b were used together
    if [ "$duplicate_ptf" = "yes" ]; then 
       if [ "x$build_dir" = "x" -o "x$prod_dir" = "x" ]; then
          echo "\n$cmd: ERROR: Flag -t requires -b and -B."
          error_code=40
       fi
    fi

    # Check that maintenance level file is readable
    if [ "x$current_mlevel" != "x" ]; then
       if [ ! -f $mlevel_file -o ! -r $mlevel_file ]; then
	  echo "$cmd: WARNING: $mlevel_file file not readable."
	  echo "$cmd will continue but will ignore -M flag value."
	  current_mlevel=""
       else
          # Check that maintenance level specified is valid
          grep $current_mlevel $mlevel_file > /dev/null
          if [ $? -ne 0 ]; then
	     echo "$cmd: WARNING: maintenance level $current_mlevel not found."
	     echo "$cmd will continue but will ignore -M flag value."
	     current_mlevel=""
          fi
       fi
    fi

    # Check for the ptfapardef mapping table (option -T) if the -V option
    # specified AIX version 4 AND -b option was used.
    if [ "$aix_version" = "41" -a "x$build_dir" != "x" -a "x$map_table" = "x" ]; 
    then
       echo "$cmd: ERROR: Please specify the ptfapardef mapping table (option -T)"
       error_code=11
    fi

    if [ "$aix_version" != "41" -a "$apar_fix" = "yes" ]; then
       echo "$cmd: ERROR: The -A flag can only be used for 41 PTFs.  Use"
       echo "$cmd  the -V flag to specify the correct AIX version level."
       error_code=26
    fi

}

#########################  Function ############################################
# The 'resolv_ptf_paths_excluding_prereqs'  function strips any user provided
#   information, then locates the ptf file in build, prod,
#   or ship (in that order).
################################################################################
resolv_ptf_paths_excluding_prereqs() {

   echo "$cmd: Resolving location of ptfs in the input list."

   # empty the ordered stack list
   > $ORD_STACK_LIST

   # loop through input stacklist resolving the pathname
   for  PTF_file in `cat $STACK_LIST` ; do

     # Convert PTF file names to just ptf# (no path, no .ptf)
     PTF_file=`basename $PTF_file | sed -e "s/.ptf$//"`

     # Add absolute path name to PTF file if in build
     if [ "x$build_dir" != "x" -a -f ${build_dir}/$PTF_file.ptf ]; then
       echo "${build_dir}/$PTF_file.ptf" >> $ORD_STACK_LIST
     else
       # Add absolute path name to PTF file if in prod
       if [ "x$prod_dir" != "x" -a -f ${prod_dir}/$PTF_file.ptf ]; then
         echo "${prod_dir}/$PTF_file.ptf" >> $ORD_STACK_LIST
       else
         # Add absolute path name to PTF file if in ship
         if [ -f ${ship_dir}/$PTF_file.ptf ]; then
           echo "${ship_dir}/$PTF_file.ptf" >> $ORD_STACK_LIST
         else
           #
           # handle case where ptf not found
           #
           echo "$cmd: ERROR: PTF file $PTF_file not found on:"
           # don't mention build directory if cmd was invoked without it
           if [ "x$build_dir" != "x" ]; then
             echo "$cmd:         $build_dir"
           fi
           if [ "x$prod_dir" != "x" ]; then
              echo "$cmd:         $prod_dir"
           fi
           echo "$cmd:         $ship_dir"
           if [ "x$prod_dir" = "x" -a "x$build_dir" = "x" ]; then
              echo "$cmd: ERROR: PTF(s) could be in PROD or BUILD SERVERS and in test phase"
              echo "$cmd:        Please use -p or -B option to set PROD SERVER"
              echo "$cmd:        and -b option to set BUILD SERVER"
           else
              if [ "x$prod_dir" = "x" ]; then
                 echo "$cmd: ERROR: PTF(s) could be in PROD SERVER and in test phase"
                 echo "$cmd:        Please use -p or -B option to set PROD SERVER"
              else
                 if [ "x$build_dir" = "x" ]; then
                    echo "$cmd: ERROR: PTF(s) could be in BUILD SERVER and in test phase"
                    echo "$cmd:        Please use -b option to set BUILD SERVER"
                 fi
              fi
           fi 
           abort
         fi
       fi
     fi
   done
}


#########################  Function ############################################
# The 'resolv_ptf_paths_including_prereqs'  function strips any user provided
#   information, then locates the ptf file in build, prod,
#   or ship (in that order).
################################################################################
resolv_ptf_paths_including_prereqs() {

   echo "$cmd: Resolving location and requisites of ptfs in the input list."

   if [ "x$current_mlevel" != "x" ]; then
      # User specified their current maintenance level.  Create an input file 
      # for media_list that includes all ptfs in this and earlier maintenance 
      # levels.  media_list will exclude these ptfs from the toc it builds.

      # Remove blank lines and comments from maint levels file
      sed '/^$/d' $mlevel_file >$WORK/mlevel_file.$$
      sed '/^#/d' $WORK/mlevel_file.$$ > $WORK/mfile.$$

      > $WORK/mlevels.$$
      cat $WORK/mfile.$$ | while read mfilename; do
	 cat $data_dir/$mfilename >> $WORK/mlevels.$$
	 if [ "$mfilename" = "$current_mlevel" ]; then
	   break
	 fi
      done
      # Remove blank lines and comments
      sed '/^$/d' $WORK/mlevels.$$ >$WORK/mlevels.$$.sed
      sed '/^#/d' $WORK/mlevels.$$.sed > $WORK/mlevels.$$
   fi

   cp $STACK_LIST $WORK/stacklist.in.$$

   # For 41 PTFs, make sure a complete fix is generated: find additional
   # PTFs associated with the APARS for the input PTFs.
   # If the -A flag was used, the stack list is already a list of APARs.

   if [ "$aix_version" = "41" ]; then
      > $WORK/aparlist.$$
      if [ "$apar_fix" = "yes" ]; then
        cp $STACK_LIST $WORK/aparlist.$$
        > $WORK/stacklist.in.$$
      else
        cat $STACK_LIST | while read ptfnumber; do
          grep ^$ptfnumber $prod_maptable | \
          awk -F"|" '{print $2}' >>$WORK/aparlist.$$
        done
      fi

      cat $WORK/aparlist.$$ | while read apar; do
        grep "|$apar|" $prod_maptable | \
        awk -F"|" '{print $1}' >> $WORK/stacklist.in.$$
      done
      sort -u $WORK/stacklist.in.$$ -o $WORK/stacklist.in.$$
   fi

   # Check for various flags and build media_list command

   if [ "x$build_dir" = "x" ]; then
      # No build directory specified
      CMD="media_list -l $WORK/stacklist.in.$$ -o $ORD_STACK_LIST -s $ship_dir"

      if [ "x$prod_dir" = "x" ]; then
	 CMD="$CMD -n"
      else
	 CMD="$CMD -p $prod_dir -n"
      fi
      if [ "$exclude_supersededs" = "yes" ]; then
	 CMD="$CMD -x"
      fi
      if [ "$include_peresl_file" = "yes" ]; then
	 CMD="$CMD -r $data_dir/ptfs.fixed.unfixed"
      fi
      if [ "x$current_mlevel" != "x" ]; then
	 CMD="$CMD -u $WORK/mlevels.$$"
      fi
      # Execute media_list command and check return code
      $CMD
      rc=$?
      if [ $rc = 99 ]; then
         if [ "x$prod_dir" = "x" ]; then
            echo "$cmd: ERROR: PTF(s) could be in PROD or BUILD SERVERS and in test phase"
            echo "$cmd:        Please use -p or -B option to set PROD SERVER"
            echo "$cmd:        and -b option to set BUILD SERVER"
         else
            echo "$cmd: ERROR: PTF(s) could be in BUILD SERVER and in test phase"
            echo "$cmd:        Please use -b option to set BUILD SERVER"
         fi
         abort
      else
         if [ $rc != 0 ]; then
            echo "\n$cmd: ERROR: Problem with media list"
	    echo $CMD
            abort
         fi
      fi 
   else
      # Build directory specified
      #   Create an 'index' file for the ptf library in BLDTMP 
      if [ "$aix_version" = "41" ]; then
         gen_ptf_index  -o $WORK/index  -T $map_table  $build_dir
	 rc=$?
      else
         gen_ptf_index  -o $WORK/index  $build_dir
	 rc=$?
      fi
      if [ $rc != 0 ]; then
         echo "\n$cmd: ERROR: Problem with gen_ptf_index"
         abort
      fi

      CMD="media_list -l $WORK/stacklist.in.$$ -o $WORK_STACK_LIST -s $ship_dir"

      if [ "x$prod_dir" = "x" ]; then
	 CMD="$CMD -b $WORK -n"
      else
	 CMD="$CMD -b $WORK -p $prod_dir -n"
      fi
      if [ "$exclude_supersededs" = "yes" ]; then
	 CMD="$CMD -x"
      fi
      if [ "$duplicate_ptf" = "yes" ]; then
	 CMD="$CMD -t"
      fi
      if [ "$include_peresl_file" = "yes" ]; then
	 CMD="$CMD -r $data_dir/ptfs.fixed.unfixed"
      fi
      if [ "x$current_mlevel" != "x" ]; then
	 CMD="$CMD -u $WORK/mlevels.$$"
      fi
      # Execute media_list command and check return code
      $CMD
      rc=$?
      if [ $rc = 99 ]; then
         if [ "x$prod_dir" = "x" ]; then
            echo "$cmd: ERROR: PTF(s) could be in PROD SERVER and in test phase"
            echo "$cmd:        Please use -p or -B option to set PROD SERVER"
         else
            echo "$cmd: ERROR: PTF(s) not found in BUILD, PROD, and SHIP SERVERS"
            echo "$cmd:        Please verify the $STACK_LIST file"
         fi
         abort
      else
         if [ $rc != 0 ]; then
            echo "\n$cmd: ERROR: Problem with media list"
	    echo $CMD
            abort
         fi 
      fi

      # change build ptf pathnames back to build directory (remember index
      #  was placed in the temp directory).
      # Substitution will change '$WORK____' to
      #                          '$build_dir____'
      sed  "s@$WORK\(.*\)@$build_dir\1@"  $WORK_STACK_LIST    \
              > $ORD_STACK_LIST
   fi
}

############################# Function ############################
# Function to save variables in a temp file for the use of recovery
#  from interruption
###################################################################     
save_var_to_file() {
  # get the size of the huge disk file
  fsize=`ls -l $WORK/disk | awk '{print $5}'`

  echo "WORK_DIR:$WORK" > $fn
  echo "device:$device" >> $fn
  echo "DEVICE:$DEVICE" >> $fn
  echo "volume:$volume" >> $fn
  echo "last_volume_num:$last_volume_num" >> $fn
  echo "skipcnt:$skipcnt" >> $fn
  echo "disk_file_size:$fsize" >> $fn
  echo "HDR:$HDR" >> $fn
  echo "TOC_DATE:$TOC_DATE" >> $fn
  echo "WORK_DATE:$WORK_DATE" >> $fn
  echo "keep:$keep" >> $fn
  echo "verify:$verify" >> $fn
}


#################### Function ######################################
# Function to get the values of variables saved in the temp file
#  for recovery from interruption. It checks the working space also
###################################################################
recovery_from_interrupt() {
  if [ ! -f $fn ]; then
    echo "\n$cmd: $fn does not exist.\n            Process terminated!"
    exit 1
  else
    if [ ! -s $fn ]; then
      echo "\n$cmd: $fn is empty.\n                Process terminated!"
      exit 1
    else
      line_cnt=`wc -l $fn | awk '{print $1}'`

      # there are exactly 12 lines in the file
      if [ $line_cnt -ne 12 ]; then
	echo "\n$cmd: the format of $fn is not correct."
	echo "              Process terminated!"
	exit 1
      else
	# get the values of variables
        WORK=`awk NR==1 $fn | cut -f2 -d:`
	device=`awk NR==2 $fn | cut -f2 -d:`
	DEVICE=`awk NR==3 $fn | cut -f2 -d:`
	volume=`awk NR==4 $fn | cut -f2 -d:`
	last_volume_num=`awk NR==5 $fn | cut -f2 -d:`
	skipcnt=`awk NR==6 $fn | cut -f2 -d:`
	fsize=`awk NR==7 $fn | cut -f2 -d:`
	HDR=`awk NR==8 $fn | cut -f2 -d:`
	TOC_DATE=`awk NR==9 $fn | cut -f2 -d:`
	WORK_DATE=`awk NR==10 $fn | cut -f2 -d:`
        keep=`awk NR==11 $fn | cut -f2 -d:`
        verify=`awk NR==12 $fn | cut -f2 -d:`

        if [ ! -f $WORK/disk -o ! -r $WORK/disk ]; then
	  echo "\n$cmd: ERROR: Unable to read file $WORK/disk "
	  echo "              Process terminated!"
	  exit 1
        fi
	
	# compare the current file size of $WORK/disk with the file
	# size before interrupted. If they are not equal, $WORK/disk
	# must be damaged
	cur_fs=`ls -l $WORK/disk | awk '{ print $5 }'`
	if [ $cur_fs -ne $fsize ]; then
	  echo "\n$cmd: $WORK/disk has been damaged."
	  echo "                Process terminated!"
	  exit 1
        fi
      fi
    fi
  fi
}

########################
# START OF SHELL LOGIC #
########################

#################################################
# Check the command line for least one argument #
#################################################
if [ $# -lt 1 ]; then
  error_code=1
  syntax
fi

set -u

# Is the restart mode requested?
# The r flag can not be used with other flags.
if [ $1 = "-r" -a $# -eq 1 ]; then
   restr_mode="y"
else
   restr_mode="n"
fi

########################
# Initialize variables #
########################
aix_version="32"
BLKSZ=512               # Block size for diskette media
block_max=2880          # Maximum block size
count_max=2879          # Maximum number of 512 blocks in a 2.0 MB diskette
cmd=`basename $0`       # Set the command name for error messages
toc_format="2"          # Header format: same for AIX v3.2 and AIX v4.1
execute=yes             # Execute flag

# the name of temp file for recovery from interruption
fn=/tmp/gendiskette_tmp_file_for_recovery

if [ $restr_mode = "n" ]; then
   TOC_DATE=`date +"%m""%d""%H""%M""%S""%y"`       # TOC constant date stamp
   WORK_DATE=`date +"%y""%m""%d""%H""%M""%S"`      # WORK date stamp
   BFF_DIR=""              # Bff directory
   CURRENT_DIR=`pwd`       # Current directory
   DEVICE=/dev/fd0         # Default output device
   STACK_LIST=""           # Stack list
   WORK=""                 # Work directory set to null
   build_dir=""            # Build directory set to null
   ship_dir=""             # Ship directory set to null
   prod_dir=""             # Production directory set to null
   BLDTMP="/tmp"  	   # Default temporary directory
   device=fd0              # Default device name
   error_code=0            # Error flag indicates when an error has occurred
   include_prereqs="yes"   # Default includes all prerequisites
   exclude_supersededs="no" # Default includes all supersededs
   include_peresl_file="yes" # Default includes fixes from problem resolution
			   # file. 
   l_or_D=""               # Specifies if -l or -D option was specified
   keep="NO"               # Keep flag to no
   verify="YES"            # verify flag to yes
   DLPPN=""                # directory containing toc_entry files extracted from
   	 	       	   #   the CCSS packets
   duplicate_ptf="no"      # Duplicate PTF's are not allowed
   ship_prod="no"   	   # Set ship directory only
   apar_fix="no"           # Read stack list file as PTFs, not APAR numbers
   current_mlevel=""	   # User can indicate current maintenance level to 
			   # minimize the number of requisites in output
   map_table=""	   	   # Set ptfapardef map file to null  

   #
   # check that 'media_list' program is in the path
   #
   type media_list | grep -s 'not found'
   if [ $? -eq 0 ]; then
     echo '$cmd: ERROR: "media_list" program not found in $PATH'
     abort
   fi
   
   # Check the command line options
   cmdline=$*
   set -- `getopt "l:D:b:f:n:w:p:s:M:V:T:AetkvxNBh?" $*`
   if [ $? != 0 ]; then
   # echo "\n$cmd: ERROR: Problem in command line: $cmdline"
     error_code=0
     syntax
   fi

   #####################
   # Parse the options #
   #####################
   while [ $1 != "--" ]; do
     case $1 in
       "-l")                                       # Stack list file
         shift
         l_or_D="l"
         # Dirname gives all except the last part of the path name
         # that is all but the last /
         # Set STACK_LIST to full path directory and filename using dirname cmd
         stk_dir=`dirname $1`
         if [ ! -d $stk_dir ]; then
           echo "\n$cmd: ERROR: Cannot find stack file directory $stk_dir"
           error_code=3
         else
           # Set the stack list name to the absolute path
           cd $stk_dir
           stk_dir=`pwd`
           cd $CURRENT_DIR
           stk_name=`basename $1`
           STACK_LIST=$stk_dir/$stk_name
           if [ ! -f $STACK_LIST ]; then
             echo "\n$cmd: ERROR: Cannot find stack list $STACK_LIST"
             error_code=4
           else
             if [ ! -r $STACK_LIST ]; then
               echo "\n$cmd: ERROR: The stack list $STACK_LIST is not readable"
               error_code=5
             fi
           fi
         fi;;
   
       "-D")                                       # Specify single PTF file
         shift
         l_or_D="D"
	 ptf_name="$1";; 

       "-k")                  			   # Set keep flag
	 keep="YES";;

       "-v")
         verify="NO";;                             # set verify flag
   
       "-b")                                       # Build directory
         shift
         build_dir=$1
         # Check build directory
         if [ ! -d $build_dir ]; then
           echo "\n$cmd: ERROR: The build directory $build_dir is invalid"
           error_code=6
         else
           if [ ! -r $build_dir ]; then
             echo "\n$cmd: ERROR: The build directory $build_dir is not readable"
             error_code=7
           else
             # Set build_dir to absolute path
             cd $build_dir
             build_dir=`pwd`
             cd $CURRENT_DIR
           fi
         fi;;
   
       "-p")                                       # Prod  directory
         shift
	 ship_prod="yes"
         prod_dir=$1;;

       "-s")                                       # Ship directory
         shift
         ship_dir=$1;;

       "-w")                                       # tempory directory
         shift
         BLDTMP=$1
         # Check tempory directory
         if [ ! -d $BLDTMP ]; then
           echo "\n$cmd: ERROR: The temporary directory $BLDTMP is invalid"
           error_code=6
         else
           if [ ! -r $BLDTMP ]; then
             echo "\n$cmd: ERROR: The temp directory $BLDTMP is not readable"
             error_code=7
           else 
	     if [ ! -w $BLDTMP ]; then
               echo "\n$cmd: ERROR: The temp directory $BLDTMP is not writable"
               error_code=7
             else 
	       if [ ! -x $BLDTMP ]; then
                 echo "\n$cmd: ERROR: The temp directory $BLDTMP is not executable"
                 error_code=7
               else
                 # Set BLDTMPto absolute path
                 cd $BLDTMP
                 BLDTMP=`pwd`
                 cd $CURRENT_DIR
               fi
	     fi  
           fi
         fi;;
   
       "-f")                                       # Set device
         shift
         device=`basename $1`
         DEVICE=/dev/$device;;
   
       "-M")                                       # Set current maint_level
         shift
         current_mlevel=$1;; 

       "-V")                                       # Set current AIX version
         shift
	 echo $1 | grep "^4" >/dev/null
	 if [ $? -eq 0 ]; then
            aix_version="41" 
	 fi;;

       "-T")                                       # PTF-to-vrmf mapping table
         shift
         map_table=$1
	 if [ ! -r $map_table ]; then
            echo "\n$cmd: ERROR: The ptfapardef file $map_table is not readable"
            error_code=10
 	 fi;;

       "-e")                                       # Exclude prerequisites
         include_prereqs="no";;
   
       "-A")                                       # Read stacklist as APARs
         apar_fix="yes";;
   
       "-n")                                       # No execute
          shift
          execute=no
          TOC_DIR=$1
          if [ ! -d $TOC_DIR ]; then
            echo "\n$cmd: ERROR: Can't find TOC directory $TOC_DIR"
            error_code=8
          else
            cd $TOC_DIR
            TOC_DIR=`pwd`
            if [ ! -w $TOC_DIR ]; then
              echo "\n$cmd: ERROR: TOC directory: $TOC_DIR not writable"
              error_code=9
            fi
          fi
          cd $CURRENT_DIR
          ;;

       "-t")
	  duplicate_ptf="yes";;                    # duplicate PTF's are allowed
						   # on BLD and (PROD or SHIP)
     
       "-x")
          exclude_supersededs="yes";;              # Exclude supersededs

       "-N")
          include_peresl_file="no";;               # Exclude peresl file fixes

       "-B")
          ship_prod="yes";;			   # Set ship directory and
						   # product directory

       "-h")                                       # Help
         syntax;;
   
     esac
     shift
   done
   shift
   
   
   ######################
   # Check for an error #
   ######################
   if [ $error_code != 0 ]; then
      syntax
   fi
   
   ##########################################
   # Verify and standardize input variables #
   ##########################################
   
   # Look for any additional file names at the end of the command line
   if [ $# -gt 0 ]; then
     echo "\n$cmd: ERROR: Additional parameters on command line: $*"
     error_code=13
   fi
   
   #-------------------------------------------------------------------
   # AIX bldenv uses <32 or 41>_pkg_environment to set values for     |
   # SHIP_DIRNAME & PROD_DIRNAME; but OEM customers might not use it. |
   #-------------------------------------------------------------------
   type "$aix_version"_pkg_environment >/dev/null 2>&1
   [ $? -eq 0 ] && . "$aix_version"_pkg_environment

   #------------------------------------------------------------------------
   # If ship_dir and prod_dir were not specified on the command line,      |
   # default to values set by <$aix_version>_pkg_environment.              |
   #------------------------------------------------------------------------
   if [ "x$ship_dir" = "x" ]; then
      ship_dir="$SHIP_DIRNAME"
   fi
   if [ "x$prod_dir" = "x" ]; then
      prod_dir="$PROD_DIRNAME"
   fi
   data_dir=`echo $PROD_DIRNAME | sed -e "s/prod/data/"`
   mlevel_file="$data_dir/ML.list" 		# List of maintenance levels 
   prod_maptable="$prod_dir/ptfapardef.constant"

   #------------------------------------------------------------------------
   # If ship_prod=no set prod_dir to null because we don't want media_list |
   # to get a value from the environment.				   |
   #------------------------------------------------------------------------
   [ "$ship_prod" = "no" ] && prod_dir=""

   # check integrity of cmdline parms
   check_cmdline_parms
   

   ############################################
   # Allocate temporary files and directories #
   ############################################
   # export BLDTMP variable to subprocesses 
   export BLDTMP

   # Use a directory name variable for the /tmp work directory
   WORK=$BLDTMP/gendiskette.$$
   rm -rf $WORK
   if [ -d $WORK ]; then
     echo "\n$cmd: ERROR: The work directory $WORK could not be removed"
     abort
   fi
   
   # Create the work directory
   mkdir $WORK
   if [ $? != 0 ]; then
     echo "\n$cmd: ERROR: The work directory $WORK could not be created"
     abort
   fi
   
   # chmod added to make sure that the directory can be overwritten in the future
   chmod 777 $WORK
   mkdir $WORK/bff
   mkdir $WORK/dir_lpp_name
   
   # Allocate temporary working variables
   BFF_DIR=$WORK/bff
   BASE_STACK_LST="$WORK/base_stack_list"
   HDR=$WORK/hdr                                 # Temporary header file
   ORD_STACK_LIST="$WORK/ordered_stack_list"
   TOC="$WORK/toc_without_header"
   VOLUME_TOC=$WORK/volume_toc
   export VOLUME_TOC                             # Do this for func1() in distoc.
   WORK_STACK_LIST="$WORK/stack_list"
   toc_name="$WORK/toc_with_header"
   DLPPN="$WORK/dir_lpp_name"
   
   if [ $l_or_D = "D" ];then      
      STACK_LIST="$WORK/none"
      echo "$ptf_name" > $STACK_LIST
   fi


   
   #################
   # Verify device #
   #################
   # Make sure that the device type is a diskette
   temp=`lsdev -C -c diskette -l $device` > /dev/null 2>&1
   if [ "x$temp" = "x" ]; then
     echo "\n$cmd: ERROR: Invalid diskette device name $device"
     error_code=23
   else
     # If the device type is found, make sure the diskette is available
     temp=`lsdev -C -c diskette -l $device -S a` > /dev/null 2>&1
     if [ "x$temp" = "x" ]; then
       echo "\n$cmd: ERROR: $DEVICE is a defined device, but is NOT available "
       error_code=24
     fi
   
     # Make sure that the device is writable
     if [ ! -w $DEVICE ]; then
       echo "\n$cmd: ERROR: Device $DEVICE is not writable."
       error_code=25
     fi
   fi
   
   ######################
   # Check for an error #
   ######################
   if [ $error_code != 0 ]; then
      syntax
   fi
   
   
   # ##############################################################
   # Create ordered list of PTF files and all associated prereqs #
   ###############################################################
   if [ $include_prereqs != "yes" ]; then
      # Resolve the location of the ptfs specified without changing
      #  the order
      resolv_ptf_paths_excluding_prereqs
   else
      # Resolve the location of the ptfs specified, include prereqs/etc.,
      #   and order them so a tape would stream.
      resolv_ptf_paths_including_prereqs
   fi
   
   
   ###############################################################
   # Create bff file according to the ORD_STACK_LIST             #
   ###############################################################
   cd $BFF_DIR
   cat $ORD_STACK_LIST | while read PTF_file; do
      PTF_name=`basename $PTF_file`
      echo "$PTF_name" >> $BASE_STACK_LST
      PTF_dir=`dirname $PTF_file`
      if [ "$PTF_dir" != "$ship_dir" ]; then
         echo "$PTF_name is in $PTF_dir" >> $WORK/notship
      fi
   
      # Extract the bff file and toc_entry from the ptf file
      ccss_unpack -p $PTF_name -c $PTF_file -t $DLPPN/${PTF_name}.toc > /dev/null 2>&1
      if [ $? != 0 ]; then
         echo "\n$cmd: ERROR: Problem unpacking $PTF_file with ccss_unpack"
         abort
      fi
   done
   cd $CURRENT_DIR
   
   ########################################################################
   
   echo "\nMaking a stacked diskette of the following description:\n"
   echo "  AIX VERSION:	......... $aix_version"
   echo "  BUILD DIRECTORY: ..... $build_dir"
   echo "  PRODUCTION DIRECTORY:  $prod_dir"
   echo "  SHIP DIRECTORY: ...... $ship_dir"
   echo "  INCLUDE PREREQUISITES: $include_prereqs"
   echo "  EXCLUDE SUPERSEDEDS:.. $exclude_supersededs"
   echo "  STACK LIST FILE: ..... $STACK_LIST"
   echo "  DISKETTE DEVICE: ..... $DEVICE"
   
   # Call distoc who will build the TOC
   echo "\nBuilding Table-of-Contents file"
   gendis_toc $BFF_DIR $BASE_STACK_LST $WORK/toc $WORK $WORK/stack.list $DLPPN
   #Check return code from distoc and exit if distoc did not return a zero
   if [ $? -ne 0 ]; then
     abort
   fi
   
   ########################################################################
   # Format of TOC specifies that first line contain volume # in          #
   # first character, the date created and a # for version of toc header. #
   ########################################################################
   volume=1
   echo "$volume $TOC_DATE $toc_format" > $VOLUME_TOC
   if [ $? != 0 ]; then
     echo "\n$cmd: ERROR: Unable to create table of contents $VOLUME_TOC"
     echo "\n               Check temporary working space"
     abort
   fi
   cat $WORK/toc >> $VOLUME_TOC
   
   if [ "$execute"  = "yes" ]; then
     skcount=0
     echo "\nWriting image files to temporary blocked file"
     # Read filename and 512 block count size from the sizefile
     # This data was computed in distoc by the func1 routine
     cat $WORK/sizefile | while read fil blkcnt junk; do
       # Write the toc and all the bff and or tar images to one huge file
       # dd command options used:
       # if=input file of=output file count=copy only this # of records
       # seek=seek to the nth # of records before writing bs=block size
       # conv=sync pad every input record to specified block size
       echo "\n`basename $fil`"
       dd if=$fil of=$WORK/disk count=$blkcnt seek=$skcount bs=$BLKSZ \
          conv=sync > /dev/null
       if [ $? != 0 ]; then
         echo "\n$cmd: ERROR: dd could not write $fil to temporary work space"
         abort
       fi
   
       # remove the bff file which has already been copied into the $WORK/disk
       rm -f $fil
   
       # increment the seek count by the 512 block count size
       skcount=`expr $skcount + $blkcnt`
     done
     if [ $? != 0 ]; then
       abort
     fi
   fi
else
   recovery_from_interrupt
fi



if [ "$execute"  = "yes" ]; then
  if [ $restr_mode = "n" ]; then
    skipcnt=0
    # awk gets the first field from record
    # That number is the largest disk volume number
    last_volume_num=`tail -1 $WORK/last | awk '{print$1}'`
    # Tell the user how many diskettes he will need
    echo "\nYou will need $last_volume_num diskettes for this stack set\n"
    echo "WRITING to diskette takes about 1 to 2 MINUTES each\n"
  fi

  # set trap to save variables for the use of recovery.
  trap  "save_var_to_file
	echo '\ngendiskette: the huge temporary file has been generated.  You can'
	echo '             restart the command by -r flag.'
	exit 44" 1 2 3 15

  # Process each diskette
  while [ $volume -le $last_volume_num ]; do
    # Each diskette must have a header file with this format:
    # Bytes 0-3 binary integer with value of 3609823512= x'D7298918'
    # Bytes 4-15 date and time
    # Bytes 16-19 binary integer volume # (1st diskett is x'00000001')
    # Bytes 20-511 binary zeros
    # Build diskett header file
    # This echo with a leading 0 for each octal # will echo the required
    # Binary integer value
    echo  "\0327\051\0211\030\c" > $HDR
    echo "$TOC_DATE\c" >> $HDR
    # This will generate three leading binary zeros for the volume #
    echo "\00\00\00\c" >>$HDR
    # This stmt will generate a binary integer for the value of volume
    oct_volume=`echo "$volume" | awk '{printf "%o", $1}'`
    echo "\0$oct_volume\c" >>$HDR
    # Write header record using dd by using count and conv=sync
    # byte 20-511 will have binary zeros
    echo "\nWriting header file onto temporary file"
    dd if=$HDR of=$WORK/temp_file count=1 bs=$BLKSZ conv=sync > /dev/null
    if [ $? -ne 0 ]; then
      echo "\n$cmd: ERROR: dd could not write $WORK/temp_file"
      abort
    fi
    # Write from huge temporary file to diskette size temporary file
    # seek=1 skips over the header record
    echo "\nWriting from huge file to diskette size file"
    dd if=$WORK/disk of=$WORK/temp_file count=$count_max seek=1 \
       bs=$BLKSZ skip=$skipcnt conv=sync > /dev/null
    if [ $? -ne 0 ]; then
      echo "\n$cmd: ERROR: dd could not write $WORK/temp_file"
      abort
    fi


    # Prompt for a diskette mount.
    echo "\07\nMount diskette volume $volume in $DEVICE and press enter"
    read answer

    # dd diskette sized file out to a diskette
    # bs equals one entire track (improves original performance)
    echo "\nWriting diskette size file to diskette"
    dd if=$WORK/temp_file of=$DEVICE bs=18432 > /dev/null
    while [ $? -ne 0 ]; do
      echo "\n$cmd: ERROR: dd could not write $WORK/temp_file to diskette"
      echo "\nDo you want to try again? (y/n)"
      read answer junk 
      if [ $answer = "y" -o $answer = "Y" ]; then
         echo "\07\nRemount diskette volume $volume in $DEVICE and press enter"
         read response
         echo "\nWriting diskette size file to diskette"
         dd if=$WORK/temp_file of=$DEVICE bs=18432 > /dev/null
      else	 
         abort
      fi
    done

    if [ "$verify" = "YES" ]; then
    # verify the diskette data is correct
       size=`ls -l $WORK/temp_file | awk '{print $5}'`
       blk=`expr $size / $BLKSZ`
       echo "\nWriting diskette data to the verify file"
       dd if=$DEVICE of=$WORK/verify count=$blk > /dev/null
       if [ $? -ne 0 ]; then
          echo "\n$cmd: ERROR: dd could not write to $WORK/verify file"
          echo "             Will rewrite the data to the $DEVICE and verify the diskette data again."
          continue
       fi
       echo "\nVerify the diskette"
       cmp -s $WORK/temp_file $WORK/verify > /dev/null
       if [ $? -ne 0 ]; then
          echo "\n$cmd: ERROR: data on the $DEVICE is wrong."
          echo"              Will rewrite the data to the $DEVICE again."
       else
          echo "\n**** VERIFY diskette volume $volume SUCCESS ****"
          volume=`expr $volume + 1`
          skipcnt=`expr $skipcnt + $count_max`
       fi
    else
       volume=`expr $volume + 1`
       skipcnt=`expr $skipcnt + $count_max`
    fi
  done
fi
 
# reset signals
trap 1 2 3 15

if [ -s $WORK/notship ]; then
   echo "\nFollowing PTF files are not from SHIP SERVER"
   cat $WORK/notship
fi

if [ "$execute" = "yes" ]; then
  # remove the huge $WORK/disk file that has been written to $WORK/temp_file
  # to save working space
  if [ "$keep" = "NO" ]; then
      rm -f $WORK/disk
  fi

  echo "\n$cmd SUCCESSFUL!  Please remove diskette from drive:\c"
  echo " /dev/$device\n"

  if [ $restr_mode = "y" ]; then
    # remove the temp file  used for recovery
    rm -f $fn
  fi 
else
  echo "\n$cmd SUCCESSFUL!  NO diskette was written"
  cp $VOLUME_TOC $TOC_DIR/toc.$WORK_DATE > /dev/null 2>&1
  if [ $? != 0 ]; then
    echo "\n$cmd: ERROR: Unable to write the toc file to $TOC_DIR"
    abort
  fi
  VOLUME_NAME=`basename $VOLUME_TOC`
  echo "\nNOTE: TOC file: toc.$WORK_DATE is now in $TOC_DIR\n"
fi
   
########################################################################
# remove temporary files and directories
if [ "$keep" = "NO" ]; then
   rm -rf $WORK
fi
sync
   
exit 0
# END OF SHELL
   
