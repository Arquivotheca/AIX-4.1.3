 static char sccsid[] = "@(#)09  1.8  src/bos/kernel/db/POWER/dbstream.c, sysdb, bos411, 9437B411a 9/13/94 16:53:43";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *		dmodsw(ps) 
 *		fmodsw(ps) 
 *		mblk(ps) 
 *		buckets(ps) 
 *		Streams(ps) 
 *		strqueue(ps) 
 *		do_modsw() 
 *		address_map() 
 *		pr_modsw() 
 *		pr_mblk() 
 *		pr_kmemstats() 
 *		pr_kmembuckets() 
 *		pr_sth() 
 *		pr_queue() 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1984
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <pse/str_stream.h>
#include <sys/xcoff.h>
#include <net/net_malloc.h>
#include "../ldr/ld_data.h"

#define DEF_STORAGE 1
#include "add_cmd.h"			/* defines for Copyin() */

/* debug copies of pointers based in streams-related kernel exts */
/* declared in dbkern.c to make sure they are pinned. */
extern struct modsw** dbp_fmodsw;
extern struct modsw** dbp_dmodsw;
extern struct sth_s** *dbp_sth_open_streams;
extern struct msgb** dbp_mh_freelater;
extern struct sqh_s* dbp_streams_runq;

extern char *getterm();
void do_modsw();

/* WARNING!!!!!!!!!:
 * To avoid a lot of tedious usage of Copyin, we can make the following assump-
 * tion:  All the storage that we are looking at in these routines is pinned.
 * We leave in one line of defense by attempting a Copyin from the base address
 * for each of the following structures:
 * sth_open_streams
 *
 * fmodsw
 * dmodsw
 * mh_freelater
 * streams_runq
 *
 * If the Copyin fails, then for some reason, the base of the structure is not
 * present in memory, so we stop the display process right there.  Otherwise,
 * we assume that everything references to by these variables is pinned, and
 * thus, can avoid using Copyin every time we dereference a pointer.  IF THIS
 * ASSUMPTION CHANGES, ALL DEREFERENCES MUST BE CHANGED TO USE Copyin!
 */

/*
 * NAME: 	fmodsw
 * FUNCTION: 	prints fmodsw
 * RETURNS:	0 always
 */
int fmodsw(ps)
struct parse_out *ps;
{
	/* Locate the start of the target. */
	if (!dbp_fmodsw) {
		printf ("Streams framework has not been loaded yet.\n");
		return;
	}
	do_modsw(*dbp_fmodsw,"fmodsw");
	return(0);
}
/*
 * NAME: 	dmodsw
 * FUNCTION: 	prints dmodsw
 * RETURNS:	0 always
 */
int dmodsw(ps)
struct parse_out *ps;
{
	/* Locate the start of the target. */
	if (!dbp_fmodsw) {
		printf ("Streams framework has not been loaded yet.\n");
		return;
	}
	do_modsw(*dbp_dmodsw,"dmodsw");
	return(0);
}
/*
 * NAME: 	do_modsw
 * FUNCTION: 	prints modsw
 * RETURNS:	void
 */
void do_modsw(struct modsw* thehead, char* name)
{
	struct modsw *thefirst;

	thefirst = (struct modsw *) 0;
/* Print it out as long as user wants and modsw is valid. */
	while (1) {
		if (!Copyin((ulong) thehead, cbbuf, sizeof(struct modsw))) {
			printf ("%s paged out of memory at %x.\n",name,*thehead);
			return;
		}
		if (!(thehead->d_next) || thehead->d_next == thefirst) {
			printf ("\nThis is the last entry.\n");
		}
		if (pr_modsw(name,thehead))
			break;
		else 
			if (thehead->d_next && thehead->d_next != thefirst) {
				thehead = thehead->d_next;
				if (!thefirst)
					thefirst = thehead;
			} else
				break;
	}
	return;
}

/*
 * NAME: 	mblk
 * FUNCTION: 	prints mblk or kmemstat structs
 * RETURNS:	0 always
 */
