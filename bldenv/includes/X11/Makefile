# @(#)81        1.2  src/bldenv/includes/X11/Makefile, ade_build, bos412, GOLDA411a 1/10/94 13:47:27
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
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH	= ${MAKETOP}bos/usr/include/X11:${MAKETOP}gos/2d/XTOP/include:${MAKETOP}gos/2d/XTOP/lib/Xt:${MAKETOP}gos/2d/XTOP/lib/X

EXPDIR	= /usr/include/X11/

DATAFILES	= \
	keysym.h \
	aix_keysym.h \
	aix_ks_names.h \
	ks_names.h \
	keysymdef.h \
	Intrinsic.h \
	Xatom.h \
	Constraint.h \
	ConstrainP.h \
	Object.h \
	ObjectP.h \
	RectObj.h \
	RectObjP.h \
	Core.h \
	CoreP.h \
	Composite.h \
	CompositeP.h \
	StringDefs.h \
	Shell.h \
	ShellP.h \
	Xresource.h \
	VendorP.h \
	Vendor.h \
	IntrinsicP.h
	

INCLUDES	= ${DATAFILES}

.include <${RULES_MK}>

ks_names.h : keysymdef.h
	${RM} ${_RMFLAGS_} ${.TARGET}
	${AWK} 'BEGIN {							\
	       printf("/*\n * This is a machine generated file.");	\
	       printf("	* Do not put under source control. \n */\n");  \
	};								\
	/^#define/ {							\
		len = length($$2)-3;					\
		printf("{ \"%s\", %s },\n", substr($$2,4,len), $$3);	\
	}' <${keysymdef.h:P} >>$@

aix_ks_names.h : aix_keysym.h
	${RM} ${_RMFLAGS_} ${.TARGET}
	${AWK} 'BEGIN {							\
		printf("/*\n * This is a machine generated file.");	\
		printf(" * Do not put under source control. \n */\n");	\
	};								\
	/^#define/ {							\
		len = length($$2)-3;					\
		printf("{ \"%s\", %s },\n", substr($$2,4,len), $$3);	\
	}' <${aix_keysym.h:P} | ${GREP} -v "keysym_h" >>$@
