#!/bin/ksh
# @(#)12        1.20  src/bos/usr/lib/nim/methods/c_mkspot.sh, cmdnim, bos41J, 9520A_all  5/15/95  16:15:57
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mkspot.sh
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
typeset -i FUDGE_FACTOR=5

TAR_CREATE="${TAR} -cdpf - -C"
TAR_EXTRACT="${TAR} -xpf -"

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location name source type st_applied st_committed"
OPTIONAL_ATTRS="version release installp_flags auto_expand"
location=""
name=""
source=""
type=""
version=""
release=""
src_access=""
refresh=""
source_is_tape=""
tftp_enabled_by_nim=""

#---------------------------- ck_size_req       --------------------------------
#
# NAME: ck_size_req
#
# FUNCTION:
#		verifies that the target filesystem (where the SPOT is being created) has
#			enough free space to store the SPOT files (which will come from the
#			source)
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
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function ck_size_req {

	# don't check if this is for a /usr conversion
	[[ ${location} = /usr ]] && return 0

	typeset spot_fs=""
	typeset first_dir=""
	typeset image_data_file=${1}
	typeset -i size_req=0
	typeset -i free_blocks=0

	# can we read the file?
	[[ ! -r "${image_data_file}" ]] && \
		error ${ERR_FILE_ACCESS} ${image_data_file}

	# parse out the size requirement
	${AWK} '	/^[ 	]*$/ { fs_data = 0; fs_name = 0;};\
				$1 == "fs_data:" {fs_data = 1;};\
				$1 == "FS_NAME=" {if ( $2 == "/usr" ) fs_name = 1;};\
				$1 == "FS_SIZE=" {if (fs_data && fs_name) print "size_req=" $2;};' \
		${image_data_file} >${TMPDIR}/size_req 2>${ERR} || err_from_cmd ${AWK}
	[[ -s ${TMPDIR}/size_req ]] && . ${TMPDIR}/size_req || \
		error ${ERR_SIZE} ${source}

	# find the first directory which currently exists in the pathname of the
	#     target location
	first_dir=${location}
	while [[ -n "${first_dir}" ]]
	do
		[[ -d ${first_dir} ]] && break
		first_dir=${first_dir%/*}
	done

	# what filesystem will the SPOT reside in?
	spot_fs=$( ${DF} -P ${first_dir} 2>${ERR} | ${AWK} 'NR==2{print $6}' )
	[[ -z "${spot_fs}" ]] && err_from_cmd ${DF}

	# how much free space in that filesystem?
	free_blocks=$( ${DF} -P ${spot_fs} 2>${ERR} | ${AWK} 'NR==2{print $4}' )
	[[ -z "${free_blocks}" ]] && err_from_cmd ${DF}

	# is there enough space?
	if (( ${size_req} > ${free_blocks} ))
	then

		# should we auto expand the filesystem?
		[[ "${installp_flags}" != *X* ]] && [[ "${auto_expand}" = "no" ]] && \
			error ${ERR_SPACE} ${spot_fs} ${size_req} ${free_blocks}

		# subtract the currently available space
		let "size_req-=free_blocks"

		# expand the filesystem
		${CHFS} -a size=+${size_req} ${spot_fs} 2>${ERR} 1>&2 || \
			err_from_cmd ${CHFS}

	fi

} # end of ck_size_req

#---------------------------- prep_tape         --------------------------------
#
# NAME: prep_tape  
#
# FUNCTION:
#		prepares the specified tape device for use
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_tape {

	typeset bs=""

	source_is_tape=TRUE

	# tape needs a "no-rewind-on-close" extension 
	# remove whatever extension was given and use ".1"
	src_access="${source%.*}.1"

	# cache the logical device name of the tape drive
	tape_device_name=${source##*/}
	tape_device_name=${tape_device_name%.*}

	# get current block_size
	bs=$( ${LSATTR} -El ${tape_device_name} 2>${ERR} | \
			${AWK} '$1=="block_size"{print $2}' )
	[[ -z "${bs}" ]] && err_from_cmd ${LSATTR}

	# block_size needs to be 512: should we change it?
	if [[ ${bs} != 512 ]]
	then

		${CHDEV} -l ${tape_device_name} -a block_size=512 1>/dev/null 2>${ERR} ||\
			err_from_cmd ${CHDEV}

		# by setting the global "tape_block_size" var, the cleanup function will
		#		reset the block_size to the original value
		tape_block_size=${bs}

	fi

	# rewind the tape
	${TCTL} -f ${src_access} rewind 2>${ERR} || err_from_cmd ${TCTL}

	# position to 4th record
	${TCTL} -f ${src_access} fsf 3 2>${ERR} || err_from_cmd ${TCTL}

	# need to validate the AIX release level of this image & determine its
	#		size requirements
	# therefore, restore the LPP_NAME and IMAGE_DATA files now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${src_access} ${LPP_NAME} ${IMAGE_DATA} 2>${ERR} 1>&2 || \
		err_from_cmd "${RESTORE} -xqf ${src_access} ${LPP_NAME} ${IMAGE_DATA}"

	# validate the release level
	ck_rel_level ${TMPDIR}/${LPP_NAME}

	# make sure there's enough space to create the new SPOT
	ck_size_req ${TMPDIR}/${IMAGE_DATA}

	# pop_spot expects tape to be positioned to the BOS image, so reposition 
	#		to 4th record now
	${TCTL} -f ${src_access} rewind 2>${ERR} || err_from_cmd ${TCTL}
	${TCTL} -f ${src_access} fsf 3 2>${ERR} || err_from_cmd ${TCTL}

	# initialize the method to restore the files
	restore_func="${RESTORE} -xqf ${src_access}"

} # end of prep_tape

