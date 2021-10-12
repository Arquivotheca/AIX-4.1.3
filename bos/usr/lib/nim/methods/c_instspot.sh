#!/bin/ksh
# @(#)13        1.31  src/bos/usr/lib/nim/methods/c_instspot.sh, cmdnim, bos41J, 9523C_all  6/9/95  19:53:12
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_instspot.sh
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
SPOT_INSTALLP_FLAGS="-agQb"

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="location name"
OPTIONAL_ATTRS="lpp_source installp_bundle installp_flags filesets auto_expand fixes fix_bundle"
typeset -i version=0
typeset -i release=0
location=""
lpp_source=""
src_access=""
name=""
installp_bundle=""
bundle_access=""
installp_flags=""
filesets=""
spot_fs=""
auto_expand=""
installp_env_variable=""
fileset_list=""

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

	# tape needs a "no-rewind-on-close" extension 
	# remove whatever extension was given and use ".1"
	src_access="${lpp_source%.*}.1"

	# cache the logical device name of the tape drive
	tape_device_name=${lpp_source##*/}
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
	${TCTL} -f ${src_access} rewind 2>${ERR} || err_from_cmd tctl

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

	# mount the CDROM at a specific offset
	nim_mount ${lpp_source} ${new_root}${IMAGES}

	# images are actually at another offset
	src_access=${IMAGES}${OFFSET_FOR_CDROM}

} # end of prep_cd

#---------------------------- prep_dir         --------------------------------
#
# NAME: prep_dir       
#
# FUNCTION:
#		prepares environment to use a directory as the source for installp images
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

	# need to mount lpp_source if it's remote or this is non-/usr SPOT
	if [[ ${lpp_source} = ?*:?* ]] || [[ -n "${new_root}" ]]
	then

		# mount images at a specific offset
		nim_mount ${lpp_source} ${new_root}${IMAGES}
		src_access=${IMAGES}

	else

		# use the local directory
		src_access=${lpp_source}

	fi

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

	typeset objrepos=${new_root}/etc/objrepos
	typeset i=""

	# what kind of source?
	case ${lpp_source} in

		/dev/*)	# non-/usr SPOT?
					if [[ ${location} != /usr ]]
					then
						# we need to use the server's device database in order to
						#		access the specified device once we chroot into the SPOT
						# to do this, we'll copy the server's Cu* database into the
						#		SPOT, then create an undo_on_exit script to clean out
						#		the SPOT's database

						for i in /etc/objrepos/Cu*
						do

							# copy the object class into the SPOT
							${CP} ${i} ${objrepos} 2>${ERR} || err_from_cmd ${CP}

							# cleanup the SPOT's database when we're done
							[[ ${i} != ?*.vc ]] && \
								print "ODMDIR=${objrepos} ${ODMDELETE} -o ${i}" >> \
									undo_on_exit

						done
					fi

					# tape or CDROM?
					if [[ ${lpp_source} = /dev/rmt[0-9]* ]]
					then
						prep_tape
					elif [[ ${lpp_source} = /dev/cd[0-9]* ]]
					then
						prep_cd
					else
						error ${ERR_SYS} "invalid lpp_source device \"${lpp_source}\""
					fi
					;;

		*/*)		# source is a directory
					prep_dir
					;;

		*)			# unknown type
					error ${ERR_SYS} "unknown lpp_source type - \"${lpp_source}\""
					;;
	esac

} # end of prep_source

