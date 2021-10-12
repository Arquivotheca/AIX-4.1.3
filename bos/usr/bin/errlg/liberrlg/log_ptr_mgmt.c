static char sccsid[] = "@(#)09	1.5  src/bos/usr/bin/errlg/liberrlg/log_ptr_mgmt.c, cmderrlg, bos411, 9428A410j 6/14/94 13:12:41";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: set_append_pointer, move_out_data_wrap
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <errlg.h>

#ifdef DUMPLOG
/********************************************************************/
/*   Global variables to be used in debugging logfile pointer       */
/*   management problems.  To use this, compile with -D option      */
/*   ie. "build OPT_LEVEL=-DDUMPLOG"                                */
/*   Debug information for this errdemon will then go to            */
/*   /tmp/errlog.dump.                                              */


extern int  Global_New_Staleoff;
extern int  Global_New_Outoff;
extern int  Global_Entry_Length;

/**************************************************************************/
#endif /* DUMPLOG */

int newend;


int set_append_pointer(struct log_hdr *lh, int length);
void move_out_data_wrap(struct log_hdr *lh, int end);

/*
 ******************************************************************************
	Strategy for dynamic error log entry pointer management for a circular
	error log with entry length at both ends of each entry.  The duplicate
	length entries will facilitate the reading of the error log forwards or
	backwards.  This is important because the 'normal' mode of errpt is to
	present the selected entries in reverse order of occurance.

	The log header will contain the pointers to be managed for the circular
	error log.  They are:
		lh_inoff	The seek value for the next write of an entry.
		lh_outoff	The seek value for the next read of an entry.
		lh_staleoff	The seek value for the beginning of any stale data.
					Stale data is created when an entry will not fit at
					the physical end of the file, and is wrapped.  That
					means the space following the last entry in the file
					is STALE and cannot be processed.  The stale data will
					occupy the rest of the physical file.

	The log header will also contain values used in conjunction with the
	aformentioned management pointers.  They are:
		lh_entries	The number of entries contained in the log.
		lh_maxsize	The maximum size, or wrap value, for the error log file.

	There are two kinds of wrapping we are concerned with: 
		file wrap	Is simply the implementation of the circularness of the
					file.
		data wrap	Is the destruction of 'old' data by writing over it.

	Adding Entries
		Entries will be added by errdemon, errdead, and errclear.  Adding 
		entries can cause file and data wrap.  An entry can be added at
		lh_inoff.  The pointer management will be done before the add.

	Reading Entries
		Entries will be read by errpt and errclear.  Forwards by concurrent 
		errpt and errclear, and backwards by all other modes.  An assumption
		here is that records being fetched by notify methods will on average
		have more records in front of them than behind them.  Reading entries
		will need to follow file wrap.

	Deleting Entries
		Entries will be deleted by omission by errclear.  Remember to reset 
		management pointers to the physical beginning of the file when no
		more entries are left.  Deleting entries can cause file unwrap.
 ******************************************************************************
 */


 /*
  * NAME:      set_append_pointer()
  * FUNCTION:  Set the in offset for an append.  This function implements the 
  *            circularness, of the log file by wrapping the in offset, when
  *            appropriate, and writing over the oldest data.
  * RETURNS:   0 - no file wrap occured.
  *			   1 - file wrap occured.
  */

