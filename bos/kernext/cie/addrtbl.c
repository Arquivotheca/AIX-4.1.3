static char sccsid[]="@(#)12   1.8  src/bos/kernext/cie/addrtbl.c, sysxcie, bos411, 9428A410j 4/26/94 10:46:21";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   new_ADDRTABLE
 *   free_ADDRTABLE
 *   find
 *   alloc
 *   dealloc
 *   enableGroup
 *   disableGroup
 *   disableAllGroups
 *
 * DESCRIPTION:
 *
 *    Group/Multicast Address Tracking
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

#include <sys/m_param.h>
#include <sys/malloc.h>
#include <sys/ndd.h>
#include <sys/syspest.h>

#include <stddef.h>
#include <errno.h>

#include "ciedd.h"
#include "addrtbl.h"
#include "dmalloc.h"

/*---------------------------------------------------------------------------*/
/*     Allocate a new address table tSize elements with addr aBytes long     */
/*---------------------------------------------------------------------------*/

ADDRTABLE *
   new_ADDRTABLE(
      ushort                 tSize       ,
      ushort                 aBytes
   )
{
   FUNC_NAME(new_ADDRTABLE);

   int                       eBytes;
   int                       tBytes;

   register ADDRTABLE      * t;
   register ADDRENTRY      * e;
   register ADDRENTRY     ** p;
   register int              i;

   /*-----------------------------------------------------------*/
   /*  Calculate size of a single entry, rounded up to 8 bytes  */
   /*-----------------------------------------------------------*/

   eBytes = (aBytes + offsetof(ADDRENTRY,addr) + 3) & 0xFFFFFFFC;

   /*---------------------------------------------*/
   /*  Calculate total size of the address table  */
   /*---------------------------------------------*/

   tBytes = offsetof(ADDRTABLE,e) + ((tSize) * eBytes);

   /*-------------------*/
   /*  Allocate memory  */
   /*-------------------*/

   if ((t = (ADDRTABLE *)xmalloc(tBytes,2,pinned_heap)) == NULL)
   {
      XMFAIL(tBytes);
      return NULL;
   }

   /*----------------------------------------------------*/
   /*  Initialize the base portion of the address table  */
   /*----------------------------------------------------*/

   memset(t,0x00,tBytes);

   t->tSize     = tSize;
   t->aBytes    = aBytes;
   t->eBytes    = eBytes;
   t->freeList  = NULL;
   t->allocList = NULL;

   /*----------------------------------------------------------------------*/
   /*  Initialize the table proper, putting all elements in the free list  */
   /*----------------------------------------------------------------------*/

   p = &(t->freeList);       // Ptr to forward link pointer
   e = &(t->e[0]);           // Ptr to first element

   for (i = 0; i < tSize; i++)
   {
      *p = e;                // Set forward link from previous element

      p  = &(e->next);
      e  = (ADDRENTRY *)((char *)(e) + eBytes);
   }

   *p = NULL;

   return t;
}

/*---------------------------------------------------------------------------*/
/*                         Destroy an address table                          */
/*---------------------------------------------------------------------------*/

void
   free_ADDRTABLE(
      ADDRTABLE            * t
   )
{
   FUNC_NAME(free_ADDRTABLE);

   if (xmfree(t,pinned_heap) != 0) XFFAIL();
}

/*---------------------------------------------------------------------------*/
/*      Find an address in the table and move it to head of alloc list       */
/*---------------------------------------------------------------------------*/

static
ADDRENTRY *
   findAddr(
      register ADDRTABLE   * t           ,// IO-The Address Table
      register const char  * addr         // I -Address to be found
   )
{
   FUNC_NAME(findAddr);

   register ADDRENTRY      * e;
   register ADDRENTRY      * p;

   for (p=NULL, e=t->allocList; e; p=e, e=e->next)
      if (memcmp(addr,e->addr,t->aBytes) == 0) break;

   if (e && p)
   {
      p->next      = e->next;
      e->next      = t->allocList;
      t->allocList = e;
   }

   return e;
}

/*---------------------------------------------------------------------------*/
/*      Allocate a new address entry - move it from freeList to allocList    */
/*---------------------------------------------------------------------------*/

static
ADDRENTRY *
   alloc(
      register ADDRTABLE   * t            // IO-The Address Table
   )
{
   FUNC_NAME(alloc);

   register ADDRENTRY      * e = t->freeList;

   if (e)
   {
      t->freeList  = e->next;

      e->next      = t->allocList;
      t->allocList = e;

      e->refCount  = 0;
      memset(e->addr,0x00,t->aBytes);
   }

   return e;
}

/*---------------------------------------------------------------------------*/
/*     Deallocate an address entry - move it from allocList to freeList      */
/*---------------------------------------------------------------------------*/

static
ADDRENTRY *
   dealloc(
      register ADDRTABLE   * t            // IO-The Address Table
   )
{
   FUNC_NAME(dealloc);

   register ADDRENTRY      * e = t->allocList;

   if (e)
   {
      t->allocList = e->next;

      e->next      = t->freeList;
      t->freeList  = e;
   }

   return e;
}