#---------------------------- prep_cd           --------------------------------
#
# NAME: prep_cd
#
# FUNCTION:
#		prepares a CDROM device as the source for a SPOT
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_cd {

	# mount the CDROM
	nim_mount ${source}
	src_access=${access_pnt}/${BOS_PATH_ON_CDROM}

	# need to validate the AIX release level of this image & determine its
	#		size requirements
	# therefore, restore the LPP_NAME and IMAGE_DATA files now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${src_access} ${LPP_NAME} ${IMAGE_DATA} >/dev/null \
		2>${ERR} || err_from_cmd ${RESTORE}

	# validate the release level
	ck_rel_level ${TMPDIR}/${LPP_NAME}

	# make sure there's enough space to create the new SPOT
	ck_size_req ${TMPDIR}/${IMAGE_DATA}

	# initialize the method to restore the files
	restore_func="${RESTORE} -xqf ${src_access}"

} # end of prep_cd

#---------------------------- prep_SPOT         --------------------------------
#
# NAME: prep_SPOT
#
# FUNCTION:
#		prepares an existing SPOT to be used as a source for the creation of the
#			new SPOT
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_SPOT {

	typeset -i spot_version=0
	typeset -i spot_release=0

	# make sure source pathname ends with "/usr"
	if [[ "${source##*/}" != usr ]]
	then
		source=source/usr
	fi

	# setup local access
	nim_mount ${source}
	src_access=${access_pnt}

	# validate the version/release level by looking in the product database of
	#		the SPOT
	ODMDIR=${src_access}/lib/objrepos ${ODMGET} -qlpp_name=bos.rte product \
		>${TMPDIR}/spot.product 2>${ERR} || err_from_cmd ${ODMGET}
	let spot_version=$( ${AWK} '$1=="ver"{print $3}' ${TMPDIR}/spot.product \
								2>/dev/null )
	let spot_release=$( ${AWK} '$1=="rel"{print $3}' ${TMPDIR}/spot.product \
								2>/dev/null )
	if ((${spot_version}<${version})) ||
		((${spot_release}<${release}))
	then
		error ${ERR_RELEASE} ${spot_version} ${spot_release} ${source}
	fi

	# make sure there's enough space to create the new SPOT
	ck_size_req ${src_access}/${IMAGE_DATA_IN_SPOT}

	# append "/usr" to <location> (because "/usr" part will not come from source)
	location=${location}/usr

	# initialize the method to restore the files
	restore_func="copy_SPOT ${src_access} ${location}"

} # end of prep_SPOT

