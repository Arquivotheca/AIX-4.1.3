#! /bin/ksh
# @(#)17	1.14  src/bldenv/pkgtools/subptf/subptf.sh, pkgtools, bos412, GOLDA411a 6/30/94 11:04:09
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: subptf
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. bldloginit
. bldinitfunc  # define build init functions
. bldkshconst  # define constants
  bldinit subptf # initialize build environment

[[ ! -f ${RELEASE_LIST} ]] && \
   log +l -x "Cannot find/open RELEASE_LIST file: ${RELEASE_LIST}"

function logpath {
	typeset -i num=1
	path=$(bldlogpath)
	while [[ -r $path/subptf.$num ]]
	do
		let num=num+1
	done
	echo $path/subptf.$num
}

function initenvironment {
	typeset historypath=$(bldhistorypath)/bldhistory

	if [[ ! -d $(bldglobalpath)/bldinfo ]]
	then
		mkdir $(bldglobalpath)/bldinfo || 
			log -x "unable to create $(bldglobalpath)/bldinfo"
	fi
	if [[ ! -d $historypath ]]
	then
		mkdir $historypath || 
			log -x "unable to create $historypath"
	fi
	if [[ ! -r $historypath/Cprimeptfhistory.dir ]]
	then
		touch $historypath/Cprimeptfhistory.dir
	fi
	if [[ ! -r $historypath/Cprimeptfhistory.pag ]]
	then
		touch $historypath/Cprimeptfhistory.pag
	fi
	if [[ ! -r $historypath/Csecondptfhistory.dir ]]
	then
		touch $historypath/Csecondptfhistory.dir
	fi
	if [[ ! -r $historypath/Csecondptfhistory.pag ]]
	then
		touch $historypath/Csecondptfhistory.pag
	fi
}

function ClearStatus {
	typeset release=$1
       	DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDPTFDEPEND \
	             $_BLDCYCLE $BLDCYCLE $_RELEASE $release
	DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDPTFPKG \
                     $_BLDCYCLE $BLDCYCLE
	DeleteStatus -F $_TYPE $T_BLDPTF $_SUBTYPE $S_LPP $_BLDCYCLE $BLDCYCLE
	DeleteStatus -F $_TYPE $T_PTFPKG $_BLDCYCLE $BLDCYCLE
	return 0
}

function ConfirmPhase {
    CheckPhases test
    if [[ $? = 1 ]]  ## if not is test phase
    then
        if CheckPhases hold ; then
	    EnterSelfixPhase
            CheckPhases selfix || log -x "$BLDCYCLE is in the hold phase"
        else
            if CheckPhases selfix package ; then
                if CheckPhases package ; then
                    bldstatus -s $BLDCYCLE
                fi
            else
		EnterSelfixPhase
                CheckPhases selfix || log -x "$BLDCYCLE is in the wrong phase"
            fi 
        fi 
    fi
}

function EnterSelfixPhase {
	typeset	phase
	typeset choice
	typeset force

	phase=$(QueryStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE $BLDCYCLE \
		$_DATE "$_DONTCARE" $_HOSTNAME "$_DONTCARE" \
		$_BUILDER "$_DONTCARE" $_TRACKINGDEFECT "$_DONTCARE")
	while [ "${phase}" != selfix ]
	do
	  print "$BLDCYCLE is in the ${phase} phase.  Needs to \c" > /dev/tty
	  print "be in the selfix phase to run\nsubptf.  Press \c" > /dev/tty
	  print "enter to go to selfix phase now or 'q' to quit? \c" > /dev/tty
	  read choice < /dev/tty
	  if [[ "${choice}" != [Qq]* ]]
	  then
		print "Use force flag? (y/n) \c"
		read choice < /dev/tty
		force=""
	  	if [[ "${choice}" = [Yy]* ]]
		then
			force="-f"
		fi
		bldstatus -s $BLDCYCLE ${force}
	  else
		return
	  fi
	  phase=$(QueryStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE $BLDCYCLE \
		  $_DATE "$_DONTCARE" $_HOSTNAME "$_DONTCARE" \
		  $_BUILDER "$_DONTCARE" $_TRACKINGDEFECT "$_DONTCARE")
	done
}

