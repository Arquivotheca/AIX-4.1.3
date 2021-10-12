#!/usr/bin/bsh
# @(#)44	1.7  src/bos/usr/bin/chtz/chtz.sh, cmdtz, bos411, 9428A410j 5/6/94 15:05:07
#
# COMPONENT_NAME: (CMDTZ)
#
# FUNCTIONS: chtz.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


# 
# FILE NAME: chtz
#
# FILE DESCRIPTION: High-level shell command for changing the timezone
#   (TZ) environment variable in the /etc/environment file.
#
#   Basic functions performed are:
#   1)  change/add the TZ environment variable
#
#   See Usage message for explanation of parms
#
#
# RETURN VALUE DESCRIPTION: 
#                             0         Successful
#                             non-zero  Unsuccessful
# 
#
# EXTERNAL PROCEDURES CALLED: none (other than standard utilities)
#
#
# GLOBAL VARIABLES: none
#

################################# usage #######################################
#
# NAME: usage()
#
# DESCRIPTION: Issue "usage" message and exit.
#
# INPUT: 
#        None
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           2
#
# NOTE: This function will not return (i.e., it will exit the entire
#       script with exit status of 2).
#
usage(){
   dspmsg -s $MSGSET $MSGCAT 2 "\
   Usage:  %s TimeZoneInfo\n\n\
   TimeZoneInfo         Information that will go into the TZ environment\n\
                        variable located in %s.\n\
                        NOTE: the TZ variable must be located at the line's\n\
                        beginning.\n\n\
   Example: %s 'CST-6CDT1'\n\n" $NAME $PROFILE $NAME 1>&2

   RTNCODE=2
   exit $RTNCODE                 # don't return
}


############################## main ############################################

PATH=/usr/bin:/bin:/etc export PATH	# avoid trojan horses

NAME=`basename $0`

PROFILE=/etc/environment
MSGCAT=chtz.cat 		# message catalog name
MSGSET=1			# message set number
T1=/tmp/$NAME$$.1
trap 'rm -f $T1; trap 0; exit $RTNCODE' 0 1 2 3 15

# Parse command flags and arguments

if [ $# -ne 1 ] ; then		# test for proper number of parms
   usage			# issue msg and don't return
fi

if [ "$1" = "-?" ] ; then	# be nice to the user and give a usage
   usage			# message if requested with -?
fi

# To really do this, you need to allow for the shell's free form syntax
if egrep '^TZ=' $PROFILE 2>&1 > /dev/null ; then
   # sed search separator should be % because / can be part of TZ variable
   sed "s%^TZ=.*%TZ=$1%" $PROFILE > $T1 && cp $T1 $PROFILE
   RTNCODE=$?
   if [ $RTNCODE -ne 0 ] ; then
      dspmsg -s $MSGSET $MSGCAT 1 "0548-001 %s: Could not write to %s\n\
\tCheck permissions on the file.\n" $NAME $PROFILE
      exit $RTNCODE
   fi
else
   echo "TZ=$1" >> $PROFILE
   RTNCODE=$?
   if [ $RTNCODE -ne 0 ] ; then
      dspmsg -s $MSGSET $MSGCAT 1 "0548-001 %s: Could not write to %s\n\
\tCheck permissions on the file.\n" $NAME $PROFILE
      exit $RTNCODE
   fi
fi

exit 0
