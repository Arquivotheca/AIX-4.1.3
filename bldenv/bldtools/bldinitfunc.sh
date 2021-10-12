#
# @(#)26	1.55  src/bldenv/bldtools/bldinitfunc.sh, bldtools, bos412, GOLDA411a 9/14/94 09:21:51
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: CheckPhases
#            bldinit
#            chkcreate
#            chkexist
#            chkmaindirs
#            chksubdirs
#            chkset_build_type
#            chkset_ode_tools
#            chkset_ship_path
#            chksetbecome
#            chksetbldcycle
#            chksetbldowner
#            chksetbuilder
#            chksetdisplay
#	     chksetfamily
#            chksetklog
#            chksetlevelname
#            chksetrelease
#            chksetstatus
#            chksetsuper
#            chksettop
#            confirm
#            get_filename_definitions
#            touchfiles
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#**********************************************************************
#
# Don't include this file if it has already been included once
#
[[ "$BLDINITFUNC" = "TRUE" ]] && return
BLDINITFUNC="TRUE"

##############################################################################
#
# NAME        : chkset_build_type
#
# FUNCTION    : Set value of environment variable ${BUILD_TYPE} if not set.
#
# SYNTAX      : chkset_build_type
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${BUILD_TYPE} may be changed.
#
##############################################################################
function chkset_build_type
{
   typeset -l build_type=${BUILD_TYPE}

   case "${build_type}" in 
      sandbox | \
      production | \
      area)
         ;;
      *) print "\nBuild type must be sandbox, production or area." > /dev/tty
         print "Enter the build type: \c" > /dev/tty
         read build_type < /dev/tty
         confirm "BUILD_TYPE = ${build_type}\n  Is this correct? (y/n): \c"
         if [[ $? != 0 ]]
         then
            build_type=
         fi
         export BUILD_TYPE=${build_type}
         chkset_build_type
         ;;
   esac

   return 0
}

##############################################################################
#
# NAME        : chkset_ode_tools
#
# FUNCTION    : Set value of environment variable ${ODE_TOOLS} if not set.
#
# SYNTAX      : chkset_ode_tools
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${ODE_TOOLS} may be changed.
#
##############################################################################
function chkset_ode_tools
{
   if [[ -z "${ODE_TOOLS}" ]]
   then
      print "\nEnter the directory for ode tools: \c" > /dev/tty
      read ODE_TOOLS < /dev/tty
      confirm "ODE_TOOLS = ${ODE_TOOLS}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         ODE_TOOLS=
         chkset_ode_tools
      fi
      export ODE_TOOLS
   fi

   return 0
}

##############################################################################
#
# NAME        : chkset_ship_path
#
# FUNCTION    : Set value of environment variable ${SHIP_PATH} if not set.
#
# SYNTAX      : chkset_ship_path
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${SHIP_PATH} may be changed.
#
##############################################################################
function chkset_ship_path
{
   if [[ -z "${SHIP_PATH}" ]]
   then
      print "\nEnter the directory for ship: \c" > /dev/tty
      read SHIP_PATH < /dev/tty
      confirm "SHIP_PATH = ${SHIP_PATH}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         SHIP_PATH=
         chkset_ship_path
      fi
      export SHIP_PATH
   fi

   return 0
}

##############################################################################
# chksettop - attempt to set TOP, prompt user if unable through normal methods
##############################################################################
chksettop() {
    if [[ -z "$TOP" ]]
    then
        print "Enter full path for TOP: \c" >/dev/tty
        read TOP < /dev/tty
        confirm "TOP = $TOP\n  Is this correct? (y/n): \c"
        if [[ $? != 0 ]]
        then
            TOP=""
            chksettop
        fi
        export TOP
    fi

    return 0
}

