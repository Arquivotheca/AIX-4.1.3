/* @(#)52   1.6  src/bos/kernext/cie/mibaddr.h, sysxcie, bos411, 9428A410j 4/1/94 15:50:24 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   first_mib_addr
 *   next_mib_addr
 *   mib_addr_count
 * 
 * DESCRIPTION:
 * 
 *    MIB Address Block Macro
 * 
 * ORIGINS: 27
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */

#if !defined(MIB_ADDR_H)
#define  MIB_ADDR_H

/******************************************************************************
*                                                                             *
*  Macro:                                                                     *
*                                                                             *
*     first_mib_addr                                                          *
*                                                                             *
*  Description:                                                               *
*                                                                             *
*     Given a pointer to a ndd_mib_addr structure, this macro returns         *
*     a pointer to first ndd_mib_addr_elem_t contained in the                 *
*     ndd_mib_addr.                                                           *
*                                                                             *
******************************************************************************/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

ndd_mib_addr_elem_t *
   first_mib_addr(
      ndd_mib_addr_t * a       // Ptr to ndd_mib_addr structure
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  first_mib_addr(a)    \
   ((ndd_mib_addr_elem_t *)(((ndd_mib_addr_t *)(a))->mib_addr))

/******************************************************************************
*                                                                             *
*  Macro:                                                                     *
*                                                                             *
*     next_mib_addr                                                           *
*                                                                             *
*  Prototype:                                                                 *
*                                                                             *
*  Description:                                                               *
*                                                                             *
*     Given a pointer to a ndd_mib_addr_elem_t within a ndd_mib_addr          *
*     structure, this macro returns a pointer to the next                     *
*     ndd_mib_addr_elem_t.                                                    *
*                                                                             *
******************************************************************************/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

ndd_mib_addr_elem_t *
   next_mib_addr(
      ndd_mib_addr_elem_t * e  // Ptr to current ndd_mib_addr_elem_t
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/


#define  next_mib_addr(e)         \
   ((ndd_mib_addr_elem_t *)       \
      ((char *)(e) + (((ndd_mib_addr_elem_t *)(e))->addresslen) + offsetof(ndd_mib_addr_elem_t,address)))

/******************************************************************************
*                                                                             *
*  Macro:                                                                     *
*                                                                             *
*     mib_addr_count                                                          *
*                                                                             *
*  Prototype:                                                                 *
*                                                                             *
*  Description:                                                               *
*                                                                             *
*     Given a pointer to a ndd_mib_addr_elem_t within a ndd_mib_addr          *
*     structure, this macro returns a pointer to the next                     *
*     ndd_mib_addr_elem_t.                                                    *
*                                                                             *
******************************************************************************/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

int
   mib_addr_count(
      ndd_mib_addr_t        t  // Ptr to ndd_mib_addr_t structure
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/


#define  mib_addr_count(t)        (((ndd_mib_addr_t *)(t))->count)

#endif
