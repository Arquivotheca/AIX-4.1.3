static char sccsid[] = "@(#)95        1.38  src/bos/kernel/io/errdd.c, syserrlg, bos41J, 9520A_all 5/11/95 19:30:01";

/*
 * COMPONENT_NAME: SYSERRLG   /dev/error pseudo-device driver
 *
 * FUNCTIONS: errread, errput, errsave, errnv_write, err_wakeup_dd,
 * set_state, reset_state, errcdtf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * /dev/error device driver
 * This file contains the pinned portion of the device driver.
 * The pageable functions are in errdd_pg.c, and the functions
 * used during system initialization are in errdd_si.c.
 * None of these routines run on a fixed stack.
 */

#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/systm.h>
#include <sys/sysmacros.h>
#include <sys/low.h>
#include <sys/m_intr.h>
#include <sys/malloc.h>
#include <sys/uio.h>
#include <sys/lockl.h>
#include <sys/intr.h>
#include <sys/trchkid.h> 
#include <sys/sleep.h>
#include <sys/mdio.h>
#include <sys/dump.h>
#include <sys/err_rec.h>
#include <sys/erec.h>
#include <sys/syspest.h>
#include <sys/lock_def.h>
#include <sys/lock_def.h>
#include <errno.h>
#include "errdd.h"

#ifdef _POWER_MP
#include <sys/thread.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
Simple_lock errdd_lock;
#endif /* _POWER_MP */

int errread(dev_t mmdev,struct uio *uiop);
int errnv_write(char *ptr,int len);
void errput(struct errvec *ev,int len);
static void errpagable(struct err_rec *, int);
static void errput_active_demon(struct errvec *ev,int len);
static void errput_inactive_demon(struct errvec *ev,int len);
void err_wakeup_dd(int flag);
void set_state(int flag);
void reset_state(int flag);
void errsave(struct err_rec *ep, int len);
struct cdt_head *errcdtf();


int	lockl_rc;		/* return code from lockl */

struct errc errc = {	/* buffer control structure */
	0,					/* state */
	LOCK_AVAIL,			/* e_lockword */
	EVENT_NULL,			/* e_sleepword_dd */
	EVENT_NULL,			/* e_sleepword_sync */
	0,					/* e_eize */
	(char *)0,			/* e_start */
	(char *)0,			/* e_end */
	(char *)0,			/* e_inptr */
	(char *)0,			/* e_outptr */
	(char *)0,			/* e_stale_data_ptr */
	0,                  /* e_over_write_count */
	0,                  /* e_err_count */
	0,                  /* e_discard_count */
	0,                  /* e_errid_1st_discarded */
	0                   /* e_errid_last_discarded */
};

static struct errc_io errc_io;

struct errstat errstat;



/*
 * Calculate the UNOCCcupied SIZE of the buffer
 */
#define UNOCCSIZE() \
(errc.e_inptr - errc.e_outptr >= 0 ? EREC_MAX : errc.e_outptr - errc.e_inptr)

/* Test to see if we can page fault */
#define CAN_FAULT (csa->intpri == INTBASE)

/*
 * NAME:      errread
 *
 * FUNCTION:  devsw[] read routine for /dev/error
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : Yes
 *    Runs on Fixed Stack  : No
 *    May Page Fault       : Yes
 *    Serialization:       : i_disable/i_enable for UP environment
 *	         	   : disable_lock/unlock_enable for MP environment		
 *
 * DATA STRUCTURES:
 *    errc.e_state, errc.e_outptr
 *
 * RETURNS:
 *   EFAULT   (from uiomove) target address is invalid
 *        0   Success
 */

