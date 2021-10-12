#!/bin/ksh
# @(#)53        1.29  src/bldenv/pkgtools/gentape.sh, pkgtools, bos41B, 9506B 1/25/95 15:38:40
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS:  abort
#		check_cmdline_parms		
#		cktape_avail
#		gen_tape_stack
#		gen_tape_toc
#		quiet_abort
#		resolv_ptf_paths_excluding_prereqs
#		resolv_ptf_paths_including_prereqs
#		syntax
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

#===============================================================================
#
#  Purpose:
#       This shell creates stacked output media from the specified list of
#       PTF files.  The order of search on the servers is:  Build, production,
#       then ship.
#
#  Syntax:
#      gentape  -l stacklist [-b dirname] [-p prod_dir] [-s ship_dir] 
#               [-f device] [-w temp_dir] [-M maint_level] [-V aix_version]
#               [-T ptfapardef_file] [-A] [-e] [-m] [-t] [-x] [-N] [-B] 
#               [-h] [-?] [-1 -2]
#
#===============================================================================
trap '
rm -rf $WORK
exit  1
' 1 2 3 15

#########################  Function ############################################
# The 'syntax' function simply echos command syntax, then exits the shell.     #
################################################################################
syntax() {
  echo "$cmd  -l stack_file  [-b build_dir] [-p prod_dir] [-s ship_dir]"
  echo "      [-f device] [-w temp_dir] [-M maint_level] [-V aix_version]"
  echo "      [-T ptfapardef_file] [-A] [-e] [-m] [-t] [-x] [-N] [-B] "
  echo "      [-h] [-?] [-1 -2]\n"
  echo "FLAGS: -l  File name of stack list containing PTF file names (one" 
  echo "           per line), or, APAR numbers if the -A flag is specified."
  echo "           By default, all prereqs are included."
  echo "       -A  Include only PTFs for the APAR numbers listed in the stack"
  echo "           list file (specified by the -l flag).  For version 4 PTFs"
  echo "           only.  By default, any existing prereqs are also included."
  echo "       -b  Build directory."
  echo "       -p  Prod directory."
  echo "       -s  Ship directory."
  echo "       -f  Device name.  Default: rmt0"
  echo "       -w  Directory_name where you want the temporary files generated"
  echo "            in running the program to be written.  The default "
  echo "            directory is /tmp. "
  echo "       -M  Current maintenance level.  Exclude all PTFs in this and "
  echo "	    earlier maintenance levels."
  echo "       -e  Exclude PTF prereqs (overrides default)."
  echo "             PTF files will be installed in the order specified."
  echo "       -m  Match media format (default is AIX v3.2 format)."
  echo "       -t  There can be duplicate PTF's on BUILD_SVR and PROD or"
  echo "             SHIP server."
  echo "HIT <ENTER> TO RECEIVE THE REST OF MESSAGE"
  read
  echo "       -x  Exclude all superseded images."
  echo "       -N  Do not include fixes from Problem Resolution file."
  echo "             (overrides default)."
  echo "       -B  Set both Ship directory and Prod directory;"
  echo "           Default is setting Ship directory only."
  echo "       -V  The target AIX version for the output tape. "
  echo "           Default is '3' for AIX version 3."      
  echo "       -T  The mapping table for PTF to vrmf information per fileset."
  echo "           (i.e. the ptfapardef file)  This flag must be specified when"
  echo "           both the -b and -V options are used, IF the -V option "      
  echo "           specifies AIX version 4."  
  echo "    -h/-?  Prints this help screen."
  echo "       -1  Bosboot file name and path."
  echo "             This file is the first file on the output tape."
  echo "       -2  Bos install file name and path."
  echo "             This file is the second file on the output tape."
  echo "\nNotes:     File and Directory names can be absolute or relative."
  echo "           -1 and -2 flag must be used together. "
  echo "           -t requires -b and -B."
  echo "           Default values for product directory and ship directory are"
  echo "           taken from 41_pkg_environment if AIX version 4 is specified."
  echo "\nExamples:"
  echo "        gentape -l stack_list -m"
  echo "        gentape -l ../stack_list -f rmt1"
  echo "        gentape -l /tmp/stack_list  -f rmt2 -e -b build_dir\n"

  if [ "x$WORK" != "x" ]; then
     rm -rf $WORK
  fi
  if [ $error_code != 0 ]; then
    echo "gentape FAILED!"
  fi
  exit $error_code
}

