static char sccsid[]="@(#)30   1.7  src/bos/kernext/cie/queue.c, sysxcie, bos411, 9428A410j 4/18/94 16:21:59";

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   new_QUEUE
 *   free_QUEUE
 *   xNew
 *   xQueue
 *   xPush
 *   xInsert
 *   xRemove
 * 
 * DESCRIPTION:
 * 
 *    General-purpose queue routines
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

#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/malloc.h>
#include <sys/syspest.h>

#include "queue.h"
#include "dmalloc.h"
#include "trccap.h"

/*---------------------------------------------------------------------------*/
/*                 Allocate a Variable-Length Circular Queue                 */
/*---------------------------------------------------------------------------*/

QHANDLE
   new_QUEUE(
      register int           size        ,// I - Number of nodes to prealloc
      register int           allocType    // I - Allocation Flags
   )
{
   FUNC_NAME(new_QUEUE);

   register QHANDLE          q;

   if (size == 0)
   {

      if ((q = xmalloc(sizeof(QUEUE),3,pinned_heap)) == NULL) return NULL;

      q->head      = NULL;
      q->free      = NULL;
      q->size      = 0;
      q->allocType = allocType;
   }
   else
   {
      register CURSOR        e;
      register int           bytes = sizeof(QUEUE) + (size+1)*sizeof(QNODE);
      register int           i;

      if ((q = xmalloc(bytes,3,pinned_heap)) == NULL) return NULL;

      e = (CURSOR)(q+1);

      q->head      = NULL;
      q->free      = e;
      q->size      = 0;
      q->allocType = allocType;

      for (i=0; i<size; i++)
      {
         e[i].prev  = NULL;
         e[i].next  = &(e[i+1]);
         e[i].data  = NULL;
         e[i].flags = QNODE_PREALLOC;
      }

      e[size-1].next = NULL;
   }

   return q;
}

/*---------------------------------------------------------------------------*/
/*                 Destroy a Variable-Length Circular Queue                  */
/*---------------------------------------------------------------------------*/

void
   free_QUEUE(
      register QHANDLE       q            // IO-Queue to be freed
   )
{
   FUNC_NAME(free_QUEUE);

   int                       rc = 0;

   if (q)
   {
      CURSOR                 e;
      CURSOR                 next;

      if (q->head)
      {
         /*-----------------------------------------------------*/
         /*  Break the forward chain just before the head node  */
         /*-----------------------------------------------------*/

         q->head->prev->next = NULL;

         /*---------------------------------------------------*/
         /*  Walk the forward chain, deleting nodes as we go  */
         /*---------------------------------------------------*/

         for (e=q->head; e; e=next)
         {
            next = e->next;

            if (!(e->flags & QNODE_PREALLOC))
               assert(xmfree(e,pinned_heap)==0);
         }
      }

      /*----------------------------------------*/
      /*  Free any nodes left on the free list  */
      /*----------------------------------------*/

      for (e=q->free; e; e=next)
      {
         next = e->next;

         if (!(e->flags & QNODE_PREALLOC))
            assert(xmfree(e,pinned_heap) == 0);
      }

      /*--------------------------------*/
      /*  Free the queue anchor itself  */
      /*--------------------------------*/

      assert(xmfree(q,pinned_heap)==0);
   }
}

/*---------------------------------------------------------------------------*/
/*           Allocate a new Variable-Length Circular Queue Node              */
/*---------------------------------------------------------------------------*/

CURSOR
   xNew(
      register QHANDLE       q           ,// IO-The queue
      register const void  * data         // I -Data pointer to be stored
   )
{
   FUNC_NAME(xNew);

   CURSOR                    e;

   /*------------------------------------------------------------------------*/
   /*  If free list is not empty use free node; otherwise make a new one     */
   /*------------------------------------------------------------------------*/

   if ((e = q->free) != NULL)
   {
      q->free = e->next;
      e->data = data;
      e->next = e;
      e->prev = e;
   }
   else if (q->allocType == QUEUE_DYNAMIC)
   {
      e = xmalloc(sizeof(*e),2,pinned_heap);

      if (e)
      {
         /*-------------------------------------------------------------*/
         /*  Initialize the node as a single-node circular queue        */
         /*-------------------------------------------------------------*/

         e->data  = data;
         e->flags = 0;
         e->next  = e;
         e->prev  = e;
      }
   }

   return e;
}

/*---------------------------------------------------------------------------*/
/*              Add a new entry at the end of the queue (FIFO)               */
/*---------------------------------------------------------------------------*/

CURSOR
   xQueue(
      register QHANDLE       q           ,
      register const void  * d
   )
{
   CURSOR                    c;

   if ((c=xNew(q,d)) == NULL) return NULL;

   if (q->size++ == 0)
      q->head = c;
   else
   {
      register CURSOR        pred = q->head->prev;

      c->prev       = pred;
      c->next       = q->head;
      pred->next    = c;
      q->head->prev = c;
   }

   return c;
}

/*---------------------------------------------------------------------------*/
/*             Add a new entry at the front of the queue (LIFO)              */
/*---------------------------------------------------------------------------*/

CURSOR
   xPush(
      register QHANDLE       q           ,
      register const void  * d
   )
{
   CURSOR                    c;

   if ((c=xNew(q,d)) == NULL) return NULL;

   if (q->size++ == 0)
      q->head = c;
   else
   {
      register CURSOR        pred = q->head->prev;

      c->prev       = pred;
      c->next       = q->head;
      pred->next    = c;
      q->head->prev = c;
      q->head       = c;
   }

   return c;
}

/*---------------------------------------------------------------------------*/
/*                Insert a new entry in the middle of a queue                */
/*---------------------------------------------------------------------------*/

CURSOR
   xInsert(
      register QHANDLE       q           ,
      register CURSOR        p           ,
      register const void  * d
   )
{
   CURSOR                    c;

   assert(q->size++ > 0);

   if ((c=xNew(q,d)) == NULL) return NULL;

   c->prev       = p;
   c->next       = p->next;
   p->next       = c;
   c->next->prev = c;

   return c;
}

/*---------------------------------------------------------------------------*/
/*                  Remove an arbitrary entry from a queue                   */
/*---------------------------------------------------------------------------*/

void *
   xRemove(
      register QHANDLE       q           ,// IO-The queue
      register CURSOR        c            // IO-Node to be removed
   )
{
   if (--q->size == 0)                     // Will queue be empty?
      q->head = NULL;                      //    Set head to NULL
   else                                    // Otherwise
   {                                       //
      c->prev->next = c->next;             //    Detach the node from...
      c->next->prev = c->prev;             //    ...the queue
      if (q->head == c) q->head = c->next; //    Adjust head if necessary
   }

   c->next = q->free;                      // Attach node to free list
   q->free = c;

   return c->data;
}
