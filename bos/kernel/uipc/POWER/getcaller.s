# @(#)39	1.5  src/bos/kernel/uipc/POWER/getcaller.s, sysuipc, bos411, 9428A410j 6/15/94 17:02:54
# 
# COMPONENT_NAME: (SYSUIPC) Socket services
# 
# FUNCTIONS: getcaller, getcaller2
#
# ORIGINS: 27 
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1991 
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 

	.file   "getcaller.s"

##############################################################################
#
# NAME : getcaller
#
#	Returns address of caller.	
#
##############################################################################
        include(sys/comlink.m4)
        FCNDES(getcaller)

ENTRY(getcaller):

        S_PROLOG(getcaller)
        l       r3,stkback(stk) # back to previous stack frame
        l       r3,stklink(r3)  # get callers return address
        br                      # return
	_DF(_DF_NOFRAME)


##############################################################################
#
# NAME : getcaller2
#
#	Returns address of caller's caller.
#
##############################################################################
        FCNDES(getcaller2)

ENTRY(getcaller2):

        S_PROLOG(getcaller2)
        l       r3,stkback(stk) # back to previous stack frame
        l       r3,stkback(r3)  # back to previous stack frame
        l       r3,stklink(r3)  # get caller's return address
        br                      # return
	_DF(_DF_NOFRAME)

##############################################################################
#
# NAME : getcaller3
#
#	Returns address of caller's caller.
#
##############################################################################
        FCNDES(getcaller3)

ENTRY(getcaller3):

        S_PROLOG(getcaller3)
        l       r3,stkback(stk) # back to previous stack frame
        l       r3,stkback(r3)  # back to previous stack frame
        l       r3,stkback(r3)  # back to previous stack frame
        l       r3,stklink(r3)  # get caller's return address
        br                      # return
	_DF(_DF_NOFRAME)
