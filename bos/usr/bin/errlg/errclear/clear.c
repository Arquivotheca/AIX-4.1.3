static char sccsid[] = "@(#)39	1.6  src/bos/usr/bin/errlg/errclear/clear.c, cmderrlg, bos411, 9428A410j 7/12/94 10:05:31";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: clear, clr_logget, keep_entry, clr_hdr_reset
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


#include <stdio.h>
#include <fcntl.h>
#include <errlg.h>


void clr_hdr_reset(struct log_hdr *lh);
void loggetinit();

static int read_curr = sizeof(struct log_hdr);	/* current read position */
static int write_curr = sizeof(struct log_hdr);	/* current write position */
static int entries_curr;						/* current number entries */
static int save_inoff;							/* save offset for lh_inoff */
static char state = '0';						/* errclear state variable */


/*
 * NAME		clear()
 * FUNCTION Central routine for clearing selected error log entries.
 *			Clear records by omission, that is rewrite all records that
 *			are to be kept.
 * INPUTS	NONE
 * RETURNS	NONE
 */

void
clear(void)
{
	int rv = 0;
	char le[LE_MAX_SIZE];		/* error log entry as read from disk. */
	struct log_hdr log_hdr;

	struct le_bad le_bad;		/* contain code for bad log entry. */

	int entry_size;

	if(logopen(O_RDWR)) {
		genexit(1);
	}
	hdrlock();
	loggetinit();		/* initialize the sequence id, etc. */

	if(!(rv=get_log_hdr(&log_hdr)) && log_hdr.lh_entries > 0) {

		for(;;) {				/* for all records in the log */

			if((rv=clr_logget(&le,&log_hdr,&le_bad)) <= LE_BAD_LEN2)
				break;	/* no more log entries or there is something    */
                        /* wrong with this entry such that the rest     */
                        /* of the log file cannot be processed.         */
                    
                        /* If it's a valid entry, rv will be greater than             */ 
                        /* LE_MIN_SIZE.  So, if it's a valid entry - and if it meets  */
                        /* the selection criteria - write it back out. (If it's an    */
                        /* invalid entry - rv is less than LE_MIN_SIZE - we have      */
                        /* already ruled out a problem with the entry length, so we   */
                        /* will just skip past it and continue to process the rest of */
                        /* the log file.)                                             */

			if ( (rv >= LE_MIN_SIZE) && (!lst_chk()) ){ 
				if(rv=keep_entry(&le,&log_hdr)) /* If we were unable to write the */
					break;                  /* entry back to the logfile, break */
                                            /* out and reset the header with    */
                                            /* the current log pointer values.  */
				else
					logfsyncmode(1); /* This will cause an fsync */
			}
				
		}
		clr_hdr_reset(&log_hdr);
		logfsyncmode(1);			/* This will cause an fsync */

	}
	hdrunlock();
	exit(rv);
}



/* 
 * NAME		clr_logget()
 * FUNCTION	Get a log entry for an errclear process.  This 'get' is in
 *		physical file order of the log entries, NOT in logical order.
 *		This will in effect compress the file by expanding the contiguous
 *		available space in the file.  Since the errlog may have a hole in
 *		it, two phases of processing may be required.  The first phase is
 *		processing the records before the hole.  The second phase is proces-
 *		sing the records after the hole.
 *		The process states are defined as:
 *			0 - initialization
 *			1 - 1st phase; beginning of file through lh_inoff entries.
 *			    Begin writes at beginning of file.
 *			2 - 2nd phase when records saved in first phase;  lh_staleoff
 *			    backwards through lh_outoff entries.  Begin writes at end of file.
 *			3 - 2nd phase when reords NOT saved in first phase;  lh_outoff
 *			    backwards through lh_staleoff entries.  Begin writes at begining
 *			    of file.
 *
 * INPUTS
 *		pointer to a log entry structure
 *		pointer to a log header structure
 *		pointer to a bad entry structure
 * RETURN
 *		< 0 error
 *		= 0 no record available.
 *		> 0 number bytes in record.
 */

