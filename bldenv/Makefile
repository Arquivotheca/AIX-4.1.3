# @(#)24        1.65  src/bldenv/Makefile, ade_build, bos41J, 9513A_all 3/23/95 09:50:19
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
#        DEVELOPERS: If you need to add a tool to the build env.
#        and you do not understand this Makefile, or you would like
#        to have your work checked, send a note to 41blders@landru
#

SUBDIRS		=
EXPINC_SUBDIRS	=
EXPLIB_SUBDIRS	=
ALL_TOOLS	= yes

.if defined(BOS_TOOLS)  || defined(CMDTEXT_TOOLS) || \
    defined(INED_TOOLS) || defined(GOS_TOOLS) || defined (CDE_TOOLS)
.undef ALL_TOOLS
.endif

#-----------------------------------------------------------------------------
# B O S   T O O L S
#-----------------------------------------------------------------------------
#
# Build bos tools
#
.if defined(BOS_TOOLS) || defined(ALL_TOOLS)

.if defined(IN_SANDBOX)
#
# Order dependent tools.  Any tool which are order dependent (i.e. is
# needed in the build environment by other build environment tools)
# should be added to this section and setup.sh appropriately.  setup.sh
# directly walks the SUBDIRS listed below and the ".if defined(IN_SANDBOX)"
# prevents setup.sh from walking the SUBDIRS twice and tells  to
# walk the order dependent tools in a sandbox.
#
SUBDIRS		+=  makepath md genpath findfile \
		   mkcatdefs gencat libc libc_r ld rm mv cp \
		   mkdir echo cat chmod date test basename \
		   false true tr grep egrep fgrep ed sed ar tar \
		   expr touch cut cmp yacc lex awk as m4 \
		   what wc xargs libodm odmcreate odmadd odmget \
		   odmchange brand genxlt ls 

EXPINC_SUBDIRS	+= includes
.endif

SUBDIRS		+= bdftopcf bdftosnf bldtools compress \
		   cpio ctags dd dspcat dspmsg du dump errlg getopt \
		   hostname iconv keycomp libswvpd libIN lint locdef man meth \
	  	   mkboot mkfs mkram tic paste piosk pkgtools \
		   rastools sbtools sort strip strfile tail uconvdef uniq \
		   v3fshelpers pios inutoc loc lockname locales libc_r \
		   libg

#.if exists(${MAKETOP}hcon/usr/lib/nls/msg/whpmri/makemsgs.c)
#SUBDIRS		+= makemsgs hcon
#.endif

.if exists(${MAKETOP}bos/usr/bin/panel20/makemsgs.c)
SUBDIRS		+= p20makemsgs
.endif

EXPINC_SUBDIRS	+= libodm errlg lex locdef sbtools uconvdef v3fshelpers xlC
EXPLIB_SUBDIRS	+= libodm errlg libswvpd sbtools v3fshelpers libiconv libIN
.endif

#-----------------------------------------------------------------------------
# C M D T E X T   T O O L S
#-----------------------------------------------------------------------------
#
# Build cmdtext tool
#
.if defined(CMDTEXT_TOOLS) || defined(ALL_TOOLS)

.if exists(${MAKETOP}cmdtext/usr/bin/makedev/makedev.c)
SUBDIRS		+= cmdtext
.endif

.endif

#-----------------------------------------------------------------------------
# I N E D   T O O L S
#-----------------------------------------------------------------------------
#
# Build tenplus tools
#
.if defined(INED_TOOLS) || defined(ALL_TOOLS)

.if exists(${MAKETOP}tenplus/sf_progs/tosf.c)
SUBDIRS		+= tosf
.endif

.if exists(${MAKETOP}tenplus/sf_progs/mri2sf.c)
SUBDIRS		+= mri2sf
.endif

.if exists(${MAKETOP}tenplus/keys/keys.c)
SUBDIRS		+= keys
.endif

.if exists(${MAKETOP}tenplus/sf_progs/scat.c)
SUBDIRS		+= scat
.endif

.if exists(${MAKETOP}tenplus/tools/fc/fc.c)
SUBDIRS		+= fc
.endif

.if exists(${MAKETOP}tenplus/util/tdigest.c)
SUBDIRS		+= tdigest
.endif

.endif

#-----------------------------------------------------------------------------
# G O S   T O O L S
#-----------------------------------------------------------------------------
#
# Build gos tools
#
.if defined(GOS_TOOLS) || defined(ALL_TOOLS)

# rpcgen needed by cose

.if exists(${MAKETOP}nfs/usr/bin/rpcgen/Makefile)
SUBDIRS		+= rpcgen
.endif

.if exists(${MAKETOP}gos/2d/XTOP/fonts/clients/mkfontdir/mkfontdir.c)
SUBDIRS       	+= gos
EXPINC_SUBDIRS	+= gos
EXPLIB_SUBDIRS	+= gos
.endif

.endif

#-----------------------------------------------------------------------------
# C D E   T O O L S
#-----------------------------------------------------------------------------
#
# Build  tools
#
.if defined(CDE_TOOLS) || defined(ALL_TOOLS)
SUBDIRS         += 
EXPINC_SUBDIRS  += 
EXPLIB_SUBDIRS  += 
.endif
#-----------------------------------------------------------------------------

.include <${RULES_MK}>
