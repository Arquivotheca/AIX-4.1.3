# @(#)90        1.3.1.14  src/bos/kernel/ml/POWER/ufcp.m4, sysml, bos411, 9438A411a 9/19/94 12:22:04
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27
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

#****************************************************************************
#
# NAME: ufcp.m4
#
# FUNCTION:
#
# Constants and global variables which must be accessible in
# non-privileged state (read access with key 1).
# These must be located at a fixed address and page-aligned so that
# the page-protection can be set appropriately. The address 'nonpriv_page'
# is used by vmsi to determine the location of this non-privileged page
# (it is assumed to be only 1 page).
#
# NOTE: The variable DEFINE_GLOBL_ENTRIES is used only by low.s to define the
# appropriate .globl entries for the millicode routines that get overlaid
# into low memory.  No other module should be defining that variable.  This
# is to avoid possible problems with either multiple definitions of the 
# entries or problems with "invalid .globl symbol" messages in other modules
# that include this file.
#****************************************************************************

		.org    real0+0x3000
DATA(nonpriv_page):

# Product proprietary statement (as per Corporate Standard C-S 0-6045-002)
# Must be located within 1st 15,000 bytes (0x3a98)

DATA(g_copyr):  .byte   'L,'i,'c,'e,'n,'s,'e,'d,' ,'M,'a,'t,'e,'r,'i,'a,'l,'s
		.byte   ' ,'-,' ,'P,'r,'o,'p,'e,'r,'t,'y,' ,'o,'f,' ,'I,'B,'M
		.byte   ' ,'5,'7,'5,'6,'-,'0,'3,'0
		.byte   ' ,'(,'C,'),' ,'C,'o,'p,'y,'r,'i,'g,'h,'t,'s,' ,'b,'y
                .byte   ' ,'I,'B,'M,' ,'a,'n,'d,' ,'b,'y,' ,'o,'t,'h,'e,'r,'s
                .byte   ' ,'1,'9,'8,'2,',,' ,'1,'9,'9,'4
                .byte   ' ,'A,'l,'l,' ,'R,'i,'g,'h,'t,'s
                .byte   ' ,'R,'e,'s,'e,'r,'v,'e,'d
                .byte   ' ,'U,'S,' ,'G,'o,'v,'e,'r,'n,'m,'e,'n,'t
                .byte   ' ,'U,'s,'e,'r,'s,' ,'R,'e,'s,'t,'r,'i,'c,'t,'e,'d
                .byte   ' ,'R,'i,'g,'h,'t,'s,' ,'-,' ,'U,'s,'e,',
                .byte   ' ,'d,'u,'p,'l,'i,'c,'a,'t,'i,'o,'n,' ,'o,'r
                .byte   ' ,'d,'i,'s,'c,'l,'o,'s,'u,'r,'e
                .byte   ' ,'r,'e,'s,'t,'r,'i,'c,'t,'e,'d,' ,'b,'y
                .byte   ' ,'G,'S,'A,' ,'A,'D,'P,' ,'S,'c,'h,'e,'d,'u,'l,'e
                .byte   ' ,'C,'o,'n,'t,'r,'a,'c,'t,' ,'w,'i,'t,'h
                .byte   ' ,'I,'B,'M,' ,'C,'o,'r,'p,'.

#	reserve space for the millicode routines

#	The multiply & divide routines are statically defined
#	for KDB since it runs before the overlay table is initialized.

	.org	real0+mulh_addr
