# @(#)87        1.51  src/bos/usr/ccs/lib/libc/Makefile, libc, bos41J, 9521B_all 5/25/95 10:02:32
#
#   COMPONENT_NAME: LIBC
#
#   FUNCTIONS: defined
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#

#   This Makefile is used to build 4 libraries
#	lib/libc.a		normal libc
#	lib/libc_r.a		thread safe libc
#	lib/libp/libc.a		profiled normal libc
#	lib/libp/libc_r.a	profiled thread safe libc
#   PROFILED is defined if building a profiled library
#   THREAD_SAFE is defined if building an _r library  

#
# common between all 4 libraries
#
LIBC_PATH		= ${MAKETOP}/bos/usr/ccs/lib/libc
LIBC_R_PATH		= ${MAKETOP}/bos/usr/ccs/lib/libc_r
LIBM_PATH		= ${MAKETOP}/bos/usr/ccs/lib/libm
KERN_EXP_PATH		= ${MAKETOP}/bos/kernel/exp
KERN_EXT_EXP_PATH	= ${MAKETOP}/bos/kernext/exp

#
# OTHER_LIBRARIES is used to tell an update build that these are libraries
# Referenced in osf.aix.std.mk
#

#
# For thread safe libraries, add _r
#
.if defined(THREAD_SAFE)
LIBC		= libc_r.a
CRT0		= crt0_r.o
GCRT0		= gcrt0_r.o
MCRT0		= mcrt0_r.o
THREADS_INIT	= __threads_init.o
OTHER_LIBRARIES	= libc_r.a
.else
LIBC		= libc.a
CRT0		= crt0.o
GCRT0		= gcrt0.o
MCRT0		= mcrt0.o
THREADS_INIT	= 
OTHER_LIBRARIES	= libc.a libc.a.min libcsys.a
.endif

.if !defined(PROFILED)
#
# Common between non-profiled libc.a and libc_r.a
#
EXPLIB_SUBDIRS	= profiled
SUBDIRS		= profiled
IDIR		= /usr/ccs/lib/
LIBS		= libs_nshr.a libdes_nshr.a

#
# thread_safe specific
#
.if defined (THREAD_SAFE)
INCFLAGS	= -I${LIBC_PATH} -I${LIBC_PATH}/${TARGET_MACHINE}
VPATH		= ${TARGET_MACHINE}:${LIBC_PATH}:${LIBC_PATH}/${TARGET_MACHINE}:${LIBM_PATH}:${LIBM_PATH}/${TARGET_MACHINE}:${KERN_EXP_PATH}:${KERN_EXT_EXP_PATH}
EXPLIB_TARGETS	= export_libc_r.a
ILIST		= libc_r.a ${ILIST_OTHERS}
BINARIES	= libc_r.a
CFLAGS		= -D_THREAD_SAFE
M4FLAGS		= -D_THREAD_SAFE


#
# libc.a specific
#
.else
INCFLAGS	= -I${LIBC_PATH}/${TARGET_MACHINE}
VPATH		= ${TARGET_MACHINE}:${LIBM_PATH}:${LIBM_PATH}/${TARGET_MACHINE}:${KERN_EXP_PATH}:${KERN_EXT_EXP_PATH}
EXPLIB_TARGETS	= export_libc.a export_libcsys.a export_libc.a.min
ILIST		= libc.a libcsys.a libc.a.min bmalloc.o ${ILIST_OTHERS}
BINARIES	= libc.a libcsys.a libc.a.min

.endif

#
# Profiled libraries
#
.else

#
# common between libc.a and libc_r.a
#
IDIR		= /usr/ccs/lib/libp/
LIBS		= libs_nshr.a libdes_nshr.a ${PROF_LIBS}

#
# thread_safe specific
#
.if defined(THREAD_SAFE)
INCFLAGS	= -I${LIBC_R_PATH} -I${LIBC_R_PATH}/${TARGET_MACHINE} \
		  -I${LIBC_PATH}   -I${LIBC_PATH}/${TARGET_MACHINE}
