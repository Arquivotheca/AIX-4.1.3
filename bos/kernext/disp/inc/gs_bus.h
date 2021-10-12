/* @(#)80	1.2  src/bos/kernext/disp/inc/gs_bus.h, dispcfg, bos411, 9428A410j 5/10/94 08:28:30 */

#ifndef _H_GS_BUS
#define _H_GS_BUS

/*
 *   COMPONENT_NAME: (dispcfg)
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
*******************************************************************************
**                                                                           **
** Contains the bus access macros					     **
**                                                                           **
*******************************************************************************
******************************************************************************/


/******************************************************************************
*                                                                             *
*	Use these macros to access a T=0 bus such as the 60X bus or	      *
*	the PCI bus.							      *
*                                                                             *
******************************************************************************/

#define GS_ATTACHGRAPHICSBUS(io_map_ptr)			\
        (unsigned long)iomem_att(&(io_map_ptr));                \
        __iospace_eieio()

#define GS_DETACHGRAPHICSBUS(address)				\
        (void)iomem_det((void *)(address))


/******************************************************************************
*                                                                             *
*	Use these macros to access a T=1 bus such as the MCA bus.	      *
*                                                                             *
******************************************************************************/

#define GS_ATTACHMCABUS(vmhandle, offset)		        \
        (unsigned long)io_att((vmhandle), (offset));

#define GS_DETACHMCABUS(address)				\
        (void)io_det((void *)(address))


#endif /* _H_GS_BUS */