###############################################################################
#
# NAME: chksetbldcycle
#
# FUNCTION: Prompts user for BLDCYCLE value.  Inforces the build cycle
#           naming convention unless the -o option is supplied.  The
#           build naming convention is YYWWS where YY=year, WW=week,
#           and S=Alpha suffix.
#
# INPUT: BLDCYCLE (global) - Current value of build cycle to be validated.
#        option ($1) - Only value accepted is '-o' which uses the old build
#                      cycle format.  The old build cycle format did not have
#                      any validation.
#
# OUTPUT: BLDCYCLE (global) - Validated value of build cycle.
#
# SIDE EFFECTS: none
#
# RETURNS: 0 always.
#
###############################################################################
chksetbldcycle() {

    typeset option=$1

    if [[ -z "$BLDCYCLE" ]]
    then
        print "\nEnter the Build Cycle: \c" > /dev/tty
        read BLDCYCLE < /dev/tty
        confirm "BLDCYCLE = $BLDCYCLE\n  Is this correct? (y/n): \c"
        [[ $? != 0 ]] && { BLDCYCLE=""; chksetbldcycle -o; }
    fi
    if [[ "${option}" != "-o" && "$BLDCYCLE" != [0-9][0-9][0-9][0-9][A-Z] ]]
    then
        print "\nBuild Cycle must have following format:"
        print "\n\tYYWWS, where YY=year, WW=week, S=Alpha suffix \c"
        print "(e.g 9329B)"
        BLDCYCLE=""
	chksetbldcycle
    fi
    if [[ -n "$BLDCYCLE" ]]
    then
	export BLDCYCLE
    else
        print "\nWARNING:  BLDCYCLE is not set.\n"
    fi
    return 0
}

##############################################################################
#
# NAME        : chksetlevelname
#
# FUNCTION    : Set value of environment variable ${LEVELNAME} if not set.
#
# SYNTAX      : chksetlevelname
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${LEVELNAME} may be changed.
#
##############################################################################
function chksetlevelname
{
   if [[ -z "${LEVELNAME}" ]]
   then
      print "\nEnter the Level Name: \c" > /dev/tty
      read LEVELNAME < /dev/tty
      confirm "LEVELNAME = ${LEVELNAME}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         LEVELNAME=
         chksetlevelname
      fi
      export LEVELNAME
   fi

   return 0
}

##############################################################################
#
# NAME        : chksetbldowner
#
# FUNCTION    : Set value of environment variable ${BLDOWNER} if not set.
#
# SYNTAX      : chksetbldowner
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${BLDOWNER} may be changed.
#
##############################################################################
function chksetbldowner
{
   if [[ -z "${BLDOWNER}" ]]
   then
      print "\nEnter the Build Owner name: \c" > /dev/tty
      read BLDOWNER < /dev/tty
      confirm "BLDOWNER = ${BLDOWNER}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         BLDOWNER=
         chksetbldowner
      fi
      export BLDOWNER
   fi

   return 0
}

##############################################################################
#
# NAME        : chksetbuilder
#
# FUNCTION    : Set value of environment variable ${BUILDER} if not set.
#
# SYNTAX      : chksetbuilder
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${BUILDER} may be changed.
#
##############################################################################
function chksetbuilder
{
   if [[ -z "${BUILDER}" ]]
   then
      print "\nEnter the Builder name: \c" > /dev/tty
      read BUILDER < /dev/tty
      confirm "BUILDER = ${BUILDER}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         BUILDER=
         chksetbuilder
      fi
      export BUILDER
   fi

   return 0
}

##############################################################################
#
# NAME        : chksetrelease
#
# FUNCTION    : Set value of environment variable ${CMVC_RELEASE} if not set.
#
# SYNTAX      : chksetrelease
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${CMVC_RELEASE} may be changed.
#
##############################################################################
function chksetrelease
{
   if [[ -z "${CMVC_RELEASE}" ]]
   then
      print "\nEnter the CMVC Build Release: \c" > /dev/tty
      read CMVC_RELEASE < /dev/tty
      confirm "CMVC_RELEASE = ${CMVC_RELEASE}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         CMVC_RELEASE=
         chksetrelease
      fi
      export CMVC_RELEASE
   fi

   return 0
}

##############################################################################
#
# NAME        : chksetbecome
#
# FUNCTION    : Set value of environment variable ${CMVC_BECOME} if not set.
#
# SYNTAX      : chksetbecome
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${CMVC_BECOME} may be changed.
#
##############################################################################
function chksetbecome
{
   if [[ -z "${CMVC_BECOME}" ]]
   then
      print "\nEnter the CMVC ID to become: \c" > /dev/tty
      read CMVC_BECOME < /dev/tty
      confirm "CMVC_BECOME = ${CMVC_BECOME}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         CMVC_BECOME=
         chksetbecome
      fi
      export CMVC_BECOME
   fi

   return 0
}

