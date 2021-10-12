static char sccsid[] = "@(#)81	1.4  src/bos/usr/lib/asw/mpqp/queues.c, ucodmpqp, bos411, 9428A410j 1/18/94 11:57:53";

/*--------------------------------------------------------------------------
*
*				  QUEUES.C
*
*  COMPONENT_NAME:  (UCODEMPQ) Multiprotocol Quad Port Adapter Software.
*
*  FUNCTIONS: queue_init, queue_writeb, queue_readb,
*	      queue_insertb, queue_previewb, queue_writew, queue_readw, 
*	      queue_insertw, queue_previeww, queue_writel, queue_readl,
*	      queue_insertl, queue_previewl
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/

/*----------------------------------------------------------------------*/
/*									*/
/*  This file contains the primitive queue routines used to access all	*/
/*  queues in the adapter software, including those accessed by the	*/
/*  host device driver.  The device driver is expected to adhere to 	*/
/*  the queue access algorithms implemented here.  Only one side of a	*/
/*  queue may be accessed by a given entity at any time; neither the	*/
/*  device driver nor the adapter software is allowed to modify both	*/
/*  ends of a queue that is shared between the two parties.  As long	*/
/*  as this rule is strictly enforced, either party may access a queue	*/
/*  asynchronously from the other without corruption of the data	*/
/*  structure and without the need for complex locking algorithms.	*/
/*									*/
/*----------------------------------------------------------------------*/

#include "mpqp.def"

/* Internal queue definitions used by queue routines */

typedef struct {
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last element of queue */
	unsigned char	out;		/* first item to remove */
	unsigned char	in;		/* last item inserted */
} QUEUE_HEADER;
	
typedef struct {
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last element of queue */
	unsigned char	out;		/* first item to remove */
	unsigned char	in;		/* last item inserted */
	unsigned char	q_elem[1];	/* queue elements */
} BYTE_QUEUE;

typedef struct {
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last element of queue */
	unsigned char	out;		/* first item to remove */
	unsigned char	in;		/* last item inserted */
	unsigned int	q_elem[1];	/* queue elements */
} WORD_QUEUE;

typedef struct {
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last element of queue */
	unsigned char	out;		/* first item to remove */
	unsigned char	in;		/* last item inserted */
	unsigned long	q_elem[1];	/* queue elements */
} LONG_QUEUE;


/*----------------------------------------------------------------------*/
/*  Queue Write Flavor 							*/
/*									*/
/*  This is the basic algorithm definition for writing to all queues	*/
/*  (regardless of width): disable interrupts; if the queue is full,	*/
/*  reenable interrupts and return an error.  See if the queue is	*/
/*  empty, then update the in pointer to the next slot in the queue.	*/
/*  Write the new queue element to this location then reenable		*/
/*  interrupts.  If the queue was empty, return 2, else return 1.	*/


# define Q_WRITE( p_queue, val )					\
	register int		empty;					\
        register unsigned char	in;					\
	disable();							\
	if ( Q_FULL( p_queue )) 					\
	{								\
	    enable();							\
	    return(-1);							\
	}								\
	empty = Q_EMPTY( p_queue );					\
	in = NEXT_IN( p_queue );					\
	(p_queue)->q_elem[ in ] = (val);				\
	(p_queue)->in = in;						\
	enable();							\
	return( empty ? 2 : 1 )

		
/*----------------------------------------------------------------------*/
/*  Queue Read Flavor 							*/
/*									*/
/*  This is the basic algorithm definition for reading from all queues	*/
/*  (regardless of width): disable interrupts; if the queue is empty,	*/
/*  reenable interrupts and return an error.  Update the out pointer	*/
/*  to point to the next slot to read from, then remove that element	*/
/*  from the queue; reenable interrupts before returning its value.	*/


# define Q_READ( p_queue, val )						\
	disable();							\
	if ( Q_EMPTY( p_queue )) { 					\
	    val = -1;							\
	} else {							\
	    (p_queue)->out = NEXT_OUT( p_queue );			\
	    val = (p_queue)->q_elem[ (p_queue)->out ];			\
	}								\
	enable();							\
	return(val)


/*----------------------------------------------------------------------*/
/*  Queue Insert Flavor 						*/
/*									*/
/*  This is the basic algorithm definition for inserting at the 	*/
/*  beginning of all queues (regardless of width): disable interrupts; 	*/
/*  if the queue is full, re-enable interrupts and return -1.  If the 	*/
/*  queue is empty, essentially just do a queue_write to prevent 	*/
/*  having to decrement the out pointer.  If there are items on the 	*/
/*  queue, write the data to the current out pointer offset and then 	*/
/*  decrement the out pointer so it will point to the right place for 	*/
/*  the next read.  Reenable interrupts.  If the queue was empty return */
/*  2, else return 1.							*/
/* 									*/
/*  Care should be taken when calling this function.  Inserting to the  */
/*  beginning of a queue by the queue reader violates the assumption 	*/
/*  that there must be only queue reader and one queue writer.  	*/
/*  However, in some cases the ASW reads a command from a queue and 	*/
/*  cannot process it; and so has to write it back the queue using 	*/
/*  queue_insert.  							*/
/* 									*/
/*  Never use queue_insert on queues shared with the driver as there is */
/*  no locking to prevent the driver from accessing the queue header 	*/
/*  record at the same time as the ASW.	  Those queues are the adapter 	*/
/*  response queue, txfree_q, and adapter command queue.		*/


