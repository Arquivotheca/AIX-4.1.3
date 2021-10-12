# @(#)83        1.2  src/bos/kernel/ml/POWER/dispauth.m4, sysml, bos411, 9428A410j 5/5/94 19:12:10
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

		.dsect	gruprt

gp_next:	.long	0	# pointer to next gruprt for process/thread
gp_busprt:      .long   0       # pointer to the first busprt structure in group
gp_owner:	.long	0	# pointer to owning process/thread structure
gp_type:	.byte	0	# access control type
gp_primxid:	.byte	0	# primary xid display
gp_own:         .short  0       # number of busprt owned in this group
gp_lock:	.long	0	# simple lock, reserved for SMPs
gp_resv:	.long	0	# reserved for 64 bit processors
gp_eaddr:	.long	0	# 32 bit or 64 bit effective address
gp_nbat:	.long   0	# number of BAT type gruprts in list
gp_acw0:	.long	0	# access control words
gp_acw1:	.long	0
gp_acw2:	.long	0

		# masked value for the type field in gruprt structure
		.set	DISP_NOACC,0	       # no access 
	        .set    CSR15TYPE,0x08	       # Power Micro Channel display
		.set    SRMCTYPE,0x10	       # POWER PC Micro Channel display
		.set	PBUSTYPE,0x18	       # processor bus type display 
		.set	BATTYPE,0x20	       # generic bat type
		.set    XID7FTYPE,0x1B	       # 7FXID type
		.set    XIDBATTYPE,0x21        # BATXID type
		.set	DISP_MASK,0xF8	       # display type mask	

		# segment register value that ensures exception when accessing
		# Power PC Micro Channel display
		.set 	INV_MC_SEGREG,0xE200   # T=1, Ks=Kp=1, BUID=0x20