##############################################################################
#
# NAME        : chksetsuper
#
# FUNCTION    : Set value of environment variable ${CMVC_BECOME} to a valid 
#		superuser ID
#
# SYNTAX      : chksetsuper
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${CMVC_BECOME} may be changed.
#
##############################################################################
function chksetsuper
{
   typeset ANS
   typeset superpriv superuser superhost valid_super

   valid_super="no"
   while [[ "${valid_super}" != "yes" ]]
   do
	chksetbecome
	superpriv=$(Report -view users -where "login='${CMVC_BECOME}'" \
		    -raw 2> /dev/null | cut -d"|" -f8)
	if [[ "${superpriv}" = "yes" ]]
	then
	  User -view ${CMVC_BECOME} -long | sed '1,13d' | sed '$d' |
	    while read line
	    do
		superhost=$(print "${line}" | awk -F" *" '{ print $2 }')
		superhost=$(print "${superhost}" | cut -d'.' -f1)
		superuser=$(print "${line}" | awk -F" *" '{ print $1 }')
		if [[ "$(hostname)" = "${superhost}" && \
		      "${superuser}" = "$LOGNAME" ]]
		then
			valid_super="yes"
			break
		fi
	    done
	fi
	if [[ "${valid_super}" != "yes" ]]
	then
	    printline="The CMVC ID, ${CMVC_BECOME}, is not authorized as a"
	    printline="${printline} superuser when \nrunning as"
	    print "${printline} ${LOGNAME}@$(hostname).\n" > /dev/tty
	    printline="Press 'q' to abort or any other key to"
	    print "${printline} change your CMVC ID: \c" > /dev/tty
	    read ANS < /dev/tty
	    ANS=$(print ${ANS} | tr [A-Z] [a-z])
	    if [[ "${ANS}" = "q" ]]
	    then
		return 1
	    fi
	    unset CMVC_BECOME
	fi
   done

   return 0
}

##############################################################################
#
# NAME        : chksetklog
#
# FUNCTION    : Check to make sure user is klogged.  If not prompt user for 
#		klog user ID and password and perform klog.
#
# SYNTAX      : chksetklog
#
# INPUT       : none
#
# RETURNS     : 0 if successful, 1 if not successful.
#
# SIDE EFFECTS: Environment variables KLOG_USERID and KLOG_PASSWD are set.
#		These represent the klog user ID and password respectively.
#		They are in encrypted form upon exit and must be unencrypted
#		before being used again.
#
##############################################################################
function chksetklog {
	typeset choice=""

	while tokens | perl -n -e '/\(AFS ID \d*\)/ && exit 1;'
	do
		print "Enter your klog userid: \c" > /dev/tty
		read KLOG_USERID < /dev/tty
		print "Enter your klog password: \c" > /dev/tty
		stty -echo
		read KLOG_PASSWD < /dev/tty
		stty echo
		klog $KLOG_USERID -password $KLOG_PASSWD > /dev/null 2>&1
		klogrc="$?"
		print
		if [ "$klogrc" != 0 ]
		then
		    print "Invalid klog userid and/or password.  \c" > /dev/tty
		    print "Press enter to try \nagain or 'q' to \c" > /dev/tty
		    print "abort klog procedure: \c" > /dev/tty
		    read choice < /dev/tty
		    choice=$(echo $choice | tr [A-Z] [a-z])
		    if [[ "$choice" = "q" || "$choice" = "quit" ]]
		    then
			return 1
		    fi
		fi
	done

	return 0
}