#---------------------------- ck_required_space --------------------------------
#
# NAME: ck_required_space
#
# FUNCTION:
#		uses the preview option on installp to determine how much space is
#			required for the operation
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
#			1						= installp flags to use
#			2						= non-NULL if auto expand used
#			3						= d_flag
#			4						= f_flag
#			5						= filesets
#		global:
#			location
#			spot_fs
#
# RETURNS: (int)
#		0							= enough room for operation
#
# OUTPUT:
#-------------------------------------------------------------------------------
function ck_required_space {

	typeset installp_flags=${1}
	typeset auto_expand=${2}
	typeset d_flag=${3}
	typeset f_flag=${4}
	typeset filesets=${5}

	typeset filesystem=""
	typeset -i required=0
	typeset -i available=0
	typeset -i additional=0

	# use the preview option with installp to get the space it needs
	# NOTE that we set the appropriate installp environment variable in order
	#		to make it behave correctly in the NIM environment
	${installp_env_variable}
	${chroot} ${INSTALLP} -pk ${installp_flags} ${d_flag} \
			${f_flag} ${filesets} </dev/null 1>/dev/null 2>${TMPDIR}/installp.sizes

	# if installp doesn't return anything, might as well continue
	[[ ! -s ${TMPDIR}/installp.sizes ]] && return 0

	# get the size info out of the file
	# NOTE that we have to jump through some hoops here because of the way in
	#		which installp prints this information
	root=$( echo ${location} | ${AWK} -F "/" '{print $2}' )

	# For /usr SPOTs, we need to remember the "/" size requirements to 
	# check against free space in /usr before doing sync_root on 
	# /usr/lpp/bos/inst_root.  Save this information in a file in /tmp.
	[[ ${location} = "/usr" ]] && print 0 > ${INSTROOT_CUST_SZ}

	${CAT} ${TMPDIR}/installp.sizes | \
	while read line
	do
		# ASSUMING installp preview info matches the following pattern:
		#		field 1 = "_SIZE_"
		#		field 2 = name of filesystem
		#		field 3 = free space required to install into the filesystem
		#		field 4 = currently available free space in the filesystem

		# does this line match our pattern?
		[[ "${line}" != _SIZE_?* ]] && continue
		set -- ${line}

		# ignore info if this is the "TOTAL"
		[[ "${2}" = TOTAL ]] && continue

		filesystem=${2}
		let "required=${3}"
		let "available=${4}"


		# add in a fudge factor of 10%
		let "required+=(required/10)+1"

		# Save "/" requirement for /usr SPOTs.
		[[ ${location} = "/usr" ]] \
				&& [[ ${filesystem} = "/" ]] \
				&& print ${required} > ${INSTROOT_CUST_SZ}

		# is there enough free space?
		if (( required > available ))
		then

			# auto expand?
			[[ "${auto_expand}" = "no" ]] &&
				error ${ERR_SPACE} ${filesystem} ${required} ${available}

			# calculate additional space needed
			let "additional=required-available"

			# try to expand the filesystem
			${CHFS} -a size=+${additional} ${filesystem} 2>${ERR} 1>&2 || \
				err_from_cmd ${CHFS}

		fi
	done

} # end of ck_required_space

#---------------------------- inst_spot         --------------------------------
#
# NAME: inst_spot 
#
# FUNCTION:
#		installs the specified SPOT with the specified software
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
#			spot_fs				= filesystem where SPOT resides
#			chroot				= command to execute in order to be in correct env
#											for installp
#			new_root				= pathname of new root dir
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function inst_spot {

	typeset d_flag=""
	typeset f_flag=""
	typeset i=""
	typeset auto_expand=""

	# which filesystem is SPOT in?
	spot_fs=$( ${DF} -Pk ${location} 2>/dev/null | ${AWK} 'NR==2{print $6}' )

	# setup the chroot environment
	# NOTE that we do this before the preparing the source device because local
	#		access to the source needs to be mounted over the chroot environment
	chroot=""
	setup_chroot_env

	# if a source was specified, then prepare it for use
	if [[ -n "${lpp_source}" ]]
	then

		prep_source

		# use the "-d" installp option
		d_flag="-d ${src_access}"

	fi

	# use a bundle file?
	if [[ -n "${installp_bundle}" ]]
	then
		
		prep_bundle

		# use the "-f" installp option
		f_flag="-f ${bundle_access}"
		filesets=""

	#fix_bundle specified? 
	elif [[ -n "${fix_bundle}" ]]
	then
	
		# setup local access (use installp's bundle prep routine)
		installp_bundle=${fix_bundle}
		prep_bundle

		# convert list of fix keywords to list of filesets
		prep_instfix_lst "spot" "bun" "${bundle_access}" "${d_flag}"

		filesets="-f ${fileset_list}"

	# fixes specified? 
	elif [[ -n "${fixes}" ]]
	then
		# "update_all" keyword specified?
	        if [ "${fixes}" = "update_all" ]
		then
			# get list of fileset updates needing installation.
			prep_updt_all_lst "spot" "${d_flag}"
                	if [ ! -s "${fileset_list}" ]
                	then
                        	# print sm_inst error for "nothing to update"
                        	${INUUMSG} 177 > ${ERR} 2>&1
                        	err_from_cmd "c_instspot"
                	fi

			filesets="-f ${fileset_list}"
		else
			# convert list of fix keywords to list of filesets
			prep_instfix_lst "spot" "fixes" "${fixes}" "${d_flag}"

			filesets="-f ${fileset_list}"
		fi

	elif [[ ${installp_flags} = *u* ]] || [[ ${installp_flags} = *r* ]]
	then

		# this is a deinstall operation: filesets must be specified
		[[ -z "${filesets}" ]] && error ${ERR_MISSING_ATTR} filesets

	elif [[ ${installp_flags} != *C* ]]
	then

		# this is an install operation: use default if filesets not specified
		filesets=${filesets:-${DEFAULT_SPOT_OPTIONS}}

	fi

	if [[ -n "${fixes}" ]] || [[ -n "${fix_bundle}" ]]
	then
        	# if not doing an update_all, add B flag for safety
        	# (so that base levels are not accidently instld via requisites)
        	if [[ "${fixes}" != "update_all" ]] && \
						[[ "${installp_flags}" != *B* ]]
        	then
               	 	installp_flags=${installp_flags}B
        	fi
	fi

	# installp cannot expand in a chroot'd env.  We need to do our own
 	# checks and expansion if necessary.  This is really only required 
	# for non-/usr SPOT's, since installp can expand when installing into
	# /usr SPOTs.  However, a sync_root takes place for the 
	# /usr/lpp/bos/inst_root directory for /usr SPOTs.  Therefore, we'll
	# call the space checking routine for either type of SPOT.  

	if [[ ${installp_flags} = *X* ]]
	then

		# intercept the installp "X" flag for non-/usr SPOTs since
		# it won't work correctly and may expand the wrong filesystem.
		if [[ ${location} != /usr ]]
		then
			installp_flags=$( echo ${installp_flags} | \
								${AWK} '{gsub(/X/,"");print}' )
		fi
		auto_expand="yes"

	fi

	# check the available space in the SPOT and auto expand if necessary
	ck_required_space "${installp_flags}" "${auto_expand}" "${d_flag}" \
			"${f_flag}" "${filesets}"

	#
	# Check to see if a previous installp flag file exists... 
	#
	[[ -f ${MK_NETBOOT} ]] && ${RM} ${MK_NETBOOT}

	# install into the SPOT
	# NOTE that this may just be maintenance operation where no source or
	#		bundle file is used (ie, this script can be invoked from m_instspot
	#		or m_maintspot)
	# NOTE also that we set the appropriate installp environment variable in 
	#		order to make it behave correctly in the NIM environment
	${installp_env_variable}

	# if doing an installp preview, capture the output in ERR. Cat the file
	# to stdout if there were no errors, otherwise do normal error handling.
	if [[ "${installp_flags}" = *p* ]]
	then
                ${chroot} ${INSTALLP} ${installp_flags} ${d_flag} ${f_flag} \
			${filesets} </dev/null >${ERR} 2>&1

        	if [ $? -eq 0 ]
        	then
                	cat ${ERR} 2>&1   # display the output
                	>${ERR}           # wipe out all traces of errors
        	else
                	err_from_cmd ${INSTALLP}
        	fi
	else
		${chroot} ${INSTALLP} -e ${INSTALLP_LOG} ${installp_flags} \
			${d_flag} ${f_flag} ${filesets} </dev/null 2>${ERR} 1>&2
	fi

	#
	# Save installp return code for later..
	#
	rc=$?

	#
	# If installp sez make boot images tell the caller via attr.
	#
	[[ -f ${MK_NETBOOT} ]] && echo "mk_netboot=yes"

	#
	# Its later.. Now check the return code from installp. 
	#
	[[ ${rc} != 0 ]] && err_from_cmd ${INSTALLP}


} # end of inst_spot

