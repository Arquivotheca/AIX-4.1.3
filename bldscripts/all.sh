#!/bin/sh
# @(#)75        1.4  src/bldscripts/all.sh, ade_build, bos412, GOLDA411a 2/8/94 20:47:27
#
#   COMPONENT_NAME: bldprocess
#
#   ORIGINS: 27
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

. `dirname $0`/common_funcs.sh

prog_name=`basename $0`           # Name of this script.
currdir=`dirname $0`              # Directory to this script.
list_of_lpps=aix.lpps             # File containing the list of aix LPPs.

#
# Function to check for the existence of the LPP directories.
#
lpp_dirs_exist()
{
    #
    # Check to make sure that each LPP directory exists.
    #
    for next_dir in $*
    do
        dir_found=""

        #
        # Search for the existence of the LPP directory using the list
        # of backing trees.
        #
        for next_search in $search_dirs
        do
            if [ -d $next_search/$next_dir ]
            then
                #
                # LPP directory was found so stop looking.
                #
                dir_found=yes

                break
            fi
        done
 
        #
        # If the LPP directory does not exist the return a status indicating
        # that the LPP can not be built.
        #
        [ -n "$dir_found" ] || return 1
    done

    return 0
}

#
# Determine if the user has passed in any special flags.  Theses flags
# activate control options (i.e. backing tree link).
#
control_flags=""

while [ $# -gt 0 ]
do
    case "$1" in
        #
        # User wants to auto-tail output.
        #
        "-t") tail_output="yes"
              ;;

        #
        # User want to use the backing tree link?
        #
        -*) control_flags="$control_flags$1 "

            if [ "$1" = "-l" ]
            then
                using_backing_tree_links=yes
            fi

            ;;

         *) break
            ;;
    esac

    shift
done

search_dirs=$PWD

#
# Create a list of all the backing tree directories.
#
if [ -n "$using_backing_tree_links" ]
then
    search_dirs="$search_dirs `backing_tree_links`"
fi

#
# Make sure that the logs directory exists.
#
[ -d logs ] || mkdir logs

#
# Create the list of lpps.
#
lpp_list=`awk '
          {
              #
              # Process the lines that do not start with a comment (#) and
              # are not blank.
              #
              if ((! match($0, "^#")) && (length($0) > 0))
              {
                  #
                  # Return the name of the LPP and the directory list.
                  #
                  print $0
              }
          }' $currdir/$list_of_lpps`

#
# Execute each lpp listed in the order listed.
#
echo "$lpp_list" | while read lpp_info
do
    lpp=`echo $lpp_info     | awk '{FS="|"; print $1}'`
    lppdirs=`echo $lpp_info | awk '{FS="|"; print $6}'`
    lppdirs=`echo $lppdirs  | sed -e 's|""||g' -e 's|_mp||'`

    #
    # Build the next LPP if:
    #
    #    1) The directories for the LPP exist.
    #    2) The makelpp.sh script exists in the bldscripts directory.
    #    3) The script has not completed.  A script completes
    #       when it finishes with a valid status (0).
    #
    lpp_dirs_exist $lppdirs

    if [ $? -eq 0 ] && [ -f bldscripts/makelpp.sh ] && [ ! -f logs/${lpp}_done ]
    then
        #
        # Determine the version number of the log file for the LPP.
        #
        next_version=1

        while [ -f logs/$lpp.log.$next_version ]
        do
            next_version=`expr $next_version + 1`
        done

        #
        # Remove any previous indications that the script had a break.
        #
        rm -f logs/${lpp}_broken

        #
        # Indicate that the script is currently building.
        #
        >logs/${lpp}_building

        if [ -n "$tail_output" ]
        then
            #
            # Execute the script and tail the output to the screen.  When
            # the script finishes the status is saved for validation.
            #
            { sh -x bldscripts/makelpp.sh \
                    $control_flags $lpp 2>&1; echo $?>logs/status; } |
            tee logs/$lpp.log.$next_version
        else
            #
            # Execute the script without tail the output to the screen.
            # When the script finishes the status is saved for validation.
            #
            echo
            echo -n "$prog_name: Building $lpp..."

            sh -x bldscripts/makelpp.sh \
                  $control_flags $lpp >logs/$lpp.log.$next_version 2>&1

            echo $?>logs/status
        fi

        #
        # Retrieve the exit status.
        #
        read status <logs/status

        #
        # Remove the indication that the script is building and the
        # associated exit status.
        #
        rm -f logs/${lpp}_building logs/status

        #
        # Process the exist status.
        #
        if [ $status -ne 0 ]
        then
            #
            # Show the exit status when not auto-tailing.
            #
            if [ -z "$tail_output" ]
            then
                echo "Exit status=$status"
                echo
            fi

            #
            # Indicate that a break occured.
            #
            >logs/${lpp}_broken

            #
            # Exit this script with the exit status of the break.
            #
            exit $status
        fi

        #
        # Indicate that the script complete successful.
        #
        >logs/${lpp}_done
    fi
done