VPATH		= ${LIBC_R_PATH}:${LIBC_R_PATH}/${TARGET_MACHINE}:${LIBC_PATH}:${LIBC_PATH}/${TARGET_MACHINE}:${LIBM_PATH}:${LIBM_PATH}/${TARGET_MACHINE}:${KERN_EXP_PATH}:${KERN_EXT_EXP_PATH}
EXPLIB_TARGETS	= export_libc_r.a
ILIST		= libc_r.a
BINARIES	= libc_r.a
CFLAGS		= -D_THREAD_SAFE -p

#
# libc.a specific
#
.else
INCFLAGS	= -I${LIBC_PATH} -I${LIBC_PATH}/${TARGET_MACHINE}
VPATH		= ${LIBC_PATH}:${LIBC_PATH}/${TARGET_MACHINE}:${LIBM_PATH}:${LIBM_PATH}/${TARGET_MACHINE}:${KERN_EXP_PATH}:${KERN_EXT_EXP_PATH}
EXPLIB_TARGETS	= export_libc.a
ILIST		= libc.a
BINARIES	= libc.a
CFLAGS		= -p
.endif
.endif


divide.o_CFLAGS		:= -U__MATH__ ${CFLAGS}

MSGHDRS		= fshelp_msg.h libc_msg.h
CATFILES	= fshelp.cat libc.cat
HFILES		= ${MSGHDRS}

glink.o_IDIR	= /usr/lib/

INCLUDES	= libc_msg.h
libc_msg.h_EXPDIR = /usr/include/

#
# the following files are common for all 4 libraries shr.o
#
# ######  #######    #    ######  #     # #######
# #     # #         # #   #     # ##   ## #
# #     # #        #   #  #     # # # # # #
# ######  #####   #     # #     # #  #  # #####
# #   #   #       ####### #     # #     # #
# #    #  #       #     # #     # #     # #
# #     # ####### #     # ######  #     # #######
#
# WARNING: When you need to add files to this list, please
#          add them at the end of the list.  The object files
#          are ordered this way to reduce the working set
#          on a running system.
#

