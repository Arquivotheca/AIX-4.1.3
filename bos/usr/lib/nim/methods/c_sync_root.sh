#!/bin/ksh
# @(#)27        1.22  src/bos/usr/lib/nim/methods/c_sync_root.sh, cmdnim, bos41J, 9516A_all  4/18/95  09:19:02
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_sync_root.sh
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# include common NIM shell defines/functions
NIMPATH=${0%/*}
NIMPATH=${NIMPATH%/*}
[[ ${NIMPATH} = ${0} ]] && NIMPATH=/usr/lpp/bos.sysmgt/nim
NIM_METHODS="${NIMPATH}/methods"
. ${NIM_METHODS}/c_sh_lib

#---------------------------- local defines     --------------------------------

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location"
OPTIONAL_ATTRS="installp_flags filesets bundle"
location=""
installp_flags=""
filesets=""
bundle=""

# The following are names of files that will hold lists of fileset names
# for a full root sync op.
cleanup_sync_list=${TMPDIR}/$$.cleanup
apply_sync_list=${TMPDIR}/$$.applies
commit_sync_list=${TMPDIR}/$$.commits
deinstl_sync_list=${TMPDIR}/$$.deinstls
reject_sync_list=${TMPDIR}/$$.rejects

#---------------------------- ck_instroot_cust_sz   ----------------------------
#
# NAME: ck_instroot_cust_sz
#
# FUNCTION:
#		Uses the value saved in INSTROOT_CUST_SZ during c_instspot
#		to ensure that enough space exists in /usr for the 
#		root_sync on the inst_root of the SPOT.  Expands if 
#		necessary.
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#               o No checking is done for auto-expand flag since by now,
#		  after the cust has been done on the SPOT, it would be silly
#		  to fail due to a shortage of a few hundred K (which is all
#		  there is likely to be short for the root_sync).
#
#		o calls error if cannot expand filesystem
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#               parameters:
#               global:
#			INSTROOT_CUST_SZ
#
# RETURNS: (int)
#               0   = success
#
# OUTPUT:
#-------------------------------------------------------------------------------

function ck_instroot_cust_sz {

	# May as well keep going if no root size to work with.
	[[ -s "${INSTROOT_CUST_SZ}" ]] || return 0

	# NOTE: required space should already contain a fudge factor
	required=`${CAT} ${INSTROOT_CUST_SZ}`
	available=`${DF} /usr | ${AWK} '(NR>1) {print $3}'`

	# expand if necessary
	if (( ${required} > ${available} ))
	then
		let "additional=required-available"
		# try to expand the filesystem
		${CHFS} -a size=+${additional} /usr 2>${ERR} 1>&2 || \
                                err_from_cmd ${CHFS}

	fi
	rm ${INSTROOT_CUST_SZ}

	return 0

} # end of ck_instroot_cust_sz

#---------------------------- prep_bundle      --------------------------------
#
# NAME: prep_bundle
#
# FUNCTION:
#               prepares environment to use an installp bundle file
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#               calls error on failure
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#               parameters:
#               global:
#                       bundle
#                       bundle_access
#
# RETURNS: (int)
#               0   = success
#
# OUTPUT:
#-------------------------------------------------------------------------------

function prep_bundle {
        [[ -z "${bundle}" ]] && return 0

        bundle_access=${TMPDIR}/bundle
        # local or remote file?
        if [[ ${bundle} = ?*:/* ]]
        then

                # remote - setup local access
                nim_mount ${bundle}

                # copy the bundle into /tmp so installp will have access to it
                ${CP} ${access_pnt} ${bundle_access} 2>${ERR} || \
                        err_from_cmd "${CP} ${access_pnt} ${bundle_access}"

                # unmount
                nim_unmount ${access_pnt}

        else

                # already local - just copy it
                ${CP} ${bundle} ${bundle_access} 2>${ERR} || \
                        err_from_cmd "${CP} ${bundle} ${bundle_access}"

        fi

} # end of prep_bundle

#---------------------------- lppchk_uv_root  --------------------------------
#
# NAME: lppchk_uv_root
#
# FUNCTION:
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error on failure
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= pathame of the root directory to sync
#		global:
#			installp_flags
#			filesets
#			bundle
#			tmp_files
#			chroot
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function lppchk_uv_root {
	typeset root=${1}
	typeset op=""
	typeset name=""
	typeset lev=""

	# setup the chroot environment using the specified root directory
	setup_chroot_env ${root}

	# perform the lppchk operation to find out what needs to be done
        # to sync up the current root.
	${chroot} ${LPPCHK} -uv | \
		while read op name lev
		do
    			case ${op} in
       				-C)  echo ${name} ${lev} >> ${cleanup_sync_list}
           			;;
       				-a)  echo ${name} ${lev} >> ${apply_sync_list}
           			;;
      				-ac) echo ${name} ${lev} >> ${apply_sync_list}
      		  	     	     echo ${name} ${lev} >> ${commit_sync_list}
				;;
       				-c)  echo ${name} ${lev} >> ${commit_sync_list}
				;;
       				-u)  echo ${name} ${lev} >> ${deinstl_sync_list}
				;;
       				-r)  echo ${name} ${lev} >> ${reject_sync_list}
				;;
       				ERR) # do nothing
				;;
				\?)  # do nothing
				;;
			esac
		done

} # end of lppchk_uv_root

#---------------------------- sync_root         --------------------------------
#
# NAME: sync_root
#
# FUNCTION:
#		performs the specified installp operation on the specified root directory
#			by setting up a chroot environment
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error on failure
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= pathame of the root directory to sync
#		global:
#			installp_flags
#			filesets
#			bundle
#			tmp_files
#			chroot
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function sync_root {
	typeset root=${1}
	typeset name=${root##*/}
	typeset f_flag=""

	# Setup the chroot environment using the specified root directory.
	# (if we're doing a full_sync, we should have already done this.)
	if [[ -z "${full_sync}" ]]
	then
		setup_chroot_env ${root}
	fi

	# use a bundle file?
	if [[ -n "${bundle}" ]]
	then

		# copy the bundle into the root directory
		${CP} ${bundle} ${root}/bundle 2>${ERR} || err_from_cmd ${CP}
		tmp_files="${tmp_files} ${root}/bundle"

		# use the bundle
		f_flag="-f /bundle"

	fi

	# perform the installp operation
	export INUCLIENTS=yes
	${chroot} ${INSTALLP} -e ${INSTALLP_LOG} ${installp_flags} ${f_flag} \
		${filesets} </dev/null 2>${ERR} 1>&2

	rc=$?
	# Don't exit if doing a complete sync op
	if [[ -n "${full_sync}" ]]
	then
		if [[ $rc -ne 0 ]] 
		then
			warning_from_cmd ${INSTALLP}
		fi
	else 	
		if [[ $rc -ne 0 ]]
		then
			err_from_cmd ${INSTALLP}
		fi
		exit 0
	fi

} # end of sync_root

