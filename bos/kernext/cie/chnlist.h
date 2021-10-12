/* @(#)38   1.6  src/bos/kernext/cie/chnlist.h, sysxcie, bos411, 9428A410j 4/1/94 15:47:22 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   new_CHN_LIST
 *   free_CHN_LIST
 *   expand_CHN_LIST
 * 
 * DESCRIPTION:
 * 
 *    Channel List Object External Interface
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

#if !defined(CHNLIST_H)
#define  CHNLIST_H

/*---------------------------------------------------------------------------*/
/*                          Channel List Structure                           */
/*---------------------------------------------------------------------------*/

struct CHN_LIST
{
   char                      iCatcher[4]; // Eye Catcher
   int                       size;        // Current size of channel ptr array
   int                       used;        // Number of entries in use
   int                       free;        // Index of free chain head
   CHN_LIST_ENTRY         (* table)[];    // Ptr to array
};

/*---------------------------------------------------------------------------*/
/*                            Channel List Entry                             */
/*---------------------------------------------------------------------------*/

struct CHN_LIST_ENTRY
{
   int                       nextFree;    // Ptr to next free element
   CHN                     * chn;         // Ptr to Channel Structure
};

/*---------------------------------------------------------------------------*/
/*                   Allocate a new Channel List Structure                   */
/*---------------------------------------------------------------------------*/

CHN_LIST *
   new_CHN_LIST(
      void
   );

/*---------------------------------------------------------------------------*/
/*                     Destroy a Channel List Structure                      */
/*---------------------------------------------------------------------------*/

void
   free_CHN_LIST(
      register CHN_LIST    * chl          // IO-The Channel List to be freed
   );

/*---------------------------------------------------------------------------*/
/*                Expand the CHN_LIST's channel pointer array                */
/*---------------------------------------------------------------------------*/

int
   expand_CHN_LIST(
      register CHN_LIST    * chl         ,// IO-The Chn List to be expanded
      register int           incr         // I -Size Increment
   );

#endif
