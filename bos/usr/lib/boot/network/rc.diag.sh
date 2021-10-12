# @(#)78	1.6  src/bos/usr/lib/boot/network/rc.diag.sh, cmddiag, bos411, 9428A410j 6/28/94 15:39:23
#
#   COMPONENT_NAME: CMDDIAG
#
#   FUNCTIONS: SubSet
#		mount_from_list
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# This script is to be run as a "dot" script from within rc.boot only.
# Function: BOS installation configuration on network booted machine.
#

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
#
# NOTE: This list of devices MUST be kept in sync with the other rc.* files
# used for network boot/install.

for i in 8efc 8fba 8ff4 ascsi colorgda fd fda graygda hispd3d hscsi keyboard \
	keyboard_2 lft ppr pscsi s1a s1a_1 s1a_3 s2a s2a_1 s2a_3 s3a_3 \
	serdasda serdasdc sga sga_bus sio sio_1 sio_2 sio_3 sys tty vscsi \
	wga
do
	odmget -q type=$i PdDv
done | egrep "Change|Configure" >/tmp/list1

# Disk, tape and lvm are not included in the following list. The disk and
# tape are configured if not already before testing.

for i in bus keyboard
do
	odmget -q class=$i PdDv
done | egrep "Change|Configure|Define" >>/tmp/list1

odmget -q "attribute = cfg_method_load" PdAt | egrep "values" >> /tmp/list1

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
	cfgmgr -s -v
	ln -s /SPOT/usr/lib/methods/* /usr/lib/methods

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

	# configure the console and
	# copy the empty diagnostic object classes over to
	# the RAM filesystem from the remotely mounted system.
	# then execute the diagnostic pretests before running diag.
	/usr/lib/methods/cfgcon
	${SHOWLED} 0xfff
	for i in CDiagDev TMInput MenuGoal FRUB FRUs DAVars
	do
		cp /usr/lpp/diagnostics/obj/$i /etc/objrepos/$i
	done
	mkdir -p /etc/lpp/diagnostics/data
	/usr/lib/methods/startrcm >/dev/console 2>&1
	/usr/lib/methods/cfgrcm -l rcm0 >/dev/console 2>&1
	/usr/lpp/diagnostics/bin/diagpt </dev/console >/dev/console 2>&1
	exec /usr/lpp/diagnostics/bin/diagipl </dev/console >/dev/console 2>&1
	;;
esac