int
errread(dev_t mmdev,struct uio *uiop)
{
	int mdev;
	int erec_len;
	int ucount;
	char buf[EREC_MAX];
	int rv_error;
	struct erec *ep;
	int intpri;
	rv_error = 0;

	if(uiop->uio_resid == 0)
		return(0);
loop:

	if(!(errc.e_state & ST_RDOPEN)) {	/* return EOF */
		goto ret;
	}
	if(errc.e_state & ST_STOP) {
		reset_state(ST_STOP);
		goto ret;		/* return EOF */
	}
	if(errc.e_err_count == 0){ /* empty buffer, nothing to read */
		struct erec erec;
		errnv_write((char*)&erec,0);	/* 0 length will zero out nvram */

		/* If interrupts are not disabled before setting
		   the state and left disabled, the sleep flag could
		   potentially get cleared before going to sleep
		   resulting in a sleep with no chance of wakeup.
		   Refer to defect 85290.  */

#ifdef _POWER_MP
		intpri = disable_lock(INTMAX, &errdd_lock);
#else 
		intpri = i_disable(INTMAX);
#endif /* _POWER_MP */
		errc.e_state |= ST_SLEEP;	

                /* Wait for an error to be logged.  The errput routines will
		   wake us up after placing and error into the buffer.
		   We will also be awakened if the errioctl for errstop is
		   executed.  We want to make sure also that this wait will be
		   terminated by the occurrence of signals (requested with
		   the INTERRUPTIBLE flag in e_sleep_thread and the 
		   EVENT_SIGRET flag in e_sleep).  If we don't request this,
		   the errdemon would not be killed immediately with the 
		   kill command (it would sit in this wait until errstop
		   was issued or until the next error was logged - which
		   would end up being lost). Defect 176056. */

#ifdef _POWER_MP
                rv_error = e_sleep_thread(&errc.e_sleepword_dd, &errdd_lock, LOCK_HANDLER|INTERRUPTIBLE);

#else
		rv_error = e_sleep(&errc.e_sleepword_dd, EVENT_SIGRET);
#endif /* _POWER_MP */


		/* This if statement was added for defect 176056 to take
		   care of exiting when the errdemon process is killed.
		   We are only prepared to handle termination from 
		   sleep due to an error being logged (errcount > 0) or
		   to errstop being issued (ST_STOP bit set).  Anything
		   other than these reasons (ie. kill signal) will be
		   considered abnormal, and we will exit as though errstop
		   had been issued. */

		if((errc.e_err_count == 0) && !(errc.e_state & ST_STOP)) {
			errc.e_over_write_count = 0;
#ifdef _POWER_MP
			unlock_enable(intpri, &errdd_lock);
#else
			i_enable(intpri);
#endif /* _POWER_MP */
			goto ret;
		}


#ifdef _POWER_MP
		unlock_enable(intpri, &errdd_lock);
#else
		i_enable(intpri);
#endif /* _POWER_MP */
		goto loop;

	}
#ifdef _POWER_MP
	intpri = disable_lock(INTMAX, &errdd_lock);
#else
	intpri = i_disable(INTMAX);
#endif /* _POWER_MP */
	ep = (struct erec *)errc.e_outptr;
	erec_len = ep->erec_len;

	/* The erec_len should never be greater than EREC_MAX, as a check
	   is made in both errsave() and errwrite(), and if the error length
	   is greater, it adjusts the length to equal EREC_MAX.  So we will
	   assume that if the length value stored in the buffer is now 
	   greater than EREC_MAX, that somehow the buffer has been corrupted.
	   If debug is enabled (ie. bosboot -D), and the following ASSERT
       is encountered, we will fall into the low-level debugger.  If
       debug is not enabled, this ASSERT will be a no-op, and an attempt
       will be made to just silently recover by resetting the buffer. 

       ASSERT(erec_len <= EREC_MAX);                 */ 

 if(erec_len > EREC_MAX) {

		ERR_TRCHKL(ERRREAD2,0,erec_len);
		errc.e_outptr = errc.e_start;	/* only way to fix buffer */
		errc.e_inptr = errc.e_start;
		errc.e_err_count = 0;
		errc.e_over_write_count = 0;
		errc.e_stale_data_ptr = errc.e_start;
		errc.e_discard_count = 0;
		errc.e_errid_1st_discarded = 0;
		errc.e_errid_last_discarded = 0;
#ifdef _POWER_MP
		unlock_enable(intpri, &errdd_lock);			/* restore old interrupt priorities */
#else
		i_enable(intpri);
#endif /* _POWER_MP */
		goto loop;
	}
	ucount = MIN(uiop->uio_resid,erec_len);
	bcopy(errc.e_outptr,buf,ucount);
	errc.e_outptr += erec_len;
	if ( (errc.e_outptr >= errc.e_end) ||
	     (errc.e_outptr >= errc.e_stale_data_ptr) )
	{
		if (errc.e_outptr == errc.e_inptr)
		{
			errc.e_inptr  = errc.e_start;
			errc.e_outptr = errc.e_start;
			errc.e_stale_data_ptr = errc.e_inptr;
			errc.e_err_count = 0;
		}
		else
		{
			errc.e_outptr = errc.e_start;
			errc.e_stale_data_ptr = errc.e_inptr;
			errc.e_err_count--;
		}
	}
	else
		errc.e_err_count--;


        if (errc.e_outptr == errc.e_inptr)
        {
                errc.e_inptr  = errc.e_start;
                errc.e_outptr = errc.e_start;
                errc.e_stale_data_ptr = errc.e_inptr;
                errc.e_err_count = 0;
	}

#ifdef _POWER_MP
	unlock_enable(intpri, &errdd_lock);			/* restore old interrupt priorities */
#else
	i_enable(intpri);
#endif /* _POWER_MP */

	rv_error = uiomove(buf,ucount,UIO_READ,uiop);
ret:
	if(rv_error) {
		errstat.es_readerr++;
		ERR_TRCHK(ERRREAD,rv_error);
	}
	return(rv_error);
}

