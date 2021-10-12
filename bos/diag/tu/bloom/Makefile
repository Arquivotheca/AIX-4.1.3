# @(#)44        1.2  src/bos/diag/tu/bloom/Makefile, tu_bloom, bos41J, 9514A_all 4/3/95 13:50:36
#
#   COMPONENT_NAME: TU_BLOOM
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS         = slih
EXPINC_SUBDIRS  = slih
EXPLIB_SUBDIRS  = slih

VPATH      = ${MAKETOP}/bos/kernext/exp:${MAKETOP}/bos/usr/lib/methods/common

LIBRARIES	= libbloomtu.a
EXPLIB_TARGETS	= export_libbloomtu.a
EXPDIR		= /usr/lib/

OFILES     = bloomtu.o cfgtools.o 

CFLAGS	     	= -qcpluscmt 
#CC_OPT_LEVEL 	= -g -DDEBUG -DCFGDEBUG -qsource -qlist
#CC_OPT_LEVEL 	= -g -DDEBUG -qsource -qlist
#STRIP_FLAG 	= 
INCFLAGS   	+= -I${MAKETOP}/bos/usr/lib/methods/cfgbus \
	      	-I${MAKETOP}/rspc/kernext/pci/ncr810           

IMPORTS  	= -bI:diagex.exp

.include <${RULES_MK}>
