#!/bin/sh
# COMPONENT_NAME: kill_mif
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
# Purpose: Deleted specified mifs.
#
# Usage:
#       kill_mif -b Build_dirname -p Prod_dirname -k killed_list \\
#                 {-n} {-h} {-?} MIF_filenames
#             where:
#               Build_dirname - directory name of the mif library
#                                of the build server (required).
#               Prod_dirname  - directory name of the mif library
#                                of the production server.
#               MIF_filenames - filenames of the mif that will
#                                be deleted.
#                               This will allow execution from Motif
#                                or EMACS.
#               killed_list   - output file containing list of
#                                deleted filenames for use
#                                by build_tools.
#               -n    option to just list the affected mifs
#                      (aka. NO_EXECUTE)
#               -h | -?   options to list usage message and exit
#                           regardless of other options specified.
#
# Change log:
#    07/22/91-spm-changed to use 32_pkg_environment var's
#    07/30/90-spm-changed as specified in code review
#    09/09/91-ag-Changed "rm MIF" file to renaming it to MIF.old
#    01/29/92-wt-Removed informational messages.
######################################################################

#####################################################################
#  syntax is called when parameters are entered incorrectly
#  or if help is requested
#####################################################################
syntax() {
    echo ""
    echo "$SCCSID"
    echo "Usage: "
#   echo " kill_mif [-b Build_dirname] [-p Prod_dirname] -k killed_list"
    echo ' kill_mif [-b Build_dirname] [-p Prod_dirname]  \'
    echo "          [-n] [-h] [-?] MIF_filenames"
    echo ""
    echo "     where:"
    echo "           Build_dirname - directory name of the mif library"
    echo "                            of the build server."
    echo "           Prod_dirname  - directory name of the mif library"
    echo "                            of the production server."
    echo "           MIF_filenames - filenames of the mifs that will"
    echo "                            be deleted."
#   echo "           killed_list   - output file containing list of"
#   echo "                            deleted filenames for use"
#   echo "                            by build_tools."
    echo "           -n    option to just list the affected mifs"
    echo "                  (aka. NO_EXECUTE)"
    echo "           -h | -?   options to list usage message and exit"
    echo ""

    exit 1
}

#######################################################################
#  Abort is called when an error occurs during kill
#######################################################################
abort() {
    cd $start_dir
    rm -rf $WORK
    rm -f $killed_list
    exit 1
}

#######################################################################
#######################################################################
#############   Beginning of kill mif #################################
#######################################################################
#######################################################################

# Initialize variables.

set -u  # undeclared var's are errors
SCCSID="@(#)02        1.4  src/bldenv/pkgtools/rename/kill_mif.sh, pkgtools, bos412, GOLDA411a 2/6/92 00:22:46"
help_request="n"
execute="y"
build_dir=""
#killed_list=""
killed_list=/tmp/kill_mif.$$.`date +"%y""%m""%d""%H""%M""%S"`.kill_list
err_code=0
start_dir=`pwd`

# Setup default prod directory
. 32_pkg_environment
prod_dir=$PROD_DIRNAME

# Check if no parms were given
if [ $# -eq 0 ]; then
  syntax
fi
stored_command="kill_mif $*"

#  Check the command line

set -- `getopt "b:p:k:nh?" $*`
if [ $? != 0 ]; then   # getopt failed because of a bad parameter flag
   syntax
fi

while [ $1 != "--" ]; do                # Set vars, based on params.
    case $1 in
        -b)
            build_dir=$2;               shift 2;;
        -p)
            prod_dir=$2;                shift 2;;
        -k)
            killed_list=$2;             shift 2;;
        -n)
            execute="n";                shift;;
        -[?h])
            help_request="y";           shift;;
    esac
done
shift                                   # Get past the '--' flag.

# Validate the parameters

if [ "$help_request" = "y" ]; then
   syntax
fi

# Check build directory  (if given)
if [ "$build_dir" != "" ]; then
   if [ ! -d "$build_dir" ]; then    # Does directory exist?
      echo "kill_mif: the build directory $build_dir is invalid"
      err_code=1
   else
      # do we have read and write permission?
      if [ ! -r "$build_dir" -o ! -w "$build_dir" ]; then
         echo "kill_mif: build dir $build_dir must have r/w permission"
         err_code=1
      else
         # make directory absolute and check index file
         cd $build_dir
         build_dir=`pwd`
         if [ ! -r "index" -o ! -w "index" ]; then
            echo "kill_mif: can't read and write index in $build_dir"
            err_code=1
         fi
         cd $start_dir
      fi
   fi
fi

# Check prod directory
if [ ! -d "$prod_dir" ]; then    # Does directory exist?
   echo "kill_mif: the prod directory $prod_dir is invalid"
   err_code=1
else
   if [ ! -r "$prod_dir" -o ! -w "$prod_dir" ]; then # r/w permission?
      echo "kill_mif: prod dir $prod_dir must have r/w permission"
      err_code=1
   else
      # make directory absolute and check index file
      cd $prod_dir
      prod_dir=`pwd`
      if [ ! -r "index" -o ! -w "index" ]; then
         echo "kill_mif: can't read and write index in $prod_dir"
         err_code=1
      fi
      cd $start_dir
   fi
fi

# Check killed_list file (only used when kill is actually executed)
if [ "$execute" = "y" ]; then
  if [ "$killed_list" = "" ]; then
      echo "kill_mif: the -k killed list file is required"
      err_code=1
   else
      > "$killed_list"
      if [ $? != 0 ]; then   # Was killed_list successfully created?
         # No error message neccessary as '>' already gave one
         err_code=1
      else
         # make path absolute
         cd `dirname $killed_list`
         killed_list=`pwd`/`basename $killed_list`
         cd $start_dir
      fi
      rm -f "$killed_list" # so it's not hanging around on error exit
   fi
