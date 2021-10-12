static char sccsid[] = "@(#)54	1.12  src/bos/kernext/sol/solproc.c, sysxsol, bos411, 9428A410j 7/20/92 13:05:05";

/*
 * COMPONENT_NAME: (SYSXSOLDD) Serial Optical Link Device Driver 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/trcmacros.h>	/* tracing stuff */
#include <sys/trchkid.h>	/* trace hook ids */
#include <sys/syspest.h>	/* for assert, BUGXDEF */
#include <sys/time.h>		/* for timerstruc_t */
#include <sys/sleep.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/timer.h>		/* for trb */
#include <sys/intr.h>		/* for i_disable */
#include <sys/malloc.h>		/* for xmalloc */
#include <sys/comio.h>
#include <sys/soluser.h>

#include "soldd.h"

#include "sol_proto.h"
#include "sol_extrn.h"

extern vm_cflush();

extern void tservice();

extern struct sol_ddi sol_ddi;

static void recover_imcs_one ();
static void pid_conflict();
static void rehash_sets();

/* send header we must use only one to ensure correctness */
caddr_t send_header = NULL;
static boolean_t send_header_used = FALSE;
static boolean_t returned_from_imcs;

/* saved machine id (uname system call is not pinned) */
static int mach_ids[IMCS_PROC_LIMIT];

/* hello work to do */
static struct sla_addrmap slaam[MAX_NUM_SLA]; /* addresses */
static uchar sla_work[MAX_NUM_SLA][MAX_SWITCH_ADDR];
static uchar hello_in_progress[MAX_NUM_SLA];
static int position = -1;
#define NO_WORK		0


/* timer blocks: one per sla, and one more for good luck */
void * timer_sp[MAX_NUM_SLA];

static void * rec_ts;
uchar rec_ts_used;

/* the variable setq_map should be static (internal).  I have made it
   accessible from other modules (imcs_cdd.c) to hunt down a bug
*/
uint setq_map = 0;
static uchar set_queue_save = INVALID_SETQ;
static int faked_sla_id = INVALID_SLAID;


/* addresses that are invalid sla addresses either because they are switch 
addresses of because they cause sla errors.  0x00 marks the end of the table */

static uchar forbidden_addr[] = {
	0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe,
	/* switch addresses */
	0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74,
	0x84, 0x94, 0xa4, 0xb4, 0xc4, 0xd4, 0xe4, 0xf4,
	/* ending in 4 is not allowed */
	0x0b, 0x2b, 0x4b, 0x6b, 0x8b, 0xab, 0xcb, 0xeb,
	/* even beginning and ending in b is not allowed */
	0x00};



#define NUM_PAGENO	256

struct ipool {
	int number;		/* number of frames currently available */
	int top;		/* top of stack */
	int pageno[NUM_PAGENO];
};

static volatile struct ipool ipool;

int 	bad_port[MAX_NUM_SLA];

extern struct que que;

/**/
/*
 * NAME: setq_alloc
 *                                                                    
 * FUNCTION: allocates the next available set queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: set queue number 
 */  

static int
setq_alloc()
{
	int i;
	int set_queue;

	set_queue = INVALID_SETQ;
	for (i = 0; i < IMCS_SETQ_LIMIT && set_queue == INVALID_SETQ; i++) {
		uint mask;
		mask = 1 << i;
		if (! (mask & setq_map) ) {
			set_queue = (uchar) i;
			setq_map |= mask;
		}
	}

	return set_queue;

}  /* end setq_alloc */


/**/
/*
 * NAME: sq_slas_set
 *                                                                    
 * FUNCTION: returns the set of sla's in a set
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: bit mapped representation of slas in a set queue
 */  

static uint
sq_slas_set(set_queue)
int set_queue;
{
	uint ret_val;
	int sla_id;

	ASSERT(set_queue >= 0 && set_queue < IMCS_SETQ_LIMIT);

	ret_val = 0;

	for (sla_id = 0; sla_id < MAX_NUM_SLA; sla_id ++)
		if (sla_present(sla_id) &&
		    (sla_tbl.sla[sla_id].set_queue == set_queue))
			ret_val |= 1 << sla_id;

	if (set_queue_save == set_queue)
		ret_val |= 1 << faked_sla_id;

	return ret_val;
}  /* end sq_slas_set */


/**/
/*
 * NAME: pid_slas_set
 *                                                                    
 * FUNCTION: returns the set of slas that make a pid accessible
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 */  

static uint
pid_slas_set(imcs_pid)
int imcs_pid;
{
	uint ret_val;
	int i;

	ASSERT(imcs_pid > 0 && imcs_pid < IMCS_PROC_LIMIT);

	ret_val = 0;

	for (i = 0; i < MAX_NUM_SLA; i++)
		if (imcs_addresses.proc[imcs_pid].sla[i] != INVALID_SLAID)
			ret_val |= 1 << imcs_addresses.proc[imcs_pid].sla[i];

	return ret_val;
}  /* end pid_slas_set */


/**/
/*
 * NAME: merge_sets
 *                                                                    
 * FUNCTION: places the sla's the set in the set that services imcs
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: set queue for this pid ????
 */  

static int
merge_sets(imcs_pid, set_queue)
int imcs_pid, set_queue;
{
	int pid, sla_id;

	ASSERT(set_queue >= 0 && set_queue < IMCS_SETQ_LIMIT);
	ASSERT(set_queue != imcs_addresses.proc[imcs_pid].set_queue);
	ASSERT(imcs_pid > 0 && imcs_pid < IMCS_PROC_LIMIT);

	for (pid = 0; pid < IMCS_PROC_LIMIT; pid++)
		if (imcs_addresses.proc[pid].set_queue == set_queue)
			imcs_addresses.proc[pid].set_queue = 
					imcs_addresses.proc[imcs_pid].set_queue;

	for (sla_id = 0; sla_id < MAX_NUM_SLA; sla_id++)
		if (sla_present(sla_id) && 
		    (sla_tbl.sla[sla_id].set_queue == set_queue))
			sla_tbl.sla[sla_id].set_queue = 
					imcs_addresses.proc[imcs_pid].set_queue;

	setq_map &= ~(1 << set_queue);

        cdd_qdelete(set_queue);

/*	cdd_merge(set_queue, imcs_addresses.proc[imcs_pid].set_queue); */

	return imcs_addresses.proc[imcs_pid].set_queue;
}  /* end merge_sets */


/*
/**/
/*
 * NAME: remove_sla_from_set
 *                                                                    
 * FUNCTION: remove the sla from the set queue 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
remove_sla_from_set(sla_id)
int sla_id;
{
	int set_queue;
        int pid;
        int rehash_needed = FALSE;


	/* find the set queue for this sla (note, it could be saved in set_queue_save) */
	set_queue = sla_tbl.sla[sla_id].set_queue;
	if (set_queue == FAKE_QUEUE)  {
		ASSERT(sla_id == faked_sla_id);
		set_queue = set_queue_save;
	}

	/* remove the sla from the set queue */
	if (sla_tbl.sla[sla_id].set_queue != FAKE_QUEUE)
		sla_tbl.sla[sla_id].set_queue = INVALID_SETQ;
	else
		set_queue_save = INVALID_SETQ;

	if (set_queue != INVALID_SETQ) {
		/* if no other servers in the set queue, free the setq */
               if (! sq_slas_set(set_queue) ) {
                       for(pid=0; pid < IMCS_PROC_LIMIT; pid++) {
                                       if (imcs_addresses.proc[
					    pid].set_queue == set_queue) {
                                       if (imcs_sla_count(pid) > 0)
                                               rehash_needed = TRUE;
                                       }
                       }
                       if (rehash_needed) rehash_sets();
                       else  {
                               setq_map &= ~(1 << set_queue);
                               cdd_qdelete(set_queue);
                       }
               }
       }

}  /* remove_sla_from_set */



