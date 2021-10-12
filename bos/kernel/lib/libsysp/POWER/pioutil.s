# @(#)22	1.2  src/bos/kernel/lib/libsysp/POWER/pioutil.s, libsysp, bos411, 9428A410j 6/15/90 18:01:05
#
# COMPONENT_NAME: (SYSIOS) IO Subsystem
#
# FUNCTIONS: BusCpy, BusGetLR, BusPutLR, BusGetSR, BusPutSR, BusXchgC
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# BusCpy : Indexed string movement to and from Bus I/O and Memory
#
#	Parameters			Local Variables
#	r3 : Destination		r6 : Current Count, 1 - 64 bytes
#	r4 : Source			r7 : Cumulative count, bytes
#	r5 : Count (in bytes)		r16 - r31 : Data storage area

	S_PROLOG(BusCpy)
	or	r7,r4,r4		# Place r4 as alignment address
	cmpi	cr0,r5,0		# Called with 0 count?
	bne	bus_str			# Invoke common code w/o return link
	br				# Return via link register
bus_str:
	stm	r16,-144-64(r1)		# Save registers. (r16->r31)

	xor	r7,r7,r7		# Clear cumulative count
	cal	r6,64(0)		# Set count r6 = 0x40 = 64 bytes
	mtspr	xer,r6			# Set XER(25-31) = r6
c_loop:
	cmpi	cr0,r5,64		# More than 64 bytes left?
	bge	c_ok			# Branch if at least 64
					# Set up final move count
	or	r6,r5,r5		# Use remaining byte count
	mtspr	xer,r6			# Store the XER Register
c_ok:
	lsx	r16,r7,r4		# Dest (r16->r31), from r7(r4)
	stsx	r16,r7,r3		# Source (r16->r31), to r7(r3)

	a	r7,r6,r7		# Increment source/dest. memory index
	sf.	r5,r6,r5		# Decrement remaining count.  Zero?
	bne	c_loop			# Another iteration if nonzero

	lm	r16,-144-64(r1)		# Restore pertinent registers.
	S_EPILOG(BusCpy)
	FCNDES(BusCpy)

# Perform byte reversed access to the bus.  Notice that the CPU itself
# supports byte reversed operations in single instructions, however these
# presently generate exeptions when used against addresses utilizing I/O
# segment registers.  The first line of each routine, commented out, gives
# the syntax of the routine when this facility becomes available.

# Since operations to Bus I/O and Memory may be sensitive to the size of
# operation with which they are retrieved, these routines always access
# the bus with the full operand size, performing byte reversal with
# register to register manipulation.  This being the case, unaligned
# halfword (short) and word (long) loads and stores will generate the
# (expected) alignment exceptions.  If the access to the physical bus
# requires dynamic bus sizing, this is transparent to this code.

# Parameters :
#        r3  :  All cases, the Bus Address for the requested operation
#        r4  :  Store (Put) cases only, the data value.  On halfword (short)
#               operations, the low bits (16-31) are used.
# Return Values :
#        r3  :  Load (Get) cases only, the data value obtained from the bus
#               and reversed as applicable.  On halfword (short) operations,
#               the low bits (16-31) are used.
# Temporary Storage :
#        r12 :  General Purpose Register 12 (0xC) is destroyed is all cases.

# Get/Put a full word (32 bit) value from/to the bus.

	S_PROLOG(BusGetLR)
#	lbrx	r3,0,r3			# Load from (r3) into r3
	l	r12,0(r3)		# Get the four byte value
	rlimi	r3,r12,24,0,7		# high byte to low
	rlimi	r3,r12,8,8,15		# switch middle bytes
	rlimi	r3,r12,24,16,23		# switch middle bytes
	rlimi	r3,r12,8,24,31		# high byte to low
	S_EPILOG(BusGetLR)
	FCNDES(BusGetLR)

	S_PROLOG(BusPutLR)
#	stbrx	r4,0,r3			# Store r4 to (r3)
	rlimi	r12,r4,24,0,7		# high byte to low
	rlimi	r12,r4,8,8,15		# switch middle bytes
	rlimi	r12,r4,24,16,23		# switch middle bytes
	rlimi	r12,r4,8,24,31		# high byte to low
	st	r12,0(r3)		# Store the four byte value
	S_EPILOG(BusPutLR)
	FCNDES(BusPutLR)

# Get/Put a half word (16 bit) value from/to the bus.

	S_PROLOG(BusGetSR)
#	lhbrx	r3,0,r3			# Load from (r3) into r3
	lhz	r12,0(r3)		# Get the two byte value
	xor	r3,r3,r3		# zero out r3
	rlimi	r3,r12,24,24,31		# high byte to low
	rlimi	r3,r12,8,16,23		# low byte to high
	S_EPILOG(BusGetSR)
	FCNDES(BusGetSR)

	S_PROLOG(BusPutSR)
#	sthbrx	r4,0,r3			# Store r4 to (r3)
	rlimi	r12,r4,24,24,31		# high byte to low
	rlimi	r12,r4,8,16,23		# low byte to high
	sth	r12,0(r3)		# Store the two byte value
	S_EPILOG(BusPutSR)
	FCNDES(BusPutSR)
#

# Exchange bytes:
# Parameters:
#        r3  :  Bus Address to manipulate
#        r4  :  new value
# Returns:
#        r3  :  value that was originally at specified address

	S_PROLOG(BusXchgC)
	lbz	r12,0(r3)		# get that byte
	stb	r4,0(r3)		# store the new one
	or	r3,r12,r12		# move the result into r3
	S_EPILOG(BusXchgC)
	FCNDES(BusXchgC)