/*
 * NAME:	errput
 *
 * FUNCTION:  Call the appropriate errput routine.
 *
 * INPUT:
 *    ev     errvec vector for data areas to be moved.
 *    len    length in bytes of the err_rec structure
 *
 * RETURNS:  none
 */
void 
errput(struct errvec *evp, int len)
{
	if (errc.e_state & ST_RDOPEN)
		errput_active_demon(evp,len);
	else
		errput_inactive_demon(evp,len);
}

/*
 * NAME:      errput_active_demon
 *
 * FUNCTION:  Common buffer write routine for errwrite and errsave
 *			  for when the errdemon is active
 *
 * INPUTS:
 *    ev     errvec vector for data areas to be moved.
 *    len    length in bytes of the err_rec structure
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : No
 *    Runs on Fixed Stack  : No
 *    May Page Fault       : No
 *    Serialization:       : i_disable/i_enable for UP environment
 *			   : disable_lock/unlock_enable for MP environment
 *
 *
 * RETURNS:     NONE
 *
 */

static void
errput_active_demon(struct errvec *ev,int len)
{
	struct erec *epinptr, *epoutptr;
	struct err_rec *current_event;
	int intpri, prev_overflow, curr_overflow, i;
	char *p;

	/* If the error count field of errc is less than zero
	   at the start of this routine, there's something very
	   wrong and processing should not continue. 
	 */
	assert(errc.e_err_count >= 0);

	/*
	 * Put an error record into the kernel error buffer.
	 * The record will not always be added to the buffer. 
	 * If there is not enough room in the buffer for the
	 * error, it will be discarded. 
	 * Since no overwriting of events will take place, there
	 * is only one type of wrapping to be concerned with:
	 * 	 Buffer wrap: Buffer wrap is simply the setting of
	 *   the pointers to the start of the buffer when an
	 *	 operation would cause them to run off the end of
	 *   the buffer.
	 *
	 */
	
#ifdef _POWER_MP
	intpri = disable_lock(INTMAX, &errdd_lock);
#else
	intpri = i_disable(INTMAX);
#endif /* _POWER_MP */

	curr_overflow = 0;


	if (errc.e_discard_count > 0) {
		/* A previous overflow condition existed, and certain
		   information about it must be logged in addition to  
		   the current error.  The processing at the end 
		   will log a "lost events" entry before the current 
		   entry if the prev_overflow flag is set and if it is  
		   determined that there is enough room in the buffer
		   for both entries. */
		prev_overflow = 1;
		len+= LOST_EVTS_ERRLEN + sizeof(struct erec0);
	 }
	 else
		prev_overflow = 0;


	/*
	 * Case 1: in < out
 	 *   _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _   
	 * 	|_|_|_|_|_|_|_ _ _ _ _ _ _ _|_|_|_|_|_|_|
 	 *	start	    in		       out	        end
 	 */

	if (errc.e_inptr < errc.e_outptr) { 
		if (errc.e_inptr + len > errc.e_outptr) { 
			/* overflow condition */
			if (errc.e_discard_count == 0) 
				errc.e_errid_1st_discarded  
					= ev->error_id;
			errc.e_discard_count++;
			errc.e_errid_last_discarded 
				= ev->error_id;
			/* Set the curr_overflow flag. This will be used 
			   to determine whether or not the error should
			   be written to the buffer. */
			curr_overflow = 1;
		}
	} /* end if (in < out) */

	/*
	 * Case 2: in = out
	 *
	 *  - full buffer :
 	 *   _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _  
	 * 	|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|
 	 *	start	        in/out		            end
	 *
	 *  - or - empty buffer :               
 	 *   _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _  
	 * 	|_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|
 	 *	start/in/out	        	            end
	 *
 	 */

	else if (errc.e_inptr == errc.e_outptr) { 
		if (errc.e_err_count > 0) { 
			/* Buffer is full. This is an overflow. */
			if (errc.e_discard_count == 0) 
				errc.e_errid_1st_discarded  
					= ev->error_id;
			errc.e_discard_count++;
			errc.e_errid_last_discarded 
				= ev->error_id;
			/* Set the curr_overflow flag. This will be used 
			   to determine whether or not the error should
			   be written to the buffer. */
			curr_overflow = 1;
		}
		else { /* non-overflow, empty buffer */

			errc.e_inptr = errc.e_start;
			errc.e_outptr = errc.e_start;
			errc.e_stale_data_ptr = errc.e_inptr;

		}
	} /* end else if (in == out) */
					
	/*
	 * Case 3: in > out
	 *
 	 *   _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _  
	 * 	|_|_|_|_|_|_|_|_|_|_|_ _ _ _ _ _ _ _ _ _|
 	 *	start/out	        in		            end
	 *
	 *  - or -                
 	 *   _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _  
	 * 	|_ _ _ _ _ _ _ _ _ _|_|_|_|_|_|_|_ _ _ _|
 	 *	start    	        out         in      end
	 *
 	 */

	else {
		if ((errc.e_inptr + len) > errc.e_end) {
			/* Possible overflow condition -
			   Won't fit at the end of the buffer.
			   Check to see if it will fit at the
			   beginning of the buffer, but don't 
			   adjust the inptr or the staleptr
			   unless it does. */
			if ((errc.e_start + len) > errc.e_outptr) {
				/* Overflow condition -	
				   It doesn't fit at the beginning or
				   end of the buffer, so discard it. */ 
				if (errc.e_discard_count == 0) 
					errc.e_errid_1st_discarded  
						= ev->error_id;
				errc.e_discard_count++;
				errc.e_errid_last_discarded 
					= ev->error_id;
				/* Set the curr_overflow flag. This will be used 
			   	   to determine whether or not the error should
			   	   be written to the buffer. */
				curr_overflow = 1;
			}
			else { /* This is a non-overflow condition and a 
					  buffer wrap. It doesn't fit at the end
					  of the buffer, but it will fit at the
					  beginning of it. */
				errc.e_stale_data_ptr = errc.e_inptr;
				errc.e_inptr = errc.e_start;
			}
		}
	} /* end else if (in > out) */

	if (curr_overflow == 0) {
		if (prev_overflow == 1) {

			/* First write the lost events error
			   directly to the buffer, then clear
			   the errc overflow variables. */

			errc.e_err_count++;
			epinptr = (struct erec*)errc.e_inptr;
			epinptr->erec_rec_len = LOST_EVTS_ERRLEN;
			epinptr->erec_len = epinptr->erec_rec_len +
					sizeof(struct erec0);
			epinptr->erec_symp_len = 0;
			epinptr->erec_timestamp = time;
			epinptr->erec_rec.error_id = ERRID_LOST_EVENTS;
			sprintf(epinptr->erec_rec.resource_name,"%s",
					"syserrlg");
			sprintf(epinptr->erec_rec.detail_data,
					"%08x%08x%d", 
					errc.e_errid_1st_discarded,
					errc.e_errid_last_discarded,
					errc.e_discard_count); 
			if (errc.e_inptr == errc.e_stale_data_ptr)
				errc.e_stale_data_ptr += epinptr->erec_len;
			errc.e_inptr += epinptr->erec_len;
			errstat.es_errcount++;
			len -= (LOST_EVTS_ERRLEN + sizeof(struct erec0));

			errc.e_discard_count = 0;
			errc.e_errid_1st_discarded = 0;
			errc.e_errid_last_discarded = 0;

		} /* end if (prev_overflow == 1) */


		/* Copy data into buffer at inptr. */
		errc.e_err_count++;
		epinptr = (struct erec *)errc.e_inptr;

		/* Copy each data element to the buffer */
		for (i=0, p=(char *)epinptr; i<ev->nel; i++) {
			bcopy(ev->v[i].p,p,ev->v[i].len);
			len -= ev->v[i].len;
			p += ev->v[i].len;
		}
		assert(len == 0); /* Bad news if len didn't match vector! */

		epinptr->erec_timestamp = time;	/* Set the time stamp */

		if (errc.e_inptr == errc.e_stale_data_ptr)
			errc.e_stale_data_ptr += epinptr->erec_len;
		errc.e_inptr += epinptr->erec_len; /* len + erec_len + timestamp */
		errnv_write((char*)epinptr, epinptr->erec_len);
		errstat.es_errcount++;
#ifdef _POWER_MP
		/* The following is code from err_wakeup_dd.  This needs to
		   go here, because we can't have nested simple locks. */
               	errc.e_state |= 0;
               	if (errc.e_state & ST_SLEEP) {
                       errc.e_state &= ~ST_SLEEP;
                       e_wakeup(&errc.e_sleepword_dd);
               	}
#else
		err_wakeup_dd(0);
#endif /* _POWER_MP */
	} /* end if (curr_overflow == 0) */

#ifdef _POWER_MP
	unlock_enable(intpri, &errdd_lock);
#else
	i_enable(intpri);
#endif /* _POWER_MP */
} /* end errput_active_errdemon() */