#########################  Function ############################################
# The 'abort' function terminates the shell and removes work files.            #
################################################################################
abort() {
  rm -rf $WORK
  echo "\ngentape FAILED!"
  exit 1
}

#########################  Function ############################################
# The 'quiet_abort' function terminates the shell created by a while read loop.#
################################################################################
quiet_abort() {
  exit 1
}


#------------------------------------------------------------------------
# cktape_avail checks a defined drive to see if it is available.	|
#------------------------------------------------------------------------

cktape_avail() {
  temp=`lsdev -C -c tape -l $device -t $TYPE -S a` > /dev/null 2>&1
  if [ "x$temp" = "x" ]; then
    echo "\n$cmd: ERROR: Tape device /dev/$device defined but not available"
    error_code=25
  fi
}

#########################  Function ############################################
# Function to verify required files, directories, and access privileges
################################################################################
check_cmdline_parms() {

     # Check stacklist was provided
     if [ "x$STACK_LIST" = "x" ]; then
       echo "$cmd: ERROR: PTF_list is required (ref. -l)"
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
		    echo "$cmd: ERROR: Could not read the production directory $prod_dir"
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
           echo "$cmd: ERROR: Could not read the ship directory $ship_dir"
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
         echo "$cmd: ERROR: The build and ship directories are the same"
         error_code=20
       fi
       if [ "x$prod_dir" != "x" ]; then
          if [ $build_dir = $prod_dir ]; then
             echo "$cmd: ERROR: The build and production directories are the same"
             error_code=21
          fi
       fi
     fi
     if [ "x$prod_dir" != "x" ]; then
        if [ $prod_dir = $ship_dir ]; then
           echo "$cmd: ERROR: The production and ship directories are the same"
           error_code=22
        fi
     fi

     #
     # Check that prod and ship index files are readable
     #
     if [ "x$prod_dir" != "x" -a "$ship_prod" = "yes" ]; then
        if [ ! -f $prod_dir/index  -o  ! -r $prod_dir/index ]; then
           echo "\n$cmd: ERROR: Unable to read file '$prod_dir/index'"
           error_code=22
        fi
     fi
     if [ ! -f $ship_dir/index  -o  ! -r $ship_dir/index ]; then
        echo "\n$cmd: ERROR: Unable to read file '$ship_dir/index'"
        error_code=22
     fi

     # Test to make sure flag -1 and -2 were used together
     if [ "$b1" = "yes" -a "$b2" = "yes" ]; then
       # Set flags that specify that the tape should be bootable
       # that is that files will be written to file 1 and 2 of the output tape
       bootable=yes
       export BOSBOOT
       export BOSINSTAL
     else
       if [ "$b1" = "yes" -o "$b2" = "yes" ]; then
        echo "$cmd: ERROR: FLAG -1 and -2 must be used together"
        error_code=23
       fi
     fi
     
     # Test to make sure flag -t and -b were used together
     if [ "$duplicate_ptf" = "yes" ];then
        if [ "x$build_dir" = "x" -o "x$prod_dir" = "x" ]; then
           echo "$cmd: ERROR: FLAG -b and -B are required for -t"
           error_code=40
        fi
     fi

     # Make sure the maint_levels file is readable
     if [ "x$current_mlevel" != "x" ]; then
        if [ ! -f $mlevel_file -o ! -r $mlevel_file ]; then
	   echo "$cmd: WARNING: $mlevel_file file not readable."
	   echo "$cmd continuing; will ignore -M flag value."
	   current_mlevel=""
        else
           # Check that maintenance level specified is valid
	   grep $current_mlevel $mlevel_file > /dev/null
	   if [ $? -ne 0 ]; then
	      echo "$cmd: WARNING: maintenance level $current_mlevel not found."
	      echo "$cmd continuing; will ignore -M flag value."
	      current_mlevel=""
	   fi
	fi
     fi

    # Check for the ptfapardef mapping table (option -T) if the -V option
    # specified AIX version 4 AND -b option was used.
    if [ "$aix_version" = "41" -a "x$build_dir" != "x" -a "x$map_table" = "x" ]; 
    then
       echo "$cmd: ERROR: Please specify the ptfapardef mapping table (option -T)"
       error_code=26
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
      # User specified their current maintenance level.  Create an input 
      # file for media_list that includes all ptfs in this and earlier 
      # maintenance levels.  media_list will exclude these ptfs from the 
      # toc it builds.

      # Remove blank lines and comments from maint levels file
      sed '/^$/d' $mlevel_file > $WORK/mlevel_file.$$
      sed '/^#/d' $WORK/mlevel_file.$$ > $WORK/Mfile.$$

      > $WORK/mlevels.$$
      cat $WORK/Mfile.$$ | while read mfilename; do
	 cat $data_dir/$mfilename >> $WORK/mlevels.$$
	 if [ "$mfilename" = "$current_mlevel" ]; then
	    break 
	 fi
      done
      # Remove blank lines and comments
      sed '/^$/d' $WORK/mlevels.$$ > $WORK/mlevels.$$.sed
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
      if [ "x$current_mlevel" != "x" ]; then
	 CMD="$CMD -u $WORK/mlevels.$$"
      fi
      if [ "$include_peresl_file" = "yes" ]; then
	 CMD="$CMD -r $data_dir/ptfs.fixed.unfixed"
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
      #   Create an 'index' file for the ptf library in temp_dir
      if [ "$aix_version" = "32" ]; then
         gen_ptf_index  -o $WORK/index  $build_dir
	 rc=$?
      else
         gen_ptf_index  -o $WORK/index  -T $map_table  $build_dir
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


#########################  Function ############################################
# The gen_tape_toc function creates a table of contents using toc fragments    #
# and sizes provided.                                                          #
################################################################################
gen_tape_toc() {
  # Inform the user of the run parameters
  echo "\nMaking a stacked tape of the following description:\n"
  if [ $TYPE = "150mb" ]; then
    echo "  TAPE TYPE: .............. 1/4 inch"
  else
    if [ $TYPE = "8mm" ]; then
       echo "  TAPE TYPE: .............. 8mm"
    else
       if [ $TYPE = "4mm2gb" ]; then
          echo "  TAPE TYPE: .............. 4mm"
       else
          if [ $TYPE = "8mm5gb" ]; then
             echo "  TAPE TYPE: .............. 8mm5gb"
          else
             echo "  TAPE TYPE: .............. 1/4 inch 525 mb tape"
          fi
       fi
    fi
  fi
  echo "  TAPE DEVICE: ............ /dev/$device"
  if [ $toc_format = "2" ]; then
    echo "  MEDIA FORMAT: ........... AIX v3.2"
  else
    echo "  MEDIA FORMAT: ........... AIX v3.2 Match Media"
  fi
  echo "  BUILD DIRECTORY: ........ $build_dir"
  echo "  PRODUCTION DIRECTORY: ... $prod_dir"
  echo "  SHIP DIRECTORY: ......... $ship_dir"
  echo "  STACK LIST FILE: ........ $STACK_LIST"
  echo "  INCLUDE PREREQUISITES: .. $include_prereqs"
  echo "  EXCLUDE SUPERSEDEDS: .... $exclude_supersededs"
  echo "  BOOTABLE: ............... $bootable"

  # Initialize volume number and size
  volume=1
  volume_size=0

  # Open the volume file
  echo $volume > $WORK/volume.$$

  # Start the file count at 4 since the first files on each tape are:
  # bosboot, bosinstall and TOC files
  file_count=4

  # Prompt for a tape mount.
  echo "\07\nMount volume $volume in /dev/$device and press enter"
  read answer
  echo "Now wait for rewind to complete ..."
  tctl -f$DEVICE rewind > /dev/null
  if [ ! $? = 0 ]; then
    echo "\n$cmd: ERROR: Tape device is not ready"
    abort
  fi

  echo "\nBuilding a Table of Contents file:\n"

  # For each PTF file, find the size of the file, determine what volume it
  # belongs on, add its name to the appropriate stack list, and add its toc
  # fragment to the table of contents.
  cat $ORD_STACK_LIST | while read PTF_file; do
    PTF_name=`basename $PTF_file`
    ccss_unpack -t $WORK/lpp_name.$PTF_name -s $WORK/size.$PTF_name -c $PTF_file
    if [ $? != 0 ]; then
      echo "\n$cmd: ERROR: Problem extracting toc or size file from $PTF_file"
      abort
    fi

    # Check size of image - if it's not blocked at 31744, tell user, and
    # recompute image size, rounding up to nearest multiple of 31744.
    file_size=`cat $WORK/size.$PTF_name`        # Determine file size
    file_size=`expr $file_size + 0`
    file_size_string=$file_size
    difs=`expr $file_size % $BLOCK_SIZE`
    if [ $difs != 0 ]; then
      file_size=`expr $file_size + $BLOCK_SIZE - $difs`
      adj_string="(adjusted to $file_size)"
    else
      adj_string=""
    fi

    # Keep track of volume size in a temporary file (only way I knew how
    # to overcome passing variables between shells)
    volume_size=`expr $volume_size + $file_size`
    echo $volume_size > $WORK/volume_size.$$

    # Check to see if the file will fit on the volume
    if [ $volume_size -gt $maxsize ]; then
      volume_size=`expr $volume_size - $file_size`
      echo "Total size of images on volume $volume is $volume_size bytes\n"
      volume_size=$file_size
      volume=`expr $volume + 1`
      echo $volume > $WORK/volume.$$
      echo $volume_size > $WORK/volume_size.$$
      file_count=4
    fi

    # Print the file on the current volume
    echo "Size of $PTF_name:    $file_size_string       $adj_string"

    # Add the current file onto the current volume's stack list
    echo $PTF_file >> $WORK/stack_list.$volume

    # Insert appropriate header information, then append the contents of the
    # current PTF_file toc fragment to the toc
    # The header record needs to have  volume:count:size info.
    # All other records remain the same.
    echo "$volume:$file_count:$file_size \c" >> $TOC
    #   Append the contents of the current lpp_name file to the toc
    cat ${WORK}/lpp_name.${PTF_name}         >> $TOC

    # Increment the number of files found in this volume
    file_count=`expr $file_count + 1`

    # Remove work files
    rm ${WORK}/size.${PTF_name}
    rm ${WORK}/lpp_name.${PTF_name}

  done
  # Check return code from while loop (when while loop was invoked, a
  # new shell program was started)
  if [ $? != 0 ]; then
    quiet_abort
  fi

  # Get the volume_size from the temporary file
  volume_size=`awk '{ print $1 }' $WORK/volume_size.$$ `
  volume=`awk '{ print $1 }' $WORK/volume.$$ `
  echo "Total size of images on volume $volume is $volume_size bytes\n"
}

#########################  Function ############################################
# The gen_tape_stack function outputs all files to tape                        #
################################################################################
gen_tape_stack() {
  # Process each volume from the appropriate stack list
  volume=1
  while [ -f $WORK/stack_list.$volume ]; do

    # Prompt for a tape mount for any tape after first volume
    if [ $volume != 1 ]; then
      echo "\07\nMount volume $volume in /dev/$device and press enter"
      read answer
      tctl -f$DEVICE rewind > /dev/null
      if [ ! $? = 0 ]; then
        echo "\n$cmd: ERROR: Tape device is not ready"
        abort
      fi
    fi

    # NOTE: If the tape is to be bootable, then put the boot record
    # and install image on the tape first.  Else, put the reserve1
    ## and reserve2 files on first.
    if [ $bootable = "yes" ]; then
      for x in $BOSBOOT $BOSINSTAL; do
        echo ""
        echo `basename $x`
        dd if=$x of=$DEVICE bs=$BLOCK_SIZE conv=sync
        if [ $? -ne 0 ]; then
          echo "\n$cmd: ERROR: Problem writing boot files to tape using dd"
          abort
        fi
      done
    else
      # For tapes that are non-bootable, they must have placeholders for
      # the boot files.
      for x in Reserved_file_1 Reserved_file_2; do
        echo ""
        echo `basename $x`
        echo "This is a reserved file." | dd of=$DEVICE bs=$BLOCK_SIZE conv=sync
        if [ $? -ne 0 ]; then
          echo "\n$cmd: ERROR: Problem writing reserved files to tape using dd"
          abort
        fi
      done
    fi

    ##########################################################################
    # Format of the TOC specifies that first line should contain volume # in #
    # first character, the date created and a # for version of toc header.   #
    # TOC for all volumes is same but for 1st line.                          #
    ##########################################################################
    # Put the header information into the new toc
    echo "$volume $TOC_DATE $toc_format" > $toc_name
    if [ $? != 0 ]; then
      echo "\n$cmd: ERROR: TOC file $toc_name could not be created"
      abort
    fi

    # Add the rest of the table of contents created by gen_tape_toc
    cat $TOC >> $toc_name

    # Output the table of contents out to the tape
    echo "\nTable of contents"
    dd if=$toc_name of=$DEVICE bs=512 conv=sync > /dev/null
    if [ $? != 0 ]; then
      echo "\n$cmd: ERROR: Problem writing table of contents to tape using dd"
      abort
    fi

    # Output each PTF file in stack list to the tape
    for PTF_file in `cat $WORK/stack_list.$volume`; do
      echo ""
      echo `basename $PTF_file`
      PTF_dir=`dirname $PTF_file`
      if [ "$PTF_dir" != "$ship_dir" ]; then
         echo "`basename $PTF_file` is in $PTF_dir" >> $WORK/notship
      fi
      ccss_unpack -c $PTF_file -p $WORK/PTF_file
      if [ $? != 0 ]; then
        echo "\n$cmd: ERROR: Problem unpacking $PTF_file with ccss_unpack"
        abort
      fi
      dd if=$WORK/PTF_file of=$DEVICE bs=$BLOCK_SIZE conv=sync > /dev/null
      if [ $? != 0 ]; then
        echo "\n$cmd: ERROR: Problem writing $PTF_file to tape using dd"
        abort
      fi
    done
    rm $WORK/PTF_file

    # After last PTF file is written, write an end of file mark
    tctl -f$DEVICE weof > /dev/null
    if [ ! $? = 0 ]; then
      echo "\n$cmd: ERROR: Tape device is not ready"
      abort
    fi

    # Rewind the tape
    tctl -f$DEVICE rewind > /dev/null
    if [ ! $? = 0 ]; then
      echo "\n$cmd: ERROR: Tape device is not ready"
      abort
    fi

    # Increment the volume number
    volume=`expr $volume + 1`
  done
}

################################################################################
# START OF SHELL LOGIC.  First, check number of parameters on command line.    #
################################################################################

set -u

########################
# Initialize variables #
########################
aix_version="32"
build_dir=""            # Build directory set to null
ship_dir=""             # Ship directory set to null
prod_dir=""             # Production directory set to null
TOC_DATE=`date +"%m""%d""%H""%M""%S""%y"`       # Constant date stape
WORK_DATE=`date +"%y""%m""%d""%H""%M""%S"`      # WORK date stape
CURRENT_DIR=`pwd`       # Current directory
STACK_LIST=""           # Stack list file name
toc_format="2"          # Header format: AIX v3.2
device=rmt0             # Default output device
DEVICE=/dev/rmt0        # Default output device
type8="8mm"             # 8mm tape description from lsdev command
type5="8mm5gb"		# 8mm5gb tape description from lsdev command
type4="150mb"           # 1/4 inch tape description from lsdev command
typeo="ost"		# Other SCSI tape description
typeT="525mb"		# 3.2 version of Tundra drive
typeM="4mm2gb"		# 4mm tape drive
include_prereqs="yes"   # Default includes all prerequisites
exclude_supersededs="no" # Default excludes all supersededs
include_peresl_file="yes" # Default includes fixes from problem resolution file
BLOCK_SIZE=31744        # Default block size for tape media
got_size="no"           # Did the user specify a new block size?
b1="no"                 # Bos install not specified
b2="no"                 # Bos boot not specified
bootable="no"           # If both b1 and b2 are "yes" the tape is bootable
error_code=0            # Error flag indicates when an error has occurred
cmd="gentape"           # Set the command name for error messages prior to
                        # checking for at least one argument
file_size=0             # Initial file size
duplicate_ptf="no"      # duplicate PTF's are not allowed 
ship_prod="no"		# Set ship directory only
apar_fix="no"		# Read stacklist as PTF filenames, not APAR numbers
BLDTMP="/tmp"		# Set temporary working directory
WORK=""
current_mlevel=""       # User can indicate current maintenance level to 
			# minimize the number of requisites in output
map_table=""		# Set ptfapardef map file to null

#
# check that 'media_list' program is in the path
#
type media_list | grep -s 'not found'
if [ $? -eq 0 ]; then
  echo '$cmd: ERROR: "media_list" program not found in $PATH'
  abort
fi
#

#################################################
# Check the command line for least one argument #
#################################################
cmd=`basename $0`
if [ $# -lt 1 ]; then
  error_code=1
  syntax
fi

# Check the command line options
cmdline=$*
set -- `getopt "l:b:f:1:2:p:s:w:M:V:T:AetmxNBh?" $*`
if [ $? != 0 ]; then
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

    "-p")                                       # Prod directory
      shift
      ship_prod="yes"
      prod_dir=$1;;

    "-s")                                       # Ship directory
      shift
      ship_dir=$1;;
 
    "-w")
      shift 					# Temporary working directory
      BLDTMP=$1
      if [ ! -d $BLDTMP ]; then
         echo "\n$cmd: ERROR: The temporary directory $BLDTMP is invalid"
         error_code=8
      else
         if [ ! -r $BLDTMP ]; then
            echo "\n$cmd: ERROR: The temp directory $BLDTMP is not readable"
            error_code=9
         else
            if [ ! -w $BLDTMP ]; then
               echo "\n$cmd: ERROR: The temp directory $BLDTMP is not writable"
               error_code=10
            else
               if [ ! -x $BLDTMP ]; then
                  echo "\n$cmd: ERROR: The temp directory $BLDTMP is not executable"
                  error_code=11
               else
                  # Set BLDTMP to absolute path
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

    "-M")					# Set current maint level
      shift
      current_mlevel=$1;;

    "-V")					# Set current current AIX version
      shift
      echo $1 | grep "^4" >/dev/null
      if [ $? -eq 0 ]; then
         aix_version="41"
      fi;;

    "-T")					# PTF-to-vrmf mapping table
      shift
      map_table=$1
      if [ ! -r $map_table ]; then
         echo "\n$cmd: ERROR: The ptfapardef file $map_table is not readable"
         error_code=28
      fi;;

    "-e")                                       # Exclude prerequisites
      include_prereqs="no";;

    "-m")                                       # AIX v3.2 match media format
      toc_format="2M";;
   
    "-t")
      duplicate_ptf="yes";;                     # duplicate PTF's are allowed
						#  on BLD and (PROD or SHIP)

    "-x")
      exclude_supersededs="yes";;               # Exclude supersededs

    "-N")
      include_peresl_file="no";;               # Exclude fixes from peresl file

    "-B")
      ship_prod="yes";;				# Set ship directory and 
						# prod directory

    "-A")
      apar_fix="yes";;				# Read stacklist as APARs

    "-h")                                       # Help
      syntax;;

    "-1")
      # set BOSBOOT to full path directory/filename using dirname cmd
      # also set the b1 flags which will be used later to make sure
      # that flag 1 and 2 were used together
      shift
      boot_dir=`dirname $1`
      if [ ! -d $boot_dir ]; then
        echo "\n$cmd: ERROR: Cannot find bos boot directory $boot_dir"
        error_code=12
      else
        cd `dirname $1`
        boot_dir=`pwd`
        boot_name=`basename $1`
        BOSBOOT=$boot_dir/$boot_name
        if [ ! -f $BOSBOOT ]; then
          echo "\n$cmd: ERROR: Cannot find bos boot image $BOSBOOT"
          error_code=13
        fi
      fi
      b1=yes
      cd $CURRENT_DIR;;

    "-2")
      # set BOSINSTAL to full path directory/filename using dirname cmd
      # also set the b2 flag which will be used later to make sure
      # that flag 1 and 2 were used together
      shift
      bos_instal_dir=`dirname $1`
      if [ ! -d $bos_instal_dir ]; then
        echo "\n$cmd: ERROR: Cannot find bos install directory $bos_instal_dir"
        error_code=14
      else
        cd `dirname $1`
        bos_instal_dir=`pwd`
        bos_instal_name=`basename $1`
        BOSINSTAL=$bos_instal_dir/$bos_instal_name
        if [ ! -f $BOSINSTAL ]; then
          echo "\n$cmd: ERROR: Cannot find bos install image $BOSINSTAL"
          error_code=15
        fi
      fi
      b2=yes
      cd $CURRENT_DIR;;
  esac
  shift
