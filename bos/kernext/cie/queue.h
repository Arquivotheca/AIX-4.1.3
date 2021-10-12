/* @(#)54   1.7  src/bos/kernext/cie/queue.h, sysxcie, bos411, 9428A410j 4/18/94 16:22:02 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:  (Prototypes)
 *
 *   new_QUEUE
 *   free_QUEUE
 *   qInsert
 *   qQueue
 *   qPush
 *   qRemove
 *   qDelete
 *   qPop
 *   qDrop
 *   qCurr
 *   qFirst
 *   qLast
 *   qNext
 *   qPrev
 *   qEmpty
 *   qSize
 *   xNew
 *   xQueue
 *   xPush
 *   xInsert
 *   xRemove
 *
 * DESCRIPTION:
 *
 *    This is the header file for queue.c, which defines a general-purpose
 *    double-ended queue class.
 *
 *    Queues supported by this class have the following features:
 *
 *      * The queue can be fixed-size (preallocated) or dynamic, or
 *        a combination of both.  Fixed-size queues can be used in
 *        interrupt-level processing.
 *
 *      * Contained objects to not have to 'know' they are contained
 *        in a queue.  The object (structure) does not have to include
 *        queue management pointers, and an object can be a member of
 *        many different queues simultaneously if desired, with no
 *        special coding within the object itself.
 *
 *      * A single queue can contain objects of different type; however
 *        it is up to the user's code to provide a method for
 *        determining the object's type at runtime.
 *
 *      * Queue operations have been optimized for speed with a
 *        combination of functions and macros. On a model 570,
 *        qPush, qQueue, qPop and qDrop each take between 0.4 and 0.6
 *        microseconds each when using preallocated nodes.
 *
 *      * A rudimentary form of polymorphism is provided in the node
 *        access macros.  Each macro that returns a data pointer
 *        requires that you provide the expected data type as a
 *        macro parameter, and casts the returned (void *) pointer
 *        to the specified type.  This will catch any type
 *        incompatibilities between the type coded in the macro
 *        call and the target of the pointer assignment.
 *
 *    This package is modeled after typical C++ container classes, but
 *    is written in C for the C programmer.
 *
 *    To use a queue you must first create it by calling new_QUEUE, which
 *    returns a queue 'handle' (typedef QHANDLE).  The queue handle
 *    uniquely identifies the queue, and is the first parameter to all
 *    queue manipulation functions and macros.
 *
 *    To reference individual queue nodes there is a second data type,
 *    the queue cursor (typedef CURSOR).  A cursor identifies a specifc
 *    node in a queue, and the current contents of the node can be
 *    accessed via the qCurr function macro.  However, you will rarely
 *    use the qCurr function because all the other functions that
 *    manipulate cursors return the contents pointer of the affected
 *    cursor for convenience.
 *
 *    The following functions and macros are available:
 *
 *       Function: QHANDLE new_QUEUE(size,type);
 *
 *          Creates a new queue and returns the QHANDLE.  Size specifies
 *          the number of preallocated nodes, and type can be
 *          QUEUE_FIXED or QUEUE_DYNAMIC.
 *
 *       Function: void free_QUEUE(qhandle);
 *
 *          Frees all storage associated with a queue.
 *
 *       Macro: CURSOR qQueue(queue,object-ptr);
 *
 *          Queues a new object, identified by 'object-ptr', at the end of
 *          the queue (FIFO), and returns a valid cursor pointing to the
 *          new node.
 *
 *          If QUEUE_FIXED was specified to new_QUEUE and no more space is
 *          available, returns NULL instead.
 *
 *       Macro: CURSOR qPush(queue,object-ptr)
 *
 *          Pushes a new object, identified by 'object-ptr', at the front
 *          of the queue (LIFO), and returns a valid cursor pointing to the
 *          new node.
 *
 *          If QUEUE_FIXED was specified to new_QUEUE and no more space is
 *          available, returns NULL instead.
 *
 *       Macro: CURSOR qInsert(queue,pred-cursor,object-ptr);
 *
 *          Inserts a new object, identified by 'object-ptr', after the
 *          predecessor node identified by 'pred-cursor', and returns a
 *          valid cursor pointing to the new node.
 *
 *          If QUEUE_FIXED was specified to new_QUEUE and no more space is
 *          available, returns NULL instead.
 *
 *       Macro: <object-type> * qRemove(queue,object-cursor,object-type)
 *
 *          Removes the object identified by 'object-cursor' from the
 *          queue, and returns its data pointer cast to (object-type *).
 *          The object itself is not deleted, just its node in the queue.
 *
 *          The queue handle must identify a non-empty queue, and the
 *          object-cursor must identify a node that is a member of the
 *          queue.  If either of these conditions is violated, the results
 *          are undefined.
 *
 *          On return, object-cursor is not modified, but its value becomes
 *          undefined.  Attempting to use it subsequently without assigning
 *          it a valid value will produced undefined results.
 *
 *          Note:  This macro performs the same function as qDelete with
 *          the exception of returning the data pointer.  Use this macro
 *          when you want to remove the object from the queue and access it
 *          at the same time.
 *
 *       Macro: <object-type> * qPop(queue,object-type);
 *
 *          Removes the first node (LIFO) on the queue and returns its
 *          object pointer, cast to (object-type *).  Returns NULL if the
 *          queue is empty prior to calling qPop.
 *
 *       Macro: <object-type> * qDrop(queue,object-type);
 *
 *          Removes the last node (LIFO) on the queue and returns its
 *          object pointer, cast to (object-type *).  Returns NULL if the
 *          queue is empty prior to calling qPop.
 *
 *       Macro: void qDelete(queue,object-cursor);
 *
 *          Deletes the node identified by 'object-cursor' from the
 *          queue, and discards its data pointer.  The object itself
 *          is not deleted, just its node in the queue.
 *
 *          The queue handle must identify a non-empty queue, and the
 *          object-cursor must identify a node that is a member of the
 *          queue.  If either of these conditions is violated, the results
 *          are undefined.
 *
 *          On return, object-cursor is not modified, but its value becomes
 *          undefined.  Attempting to use it subsequently without assigning
 *          it a valid value will produced undefined results.
 *
 *          Note:  This macro performs the same function as qRemove with
 *          the exception of discarding the data pointer.  Use this macro
 *          when you want to remove the object from the queue and do
 *          not need to access it.  This macro is preferred in this case
 *          because you do not need to specify the object type for the
 *          pointer type cast.
 *
 *    Traversing the queue and examining nodes are accomplished via
 *    the following functions.  All these functions manipulate a
 *    cursor and simultaneously return the data pointer of the target
 *    node addressed by the cursor.
 *
 *       Macro: <object-type> * qFirst(q,object-cursor,object-type);
 *
 *          Sets object-cursor to point to the first node in the queue and
 *          returns the node's object-pointer cast to (object-type *).
 *
 *       Macro: <object-type> * qLast(q,object-cursor,object-type);
 *
 *          Sets object-cursor to point to the last node in the queue
 *          and returns the node's object-pointer cast to (object-type *).
 *
 *       Macro: <object-type> * qNext(q,object-cursor,object-type);
 *
 *          Advances object-cursor to the node immediately after the one
 *          currently identified by object-cursor. object-cursor must
 *          identify a node within the target queue.  If the current node
 *          is not the last node in the queue, qNext returns the next
 *          node's data pointer cast to (object-type *); if it *is* the
 *          last node, NULL is returned and object-cursor is set to point
 *          to the first node in the queue (wraparound).
 *
 *       Macro: <object-type> * qPrev(q,object-cursor,object-type);
 *
 *          Backs up object-cursor to the node immediately before the one
 *          currently identified by object-cursor. object-cursor must
 *          identify a node within the target queue.  If the current node
 *          is not the first node in the queue, qPrev returns the preceding
 *          node's data pointer cast to (object-type *); if it *is* the
 *          first node, NULL is returned and object-cursor is set to point
 *          to the last node in the queue (wraparound).
 *
 *       Macro: <object-type> * qCurr(object-cursor,object-type);
 *
 *          Given a valid cursor, qCurr returns the node's data pointer
 *          cast to (object-type *).  The cursor is not moved.
 *
 *       Macro: boolean qEmpty(q);
 *
 *          Returns TRUE (1) if the queue is empty, FALSE (0) otherwise.
 *
 * EXAMPLES:
 *
 *    1) Allocate a new queue handle for a dynamic queue, with 50
 *       preallocated nodes:
 *
 *          QHANDLE          q;
 *
 *          q = new_QUEUE(50,QUEUE_DYNAMIC);
 *
 *    2) Add an object to the queue
 *
 *          struct my_data   d;
 *
 *          qQueue(q,&my_data);
 *
 *    3) Search a queue for a particular entry and delete that
 *       entry if it is present
 *
 *          CURSOR           c;
 *          struct my_data * d;
 *
 *          for
 *          (
 *             d  = qFirst(q,c,struct my_data);
 *             c != NULL;
 *             d  = qNext(q,c,struct my_data)
 *          )
 *          {
 *             if (d->some_field == data_to_delete)
 *             {
 *                qDelete(q,c); // Note: cursor 'c' is no longer valid
 *                break;
 *             }
 *          }
 *
 *
 * IMPLEMENTATION NOTES:
 *
 *    The queue is maintained as a circular doubly-linked list, with an
 *    anchor structure containing a pointer to the head of the queue.
 *    By definition, the tail is the node immediately preceding the
 *    head (q->head->prev).  The head defined as the end where 'push'
 *    adds a node and 'pop' removes a node.  Analogous functions for
 *    the tail are 'queue' and 'drop'.
 *
 *    The queue nodes themselves contain only forward and back pointers
 *    plus a (void *) pointer that holds the address of the object
 *    'contained' in that node.
 *
 *    As nodes are removed from the queue, they are kept on a free list
 *    rather than being returned to the operating system, and new node
 *    allocations are taken from this free list when possible instead
 *    of acquiring heap storage.
 *
 *    An empty queue is represented by an anchor with a NULL head pointer.
 *
 *    The internal function xNew allocates a new queue node and stores
 *    a data pointer.  The new node's next and prev pointers are initally
 *    set to point to itself, so the new node is conceptually a single-
 *    node circular queue.  The routines that add nodes to a queue
 *    use this fact if the queue to which the node is being added is
 *    currently empty (i.e. all they have to do is set the queue anchor's
 *    head pointer to point to the new node).
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#if !defined(QUEUE_H)
#define  QUEUE_H

typedef struct QUEUE         QUEUE;      // Queue Anchor
typedef struct QNODE         QNODE;      // Queue Node

typedef QUEUE              * QHANDLE;    // Queue Handle
typedef QNODE              * CURSOR;     // Queue Cursor

/*---------------------------------------------------------------------------*/
/*                       Variable-Length Queue Header                        */
/*---------------------------------------------------------------------------*/