fi

# Check if any MIF filenames were given
if [ $# -eq 0 ]; then
   echo "kill_mif: at least one MIF_filename is required"
   err_code=1
fi

# exit if error was found above
if [ "$err_code" -ne 0 ]; then
   syntax
fi


# process MIF filenames ( now that directories are verified ok )
MIF_filenames=""
for MIF_filename in $*; do
   # strip any path given from each filename
   MIF_filename=`basename $MIF_filename`

   # add .ptf to each filename if not already there
   if expr "$MIF_filename" : "^.*\.ptf$" >/dev/null 2>&1 ; then
      MIF_filename="${MIF_filename}"
   else
      MIF_filename="${MIF_filename}.ptf"
   fi

   # check for existence of MIF_filename
   if [ -f "$prod_dir/$MIF_filename" ]; then
      MIF_filename="$prod_dir/$MIF_filename" # make path absolute
   else
      # check for it on build (if build given)
      if [ "$build_dir" != "" -a -f "$build_dir/$MIF_filename" ]; then
         MIF_filename="$build_dir/$MIF_filename" # make path absolute
      else
         echo "kill_mif: input ptf $MIF_filename not found on server(s)"
         err_code=1
      fi
   fi

   # add to MIF_filenames
   MIF_filenames="$MIF_filenames $MIF_filename"
done


# exit if error was found above
if [ "$err_code" -ne 0 ]; then
   syntax
fi


# Create temporary work space

# Use a directory name variable for the tmp work directory
WORK=/tmp/kill_mif_work.$$
rm -rf $WORK > /dev/null 2>&1

# If the directory still exists, concatenate an 'A' to make a new id
if [ -d $WORK ]; then
   WORK="${WORK}A"
   rm -rf $WORK > /dev/null 2>&1
fi

mkdir $WORK >/dev/null 2>&1
if [ $? != 0 ]; then
   echo "kill_mif: Could not make a work directory $WORK"
   echo "kill_mif: Please check the amount of space available"
   abort
fi

# chmod added to ensure the directory can be overwritten in the future
chmod 777 $WORK


# Eliminate duplicates from MIF_filenames
> $WORK/mifs
for MIF in $MIF_filenames; do
   grep "$MIF" $WORK/mifs > /dev/null
   if [ $? = 0 ]; then
      echo "kill_mif: Warning: `basename $MIF` on command line twice"
      echo "kill_mif: Process continuing"
   else
      echo "$MIF" >> $WORK/mifs
   fi
done                          # awk zaps the \n's
MIF_filenames=`cat $WORK/mifs | awk '{printf("%s ",$0)}' `


# Construct dependency_list command to check dependencies

dep_list_command="dependency_list -p $prod_dir"

if [ "$build_dir" != "" ]; then
   dep_list_command="$dep_list_command -b $build_dir"
fi

# if kill is to be executed, then add -c for check mode
if [ "$execute" = "y" ]; then
   dep_list_command="$dep_list_command -c"
fi

# add MIF_filenames
dep_list_command="$dep_list_command $MIF_filenames"

# Execute dependency_list command and check return code
$dep_list_command
dep_list_rc=$?
if [ "$dep_list_rc" -ne "0" ]; then
  if [ "$dep_list_rc" -eq "2" ]; then
     if [ "$execute" = "y" ]; then
        echo "kill_mif: All affected MIF_files must be specified on the"
        echo "kill_mif: command line when executing a kill_mif."
     else
        echo "kill_mif: kill would have failed as all affected"
        echo "kill_mif: MIF_files must be specified on the command line"
     fi
  else
     echo "kill_mif: error occurred while checking dependencies."
  fi

  if [ "$execute" = "y" ]; then
     echo "kill_mif: process terminated"
  fi

  abort
fi


# Proceed to kill mif's if -n parameter was not given

if [ "$execute" = "y" ]; then

   for MIF_filename in $MIF_filenames; do

      # go to directory of MIF_filename
      cd `dirname $MIF_filename`

      # remove the MIF_filename
      # rm -f $MIF_filename

      # Change the name of the MIF to MIF.old
      mv $MIF_filename ${MIF_filename}.old

      # remove '.ptf' from $MIF_filename yielding MIF_name
      MIF_name=`expr "$MIF_filename" : "^\(.*\)\.ptf$" `

      # remove path from $MIF_name
      MIF_name=`basename $MIF_name`

      #echo "kill_mif:warning:enqueue and dequeue not implemented"
      #  PLACE CODE HERE TO 'ENQUEUE INDEX FILE'

      # remove $MIF_name entries from the index file
      # ( where $MIF_name is the second field delimited by ":" )
      if [ -s index ]; then # if index is not empty
         cat index | grep -v "^[^:]*:$MIF_name:" > $WORK/index
         mv $WORK/index index
      fi

      #  PLACE CODE HERE TO 'DEQUEUE INDEX FILE'

      # Add MIF_filename to $killed_list
      echo $MIF_filename >> $killed_list
   done

   # Append entry to logfile
   echo "`date` `hostname` `whoami` $stored_command" >> $LOG_FILE

   #echo "kill_mif:warning: notify build of kill not implemented"
   # PLACE CODE HERE TO NOTIFY BUILD MACHINES OF KILL

   sync
fi


# Clean up

cd $start_dir
rm -rf $WORK
exit 0

#### End of kill_mif #### End of kill_mif #### End of kill_mif ####