SHARED_OFILES	= \
        __getmaxdispwidth.o __getmbcurmax.o errno.o \
	cuexit.o data.o strlen.o fopen.o malloc.o malloc_y.o \
	whatbucket.o flsbuf.o printf.o printf64.o printf128.o \
	doprnt.o doprnt64.o doprnt128.o NLinit.o \
	filbuf.o strncmp.o atoi.o getenv.o strrchr.o isupper.o \
	fputc.o tolower.o _tolower.o fgets.o errlst.o strchr.o isdigit.o \
	doscan.o doscan128.o scanf.o scanf64.o scanf128.o bzero.o \
	signal.o perror.o isspace.o fwrite.o getuid.o abort.o \
	getpid.o sleep.o fread.o islower.o toupper.o _toupper.o \
	gethostent.o memset.o atol.o getpwent.o stat.o ungetc.o \
	fputs.o fseek.o isascii.o strtok.o setjmper.o ctime.o \
	gettimeofday.o system.o isalpha.o strspn.o \
	isprint.o ftell.o strpbrk.o isalnum.o setlocale.o strcspn.o \
	xdr.o clnt_udp.o gets.o fcntl.o	rpc_commondata.o qsort.o \
	atof.o getcwd.o fork.o strstr.o abs.o memcmp.o dup.o getwd.o \
	rew.o cuserid.o strtod.o bcmp.o ioctl.o getlogin.o \
	isxdigit.o mktemp.o index.o getservent.o fgetc.o  \
	rindex.o popen.o inet_addr.o opendir.o rand.o closedir.o \
	readdir.o fdopen.o execl.o alarm.o catgets.o wait.o \
	catopen.o geteuid.o sigblock.o assert.o getdtblsz.o getopt.o \
	execvp.o isatty.o iscntrl.o tempnam.o setitimer.o \
	sigops.o inet_ntoa.o getgid.o ispunct.o setbuf.o execlp.o \
	execv.o ttyname.o bsearch.o putenv.o puts.o random.o \
	tmpnam.o sigvec.o write.o setvbuf.o getitimer.o getgrent.o \
	strdup.o utime.o isgraph.o killpg.o ffs.o rexec.o \
	strerror.o tmpfile.o utimer.o strftime.o getegid.o lfind.o \
	toascii.o iswctype.o wctype.o basename.o dirname.o endvfsent.o \
	fshelp.o getbyflag.o getbyname.o getbytype.o getut.o getvfsent.o \
	parsevfs.o setvfsent.o __csid_std.o __wcsid_std.o csid.o wcsid.o \
	iswalpha.o iswcntrl.o iswdigit.o iswgraph.o iswlower.o iswprint.o \
	iswpunct.o iswspace.o iswupper.o iswxdigit.o towlower.o \
	get_wctype.o __get_wctype_std.o	is_wctype.o __is_wctype_sb.o \
	__is_wctype_std.o iswalnum.o __towlower_std.o towupper.o \
	__towupper_std.o __mbstopcs.o __mbstopcs_932.o __mbstopcs_eucjp.o \
	__mbstopcs_sb.o __mbtopc.o __mbtopc_932.o __mbtopc_eucjp.o	\
	__mbtopc_sb.o __pcstombs.o __pcstombs_sb.o __pcstombs_932.o	\
	__pcstombs_eucjp.o __pctomb.o __pctomb_sb.o __pctomb_932.o	\
	__pctomb_eucjp.o mblen.o __mblen_932.o __mblen_eucjp.o		\
	__mblen_sb.o mbstowcs.o __mbstowcs_932.o __mbstowcs_eucjp.o	\
	__mbstowcs_sb.o mbtowc.o __mbtowc_932.o __mbtowc_eucjp.o	\
	__mbtowc_sb.o wcstombs.o __wcstombs_932.o __wcstombs_eucjp.o	\
	__wcstombs_sb.o wcswidth.o __wcswidth_932.o __wcswidth_eucjp.o	\
	__wcswidth_latin.o wctomb.o __wctomb_932.o __wctomb_eucjp.o	\
	__wctomb_sb.o wcwidth.o __wcwidth_932.o __wcwidth_eucjp.o	\
	__wcwidth_latin.o strtof.o _itrunc.o _uitrunc.o \
	copysign.o floor.o a64l.o atoff.o atoi_table.o ecvt.o \
	fp_const.o gcvt.o labs.o l64a.o rint.o \
	cfgetispeed.o cfgetospeed.o cfsetispeed.o \
	cfsetospeed.o gtty.o setcsmap.o stty.o tcdrain.o \
	tcflow.o tcflush.o tcgetattr.o tcgetpgrp.o tcsendbreak.o \
	tcsetattr.o tcsetpgrp.o termdef.o ttylock.o \
	creadir.o ftw.o	mkstemp.o pathconf.o sysconf.o \
	__strftime_std.o strptime.o __strptime_std.o wcsftime.o	\
	__wcsftime_std.o __strfmon_std.o strfmon.o \
	mount.o read.o confstr.o crt0msg.o divide.o drand48.o	\
	ftok.o l3.o mon.o move.o ptrgl.o isinet_addr.o inet_lnaof.o \
	inet_mkadr.o inet_netof.o inet_netwk.o inet_ntoa.o \
	ns_addr.o ns_ntoa.o iso_addr.o linkaddr.o DStemp.o \
	clrerr.o ctermid.o feof.o ferror.o fgetpos.o \
	fgetwc.o fgetws.o fileno.o findiop.o fputwc.o fputws.o \
	fsetpos.o getc.o getchar.o getdirent.o getw.o \
	getwc.o getws.o getwchar.o putc.o \
	putchar.o putw.o putwc.o putwchar.o putws.o	\
	remove.o rewinddir.o scandir.o seekdir.o setbuffer.o \
	tell.o telldir.o ttyslot.o ungetwc.o \
	__lc_load.o NLSsetup.o __localeconv_std.o	\
	localeconv.o __nl_langinfo_std.o nl_langinfo.o		\
	calloc.o memccpy.o memchr.o moveeq.o	\
	swab.o fill.o msem_init.o msem_lock.o \
	msem_unlock.o catgetmsg.o ndbm.o		\
	ether_addr.o getnetent.o getrpcent.o \
	gprotoent.o herror.o innetgr.o rcmd.o \
	res_comp.o res_debug.o res_init.o res_mkqry.o res_query.o \
	res_send.o ruserpass.o bresvport.o gnetgrent.o \
	strcasecmp.o fnmatch.o __fnmatch_C.o __fnmatch_sb.o	\
	__fnmatch_std.o glob.o regcomp.o __regcomp_std.o	\
	regerror.o __regerror_std.o regexec.o __regexec_std.o	\
	regfree.o __regfree_std.o regex.o rpmatch.o __rpmatch_C.o	\
	__rpmatch_std.o wordexp.o \
	clock.o insque.o load.o	mkfifo.o nice.o nlist.o syslog.o \
	isastream.o quota_init.o auth_des.o auth_none.o	\
	auth_unix.o authdes_prot.o authunix_prot.o clnt_generic.o \
	clnt_perror.o clnt_raw.o clnt_simple.o clnt_tcp.o	\
	rpc_dtablesize.o get_myaddress.o key_call.o	\
	key_prot.o netname.o oncmsg.o openchild.o rtime.o		\
	pmap_clnt.o pmap_getmaps.o pmap_getport.o pmap_prot.o	\
	pmap_prot2.o pmap_rmt.o rpc_prot.o \
	rpc_callmsg.o svc.o svcauth_des.o svc_auth.o	\
	svc_auth_unix.o svc_raw.o svc_run.o svc_simple.o svc_tcp.o	\
	svc_udp.o xdr_array.o xdr_float.o xdr_mem.o	\
	xdr_rec.o xdr_reference.o xdr_stdio.o yp_all.o yp_bind.o yp_enum.o \
	yperr_str.o yp_master.o	yp_match.o yp_order.o ypprot_err.o \
	ypv1_xdr.o yp_update.o ypup_prot.o ypxdr.o maint_xdr.o \
	getpw.o getuinfo.o initgroups.o putgrent.o   \
	putpwent.o setegid.o setregid.o setrgid.o setruid.o    \
	crypt.o kleenup.o getpass.o _signal.o psignal.o raise.o siginter.o \
	siglist.o sigpause.o ssignal.o hsearch.o lsearch.o	\
	tsearch.o character.o	\
	colval.o replacement.o	\
	strcat.o strcmp.o strcoll.o __strcoll_C.o	\
	__strcoll_sb.o __strcoll_std.o strxfrm.o __strxfrm_C.o	\
	__strxfrm_sb.o __strxfrm_std.o strcpy.o strncat.o strncpy.o \
	__wcscoll_C.o __wcscoll_std.o wcsxfrm.o __wcsxfrm_C.o	\
	__wcsxfrm_std.o wcscmp.o wcsncmp.o wcscat.o wcschr.o	\
	wcscspn.o wcslen.o wcsncat.o wcscpy.o wcsncpy.o wcspbrk.o	\
	wcsrchr.o wcsspn.o wcstok.o wcswcs.o wcscoll.o exec_args.o \
	execle.o exect.o getpgsize.o stime.o time.o \
	vlimit.o difftime.o nanotimers.o settimeofday.o \
	AFfndrec.o AFgetatr.o AFgetrec.o AFnxtrec.o AFopen.o	\
	AFread.o AFrewind.o AFclose.o \
	NCchrlen.o NCcollate.o NCcoluniq.o NCcolval.o	\
	NCdec.o NCdechr.o NCdecode.o NCdecstr.o NCenc.o NCencode.o	\
	NCencstr.o NCeqvmap.o NCesc.o NCflatchr.o NCgetbuf.o	\
	NCisNLchar.o NCisalnum.o NCisalpha.o NCiscntrl.o NCisdigit.o	\
	NCisgraph.o NCislower.o NCisprint.o NCispunct.o NCisshift.o	\
	NCisspace.o NCisupper.o NCisxdigit.o NCstrcat.o NCstrchr.o	\
	NCstrcmp.o NCstrcpy.o NCstrcspn.o NCstrdup.o NCstrlen.o	\
	NCstrncat.o NCstrncmp.o NCstrncpy.o NCstrpbrk.o NCstrrchr.o	\
	NCstrspn.o NCstrtok.o NCtoNLchar.o NCtolower.o NCtoupper.o	\
	NCunesc.o NCwunesc.o NLchrlen.o NLcplen.o NLcsv.o NLescstr.o	\
	NLesctab.o NLfgetfile.o NLflatstr.o NLflattab.o NLscanf.o \
	NLisNLcp.o NLstrcat.o NLstrchr.o NLstrcmp.o NLstrcpy.o	\
	NLstrcspn.o NLstrdlen.o NLstrlen.o NLstrncat.o NLstrncmp.o	\
	NLstrncpy.o NLstrpbrk.o NLstrrchr.o NLstrspn.o NLstrtime.o	\
	NLstrtok.o NLtmtime.o NLunescstr.o NLunesctab.o NLxcol.o	\
	NLsprintf.o NLvfprintf.o NLvprintf.o NLvsprintf.o NLgetenv.o \
	NLctime.o NLfprintf.o NLprintf.o NLcatopen.o NLgetamsg.o \
	NLxio.o NLyesno.o NLcatgets.o _NCtolower.o _NCtoupper.o atojis.o \
	cjistosj.o cjistouj.o csjtojis.o csjtouj.o cujtojis.o	\
	cujtosj.o isj1kana.o isjalnum.o isjalpha.o isjdigit.o	\
	isjgraph.o isjhira.o isjis.o isjkanji.o isjkata.o	\
	isjlower.o isjparen.o isjprint.o isjpunct.o isjspace.o	\
	isjupper.o isjxdigit.o jctype_.o jistab.o jistoa.o	\
	jistosj.o jistouj.o kutentojis.o mbs.o mbsadvance.o	\
	mbsinvalid.o mbstomb.o mbswidth.o mbscat.o mbscmp.o	\
	mbscpy.o mbsncmp.o mbspbrk.o sjtojis.o sjtouj.o strncollen.o	\
	strtows.o tojhira.o tojkata.o tojlower.o tojupper.o	\
	toujis.o ujtojis.o ujtosj.o wstrcat.o wstrchr.o	\
	wstrcmp.o wstrcpy.o wstrcspn.o wstrdup.o wstrlen.o	\
	wstrncat.o wstrncpy.o wstrpbrk.o wstrrchr.o wstrspn.o	\
	wstrtok.o wstrtos.o finite.o logb.o scalb.o \
	llseek.o getproc.o getuser.o \
	fmtmsg.o getdate.o getsubopt.o realpath.o \
	${${TARGET_MACHINE}_SHARED_OFILES}
