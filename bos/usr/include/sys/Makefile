# @(#)89      1.64  src/bos/usr/include/sys/Makefile, Makefiles, bos41J, 9514A_all 4/4/95 15:43:05
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

EXPINC_SUBDIRS	= ios ldr msg

VPATH		= ${MAKETOP}/bos/kernel/sys:${MAKETOP}/bos/objclass:${MAKETOP}/bos/usr/bin/errlg:${MAKETOP}/bos/kernext/lft/inc/sys:${MAKETOP}/bos/kernext/inputdd/inc/sys:${MAKETOP}/bos/kernext/disp/sky/inc:${MAKETOP}/bos/kernext/disp/sga/inc:${MAKETOP}/bos/kernext/disp/wga/inc:${MAKETOP}/bos/usr/lpp/perfagent/lockstat:${MAKETOP}/bos/kernext/disp/rby/inc



IDIR		= /usr/include/sys/

DATAFILES	= \
	FP.h NLchar.h acl.h adspace.h access.h acct.h aio.h \
	asdef.s atomic_op.h audio.h audit.h auditk.h bbdir.h \
	buf.h cblock.h cdli.h cfgdb.h cfgodm.h chownx.h ciouser.h comio.h \
	conf.h configrec.h context.h core.h cred.h cs.h	dbexp.h	   \
	dbg_codes.h dbkersym.h ddi.h debug.h dev.h device.h deviceq.h     \
	devinfo.h dir.h dlpi.h dlpistats.h dlpi_aix.h dma.h domain.h dump.h \
	entdisp.h eth_demux.h err_rec.h errids.h errno.h ethernet_mibs.h \
	eucioctl.h events.h except.h fddi_demux.h \
	fblk.h fd.h file.h filsys.h flock.h fp_io.h fs_hooks.h fstypes.h     \
	fullstat.h generic_mibs.h gfs.h gpai.h hd_psn.h \
	i_machine.h id.h init.h ino.h inode.h intr.h io.h ioacc.h iocc.h \
	ioctl.h iostat.h ipc.h iplcb.h jctype0.h jctype1.h kinfo.h \
	lc_core.h lc_layout.h lc_sys.h ldr.h limits.h lkup.h \
	lldebug.h localedef.h lockname.h \
	localedef31.h lock.h lock_alloc.h lockf.h lockl.h lock_def.h low.h \
	lpio.h ltypes.h lvdd.h m_except.h m_intr.h m_param.h     \
	m_types.h m_wait.h machine.h malloc.h mbuf.h mman.h      \
	mntctl.h mode.h	msg.h msgbuf.h mstsave.h  \
	ndd.h ndd_var.h nlist.h  \
	param.h	pcl.h pin.h pm.h pmdev.h pmzone.h poll.h pri.h priv.h \
	probe.h proc.h protosw.h processor.h    \
	pseg.h ptrace.h pty.h  reg.h reboot.h      \
	resource.h rosinfo.h sad.h sched.h seg.h select.h      \
	sem.h shm.h signal.h sleep.h socket.h socketvar.h space.h \
	specnode.h srcobj.h stat.h statvfs.h statfs.h str_tty.h strconf.h     \
	stream.h strinfo.h strlog.h stropts.h strstat.h sysconfig.h \
	sysdma.h sysinfo.h syslog.h sysmacros.h syspest.h systm.h   \
	tape.h tcb.h termio.h termiox.h thread.h tihdr.h timod.h tiuser.h \
	time.h timeb.h timer.h timers.h times.h tokenring_mibs.h trcctl.h \
	tok_demux.h trchdr.h trchkid.h \
	trcmacros.h ttmap.h ttychars.h ttydev.h      \
	types.h	uio.h un.h unpcb.h uprintf.h user.h ustat.h uthread.h	  \
	utsname.h var.h	vattr.h	vfs.h vlimit.h vminfo.h vmount.h  \
	vmuser.h vnode.h vtimes.h wait.h watchdog.h  \
	xcoff.h	xmalloc.h xmem.h xti.h cxma.h    \
	${${TARGET_MACHINE}_DATAFILES}

INCLUDES	= ${DATAFILES} rtc.h \
	listmgr.h dasd.h hd_config.h vgsa.h hd.h vmker.h ramdd.h \
	license.h unixmsg.h fscntl.h console.h tmscsidd.h ppdd.h \
	mpadefs.h mpadd.h dmpauser.h dmpadd.h mpqp.h mpqpdd.h mpqpdiag.h \
	dartic.h darticdd.h rasprobe.h sd.h \
	scdiskdd.h lc_layoutP.h sysprobe.h sstbl.h \
	ascsidd.h vscsidd.h erec.h \
	dispauth.h tty.h \
	probeids.h retids.h encap.h\
	lc_asdef.s \
	${${TARGET_MACHINE}_INCLUDES}

ILIST		= ${DATAFILES}

.if exists (${TARGET_MACHINE}/machdep.mk)
.include <${TARGET_MACHINE}/machdep.mk>
.endif

.include <${RULES_MK}>

tty.h:  str_tty.h
	${RM} -f ${.TARGET}
	${ECHO} "/* This is tty.h and it is empty now */" > ${.TARGET}

cfgodm.h: cfgodm.cre
	${CP} ${cfgodm.cre:P} .
	${ODMCREATE} -h -p cfgodm.cre
	${RM} -f cfgodm.cre cfgodm.c 

srcobj.h: srcobj.cre
	${CP} ${srcobj.cre:P} .
	${ODMCREATE} -h -p  srcobj.cre
	${RM} -f srcobj.cre srcobj.c

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

probe.h:	rasprobe.h
	${SED} 's/CUSTPROBE_MAGIC/PROBE_MAGIC/g;/SYSPROBE_MAGIC/d' ${rasprobe.h:P} >$@

lockname.h: lockname.db
	${CP} ${lockname.db:P} .
	${NAMEDB2H}
	${RM} -f lockname.db


