#!/bin/sh
# @(#)79        1.3  src/bldscripts/makelpp.sh, ade_build, bos412, GOLDA411a 2/9/94 15:04:01
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

prog_name=`basename $0`           # The name of this script.
currdir=`dirname $0`              # Directory to this script.
list_of_lpps=aix.lpps             # File containing the list of aix LPPs.

#
# A function to read parameter information for an lpp to be built.
#
get_lpp_info()
{
    #
    # Search for the specified LPP and return all parameter information.
    #
    awk '
    {
        if ((! match($0, "^#")) && 
            (length($0) > 0)    &&
            (match($0, "^'"$aixlpp"'")))
        {
            print $0
        }
    }' $currdir/$list_of_lpps
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
        # User want to use the backing tree link?
        #
        -*) control_flags="$control_flags$1 "
            ;;

         *) break
            ;;
    esac

    shift
done

aixlpp=$1                         # LPP to build.
shift                             # Clear out the parameter.
user_args="$*"                    # Save user specified paramaters.
shift $#                          # Clear out all passed in parameters.

#
# Get the parameter info for the LPP.
#
lpp_info=`get_lpp_info`

if [ -z "$lpp_info" ]
then
    echo "$prog_name: $aixlpp is NOT valid, check $currdir/$list_of_lpps!"
    exit 1
fi

#
# Find out how many parameters to pass and the script to execute for the
# LPP.
#
lpp_args=`echo $lpp_info   | awk '{FS="|"; print NF-4}'`
lpp_script=`echo $lpp_info | cut -d"|" -f3`

#
# Pass parameter information if it exists.
#
if [ $lpp_args -gt 0 ]
then
    #
    # Get the string label indicating what is going to be done, the make pass,
    # and the list of LPPs.
    #
    label=`echo $lpp_info | cut -d"|" -f4`
    pass=`echo $lpp_info  | cut -d"|" -f5`
    lpps=`echo $lpp_info  | cut -d"|" -f6`

    #
    # Pass the LPP arguments and the user specified arguments.
    #
    set -- $control_flags "$label" $pass $lpps $user_args
else
    set -- $control_flags $user_args
fi

#
# Execute the script for the LPP.
#
. $currdir/$lpp_script