/*
 * NAME:      errput_inactive_demon()
 *
 * FUNCTION:  common buffer write routine for errwrite and errsave
 *
 * INPUTS:
 *    ev     errvec vector for data areas to be moved.
 *    len    length in bytes of the err_rec structure
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : No
 *    Runs on Fixed Stack  : No
 *    May Page Fault       : No
 *    Serialization:       : i_disable/i_enable for UP environment
 *			   : disable_lock/unlock_enable for MP environment
 *
 * DATA STRUCTURES:
 *    errc.e_state, errc.e_inptr
 *
 * RETURNS:     NONE
 *
 */

static void
errput_inactive_demon(struct errvec *ev,int len)
{
	struct erec *epinptr, *epoutptr;
	unsigned old_over;
	int intpri, i;
	char *p;

	/* If the error count field of errc is less than zero
	   at the start of this routine, there's something very
	   wrong and processing should not continue. 
	 */
	assert(errc.e_err_count >= 0);

	/*
	 * put an error record into the kernel error buffer.
	 * The record will always be added to the buffer.  This
	 * could entail writing over the oldest record.
	 * There are two types of wrapping to be concerned with:
	 * 	1. Buffer wrap: Buffer wrap is simply the setting of
	 *		the pointers to the start of the buffer when an
	 *		operation would cause them to run off the end of
	 *		the buffer.
	 *	2. Data wrap: Data wrap occurs when the data is not
	 *		removed from the buffer fast enough and unprocessed
	 *		data will be written over.
	 *
	 * Note:
	 *	If an error record is too big to fit in the remainder of
	 *	the buffer, then it will be added at the start.
	 *	The good data to process always starts out and fills through.
	 *
	 * The following summarizes the wrapping that will take place
	 * for each of the possible states:
	 *
	 * State  1:  in < out
     * State 1A:  in<out && in+len>out && in+len<=end
	 *            data wrap, no buffer wrap
	 * State 1B:  in<out && in+len>out && in+len>end
	 *            data wrap, buffer wrap
	 * State 1C:  in < out && in+len<out && in+len<=end
	 *            no data wrap, no buffer wrap
	 *
	 * State  2:  in = out
	 * State 2A:  in=out && err_count>0 && in+len<=end
	 *            data wrap, no buffer wrap
	 * State 2B:  in=out && err_count>0 && in+len>end
	 * 			  data wrap, buffer wrap
     * State 2C:  in=out && err_count=0
	 *            no data wrap, no buffer wrap
	 *
	 * State  3:  in > out
	 * State 3A:  in>out, && in+len>end && start+len<=outptr
	 *			  buffer wrap, no data wrap
	 * State 3B:  in>out && in+len>end && start+len>outptr
	 *			  buffer wrap, data wrap
	 * State 3C:  in>out && in+len<=end
	 *            no buffer wrap, no data wrap
	 *
	 */
	
#ifdef _POWER_MP
	intpri = disable_lock(INTMAX, &errdd_lock);
#else
	intpri = i_disable(INTMAX);
#endif /* _POWER_MP */

	/*
	 * Case 1: in < out
	 * 
	 * 	|----------!---------------!------------|
	 *	start	  in		  out		end
	 */

	if (errc.e_inptr < errc.e_outptr)
	{ /* watch out for data wrap*/
		if (errc.e_inptr + len > errc.e_outptr) 
		{
		/*
		 * data wrap, so compute how many records are being
		 * written over and subtract that from the count
		 */
			if (errc.e_inptr + len <= errc.e_end)  /* State 1A */
			{ /* will fit before buffer wrap */
				while (errc.e_outptr < errc.e_inptr + len)
				{ /* write over this record */
					if (errc.e_outptr>=errc.e_stale_data_ptr)
					{
						errc.e_outptr = errc.e_end;
						break;
						/*
						 * hitting begin of stale data
						 * if we don't break here,
						 * we're going to trash
						 * valid records. force
						 * e_outptr past end.
						 */
					}
					errc.e_over_write_count++;
					errc.e_err_count--;
					epoutptr =(struct erec *)errc.e_outptr;
					errc.e_outptr += epoutptr->erec_len;
				}
				if (errc.e_outptr >= errc.e_end)
				{
					errc.e_outptr = errc.e_start;
					errc.e_stale_data_ptr = errc.e_inptr + len;
					/*
					 * set all data between end of this new
					 * record and the end to be stale
				 	 */
				}
			}
			else
			{
			/*
			 * State 1B:
			 *
			 * in+len > end, so need to buffer wrap to add the
			 * error.  compute number of records to end.
			 */
				while (errc.e_outptr <= errc.e_end)
				{ /* write over this record */
					if (errc.e_outptr>=errc.e_stale_data_ptr)
					{
						break;
						/*
						 * hitting begin of stale data
						 * if we don't break here,
						 * we're going to trash
						 * valid records.
						 * e_outptr will be set to
						 * e_start after this.
						 */
					}
					errc.e_over_write_count++;
					errc.e_err_count--;
					epoutptr =(struct erec *)errc.e_outptr;
					errc.e_outptr += epoutptr->erec_len;
				}
				errc.e_stale_data_ptr = errc.e_inptr;
				/* consider everything after this stale */
				/*
				 * put the new entry at start and out at the
				 * oldest entry.  The oldest entry should now
				 * be the entry past the new.
				 */
				errc.e_inptr = errc.e_start;
				errc.e_outptr = errc.e_start;
				while (errc.e_outptr < (errc.e_inptr + len))
				{
					if (errc.e_outptr>=errc.e_stale_data_ptr)
					{
						errc.e_stale_data_ptr = 
							errc.e_inptr + len;
						errc.e_outptr = errc.e_start;
						break;
						/*
						 * hitting stale data pointer
						 * in this case should be
						 * very rare.  this must
						 * be a case of a very large
						 * new error coming in.
						 */
					}
					errc.e_over_write_count++;
					errc.e_err_count--;
					epoutptr =(struct erec *)errc.e_outptr;
					errc.e_outptr +=  epoutptr->erec_len;
				}
			}
		}
	} /* end in < out */
	/*
	 * Case 2: in = out
	 *
	 *	|--------------!----------------------|
	 *    start	    in/out		    end
	 */
	else if (errc.e_inptr == errc.e_outptr)
	{ /* buffer is empty or full */
		if (errc.e_err_count > 0)
		{ /* full, so data wrap */
			if ((errc.e_inptr + len) <= errc.e_end) /* State 2A */
			{ /* will fit before buffer wrap */
				while (errc.e_outptr < (errc.e_inptr + len))
				{ /* write over this record. */
					if (errc.e_outptr>=errc.e_stale_data_ptr)
					{
						errc.e_outptr = errc.e_end;

						break;
						/*
						 * hitting begin of stale data
						 * if we don't break here,
						 * we're going to trash
						 * valid records. force
						 * e_outptr past end.
						 */
					}
					errc.e_over_write_count++;
					errc.e_err_count--;
					epoutptr =(struct erec *)errc.e_outptr;
					errc.e_outptr += epoutptr->erec_len;
				}
				if (errc.e_outptr >= errc.e_end)
				{
					errc.e_outptr = errc.e_start;
					/* out buffer wrap */
					errc.e_stale_data_ptr = errc.e_inptr + len;
					/*
					 * set all data between end of this new
					 * record and the end to be stale
				 	 */
				}
			}
			else
			{
			/*
			 * State 2B:
			 *
			 * in+len > end so need to wrap to add the error
			 * compute number of records to end
			 */
				while (errc.e_outptr <=  errc.e_stale_data_ptr)
				{ /* write over this record */
					if (errc.e_outptr==errc.e_stale_data_ptr)
					{
						break;
						/*
						 * hitting begin of stale data
						 * if we don't break here,
						 * we're going to trash
						 * valid records.
						 * e_outptr will be set to
						 * e_start after this.
						 */
					}
					errc.e_over_write_count++;
					errc.e_err_count--;
					epoutptr =(struct erec *)errc.e_outptr;
					errc.e_outptr +=  epoutptr->erec_len;
				}
				errc.e_stale_data_ptr = errc.e_inptr;
				/* consider everything after this stale */
				/* put the new entry at start and out at the
				 * oldest entry.  The oldest entry should now
				 * be the entry past the new.
				 */
				errc.e_inptr = errc.e_start;
				errc.e_outptr = errc.e_start;
				while (errc.e_outptr < errc.e_inptr + len)
				{
					if (errc.e_outptr>=errc.e_stale_data_ptr)
					{
						errc.e_outptr = errc.e_start;
						errc.e_stale_data_ptr = 
							errc.e_inptr + len;
						break;
						/*
						 * hitting stale data pointer
						 * in this case should be
						 * very rare.  this must
						 * be a case of a very large
						 * new error coming in.
						 */
					}
					errc.e_over_write_count++;
					errc.e_err_count--;
					epoutptr =(struct erec *)errc.e_outptr;
					errc.e_outptr +=  epoutptr->erec_len;
				}
			}
		}
		else
		{/* buffer empty so reset to start to delay buf wrapping */
			errc.e_inptr = errc.e_start;
			errc.e_outptr = errc.e_start;
			errc.e_stale_data_ptr = errc.e_inptr;
		}

	}
	else
	{
	/*
	 * Case 3: in > out
	 *
	 *		|------------!----------!---------|
	 * 		start	    out		in       end
	 */
		if ((errc.e_inptr + len) >  errc.e_end) /* State 3A */
		{ /* buffer wrap */
			errc.e_stale_data_ptr = errc.e_inptr;
			/* consider everything after this stale */
			errc.e_inptr = errc.e_start;
			if (errc.e_inptr+len >  errc.e_outptr)
			{/* data wrap - State 3B */ 
				while (errc.e_outptr <  errc.e_inptr+len)
				{ /* write over this record. */
					if (errc.e_outptr>=errc.e_stale_data_ptr)
					{
						errc.e_outptr = errc.e_start;
						errc.e_stale_data_ptr = 
							errc.e_inptr + len;
						break;
						/*
						 * hitting stale data pointer
						 * in this case should be
						 * very rare.  this must
						 * be a case of a very large
						 * new error coming in.
						 */
					}
					errc.e_over_write_count++;
					errc.e_err_count--;
					epoutptr =(struct erec *)errc.e_outptr;
					errc.e_outptr +=  epoutptr->erec_len;
				}
			}
		}
	}
	/* copy data into buffer at in. */
	errc.e_err_count++;

	epinptr = (struct erec *)errc.e_inptr;

	/* Copy each data element to the buffer */
	for (i=0, p=(char *)epinptr; i<ev->nel; i++) {
		bcopy(ev->v[i].p,p,ev->v[i].len);
		len -= ev->v[i].len;
		p += ev->v[i].len;
	}
	assert(len == 0); /* Bad news if len didn't match vector! */

	epinptr->erec_timestamp = time;	/* Set the time stamp */

	if (errc.e_inptr == errc.e_stale_data_ptr)
		errc.e_stale_data_ptr += epinptr->erec_len;
	errc.e_inptr += epinptr->erec_len; /* len + erec_len + timestamp */
	ERR_TRCHK(ERRPUT,0);
	errnv_write((char*)epinptr, epinptr->erec_len);
	errstat.es_errcount++;
#ifdef _POWER_MP
	/* This is code from err_wakeup_dd.  We need this here because
	   we can't have nested simple locks. */
	errc.e_state |= 0;
	if (errc.e_state & ST_SLEEP) {
		errc.e_state &= ~ST_SLEEP;
		e_wakeup(&errc.e_sleepword_dd);
	}
#else
	err_wakeup_dd(0);
#endif /* _POWER_MP */

#ifdef _POWER_MP
	unlock_enable(intpri, &errdd_lock);
#else
	i_enable(intpri);
#endif /* _POWER_MP */
}