done
shift

#-------------------------------------------------------------------
# AIX bldenv uses <32 or 41>_pkg_environment to set values for     |
# SHIP_DIRNAME & PROD_DIRNAME; but OEM customers might not use it. |
#-------------------------------------------------------------------
type "$aix_version"_pkg_environment >/dev/null 2>&1
[ $? -eq 0 ] && . "$aix_version"_pkg_environment

#-------------------------------------------------------------------
# If ship_dir and prod_dir were not specified on the command line, |
# default to values set by <$aix_version>_pkg_environment.         |
#-------------------------------------------------------------------
if [ "x$ship_dir" = "x" ]; then
   ship_dir="$SHIP_DIRNAME"
fi
if [ "x$prod_dir" = "x" ]; then
   prod_dir="$PROD_DIRNAME"
fi
data_dir=`echo $prod_dir | sed -e "s/prod/data/"`
mlevel_file="$data_dir/ML.list"  		# List of maintenance levels
prod_maptable="$prod_dir/ptfapardef.constant"

#------------------------------------------------------------------------
# If ship_prod=no set prod_dir to null because we don't want media_list |
# to get a value from the environment.					|
#------------------------------------------------------------------------
[ "$ship_prod" = "no" ] && prod_dir=""

##########################################
# Verify and standardize input variables #
##########################################