#---------------------------- copy_SPOT         --------------------------------
#
# NAME: copy_SPOT
#
# FUNCTION:
#		copies the specified SPOT to the specified destination
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function copy_SPOT {

	typeset origin=${1}
	typeset dest=${2}

	# cd to original SPOT
	cd ${origin} 2>${ERR} || err_from_cmd cd

	# copy all the files
	${TAR_CREATE} ${origin} . | ( cd ${dest}; ${TAR_EXTRACT} ) || return 1

} # end of copy_SPOT

#---------------------------- prep_file         --------------------------------
#
# NAME: prep_file       
#
# FUNCTION:
#		prepares environment to use a file as the source
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_file {

	typeset dir=${source%/*}
	typeset filename=${source##*/}

	# setup local access to source
	nim_mount ${dir}
	src_access=${access_pnt}/${filename}

	# need to validate the AIX release level of this image & determine its
	#		size requirements
	# therefore, restore the LPP_NAME and IMAGE_DATA files now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${src_access} ${LPP_NAME} ${IMAGE_DATA} >/dev/null \
		2>${ERR} || err_from_cmd ${RESTORE}

	# validate the release level
	ck_rel_level ${TMPDIR}/${LPP_NAME}

	# make sure there's enough space to create the new SPOT
	ck_size_req ${TMPDIR}/${IMAGE_DATA}

	# initialize the method to restore the files
	restore_func="${RESTORE} -xqf ${src_access}"

} # end of prep_file

#---------------------------- prep_dir          --------------------------------
#
# NAME: prep_dir        
#
# FUNCTION:
#		prepares environment to access a directory containing a BOS image
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_dir {

	# setup local access to BOS image
	nim_mount ${source}
	src_access=${access_pnt}/bos

	# need to validate the AIX release level of this image & determine its
	#		size requirements
	# therefore, restore the LPP_NAME and IMAGE_DATA files now
	cd ${TMPDIR} 2>${ERR} || err_from_cmd cd
	${RESTORE} -xqf ${src_access} ${LPP_NAME} ${IMAGE_DATA} >/dev/null \
		2>${ERR} || err_from_cmd ${RESTORE}

	# validate the release level
	ck_rel_level ${TMPDIR}/${LPP_NAME}

	# make sure there's enough space to create the new SPOT
	ck_size_req ${TMPDIR}/${IMAGE_DATA}

	# initialize the method to restore the files
	restore_func="${RESTORE} -xqf ${src_access}"

} # end of prep_dir

#---------------------------- prep_source       --------------------------------
#
# NAME: prep_source
#
# FUNCTION:
#		prepares the specified source device for use
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_source {

	# what kind of source?
	case ${type} in

		device)	# tape or CDROM?
					if [[ ${source} = /dev/rmt[0-9]* ]]
					then
						prep_tape
					elif [[ ${source} = /dev/cd[0-9]* ]]
					then
						prep_cd
					else
						error ${ERR_SOURCE} ${source}
					fi
					;;

		spot)		# source is another SPOT
					prep_SPOT
					;;

		lpp_source) # source is directory containing a BOS image
					prep_dir
					;;

		*)			# unknown type
					error ${ERR_SYS} "unknown source type - \"${type}\""
					;;
	esac

} # end of prep_source

