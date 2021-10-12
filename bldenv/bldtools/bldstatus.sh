#! /bin/ksh
# @(#)60	1.39  src/bldenv/bldtools/bldstatus.sh, bldtools, bos412, GOLDA411a 3/24/94 13:11:01
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: Usage 
#	     MenuControl 
#            DisplayCycles
#            MenuChoices
#            DoAction
#            ActionCode
#            NoBuilders
#            CurrentPhase
#            SetPhase
#	     StartBldquerymerge
#            main (bldstatus)
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
# NAME: Usage
#
# FUNCTION: Displays syntax for bldstatus.
#
# INPUT: none
#
# OUTPUT: none
#
# SIDE EFFECTS: syntax written to tty
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function Usage {
    print -n "Usage: bldstatus [-l] [-f]" > /dev/tty
    print " -o <bldcycle> | -e <bldcycle> | -b <bldcycle> | " > /dev/tty
    print "\t\t      -s <bldcycle> | -p <bldcycle> | -d <bldcycle> |" > /dev/tty
    print "\t\t      -h <bldcycle> | -t <bldcycle> | -c <bldcycle> |" > /dev/tty
    print "\t\t      -r <bldcycle> | -u <bldcycle>" > /dev/tty
    print "       where" > /dev/tty
    print "\t    -l lists the current phases of active build cycles" > /dev/tty
    print "\t    -f set the force flag" > /dev/tty
    print "\t    -o opens new build cycle" > /dev/tty
    print "\t    -e enters prebuild phase" > /dev/tty
    print "\t    -b enters build phase" > /dev/tty
    print "\t    -s enters selfix phase" > /dev/tty
    print "\t    -p enters package phase" > /dev/tty
    print "\t    -d enters distribution phase" > /dev/tty
    print "\t    -h enters hold phase" > /dev/tty
    print "\t    -t enters test phase" > /dev/tty
    print "\t    -c closes build cycle" > /dev/tty
    print "\t    -r removes build cycle data" > /dev/tty
    print "\t    -u undefines build cycle status" > /dev/tty
    print "       Menu is displayed when no action/option selected." > /dev/tty
}

#
# NAME: MenuControl
#
# FUNCTION: Controls interaction between menu, user, and actions/options.
#
# INPUT: forceflag (parameter) - indicates if force option (-f) is in effect 
#
# OUTPUT: none
#
# SIDE EFFECTS: errors are logged
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function MenuControl {
     typeset forceflag=$1; typeset finished

     while [[ "$finished" != $TRUE ]] ; do
          DisplayCycles 8
          case $(MenuChoices $forceflag) in
              0) ( DoAction open $forceflag );;
              1) ( DoAction prebuild $forceflag );;
              2) ( DoAction build $forceflag );;
              3) ( DoAction selfix $forceflag );;
              4) ( DoAction package $forceflag );;
              5) ( DoAction distribution $forceflag );;
              6) ( DoAction closed $forceflag );;
              7) ( DoAction removed $forceflag );;
              8) ( DoAction undefined $forceflag );;
            q|9) finished=$TRUE;;
              *) print "\a$0: illegal choice" > /dev/tty;;
          esac
          sleep 1
     done
}

#
# NAME: DisplayCycles
#
# FUNCTION: Display build cycles and their current phases.  The build
#           cycles are defined in the status file.  No more than the
#           lastest eight will be displayed.
#
# INPUT: none
#
# OUTPUT: none
#
# SIDE EFFECTS: build cycle phases are displayed on tty
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function DisplayCycles { 
     typeset bldcycle phase 
     typeset TMPFILE1=$(bldtmppath)/tmpfile1
     typeset TMPFILE2=$(bldtmppath)/tmpfile2

     typeset list_length=$1
     print "\nBuild Cycle Statuses:"
     QueryStatus -A $_TYPE $T_BLDSTATUS $_FROMPHASE null $_TOPHASE open | 
	blddatesort | tail -$list_length | blddatesort -r | 
	cut -d'|' -f2 > $TMPFILE1
     while read bldcycle
     do
	  QueryStatus -A $_TYPE $T_BLDCYCLE $_BLDCYCLE $bldcycle |
		cut -d'|' -f10 >> $TMPFILE2 
     done < $TMPFILE1
     if [[ -s "$TMPFILE1" && -s "$TMPFILE2" ]]
     then
	     paste -d' ' $TMPFILE1 $TMPFILE2 | 
	        while read bldcycle phase
		do
	            print "     $bldcycle\t-->\t$phase" > /dev/tty
	        done
     else
	     print "     Status File ($STATUS_FILE) empty." > /dev/tty
     fi
     print > /dev/tty
     rm -f $TMPFILE1 $TMPFILE2
}

