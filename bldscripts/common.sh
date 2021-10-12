#!/bin/sh
# @(#)80        1.7  src/bldscripts/common.sh, ade_build, bos41J, 9519A_all 5/4/95 10:26:43
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

if [ `basename $PWD` != "src" ]
then
    echo "Please move into the \"src\" subdirectory"
    exit 1
fi

#
# Does the user want to use a backing tree link?  This will allow
# the backing tree to be accessed through a link.
#
if [ "$1" = "-l" ]
then
    USE_BACKING_TREE_LINKS=yes

    shift
fi

#
# Setup variables describing the current environment
#
target_machine=power
LANG=C

export target_machine
export LANG

#
# New build environment definitions
#
BASE=`dirname $PWD`
SOURCEBASE=${BASE}/src
MAKESYSPATH=${SOURCEBASE}/bldenv/mk:
SOURCEDIR=""
SHIP_PATH=${BASE}/ship/${target_machine}

if [ -n "$USE_BACKING_TREE_LINKS" ]
then
    #
    # Create the list of backing trees using the link at under the build
    # base directory.
    #
    SOURCEDIR="`backing_tree_links`"
    if [ -n "$SOURCEDIR" ]
    then
	IN_SANDBOX=yes
	export IN_SANDBOX
        for sourcedir in $SOURCEDIR
        do
          [ -d $sourcedir/../ship/${target_machine} ] && SHIP_PATH=$SHIP_PATH:`cd $sourcedir/../ship/${target_machine} && pwd`
        done
        SOURCEDIR=`echo ${SOURCEDIR} | sed -e "s| |:|g"`
        #
        # Set MAKESYSPATH using the list of backing tree paths.
        #
        BACKING_MAKESYSPATH=`echo $SOURCEDIR: | sed -e "s|:|/bldenv/mk:|g"`
        MAKESYSPATH=${MAKESYSPATH}${BACKING_MAKESYSPATH}
    fi
fi

#
# Put the relative path (.) at the end of MAKESYSPATH.  This tells make
# to look locally to find .include files when the file being included
# can not be found in any of the paths listed in MAKESYSPATH.
#
MAKESYSPATH=${MAKESYSPATH}.

export BASE
export MAKESYSPATH
export SOURCEBASE
export SOURCEDIR
export SHIP_PATH

#
# Constrain search paths
#
ODE_TOOLS=${BASE}/ode_tools/${target_machine}
PATH=${ODE_TOOLS}/bin:.
unset LIBPATH

export ODE_TOOLS
export PATH

#
# Unset ODMDIR so we don't wipe /etc/objrepos on this machine.
#
ODMDIR=""

export ODMDIR

#------------------------------------------------------------------------------
#
# The following code sets the 41MACHINE variable based on the results
# of the command 'lslpp -i | grep bos.rte'.  If nothing is returned,
# this indicates the build is occurring on a machine with 3.2.5 installed.
# Otherwise, the machine is considered to have a 4.1 operating system.
#
# This variable is used to determine whether locales should be shipped
# to the 325loc directory or 41loc directory under ode_tools.
#
#------------------------------------------------------------------------------

MACHLEVEL=`/usr/bin/lslpp -i | /usr/bin/grep bos.rte || /bin/echo ""`

if [ "$MACHLEVEL" = "" ]
then
  LOCPATH=${ODE_TOOLS}/usr/lib/nls/325loc
  BLDENV_LOCPATH=/usr/lib/nls/325loc
  LOCALES_BUILD_DIR=325
else
  LOCPATH=${ODE_TOOLS}/usr/lib/nls/41loc
  BLDENV_LOCPATH=/usr/lib/nls/41loc
  LOCALES_BUILD_DIR=41
fi

export LOCPATH
export BLDENV_LOCPATH
export LOCALES_BUILD_DIR
