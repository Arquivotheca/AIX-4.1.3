#!/bin/ksh
# @(#)83        1.23.1.6  src/bos/usr/lib/nim/methods/c_ckspot.sh, cmdnim, bos41J, 9511A_all  3/2/95  16:01:49
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_ckspot.sh
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
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
PARTITION_SIZE=4194304
DEFAULT_TFTPBOOT_SPACE=3100

FS_FUDGE=10							# this fudge factor is applied to the SPOT size
										# 		calculation in order to increase the size
										#		requirements
										# FS_FUDGE is a denominator, so 10 = 10% increase

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location name st_applied st_committed platform"
OPTIONAL_ATTRS="spot_options lslpp_flags debug no_mkbooti auto_expand \
installp_flags fix_query_flags fixes fix_bundle"
typeset -i version=0
typeset -i release=0
location=""
name=""
spot_options="${DEFAULT_SPOT_OPTIONS}"
list_only=""
tftpboot_fs=""
tftpboot_available=""
tmp_available=""
verbose=""
installp_flags=""
fix_query=""
bundle_access=""

#---------------------------- up_stanza         --------------------------------
#
# NAME: up_stanza
#
# FUNCTION:
#		updates the specified image.data stanza
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error on failure
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
# parameters:
#	fs	= name of filesystem to update
#	blocks	= size of <fs> in 512 byte blocks
# global:
#
# RETURNS: (int)
#	0	= success
#	1	= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function up_stanza {

	typeset fs=${1}
	typeset -i blocks=${2}
	typeset rc=0

	# how many 4 meg partitions?
	typeset -i partitions="((${blocks}*512)/${PARTITION_SIZE})+1"

	# update image.data
	if ${AWK} -v fs_name=${fs} -v LPs=${partitions} -v FS_SIZE=${blocks} \
	  -f ${NIM_AWK}/image_data.awk ${image_data} >${TMPDIR}/image.data \
	  2>${ERR}
	then

		${CAT} ${TMPDIR}/image.data >${image_data} 2>${ERR} || rc=1

	else

		rc=1

	fi

	[[ ${rc} = 1 ]] && warning ${ERR_FILE_MOD} ${image_data}

	return ${rc}

} # end of up_stanza

#---------------------------- up_image_data     --------------------------------
#
# NAME: up_image_data 
#
# FUNCTION:
#  updates the image.template file within the SPOT to reflect its current
#	size this file is used by BOS install when the source for the BOS 
#       files is a SPOT
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
#		global:
#
# RETURNS: (int)
#		0 = success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function up_image_data {

	typeset image_data=${location}/${IMAGE_DATA_IN_SPOT}
	typeset -i size=0
	typeset rc=0

	#-------------------------- NOTE ------------------------------
	#	using "find -ls" here because it will return the uncompressed size
	#	we were using "du -s", but that gives us compressed sizes, which will be
	#		much less than is needed for compressed filesystems
	typeset usr_size=$(${FIND} ${location} -xdev -ls | \
							${AWK} 'END{print (total/512)+1};{add=4096-($7%4096);total+=$7+add}' 2>/dev/null)
	typeset inst_root_size=$(${FIND} ${location}/${INST_ROOT} -xdev  -ls | \
							${AWK} 'END{print (total/512)+1};{add=4096-($7%4096);total+=$7+add}' 2>/dev/null)
	typeset var_size=$(${FIND} ${location}/${INST_ROOT}/var -xdev -ls | \
							${AWK} 'END{print (total/512)+1};{add=4096-($7%4096);total+=$7+add}' 2>/dev/null)

	# make sure the image.data file exists and is writeable
	if [[ ! -s "${image_data}" ]]
	then

		# warn user
		warning ${ERR_FILE_ACCESS} ${image_data}
		return 1

	else

		if ${CHMOD} +w ${image_data} 2>${ERR}
		then
			:
		else

			warning_from_cmd ${CHMOD}
			return 1

		fi

	fi

	# make sure sizes are valid
	[[ -z "${usr_size}" ]] && warning ${ERR_SIZE} ${location}
	[[ -z "${inst_root_size}" ]] && warning ${ERR_SIZE} ${location}/${INST_ROOT}
	[[ -z "${var_size}" ]] && warning ${ERR_SIZE} ${location}/${INST_ROOT}/var
	if [[ -z "${usr_size}" ]] ||
		[[ -z "${inst_root_size}" ]] ||
		[[ -z "${var_size}" ]]
	then

		return 1

	fi

	# apply fudge factor to /var
	let size=${var_size}+(${var_size}/${FS_FUDGE})

	# update info for /var
	up_stanza /var ${size} || rc=1

	# apply fudge factor to /
	let size="${inst_root_size}+(${inst_root_size}/${FS_FUDGE})"

	# update info for /
	up_stanza / ${size} || rc=1

	# apply fudge factor to /usr
	let size=${usr_size}+(${usr_size}/${FS_FUDGE})

	# update info for /usr
	up_stanza /usr ${size} || rc=1

	return ${rc}

} # end of up_image_data

