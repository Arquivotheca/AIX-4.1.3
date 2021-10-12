# @(#)71        1.14  src/bos/usr/lib/boot/Makefile, bosboot, bos411, 9428A410j 5/10/94 10:40:30
# COMPONENT_NAME: BOSBOOT
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS		= bootutil chramfs getrootfs mergedev network \
		protoext restbase srvboot

SCRIPTS		= rs6k.cd.proto rs6k.disk.proto rs6k.tape.proto update_proto \
		  rs6ksmp.cd.proto rs6ksmp.disk.proto rs6ksmp.tape.proto 

PROGRAMS	= ssh

ILIST		= ssh rs6k.cd.proto rs6k.disk.proto rs6k.tape.proto update_proto \
		  rs6ksmp.cd.proto rs6ksmp.disk.proto rs6ksmp.tape.proto 

IDIR		= /usr/lib/boot/

INCFLAGS	=-I${MAKETOP}/bos/kernel

.include <${RULES_MK}>