/*
 * NAME:      errsave
 *
 * FUNCTION:  error logging kernel service
 *
 * INPUTS:
 *    ep     pointer to err_rec structure (see err_rec.h)
 *    len    length in bytes of the err_rec structure
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : Yes
 *    Runs on Fixed Stack  : No
 *    May Page Fault       : Yes
 *    Serialization:       : none needed
 *    NOTE:  We may be called disabled or not.  If disabled, we can't
 *    page fault, but we can assume the data passed in is pinned.
 *
 * DATA STRUCTURES:
 *    errc.e_state, errc.e_inptr
 *
 */

void
errsave(struct err_rec *ep, int len)
{
	/* We use 2 address vector elements, one for the
	 * erec header (erec0) and one for the passed data.
	 */
	ALOC_ERRVEC(2) av;
	struct erec0 err_hdr;	/* Header record */
	int data_len = sizeof(struct erec0)+len;

	if (dump_started)
		return;
	if(errc.e_start == NULL)
		return;
	if(len > ERR_REC_MAX_SIZE)
		len = ERR_REC_MAX_SIZE;
	
	/* If we can page fault, go off and handle that case. */
	if (CAN_FAULT) {
		errpagable(ep,len);
		return;
	}

	/* Setup error header (erec0) */
	err_hdr.erec_len = sizeof(err_hdr)+len;
	err_hdr.erec_rec_len = len;
	err_hdr.erec_symp_len = 0;

	/* Setup the address vector */
	av.error_id = ep->error_id;
	av.nel = 2;
	av.v[0].p = &err_hdr;	/* Get the data type header */
	av.v[0].len = sizeof(err_hdr);
	av.v[1].p = ep;		/* point to the data */
	av.v[1].len = len;

	/* Put the data in the error log buffer */
	errput(&av,data_len);
}

