#!/bin/ksh

# @(#)73        1.2  src/bos/usr/lib/nim/methods/m_doreboot.sh, cmdnim, bos41J, 9518A_all  4/27/95  09:42:51
#   COMPONENT_NAME: CMDNIM
#
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# include common NIM shell defines/functions
NIMPATH=${0%/*}
NIMPATH=${NIMPATH%/*}
[[ ${NIMPATH} = ${0} ]] && NIMPATH=/usr/lpp/bos.sysmgt/nim
NIM_METHODS="${NIMPATH}/methods"

. ${NIM_METHODS}/c_sh_lib || exit 99

#---------------------------- globals    --------------------------------
REQUIRED_ATTRS="target"
OPTIONAL_ATTRS="inst_warning"


#---------------------------- Initiate_Boot 
#
# NAME: Initiate_Boot
#
# FUNCTION: This will reboot a list of client machines, presumably set up to 
# perform installations, but that is not actually required.  A machine
# will reboot if and only if the /.rhosts on the client has been enabled for the
# initiating system.
#
# The boots are initiated via background processes which will terminate after
# one minutes to allow more than sufficient time for initiating the reboot
# on the client system.
#
#
# DATA STRUCTURES:
#     parameters:
#        1      = The client 
#        
# RETURNS: (int)
#
#-------------------------------------------------------------------------------

function Initiate_Boot {
    C_RSH="${RSH} ${1}"
    check_push_ok ${1}
    
    ${C_RSH} ${SYNC}
    ${C_RSH} ${SYNC}

    if [[ "${inst_warning}" = "yes" ]]  
    then
        ${C_ERRMSG} ${MSG_REBOOT_WARNING} ${C_ERRMSG_MSG} "" "" "" "" > /tmp/msg 2>&1
        cat /tmp/msg | ${C_RSH} ${WALL}
        #
        sleep 4
        #
        cat /tmp/msg | ${C_RSH} ${WALL}
        #
        ${RM} /tmp/msg

    fi
    ${C_RSH} ${REBOOT} -q > /dev/null &
    PID=$!
    ${SLEEP} 60
    kill -9 ${PID} >/dev/null 2>&1
}

##
##	MAIN
##

# Need to make this for potential error processing (normally exists on clients)
mkdir -p ${TMPDIR}

# set parameters from command line
while getopts :a:qvt: c
do
        case ${c} in

        a)              # validate the attr ass
                        parse_attr_ass "${OPTARG}"

                        # include the assignment for use in this environment
                        eval ${variable}=\"${value}\" 2>${ERR} || \
                                err_from_cmd eval
                        ;;

        q)              # show attr info
                        cmd_what
                        exit 0
                        ;;

        v)              # verbose mode (for debugging)
                        set -x
                        for i in $(typeset +f)
                        do
                                typeset -ft $i
                        done
                        ;;

        \?)     # unknown option
                        error ${ERR_BAD_OPT} ${OPTARG}
                        ;;
        esac
done


# check for missing attrs
ck_attrs

# remove what we created above.
rm -rf ${TMPDIR}  

Initiate_Boot ${target} &
