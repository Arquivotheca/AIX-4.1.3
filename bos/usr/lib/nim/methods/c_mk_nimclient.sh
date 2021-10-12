#!/bin/ksh
#@(#)03	1.26  src/bos/usr/lib/nim/methods/c_mk_nimclient.sh, cmdnim, bos411, 9439A411b  9/27/94  11:22:21 

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_mk_nimclient.sh
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
NIM_CLIENT_OPTIONS="\
	${NIM_CLIENT_PACKAGE} \
	bos.net.tcp.client \
	bos.net.nfs.client"
NIM_DEINSTALL_OPTIONS="\
	bos.sysmgt.nim.master \
	bos.sysmgt.nim.spot"
LSPS=/usr/sbin/lsps
CALC_PGSPACE=/usr/lib/assist/calc_pgspace
SET_PGSPACE=/usr/lib/assist/set_pgspace

#---------------------------- module globals    --------------------------------
REQUIRED_ATTRS="hostname ip snm"
OPTIONAL_ATTRS="ring_speed cable_type installp_flags filesets"
lpp_source=""
hostname=""
ip=""
snm=""
installp_flags=""
filesets=""
ring_speed=""
cable_type=""
license_msg="Network Installed"

#---------------------------- change_rc_net     --------------------------------
#
# NAME: change_rc_net
#
# FUNCTION:
#		changes the /etc/rc.net file to configure the client's primary network 
#			interface
#		this function only gets called when MKTCPIP fails
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
function change_rc_net {

	typeset pif_name=""
	typeset seqno=""
	typeset rs=""
	typeset ct=""
	typeset i=""
	typeset addr=""
	typeset hname=""
	typeset dest=""
	typeset routing_snm=""
	typeset gateway=""
	[[ -n "${ring_speed}" ]] && rs="-r ${ring_speed}"
	[[ -n "${cable_type}" ]] && ct="-t ${cable_type}"

	# make sure all the NIM_HOSTS are in /etc/hosts
	for i in ${NIM_HOSTS}
	do

		# separate IP address from hostname
		addr=${i%%:*}
		hname=${i##*:}

		# add this hostname if not already in /etc/hosts
		${EGREP} "^${addr}.*${hname}" ${HOSTS} >/dev/null 2>/dev/null || \
			echo "${addr}\t${hname}" >> ${HOSTS}

	done

	# determine logical interface name by first getting the name of the
	#		adapter we booted off of
	pif_name=$( ${BOOTINFO} -b )

	# now, separate logical device name from sequence number
	seqno=${pif_name##*[a-zA-Z]}

	# now, map the logical device name to a logical interface name
	case ${pif_name} in
		tr[0-9]|tok[0-9])	pif_name="tr${seqno}";;
		ent[0-9])	pif_name="en${seqno}";;
		*)				error ${ERR_SYS} "unknown adapter name \"${pif_name}\"";;
	esac

	# first, change the default ring_speed (if necessary)
	if [[ -n "${ring_speed}" ]]
	then

		if ${ODMGET} -qattribute=ring_speed PdAt | \
				${AWK} -v ring_speed=16 \
				'$1 == "deflt" {gsub(/".*"/,ring_speed)};{print}' \
				>${TMPDIR}/pdat 2>${ERR}
		then

			${ODMCHANGE} -qattribute=ring_speed -oPdAt ${TMPDIR}/pdat 2>${ERR}

		fi

		if [[ $? -eq 0 ]]
		then

			${SAVEBASE}

		else

			warning_from_cmd "changing default ring_speed"

		fi

	fi

	# change /etc/rc.net
	if ${AWK} -v hostname=${hostname} -v pif_name=${pif_name} -v ip=${ip} \
		-v snm=${snm} \
		'/^#\/bin\/hostname/ {print "/bin/hostname " hostname; next;}; \
		/^#\/usr\/sbin\/ifconfig/ \
			{	if ($2==pif_name) \
		{print "/usr/sbin/ifconfig " pif_name " inet " ip " netmask " snm; next;}\
			else if ($2=="lo0") {$1="/usr/sbin/ifconfig";print;next};}; \
		{ print;}' /etc/rc.net >${TMPDIR}/rc.net 2>${ERR}
	then

		${CAT} ${TMPDIR}/rc.net >/etc/rc.net

	else

		warning_from_cmd "updating /etc/rc.net"

	fi

} # end of change_rc_net

