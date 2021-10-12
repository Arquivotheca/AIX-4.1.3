static char sccsid[] = "@(#)48	1.1  src/bos/usr/bin/trcrpt/rpt_blk.c, cmdtrace, bos411, 9428A410j 7/16/91 16:24:18";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: do_s_e, find_blk, find_event
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <malloc.h>
#include <stdlib.h>
#include <sys/trchdr.h>
#include <sys/trchkid.h>
#include "rpt.h"

typedef struct trc_blk {
	long	trcon_offset;			/* seek value for trcon */
	time_t	trcon_time;				/* date time stamp for trcon */
	struct	lblock	trcon_lb;		/* context of logfile at trcon */
	long	trcoff_offset;			/* seek value for trcoff */
	time_t	trcoff_time;			/* date time stamp for trcoff */
};

extern	int		Logtraceid;			/* traceid from logfile, set in getevent */
extern	FILE*	Logfp;

extern	struct	lblock	Lb;			/* lblock used by next_lentry via getevent.
								   We save it when we find a trcon event.  Then
								   restore it if it is the trcon event we want
								   to begin processing on */

/*
 *  find_event:
 *		Find a given event, trcon or trcoff, and fill in the timestamp
 *		and file offset in the block definition structure.
 *  Return:
 *		1 on success.
 *		0 on failure.
 */

static int
find_event(traceid,blk)
int	traceid;
struct trc_blk* blk;
{

	long	offset;		/* offset of event in trace file */
	int		rv;
	struct	lblock save_lb;

	for( Logtraceid=0; Logtraceid != traceid; ) { 	/* find next occurance of traceid */
		offset=ftell(Logfp);						/* keep beginning of event */
		memcpy(&save_lb,&Lb,sizeof(struct lblock));	/* save logical context of log file */
		rv = getevent();
		switch (rv) {
		case (RV_GOOD):		/* got a record, so possibly  get offset and time */
			if (traceid == Logtraceid) {		/* found it */
					if(traceid == HKWDTOHKID(HKWD_TRACE_TRCON)) {
						blk->trcon_offset = offset;
						blk->trcon_time = *(int *)&Eventbuf[8];
						memcpy(&blk->trcon_lb,&save_lb,sizeof(struct lblock));
						return(1);
					}
					else if(traceid == HKWDTOHKID(HKWD_TRACE_TRCOFF)) {
						blk->trcoff_offset = offset;
						blk->trcoff_time = *(int *)&Eventbuf[4];
						return(1);
					}
				}
			continue;

		case (RV_EOF):		/* no more data */
			return(0);

		case (RV_BADFORMAT):		/* badly formatted record, ignore */
			break;

		default:
			Debug("findevent: unknown rv from getevent: %d\n",rv);

		}

		return(0);
	}
}

/*
 * find_blk:
 *		Find the next trace block defined as trcon thru trcoff.
 *		Return:
 *			Null if block not found
 *			Pointer to trcblk structure defining the block.
 */

static struct trc_blk*
find_blk()
{
	struct trc_blk* blk;

	if ((blk=(struct trc_blk *)calloc(1,sizeof(struct trc_blk))) == NULL) {
		perror("calloc");
		exit(1);
	}

	if(! find_event(HKWDTOHKID(HKWD_TRACE_TRCON),blk)) {
		free(blk);
		blk = NULL;
	}
	else if(! find_event(HKWDTOHKID(HKWD_TRACE_TRCOFF),blk)) {
		free(blk);
		blk = NULL;
	}

	return(blk);
}

/*
 *  do_s_e:
 *		Process the -s and/or -e flags by outputing complete
 *		trcon/trcoff blocks.  No block is output that is not
 *		totally included by the supplied times.  Assume that
 *		the times have been checked for validity.
 *
 *  Returns:
 *		Never returns, just exits.
 */

void
do_s_e()
{

	struct trc_blk* cur_blk;
	int	begintime;
	int	stoptime;

	if (Starttime)
		begintime = Starttime;
	else
		begintime = -1;

	if (Endtime)
		stoptime = Endtime;
	else
		stoptime = 0x7fffffff;			/* largest positive int */


	while((cur_blk=find_blk()) != NULL) {		/* get a trace block */
			if(cur_blk->trcon_time > stoptime || cur_blk->trcoff_time > stoptime)
				break;		 /* report done */
			else if(begintime <= cur_blk->trcon_time && stoptime >= cur_blk->trcoff_time) {
				/* report this block, so 1st reset log file pointer to beginning
				 * trcon, and reset the Lb structure to what it was at the
				 * beginning trcon.
				 */
				eseek(cur_blk->trcon_offset);
				memcpy(&Lb,&cur_blk->trcon_lb,sizeof(struct lblock));
				pass2(cur_blk->trcoff_offset);
			}
			free(cur_blk);
	}
	exit(0);
	}
