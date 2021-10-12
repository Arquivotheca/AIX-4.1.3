# @(#)13	1.11  src/bos/kernext/inet/POWER/in_cksum.s, sysxinet, bos411, 9428A410j 6/22/94 16:15:11
#
#   COMPONENT_NAME: SYSXINET
#
#   FUNCTIONS: S_EPILOG
#		S_PROLOG
#		m_len
#		m_next
#		min
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# in_cksum.s - Internet Protocol checksum for sockets
#
# CALLED FROM: various places in sockets AF_INET code
# CALLS: (none)
#
# CALLING SEQUENCE:
#	int in_cksum(m, totlen)
#	struct mbuf	*m;	/* pointer to mbuf chain */
#	int		totlen;	/* number of bytes to checksum */
#
# INPUT:
#	r3 - mbuf pointer
#	r4 - total length of data to checksum
#
# RETURNS:
#	r3 - checksum (as unsigned short)
#
# IMPLEMENTATION NOTES:
#	This routine is intended to be small and have low overhead.
#	For example, it is competitive in speed with the in-lined C code
#	in ip_output that sums the 20 byte IP header.
#
#	This routine returns a 16-bit ones-complement sum,
#	which we compute by doing a 32-bit ones-complement sum
#	and then doing a final fold into 16 bits.
#	We fetch mbuf data four bytes at a time (via "lu")
#	and add it ones-complement (via "ae").
#	To handle lengths that are not a multiple of four,
#	there is then a partial fetch (via "lsx") and add.
#	To maintain a valid carry (CA) bit for "ae"
#	we avoid CA-setting instructions except for an initial CA-clearing
#	instruction and the data summing instructions themselves.
#	For more details, look for references to CA below.
#	The additions must be "aligned" on a 16-bit boundary,
#	so if we sum an odd number of bytes in a given mbuf,
#	we rotate the sum 8 bits so the next mbuf additions
#	are aligned properly.  We remember the rotation distance
#	(0 or 8) and do a final rotation of that amount
#	before doing the final 32->16 bit fold.
#
# MORE NOTES:
#	This routine was tested with a program that generated several
#	million mbuf chains (randomizing the number of mbufs,
#	the length of each, the data in each, and the "offset" of each).
#
#	A more elaborate strategy could avoid the current 25%
#	cache-miss penalty (and 50% unaligned data penalty)
#	by initial summing to four bytes past a 64 byte boundary,
#	then summing 64 bytes at a time, and then summing the remainder.
#	The 64 bytes would be loaded by 1 "lu" and 15 "l"s
#	(or maybe just 16 "lu"s) the last of which "trips" the cache.
#	The cache (or TLB) delay is covered by the 16 "ae"s which follow.
#	This strategy has more overhead, however, and is only
#	useful for checksumming large packets that are not in
#	the cache.  But at the point in_cksum is called
#	the data is probably in the cache!
#
#	The outer loop has three tests:
#	1) Is this the last mbuf?  (i.e. will we have summed "totlen" bytes?)
#	2) Should we sum at least 4 bytes in this mbuf?
#	3) Is there a remainder of 1, 2, or 3 bytes to be summed?
#
# FUNCTIONAL DIFFERENCES between this in_cksum and AIX 3.2 in_cksum:
#	1.  The AIX 3.2 in_cksum is incorrect when checksumming an mbuf chain
#	that contains an mbuf with m_data odd-halfword aligned and m_len == 0.
#	In this case the AIX 3.2 in_cksum will fetch and sum a halfword,
#	even though the mbuf has no data.
#
#	2.  Usual implementations of in_cksum check for a null mbuf pointer,
#	which can happen e.g. if the mbuf chain has fewer than totlen bytes.
#	The AIX 3.2 in_cksum checks only for an initially null pointer.
#	This in_cksum routine does not check at all, since it "cannot happen".
#
#	3.  The AIX 3.2 in_cksum routine returns a "signed short" result;
#	this routine (following the usual practice) returns unsigned short.
#	To return "signed short" uncomment the line marked XXX below.
#
# REGISTER USAGE:
#
	.set	sum,r0		# running sum
	.set	m,r3		# (current) mbuf pointer
	.set	totlen,r4	# (remaining) length of data to sum