#---------------------------- config_tcpip      --------------------------------
#
# NAME: config_tcpip
#
# FUNCTION:
#		configures TCP/IP with the specified information
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
function config_tcpip {

	typeset pif_name=""
	typeset seqno=""
	typeset rs=""
	typeset i=""
	typeset addr=""
	typeset hname=""
	typeset dest=""
	typeset routing_snm=""
	typeset gateway=""
	[[ -n "${ring_speed}" ]] && rs="-r ${ring_speed}"
	[[ -n "${cable_type}" ]] && ct="-t ${cable_type}"

	# make sure all the NIM_HOSTS are in /etc/hosts
	for i in ${NIM_HOSTS}
	do

		# separate IP address from hostname
		addr=${i%%:*}
		hname=${i##*:}

		# add this hostname if not already in /etc/hosts
		${EGREP} "^${addr}.*${hname}" ${HOSTS} >/dev/null 2>/dev/null || \
			echo "${addr}\t${hname}" >> ${HOSTS}

	done

	# determine logical interface name by first getting the name of the
	#		adapter we booted off of
	pif_name=$( ${BOOTINFO} -b )

	# now, separate logical device name from sequence number
	seqno=${pif_name##*[a-zA-Z]}

	# now, map the logical device name to a logical interface name
	case ${pif_name} in
		tr[0-9]|tok[0-9])	pif_name="tr${seqno}";;
		fd*[0-9])			pif_name="fi${seqno}";;
		e[nt]*[0-9])		# determine whether to use "en" or "et"
								EN=$( /usr/bin/netstat -i | \
										${EGREP} "en${seqno}.*" )
								[[ -n "${EN}" ]] && 
									pif_name=en${seqno} ||
									pif_name=et${seqno}
								;;
		*)				error ${ERR_SYS} "unknown adapter name \"${pif_name}\"";;
	esac

	# if this is standalone machine...
	if [[ "${NIM_CONFIGURATION}" = standalone ]]
	then

		# flush the routes added during the network boot process so that we
		#		can add them back using chdev
		# NOTE that we don't want to do this for diskless or dataless machines
		#		because flushing the routes at this point might cause them to
		#		loose contact with their SPOT server (and hang forever)
		${ROUTE} -f 2>${ERR} || warning_from_cmd ${ROUTE}

	fi

	# configure the primary network interface using mktcpip
	if ${MKTCPIP} -h ${hostname} -a ${ip} -i ${pif_name} -m ${snm} \
		${rs} ${ct} 2>${ERR}
	then

		# everything ok - proceed
		:

	else

		# MKTCPIP failed
		warning_from_cmd ${MKTCPIP}

		# change the /etc/rc.net file directly
		change_rc_net

	fi

	# if this is a standalone machine...
	if [[ "${NIM_CONFIGURATION}" = standalone ]]
	then

		# add any routes specified in ROUTES
		# NOTE that we don't want to do this for diskless or dataless machines
		#		because the routes which are added during the network boot process
		#		are still present at this point, so chdev will fail
		for i in ${ROUTES}
		do
	
			# format of each stanza is:
			#     <dest net>:<dest net snm>:<gateway>
			# separate the fields
			dest=${i%%:*}
			gateway=${i##*:}
			routing_snm=${i#*:}
			routing_snm=${routing_snm%:*}
	
			# add the route
			${CHDEV} -l inet0 \
				-a route="net,${dest},-netmask,${routing_snm},${gateway}" \
				2>${ERR} || warning_from_cmd ${CHDEV}
	
		done

	fi

	# for mksysb installs, put mktcpip in /etc/firstboot
	[[ "$NIM_BOS_FORMAT" = "mksysb" ]] && \
		print "${MKTCPIP} -h  ${hostname} -a ${ip} -i ${pif_name} -m ${snm} \
${rs} ${ct}" >> /etc/firstboot

} # end of config_tpcip

#*---------------------------- c_mk_nimclient           ------------------------
#
# NAME: c_mk_nimclient
#
# FUNCTION:
#		performs NIM client customization in the BOS install environment
#		this customization includes:
#			- installing the NIM client package
#			- initializing a /.rhosts file
#			- adding an inittab entry for nimclient to change the Mstate
#
#		in the future, when TCPIP configuration changes, this function will also
#			propogate the RAM filesystem TCPIP configuration into the real env
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

# initialize local variables
typeset c=""
just_config=""
cur_pgsize=""
new_pgsize=""

# set parameters from command line
while getopts :a:qtv c
do
	case ${c} in

		a)    # validate the attr ass
				parse_attr_ass "${OPTARG}"

				# include the assignment for use in this environment
				eval ${variable}=\"${value}\"
		;;

		q)    # show attr info
				cmd_what
				exit 0
		;;

		t)		# just configure tcpip
				# The assumtion is that we're doing diskless/dataless
				# config.
				just_config=TRUE
		;;

		v)    # verbose mode (for debugging)
				set -x
				for i in $(typeset +f)
				do
					typeset -ft $i
				done
		;;

		\?)   # unknown option
				error ${ERR_BAD_OPT} ${OPTARG}
		;;

	esac
