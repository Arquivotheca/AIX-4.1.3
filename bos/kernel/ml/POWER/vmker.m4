# @(#)45	1.13  src/bos/kernel/ml/POWER/vmker.m4, sysml, bos411, 9436B411a 9/1/94 17:48:21
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#	Data definitions for VMM code

		.set	vmmsr,11	# Segment registers used by VMM
		.set	ptasr,12
		.set	tempsr,13

#	Following is an assembly-language mapping of "<sys/vmker.h>"
#	Changes made here should must also be made in the header file.

		.dsect   vmkerdata
vmmsrval:	.long	0	# sreg value for vmmdseg 
ptasrval:	.long	0	# sreg value for ptaseg
dmapsrval:	.long	0	# sreg value for pg space disk maps
ramdsrval:	.long	0	# sreg value for ram disk
kexsrval:	.long	0	# sreg value for kernel extension segment
iplcbptr:	.long	0	# virtual addr of ipl control block
hashbits:	.long	0	# number of bits in sid/vpn hash
stoibits:	.long	0	# hash shift amount for STOI/ITOS
psrsvdblks:	.long	0	# number of reserved paging space blocks
nrpages:	.long	0	# biggest page frame number + 1
badpages:	.long	0	# number bad memory pages
numfrb:		.long	0	# number of pages on free list
maxperm:	.long	0	# max number of frames non-working
numperm:	.long	0	# number frames non-working segments
numpsblks:	.long	0	# number of paging space blocks
psfreeblks:	.long	0	# number of free paging space blocks
bconfsrval:	.long	0	# sreg value for base config segment
pfrsvdblks:	.long	0	# number of reserved (non-pinable) frames
nofetchprot:	.long	0	# no fetch protect allowed due to h/w prob
ukernsrval:	.long	0	# user's shadow of kernel srval
numclient:	.long	0	# number of client frames
maxclient:	.long	0	# max number of client frames
kernsrval:	.long	0	# sreg value for kernel segment
stoimask:	.long	0	# STOI/ITOS mask
stoinio:	.long	0	# STOI/ITOS sid mask
maxpout:	.long	0	# non-fblru pageout up limit - i/o pacing
minpout:	.long	0	# non-fblru pageout down limit - i/o pacing
rptsize:	.long	0	# size of repaging table
rptfree:	.long	0	# next free repaging table entry
rpdecay:	.long	0	# decay rate for repaging values
sysrepage:	.long	0	# global repaging count
swhashmask:	.long	0	# bit mask to use in sid/vpn s/w hash
hashmask:	.long	0	# bit mask to use in sid/vpn hash
cachealign:	.long	0	# alignment to avoid cache aliasing
overflows:	.long	0	# number of page table overflows
reloads:	.long	0	# number of page table reloads
pmap_lock_addr: .long   0	# self explanatory
numcompress:    .long   0	# number of pages in compressed segments
noflush:        .long   0	# 0 => write compressed files at iclose()
iplcbxptr:	.long	0	# virtual addr of extended ipl control block
ahashmask:	.long	0	# bit mask to use in sid/vpn alias hash
vmkerlock:	.long	0	# lockword for vmker lock
pd_npages:      .long	0	# max number of pages to delete in one c.s.