RELEASELIST=$(bldtmppath)/bldptf.releaselist$$; readonly RELEASELIST
LOG=${LOGBLDPTF:-$(logpath)}
rc=$UNSET; logset -L1 -c$0 -C"subptf" -F$LOG  # init log command
logarg="-e"
trap 'rm -f $RELEASELIST;bldlock -u $PTFMAIN $PERM; \
      logset -r;trap - EXIT;exit $rc' EXIT HUP INT QUIT TERM

ConfirmPhase

BLDPTFRETAIN=${1:-$BLDPTFRETAIN}
if [[ -z "$BLDPTFRETAIN" ]] ; then
    if [[ $BUILD_TYPE = "sandbox" || $BUILD_TYPE = "area" ]]; then
        BLDPTFRETAIN="noretain"
    else
      if [[ $BUILD_TYPE = "production" ]] ; then
         if confirm "Do you want to use real PTFIDs from Retain:" ; then
           BLDPTFRETAIN="retain"
           chksetdisplay
         else
           BLDPTFRETAIN="noretain"
         fi
      fi
   fi
fi

log -b "starting subptf for build cycle $BLDCYCLE"

if [[ "$BLDPTFRETAIN" = "retain" ]] ; then
    log -b "Retain PTF ids will be used (BLDPTFRETAIN=$BLDPTFRETAIN)"
else
    log -b "dummy PTF ids will be used (BLDPTFRETAIN=$BLDPTFRETAIN)"
fi

if [[ -z "$BLDPTFPREVIF" ]] ; then
    log -b "previous PTFs NOT referenced (BLDPTFPREVIF=$BLDPTFPREVIF)"
else
    log -b "previous PTFs referenced (BLDPTFPREVIF=$BLDPTFPREVIF)"
fi

if [[ -z "$BLDOPTLIBS" ]] ; then
    log -b "library references will NOT be optimized (BLDOPTLIBS=$BLDOPTLIBS)"
else
    log -b "library references will be optimized (BLDOPTLIBS=$BLDOPTLIBS)"
fi

if [[ "$BLDPTFRETAIN" = "retain" ]] ; then
    CheckStatus $_TYPE $T_BLDABSTRACTS $_SUBTYPE $S_SYMPTOMS \
                $_STATUS success
    if [[ $? != 0 ]] ; then
        log -x "bldabstracts did not post a successful status for symptoms"
    fi
else
    if [[ -n "$PTFSEED" ]] ; then
        log -b "the starting PTFID is $PTFSEED (PTFSEED=$PTFSEED)"
    elif [[ -n "$PTFPREFIX" ]] ; then
        log -b "$PTFPREFIX is prefix for dummy PTFIDs (PTFPREFIX=$PTFPREFIX)"
    fi
    CheckStatus $_TYPE $T_PREBUILD \
                $_BLDCYCLE $BLDCYCLE \
                $_SUBTYPE $S_PTFSETUP \
                $_STATUS $ST_SUCCESS
    if [[ $? != 0 ]] ; then  ## ptfsetup has not already successfully run
         log -w "ptfsetup has not been successfully run for any release"
    fi
    CheckStatus $_TYPE $T_BLDPTF \
	        $_SUBTYPE $S_BLDMID \
	        $_BLDCYCLE $BLDCYCLE \
	        $_STATUS $ST_SUCCESS
    if [[ $? != 0 ]] ; then  ## bldmid has not already successfully ran
        if bldmid  ## perform non-retain init
	then
		bldsetstatus $_TYPE $T_BLDPTF \
	       		     $_SUBTYPE $S_BLDMID \
	       		     $_BLDCYCLE $BLDCYCLE \
	       		     $_STATUS $ST_SUCCESS ||
					log -x "setting bldmid status"
	else
		[[ $rc -ne 1 ]] && logarg="-x"
                bldsetstatus $_TYPE $T_BLDPTF \
                             $_SUBTYPE $S_BLDMID \
                             $_BLDCYCLE $BLDCYCLE \
                             $_STATUS $ST_FAILURE ||
                                        log -x "setting bldmid status"
		log $logarg "error from bldmid"
	fi
    fi
