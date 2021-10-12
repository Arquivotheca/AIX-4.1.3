# @(#)77        1.3  src/bos/kernel/lib/libsys/POWER/d_roundup.s, libsys, bos411, 9428A410j 4/30/93 10:29:56
#
#  COMPONENT_NAME: (SYSIOS) IO subsystem
# 
#  FUNCTIONS: d_roundup
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


#
# Round the specified value up to an integer number of cache lines.
#
# usage:length = D_ROUNDUP (length);
#	
# length = (length + (cls(max)-1)) & (~(cls(max) - 1));
#
        S_PROLOG(d_roundup)
	LTOC(r4, _system_configuration, data) # get address of sysconfig struct
	l	r4,scfg_dcb(r4)		      # load data cache line size
	neg	r5,r4			      # save negated cache line size
	ai	r4,r4,-1		      # (cache_line_size - 1)
	a	r3,r3,r4		      # (cache_line_size - 1) + length
	and	r3,r3,r5		      # mask off to cache line bound
        S_EPILOG

	.toc
        TOCE(_system_configuration, data)

include(INCLML/systemcfg.m4)

