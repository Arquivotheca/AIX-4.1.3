static char sccsid[]="@(#)15   1.7  src/bos/kernext/cie/chnlist.c, sysxcie, bos411, 9428A410j 4/18/94 16:20:37";

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
 *    Channel List Object Implementation
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

#include "ciedd.h"
#include "chnlist.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*                   Allocate a new Channel List Structure                   */
/*---------------------------------------------------------------------------*/

CHN_LIST *
   new_CHN_LIST(
      void
   )
{
   FUNC_NAME(new_CHN_LIST);

   register CHN_LIST       * chl;

   if ((chl = xmalloc(sizeof(CHN_LIST),2,pinned_heap)) != NULL)
   {
      strcpy(chl->iCatcher,"CHL");

      chl->size   = 0;
      chl->used   = 0;
      chl->free   = 0;
      chl->table  = NULL;
   }
   else
      XMFAIL(sizeof(CHN_LIST));

   return chl;
}

/*---------------------------------------------------------------------------*/
/*                     Destroy a Channel List Structure                      */
/*---------------------------------------------------------------------------*/

void
   free_CHN_LIST(
      register CHN_LIST    * chl          // IO-The Channel List to be freed
   )
{
   FUNC_NAME(free_CHN_LIST);

   register int              i;

   if (chl)
   {
      if (xmfree(chl->table,pinned_heap) != 0) XFFAIL();
      if (xmfree(chl,pinned_heap)        != 0) XFFAIL();
   }
}

/*---------------------------------------------------------------------------*/
/*                Expand the CHN_LIST's channel pointer array                */
/*---------------------------------------------------------------------------*/

int
   expand_CHN_LIST(
      register CHN_LIST    * chl         ,// IO-The Chn List to be expanded
      register int           incr         // I -Size Increment
   )
{
   FUNC_NAME(expand_CHN_LIST);

   CHN_LIST_ENTRY      (* oldTable)[] = chl->table;
   CHN_LIST_ENTRY      (* newTable)[] = NULL;

   register int           oldSize  = chl->size;
   register int           oldBytes = oldSize * sizeof(CHN_LIST_ENTRY);
   register int           newSize  = chl->size + incr;
   register int           newBytes = newSize * sizeof(CHN_LIST_ENTRY);

   register int           i;

   /*------------------------------------------------------*/
   /*  Allocate a new CHN pointer array, 8 entries larger  */
   /*------------------------------------------------------*/

   if ((newTable = xmalloc(newBytes,2,pinned_heap)) == NULL)
   {
      XMFAIL(newBytes);
      return ENOMEM;
   }

   if (oldTable)
   {
      /*-----------------------------------------------------*/
      /*  Copy existing the existing array to the new array  */
      /*-----------------------------------------------------*/

      memcpy(newTable,oldTable,oldBytes);

      /*---------------------------------------------*/
      /*  Free the memory occupied by the old table  */
      /*---------------------------------------------*/

      if (xmfree(oldTable,pinned_heap) != 0) XFFAIL();
   }

   /*------------------------------------------*/
   /*  Chain the new entries on the free list  */
   /*------------------------------------------*/

   for (i=oldSize; i < newSize; i++)
   {
      (*newTable)[i].nextFree = i+1;
      (*newTable)[i].chn      = NULL;
   }

   /*----------------------------------------------------------------*/
   /*  Adjust table pointer/size in CHN_LIST structure to new table  */
   /*----------------------------------------------------------------*/

   chl->size  = newSize;
   chl->table = newTable;

   return 0;
}
