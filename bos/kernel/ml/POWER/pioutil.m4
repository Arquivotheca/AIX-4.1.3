# @(#)05	1.3  src/bos/kernel/ml/POWER/pioutil.m4, sysml, bos411, 9428A410j 7/27/93 20:54:17
#
# COMPONENT_NAME: (SYSIOS) IO Subsystem
#
# FUNCTIONS: buscpy, busgetlr, busputlr, busgetc, busputc
#	busgets, busputs, busgetl, busputl
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990,1992,1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
# NOTES:
#	These are the UP system overlays for pio services.  These routines
# should be callable from a kernel extension with a branch absolute, or
# through the glink interface.
#
#-----------------------------------------------------------------------------


#------------------------------------------------------------------------------
#
# NAME: busgetc
#
# FUNCTION: set up a exception handler and get a byte from IO space
#
# CALL:	int busgetc(ioaddr, data)
#	char *ioaddr;			/* effective address to load from */
#	char *data;			/* byte value returned here	  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busgetc_addr+real0
ENTRY(busgetc):
	.globl	ENTRY(busgetc)

	GET_CSA(cr0, r6, r5)			# get mst pointer
	mflr	r6				# return address
	st	r6, excbranch(r5)		# set exception handler
	lbz	r8, 0(r3)			# do load
	lil	r3, 0				# clear exception handler
	st	r3, excbranch(r5)
	stb	r8, 0(r4)			# set return value
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busgets
#
# FUNCTION: set up a exception handler and read a short form IO space
#
# CALL:	int busgets(ioaddr, data)
#	short *ioaddr;			/* effective address to load from */
#	short *data;			/* short value returned here	  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busgets_addr+real0
ENTRY(busgets):
	.globl	ENTRY(busgets)

	GET_CSA(cr0, r6, r5)			# get pointer to mst
	mflr	r6				# get return address
	st	r6, excbranch(r5)		# set up exception handler
	lhz	r8, 0(r3)			# do load
	lil	r3, 0				# clear exception handler
	st	r3, excbranch(r5)
	sth	r8, 0(r4)			# store return data
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busgetsr
#
# FUNCTION: set up a exception handler, and read a little-endian short
#
#
# CALL:	int busgetsr(ioaddr, data)
#	short *ioaddr;			/* effective address to load from */
#	short *data;			/* short value returned here	  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busgetsr_addr+real0
ENTRY(busgetsr):
	.globl	ENTRY(busgetsr)

	GET_CSA(cr0, r6, r5)		# get mst pointer
	mflr	r6			# get return address
	st	r6, excbranch(r5)	# set excption handler
	lhz	r12,0(r3)		# Get the two byte value
	lil	r3, 0
	st	r3, excbranch(r5)	# clear exception handler
	rlimi	r7, r12, 24, 0xff	# move high byte to low
	rlimi	r7, r12, 8, 0xff00	# move low byte to high
	sth	r7, 0(r4)		# store result
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busgetl
#
# FUNCTION: set up a exception handler and read a long form IO space
#
# CALL:	int busgetl(ioaddr, data)
#	long *ioaddr;			/* effective address to load from */
#	long *data;			/* long value returned here	  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busgetl_addr+real0
ENTRY(busgetl):
	.globl	ENTRY(busgetl)

	GET_CSA(cr0, r6, r5)		# get mst pointer
	mflr	r6			# get return value
	st	r6, excbranch(r5)	# set exception handler
	l	r8, 0(r3)		# do load
	lil	r3, 0			# clear exception handler
	st	r3, excbranch(r5)
	st	r8, 0(r4)		# store result
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busgetlr
#
# FUNCTION: set up a exception handler, and read a little-endian long
#
#
# CALL:	int busgetlr(ioaddr, data)
#	long *ioaddr;			/* effective address to store to */
#	long *data;			/* long value returned here	  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busgetlr_addr+real0
ENTRY(busgetlr):
	.globl	ENTRY(busgetlr)

	GET_CSA(cr0, r6, r5)		# get mst pointer
	mflr	r6			# get return address
	st	r6, excbranch(r5)	# set exception handler
	l	r12,0(r3)		# do load
	lil	r3, 0			# clear exception handler
	st	r3, excbranch(r5)	
	rlimi	r7, r12, 24, 0xff000000	# do byte swapping, lo to high
	rlimi	r7, 12, 8, 0x00ff0000	# swap middle byte
	rlimi	r7, r12, 24, 0x0000ff00 # swap middle byte
	rlimi	r7, r12, 8, 0x000000ff	# high byte to low byte
	st	r7, 0(r4)		# store result
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busputc
#
# FUNCTION: set up a exception handler, and store a byte to IO space
#
# CALL:	int busputc(ioaddr, data)
#	char *ioaddr;			/* effective address to store to */
#	char data;			/* byte to store		  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busputc_addr+real0
ENTRY(busputc):
	.globl	ENTRY(busputc)

	GET_CSA(cr0, r6, r5)		# get mst address
	mflr	r6			# get return address
	st	r6, excbranch(r5)	# set exception handler
	stb	r4, 0(r3)		# do store
	lil	r3, 0			# clear exception hander
	st	r3, excbranch(r5)
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busputs
#
# FUNCTION: set up a exception handler, and store a short to IO space
#
# CALL:	int busputs(ioaddr, data)
#	short *ioaddr;			/* effective address to store to  */
#	short data;			/* short to store		  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busputs_addr+real0
ENTRY(busputs):
	.globl	ENTRY(busputs)

	GET_CSA(cr0, r6, r5)		# get mst address
	mflr	r6			# get return address
	st	r6, excbranch(r5)	# set exception hander
	sth	r4, 0(r3)		# do store
	lil	r3, 0			# clear exception hander
	st	r3, excbranch(r5)
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busputl
#
# FUNCTION: set up a exception handler, and store a long to IO space
#
# CALL:	int busputl(ioaddr, data)
#	long *ioaddr;			/* effective address to store to  */
#	long data;			/* long to store		  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busputl_addr+real0
ENTRY(busputl):
	.globl	ENTRY(busputl)
	GET_CSA(cr0, r6, r5)		# get mst pointer
	mflr	r6			# get return addresse
	st	r6, excbranch(r5)	# set exception hander
	st	r4, 0(r3)		# do store
	lil	r3, 0			# clear exception hander
	st	r3, excbranch(r5)
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busputsr
#
# FUNCTION: set up a exception handler, and store a short to IO space in
#	littel endian format
#
# CALL:	int busputsr(ioaddr, data)
#	short *ioaddr;			/* effective address to store to */
#	short data;			/* short to store		  */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busputsr_addr+real0
ENTRY(busputsr):
	.globl	ENTRY(busputsr)

	GET_CSA(cr0, r6, r5)		# get mst pointer
	mflr	r6			# get return address
	rlimi	r12, r4, 24, 0x000000ff	# high byte to low
	rlimi	r12, r4, 8, 0x0000ff00	# low byte to high
	st	r6, excbranch(r5)	# set up exception hander
	sth	r12, 0(r3)		# Store the two byte value
	lil	r3, 0			# clear exception handler
	st	r3, excbranch(r5)
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: busputlr
#
# FUNCTION: set up a exception handler, and store a long to IO space in
#	littel endian format
#
# CALL:	int busputlr(ioaddr, data)
#	long *ioaddr;			/* effective address to store to */
#	long data;			/* long to store		 */
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if load was successful
#	exception code if an exception occured
#
#------------------------------------------------------------------------------

	.org	busputlr_addr+real0