# define Q_INSERT( p_queue, val )                                       \
        register int    empty;                                          \
        register unsigned char	in;					\
        disable();                                                      \
        if ( Q_FULL( p_queue ))                                         \
        {                                                               \
            enable();                                                   \
            return(-1);                                                 \
        }                                                               \
        empty = Q_EMPTY( p_queue );                                     \
        if (empty) {                                                    \
	    in = NEXT_IN( p_queue );					\
            (p_queue)->q_elem[ in ] = (val);                 		\
	    (p_queue)->in = in;						\
        } else {                                                        \
            (p_queue)->q_elem[ (p_queue)->out ] = (val);                \
            (p_queue)->out = PREV_OUT( p_queue );                       \
        }                                                               \
        enable();                                                       \
        return( empty ? 2 : 1 )

/*----------------------------------------------------------------------*/
/*  Queue Preview Flavor                                                */
/*                                                                      */
/*  This is the basic algorithm definition for previewing all queues    */
/*  (regardless of width): disable interrupts; if the queue is empty,   */
/*  reenable interrupts and return an error.  If not empty, just return */
/*  the value that would be next out, don't update the out pointer.	*/
/*  Reenable interrupts before returning the value.     		*/


# define Q_PREVIEW( p_queue, val )                                      \
        disable();                                                      \
        if ( Q_EMPTY( p_queue )) {                                      \
            val = -1;                                                   \
        } else {                                                        \
            val = (p_queue)->q_elem[ NEXT_OUT( p_queue ) ];             \
        }                                                               \
        enable();                                                       \
        return(val)

/*----------------------------------------------------------------------*/
/*  Byte Queue Instances:						*/

queue_writeb (
	BYTE_QUEUE	*p_queue,
	unsigned char	byte )
{
	Q_WRITE( p_queue, byte );
}

unsigned char
queue_readb (
	BYTE_QUEUE	*p_queue)
{
	unsigned char	byte;

	Q_READ( p_queue, byte );
}

queue_insertb (
        BYTE_QUEUE      *p_queue,
        unsigned char   byte)
{
        Q_INSERT ( p_queue, byte );
}

unsigned char
queue_previewb (
        BYTE_QUEUE      *p_queue)
{
        unsigned char   byte;

        Q_PREVIEW( p_queue, byte );
}

/*----------------------------------------------------------------------*/
/*  Word Queue Instances:						*/

queue_writew (
	WORD_QUEUE	*p_queue,
	unsigned short	word )
{
	Q_WRITE( p_queue, word );
}

unsigned short
queue_readw (
	WORD_QUEUE	*p_queue)
{
	register int	word;

	Q_READ( p_queue, word );
}

queue_insertw (
        WORD_QUEUE      *p_queue,
        unsigned short   word)
{
        Q_INSERT ( p_queue, word );
}

unsigned short
queue_previeww (
        WORD_QUEUE      *p_queue)
{
        register int 	word;

        Q_PREVIEW( p_queue, word );
}

/*----------------------------------------------------------------------*/
/*  Long Queue Instances:						*/

queue_writel (
	LONG_QUEUE	*p_queue,
	unsigned long	long_word )
{
	Q_WRITE( p_queue, long_word );
}

unsigned long
queue_readl (
	LONG_QUEUE	*p_queue)
{
	unsigned long	long_word;

	Q_READ( p_queue, long_word );
}

queue_insertl (
        LONG_QUEUE      *p_queue,
        unsigned long   long_word)
{
        Q_INSERT ( p_queue, long_word );
}

unsigned long
queue_previewl (
        LONG_QUEUE      *p_queue)
{
        unsigned long 	long_word;

        Q_PREVIEW( p_queue, long_word );
}

/*----------------------------------------------------------------------*/
/*  Generic Queue Routines (applicable to all queues):			*/

queue_init (
	QUEUE_HEADER	*p_queue,
	int		q_len )
{
	disable();			/* disable interrupts */
	p_queue->length = q_len;	/* set length */
	p_queue->end 	= q_len - 1 ;	/* index of last element */
	p_queue->out	= 0;		/* index of first element */
	p_queue->in	= 0;		/* index of first element */
	enable();			/* reenable interrupts */
	return((int)p_queue);
}