/**/
/*
 * NAME: imcs_addr_add
 *                                                                    
 * FUNCTION: stores the triplets <imcs_pid, sla_id, sla_address> so that
 *       1) sla whose id is sla_id will be used (among oher things) to send 
 *	    messages to processor imcs_pid
 *       2) upon receiving a message on sla sla_id, and with address 
 *	    sla_address, imcs will correctly identify the sender as imcs_pid
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * set commentary follows:
1) There is no set for the pid, and the sla belongs to no set (this would be
the initial state).  In this case allocate a new set and place the sla in it.
Assign this set to the pid.  This case can occur in both in both switched
and point to point connections.

2) There is no set for the pid, but the sla belongs to a set.  This implies
that the only way to talk to this pid is through this sla.  If the set to
which the sla belongs has only one element, namely the sla, then assign this
set to the pid.  Otherwise allocate a new set, place the sla in it removing
it from its previous set.  Assign this new set to the pid.  This case can
only occur in switched connections.

3) The pid has a set, but the sla does not belong to any set.  This implies
that the sla reaches only this pid. If the other sla's in the set can reach only
this pid, then place the sla in the set assigned to the pid.  Otherwise,
allocate a new set, and place the sla in it.  Assign the set to the pid.
This case can occur in swithed and point to point connections.  However,
for point to point connection it will always be the case that the sla's in
the set talk only to this processor.

4) The pid has a set, and the sla belongs to a set.  If the sets are the
same then no further processing is needed.  If the set are not the same,
then compute the set of pid's reachable through the sla's in the set to
which sla belongs, and the set of pid's reachable though the sla's in the
set assigned to pid.  If this sets are equal then merge the sets (place
all sla's in one set, and assign this set to all pids involved.  Deallocate
one set).  Otherwise leave thing alone.  This case can only occur in
switched connections.

Note that since the sla might be "faked" we will use the saved set queue
instead that the set to which the sla belong at this time

 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
imcs_addr_add(imcs_pid, sla_id, sla_address)
int imcs_pid;
int sla_id;
slalinka_t sla_address;
{
	boolean_t equal, duplicate = FALSE;
	int i, j, k;
	int sla_setq;
	uchar pids[IMCS_PROC_LIMIT];
	struct sol_open_struct	*open_ptr;
	cio_stat_blk_t	stat_blk;
	int tmp_sla_id;

	ASSERT(imcs_pid > 0 && imcs_pid < IMCS_PROC_LIMIT);
	ASSERT(sla_id >= 0 && sla_id < MAX_NUM_SLA);
	ASSERT(sla_address != INVALID_SLA_ADDR);

SOL_TRACE("adrB",imcs_pid,sla_id,sla_address);

	/* first look for duplicates */
	for (i = 0; i < MAX_NUM_SLA && ! duplicate; i++) {
		if (imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i] == 
								sla_address)
			duplicate = TRUE;
	}

	if (duplicate) return;

	/*
	 *  Send notifications to all users about this new processor id.
	 */
	for (i = 0 ; i < SOL_TOTAL_NETIDS ; i += 2) {
		open_ptr = sol_ddi.netid_table[i>>1];
		if (open_ptr != NULL) {
			stat_blk.code = CIO_ASYNC_STATUS;
			stat_blk.option[0] = SOL_NEW_PRID;
			stat_blk.option[1] = imcs_pid;
			sol_report_status(open_ptr, &stat_blk);
		}
	}
	for (i = 0; i < MAX_NUM_SLA && 
			imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i] !=
		    	INVALID_SLA_ADDR;
	    i++);

	ASSERT(i < MAX_NUM_SLA);

	imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i] = sla_address;
	imcs_addresses.proc[imcs_pid].sla[sla_id] = (slaid_t) sla_id;

	/* record this address comes from processor pid in the inverted table */
	ASSERT(imcs_addresses.sla_addr_tb[sla_id][sla_address] != 
							FORBIDDEN_SLA_ADDR);
	ASSERT(imcs_addresses.sla_addr_tb[sla_id][sla_address] == INVALID_PID);
	imcs_addresses.sla_addr_tb[sla_id][sla_address] = (short) imcs_pid;


	sla_setq = sla_tbl.sla[sla_id].set_queue;

	if (sla_setq == FAKE_QUEUE) {
		ASSERT(sla_id == faked_sla_id);
		sla_setq = set_queue_save;
	}

	if (imcs_addresses.proc[imcs_pid].set_queue == INVALID_SETQ) {
		if (sla_setq == INVALID_SETQ) {
			/* CASE 1 */
			sla_setq = setq_alloc();
			ASSERT(sla_setq >= 0 && sla_setq < IMCS_SETQ_LIMIT);
			imcs_addresses.proc[imcs_pid].set_queue=(uchar)sla_setq;
		}
		else {  /* the sla belongs to a set */
			/* CASE 2 */
			ASSERT(sla_tbl.sla[sla_id].connection!= POINT_TO_POINT);
			if (sq_slas_set(sla_setq) == 1 << sla_id) {
				/* the only sla in the set is this sla */
				imcs_addresses.proc[imcs_pid].set_queue = 
								sla_setq;
			}
			else {
				/* there is more than one sla in the set */
				sla_setq = setq_alloc();
				ASSERT(sla_setq>=0 && sla_setq<IMCS_SETQ_LIMIT);
				imcs_addresses.proc[imcs_pid].set_queue = 
								(uchar)sla_setq;
			}
		}
	}
	else {  /* pid has a set */
		bzero(pids, IMCS_PROC_LIMIT);
		for (i = 0; i < MAX_NUM_SLA; i++) {
			for (k = 0; k < IMCS_PROC_LIMIT; k++) {
				for (j = 0; j < MAX_NUM_SLA; j++) {
					if (imcs_addresses.proc[k].sla[j] == i){
						pids[k] |= 1<<i;
					}
				}
			}
		}
		if (sla_setq == INVALID_SETQ) {
			/* CASE 3 */
			equal = TRUE;
			for (tmp_sla_id = 0; tmp_sla_id < MAX_NUM_SLA;
			    tmp_sla_id++){
				if (sla_present(tmp_sla_id) &&
				    (sla_tbl.sla[tmp_sla_id].set_queue ==
				    imcs_addresses.proc[imcs_pid].set_queue)){
					for (k=0 ; k < IMCS_PROC_LIMIT ; k++) {
						if ((pids[k] & (1<<sla_id)) &&
						    (k != imcs_pid)) {
							equal = FALSE;
							break;
						}
					}
				}
			}

			if (equal && (set_queue_save ==
			    imcs_addresses.proc[imcs_pid].set_queue)) {
				for (k=0 ; k < IMCS_PROC_LIMIT ; k++) {
					if ((pids[k] & (1<<faked_sla_id)) &&
					    (k != imcs_pid)) {
						equal = FALSE;
						break;
					}
				}
			}

			if (equal)
				/* all sla's in set talk only to this pid */
				sla_setq = 
					imcs_addresses.proc[imcs_pid].set_queue;
			else {
				/* the set of processors reachable through the 
				set of sla contains elements other than this 
				processor */
				ASSERT(sla_tbl.sla[sla_id].connection != 
								POINT_TO_POINT);
				sla_setq = setq_alloc();
				ASSERT(sla_setq>=0 && sla_setq<IMCS_SETQ_LIMIT);
				imcs_addresses.proc[imcs_pid].set_queue = 
							(uchar) sla_setq;
			}
		}
		else {  /* sla belongs to a set */
			/* CASE 4 */
			for (tmp_sla_id = 0; tmp_sla_id < MAX_NUM_SLA;
			    tmp_sla_id++){
				if (sla_present(tmp_sla_id)) {
					for (k=0 ; k < IMCS_PROC_LIMIT ; k++) {
						if (pids[k] & (1<<tmp_sla_id)) {
							if (sla_tbl.
							    sla[tmp_sla_id].
							    set_queue ==
							    imcs_addresses.
							    proc[imcs_pid].
							    set_queue) {
								pids[k] |=
								    SETQ1;
							}
							if (sla_tbl.
							    sla[tmp_sla_id].
							    set_queue ==
							    sla_setq) {
								pids[k] |=
								    SETQ2;
							}
						}
					}
				}
			}

			if (set_queue_save ==
			    imcs_addresses.proc[imcs_pid].set_queue) {
				for (k=0 ; k < IMCS_PROC_LIMIT ; k++) {
					if (pids[k] & (1<<faked_sla_id)) {
						pids[k] |= SETQ1;
					}
				}
			}
			if (set_queue_save == sla_setq) {
				for (k=0 ; k < IMCS_PROC_LIMIT ; k++) {
					if (pids[k] & (1<<faked_sla_id)) {
						pids[k] |= SETQ2;
					}
				}
			}
			equal = TRUE;
			for (k=0 ; k < IMCS_PROC_LIMIT ; k++) {
				if (((pids[k] & SETQ1) && !(pids[k] & SETQ2)) ||
				    ((pids[k] & SETQ2) && !(pids[k] & SETQ1))) {
					equal = FALSE;
					break;
				}
			}

			ASSERT(sla_tbl.sla[sla_id].connection !=POINT_TO_POINT);
			if ((imcs_addresses.proc[imcs_pid].set_queue !=
			    sla_setq) && equal)
				sla_setq = merge_sets(imcs_pid, sla_setq);
		}
	}

	if (sla_tbl.sla[sla_id].set_queue == FAKE_QUEUE)
		set_queue_save = sla_setq;
	else
		sla_tbl.sla[sla_id].set_queue = (uchar) sla_setq;