##############################################################################
#
# NAME        : chksetfamily
#
# FUNCTION    : Set value of environment variable ${CMVC_FAMILY} if not set.
#
# SYNTAX      : chksetfamily
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${CMVC_FAMILY} may be changed.
#
##############################################################################
function chksetfamily
{
   if [[ -z "${CMVC_FAMILY}" ]]
   then
      print "\nEnter the CMVC family: \c" > /dev/tty
      read entered_CMVC_FAMILY < /dev/tty
      CMVC_FAMILY=$(bldCMVCfamily ${entered_CMVC_FAMILY})
      if [[ $? -ne 0 ]]
      then
         CMVC_FAMILY=
         chksetfamily
      else
         if [[ "${CMVC_FAMILY}" != "${entered_CMVC_FAMILY}" ]]
         then
            print "${entered_CMVC_FAMILY} converted to full \c" > /dev/tty
            print "family name of ${CMVC_FAMILY}" > /dev/tty
         fi
         confirm "CMVC_FAMILY = ${CMVC_FAMILY}\n  Is this correct? (y/n): \c"
         if [[ $? != 0 ]]
         then
            CMVC_FAMILY=
            chksetfamily
         fi
      fi
   else
      CMVC_FAMILY=$(bldCMVCfamily ${CMVC_FAMILY})
      if [[ $? -ne 0 ]]
      then
         exit 1
      fi
   fi

   export CMVC_FAMILY

   return 0
}

##############################################################################
#
# NAME        : chksetdisplay
#
# FUNCTION    : Set value of environment variable ${DISPLAY} if not set.
#
# SYNTAX      : chksetdisplay
#
# INPUT       : none
#
# RETURNS     : 0 if successful, will not return until successful.
#
# SIDE EFFECTS: Environment variable ${DISPLAY} may be changed.
#
##############################################################################
function chksetdisplay
{
   if [[ -z "${DISPLAY}" ]]
   then
      print "\nEnter the X Server to display on: \c" > /dev/tty
      read DISPLAY < /dev/tty
      confirm "DISPLAY = ${DISPLAY}\n  Is this correct? (y/n): \c"
      if [[ $? != 0 ]]
      then
         DISPLAY=
         chksetdisplay
      fi
      export DISPLAY
   fi

   return 0
}

##############################################################################
# chksetstatus - prompt user for STATUS_FILE if not already set
##############################################################################
chksetstatus() {
    if [[ -z "$STATUS_FILE" ]]
    then
        if [[ -d $(bldhistorypath) ]]
        then
            STATUS_FILE=$(bldhistorypath)/STATUS_FILE
        else
            STATUS_FILE=$(bldtmppath)/status.db
        fi
        print "\nEnter the Status File" > /dev/tty
        print "  (default - $STATUS_FILE): \c" > /dev/tty
        read ANS < /dev/tty
        [[ -z "$ANS" ]] || STATUS_FILE=$ANS
        confirm "STATUS_FILE = $STATUS_FILE\n  Is this correct? (y/n): \c"
        if [[ $? != 0 ]]
        then
            STATUS_FILE=
            chksetstatus
        fi
        export STATUS_FILE
        touch $STATUS_FILE
    fi
}

##############################################################################
# chkcreate - create a directory if it doesn't already exist
##############################################################################
chkcreate() {
    DIR=$1
    if [[ ! -d "$DIR" ]]
    then
       mkdir "$DIR"
       if [[ "$?" != 0 ]]
       then 
           print "Unable to create directory $DIR." > /dev/tty
           kill -QUIT $$
       fi
    fi
}

##############################################################################
# chkexist - check to make sure a directory exists
##############################################################################
chkexist() {
    DIR=$1
    if [[ ! -d "$DIR" ]]
    then
        print "Directory $DIR does not exist." > /dev/tty
        kill -QUIT $$
    fi
}

###############################################################################
# chkmaindirs - check list of main dirs for existence and possible creation
###############################################################################
chkmaindirs() {
    if [[ "$1" = create ]]
    then
        FUNCTION=chkcreate
    else
        FUNCTION=chkexist
    fi
    $FUNCTION $TOP/PTF
    $FUNCTION $TOP/LOG
    $FUNCTION $(bldupdatepath)                           # $TOP/UPDATE
    $FUNCTION $(bldhistorypath)                          # $TOP/HISTORY
}

