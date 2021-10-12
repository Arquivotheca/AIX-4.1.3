#! /bin/ksh
# @(#)33	1.22  src/bldenv/pkgtools/subptf/bldgetalllists.sh, pkgtools, bos412, GOLDA411a 6/25/94 17:13:46
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldgetalllists
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

#
# NAME: bldgetalllists
#
# FUNCTION:
#   Selective fix requires multiple data lists for input.  These lists exist
#   for each release in the build cycle.  Before the lists can be used they
#   must be "processed" so they will be in the correct form.  This command
#   processes the lists for each release in the build cycle which has been
#   built for selective fix; this is done by determining which releases to
#   process and passing them on to the lower level command (bldgetlists)
#
#   Normally, this command is called either from v3bld when the FINAL flag
#   is set, or from subptf.  
#
# INPUT: RELEASE_LIST (file) - list of valid releases
#
# NOTES: 1) the status file is indirectly read and updated 
#        2) the lists are created by bldgetlists
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (success) or non-zero (failure)
#

logarg="-e"

. bldloginit
. bldkshconst   # define constants
. bldinitfunc   # load bldinit functions
bldinit subptf  # initialize build environment

logset +l -c$0 -C"bldgetalllists" # init log command

RELEASELIST=$1
rc=$UNSET

trap 'logset -r; trap - EXIT; exit $rc' \
	EXIT HUP INT QUIT TERM

[[ ! -f ${RELEASE_LIST} ]] && \
   log +l -x "Cannot find/open RELEASE_LIST file: ${RELEASE_LIST}"

log -b "getting selective-fix data lists for build cycle $BLDCYCLE"

while read release
do
	## only releases in the RELEASE_LIST are processed; others are ignored
        grep -w $release ${RELEASE_LIST} > /dev/null
        if [[ $? != 0 ]] ; then  ## if release is not in main list
                log -w "$release ignored since not in RELEASE_LIST"
                continue
        fi

	## only get lists if they have not already successfully been done
	if CheckStatus $_TYPE $T_BLDPTF \
		       $_SUBTYPE $S_BLDGETLISTS \
                       $_BLDCYCLE $BLDCYCLE \
		       $_RELEASE $release \
		       $_STATUS $ST_SUCCESS
	then
		log -b "$release lists are ready"
	else
			log -b "getting lists for $release"
			## getting lists implies all the later steps must be
			## redone if this is not the first pass; so delete
			## the status for any later steps to force reprocessing
                        DeleteStatus $_TYPE $T_BLDPTF \
                                     $_SUBTYPE $S_BLDPTFDEPEND \
                                     $_BLDCYCLE $BLDCYCLE \
                                     $_RELEASE $release
                        DeleteStatus -F $_TYPE $T_BLDPTF $_SUBTYPE $S_LPP \
                                        $_BLDCYCLE $BLDCYCLE
                        DeleteStatus -F $_TYPE $T_PTFPKG $_BLDCYCLE $BLDCYCLE
                        DeleteStatus $_TYPE $T_BLDPTF \
                                     $_SUBTYPE $S_BLDPTFPKG \
                                     $_BLDCYCLE $BLDCYCLE

			## get the lists and set the appropriate status
			if bldgetlists $release
			then
       		    		DeleteStatus $_TYPE $T_BLDPTF \
		       		     	     $_SUBTYPE $S_BLDGETLISTS \
                       		     	     $_BLDCYCLE $BLDCYCLE \
		       		     	     $_RELEASE $release 
       		    		bldsetstatus $_TYPE $T_BLDPTF \
		       			     $_SUBTYPE $S_BLDGETLISTS \
                       			     $_BLDCYCLE $BLDCYCLE \
		       			     $_RELEASE $release \
					     $_STATUS $ST_SUCCESS
			else
				[[ $? -ne 1 ]] && logarg="-x"
		    		log $logarg "bldgetlists found errors for $release"
       		    		DeleteStatus $_TYPE $T_BLDPTF \
		       		     	     $_SUBTYPE $S_BLDGETLISTS \
                       		     	     $_BLDCYCLE $BLDCYCLE \
		       		     	     $_RELEASE $release 
       		    		bldsetstatus $_TYPE $T_BLDPTF \
		       			     $_SUBTYPE $S_BLDGETLISTS \
                       			     $_BLDCYCLE $BLDCYCLE \
		       			     $_RELEASE $release \
					     $_STATUS $ST_FAILURE
			fi
	fi
done < $RELEASELIST

## error if there was not at least one release to process
[[ -s "$RELEASELIST" ]] || log -x "no releases have successful v3bld status"

[[ $rc = $UNSET ]] && rc=$SUCCESS
exit $rc