#
# NAME: MenuChoices
#
# FUNCTION: Display menu on tty and read user's selection.  Notify the user
#           if the force option is in effect.
#
# INPUT: force (parameter) - indicates if force (-f) option is in effect;
#        user input (menu selection) is read from the tty
#
# OUTPUT: the user's menu selection is written to stdout
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function MenuChoices {
     typeset force=$1; typeset input

     if [[ -n "$force" ]] ; then
          print "  ***** FORCE MODE *****" > /dev/tty
          sleep 2  ## wait a moment for emphasis
     fi
     print "  0) OPEN NEW BUILD CYCLE" > /dev/tty
     print "  1) Enter 'prebuild' Phase" > /dev/tty
     print "  2) Enter 'build' Phase" > /dev/tty
     print "  3) ENTER SELECTIVE FIX PHASE" > /dev/tty
     print "  4) Enter Package Phase" > /dev/tty
     print "  5) ENTER DISTRIBUTION PHASE" > /dev/tty
     print "  6) CLOSE BUILD CYCLE" > /dev/tty
     print "  7) Remove build cycle data" > /dev/tty
     print "  8) Undefine build cycle status" > /dev/tty
     print "  9) q)uit" > /dev/tty
     print -n "\nChoice: " > /dev/tty; read input < /dev/tty
     print $input
}

