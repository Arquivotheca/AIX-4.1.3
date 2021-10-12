/* @(#)71   1.1  src/bos/kernext/cie/mbqueue.h, sysxcie, bos411, 9428A410j 4/18/94 16:23:26 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 * DESCRIPTION:
 * 
 *   mbuf Queue Management
 * 
 * ORIGINS: 27
 * 
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994    
 *   All Rights Reserved                                               
 *   Licensed Materials - Property of IBM                              
 *                                                                   
 *   US Government Users Restricted Rights - Use, duplication or       
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
 * 
 */

#if !defined(MBQUEUE_H)
#define  MBQUEUE_H

/*---------------------------------------------------------------------------*/
/*                      mbuf Queue Management Structure                      */
/*---------------------------------------------------------------------------*/

typedef struct MBQUEUE       MBQUEUE;

struct MBQUEUE
{
   mbuf_t                  * head;
   mbuf_t                 ** tail;
   unsigned short            limit;
   unsigned short            size;
   unsigned short            hwm;
};

#endif
