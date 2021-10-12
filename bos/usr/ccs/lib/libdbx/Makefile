# @(#)97        1.16  src/bos/usr/ccs/lib/libdbx/Makefile, libdbx, bos41J, 9523A_all 5/31/95 17:16:49
# COMPONENT_NAME: LIBDBX
#
# FUNCTIONS:
#
# ORIGINS: 10,27,83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#


VPATH		= ${TARGET_MACHINE}
INCFLAGS	= -I${TARGET_MACHINE} \
                  -I${MAKETOP}/bos/usr/ccs/lib/libpthreads \

SHARED_LIBRARIES	= libdbx.a
MSGHDRS         =dbx_msg.h
CATFILES        =dbx.cat
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libdbx.a

ILIST           =libdbx.a
IDIR            = /usr/ccs/lib/

LIBS=libdemangle.a libPW.a libld.a libm.a libc.a
EXPORTS= -bE:libdbx.exp

YFLAGS= -d -l
CFLAGS= -DAIXV3AR -DYACC_MSG -DK_THREADS -DLONGLONGTYPE
LDFLAGS=-s

stabstring_CFLAGS= -qnoro
printsym_CFLAGS= -qnoro

SHARED_OFILES= \
    commands.o asm.o c.o check.o cobol.o cplusplus.o coredump.o \
    debug.o dpi_command.o dpi_events.o dpi_file.o dpi_init.o \
    dpi_ldinfo.o dpi_location.o dpi_memory.o dpi_message.o \
    dpi_stack.o dpi_token.o dpi_value.o dpi_var.o eval.o \
    events.o examine.o execute.o fortran.o help.o keywords.o \
    languages.o library.o lists.o mappings.o main.o \
    names.o object.o operators.o overload.o \
    pascal.o printsym.o process.o qbridge.o resolve.o \
    runtime.o scanner.o source.o stabstring.o \
    k_thread.o cma_thread.o \
    symbols.o tree.o \
    ${${TARGET_MACHINE}_SHARED_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>
.include <${RULES_MK}>

${SHARED_OFILES}: dbx_msg.h

dpi_command.o: commands.h

commands.h:  commands.y
