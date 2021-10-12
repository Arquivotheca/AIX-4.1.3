# @(#)03        1.1  src/bos/usr/lpp/bos/migrate/Makefile, cmdsmit, bos411, 9428A410j 4/1/94 23:38:48
#-----------------------------------------------------------------------
#
# COMPONENT_NAME: CMDSMIT
#
# FUNCTIONS: Makefile (merge_smit_db)
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------------

PROGRAMS        = merge_smit_db
MSGHDRS         = 
CATFILES        =
HFILES          = ${MSGHDRS} 

ILIST           = ${PROGRAMS}

IDIR            = /usr/lpp/bos/migrate/

INCFLAGS        = -I${MAKETOP}/bos/usr/bin/smit

OFILES            = merge_smit_db.o

LIBS=  -lodm -lc 

.include <${RULES_MK}>
