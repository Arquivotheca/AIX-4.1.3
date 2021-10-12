#!/bin/ksh
#@(#)37	1.20.1.2  src/bos/usr/lib/nim/methods/c_mkroot.sh, cmdnim, bos411, 9439A411b  9/27/94  11:23:04 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mkroot.sh
#		
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# include common NIM shell defines/functions
NIMPATH=${0%/*}
NIMPATH=${NIMPATH%/*}
[[ ${NIMPATH} = ${0} ]] && NIMPATH=/usr/lpp/bos.sysmgt/nim
NIM_METHODS="${NIMPATH}/methods"
. ${NIM_METHODS}/c_sh_lib

#---------------------------- local defines     --------------------------------
FS_STANZAS="/ root:/usr spot ro:/home home:/tmp tmp:/var/adm/ras log:/var var"
STRIP_FS="/: /home: /usr: /var: /tmp:"
INST_ROOT="lpp/bos/inst_root"
ATTR_LOCATION=location
ATTR_PERMS=perms
ATTR_NFSPARAMS=nfsparams
STATE_RUNNING="running"
NIMCLIENT_ITAB="nimclient:2:wait:${NIMCLIENT} -S ${STATE_RUNNING} \
>/dev/console 2>&1 # NIM Mstate"
CUTFS=${NIM_AWK}/cutfs.awk

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="name hostname boot_info adpt_name server root spot dump \
master type"
OPTIONAL_ATTRS="paging ring_speed cable_type home tmp log root_initialized"
name=""
hostname=""
boot_info=""
adpt_name=""
server=""
root=""
spot=""
dump=""
master=""
type=""
paging=""
dump=""
ring_speed=""
cable_type=""
home=""
tmp=""
log=""
license_msg="diskless/dataless"

#---------------------------- undo              --------------------------------
#
# NAME: undo
#
# FUNCTION:
#		removes a newly created root directory
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
#			1						= optional; if present, name of failing function
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function undo {

	typeset func=${1}

	# cache current error
	[[ -n "${func}" ]] && ${CP} ${ERR} ${TMPDIR}/errstr 2>/dev/null

	# remove all files in the root
	${RM} -r ${root}/* 2>${ERR} || warning_from_cmd "${RM} -r ${root}"

	if [[ -n "${func}" ]] 
	then

		${CP} ${TMPDIR}/errstr ${ERR}
		err_from_cmd ${func}

	else

		error ${ERR_SYS} "$( ${CAT} ${ERR} )"

	fi

} # end of undo

#---------------------------- strip_stanzas     --------------------------------
#
# NAME: strip_stanzas
#
# FUNCTION:
#		removes the specified stanzas from the specified file
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		ASSUMES that stanzas are structured as follows:
#			1) beginning of stanza can be matched with /^.*:$/
#			2)	1st blank line after stanza beginning terminates the stanza
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= file to operate on
#			2						= one or more stanza beginnings (separated by " ")
#											or the keyword "all"
#		global:
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function strip_stanzas {

	typeset file=${1}
	typeset stanzas=${2}

	[[ ! -r "${file}" ]] && return 1

	if ${AWK} -v s="${stanzas}" '\
			BEGIN{num=split(s,sa);};\
			function found(a){for(i in sa){if(sa[i]==a)return 1;};return 0;};\
			/^.+:$/{in_stanza=((s=="all") || (found($1)>0));};\
			/^[ 	]*$/{in_stanza=0};\
			in_stanza==0 {print;}' ${file} >${TMPDIR}/stripd 2>${ERR}
	then

		${CAT} ${TMPDIR}/stripd >${file} 2>${ERR} && return 0

	fi

	return 1

} # end of strip_stanzas

#*---------------------------- pop_root        ------------------------------
#
# NAME: pop_root   
#
# FUNCTION:
#		creates a new root directory
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
#			root
#
# RETURNS: (int)
#		0							= no errors
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function pop_root {

	typeset inst_root=""
	typeset i=""

	# root dir must already exist (if not, there was an error during allocation)
	[[ ! -d ${root} ]] && error ${ERR_FILE_ACCESS} ${root}

	# make sure root has correct permissions
	${CHMOD} 755 ${root} 2>${ERR} || undo "${CHMOD} 755 ${root}"

	# SPOT local or remote?
	if [[ -n "${spot_host}" ]]
	then

		# mount the SPOT
		nim_mount "${spot_host}:${spot}"
		spot=${access_pnt}

	fi

	inst_root="${spot}/${INST_ROOT}"


	# go to the INST_ROOT
	cd ${inst_root} 2>${ERR} || err_from_cmd "cd ${inst_root}"

	# root directory already populated?
	if [[ -n "$(${LS} ${root} 2>/dev/null)" ]]
	then

		# generate list of files which do exist in INST_ROOT but don't exist
		#		in root yet
		>${TMPDIR}/new_files
		for i in $( ${FIND} . -print )
		do
			[[ ! -r ${root}/${i} ]] && print ${i} >>${TMPDIR}/new_files
		done

		# copy the new files
		if [[ -s ${TMPDIR}/new_files ]]
		then

			${CAT} ${TMPDIR}/new_files | \
				${CPIO} -pd ${root} 2>${ERR} || undo ${CPIO}

		fi

	else

		# copy all the INST_ROOT files
		${FIND} . -print | ${CPIO} -pd ${root} 2>${ERR} || undo ${CPIO}

	fi

	return 0

} # end of pop_root

#*---------------------------- add_host           ------------------------------
#
# NAME: add_host 
#
# FUNCTION:
#		adds the specified host to the client's /etc/hosts file
#		this function prevents duplicate entries from being added
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		this function ASSUMES that the current working directory is the client's
#			root directory
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= hostname to add
#		global:
#			ip						= IP address of specified hostname
#
# RETURNS: (int)
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function add_host {

	# local variables
	typeset hostname=${1}
	typeset hosts="${root}${HOSTS}"
	typeset name=""
	typeset alias=""
	typeset yes=""
	typeset ip=""

	# is host already in the file?
	[[ -s ${hosts} ]] && \
		yes=$(${AWK} -v host=${hostname} '$2==host || $3==host {print}' ${hosts})
	if [[ -z "${yes}" ]]
	then
		# not there already
		# resolve it to an IP address
		${HOSTCMD} ${hostname} >${TMPDIR}/host 2>${ERR} || return 1

		# is the resolved name different from $hostname?
		name=$(${AWK} '{print $1}' ${TMPDIR}/host)
		if [[ -n "${name}" ]] && [[ "${name}" != "${hostname}" ]]
		then
			# treat $hostname as an alias
			alias=${hostname}
			hostname=${name}
		fi

		# get IP address
		ip=$( ${AWK} '{gsub(/,/,"");print $3}' ${TMPDIR}/host 2>${ERR} )

		# add an entry
		if [[ -n "${ip}" ]] && [[ -n "${hostname}" ]]
		then

			print "${ip}\t${hostname}\t${alias}" >> ${hosts}

		else

			print "unable to resolve \"${hostname}\" to an IP address" >${ERR}
			return 1

		fi
	fi

	# success
	return 0

} # add_host

#*---------------------------- add_fs_stanza      ------------------------------
#
# NAME: add_fs_stanza
#
# FUNCTION:
#		adds the specified filesystems stanza
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= local mount point
#			2						= variable prefix for remote mount info
#			3						= filesystems options
#		global:
#			root
#
# RETURNS: (int)
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function add_fs_stanza {

	# local variables
	typeset mntpnt=${1}
	typeset prefix=${2}
	eval typeset dir=\$${prefix}
	eval typeset host=\$${prefix}"_host"
	host=${host:-${server}}
	typeset options=${3:-"rw"}
	typeset fs="${root}${FILESYSTEMS}"

	# remove the filesystems stanza for this mount point if it exists
	if ${AWK} -v mntpnt=${mntpnt} -f ${CUTFS} ${fs} >${TMPDIR}/fs 2>${ERR}
	then
		${CAT} ${TMPDIR}/fs > ${fs}
	else
		return 1
	fi

	# make sure that EVERY nodename has an entry in the client's /etc/hosts
	add_host $host || return 1

	# add the stanza
	print "\n${mntpnt}:" >> ${fs}
	print "\tnodename\t=\t${host}" >> ${fs}
	print "\tdev\t\t=\t${dir}" >> ${fs}
	print "\tcheck\t\t=\tfalse" >> ${fs}
	print "\tvfs\t\t=\tnfs" >> ${fs}
	print "\ttype\t\t=\tdd_boot" >> ${fs}

	# mount -t used during boot, so set that type here
	if [[ "${mntpnt}" = "/" ]] || [[ "${mntpnt}" = "/usr" ]]
	then
		print "\toptions\t\t=\t${options},hard,intr" >> ${fs}
		print "\tmount\t\t=\tautomatic" >> ${fs}
	else
		print "\toptions\t\t=\t${options},bg" >> ${fs}
		print "\tmount\t\t=\ttrue" >> ${fs}
	fi

	# success
	return 0

} # add_fs_stanza

#*---------------------------- init_filesystems   ------------------------------
#
# NAME: init_filesystems
#
# FUNCTION:
#		initializes the client's /etc/filesystems file
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		this function ASSUMES that the current working directory is the client's
#			root directory
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= 
#			2						=  
#			3						= 
#		global:
#
# RETURNS: (int)
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function init_filesystems {

	typeset fs="${root}${FILESYSTEMS}"
	typeset i=""

	# add the filesystems stanzas
	IFS=':'
	set ${FS_STANZAS}
	IFS=${OLD_IFS}
	for i
	do
		[[ -z "${i}" ]] && continue

		# separate mount point from variable name from options
		set $i

		# strip the current value for this mount point out of the filesystems file
		strip_stanzas ${root}${FILESYSTEMS} "${1}:"

		# value specified for this stanza?
		if eval [[ -z \"\$${2}\" ]]
		then
			continue
		fi

		add_fs_stanza "${1}" "${2}" "${3}" || err_from_cmd add_fs_stanza
	done

	print >> ${fs}

	# success
	return 0

} # init_filesystems

#*---------------------------- init_swapspaces    ------------------------------
#
# NAME: init_swapspaces
#
# FUNCTION:
#		initializes the client's /etc/swapspaces file
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
#			root
#			paging
#			paging_host
#
# RETURNS: (int)
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function init_swapspaces {

	# strip the file if the root hasn't been initialized before
	[[ -z "${root_initialized}" ]] && strip_stanzas ${root}${SWAPSPACES} "all"

	typeset ss=${root}${SWAPSPACES}
	paging_host=${paging_host:-${server}}

	# strip out current SWAPNFS entry
	strip_stanzas ${root}${SWAPSPACES} "${SWAPNFS}0:"

	# if "paging" isn't specified, then this is a dataless machine and the
	#		swapspaces file will be initialized during the boot process
	# so, skip the rest of the processing which follows
	[[ -z "${paging}" ]] && return 0

	# ASSUMING: remote paging & sequence number is zero
	print "\n${SWAPNFS}0:" >> ${ss}
	print "\tdev\t=\t/dev/${SWAPNFS}0" >> ${ss}
	print "\tremdev\t=\t$paging_host:$paging/${SWAPNFS}0" >> ${ss}

	# add another hosts entry if necessary
	add_host "${paging_host}" || return 1

	print >> ${ss}

	# success
	return 0

} # end of init_swapspaces

#*---------------------------- init_firstboot     ------------------------------
#
# NAME: init_firstboot
#
# FUNCTION:
#		initializes the client's /etc/firstboot file
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		this function ASSUMES that the current working directory is the client's
#			root directory
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#			boot_info			= information used to set the boot device list
#
# RETURNS: (int)
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function init_firstboot {

	typeset fb="${root}${FIRSTBOOT}"
	typeset dump_host=${dump_host:-${server}}
	typeset i=""
	typeset client=""
	typeset gateway=""
	typeset snm=""
	typeset bserver=""
	typeset rs=${ring_speed:+"-a ring_speed=${ring_speed}"}
	typeset ct=${cable_type:+"-a cable_type=${cable_type}"}

	# parse the boot_info
	set -- ${boot_info}
	for i
	do

		case ${i} in

			-a*ip=*) # client IP address
				client=${i##*=}
			;;

			-a*gw=*) # IP address of gateway to BOOTP server
				gateway=${i##*=}
			;;

			-a*sm=*) # subnetmask
				snm=${i##*=}
			;;

			-a*sa=*) # BOOTP server IP address
				bserver=${i##*=}
			;;

		esac
	done

	# the bootlist command has specific rules about how to set the boot dev list
	# 		and init_bootlist_attrs knows how to format it correctly
	# call it now - info returned via the global variable "bootlist_attrs"
	init_bootlist_attrs "${client}" "${bserver}" "${gateway}"

	# if adapter seqno not given, ASSUME zero
	[[ ${adpt_name} != ?*[0-9] ]] && adpt_name=${adpt_name}0

	# /etc/firstboot is a shell script
	print "#!/bin/ksh" > ${fb}

	# set the bootlist
	print "${BOOTLIST} -m normal ${adpt_name} ${bootlist_attrs}" >> ${fb}

	# invoke C_MK_NIMCLIENT to configure the network interface using MKTCPIP
	print "${C_MK_NIMCLIENT} -t -a hostname=${hostname} -a ip=${client} \
-a snm=${snm} ${rs} ${ct}" >> ${fb}

	# if this is a diskless machine...
	if [[ ${type} = diskless ]]
	then

		# set the dump device
		print "sysdumpdev -P -p ${dump_host}:${dump}/dump" >> ${fb}
		print "sysdumpdev -P -s /dev/sysdumpnull" >> ${fb}

	fi
	#
	# Check to see if we have a license signature file.
	#  if so, sign it with network install.
	#
	echo "[[ -f ${LICENSE_SIG} ]] && echo ${license_msg} >> ${LICENSE_SIG}" >> ${fb}

	# add the dump host
	add_host ${dump_host} || return 1

	# set execute permissions
	${CHMOD} 660 ${fb} 2>${ERR} || return 1

	# success
	return 0

} # end of init_firstboot

#*---------------------------- inittab            ------------------------------
#
# NAME: inittab       
#
# FUNCTION:
#		adds an entry in the clien's /etc/inittab to perform a root sync whenever
#			it boots
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
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/
function inittab {

	typeset itab=${root}${INITTAB}
	typeset dump_host=${dump_host:-${server}}
	typeset dumpcheck_itab=""

	# inittab better be there
	if [[ ! -s ${itab} ]]
	then

		print "/etc/inittab missing: expecting it at ${itab}" >${ERR}
		return 1

	fi

	# nimclient entry already there?
	if ${EGREP} "^nimclient:" ${itab} >/dev/null 2>&1
	then
		:
	else

		# add an entry right after fbcheck
		${AWK} -v new_entry="${NIMCLIENT_ITAB}" \
			'/^fbcheck:/ {add=1};{print; if (add){print new_entry;add=0}}' \
			${itab} >${TMPDIR}/inittab 2>${ERR} || return 1

		${CAT} ${TMPDIR}/inittab >${itab} 2>${ERR} || return 1

	fi

	# success
	return 0

} # end of inittab

#---------------------------- init_rhosts       --------------------------------
#
# NAME: init_rhosts
#
# FUNCTION:
#		initializes the client's /.rhosts file by putting the master's hostname
#			and UID there
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
function init_rhosts {

	typeset rh=${root}${RHOSTS}

	# skip if entry already exists
	[[ -s ${rh} ]] && \
		${EGREP} "^${master}[ 	]" ${rh} >/dev/null 2>&1 && \
		return 0

	# add new entry
	print "${master}	root" >>${rh} 2>${ERR} || return 1

	# make sure hostname is resolved
	add_host ${master} || return 1

	return 0

} # end of init_rhosts

#*---------------------------- c_mkroot           ------------------------------
#
# NAME: c_mkroot
#
# FUNCTION:
#		creates a root directory for a diskless or dataless client
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
#		0							= no errors
#		>0							= failure
#
# OUTPUT:
#-----------------------------------------------------------------------------*/

# signal processing
trap cleanup 0
trap err_signal 1 2 11 15

# NIM initialization
nim_init

# set parameters from command line
while getopts :a:qv c
do
	case ${c} in

		a)		# validate the attr ass
				parse_attr_ass "${OPTARG}"

				# if host:dir format given, separate them
				if [[ ${value} = ?*:/* ]]
				then
					eval ${variable}=\"${value#*:}\"
					eval ${variable}"_host"=${value%:*}
				else
					eval ${variable}=\"${value}\"
				fi
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

# populate the root directory
pop_root

# now, sync the root with the SPOT before we edit any files in the root
${C_SYNC_ROOT} -s -a location=${spot_host:+${spot_host}:}${spot} \
	-a installp_flags=-aOr -a filesets=all ${root} 2>${ERR} || \
	warning_from_cmd ${C_SYNC_ROOT}

# prepare for interrupts
undo_on_interrupt=undo

# add the client's hostname to its /etc/hosts file
add_host ${hostname} || undo add_host

# initialize the client's /etc/filesystems
init_filesystems || undo init_filesystems

# initialize the client's /etc/swapspaces
init_swapspaces || undo init_swapspaces

# initialize the client's /etc/firstboot
init_firstboot || undo init_firstboot

# add entry in inittab for nimclient
inittab || undo inittab

# initialize client's /etc/rhosts
init_rhosts || undo init_rhosts

undo_on_interrupt=""

# success
exit 0