ifdef(`DEFINE_GLOBL_ENTRIES',`
ENTRY(mulh):
	.globl	ENTRY(mulh)
',)
ifdef(`_KDB',`
	.extern	DATA(mulh_ppc)
	.extern	DATA(mulh_pwr)
	ba	DATA(mulh_ppc)
	ba	DATA(mulh_pwr)
	.space	mulh_size-8
',`
	.space	mulh_size
') #endif _KDB

	.org	real0+mull_addr
ifdef(`DEFINE_GLOBL_ENTRIES',`
ENTRY(mull):
	.globl	ENTRY(mull)
',)
ifdef(`_KDB',`
	.extern	DATA(mull_ppc)
	.extern	DATA(mull_pwr)
	ba	DATA(mull_ppc)
	ba	DATA(mull_pwr)
	.space	mull_size-8
',`
	.space	mull_size
') #endif _KDB

	.org	real0+divss_addr
ifdef(`DEFINE_GLOBL_ENTRIES',`
ENTRY(divss):
	.globl	ENTRY(divss)
',)
ifdef(`_KDB',`
	.extern	DATA(divss_ppc)
	.extern	DATA(divss_pwr)
	ba	DATA(divss_ppc)
	ba	DATA(divss_pwr)
	.space	divss_size-8
',`
	.space	divss_size
') #endif _KDB

	.org	real0+divus_addr
ifdef(`DEFINE_GLOBL_ENTRIES',`
ENTRY(divus):
	.globl	ENTRY(divus)
',)
ifdef(`_KDB',`
	.extern	DATA(divus_ppc)
	.extern	DATA(divus_pwr)
	ba	DATA(divus_ppc)
	ba	DATA(divus_pwr)
	.space	divus_size-8
',`
	.space	divus_size
') #endif _KDB

	.org	real0+quoss_addr
ifdef(`DEFINE_GLOBL_ENTRIES',`
ENTRY(quoss):
	.globl	ENTRY(quoss)
',)
ifdef(`_KDB',`
	.extern	DATA(quoss_ppc)
	.extern	DATA(quoss_pwr)
	ba	DATA(quoss_ppc)
	ba	DATA(quoss_pwr)
	.space	quoss_size-8
',`
	.space	quoss_size
') #endif _KDB

	.org	real0+quous_addr
ifdef(`DEFINE_GLOBL_ENTRIES',`
ENTRY(quous):
	.globl	ENTRY(quous)
',)
ifdef(`_KDB',`
	.extern	DATA(quous_ppc)
	.extern	DATA(quous_pwr)
	ba	DATA(quous_ppc)
	ba	DATA(quous_pwr)
	.space	quous_size-8
',`
	.space	quous_size
') #endif _KDB


	.org	real0+_clear_lock_addr
ENTRY(_clear_lock):
ifdef(`DEFINE_GLOBL_ENTRIES',`
	.globl	ENTRY(_clear_lock)
',)
	.space	_clear_lock_size

	.org	real0+_check_lock_addr
ENTRY(_check_lock):
ifdef(`DEFINE_GLOBL_ENTRIES',`
	.globl	ENTRY(_check_lock)
',)
	.space	_check_lock_size

	.org	real0+cs_addr
ENTRY(cs):
ifdef(`DEFINE_GLOBL_ENTRIES',`
	.globl	ENTRY(cs)
',)
	.space	cs_size

# 	this is a function descriptor for cs. This must be user readable

DATA(cs):
	.long	.cs		# address of code
	.long	0		# TOC - not needed

# 	this is a function descriptor for _clear_lock. This must be user readable

DATA(_clear_lock):
	.long	._clear_lock	# address of code
	.long	0		# TOC - not needed

# 	this is a function descriptor for _check_lock. This must be user readable

DATA(_check_lock):
	.long	._check_lock	# address of code
	.long	0		# TOC - not needed

DATA(cmp_swap_index):
	.long	0x1000


# The svc_flih references these while running with the user-level
# segment register 0.

DATA(g_ksrval):  .long   0              # kernel segment register value
DATA(g_kxsrval): .long   0              # kernel extension seg reg value

# The user-level trace code references this with the user-level sreg 0.

DATA(Trconflag): .byte   0,0,0,0,0,0,0,0	# trace mode for 8 channels

# keep up to date with systemcfg.h and systemcfg.m4

DATA(_system_configuration):
syscfg_arch:		.long	0	# processor architecture 
syscfg_impl:		.long	0	# processor implementation
syscfg_version:		.long	0	# processor version
syscfg_width:		.long	0	# width (32 || 64)
syscfg_ncpus:		.long	0	# 1 = UP, n = n-way MP
syscfg_cattrib:		.long	0	# bit 31: 0=no cache, 1 = cache present
					# bit 30: 0=separate I/D, 1 = combined
syscfg_icsize:		.long	0	# size of L1 instruction cache
syscfg_dcsize:		.long	0	# size of L1 data cache
syscfg_icasc:		.long	0	# L1 instruction cache associativity
syscfg_dcasc:		.long	0	# L1 data cache associativity
syscfg_icb:		.long	0	# L1 instruction cache block size
syscfg_dcb:		.long	0	# L1 data cache block size
syscfg_icline:		.long	0	# L1 instruction cache line size
syscfg_dcline:		.long	0	# L1 data cache line size
syscfg_L2csize:		.long	0	# size of L2 cache, 0 = No L2 cache
syscfg_L2casc:		.long	0	# L2 cache associativity
syscfg_tlbattrib:	.long	0	# bit 31: 0=no tlb, 1 = tlb present
					# bit 30: 0=separate I/D, 1 = combined
syscfg_itsize:		.long	0	# entries in instruction TLB
syscfg_dtsize:		.long	0	# entries in data TLB
syscfg_itasc:		.long	0	# instruction tlb associativity
syscfg_dtasc:		.long	0	# data tlb associativity
syscfg_ressize:		.long	0	# size of reservation
syscfg_priv_lcnt:	.long	0	# spin lock count in supervisor mode
syscfg_prob_lcnt:	.long	0	# spin lock count in problem state
syscfg_rtctype:		.long	0	# RTC type 
syscfg_virtalias:	.long	0	# 1 if hardware aliasing is supported
syscfg_cachcong:	.long	0	# number of page bits for cache synon
syscfg_modarch:		.long	0	# model architecture
syscfg_modimpl:		.long	0	# model implementation
syscfg_Xint:		.long	0	# time base conversion
syscfg_Xfrac:		.long	0	# time base conversion
syscfg_rserved:		.space	4*33	# reserve for expansion of structure
