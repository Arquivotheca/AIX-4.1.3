#! /bin/ksh
# @(#)08	1.12  src/bldenv/bldtools/postpackage.sh, bldprocess, bos412, GOLDA411a 4/28/93 17:54:45
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: postpackage
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: postpackage
#
# FUNCTION:  postpackage returns the unused ptds to RETAIN, calls bldcommitall
#	     to commit HISTORY, calls bldCleanup to cleanup the PTF, LOG, and 
#	     the bldtd directories.
#
# INPUT: BLDNOCLEANUP ( Env variable for cleanups)
#
# OUTPUT: none
#
# RETURNS: Zero on success and a non zero on failure
#
# SIDE EFFECTS: Modifies the status file.
#
# EXECUTION ENVIRONMENT: BUILD ENVIRONMENT

###############################################################################

function clean_up {

	typeset tmpfile="$(bldtmppath)/retcancel_out"

	# Retain system calls to logoff and undial, This is done because of
        # -c option used on all retain calls which basically says not to
        # logoff after the call is done with. This is done as a performance
        # enhancement

        hsend -t 5 logoff
        hsend -t 5 undial
	if [ -d "$RETAIN_OUTPUT" ]
	then
		li $RETAIN_OUTPUT > $tmpfile
       		[[ -s "$tmpfile" ]] || rmdir $RETAIN_OUTPUT
       		rm -f $tmpfile
	fi
        logset -r
}

###############################################################################

# This function returns the unused ptfids to RETAIN

function return_unusedptfids {

	[[ -d $RETAIN_OUTPUT ]] || mkdir $RETAIN_OUTPUT
	chksetdisplay
	typeset unusedptfs="${RETAIN_OUTPUT}/${BLDCYCLE}.ptfids"
	typeset notoutput="${RETAIN_OUTPUT}/${BLDCYCLE}.notcancelled"
	typeset failed="${RETAIN_OUTPUT}/${BLDCYCLE}.failed"
	typeset ptfnotcancel="${RETAIN_OUTPUT}/${BLDCYCLE}.ptfnotcancel"
	typeset ptfimagesdir="/afs/austin/aix/ptf.images/320"
	typeset ptfimagesdirs="$ptfimagesdir/prod $ptfimagesdir/prod/KILL_PTF"
		ptfimagesdirs="$ptfimagesdirs $ptfimagesdir/ship $BLDTMP"
	typeset -i times=0

	[[ -s "$(bldglobalpath)/ptfids" ]] || return 0
	rm -f ${unusedptfs}
	cat "$(bldglobalpath)/ptfids" | cut -d\| -f2 |
		while read ptf
		do
			used=""
			for dir in ${ptfimagesdirs}
			do
				if [ -f "${dir}/${ptf}.ptf" ]
				then
					used=1
					break
				fi
			done
			if [ ! -n "${used}" ]
			then
				print "$ptf" >> ${unusedptfs}
			fi
		done

	while ((times < 6))
	do
		log -b "Calling CancelPtf to cancel unused ptfids"
		CancelPtf -c $unusedptfs
		[[ $? = 0 ]] || {
                                        logline="Failed most likely in CMVC"
                                        logline="$logline, or RETAIN"
					log -e "$logline Please check logs"
					return 1
                                }

                # if the notcancelled file is empty break out of the loop

                [[ -s $notoutput ]] || break
		cp $notoutput $failed
                unusedptfs=$failed       # the not file becomes the input 
					 # for the next run in the loop
                let times=times+1
                log -b "CancelPtf was unsuccessfull for the following"
                cat $notoutput
                cat $notoutput >> $MAINLOG
                log -b "Trying CancelPtf again after sleeping for 10 mins"

                sleep 600       # waiting for 10 mins to prevent from getting
                                # into the same glitch in the retain system
        done
	if [[ -s $notoutput ]]
	then
		while read ptf msg
		do
     			grep $ptf "$(bldglobalpath)/ptfids" >> $ptfnotcancel
		done < $notoutput
		mv $ptfnotcancel "$(bldglobalpath)/ptfids"
	else
		rm -f ${RETAIN_OUTPUT}/${BLDCYCLE}.*
		rm -f "$(bldglobalpath)/ptfids"
		return 0
	fi
return 1
}

###############################################################################

. bldloginit

trap 'clean_up;exit 2' INT QUIT HUP TERM
logset -C $0 -c $0
. bldinitfunc
. bldkshconst
. bldhostsfile
. bldnodenames
bldinit
export RETAIN_OUTPUT=$(bldlogpath)/RETAIN_CANCEL

CheckStatus $_TYPE $T_POSTPACKAGE $_SUBTYPE $S_MAIN \
		$_BLDCYCLE $BLDCYCLE $_STATUS $ST_SUCCESS 
[[ $? != 0 ]] || {
			logline=" postpackage has been successfull already for"
			log -b "$logline $BLDCYCLE"
			exit 0
		 }

# Complete all levels from the build cycle.
bldhostsfile_release=""
log -b "Moving all levels in build cycle ${BLDCYCLE} to complete"
QueryStatus $_TYPE $T_PREBUILD $_BLDCYCLE $BLDCYCLE $_SUBTYPE $S_CMVCCOMMIT \
            $_STATUS $ST_SUCCESS -A | \
   awk -F"|" '{ print $3,$4 }' | \
   sort | \
   while read release levelname
   do
      if [[ "${release}" != "${bldhostsfile_release}" ]]
      then
         bldhostsfile ${release} "${TRUE}"
         RC=$?
         [[ ${RC} -ne 0 ]] && log -x "bldhostsfile ${release} returned ${RC}"
         bldhostsfile_release=${release}
      fi
      state="$(Report -view levelview \
                      -where "name='${levelname}' and 
                              releasename='${release}'" \
                      -family ${HOSTSFILE_CMVCFAMILY} \
                      -raw | \
               cut -d\| -f10)"
      if [[ "${state}" != "complete" ]]
      then
         Level -complete ${levelname} -release ${release} \
               -family ${HOSTSFILE_CMVCFAMILY}
         [[ "$?" != 0 ]] && \
            log -x "Level -complete ${levelname} -release ${release}"
         log -b +l "${release} level ${levelname} moved to complete"
      else
         log -b +l "${release} level ${levelname} already complete"
      fi
   done
log -b "Finished moving all levels in build cycle ${BLDCYCLE} to complete"

[[ -n "$BLDNOCOMMITALL" ]] || {
			  bldcommitall
			  [[ $? = 0 ]] || \
			      log -x "bldcommitall failure, Please check logs"
			}

[[ -n "$BLDNOCANCEL" ]] || {
				return_unusedptfids
				[[ $? = 0 ]] || 
				    log -x "Cancelling unused ptfids in RETAIN"
			   }

[[ "$BLDNOCLEANUP" = "Y" ]] || {
				bldCleanup
				[[ $? = 0 ]] || \
				 log -x "bldCleanup failure, Please check logs"
				}
DeleteStatus $_TYPE $T_POSTPACKAGE $_SUBTYPE $S_MAIN \
		$_BLDCYCLE $BLDCYCLE $_STATUS $ST_SUCCESS 
bldsetstatus $_TYPE $T_POSTPACKAGE $_SUBTYPE $S_MAIN \
		$_BLDCYCLE $BLDCYCLE $_STATUS $ST_SUCCESS 
[[ $? = 0 ]] || log -x "Setting PostPackage to success for $cycle "
clean_up