int
clr_logget(char *le,struct log_hdr *lh,struct le_bad *le_bad)
{
	int rc;
	int esize;

        /* The possible return codes from udb_get_log_entry() are as follows:  */
        /* 0:  Failure                                                         */
        /* 1:  Bad length value in log entry                                   */
        /* 2:  Length at beginning of entry doesn't match length at end        */
        /* 3:  Bad magic number in entry                                       */
        /* 4:  Bad sequence number in entry                                    */
        /* 6:  Bad machine id in entry                                         */
        /* 7:  Bad node id in entry                                            */
        /* LE_MIN_SIZE through LE_MAX_SIZE:   size of entry                    */ 
        /* If the return code does not indicate failure or corrupted length    */
        /* values in the entry, but if it indicates a corrupted entry, the     */
        /* the entry will not be written back to the logfile. The errclear     */
        /* command, however, will continue to process the logfile.             */


	if(state < '2' && read_curr < lh->lh_inoff) {		/* 1st phase of processing */
		state = '1';
		if((rc=udb_get_log_entry(le,read_curr,le_bad)) >= LE_BAD_MAGIC)	{ /* got an entry */
			esize=*(int *)le;
			read_curr += esize;					/* add curr esize */
		}
	}
	else if(state == '1' && read_curr <= lh->lh_outoff) {/* end first phase process */
		if(entries_curr > 0) {		/* entries saved in phase 1 */
			state = '2';
			save_inoff = write_curr;	/* this will be lh_inoff, when finished. */
			read_curr = lh->lh_staleoff;	/* skip to end of data. */
			write_curr = lh->lh_maxsize;	/* write at end of file */
			if((rc=udb_get_log_entry_rv(le,read_curr,le_bad)) >= LE_BAD_MAGIC) { /* got entry */
				esize=*(int *)le;
				read_curr -= esize;					/* sub curr esize */
			}
		}
		else {	/* no entries saved in phase 1 */
			state = '3';
			read_curr = lh->lh_outoff;
			if((rc=udb_get_log_entry(le,read_curr,le_bad)) >= LE_BAD_MAGIC) { /* got entry */
				esize=*(int *)le;
				read_curr += esize;					/* add curr esize */
			}
		}

	}
	else if(state == '2' && read_curr > lh->lh_outoff) {		/* 2nd phase processing */
		if((rc=udb_get_log_entry_rv(le,read_curr,le_bad)) >= LE_BAD_MAGIC) {	/* got entry */
			esize=*(int *)le;
			read_curr -= esize;					/* sub curr esize */
		}
	}
	else if(state == '3' && read_curr < lh->lh_staleoff) {
		if((rc=udb_get_log_entry(le,read_curr,le_bad)) >= LE_BAD_MAGIC)	{ /* got entry */
			esize=*(int *)le;
			read_curr += esize;					/* add curr esize */
		}		
	}
	else 	/* done */
		rc = 0;
	
	return(rc);
}

/* 
 * NAME		keep_entry()
 * FUNCTION Put a log entry for an errclear process.  This 'put' is in
 *			physical file order, NOT in logical order.  The put is
 *			either beginning at the current write position (forward
 *			processing) or ending at the current write position (reverse
 *			processing when there is file wrap.)
 * INPUTS	pointer to a log entry
 *			pointer to a log header structure
 * RETURN
 *		! 0 error
 *		= 0 success
 */

int
keep_entry(char *le,struct log_hdr *lh)	/* write to the first available place */
{
	int rc;
	int count = *(int *)le;

	switch (state) {
	case '0':
	case '1':
	case '3':		/* forward direction */
		if((rc=logwrite(le, count, write_curr)) == 0) {	/* success */
			entries_curr++;
			write_curr += count;
		}
		break;

	case '2':		/* backward direction */
		if((rc=logwrite(le, count, (write_curr-count))) == 0) { 	/* success */
			write_curr-=count;
			entries_curr++;
		}
		break;
	}
		
	
	return(rc);
}

/*
 * NAME		clr_hdr_reset()
 * FUNCTION	Reset the log header pointers following the clearing of error
 *			records.
 * INPUTS	pointer to a log header structure.
 * RETURN	NONE
 */

void
clr_hdr_reset(struct log_hdr *lh)
{
	if ( entries_curr == 0)	{	/* no entries left */
		lh->lh_outoff = sizeof(struct log_hdr);
		lh->lh_inoff = sizeof(struct log_hdr);
		lh->lh_staleoff = lh->lh_inoff;
	}
	else {
		switch (state) {
		case '0':
		case '1':
		case '3':	/* normal forward write */
			lh->lh_inoff = write_curr;
			lh->lh_outoff = sizeof(struct log_hdr);
			lh->lh_staleoff = lh->lh_inoff;
			break;

		case '2':	/* hole in log file. */
			if (write_curr >= lh->lh_maxsize) {	/* no entries left in 2nd part */
				lh->lh_outoff = sizeof(struct log_hdr);
				lh->lh_staleoff = save_inoff;
			}
			else {
				lh->lh_outoff = write_curr;
				lh->lh_staleoff = lh->lh_maxsize;
			}
			lh->lh_inoff = save_inoff;
		}
	}

	lh->lh_entries = entries_curr;

	logwrite(lh,sizeof(struct log_hdr),0);
}
