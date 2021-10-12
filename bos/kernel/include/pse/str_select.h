/* @(#)03       1.4  src/bos/kernel/include/pse/str_select.h, sysxpse, bos411, 9437B411a 9/7/94 06:52:11 */
/*
 * COMPONENT_NAME: SYSXPSE - Streams framework
 *
 * ORIGINS: 83
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */


#ifndef _STR_POLL_H
#define _STR_POLL_H

#include <pse/str_lock.h>

/*
 *      File:   queue.h
 *
 *      Type definitions for generic queues.
 *
 *      Queue of abstract objects.  Queue is maintained
 *      within that object.
 *
 *      Supports fast removal from within the queue.
 *
 *      How to declare a queue of elements of type "foo_t":
 *              In the "*foo_t" type, you must have a field of
 *              type "queue_chain_t" to hold together this queue.
 *              There may be more than one chain through a
 *              "foo_t", for use by different queues.
 *
 *              Declare the queue as a "queue_t" type.
 *
 *              Elements of the queue (of type "foo_t", that is)
 *              are referred to by reference, and cast to type
 *              "queue_entry_t" within this module.
 */

typedef struct queue_entry      *g_queue_t;
typedef struct queue_entry      queue_head_t;
typedef struct queue_entry      queue_chain_t;
typedef struct queue_entry      *queue_entry_t;

/*
 *      enqueue puts "elt" on the "queue".
 *      dequeue returns the first element in the "queue".
 */

#define enqueue(queue,elt)      enqueue_tail(queue, elt)
#define dequeue(queue)          dequeue_head(queue)

extern void             enqueue_head();
extern queue_entry_t    dequeue_head();
extern void             enqueue_tail();
extern queue_entry_t    dequeue_tail();

extern queue_entry_t    remque();
extern void             insque();

/*
 *      Macro:          queue_first
 *      Function:
 *              Returns the first entry in the queue,
 *      Header:
 *              queue_entry_t queue_first(q)
 *                      queue_t q;
 */
#define queue_first(q)  ((q)->next)

/*
 *      Macro:          queue_next
 *      Header:
 *              queue_entry_t queue_next(qc)
 *                      queue_t qc;
 */
#define queue_next(qc)  ((qc)->next)

/*
 *      Macro:          queue_end
 *      Header:
 *              boolean_t queue_end(q, qe)
 *                      queue_t q;
 *                      queue_entry_t qe;
 */
#define queue_end(q, qe)        ((q) == (qe))

#define queue_empty(q)          queue_end((q), queue_first(q))

#endif  /* _STR_POLL_H */

