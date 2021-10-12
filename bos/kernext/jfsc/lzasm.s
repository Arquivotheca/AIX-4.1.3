# @(#)87	1.1  src/bos/kernext/jfsc/lzasm.s, sysxjfsc, bos411, 9428A410j 3/29/94 17:32:11
#*****************************************************************************
#
# COMPONENT_NAME: SYSXJFSC (JFS Compression)
#
# FUNCTIONS: lzencode lzdecode
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
	.file "lzasm.s"
	.machine "com"
#**********************************************************************
#
#  NAME: lzdecode
#
#  lz decoder. assembly language version of decode.c
#
#  lzdecode(input, output, nbytes)
#  parameters:
#
#	input = r3 = pointer to first input character. assumed to
#		be word aligned.
#
#	output = r4 = pointer to output buffer. assumed to be word
#		 aligned and to be PAGESIZE in length
#
#	nbytes = r5 = length of input in bytes.
#
#  returns length of decoded output or -1 if no ENDMARKER is found 
#  in input buffer.
#**********************************************************************
	define(`rin', 3)
	define(`rout', 4)
	define(`rinword', 5)
	define(`routcnt', 6)
	define(`rwbuf', 7)
	define(`rneed', 8) 
	define(`rtmp', 9)
	define(`rslen', 10)
	define(`rpos', 11)
	define(`rmask', 12)
	define(`roff', 13)
	define(`rones', 14) 
	define(`rc', 15)
	define(`rm1', 16)
	define(`rrem', 17)
	define(`rtmp1', 18)
	define(`rendin', 19)
	define(`rm2', 20)
	define(`rmove', 24)
	.set WIND,1024
	.set WMASK, 1023
	.set LOGW, 10
	.set HMASK,0x1ffe
	.set NXT, 0
	.set POS, 2
	.set SYMCHAR, 4
	.set MAXLEN, 271
	.set PSIZE, 4096
	.set ENDMARK, 0x1fff
	.set NBITS, (LOGW+13)
	.set ENDSYM, 287
	.set S1, (NBITS - 3)
	.set S2, (NBITS - 4)
	.set S3, (NBITS - 5)
	
	S_PROLOG(lzdecode)

	stm	r12,-80(r1)		# save r12-r31
	mfcr	r0			# save condition reg in r0
	ai	r5,r5,3			# increment nbytes by 3
	andil.  r5,r5,0xfffc 		# round to word boundary 
	a	rendin,rin,r5		# rendin points to end of input 
	cal	rrem, 32(r0)		# set remainder to 32
	cal	rneed,NBITS(r0)		# set bits needed to NBITS
	l	rinword,0(rin)		# rinword = first word of input
	cal	routcnt,0(r0)		# set outcnt to 0
	ai	rones,routcnt,-1	# rones = ONES
	cal	rm1,1(r0)		# rm1 = 1
	sli	rm1, rm1, (NBITS - 1)	# rm1 = 1 << (NBITS - 1)
	sli 	rm2,rones,LOGW		# rm2 = ~(WINDOWSIZE - 1)

# put NBITS bits into wbuf right justfied. NBITS is equal to
# the length of the longest (len, ptr). remainder is number of
# bits left in inword, and bitsneeded is the number of bits
# needed from inword. 

decodeloop:
	sl	rwbuf,rwbuf,rneed	# make room for bitsneeded
	sf.	rrem,rneed,rrem		# remainder -= bitsneeded
	sfi	rtmp,rneed,32		# rtmp = 32 - bitsneeded
	sr	rmask,rones,rtmp	# rmask = ONES >> (32 - bitsneeded)
	blt	0,nxtword		# branch if remainder < 0
	sr	rtmp,rinword,rrem	# rtmp = (inword >> remainder)
	and	rtmp,rtmp,rmask		# rtmp (inword >> remainder) & mask
	or	rwbuf,rwbuf,rtmp	# rwbuf |= (inword >> rem) & mask 
	b	decode			# go to decode
# not enuff bits in inword to makeup NBITS
nxtword:
	ai  	rin,rin,4		# increment input pointer to next word
	sfi	rtmp,rrem,0		# rtmp = -remainder
	cmp	0,rin,rendin		# test for end of input
	sl	rtmp,rinword,rtmp	# rtmp = (inword << (-rem))
	and	rtmp,rtmp,rmask		# rtmp (inword << (-rem)) & mask
	or	rwbuf,rwbuf,rtmp	# rwbuf |= (inword >> rem) & mask 
	bge     0,nearend		# nearend
	l	rinword,0(rin)		# get next input word
	ai	rrem,rrem,32		# remainder += 32
	sr	rtmp,rinword,rrem	# rtmp = inword >> remainder
	or	rwbuf,rwbuf,rtmp	# wbuf |= (inword >> remainder)
	b	decode

# if (len,ptr) has len = ENDSYMBOL, the ptr field is not in the
# encoded output. so we may not be able to get the ptr field.
# the label nearend can be reached twice legimitately.
nearend:	
	ai	rtmp,rendin,4		# rtmp = rendin + 4
	cmp	0,rin,rtmp              # test for end
	bgt	0,noendsym		# branch if input exhausted
	cal	rrem, 0(r0)		# remainder = 0

# decode wbuf. the rightmost NBITS of wbuf contain the next bits to
# decode. NBITS is equal to the longest possible (len,ptr) pair.

decode:  
# test for raw character = (0,byte)
	and.	rtmp,rwbuf,rm1		# test for high order 1
	cmpi	1,routcnt,PSIZE		# test for end of output 
	bne	0,s1       		# branch if not raw character
	cal	rneed,9(r0)		# bitsneeded = 9
	sri	rtmp,rwbuf,(NBITS - 9)  # low order byte rtmp = byte
	bge	1,noendsym		# branch if at end of output buf
	stbx	rtmp, rout,routcnt	# put byte to output
	ai	routcnt,routcnt,1	# increment output counter
	b	decodeloop

# wbuf = (slen,ptr). determine slen and position (ptr)
s1:
	rlinm	rc,rwbuf,(32-S1),0x3	# rc = (wbuf >> S1) & 0x3
	cmpi	1,rc,2			# test for c <= 1 or c >= 2
	andil.	roff,routcnt,WMASK	# roff = outcnt & (WIND - 1)
	bge	1,s2			# branch if c >= 2
# c <= 1
	rlinm	rpos,rwbuf,(32-NBITS+3+LOGW),WMASK	# rpos = position
	cmp	1,rpos,roff		# compare pos with offset
	ai	rslen,rc,2		# slen = c + 2
	cal	rneed,(LOGW+3)(r0)	# bitsneeded = LOGW + 3
	b	common			# go to common
# c >= 2
s2: 	
	bgt	1,s3			# branch if c > 2
	rlinm	rpos,rwbuf,(32-NBITS+5+LOGW),WMASK  # rpos = position
	cmp	1,rpos,roff		# compare pos with offset
	rlinm	rtmp,rwbuf,(32-S1+2),0x3    # rtmp = (wbuf >> (S1-2)) & 0x3
	ai	rslen,rtmp,4		# slen = 4 + rtmp
	cal	rneed, (LOGW+5)(r0)	# bitsneeded = LOGW + 5
	b	common
# c > 2
s3:
	rlinm	rc,rwbuf,(32-S2),0x7	# rc = (wbuf >> S2) & 0x7
	cmpi	0,rc,6			# test for c == 6
	bgt	0,s4			# branch if c > 6
	rlinm	rpos,rwbuf,(32-NBITS+7+LOGW),WMASK  # rpos = position
	cmp	1,rpos,roff		# compare pos with offset
	rlinm	rtmp,rwbuf,(32-S2+3),0x7    # rtmp = (wbuf >> (S2-3)) & 0x7
	ai	rslen,rtmp,8		# slen = 8 + rtmp
	cal	rneed, (LOGW+7)(r0)	# bitsneeded = LOGW + 7
	b	common
# c > 6
s4:
	rlinm	rc,rwbuf,(32-S3),0xf	# rc = (wbuf >> S3) & 0xf
	cmpi	0,rc,14			# test for c == 14
	bgt	0,s5			# branch if c > 14
	rlinm	rpos,rwbuf,(32-NBITS+9+LOGW),WMASK  # rpos = position
	cmp	1,rpos,roff		# compare pos with offset
	rlinm	rtmp,rwbuf,(32-S3+4),0xf    # rtmp = (wbuf >> (S3-4)) & 0xf
	ai	rslen,rtmp,16		# slen = 16 + rtmp
	cal	rneed, (LOGW+9)(r0)	# bitsneeded = LOGW + 9
	b	common

# c > 14
s5:
	rlinm	rtmp,rwbuf,(32-S3+8),0xff   # rtmp = (wbuf >> (S3-8)) & 0xff
	ai	rslen,rtmp,32		# slen = 32 + rtmp
	cmpi	0,rslen,ENDSYM		# test for ENDSYM
	beq 	0,decodefini			# branch if so
	rlinm	rpos,rwbuf,0,WMASK  	# rpos = position
	cmp	1,rpos,roff		# compare pos with offset
	cal	rneed, (LOGW+13)(r0)	# bitsneeded = LOGW + 13 
	b	common

# move slen bytes to output. cr 1 has compare of pos and offset
common:
	a	rtmp,routcnt,rslen	# rtmp = outcnt + slen
	cmpi	0,rtmp,PSIZE		# test for output buf overflow
	and	rtmp,routcnt,rm2	# rtmp = routcnt & ~(WINDOWSIZE - 1)
	blt 	1, m1			# branch if pos < offset
	ai	rpos,rpos,-WIND		# pos -= WINDOWSIZE
m1:
	a	rpos,rpos,rtmp		# rpos += outcnt & ~(WIND - 1)
	a	rtmp, rpos,rslen	# rtmp = pos + slen
	cmp	1,rtmp,routcnt		# compare pos + slen with outcnt
	bgt	0,noendsym		# branch if output buf overflow

# if source overlaps output, copy one byte at a time.
	bgt	1,onebyone		# branch if overlap

# copy bytes from output buffer starting at rpos to output buffer
# starting at outcnt

	sri.   rtmp1,rslen,5 		# Number of 32-byte chunks 
	bgt    0,movelong		# Branch if > 32 bytes to move.

# short move (0 to 32 bytes).
	mtxer  rslen        		# set move length 
	lsx    rmove,rout,rpos		# get source string (kills r24-r31)
	stsx   rmove,rout,routcnt       # store it.
	b movedone			# go to movedone
	 
# more than 32 bytes to move
movelong:
	mtctr  rtmp1			# CTR = num 32-byte chunks to move.
	cal    rtmp,32(r0)		# rtmp = 32 
        mtxer  rtmp          		# XER = 32 (move length per iteration).
        rlinm  rtmp1,rslen,0,0x1f	# rtmp1 = remainder length.
	cal    rtmp,0(rout)		# rtmp -> begin output buffer
#
mloop:
	lsx    rmove,rtmp,rpos          # get 32 bytes of source.
	stsx   rmove,rtmp,routcnt	# store it.
	ai     rtmp,rtmp,32    		# increment source and target address.
	bdn    mloop   			# decr count, Br if chunk(s) left to do.

# move remainder
	mtxer  rtmp1  		        # XER = remainder length.
	lsx    rmove,rtmp,rpos          # get source string.
        stsx   rmove,rtmp,routcnt	# store it.
	b movedone			# go to move done

# copy bytes one at a time
onebyone:
	mtctr	rslen			# number of bytes to move
	cal	rtmp,0(rout)		# rtmp -> output buffer
mloop1:
	lbzx	rtmp1,rtmp,rpos		# get a byte
	stbx	rtmp1,rtmp,routcnt	# store a byte
	ai	rtmp,rtmp,1		# increment source and target addr
	bdn	mloop1			# decrement count and loop

movedone:
	a	routcnt,routcnt,rslen	# increment outcnt
	b	decodeloop
	
# no endsymbol found 
noendsym:
	cal	r3,1(r0)		# r3 = 1
	sfi	r3,r3,0			# r3 = - 1
	b	return

decodefini:
	cal	r3,0(routcnt)		# r3 = out counter

return:
	mtcr	r0			# restore cr
	lm	r12,-80(r1)		# restore registers
	br				# return

	S_EPILOG
	FCNDES(lzdecode)

#**********************************************************************
#
#  NAME: lzencode
#
#  lz encoder. assembly language version of encode.c
#
#  lzencode(input, output, endout, htab, freeh, rand)
#  number of bytes of input assumed to be 4k.
#  parameters:
#
#	input = r3 = pointer to first input character. assumed to
#		be word aligned.
#
#	output = r4 = pointer to output buffer. assumed to be word
#		 aligned. 
#
#	endout = r5 = pointer to one past last byte of output buffer.
#		 assumed to be word aligned.
#
#	htab = r6 = pointer to begin htab. initialized to zeros.
#
#	freeh = r7 = pointer to array of htab entries.
#
#	rand = r8 = pointer to array of random bytes.
#
#  returns length of encoded data or PAGESIZE if output buffer is
#  filled before all input data is processed.
#
#**********************************************************************
	define(`rin', 3)
	define(`rout', 4)
	define(`routend', 5)
	define(`rhtab', 6)
	define(`rfreeh', 7)
	define(`rrand', 8)
	define(`rm', 9)
	define(`rj', 10)
	define(`rjm', 11) 
	define(`rsym', 12)
	define(`rc', 13)
	define(`rc1', 14)
	define(`rtmp', 15)
	define(`rhash', 16)
	define(`rprev', 17)
	define(`rqptr', 18)
	define(`rpos', 19)
	define(`rwin', 20)
	define(`rkp', 21)
	define(`rc2', 22)
	define(`routw', 23)
	define(`rrem', 24)
	define(`rqbase', 25)
	define(`rq', 26)
	define(`routorg', 27)
	
	S_PROLOG(lzencode)

	stm	r12,-80(r1)		# save r12-r31
	mfcr	r0			# save cr in r0
	cal	routw,0(r0)		# set outword to 0
	cal	rrem, 32(r0)		# set remainder to 32
	cal	rj,0(r0)		# set rj to 0
	cal	rm,0(r0)		# set rm to 0
	ai 	rwin, rj,-WIND		# rwin = - WINDOWSIZE
	cal	rqbase,0(rfreeh)	# rqbase -> origin of hentry array
	ai	rout,rout,-4		# adjust output ptr for update stores
	ai	routend,routend,-4	# adjust routend 
	cal	routorg, 0(rout)	# remember begin output buffer

#
# begin search loop. exit if less than 3 characters left to process.
# on exit at least one character left to process. (reason is at
# bottom of loop we load the next character before exiting loop)
#
encodeloop:
	a	rj,rj,rm		# j = j + m
	cmpi	0, rj, 4094 		# s[4094] is next to last
	a	rwin, rwin, rm		# win = win + m
	cal	rm,1(r0)		# set rm to 1
	bge	0, encodefini		# branch if at < 3 to go
	ai	rjm, rj, 1		# rjm = j + 1
	lbzx	rc, rin, rjm		# rc = s[j + 1]
	lbzx	rsym,rin,rj		# rsym = s[j] 
loop1:
	sli	rtmp,rc,1		# rtmp = 2*c
	lhzx	rhash,rrand,rtmp	# rhash = random[c]
	sli	rc1,rsym,16		# rc1 = sym << 16
	xor 	rhash,rhash,rsym	# rhash = xor(rhash,sym)
	or	rc1,rc1,rc		# rc1 = sym << 16 | c
	rlinm	rhash, rhash, 1, HMASK  # rhash = offset in htab
	lhzx	rq,rhtab,rhash		# rq = htab[h0]
	cax	rprev,rhtab,rhash	# previous pointer

# search hash table for c1
#
sloop1:
	cmpi	0,rq,0			# test for rq = 0
	cax	rqptr,rqbase,rq		# rqptr->first on hash chain
	l	rtmp,SYMCHAR(rqptr)	# rtmp = symchar
	beq	0,loopexit		# exit loop if rq = 0
	lhz	rpos,POS(rqptr)		# get position (age) of entry
	cmp	0,rtmp,rc1		# test for symchar == c1
	cmp	1,rpos,rwin		# test for too old
	bne	0, nothere		# branch if symchar != c1
	blt	1, tooold		# branch if too old
	sth	rj, POS(rqptr)		# change postion of old entry to j
	ai	rsym, rq, 256		# sym = q + 256
	cal	rkp, 0(rpos)		# save position of match in rkp	
	b	extend			# go to extend.

nothere:	
	lhz	rq,NXT(rqptr)		# rq = next on hash chain
	cal	rprev, 0(rqptr)		# update previous pointer
	b	sloop1			# contnue search 

# match was found in hentry[rq]. add lengthened string to hash 
# table, overwriting existing entry if it exists.
#
extend:
	ai	rm, rm, 1		# increment m 
	cax	rtmp, rkp, rm		# rtmp = kp + m
	lbzx	rc2, rin, rtmp		# rc2 = s[kp + m]
	sli	rc1,rsym,16		# rc1 = sym << 16
	sli	rtmp, rc2, 1		# rtmp = 2*rc2
	lhzx	rhash,rrand,rtmp	# rhash = random[c2]
	or	rc1,rc1,rc2		# c1 = sym << 16 | c2
	xor 	rhash,rhash,rsym	# rhash = xor(rhash,sym)
	rlinm	rhash, rhash, 1, HMASK  # rhash = offset in htab
	ai	rjm, rjm, 1		# increment j + m
	lhzx	rq,rhtab,rhash		# rq = htab[h1]
	lbzx	rc, rin, rjm		# rc = s[j + m]
	cmpi	0,rm,(MAXLEN-1)		# test for loop termination
	cmpi	1,rjm, 4094		# test for loop termination
	cmp	2,rc,rc2		# test for c == c2
	cror	gt+cr3*4,gt+cr0*4,gt+cr1*4  # combine loop term test into cr3

# search for c1 in hash table
sloop2:
	cmpi	0,rq,0			# test for rq = 0
	cax	rqptr,rqbase,rq	 	# rqptr->first on hash chain
	l	rtmp,SYMCHAR(rqptr)	# rtmp = symchar
	beq	0, insert		# rq = 0.
	cmp	1,rtmp,rc1		# test for symchar == c1
	beq	1, found1		# branch if symchar == c1
	lhz	rq,NXT(rqptr)		# rq = next on hash chain
	b	sloop2			# continue search

# insert c1 into hash table. 
insert: 
	ai	rqptr, rfreeh, 8	# rqptr->next free hentry
	sf	rq, rqbase, rqptr	# rq = diff in addresses
	lhzx	rtmp,rhtab, rhash	# rtmp = htab[q1]
	st	rc1, SYMCHAR(rqptr)	# h[].symchar = c1
	sthx	rq,rhtab,rhash 		# new head of hash chain
	sth	rtmp, NXT(rqptr)	# next on hash chain
	cal	rfreeh, 0(rqptr)	# update rfreeh 
found1:
	sth	rkp, POS(rqptr)		# h[].pos = kp

# test for loop continuation
	bgt	3,loopexit		# see above 
	bne 	2,loop1			# branch if c != c2
	sth	rj, POS(rqptr)		# update position to j
	ai	rsym, rq, 256		# sym = q1 + 256
	b	extend			# go to extend

# delete entry pointed to by rqptr from hash chain. 
tooold:
	lhz	rtmp, NXT(rqptr)
	sth	rtmp, NXT(rprev)

loopexit:
	cmpi	0, rm, 1		# test for no matching strings
	cmpi	1, rm, 3		# may as well test for 3
	bgt	0,m3			# branch if match found

# insert c1 into hash table
	ai.	rrem,rrem,-9		# rem = rem - 9
	ai	rqptr, rfreeh, 8	# rqptr->next free hentry
	sf	rq, rqbase, rqptr	# rq = diff in addresses
	lhzx	rtmp,rhtab, rhash	# rtmp = htab[q1]
	st	rc1, SYMCHAR(rqptr)	# h[].symchar = c1
	sthx	rq,rhtab,rhash 		# new head of hash chain
	sth	rtmp, NXT(rqptr)	# next on hash chain
	cal	rfreeh, 0(rqptr)	# update rfreeh 
	sth	rj,POS(rqptr)		# h[].pos = j
# output raw character
	blt	0,put1			# branch if rem was negative
	sl	rtmp, rsym, rrem	# rtmp = sym << rem   
	or	routw, routw, rtmp	# outword |= sym << rem
	b	encodeloop		# go to top of loop
put1:	sfi	rtmp,rrem,0		# rtmp = -rem
	sr	rtmp,rsym,rtmp		# rtmp = sym >> (-rtmp)
	or	routw,routw,rtmp	# outword | (sym >> (-rem))
	stu     routw, 4(rout)		# *++output = outword
	cmp	0,rout,routend		# test for end of output buffer
	ai	rrem,rrem,32		# rem = rem + 32
	sl	routw, rsym, rrem	# outword = sym << remainder	
	blt	0,encodeloop		# go to top of loop
	b	toolong			# output buffer full
#
#  here m > 1. test m <= 3 is in cr 1
m3:
	bgt	1, m7			# branch if m > 3
	ai	rsym, rm, 2		# sym = (4 + m - 2)
	cal	rtmp,(LOGW + 3)(r0)     # rtmp = LOGWINDOW + 3
	b	cont2
# here m > 3
m7:
	cmpi	0, rm, 7		# test for m > 7
	cmpi	1, rm, 15		# test for m > 15
	bgt	0, m15			# branch if m > 7
	ai	rsym, rm, 20		# sym = (24 + m - 4)
	cal	rtmp,(LOGW + 5)(r0)     # rtmp = LOGWINDOW + 5
	b	cont2

# here m > 7 test for m <= 15 is in cr 1
m15:
	bgt	1, m31			# branch if m > 15
	ai	rsym, rm, 104		# sym = (112 + m - 8)
	cal	rtmp,(LOGW + 7)(r0)     # rtmp = LOGWINDOW + 7
	b	cont2

# here m > 15
m31:
	cmpi	0, rm, 31		# test for m > 31
	bgt	0, m271			# branch if m > 31
	ai	rsym, rm, 464		# sym = (480 + m - 16)
	cal	rtmp,(LOGW + 9)(r0)     # rtmp = LOGWINDOW + 9
	b	cont2

# here m > 31. m should be <= 271 = MAXLEN
m271:	
	ai	rsym, rm, 7904		# sym = (7936 + m - 32)
	cal	rtmp,(LOGW + 13)(r0)    # rtmp = LOGWINDOW + 13
	
# output (sym << LOGW) | (kp & (WIND - 1). rtmp contains length to output
cont2:
	sli	rsym, rsym, LOGW	# sym = sym << LOGW
	andil.	rkp, rkp, WMASK		# rkp = rkp & (WIND - 1)
	sf.	rrem,rtmp,rrem		# rem = rem - rtmp
	or	rsym,rsym,rkp  		# rsym = value to output
	blt	0,put2			# branch if rem was negative
	sl	rtmp, rsym, rrem	# rtmp = sym << rem   
	or	routw, routw, rtmp	# outword |= sym << rem
	b	encodeloop  		# go to continuation
put2:	sfi	rtmp,rrem,0		# rtmp = -rem
	sr	rtmp,rsym,rtmp		# rtmp = sym >> (-rtmp)
	or	routw,routw,rtmp	# outword | (sym >> (-rem))
	stu     routw, 4(rout)		# *++output = outword
	cmp	0,rout,routend		# test for end of output buffer
	ai	rrem,rrem,32		# rem = rem + 32
	sl	routw, rsym, rrem	# outword = sym << remainder	
	blt	0,encodeloop		# continue if output buffer not full
	b	toolong


# ouput remaining characters as raw bytes. 
encodefini:
	cmpi	0,rj,4095		# s[j] is next character 
	bgt	0, endmark		# branch if done
	lbzx	rsym,rin,rj		# sym = s[j]
	ai	rj,rj,1			# increment j
# putnext(9)
	ai.	rrem,rrem,-9		# rem = rem - 9
	blt	0,put3			# branch if rem was negative
	sl	rtmp, rsym, rrem	# rtmp = sym << rem   
	or	routw, routw, rtmp	# outword |= sym << rem
	b	encodefini		# continue fini loop
put3:	sfi	rtmp,rrem,0		# rtmp = -rem
	sr	rtmp,rsym,rtmp		# rtmp = sym >> (-rtmp)
	or	routw,routw,rtmp	# outword | (sym >> (-rem))
	stu     routw, 4(rout)		# *++output = outword
	cmp	0,rout,routend		# test for end of output buffer
	ai	rrem,rrem,32		# rem = rem + 32
	sl	routw, rsym, rrem	# outword = sym << remainder	
	blt	0,encodefini		# continue fini loop
	b	toolong			# output buffer full

# output ENDMARKER
endmark:
	cal	rsym,ENDMARK(r0)	# rsym = ENDMARK
# putnext(13)
	ai.	rrem,rrem,-13		# rem = rem - 13
	blt	0,put4			# branch if rem was negative
	sl	rtmp, rsym, rrem	# rtmp = sym << rem   
	or	routw, routw, rtmp	# outword |= sym << rem
	b	retval			# go to retval 
put4:	sfi	rtmp,rrem,0		# rtmp = -rem
	sr	rtmp,rsym,rtmp		# rtmp = sym >> (-rtmp)
	or	routw,routw,rtmp	# outword | (sym >> (-rem))
	stu     routw, 4(rout)		# *++output = outword
	cmp	0,rout,routend		# test for end of output buffer
	ai	rrem,rrem,32		# rem = rem + 32
	sl	routw, rsym, rrem	# outword = sym << remainder	
	bge	toolong			# branch if output buffer full

# calculate return value.
retval:
	sf	rtmp,routorg,rout	# rtmp = bytes in full words
	sfi	rc,rrem,39		# rc = 39 - rem 
	sri	rc, rc, 3		# rc = number of bytes in remainder
	a	r3, rtmp, rc		# r3 = number of bytes in output
	stu	routw,4(rout)		# store last possibly extra word
	
return1:
	lm	r12,-80(r1)		# restore registers
	mtcr	r0			# restore cr
	br				# return

toolong:
	cal	r3,PSIZE(r0)		# return pagesize
	b	return1

	S_EPILOG
	FCNDES(lzencode)
