#!/bin/ksh
# @(#)72        1.6  src/bos/usr/lib/nim/methods/c_nnc_setup.sh, cmdnim, bos41J, 9523C_all  6/9/95  20:56:22
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

NIMPATH=${0%/*}
NIMPATH=${NIMPATH%/*}
[[ ${NIMPATH} = ${0} ]] && NIMPATH=/usr/lpp/bos.sysmgt/nim
NIM_METHODS="${NIMPATH}/methods"

# Check to see if this is not a standalone system - if it's not, then we
# will not be able to succeed unless this is a Version 4 system with
# nim client installed

touch /usr >/dev/null 2>&1
if [[ $? -ne 0 ]] 
then
    standalone=0
else
    standalone=1
fi

. ${NIM_METHODS}/c_sh_lib || exit 99


#---------------------------- isBootable 
#
# NAME: isBootable
#
# FUNCTION: tests if specified adapter can be booted from
#
# DATA STRUCTURES:
#     parameters:
#        1	= The adapter 
#        
# RETURNS: (int)
#     0        	= success
#     1        	= can not boot over this device
#
#-------------------------------------------------------------------------------

function isBootable {

    isbootable=`${BOOTINFO} -q ${1}`

    if [[ -n "${isbootable}" ]] && [[ "${isbootable}" = "1" ]]
    then
	return 0
    fi
    # not a good one
    return 1
}

#---------------------------- expand 
#
# NAME: expand
#
# FUNCTION:  Add one partition to an existing filesystem
#
# DATA STRUCTURES:
#     parameters:
#        1	= The filesystem 
#
#-------------------------------------------------------------------------------
function expand {
    ${CHFS} -a size=+1 $1 >/dev/null
}


#---------------------------- copybootimage 
#
# NAME: copybootimage
#
# FUNCTION:  Copy the bootimage from /dev/ipldevice to the save directory
#
# DATA STRUCTURES:
#     parameters:
#
#-------------------------------------------------------------------------------
function copybootimage {
	# 
	# Compress boot images greater than 4MB.  If image is less than 4MB, then it
	# is probably compressed already.
	#
	if [ $boot_length -gt 8192 ]
	then
		${DD} if=/dev/ipldevice bs=512 skip=$boot_offset count=$boot_length \
			conv=sync 2>/dev/null | ${COMPRESS} > bootimage.save.Z

		# compress returns 2 if the compressed output was larger than
		# the input;  That's not what we want, but it's valid
		if [ $? -eq 1 ]
		then
	   	   return 1
		else
		   return 0
		fi
	else
       		${DD} if=/dev/ipldevice of=bootimage.save bs=512 skip=$boot_offset \
			count=$boot_length conv=sync 2>/dev/null
    	fi
    	return $?
}


#---------------------------- saveboot 
#
# NAME: saveboot
#
# FUNCTION:  Attempt to use /usr as the boot record save directory since 
# the compressed boot image is probably over 2MB in size and it is more 
# likely that there will be more space available there AND it is likely that 
# the space will eventually be used if it becomes necessary to expand the 
# filesystem to accomodate the saved boot image.
#
# DATA STRUCTURES:
#     parameters:
#
#-------------------------------------------------------------------------------
function saveboot {

	brec=${BOOT_SAVEDIR}/bootrec.save
	bimg=${BOOT_SAVEDIR}/bootimage.save
	
	if [ ! -d ${BOOT_SAVEDIR} ]
	then
		${MKDIR} -p ${BOOT_SAVEDIR}
		if [ $? != 0 ]
		then
			error ${ERR_MKDIR} ${BOOT_SAVEDIR}
		fi
	fi

	cd ${BOOT_SAVEDIR}
    	#
    	# Get the filesystem associated with the SAVE directory just in case we need
    	# to expand the filesystem or issue an error message.
	#
       	SAVEFS=`${DF} ${BOOT_SAVEDIR} | ${TAIL} -1 | ${AWK} '{print $NF}'`

    	if [ -s "${brec}" ] && [ -s "${bimg}" -o -s "${bimg}.Z" ]
    	then
       		return 0  #previously saved the boot record, don't do it again
    	fi

    	${DD} if=/dev/ipldevice of=${brec} bs=512 count=1 >/dev/null 2>&1
    	bootrecsaved=$?

    	if [ $bootrecsaved != 0 ] 
    	then
    		# Could not save the boot record, probably due to lack of space 
       		if [ -n "${EXPAND}" ]
       		then
	   		expand ${SAVEFS}
	   		if [ $? = 0 ]
	   		then
    				${DD} if=/dev/ipldevice of=${brec} bs=512 count=1 \
					>/dev/null 2>&1
    	       			bootrecsaved=$?
	   		fi
       		fi
	fi

	# Report failure to save bootrecord if necessary
    	if [ $bootrecsaved != 0 ] 
    	then
       		${RM} -f ${brec}
       		error ${ERR_BOOTREC} ${BOOT_SAVEDIR}
    	fi

    	boot_length=`${OD} -t u -j 36 -w4 -N4  ${brec} | \
		${AWK} 'NF==2 {print $2 + 0}'`
    	boot_offset=`${OD} -t u -j 48 -w4 -N4  ${brec} | \
		${AWK} 'NF==2 {print $2 + 0}'`

    	if [ -z "$boot_offset" ] || [ -z "$boot_length" ]
    	then
		error ${ERR_BOOT_INFO} ${SAVEFS}
    	fi 

	#
    	# Compress boot images greater than 4MB.  If image is less than 4MB, then it
    	# is probably compressed already.
    	copybootimage
   	bootimagesaved=$?

    	# If EXPAND set, try to increase the filesystem by one partition and
    	# try again
    	if [ $bootimagesaved != 0 ] && [ -n "$EXPAND" ]
    	then
		expand $SAVEFS
		if [ $? -eq 0 ]
		then
	   		copybootimage
    	   		bootimagesaved=$?
		fi
    	fi

    	if [ $bootimagesaved != 0 ]
    	then
       		/usr/bin/rm -f ${brec} ${bimg} ${bimg}.Z
       		error ${ERR_BOOT_INFO} ${SAVEFS}
    	fi 
    	return 0
}


#---------------------------- iplromblv 
#
# NAME: iplromblv
#
# FUNCTION:  Write the IPL ROM emulation to the boot logical volume
#
#-------------------------------------------------------------------
function iplromblv {
    PATH=${NIM_METHODS}:/usr/bin:/usr/sbin:/etc
    #
    BLV=hd5
    #
    BLVDISK=$(${LSLV} -l ${BLV} 2>/dev/null | ${AWK} '/^hdisk/ { print $1 }' )
    [ $? != 0 ] || [ -z "${BLVDISK}" ] && error  ${ERR_BLV} /dev/${BLV}
    #
    #

    >/tmp/fake.fs
    ${MKBOOT} -whir \
	    -k${NIM_METHODS}/IPLROM.emulation \
	    -d /dev/${BLVDISK} \
	    -l /dev/${BLV} \
	    -f /tmp/fake.fs \
      | ${DD} of=/dev/${BLV} 2>/dev/null

    [ $? != 0 ] && error ${ERR_IPLR_WRITE} /dev/${BLV} 

    ${RM} -f /tmp/fake.fs
    return 0
}

#
# 	Main program
#
BSERVER=""
CLIENT=""
GATEWAY="0.0.0.0"
NETADPT=""
IPLROMDEV=""
BOOT_SAVEDIR=/usr/lib/boot/iplrom.boot

#
if [ $# -eq 0 ]
then
	echo "Usage: $0 -c <client ip> -n <adapter name (tok0, ent0)> -b <bootp server> [-g <gateway>] [-X]"
	exit 1
fi
#
# Get the command line opts: 
#
while getopts c:n:b:g:X c
do
	case ${c} in

	c) # the client ip address 
		CLIENT=${OPTARG}
	;;

	n) # network adapter to boot from.	
		NETADPT=${OPTARG}
	;;

	b) # bootp server 
		BSERVER=${OPTARG}
	;;
  	
	g) # gateway (if reqd) 
		GATEWAY=${OPTARG} 
	;;

	X) # expand filesystem (if reqd) 
		export EXPAND="-X"
	;;

	\?) # what ?
		echo "ERROR: ${OPTARG} not supported"
		exit 1
	;;
	esac
done

#

if [[ -z "${CLIENT}" ]] || [[ -z "${BSERVER}" ]] || [[ -z "${NETADPT}" ]]
then
	echo "Usage: $0 -c <client ip> -n <adapter name (tok0, ent0)> -b <bootp server> [-g <gateway>] [-X]"
	exit 1
fi
#


#
# Use /usr/sbin/bootinfo if this is a 4.1 system
# Use ${NIM_METHODS}/c_nim_bootinfo32 is this is a 3.2 system
# 3.1 systems are not supported for this command
#


OS_VERSION=`/usr/bin/uname -v`
if [[ -n "$OS_VERSION" ]] && [[ "$OS_VERSION" = "3" ]] 
then
    BOOTINFO="${NIM_METHODS}/c_nim_bootinfo32"
    MKBOOT="${NIM_METHODS}/c_nim_mkboot32"
    [[ ! -d /var/adm/nim ]] && ${MKDIR} -p /var/adm/nim
else
    BOOTINFO="/usr/sbin/bootinfo"
    MKBOOT="/usr/sbin/mkboot"
fi
#
# check the key position (needs to be in normal)
#
[ "$(${BOOTINFO} -k)" != 3 ] && error ${ERR_KEY_POSN}

#
# Find the boot disk
#
export BLV=hd5
export BLVDISK=$(${LSLV} -l ${BLV} 2>/dev/null | ${AWK} '/^hdisk/ { print $1 }' )
[ $? != 0 ] && error ${ERR_BLV} ${BLV}

# was a specific adapter name given?
if [[ ${NETADPT} != *[0-9] ]]
then

        # no specific adapter given - see if we can find it.  If there is
	# more than one, error off and tell user to specify an adapter in
	# machine object's "if" attribute.

        list=$( ${ODMGET} -q"name like '${NETADPT}*'" CuDv | \
                                ${AWK} '$1=="name"{gsub(/"/,"");printf ("%s ",$3)}' )
	if [ -z "${list}" ]
	then
		error ${ERR_INVALID_DEVICE} ${NETADPT}
	fi

	i=1
	for dev in ${list}
	do
		[ $i -gt 1 ] && error ${ERR_MULT_ADPTS_FPUSH} "${list}"
		let i=$i+1
	done
	NETADPT=${list}
fi

#
# Check to see if the ROS can issue bootp over the 
# network adapter
#
isBootable ${NETADPT} 
if [ $? != 0 ]
then
	#
	# Need to write the IPLROM emulation
	#
	# But first, make sure that we can save the old boot record off; if
	# we can't, then the system would become unbootable if a migration
	# installation had to back out
	if [ $standalone -eq 1 ]
	then
	    saveboot 
	    [ $? != 0 ] &&	 exit 1
	fi

 	iplromblv
	[ $? != 0 ] &&	 exit 1

	IPLROMDEV=${BLVDISK}

fi

# default is no specific bootlist info
bootlist_attrs=""

#
# Make sure we use ip addresses.
#

if [[ -n "${GATEWAY}" ]] && \
      [[ -n "${CLIENT}" ]] && \
      [[ -n "${BSERVER}" ]]
then
      # we can specify the gateway info
      bootlist_attrs="gateway=${GATEWAY} client=${CLIENT} bserver=${BSERVER}"

elif  [[ -n "${BSERVER}" ]]
then
      # we may pass bserver alone
      bootlist_attrs="bserver=${BSERVER}"
fi

#
# we're ready to set the boot list. 
#
${BOOTLIST} -m normal ${IPLROMDEV} ${NETADPT} ${bootlist_attrs} ${BLVDISK}
if [ $standalone -eq 1 ]
then
    > /usr/lib/boot/bootlist.altered
fi
#
#
exit 0