# Look for any additional file names at the end of the command line
if [ $# -gt 0 ]; then
  echo "$cmd: ERROR: Additional parameters on command line: $*"
  error_code=16
fi
 
# check integrity of cmdline parms
check_cmdline_parms


#################
# Verify device #
#################
case $device in
  rmt*) # 8mm, 1/4 inch streaming tape or 7207 tape
    # Determine if device name and type are found
    # Here is a sample output of lsdev -C -c tape:
    # rmt0 Available 00-08-60-00 2.3 8mm tape drive
    temp=`lsdev -C -c tape -l $device` > /dev/null 2>&1
    if [ "x$temp" = "x" ]; then
      echo "$cmd: ERROR: Invalid tape device: $device"
      error_code=24
    else
      # Determine if type is a 1/4 inch tape, 8mm tape or 7207 tape
      temp=`lsdev -C -c tape -l $device -t $type8` > /dev/null 2>&1
      if [ "x$temp" != "x" ]; then
        TYPE=$type8
        # Determine maximum size
        if [ "$got_size" = "no" ]; then
          maxsize=2100000000
        else
          if [ $maxsize -gt 2100000000 ]; then
            echo "$cmd: ERROR: Maximum size for an 8mm tape is 2100 megs"
            error_code=25
          fi
        fi
	cktape_avail
      else
        temp=`lsdev -C -c tape -l $device -t "$type4"` > /dev/null 2>&1
        if [ "x$temp" != "x" ]; then
          TYPE=$type4
          # Determine maximum size
          if [ $got_size = no ]; then
            maxsize=120000000
          else
            if [ $maxsize -gt 120000000 ]; then
              echo "$cmd: ERROR: Maximum size for a 1/4 inch tape is 120 megs"
              error_code=27
            fi
          fi
	  cktape_avail
        else
          temp=`lsdev -C -c tape -l $device -t "$typeo"` > /dev/null 2>&1
          if [ "x$temp" != "x" ]; then
             TYPE=$typeo
             # Determine maximum size
             if [ $got_size = no ]; then
                maxsize=525000000
             else
               if [ $maxsize -gt 525000000 ]; then
                  echo "$cmd: ERROR: Maximum size for 7207 tape is 525 megs"
                  error_code=40
               fi
             fi
	     cktape_avail
          else 
		#----------------------------------------
		# 3.2 version of Tundra (525) drive.    |
		#----------------------------------------
		temp=`lsdev -C -c tape -l $device -t $typeT` > /dev/null 2>&1
		if [ "x$temp" != "x" ]; then
		    TYPE=$typeT
		    # Determine maximum size
		    if [ "$got_size" = "no" ]; then
		        maxsize=525000000
		    else
			if [ $maxsize -gt 525000000]; then
			    echo "\n$cmd: ERROR: Maximum size for a 7207 tape is 525 megs"
			    error_code=24
			fi
		    fi
		    cktape_avail
		else
                    # 4mm2gb tape drive
                    temp=`lsdev -C -c tape -l $device -t $typeM` > /dev/null 2>&1
                    if [ "x$temp" != "x" ]; then
                       TYPE=$typeM
                       # Determine maximum size
                       if [ "$got_size" = "no" ]; then
                          maxsize=2000000000
                       else
                          if [ "$maxsize" -gt 2000000000 ]; then
                             echo "\n$cmd: ERROR: Maximum size for 4mm tape is 4GB"
                             error_code=24
                          fi
                          cktape_avail
                       fi
                    else
                       # 8mm5gb tape drive
                       temp=`lsdev -C -c tape -l $device -t $type5` > /dev/null 2>&1
                       if [ "x$temp" != "x" ]; then
                          TYPE=$type5
                          # Determine maximum size
                          if [ "$got_size" = "no" ]; then
                             maxsize=2100000000
                          else
                             if [ "$maxsize" -gt 2100000000 ]; then
                                echo "\n$cmd: ERROR: Maximum size for 8mm5gb tape is 2.3GB"
                                error_code=24
                             fi
                             cktape_avail
                          fi
                       else
                          echo "$cmd: ERROR: Device /dev/$device not 8mm, 1/4 inch tape, 7207 tape, 8mm5gb or 4mm"
                          error_code=29
                          syntax
                       fi
                    fi
		fi
          fi
        fi
      fi
      # Check the block size of device.  It must be 512.
      tapebs=`lsattr -E -a block_size -l $device | cut -d' ' -f2`
      if [ "$tapebs" != "512" ]; then
          echo "$cmd: ERROR: Block size on tape device must be set to 512"
          echo "       Use 'smit tape' to set block size to 512 or use"
          echo "       chdev -l $device -a block_size=512"
          error_code=30
      fi
    fi

    # Set DEVICE to /dev/devicename.5 or /dev/devicename.1
    # Both devices do not rewind the tape)
    if [ "$TYPE" = "$typeo" -o "$TYPE" = "$typeT" ]; then
       DEVICE=/dev/$device.1
    else
       DEVICE=/dev/$device.5
    fi;;


  *)
    echo "$cmd: ERROR: The device $device is not recognized."
    error_code=39;;