SOL_TRACE("adrE",imcs_pid,sla_id,sla_address);

}  /* end imcs_addr_add */


/**/
/*
 * NAME: rehash_sets
 *                                                                    
 * FUNCTION: re-establishes set queues after a deletetion
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
rehash_sets()
{
	int sla1, sla2, found;
	int i, j, imcs_pid;

	int S[MAX_NUM_SLA];		/* set to which sla i belongs */
	uchar pids[IMCS_PROC_LIMIT];	/* sla's that can reach each pid*/
	uchar equal, not_empty, sla1mask, sla2mask;

	setq_map = 0;  /* all setq are available */
	bzero(pids,IMCS_PROC_LIMIT);

	/* initialize the set free map to all allocated,
	   save the set of processors reachable through each sla
	   in P */
	for (i = 0; i < MAX_NUM_SLA; i++) {
		found = FALSE;
		for (imcs_pid = 0; imcs_pid < IMCS_PROC_LIMIT; imcs_pid++) {
			for (j = 0; j < MAX_NUM_SLA; j++) {
				if (imcs_addresses.proc[imcs_pid].sla[j] == i){
					found = TRUE;
					pids[imcs_pid] |= 1<<i;
				}
			}
		}
		if (found) {
			S[i] = i;
			setq_map |= 1 << i;
		}
		else {
			S[i] = INVALID_SETQ;
		}
	}

	/* see if any set can be merged */
	for (sla1 = 0; sla1 < (MAX_NUM_SLA - 1); sla1++) {
		for (sla2 = sla1 + 1; sla2 < MAX_NUM_SLA; sla2++) {
			/*
			 *  Look for a case where a processor is reachable
			 *  through sla1, and the reachable sets are different.
			 */
			equal = TRUE;
			not_empty = FALSE;
			for (imcs_pid = 0 ; imcs_pid < IMCS_PROC_LIMIT ; 
			    imcs_pid++) {
				sla1mask = pids[imcs_pid] & (1<<sla1);
				sla2mask = pids[imcs_pid] & (1<<sla2);
				not_empty |= sla1mask;
				if ((sla1mask && !sla2mask) ||
				    (sla2mask && !sla1mask)) {
					equal = FALSE;
				}
			}
			if (not_empty && equal) {
				/* if not empty and equal sets merge sets */
				setq_map &= ~(1 << S[sla1]);
				S[sla1] = S[sla2];
			}
		}
	}

	/* assign the sets to the slas */
	for (i = 0; i < MAX_NUM_SLA; i++)
		if (sla_tbl.sla[i].set_queue == FAKE_QUEUE) {
			ASSERT(faked_sla_id == i);
			set_queue_save = S[i];
		}
		else
			sla_tbl.sla[i].set_queue = S[i];

	/* now hash in the processors */
	for (imcs_pid = 1; imcs_pid < IMCS_PROC_LIMIT; imcs_pid++) {
		if (imcs_sla_count(imcs_pid) > 0) {
			for (i = (int) (imcs_pid & 0x00000003); ;
			    i = (int) ((i + 1) & 0x00000003) ) {
				if (pids[imcs_pid] & (1<<i)) {
					imcs_addresses.proc[
					    imcs_pid].set_queue = S[i];
					break;
				}
			}
		}
		else 
			imcs_addresses.proc[imcs_pid].set_queue=INVALID_SETQ;
	}

	cdd_reshuffle();  /* and fix things up at the cdd level */

}  /* end rehash_sets */


/**/
/*
 * NAME: imcs_addr_delete
 *                                                                    
 * FUNCTION:  remove the triplet from <*, sla_id, sla_address>
 *	       Now sla sla_id will no longer use sla_address.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
imcs_addr_delete(sla_id, sla_address)
int sla_id;
slalinka_t sla_address;
{
	int imcs_pid, addr_count;
	int i;
	boolean_t found;
	int sla_setq;

	ASSERT(sla_id >= 0 && sla_id < MAX_NUM_SLA);
	ASSERT(sla_address != INVALID_SLA_ADDR);

	/* remove the address from the inverted table */
	imcs_pid = imcs_addresses.sla_addr_tb[sla_id][sla_address];
	imcs_addresses.sla_addr_tb[sla_id][sla_address] = INVALID_PID;

	ASSERT(imcs_pid > 0 && imcs_pid < IMCS_PROC_LIMIT);
	ASSERT(imcs_addresses.proc[imcs_pid].set_queue != INVALID_SETQ);

	/* remove the address from one processor/address table */

	/* find where the address is in the table */
	i = 0;
	found = FALSE;
	while (i < MAX_NUM_SLA && ! found)
		if (imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i] == 
								sla_address)
			found = TRUE;
		else
			i++;
	ASSERT(found);

	/* move the table back one place, counting sla addresses left */
	for (addr_count = i; (i + 1) < MAX_NUM_SLA; i++) {
		imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i] =
		        imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i + 1];
		if (imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i] != 
							INVALID_SLA_ADDR)
			addr_count++;
	}
	/* set the last one to invalid */
	imcs_addresses.proc[imcs_pid].sla_addr[sla_id][i] = INVALID_SLA_ADDR;

	/* invalidate the cache info for the destination processor,
	   if it is this sla */
	if (imcs_addresses.proc[imcs_pid].av_sla == sla_id)
		imcs_addresses.proc[imcs_pid].av_sla = INVALID_SLAID;

	/* if there are no addresses left, invalidate the sla also */
	if (addr_count == 0) {
		int t_imcs_pid;

		/* sla sla_id not longer reaches processor pid */
		imcs_addresses.proc[imcs_pid].sla[sla_id] = INVALID_SLAID;

		sla_setq = sla_tbl.sla[sla_id].set_queue;

		if (sla_setq == FAKE_QUEUE) {
			ASSERT(sla_id == faked_sla_id);
			sla_setq = set_queue_save;
		}

		found = FALSE;
		for (t_imcs_pid = 0; t_imcs_pid < IMCS_PROC_LIMIT; t_imcs_pid++) {
			for (i = 0; i < MAX_NUM_SLA; i++) {
				if (imcs_addresses.proc[t_imcs_pid].sla[i] ==
				    sla_id){
					found = TRUE;
				}
			}
		}
		if (!found)
			/* CASE 1 */
			remove_sla_from_set(sla_id);
		else
			if (sla_setq == imcs_addresses.proc[imcs_pid].set_queue)
				rehash_sets();
	}

}  /* end imcs_addr_delete */



