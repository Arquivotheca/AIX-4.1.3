static char sccsid[] = "@(#)14	1.50.1.2  src/bos/kernel/ios/select.c, sysios, bos41J, 9516B_all 4/19/95 10:36:30";
/*
 * COMPONENT_NAME: (SYSIOS) Select system call
 *
 * FUNCTIONS:	chk_timeout	select		
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/device.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include "selpoll.h"
#include <sys/thread.h>
#include <sys/uthread.h>
#include <sys/atomic_op.h>

/*
 * Turn ON the specified bit in the elem-th element in 'array'.
 */
#define	SETBIT(array_elem, bit)					\
	(array_elem |= (1 << bit))

/*
 * Turn OFF the specified bit in the elem-th element in 'array'.
 */
#define	CLRBIT(array_elem, bit)					\
	(array_elem &= ~(1 << bit))

/*
 * See if the specified bit in the elem-th element of 'array' is ON.
 */
#define	ISSET(array_elem, bit)					\
	(array_elem & (1 << bit))

/*
 * See if the specified bit in the elem-th element of 'array' is OFF.
 */
#define	ISCLR(array_elem, bit)					\
	((array_elem & (1 << bit)) == 0)

/*
 * NAME:  chk_timeout
 *
 * FUNCTION:  Check the validity of the values in a timeval structure.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by select.
 *	It is pageable.
 *
 * NOTES:
 *
 * DATA STRUCTURES:	timeval
 *
 * RETURN VALUES DESCRIPTION:	0 upon successful completion;
 *				-1 if timeval struct contained negative seconds
 *				or microseconds values.
 *
 * EXTERNAL PROCEDURES CALLED:	none.
 */
int
chk_timeout(timeout, reqev_mskp)

struct timeval	*timeout;	/* length of time to wait for events	*/
int		*reqev_mskp;	/* requested event mask			*/
{
	if (timeout == NULL)
	{
		return(0);
	}
	if ((timeout->tv_sec < 0) || (timeout->tv_usec < 0) ||
		(timeout->tv_usec >= uS_PER_SECOND))
	{
		/*
		 * Negative time values are invalid.
		 */
		return(-1);
	}
	else if ((timeout->tv_sec == NO_TIMEOUT) && (timeout->tv_usec == NO_TIMEOUT))
	{
		/*
		 * Zero timeout value specified, so we will not wait.
		 */
		*reqev_mskp = POLLSYNC;
	}

	return(0);

}  /* end chk_timeout */

/*
 * NAME:  select
 *
 * FUNCTION:  The SELECT system call is used to check the I/O status of
 *	multiple file descriptors and message queues to see if they are
 *	ready for reading (receiving) or writing (sending), or if they
 *	have an exceptional condition pending.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called by user programs.
 *	It is pageable.
 *
 * NOTES:	nfdsmsgs is an integer specifying the number of file descriptors
 * 	and message queues to check.  The low order 16 bits give the length of a
 * 	bit mask which indicates which file descriptors to check; the high order
 *	16 bits give the size of an array that contains message queue
 *	identifiers.
 *
 * DATA STRUCTURES:	sellist_2, thread, uthread,
 *			timeval,
 *			sel_ptrs,
 *			sel_cb
 *
 * RETURN VALUES DESCRIPTION:	non-negative value upon successful completion:
 *					>0 -- the number of file descriptors and
 *					message queues that satisfy the
 *					selection criteria.  The file descriptor
 *					bit masks are modified so that bits set
 *					to one indicate file descriptors that
 * 					met the criteria.  The msgid arrays are
 *					altered so that message queue arrays
 *					that did not meet the criteria are
 *					replaced with a value of -1.
 *					0 -- the call timed out and no file
 *					descriptors satisfied the selection
 *					criteria or no events specified in
 *					selection criteria.
 *				upon unsuccessful completion:              
 *                                      -1 and errno set to EINVAL, EINTR, EBADF,
 *			                EAGAIN, or EFAULT,  - OR -
 *				        the non-zero return code from poll_wait.
 *        
 *                              note:   if selpoll returns an error and it is not 
 *                                      EINTR, then all requested events are set true for 
 *                                      the object that had the error.
 *                                      
 *
 * EXTERNAL PROCEDURES CALLED:
 *	assert
 *	chk_timeout		check values in timeval struct
 *	xmalloc			allocate storage for file descriptor/msgid
 *				arrays
 *	xmfree			free previously allocated storage
 *	copyin			copy data from user to kernel space
 *	copyout			copy data from kernel to user space
 *	selpoll			poll a device
 *      et_wait                 clear EVENT_SYNC flag possibly set by selnotify
 *      msgselect               poll a msg queue
 *	poll_wait		wait for events to occur
 *	selpoll_cleanup		unchain control block from the device and
 *				thread chains; free the control block's storage
 *	fetch_and_or		to modify TSELECT
 *	get_curthread		get current thread pointer
 */