#
# If you haven't read the WARNING above, please do so before
# adding files to the SHARED_OFILES list.
#

#
# files specific to thread safe libc
#
LIBC_R_SHARED_OFILES	= \
	flockfile.o	\
	libc_data.o	\
	libc_locks.o	\
	getpwnamuid.o	\
	rec_mutex.o

#
# files that are only in libc.a
#
LIBC_SHARED_OFILES	=

# finite.o logb.o scalb.o are included in shr.o (but not exported)
# because they are required to frexpl.o etc.  The user sees the non-shared
# version below.

#
# all non shared files are common 
#
NOSHARED_OFILES=		\
        frexp.o itrunc.o ldexp.o modf.o logb.o scalb.o finite.o        \
        uitrunc.o _itrunc.o _uitrunc.o fstab.o move.o ptrgl.o		\
	getttyent.o getttynam.o moveeq.o fill.o strcat.o strcpy.o	\
	strcmp.o strncat.o strncpy.o	\
	${${TARGET_MACHINE}_NOSHARED_OFILES}

METHOD_OFILES=		\
	__method.o

.if !defined(PROFILED)
#
# This is only used when the non-profiled version of libc is built.
SYS_OFILES=		\
        a64l.o atoi_sys.o atoi_table.o l64a.o move.o ptrgl.o bcmp.o		\
	bzero.o memccpy.o memchr.o memcmp.o memset.o moveeq.o	\
	clc.o _uitrunc.o _itrunc.o \
	fill.o NCisshift_tmp.o strcat.o strchr.o strcmp.o	\
	strcpy.o strcspn.o strlen.o strncat.o strncmp.o	\
	strncpy.o strpbrk.o strrchr.o strspn.o strstr.o	strtok.o \
	${${TARGET_MACHINE}_SYS_OFILES}

