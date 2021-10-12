#! /bin/ksh
# @(#)17        1.37  src/bldenv/bldtools/rename/bldretain.sh, bldprocess, bos412, GOLDA411a 5/5/94 14:09:18
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldretain
#	     create_apar_ptfids
#	     reopen_apars
#	     call_retain_tools
#	     execute_retain
#	     error_msg
#	     create_ptfids
#	     create_defectapars
#	     create_apar_abs
#	     create_multiple_ptfs
#	     retain_login
#	     setup_logs
#	     clean_up
#
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
# NAME: bldretain
#
# FUNCTION:  bldretain calls the retain interface tools for creating apars and,
#	     ptfids for releases for which prebuild was run
#
# INPUT: BLDCYCLE, From which it Queries status from the status file to get all
#	 the releases for which prebuild was successfull
#
# OUTPUT: Generates three output files, 'defectapars' and 'ptfids' in 
#	  'bldglobalpath' and 'abstracts' in 'bldhistorypath'
#
# SIDE EFFECTS: Modifies the reference field in CMVC for any defect for which
#		a new apar has been created
#
#
###############################################################################

# This function uses the retain tools to create apars and ptfids. This is a 
# recursive function which exits only when valid apars are created for all 
# the defects of a release in a build

function create_apars_ptfids
{
	typeset logline
	typeset OPENED="${basename}.OPENED"
	
	log -b "Calling Retain interface AparsNeeded for release $relname"
	AparsNeeded $FLAGS $lvlname $relname
	[[ $? = 0 ]] || log -x "AparsNeeded failed, Please check logs"
	if [[ -s "${basename}.badrefs" ]]
					# The reference fields have bad apars
	then
		logline="The Level has defects with incorrect reference fields"
		logline="${logline}, Please check the file ${basename}.badrefs"
		log -e "$logline for these defects"
		return 1
	elif [[ ! -s "${basename}.needed" ]]

					# all the defects have closed apars
	then
		logline="Calling Retain interface CheckAparStatus for"
		log -b "${logline} release $relname"

		# Now check whether apars have been closed properly - validity

		CheckAparStatus -c $FLAGS "${basename}.notneeded"
		[[ $? = 0 ]] || log -x "CheckAparStatus failed, Check logs"
		if [[ -s "${basename}.notfound" ]]
		then
			error_msg 1
			return 1
		fi

		# apars exist that are not open but have been created (intran)
		if [[ -s "${basename}.intran" ]]
		then
			log -b "Apars intran:"
			cat "${basename}.intran"
			cat "${basename}.intran" >> $MAINLOG
			execute_retain "OpenApar" ${basename}.intran $OPENED \
					${basename}.opened \
                                        ${basename}.notopened
			# add the apars to the stillopen file
			while read DEFECT APAR
			do
			    	echo "$DEFECT  $APAR  OPEN" \
				      >> ${basename}.stillopen
			done < ${basename}.opened
		fi
		# apars are in state CLOSED *** ,where *** is not PER
		if [[ -s "${basename}.badclose" ]]
		then
			log -b "Apars closed but not with PER: "
			cat "${basename}.badclose"
			cat "${basename}.badclose" >> $MAINLOG
			reopen_apars "${basename}.badclose" \
                                     "${basename}.stillopen"
			rc=$?
			if [[ $rc = 1 ]] 
			then
			 	cat "${basename}.badclose" >> $ERRORLOG
			fi
		fi
		# apars are in state OPEN or REOP
		if [[ -s "${basename}.stillopen" ]]
		then	
			
			cp "${basename}.closedper" "${basename}.CLOSEDPER"
			log -b "Apars that have not been closed: "
			cat "${basename}.stillopen"
			cat "${basename}.stillopen" >> $MAINLOG
			log -b "Calling function checkapars"
			checkapars
		fi
		logline="Calling Retain interface CreatePtf for"
		log -b " ${logline} release $relname"

		# Now create ptf's for valid apars in retain system
	
		execute_retain "CreatePtf" "${basename}.closedper" \
			"${basename}.PTF" "${basename}.ptf" \
			"${basename}.noptf"
		[[ $? = 0 ]] || log -x "CreatePtf failed, Check logs"
		if [[ ! -s "${basename}.noptf" ]] && \
		   [[ ! -s "${basename}.invalid" ]] && \
		   [[ -s "${basename}.ptf" ]]
		then

		# if everything was successfull then append to the 
		# files, defectapars and ptfids in the globalpath and 
		# abstracts in the Historypath

			cp "${basename}.PTF" "${basename}.ptf"
			create_defectapars
			create_ptfids
		else
			error_msg 0
			return 1
		fi
	else

		# calling retain tools for defects that need apars

		call_retain_tools

		# Calling the function recursively until the needed file is
		# empty. i.e until all the defects have valid apars

		create_apars_ptfids
	fi
}