###############################################################################
# chksubdirs - check list of subdirectories for existence and possible creation
###############################################################################
chksubdirs() {
    if [[ "$1" = create ]]
    then
        FUNCTION=chkcreate
    else
        FUNCTION=chkexist
    fi
    $FUNCTION $(bldlogpath)                              # $TOP/LOG/$BLDCYCLE
    $FUNCTION $(bldglobalpath)                           # $TOP/PTF/$BLDCYCLE
    $FUNCTION $(bldreleasepath $CMVC_RELEASE)            # globalpath/
							 # $CMVC_RELEASE
}

###############################################################################
# touchfiles - touch a list of files to make sure they exist
###############################################################################
touchfiles() {
    typeset RELEASE=$1
    RELEASEPATH=$(bldreleasepath $RELEASE)
    GLOBALPATH=$(bldglobalpath)
    case $PARENT in
        build)
            touch $RELEASEPATH/lmupdatelist
            touch $RELEASEPATH/lmbldenvlist
            touch $RELEASEPATH/libdeplist
            touch $RELEASEPATH/defects
            touch $RELEASEPATH/ptfids
            touch $RELEASEPATH/changeview
            touch $RELEASEPATH/cmvcreqslist
            touch $RELEASEPATH/cmvcchangelist
            touch ${GLOBALPATH}/all_defects
            touch ${GLOBALPATH}/ptfrequisites
            ;;
        prebuild)
            touch $RELEASEPATH/defects
            touch $RELEASEPATH/ptfids
            touch $RELEASEPATH/changeview
            touch $RELEASEPATH/cmvcreqslist
            touch $RELEASEPATH/cmvcchangelist
            touch ${GLOBALPATH}/abstractlist
            touch ${GLOBALPATH}/all_defects
            touch ${GLOBALPATH}/ptfrequisites
            ;;
	subptf)
	    touch $RELEASEPATH/lmupdatelist
	    touch $RELEASEPATH/lmbldenvlist
	    touch $RELEASEPATH/libdeplist
	    touch $RELEASEPATH/changeview
	    touch ${GLOBALPATH}/all_defects
	    ;;
        *)
            ;;
    esac
}

###############################################################################
#
# NAME: confirm
#
# FUNCTION: Prompts user for y/n answer.  Returns 0 if 'y', 1 if 'n',
#           2 if 'q' and -r flag or exits if 'q' and no -r flag.
#
# INPUT: -n - Make default option NO if user responds with enter.
#        -r - Returns to calling routine with return value of 2 if the Q or
#             QUIT option is selected.  If flag is not supplied Q or QUIT
#             will exit with a value of 1.
#        -q - Make default option QUIT if user responds with enter.
#        -y - Make default option YES if user responds with enter.
#        prompt (follows -n, -r, -q and -y flags) - Question to prompt the
#                                                   user with.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if response is 'y', 1 if response is 'n', 2 if response is 'q'
#          and -r flag supplied or exit if response is 'q' and no -r flag
#          is supplied.
#
###############################################################################
function confirm
{
   typeset -i RC=0
   typeset -u default_response=""
   typeset    finished value
   typeset -u input
   typeset    option
   typeset    options
   typeset    prompt
   typeset    return_to_caller="N"
   typeset    value

   while getopts :nqry option
   do
      case $option in
          n | q | y)
             if [[ -n "${default_response}" ]]
             then
                print "confirm: Only one of -n, -q or -y can be supplied" \
                > /dev/tty
                RC=1
             fi
             default_response="${option}"
             ;;
          r) return_to_caller="Y"
             ;;
          :)
             print "confirm: $OPTARG requires a value" > /dev/tty
             RC=1
             ;;
         \?)
             print "confirm: unknown option $OPTARG" > /dev/tty
             RC=1
             ;;
      esac
   done
   shift OPTIND-1

   prompt="$*"
   if [[ -z "$prompt" ]] 
   then
      print "confirm: requires a prompt" >/dev/tty
      RC=1
   fi

   if [[ ${RC} -ne 0 ]]
   then
      print "Usage: confirm [ -n | -q | -y ] [ -r ] \"<prompt>\"" >/dev/tty
      exit 1
   fi

   while [[ -z "$finished" ]]
   do
      print -n "$prompt " > /dev/tty
      read input < /dev/tty
      [[ -z "${input}" ]] && input="${default_response}"
      case "$input" in
         Y|YES)
            RC=0
            finished=TRUE
            ;;
         N|NO)
            RC=1
            finished=TRUE
            ;;
         Q|QUIT)
            if [[ "${return_to_caller}" = "Y" ]]
            then
               RC=2
               finished=TRUE
            else
               exit 1
            fi
            ;;
         *)
            print "confirm: Illegal response" > /dev/tty
            ;;
      esac
   done

   return ${RC}
}

