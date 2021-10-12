# @(#)53 1.9 src/bos/usr/lib/methods/cfgtty/Makefile, cfgtty, bos41J, 9520A_all 4/27/95 14:22:33
#
# COMPONENT_NAME: (CFGMETHODS)
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH       = ../common:./exp:./rsdd:./cx:./lion
VPATH      := ${VPATH}:${MAKETOP}/rspc/usr/lib/methods/cfgtty

PROGRAMS    = cfgtty rsddload isaload cxload lionload


cfgtty_OFILES    = cfgtoolsx.o\
                   cfgtty.o\
                   commondds.o\
                   ldtermdds.o\
                   nlsdds.o\
                   pop_push.o\
                   set_inittab.o\
                   set_termios.o\
                   set_termiox.o\
                   sptrdds.o\
                   tiocdds.o\
                   utility.o

rsddload_OFILES		= rslinedds.o\
			  rsddutil.o\
                          commondds.o\
			  cfgtoolsx.o

cxload_OFILES		= cxmalinedds.o\
			  cxutil.o\
			  cfgtoolsx.o

isaload_OFILES		= sfdds.o\
			  sfutil.o\
			  cfgtoolsx.o

lionload_OFILES		= lionlinedds.o\
                          lionutil.o\
                          commondds.o\
			  cfgtoolsx.o
                 

cfgtty_IMPORTS		= -bI:module.exps
cfgtty_EXPORTS		= -bE:module.imps

rsddload_EXPORTS	= -bE:module.exps
rsddload_IMPORTS	= -bI:module.imps

isaload_EXPORTS		= -bE:module.exps
isaload_IMPORTS		= -bI:module.imps

cxload_EXPORTS		= -bE:module.exps
cxload_IMPORTS		= -bI:module.imps

lionload_EXPORTS	= -bE:module.exps
lionload_IMPORTS	= -bI:module.imps

INCFLAGS    = -I../common\
		-I${MAKETOP}/bos/kernext/isa_async\
		-I${MAKETOP}/bos/kernext/isa_async/${TARGET_MACHINE}\
		-I${MAKETOP}/bos/kernext/tty\
		-I${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}\
		-I${MAKETOP}rspc/kernext/isa/tty

cfgtty_LINTFILES     = cfgtty.c\
                       commondds.c\
                       ldtermdds.c\
                       nlsdds.c\
                       pop_push.c\
                       set_inittab.c\
                       set_termios.c\
                       set_termiox.c\
                       sptrdds.c\
                       tiocdds.c\
                       utility.c

LINTFLAGS               = -wk -u -x

ILIST			= ${PROGRAMS}

IDIR			= /usr/lib/methods/

LIBS			= -ls -lodm -lcfg -lrts -lPW -lqb

.include <${RULES_MK}>