/**/
/*
 * NAME: imcs_sla_delete
 *                                                                    
 * FUNCTION: removes all triplet of the form <*, sla_id, *> so that
 *       1) the sla should not longer be used to send, and
 *       2) it should not be able to identify messages received on the sla
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void 
imcs_sla_delete(sla_id)
int sla_id;
{
	int imcs_pid;
	uchar sla_address;
	int i;
	int j;

	ASSERT(sla_id >= 0 && sla_id < MAX_NUM_SLA);
/*	ASSERT(sla_id != faked_sla_id); */


	/* remove sla from all processor/address tables */
	for (imcs_pid = 0; imcs_pid < IMCS_PROC_LIMIT; imcs_pid++) {
		for (i = 0; i < MAX_NUM_SLA; i++) {
			if (imcs_addresses.proc[imcs_pid].sla[i] == sla_id) {
				/* processor imcs_pid uses sla sla_id */
				/* remove the sla from the set of sla */
				/* remove machine id to free up slot */

				mach_ids[imcs_pid]= -1;

				imcs_addresses.proc[imcs_pid].sla[i] = 
								INVALID_SLAID;
				/* remove all addresses that this sla used, 
				also from the inverted table */
				for (j = 0; j < MAX_NUM_SLA; j++) {
					sla_address = imcs_addresses.proc[imcs_pid].sla_addr[sla_id][j];
					if (sla_address != INVALID_SLA_ADDR) {
						imcs_addresses.sla_addr_tb[sla_id][sla_address] = INVALID_PID;
						imcs_addresses.proc[imcs_pid].sla_addr[sla_id][j] = INVALID_SLA_ADDR;
					}
				}  /* end of for loop on j */
					/* and remove the cached sla */
				if (imcs_addresses.proc[imcs_pid].av_sla==
									sla_id)
					imcs_addresses.proc[imcs_pid].av_sla = 
								INVALID_SLAID;
			}  /* end processor imcs_pid uses sla sla_id */
		}  /* end loop on imcs_pid */
	}

	/* remove the sla as a server from its set queue */
	remove_sla_from_set(sla_id);

}  /* end imcs_sla_delete */

/**/
/*
 * NAME: imcs_sla_count
 *                                                                    
 * FUNCTION: calculates the number of sla's that processor imcs_pid can reach
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: count of sla's
 */  

int
imcs_sla_count(imcs_pid)
int imcs_pid;
{
	int sla_count;
	int i;

	ASSERT(imcs_pid > 0 && imcs_pid < IMCS_PROC_LIMIT);

	sla_count = 0;

	for (i = 0; i < MAX_NUM_SLA; i++)
		if (imcs_addresses.proc[imcs_pid].sla[i] != INVALID_SLAID) 
								sla_count++;
	return sla_count;

}  /* end imcs_sla_count */


/**/
/*
 * NAME: imcs_flush_addr 
 *                                                                    
 * FUNCTION: initializes the address table

 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
imcs_flush_addr()
{
	int i;

	for (i = 0; i < IMCS_PROC_LIMIT; i++) {
		int j;
		imcs_addresses.proc[i].set_queue = INVALID_SETQ;
		imcs_addresses.proc[i].av_sla = INVALID_SLAID;
		imcs_addresses.proc[i].av_addr = INVALID_SLA_ADDR;
		for (j = 0; j < MAX_NUM_SLA; j++) {
			int k;
			imcs_addresses.proc[i].sla[j] = INVALID_SLAID;
			for (k = 0; k < MAX_NUM_SLA; k++) {
				imcs_addresses.proc[i].sla_addr[j][k] = 
							INVALID_SLA_ADDR;
			}
		}
	}

	for (i = 0; i < MAX_NUM_SLA; i++) {
		int j;
		for (j = 0; j < SLA_ADDR_RANGE; j++)
			imcs_addresses.sla_addr_tb[i][j] = INVALID_PID;
	}

	for (i = 0; i < MAX_NUM_SLA; i++) {
		int j;
		imcs_addresses.sla_addr_tb[i][0] = FORBIDDEN_SLA_ADDR;
		for (j = 0; forbidden_addr[j] != 0x00; j++)
			imcs_addresses.sla_addr_tb[i][forbidden_addr[j]] = 
							FORBIDDEN_SLA_ADDR;
	}

}  /* end imcs_flush_addr */


/**/
/*
 * NAME: imcs_find_slaa
 *                                                                    
 * FUNCTION: generate a good (random) sla address  
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: good random number
 */  

uchar
imcs_find_slaa(sla_id)
short sla_id;
{
	int i;
	uchar address;
	uchar address_found;  /* flag */
	struct timestruc_t time;


	/* pick an ala address */
	address_found = FALSE;
	while (! address_found) {

		curtime(&time);	/* use the clock as a random number */

		address = (uchar) ((time.tv_nsec & 0x0000ff00) >> 8);

		if (imcs_addresses.sla_addr_tb[0][address]==FORBIDDEN_SLA_ADDR 
			     || imcs_addresses.sla_addr_tb[0][address - 1] ==
		   			 FORBIDDEN_SLA_ADDR || address >= 0xe0)
			address_found = FALSE;
		else
			address_found = TRUE;

	}  /* end while loop */

	return address;

}  /* end imcs_find_slaa */


/**/
/*
 * NAME: imcs_addr_fake 
 *                                                                    
 * FUNCTION: makes an sla usable for sends. This is used to send the 
 *		first message after the ala (the hello message)
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: IMCS_FAKE_PID 
 */  

int
imcs_addr_fake(sla_id, sla_address)
int sla_id;
slalinka_t sla_address;
{

	ASSERT(sla_id >= 0 && sla_id < MAX_NUM_SLA);
	ASSERT(sla_address != INVALID_SLA_ADDR);
SOL_TRACE("iadf",sla_id,faked_sla_id,0);
	ASSERT(faked_sla_id == INVALID_SLAID);
	faked_sla_id = sla_id;

	ASSERT(imcs_addresses.proc[IMCS_FAKE_PID].sla[0] == INVALID_SLAID);
	ASSERT(imcs_addresses.proc[IMCS_FAKE_PID].sla_addr[sla_id][0] == INVALID_SLA_ADDR);

	imcs_addresses.proc[IMCS_FAKE_PID].sla[0] = sla_id;
	imcs_addresses.proc[IMCS_FAKE_PID].av_sla = INVALID_SLAID;
	imcs_addresses.proc[IMCS_FAKE_PID].sla_addr[sla_id][0] = sla_address;
	imcs_addresses.proc[IMCS_FAKE_PID].set_queue = FAKE_QUEUE;
	set_queue_save = sla_tbl.sla[sla_id].set_queue;
	sla_tbl.sla[sla_id].set_queue = FAKE_QUEUE;

	return IMCS_FAKE_PID;

}  /* imcs_addr_fake */