###############################################################################
# CheckPhases - returns true if build cycle phase is one of input phases
###############################################################################
function CheckPhases {
    typeset result=1
    while [[ -n "$1" ]] ; do
        if CheckStatus $_TYPE $T_BLDCYCLE $_BLDCYCLE $BLDCYCLE \
            $_PHASE $1
        then
            result=0
        fi
            shift
    done
    return $result
}

###############################################################################
# bldinit - main initialize routine, operations depend on calling routine
###############################################################################
bldinit() {
    typeset -u local_PTF_UPDATE=${PTF_UPDATE}

    if [[ $# -gt 1 || "$1" = "-?" ]]
    then
        print -u2 "Usage:  bldinit [prebuild|build|subptf|ptfpkg|ptfsetup]"
        kill -QUIT $$
    fi

    PARENT="$1"

    chksettop
    chksetbldcycle -o
    [[ "$PARENT" != ptfpkg ]] && chksetstatus
    chkset_ode_tools

    # set up a default PATH.
    [[ $PATH = *${ODE_TOOLS}/usr/bld* ]] || PATH=$PATH:${ODE_TOOLS}/usr/bld
    [[ $PATH = *${ODE_TOOLS}/usr/bin* ]] || PATH=$PATH:${ODE_TOOLS}/usr/bin
    [[ $PATH = *${BLDENV}/usr/bin* ]] || PATH=$PATH:${BLDENV}/usr/bin

    get_filename_definitions

    case $PARENT in
        prebuild)
            chksetbecome
            # NOTE: TOP is explicitly set in prebuild to /selfix
	    chksetrelease
            chkmaindirs exist
            chksubdirs create
            touchfiles $CMVC_RELEASE
            ;;
        build)
            chkset_build_type
            if [[ -z "${local_PTF_UPDATE}" || "${local_PTF_UPDATE}" = "YES" ]]
            then
                if [[ "$PRODBLD" = yes ]]
                then
                    chkmaindirs exist
                else
                    chkmaindirs create
                fi
                chksubdirs create
                touchfiles $CMVC_RELEASE
            else
                chkcreate $TOP/LOG
                chkcreate $(bldlogpath)
                chkcreate $(bldhistorypath)
            fi
            ;;
        subptf)
            chksetbecome
            chkset_build_type
            chkmaindirs exist
            chkexist $(bldglobalpath)                   #  $TOP/PTF/$BLDCYCLE
            ;;
        ptfpkg)
            chkset_build_type
            chkset_ship_path
            chkexist $TOP/HISTORY
            chkexist $TOP/UPDATE
            ;;
        ptfsetup)
            chksetrelease
            chkmaindirs create
            chksubdirs create
            ;;
        "")
            ;;
        *)
            print -u2 "Usage:  bldinit [prebuild|build|subptf|ptfpkg|ptfsetup]"
            ;;
    esac
}

#
# NAME: get_filename_definitions
#
# FUNCTION: Set default filename definitions.  If input environment
#           variables are not set correctly when calling this function
#           filenames generated will be incorrect.
#
# INPUT: If the input environment variables listed below are not set
#        correctly when calling this function the filenames generated
#        will not be correct.
#           BLDCYCLE (global) - Build cycle.
#           BLDENV (global) - Build environment directory.
#           ODE_TOOLS (global) - Location of ode_tools directory.
#           TOP (global) - Top of the selfix data.
#
# OUTPUT: All of the environment variables defined in the body of the
#         function.  If the environment variable is already set when
#         this function is called the value will not be changed.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 always.
#
export ABSTRACTS
export BLDCMVCFAMILY_FILE
export BLDTRACKINGDEFECT_FILE
export COMPIDS
export FORCED_ROOT
export GOOD_PTFS
export GREPLIST
export HOSTSFILE
export INSTALL_FORCED_PREREQ
export LABEL_TABLE
export LIBNAMES
export NODENAMES
export PKGPTFNAME
export PRODUCTION_FILE
export PTFINFO
export PTFOPTIONS
export RELEASE_LIST

