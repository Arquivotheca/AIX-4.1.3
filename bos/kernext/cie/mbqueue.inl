/* @(#)72   1.2  src/bos/kernext/cie/mbqueue.inl, sysxcie, bos411, 9428A410j 4/19/94 14:00:20 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   mbqInit
 *   mbqQueue
 *   mbqDeQueue
 *   mbqEmpty
 *   mbqSize
 *   mbqHWM
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

#if !defined(MBQUEUE_INL)
#define  MBQUEUE_INL

/*---------------------------------------------------------------------------*/
/*                      Initialize an MBQUEUE structure                      */
/*---------------------------------------------------------------------------*/

static
void
   mbqInit(
      MBQUEUE              * q           ,// IO-The mbuf Queue
      int                    limit        // I -Queue Size Limit
   )
{
   q->head  = NULL;
   q->tail  = &(q->head);
   q->limit = limit;
   q->size  = 0;
   q->hwm   = 0;
}

/*---------------------------------------------------------------------------*/
/*                         Add an mbuf to an MBQUEUE                         */
/*---------------------------------------------------------------------------*/

static
mbuf_t *
   mbqQueue(
      MBQUEUE              * q           ,// IO-The queue
      mbuf_t               * m            // IO-The mbuf to be queued
   )
{
   if (q->size >= q->limit) return NULL;

   q->size ++;

   *(q->tail)   = m;
   m->m_nextpkt = NULL;
   q->tail      = &m->m_nextpkt;
   q->hwm       = (q->size > q->hwm) ? q->size : q->hwm;

   return m;
}

/*---------------------------------------------------------------------------*/
/*                   Remove the first mbuf from the queue                    */
/*---------------------------------------------------------------------------*/

static
mbuf_t *
   mbqDeQueue(
      MBQUEUE              * q            // IO-The queue
   )
{
   mbuf_t                  * m;

   if (q->size == 0) return NULL;

   m       = q->head;
   q->head = m->m_nextpkt;
   if (--q->size == 0) q->tail = &q->head;

   return m;
}

/*---------------------------------------------------------------------------*/
/*                     Determine if mbuf queue is empty                      */
/*---------------------------------------------------------------------------*/

#define  mbqEmpty(q) ((q)->size == 0)

/*---------------------------------------------------------------------------*/
/*                    Return the current mbuf queue size                     */
/*---------------------------------------------------------------------------*/

#define  mbqSize(q)  ((q)->size)

/*---------------------------------------------------------------------------*/
/*         Return the mbuf queue high-water-mark (max mbufs queued)          */
/*---------------------------------------------------------------------------*/

#define  mbqHWM(q)   ((q)->hwm)

#endif