###############################################################################

function reopen_apars {
	typeset reopen_list=$1
	typeset apars_reopened=$2
	typeset not_reopened="${basename}.notoutput"
	typeset REOPEN_LIST="${basename}.REOPEN_LIST"
	typeset -i times=1
	typeset apars
	typeset delay_time=600
	typeset -u APAR
        typeset DEFECT
        typeset CODE
        typeset note="$RETDIR/note"
        typeset mail_msg="$RETDIR/mail_msg"
        
        echo "The following apars are CLOSED but not with PER" \
              > $note
        cat $reopen_list >> $note
        echo "These apars will be reopened and then closed with PER" >> $note
        bldtrackingdefect NOTE ${BLDCYCLE} ${TRUE} -f $note
        rm -f $note
	# try to reopen the apars at the most 6 times
	while ((times<6)) 
	do
		#wipe out the old Log
		rm -f $RETDIR/Log
		cp $reopen_list $REOPEN_LIST
		apars=

		# get all the apars that need to be reopened
		while read DEFECT APAR F1 F2
		do
			apars="$apars $APAR"
		done < $reopen_list

		logline="Calling Retain interface tool aparopen to reopen"
		log -b "${logline} apars $apars"
		aparopen -r $apars 2> $not_reopened
			
		# need to update output file ( stillopen )
		while read DATE SCRIPT APAR ACTION
		do
			DEFECT=`grep "$APAR" $REOPEN_LIST | \
				awk '{print $1}'` 
                        CODE=`grep "$APAR" $REOPEN_LIST | \
                                awk '{print $4}'`
			echo "$DEFECT  $APAR  REOP" >> $apars_reopened
                        echo "Apar $APAR was originally CLOSED $CODE and" \
                             > $mail_msg
                        echo "has now been CLOSED PER during building." \
                             >> $mail_msg
                        echo "It fixes defect $DEFECT." >> $mail_msg
                        mail -s "Apar $APAR has been reopened and CLOSED PER" \
                                 bteam@bahama ctcb@ctgate < $mail_msg
                        rm -f $mail_msg
		done < $RETDIR/Log

		log -b "The following apars were successfully reopened: "
		cat $RETDIR/Log 
		cat $RETDIR/Log >> $MAINLOG

		if [[ -s $not_reopened ]]
		then
		   logline "The following errors were found when reopening "
		   log -b "${logline} apars: "		
		   cat $not_reopened
		   cat $not_reopened >> $MAINLOG
		fi

		rm -f $reopen_list

		# fins all the apars that were not reopened
		while read DEFECT APAR F1 F2
		do
			grep $APAR $apars_reopened >/dev/null
			if [ $? -ne 0 ]; then
				echo "$DEFECT  $APAR  $F1  $F2" \
				>> $reopen_list
			fi
		done < $REOPEN_LIST

		# check if apar(s) was not reopened successfully
		if [[ -s $reopen_list ]]
		then		
			let times=times+1
			logline="The retain tool aparopen was unsuccessful for"
			log -b "$logline the following apars "
			cat $reopen_list
			cat $reopen_list >> $MAINLOG
			(( times >= 6 )) && break
			logline="Trying aparopen again after sleeping for"
			log -b "$logline sleeping for 10 minutes"
			sleep $delay_time
		else
			# reset RETAIN to beginning screen
			hsend "n;"
			return 0
		fi
	done
	return 1
}	
						