#	r5		temp reg for mbuf data to be summed, and "newtotlen"
#	r6		temp reg for mbuf data to be summed, and "wcount"
#	r7		unused
#	r8		unused
#	r9		unused
	.set	ptr,r10		# pointer to mbuf data
	.set	mlen,r11	# length of mbuf data to be summed,
				#     then updated to be mlen%4.
	.set	odd,r12		# even(0)/odd(8) alignment indicator

#
#	equates for mbuf structure
#
	.set	m_next,0	# pointer to next mbuf - fullword
	.set	m_len,8		# length of data to be checksummed - fullword
	.set	m_data,12	# pointer to data to be checksummed - fullword
#
#
	.csect	.in_cksum[PR],5	# create 32-byte aligned csect
	.globl	.in_cksum[PR]

in_ckstart:
	cal	sum,0(0)	# initialize running sum
	a	odd,sum,sum	# initialize even/odd indicator + clear CA

top:
	l	mlen,m_len(m)	# length of this packet
	l	ptr,m_data(m)	# address of data for this packet

	subfc.	totlen,mlen,totlen # remaining = total - this packet
	cmpi	cr7,totlen,0x0	# set cr7(gt) if more remains
	blt	totlow
	l	m,m_next(m)	# point m to next packet

continu:
	srai.	r6,mlen,2	# wcount := mlen/4 (clear CA)
	mtctr	r6		# load ctr with count of words to add
	beq	onlyrem		# no word! only a remander

	l	r5,0(ptr)	# load 1st word to add to sum
	andil.	mlen,mlen,0x3	# mlen := mlen%4 ; cr0(eq) will
				# indicate if there's a remainder
	bdn	loop
	b	add_r5		# fewer than 8 bytes to sum
	nop

loop:	lu	r6,4(ptr)	# next word to sum & bump ptr
	ae	sum,sum,r5	# add previous word and carry
	bdz	add_r6		# decr ctr, if zero we have one word
	lu	r5,4(ptr)	# next word to sum & bump ptr
	ae	sum,sum,r6	# add ...
	bdn	loop		# close of primary sum loop

add_r5:	ae	sum,sum,r5
	addze	sum,sum		# add in CA
	bne	dorem
	bgt	cr7,top		# more bytes to sum
	b	done

add_r6:	ae	sum,sum,r6
	addze	sum,sum		# add in CA
	bne	dorem
	bgt	cr7,top		# more bytes to sum

#
# return final sum to caller
#
done:
	rlnm	sum,sum,odd,0xffffffff
	rlinm	r6,sum,16,0xffff0000
	a	sum,sum,r6
	rlinm	sum,sum,16,0x0000ffff
	aze	sum,sum
	xoril	r3,sum,0xffff
#	exts	r3,r3		# XXX cast to signed short
	br

dorem:
	cal	ptr,4(ptr)	# Point to final piece
dorem2:
	mtxer	mlen
	lsx	r5,0,ptr
	rlinm	r6,mlen,3,0x8
	xor	odd,odd,r6
	a	sum,sum,r5
	aze	sum,sum		# must add in CA here since we might rotate
	rlnm	sum,sum,r6,0xffffffff
	bgt	cr7,top		# more bytes to sum
	b	done

onlyrem:
	andi.	mlen,mlen,0x3	# isolate remainder
	bne	dorem2		# if there is one do it
	bgt	cr7,top		# more bytes to sum
	b	done		# not even remainder

totlow:
	add.	mlen,totlen,mlen # restore mlen to totlen
	bne	continu		# continue if not zero
	b	done		# no more to do

	.align 	2
trback:
	.tbtag	0,12,0x20,0x40,0,0,2,0,0,(trback-in_ckstart),0,0,0,8,0x63,0
	.org	trback+20
	.short	8
	.string "in_cksum"
	FCNDES(in_cksum)
