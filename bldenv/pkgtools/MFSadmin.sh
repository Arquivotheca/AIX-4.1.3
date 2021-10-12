#!/bin/ksh
# @(#)82	1.2  src/bldenv/pkgtools/MFSadmin.sh, pkgtools, bos41J, 9509A_all 2/15/95 10:50:52
#**************************************************************************
#
# COMPONENT_NAME: pkgtools
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#**************************************************************************

#-------------------------------------------------------------------
# Some global variables
#-------------------------------------------------------------------
cmd=${0##*/}
version="v1.0"

#
#********************************************************************
# NAME: Usage
#                                                                    
# DESCRIPTION: 
#      Generates the usage statement describing parameters.
#                                                                    
# PRE CONDITIONS: 
#      expects variables "cmd" and "version" to be set.
#
# POST CONDITIONS: 
#      usage statement output to STDERR.
#
# PARAMETERS: 
#      none.
#
# NOTES: 
#      none.
#
# RETURNS: 
#      Returns nothing.
#********************************************************************
function usage
{
    cat <<END_OF_USAGE_STATEMENT: >&2

$cmd $version

 Usage: $cmd -s | -c [-b] [-f]
        where
	  -  Either the -s or the -c flag must be specified.
             -s sets up the mixed fileset environment.
             -c cleans up by sorting and copying the list of 
              changed filesets into the export tree.
	  -  -s and -c are mutually exclusive.  The last one
	     on the command line takes precedence.
	  -  -b is optional and only relevant if doing the 
	     setup function.  It is silently ignored in all
             other cases.
          -  -f is used to force the copy of lists from the
             export tree into the LOG directory even if they
             already exist.
        NOTE:
          - This script expects to be run in the build environment
            with \$BASE and \$TD_BLDCYCLE set.

END_OF_USAGE_STATEMENT:

    return 0
} # END usage

#
#********************************************************************
# NAME: setup Mixed Filesets
#                                                                    
# DESCRIPTION: 
#      performs the setup operation for Mixed Filesets.
#                                                                    
# PRE CONDITIONS: 
#      Expects the following Environment Variables:
#        - EXPORTDIR = export directory to get FILESET.list[.err]
#                      from.
#        - LOGDIR    = log directory to put FILESET.list.[err] in.
#        - bldDB     = Empty means don't build it, otherwise do it.
#                      Copy FILESET.db to EXPORTDIR.
#        - forceCopy = Empty means don't overwrite the lists, 
#                      otherwise copy them even if they already 
#                      exist.
#
# POST CONDITIONS: 
#      Already described.
#
# PARAMETERS: 
#      none.
#
# NOTES: 
#      none.
#
# RETURNS: 
#      Returns 0 for success, aanything else is a failure.  
#********************************************************************
function setupMFS
{
    $DEBUG
    typeset fname
    typeset RC=0

    #---------------------------------------------------
    # 1st create the list files.
    #---------------------------------------------------
    for fname in FILESET.list FILESET.list.err
    do
	if [[ -w ${LOGDIR}/${fname} ]]
	then
	    if [[ ! -z ${forceCopy} ]]
	    then
		${cp} -f ${EXPORTDIR}/${fname} ${LOGDIR}
		RC=$?
	    fi
	elif [[ -r ${EXPORTDIR}/${fname} ]]
	then
	    ${cp} ${EXPORTDIR}/${fname} ${LOGDIR}
	    RC=$?
	else
	    ${touch} ${LOGDIR}/${fname}
	    RC=$?
	fi

	if [[ $RC -ne 0 ]]
	then
	    print -u2 "$cmd: Could not create ${LOGDIR}/${fname}!!"
	    return $RC
	fi
    done

    #---------------------------------------------------------------
    # Now create the DB if requested to do so and copy to EXPORTDIR.
    #---------------------------------------------------------------
    if [[ ! -z ${bldDB} ]]
    then
	${build_fileset_db}
	${cp} -f ${LOGDIR}/FILESET.db ${EXPORTDIR}/FILESET.db
	if [[ $? -ne 0 ]]
	then
	    print -u2 "$cmd: Could not copy '${LOGDIR/FILESET.db' to '${EXPORTDIR}'"
	    RC=1
	fi
    fi

    return $RC
}

#
#********************************************************************
# NAME: cleanup Mixed Filesets
#                                                                    
# DESCRIPTION: 
#      performs the cleanup operation for Mixed Filesets.
#                                                                    
# PRE CONDITIONS: 
#      Expects the following Environment Variables:
#        - EXPORTDIR = export directory to put FILESET.list[.err]
#                      in.
#        - LOGDIR    = log directory to get FILESET.list.[err] from.
#
# POST CONDITIONS: 
#      Already described.
#
# PARAMETERS: 
#      none.
#
# NOTES: 
#      none.
#
# RETURNS: 
#      Returns 0 for success, aanything else is a failure.  
#********************************************************************
function cleanupMFS
{
    $DEBUG
    typeset RC=0
    typeset fname
    typeset tmpfile=/tmp/MFS.sort.$$

    for fname in FILESET.list FILESET.list.err
    do
	${cp} ${LOGDIR}/${fname} ${tmpfile}
	if [[ $? -ne 0 ]]
	then
	    print -u2 "$cmd: Unable to copy ${fname} to ${tmpfile}; check for space in /tmp"
	    print -u2 "    will still try to copy to export tree"
	    RC=2
	else
	    ${sort} -u $tmpfile >${LOGDIR}/${fname}
	    if [[ $? -ne 0 ]]
	    then
		print -u2 "$cmd: Unable to sort ${fname}"
		print -u2 "    will still try to copy to export tree"
		RC=2
	    fi
	fi

	${cp} -f ${LOGDIR}/${fname} ${EXPORTDIR}/${fname}
	if [[ $? -ne 0 ]]
	then
	    print -u2 "$cmd: Could not copy '${fname}' to '${EXPORTDIR}'"
	    RC=1
	elif [[ ${RC} -eq 2 ]]
	then
	    print -u2 $"cmd: Copied '${fname}' to '${EXPORTDIR}', but it is not sorted"
	fi
    done

    return $RC
}

#
#********************************************************************
# NAME: parse command line
#                                                                    
# DESCRIPTION: 
#      Parses the command line and sets up variables with 
#      appropriate information.
#                                                                    
# PRE CONDITIONS: 
#      none.
#
# POST CONDITIONS: 
#      Following variable are modified:
#         operation = the operation to be done 
#                     (either setup or finish).
#         bldDB     = 1 indicates the fileset.db needs to be built.
#
# PARAMETERS: 
#      The entire command line is passed in.
#
# NOTES: 
#      none.
#
# RETURNS: 
#      Returns 0 for success, aanything else is a failure.  
#      Specific values and their meanings are:
#         0 = success
#         4 = invalid invocation (missing or invalid parameters)
#********************************************************************
function parseCmdLine
{
    typeset RC=0 ;

    while getopts :Dbcfs c
    do
	case $c in
	  D)
	     DEBUG="set -xv"
	     $DEBUG ;
	     ;;
	  b)
	     bldDB="true" ;
	     ;;
	  c)
	     operation="cleanup" ;
	     ;;
	  f)
	     forceCopy="true" ;
	     ;;
	  s)
	     operation="setup" ;
	     ;;
	  \?)
	     print -u2 "$cmd: '${OPTARG}' is an invalid flag."
	     usage 
	     RC=4 ; 
	     ;;
	esac
    done
    shift OPTIND-1
    return $RC
} # END parseCmdLine