ENTRY(busputlr):
	.globl	ENTRY(busputlr)

	GET_CSA(cr0, r6, r5)		# get mst pointer
	mflr	r6			# get return address
	rlimi	r12, r4, 24, 0xff000000	# low to high byte
	rlimi	r12, r4, 8, 0x00ff0000	# switch middle byte
	st	r6, excbranch(r5)	# set up exception hander
	rlimi	r12, r4, 24, 0x0000ff00	# switch middle byte
	rlimi	r12, r4, 8, 0x000000ff	# high byte to low
	st	r12, 0(r3)		# Store the four byte value
	lil	r3, 0
	st	r3, excbranch(r5)	# clear exception hander
	br
	_DF(_DF_NOFRAME)

#------------------------------------------------------------------------------
#
# NAME: buscpy
#
#
# FUNCTION: Indexed string movement to and from Bus I/O and Memory, with
#	exception handler
#
# CALL:	int buscpy(source, dest, cnt)
#	void *source;			/* source address	*/
#	void *dest;			/* dest address		*/
#	int cnt;			/* count in bytes	*/
#
# EXECUTION ENVIORNMENT:
#	can be called from process or interrupt level
#
# RETURNS:
#	0 if successful
#	Exception Code if an exception occured
#
#------------------------------------------------------------------------------

	.set CHUNK, 32				# bytes to move per chunk
	.set L2CHUNK, 5				# log base 2 of CHUNK

	.org	real0+buscpy_addr
ENTRY(buscpy):
	.globl	ENTRY(buscpy)

	GET_CSA(cr0, r7, r6)			# get pointer to current mst
	lil	r8, CHUNK			# number of bytes to move
						# per chunk

	st	r13, -8(r1)			# save nonvolatile gprs
	st	r14, -4(r1)

	l	r11, DATA(g_toc)		# get kernel TOC pointer
	sri.	r7, r5, L2CHUNK			# number of chunks to move
	LTOCR(r11, busexcpt, data, r11)		# get address out of TOC
	rlinm	r0, r5, 0, CHUNK-1		# remainder to move
	lil	r5, 0				# move index
	st	r11, excbranch(r6)		# set up exception hander
	beq	cr0, small_copy			# branch if less then CHUNK
						#  bytes to move
	mtxer	r8				# set for string move
	mtctr	r7				# loop counter

cp_loop:
	lsx	r7, r5, r3			# load CHUNK bytes
	stsx	r7, r5, r4			# store CHUNK bytes
	ai	r5, r5, CHUNK			# move to next chunk
	bdn	cp_loop				# banch if more CHUNKs

small_copy:
	mtxer	r0				# remaining bytes to move
	lsx	r7, r5, r3
	stsx	r7, r5, r4

	lil	r3, 0				# clear exception handler
	st	r3, excbranch(r6)		
busexcpt:
	l	r13, -8(r1)			# restore nonvolatile gprs
	l	r14, -4(r1)
	br
	_DF(_DF_NOFRAME)

	.toc
	TOCL(busexcpt, data)


	FCNDES(busgetc, label)
	FCNDES(busgets, label)
	FCNDES(busgetsr, label)
	FCNDES(busgetl, label)
	FCNDES(busgetlr, label)
	FCNDES(busputc, label)
	FCNDES(busputs, label)
	FCNDES(busputl, label)
	FCNDES(busputsr, label)
	FCNDES(busputlr, label)
	FCNDES(buscpy, label)

	.csect	ENTRY(low[PR])