struct QUEUE
{
   CURSOR                    head;        // Ptr to head of list
   CURSOR                    free;        // Ptr to free list
   int                       size;        // Number of nodes in list
   int                       allocType;   // Allocation Type
   int                       pSave;       // Interrupt priority save area
};

#define  QUEUE_FIXED         0x00000001
#define  QUEUE_DYNAMIC       0x00000002

/*---------------------------------------------------------------------------*/
/*                               Queue Node                                  */
/*---------------------------------------------------------------------------*/

struct QNODE
{
   CURSOR                    prev;        // Ptr to previous node
   CURSOR                    next;        // Ptr to next node
   void                    * data;        // Ptr to 'contained' data
   int                       flags;       // Allocation Flags
};

#define  QNODE_PREALLOC      0x00000001

/*---------------------------------------------------------------------------*/
/*                 Allocate a Variable-Length Circular Queue                 */
/*---------------------------------------------------------------------------*/

QHANDLE
   new_QUEUE(
      register int           size        ,// I - Number of nodes to prealloc
      register int           allocType    // I - Allocation Flags
   );

/*---------------------------------------------------------------------------*/
/*                 Destroy a Variable-Length Circular Queue                  */
/*---------------------------------------------------------------------------*/

void
   free_QUEUE(
      register QHANDLE       q            // IO-Queue to be freed
   );

