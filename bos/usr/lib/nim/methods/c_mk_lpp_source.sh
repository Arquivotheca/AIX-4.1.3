#!/bin/ksh
# @(#)68	1.23.1.2  src/bos/usr/lib/nim/methods/c_mk_lpp_source.sh, cmdnim, bos41J, 9520A_all 5/15/95 15:11:04
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mk_lpp_source.sh
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
OPTIONAL_ATTRS="source packages"
location=""
source=""
packages=""
typeset -i max_options=0
missing=""
src_access=""

#---------------------------- prep_source       --------------------------------
#
# NAME: prep_source
#
# FUNCTION:
#		prepares the source of the LPP images for use
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
#			source				= source of LPP images
#		global:
#			access_pnt			= local access point
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_source {

	typeset source=${1}

	# what kind of source?
	case ${source} in

		/dev/cd+([0-9]))
			# source is CDROM; mount it
			nim_mount ${source}
			src_access=${access_pnt}${OFFSET_FOR_CDROM}
		;;

		/dev/rmt+([0-9]))
			# source is tape; nothing to do because installp will handle block size
			src_access=${source}
		;;

		*)
			# assume source is a directory
			# make sure we've got local access
			nim_mount ${source}
			src_access=${access_pnt}
		;;

	esac

} # end of prep_source

#---------------------------- check_for_bos     --------------------------------
#
# NAME: check_for_bos
#
# FUNCTION:
#		checks for a BOS runtime image and renames the file if necessary
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
#			location				= location of lpp_source
#		global:
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function check_for_bos {

	typeset location=${1}
	typeset i=""

	# is "location" a local directory?
	if ${C_STAT} -a location=${location} -a vfstype=3 2>/dev/null
	then

		# is there a BOS runtime which which was created by BFFCREATE? (it
		#		gives the file a weird name)
		for i in $( ${LS} ${source}/bos.usr* 2>/dev/null )
		do

			# rename the file
			${MV} ${i} ${source}/bos 2>${ERR} || err_from_cmd ${MV}

			break

		done

		# remove any other of BOS runtime files left around (we only want
		#		one "bos" file in an lpp_source)
		for i in $( ${LS} ${source}/bos.usr* 2>/dev/null )
		do

			${RM} ${i} 2>${ERR} || warning_from_cmd ${RM}

		done

		# now, update the TOC so the new name is used from now on
		${RM} ${source}/.toc 2>/dev/null 1>&2
		${INUTOC} ${source} 2>${ERR} || err_from_cmd ${INUTOC}

	fi

	# is "location" a local directory, remote directory, or CDROM?
	if ${C_STAT} -a location=${location} -a vfstype="2 3 5" 2>/dev/null
	then

      # is there a BOS runtime image present?
      if [[ -f ${location}/bos ]]
      then

         # add it to the options list
         print "bos bos.rte" >>${TMPDIR}/options.list

      fi

	fi

} # end of check_for_bos

#---------------------------- list_options      --------------------------------
#
# NAME: list_options
#
# FUNCTION:
#		generates a list of LPPs which reside on the specified device
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
#			1						= source of LPP images
#			2						= non-NULL if list is just going to be displayed
#		global:
#
# RETURNS: (int)
#		0							= list of LPPs generated
#		1							= do options exist on <device> or unable to gen list
#
# OUTPUT:
#-------------------------------------------------------------------------------
function list_options {

	typeset source=${1}
	typeset list_only=${2}

	# generate a list of product names
	# NOTE that INUSERVERS is an environment variable which effects the behavior
	#		of installp.  When set, it causes installp to bypass looking in the
	#		/etc/niminfo file for the NIM_USR_SPOT variable, which is used to
	#		prevent users from executing installp from the command line on machines
	#		whose /usr filesystem has been converted into a SPOT
	export INUSERVERS=yes
	${INSTALLP} -d ${source} -L </dev/null >${TMPDIR}/installp.L 2>${ERR} || \
		err_from_cmd ${INSTALLP}

	# return if the list is empty
	[[ ! -s ${TMPDIR}/installp.L ]] && return 1

	# filter out extraneous info
 	${AWK} 'BEGIN { FS=":" } /:/ {print $1 " " $2}' ${TMPDIR}/installp.L \
			>${TMPDIR}/options.list 2>${ERR} || err_from_cmd ${AWK}

	[[ -z "${list_only}" ]] && return 0

	# check for the BOS runtime image
	check_for_bos ${source}

	# display the list of available options
	${AWK} 'END{ for (i=1; i<=p; i++) \
						{	print packs[i,0]; \
							for (j=2; j<=packs[i,1]; j++) \
								print "\t" packs[i,j]; \
						}} \
				{ for( i=1; i<=p; i++) \
						if ($1 == packs[i,0]) break; \
					if (i > p){ packs[i,0] = $1; packs[i,1] = 1; p=i;} \
					packs[i,1] = packs[i,1] + 1; \
					packs[i, packs[i,1] ] = $2;}' ${TMPDIR}/options.list

	exit 0

} # end of list_options