int mblk(ps)
struct parse_out *ps;
{
struct msgb *thehead;
int i,j;
	/* Determine if we have an address passed. */
	if (ps->num_tok >= 1) {
		if (ps->token[1].tflags & HEX_OVERFLOW) {
			printf("Usage: mblk [addr].\n");
			return(0);
		}
		thehead = ps->token[1].hv;
	/* Print it out as long as the user wants. */
		while (1) {
			if (!Copyin(thehead, cbbuf, sizeof(struct msgb))) {
				printf ("%08x paged out of memory.\n",thehead);
				return(0);
			}
			if (pr_mblk(thehead))
				break;
			else 
				if (thehead->b_next)
					thehead = thehead->b_next;
				else
					printf ("\nThis is the last entry.\n");
		}
	} else {	/* print the mblk stats. */
		i = M_MBLK;
		while (1) {
			if (!Copyin(&kmemstats[i], cbbuf, sizeof(struct kmemstats))) {
				printf ("kmemstats paged out of memory.\n");
				return (0);
			}
			j = pr_kmemstats(&kmemstats[i],i);
			if (j == 1)
				break;
			if (j == 2) {
				if (i > 0) 
					i--;
				else
					printf ("\nThis is the first entry.\n");
			} else {
				if (i < M_LAST)
					i++;
				else
					printf ("\nThis is the last entry.\n");
			}
		}
	}
	return(0);
}
/*
 * NAME: 	buckets
 * FUNCTION: 	prints kmembucket structs
 * RETURNS:	0 always
 */
int buckets(ps)
struct parse_out *ps;
{
int i,j;
	i = 0;
	while (1) {
		if (!Copyin(&bucket[i], cbbuf, sizeof(struct kmembuckets))) {
			printf ("kmembuckets paged out of memory.\n");
			return (0);
		}
		j = pr_kmembuckets(&bucket[i],i);
		if (j == 1)
			break;
		if (j == 2) {
			if (i > 0) 
				i--;
			else
				printf ("\nThis is the first entry.\n");
		} else {
			if (i < (MINBUCKET + 16)) 
				i++; 
			else
				printf ("\nThis is the last entry.\n");
		}
	}
	return(0);
}
/*
 * NAME: 	Stream
 * FUNCTION: 	prints Stream head structures
 * RETURNS:	0 always
 */
int Stream(ps)
struct parse_out *ps;
{
struct sth_s *thehead, **openone;
int i,exit_flag = 0;
struct sth_s working_sth;

	openone = NULL;
	/* Determine if we have an address passed. */
	if (ps->num_tok >= 1) {
		if (ps->token[1].tflags & HEX_OVERFLOW) {
			printf("Usage: streams [addr].\n");
			return(0);
		}
		thehead = ps->token[1].hv;
	} else {	/* find one. */
		/* Locate the start of the target. */
		if (!dbp_sth_open_streams) {
			printf ("Streams framework has not been loaded yet.\n");
			return 0;
	}
		if (!Copyin( (ulong) dbp_sth_open_streams, cbbuf, 
				(STH_HASH_TBL_SIZE * sizeof(thehead)))) {
			printf ("sth_open_streams is paged out of memory.\n");
			return(0);
		}
		openone = (struct sth_s **) cbbuf;
		for (i=0; i < STH_HASH_TBL_SIZE; i++) {
			if (*openone) 
				break;
			openone++;
		}
		if (!(*openone)) {
			printf ("open stream head not found.\n");
			return(0);
		}
		thehead = *openone;
	}
	while (1) {
		/* Print out all sth_s structs on this chain */
		while (1) {
			if (!Copyin(thehead, &working_sth, sizeof(struct sth_s))) {
				printf ("%08x paged out of memory.\n",thehead);
				return(0);
			}
			if (pr_sth(thehead,&working_sth)) { /* User typed 'x'?*/
				exit_flag = 1;	/* Set flag for outer loop */
				break;		/* Leave inner loop */
			}
			else 
				if (working_sth.sth_next)
					thehead = working_sth.sth_next;
				else {
					break;
				}
		}

		/* user typed 'x', so we stop displaying stream heads. */
		if (exit_flag) break;

		/* user specified address on command line, so only print one chain */
		if (!openone) break;
		for (i++, openone++; i < STH_HASH_TBL_SIZE; i++, openone++) {
			if (*openone) 
				break;
		}
		if (i>=STH_HASH_TBL_SIZE) break;
		thehead = *openone;
	}
	return(0);
}
/*
 * NAME: 	strqueue
 * FUNCTION: 	prints Streams queue structures
 * RETURNS:	0 always
 */
