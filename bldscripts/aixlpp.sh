#!/bin/sh
# @(#)78        1.9  src/bldscripts/aixlpp.sh, ade_build, bos412, 9443A412a 10/25/94 15:33:23
#
#   ORIGINS: 27
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. `/usr/bin/dirname $0`/common_funcs.sh
. `/usr/bin/dirname $0`/common.sh


#
# Convert BLDCYCLE to 4 digit number to assign to 
# LPPFIXLEVEL or LPPMAINT in osf.aix.packages.mk 
#
conv_bld_cycle ${BLDCYCLE}

#
# Build LPP
#
lpplabel="$1"                    # Label of LPP.
make_pass=$2                     # The pass to make the LPP.
shift 2                          # Move to next set of parameters.

/usr/bin/echo "$lpplabel started at `/usr/bin/date`"

while [ $# -gt 0 ]
do
    lppdir=$1                    # Directory containing LPP source.
    MP=""                        # Assume this is not an MP build.
    shift

    #
    # Did the user pass in an MP parameter?
    #
    if [ $# -gt 0 ]
    then
        #
        # Is this an MP build?
        #
        MP="`eval /usr/bin/echo $1`"
        shift
    fi

    OBJECTDIR=../obj/${target_machine}${MP}

    export MP
    export OBJECTDIR

    #
    # Create the object directory if it does not exist.
    #
    [ -d ${OBJECTDIR} ] || /usr/bin/mkdir -p ${OBJECTDIR}

    #
    # Make sure that there is a directory to build under.  If the source
    # for this directory is accessed through the backing tree link,
    # then the directory to build under may not exist, so create it if
    # it does not.
    #
    if [ ! -d $lppdir ]
    then
        #
        # Note that the build directory was created.  The directory
        # will be removed after the building has finished.
        #
        dir_created="yes"

        #
        # Create the directory.
        #
        /usr/bin/mkdir -p $lppdir
    fi

    #
    # Allow for exporting include files or building
    # and export libraries only
    #
    case ${make_pass} in
           include) make_pass_arg="MAKEFILE_PASS=FIRST"
                 ;;

              libs) make_pass_arg="MAKEFILE_PASS=FIFTH"
                 ;;

 install|build|comp|export) make_pass_arg=${make_pass}"_all"
                 ;;

                 *) make_pass_arg=${make_pass}
                 ;;
    esac

    (cd $lppdir; make -r ${make_pass_arg} )

    status=$?

    #
    # Removed the directory that was previously created.  This will
    # only occur when the source for this directory is accessed
    # through the backing tree link.
    #
    if [ -n "$dir_created" ]
    then
        #
        # Reset the indication that the directory was previously created.
        #
        unset dir_created

        #
        # Remove the previously created directory.
        #
        /usr/bin/rm -rf $lppdir
    fi

    [ "$status" = 0 ] || exit_function
done

/usr/bin/echo "$lpplabel ended at `/usr/bin/date`"

exit 0