/**/
/*
 * NAME: imcs_addr_clean
 *                                                                    
 * FUNCTION: fixes up thigs after a fake processor id has been assigned to a sla
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:  NONE
 */  

void
imcs_addr_clean(sla_id,outcome)
int sla_id;
int outcome;
{

	ASSERT(faked_sla_id == sla_id);
SOL_TRACE("iadc",sla_id,0,0);
	imcs_addresses.proc[IMCS_FAKE_PID].sla_addr[sla_id][0]=INVALID_SLA_ADDR;
	imcs_addresses.proc[IMCS_FAKE_PID].set_queue = INVALID_SETQ;

	sla_tbl.sla[sla_id].set_queue = set_queue_save;

	set_queue_save = INVALID_SETQ;
	faked_sla_id = INVALID_SLAID;
	/*
	 * If a message was queued for this SLA, we need to dequeue it and
	 * send it at this point (defect 46683).  Check to make sure the
	 * SLA is serving a queue and it has not failed.  Without the
	 * check for outcome, calling cdd_next will cause a crash.
	 */
	if ((sla_tbl.sla[sla_id].set_queue != INVALID_SETQ) &&
	    (sla_tbl.sla[sla_id].imcs_st & SLA_SENDING_CODE) &&
	    (outcome == IMCS_ACK)) {
		cdd_next(sla_id);
	}

}  /* end imcs_addr_clean */

/**/
/*
 * NAME: ipool_put
 *                                                                    
 * FUNCTION: adds to the imcs pool
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 	0 success
 *	 	1 stack full
 */  


int
ipool_put(pageno)
int pageno;
{
	int processor_priority;
	int rc = 0;

	processor_priority = i_disable(INTMAX);

	if (ipool.number == NUM_PAGENO)  /* stack full */
		rc = 1;
	else {
		ipool.top++;
		ipool.pageno[ipool.top] = pageno;
		ipool.number++;
	}

	i_enable(processor_priority);

	return rc;
}  /* end ipool_put */


/**/
/*
 * NAME: ipool_get
 *                                                                    
 * FUNCTION: removes a page from the imcs pool
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) this routine must be called when interrupts are masked
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 	number of page
 *		-1 if no pages available
 */  

int
ipool_get()
{
	int pageno;

	if (ipool.number == 0)
		pageno = -1;
	else {
		pageno = ipool.pageno[ipool.top];
		ipool.top--;
		ipool.number--;
	}

	return pageno;
}  /* end ipool_get */


/**/
/*
 * NAME: ipool_count
 *                                                                    
 * FUNCTION: returns the number of pages currently in the pool
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: ipool.number
 */  

int
ipool_count()
{
	return ipool.number;
}  /* end ipool_count */


/**/
/*
 * NAME: ipool_init
 *                                                                    
 * FUNCTION: initializes the imcs pool area
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
ipool_init(num_fr)
int num_fr;
{
	ipool.number = 0;
	ipool.top = -1;


	#ifdef RTSILONG
	{
		/* prime imcs up, by asking the pager for pages */
		int i, pageno;
		for (i = 0; i < num_fr; i++) {
			pageno = get_imcsbuf();
			ipool_put(pageno);
		}
	}
	#endif

}  /* end ipool_init */


/**/
/*
 * NAME: imcs_sendhello
 *                                                                    
 * FUNCTION: sends a hello request or response message
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
imcs_sendhello(sla_id)
int sla_id;
{

	struct imcs_header * ihdr;
	struct imcs_hello_message1 * msg;
	int rc, fake_pid;


	ASSERT(sla_work[sla_id][position] == HELLO_REQUEST || 
				sla_work[sla_id][position] == HELLO_REPLY ||
				sla_work[sla_id][position] == HELLO_REJECT);
	/* address the imcs header and the user header */
	msg = (struct imcs_hello_message1 *) send_header;
	ihdr = (struct imcs_header *) (send_header + IMCS_RTSI_UHDR_SIZE);
SOL_TRACE("shel",sla_id,msg,ihdr);

	/* ask imcs to fake a triplet <fake-pid, sla-id, link-address> so that
	   when we direct a message to fake-pid sla sla-id and address 
	sla-address will be used */
	fake_pid = imcs_addr_fake(sla_id, slaam[sla_id].link_address[position]);

	/*fill in the imcs header(the other fields were set at initialization)*/
	ihdr -> IMCS_DEST_PROC = fake_pid;

	/* fill in the user header, which will be sent */
	msg -> imcs_version = IMCSHELLO_VERSION;
	msg -> sla_id = sla_id;
	msg -> imcs_pid = imcs_host;
	msg -> link_address = my_addr(sla_id);
	msg -> configuration_reg = sla_tbl.sla[sla_id].config_reg;
	msg -> type = sla_work[sla_id][position];
	msg -> machine_type = RS_6000;

	vm_cflush(send_header, IMCS_RTSI_SHDR_SIZE);	/* flush the cache */

        returned_from_imcs = FALSE;

	rc = imcs_sendmsg(ihdr);	/* send the message */
	ASSERT(rc == 0);

	if (! returned_from_imcs) {
		returned_from_imcs = TRUE;
		/* remove the triplet added by imcs_addr_fake */
		imcs_addresses.proc[IMCS_FAKE_PID].sla[0]= INVALID_SLAID;
	}


}  /* end imcs_sendhello */

/**/
/*
 * NAME: imcs_hello_work
 *                                                                    
 * FUNCTION: if there are hello messages to send, either send them
 *           or start a timer request so that they will be sent later
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


void
imcs_hello_work(sla_id)
int sla_id;
{

	int i;
	boolean_t none_found;
	TVARS tvar;

	none_found = TRUE;
	for (i = 0; i < slaam[sla_id].count && none_found; i++)
		if (sla_work[sla_id][i]) {
			none_found = FALSE;
			if (send_header_used) {  /* try again later */
				tvar.id=sla_id;
				tvar.func=TIMER_HELLO;
				tvar.setup=FALSE;
				START_TIMER(timer_sp[sla_id], tservice, 
						tvar, HLMSG_RS, HLMSG_RNS);
			}
			else {
				send_header_used = TRUE;
				position = i;
				imcs_sendhello(sla_id);
			}
		}

	/*
	 *  At this point, the CIO_START is complete, so call start_done.
	 */
	sol_start_done(CIO_OK);

	if (none_found) {
		hello_in_progress[sla_id] = FALSE;
		tvar.id=sla_id;
		tvar.func = RESTART_HELLO;
		tvar.setup=FALSE;
		START_TIMER(timer_sp[sla_id], tservice, tvar, 100, 0);  /* 10 seconds */
	}

}  /* end imcs_hello_work */


/**/
/*
 * NAME: imcs_start_hello
 *                                                                    
 * FUNCTION: initialize the work structure to broadcast hello request to all
 *          addresses indicated by the driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:  NONE
 */  

void
imcs_start_hello(sla_id)
int sla_id;
{
	int i;

	for (i = 0; i < slaam[sla_id].count; i++)
		if (sla_work[sla_id][i] == NO_WORK)
			sla_work[sla_id][i] = HELLO_REQUEST;

	hello_in_progress[sla_id] = TRUE;

	if (imcs_host != -1) {
		STOP_TIMER(timer_sp[sla_id]);
		imcs_hello_work(sla_id);  /* and start working on it */
	}

}  /* end imcs_start_hello */


/**/
/*
 * NAME: auditor_notify 
 *                                                                    
 * FUNCTION:  notification routine called after a send completes
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
auditor_notify(header)
struct imcs_header *header;
{

	int sla_id;
	struct imcs_hello_message1 * msg;
	TVARS tvar;
	/* address the user header */
	msg = (struct imcs_hello_message1 *) 
				((caddr_t) header - IMCS_RTSI_UHDR_SIZE);
