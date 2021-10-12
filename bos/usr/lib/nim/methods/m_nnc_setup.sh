#!/bin/ksh
#
# @(#)74        1.2  src/bos/usr/lib/nim/methods/m_nnc_setup.sh, cmdnim, bos41J, 9516B_all  4/21/95  15:55:51
#
#   COMPONENT_NAME: CMDNIM
#
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
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

#---------------------------- chk_dir 
#
# NAME: chk_dir
#
# FUNCTION: 	CHeck to see if the directory exists, if not then 
#	make it. It can be used for a mount point or the place we want 
#	to copy the commands to.... 
#
# DATA STRUCTURES:
#     parameters:
#        1         	= client name
#        2         	= directory to check
#        
# RETURNS: (int)
#	Exits if an error occurs... 
#
#-------------------------------------------------------------------------------
function chk_dir
{
	client=$1
	client_dir=$2
	getrc="echo \$?"

	#
	# Check to see if client has a nim methods directory already
	#
	clientdir_exists=`${RSH} ${client} "test -d ${client_dir}; $getrc"`
	[ $clientdir_exists -ne 0 ] && {\
		clientdir_created=`${RSH} ${client} "${MKDIR} -p ${client_dir} >/dev/null; $getrc"`
	    	[ $clientdir_created -ne 0 ] && error ${ERR_MKDIR} ${client_dir} 
        }
}

#---------------------------- mnt_dir 
#
# NAME: mnt_dir
#
# FUNCTION: 	Attempt to mount the support directory .... 
#
# DATA STRUCTURES:
#     parameters:
#        1         	= client name
#        
# RETURNS: (int)
#	Exits if an error occurs... 
#
#-------------------------------------------------------------------------------
function mnt_dir
{
	client=$1
	getrc="echo \$?"
	thishost=`/usr/bin/hostname`
	#
	# Make sure that the client NIM methods mount point exists
	chk_dir ${client} ${NIM_METHODS}

	# Check to see if client has nfs running; if not, have to copy the
	# files explicitly.
	nfs_ok=`${RSH} ${client} "/usr/bin/ps -u root | /usr/bin/grep -q biod; $getrc"`
	if [ -z "$nfs_ok" ] || [ "$nfs_ok" != "0" ]
	then
		copy_files ${client} 
		return $?
	fi


	nfsmount=`${RSH} ${client} "${MOUNT} -rohard,intr -vnfs ${thishost}:${NIM_METHODS} ${NIM_METHODS}; $getrc"`
	if [ "${nfsmount}" != "0" ] || [ -z "${nfsmount}" ]
	then 
		error ${ERR_NFS_MNT} ${thishost}:${NIM_METHODS} ${NIM_METHODS}
        fi
	return 0
}

#---------------------------- copy_files 
#
# NAME: copy_files
#
# FUNCTION: 	Install a local copy of the IPLROM emulation setup directory on 
#   machines (for those customers who do not have the nfs client installed)
#
#
# DATA STRUCTURES:
#     parameters:
#        1         	= client name
#        
# RETURNS: (int)
#     0              = success
#     1              = something is a foot
#
#
#-------------------------------------------------------------------------------
function copy_files
{
	file_list="\
		./IPLROM.emulation	\
		./c_nim_bootinfo32	\
		./c_nnc_setup 		\
		./c_sh_lib 		\
		./cmdnim_errors.shh 	\
		./c_errmsg		\
		./c_nimpush		\
		./c_nim_mkboot32 "

	client=$1
	getrc="echo \$?"

	#
	# Copy over the files.  If copy fails, then expand the filesystem if
	# the user allows us. 
	#
	cd ${NIM_METHODS}
	${TAR} -cf - ${file_list} 2>/dev/null | \
		copy_okay=`${RSH} ${client} "cd ${NIM_METHODS}; ${TAR} -xf - >/dev/null 2>&1; $getrc"`
        [ -n "$copy_okay" ] && [ $copy_okay -ne 0 ] && {\
		#
		# Expand the filesystem on the client if requested to accomodate
	    	# the files, which should occupy approximately 500KB. Unless they 
		# explicitly tell us not to do so... 
		#
	    	clientfs=`${RSH} ${client} ${DF} ${NIM_METHODS} | ${TAIL} -1 | ${AWK} '{print $NF}'`
	    	[ -n "$DO_NOT_EXPAND" ] && error ${ERR_CP_FAILED} ${client} ${client} ${clientfs}
		#
		# Attempt to expand and copy again
		#
	       	expand_ok=`${RSH} ${client} "${CHFS} -a size=+1 ${clientfs} >/dev/null; $getrc"`
	       	[ $expand_ok -ne 0 ] && error ${ERR_EXP_FAILED} ${clientfs} ${client}
		${TAR} -cf - ${file_list} | \
			copy_okay=`${RSH} ${client} "cd ${NIM_METHODS}; ${TAR} -xf - >/dev/null; $getrc"`
		[ $copy_okay -ne 0 ] && error ${ERR_CP_FAILED} ${client} ${client} ${clientfs}
	}
	return 0
}


##
##  Main 
##

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# set parameters from command line
while getopts vzc:m: c
do
case ${c} in
  
	v)	# verbose mode (for debugging)
		set -x
		for i in $(typeset +f)
		do
			typeset -ft $i
		done
	;;

	z)	# Do not expand 
		DO_NOT_EXPAND="yes"	
	;;

	c)	# Check that the prospective client has given us .rhosts
		# access and allows push operations
	
		cmd="check_push_ok ${OPTARG}"
	;;

	m)	# Attempt to mount the stuff... 
		cmd="mnt_dir ${OPTARG}"
	;;

	\?)   	# unknown option
		error ${ERR_BAD_OPT} ${OPTARG}
	;;
esac
done

${cmd}

exit 0
