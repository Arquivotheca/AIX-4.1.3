# @(#)74        1.2  src/bldenv/gos/desktop/imageutil/Makefile, desktop, bos412, GOLDA411a 6/3/94 09:54:29
#
#   COMPONENT_NAME: ade_build
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH = ${MAKETOP}/gos/desktop/doc/cde1/LearningProducts/tools/imageutil:${MAKETOP}/gos/desktop/lib1/il:${MAKETOP}/gos/2d/XTOP/lib/X

PROGRAMS 	= imageutil

LIBRARIES 	= libilefs.a libil.a
EXPLIB_TARGETS	= export_libilefs.a export_libil.a

INCFLAGS 	= -I${MAKETOP}/gos/desktop/doc/cde1/LearningProducts/tools/imageutil \
		  -I${MAKETOP}/gos/desktop/lib1 -I${MAKETOP}/gos/desktop/lib1/il

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix \
		  -D_aix -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

imageutil_OFILES =  ilu.o imageutil.o ilucheck.o ilulist.o

imageutil_LIBS 	= -lilefs -lil -lX11 -lm 

ILIST 		= imageutil
IDIR 		= /opt/dt/etc/
EXPDIR		= /usr/ccs/lib/

libilefs.a_OFILES = efsgif.o efsinit.o efsjfif.o efsstarbm.o efstiff.o \
			efsxbm.o efsxpm.o efsxpmcreate.o efsxpmdata.o \
			efsxpmmisc.o efsxpmparse.o efsxpmparse_color.o \
			efsxwd.o
 
libil.a_OFILES	= ilX.o ilbigray.o ilcodec.o ilcompg3g4.o ilcomplzw.o \
		        ilcomppackbits.o ilcompress.o ilcompressjpeg.o \
		        ilcontext.o ilconvert.o ilcrop.o ildata.o ildecompg3.o \
		        ildecompg4.o ildecompjpeg.o ildecomplzw.o \
		        ildecomppackbits.o ildecompress.o ildither.o ilefs.o \
		        ilformat.o ilgraybi.o ilimage.o ilinvert.o \
		        iljpgdecode.o iljpgdecodejif.o iljpgdedct.o \
		        iljpgdehuff.o iljpgencode.o iljpgencodedata.o \
		        iljpgencodejif.o iljpgendct.o iljpgenhuff.o \
		        iljpgutil.o ilmap.o ilmirror.o ilobject.o ilpalette.o \
		        ilpipe.o ilreadX.o ilreadimage.o ilrotate.o \
		        ilrwimage.o ilscale.o ilscalesample.o iltapfeed.o \
			iltiff.o iltiffread.o iltiffwrite.o iltruegray.o \
		        ilupsample.o ilutiljpeg.o ilversion.o ilwriteimage.o \
		        ilycbcr.o

.include <${RULES_MK}>