ILIST_OTHERS	= ${CRT0} ${GCRT0} ${MCRT0} glink.o ${THREADS_INIT}
OTHERS		= ${ILIST_OTHERS} bmalloc.o
.endif

ether_addr.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
getgrent.o_CFLAGS	:= -DYP ${CFLAGS}
getpwent.o_CFLAGS	:= -DYP ${CFLAGS}
gethostent.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
getnetent.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
getrpcent.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
getservent.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
gprotoent.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
herror.o_CFLAGS		:= -D_BSD=44 -D_SUN ${CFLAGS}
innetgr.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
rcmd.o_CFLAGS		:= -D_BSD=44 -D_SUN ${CFLAGS}
res_comp.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
res_debug.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
res_init.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
res_mkqry.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
res_query.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
res_send.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
rexec.o_CFLAGS		:= -D_BSD=44 -D_SUN ${CFLAGS}
ruserpass.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
bresvport.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
gnetgrent.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}
strcasecmp.o_CFLAGS	:= -D_BSD=44 -D_SUN ${CFLAGS}

key_call.o_CFLAGS	:= -D_BSD ${CFLAGS}

maint_xdr.o_CFLAGS	:= -D_BSD ${CFLAGS}
yp_all.o_CFLAGS		:= -D_BSD ${CFLAGS}
yp_bind.o_CFLAGS	:= -D_BSD ${CFLAGS}
yp_enum.o_CFLAGS	:= -D_BSD ${CFLAGS}
yperr_str.o_CFLAGS	:= -D_BSD ${CFLAGS}
yp_master.o_CFLAGS	:= -D_BSD ${CFLAGS}
yp_match.o_CFLAGS	:= -D_BSD ${CFLAGS}
yp_order.o_CFLAGS	:= -D_BSD ${CFLAGS}
ypprot_err.o_CFLAGS	:= -D_BSD ${CFLAGS}
ypv1_xdr.o_CFLAGS	:= -D_BSD ${CFLAGS}
yp_update.o_CFLAGS	:= -D_BSD ${CFLAGS}
ypup_prot.o_CFLAGS	:= -D_BSD ${CFLAGS}
ypxdr.o_CFLAGS		:= -D_BSD ${CFLAGS}