#
# NAME: DoAction
#
# FUNCTION: Perform the action associated with the transition to a new
#           build phase. 
#
# INPUT: tophase (parameter) - the build cycle phase to be entered
#        forceflag (parameter) - indicates if force option is in effect
#        message strings (global) - message string constants
#
# OUTPUT: none
#
# SIDE EFFECTS: See action-code sections for a full list of effects.  In
#   general the build cycle phase will be modified; various programs run
#   depending on the phase transition, and errors logged.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
## These strings define messages used by the DoAction function.
ALREADY_BUILDER1="another cycle is already in a build/selfix/package/hold phase"
ALREADY_BUILDER2="another cycle is already in a build/selfix/package phase"
BEWARE="ODM versions & PTF History could be at risk of invalidation"
function DoAction {
     typeset tophase=$1 forceflag=$2; typeset fromphase actioncode 
     typeset status force

     fatal_err=""

     if [[ "${tophase}" = "open" ]] 
     then
        # if new build cycle is being opened use the build cycle validation.
        chksetbldcycle
     else
        # if build cycle is already open the don't inforce build cycle
        # validation.  possible that build cycles in old style name format
        # are still open.  this check can be removed once all old style
        # build cycle formats have been removed from status file.
        chksetbldcycle -o
     fi

     [[ -d "$(bldlogpath)" ]] || mkdir $(bldlogpath)  ## make log dir if needed
     LOG=${LOGBLDSTATUS:-"$(bldlogpath)/bldstatus.all"}
     logset -F$LOG   ## change log files 
     trap 'logset -r; logset -r; trap - EXIT; rm -f $TMPLOG; exit $rc' \
           HUP INT QUIT TERM

     fromphase=$(CurrentPhase)
     actioncode=$(ActionCode $fromphase $tophase $forceflag)
     case "$?" in
	1) fatal_err="unknown from-phase: $fromphase";;
     esac

     if [ -z "${fatal_err}" ]
     then
       case $actioncode in
          S00)  [[ "${fromphase}" = "closed" && "${PRODBLD}" = "YES" ]] && \
                   bldtrackingdefect OPEN ${BLDCYCLE} ${TRUE}
	        SetPhase $tophase;;
          S01)  if DeleteStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE} ; then
                        typeset FORCEVALUE CUMLOG="$TOP/LOG/bldstatus.global"
                        if [[ -n "$forceflag" ]] ; then
                            FORCEVALUE="(**FORCED**)"
                        fi
			log -f \
                           "build cycle ${BLDCYCLE} undefined/deleted $FORCEVAULE"
                        log -F $CUMLOG -f \
                           "build cycle ${BLDCYCLE} undefined/deleted $FORCEVALUE"
                else
                        log -e "unable to undefine ${BLDCYCLE}"
                fi;;
          S02)  if bldcycleRemove -s ${BLDCYCLE} ; then
			SetPhase $tophase
		else
			log -e "bldcycleremove failure - phase unchanged"
                fi;;
          S03)	SetPhase $tophase
                log -w "no build cycle cleanup was done";;
          S04)	SetPhase $tophase
                log -w "user must restore ${BLDCYCLE} data from backup";;
          S10)	log -b "${BLDCYCLE} already in $tophase phase";;
          S12)	if postbuild ${BLDCYCLE} ; then
                        SetPhase $tophase 
		else
                        log -e "postbuild failure -- phase set to hold"
			SetPhase hold
                fi;;
          S13)  [[ "${fromphase}" = "closed" && "${PRODBLD}" = "YES" ]] && \
                   bldtrackingdefect OPEN ${BLDCYCLE} ${TRUE}
          	SetPhase $tophase
                log -w "$BEWARE";;
          S14)	if NoBuilders build selfix package hold ; then
                     SetPhase $tophase
                     log -w "$BEWARE"
                else
                     log -e "$ALREADY_BUILDER1" 
                fi;;
          S15)  [[ "${fromphase}" = "closed" && "${PRODBLD}" = "YES" ]] && \
                   bldtrackingdefect OPEN ${BLDCYCLE} ${TRUE}
          	SetPhase $tophase
                log -w "$BEWARE"
                if NoBuilders build selfix package hold ; then
                     log -w "$ALREADY_BUILDER1"
		fi;;
          S16)	SetPhase $tophase
                log -w "use must restore ${BLDCYCLE} data from backup"
                log -w "$BEWARE";;
          S17)	SetPhase $tophase
                postbuild -u ${BLDCYCLE}
		if [[ $? != 0 ]] ; then
                        log -e "postbuild -u failure -- phase set to hold"
			SetPhase hold
                fi;;
          S18)	if NoBuilders build selfix package hold ; then
                     SetPhase $tophase
                else
                     log -e "$ALREADY_BUILDER1"
                fi;;
          S19)  if postpackage ${BLDCYCLE} ; then
                        SetPhase $tophase
                else
                        log -e "postpackage failure -- phase set to hold"
                        SetPhase hold
                fi;;
          S20)	SetPhase $tophase
                log -w "use must restore ${BLDCYCLE} data from backup"
                log -w "$BEWARE"
                if NoBuilders build selfix package hold ; then
                     log -w "$ALREADY_BUILDER1"
		fi;;
          S21)	if NoBuilders build selfix package hold ; then
                     SetPhase $tophase
                     log -w "insure no intervening builds from other cycles"
                else
                     log -e "$ALREADY_BUILDER1" 
                fi;;
          S22)	SetPhase $tophase
                log -w "insure this is the correct restart phase";;
          S23)	SetPhase $tophase
                log -w "insure this is the correct restart phase"
                if NoBuilders build selfix package ; then
                     log -w "$ALREADY_BUILDER2"
		fi;;
          S24)	if NoBuilders build selfix package ; then
                     SetPhase $tophase
                     log -w "insure this is the correct restart phase"
                else
                     log -e "$ALREADY_BUILDER2" 
                fi;;
          S25)	if [[ -z "$BLDNOQMERGE" ]] ; then
                     SetPhase $tophase
		     StartBldquerymerge
		else
                     SetPhase $tophase
		     log -b \
			"bldquerymerge was NOT started on bldcycle $BLDCYCLE"
		fi
                ;;
          S26)	SetPhase $tophase
                log -w "bldquerymerge was NOT started on bldcycle ${BLDCYCLE}" ;;
          S50)	log -e "illegal phase transition";;
          S51)	log -e "\"${BLDCYCLE}\" does not exist";;
          S52)	log -e "${BLDCYCLE} is \"closed\"";;
          S53)	log -e "${BLDCYCLE} already exist"
                log -b "may use force at your own risk to return to open";;
          S54)	log -e "${BLDCYCLE} must be in \"open\" or \"removed\" phase";;
          S55)	log -e "${BLDCYCLE} is \"removed\"";;
          S80)	SetPhase $tophase
                log -w "postbuild was NOT done."
		log -w "$BEWARE";;
          S81)	SetPhase $tophase
                log -w "postbuild and postpackage were NOT done."
		log -w "$BEWARE";;
          S82)	SetPhase $tophase
                log -w "postpackage was NOT done."
		log -w "$BEWARE";;
          S83)	SetPhase $tophase
                log -w "postbuild, postpackage, and distribution were NOT done."
		log -w "$BEWARE";;
          S84)	SetPhase $tophase
                log -w "distribution may not be complete."
		log -w "$BEWARE";;
          S85)	SetPhase $tophase
                log -w "postpackage and distribution were NOT done."
		log -w "$BEWARE";;
          S86)	SetPhase $tophase
		log -w "$BEWARE"
                log -w "postbuild -u was NOT done.";;
          S87)	[[ "$PRODBLD" = "YES" ]] && \
                   bldtrackingdefect OPEN ${BLDCYCLE} ${TRUE}
		SetPhase $tophase;;
            *)  fatal_err="undefined action code: $actioncode";;
       esac
     fi

     if [ "$tophase" = "$(CurrentPhase)" ]
     then
	status="$ST_SUCCESS"
     else
	status="$ST_FAILURE"
     fi

     if [[ -n "$forceflag" ]] ; then
     	force="FORCED"
     else
     	force=""
     fi

     SetStatus $_TYPE $T_BLDSTATUS $_BLDCYCLE ${BLDCYCLE} \
	       $_DATE "$(date +"%h %d %H:%M")" $_FROMPHASE $fromphase \
	       $_TOPHASE $tophase $_FORCEFLG "$force" $_STATUS $status

     if [ -n "${fatal_err}" ]
     then
	log -x "${fatal_err}"
     fi

     logset -r ## return to temp log
     trap 'logset -r; trap - EXIT; rm -f $TMPLOG; exit $rc' \
           HUP INT QUIT TERM
}

