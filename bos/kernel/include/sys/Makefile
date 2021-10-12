#  @(#)35	1.5.1.43  src/bos/kernel/include/sys/Makefile, Makefiles, bos41J, 9514A_all 4/4/95 15:41:09 
#
#   COMPONENT_NAME: INCSYS
#
#   FUNCTIONS: none
#
#   ORIGINS: 10,27,83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

EXPINC_SUBDIRS  = msg ios ldr

VPATH		=${MAKETOP}bos/kernel/sys:${MAKETOP}bos/kernel/sys/${TARGET_MACHINE}:${MAKETOP}bos/objclass:${MAKETOP}bos/usr/bin/errlg:${MAKETOP}bos/usr/lpp/perfagent/lockstat

IDIR		= /kernel/include/sys/

SUBDIRS		= msg

DATAFILES	= \
	FP.h NLchar.h acl.h acpa.h adspace.h access.h      \
	acct.h aio.h asdef.s atomic_op.h audio.h audit.h  	\
	auditk.h bbdir.h buf.h cblock.h cdli.h cfgdb.h cfgodm.h      \
	chownx.h ciouser.h comio.h conf.h configrec.h   \
	context.h core.h cred.h cs.h dbexp.h dbg_codes.h   \
	dbkersym.h ddi.h debug.h dev.h device.h deviceq.h     \
	devinfo.h dir.h dlpi.h dlpistats.h dlpi_aix.h dma.h domain.h      \
	dump.h encap.h erec.h err_rec.h errids.h errno.h ethernet_mibs.h     \
	eucioctl.h eth_demux.h \
	events.h except.h fblk.h fd.h file.h filsys.h flock.h fp_io.h	      \
	fddi_demux.h \
	fs_hooks.h fs_locks.h fstypes.h fullstat.h gea.h generic_mibs.h      \
	gfs.h gpai.h hd_psn.h i_machine.h id.h init.h init_lock.h      \
	ino.h inode.h intr.h io.h ioacc.h iocc.h ioctl.h    \
	ipc.h iostat.h iplcb.h jctype0.h jctype1.h kinfo.h lc_core.h ldr.h   \
	limits.h lkup.h lldebug.h localedef.h localedef31.h \
	lock.h lockf.h lockl.h lock_alloc.h lock_def.h lockname.h low.h	\
	lpio.h ltypes.h lvdd.h m_except.h m_intr.h m_param.h m_types.h 	\
	m_wait.h machine.h malloc.h mbuf.h mman.h mntctl.h mode.h \
	mpc.h msg.h msgbuf.h mstsave.h ndd.h ndd_var.h nlist.h \
	param.h pcl.h pin.h pm.h pmdev.h pmzone.h \
	poll.h pri.h priv.h proc.h processor.h protosw.h pseg.h ptrace.h      \
	pty.h rasprobe.h reg.h reboot.h resource.h rosinfo.h \
	sad.h sched.h seg.h select.h sem.h shm.h signal.h sjisioctl.h \
	sleep.h socket.h socketvar.h space.h specnode.h srcobj.h sstbl.h \
	stat.h	statvfs.h \
	statfs.h str_tty.h strconf.h stream.h strinfo.h strlog.h stropts.h     \
	strstat.h sysconfig.h sysdma.h sysinfo.h syslog.h sysmacros.h   \
	syspest.h sysprobe.h systm.h tape.h tcb.h termio.h termiox.h	\
	thread.h tihdr.h  \
	timod.h tiuser.h time.h timeb.h timer.h timers.h times.h  \
	tokenring_mibs.h trcctl.h trchdr.h trchkid.h trcmacros.h ttmap.h  \
	tok_demux.h tty.h ttychars.h ttydefaults.h    \
	ttydev.h types.h uio.h un.h unpcb.h uprintf.h user.h	      \
	ustat.h uthread.h utsname.h var.h vattr.h vfs.h vlimit.h vminfo.h  \
	vmount.h vmuser.h vnode.h vtimes.h wait.h watchdog.h    \
	xcoff.h xmalloc.h xmem.h xti.h cxma.h	      \
	${${TARGET_MACHINE}_DATAFILES}

INCLUDES	= ${DATAFILES} vmker.h ramdd.h console.h pathname.h \
	vmdisk.h buid.h ctlaltnum.h nio.h buid0.h rtc.h \
	nettrace.h unixmsg.h fscntl.h prio_calc.h dasd.h bootsyms.h \
	hd.h vgsa.h hd_config.h lvmd.h ppdd.h ddtrace.h sd.h \
	listmgr.h hscsidd.h pscsidd.h tmscsidd.h \
	dartic.h darticdd.h mpadefs.h mpadd.h dmpauser.h dmpadd.h \
	mpqpdd.h mpqpdiag.h  \
	pscsi720dd.h scdiskdd.h ddconc.h \
	ascsidd.h vscsidd.h scb_user.h scsi_scb.h \
	probeids.h retids.h \
	lc_asdef.s \
	${${TARGET_MACHINE}_INCLUDES}

BINARIES=yes

.if exists (${TARGET_MACHINE}/machdep.mk)
.include <${TARGET_MACHINE}/machdep.mk>
.endif

.include <${RULES_MK}>

errids.h: ${errids.desc:P}
	${ERRTMPLTBLD} -d ${errids.desc:P} -o ${.TARGET}

retids.h: ${COMPIDSTABLE}
	${RETIDSBLD} ${COMPIDSTABLE} >${.TARGET}

probeids.h: ${probeids.desc:P}
	# This recreates the file that's the 1st parameter
	${PROBEIDSBLD} ${.TARGET} ${probeids.desc:P}

# Build .h files
sysprobe.h:	rasprobe.h
	${SED} 's/SYSPROBE_MAGIC/PROBE_MAGIC/g;/CUSTPROBE_MAGIC/d' ${rasprobe.h:P} >$@

lockname.h: ${lockname.db:P}
	${CP} ${lockname.db:P} .
	${NAMEDB2H}
	${RM} -f lockname.db

