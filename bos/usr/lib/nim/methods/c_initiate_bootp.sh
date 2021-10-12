#!/usr/bin/ksh

# @(#)16 1.26  src/bos/usr/lib/nim/methods/c_initiate_bootp.sh, cmdnim, bos41J, 9518A_all  4/27/95  09:39:42

#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_initiate_bootp.sh
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
. ${NIM_METHODS}/c_sh_lib || exit 99 

#---------------------------- local defines     --------------------------------
#---------------------------- globals    --------------------------------
REQUIRED_ATTRS="adpt_name"
OPTIONAL_ATTRS="iplrom_emu mode ha gw sa ip sm no_client_boot"
BOOTLIST_OPTS="ha gw sa ip"

adpt_name=""
iplrom_emu=""
mode="normal"
ha=""
gw=""
sa=""
ip=""
bootlist_attrs=""

#-------------------------------------------------------------------------------

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

			# include the assignment for use in this environment
			eval ${variable}=\"${value}\" 2>${ERR} || \
				err_from_cmd eval
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

# initialize bootlist attr information
# this information is returned via the "bootlist_attrs" global variable
init_bootlist_attrs "${ip##*=}" "${sa##*=}" "${gw##*=}"

# was specific adapter name given?
if [[ ${adpt_name} != [!0-9]+[0-9]+ ]]
then

	# no specific adpter given - try all adapters of this type
	# generate the list from devices in CuDv
	list=$(	${ODMGET} -q"name like '${adpt_name}*'" CuDv | \
				${AWK} '$1=="name"{gsub(/"/,"");print $3}' )

else

	# just use specified adapter
	list=${adpt_name}

fi

# check all adapters specified in the list
adpt_list=""
found=""
for i in ${list}
do

	# add adpt to the list
	adpt_name=${i##*/}
	adpt_list="${adpt_list} ${adpt_name} ${bootlist_attrs} "

	# can this adapter be booted from?
	if ${BOOTINFO} -q ${adpt_name} >${TMPDIR}/bootinfo 2>${ERR} && \
		[[ "$(${CAT} ${TMPDIR}/bootinfo)" = 1 ]]
	then

		# found a bootable one
		found=TRUE

	fi

done

# were any adapters found we can network boot off of?
if [[ -z "${found}" ]]
then
	# IPL ROM emulation is needed... better have 
	# specified a device on cmd line.. 

	[[ -z "${iplrom_emu}" ]] && error ${ERR_MISSING_ATTR} iplrom_emu

	# Validate the device specified has the real ROS 
	# Emulation code on it. 

	${C_CKROS_EMU} -a iplrom_emu=${iplrom_emu} 2>${ERR} || \
		err_from_cmd ${C_CKROS_EMU}

	# Leave just the device information (rip off /dev)
	iplrom_emu=${iplrom_emu##*/}
fi	

#
# Build the command line 
#
cmdline="-m ${mode} "

if [[ -n ${iplrom_emu} ]] 
then 
	cmdline="${cmdline} ${iplrom_emu} ${adpt_list}"
else 
	cmdline="${cmdline} ${adpt_list}"

fi 

#
# Everything should be tickity boo Now lets set the bootlist. 
#
${BOOTLIST} ${cmdline} fd rmt scdisk 2>${ERR} || err_from_cmd ${BOOTLIST}


if [[ "${no_client_boot}" != "yes" ]]
then
	#
	# Wall the users... 
	#
	${C_ERRMSG} ${MSG_REBOOT_WARNING} ${C_ERRMSG_MSG} "" "" "" "" > /tmp/msg 2>&1
	cat /tmp/msg | ${WALL} 
	#
	sleep 4
	#
	cat /tmp/msg | ${WALL} 
	#
	${RM} /tmp/msg 

	${NIMCLIENT} -S shutdown 2>${ERR} || warning_from_cmd ${NIMCLIENT}

	${REBOOT} 2>${ERR} || err_from_cmd ${REBOOT}
fi

#
exit 0