SOL_TRACE("audn",header->outcome,msg->link_address,msg->imcs_pid);
ASSERT(msg->imcs_pid != 0);
	switch (msg -> type) {
	case HELLO_REQUEST :
	case HELLO_REPLY : 
	case HELLO_REJECT :
		sla_id = msg -> sla_id;
		switch (header -> outcome) {
		case IMCS_ACK :
			/* clean up */
			ASSERT(position < slaam[sla_id].count);
			switch (msg -> type) {
			case HELLO_REQUEST :
				if (sla_work[sla_id][position] == HELLO_REQUEST)
					sla_work[sla_id][position] = NO_WORK;
				break;
			case HELLO_REPLY :
				sla_work[sla_id][position] = NO_WORK;
				break;
			default : break;
			}
			break;
		default: 
			sla_work[sla_id][position] = NO_WORK; /* do not retry */
			break;
		}  /* end switch on outcome */


	        if (! returned_from_imcs) {
        	        returned_from_imcs = TRUE;
                	/* remove the triplet added by imcs_addr_fake */
                	imcs_addresses.proc[IMCS_FAKE_PID].sla[0]
			    = INVALID_SLAID;
        	}
		imcs_addr_clean(sla_id,header->outcome);
		/* try the next message if any, after a delay */
		tvar.id=sla_id;
		tvar.func=TIMER_HELLO;
		tvar.setup=FALSE;
		START_TIMER(timer_sp[sla_id], tservice, tvar,
		    HLMSG_NEXT_RS, HLMSG_NEXT_RNS);
		break;
	default: break;
	}  /* end switch on type */

	send_header_used = FALSE;

}  /* end auditor_notify */


/**/
/*
 * NAME: set_reply 
 *                                                                    
 * FUNCTION:  set up to set a reply
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
set_reply(sla_id, sla_address,type)
int sla_id;
slalinka_t sla_address;
int type;
{
	int i;

	for (i = 0; i < slaam[sla_id].count; i++)
		if (slaam[sla_id].link_address[i] == sla_address)
			break;
	if( i >= slaam[sla_id].count)
		return;

	sla_work[sla_id][i] = type;
	SYS_SOL_TRACE("hrpl",sla_id, type, 0);

	if (! hello_in_progress[sla_id] ) {
		/* kick if off */
		STOP_TIMER(timer_sp[sla_id]);
		hello_in_progress[sla_id] = TRUE;
		imcs_hello_work(sla_id);
	}

}  /* end set_reply */


/**/
/*
 * NAME: ok_pid 
 *                                                                    
 * FUNCTION: validate pid
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:	TRUE if the pid and machine id satisfy certain rules
 *              FALSE otherwise
 */  

static boolean_t
ok_pid(imcs_pid, mach_id, machine_type)
int imcs_pid;
int mach_id;
char machine_type;
{
	int our_host;

	if (imcs_pid == -1) return FALSE;

	our_host = imcs_host;
	if (our_host != -1 &&
	    mach_ids[our_host] == -1)
		mach_ids[our_host] = sol_ddi.ops_info.machine_id;

	if (imcs_pid == our_host)
		if (mach_ids[our_host] == mach_id)
			return TRUE;
		else return FALSE;

	if (mach_ids[imcs_pid] == -1) { /* processor not already known */
		mach_ids[imcs_pid] = mach_id;
		return TRUE;
	} else {
		if (mach_ids[imcs_pid] == mach_id)
			return TRUE;
		else
			if (cck_proc[imcs_pid] & PID_PRESENT) 
				return FALSE;
			else {
				mach_ids[imcs_pid] = mach_id;
				return TRUE;
			}
	}
} /* end ok_pid */



/**/
/*
 * NAME: rcv_hello
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
rcv_hello(imcs_pid, sla_id, sla_address, hdw_config)
int imcs_pid, sla_id;
slalinka_t sla_address;
uchar hdw_config;
{
	int existing_pid;

	SYS_SOL_TRACE("rhel",sla_id,imcs_pid,0);
	ASSERT (imcs_pid > 0 && imcs_pid < IMCS_PROC_LIMIT);
	cck_proc[imcs_pid] |= PID_PRESENT;
	cck_proc[imcs_pid] |= PID_EVER_PRESENT;

	existing_pid = imcs_addresses.sla_addr_tb[sla_id][sla_address];

	if (existing_pid == INVALID_PID)
		/* there is no tuple <imcs_pid, sla_id, sla_address> */
		imcs_addr_add(imcs_pid, sla_id, sla_address);
			/* tell imcs about the association
			   betweenen the sla address and processor id */
	else if (existing_pid == FORBIDDEN_SLA_ADDR ) {
		return;
	}
	else if (existing_pid == imcs_pid);
	/* there is such a tuple */
	else {
		/* there is a tuple <existing_pid, sla_id, sla_address>.  
		This tuple should be erased */
		imcs_addr_delete(sla_id, sla_address);  /* delete old tuple */
		if ((cck_proc[existing_pid] & PID_PRESENT) &&
		    (imcs_sla_count(existing_pid) == 0) )
			recover_imcs_one(existing_pid);	/*recover if necessary*/
		imcs_addr_add(imcs_pid, sla_id, sla_address);/* insert tuple */
	}

	cdd_retry_msgs(imcs_pid);

	switch (sla_tbl.sla[sla_id].connection) {
	case POINT_TO_POINT :
		switch (hdw_config) {
		case SLAC_CMB2S :
		case SLAC_CMB3E :
		case SLAC_4S :
			/* since the other side will not do
		   dma recovery we do not have to wait for
		   it to do it */
			sla_sdma_wait(sla_id) = FALSE;
			break;
		case SLAC_INIT :
		case SLAC_CMB2E :
			break;
		default :
			PANIC("imcs_audit (rcv_hello): unknown config value");
		}
		break;

	case DX_SWITCH_PRESENT :
		if (sla_address == 0xfd)
			switch (hdw_config) {
			case SLAC_CMB2S : /* no dma recovery */
				/* since the other side will not do
		     dma recovery we do not have to wait for
		     it to do it */
				sla_sdma_wait(sla_id) = FALSE;
				break;
			default :
				break;
			}
		break;

	case SWITCH_PRESENT :
		break;

	default :
		PANIC("unknown connection in rcv_hello");
	}  /* end switch on connection type */

}  /* end rcv_hello */


/**/
/*
 * NAME: rcv_hello_msg1
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
rcv_hello_msg1(header, ha)
struct imcs_header * header;
caddr_t ha;
{
	struct imcs_hello_message1 * msg;
	int pid, sla_id;
	slalinka_t address;
	uchar hdw_config;
	msg = (struct imcs_hello_message1 *) ha;

SOL_TRACE("hel1",header,msg,0);
	address = msg -> link_address;
	sla_id = header -> sla_id;
	pid = msg -> imcs_pid;
	hdw_config = msg -> configuration_reg;

	switch (msg -> type) {
	case HELLO_REQUEST :
		if (ok_pid(pid, msg -> machine_id, msg -> machine_type)) {
			rcv_hello(pid, sla_id, address, hdw_config);
			set_reply(sla_id, address,HELLO_REPLY);
		}
		else {
	 		set_reply(sla_id, address,HELLO_REJECT);
		}
		break;

	case HELLO_REPLY :
		if (ok_pid(pid, msg -> machine_id, msg -> machine_type)) {
			rcv_hello(pid, sla_id, address, hdw_config);
		}
		else {
	 		set_reply(sla_id, address,HELLO_REJECT);
		}
		break;

	default:
		break;
	}  /* end switch in message type */

}  /* end rcv_hello_msg1 */