#---------------------------- c_instspot        --------------------------------
#
# NAME: c_instspot
#
# FUNCTION:
#		installs the specified software into a SPOT
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		this method does NOT create a SPOT - that work is done by c_mkspot
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#		0							= SPOT installed
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
typeset c=""

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
installp_flags=${installp_flags:-${SPOT_INSTALLP_FLAGS}}
installp_flags=$( ck_installp_flags "${installp_flags}" )

# if auto_expand specified, make sure there's an "X" in the flags
[[ "${auto_expand}" = "yes" ]] && [[ ${installp_flags} != *X* ]] && \
	installp_flags="${installp_flags}X"

# which installp environment variable should be used?
#		INUSERVERS = set for /usr SPOTs so that installp bypasses looking in the
#								/etc/niminfo file for the NIM_USR_SPOT variable
# 		INUCLIENTS = set for non-/usr SPOTs so that installp doesn't create a
#								boot image and to prevent any install scripts from
#								mucking with device configuration ont the server
if [[ ${location} = /usr ]]
then

	installp_env_variable="export INUSERVERS=yes"

	# make sure "b" not passed
	installp_flags=$( echo ${installp_flags} | \
							${AWK} '{gsub(/b/,"");print}' )

else

	installp_env_variable="export INUCLIENTS=yes"

fi

# check for errors
if [[ -n "${installp_bundle}" ]] && [[ -n "${filesets}" ]]
then

	error ${ERR_ATTR_CONFLICT} installp_bundle filesets

elif [[ -n "${fix_bundle}" ]] && [[ -n "${filesets}" ]]
then

	error ${ERR_ATTR_CONFLICT} fix_bundle filesets

elif [[ -n "${fixes}" ]] && [[ -n "${filesets}" ]]
then

	error ${ERR_ATTR_CONFLICT} fixes filesets

elif [[ -n "${fix_bundle}" ]] && [[ -n "${installp_bundle}" ]]
then

	error ${ERR_ATTR_CONFLICT} fix_bundle installp_bundle

elif [[ -n "${fix_bundle}" ]] && [[ -n "${fixes}" ]]
then

	error ${ERR_ATTR_CONFLICT} fix_bundle fixes

elif [[ ${installp_flags} = *a* ]] && [[ -z "${lpp_source}" ]]
then

	error ${ERR_MISSING_ATTR} lpp_source

fi

# perform the install
inst_spot

# all done
exit 0

