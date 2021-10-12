# @(#)13        1.4  src/bos/usr/lib/nls/loc/uconv/Makefile, cmdiconv, bos41J, 9509A_all 2/19/95 23:27:10
#
#   COMPONENT_NAME: LIBICONV
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# CC_OPT_LEVEL	= -g

PROGRAMS	= UCSTBL UTF-8 ct_UCS UCS_ct

ILIST		= ${PROGRAMS} README_iso2022.cfg
IDIR		= /usr/lib/nls/loc/uconv/

UCSTBL_OFILES	= UCSTBL.o get_modifier.o uc_conv.o uc_aix.o
UTF-8_OFILES	= UTF-8.o  get_modifier.o
ct_UCS_OFILES	= ct_UCS.o ctInit.o get_modifier.o ucs_utf.o
UCS_ct_OFILES	= UCS_ct.o ctInit.o get_modifier.o ucs_utf.o

INCFLAGS	= -I${MAKETOP}/bos/usr/include
CFLAGS		= -DAIXV3AR
LDFLAGS		= -einstantiate
LIBS            = -liconv

.include <${RULES_MK}>