#---------------------------- full_sync_root -------------------------------
#
# NAME: full_sync_root
#
# FUNCTION:
#		
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= pathame of the root directory to sync
#		global:
#			installp_flags
#			filesets
#			bundle
#			tmp_files
#			chroot
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function full_sync_root {
	typeset orig_flags="${installp_flags}"
	lppchk_uv_root ${1}
	if [[ -s "${cleanup_sync_list}" ]]
	then
		installp_flags="-C ${orig_flags}"	
		sync_root ${1}
	fi

	if [[ -s "${apply_sync_list}" ]]
	then
		installp_flags="-aXg ${orig_flags}"	
		bundle="${apply_sync_list}"
		sync_root ${1}
	fi

	if [[ -s "${commit_sync_list}" ]]
	then
		installp_flags="-cg ${orig_flags}"	
		bundle="${commit_sync_list}"
		sync_root ${1}
	fi

	if [[ -s "${deinstl_sync_list}" ]]
	then
		installp_flags="-ugX ${orig_flags}"	
		bundle="${deinstl_sync_list}"
		sync_root ${1}
	fi

	if [[ -s "${reject_sync_list}" ]]
	then
		installp_flags="-rgX ${orig_flags}"	
		bundle="${reject_sync_list}"
		sync_root ${1}
	fi

	exit 0

} # end of full_sync_root

#---------------------------- lslpp_root         --------------------------------
#
# NAME: lslpp_root
#
# FUNCTION:
#		displays LSLPP info for the specified root
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error on failure
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= pathame of the root directory to sync
#		global:
#			chroot
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function lslpp_root {
	typeset root=${1}

	# setup the chroot environment using the specified root directory
	setup_chroot_env ${root}

	# display the LSLPP info
	${chroot} ${LSLPP} -al 2>${ERR} || err_from_cmd ${LSLPP}

	exit 0

} # end of lslpp_root

