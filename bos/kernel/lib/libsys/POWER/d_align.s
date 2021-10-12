# @(#)76        1.3  src/bos/kernel/lib/libsys/POWER/d_align.s, libsys, bos411, 9428A410j 4/30/93 10:29:20
#
#  COMPONENT_NAME: (SYSIOS) IO subsystem
# 
#  FUNCTIONS: d_align
# 
#  ORIGINS: 27
# 
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1993
#  All Rights Reserved
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# FUNCTION: 
#	    eg.	x = D_ALIGN; 
#               y = D_ROUNDUP( length );
#		ptr = palloc( x,y );	
# 
#######################################################################


# Get the alignment value for a cache line. 
#
# usage: align = D_ALIGN;


        S_PROLOG(d_align)
	LTOC(3,_system_configuration,data) 	# get address of sys config
	l	r3,scfg_dcb(r3)			# load data cache line size
	cntlz	r3,r3				# returns log base2 of
	sfi	r3,r3,0x1f			# cache line size
        S_EPILOG

	.toc
        TOCE(_system_configuration, data)

include(INCLML/systemcfg.m4)