#---------------------------- ck_booti          --------------------------------
#
# NAME: ck_booti
#
# FUNCTION:
#		checks the TFTPBOOT directory for boot images which have been created
#			for the SPOT
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#		writes ATTR_IF_SUPPORTED attr assignments to stdout
#-------------------------------------------------------------------------------
function ck_booti {

	typeset ifs=""
	typeset if_type=""
	typeset i=""

	# for each boot image the SPOT currently has...
	for i in $(ls ${TFTPBOOT}/${name}.*.[mu]p 2>/dev/null | \
					${AWK} -F "." '{print $2}' 2>/dev/null)
	do

		# interface type already seen?
		if [[ "${ifs}" != " ${i} " ]]
		then

			# nope - add to list and print it
			ifs="${ifs} ${i} "
			print "if_supported=${i}"

		fi

	done

	[[ -z "${ifs}" ]] && print "missing=\"network boot image\""

} # end of ck_booti

#---------------------------- list_installed_info ---------------------------
#
# NAME: list_installed_info 
#
# FUNCTION:
#		displays either:
#               1) a list of options, via lslpp, which have been installed 
#		   into a SPOT (or the /usr filesystem of a client)
#                                    **** OR ****
#		2) installed fix information, via instfix, for a SPOT
#		   (or the /usr filesystem of a client)
#
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error on failure
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#  parameters:
#  global:
#   chroot  = command to execute in order to be in correct environment for 
#		performing the lslpp
#
# RETURNS: (int)
#	0	= success
#
# OUTPUT:
#		option info displayed on stdout
#-------------------------------------------------------------------------------
function list_installed_info {

        # make sure flags are set
        lslpp_flags=${lslpp_flags:-"-al"}

        >${ERR}

        # display fix info?
        if [ -n "${fix_query}" ]
        then
		# need to pre-process optional flags to instfix?
		if [ -n "${fix_query_flags}" ]
		then
			# use installp's "flag fixer-upper" to remove and/or
			#	add "-" as appropriate.
			fix_query_flags=$( ck_installp_flags "${fix_query_flags}" )
			# Make sure flags are one of those listed in getopt
			getopt cqvFa ${fix_query_flags} 1>/dev/null
			[ $? -ne 0 ] && error ${ERR_BAD_FIXQ_FLAG}
		fi

		# Call instfix with the appropriate flag depending upon
		#	whether fixes, bundles or neither were specified
                if [ -n "${fixes}" ]
                then
                        ${chroot} ${INSTFIX} -ik "${fixes}" \
                                                ${fix_query_flags} 2>${ERR}
                elif [ -n "${fix_bundle}" ]
		then
			# prepare local access to bundle (use the installp
			#	bundle prep code for convenience) 
			installp_bundle=${fix_bundle}
			prep_bundle

                        ${chroot} ${INSTFIX} -if "${bundle_access}" \
                                                ${fix_query_flags} 2>${ERR}
		else
                        ${chroot} ${INSTFIX} -i ${fix_query_flags} 2>${ERR}

                fi

		# NOTE on error output from instfix: instfix sometimes returns
		#	non-zero, yet all output goes to stdout.  Other times
		#	it writes "more severe" errors to stderr while returning
		#	non-zero.  We'll only do error processing if it wrote 
		#	to stderr.
		[ -s ${ERR} ] && err_from_cmd ${INSTFIX}
        else
                # display installed lpp fileset info
                ${chroot} ${LSLPP} ${lslpp_flags} \
                                2>${ERR} || err_from_cmd ${LSLPP}
        fi

} # end of list_installed_info