#---------------------------- pop_spot          --------------------------------
#
# NAME: pop_spot  
#
# FUNCTION:
#		creates a new SPOT by creating the directory and filling it files from
#			the specified source
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function pop_spot {

	# does location exist?
	if [[ ! -d ${location} ]]
	then

		# create the new directory & include the first_dir info
		${C_MKDIR} -alocation=${location} -aperms=${DEFAULT_PERMS} \
			>${TMPDIR}/tmp 2>${ERR} || err_from_cmd ${C_MKDIR}
		[[ -s ${TMPDIR}/tmp ]] && . ${TMPDIR}/tmp
		first_dir=${first_dir:-${location}}

	fi

	# let's get into the new directory
	cd ${location} 2>${ERR} || err_from_cmd cd

	# restore the source files
	[[ -z "${restore_func}" ]] && error ${ERR_SOURCE} ${source}
	if ${restore_func} >/dev/null 2>${ERR}
	then

		# keep what we've got so far
		:

	else

		# SPOT creation failed - remove the directory
		${RM} -r ${first_dir} 1>/dev/null 2>&1

		# display error msg
		err_from_cmd "${restore_func}"

	fi

} # end of pop_spot

#---------------------------- ck_this_machine -------------------------------
#
# NAME: ck_this_machine 
#
# FUNCTION:
#		checks to make sure that the machine we're currently running on is at
#			version 4 or later
#		this is important because when we build a SPOT, we need to use version 4
#			commands/libs to do it correctly
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function ck_this_machine {

	typeset msg=""
	typeset i=""
	typeset -i bos_version=0
	typeset -i bos_release=0

	# get the current version & release
	let bos_version=$(${UNAME} -v 2>/dev/null)
	let bos_release=$(${UNAME} -r 2>/dev/null)

	# check version/release
	if ((${bos_version}<${version})) || ((${bos_release}<${release}))
	then

		error ${ERR_RELEASE} ${bos_version} ${bos_release} server

	fi

	# check for options required to be a SPOT server
	if ck_spot_options /usr "${SPOT_SERVER_OPTIONS}" >${TMPDIR}/missing
	then

		# nothing else to do - server has all the required options
		return 0

	fi

	# missing some stuff - format the error message
	if [[ -s ${TMPDIR}/missing ]]
	then

		# missing some stuff - format the error message
		error ${ERR_MISSING_OPTIONS_OLD} server \
			"$( ${AWK} -F "=" '$1=="missing"{print "\t" $2}' ${TMPDIR}/missing)"

	else

		# missing some stuff - format the error message
		for i in ${SPOT_SERVER_OPTIONS}
		do

			msg="${msg}\t${i}\n"

		done
		error ${ERR_MISSING_OPTIONS_OLD} server "${msg}"

	fi

} # end of ck_this_machine

#---------------------------- secure_tftp_access ----------------------------
#
# NAME: secure_tftp_access 
#
# FUNCTION:
#		Modifies or creates /etc/tftpaccess.ctl to ensure tftp access 
#		is not granted to more than /tftpboot as a result of SPOT
#		creation on a machine in the NIM env.
#		
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		-- uncomments #allow:/tftpboot
#		-- comments out deny:/tftpboot
#		-- when looking for the above, assumes that whitespace may 
#		   have been inserted anywhere in the line (even though this
#		   is illegal).
#		-- creates file if does not exist
#		-- changes/sets permissions on /etc/tftpaccess.ctl
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#			tftp_enabled_by_nim     = tells if NIM uncommented the
#						  tftp entry in /etc/inetd.conf
#
# RETURNS: (int)
#		0				= success
#
# OUTPUT:
#-------------------------------------------------------------------------------