###############################################################################

function checkapars {
	typeset OPENED="${basename}.OPENED" CLOSED="${basename}.CLOSED"

	if [[ -s "${basename}.stillopen" ]]
	then
		rm -f "${basename}.OPENED" "${basename}.CLOSED"
		CheckStatus $_TYPE $T_BLDRETAIN  \
			$_SUBTYPE $S_OPENAPAR \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE $relname \
			$_LEVEL $lvlname \
			$_STATUS $ST_SUCCESS
		if [[ $? != 0 ]] 
		then
			# execute OpenApar for apars that are not reopened
			execute_retain "OpenApar" "${basename}.stillopen"  \
			$OPENED "${basename}.opened" "${basename}.notopened"
			if [[ -s "${basename}.notopened" ]]
			then
				DeleteStatus $_TYPE $T_BLDRETAIN \
					$_SUBTYPE $S_OPENAPAR \
					$_BLDCYCLE $BLDCYCLE \
					$_RELEASE $relname \
					$_LEVEL $lvlname \
					$_STATUS $ST_FAILURE
				bldsetstatus $_TYPE $T_BLDRETAIN \
					$_SUBTYPE $S_OPENAPAR \
					$_BLDCYCLE $BLDCYCLE \
					$_RELEASE $relname \
					$_LEVEL $lvlname \
					$_STATUS $ST_FAILURE
			else
				DeleteStatus $_TYPE $T_BLDRETAIN \
					$_SUBTYPE $S_OPENAPAR \
					$_BLDCYCLE $BLDCYCLE \
					$_RELEASE $relname \
					$_LEVEL $lvlname \
					$_STATUS $ST_SUCCESS
				bldsetstatus $_TYPE $T_BLDRETAIN \
					$_SUBTYPE $S_OPENAPAR \
					$_BLDCYCLE $BLDCYCLE \
					$_RELEASE $relname \
					$_LEVEL $lvlname \
					$_STATUS $ST_SUCCESS
			fi
			execute_retain "CloseApar" $OPENED $CLOSED \
				"${basename}.closed" "${basename}.notclosed"
		else
			execute_retain "CloseApar" ${basename}.stillopen \
			$CLOSED "${basename}.closed" "${basename}.notclosed"
		fi
		cp ${basename}.stillopen ${basename}.STILLOPEN
		CheckAparStatus -c $FLAGS ${basename}.STILLOPEN
		cat ${basename}.closedper >> ${basename}.CLOSEDPER
		if [[ -s "${basename}.stillopen" ]]
 		then 
		 aparnum=`cat ${basename}.STILLOPEN |cut -f2`
                 print "*****************************************************"
		 print "Program exits with bad closing code (apar = $aparnum)!"
                 print "*****************************************************"
		 exit 1
		fi
	fi
	cp ${basename}.CLOSEDPER ${basename}.closedper
}

###############################################################################

# This is a higher level function for calling the retain tools, CreateApar,
# OpenApar and CloseApar with the right IO filenames

function call_retain_tools
{
	typeset CREATED="${basename}.CREATED" 
	typeset OPENED="${basename}.OPENED" CLOSED="${basename}.CLOSED"

	execute_retain "CreateApar" "${basename}.needed" $CREATED \
			"${basename}.created" "${basename}.notcreated"
	execute_retain "OpenApar" $CREATED $OPENED \
		"${basename}.opened" "${basename}.notopened"
	if [[ -s "${basename}.notopened" ]]
	then
		DeleteStatus $_TYPE $T_BLDRETAIN \
			$_SUBTYPE $S_OPENAPAR \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE $relname \
			$_LEVEL $lvlname \
			$_STATUS $ST_FAILURE
		bldsetstatus $_TYPE $T_BLDRETAIN \
			$_SUBTYPE $S_OPENAPAR \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE $relname \
			$_LEVEL $lvlname \
			$_STATUS $ST_FAILURE
	else
		DeleteStatus $_TYPE $T_BLDRETAIN \
			$_SUBTYPE $S_OPENAPAR \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE $relname \
			$_LEVEL $lvlname \
			$_STATUS $ST_SUCCESS
		bldsetstatus $_TYPE $T_BLDRETAIN \
			$_SUBTYPE $S_OPENAPAR \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE $relname \
			$_LEVEL $lvlname \
			$_STATUS $ST_SUCCESS
	fi
	execute_retain "CloseApar" $OPENED $CLOSED \
			"${basename}.closed" "${basename}.notclosed"
}