esac

######################
# Check for an error #
######################
if [ $error_code != 0 ]; then
   syntax
fi

############################################
# Allocate temporary files and directories #
############################################
# export BLDTMP for its subprocesses.
export BLDTMP

# Use a directory name variable for the /tmp work directory
WORK=$BLDTMP/gentape_work.$$
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

# Allocate temporary working variables
toc_name="$WORK/toc_with_header"
TOC="$WORK/toc_without_header"
WORK_STACK_LIST="$WORK/stack_list"
ORD_STACK_LIST="$WORK/ordered_stack_list"

###############################################################
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



########################################################
# Generate the media specific toc and output all files #
########################################################
case $device in
   rmt*) # 8mm, 1/4 inch streaming tape, 7207 tape or 4mm tape
      # Generate a toc of all ptf files
      gen_tape_toc

      # Output stacked files to tape
      gen_tape_stack

      # Tell the user that gentape was successful
      echo "\nTo get a copy of the table of contents you can use the command:"
      echo "     dd if=/dev/$device of=table_of_contents fskip=2"
      echo "\nPlease remove tape from drive: /dev/$device";;


   *)
      echo "$cmd: ERROR: The device $device is not implemented."
      abort;;
esac
  
if [ -s $WORK/notship ]; then
   echo "\nFollowing PTF files are not from SHIP SERVER"
   cat $WORK/notship
fi

# Graceful exit
echo "\ngentape was SUCCESSFUL!"
rm -rf $WORK
sync
exit 0