/*
 * NAME:      errpagable
 *
 * FUNCTION:  error logging kernel service for pagable Kernel code.
 *	If the caller can page fault, we'd better copy their error
 *	log entry to pinned space before calling errput which
 *	disables.
 *
 * INPUTS:
 *    ep     pointer to err_rec structure (see err_rec.h)
 *    len    length in bytes of the err_rec structure
 *
 * EXECUTION ENVIRONMENT:
 *    Called from errsave if we're not disabled.
 *
 */
static void
errpagable(struct err_rec *ep, int len)
{
	int data_len;
	struct erec *e;
	struct errvec *evp;		/* Address vector for errput() */

	/* Allocate pinned buffer space.
	 * This must include the address vector.
	 */
	data_len = len+sizeof(struct erec0);
	if (!(e = (struct erec *)xmalloc(data_len+sizeof(struct errvec),BPWSHIFT,pinned_heap))) {
		ERR_TRCHK(ERRWRITE,EAGAIN);
		return;
	}
	/* Point to address vector. */
	evp = (struct errvec *)((int)e+data_len);

	/* Get the data */
	bcopy(ep,&e->erec_rec,len);

	/* Setup the error record header and address vector */
	e->erec_len = data_len;
	e->erec_rec_len = len;
	e->erec_symp_len = 0;
	evp->error_id = e->erec_rec.error_id;
	evp->nel = 1;
	evp->v[0].p = e;
	evp->v[0].len = data_len;

	/* Put the data in the buffer. */
	errput(evp,data_len);

	xmfree(e,pinned_heap);

	return;
}