int
set_append_pointer(struct log_hdr *lh, int length)
{
	int stopat;
	int wrap = FALSE;		/* true when adding entry will wrap the log file. */

	newend = lh->lh_inoff + length;

#ifdef DUMPLOG
	Global_Entry_Length = length;
#endif  /* DUMPLOG */


/*
 * Case 1: in < out
 *
 *     |------!----------!------|
 *   start   in         out    max
 */
	if(lh->lh_inoff < lh->lh_outoff) {
		/* watchout for data and/or file wrap */
		if (newend > lh->lh_outoff) {	/* data wrap */
			if (newend <= lh->lh_maxsize) { /* fits before end */
				if (newend >= lh->lh_staleoff) 
					stopat = lh->lh_staleoff;
				else
					stopat = newend;

				move_out_data_wrap(lh,stopat);

			}
			else {		/* newend > maxsize; so data and file wrap */
				wrap = TRUE;
				move_out_data_wrap(lh,lh->lh_staleoff); /* get to end */
				lh->lh_staleoff = lh->lh_inoff;
				lh->lh_inoff = sizeof(struct log_hdr);
				newend = sizeof(struct log_hdr)+length;
				move_out_data_wrap(lh,newend);
			}
		}
	}	/* end in < out */
						
/*
 * Case 2: in = out
 *
 *     |----------!-----------|
 *   start      in/out       max
 */
	else if (lh->lh_inoff == lh->lh_outoff) {		/* empty or full */
		if (lh->lh_entries > 0)	{	/* full so data wrap, maybe file too */
			if(newend <= lh->lh_maxsize) {		/* only data wrap */
				if(newend > lh->lh_staleoff) 
					stopat = lh->lh_staleoff;
				else
					stopat = newend;

				move_out_data_wrap(lh,stopat);

			}
			else {		/* newend > maxsize; so data and file wrap. */
				wrap = TRUE;
				move_out_data_wrap(lh,lh->lh_staleoff); /* get to end */
				lh->lh_staleoff = lh->lh_inoff;
				lh->lh_inoff = sizeof(struct log_hdr);
				newend = sizeof(struct log_hdr)+length;
				move_out_data_wrap(lh,newend);
			}
		}
		else {		/* log file empty, just add entry at start. */
			lh->lh_inoff = sizeof(struct log_hdr);
			lh->lh_outoff = sizeof(struct log_hdr);
			lh->lh_staleoff = lh->lh_inoff;
		}
	}	/* end in = out */
				
/*
 * Case 3: in > out
 *
 *     |------!----------!------|
 *   start   out        in     max
 */
	else {	/* in > out; no stale data by definition */
		if(newend > lh->lh_maxsize) {	/* file wrap */
			wrap = TRUE;
			lh->lh_staleoff = lh->lh_inoff;
			lh->lh_inoff = sizeof(struct log_hdr);	/* reset to beginning */
			newend = lh->lh_inoff + length;
			if(newend > lh->lh_outoff) 	/* data wrap too */
				move_out_data_wrap(lh,newend);
		}
		else
			lh->lh_staleoff = newend;
	}
	/* Now  ready to add the record at lh->lh_inoff. */

#ifdef DUMPLOG
	Global_New_Staleoff = lh->lh_staleoff;
	Global_New_Outoff = lh->lh_outoff;
#endif  /* DUMPLOG */

	return (wrap);
}


 /*
  * NAME:      move_out_data_wrap()
  * FUNCTION:  Move the out offset for data wrap. Subtract the number of entries
  *            that will be destroyed, and move the lh_outoff pointer to the next
  *            entry that would not be destroyed.
  *            Note:  The movement of the lh_outoff pointer is only forward
  *            through the end pointer.  If lh_outoff ends up in the stale
  *            data, it will be moved to the beginning of the file.
  * RETURNS:   None
  */

void
move_out_data_wrap(struct log_hdr *lh, int end)
{
	int	esize;
	int	stopat;

	if (end < lh->lh_staleoff)
		stopat = end;
	else
		stopat = lh->lh_staleoff;

	while(lh->lh_outoff < stopat) {	 /* compute # records destroyed. */ 
		if(logread(&esize,sizeof(log_entry.le_length),lh->lh_outoff))
			break;	/* error */
		else if(esize <= LE_MAX_SIZE && esize >= LE_MIN_SIZE) {
			lh->lh_outoff += esize;
			lh->lh_entries--;
		}
	}
	if(lh->lh_outoff >= lh->lh_staleoff) {	/* no stale data left. */
		lh->lh_outoff = sizeof(struct log_hdr);
		lh->lh_staleoff = newend;
	}
}
