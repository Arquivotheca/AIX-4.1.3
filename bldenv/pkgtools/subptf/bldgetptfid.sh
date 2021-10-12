#! /bin/ksh
# @(#)17	1.6  src/bldenv/pkgtools/subptf/bldgetptfid.sh, pkgtools, bos412, GOLDA411a 6/24/93 15:06:12
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldgetptfid
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bldgetptfid
#
# PURPOSE: takes defect number and APAR and prints out PTF ID
#
# INPUT: defect number, APAR and cmvc family name,
#	 environment variables CMVC_BECOME and DISPLAY and LOGBLDGETPTFID
#        required
#
# OUTPUT: PTF ID
#
# RETURNS: 0 if successful, 1 if errors
#
# EXECUTION ENVIRONMENT: Build process environment
#

#
# FUNCTION:	clean_up
# DESCRIPTION:	removes temporary files
# INPUT:	error message string (null string if no error)
# OUTPUT:	prints out error message string; exits with 0 if no error
#		or 1 if error
#
function clean_up {
	# remove all temporary files
	rm -f ${BASE}.CreatePtf.errmsg    \
	      ${BASE}.noptf               \
	      ${BASE}.CreatePtf.cmvc     \
	      ${BASE}.ptf                 \
	      ${BASE}.CreatePtf.retainlog \
	      ${BASE}.invalid* 

	logset -r
}

################################## M A I N ################################
. bldloginit

# call cleanup on interrupts
trap "clean_up; exit 1" HUP INT QUIT TERM

. bldinitfunc	# define build init functions
. bldkshconst	# define constants

command=`basename "$0"`	# get command name

logset -c"${command}" +l	# set up log environment

bldinit		# initialize environment

defect="$1"	# get defect number
apar="$2"	# get APAR
family="$3"     # get cmvc family name

# both defect number and APAR are required
if [[ -z "${defect}" || -z "${apar}" ]]
then
	log -x "Usage: ${command} <defect number> <APAR> [<cmvc family>]"
fi

# DISPLAY environment variable must be set
if [[ -z "$DISPLAY" ]]
then
	log -x "DISPLAY environment variable must be set."
fi

# CMVC_BECOME environment variable must be set
if [[ -z "$CMVC_BECOME" ]]
then
	log -x "CMVC_BECOME environment variable must be set."
fi

# set CMVC_ID to the value of CMVC_BECOME
CMVC_ID="${CMVC_BECOME}"; export CMVC_ID

# set CMVC_FAMILY in environment for CreatePtf if value specified
if [[ -n "$family" ]]
then
	export CMVC_FAMILY=$family
fi


# make sure LOGBLDGETPTFID environment variable is set
if [ -z "${LOGBLDGETPTFID}" ]
then
	LOGBLDGETPTFID="$(bldlogpath)/${command}"
fi

# define base name for files created by CreatePtf command
BASE="${command}$$"

print "${defect} ${apar}" | CreatePtf -i -n${BASE} > /dev/null \
       2>> ${LOGBLDGETPTFID} || log -x "could not log onto retain."

# two possible error files; errors occurred if either of them has a file
# size greater than 0
ERRFILES=`ls ${BASE}.noptf ${BASE}.invalid*`
for i in ${ERRFILES}
do
	if [ -s "$i" ]
	then
		log -x "could not create PTFID; see ${LOGBLDGETPTFID} for details."
	fi
done

# if no errors then the PTF ID can be found in the 4th field of 
# the ${BASE}.ptf file
cat ${BASE}.ptf | cut -d' ' -f4

# clean up and exit with no errors
clean_up
exit 0

