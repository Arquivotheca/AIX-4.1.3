# @(#)89	1.104.3.28  src/bos/kernel/ml/POWER/low.s, sysml, bos41J, 9513A_all 3/23/95 17:39:52
#
#*******************************************************************************
#
# COMPONENT_NAME: (SYSML) machine language routines
#
# FUNCTIONS: 
#
# ORIGINS: 27, 83
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
#******************************************************************************
 
#
# LEVEL 1,  5 Years Bull Confidential Information
#

	.file "low.s"
	.machine "com"
	.using   low,0

#*******************************************************************************
#
# This moudule defines low memory starting at real address zero.  There are
# several reasons to put things in this module:
#
# 	1) ABI (millicode) etc
#	2) Hardware requirement (flih vector)
#	3) Overlay routines
#	4) Code/Data required to be accessible in user/kernel mode
#
# Do not alter this module without an approved Feature
#
# 00000000-000000FF	ALL		zero memory
# 00000100-000001FF	ALL		system reset interrupt
# 00000200-000002FF	ALL		machine check interrupt
# 00000300-000003FF	ALL		data storage interrupt
# 00000400-000004FF	ALL		instruction storage interrupt
# 00000500-000005FF	ALL		external interrupt
# 00000600-000006FF	ALL		alignment interrupt
# 00000700-000007FF	ALL		program interrupt
# 00000800-000008FF	ALL		floating point unavailable interrupt
# 00000900-000009FF	PPC		decrementer
#			RS1		reserved
#                   	RS2     	trace
# 00000A00-00000AFF	PPC, RS1 	reserved
#                     	RS2		floating point imprecise interrupt
#       		601		direct store error
# 00000B00-00000BFF	ALL		reserved
# 00000C00-00000CFF	RS1,RS2		reserved
#			PPC		system call
# 00000D00-00000DFF	RS1,RS2		reserved
#               	PPC             trace
# 00000E00-00000FFF   	ALL             reserved
# 00001000-00002FFF	RS1,RS2		1000-1FFF svc vectors
#                   			2000-2FFF unused
#               	PPC             implementation specific
# 00003000-00003100   	ALL       	IBM Copyright
# 00003100-000037FF  	ALL		user mode millicode routines
# 00003800-00004FFF	ALL		user mode kernel routines svc,cs,
#					kgetsig machine information
#					structure, user readable kernel vars
# 00005000-000053FF	ALL		low memory kernel varaibles
# 00005400-00005FFF	ALL		kernel patch area, kernel trace buffer
# 00006000-00007FFF	ALL		alignment handler, branch table
#
# 00008000-00008FFF     ALL		reserved for future non-privileged use
# 00009000-0000B3FF     ALL		system overlay area
#*******************************************************************************

include(pri.m4)
include(systemcfg.m4)
include(ppda.m4)
include(flihs.m4)
include(mstsave.m4)
include(overlay.m4)
include(macros.m4)


	.csect	ENTRY(low[PR])
low:
include(low.m4)
	.machine "any"
include(start.m4)
	.machine "com"

#*******************************************************************************
#
# This is only here for debugging.
#
#*******************************************************************************

ifdef(`DBI_FLIH',`
       .org     real0+0x700             # Program interrupt

	b	dbpr_flih
',)

include(svc_tab.m4)

#*******************************************************************************
#
# User readable low memory
#
#*******************************************************************************

define(`DEFINE_GLOBL_ENTRIES')
include(ufcp.m4)
include(svc_flih.m4)
include(kgetsig.m4)
include(fast_sc.m4)
include(utrc_svc.m4)
include(fmap_svc.m4)
include(fp_fpscrx.m4)
include(primxid_svc.m4)
include(thread_svc.m4)

#*******************************************************************************
#
# System overlay area
#
#*******************************************************************************

include(fcp.m4)
	.machine "any"
include(al_flih.m4)
	.machine "com"
include(branch.m4)
include(pioutil.m4)
include(sysoverlay.m4)

#*******************************************************************************
#
# Misc low memory definitions
#
#*******************************************************************************

include(dbi_flih.m4)
include(power_emul_ppc.m4)

        .toc
	TOCE(__ublock,data)
        TOCE(errno,data)
        TOCE(sysinfo,data)
        TOCE(audit_flag,data)
	TOCE(emulate_count,data)
ifdef(`_POWER_MP',`',`
	TOCE(fp_owner,data)
') #endif _POWER_MP

	.globl	start
	.globl  ENTRY(execexit)
	.globl  ENTRY(kgetsig)
	.globl  DATA(ipl_cb)
	.globl	DATA(ipl_cb_ext)
	.globl  DATA(pin_obj_start)
        .globl  DATA(pin_com_start)
	.globl  DATA(nonpriv_page)
	.globl  DATA(fetchprot_page)
	.globl  DATA(g_ksrval)
	.globl  DATA(g_kxsrval)
	.globl  DATA(Trconflag)
	.globl	DATA(_system_configuration)
	.globl	DATA(cs)
	.globl	DATA(_check_lock)
	.globl	DATA(_clear_lock)
	.globl	DATA(cmp_swap_index)
	.globl  DATA(g_copyr)
ifdef(`_POWER_MP',`',`
	.globl	DATA(ppda)
	.globl  DATA(csa)
	.globl  DATA(g_mstack)
	.globl	DATA(fp_owner)
	.globl	DATA(curthread)
') #endif _POWER_MP
	.globl  DATA(curid)
	.globl  DATA(runrun)
	.globl  DATA(g_toc)
	.globl  DATA(g_abndc)
	.globl  DATA(g_sysst)
	.globl  DATA(g_config)
	.globl  DATA(phantom_int)
	.globl	DATA(phantom_pri)
	.globl  DATA(phantom_sec)
	.globl  DATA(intctl_pri)
	.globl  DATA(intctl_sec)
	.globl	DATA(i8259_base)
	.globl	DATA(mpic_base)
	.globl  DATA(g_data)
	.globl  DATA(trtScon)
	.globl  DATA(svc_table)
	.globl  DATA(svc_table_entries)
	.globl  DATA(kernpatch)
	.globl	DATA(proc_arr_addr)
ifdef(`_POWER_MP',`
	.globl	DATA(ppda_p_tab)
') # _POWER_MP
	.globl	DATA(key_value)
	.globl	DATA(alignlo)
	.globl	DATA(alignhi)
	.globl	ENTRY(svc_instr)
	.globl	ENTRY(sc_flih)
	.globl	DATA(kerntrc)
	.globl	DATA(_fp_fpscrx)
	.globl  DATA(utrchook)
	.globl DATA(_set_primxid)
	.globl DATA(thread_userdata)
ifdef(`_KDB',`
	.globl	DATA(kdb_ds_wanted)
') #endif _KDB

include(machine.m4)
include(user.m4)
include(audit.m4)
include(proc.m4)
include(seg.m4)
include(scrs.m4)
include(errno.m4)
include(trchkid.m4)
include(sysinfo.m4)
include(context.m4)
include(dbg_codes.m4)
include(dispauth.m4)