###############################################################################

# This function actually creates,opens and closes apars by calling the appropr
# iate retain interface tools. Each of these tools create two output files with
# suffix like in 'notcreated' or 'notopened' and 'created' or 'opened'. These
# tools are called in a loop until the 'notcreated's etc become empty. The
# primary reason for doing this because of the instability of the retain
# system

function execute_retain
{
	typeset retain_tool=$1
	typeset input=$2
	typeset total_output=$3  # every output file for each run in the
				 # loop is concatenated to this total_output
	typeset output=$4	 # the output file generated for each run
	typeset notoutput=$5     # the not file for each run in the loop
	typeset failed="${basename}.failed" # The retry file - notoutput
	typeset -i times=1

	while ((times < 6))
	do
		log -b "Calling $retain_tool for release $relname"
		$retain_tool -c $FLAGS $input
		[[ $? = 0 ]] || {
					logline="Failed most likely in RETAIN"
					log -x "$logline, Please check logs"
				}
		cat $output >> $total_output
		[[ "$retain_tool" = "CreateApar" ]] && Check_updates

		# if the not file is empty break out of the loop

		[[ -s $notoutput ]] || break 
		cp $notoutput $failed
		input=$failed	       # the not file becomes the input for the
				       # next run in the loop
		let times=times+1
		logline="The retain tool $retain_tool was unsuccessful for" 
		log -b "$logline the following "
		cat $notoutput
		cat $notoutput >> $MAINLOG
		(( times >= 6 )) && break
		log -b "Trying $retain_tool again after sleeping for 10 mins"

		sleep 600	# waiting for 10 mins to prevent from getting
				# into the same glitch in the retain system
	done
}

###############################################################################

function Check_updates {
	typeset logline

	if [[ -s "${basename}.notupdated" ]]
	then
		logline="The CMVC ID that is being used does not have the"
		logline="$logline right authority to modify reference fields"
		DeleteStatus $_TYPE $T_BLDRETAIN \
			$_SUBTYPE $S_UPDATEREF \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE $relname \
			$_LEVEL $lvlname \
			$_STATUS $ST_FAILURE
		bldsetstatus $_TYPE $T_BLDRETAIN \
			$_SUBTYPE $S_UPDATEREF \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE $relname \
			$_LEVEL $lvlname \
			$_STATUS $ST_FAILURE
		log -x "$logline"
	fi
}

###############################################################################

function Update_reference {
	typeset NOTUPDATEDFILE="${basename}.notupdated"
	typeset logline

	if [[ -s "$NOTUPDATEDFILE" ]]
	then
		UpdateReference $NOTUPDATEDFILE
		[[ $? = 0 ]] || log -x "UpdateReference failed,Check logs"
		if [[ -s "${basename}.notupd" ]]
		then
			logline="The CMVC ID used still does not have the"
			logline="$logline right authority to modify reference"
			log -x "$logline fields"
		else
			DeleteStatus $_TYPE $T_BLDRETAIN \
				$_SUBTYPE $S_UPDATEREF \
				$_BLDCYCLE $BLDCYCLE \
				$_RELEASE $relname \
				$_LEVEL $lvlname \
				$_STATUS $ST_FAILURE
		fi
	else
		log -e "Could not find the file $NOTUPDATEDFILE"
		return 1
	fi
}

###############################################################################