#---------------------------- check_options     ------------------------------
#
# NAME: check_options 
#
# FUNCTION:
#		checks to see if there are any requested options which are not present
#			on the specified source device
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
#			1						= source of option images
#			2						= list of options to check
#		global:
#			max_options			= max number of options
#			option_names		= array of option names
#			ere					= array of extended regular expressions for options
#			missing				= list of missing option names
#
# RETURNS: (int)
#		0							= success - all requested options are present
#		1							= failure - one or more options missing
#
# OUTPUT:
#-------------------------------------------------------------------------------
function check_options {

	typeset source=${1}
	typeset options="${2}"
	typeset i=
	typeset name=
	typeset -i j=1
	typeset put_bos_back_in=""

	# reset the list of "missing" options
	missing=""

	if [[ ${source} != /dev/* ]]
	then

		# check for the BOS runtime image
		check_for_bos ${source}

	elif	[[ ${options} = **([ 	])bos+([ 	])* ]]
	then

		# take "bos" out of the list
		options=$(print ${options} | ${AWK} '{gsub(/[ 	]*bos[ 	]/," ");print}')
		put_bos_back_in=TRUE

	elif [[ ${options} = **([ 	])bos ]]
	then

		# take "bos" out of the list
		options=${options%bos}
		put_bos_back_in=TRUE

	fi

	# first, we transform the option list into extended regular expressions
	let max_options=0
	for i in ${options}
	do
		let max_options=max_options+1

		option_names[max_options]=${i}

		# does package name have any keywords in it?
		if [[ ${i} = ?*.any ]]
		then

			# protect any periods
			name=$(  echo ${i} | ${AWK} '{gsub(/\./,"\\.");print}' )

			# "<name>.any" means match everything up to ".any", but no deeper (ie,
			#		don't match on "<name>.something.somethingelse"
			ere[max_options]=$(  echo ${name} | \
										${AWK} '{gsub(/\\\.any$/,"\\.[^. ]+ ");print}' )

		elif [[ ${i} = ?*.all.?* ]]
		then

			# protect any periods
			name=$(  echo ${i} | ${AWK} '{gsub(/\./,"\\.");print}' )

			# ".all." indicates that the wildcard exists in the middle of a name
			ere[max_options]=$(  echo ${name} | \
										${AWK} '{gsub(/\\\.all\\\./,"\\.[^. ]+\\.");\
													print}' )

		else

			if [[ ${i} = ?*.all!(.*) ]]
			then

				# remove the word "all"
				option_names[max_options]=${i%.all}

				# replace ".all" with the appropriate ere
				name=${i}
				name=$(  echo ${name} | ${AWK} '{gsub(/\.all$/,"\\..+");print}' )

				# protect any periods
				tmp=$(   echo ${option_names[max_options]} | \
							${AWK} '{gsub(/\./,"\\.");print}' )

				# "<name>.all" means match on:
				#		1) just <name>
				#		2) <name>.anything
				ere[max_options]="(^${tmp} )|(^${tmp}\.[^. ]+ )|( ${name}$)"

			else
				
				# no keywords used: match on the name explicitly
				ere[max_options]="(^${option_names[max_options]} )"


				# protect any periods
				ere[max_options]=$(  echo ${ere[max_options]} | \
											${AWK} '{gsub(/\./,"\\.");print}' )
			fi
		fi
	done

	# now, look for missing options & generate a list of package names
	>${TMPDIR}/packages
	while (( j<=max_options ))
	do

		package=$(${EGREP} "${ere[j]}" ${TMPDIR}/options.list 2>/dev/null | \
							${AWK} 'END{for (i in names) print names[i]}; \
										{if (!($1 in names)) names[$1]=$1}' 2>/dev/null)

		if [[ -n "${package}" ]]
		then

			for i in ${package}
			do

				# add package name to the list
				${EGREP} "^${i}$" ${TMPDIR}/packages >/dev/null 2>&1 || \
					print ${i} >> ${TMPDIR}/packages

			done

		else

			missing="${missing}\t\t${option_names[j]}\n"

		fi

		let j=j+1
	done

	[[ -n "${put_bos_back_in}" ]] && print "bos" >>${TMPDIR}/packages

	[[ -n "${missing}" ]] && return 1

	return 0

} # end of check_options

#---------------------------- undo              --------------------------------
#
# NAME: undo
#
# FUNCTION:
#		backs out changes made by mk_lpp_source
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= source of LPP options
#			2						= non-NULL if $1 is to be removed
#			3->					= error message stuff
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function undo {

	typeset dir=${1}
	typeset rmdir=${2}
	shift 2

	[[ -n "${rmdir}" ]] && [[ "${dir}" != /usr ]] && ${RM} -r ${dir} 2>/dev/null

	[[ $# = 1 ]] && err_from_cmd $1 || error $@

} # end of undo

#---------------------------- mk_lpp_source     --------------------------------
#
# NAME: mk_lpp_source
#
# FUNCTION:
#		creates an lpp_source type resource using the bffcreate command
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
#			1						= location to put images in
#			2						= options to put in <location>
#		global:
#			missing				= list of missing option names
#			undo_on_interrupt
#			src_access			= local access point for source of LPP images
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function mk_lpp_source {

	typeset location=${1}
	typeset options="${2:-${SIMAGES_OPTIONS}}"
	typeset rmdir=

	# check for options which do not reside on the source device
	check_options ${src_access} "${options}" || \
		warning ${ERR_MISSING_OPTIONS} ${source} "$( echo ${missing} )"

	# create <location> if it doesn't already exist
	if [[ ! -d ${location} ]]
	then

		${C_MKDIR} -a location=${location} 2>${ERR} || err_from_cmd ${C_MKDIR}
		rmdir=TRUE
		undo_on_interrupt="undo ${location} TRUE"

	fi

	# use the bffcreate command to retrieve the images from the source device
	# NOTE that bffcreate only takes package names, not option names, and that
	# 		these names get generated by the check_options function
	if ${BFFCREATE} -qXt ${location} -d ${src_access} \
			$(${CAT} ${TMPDIR}/packages) 2>${ERR} 1>&2
	then

		:

	else

		undo ${location} "${rmdir}" ${BFFCREATE}

	fi

	undo_on_interrupt=""

	# generate a list of the options which just got retrieved so that we
	#		can check for missing options when we return to main
	# make sure that all our options got created by bffcreate
	list_options ${location}

} # end of mk_lpp_source
	
#---------------------------- c_mk_lpp_source       ----------------------------
#
# NAME: c_mk_lpp_source
#
# FUNCTION:
#		creates an lpp_source type NIM resource
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
#		0	= success
#		1	= failure
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
typeset check_only=""
typeset list_only=""

# set parameters from command line
while getopts :a:clqv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
				;;

		c)		# check for missing options
				check_only=yes
				;;

		l)		# list option info only
				list_only=yes
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

# check for errors
if [[ -n "${list_only}" ]] && [[ -n "${check_only}" ]]
then

	error ${ERR_SYNTAX} "c_mk_lpp_source: -a location=<> [-a attr=val] [-c]|[-l]"

fi
if [[ -z "${source}" ]]
then

	# not going to create an lpp_source, so location must already exist and be
	#		a local directory
	# first, check to see if it is a CDROM filesystem
	if ${C_STAT} -a location=${location} -a vfstype=5 >/dev/null 2>&1
	then

		# CDROM filesystem - return this information
		# we need to know when an lpp_source is a CDROM so that we can prevent
		#		it from being used in define operations for non-/usr SPOTs (which
		#		would require mounting the CDROM inside of the SPOT's inst_root)
		[[ -z "${list_only}" ]] && print "cdrom=yes"

	elif ${C_STAT} -a location=${location} 2>${ERR}
	then

		# remove the current TOC (if present)
		if [[ -f ${location}/.toc ]]
		then

			${RM} ${location}/.toc 2>${ERR} || err_from_cmd ${RM}

		fi

	else

		err_from_cmd ${C_STAT}

	fi

elif [[ -n "${check_only}" ]]
then

	error ${ERR_CONTEXT} "-c"

else

	# prepare the source device for use
	prep_source ${source}

fi

# generate list of options which are present on the source
# also, display the list & exit if list_only was specified
list_options ${src_access:-${location}} "${list_only}"

# if we get here, then the ${TMPDIR}/options.list file (generated in the
#		previous step) must have something in it
[[ ! -s ${TMPDIR}/options.list ]] && \
	error ${ERR_SOURCE} "${source:-${location}}"

# create the lpp_source?
if [[ -n "${source}" ]]
then

	mk_lpp_source ${location} "${packages:-${SIMAGES_OPTIONS}}"

fi

# stat the lpp_source directory: it must be a local directory
${C_STAT} -a location=${location} -a vfstype="3 5" 2>${ERR} || \
	err_from_cmd ${C_STAT}

# check the options which reside in the lpp_source

# first, we'll check to see if the lpp_source provides SIMAGES (ie, the
#		"s"upport "images" which NIM needs in order to support install operations
# if so, display an attribute assignment so that it gets added to the
#		object definition for this lpp_source
check_options ${location} "${REQUIRED_SIMAGES}" && \
	print "simages=yes"

# were specific options specified on the command line?
if [[ -n "${packages}" ]]
then

	# check for missing options
	check_options ${location} "${packages}"

fi

# any missing options?
[[ -n "${missing}" ]] && \
	warning ${ERR_MISSING_OPTIONS} ${location} "$( echo ${missing} )"

exit 0

