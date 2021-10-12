# @(#)89	1.5  src/bos/usr/lib/microcode/serdasd/instal.sh, ucodserdasd, bos411, 9428A410j 10/22/91 11:22:19
##
# COMPONENT_NAME: (ucodserdasd) Serial Dasd Subsystem Microcode 
#
# FUNCTIONS: none
#                                                                    
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
##                                                                    

##
# NAME: instal
#                                                                    
# FUNCTION: script called from installp to instal an LPP
#	    without options
#
# RETURN VALUE DESCRIPTION:
#        the return code from ckprereq if it fails
#        the return code from inurest
##   

##########
## set the device and Lpp name
##########
Device=$1
OptionList=$2
LppName=`pwd`
LppName=`basename $LppName`

##########
## If there is a prereq file, run 
## ckprereq
#########
if [ -s prereq ]
then
	ckprereq 
	rc=$?
	if [ $rc -ne 0 ]
	then
		##########
		## chkprereq failed
		## issue message and exit
		##########
		inuumsg 3
		exit $rc
	fi
fi

##########
## restore using the inurest
##########
inurest -q -d $Device `pwd`/serdasd.mc.al $LppName
rc=$?

##########
## exit with the return code from inurest
##########
if [ $rc -ne 0 ]
then
	inuumsg 25
fi

exit $rc
