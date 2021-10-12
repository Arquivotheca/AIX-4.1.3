# @(#)28        1.3  src/bos/usr/lib/dwm/Makefile, Makefiles, bos411, 9428A410j 1/12/93 13:28:48
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
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

MSGHDRS		= cmddwm_msg.h
CATFILES	= cmddwm.cat
OTHERS		= ${MSGHDRS}
SCRIPTS         = dwm.fs.awk dwm_defaults dwm_errors dwm_functions \
		  dwm_platform lsdclient rmdclient mkdclient

ILIST		= chd.bootptab.awk chd.exp.awk chd.expfs.awk \
		  chd.fs.awk chd.hosts.awk chdclient dumpcheck \
		  dwm.bootptab.awk dwm.chexports.awk dwm.cutfs.awk \
		  dwm.etcfb.awk dwm.fs.awk dwm.getfs.awk dwm.hosts.awk \
		  dwm.ip.awk dwm.rmhosts.awk dwm_common dwm_defaults \
		  dwm_errors dwm_functions dwm_platform \
		  lsd.spot.awk lsdclient lss.long.awk lss.max.awk \
		  lsspot mkd.bootptab.awk mkd.env.awk mkd.names.awk \
		  mkd.swap.awk mkdataless mkdclient mkspot \
		  rmd.bootptab.awk rmd.swap.awk rmdclient rmspot \
		  xlpp xlpp.toc.awk tftpd.c tftpsubs.c tftpd.mak \
		  daemon.mak

IDIR		= /usr/lib/dwm/
tftpd.c_IDIR	= /usr/lib/dwm/tftp/
tftpsubs.c_IDIR	= /usr/lib/dwm/tftp/
tftpd.mak_IDIR	= /usr/lib/dwm/tftp/


.include <${RULES_MK}>