/*
 * NAME:     errcdtf()
 * FUNCTION: Called by dmp_do at dump time.
 * RETURNS:  Returns a pointer to a component dump table (cdt).
 *           The cdt for errlog is the errlog buffer control structure 
 *           errc and the buffer itself.
 */

static struct {
	struct cdt_head  _cdt_head;
	struct cdt_entry  cdt_entry[3];
} errcdt = {
	{ DMP_MAGIC, "errlg", sizeof(errcdt) },
{	{ "errc",    sizeof(errc),    (char *)&errc,    0 },
	{ "errc_io", sizeof(errc_io), (char *)&errc_io, 0 },
	{ "log",     0,               0,                0 }   }
};

struct cdt_head *errcdtf()
{

	errc_io.e_start  = errc.e_start;
	errc_io.e_end    = errc.e_end;
	errc_io.e_inptr  = errc.e_inptr;
	errc_io.e_outptr = errc.e_outptr;
	errc_io.e_stale_data_ptr = errc.e_stale_data_ptr;
	errc_io.e_over_write_count = errc.e_over_write_count;
	errc_io.e_err_count = errc.e_err_count;
	errc_io.e_discard_count = errc.e_discard_count;
	errc_io.e_errid_1st_discarded = errc.e_errid_1st_discarded;
	errc_io.e_errid_last_discarded = errc.e_errid_last_discarded;