function secure_tftp_access {

	# if file already exists...
	if [ -s ${TFTPACCESS} ]
	then
		# is there a "deny" for /tftpboot that needs commenting out?
		DENY="^[ 	]*deny:[ 	]*\/tftpboot[ 	]*$"
		${GREP} -qE "${DENY}" ${TFTPACCESS}
		if [[ $? -eq 0 ]]
		then
			# comment it out
			${SED} "s/${DENY}/#deny:\/tftpboot/g" ${TFTPACCESS} \
					> ${TMPDIR}/.tftpaccess.ctl.$$
			[ $? -eq 0 ] && \
				mv ${TMPDIR}/.tftpaccess.ctl.$$ ${TFTPACCESS}
		fi

		# is there an "allow" for /tftpboot that needs uncommenting?
		ALLOW="^[ 	]*#[ 	]*allow:[ 	]*\/tftpboot[ 	]*$"
		${GREP} -qE "${ALLOW}" ${TFTPACCESS} 
		if [[ $? -eq 0 ]]
		then
			# uncomment the allow
			${SED} "s/${ALLOW}/allow:\/tftpboot/g" ${TFTPACCESS} \
					> ${TMPDIR}/.tftpaccess.ctl.$$
			[ $? -eq 0 ] && \
				mv ${TMPDIR}/.tftpaccess.ctl.$$ ${TFTPACCESS}
			has_access=1
		else
			# is there already access to /tftpboot?
			${GREP} -E "^allow:\/tftpboot$" ${TFTPACCESS} >/dev/null 2>&1
			[[ $? -eq 0 ]] && has_access=1
		fi
	else
		# File doesn't exist.  Leave it that way (unrestricted) if
		# NIM didn't just grant tftp access to this machine
		[[ -z "${tftp_enabled_by_nim}" ]] && return 0
	fi

	# append to file if necessary (or create if not there)
	if [[ -z "${has_access}" ]]
	then
		# add access for /tftpboot
		echo "# NIM access for network boot" >> ${TFTPACCESS}
		echo "allow:/tftpboot" >> ${TFTPACCESS}
	fi
	
	# make sure permissions and owner are correct
	${CHMOD} 644 ${TFTPACCESS}	
	${CHOWN} root.system ${TFTPACCESS}
	return 0
}

#---------------------------- usr_spot       --------------------------------
#
# NAME: usr_spot
#
# FUNCTION:
#		converts a /usr filesystem into a SPOT
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
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function usr_spot {

	typeset inst_root="${location}/${INST_ROOT}"

	# first, check to make sure that inst_roots haven't been removed
	${INURID} -q >/dev/null 2>${ERR} || err_from_cmd ${INURID}

	# remove any INST_ROOT remaining around
	# we do this to ensure we're not left with a partial INST_ROOT
	protected_dir ${inst_root} || ${RM} -r ${inst_root} 2>/dev/null

	# generate the list of inst_root files to be restored
     ${RESTORE} -Tvqf${src_access} 2>/dev/null | \
        ${GREP} inst_root | ${AWK} '{print $2}' \
        >${TMPDIR}/inst_root.files 2>${ERR} || err_from_cmd ${AWK}
	[[ ! -s ${TMPDIR}/inst_root.files ]] && \
		error ${ERR_SYS} "unable to determine the list of inst_root files"

	# "cd" to "/"
	cd / 2>${ERR} || err_from_cmd cd

	# if source is a tape...
	if [[ -n "${source_is_tape}" ]]
	then

		# rewind the tape
		${TCTL} -f ${src_access} rewind 2>${ERR} || err_from_cmd ${TCTL}

		# position to 4th record
		${TCTL} -f ${src_access} fsf 3 2>${ERR} || err_from_cmd ${TCTL}

	fi

	# restore the BOS inst_root files from the source
	${restore_func} $(${CAT} ${TMPDIR}/inst_root.files) \
		>/dev/null 2>${ERR} || err_from_cmd ${restore_func}

	# add variable to NIMINFO file
	${C_NIMINFO} -alocation=${NIMINFO} NIM_USR_SPOT=${name} 2>${ERR} || \
		err_from_cmd ${C_NIMINFO}

} # end of usr_spot

