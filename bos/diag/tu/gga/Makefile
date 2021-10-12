#
#   COMPONENT_NAME: tu_gla
#
#   FUNCTIONS: none
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

VPATH = ${MAKETOP}/bos/kernext/exp

LIBRARIES = libtu_gga.a

INCFLAGS  = -I${MAKETOP}/rspc/kernext/disp/pcigga/inc  \
	    -I${MAKETOP}/bos/kernext/disp/inc

EXPLIB_TARGETS  = export_libtu_gga.a
EXPDIR          = /usr/lib/

IMPORTS = -bI:rcm.exp

#OPT_LEVEL = -g

#STRIP_FLAG =
#CFLAGS = ${RS2_601} -D_POWER -D_BSD -qdbxextra -DLOGMSGS -DALL_TUS

OFILES = \
	 sigmom.o \
	 advtu.o \
	 blittu.o \
	 colorbartu.o \
	 copystr.o \
	 crosstu.o \
	 cursortu.o \
	 drawcmds.o \
	 exectu.o \
	 fastcpytu.o \
	 ggaodm.o \
	 ggamisc.o \
	 i2c.o \
	 luminancetu.o \
	 meminfo.o \
	 modeset.o \
	 moveit.o \
	 palettetu.o \
	 param.o \
	 pixelcmdstu.o \
	 regtu.o \
	 rgbtu.o \
	 scrolltu.o \
	 scroll2tu.o \
	 sqrboxtu.o \
	 strtesttu.o \
	 util.o \
	 videotu.o \
	 vramtu.o

.include <${RULES_MK}>