#---------------------------- ck_bosboot_space  --------------------------------
#
# NAME: ck_bosboot_space
#
# FUNCTION:
#		uses the preview option on bosboot to determine whether there'e enough
#			free space to run bosboot
#		if not, and auto_expand was specified, we'll expand the filesystems
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
#		global:
#			tftpboot_fs
#			tftpboot_available
#			tmp_available
#			chroot
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function ck_bosboot_space {

	typeset -i tftpboot_needed=0
	typeset -i tmp_needed=0
	typeset -i root_needed=0
	typeset -i booti_already=0

	# chroot into the SPOT and call C_MKBOOTI with preview option ("-p")
	${chroot} ${C_MKBOOTI} -p -a location=/usr -a name=${name} \
		-a platform="${platform}" ${verbose:+-v} >${TMPDIR}/bosboot.sizes 2>${ERR}

	[[ -n "${verbose}" ]] && ${CAT} ${ERR}

	# include the bosboot space requirements
	[[ -s ${TMPDIR}/bosboot.sizes ]] && . ${TMPDIR}/bosboot.sizes

	# is there enough free space in /tmp?
	if (( tmp_needed > tmp_available ))
	then

		# should we expand /tmp?
		if [[ "${auto_expand}" = "no" ]]
		then

			print "missing=\"${MSG_MISSING_BOOTI}\""
			error ${ERR_SPACE} /tmp "${tmp_needed}" "${tmp_available}"

		fi

		# subtract the currently available space
		let "tmp_needed-=tmp_available"

      # multiply the required size by 2 because its currently specified with
      #     block size of 1024 and chfs wants a block size of 512
      let "tmp_needed*=2"

		# expand to fill requirements
		if ${CHFS} -a size=+${tmp_needed} /tmp 2>${ERR} 1>&2
		then
			:
		else

			print "missing=\"${MSG_MISSING_BOOTI}\""
			err_from_cmd "${CHFS} /tmp"

		fi

	fi

	# in a chroot environment, bosboot seems to return "0" for space needed
	#		for /tftpboot, so, we'll use a default
	(( tftpboot_needed <= 0 )) && let "tftpboot_needed=${DEFAULT_TFTPBOOT_SPACE}"

	# take into account any boot images which already exist in TFTPBOOT for
	#		this SPOT
	# since we haven't removed them yet, they're taking up space which will
	#		be free'd up by C_MKBOOTI
	# how many boot images in TFTPBOOT already?
	let "booti_already=$( ${LS} -1 $TFTPBOOT/${name}.* 2>/dev/null | ${WC} -l )"

	# add back in the space we'll get when those boot images are deleted
	let "tftpboot_available+=(booti_already*tftpboot_needed)"

	# is there enough free space in TFTPBOOT?
	# multiply out the number of boot images we can make
	#
	let "tftpboot_needed*=num_booti"
	if (( tftpboot_needed > tftpboot_available ))
	then

		# should we expand TFTPBOOT?
		if [[ "${auto_expand}" = "no" ]]
		then

			print "missing=\"${MSG_MISSING_BOOTI}\""
			error ${ERR_SPACE} ${TFTPBOOT} \
				"${tftpboot_needed}" "${tftpboot_available}"

		fi

		# subtract the currently available space
		let "tftpboot_needed-=tftpboot_available"

		# multiply the required size by 2 because its currently specified with
		#     block size of 1024 and chfs wants a block size of 512
		let "tftpboot_needed*=2"

		# expand to fill requirements
		if ${CHFS} -a size=+${tftpboot_needed} ${tftpboot_fs} 2>${ERR} 1>&2
		then
			:
		else

			print "missing=\"${MSG_MISSING_BOOTI}\""
			err_from_cmd "${CHFS} ${tftpboot_fs}"

		fi

	fi

} # end of ck_bosboot_space