#---------------------------- up_inetd_conf      -------------------------------
#
# NAME: up_inetd_conf
#
# FUNCTION:
#		updates the inetd.conf file in leu of an AIX command to do so (inetserv
#			has been withdrawn)
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
#			1						= name of daemon to uncomment
#		global:
#			refresh				= non-NULL if inetd daemon needs to be refreshed
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function up_inetd_conf {

	typeset daemon=${1}
	typeset line=""
	typeset first_field=""

	[[ -z "${daemon}" ]] && return 1

	# make sure /etc/inetd.conf file is writable
	if [[ ! -w ${INETD_CONF} ]]
	then
		warning ${ERR_FILE_ACCESS} ${INETD_CONF}
		return 1
	fi

	# is there a line referencing <daemon> already?
	line=$(${GREP} -E ${daemon} ${INETD_CONF} 2>/dev/null)
	if [[ -z "${line}" ]]
	then
		warning ${ERR_FILE_MOD} ${INETD_CONF}
		return 1
	fi

	# is it active?
	if [[ ${line} = ${daemon}?* ]]
	then
		# already uncommented - nothing else to do
		return 0
	fi

	if [[ ${daemon} = "tftp" ]]
	then
		tftp_enabled_by_nim=1
	fi

	# uncomment the entry
	first_field=${line%%[ 	]*}
	line=${line#\#}
	if ${AWK} -v ff="${first_field}" -v new="${line}" \
		'{if ($1==ff) $0=new; print}' ${INETD_CONF} \
		>${TMPDIR}/$$.inetd_conf 2>${ERR}
	then

		if cat ${TMPDIR}/$$.inetd_conf >${INETD_CONF} 2>${ERR}
		then
			refresh=yes
		else
			warning ${ERR_FILE_MOD} ${INETD_CONF}
		fi

	else

		warning ${ERR_FILE_MOD} ${INETD_CONF}
		return 1

	fi

} # end of up_inetd_conf

#---------------------------- start_daemons     --------------------------------
#
# NAME: start_daemons
#
# FUNCTION:
#		starts the daemons required in order to access a SPOT (boopd & tftp)
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
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function start_daemons {

	# NOTE that the inetserv command is no longer available and, currently, there
	#		is no analogous command available
	# therefore, we'll update the inetd info ourselves

	refresh=""

	# for bootpd
	up_inetd_conf bootps

	# for tftp
	up_inetd_conf tftp

	# refresh the inetd daemon?
	if [[ -n "${refresh}" ]]
	then

		${REFRESH} -s inetd 2>${ERR} 1>&2 || \
			warning_from_cmd "${REFRESH} -s inetd"

	fi

} # end of start_daemons

#---------------------------- c_mkspot          --------------------------------
#
# NAME: c_mkspot
#
# FUNCTION:
#		creates a SPOT by populating the specified directory with the bos.rte
#			files from the specified source
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		this method does NOT install packages into the SPOT - that work is done
#			by c_instspot
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#		0							= SPOT created
#		1							= error encountered - message on stderr
#
# OUTPUT:
#-------------------------------------------------------------------------------

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# initialize local variables
location=
source=

# set parameters from command line
while getopts :a:qv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
				;;

		q)		# show attr info
				cmd_what
				exit 0
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

# check for missing attrs
ck_attrs

# set defaults
let version=${version:-4}
let release=${release:-0}
installp_flags=$( ck_installp_flags "${installp_flags}" )

# check this machine's version  - it must be at version 4 or greater
ck_this_machine

# need to append the SPOTname to <location>?
[[ ${location} != /usr ]] && location=${location}/${name}

# prepare the source device for use
prep_source

# /usr SPOT or new location?
if [[ ${location} = /usr ]]
then

	# converting a /usr filesystem is relatively easy, but does require some
	#		checking
	# do so now
	usr_spot

else

	# need to create a directory and populate it
	pop_spot

fi

# if we get this far, we've go something worth keeping
# return the version & release on stdout as attr assignments so the calling
#		method will pick them up
print "version=${version}"
print "release=${release}"

# start the daemons
start_daemons

# set up tftp permissions for /tftpboot
secure_tftp_access

# all done
exit 0