/*---------------------------------------------------------------------------*/
/*                     Enable a Group/Multicast Address                      */
/*---------------------------------------------------------------------------*/

int
   enableGroup(
      register ndd_t       * nddp        ,// I -NDD
      register ADDRTABLE   * t           ,// IO-The Address Table
      register int         * chnRef      ,// IO-Channel's Address Ref Table
      register const char  * addr         // I -Group Addr to be enabled
   )
{
   FUNC_NAME(enableGroup);

   register ADDRENTRY      * e;
   register int              rc = 0;

   /*----------------------------------------------*/
   /*  Look for the address in the existing table  */
   /*----------------------------------------------*/

   if ((e = findAddr(t,addr)) == NULL)
   {
      /*---------------------------------------------------------*/
      /*  Address not currently present -- allocate a new entry  */
      /*---------------------------------------------------------*/

      if ((e = alloc(t)) == NULL)
      {
         TRC_OTHER(egas,0,0,0);
         rc = ENOSPC;
      }
      else
      {
         /*---------------------------------*/
         /*  Enable the address in the NDD  */
         /*---------------------------------*/

         rc = NDD_CTL(                   /* macro */
                   nddp,
                   NDD_ENABLE_ADDRESS,
                   addr,
                   t->aBytes);

         /*----------------------------------------------------------------*/
         /*  If enabled ok, save the info; otherwise deallocate the entry  */
         /*----------------------------------------------------------------*/

         if (rc == 0)
         {
            e->refCount++;
            chnRef[eIndex(t,e)]++;
            memcpy(e->addr,addr,t->aBytes);
         }
         else
         {
            TRC_OTHER(ndeg,rc,NETADDR1(addr),NETADDR2(addr));
            dealloc(t);
         }
      }
   }
   else
   {
      /*-------------------------------------------------------*/
      /*  Existing address -- just increment reference counts  */
      /*-------------------------------------------------------*/

      e->refCount++;
      chnRef[eIndex(t,e)]++;
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                     Disable a Group/Multicast Address                     */
/*---------------------------------------------------------------------------*/

int
   disableGroup(
      register ndd_t       * nddp        ,// I -NDD
      register ADDRTABLE   * t           ,// IO-The Address Table
      register int         * chnRef      ,// IO-Channel's Address Ref Table
      register const char  * addr         // I -Group Addr to be enabled
   )
{
   FUNC_NAME(disableGroup);

   register ADDRENTRY      * e;
   register int              rc = 0;

   /*----------------------------------------------*/
   /*  Look for the address in the existing table  */
   /*----------------------------------------------*/

   if ((e = findAddr(t,addr)) == NULL)
   {
      rc = ENOCONNECT;
      TRC_OTHER(nddf,rc,NETADDR1(addr),NETADDR2(addr));
   }
   else
   {
      /*-----------------------------*/
      /*  Decrement Reference Count  */
      /*-----------------------------*/

      e->refCount--;
      chnRef[eIndex(t,e)]--;

      /*-------------------------------------------------------*/
      /*  If no references left (from any channel) disable it  */
      /*-------------------------------------------------------*/

      if (e->refCount == 0)
      {
         rc = NDD_CTL(                   /* macro */
                   nddp,
                   NDD_DISABLE_ADDRESS,
                   addr,
                   t->aBytes);

         if (rc) TRC_OTHER(nddg,rc,NETADDR1(addr),NETADDR2(addr));

         dealloc(t);
      }
   }

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                 Disable all group addresses for a channel                 */
/*---------------------------------------------------------------------------*/

int
   disableAllGroups(
      register ndd_t       * nddp        ,// I -NDD
      register ADDRTABLE   * t           ,// IO-The Address Table
      register int         * chnRef       // IO-Channel's Address Ref Table
   )
{
   FUNC_NAME(disableAllGroups);

   int                       rc  = 0;
   int                       trc = 0;
   int                       i;

   for (i=0; i<t->tSize; i++)
   {

      if (chnRef[i] > 0)
      {
         register ADDRENTRY * e = ePtr(t,i);
         /*
            The following statement uses the find() routine to locate
            the address table entry even though we have a direct index
            in chnRef[i].  This is so the table entry will be moved
            to the head of the allocated list, so dealloc() will
            deallocate the correct entry later.  Any implementation
            that allowed dealloc() to remove an arbitrary entry would
            have required a doubly-linked list, which I didn't think
            was worth the effort in this case given the low usage of
            the data structure.
         */

         e = findAddr(t,e->addr);

         if ((e->refCount -= chnRef[i]) == 0)
         {
            trc = NDD_CTL(                   /* macro */
                      nddp,
                      NDD_DISABLE_ADDRESS,
                      e->addr,
                      t->aBytes);

            if (rc == 0)
            {
               rc = trc;
               TRC_OTHER(nddk,rc,NETADDR1(e->addr),NETADDR2(e->addr));
            }

            dealloc(t);
         }
      }
   }

   return rc;
}
