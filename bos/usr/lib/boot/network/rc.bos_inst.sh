# @(#)19  1.19  src/bos/usr/lib/boot/network/rc.bos_inst.sh, cmdnim, bos41J, 9521A_all 5/23/95 15:02:06
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: SubSet
#		mount_from_list
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
#
# This script is to be run as a "dot" script from within rc.boot only.
# Function: BOS installation configuration on network booted machine.

# NOTE: Since the rc.diag file is very similar to this file, please check
# to see if changes made to either file are appropriate for the other.

mount_from_list()
{
${NIM_DEBUG}

	# Mount anything else from the info file
	# format of each stanza is:
	#		<hostname>:<remote dir>:<local mntpnt>
	if [ -n "${NIM_MOUNTS}" ]
	then
		${SHOWLED} 0x610
		OIFS="$IFS"
		for mount_args in ${NIM_MOUNTS}
		do
			IFS=':'
			set -- ${mount_args}
			IFS="$OIFS"

			# make sure local mount point exists
			if [ ! -d "${3}" ]
			then
				/SPOT/usr/bin/mkdir -p ${3} || loopled 0x611
			fi

			mount $1:$2 $3 || loopled 0x611

		done
	fi
}

SubSet()
{
set +x

# This function creates links for selected device types and classes.
# This provides a means to control which devices are configured, which
# saves time and system resources.  Links are from the remotely mounted
# filesystem which is mounted over /SPOT to the /usr/lib/methods directory.

for i in 3353c088 3353c188 3353c288 3353c388 8efc 8fba 8ff4 ascsi bbl colorgda \
	fd fda fga gga graygda hga hispd3d hscsi ide0 ide1 isa_keyboard \
	keyboard keyboard_2 kma lft ncr810 nep ppr pscsi s1a s1a_1 s1a_3 s2a \
	s2a_1 s2a_3 s3a_3 serdasda serdasdc sga sga sga_bus sio sio_1 sio_2 \
	sio_3 sys tty vscsi wfg wga
do
	odmget -q type=$i PdDv
done | egrep "Change|Configure" >/tmp/list1

for i in bus disk keyboard lvm tape
do
	odmget -q class=$i PdDv
done | egrep "Change|Configure|Define" >>/tmp/list1

odmget -q "uniquetype = adapter/mca/ppr and attribute = cfg_method_load" PdAt| \
	egrep "values" >> /tmp/list1

odmget -q "uniquetype like adapter/*sio/* and attribute = load_module" PdAt| \
        egrep "values" >> /tmp/list1

# sort it with unique option to prevent duplicates
sort -u /tmp/list1 |
	awk '$3 != "\"\"" {
		gsub(/"/, "", $3);
		gsub("/etc/", "/usr/lib/", $3);
		print "/SPOT" $3;
	}' > /tmp/list2

rm /tmp/list1
ln -fs $(cat /tmp/list2) /usr/lib/methods
rm /tmp/list2
}

##################### MAIN ########################
#
# -----------------------  PHASE ONE
#
${NIM_DEBUG}

case "$PHASE" in
	1)
	# update the Mstate
	/SPOT/usr/sbin/nimclient -S booting

	mount_from_list
	${SHOWLED} 0x622
set +x
	/SPOT/usr/bin/rm -fr /etc/init /usr/bin /usr/lib/drivers \
		/usr/lib/microcode /usr/lib/netsvc /usr/sbin

	/SPOT/usr/bin/ln -s /SPOT/usr/* /usr
	ln -fs /SPOT/usr/lib/!(libc.a|libs.a) /usr/lib
	ln -s /SPOT/usr/lib/boot/ssh /etc/init
${NIM_DEBUG}

	SubSet
	strload -f /dev/null
	cfgmgr -f -v
	${SHOWLED} 0x622
set +x
	# now link to the full databases
	ln -fs /usr/lib/objrepos/* /etc/objrepos
	ln -s /SPOT/usr/lib/methods/cfgcon /SPOT/usr/lib/methods/startlft \
		/SPOT/usr/lib/methods/starttty /usr/lib/methods
${NIM_DEBUG}
	SubSet
	export DEV_PKGNAME=ALL
	cfgmgr -s -v

	# do this link at the end of this phase because ksh has libs.a
	# open and we don't want multiple copies in memory at the same
	# time.  After this phase ends, init will restart ksh and ksh
	# will load the linked copy and the in-core copy will be removed
	#
	# we cannot do the same thing for libc.a because init has libc.a
	# open and init doesn't get restarted until phase 3 newroot
	ln -fs /SPOT/usr/lib/libs.a.min /usr/lib/libs.a

	${SHOWLED} 0x622
	;;

#
# -----------------------  PHASE TWO
#
	2)

set +x
	unset fd_invoker loopled
	shift $#

	rm -f /sbin/rc.boot

${NIM_DEBUG}

	# set TCP timeouts to reasonable values (default is 2 hours!!!)
	/SPOT/usr/sbin/no -o tcp_keepintvl=150
	/SPOT/usr/sbin/no -o tcp_keepidle=1200

	# the next three lines are only for testing.  for actual installs,
	# these three lines will be replaced by the exec of bi_main
#	/usr/lib/methods/cfgcon
#	PS1='RAMFS> '; export PS1
#	exec /usr/bin/ksh

	${SHOWLED} 0xfff
	exec /usr/lpp/bosinst/bi_main
	;;
esac
