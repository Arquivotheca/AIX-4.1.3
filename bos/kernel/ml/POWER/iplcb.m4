# @(#)85	1.10  src/bos/kernel/ml/POWER/iplcb.m4, sysml, bos411, 9434B411a 8/23/94 15:18:05
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
#
#       equates for IPL ROS data -- R2 version
#
	.set    ASCIIID,4*32            # Offset of ros ascii id in iplcb
	.set    rammapo,4*4		# Offset in ipl dir of offset to rbm
	.set    ipldir,ASCIIID		# Offset in iplcb to ipl directory
	.set    bytesperbit,11*4	# Offset to field in ipl info struct
	.set    iplinfoo,6*4		# Offset in ipl dir of offset to iplinfo
	.set    mapsz,5*4		# Offset in ipldir of map size info
	.set    totsiz,3*4		# Offset in ipldir of total size info
	.set    rosentry,9*4		# Offset to ROS warm ipl pointer
	.set    roslvl,945		# Offset to ROS level time stamp
        .set    IPLINFO,4*32+4*6        # offset to ipl_info_offset field
        .set    PROCINFO,4*32+4*130     # offset to processor_info_offset field
        .set    MODEL,4*13              # offset to model field from ipl_info
        .set    ICACHEBK,4*19           # offset to icache_block field 
        .set    DCACHEBK,4*20           # offset to dcache_block field 
        .set    RSC_MODEL,0x0200        # code for RSC model
        .set    RS2_MODEL,0x0400        # code for RS2 model
        .set    PPC_MODEL,0x0800        # code for PPC model
        .set    PROC_SIZE,4*2     	# offset to struct_size field from proc_info
        .set    PROC_PRESENT, 4*6       # offset to proc_present field from proc_info
	.set	RUNNING_AIX,1		# processor_present value
        .set    IMPLEMENT, 4*13         # offset to implement. field, proc_info
        .set    BOX_MASK,0x00FF		# model code is low byte of model field   
        .set    SFOOT_MODEL,0x004D	# low byte for sandalfoot models        
