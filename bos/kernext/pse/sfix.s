# @(#)96	1.1  src/bos/kernext/pse/sfix.s, sysxpse, bos411, 9440A411d 10/5/94 16:07:02
# COMPONENT_NAME: (sysxpse) System Extension for Streams support
#
# FUNCTIONS: new_stack, rest_stack
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# r3 addr of new stack
# r4 size in bytes of new stack

S_PROLOG(new_stack)
	a	r0, r3, r4
	ai	r3, r0, -0xC
	st	r1, 0(r3)
	oril	r1, r3, 0
	stu	r1, -0x50(r1)
	br
S_EPILOG
FCNDES(new_stack)

S_PROLOG(rest_stack)
	ai	r3, r1, 0x50
	l	r1, 0(r3)
	br
S_EPILOG
FCNDES(rest_stack)