doprnt.o_CFLAGS		:=  -qdebug=austlib ${CFLAGS}
doscan.o_CFLAGS		:=  -qdebug=austlib ${CFLAGS}
doprnt128.o_CFLAGS	:=  -qdebug=austlib -qldbl128 -D__LONGDOUBLE128 ${CFLAGS}
doscan128.o_CFLAGS	:=  -qdebug=austlib -qldbl128 -D__LONGDOUBLE128 ${CFLAGS}

# the following .o's are actually found in POWER/machdep.mk
# but if the following defines are found there profiling doesn't
# work right.

f128toi64rz.o_CFLAGS	:=  -qdebug=austlib -qldbl128 ${CFLAGS}
f128tou64rz.o_CFLAGS	:=  -qdebug=austlib -qldbl128 ${CFLAGS}
f64toi64rz.o_CFLAGS	:=  -qdebug=austlib ${CFLAGS}
f64tou64rz.o_CFLAGS	:=  -qdebug=austlib ${CFLAGS}
strtoll.o_CFLAGS	:=  -qdebug=austlib ${CFLAGS}
strtoull.o_CFLAGS	:=  -qdebug=austlib ${CFLAGS}
wcstoll.o_CFLAGS	:=  -qdebug=austlib ${CFLAGS}
wcstoull.o_CFLAGS	:=  -qdebug=austlib ${CFLAGS}
lldiv.o_CFLAGS		:=  -qdebug=austlib ${CFLAGS}
llabs.o_CFLAGS		:=  -qdebug=austlib ${CFLAGS}
strtold.o_CFLAGS	:= -qldbl128 -D__LONGDOUBLE128 ${CFLAGS}
_quitrunc.o_CFLAGS	:= -qldbl128 -D__LONGDOUBLE128 ${CFLAGS}
frexpl.o_CFLAGS		:= -qldbl128 -D__LONGDOUBLE128 ${CFLAGS}
ldexpl.o_CFLAGS		:= -qldbl128 -D__LONGDOUBLE128 ${CFLAGS}
modfl.o_CFLAGS		:= -qldbl128 -D__LONGDOUBLE128 ${CFLAGS}

