#!/bin/ksh
# @(#) 37 1.3 src/bos/usr/lpp/bosinst/bicfgsup/bicfgsup.sh, bosinst, bos411, 9428A410j 1/25/94 07:02:44
#
# COMPONENT_NAME: (BOSINST) Base Operating System Install
#
# FUNCTIONS: biinvoker
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp.  1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                   

# Set environment
PATH=/bin:/usr/bin:/etc:/usr/etc:/etc/methods:/usr/lpp/bosinst
PATH=$PATH:/usr/sbin:/usr/lib/boot:/usr/lib/methods
ODMDIR=/etc/objrepos
HOME=/
LIBPATH=/lib:/usr/lib
export PATH LIBPATH ODMDIR HOME
RESTORE=/usr/sbin/restbyname
DEVICE="/dev/rfd0"

rm -f ./signature >/dev/null
rm -f ./startup >/dev/null

# restore the signature file
$RESTORE -xqf - ./signature > /dev/null 2>&1 <$DEVICE
if [[ "$?" -eq 0 ]]
then
	[ -s ./signature ] && read a b <./signature || a=none

	# At this point, the signature file exists, or the variable "a" = none.

	if [[ $a =  "target" ]]
	then
		# extract the startup file
		$RESTORE -xqf$DEVICE ./startup 
		if [[ "$?" -eq 0 ]]
		then
			if [[ -s ./startup ]]
			then
				# run the startup file
				./startup
				rm ./startup >/dev/null

				# rebuild the targetvgs file
				/usr/lpp/bosinst/Get_RVG_Disks
			fi
		fi
	fi
fi