#---------------------------- c_ckspot          --------------------------------
#
# NAME: c_ckspot
#
# FUNCTION:
#   checks the specified SPOT for options which are required in order to
#   make the SPOT functional to NIM when the "-l" option is used, c_ckspot 
#   lists option information only
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#  this method does NOT create a SPOT - that work is done by c_mkspot
#   ( well duh...)
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#	0  = SPOT installed
#	1  = error encountered - message on stderr
#
# OUTPUT:
#  stdout = contains option info when "-l" option used
#  stderr = contains attr assignments for missing options
#-------------------------------------------------------------------------------

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
typeset c=""
typeset rc=0
typeset just_update=""

# set parameters from command line
while getopts :a:lqfuv c
do
	case ${c} in

		a) 	# validate the attr ass
			parse_attr_ass "${OPTARG}"

			# include the assignment for use in this environment
			eval ${variable}=\"${value}\"
		;;

		l)	# list option info only
			list_only=yes
		;;

		f)	# list fix info only
			list_only=yes
			fix_query=yes
		;;

		q)	# show attr info
			cmd_what
			exit 0
		;;

		u)	# just update the image.template file
			just_update=TRUE
		;;

		v)	# verbose mode (for debugging)
			verbose=TRUE
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

# check for missing attrs
ck_attrs

# check for auto expand
[[ "${installp_flags}" = *X* ]] && auto_expand="yes"

# just listing LPP info?
if [[ -z "${list_only}" ]]
then
	# make sure the inst.images directory exists
	# this directory is required because the lpp_source will be 
	# mounted for this directory during a network boot to perform a 
	# BOS install
	if [[ ! -d ${location}/sys/inst.images ]]
	then
		# create it
		${MKDIR} -p ${location}/sys/inst.images 2>${ERR} || err_from_cmd ${MKDIR}

	fi

	# "runtime" PdAt entry must exist or savebase will fail
	if [[ -z "$( ODMDIR=${location}/lib/objrepos ${ODMGET} \
			-quniquetype=runtime PdAt 2>/dev/null )" ]]
	then

		# add it so savebase is happy when called during 
		#  diskless/dataless boot
		print "PdAt:\n\tuniquetype = \"runtime\"" >${TMPDIR}/runtime.PdAt
		ODMDIR=${location}/lib/objrepos ${ODMADD} ${TMPDIR}/runtime.PdAt \
			2>${ERR} || warning_from_cmd ${ODMADD}

	fi

	# update filesystem sizes in the image.template file (BOS install 
	# uses it)
	# NOTE that we do this BEFORE setting up the chroot env because we 
	# don't want to include anything that's not normally there
	#
	if up_image_data 2>${ERR}
	then
		:
	elif [[ -s ${ERR} ]]
	then

		# add an attribute assignment to cache the warning
		print "info=$( ${CAT} ${ERR} )"
		>${ERR}

	fi
	[[ -n "${just_update}" ]] && exit 0

	# cache name of filesystem where /tftpboot resides
	# we do this in preparation for checking bosboot free space requirements
	tftpboot_fs=$( ${DF} -Pk ${TFTPBOOT} 2>/dev/null | ${AWK} 'NR==2{print $6}' )

	# get the current free space for TFTPBOOT & /tmp
	tftpboot_available=$(	${DF} -Pk ${TFTPBOOT} 2>/dev/null | \
									${AWK} 'NR==2{print $4}' )
	tmp_available=$( ${DF} -Pk /tmp 2>/dev/null | ${AWK} 'NR==2{print $4}' )