#
#********************************************************************
# NAME: Mixed Fileset Administration
#                                                                    
# DESCRIPTION: 
#      The command does the administration work for mixed filesets.
#      During setup, the FILESET.list and FILESET.list.err
#      are copied from the export tree into the LOG directory if
#      they don't already exist there.  If -f is also specified,
#      these files are copied even if they do exist.  If -b was set, 
#      the FILESET.db is also created.
#      During cleanup processing, the FILESET.list and 
#      FILESET.list.err are sorted uniquely and then copied out
#      into the export tree.
#                                                                    
# EXECUTION ENVIRONMENT: 
#      This command is expected to be run in the build environment.
#                                                                   
# PRE CONDITIONS: 
#      Following Environment variables are set:
#        BASE = the base of the build tree.
#        TD_BLDCYCLE = the build cycle 
#
# POST CONDITIONS: 
#      Depends on the operation requested.
#      For "setup",
#         - FILESET.list, FILESET.list.err may be modified
#           in the LOG directory (depending upon their
#           prior existence and the state of -f).
#         - The fileset DB may be created if -b is specified.
#      For "cleanup",
#         - The FILESET.list and FILESET.list.err are sorted
#           uniquely and copied into the export tree.
#
# PARAMETERS: 
#      Refer to "Usage" function for parameter description.
#
# NOTES: none.
#
# RECOVERY OPERATION: 
#      Rerun the command.
#
# RETURNS: 
#      Returns 0 for success, anything else is a failure.  
#      Specific values and their meanings are:
#         0 = success
#         1 = invalid invocation (missing or invalid parameters)
#         2 = Could not create the FILESET.list[.err] files in
#             the LOG directory.
#********************************************************************

if [[ -z ${BASE} ]]
then
    print -u2 "$cmd: \$BASE not set"
    usage
fi

if [[ -z ${TD_BLDCYCLE} ]]
then
    print -u2 "$cmd: \$BASE not set"
    usage
fi

typeset cp="${ODE_TOOLS}/usr/bin/cp"
typeset touch="${ODE_TOOLS}/usr/bin/touch"
typeset sort="${ODE_TOOLS}/usr/bin/sort"
typeset build_fileset_db="${ODE_TOOLS}/usr/bin/build_fileset_db"

typeset LOGDIR="${BASE}/selfix/LOG/${TD_BLDCYCLE}"
typeset EXPORTDIR="${BASE}/export/power/usr/fileset"
typeset operation=""
typeset bldDB=""
typeset forceCopy=""
typeset RC=0

parseCmdLine "$@"
RC=$?

$DEBUG

if [[ $RC -eq 0 ]]
then
    #----------------------------------------------------
    #  Action depends upon the operation requested.
    #----------------------------------------------------
    case $operation in
      setup)
        setupMFS
	;;
      cleanup)
        cleanupMFS
	;;
       *)
         print -u2 "$cmd: Unknown operation '${operation}'"
	 usage
    esac
fi
exit $RC
