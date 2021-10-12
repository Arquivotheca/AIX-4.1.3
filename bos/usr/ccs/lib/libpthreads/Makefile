#@(#)03	1.12  src/bos/usr/ccs/lib/libpthreads/Makefile, libpth, bos41J, 9513A_all 3/27/95 09:56:52
#
# COMPONENT_NAME:a libpth
#
# FUNCTIONS:
#
# ORIGINS: 71, 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		= ${TARGET_MACHINE}:${MAKETOP}/bos/kernel/exp
SHARED_LIBRARIES	= libpthreads.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

pthread.h_IDIR	= /usr/include/
EXPLIB_TARGETS  = export_libpthreads.a
INCLUDES	= pthread.h

ILIST           = libpthreads.a pthread.h
IDIR            = /usr/ccs/lib/
IMODE           = 644
LIBS		= libc_r.a
IMPORTS		= -bI:syscalls.exp -bI:threads.exp
EXPORTS		= -bE:libpthreads.exp

SHARED_OFILES		= \
	cancel.o cond.o evt.o fork.o \
	lib_lock.o mutex.o \
	pthread.o pthread_attr.o sched.o \
	signal.o specific.o spin.o stack.o \
	vp.o malloc.o \
	${${TARGET_MACHINE}_SHARED_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>
.include <${RULES_MK}>