fi

# setup the chroot environment
chroot=""
setup_chroot_env

# just list info or actually check?
if [[ -n "${list_only}" ]]
then

	list_installed_info
	exit 0

fi

# NOTE that in the section which follows, stdout is not directed anywhere
# this is important because these functions write information to stdout which
#		needs to get back to the caller

# check for any missing options
ck_spot_options ${location} "${spot_options}" 1>>${TMPDIR}/missing

# anything missing?
if [[ -s ${TMPDIR}/missing ]]
then

	# display the missing attrs
	${CAT} ${TMPDIR}/missing 2>/dev/null

	# were any of the critical filesets missing?
	# we check for these explicitly because they are not used to create
	# network boot images, which means that we may be able to create boot
	# images, but the SPOT would still be unusable because these 
	# are missing

	if [[ -n "$( ${GREP} bos.sysmgt.nim.client ${TMPDIR}/missing )" ]] ||
		[[ -n "$( ${GREP} bos.sysmgt.nim.spot ${TMPDIR}/missing )" ]] ||
		[[ -n "$( ${GREP} bos.sysmgt.sysbr ${TMPDIR}/missing )" ]] ||
		[[ -n "$( ${GREP} bos.net.tcp.client ${TMPDIR}/missing )" ]] ||
		[[ -n "$( ${GREP} bos.net.nfs.client ${TMPDIR}/missing )" ]]
	then

		# SPOT cannot support all types of NIM network boot operations
		print "missing=\"network boot image\""
		no_mkbooti="yes"

	fi

	[[ "${no_mkbooti}" = "yes" ]] && error ${ERR_MISSING} ${name}
fi

if [[ "${no_mkbooti}" != "yes" ]]
then
	# update boot images
	if [[ ! -f ${location%/usr}${C_MKBOOTI} ]]
	then
		print "missing=\"network boot image\""
		error ${ERR_ENOENT} ${location%/usr}${C_MKBOOTI}
	else
		# make sure C_MKBOOTI is executable
		if [[ ! -x ${location%/usr}${C_MKBOOTI} ]]
		then
			if ${CHMOD} +x ${location%/usr}${C_MKBOOTI} 2>${ERR}
			then
				:
			else
				print "missing=\"${MSG_MISSING_BOOTI}\""
				err_from_cmd ${CHMOD}
			fi
		fi
	
		if [[ -n "${new_root}" ]]
		then
			# C_MKBOOTI needs NIMINFO file - copy it
			${CP} ${NIMINFO} ${new_root}${NIMINFO} 2>${ERR} || \
				err_from_cmd "${CP} ${NIMINFO} ${new_root}"
		fi
	
		# check the bosboot's free space requirements & auto 
		# expand if specified
		ck_bosboot_space

		#
		# We're committed to re-building the boot images, so
		# delete all existing boot images for this SPOT.
		#
		if [[ -n "$( ${LS} ${TFTPBOOT}/${name}.* 2>/dev/null )" ]]
		then
			${RM} ${TFTPBOOT}/${name}.* 2>${ERR} || err_from_cmd ${RM}
		fi

		>${ERR}
		${chroot} ${C_MKBOOTI} ${verbose:+-v} ${debug:+-a debug=yes} \
			-a platform="${platform}" -a location=/usr -a name=${name} \
			2>${ERR} || rc=2
	
		[[ -n "${new_root}" ]] && ${RM} ${new_root}${NIMINFO} 2>/dev/null
	
		# any C_MKBOOTI errors?
		if [[ ${rc} = 2 ]]
		then
	
			err_from_cmd ${C_MKBOOTI}
	
		elif [[ -s ${ERR} ]]
		then
	
			warning_from_cmd ${C_MKBOOTI}
	
		fi
	
	fi

fi

# all done
exit ${rc}