/*---------------------------------------------------------------------------*/
/*                Insert a new node after a given predecessor                */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

CURSOR
   qInsert(
      QHANDLE                q           ,// IO-Queue
      CURSOR &               p           ,// IO-Predecessor
      const <obj-type *>     d            // I -Ptr to node data
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qInsert(q,p,d)      \
   (xInsert((q),(p),(const void *)(d)))

/*---------------------------------------------------------------------------*/
/*              Queue a new node at the end of the queue (FIFO)              */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

CURSOR
   qQueue(
      QHANDLE                q           ,// IO-Queue
      const <obj-type *>     d            // I -Ptr to node data
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qQueue(q,d)         \
   (xQueue((q),(const void *)(d)))

/*---------------------------------------------------------------------------*/
/*             Push a new node at the front of the queue (LIFO)              */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qPush(
      QHANDLE                q           ,// IO-Queue
      const <obj-type *>     d            // I -Ptr to node data
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qPush(q,d)          \
   (xPush((q),(const void *)(d)))


/*---------------------------------------------------------------------------*/
/*                        Remove a node from a queue                         */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qRemove(
      QHANDLE                q           ,// IO-Queue
      CURSOR &               c           ,// IO-Cursor for node to remove
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qRemove(q,c,t)      \
   ((t *)xRemove((q),(c)))

#define  qDelete(q,c)        \
   ((void)xRemove((q),(c)))

/*---------------------------------------------------------------------------*/
/*          Remove first node from queue and return data pointer             */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qPop(
      QHANDLE                q           ,// IO-Queue
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qPop(q,t)           \
   ((t *)(((q)->head) ? xRemove((q),(q)->head) : NULL))

/*---------------------------------------------------------------------------*/
/*          Remove last node from queue and return data pointer              */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qDrop(
      QHANDLE                q           ,// IO-Queue
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qDrop(q,t)          \
   ((t *)(((q)->head) ? xRemove((q),(q)->head->prev) : NULL))

/*---------------------------------------------------------------------------*/
/*          Get data pointer from queue node addressed by cursor c           */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qCurr(
      CURSOR &               c           ,// IO-Cursor for node
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qCurr(c,t)          \
   ((t *)((c) ? (c->data) : NULL))

/*---------------------------------------------------------------------------*/
/*      Set cursor c to point to first node and return data pointer          */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qFirst(
      QHANDLE                q           ,// IO-Queue
      CURSOR &               c           ,// IO-Cursor to use
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qFirst(q,c,t)       \
   ((t *)(((q)->head) ? (c=(q)->head)->data : NULL))

/*---------------------------------------------------------------------------*/
/*       Set cursor c to point to last node and return data pointer          */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qLast(
      QHANDLE                q           ,// IO-Queue
      CURSOR &               c           ,// IO-Cursor to use
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qLast(q,c,t)        \
   ((t *)(((q)->head) ? (c=(q)->head->prev)->data : NULL))

/*---------------------------------------------------------------------------*/
/*         Advance cursor c to next node and return data pointer             */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qNext(
      QHANDLE                q           ,// IO-Queue
      CURSOR &               c           ,// IO-Cursor to use
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qNext(q,c,t)        \
   ((t *)(((c=(c)->next)==(q)->head) ? NULL : (c)->data))

/*---------------------------------------------------------------------------*/
/*              Move cursor c backwards and return data pointer              */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

<obj-type *>
   qPrev(
      QHANDLE                q           ,// IO-Queue
      CURSOR &               c           ,// IO-Cursor to use
      <literal-type-name>    <obj-type>   //   -Datatype for cast
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qPrev(q,c,t)        \
   ((t *)((c==(q)->head) ? ((c=(c)->prev),NULL) : (c=(c)->prev)->data))

/*---------------------------------------------------------------------------*/
/*                  Determine whether or not queue is empty                  */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

BOOLEAN
   qEmpty(
      QHANDLE                q            // IO-Queue
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qEmpty(q)           \
   ((q) ? (q)->size==0 : 1)

/*---------------------------------------------------------------------------*/
/*               Return number of nodes currently in the queue               */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

BOOLEAN
   qSize(
      const QHANDLE          q            // I -Queue
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  qSize(q)            \
   ((q) ? (q)->size : 0)

/*---------------------------------------------------------------------------*/
/*           Allocate a new Variable-Length Circular Queue Node              */
/*---------------------------------------------------------------------------*/

CURSOR
   xNew(
      register QHANDLE       q           ,// IO-The queue
      register const void  * data         // I -Data pointer to be stored
   );

/*---------------------------------------------------------------------------*/
/*                 Queue a new node at the end of the queue                  */
/*---------------------------------------------------------------------------*/

CURSOR
   xQueue(
      register QHANDLE       q           ,
      register const void  * d
   );

/*---------------------------------------------------------------------------*/
/*                 Push a new node at the front of the queue                 */
/*---------------------------------------------------------------------------*/

CURSOR
   xPush(
      register QHANDLE       q           ,
      register const void  * d
   );

/*---------------------------------------------------------------------------*/
/*            Insert a new node after a specific predecessor node            */
/*---------------------------------------------------------------------------*/

CURSOR
   xInsert(
      register QHANDLE       q           ,
      register CURSOR        p           ,
      register const void  * d
   );

/*---------------------------------------------------------------------------*/
/*                       Remove an entry from a queue                        */
/*---------------------------------------------------------------------------*/

void *
   xRemove(
      register QHANDLE       q           ,// IO-The queue
      register CURSOR        c            // IO-Node to be removed
   );

#endif
