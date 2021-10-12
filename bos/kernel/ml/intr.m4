# @(#)83	1.20  src/bos/kernel/ml/intr.m4, sysml, bos41B, 9504A 12/5/94 11:06:42
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
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
#       intr.m4 -- Interface definition to kernel interrupt management.      
#                                                                             
#*****************************************************************************
#
#  IMPORTANT NOTE:  This file is a subset of sys/intr.h.
#
#*****************************************************************************


# struct intr                           /* interrupt handler structure */
# {
#	struct intr     *next;          /* list of interrupt handlers  */
#	int             (*handler)();   /* interrupt handler           */
#	unsigned short	bus_type;       /* bus interrupt level type    */
#	unsigned short	flags;          /* miscellaneous attributes    */
#	int             level;          /* bus interrupt level         */
#	int             priority;       /* interrupt priority          */
#	unsigned long	bid;		/* parameter for BUSACC	       */
# };

        .dsect   intr
intr_next:      
       .long    0                       # list of interrupt handlers
intr_handler:   
       .long    0                       # function pointer to interrupt handler
intr_bus_type:  
       .short   0                       # bus interrupt level type          
intr_flags:
       .short   0                       # miscellaneous attributes
intr_level:
       .long    0                       # bus interrupt level
intr_priority:
       .long    0                       # intrrupt priority
intr_bid:
       .long    0                       # parameter for BUSACC

#
# values for intr_bus_type
#
	.set	BUS_NONE, 0
	.set	BUS_MICRO_CHANNEL, 1
	.set	BUS_PLANAR, 2
	.set	BUS_60X, 3
	.set	BUS_BID, 4

# values for intr_flags
	.set	INTR_EDGE, 0x20		# intr is edge-triggered
	.set	INTR_LEVEL, 0x40	# intr is level-triggered
 
#
# The following defines the interrupt priorities.
#
include(INCLML/i_machine.m4)