# Create one additional ptf, giving us a total of 2 for the release.  We used
# to create 2 additional ptf's here, but it seems we've been giving too many
# back to the system.
function create_multiple_ptfs {
	logline="Calling Retain interface CreatePtf for release"
	log -C "Creating multiple ptfs" -b " ${logline} $relname"
	CreatePtf -c $FLAGS "${basename}.closedper"
	[[ $? = 0 ]] || \
	log -C "Creating multiple ptfs" -e "CreatePtf failed,Check logs"
	if [[ ! -s "${basename}.noptf" ]] && \
	   [[ ! -s "${basename}.invalid" ]] && \
	   [[ -s "${basename}.ptf" ]]
	then
		create_ptfids
	else
		error_msg 0
		return 1
	fi
}

###############################################################################

# This function prints out the error message for any failures with CreateAparSt
# atus or CreatePtf

function error_msg
{
	typeset -i msgnum=$1
	typeset msg_ary[2]
	typeset basename 


	msg_ary[0]="The error file, ${basename}.noptf or "
        msg_ary[0]="${msg_ary[0]}${basename}.invalid was found, or the "
	msg_ary[0]="${msg_ary[0]}file ${basename}.ptf was not found"

	msg_ary[1]="The error file, ${basename}.notfound"
        msg_ary[1]="${msg_ary[1]} was found"

	log -e "${msg_ary[msgnum]}, Please Check logs"

}

###############################################################################

# This function uses the file with suffix 'ptf' created by CreatePtf to get the
# apars and the corresponding ptfs and appends them to ptfids file in the raw
# format

function create_ptfids
{
	typeset PTFIDS PTFIDS_ORG
	typeset ptfid apar defects rel
	typeset PTFSFILE="${basename}.ptf"

	PTFIDS="$(bldglobalpath)/ptfids"
	PTFIDS_ORG="$(bldglobalpath)/ptfids.org"
	log -b "Appending to the file $PTFIDS with apars and ptfids"
	while read defects apar rel ptfid
	do
		print "$apar|$ptfid" >> $PTFIDS
		print "$apar|$ptfid" >> $PTFIDS_ORG
	done < $PTFSFILE
}

###############################################################################

# This function uses the file with suffix 'CLOSED' created by CloseApar to get
# the defects and the corresponding apars and appends them to defectapars file
# in the raw format

function create_defectapars
{
	typeset DEFECTAPARS
	typeset -i defect 
	typeset apar rel ptfid
	typeset PTFSFILE="${basename}.ptf"

	DEFECTAPARS="$(bldglobalpath)/defectapars"
	log -b "Appending to the file $DEFECTAPARS with defects and apars"
	while read defect apar rel ptfid
	do
		print "$defect|$apar|$rel|$CMVC_FAMILY" >> $DEFECTAPARS
	done < $PTFSFILE
}

###############################################################################

# This function calls the command 'bldabstracts' which gets the apars from the 
# defectapars file, the corresponding abstract form the abstractlist file 
# created from 'prebuild', and the symptoms from the 'symptoms' file in the 
# globalpath and appends them to the abstracts file in the Historypath in a 
# free format

function create_apar_abs
{
	log -b "calling bldabstracts"
	bldabstracts -l $MAINLOG
	[[ $? = 0 ]] || log -e "Creating the abstracts file, Please check logs"
}

###############################################################################

# This function cleans up the files and logs off retain

function clean_up
{
	typeset tempfile="$(bldtmppath)/ret_out"
	#cleanup temp files

	log +l -b "Cleaning up"

	# Now retain system calls to logoff and undial because of 
	# -c option used on all retain calls which basically says not to
	# logoff after the call is done with. This is done as a performance
	# enhancement

	# use Change Team retain logoff tool
	hlogoff 
      
        rm -f $RETAIN_OUTPUT/Log $RETAIN_OUTPUT/cs.invalid
	li $RETAIN_OUTPUT > $tmpfile
	[[ -s "$tmpfile" ]] || rmdir $RETAIN_OUTPUT
	rm -f $RELLVLNAMES $tmpfile $ERRORLOG
	logset -r
}

###############################################################################

# This function opens the log file

