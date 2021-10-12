# @(#)22        1.2  src/bos/kernel/ml/POWER/dsisrtab.m4, sysml, bos411, 9428A410j 3/29/94 22:22:48
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
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#****************************************************************************
#
#	Table used by the Alignment Flih.  Indexed by DSISR bits 15-21, 
#	and contains the address of the appropriate code module for the
#	offending instruction.
#
#****************************************************************************

	.long	l_word			# 0x00	lwarx, lwz
	.long	trap			# 0x01	<ldarx>, 64-bit 
	.long	s_word			# 0x02	stw
	.long	trap			# 0x03	-
	.long	l_half			# 0x04	lhz
	.long	l_half			# 0x05	lha
	.long	s_half			# 0x06	sth
	.long	l_mul			# 0x07	lmw
	.long	f_l_single		# 0x08	lfs
	.long	f_l_double		# 0x09	lfd
	.long	f_s_single		# 0x0A	stfs
	.long	f_s_double		# 0x0B	stfd
	.long	f_l_quad		# 0x0C	lfq 
	.long	trap			# 0x0D	<ld, ldu, lmd, lwa>, 64-bit
	.long	f_s_quad		# 0x0E	stfq 
	.long	trap			# 0x0F	<std, stdu, stmd>, 64-bit
	.long	l_word_u		# 0x10	lwzu 
	.long	trap			# 0x11	-
	.long	s_word			# 0x12	stwu 
	.long	trap			# 0x13	-
	.long	l_half_u		# 0x14	lhzu 
	.long	l_half_u		# 0x15	lhau 
	.long	s_half			# 0x16	sthu 
	.long	s_mul			# 0x17	stmw 
	.long	f_l_single		# 0x18	lfsu 
	.long	f_l_double		# 0x19	lfdu 
	.long	f_s_single		# 0x1A	stfsu 
	.long	f_s_double		# 0x1B	stfdu 
	.long	f_l_quad		# 0x1C	lfqu 
	.long	trap			# 0x1D	-
	.long	f_s_quad		# 0x1E	stfqu 
	.long	trap			# 0x1F	-
	.long	trap			# 0x20	<ldx>, 64-bit
	.long	trap			# 0x21	-
	.long	trap			# 0x22	<stdx>, 64-bit
	.long	trap			# 0x23	-
	.long	l_str_cmp		# 0x24	lscbx, lscbx.
	.long	trap			# 0x25	<lwax>, 64-bit
	.long	trap			# 0x26	-
	.long	trap			# 0x27	-
	.long	l_str_ind		# 0x28	lswx 
	.long	l_str_imm		# 0x29	lswi 
	.long	s_str_ind		# 0x2A	stswx 
	.long	s_str_imm		# 0x2B	stswi 
	.long	trap			# 0x2C	-
	.long	trap			# 0x2D	-
	.long	trap			# 0x2E	-
	.long	trap			# 0x2F	-
	.long	trap			# 0x30	<ldux>, 64-bit
	.long	trap			# 0x31	-
	.long	trap			# 0x32	<stdux>, 64-bit
	.long	trap			# 0x33	-
	.long	trap			# 0x34	-
	.long	trap			# 0x35	<lwaux>, 64-bit
	.long	trap			# 0x36	-
	.long	trap			# 0x37	-
	.long	trap			# 0x38	-
	.long	trap			# 0x39	-
	.long	trap			# 0x3A	-
	.long	trap			# 0x3B	-
	.long	trap			# 0x3C	-
	.long	trap			# 0x3D	-
	.long	trap			# 0x3E	-
	.long	trap			# 0x3F	-
	.long	trap			# 0x40	-
	.long	trap			# 0x41	-
	.long	s_cond			# 0x42	stwcx. 
	.long	trap			# 0x43	<stdcx.>, 64-bit
	.long	trap			# 0x44	-
	.long	trap			# 0x45	-
	.long	trap			# 0x46	-
	.long	trap			# 0x47	-
	.long	l_word			# 0x48	lwbrx 
	.long	trap			# 0x49	-
	.long	s_word			# 0x4A	stwbrx 
	.long	trap			# 0x4B	-
	.long	l_half			# 0x4C	lhbrx 
	.long	trap			# 0x4D	-
	.long	s_half			# 0x4E	sthbrx 
	.long	trap			# 0x4F	-
	.long	trap			# 0x50	-
	.long	trap			# 0x51	<ldarx>, 64-bit
	.long	trap			# 0x52	-
	.long	trap			# 0x53	-
	.long	ext_word_in		# 0x54	eciwx
	.long	trap			# 0x55	-
	.long	ext_word_out		# 0x56	ecowx
	.long	trap			# 0x57	-
	.long	trap			# 0x58	-
	.long	trap			# 0x59	-
	.long	trap			# 0x5A	-
	.long	trap			# 0x5B	-
	.long	trap			# 0x5C	-
	.long	trap			# 0x5D	-
	.long	trap			# 0x5E	-
	.long	cache_op		# 0x5F	dcbz 
	.long	l_word			# 0x60	lwzx 
	.long	trap			# 0x61	-
	.long	s_word			# 0x62	stwx 
	.long	trap			# 0x63	-
	.long	l_half			# 0x64	lhzx 
	.long	l_half			# 0x65	lhax 
	.long	s_half			# 0x66	sthx 
	.long	trap			# 0x67	-
	.long	f_l_single		# 0x68	lfsx 
	.long	f_l_double		# 0x69	lfdx 
	.long	f_s_single		# 0x6A	stfsx 
	.long	f_s_double		# 0x6B	stfdx 
	.long	f_l_quad		# 0x6C	lfqx 
	.long	trap			# 0x6D	-
	.long	f_s_quad		# 0x6E	stfqx 
	.long	f_s_stfiwx		# 0x6F	stfiwx
	.long	l_word_u		# 0x70	lwzux 
	.long	trap			# 0x71	-
	.long	s_word			# 0x72	stwux 
	.long	trap			# 0x73	-
	.long	l_half_u		# 0x74	lhzux 
	.long	l_half_u		# 0x75	lhaux 
	.long	s_half			# 0x76	sthux 
	.long	trap			# 0x77	-
	.long	f_l_single		# 0x78	lfsux 
	.long	f_l_double		# 0x79	lfdux 
	.long	f_s_single		# 0x7A	stfsux 
	.long	f_s_double		# 0x7B	stfdux 
	.long	f_l_quad		# 0x7C	lfqux 
	.long	trap			# 0x7D	-
	.long	f_s_quad		# 0x7E	stfqux 
	.long	trap			# 0x7F	-
	