fi

bldlock -l $PTFMAIN $PERM || 
    log -x "conflicting processes - cannot get lock ($PTFMAIN)"
initenvironment  

bldorderstatus $STATUS_FILE || log -x "ordering status file"

#----------------------------------------------------------------
# For v4 development get the release info by looking for	|
# unprocessed lmmakelist data in the				|
#		selfix/PTF/$BLDCYCLE/* directories.		|
#----------------------------------------------------------------
/bin/ls $TOP/PTF/$BLDCYCLE/*/lmmakelist.1* > /dev/null 2>&1
if [[ $? -ne 0 ]]
then
	log -x "no lmmakelist data found"
else
	> $RELEASELIST
	/bin/ls -1 $TOP/PTF/$BLDCYCLE/*/lmmakelist.1* | while read makelist
	do
		makelistdir=`dirname $makelist`
		release=`basename $makelistdir`
		grep $release $RELEASELIST >/dev/null 2>&1 \
			|| echo $release >> $RELEASELIST
		PARENT=subptf touchfiles $release
	done
fi

bldgetalllists $RELEASELIST || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "unable to get all lists (bldgetalllists found errors)"; }

if CheckStatus $_TYPE $T_BLDPTF \
	       $_SUBTYPE $S_BLDGETLINK \
	       $_BLDCYCLE $BLDCYCLE \
	       $_STATUS $ST_SUCCESS
then
	log -b "command link data is ready"
else
	log -b "preparing command link data"
	DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDGETLINK \
		     $_BLDCYCLE $BLDCYCLE 
        if bldlinkdata
	then
		bldsetstatus $_TYPE $T_BLDPTF \
	       		     $_SUBTYPE $S_BLDGETLINK \
	       		     $_BLDCYCLE $BLDCYCLE \
	       		     $_STATUS $ST_SUCCESS ||
					log -x "setting bldlinkdata status"
	else
                bldsetstatus $_TYPE $T_BLDPTF \
                             $_SUBTYPE $S_BLDGETLINK \
                             $_BLDCYCLE $BLDCYCLE \
                             $_STATUS $ST_FAILURE ||
                                        log -x "setting bldlinkdata status"
		log -x "preparing command link data"
	fi
fi

if CheckStatus $_TYPE $T_BLDPTF \
	       $_SUBTYPE $S_BLDGETXREF \
	       $_BLDCYCLE $BLDCYCLE \
	       $_STATUS $ST_SUCCESS
then
	log -b "xref is ready"
else
	log -b "preparing xref data"
	DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDGETXREF \
		     $_BLDCYCLE $BLDCYCLE 
	DeleteStatus $_TYPE $T_BLDPTF $_SUBTYPE $S_BLDPTFPKG \
		     $_BLDCYCLE $BLDCYCLE
	DeleteStatus -F $_TYPE $T_BLDPTF $_SUBTYPE $S_LPP $_BLDCYCLE $BLDCYCLE
	DeleteStatus -F $_TYPE $T_PTFPKG $_BLDCYCLE $BLDCYCLE
	if bldxreflist
	then
		bldsetstatus $_TYPE $T_BLDPTF \
	       		     $_SUBTYPE $S_BLDGETXREF \
	       		     $_BLDCYCLE $BLDCYCLE \
	       		     $_STATUS $ST_SUCCESS ||
					log -x "setting bldxrefstatus status"
	else
		[[ $? -ne 1 ]] && logarg="-x"
		log $logarg "preparing xref data"
		bldsetstatus $_TYPE $T_BLDPTF \
	       		     $_SUBTYPE $S_BLDGETXREF \
	       		     $_BLDCYCLE $BLDCYCLE \
	       		     $_STATUS $ST_FAILURE ||
					log -x "setting bldxrefstatus status"
	fi