done

# check for missing attrs
ck_attrs

if [[ -z "${just_config}" ]]
then

	# set defaults
	installp_flags=$( ck_installp_flags "${installp_flags}" )
	filesets=${filesets:-${NIM_CLIENT_OPTIONS}}

	# set the NIM_BOOT_ENV variable so that the bos.sysmgt.nim.master
	#		unconfig scripts know not to do bad things
	export NIM_BOOT_ENV=yes

	# make sure that the bos.sysmgt.nim.master & spot pieces are not present
	# this is important because the source for the BOS image could be a /usr
	#		SPOT, which will at least contain the bos.sysmgt.nim.spot package
	${INSTALLP} -e ${INSTALLP_LOG} -ub ${NIM_DEINSTALL_OPTIONS} 2>${ERR} || \
		warning_from_cmd ${INSTALLP}

	# unset the NIM_BOOT_ENV variable so we don't mess anything else up
	unset NIM_BOOT_ENV

	# install the NIM client package
	# since this script should only be run during a BOS install, we know that
	#		simages is already mounted and accessable, so that's what our source is
	${INSTALLP} -e ${INSTALLP_LOG} -agXb \
			-d/SPOT/usr/sys/inst.images bos.net.tcp.client bos.net.nfs.client \
			bos.sysmgt.nim.client 2>${ERR} 1>&2 || warning_from_cmd ${INSTALLP}

	# add entry in inittab for nimclient
	# NOTE that we want the entry to come late enough for ensure that the
	#		network interface has been configured by the time it is executed
	${MKITAB} -i cron nimclient:2:wait:"${NIMCLIENT} -S ${STATE_RUNNING}" \
		2>${ERR} 1>&2 || warning_from_cmd ${MKITAB}

	# initialize the /.rhosts file
	echo "${NIM_MASTER_HOSTNAME} ${NIM_MASTER_UID:-root}" >>/.rhosts

	# set the default paging space if not already set via image.data file
	#		(ie, if current paging space is 32meg, then no image.data was used
	#		so call the install assistant commands to increase the paging space
	#		to the recommended amount)
	cur_pgsize=$( ${LSPS} -s | ${AWK} 'NR>1{gsub(/[^0-9]/,"",$1);print $1}' )
	if [[ "${cur_pgsize}" = 32 ]]
	then

		new_pgsize=$( ${CALC_PGSPACE} default_ps )
		[[ -n "${new_pgsize}" ]] && ${SET_PGSPACE} ${new_pgsize}

	fi

	#
	# Check to see if we have a license signature file.
	#  if so, sign it with network install.
	#
	echo "[[ -f ${LICENSE_SIG} ]] && echo ${license_msg} >> ${LICENSE_SIG}" >> /etc/firstboot
fi

# configure TCPIP
config_tcpip

# configure NFS
${MKNFS} -I 2>${ERR} 1>&2 || warning_from_cmd ${MKNFS}
#
# Set the date/time and time zone.
#
echo "${NIMCLIENT} -d" >> /etc/firstboot
# success
exit 0