function setup_logs {
        typeset -i num=1
	typeset path
        path=$(bldlogpath)
	[[ -d $path ]] || mkdir $path; readonly path
        while [[ -r $path/bldretain.$num ]]
        do
                let num=num+1
        done
        MAINLOG=$path/bldretain.$num
	ERRORLOG=$path/bldretain.errors.$num
}

###############################################################################

# This function logs the user on retain

function retain_login {
	typeset userid
	typeset retain_userid
	typeset logon_rc
	typeset retain_tries=0

	# if we are not in test mode find the user's first retain login from
	# the user's .netrc file
	if [[ $test != 0 ]]
	then
	 	userid=$(grep "machine* retain login " $HOME/.netrc \
                       2> /dev/null | awk '{print $4}')
		if [[ "x$userid" != "x" ]]
		then
			print "${userid}" | \
			read retain_userid
		else
			log -b "Failed to find any RETAIN userids in .netrc"
			log -b "Exiting bldretain"
			clean_up
			exit 1
		fi
		while [[ $retain_tries -lt 25 ]]
                do		
		   # login to retain via ausncs
		   hlogin -H ausncs -a 1 -r $retain_userid 
		   logon_rc=$?
		   if [[ $logon_rc -eq 0 ]]
                   then
                      break
                   else
		      # logon was not successful
                      sleep 120
                      ((retain_tries=retain_tries+1))
		   fi
                done
	else
		# for testing purposes to login into the test retain system
		hlogin -M -H ausncs -a 1  
		# sleep for 30 seconds so that the user can manually login
		sleep 30
		logon_rc=$?
	fi

	# exit if we could not access the retain system
	if [[ ${logon_rc} != 0 ]]
	then
		log -b "Logon to retain was not successful"
		log -b "Exiting bldretain"
		clean_up
		exit 1
	fi
}
##############################HouseKeeping###################################

. bldloginit
trap 'clean_up;exit 2' INT QUIT HUP TERM
FLAGS=$*
RELEASELIST="$BLDENV/usr/bin/RELEASE_LIST"
. bldinitfunc
bldinit
RELLVLNAMES="$(bldtmppath)/rellvlname.$$"
export RETAIN_OUTPUT="$(bldlogpath)/RETAIN"
[[ -d $RETAIN_OUTPUT ]] || mkdir $RETAIN_OUTPUT
chksetbecome
test ${CMVC_ID:=$CMVC_BECOME}
export CMVC_ID
chksetdisplay
. bldkshconst		# Defines global constants
YES='Y*'; readonly YES
. bldnodenames
. bldhostsfile
. bldtrackingdefect

# Retain information for Change Team retain tools
export RETDIR=$RETAIN_OUTPUT
test=1

# check if we are running in test mode
for flag in $FLAGS
do
   	case "${flag}" in 
	-t) test=0;;
	 *) ;;
	esac
done

# Querying for all prebuild runs for this bldcycle having successful CMVC
# commits

QueryStatus $_TYPE $T_PREBUILD \
	$_SUBTYPE $S_CMVCCOMMIT \
	$_BLDCYCLE $BLDCYCLE \
	$_STATUS $ST_SUCCESS \
	$_DATE "$_DONTCARE" \
	$_BUILDER "$_DONTCARE" -A | \
	awk -F"|" ' { \
		print $3"|"$4 \
	} ' | sort +0 -u | sed 's/\|/ /' > $RELLVLNAMES
setup_logs

##################################Main#######################################

logset -F $MAINLOG -c $0 -C $0
if [[ "$AUTO" != $YES ]]
then
	cfrm_str="Are you sure you have the right authority to modify"
	cfrm_str="$cfrm_str reference \nfields of defects in CMVC"
	confirm "$cfrm_str (CMVC_BECOME = \"${CMVC_BECOME}\") (y/n)"
	[[ $? = 0 ]] || {
				unset CMVC_BECOME
				chksetbecome
			}
fi
log -b "Starting bldretain"

# check if /retain/bin exists
if [[ ! -d /retain/bin ]]
then
 	log -b "Directory /retain/bin does not exist - check setup!"
	log -b "Exiting from bldretain"
	clean_up
	exit 1
fi