fi

while read release
do
        grep -w $release ${RELEASE_LIST} > /dev/null
        if [[ $? != 0 ]] ; then  ## if release is not in main list
        	log -w "$release ignored since not in RELEASE_LIST"
 		continue
	fi
	logset -1$release -2$BLDCYCLE
	if CheckStatus $_TYPE $T_BLDPTF \
		       $_SUBTYPE $S_BLDPTFDEPEND \
		       $_BLDCYCLE $BLDCYCLE \
		       $_RELEASE $release \
		       $_STATUS $ST_SUCCESS
	then
		log -b "$release dependencies are ready"
	else
		if CheckStatus $_TYPE $T_BLDPTF \
			       $_SUBTYPE $S_BLDGETLISTS \
		      	       $_BLDCYCLE $BLDCYCLE \
		               $_RELEASE $release \
		               $_STATUS $ST_SUCCESS
		then
			ClearStatus $release
			log -b "calculating dependencies for $release"
			if bldptfdepend $release
			then
       		    		bldsetstatus $_TYPE $T_BLDPTF \
				     	     $_SUBTYPE $S_BLDPTFDEPEND \
				     	     $_BLDCYCLE $BLDCYCLE \
				     	     $_RELEASE $release \
					     $_STATUS $ST_SUCCESS ||
					log -x "setting bldptfdepend status"
			else
				[[ $? -ne 1 ]] && logarg="-x"
		    		log $logarg "bldptfdepend found errors for $release"
       		    		bldsetstatus $_TYPE $T_BLDPTF \
				     	     $_SUBTYPE $S_BLDPTFDEPEND \
				     	     $_BLDCYCLE $BLDCYCLE \
				     	     $_RELEASE $release \
					     $_STATUS $ST_FAILURE ||
					log -x "setting bldptfdepend status"
			fi
		else
			log -e "PTF data lists not normalized for $release"
			log -b "bldgetlists was not run or is not complete"
		fi
	fi
	logset -r
done < $RELEASELIST
[[ -s "$RELEASELIST" ]] || log -x "no releases have v3bld status"

if CheckStatus $_TYPE $T_BLDPTF \
	       $_SUBTYPE $S_BLDPTFPKG \
	       $_BLDCYCLE $BLDCYCLE \
	       $_STATUS $ST_SUCCESS
then
	log -b "PTF packaging is done"
else
	log -b "packaging PTFs for $BLDCYCLE"
	DeleteStatus $_TYPE $T_BLDPTF \
		     $_SUBTYPE $S_BLDPTFPKG \
		     $_BLDCYCLE $BLDCYCLE
	DeleteStatus -F $_TYPE $T_BLDPTF $_SUBTYPE $S_LPP $_BLDCYCLE $BLDCYCLE
	DeleteStatus -F $_TYPE $T_PTFPKG $_BLDCYCLE $BLDCYCLE
	if subptfpkg $BLDCYCLE $BLDPTFRETAIN
	then
		bldsetstatus $_TYPE $T_BLDPTF \
			     $_SUBTYPE $S_BLDPTFPKG \
			     $_BLDCYCLE $BLDCYCLE \
			     $_STATUS $ST_SUCCESS ||
				log -x "setting postbuild status"
	else
		[[ $? -ne 1 ]] && logarg="-x"
		log $logarg "packaging $BLDCYCLE (subptfpkg found errors)"
		bldsetstatus $_TYPE $T_BLDPTF \
			     $_SUBTYPE $S_BLDPTFPKG \
			     $_BLDCYCLE $BLDCYCLE \
			     $_STATUS $ST_FAILURE ||
				log -x "setting postbuild status"
	fi
fi

bldmissdefects  # list any defects not in a ptf