.if defined(THREAD_SAFE)
shr_IMPORTS		= libpthreads.imp syscalls.exp libc_r.meth.exp libc_t.imp libdes_r.imp
shr_EXPORTS		= libpthreads.imp syscalls.exp libc_r.exp

meth_IMPORTS		= libc_r.exp libc_t.imp
meth_EXPORTS		= libc_r.meth.exp

aio_IMPORTS		= aio.exp syscalls.exp libc_r.exp libc_t.imp
.else
shr_IMPORTS		= syscalls.exp libc.meth.exp libdes.imp
shr_EXPORTS		= libc.exp syscalls.exp

meth_IMPORTS		= libc.exp
meth_EXPORTS		= libc.meth.exp

aio_IMPORTS		= aio.exp syscalls.exp libc.exp.min libc.exp.full
.endif

aio_EXPORTS		= libcaio.exp
aio_OFILES		= aio_posix.o
aio_posix.o_INCFLAGS	= -I${MAKETOP}/bos/kernext/aio

.include <${LIBC_PATH}/${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>

__LIBS__		= ${LIBS:!${FINDFILE} ${LIBDIRS} ${LIBS}!}
__LIB_S__		= ${LIBS:S/^lib/-l/g:S/.a$//g}

.if defined(THREAD_SAFE)
ALL_SHARED_OFILES	= ${SHARED_OFILES} ${LIBC_R_SHARED_OFILES}
.else
ALL_SHARED_OFILES	= ${SHARED_OFILES} ${LIBC_SHARED_OFILES}
.endif

${ALL_SHARED_OFILES}:	${HFILES}

${LIBC} : $${.TARGET}(${NOSHARED_OFILES} aio.o shr.o meth.o)
	${AR} ${DEF_ARFLAGS} ${.TARGET} ${.OODATE}

#
# only build libc.a min in the non profiled - non thread safe pass
#
.if !defined(PROFILED)
.if !defined(THREAD_SAFE)
libc.a.min : meth.o libc.exp.min ${__LIBS__} ${shr_IMPORTS} ${ALL_SHARED_OFILES}
	@${RM} -f ${.TARGET} min/shr.o min/meth.o min/shr.o.out
	@[ -d min ] || ${MKDIR} min
	${CP} ${meth.o:P} min
	${STRIP} min/meth.o
	${LD} -s -bM:Sre -o min/shr.o ${ALL_SHARED_OFILES} \
		${shr_IMPORTS:@TMP@-bI:${${TMP}:P}@} \
		${__LIB_S__} ${LIBDIRS} -blibpath:/usr/lib:/lib \
		-bE:${libc.exp.min:P} \
		-bE:${libc.meth.exp:P} \
		-bE:${syscalls.exp:P} \
		-bloadmap:min/shr.o.out -bR:min/shr.o.map \
		${LIBC_LDENV} ${LIBC_LDARGS} \
	    && ${RM} -f min/shr.o.map
	${AR} ${DEF_ARFLAGS} ${.TARGET} min/shr.o min/meth.o || ( ${RM} -f ${.TARGET}; false; )
.endif
.endif

LIBC_LDFLAGS	= -bnoentry -bloadmap:${.PREFIX}.out -bR:${.PREFIX}.map
_LIBC_LDFLAGS_	= ${LIBC_LDFLAGS} ${LIBC_LDENV} ${LIBC_LDARGS}

shr.o: ${ALL_SHARED_OFILES} ${shr_IMPORTS} ${shr_EXPORTS} ${__LIBS__}
	${RM} -f shr.out shr.map
	${LD} -bM:Sre -o ${.TARGET} \
		${ALL_SHARED_OFILES} \
		${shr_IMPORTS:@TMP@-bI:${${TMP}:P}@} \
		${__LIB_S__} ${LIBDIRS} \
		${shr_EXPORTS:@TMP@-bE:${${TMP}:P}@} \
		-blibpath:/usr/lib:/lib -bnoobjreorder ${_LIBC_LDFLAGS_}

meth.o: ${METHOD_OFILES} ${meth_IMPORTS} ${meth_EXPORTS} ${__LIBS__}
	${RM} -f meth.out meth.map
	${LD} -bM:Sre -o ${.TARGET} ${METHOD_OFILES} \
		${meth_IMPORTS:@TMP@-bI:${${TMP}:P}@} \
		${__LIB_S__} ${LIBDIRS} \
		${meth_EXPORTS:@TMP@-bE:${${TMP}:P}@} \
		-blibpath:/usr/lib:/lib ${_LIBC_LDFLAGS_}

aio.o:	${aio_OFILES} ${aio_IMPORTS} ${aio_EXPORTS}
	${RM} -f aio.out aio.map
	${LD} -bM:Sre -o ${.TARGET} ${aio_OFILES} \
		${aio_IMPORTS:@TMP@-bI:${${TMP}:P}@} \
		${__LIB_S__} ${LIBDIRS} \
		${aio_EXPORTS:@TMP@-bE:${${TMP}:P}@} \
		-blibpath:/usr/lib:/lib ${_LIBC_LDFLAGS_}

fshelp.o: fshelp_msg.h

#
# if building thread safe, create libc_t.exp,libc_r.meth.exp, libc_r.exp
#
.if defined(THREAD_SAFE)
libc_t.imp: libc_r.exp.only libc_t.imp.non_r
	$(ECHO) '#!libc_t.a(shr.o)' > ${.TARGET}
	$(SED) -n "/_r[ \t]*$$/s///p" ${libc_r.exp.only:P} \
		| $(CAT) - ${libc_t.imp.non_r:P} >> ${.TARGET}

libc_r.meth.exp: libc.meth.exp
	$(SED) 's/#!libc.a(meth.o)/#!libc_r.a(meth.o)/' ${libc.meth.exp:P} > ${.TARGET}

libc_r.exp: libc.exp libc_r.exp.only
	$(ECHO) '#!libc_r.a(shr.o)' | $(CAT) - ${libc.exp:P} \
		| $(SED) '/#!libc.a(shr.o)/d' 		     \
		| $(CAT) - ${libc_r.exp.only:P} > ${.TARGET}
	
#
# otherwise, build libc.exp
#
.else
libc.exp: libc.exp.min libc.exp.full
	${CAT} ${libc.exp.full:P} ${libc.exp.min:P} > ${.TARGET}
.endif

#
# The rest of the file does not need to be done in the profiling case
#
.if !defined(PROFILED)
#
# don't need libcsys.a in the thread safe case
#
.if !defined(THREAD_SAFE)
libcsys.a: $${.TARGET}(${SYS_OFILES})
	${AR} ${DEF_ARFLAGS} ${.TARGET} ${SYS_OFILES}
.endif

CRT0LDFLAGS = -r -bloadmap:${.PREFIX}.out -bR:${.PREFIX}.map
_CRT0LDFLAGS_ = ${CRT0LDFLAGS} ${CRT0LDENV} ${CRT0LDARGS}

${CRT0}: crt0main.o crt0msg.o ${THREADS_INIT}
	${RM} -f ${.TARGET:.o=.out} ${.TARGET:.o=.map}
	${LD} ${_CRT0LDFLAGS_} -o ${.TARGET} ${.ALLSRC}

${GCRT0}: pgcrt0.o __mcount.o __mcount_construct.o _stk.o crt0msg.o _getcall.o _getcall2.o ${THREADS_INIT}
	${RM} -f ${.TARGET:.o=.out} ${.TARGET:.o=.map}
	${LD} ${_CRT0LDFLAGS_} -o ${.TARGET} ${.ALLSRC}

${MCRT0}: pcrt0.o __mcount.o __mcount_construct.o _stk.o crt0msg.o _getcall.o _getcall2.o ${THREADS_INIT}
	${RM} -f ${.TARGET:.o=.out} ${.TARGET:.o=.map}
	${LD} ${_CRT0LDFLAGS_} -o ${.TARGET} ${.ALLSRC}

bmalloc.o: spec_malloc.o whatbucket.o
	${LD} -r spec_malloc.o whatbucket.o -o bmalloc.o 

.endif		#!PROFILED