#
# NAME: NoBuilders
#
# FUNCTION: Determines if there are other builders.  A "builder" is any
#  build cycle phase in the build, selfix, package, or hold phase.
#
# INPUT: none
#
# OUTPUT: none
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: return 0 if no other "builder"; 1 if there are other builders
#
function NoBuilders {
    typeset result=0
    while [[ -n "$1" ]] ; do
        if CheckStatus $_TYPE $T_BLDCYCLE $_PHASE $1
        then
            result=1
        fi
            shift
    done
    return $result
}

#
# NAME: ActionCode
#
# FUNCTION: Returns an action code for the indicated phase transition.
#   A state-transition table is used to determine the action code.
#
# INPUT: fromphase (parameter) - the current build cycle phase
#        tophase (parameter) - the build cycle phase to be entered
#        forceflag (parameter) - indicates if force option is in effect
#
# OUTPUT: an action code is written to stdout (see list of action codes)
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function ActionCode {
     typeset fromphase=$1 tophase=$2 forceflag=$3
     typeset SELECT to_offset AC[11]

     [[ -n "$forceflag" ]] && SELECT="-F"  # to select force action codes

     #   F P     TO PHASE ----->               Phase Transition Table
     #   R H                                          package
     #   O A                                      selfix  distribution
     #   M S                                  build   |   |   hold
     #     E                   	          prebuild|   |   |   |   test
     #   |                   	      open    |   |   |   |   |   |   close
     #   |                   	  null|   |   |   |   |   |   |   |   |  removed
     #   V                        |   |   |   |   |   |   |   |   |   |   |
     case "$fromphase$SELECT" in #|   |   |   |   |   |   |   |   |   |   |
        null)		set -A AC S51 S87 S51 S51 S51 S51 S51 S51 S00 S51 S51;;
        null-F)	  	set -A AC S51 S87 S51 S51 S51 S51 S51 S51 S00 S51 S51;;
        open)		set -A AC S01 S10 S00 S50 S50 S50 S50 S00 S00 S50 S50;;
        open-F)   	set -A AC S01 S10 S00 S00 S80 S80 S81 S00 S00 S83 S03;;
        prebuild)	set -A AC S54 S53 S10 S18 S50 S50 S50 S00 S00 S50 S50;;
        prebuild-F)     set -A AC S01 S00 S10 S00 S80 S80 S81 S00 S00 S83 S03;;
        build)	  	set -A AC S54 S53 S00 S10 S12 S50 S50 S00 S00 S50 S50;;
        build-F)	set -A AC S01 S00 S00 S10 S80 S80 S81 S00 S00 S83 S03;;
        selfix)	  	set -A AC S54 S53 S17 S17 S10 S25 S50 S00 S00 S50 S50;;
        selfix-F)	set -A AC S01 S86 S86 S86 S10 S26 S82 S00 S00 S85 S03;;
        package)	set -A AC S54 S53 S17 S17 S00 S10 S19 S00 S00 S50 S50;;
        package-F)	set -A AC S01 S86 S86 S86 S00 S10 S82 S00 S00 S85 S03;;
        distribution)	set -A AC S54 S53 S50 S50 S50 S50 S10 S00 S00 S00 S50;;
        distribution-F) set -A AC S01 S86 S86 S86 S00 S00 S10 S00 S00 S84 S03;;
        hold)		set -A AC S54 S22 S22 S24 S24 S24 S22 S10 S00 S50 S50;;
        hold-F)	  	set -A AC S01 S22 S22 S23 S23 S23 S22 S10 S00 S00 S03;;
        test) 	  	set -A AC S01 S13 S13 S14 S14 S14 S13 S00 S10 S00 S03;;
        test-F)	  	set -A AC S01 S13 S13 S15 S15 S15 S13 S00 S10 S00 S03;;
        closed)	  	set -A AC S54 S52 S52 S52 S52 S52 S52 S52 S52 S10 S02;;
        closed-F)	set -A AC S01 S13 S13 S15 S15 S15 S00 S00 S00 S10 S03;;
        removed)  	set -A AC S01 S55 S55 S55 S55 S55 S55 S55 S55 S55 S10;;
        removed-F)	set -A AC S01 S16 S16 S20 S20 S20 S04 S04 S04 S04 S10;;
	*)  		return 1;;
     esac
     case $tophase in
          null)		to_offset=0;;
          open)		to_offset=1;;
          prebuild)	to_offset=2;;
          build)	to_offset=3;;
          selfix)	to_offset=4;;
          package)	to_offset=5;;
          distribution)	to_offset=6;;
          hold)		to_offset=7;;
          test) 	to_offset=8;;
          closed)	to_offset=9;;
          removed)	to_offset=10;;
     esac
     print ${AC[$to_offset]}
     return 0
}