int strqueue(ps)
struct parse_out *ps;
{
struct queue *thehead;

	/* Validate the address passed. */
	if (ps->num_tok >= 1) {
		if (ps->token[1].tflags & HEX_OVERFLOW) {
			printf("Usage: queue [addr].\n");
			return(0);
		}
		thehead = ps->token[1].hv;
	} 
	/* Print it out as long as the user wants. */
	while (1) {
		if (!Copyin(thehead, cbbuf, sizeof(struct queue))) {
			printf ("%08x paged out of memory.\n",thehead);
			return(0);
		}
		if (pr_queue(thehead))
			break;
		else 
			if (thehead->q_next)
				thehead = thehead->q_next;
	}
	return(0);
}
/*
 * NAME: 	pr_modsw
 * FUNCTION: 	prints modsw
 * RETURNS:	0 if user wants more. 1 if no more
 */
int pr_modsw(header,thetarget) 
char *header;
struct modsw *thetarget;
{
struct modsw *thehead;

	thehead = (struct modsw *) cbbuf;
	printf("\n%s\n",header);
	printf("address..................%08x\n", thetarget);
	printf("next.....................%08x\n", thehead->d_next);
	printf("previous.................%08x\n", thehead->d_prev);
	printf("name.....................%08s\n", thehead->d_name);
	printf("flags....................%08x\n", thehead->d_flags);
	printf("sqh......................%08x\n", thehead->d_sqh);
	printf("streamtab................%08x\n", thehead->d_str);
	printf("level....................%08d\n", thehead->d_sq_level);
	printf("refcnt...................%08d\n", thehead->d_refcnt);
	printf("major....................%08d\n", thehead->d_major);
	printf("\nPress enter to display next entry, x to exit: ");
	if (*getterm() == 'x')
		return(1);
	else
		return(0);
}
/*
 * NAME: 	pr_sth
 * FUNCTION: 	prints a Stream head
 * PARAMETERS:	actual memory addr of sth_s, addr of copy of sth_s
 * RETURNS:	0 if user wants more. 1 if no more
 */
int pr_sth(thetarget,sth_copy) 
struct sth_s *thetarget, *sth_copy;
{

	printf("address..................%08x\n", thetarget);
	printf("read queue...............%08x\n", sth_copy->sth_rq);
	printf("write queue..............%08x\n", sth_copy->sth_wq);
	printf("device number (x)........%08x\n", sth_copy->sth_dev);
	printf("read mode................%08x\n", sth_copy->sth_read_mode);
	printf("write mode...............%08x\n", sth_copy->sth_write_mode);
	printf("close wait timeout.......%08d\n", sth_copy->sth_close_wait_timeout);
	printf("read error...............%08x\n", sth_copy->sth_read_error);
	printf("write error..............%08x\n", sth_copy->sth_write_error);
	printf("flags....................%08x\n", sth_copy->sth_flags);
	printf("ioctl id (x).............%08x\n", sth_copy->sth_ioc_id);
	printf("ioctl mb.................%08x\n", sth_copy->sth_ioc_mp);
	printf("write offset (x).........%08x\n", sth_copy->sth_wroff);
	printf("tty info.................%08x\n", sth_copy->shttyp);
	printf("next.....................%08x\n", sth_copy->sth_next);
	printf("poll queue...............%08x\n", sth_copy->sth_pollq);
	printf("signal queue.............%08x\n", sth_copy->sth_sigsq);
	printf("push count...............%08d\n", sth_copy->sth_push_cnt);
	printf("\nPress enter to display next entry, x to exit: ");
	if (*getterm() == 'x')
		return(1);
	else
		return(0);
}
/*
 * NAME: 	pr_mblk
 * FUNCTION: 	prints mblk
 * RETURNS:	0 if user wants more. 1 if no more
 */
int pr_mblk(thetarget) 
struct msgb *thetarget;
{
struct msgb *thehead;