# log onto the retain system
log -b "Logging onto Retain"
retain_login

while read relname lvlname
do
	RELEASE=`grep "^$relname" $RELEASELIST`
	[[ -n $RELEASE ]] || { 
				log -b "$relname not in the file $RELEASELIST"
				continue
		 	      }
     
        # Support multiple families;
        # Search hostsfile.dat file to get family name by giving release name.
        bldhostsfile ${RELEASE} ${TRUE}
        [[ -n $HOSTSFILE_CMVCFAMILY ]] || {
                       logline="CMVC_FAMILY for *RELEASE not found in the"
		       log -b "$logline hostsfile.dat file"
                       continue
                       }
        export CMVC_FAMILY=${HOSTSFILE_CMVCFAMILY}
	# reset cmvc family here for testing purposes
	if [[ $test = 0 ]]
	then
		export CMVC_FAMILY=test01@ausaix05@2550
		echo "cmvc family = $CMVC_FAMILY"
	fi
	export RELEASE_SUFFIX=$RELEASE 

	basename="${RETAIN_OUTPUT}/${lvlname}.${relname}"

	# Checking the success status for bldretain for this release

	CheckStatus $_TYPE $T_BLDRETAIN $_BLDCYCLE $BLDCYCLE $_SUBTYPE "" \
		$_RELEASE $relname $_LEVEL $lvlname $_STATUS $ST_SUCCESS
	[[ $? != 0 ]] || {
				logline="bldretain has been run already and"
				logline="$logline was successful for $relname"
				logline="$logline for $lvlname"
				log -b "$logline"
				continue
			}
				
	bldshowreqs $relname

	logset -1 $relname -2 $lvlname
	log -b "Processing of $relname for $lvlname starting"

	CheckStatus $_TYPE $T_BLDRETAIN $_BLDCYCLE $BLDCYCLE \
		$_SUBTYPE $S_UPDATEREF $_RELEASE $relname \
		$_LEVEL $lvlname $_STATUS $ST_FAILURE
	[[ $? != 0 ]] || {
				Update_reference
				[[ $? = 0 ]] || {
							logset -r
							continue
						}
			 }
	create_apars_ptfids 
	[[ $? = 0 ]] || {
				logline="The creation of apars and ptfs were"
				logline="$logline not successful for the"
				log -b "release $relname for $lvlname"
				DeleteStatus $_TYPE $T_BLDRETAIN \
					$_BLDCYCLE $BLDCYCLE \
					$_RELEASE $relname \
					$_LEVEL $lvlname \
					$_STATUS $ST_FAILURE
				bldsetstatus $_TYPE $T_BLDRETAIN \
					$_BLDCYCLE $BLDCYCLE \
					$_RELEASE $relname \
					$_LEVEL $lvlname \
					$_STATUS $ST_FAILURE
				logset -r
				continue
			}
	logline="Creation of apars and ptfids successful for $relname"
	log -b "$logline for $lvlname"
	DeleteStatus $_TYPE $T_BLDRETAIN $_BLDCYCLE $BLDCYCLE \
		$_RELEASE $relname $_LEVEL $lvlname $_STATUS $ST_SUCCESS
	DeleteStatus $_TYPE $T_BLDRETAIN $_BLDCYCLE $BLDCYCLE \
		$_RELEASE $relname $_LEVEL $lvlname $_STATUS $ST_FAILURE
	bldsetstatus $_TYPE $T_BLDRETAIN $_BLDCYCLE $BLDCYCLE \
		$_RELEASE $relname $_LEVEL $lvlname $_STATUS $ST_SUCCESS
	create_multiple_ptfs
	[[ $? = 0 ]] || log -e "Creating multiple PTF's"
	rm -f ${basename}.*
	logset -r
done < $RELLVLNAMES
[[ -s $RELLVLNAMES ]] && create_apar_abs
if [[ -s $ERRORLOG ]]
then
	log -b "The following apars still need to be closed PER: "
	cat $ERRORLOG
	cat $ERRORLOG >> $MAINLOG
	rm -f $ERRORLOG
fi 
clean_up
