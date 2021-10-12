# @(#)08        1.22  src/bos/kernel/ml/POWER/low.m4, sysml, bos411, 9428A410j 3/31/94 07:58:35

#*****************************************************************************
# COMPONENT_NAME: (SYSML) machine language routines
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#*****************************************************************************
 
#
# LEVEL 1,  5 Years Bull Confidential Information
#

#
# NAME: low.m4
#
# FUNCTION: Layout of low memory and constants used by low.s.
#

DATA(pin_obj_start):
real0:

# Set up a dummy function descriptor at location 0 so if anybody uses
# a null function pointer they get 0 as an iar and 0xBADFCA11 for a toc
# (they will branch to zero and execute 0x00000000 which is an invalid op) 

        .long   0
        .long   0xBADFCA11

# These fields are initialized by the tool that creates the IPL image.
# Very early in system initialization, these variables are moved into global
# variables in hardinit.c (with the exception of dbg_avail, which can not
# be in the init object---it lies in the pinned object).
#
DATA(lowcore_header_offset):
        .long   0                  # offset in IPL file to start of hdr
DATA(lowcore_ram_disk_start):
        .long   0                  # pointer to RAM disk in IPL file
DATA(lowcore_ram_disk_end):
        .long   0                  # pointer to RAM disk end + 1
DATA(lowcore_dbg_avail):
        .long   0                  # debugger available flags
DATA(lowcore_base_conf_start):
        .long   0                  # pointer to base config area in IPL image
DATA(lowcore_base_conf_end):
        .long   0                  # pointer to base config end
DATA(lowcore_base_conf_disk):
        .long   0                  # disk address of base config area

# This is used to reserve the first part of common for the
# loader information (so variables in the common area don't overlay
# the loader section).

        .lcomm  low_com_start,1024*1024
ifdef(`_KDB',`
# This is used to reserve the part of common for the symbol table 
# so common area do not overlay with symbols
        .lcomm  low_com_kdb,512*1024
') #endif _KDB

        .lcomm  low_com_end,0

include(INCL/sys/comlink.m4)