int
select(ulong nfdsmsgs, void *readlist, void *writelist, void *exceptlist, struct timeval *utimeoutp)

/* ulong 	nfdsmsgs;	#file descriptors/message queues	*/
/* void 	*readlist;	check file descriptors for reads	*/
/* void 	*writelist;	check file descriptors for writes	*/
/* void 	*exceptlist;	check file descriptors for exceptions*/
/* struct timeval *utimeoutp;	length of time to wait for events	*/
{
	register int	rc;	/* return code from various routines	*/
	register int	rc_save;/* return code from various routines	*/
	register int	fdes;	/* number of file descriptors		*/
	register int	msgs;	/* number of message queues		*/
	register int	nfilewords;/* #words needed to hold fdes	*/
	register int	array_size;/* #bytes needed to hold fdes & msgids*/
	register int	num_valid;/* number of valid input pointers	*/
	register int	num_req_events;/* number of requested events	*/
	register int	num_found;/* offset in allocated area		*/
	register int	fd;	/* counter:  #bits processed		*/
	register int	num_fdes;/* #file descriptors with occurred events*/
	register int	num_msgs = 0;/* #msg queue ids with occurred events*/
	caddr_t		k_list;	/* malloc'd area:  kernel's copy of user data*/
	struct sel_ptrs	sel_ptrs[3];/* array containing user & kernel ptrs*/
	struct sel_cb	*cb;	/* ctl blk at head of thread chain	*/
	ushort		reqevents;/* requested events; used in call to selpoll*/
	ushort		rtnevents;/* returned events; used in call to selpoll*/
	int		reqev_msk;/* requested event mask		*/
	register int	elem;	/* counter used in for loops		*/
	register int	bit;	/* counter used in for loops		*/
	register int	list_num;/* counter used in for loops		*/
	struct timeval	stimeout, *ktimeoutp;
	extern int	chk_timeout();
	extern int	copyin();
	extern int	copyout();
	extern int	selpoll();
	extern int	poll_wait();
	extern void	selpoll_cleanup();
	int bitmask[3];	/* used to copy the user file-descriptor masks for the
			most common cases	*/
	ushort malloc_req; /* flag for malloc'ing	*/
	register struct thread *temp_threadp;	/* temp pointer to thread */
	register struct uthread *temp_uthreadp;	/* temp pointer to uthread */

	temp_threadp = CURTHREAD;
	temp_uthreadp = temp_threadp->t_uthreadp;


	ktimeoutp = &stimeout;
	if (utimeoutp != NULL) {
		if (copyin((caddr_t) utimeoutp, (caddr_t) ktimeoutp, sizeof(stimeout)))
		{
			temp_uthreadp->ut_error = EFAULT;
			return -1;
		}
	} else
		ktimeoutp = (struct timeval *) 0;
			
	/*
	 * Validate nfdsmsgs and then extract from it
	 * the number of file descriptors and number of
	 * message queues.
	 */
	fdes = NFDS(nfdsmsgs);
	msgs = NMSGS(nfdsmsgs);

	/*
	 * Initialize array with user data pointers.
	 */
	sel_ptrs[0].user_ptr = readlist;
	sel_ptrs[1].user_ptr = writelist;
	sel_ptrs[2].user_ptr = exceptlist;

	/*
	 * Determine the number of valid (i.e. non-NULL) input pointers.
	 */
	num_valid = 0;
	for (elem = 0; elem < 3; elem++)
	{
		if (sel_ptrs[elem].user_ptr != NULL)
		{
			num_valid++;
		}
	}

        /*
	 * If there are no valid input pointers, select acts as a timer.
	 * Returns when a signal occurs or when the timeout is reached.
	 * If the ktimeout is null, we could wait forever.
	 */
	if (num_valid == 0)
	{
		rc = chk_timeout(ktimeoutp, &reqev_msk);
		if (rc != 0)
		{
			temp_uthreadp->ut_error = EINVAL;
			return(rc);
		}
		if ((rc = poll_wait(ktimeoutp, temp_threadp)) != 0)
		{
			temp_uthreadp->ut_error = rc;	/* EINTR if signal */
			/* poll_wait() was interrupted by a signal,
			 * clear a possible timer et_post()
			 */
			(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
			return(-1);
		}
		else
			return(rc);
	}

	/*
	 * Compute number of file descriptor mask words in parameter arrays.
	 *
	 * We need to allocate an array, each element of which
	 * will be 32 (the size of an integer) bits (1 word) long.
	 * If fdes is not an even number of words, we will need
	 * to allocate one more array element for the 'leftover'
	 * bits...
	 * ...However, we only want to use the number of 'leftover' bits
	 * of that extra word.  For example, if fdes is 68, then the
	 * array would be 3 elements long, but we would only look at the
	 * 0th element (bits 0-31), the 1st element (bits 32-63), and the
	 * last 4 bits of the 2nd element (bits 64-67).
	 */

	/*
	 * Compute size of each array.
	 */
	if (fdes & (NBITS - 1))
	{
		/*
		 * Not an even number of words.
		 */
		nfilewords = 1 + (fdes >> UWSHIFT);
	}
	else
	{
		nfilewords = fdes >> UWSHIFT;
	}
	array_size = (nfilewords + msgs) * NBPW;

	/*
	 * Allocate storage for each array, only if nfilewords > 1 or if there is at least
	 * one msgid. This avoids malloc'ing for the most common case of calling
	 * select with less than 32 file-descriptors and no msgids.
	 */

	if((nfilewords > 1)  || (msgs != 0))
		malloc_req = 1;
	else
		malloc_req = 0;

	if(malloc_req)
	{
		k_list = xmalloc(num_valid * array_size,
				 0,
				 kernel_heap);
		if (k_list == NULL)
		{
			temp_uthreadp->ut_error = EAGAIN;
			return(-1);
		}
	}

	num_found = 0;		/* offset in allocated area		*/


	/*
	 * In sel_ptrs array:
	 * readlist (user/kernel) is 0th elem;
	 * writelist (user/kernel) is 1st elem;
	 * exceptlist (user/kernel) is 2nd elem.
	 *
	 * Process the 3 lists (readlist, writelist, exceptlist).
	 * 
	 * k_list________________________________
	 *					|
	 *					|  (Malloc'd area that user
	 *	       				V   info. is copied into)
	 *	       	 			_________________
	 * sel_ptrs[0].kernel_ptr.fdsmask----->	|<--nfilewords-->|
	 *					|    (#words)    |
	 *					|________________|
	 * sel_ptrs[0].kernel_ptr.msgids------>	|		 |
	 *					|<-msgs #words-->|
	 *					|________________|
	 * sel_ptrs[1].kernel_ptr.fdsmask----->	|<--nfilewords-->|
	 *					|    (#words)    |
	 *					|________________|
	 * sel_ptrs[1].kernel_ptr.msgids------>	|		 |
	 *					|<-msgs #words-->|
	 *					|________________|
	 * sel_ptrs[2].kernel_ptr.fdsmask----->	|<--nfilewords-->|
	 *					|    (#words)    |
	 *					|________________|
	 * sel_ptrs[2].kernel_ptr.msgids------>	|		 |
	 *					|<-msgs #words-->|
	 *					|		 |
	 *			       		------------------
	 */ 
	for (elem = 0; elem < 3; elem++)
	{
		if (sel_ptrs[elem].user_ptr != NULL)
		{
			/*
			 * Initialize kernel file descriptor ptr to correct
			 * offset in allocated area or to the correct element
			 * in the local array bitmask.
			 */
			if(malloc_req)
				sel_ptrs[elem].kernel_ptr.fdsmask = 
					    (int *)(k_list + (num_found * array_size));
			else
				sel_ptrs[elem].kernel_ptr.fdsmask = &bitmask[elem];

			num_found++;	/* bump offset in allocated area*/
			/*
			 * Copy data from user to kernel space.
			 */
			rc = copyin(sel_ptrs[elem].user_ptr,
				    sel_ptrs[elem].kernel_ptr.fdsmask,
				    array_size);
			if (rc != 0)
			{
				if(malloc_req)
				{
					(void)xmfree(k_list, kernel_heap);
				}
				temp_uthreadp->ut_error = EFAULT;
				return(-1);
			}

			/*
			 * Initialize kernel message queue ptr to correct
			 * offset in allocated area.
			 */
			if (msgs > 0)
			{
				sel_ptrs[elem].kernel_ptr.msgids = 
					sel_ptrs[elem].kernel_ptr.fdsmask + nfilewords;
			}
			else
				sel_ptrs[elem].kernel_ptr.msgids = (int *)NULL;
		}
	}  /* end for loop */

	reqev_msk = 0;
	/*
	 * Validate values in timeval struct.
	 */
	rc = chk_timeout(ktimeoutp, &reqev_msk);
	if (rc != 0)
	{
		if(malloc_req)
		{
			(void)xmfree(k_list, kernel_heap);
		}
		temp_uthreadp->ut_error = EINVAL;
		return(rc);
	}

	/*
	 * This flag is used to determine if notification has been
	 * received of events that have occurred.
	 */
	fetch_and_or(&temp_threadp->t_atomic, TSELECT);

	/*
	 * Check no. of file descriptors against the max no. of open files.
	 */
	if (fdes > OPEN_MAX)
	{
		fdes = OPEN_MAX;
	}

	/*
	 * fd (file descriptor) = column number (bit position)
	 *
	 * Look down each column (fd), picking up a bit from each of the 3
	 * arrays.  If the readlist bit is set, then set reqevents to POLLIN.
	 * If the writelist bit is set, then set the POLLOUT flag in reqevents.
	 * If the exceptlist bit is set, then set the POLLPRI flag in reqevents.
	 * This mask of flags is the reqevents for that fd.
	 *
	 *		      bit 31			 bit 0  bit 63			  bit 32
	 *		      |				     |  |			       |
	 *		      |	<--------- elem 0 ---------> |  | <--------- elem 1 ---------> |
	 *		      V				     V  V			       V
	 *	readlist:    [00010001000100010001000100010001][01010101000100011001000100010010]
	 *	writelist:   [01000100010001000100010001000100][00010101000100011001000100010001]
	 *	exceptlist:  [10010010001000100001000100010001][00010101000100010001000100010001]
	 *				|		|            |		|
	 *				|		|            |		|
	 *				V		V            V		V
	 *				bit 21		bit 5        bit 58	bit 47
 	 *
	 * Example:  column/bit 21:  reqevents = 001,
	 * 	     column/bit 5:   reqevents = 000,
	 * 	     column/bit 58:  reqevents = 111,
	 * 	     column/bit 47:  reqevents = 110, etc.
	 */
	num_fdes = 0;	/* total number of events that have occurred	*/
	num_req_events = 0;/* total number of events that were requested	*/
	rc_save = 0;	/* return code from selpoll			*/
	fd = 0;		/* total number of bits that have been processed*/
	for (elem = 0; ((elem < nfilewords) && (fd < fdes) && (rc_save == 0)); elem++)
	{
		/*
		 * Look at each word (elem) in the array. Skip the entire word
		 * if it is null.
		 */
		if (((sel_ptrs[0].user_ptr == NULL) ||
		      (!sel_ptrs[0].kernel_ptr.fdsmask[elem]))&&

		    ((sel_ptrs[1].user_ptr == NULL)||
		      (!sel_ptrs[1].kernel_ptr.fdsmask[elem]))&&

		    ((sel_ptrs[2].user_ptr == NULL)||
		      (!sel_ptrs[2].kernel_ptr.fdsmask[elem])))
		{
			fd += NBITS;
			continue;
		}

		for (bit = 0; ((bit < NBITS) && (fd < fdes)); bit++, fd++)
		{
			/*
			 * Look at each bit in the word, scanning from
			 * right (bit 0) to left (bit 31).
			 */

			/*
			 * If the following is true for all 3 ptrs, then
			 * don't process this bit (file descriptor):
			 * a) the ptr IS NOT null and the bit is 0, OR
			 * b) the ptr IS null
			 */
			if (((sel_ptrs[0].user_ptr == NULL) ||
			      (ISCLR(sel_ptrs[0].kernel_ptr.fdsmask[elem], bit))) &&

			    ((sel_ptrs[1].user_ptr == NULL) ||
			      (ISCLR(sel_ptrs[1].kernel_ptr.fdsmask[elem], bit))) &&

			    ((sel_ptrs[2].user_ptr == NULL) ||
			      (ISCLR(sel_ptrs[2].kernel_ptr.fdsmask[elem], bit))))
			{
				continue;
			}

                        /*
                         * At this point, there's at least one requested event.
                         */


 		        reqevents = reqev_msk;	/* either 0 or POLLSYNC	*/

			/*
			 * See if readlist ptr's bit is set.
			 */
			if ((sel_ptrs[0].user_ptr != NULL) &&
			    (ISSET(sel_ptrs[0].kernel_ptr.fdsmask[elem], bit)))
			{
				/*
				 * Set bit on for call to selpoll.
				 */
				reqevents |= POLLIN;
				num_fdes++;	/* #occurred events	*/
				num_req_events++;	/* #requested events	*/
			}

			/*
			 * See if writelist ptr's bit is set.
			 */
			if ((sel_ptrs[1].user_ptr != NULL) &&
			    (ISSET(sel_ptrs[1].kernel_ptr.fdsmask[elem], bit)))
			{
				/*
				 * Set bit on for call to selpoll.
				 */
				reqevents |= POLLOUT;
				num_fdes++;	/* #occurred events	*/
				num_req_events++;	/* #requested events	*/
			}

			/*
			 * See if exceptlist ptr's bit is set.
			 */
			if ((sel_ptrs[2].user_ptr != NULL) &&
			    (ISSET(sel_ptrs[2].kernel_ptr.fdsmask[elem], bit)))
			{
				/*
				 * Set bit on for call to selpoll.
				 */
				reqevents |= POLLPRI;
				num_fdes++;	/* #occurred events	*/
				num_req_events++;	/* #requested events	*/
			}

			/*
			 * Call selpoll to check the status of the
			 * specified file descriptor.
			 */
			rtnevents = 0;	/* initialize returned events	*/
			rc_save = selpoll(fd, fd, reqevents, &rtnevents, POLL_FDMSG, NULL);
			/*
			 *  If rc=0 but rtnevents has POLLHUP or POLLERR set,
			 *  set all the bits and continue (treat the same as
			 *  a non-EINTR and non-EBADF error).
			 */
			if ((rc_save == 0) && (!(rtnevents & (POLLHUP | POLLERR))))
			{
				/*
				 * If there were no reads, turn off the bit
				 * in the malloc'd array.
				 */
				if ((reqevents & POLLIN) &&
				    (!(rtnevents & POLLIN)) &&
				    (sel_ptrs[0].user_ptr != NULL))
				{
					CLRBIT(sel_ptrs[0].kernel_ptr.fdsmask[elem], bit);
					num_fdes--;	/* #occurred events*/
				}

				/*
				 * If there were no writes, turn off the bit
				 * in the malloc'd array.
				 */
				if ((reqevents & POLLOUT) &&
				    (!(rtnevents & POLLOUT)) &&
				    (sel_ptrs[1].user_ptr != NULL))
				{
					CLRBIT(sel_ptrs[1].kernel_ptr.fdsmask[elem], bit);
					num_fdes--;	/* #occurred events*/
				}

				/*
				 * If there were no exceptions, turn off the bit
				 * in the malloc'd array.
				 */
				if ((reqevents & POLLPRI) &&
				    (!(rtnevents & POLLPRI)) &&
				    (sel_ptrs[2].user_ptr != NULL))
				{
					CLRBIT(sel_ptrs[2].kernel_ptr.fdsmask[elem], bit);
					num_fdes--;	/* #occurred events*/
				}
			}
			else		/* selpoll's rc != 0		*/
			{
				/* exit for loop if signal caught */
				if ((rc_save == EINTR) || (rc_save == EBADF))
					break;
				else
				/* if error was not signal, then indicate
				   error by setting all request events to
				   true for the object that had the error
				 */
				{
					rtnevents = reqevents & (~POLLSYNC);
					rc_save = 0;
				}	
			}

			if (rtnevents != 0)
			{
				/*
				 * At least 1 of the reqevents has occurred,
				 * so process the remaining data synchronously.
				 */
				reqev_msk = POLLSYNC;
			}
			/*
			 * If TSELECT is off, notification has been received
			 * that at least one event has occurred; therefore,
			 * all other selpoll() calls will be synchronous.
			 */
			if (!(temp_threadp->t_atomic & TSELECT))
			{
				reqev_msk = POLLSYNC;
			}

		}  /* end for bit loop */
	}  /* end for elem loop */


	/*
	 * See if any errors or signals caught.
	 */
        if (rc_save != 0)
        {
                /* remove any select control blocks from chain */
		/* and decrement fd hold counts */
                while (temp_uthreadp->ut_selchn != NULL)
                        selpoll_cleanup(((struct sel_cb *)
					temp_uthreadp->ut_selchn)->corl);

		if (!(temp_threadp->t_atomic & TSELECT))
		{
			(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
		}
		else
		{
			/*
			 * clear TSELECT flag
			 */  
			fetch_and_and(&(temp_threadp->t_atomic), ~TSELECT);
		}
		
                if(malloc_req)
                {
                        (void)xmfree(k_list, kernel_heap);
                }
                temp_uthreadp->ut_error = rc_save;
                return(-1);
        }


	if (msgs > 0)
	{
	/*
	 * Process Message Queue ids.
	 *
	 * One msg (msg queue id) per array element (i.e. msgid = int).
	 *
	 * In the readlist, look at an array element.  If it does not
	 * contain -1, then set reqevents to POLLIN, and then look at
	 * the corresponding array element in the writelist and
	 * exceptlist.  If either/both contain the same msgid as the
	 * one in readlist, then set the corresponding bits (POLLOUT/
	 * POLLPRI) in the reqevents, and set did_wr/did_ex as appropriate.
	 * The purpose of this is to minimize the number of calls to msgselect,
	 * since in most cases where the same msgid is being selected for
	 * more than one event, the indices into the read/write/except
	 * arrays will be the same for each instance of the msgid.
	 * 
	 * In the writelist, look at an array element.  If it does not
	 * contain -1, then set reqevents to POLLOUT, and then look at
	 * the corresponding array element in the exceptlist.  If it
	 * contains the same msgid as the one in writelist, then set
	 * the corresponding bit (POLLPRI) in the reqevents, and set
	 * did_ex to 1.
	 *
	 * In the exceptlist, look at an array element.  If it does not
	 * contain -1, then set reqevents to POLLPRI.
	 *
	 * The loop through read/write/except lists is un-rolled to reduce
	 * the number of conditionals that would be required to implement this.
	 *
	 *              elem 0          elem 4
	 *                 |               |
	 *                 |               |
	 *                 V               V
	 *  readlist:    [01][02][04][06][07]
	 *  writelist:   [01][02][03][05][08]
	 *  exceptlist:  [01][03][04][05][09]
 	 *
	 * Example:
	 *	elem 0:  reqevents = POLLIN | POLLOUT | POLLPRI
	 *	         set did_wr/did_ex = 1
     *           1st call to msgselect
	 *           skip write and except check
	 *	elem 1:  reqevents = POLLIN | POLLOUT
	 *           set did_wr
	 *           2nd call to msgselect
	 *           skip write check
	 *           reqevents = POLLPRI
	 *           3rd call to msgselect
	 *	elem 2:  reqevents = POLLIN | POLLPRI
	 *           set did_ex
	 *           4th call to msgselect
	 *           reqevents = POLLOUT
	 *           5th call to msgselect
	 *           skip except check
	 *	elem 3:  reqevents = POLLIN
	 *           6th call to msgselect
	 *           reqevents = POLLOUT | POLLPRI
	 *           set did_ex
	 *           7th call to msgselect
	 *           skip except check
	 *	elem 4:  reqevents = POLLIN
	 *           8th call to msgselect
	 *           reqevents = POLLOUT
	 *           9th call to msgselect
	 *           reqevents = POLLPRI
	 *           10th call to msgselect
	 *
	 *  Thus, msgselect is called 10 times instead of 15...
	 *  Arguably, since only nine msgid's were specified, this could be
	 *  reduced to 9, but the overhead required for that optimization would
	 *  probably outweigh its benefit since like msgid's will probably 
	 *  have the same index.
	 */
		num_msgs = 0;

		for (elem = 0; elem < msgs; elem++)
		{
			int did_wr, did_ex;
			ushort tmpevents;

			did_wr = 0;	/* default is all lists un-done */
			did_ex = 0;
			rtnevents = 0;		/* no returned events yet */
			
				/* check for read request */
			if ((sel_ptrs[0].user_ptr != NULL) &&
				(sel_ptrs[0].kernel_ptr.msgids[elem] != -1))
			{
				reqevents = reqev_msk | POLLIN;	/* checking for read */
				num_req_events++;

					/* check to see if write/except entries same */
				if ((sel_ptrs[1].user_ptr != NULL) &&
					(sel_ptrs[0].kernel_ptr.msgids[elem] ==
				     sel_ptrs[1].kernel_ptr.msgids[elem]))
				{
					reqevents |= POLLOUT;	/* checking for write */
					num_req_events++;
					did_wr = 1;
				}
				if ((sel_ptrs[2].user_ptr != NULL) &&
					(sel_ptrs[0].kernel_ptr.msgids[elem] ==
				     sel_ptrs[2].kernel_ptr.msgids[elem]))
				{
					reqevents |= POLLPRI;	/* checking for except */
					num_req_events++;
					did_ex = 1;
				}

					/* check for events now */
				tmpevents = 0;
				rc_save = msgselect(sel_ptrs[0].kernel_ptr.msgids[elem],
					elem, reqevents, &tmpevents);

					/* break out if error and no events */
				if (rc_save != 0)
				{
					if (rc_save == EINVAL)
					{
						/*
						 *  select is documented to
						 *  return EBADF if the msgid
						 *  is bad, but msgselect
						 *  returns EINVAL instead ...
						 */
						rc_save = EBADF;
						break;
					}
					if (tmpevents == 0)
					{
						break;
					}
				}
				rtnevents |= tmpevents;
			}

				/* check for write request */
			if ((!did_wr) && (sel_ptrs[1].user_ptr != NULL) &&
				(sel_ptrs[1].kernel_ptr.msgids[elem] != -1))
			{
				reqevents = reqev_msk | POLLOUT;	/* checking for write */
				num_req_events++;

					/* check to see if except msgid is same */
				if ((!did_ex) && (sel_ptrs[2].user_ptr != NULL) &&
					(sel_ptrs[1].kernel_ptr.msgids[elem] ==
				     sel_ptrs[2].kernel_ptr.msgids[elem]))
				{
					reqevents |= POLLPRI;	/* checking for exception */
					num_req_events++;
					did_ex = 1;
				}

					/* check for events now */
				tmpevents = 0;
				rc_save = msgselect(sel_ptrs[1].kernel_ptr.msgids[elem],
					elem, reqevents, &tmpevents);

					/* break out if error and no events */
				if (rc_save != 0)
				{
					if (rc_save == EINVAL)
					{
						/*
						 *  select is documented to
						 *  return EBADF if the msgid
						 *  is bad, but msgselect
						 *  returns EINVAL instead ...
						 */
						rc_save = EBADF;
						break;
					}
					if (tmpevents == 0)
					{
						break;
					}
				}
				rtnevents |= tmpevents;
			}

				/* check for exception request */
			if ((!did_ex) && (sel_ptrs[2].user_ptr != NULL) &&
				(sel_ptrs[2].kernel_ptr.msgids[elem] != -1))
			{
				reqevents = reqev_msk | POLLPRI;	/* checking for exception */
				num_req_events++;

					/* check for events now */
				tmpevents = 0;
				rc_save = msgselect(sel_ptrs[2].kernel_ptr.msgids[elem],
					elem, reqevents, &tmpevents);

					/* break out if error and no events */
				if (rc_save != 0)
				{
					if (rc_save == EINVAL)
					{
						/*
						 *  select is documented to
						 *  return EBADF if the msgid
						 *  is bad, but msgselect
						 *  returns EINVAL instead ...
						 */
						rc_save = EBADF;
						break;
					}
					if (tmpevents == 0)
					{
						break;
					}
				}
				rtnevents |= tmpevents;
			}

			/*
			 * All events for this element have been collected, now
			 * change event-less entries to -1, leaving and counting
			 * those msgid's which do have events.
			 */
			if (rtnevents != 0)
			{
				/*
				 * At this point, any errors left from above are interpreted
				 * as events and returned through normal channels.
				 */
				rc_save = 0;

				if (reqevents & POLLIN)
				{
					if (rtnevents & POLLIN)
					{
						num_msgs++;
					}
					else
					{
						sel_ptrs[0].kernel_ptr.msgids[elem] = -1;
					}
				}
				if (reqevents & POLLOUT)
				{
					if (rtnevents & POLLOUT)
					{
						num_msgs++;
					}
					else
					{
						sel_ptrs[1].kernel_ptr.msgids[elem] = -1;
					}
				}
				if (reqevents & POLLPRI)
				{
					if (rtnevents & POLLPRI)
					{
						num_msgs++;
					}
					else
					{
						sel_ptrs[2].kernel_ptr.msgids[elem] = -1;
					}
				}

				/*
				 * At least 1 of the reqevents has occurred,
				 * so process the remaining data synchronously.
				 */
				reqev_msk = POLLSYNC;
			}
			else	/* rtnevents == 0	*/
			{
				if (sel_ptrs[0].user_ptr != NULL)
				{
					sel_ptrs[0].kernel_ptr.msgids[elem] = -1;
				}
				if (sel_ptrs[1].user_ptr != NULL)
				{
					sel_ptrs[1].kernel_ptr.msgids[elem] = -1;
				}
				if (sel_ptrs[2].user_ptr != NULL)
				{
					sel_ptrs[2].kernel_ptr.msgids[elem] = -1;
				}

				/*
				 * If TSELECT is off, notification been received
				 * that at least one event has occurred; therefore,
				 * all other selpoll() calls will be synchronous.
				 */
				if (!(temp_threadp->t_atomic & TSELECT))
				{
					reqev_msk = POLLSYNC;
				}
			}
		}  /* for loop */
	}	

	/*
	 * See if any errors or signals caught.
	 */
        if (rc_save != 0)
        {
                /* remove any select control blocks from chain */
		/* and decrement fd hold counts */
                while (temp_uthreadp->ut_selchn != NULL)
                        selpoll_cleanup(((struct sel_cb *)
					temp_uthreadp->ut_selchn)->corl);

		if (!(temp_threadp->t_atomic & TSELECT))
		{
			(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
		}
		else
		{
			/*
			 * clear TSELECT flag
			 */  
			fetch_and_and(&(temp_threadp->t_atomic), ~TSELECT);
		}

                if(malloc_req)
                {
                        (void)xmfree(k_list, kernel_heap);
                }
                temp_uthreadp->ut_error = rc_save;
                return(-1);
        }
	
	/*
	 * selpoll did not report an error condition.
	 */
	if ((!reqev_msk) && (temp_threadp->t_atomic & TSELECT))
	{
		/*
		 * None of the reqevents have occurred yet, so
		 * wait the specified length of time for a
		 * selnotify.
		 */
		rc_save = poll_wait(ktimeoutp, temp_threadp);
		if (rc_save != 0)
		{
			/* remove any select control blocks from chain */
			/* and decrement fd hold counts */
			while (temp_uthreadp->ut_selchn != NULL)
				selpoll_cleanup(((struct sel_cb *)
					temp_uthreadp->ut_selchn)->corl);
			if(malloc_req)
			{
				(void)xmfree(k_list, kernel_heap);
			}
			temp_uthreadp->ut_error = rc_save;

			if ((temp_threadp->t_atomic & TSELECT))
			{
				/*
				 * clear TSELECT flag
				 */  
				fetch_and_and(&(temp_threadp->t_atomic), ~TSELECT);
			}

			/* poll_wait() was interrupted by a signal,
			 * clear a possible timer et_post()
			 */
			(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
			return(-1);
		}
	}

	/*
	 * Run the thread chain (temp_uthreadp->ut_selchn).
	 */
	while ((cb = temp_uthreadp->ut_selchn) != NULL)
	{
		if (cb->dev_id != POLL_MSG)
		{
			/*
			 * Validate cb's correlator (i.e. file descriptor).
			 */
			assert(cb->corl < fdes);

			/*
		 	 * Determine elem/bit offset into malloc'd array.
		 	 */
			elem = cb->corl >> UWSHIFT;
			bit = cb->corl % NBITS;

			/*
			 * If there was a read, turn on the bit
			 * in the malloc'd array.
			 */
			if ((cb->rtnevents & cb->reqevents & POLLIN) &&
			    (sel_ptrs[0].user_ptr != NULL) &&
			    (!ISSET(sel_ptrs[0].kernel_ptr.fdsmask[elem], bit)))
			{
				SETBIT(sel_ptrs[0].kernel_ptr.fdsmask[elem], bit);
				num_fdes++;	/* #occurred events	*/
			}

			/*
			 * If there was a write, turn on the bit
			 * in the malloc'd array.
			 */
			if ((cb->rtnevents & cb->reqevents & POLLOUT) &&
			    (sel_ptrs[1].user_ptr != NULL) &&
			    (!ISSET(sel_ptrs[1].kernel_ptr.fdsmask[elem], bit)))
			{
				SETBIT(sel_ptrs[1].kernel_ptr.fdsmask[elem], bit);
				num_fdes++;	/* #occurred events	*/
			}

			/*
			 * If there was an exception, turn on the bit
			 * in the malloc'd array.
			 */
			if ((cb->rtnevents & cb->reqevents & POLLPRI) &&
			    (sel_ptrs[2].user_ptr != NULL) &&
			    (!ISSET(sel_ptrs[2].kernel_ptr.fdsmask[elem], bit)))
			{
				SETBIT(sel_ptrs[2].kernel_ptr.fdsmask[elem], bit);
				num_fdes++;	/* #occurred events	*/
			}

		}  /* dev_id is NOT a message id */
		else			/* dev_id is a message id	*/
		{
			if ((sel_ptrs[0].user_ptr != NULL) &&
			    (cb->reqevents & cb->rtnevents & POLLIN))
			{
				sel_ptrs[0].kernel_ptr.msgids[cb->corl] = cb->unique_id;
				num_msgs++;
			}

			if ((sel_ptrs[1].user_ptr != NULL) &&
			    (cb->reqevents & cb->rtnevents & POLLOUT))
			{
				sel_ptrs[1].kernel_ptr.msgids[cb->corl] = cb->unique_id;
				num_msgs++;
			}

			if ((sel_ptrs[2].user_ptr != NULL) &&
			    (cb->reqevents & cb->rtnevents & POLLPRI))
			{
				sel_ptrs[2].kernel_ptr.msgids[cb->corl] = cb->unique_id;
				num_msgs++;
			}

		}

		/*
		 * Remove control block from device and
		 * thread chains and free its storage.
		 * And decrement fd hold counts.
		 */
		selpoll_cleanup(cb->corl);

	}  /* end while */

	for (elem = 0; elem < 3; elem++)
	{
		if (sel_ptrs[elem].user_ptr != NULL)
		{
			/*
			 * Copy data from kernel to user space.
			 */
			rc = copyout(sel_ptrs[elem].kernel_ptr.fdsmask,
				     sel_ptrs[elem].user_ptr,
				     array_size);
			if (rc != 0)
			{
				if (!(temp_threadp->t_atomic & TSELECT))
				{
					(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
				}
				else
				{
					/*
					 * clear TSELECT flag
					 */  
					fetch_and_and(&(temp_threadp->t_atomic), ~TSELECT);
				}

				if(malloc_req)
				{
					(void)xmfree(k_list, kernel_heap);
				}
				temp_uthreadp->ut_error = EFAULT;
				return(-1);
			}
		}
	}

	/*
	 * Free the previously-allocated memory.
	 */
	if(malloc_req)
	{
		(void)xmfree(k_list, kernel_heap);
	}

	/*
	 * If a selnotify() has occurred, but we didn't have to
	 * poll_wait() for an event to occur, then we must clear
	 * the event (EVENT_SYNC) that selnotify() e_post()ed.
	 */
	if (!(temp_threadp->t_atomic & TSELECT))
	{
		(void)et_wait(EVENT_NDELAY, EVENT_SYNC, EVENT_SHORT);
	}
	else
	{
		/*
		 * clear TSELECT flag
		 */  
		fetch_and_and(&(temp_threadp->t_atomic), ~TSELECT);
	}

	/*
	 * Total #events that occurred.
	 */
	return((num_fdes) | (num_msgs << 16));


}  /* end select */       