	thehead = (struct msgb *) cbbuf;
	printf("address..................%08x\n", thetarget);
	printf("next.....................%08x\n", thehead->b_next);
	printf("previous.................%08x\n", thehead->b_prev);
	printf("cont.....................%08x\n", thehead->b_cont);
	printf("rptr.....................%08x\n", thehead->b_rptr);
	printf("wptr.....................%08x\n", thehead->b_wptr);
	printf("datap....................%08x\n", thehead->b_datap);
	printf("message priority.........%08x\n", thehead->b_band);
	printf("message flags............%08x\n", thehead->b_flag);
	printf("\nPress enter to display next entry, x to exit: ");
	if (*getterm() == 'x')
		return(1);
	else
		return(0);
}
/*
 * NAME: 	pr_kmemstats
 * FUNCTION: 	prints kmemstats
 * RETURNS:	0 for down. 1 for quit. 2 for up.
 */
int pr_kmemstats(thetarget, offset) 
struct kmemstats *thetarget;
int offset;
{
struct kmemstats *thehead;
struct msgb *mhfree;
char *c;

	thehead = (struct kmemstats *) cbbuf;
	printf("\nmblk displaying kmemstats for offset %d\n",offset);
	printf("address..................%08x\n", thetarget);
	printf("inuse..(x)...............%08x\n", thehead->ks_inuse);
	printf("calls..(x)...............%08x\n", thehead->ks_calls);
	printf("memuse..(x)..............%08x\n", thehead->ks_memuse);
	printf("limit blocks..(x)........%08x\n", thehead->ks_limblocks);
	printf("map blocks..(x)..........%08x\n", thehead->ks_mapblocks);
	printf("maxused..(x).............%08x\n", thehead->ks_maxused);
	printf("limit..(x)...............%08x\n", thehead->ks_limit);
	printf("failed..(x)..............%08x\n", thehead->ks_failed);
	printf("lock..(x)................%08x\n", thehead->ks_lock);
	if (!dbp_mh_freelater) 
		printf ("mh_freelater has not been defined.\n");
	else
		printf("\nmh_freelater...........%08x\n", *dbp_mh_freelater);
	printf("\nPress enter for next entry, u for previous entry, x to exit:");
	c = getterm();
	if (*c == 'x')
		return(1);
	if (*c == 'u')
		return(2);
	return(0);
}
/*
 * NAME: 	pr_kmembuckets
 * FUNCTION: 	prints kmembuckets
 * RETURNS:	0 for down. 1 for quit. 2 for up.
 */
int pr_kmembuckets(thetarget, offset) 
struct kmembuckets *thetarget;
int offset;
{
struct kmembuckets *thehead;
char *c;

	thehead = (struct kmembuckets *) cbbuf;
	printf("\ndisplaying kmembucket for offset %d size %d\n",offset, (1 << (offset+1)));
	printf("address..................%08x\n", thetarget);
	printf("b_next..(x)..............%08x\n", thehead->kb_next);
	printf("b_calls..(x).............%08x\n", thehead->kb_calls);
	printf("b_total..(x).............%08x\n", thehead->kb_total);
	printf("b_totalfree..(x).........%08x\n", thehead->kb_totalfree);
	printf("b_elmpercl..(x)..........%08x\n", thehead->kb_elmpercl);
	printf("b_highwat..(x)...........%08x\n", thehead->kb_highwat);
	printf("b_couldfree (sic)..(x)...%08x\n", thehead->kb_couldfree);
	printf("b_failed..(x)............%08x\n", thehead->kb_failed);
	printf("lock..(x)................%08x\n", thehead->kb_lock);
	printf("\nPress enter for next entry, u for previous entry, x to exit: ");
	c = getterm();
	if (*c == 'x')
		return(1);
	if (*c == 'u')
		return(2);
	return(0);
}
/*
 * NAME: 	pr_queue
 * FUNCTION: 	prints a queue
 * RETURNS:	0 if user wants more. 1 if no more
 */
int pr_queue(thetarget) 
struct queue *thetarget;
{
struct queue *thehead;

	thehead = (struct queue *) cbbuf;
	printf("address..................%08x\n", thetarget);
	printf("queue info...............%08x\n", thehead->q_qinfo);
	printf("head of msg queue........%08x\n", thehead->q_first);
	printf("next.....................%08x\n", thehead->q_next);
	printf("ptr......................%08x\n", thehead->q_ptr);
	printf("flag..(x)................%08x\n", thehead->q_flag);
	printf("\nPress enter to display next entry, x to exit: ");
	if (*getterm() == 'x')
		return(1);
	else
		return(0);
}
