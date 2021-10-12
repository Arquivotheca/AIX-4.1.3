# @(#)18	1.2  src/bos/kernel/ml/POWER/vminfo.m4, sysml, bos411, 9428A410j 6/15/90 19:03:05
#*****************************************************************************
#
# COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
#       Any changes to this file should also be reflected in:
#               ( com/inc/sys/vminfo.h )
#
#****************************************************************************

             .dsect   vminfo
pgexct:      .long    0    # count of page faults     	
pgrclm:      .long    0    # count of page reclaims 	
lockexct:    .long    0    # count of lockmisses	    	
backtrks:    .long    0    # count of backtracks	    	
pageins:     .long    0    # count of pages paged in  	
pageouts:    .long    0    # count of pages paged out 	
pgspgins:    .long    0    # count of pages paged in from paging space 	
pgspgouts:   .long    0    # count of pages paged out from paging space	
numsios:     .long    0    # count of start I/Os	    	
numiodone:   .long    0    # count of iodones	    	
zerofills:   .long    0    # count of zero filled pages 	
exfills:     .long    0    # count of exec filled pages	
scans:       .long    0    # count of page scans by clock 
cycles:      .long    0    # count of clock hand cycles	
pgsteals:    .long    0    # count of page steals	   	
freewts:     .long    0    # count of free frame waits	
extendwts:   .long    0    # count of extend XPT waits	
pendiowts:   .long    0    # count of pending I/O waits  	
