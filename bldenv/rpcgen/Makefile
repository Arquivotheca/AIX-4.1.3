# @(#)56       1.1  src/bldenv/rpcgen/Makefile, ade_build, bos412, GOLDA411a 3/17/94 20:00:19
# COMPONENT_NAME: BOSBUILD
# 
# FUNCTIONS: 
#
# ORIGINS: 27 
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 

VPATH		= ${MAKETOP}/nfs/usr/bin/rpcgen:${MAKETOP}/nfs/msghdr

PROGRAMS	= rpcgen

MSGHDRS		= cmdnfs_msg.h
HFILES		= ${MSGHDRS}

OFILES 		= rpc_cltout.o rpc_cout.o rpc_hout.o rpc_main.o	rpc_parse.o \
		  rpc_scan.o rpc_svcout.o rpc_util.o

CFLAGS  	=  -D_BSD=44 -D_SUN -D_IBMRPC_

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

.include <${RULES_MK}>

