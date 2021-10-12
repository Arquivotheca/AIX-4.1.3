# @(#)66        1.38  src/bos/usr/include/Makefile, incstd, bos41J, 9513A_all 3/28/95 16:10:09
#
#   COMPONENT_NAME:
#
#   FUNCTIONS:
#
#   ORIGINS: 10,27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPONE		= ${TARGET_MACHINE}:${MAKETOP}/bos/kernel/sys:${MAKETOP}/bos/kernext/inc:${MAKETOP}/bos/kernext/inc/${TARGET_MACHINE}
VPTWO		= 
VPTHREE		= ${MAKETOP}/info/usr/lpp/info/bin/info_gr/genrinc
VPATH		= ${VPONE}:${VPTWO}:${VPTHREE}

SUBDIRS		= IN sys arpa net aixif protocols netinet netiso graphics \
		netns pse rpc rpcsvc jfs X11 ${${TARGET_MACHINE}_SUBDIRS}

EXPINC_SUBDIRS	= ${SUBDIRS}

IDIR		= /usr/include/

DATAFILES	= \
	NLchar.h NLctype.h NLregexp.h NLxio.h a.out.h aio.h		       \
	aouthdr.h ar.h asl.h assert.h cf.h cfgresid.h ccsid.h core.h		       \
	cpio.h crypt.h ctype.h cur00.h cur01.h cur02.h cur03.h cur04.h	       \
	cur05.h dbm.h dbug.h dbxstclass.h defenv.h		       \
	dirent.h dos.h doserrno.h dumprestor.h execargs.h exceptab.h   \
	fatal.h  fcntl.h filehdr.h float.h fnmatch.h fp.h fp_fort_c.f	       \
	fp_fort_t.f fptrap.h fpxcp.h fshelp.h fstab.h ftw.h glob.h	       \
	gpoff.h grp.h im.h imerrno.h imlang.h inuerr.h jcode.h jctype.h	       \
	langinfo.h ldfcn.h lib_data.h lib_lock.h limits.h linenum.h loader.h   \
	locale.h login.h lprio.h lvm.h lvmrec.h macros.h malloc.h math.h       \
	mbstr.h memory.h mesg.h mon.h monetary.h mp.h ndbm.h netdb.h	       \
	nl_types.h nlist.h odmi.h paths.h poll.h procinfo.h pwd.h	       \
	regex.h regexp.h reloc.h resolv.h scnhdr.h search.h setjmp.h	       \
	sgtty.h signal.h spc.h srcerrno.h standards.h stdarg.h stddef.h        \
	stdio.h stdlib.h storclass.h string.h strings.h stropts.h syms.h       \
	sysexits.h syslog.h tar.h termio.h termios.h time.h toc.h ttyent.h     \
	typchk.h uinfo.h ulimit.h unctrl.h unistd.h ustat.h userconf.h         \
	userpw.h usersec.h utime.h utmp.h values.h varargs.h sysck.h va_list.h \
	wchar.h wcstr.h wordexp.h wstring.h xcoff.h tiuser.h iconv.h           \
	acl.h bim.h \
	xti.h fmtmsg.h libgen.h

INCLUDES	= ${DATAFILES} curshdr.h unpack.h \
	swvpd0.h imP.h iconvTable.h iconvP.h \
	iconv932.h fcs.h hcs.h liblvm.h inu_toc.h \
	mirror.h ras.h security.h demangle.h tcatst.h \
	hxihtx.h ethtst.h x25tst.h toktst.h dim.h swvpd1.h \
	uconv.h uc_conv.h uc_convP.h \
	cmdnim.h cmdnim_ip.h cmdnim_mstr.h cmdnim_cmd.h cmdnim_structs.h \
	cmdnim_defines.h cmdnim_obj.h timeb.h \
	errno.h swvpd.h

ILIST		= ${DATAFILES}

.if exists (${TARGET_MACHINE}/machdep.mk)
.include <${TARGET_MACHINE}/machdep.mk>
.endif

.include <${RULES_MK}>