#
# NAME: CurrentPhase
#
# FUNCTION: Return current build cycle phase.
#
# INPUT: BLDCYCLE (global) - the current build cycle
#
# OUTPUT: the current phase
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function CurrentPhase {
     typeset bldcycle=$1; typeset phase

     phase=$(QueryStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE} -A | 
             cut -d'|' -f10)
     [[ -z "$phase" ]] && phase=null
     print $phase
}

#
# NAME: SetPhase
#
# FUNCTION: Set build cycle phase.
#
# INPUT: BLDCYCLE (global) - the build cycle
#        phase (parameter) - the phase
#
# OUTPUT: none
#
# SIDE EFFECTS: The build cycle phase is set/updated in the status file.  Any
#   error are logged.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function SetPhase {
     typeset phase=$1
     typeset FORCEVALUE CUMLOG="$TOP/LOG/bldstatus.global"
     typeset TRACKINGDEFECT

     if [[ -n "$forceflag" ]] ; then
         FORCEVALUE="(**FORCED**)"
     fi
     TRACKINGDEFECT=$(QueryStatus -A $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE} |
                      cut -f7 -d\|)
     DeleteStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE}
     bldsetstatus $_TYPE $T_BLDCYCLE $_BLDCYCLE ${BLDCYCLE} $_PHASE $phase \
		  $_TRACKINGDEFECT "$TRACKINGDEFECT" || {
          fatal_err="unable to set ${BLDCYCLE} phase to $phase"
	  return
     }
     if [[ "${PRODBLD}" = "YES" ]]
     then
	bldtrackingdefect NOTE ${BLDCYCLE} ${TRUE} \
                              "Build cycle set to $phase phase $FORCEVALUE."
	[[ $? -ne 0 ]] && log -e \
          "Unable to log build cycle state change to tracking defect"
     fi
     if [[ "$phase" = "distribution" ]] 
     then
        bldtrackingdefect VERIFY ${BLDCYCLE} ${TRUE}
     fi
     if [[ "$phase" = "closed" ]] 
     then
        bldtrackingdefect CLOSE ${BLDCYCLE} ${TRUE}
     fi
     log -f "build cycle ${BLDCYCLE} set to $phase phase $FORCEVALUE"
     log -F $CUMLOG -f "build cycle ${BLDCYCLE} set to $phase phase $FORCEVALUE"
}