function get_filename_definitions
{
   typeset -r odebld=${ODE_TOOLS}/usr/bld
   typeset -r odebin=${ODE_TOOLS}/usr/bin
   typeset -r bldenvbin=${BLDENV}/usr/bin

   test ${ABSTRACTS:=${TOP}/HISTORY/abstracts}
   test ${FORCED_ROOT:=${TOP}/HISTORY/forced_root}
   test ${INSTALL_FORCED_PREREQ:=${TOP}/HISTORY/install_forced_prereq}
   test ${GOOD_PTFS:=${TOP}/UPDATE/good_ptfs.${BLDCYCLE}}
   test ${PKGPTFNAME:=${TOP}/HISTORY/pkgptf.name}
   test ${PTFINFO:=${TOP}/HISTORY/ptfinfo.${BLDCYCLE}}
   test ${PTFOPTIONS:=${TOP}/HISTORY/ptfoptions}

   test ${COMPIDS:=${ODE_TOOLS}/usr/lib/compids.table}

   LABEL_TABLE=${LABEL_TABLE:=${ODE_TOOLS}/usr/lib/label_text.table}

   BLDCMVCFAMILY_FILE=${BLDCMVCFAMILY_FILE:=${odebld}/bldCMVCfamily.dat}
   [[ -f ${BLDCMVCFAMILY_FILE} ]] || \
      BLDCMVCFAMILY_FILE=${bldenvbin}/bldCMVCfamily.dat

   NODENAMES=${NODENAMES:=${odebld}/nodenames.dat}
   [[ -f ${NODENAMES} ]] || NODENAMES=${bldenvbin}/nodenames.dat

   PRODUCTION_FILE=${PRODUCTION_FILE:=${odebld}/production_areas.dat}
   [[ -f ${PRODUCTION_FILE} ]] || \
      PRODUCTION_FILE=${bldenvbin}/production_areas.dat

   GREPLIST=${GREPLIST:=${odebin}/bldgreplist}
   [[ -f ${GREPLIST} ]] || GREPLIST=${bldenvbin}/bldgreplist

   LIBNAMES=${LIBNAMES:=${odebin}/LIBNAMES}
   [[ -f ${LIBNAMES} ]] || LIBNAMES=${bldenvbin}/LIBNAMES

   RELEASE_LIST=${RELEASE_LIST:=${odebld}/RELEASE_LIST}
   [[ -f ${RELEASE_LIST} ]] || RELEASE_LIST=${bldenvbin}/RELEASE_LIST

   if [[ "${PRODENV}" = SVC410 ]]
   then
       test ${BLDTRACKINGDEFECT_FILE:=${odebld}/bldtrackingdefect.dat.SVC410}
       test ${HOSTSFILE:=${odebld}/hostsfile.dat.SVC410}
       typeset logline=""
       [[ -f ${BLDTRACKINGDEFECT_FILE} ]] || \
           logline="ERROR: ${BLDTRACKINGDEFECT_FILE} not found.\n"
       [[ -f ${HOSTSFILE} ]] || \\
           logline="ERROR: ${logline}${HOSTSFILE} not found.\n"
       if [[ -n "${logline}" ]]
       then
	   echo "$logline"
	   exit 100
       fi
   else
       test ${BLDTRACKINGDEFECT_FILE:=${odebld}/bldtrackingdefect.dat}
       [[ -f ${BLDTRACKINGDEFECT_FILE} ]] || \
           BLDTRACKINGDEFECT_FILE=${bldenvbin}/bldtrackingdefect.dat

       test ${HOSTSFILE:=${odebld}/hostsfile.dat}
       [[ -f ${HOSTSFILE} ]] || HOSTSFILE=${bldenvbin}/hostsfile.dat
   fi

   return 0
}