/**/
/*
 * NAME: auditor_slih
 *                                                                    
 * FUNCTION: slih extension for the imcs auditor.
 *	       called by imcs when a message arrives for the auditor queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
auditor_slih(ha)
caddr_t ha;
{

	struct imcs_hello_message1 * msg;
	struct imcs_header * header;

SOL_TRACE("auds",ha,0,0);
	/* address the user header and the imcs header */
	msg = (struct imcs_hello_message1 *) ha;
	header = (struct imcs_header *) (ha + IMCS_RTSI_UHDR_SIZE);

	switch (header -> outcome) {
	case IMCS_ACK :
		switch(msg -> type) {
		case HELLO_REPLY:
		case HELLO_REQUEST:
			rcv_hello_msg1(header, ha);
			break;
		case HELLO_REJECT:
			pid_conflict(header->sla_id, msg->imcs_pid);
			break;
		default :
			break;
		}  /* end switch on imcs version */
		break;

	default :
		break;
	}  /* end switch on outcome */

	/* return the header to imcs */
	imcs_rethdr(IMCS_AUDITOR_QUEUE, ha);

}  /* end iti_slih */


/**/
/*
 * NAME: init_auditor
 *                                                                    
 * FUNCTION: installs the auditor queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
init_auditor()
{

	int sla_id;
	struct trb *trb;
	int imcs_pid;
	struct imcs_header *header;
	struct imcs_hello_message1 * msg;
	ushort queue_id;
	int tag = 0;
	int rc;

	for (imcs_pid = 0; imcs_pid < IMCS_PROC_LIMIT; imcs_pid++) {
		/* initalize the imcs portion of the cluster common knowledge */
		cck_proc[imcs_pid] = 0;
		mach_ids[imcs_pid] = -1;
	}

	for (sla_id = 0; sla_id < MAX_NUM_SLA; sla_id++) {
		trb = talloc();
		trb -> ipri = IMCS_INT_PRIORITY;
		timer_sp[sla_id] = (void *) trb;
		ASSERT(timer_sp[sla_id] != (void *) NULL);
		bad_port[sla_id]=0;
	}

	for (sla_id = 0; sla_id < MAX_NUM_SLA; sla_id++)
		hello_in_progress[sla_id] = FALSE;

	/*
	For the time being I like to run the two part of recovery as one
	so I will set rec_ts_used to true and the trb does not have to init'ed
	trb = talloc();
	trb -> ipri = IMCS_INT_PRIORITY;
	rec_ts = (void *) trb;
	ASSERT(rec_ts != (void *) NULL);
	rec_ts_used = FALSE; */
	rec_ts_used = TRUE;


	send_header = xmalloc(IMCS_RTSI_UHDR_SIZE + IMCS_HDR_SIZE,
	    IMCS_LOG2_LINE_SIZE, pinned_heap);
	bzero((caddr_t) send_header,IMCS_RTSI_UHDR_SIZE + IMCS_HDR_SIZE);
	send_header_used = FALSE;
	ASSERT(NULL != send_header);

	/* set it up to send to the imcs-to-imcs queue */
	header = (struct imcs_header *) (send_header + IMCS_RTSI_UHDR_SIZE);
	header -> IMCS_QUEUE = IMCS_AUDITOR_QUEUE;
	header -> notify_address = (void (*)()) auditor_notify;
	header -> IMCS_PROTOCOL = IMCS_RTSI_SND_CODE;
	header -> IMCS_SERIALIZE = FALSE;

	msg = (struct imcs_hello_message1 *) send_header;
	msg->machine_id = sol_ddi.ops_info.machine_id;
	rc=tagwords(send_header,IMCS_RTSI_SHDR_SIZE,(caddr_t)header,&tag);
	ASSERT(!rc);
	for (; tag < NUM_HDR_TCWS; tag++) header -> IMCS_TAG(tag) = LAST_TCW;

	header -> IMCS_MSGLEN = IMCS_RTSI_SHDR_SIZE;

	queue_id = IMCS_AUDITOR_QUEUE;
	imcs_declare(&queue_id, (void (*) ()) auditor_slih,
	    DCL_STRICT_Q, 1); /* 1 is the number of headers used to receive
						  on the imcs-to-imcs queue */
	imcs_ctl(IMCS_AUDITOR_QUEUE, RCV);	/* enable receives */

}  /* end init_auditor */



/**/
/*
 * NAME: recover_imcs_two
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * (RECOVERY OPERATION:) 
 *
 *
 * RETURNS: NONE
 */  

void
recover_imcs_two(processor_id)
int processor_id;
{
int num;

	ASSERT(processor_id >= 0 && processor_id < IMCS_PROC_LIMIT);

	/* now rececover other system components */
	/* there are none yet */

	/* tell imcs not to hold messages for the processor */
	cck_proc[processor_id] &= ~PID_HOLDING;
				/* return all header already queued */
        if(imcs_addresses.proc[processor_id].set_queue != INVALID_SETQ) {
		num =cdd_return_msgs(processor_id, IMCS_DOWN_CONN,
		    imcs_addresses.proc[processor_id].set_queue);  
		que.cnt[processor_id]-=num;
		que.total_queued-=num;
	}
	/*
	 * At this point we used to return the messages from the LIMBO queue,
	 * but that prevented recovery for some errors.  Just leave them
	 * on the LIMBO queue at this point.
	 */
#if 0
	if( (num=cdd_return_msgs(processor_id,IMCS_DOWN_CONN,LIMBO_QUEUE)) )  {
		que.cnt[LIMBO_SLOT]-=num;
		que.total_queued-=num;
	}
#endif

}  /* end recover_imcs_two */


/**/
/*
 * NAME: recover_imcs_one
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

static void
recover_imcs_one(processor_id)
int processor_id;
{
	TVARS tvar;
	ASSERT(processor_id >= 0 && processor_id < IMCS_PROC_LIMIT);

	/* tell imcs not to return msg destined for the downed location, but
	   to queue them */
	cck_proc[processor_id] |= PID_HOLDING;

	/* tell users that the processor went down */
	cck_proc[processor_id] &= ~PID_PRESENT;

	/*
	temporarly we are keeping the processor in the set so we will not call 
	this now. rather will call it after returning messages
	imcs_addresses.proc[processor_id].set_queue = INVALID_SETQ; 
	*/  /* this cleans up the address table */

	if (rec_ts_used) {
		recover_imcs_two(processor_id);
	}
	else {
		tvar.id = processor_id;
		tvar.func = TIMER_RECOVER;
		tvar.setup = FALSE;
		START_TIMER(rec_ts, tservice, tvar, RC_DEL_S, RC_DEL_NS);
		rec_ts_used = TRUE;
	}
	imcs_addresses.proc[processor_id].set_queue = INVALID_SETQ; 

}  /* end recover_imcs_one */


/**/
/*
 * NAME: imcs_sla_addr_off
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void imcs_sla_addr_off(int sla_id, slalinka_t sla_address)
{
	int imcs_pid;
	int i;
	TVARS tvar;
	imcs_pid = imcs_addresses.sla_addr_tb[sla_id][sla_address];

	/* find the address in the hello work table */
	for (i = 0; i < slaam[sla_id].count; i++)
		if (slaam[sla_id].link_address[i] == sla_address)
			break;

	if (imcs_pid >= INVALID_PID) {
		/* remove this address from list of addresses that should
	     receive a hello message */
		if (i < slaam[sla_id].count)
			sla_work[sla_id][i] = NO_WORK;
SOL_TRACE("iao1",sla_id,sla_address,imcs_pid);
	}
	else {
SOL_TRACE("iao2",sla_id,sla_address,imcs_pid);
		/* remove address from addressing table */
		imcs_addr_delete(sla_id, sla_address);
		if ((cck_proc[imcs_pid] & PID_PRESENT) &&
		    (imcs_sla_count(imcs_pid) == 0) )
			recover_imcs_one(imcs_pid);	/* start recovery */
		/* add this address to the list of addresses that should
	     receive an hello message */
		if (i < slaam[sla_id].count)
			sla_work[sla_id][i] = HELLO_REQUEST;
		if (! hello_in_progress[sla_id]) {
			/* kick if off */
			STOP_TIMER(timer_sp[sla_id]);
			hello_in_progress[sla_id] = TRUE;
			tvar.id=sla_id;
			tvar.func=TIMER_HELLO;
			tvar.setup=FALSE;
			START_TIMER(timer_sp[sla_id], tservice, tvar,
			    HLMSG_NEXT_RS, HLMSG_NEXT_RNS);
		}
	}

}  /* end imcs_sla_addr_off */


