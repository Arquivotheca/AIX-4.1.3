# @(#)06 1.2 src/bos/usr/lib/pios/fmtrs/Makefile, cmdpios, bos411, 9428A410j 6/23/94 10:33:53
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH		= ..

PROGRAMS	= piof420x piof5202 piof42x4 piof2000 piof5152 piof630 \
		  piof855 piofpcl piofpt piofasci piofbull piofbull5
MSGHDRS		= 
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS}
IDIR		= /usr/lib/lpd/pio/fmtrs/

INCFLAGS	= -I.. -I../msg

piof420x_OFILES		= piof420x.o piotrans.o
piof420x_IMPORTS	= -bI:piof.imps
piof420x_EXPORTS	= -bE:pioformat.imps
piof420x_ENTRYPOINT	= setup

piof5152_OFILES		= piof5152.o piotrans.o
piof5152_IMPORTS	= -bI:piof.imps
piof5152_EXPORTS	= -bE:pioformat.imps
piof5152_ENTRYPOINT	= setup

piof5202_OFILES		= piof5202.o piotrans.o
piof5202_IMPORTS	= -bI:piof.imps
piof5202_EXPORTS	= -bE:pioformat.imps
piof5202_ENTRYPOINT	= setup

piof42x4_OFILES		= piof42x4.o piotrans.o
piof42x4_IMPORTS	= -bI:piof.imps
piof42x4_EXPORTS	= -bE:pioformat.imps
piof42x4_ENTRYPOINT	= setup

piof2000_OFILES		= piof2000.o piotrans.o
piof2000_IMPORTS	= -bI:piof.imps
piof2000_EXPORTS	= -bE:pioformat.imps
piof2000_ENTRYPOINT	= setup

piof630_OFILES		= piof630.o piotrans.o
piof630_IMPORTS		= -bI:piof.imps
piof630_EXPORTS		= -bE:pioformat.imps
piof630_ENTRYPOINT	= setup

piof855_OFILES		= piof855.o piotrans.o
piof855_IMPORTS		= -bI:piof.imps
piof855_EXPORTS		= -bE:pioformat.imps
piof855_ENTRYPOINT	= setup

piofpcl_OFILES		= piofpcl.o piotrans.o
piofpcl_IMPORTS		= -bI:piof.imps
piofpcl_EXPORTS		= -bE:pioformat.imps
piofpcl_ENTRYPOINT	= setup

piofpt_OFILES		= piofpt.o piotrans.o
piofpt_IMPORTS		= -bI:piof.imps
piofpt_EXPORTS		= -bE:pioformat.imps
piofpt_ENTRYPOINT	= setup

piofasci_OFILES		= piofasci.o piotrans.o
piofasci_IMPORTS	= -bI:piof.imps
piofasci_EXPORTS	= -bE:pioformat.imps
piofasci_ENTRYPOINT	= setup

piofbull_OFILES         = piofbull.o piotrans.o
piofbull_IMPORTS        = -bI:piof.imps
piofbull_EXPORTS        = -bE:pioformat.imps
piofbull_ENTRYPOINT     = setup

piofbull5_OFILES        = piofbull5.o piotrans.o
piofbull5_IMPORTS       = -bI:piof.imps
piofbull5_EXPORTS       = -bE:pioformat.imps
piofbull5_ENTRYPOINT    = setup

LIBS		= -lIN -lqb

.include <${RULES_MK}>