#
# NAME: StartBldquerymerge
#
# FUNCTION: Start bldquerymerge running or skip it
#
# INPUT: name of build machine on which bldquerymerge will be run
#
# OUTPUT: prompts, error/status messages
#
# SIDE EFFECTS: The bldquery database files are created and copied out to AFS
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function StartBldquerymerge {
	typeset choice=""

	print "Do you wish to run bldquerymerge?  If so, it will \c" > /dev/tty
	print "begin execution at\napproximately midnight \c" > /dev/tty
	print "tonight (y/n): \c" > /dev/tty
	read choice < /dev/tty
	choice=$(echo $choice | tr [A-Z] [a-z])
	if [[ "$choice" != "y" && "$choice" != "yes" ]]
	then
		log -b "bldquerymerge was NOT started on bldcycle $BLDCYCLE"
		return 0
	fi

	chksetklog || {
		log -b "bldquerymerge was NOT started on bldcycle $BLDCYCLE"
		return 0
	}

	while [ -z "$BUILDER" ]
	do
		print "Builder name required." > /dev/tty
		chksetbuilder
	done

	export KLOG_USERID KLOG_PASSWD

	nohup bldquerymerge -d &
	log -b "bldquerymerge started on bldcycle $BLDCYCLE"

	unset KLOG_USERID KLOG_PASSWD
}

#
# NAME: main
#
# FUNCTION: A new build cycle is created or a new phase is selected.  See
#   the bldstatus manual page (i.e. bldman bldstatus) for a complete 
#   description.
#
# INPUT: command line arguments -- see syntax
#
# OUTPUT: none
#
# SIDE EFFECTS: Indirectly: the status file is updated with new build cycle
#   phase; if errors occur, the errors are logged; prompts are written to
#   the tty.
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#

. bldloginit
. bldinitfunc  # define build init functions
. bldkshconst  # define constants
. bldtrackingdefect

logset -L1 -c$0 -C"bldstatus" -F/dev/null   ## init log 
trap 'logset -r; trap - EXIT; rm -f $TMPLOG; exit $rc' EXIT HUP INT QUIT TERM
typeset -i actions=0
typeset fatal_err
typeset -x BLDCYCLE=""

while getopts :C:lfo:e:b:s:p:d:h:t:c:r:u: option
do case $option in
     C)  (( actions=actions+1 )); phase=closed; BLDCYCLE=$OPTARG;;
     l)  listflag=1;;
     f)  forceflag=1;;
     o)  (( actions=actions+1 )); phase=open; BLDCYCLE=$OPTARG;;
     e)  (( actions=actions+1 )); phase=prebuild; BLDCYCLE=$OPTARG;;
     b)  (( actions=actions+1 )); phase=build; BLDCYCLE=$OPTARG;;
     s)  (( actions=actions+1 )); phase=selfix; BLDCYCLE=$OPTARG;;
     p)  (( actions=actions+1 )); phase=package; BLDCYCLE=$OPTARG;;
     d)  (( actions=actions+1 )); phase=distribution; BLDCYCLE=$OPTARG;;
     h)  (( actions=actions+1 )); phase=hold; BLDCYCLE=$OPTARG;;
     t)  (( actions=actions+1 )); phase=test; BLDCYCLE=$OPTARG;;
     c)  (( actions=actions+1 )); phase=closed; BLDCYCLE=$OPTARG;;
     r)  (( actions=actions+1 )); phase=removed; BLDCYCLE=$OPTARG;;
     u)  (( actions=actions+1 )); phase=null; BLDCYCLE=$OPTARG;;
     :)  log -e "\"-$OPTARG\" requires a value"; Usage; exit 1;;
    \?)  log -e "unknown option -$OPTARG"; Usage; exit 1;;
     esac
     if (( $actions > 1 )) ; then
          log -e "too many options -- only one phase change allowed"
          Usage; exit 1 
     fi
done
shift OPTIND-1

chksettop
chksetstatus
TMPLOG=$(bldtmppath)/bldstatus.tmp; LOG=${LOGBLDSTATUS:-"$TMPLOG"}

if [[ -n "$1" ]] 
then
     log -e "unexpected argument -- $1"
     Usage; exit 1
fi

[[ "$PRODBLD" = [Yy]*([Ee])*([Ss]) ]] && PRODBLD="YES"

if [[ -n "$listflag" ]]
then
     if [[ -n "$forceflag" || "$actions" != 0 ]]
     then
          log -e "too many options -- no other arguments allowed with -l option"
          Usage; exit 1 
     fi
     DisplayCycles 20
     exit 0
fi

if (( $actions == 0 ))
then
     MenuControl $forceflag  ## default to menu
else
     ( DoAction $phase $forceflag )
fi 

exit
