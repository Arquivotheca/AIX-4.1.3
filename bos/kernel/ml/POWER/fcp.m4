# @(#)41	1.14.1.22  src/bos/kernel/ml/POWER/fcp.m4, sysml, bos41J, 9513A_all 3/23/95 17:39:48

#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27,83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************
 

#****************************************************************************
#
# NAME: fcp.m4
#
# FUNCTION:
#
# Certain fixed constants and global variables are stored
# in low memory (below 64K) because:
#
#	1. No base register is required to access them.
#	2. Oft used variables are at well known locations which
#	   remained invariant on different system builds.
#
# The "fixed constant" pool is located past the hardware-related
# interrupt addresses.
#
#****************************************************************************

		.set    FETCHPROT_PAGE,    0x5000
		.org    real0+FETCHPROT_PAGE
DATA(fetchprot_page):

#	al_flih assumes quad word alignment of its save area in the PPDA
#	so be careful about changing this org

                .org    real0+FETCHPROT_PAGE+0x00c0

ifdef(`_POWER_MP',`
		.space ppdend-ppdarea',`

DATA(ppda):
		.space	ppda_csa-ppdarea
DATA(csa):
		.long	0
DATA(g_mstack):
		.long	0
DATA(fp_owner):
		.long	0
ifdef(`_THREADS',`
DATA(curthread):
		.long	0
		.space	ppda_softis-ppda_curthread-4
',`
DATA(curproc):
		.long	0
		.space	ppda_softis-ppda_curproc-4
') #endif _THREADS
DATA(softis):
		.short  0
DATA(softpri):
		.short  0
		.space	ppdend-ppda_softis-4

') #endif _POWER_MP

DATA(curid):    .long   0               # current id (process or slih)

DATA(runrun):	.long	0               # process switch requested flag

DATA(pin_com_start):
	        .long   low_com_end	# hold address of ".lcomm low_com_end"
DATA(g_toc):    .long   0               # kernel TOC pointer
DATA(g_data):	.long	0		# ptr to intrupt hdler struct for rs1/2
DATA(g_svr0):   .long   0               # FLIH save area for gpr 0
DATA(g_svr1):   .long   0               # FLIH save area for gpr 1
DATA(g_svr2):   .long   0               # FLIH save area for gpr 2
DATA(g_svcr):   .long   0               # data storage FLIH save area for CR
DATA(g_segr0):  .long   0               # save area for seg reg 0
DATA(g_segr2):  .long   0               # save area for seg reg 2

DATA(g_abndc):  .long   0               # abend code
DATA(g_sysst):  .long   0               # system status flag
DATA(g_config): .long	0               # system configuration data

DATA(phantom_int):			# phantom interrupt count, only ctlr
DATA(phantom_pri):			# phantom interrupt count, primary ctlr
		.long	0
DATA(phantom_sec):			# phantom interrupt count, second. ctlr
		.long	0		# this must remain at phantom_pri + 4!
DATA(intctl_pri):			# Primary interrupt controller type
		.short	0
DATA(intctl_sec):			# Secondary int. controller type
		.short	0
DATA(i8259_base):			# Base address (w/o seg), 8259s
		.long	0
DATA(mpic_base):			# Real address, MPIC
		.long	0xBF000000	#  default value (Victory class)

DATA(key_value): .long	0		# sr key value

DATA(trtScon):                          # trace table pointers
trace_start:    .long   0               # start of the table
trace_end:      .long   0               # end of the table
trace_current:  .long   0               # current entry in the table
DATA(dummy):	.long	0		# for clearing reservations
DATA(ipl_cb):	.long	0		# ipl control block pointer
DATA(ipl_cb_ext): .long 0		# extended ipl control block pointer

DATA(svc_table):
	.long   0			# filled in by init_ldr
DATA(svc_table_entries):
	.long   0			# filled in by init_ldr
DATA(ra_save):	.long	0		# temporary save area
DATA(proc_arr_addr):
		.long	0
ifdef(`FLIH_LEDS',`
DATA(int_count):
		.long	0
',)
ifdef(`_POWER_MP',`
DATA(ppda_p_tab):	
		.space	MAXCPU*4
',) # _POWER_MP

ifdef(`_KDB',`
DATA(kdb_ds_wanted):	.long	0		# ds_flih send to kdb
DATA(kdb_ra_save):	.long	0		# temporary save area
') #endif _KDB

# Space reserved for applying patches to the kernel.
                .org    real0+FETCHPROT_PAGE+0x0400
DATA(kernpatch):
		.space	0x400		# reserve 1K bytes

# Space originally reserved for kernel trace buffer
# Also used by kernel dump for a page number table

                .org    real0+FETCHPROT_PAGE+0x0800
DATA(kerntrc):
		.space	0x800		# reserve 2K bytes