#---------------------------- c_sync_root       --------------------------------
#
# NAME: c_sync_root
#
# FUNCTION:
#		performs installp operations on diskless/dataless root parts
#		this is done by mounting the SPOT over each clients root, then chrooting
#			into the client's root and performing the requested installp operation
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		this script will stop when the first time installp returns an error
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#		0							= installp success on all roots
#		1							= installp or other error encountered
#
# OUTPUT:
#-------------------------------------------------------------------------------
# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
typeset c=""
typeset sync_one=""
typeset list_only=""
typeset params=""
typeset root=""
typeset name=""
typeset rc=0
typeset bundle_access=""
typeset full_sync=""

# set parameters from command line
while getopts :a:lqsvf c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
		;;

		f)		# Doing a full sync op 
				full_sync=TRUE
		;;
		l)		# list LPP info
				list_only=TRUE
		;;

		q)		# show attr info
				cmd_what
				exit 0
		;;

		s)		# perform one root sync only
				sync_one=TRUE
		;;

		v)		# verbose mode (for debugging)
				set -x
				for i in $(typeset +f)
				do
					typeset -ft $i
				done
		;;

		\?)	# unknown option
				error ${ERR_BAD_OPT} ${OPTARG}
		;;
	esac
done
shift OPTIND-1

# check for missing attrs
ck_attrs

# do lslpp stuff and exit if that's what we were called to do
if [[ -n "${list_only}" ]]
then

	lslpp_root ${1}

fi

# were neither filesets nor bundle file specified?
if [[ -z "${filesets}" ]] && [[ -z "${bundle}" ]]
then
	if [[ -z "${installp_flags}" ]]
	then
		full_sync=TRUE

	elif [[ "${installp_flags}" = *a* ]]
	then
			filesets=all		
	fi
# either filesets or bundles were specified, how about installp flags?
elif [[ -z "${installp_flags}" ]]
then
		# nope, assume apply operation.
		installp_flags="-agX"
fi

# make sure "Or" is in the installp_flags
[[ "${installp_flags}" != *Or* ]] && installp_flags="-Or ${installp_flags}"

# make sure dashes are acceptable
installp_flags=$( ck_installp_flags "${installp_flags}" )

# perform one root sync (if that's what we were called to do)
if [[ -n "${sync_one}" ]]
then

	
	if [[ -n "${full_sync}" ]]
	then
		# perform all installp ops necessary to sync root
		full_sync_root ${1}
	else
		# If syncing the inst_root of a /usr SPOT, 
		#   a) do some space checking and auto-expand if necessary.
		#   b) prep bundle file if necessary (since we didn't follow
		#      the normal path of execution for this sync op).
                if [[ ${1} = "/usr/lpp/bos/inst_root" ]]
                then
			[[ "${installp_flags}" = *a* ]] && ck_instroot_cust_sz
                        [[ -n "${bundle}" ]] && prep_bundle && \
                                       bundle=${bundle_access}
                fi

		# perform installp op that was specified
		sync_root ${1}
	fi
fi
# else, perform multiple root syncs in parallel

# Before doing multiple root syncs, do some semantic checking on any user 
# specified arguments.  (Do it here rather than prior to sync_root call so 
# that it's not performed for every client -- not necessary.)

# was bundle specified?
if [[ -n "${bundle}" ]]
then

	# check for errors
	[[ -n "${filesets}" ]] && \
		error ${ERR_ATTR_CONFLICT} bundle filesets

	# setup local access
	prep_bundle

elif [[ ${installp_flags} = *u* ]] || [[ ${installp_flags} = *c* ]] || \
     [[ ${installp_flags} = *+([ -])r* ]]
then

	# this is a remove or commit operation: filesets must be specified
	[[ -z "${filesets}" ]] && error ${ERR_MISSING_ATTR} filesets

fi

# for each root pathname given as an operand...
for root
do

	# sync this root directory in the background
	${C_SYNC_ROOT} -s -a location=${location} \
		-a installp_flags="${installp_flags}" \
		${bundle:+"-abundle=${bundle_access}"} \
		${filesets:+-afilesets="${filesets}"} \
		${full_sync:+-f} \
		${root} 2>${TMPDIR}/${root##*/}.failed &

done

# wait for all syncs to finish
wait

# look for failures
failed=$( ${LS} -1 ${TMPDIR}/*.failed 2>/dev/null )
if [[ -n "${failed}" ]]
then

	for root in ${failed}
	do

		# make sure the operation really failed (ie, there's output in the file)
		[[ ! -s ${root} ]] && continue

		name=${root%.failed}
		name=${name##*/}

		print "\t${name}"

		rc=${ERR_ROOT_SYNC}

	done

fi

# all done
exit ${rc}