void
imcs_sla_off(sla_id, mode)
int sla_id;
int mode;
{
	int processor_id;

	if (mode == NO_ERROR) return;
SOL_TRACE("soff",sla_id,mode,0);
	/* it is possible that a failure occurs before the initial hand shake
	   messages have been exchanged.  It is also possible that there is a 
	   timer request to send the message.  Stop it now */

	STOP_TIMER(timer_sp[sla_id]);

	imcs_sla_delete(sla_id);

	for (processor_id = 0; processor_id < IMCS_PROC_LIMIT; processor_id++) {
		if ((cck_proc[processor_id] & PID_PRESENT) &&
		     (imcs_sla_count(processor_id) == 0) )
			recover_imcs_one(processor_id);	/* start recovery */
	}

}  /* end imcs_sla_off */

/**/
/*
 * NAME: imcs_ctl
 *                                                                    
 * FUNCTION: changes the status of an imcs receive queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *	   IMCS_DONE -- receive status set to input value
 *	   QUEUE_NOT_DCL -- queue was not declare yet
 */  

int
imcs_ctl(queue_id, status)	
uint queue_id;
int status;
{
#define RCV		TRUE
#define DO_NOT_RCV	FALSE

	struct irq_block *irq;

	irq = rqctl_find(queue_id);

	if (irq == NULL_IRQ) return QUEUE_NOT_DCL;

	switch (status) {
	  case RCV :
		irq -> status |= IRQ_RECEIVING;
		break;
	  case DO_NOT_RCV :
		irq -> status &= ~IRQ_RECEIVING;
		break;
	  default :
		return -1;
	}

	return IMCS_DONE;

}  /* end imcs_ctl */

/**/
/*
 * NAME: sla_ala_completed
 *                                                                    
 * FUNCTION: called by the interrupt handler after the sla has 
 *		completed the ala sequence
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void sla_ala_completed(int sla_id, uchar sla_address, void *slaamp)
{
        /* note this assert will go off if the channel interrupts before being
           started */
/*        ASSERT(sla_tbl.sla[sla_id].set_queue == INVALID_SETQ);  10/25/90 */

SOL_TRACE("alco",sla_id,0,0);
        sla_tbl.sla[sla_id].imcs_st = SLA_WORKING_CODE;
        my_addr(sla_id) = sla_address;

        slaam[sla_id] = *(struct sla_addrmap *) slaamp;
        imcs_start_hello(sla_id);

        /* imcs, possibly built a queue of messages to do rtsi receive.
           Grab one */
        cdd_rcv_start(sla_id);

}  /* end sla_ala_completed */

/**/
/*
 * NAME: pid_conflict
 *                                                                    
 * FUNCTION: called when a processor id conflict is detected to do notification
 *		and general cleanup
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */ 
 
static void
pid_conflict(sid,pid)
int sid;
int pid;
{
        int i;
        struct sol_open_struct  *open_ptr;
        cio_stat_blk_t  stat_blk;
SOL_TRACE("pidc",sid,pid,0);
        /*
         *  Send notifications to all users about this new prid conflict.
         */
        for (i = 0 ; i < SOL_TOTAL_NETIDS ; i += 2) {
                open_ptr = sol_ddi.netid_table[i>>1];
                if (open_ptr != NULL) {
                        stat_blk.code = CIO_ASYNC_STATUS;
                        stat_blk.option[0] = SOL_PRID_CONFLICT;
                        stat_blk.option[1] = pid;
			stat_blk.option[2] = imcs_host;
                        sol_report_status(open_ptr, &stat_blk);
                }
        }
	/* disable the sla with a pid conflict */
	sla_kill(sid,SLA_STOPPED,0);
	clear_sla_tbl(sid);
	faked_sla_id = INVALID_SLAID;
	bad_port[sid]=TRUE;
}

/**/
/*
 * NAME: clear_sla_tbl
 *                                                                    
 * FUNCTION: sets sla_tbl members back to their default values. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */ 
 
clear_sla_tbl(sid)
{
                active_hdr(sid) = NULL_HDR;
                passive_hdr(sid) = NULL_HDR;
                primary_hdr(sid) = NULL_HDR;
                rtsi_save(sid) = NULL_HDR;
                sla_tbl.sla[sid].s1_save = 0;
                sla_tbl.sla[sid].s2_save = 0;
                sla_tbl.sla[sid].ccr_all = 0;
                sla_tbl.sla[sid].ccr_con = 0;
                sla_tbl.sla[sid].diag_process = EVENT_NULL;
                sla_address(sid) = 0;
                sla_tbl.sla[sid].status.word = 0;  /* set all bits off */
                sla_tbl.sla[sid].event_count = 0;
                sla_tbl.sla[sid].imcs_st = SLA_BROKEN_CODE;
                sla_tbl.sla[sid].sla_st = SLA_NOTSYNC_CODE;
                sla_tbl.sla[sid].set_queue = INVALID_SETQ;
                sla_tbl.sla[sid].connection = 0;
                sla_tbl.sla[sid].ack_type = 0;
}

/**/
/*
 * NAME: clear_start_parms
 *                                                                    
 * FUNCTION: set static start variables on all calls to imcs_start 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */ 
 
clear_start_parms()
{
	send_header_used = FALSE;
	setq_map = 0;
	set_queue_save = INVALID_SETQ;
	faked_sla_id = INVALID_SLAID;
	position = -1;
}

/*^L*/
/*
 * NAME: cdd_retry_msgs
 *
 * FUNCTION: Try and resend messages saved on the limbo queue
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: NONE
 */


cdd_retry_msgs(processor_id)
int processor_id;
{
        int qa;
        struct imcs_header *this, *next;
        struct imcs_header *next_in_cdd_chain, *prev_in_cdd_chain;

        ASSERT(processor_id >= 0 && processor_id < IMCS_PROC_LIMIT);

        qa = SLA_OUTQ + LIMBO_QUEUE;

	SYS_SOL_TRACE("rety",processor_id,cddq.anchor[qa],0);
        prev_in_cdd_chain = NULL_HDR;

        for (this=cddq.anchor[qa]; this!=NULL_HDR; this = next_in_cdd_chain) {
		next_in_cdd_chain = this -> cdd_chain_word;

          	if (this -> IMCS_DEST_PROC == processor_id) {
			que.total_queued--;
			que.cnt[LIMBO_SLOT]--;
            	/* dequeue this from the cdd chain */
            		if (prev_in_cdd_chain == NULL_HDR) {
              		/* this is at the head of the list */
              			cddq.anchor[qa] = next_in_cdd_chain;
            		}
            		else {
              			prev_in_cdd_chain -> cdd_chain_word = 
				    next_in_cdd_chain;
            		}

            		cdd_enqueue(this);
		}
		else {
            		prev_in_cdd_chain = this;
          	}
        }  /* end for loop */


}  /* end cdd_retry_msgs */