	errcdt.cdt_entry[2].d_len = errc.e_size;
	errcdt.cdt_entry[2].d_ptr = errc.e_start;
	return((struct cdt_head *)&errcdt);
}

/*
 * NAME:     errnv_write()
 * FUNCTION: Read/write one err_rec0 structure from offset ERRLOG.0 of nvram
 * RETURNS:  1 Success
 *          -1 Failure
 */
int
errnv_write(char *ptr, int len)
{
	int rv;
	union errnv_buf errnv_buf;

	if(len > sizeof(errnv_buf))
		len = sizeof(errnv_buf);
	if(len == 0)
	{
		/*
		  Make sure all length fields in erec structure are zeroed
		 */
		bzero(&(errnv_buf.erec), sizeof(errnv_buf.erec));
	}
	else {
		bcopy(ptr,&errnv_buf,len);
		errnv_buf.erec.erec_len = len;
	}
	rv = nvwrite(3,(char*)&errnv_buf,0,sizeof(errnv_buf));
	if(rv != sizeof(errnv_buf)) 
		return(-1);
	return(1);
}


/*
 * NAME:     err_wakeup_dd()
 * FUNCTION: Wakes up processes waiting on a non-empty buffer.
 *           (Refer to the definition of the errc structure in
 *            errdd.h for a description of the intended use of
 *            the sleep words.)
 * RETURNS:  NONE
 */

void
err_wakeup_dd(int flag)
{
	int intpri;

#ifdef _POWER_MP
	intpri = disable_lock(INTMAX, &errdd_lock);
#else 
	intpri = i_disable(INTMAX);
#endif /* _POWER_MP */

	errc.e_state |= flag;
	if (errc.e_state & ST_SLEEP) {
		errc.e_state &= ~ST_SLEEP;
		e_wakeup(&errc.e_sleepword_dd);
	}
#ifdef _POWER_MP
	unlock_enable(intpri, &errdd_lock);
#else
	i_enable(intpri);
#endif /* _POWER_MP */
}


/*
 * NAME:     set_state()
 * FUNCTION: Sets the state bit in errc.e_state as specified by
 *           the flag parameter, disabling and enabling interrupts
 *           around this operation.
 * RETURNS:  NONE
 */

void
set_state(int flag)
{
	int intpri;

#ifdef _POWER_MP
	intpri = disable_lock(INTMAX, &errdd_lock);
#else
	intpri = i_disable(INTMAX);
#endif /* _POWER_MP */

	errc.e_state |= flag;
	
#ifdef _POWER_MP
	unlock_enable(intpri, &errdd_lock);
#else
	i_enable(intpri);
#endif /* _POWER_MP */
}



/*
 * NAME:     reset_state()
 * FUNCTION: resets the state bit in errc.e_state as specified by
 *           the flag parameter, disabling and enabling interrupts
 *           around this operation.
 * RETURNS:  NONE
 */

void
reset_state(int flag)
{
	int intpri;

#ifdef _POWER_MP
	intpri = disable_lock(INTMAX, &errdd_lock);
#else
	intpri = i_disable(INTMAX);
#endif /* _POWER_MP */

	errc.e_state &= ~flag;

#ifdef _POWER_MP
	unlock_enable(intpri, &errdd_lock);
#else
	i_enable(intpri);
#endif /* _POWER_MP */
}


