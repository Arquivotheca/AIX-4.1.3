#!/bin/ksh
# @(#)52	1.37.1.23  src/bos/usr/lib/nim/methods/c_sh_lib.sh, cmdnim, bos41J, 9523C_all 6/9/95 19:53:09
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_sh_lib.sh
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

# this file contains shell library routines which are included by all NIM
#		shell scripts

#---------------------------- local defines     --------------------------------
PROG=${0}
PROGNAME=${PROG##*/}
[[ ${0} = /SPOT/?* ]] && SPOT=/SPOT || SPOT=""
TMPDIR=/tmp/$$
ERR=${TMPDIR}/err
OLD_IFS=${IFS}
C_ERRMSG_MSG=0
C_ERRMSG_ERR=1
#---------------------------- include file for NIM error message definitions
. ${NIM_METHODS}/cmdnim_errors.shh

#---------------------------- NIM specific stuff
REQUIRED_TMP_SPACE=2048
NIM_CLIENT_PACKAGE="bos.sysmgt.nim.client"
NIM_SPOT_PACKAGE="bos.sysmgt.nim.spot"
MASTER_UID=root
MSG_MISSING_BOOTI="network boot images"

SPOT_SERVER_OPTIONS="\
	${NIM_CLIENT_PACKAGE} \
	bos.net.tcp.client \
	bos.net.nfs.client"

#--------------------------------- README -------------------------------------
# a SPOT is a very important NIM resource which is used to support all network 
#		boot operations
# in order to construct a SPOT, many optional packages are required
# here's why they're needed:
#		bos.sysmgt.nim.client		: required to support NIM operations
#		bos.sysmgt.nim.spot			: required to support network boot
#		bos.net.nfs.client			: required for NFS support
#		bos.net.tcp.client			: network interface support
#		bos.net.tcp.smit				: tcpip SMIT screens
#		bos.rte.up						: required to support diskless/dataless boot
#		bos.rte.mp						: required to support standalone install
#		bos.diag							: required for diagnostic support
#		bos.sysmgt.sysbr				: required to support standalone install
#		bos.sysmgt.smit				: required for diskless/dataless clients
#		bos.terminfo					: required for diskless/dataless term support
#		devices.all						: NIM needs all the device support it can get.
#											  this is important because NIM has no
#													knowledge about device support needed by
#													any NIM client and, if a client needs
#													support for a device which the SPOT
#													doesn't have, then the network boot
#													operation being performed MIGHT fail
#											  therefore, all device support is needed
DEFAULT_SPOT_OPTIONS="\
	${NIM_CLIENT_PACKAGE} \
	${NIM_SPOT_PACKAGE} \
	bos.net.nfs.client \
	bos.net.tcp.client \
	bos.net.tcp.smit \
	bos.rte.up \
	bos.rte.mp \
	bos.diag \
	bos.sysmgt.sysbr \
	bos.sysmgt.smit \
	bos.terminfo \
	devices.all"

#--------------------------------- README -------------------------------------
# in order to support install operations, NIM must have access to a directory
#		which contains all the images which will be required for an install
#		operation
# the set of images which NIM requires is called "simages" ("s"upport "images")
# if a NIM lpp_source has the complete set of these images, then it the
#		"simages" attribute will be added to the definition of the lpp_source
# the SIMAGES_OPTIONS list is used when creating an lpp_source
# the REQUIRED_SIMAGES list is used to determine whether an lpp_source gets
#		the "simages" attribute or not
SIMAGES_OPTIONS="\
	bos \
	bos.info.any \
	bos.net \
	bos.rte.up \
	bos.rte.mp \
	bos.diag \
	bos.powermgt \
	bos.sysmgt \
	bos.terminfo.all \
	devices.all \
	X11.apps \
	X11.base \
	X11.compat \
	X11.Dt \
	X11.fnt \
	X11.loc \
	X11.motif \
	X11.msg.all \
	X11.vsm"

REQUIRED_SIMAGES="\
	bos \
	bos.net \
	bos.rte.up \
	bos.rte.mp \
	bos.diag \
	bos.sysmgt \
	bos.terminfo \
	bos.terminfo.all.data \
	devices.base.all \
	devices.buc.all \
	devices.graphics.all \
	devices.mca.all \
	devices.scsi.all \
	devices.sio.all \
	devices.sys.all \
	devices.tty.all"

NIMINFO=/etc/niminfo
NIM_AWK="${NIMPATH}/awk"
NIM_CMDS="/usr/sbin"
NIMCLIENT="${NIM_CMDS}/nimclient"
MK_NETBOOT="/tmp/mk_netboot"
C_CH_RHOST="${NIM_METHODS}/c_ch_rhost"
C_CKROS_EMU="${NIM_METHODS}/c_ckros_emu"
C_DUMPCHECK=${NIM_METHODS}/c_dumpcheck
C_ERRMSG=${NIM_METHODS}/c_errmsg
C_INSTALLP="${NIM_METHODS}/c_installp"
C_MK_NIMCLIENT="${NIM_METHODS}/c_mk_nimclient"
C_MKBOOTI="${NIM_METHODS}/c_mkbooti"
C_MKDIR="${NIM_METHODS}/c_mkdir"
C_MKPFILE=${NIM_METHODS}/c_mkpfile
C_NIMINFO="${NIM_METHODS}/c_niminfo"
C_RMDIR=${NIM_METHODS}/c_rmdir
C_STAT=${NIM_METHODS}/c_stat
C_SYNC_ROOT=${NIM_METHODS}/c_sync_root

LPP_NAME="./lpp_name"
IMAGE_DATA="./image.data"
LPP_NAME_IN_SPOT="./lpp/bos/inst_root/lpp_name"
IMAGE_DATA_IN_SPOT="./lpp/bosinst/image.template"
DEFAULT_REQUIRED_VERSION=4
DEFAULT_REQUIRED_RELEASE=0
BOS_PATH_ON_CDROM="usr/sys/inst.images/bos"
DEFAULT_PERMS="775"
INST_ROOT="./lpp/bos/inst_root"
INST_IMAGES="/usr/sys/inst.images"
DEFAULT_INSTALLP_FLAGS="-agQX"
DEFAULT_INSTALLP_OPTIONS="all"
INSTALLP_LOG=/var/adm/ras/nim.installp
OFFSET_FOR_CDROM="/usr/sys/inst.images"
TFTPBOOT="/tftpboot"
SWAPNFS="swapnfs"
INSTROOT_CUST_SZ=/tmp/.installp.root_sz #Used to track root size requirements
					#between c_instspot and c_sync_root
					#for /usr-SPOT inst_root.

STATE_RUNNING=running

PROTECTED_DIRS="/ /usr /var /tmp /home $TFTPBOOT"

# NOTE: The following variable gets used by c_instspot when creating
#       a chroot environment.  The value of IMAGES is the directory
#       over which the lpp_source will be mounted.  This mount point
#       must NOT correspond to any existing filesystem on the server
#       or installp will fail because the stat system call fails
#       because it gets confused.  So, we're using the process ID of
#       c_instspot in the hopes that it will be unique enough that
#       the server will not have a filesystem which uses that name
IMAGES="/$$"

#---------------------------- AIX command pathnames
INURID=${SPOT}/usr/lib/instl/inurid

RCBOOT=${SPOT}/sbin/rc.boot

BACKUP=${SPOT}/usr/sbin/backup
BFFCREATE=${SPOT}/usr/sbin/bffcreate
BOOTINFO=${SPOT}/usr/sbin/bootinfo
BOSBOOT=${SPOT}/usr/sbin/bosboot
CHDEV=${SPOT}/usr/sbin/chdev
CHFS=${SPOT}/usr/sbin/chfs
CHNFSEXP=${SPOT}/usr/sbin/chnfsexp
CHROOT=${SPOT}/usr/sbin/chroot
COMPRESS=${SPOT}/usr/bin/compress
INSTALLP="${SPOT}/usr/sbin/installp"
INUTOC="${SPOT}/usr/sbin/inutoc"
LSATTR=${SPOT}/usr/sbin/lsattr
LSFS=${SPOT}/usr/sbin/lsfs
MKBOOT=${SPOT}/usr/sbin/mkboot 
MKITAB=${SPOT}/usr/sbin/mkitab
MKNFS=${SPOT}/usr/sbin/mknfs
MKNFSEXP=${SPOT}/usr/sbin/mknfsexp
MKTCPIP=${SPOT}/usr/sbin/mktcpip
MOUNT=${SPOT}/usr/sbin/mount
REBOOT=${SPOT}/usr/sbin/reboot
RESTORE=${SPOT}"/usr/sbin/restbyname -Sq"
ROUTE=${SPOT}"/usr/sbin/route"
RSH=${SPOT}"/usr/bin/rsh"
RMNFSEXP=${SPOT}/usr/sbin/rmnfsexp
SAVEBASE=${SPOT}/usr/sbin/savebase
SHUTDOWN=${SPOT}/usr/sbin/shutdown
UNMOUNT=${SPOT}/usr/sbin/unmount

AWK=${SPOT}/usr/bin/awk
BOOTLIST=${SPOT}/usr/bin/bootlist
BASENAME=${SPOT}/usr/bin/basename
CAT=${SPOT}/usr/bin/cat
CHMOD=${SPOT}/usr/bin/chmod
CHOWN=${SPOT}/usr/bin/chown
CP=${SPOT}/usr/bin/cp
CUT=${SPOT}/usr/bin/cut
CPIO=${SPOT}/usr/bin/cpio
DD=${SPOT}/usr/bin/dd
DF=${SPOT}/usr/bin/df
DIRNAME=${SPOT}/usr/bin/dirname
DU="${SPOT}/usr/bin/du -x"
EGREP=${SPOT}/usr/bin/egrep
FGREP=${SPOT}/usr/bin/fgrep
FIND=${SPOT}/usr/bin/find
GREP=${SPOT}/usr/bin/grep
HOSTCMD=${SPOT}/usr/bin/host
HOSTNAME=${SPOT}/usr/bin/hostname
INSTFIX=${SPOT}/usr/sbin/instfix
INUUMSG=${SPOT}/usr/sbin/inuumsg
LN=${SPOT}/usr/bin/ln
LPPCHK=${SPOT}/usr/bin/lppchk
LS=${SPOT}/usr/bin/ls
LSLPP=${SPOT}/usr/bin/lslpp
LSLV=${SPOT}/usr/sbin/lslv
MKDIR=${SPOT}/usr/bin/mkdir
MV=${SPOT}/usr/bin/mv
NM=${SPOT}/usr/bin/nm
OD=${SPOT}/usr/bin/od
ODMADD=${SPOT}/usr/bin/odmadd
ODMCHANGE=${SPOT}/usr/bin/odmchange
ODMDELETE=${SPOT}/usr/bin/odmdelete
ODMGET=${SPOT}/usr/bin/odmget
REFRESH=${SPOT}/usr/bin/refresh
RM="${SPOT}/usr/bin/rm -f"
RMDIR=${SPOT}/usr/bin/rmdir
SED=${SPOT}/usr/bin/sed
SLEEP=${SPOT}/usr/bin/sleep
SLIBCLEAN=${SPOT}/usr/sbin/slibclean
SORT=${SPOT}/usr/bin/sort
SYSDUMPDEV=${SPOT}/usr/bin/sysdumpdev
SYNC=${SPOT}/usr/sbin/sync
TAIL=${SPOT}/usr/bin/tail
TAR=${SPOT}/usr/bin/tar
TCTL=${SPOT}/usr/bin/tctl
UNAME=${SPOT}/usr/bin/uname
WALL=${SPOT}/usr/sbin/wall
WC=${SPOT}/usr/bin/wc

#---------------------------- AIX file pathnames
BOOTPTAB=/etc/bootptab
FILESYSTEMS=/etc/filesystems
FIRSTBOOT=/etc/firstboot
HOSTS=/etc/hosts
INETD_CONF=/etc/inetd.conf
INITTAB=/etc/inittab
IPLROM_SETUP=/usr/lib/boot/iplrom.setup
LICENSE_SIG=/var/adm/.license.sig
RHOSTS="/.rhosts"
SWAPSPACES=/etc/swapspaces
TFTPACCESS=/etc/tftpaccess.ctl

#---------------------------- module globals    --------------------------------
variable=""
value=""
access_pnt=""
mounts=""
tmp_dirs=""
tmp_files=""
unmounts=""
location=""
new_root=""
chroot=""
undo_on_interrupt=""
tape_device_name=""
tape_block_size=""

#---------------------------- bname 
#
# NAME: bname
#
# FUNCTION: do a basename equiv in ksh
#
# DATA STRUCTURES:
#		parameters:
#			 1		- 	The string to work on... 
#
# RETURNS: (char)
#		The result of the basename function	
#
#-------------------------------------------------------------------------------
function bname {

        S=$1
        S=${S%%+(/)}    # remove rightmost /s if any
        S=${S##*/}      # remove path
        echo $S
}

#---------------------------- DEBUG             --------------------------------
#
# NAME: DEBUG
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
#		global:
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function DEBUG {
	typeset tmp=""

	[[ $# > 0 ]] && print $*

	read tmp?">>>>>>>>>>>>>>>>>> DEBUG: any key to continue <<<<<<<<<<<<<<<<<<<"

} # end of DEBUG

#---------------------------- cleanup           --------------------------------
#
# NAME: cleanup
#
# FUNCTION:
#		performs cleanup operations on exit
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
#
# OUTPUT:
#-------------------------------------------------------------------------------
function cleanup {

	typeset i=""

	# anything that needs executing?
	if [[ -s ${TMPDIR}/undo_on_exit ]]
	then

		${CHMOD} +x ${TMPDIR}/undo_on_exit && ${TMPDIR}/undo_on_exit

	fi

	# reset the tape block size if necessary
	if [[ -n "${tape_block_size}" ]] && [[ -n "${tape_device_name}" ]]
	then

		${CHDEV} -l ${tape_device_name} -a block_size=${tape_block_size} \
			1>/dev/null 2>${ERR} || warning_from_cmd ${CHDEV}

	fi

	# unmount anything mounted
	nim_unmount all

	# remove tmp files
	for i in ${tmp_files}
	do
		${RM} ${i} 2>/dev/null
	done

	# remove tmp dirs
	for i in ${tmp_dirs}
	do
		protected_dir ${i} || ${RM} -r ${i} 2>/dev/null
	done

	# remove tmp stuff
	# for unknown reasons, if "eval" is not used at this point, the shell will
	#		complain about not being able to exeucte "/usr/bin/rm -f" (why?????)
	eval ${RM} /tmp/$$.err 2>/dev/null
	eval ${RM} -r ${TMPDIR} 2>/dev/null

} # end of cleanup

#---------------------------- warning           --------------------------------
#
# NAME: warning
#
# FUNCTION:
#		displays warning messages
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
#			1				= errno
#			2				= str1 of error msg
#			3				= str2 of error msg
#			4				= str3 of error msg
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function warning {

	typeset errno=${1:-${ERR_SYS}}
  
	tmpwarn=${TMPDIR}/$$.tmp_warn

	# display the error msg as a warning
        # prepend a translated "warning" string
        warn_str=`${C_ERRMSG} ${MSG_WARNING} ${C_ERRMSG_MSG} "" "" "" "" 2>&1`
	print -n ${warn_str}": " > ${tmpwarn} 2>&1
	${C_ERRMSG} ${1} ${C_ERRMSG_ERR} ${PROGNAME} "${2}" "${3}" "${4}" \
                                 >> ${tmpwarn} 2>&1
	cat ${tmpwarn} 1>&2
	rm ${tmpwarn}
 
	if [[ ${errno} = ${ERR_CMD} ]] && [[ -s ${ERR} ]]
	then

		# remove any "rc=" that might exist in the output
		${AWK} '{gsub(/rc=[0-9]+/,"");print}' ${ERR} 1>&2

	fi


} # end of warning

#---------------------------- warning_from_cmd  --------------------------------
#
# NAME: warning_from_cmd
#
# FUNCTION:
#		displays error message from failed commands but does NOT exit
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		ASSUMES error output from failed commad is in the ${ERR} file
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= name of command which failed
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function warning_from_cmd {

	# display the error msg
	warning ${ERR_CMD} "${1}"

} # end of warning_from_cmd

#---------------------------- error             --------------------------------
#
# NAME: error
#
# FUNCTION:
#		displays the specified error message & exits
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		exits with a "1"
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1				= errno
#			2				= str1 of error msg
#			3				= str2 of error msg
#			4				= str3 of error msg
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function error {

	typeset errno=${1:-${ERR_SYS}}

	# display the error msg
	print "rc=${errno}" 1>&2
	${C_ERRMSG} ${errno} ${C_ERRMSG_ERR} ${PROGNAME} "${2}" "${3}" "${4}"

	# display captured info if necessary
	if [[ ${errno} = ${ERR_CMD} ]] && [[ -s ${ERR} ]]
	then

		# remove any "rc=" that might exist in the output
		${AWK} '{gsub(/rc=[0-9]+/,"");print}' ${ERR} 1>&2

	fi

	# cd to neutral dir
	cd /tmp 2>/dev/null

	# exit
	exit ${1}

} # end of error

#---------------------------- err_from_cmd      --------------------------------
#
# NAME: err_from_cmd
#
# FUNCTION:
#		displays error message from failed commands
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		ASSUMES error output from failed commad is in the ${ERR} file
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= name of command which failed
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function err_from_cmd {

	# display the error msg
	error ${ERR_CMD} "${1}"

} # end of err_from_cmd

#---------------------------- err_signal        --------------------------------
#
# NAME: err_signal
#
# FUNCTION:
#		error signal handler
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#		global:
#
# RETURNS: (int)
#
# OUTPUT:
#-------------------------------------------------------------------------------
function err_signal {

	trap "" 1 2 11 15

	[[ -n "${undo_on_interrupt}" ]] && ${undo_on_interrupt}

	error ${ERR_SIGNAL} 1

} # end of err_signal

#---------------------------- nim_init          --------------------------------
#
# NAME: nim_init  
#
# FUNCTION:
#		initializes the NIM environment for a shell script
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
function nim_init {

	typeset -i tmp_free=0

	# set umask
	umask 022

	# create a dir for tmp files
	# do this now, before anything else because other functions may be called
	#		which depend on this directory
	${MKDIR} ${TMPDIR} 2>/tmp/$$.err || error ${ERR_MKDIR} ${TMPDIR}

	# NIMINFO file must exist
	# in the network boot environment, /SPOT will always contain the niminfo 
	#		file which was tftp'd from the boot server, so look for this version 
	#		of the file first
	if [[ -s /SPOT/niminfo ]]
	then
		. /SPOT/niminfo
	elif [[ -s ${NIMINFO} ]]
	then
		# we're not currently executing in the network boot environment
		# use the version which is in the traditional place
		. ${NIMINFO}
	else
		print "unable to access the ${NIMINFO} file" 1>&2
		exit 1
	fi

	# must have at least 5 meg of free space in /tmp
	tmp_free="$( ${DF} /tmp | ${AWK} 'NR==2{print $3}' 2>/dev/null )"
	(( ${tmp_free} < ${REQUIRED_TMP_SPACE} )) && \
		error ${ERR_SPACE} /tmp ${REQUIRED_TMP_SPACE} ${tmp_free}

} # end of nim_init

#---------------------------- nim_mount         --------------------------------
#
# NAME: nim_mount 
#
# FUNCTION:
#		mounts the specified "object" over the specified local mount point; when
#			no local mount points are specified, this function will create one
#		"object"s may be on of the following:
#			1) local directory
#			2) remote directory (in the form of "host:dir")
#			3) local CDROM
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
#			1						= object to mount
#			2						= local mount point (optional)
#		global:
#			access_pnt			= set to local access point
#			mounts				= list of mounts performed by nim_mount
#
# RETURNS: (int)
#		0							= success; local mount pnt returned via access_pnt
#
# OUTPUT:
#-------------------------------------------------------------------------------

MAX_MNT=5

function nim_mount {

	typeset object=${1}
	typeset -i i=0
	typeset mnt_params=""

	# NOTE that access_pnt is a global, not a local variable
	access_pnt=${2:-""}

	[[ -z "${object}" ]] && return 0

	# check for unnecessary mounts
	if [[ ${object} != ?*:?* ]] && \
		[[ ${object} != /dev/cd[0-9]* ]] && \
		[[ -z "${access_pnt}" ]]
	then
		# object is local directory and no specific mount point requested
		# therefore, no reason to mount it - return current path
		access_pnt=${object}
		return 0
	fi

	# need a local mount point?
	if [[ -z "${access_pnt}" ]]
	then
		# going to create a tmp directory in TMPDIR with a prefix of "mnt"
		# look for lowest, unused seqno
		while (( ${i} < ${MAX_MNT} ))
		do
			access_pnt=${TMPDIR}/mnt${i}
			[[ ! -d ${access_pnt} ]] && break
			let i=i+1
		done
		(( ${i} >= ${MAX_MNT} )) && error ${ERR_GEN_SEQNO} ${TMPDIR}/mnt
	fi

	# need to create the local mount point?
	if [[ ! -r ${access_pnt} ]]
	then

		# yes - doesn't already exist
		${MKDIR} ${access_pnt} 2>${ERR} || err_from_cmd ${MKDIR}

		# if access_pnt ends in "/tftpboot", then don't add it to the list of
		#		dirs to be removed
		# we do this in order to avoid removing the /tftpboot directory in the
		#		inst_root so that clients installed with "source=spot" will get
		#		this directory
		[[ ${access_pnt##*/} != tftpboot ]] && \
			tmp_dirs="${access_pnt} ${tmp_dirs}"
	fi

	# mount the "object" (either local CDROM, remote dir, or local dir)
	case ${object} in

		/dev/cd[0-9]*)	# CDROM
			mnt_params="-r -vcdrfs"
			;;

		?*:?*)			# remote dir
			mnt_params="-ohard,intr -vnfs"
			;;

		*)					# local dir
			mnt_params="-vjfs"
			;;
	esac
	${MOUNT} ${mnt_params} ${object} ${access_pnt} 2>${ERR} || \
		err_from_cmd ${MOUNT}

	# remember that we've got something mounted
	# NOTE that we're pushing this onto a stack - it is important that we do
	#		the unmounts in reverse order
	mounts="${object}:${access_pnt} ${mounts}"

} # end of nim_mount

#---------------------------- nim_unmount       --------------------------------
#
# NAME: nim_unmount
#
# FUNCTION:
#		unmounts anything which has been mounted by nim_mount
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
#			1						= either the keyword "all" or local mount pnt
#		global:
#			mounts				= list of mounts performed by nim_mount
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#-------------------------------------------------------------------------------
function nim_unmount {

	typeset dir=${1:-all}
	typeset i=""
	typeset mnt_pnt=""
	typeset tmp=""

	# cd to neutral dir
	if cd /tmp 2>${ERR}
	then
		:
	else
		warning_from_cmd cd
		return 1
	fi

	# look for the specified <dir>
	for i in ${mounts}
	do
		# separate stanza
		mnt_pnt=${i##*:}

		# this the one we're looking for?
		if [[ ${dir} = ${mnt_pnt} ]] || [[ ${dir} = all ]]
		then

			# unmount it, but first perform a sync and an slibclean to make
			# sure that nothing in the mount is still thought to be busy.
			${SYNC}
			${SLIBCLEAN}
			${UNMOUNT} ${mnt_pnt} 2>${ERR} || warning_from_cmd ${UNMOUNT}

		else

			# not unmounting this dir - add it back to the list
			tmp="${i} ${tmp}"

		fi
	done

	# new list reflects the mounts that are still active
	mounts=${tmp}

} # end of nim_unmount

#---------------------------- parse_attr_ass    --------------------------------
#
# NAME: parse_attr_ass
#
# FUNCTION:
#		validates command line attribute assignements
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		calls error
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
function parse_attr_ass {

	typeset i=""

	# is it in the correct format (<var>=<value>)?
	if [[ "${1}" != +(?)=+(?) ]]
	then
		error ${ERR_VALUE} "${1}" "valid attribute assignment"
	fi

	# separate variable from value
	# NOTE that we must be careful here as the "value" may have "=" chars in it
	variable=${1%%=*}
	value=${1#*=}

	# make sure its in our list of acceptable attributes
	for i in ${REQUIRED_ATTRS}
	do
		if [[ ${variable} = ${i} ]]
		then
			return 0
		fi
	done
	for i in ${OPTIONAL_ATTRS}
	do
		if [[ ${variable} = ${i} ]]
		then
			return 0
		fi
	done
	
	# not an acceptable attr
	error ${ERR_CONTEXT} "${variable}"

} # end of parse_attr_ass

#---------------------------- ck_attrs          --------------------------------
#
# NAME: ck_attrs  
#
# FUNCTION:
#		checks for missing attributes
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
function ck_attrs {

	typeset i=""
	typeset curval=""

	for i in ${REQUIRED_ATTRS}
	do
		eval curval=\$\{${i}\}
		if [[ -z "${curval}" ]]
		then
			error ${ERR_MISSING_ATTR} ${i}
		fi
	done

} # end of ck_attrs

#---------------------------- cmd_what          --------------------------------
#
# NAME: cmd_what
#
# FUNCTION:
#		displays the list of required and optional attributes which this method
#			will accept
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
#			REQUIRED_ATTRS
#			OPTIONAL_ATTRS
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#		writes info to stdout
#-------------------------------------------------------------------------------
function cmd_what {

	typeset i=""

	if [[ -n "${REQUIRED_ATTRS}" ]]
	then
		print "\nrequired attrs are:"
		for i in ${REQUIRED_ATTRS}
		do
			print "\t${i}"
		done
	else
		print "\nno required attrs"
	fi

	if [[ -n "${OPTIONAL_ATTRS}" ]]
	then
		print "\noptional attrs are:"
		for i in ${OPTIONAL_ATTRS}
		do
			print "\t${i}"
		done
	else
		print "\nno optional attrs"
	fi

	print

} # end of cmd_what

#---------------------------- ck_inst_root_dirs --------------------------------
#
# NAME: ck_inst_root_dirs 
#
# FUNCTION:
#		ensures that the SPOT's inst_root has a directory entry for the filesystem
#			where the SPOT resides
#		this is required by installp because, once we've chroot'd into the
#			inst_root, installp is going to stat "/", which will be the SPOT's
#			inst_root at that point - stat will fail if there's no entry for
#			where we've chroot'd into
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
#			location				= SPOT pathname
#			new_root				= pathname of new root (where we'll chroot to)
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function ck_inst_root_dirs {

	typeset fs=""

	# what is the mount point for the filesystem that the SPOT resides in?
	fs=$(${DF} ${location} 2>/dev/null | ${AWK} 'NR==2{print $7}' 2>/dev/null)
	[[ -z "${fs}" ]] && error ${ERR_FS_INFO} ${location}

	# does the directory exist in the new root?
	if [[ ! -d "${new_root}${fs}" ]]
	then

		# create it now
		${MKDIR} ${new_root}${fs} 2>${ERR} || err_from_cmd ${MKDIR}

	fi

} # end of ck_inst_root_dirs

#---------------------------- setup_chroot_env  --------------------------------
#
# NAME: setup_chroot_env
#
# FUNCTION:
#		mounts the appropriate dirs in order to setup a chroot environment
#			within a SPOT
#		this kind of environment is required because we use installp to install
#			software into a SPOT, but it doesn't understand any pathnames other
#			than "/" and "/usr"
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
#			1						= optional; when present, it is the pathname of the
#											inst_root directory to use
#		global:
#			location				= SPOT pathname
#			new_root				= pathname of new root (where we'll chroot to)
#			chroot				= command to execute in order to chroot
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function setup_chroot_env {

	typeset root_sync=""
	typeset inr=""
	typeset inr_path=""
	typeset inr_ln=""
	typeset spot_fs=""
	typeset spot_fs_path=""
	typeset first_dir=""
	typeset dir_created=""

	# initialize pathnames for the inst_root directory
	# this directory will become the root ("/") directory after we chroot
	if [[ -n "${1}" ]]
	then

		root_sync=TRUE
		inr=${1}
		inr_path=${inr}${inr%/*}
		inr_ln=${inr}${inr}

	else

		inr="${location}/${INST_ROOT}"
		inr_path=${inr}/lpp/bos
		inr_ln=${inr_path}/inst_root

	fi

	# initialize pathnames for the SPOT's filesystem which will be created in
	#		the inst_root
	spot_fs=${location}
	[[ -n "${spot_fs%/*}" ]] && \
		spot_fs_path=${inr}${spot_fs%/*} || \
		spot_fs_path=""

	# make sure SPOT exists
	[[ ! -d ${location} ]] && error ${ERR_DIR_DNE} spot ${location}
	[[ ! -d ${location}/${INST_ROOT} ]] && \
		error ${ERR_DIR_DNE} inst_root "${location}/${INST_ROOT}"

	# we use the installp command to install optional software
	# it only understands the absolute pathnames of / & /usr, so we might have
	#		to setup a choot environment to support the use of installp

	# is this a non-/usr SPOT or are we going to root-sync a client?
	if [[ ${location} != /usr ]] || [[ -n "${root_sync}" ]]
	then
	
		# not a /usr SPOT (or a root-sync), so we've got some things to do
		# we've got to construct an environment which we can chroot into in before
		#		calling installp because installp only knows how to install into
		#		the /usr filesystem
		new_root=${inr}

		# make sure no previous mounts have been left dangling
		${UNMOUNT} ${new_root}/usr >/dev/null 2>&1
		${UNMOUNT} ${new_root}/tmp >/dev/null 2>&1
		${UNMOUNT} ${new_root}/tftpboot >/dev/null 2>&1
		${UNMOUNT} ${new_root}/images >/dev/null 2>&1

		# create subdir in the root which corresponds to the filesystem
		#		where the root resides
		if [[ ! -d ${inr_path} ]]
		then

			first_dir=""
			${C_MKDIR} -a location=${inr_path} >${TMPDIR}/first.dir 2>${ERR} || \
				err_from_cmd ${C_MKDIR}
			. ${TMPDIR}/first.dir

			tmp_dirs="${first_dir} ${tmp_dirs}"

		fi
		if [[ ${inr_ln} != / ]]
		then

			# make sure no previous links have been left around
			${RM} -r ${inr_ln} >/dev/null 2>&1

			# create sym link (to fake out the filesystem)
			${LN} -s / ${inr_ln} 2>${ERR} || err_from_cmd ${LN}
			tmp_files="${inr_ln} ${tmp_files}"

		fi


		# create subdir in the inst_root which corresponds to the filesystem
		#		where the SPOT resides
		if [[ -n "${spot_fs_path}" ]] && [[ ! -d ${spot_fs_path} ]]
		then

			first_dir=""
			${C_MKDIR} -a location=${spot_fs_path} >${TMPDIR}/first.dir \
				2>${ERR} || err_from_cmd ${C_MKDIR}
			. ${TMPDIR}/first.dir

			tmp_dirs="${first_dir} ${tmp_dirs}"

		fi
		if [[ ${spot_fs} != /usr ]]
		then

			# make sure no previous links have been left around
			${RM} -r ${inr}${spot_fs} >/dev/null 2>&1

			# create sym links (to fake out the filesystem)
			${LN} -s / ${inr}${spot_fs} 2>${ERR} || err_from_cmd ${LN}
			tmp_files="${inr}${spot_fs} ${tmp_files}"

		fi

		# mount the SPOT over the new_root /usr directory
		nim_mount ${location} ${new_root}/usr

		# mount the server's /tmp over the new_root /tmp directory
		nim_mount /tmp ${new_root}/tmp

		# mount the server's /dev over the new_root /dev directory
		nim_mount /dev ${new_root}/dev

		if [[ -z "${root_sync}" ]]
		then

			# mount the server's /tftpboot over the new_root /tftpboot directory
			nim_mount /tftpboot ${new_root}/tftpboot

		fi

		# initialize the chroot syntax
		chroot="${CHROOT} ${new_root}"

	else
	
		# /usr SPOT
		new_root=""
		chroot=""

	fi

} # end of setup_chroot_env

#---------------------------- ck_rel_level       -------------------------------
#
# NAME: ck_rel_level 
#
# FUNCTION:
#		examines the AIX version/release level of the specified LPP_NAME file
#		returns 0 if the levels are acceptable to NIM; errors out otherwise
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
function ck_rel_level {

	typeset lpp_name_file=${1}
	typeset -i ok_version=${2:-${DEFAULT_REQUIRED_VERSION}}
	typeset -i ok_release=${3:-${DEFAULT_REQUIRED_RELEASE}}

	# can we read the file?
	[[ ! -r "${lpp_name_file}" ]] && error ${ERR_FILE_ACCESS} ${lpp_name_file}

	# parse out the version & release
	${AWK} '	/^. . . bos {$/ {found=1;next};\
				/^bos.rte .*/ {	if (found)\
										{	split($2,info,".");\
											print "version=" info[1];\
											print "release=" info[2];
										}\
									}' \
			${lpp_name_file} >${TMPDIR}/lpp_info 2>${ERR} || err_from_cmd ${AWK}

	# include that info in the current environment
	[[ -s ${TMPDIR}/lpp_info ]] && . ${TMPDIR}/lpp_info

	# now check the values
	if (( ${version} < ${ok_version} ))
	then
		error ${ERR_RELEASE_LEVEL} ${version} ${release} ${lpp_name_file}
	elif (( ${ok_release} > 0 ))
	then
		if (( ${release} < ${ok_release} ))
		then
			error ${ERR_RELEASE_LEVEL} ${version} ${release} ${lpp_name_file}
		fi
	fi

} # end of ck_rel_level

#---------------------------- ck_spot_options        ---------------------------
#
# NAME: ck_spot_options
#
# FUNCTION:
#		determines whether the specified options are installed into the specified
#			SPOT
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= spot pathname
#			2						= options to check on
#		global:
#			st_applied			= value of ST_APPLIED from /usr/include/swvpd.h file
#			st_committed		= value of ST_COMITTED from /usr/include/swvpd.h file
#
# RETURNS: (int)
#		0							= success
#		1							= something missing
#
# OUTPUT:
#		writes missing options as attr assignments to stdout
#-------------------------------------------------------------------------------
function ck_spot_options {

	# make sure st_applied and st_comitted attrs have been supplied
	[[ -z "${st_applied}" ]] && error $ERR_MISSING_ATTR st_applied
	[[ -z "${st_committed}" ]] && error $ERR_MISSING_ATTR st_committed

	typeset spot=${1}
	typeset optlist=${2}

	typeset rc=0
	typeset i=""
	typeset state=""
	typeset database=""

	# get info out of the /usr product database
	if ODMDIR=${spot}/lib/objrepos ${ODMGET} product \
			>${TMPDIR}/product.usr 2>${ERR}
	then
		:
	else
		warning_from_cmd "ODMDIR=${spot}/lib/objrepos ${ODMGET} product"
		return 1
	fi

	# get info out of the /usr/share product database
	if ODMDIR=${spot}/share/lib/objrepos ${ODMGET} product \
			>${TMPDIR}/product.share 2>${ERR}
	then
		:
	else
		warning_from_cmd "ODMDIR=${spot}/share/lib/objrepos ${ODMGET} product"
		return 1
	fi

	# for each option required by NIM for a vaild SPOT...
	for i in ${optlist}
	do
		state=""

		# which database: usr or share?
		[[ ${i} = ?*.data ]] && \
			database=${TMPDIR}/product.share || \
			database=${TMPDIR}/product.usr

		# is the option installed in the SPOT currently?
		state=$(${AWK} -v option=${i} '\
						$1=="product:"{in_stanza=0;};\
						$1=="lpp_name" || $1=="name" {\
							field = "\"" option "\"";\
							if( ($3==field) || (index($3,option) > 0) )\
								in_stanza=1;\
							next};\
					  $1=="state" && in_stanza {print $3; exit}\
							' ${database} 2>/dev/null)

		# is the state acceptable?
		if [[ "${state}" != ${st_applied} ]] && \
			[[ "${state}" != ${st_committed} ]]
		then

			# either totally missing the option or its in a bad state
			print "missing=${i}"

			rc=1
		fi

	done

	return rc

} # end of ck_spot_options

#---------------------------- protected_dir     --------------------------------
#
# NAME: protected_dir
#
# FUNCTION:
#		returns SUCCESS (0) is the specified directory is protected from removal
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= directory to be removed
#		global:
#
# RETURNS: (int)
#		0							= directory is protected and should NOT be removed
#		1							= ok to remove dir
#
# OUTPUT:
#-------------------------------------------------------------------------------
function protected_dir {

	typeset dir=${1}
	typeset i=""

	[[ -z "${dir}" ]] && return 1

	for i in ${PROTECTED_DIRS}
	do

		[[ ${dir} = ${i} ]] && return 0

	done

	# <dir> is not protected from removal
	return 1

} # end of protected_dir

#---------------------------- prep_bundle      --------------------------------
#
# NAME: prep_bundle    
#
# FUNCTION:
#		prepares environment to use an installp bundle file
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
#			installp_bundle	= pathname of bundle file
#			bundle_access		= local access point for the bundle file
#
# RETURNS: (int)
#		0							= success
#
# OUTPUT:
#-------------------------------------------------------------------------------
function prep_bundle {

	[[ -z "${installp_bundle}" ]] && return 0

	bundle_access=${TMPDIR}/bundle

	# local or remote file?
	if [[ ${installp_bundle} = ?*:/* ]]
	then

		# remote - setup local access
		nim_mount ${installp_bundle}

		# copy the bundle into /tmp so installp will have access to it
		${CP} ${access_pnt} ${bundle_access} 2>${ERR} || \
			err_from_cmd "${CP} ${access_pnt} ${bundle_access}"

		# unmount
		nim_unmount ${access_pnt}

	else

		# already local - just copy it
		${CP} ${installp_bundle} ${bundle_access} 2>${ERR} || \
			err_from_cmd "${CP} ${installp_bundle} ${bundle_access}"

	fi

} # end of prep_bundle

#---------------------------- init_bootlist_attrs ------------------------------
#
# NAME: init_bootlist_attrs
#
# FUNCTION:
#		formats the global variable "boolist_attrs" based on the settings of the
#			parameters which are passed in
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#		the formatting of this information is VERY specific and the man page for
#			the bootlist command should be consulted BEFORE changing this function
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#		parameters:
#			1						= client IP address
#			2						= BOOTP server IP address
#			3						= gateway IP address
#		global:
#			bootlist_attrs		= boot device list info
#
# RETURNS: (int)
#		0							= bootlist_attrs set
#
# OUTPUT:
#-------------------------------------------------------------------------------
function init_bootlist_attrs {

	typeset client=${1}
	typeset bserver=${2}
	typeset gateway=${3:-0.0.0.0}

	# default is no specific bootlist info
	bootlist_attrs=""

	# the syntax for the bootlist command dictates that we can only combine
	#		certain elements of the boot_info
	# refer to the bootlist man page for futher details
	if [[ -n "${gateway}" ]] && \
		[[ -n "${client}" ]] && \
		[[ -n "${bserver}" ]]
	then

		# we can specify the gateway info
		bootlist_attrs="gateway=${gateway} client=${client} bserver=${bserver}"

	elif	[[ -n "${bserver}" ]]
	then

		# we may pass bserver alone
		bootlist_attrs="bserver=${bserver}"

	fi

} # end of init_bootlist_attrs

#---------------------------- ck_installp_flags --------------------------------
#
# NAME: ck_installp_flags
#
# FUNCTION:
#		makes sure that there is a "-" in front of all installp_flags
#		any missing "-" are added
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
#			1						= the installp_flags to check
#		global:
#
# RETURNS: (int)
#		0							= success
#		1							= failure
#
# OUTPUT:
#		writes the installp_flags which should be used to stdout
#-------------------------------------------------------------------------------
function ck_installp_flags {

	typeset flags=${1:-${DEFAULT_INSTALLP_FLAGS}}
	typeset new_flags=""

	# make sure "-" is in front of all installp_flags
	new_flags=$( echo ${flags} | ${AWK} '{	for(i=1;i<=NF;i++) \
														{	if (! match($i,/^-.+/) ) \
																printf( "-" ); \
															print $i \
														} }' 2>/dev/null )

	echo ${new_flags:-${flags}}

} # end of ck_installp_flags

#---------------------------- prep_updt_all_lst ----------------------------- 
#
# NAME: prep_updt_all_lst
#
# FUNCTION:
#		generates a list of fileset updates on the media which match
#		installed filesets on the target (SPOT or client)
#
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#               calls error on failure
#		uses lslpp to generate list of installed filesets
#		uses "installp -L" to generate media list
#		uses awk to match the two lists -- matches take v.r.m.f of
#			installed filesets into account
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#               parameters:
#			target_type	= type of target "spot" or "client"
#			lpp_source	= installation media 
#               global:
#			fileset_list	= file containing list generated by this routine
#
# RETURNS: (int)
#               0                        = success if no errors, else errors off
#
# OUTPUT:
#		fileset_list contains list of updates to install
#-------------------------------------------------------------------------------
function prep_updt_all_lst {

	target_type=${1}
        lpp_source=${2}
	lslpp_cmd=${LSLPP}
	instp_cmd=${INSTALLP}
	fileset_list=${TMPDIR}/updt_all.lst

	#initialize some temp files
	> ${TMPDIR}/up_all.instld
	> ${TMPDIR}/up_all.media

	#initialize global "out-list"
	> ${fileset_list}

	#augment lslpp cmd depending upon caller
	[ ${target_type} = "spot" ] && lslpp_cmd="${chroot} ${LSLPP}"

	#list the installed filesets capturing name and level
	${lslpp_cmd} -qLc | \
		${AWK} 'BEGIN { FS=":" } {printf ("%s %s\n", $2, $3)}' | \
        	${SORT} -A -u 1> ${TMPDIR}/up_all.instld 2>${ERR}

	#outta here if nothing to work with or errors
        [ ! -s ${TMPDIR}/up_all.instld ] ||  [ -s "${ERR}" ]  && \
                                                        err_from_cmd ${LSLPP}

	#augment installp cmd depending upon caller
	[ ${target_type} = "spot" ] && instp_cmd="${chroot} ${INSTALLP}"

	#list the contents of the installation media
	${instp_cmd} -qL ${lpp_source} 1>${TMPDIR}/up_all.media 2>${ERR}

	#outta here if nothing to work with or errors
        [ ! -s ${TMPDIR}/up_all.media ] ||  [ -s "${ERR}" ]  && \
                                                        err_from_cmd ${INSTALLP}

	#loop on the list of installed filesets
	#  -- grep the media listing for matches on that name.
	#  -- awk the resultant list looking for filesets with 
	#     greater level than that which is installed.  print them. 
	#     accumulate output in ${fileset_list} file.
        while read fs_name level
        do
                ${FGREP} ${fs_name} ${TMPDIR}/up_all.media \
                                        > ${TMPDIR}/up_all.grep 2>&1
                [ $? -eq 0 ] && \
                        ${AWK} -v name=${fs_name} -v lev=${level} 'BEGIN   {
                        FS=":"
			split(lev, inst_lev, ".")
                        }
                        {
				# check for exact match for name, since the
				# grep may have pulled in supersets of the
				# name being grepped for.
                                if ($2 == name)
				{
					split($3, updt_lev, ".") 
					if (	(inst_lev[1] == updt_lev[1]) \
								&&  \
						(inst_lev[2] == updt_lev[2])  \
								&& \
						((inst_lev[3] < updt_lev[3]) ||\
						 ((inst_lev[3] == updt_lev[3])\
						  && (inst_lev[4] < updt_lev[4]))) )

                                        	printf ("%s %s\n", $2, $3)
				}
                        }' ${TMPDIR}/up_all.grep >> ${fileset_list}

        done < ${TMPDIR}/up_all.instld

        ${RM} ${TMPDIR}/up_all.instld > /dev/null 2>&1
        ${RM} ${TMPDIR}/up_all.media > /dev/null 2>&1

} # end of prep_updt_all_lst

#---------------------------- prep_instfix_lst  ---------------------------
#
# NAME: prep_instfix_lst
#
# FUNCTION:
#		generates a list of fileset updates from a list (string)
#		of keywords or file containing keywords.
#
#
# EXECUTION ENVIRONMENT:
#
# NOTES:
#               calls error on failure.
#		calls instfix command to generate list of filesets.
#		if non-zero returned from instfix, errors off
#
# RECOVERY OPERATION:
#
# DATA STRUCTURES:
#               parameters:
#			target_type	= type of target ("spot" or "client")
#			fix_type	= type of fixes specified ("fixes" list or "bun"
#			fixes		= list of fixes or bundle name
#               global:
#			fileset_list	= file containing list generated by this routine
#
# RETURNS: (int)
#               0                       = success if ok, else errors off.
#
# OUTPUT:
#		fileset_list contains list of filesets matching fix keys input
#-------------------------------------------------------------------------------
function prep_instfix_lst {
	
	target_type=${1}
	fix_type=${2}
	fixes=${3}
	lpp_src=${4}
	instfix_cmd=${INSTFIX}
	fileset_list=${TMPDIR}/fsets.frm.fixes.lst

	#augment instfix  cmd depending upon caller
	[ ${target_type} = "spot" ] && instfix_cmd="${chroot} ${INSTFIX}"

	#initialize global "out-list"
	> ${fileset_list}

        #generate the list from instfix
        if [ ${fix_type} = "fixes" ]
        then   
		# use string of fix keywords
                ${instfix_cmd} -p -k "${fixes}" ${lpp_src} >${fileset_list} 2>&1
        else
		# use fix bundle
                ${instfix_cmd} -p -f ${fixes} ${lpp_src} >${fileset_list} 2>&1
        fi

	#NOTE:  if instfix encounters errors with *any* of the fixes in
	#	a list, it returns non-zero, but sends output to stdout.
	#	So, we need to re-capture the errors and proceed accordingly.
        if [ $? -ne 0 ]
	then
		cat ${fileset_list} > ${ERR} 2>&1
		err_from_cmd ${INSTFIX}
	fi

} # end of prep_instfix_lst

#---------------------------- check_access
#
# NAME: check_access
#
# FUNCTION:
# 	Make sure that client allows the server .rhosts access
#
# RETURNS: (int)
#     0              = success
#     1              = no access
#
#-------------------------------------------------------------------------------
function check_access {

	client=$1

	${RSH} ${client} "echo >/dev/null" 2>/dev/null
	if [ $? -ne 0 ]
	then
		thishost=`/usr/bin/hostname`
		error ${ERR_NO_RHOST} ${client} ${thishost}
	fi
	return 0
}

#---------------------------- check_push_ok
#
# NAME: check_push_ok
#
# FUNCTION:
# 	Make sure that target allows the master to perform push operations
# 	'Target' may not actually be a NIM client.
#
# RETURNS: (int)
#     0              = success
#     1              = no access
#
#-------------------------------------------------------------------------------
function check_push_ok {

	client=$1
	getrc="echo \$?"

	check_access ${client}
	nimpush_ok=`${RSH} ${client} "test -f /etc/nimstop; $getrc"`
	[ $nimpush_ok -eq 0 ] && {\
		error ${ERR_NO_PUSH} ${client} ${client}
	}

	return 0
}

