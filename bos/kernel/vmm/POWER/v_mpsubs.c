static char sccsid[] = "@(#)90	1.10  src/bos/kernel/vmm/POWER/v_mpsubs.c, sysvmm, bos412, 9445C412a 10/25/94 11:02:34";
/* 
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: teststackfix, v_scoreboard, v_descoreboard,
 *		v_descoreboard_all, v_descoreboard_most,
 *		v_chknolock
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifdef _POWER_MP

#include "vmsys.h"
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/mstsave.h>
#include <sys/ppda.h>
#include <sys/lock_def.h>
#include "mplock.h"

/*
 * NAME: teststackfix
 *
 * FUNCTION:	Determine if a CPU is in a stackfix or VMM critical section.
 *
 * PARAMETERS:
 *
 *	cpu	- cpu id
 *
 * RETURN VALUE DESCRIPTION:
 *
 *	0	- CPU is not running in a critical section
 *   non-zero	- CPU is in a critical section
 */

teststackfix(cpu)
int cpu;
{
	/*
	 * To avoid a potential deadlock between the synchronous MPC
	 * mechanism and page replacement, we must clear this processor's
	 * sync word.
	 *
	 * The deadlock would occur because the processor running this code,
	 * at INTPAGER, won't service the INTOFFL3 MPC offlevel until it
	 * tries to return to INTBASE.  That would cause the processor here
	 * and any processor(s) in cs_mpc_issue to wait forever.
	 */
	(void)fetch_and_and(&PPDA->cs_sync, 0);

	/*
	 * We are accessing the PPDA of another processor and must do so
	 * carefully to ensure we get the latest value.  fetch_and_nop
	 * returns the word which contains both stackfix and lru flag.
	 */
	return(fetch_and_nop(&(GET_PPDA(cpu)->cpuid)) & 0x0000ffff);
}

/************************* MPSAFE LOCK ***************************/

/*
 * Definition of the global VMM mpsafe lock.
 * Needs to be initialized so it resides in pinned storage
 * rather than common.
 */
#ifdef _VMM_MP_SAFE
struct vmmlock vmm_mpsafe_lock = { -1, 0 };
#endif


/************************* SCOREBOARDING *************************/

#ifdef _VMM_MP_EFF

#ifdef _VMMDEBUG

/*
 * Record the file name and the line number of the source code which
 * last modified (acquired or released) the lock at the corresponding
 * index in the scoreboard (in the PPDA).
 * In case of a release, the line number is multiplied by 2**4,
 * and the lock address is lost since the scoreboard entry is cleaned.
 */
char scoreboard_file[MAXCPU][VMM_MAXLOCK][256] = {0};
int  scoreboard_line[MAXCPU][VMM_MAXLOCK] = {0};

/*
 * Record the highest entry index used in the scoreboard.
 */
int  maxhint = 0;

/*
 * Lock hierarchy building/checking:
 *
 * lock_depth is the current number of vmm locks held.
 * max_depth_scoreboard the maximum number of vmm locks held.
 * scoreboard_lock is a duplicate of the scoreboard but
 *		  w/ the lock number instead of the lock addr.
 * lock_sequence_number keeps track of the number lock acquired in this cs.
 * scoreboard_order keeps track of the order the locks are acquired.
 * scoreboard_vertex is the lock preceding order matrix.
 * lock_hierarchy_number records the lock hierarchy indexed by the lock number.
 * vmm_locks records the name of the locks indexed by the lock number.
 *
 * Note that all per-processor structures are padded to ensure that each
 * CPU references a different cache line (assumed to be 64 bytes).
 */
int lock_depth[MAXCPU][16]={0};
int max_depth_scoreboard = 0;
int scoreboard_lock[MAXCPU][VMM_MAXLOCK] = {0};
int lock_sequence_number[MAXCPU][16]={0};
int scoreboard_order[MAXCPU][VMM_MAXLOCK] = {0};
int scoreboard_vertex[VMMNUMLOCKS][VMMNUMLOCKS] = {0};
int lock_hierarchy_number[VMMNUMLOCKS]={0,1,2,2,2,3,4,5,6,6,6,7};
char *vmm_locks[VMMNUMLOCKS]={ "NO_LOCK", "SCB", "AME", "VMAP", "FS", "ALLOC", "LW", "PDT", "PG", "RPT", "APT", "VMKER"};

/*
 * We use the following per processor structure to insure that we do not
 * fault while holding a non scoreboarded vmm lock.
 * v_backt_save records the initial csa->backt value.
 * v_backt records the depth of nesting of non scoreboarded locks.
 */
int  v_backt[MAXCPU][16] = {0};
int  v_backt_save[MAXCPU][16] = {0};

#endif /* _VMMDEBUG */

/*
 * Scoreboarding routines:
 *
 * The purpose of the scoreboarding is to be able to release the vmm locks
 * on backtrack. Locks are scoreboarded iff they are taken in a section where
 * page faults are possible. Since the vmm locks are taken only in critical
 * or disabled sections, locks can be seen as per cpu ressources and
 * are scoreboard in the ppda.
 * For efficiency, we manage a hint that points to the last modified location
 * in the scoreboard array. We have coded the search algorithm based on the
 * hypothesis that most of the time the locks are nested, but we have also
 * taken into account the climb up of trees in getvmpage and getparent.
 */

#ifdef _VMMDEBUG
void vmm_b() {} /* to put breakpoint on it */
int vmm_prlock= 0;
#define PRINTF	if(vmm_prlock) printf
#endif /* _VMMDEBUG */

/*
 * NAME: v_scoreboard
 *
 * FUNCTION:	register a lock in the per cpu scoreboard array
 *		so that we can unlock it on backtrack.
 *
 * PARAMETERS: 	x	- lock address
 *
 * RETURN VALUE DESCRIPTION: None
 *
 */
void
#ifndef _VMMDEBUG
v_scoreboard(x)
#else  /* _VMMDEBUG */
v_scoreboard(x, LINE, FILE,locknum)
int LINE;
char *FILE;
int locknum;
#endif /* _VMMDEBUG */
simple_lock_t x;
{
	struct ppda *ppda = my_ppda();
	int i, hint, found;
#ifdef _VMMDEBUG
	int neworder, line;
	int my_cpuid = CPUID; 
#endif

	hint = ppda->ppda_hint;
	ASSERT(hint >= 0 && hint < VMM_MAXLOCK);
	

	/* We want to try the hint first in case the last action was an unlock.
	 */
	if (ppda->scoreboard[hint] == NULL)
	{
		found = hint;
		goto scoreboard;
	}

	/* Then we try hint+1 in case the last action was a lock.
	 */
	if (hint+1 < VMM_MAXLOCK && ppda->scoreboard[hint+1] == NULL)
	{
		found = hint+1;
		goto scoreboard;
	}

	/* Full search, trying the lower index first
	 */
	for (i = 0; i < hint; i++)
		if (ppda->scoreboard[i] == NULL)
		{
			found = i;
			goto scoreboard;
		}
	
	for (i = hint+2; i < VMM_MAXLOCK; i++)
		if (ppda->scoreboard[i] == NULL)
		{
			found = i;
			goto scoreboard;
		}

	panic("Out of entries in the vmm scoreboard");

scoreboard:
	ppda->scoreboard[found] = x;
	ppda->ppda_hint = found;

#ifdef _VMMDEBUG

	/* record file and line number of the source code 
	 * (i.e. of the caller) that acquired the lock 
	 */
	scoreboard_line[my_cpuid][found]= LINE;
	strcpy(&scoreboard_file[my_cpuid][found], FILE);

	/* Track the highest entry index used.
	 */
	if(found > maxhint) 
	{
		maxhint = found;
		PRINTF("\n -------------------- \n");
		PRINTF(" maxhint increased to %d \n", maxhint);
		PRINTF(" -------------------- \n");
	}

	/* Track the maximum number of vmm locks held 
	 */
	i = ++lock_depth[my_cpuid][0];
	if (i > max_depth_scoreboard)
	{
		max_depth_scoreboard = i;
		PRINTF("\n -------------------- \n");
		PRINTF(" maxhint %d \n", maxhint);
		PRINTF(" max_scoreboard increased to %d \n", i);
		PRINTF(" -------------------- \n");
	}

	/* update data struct for building/checking of the lock hierarchy
	 */
	scoreboard_lock[my_cpuid][found]= locknum;
	scoreboard_order[my_cpuid][found]= lock_sequence_number[my_cpuid];
	lock_sequence_number[my_cpuid][0]++;

	neworder = 0;
	for (i = 0; i < VMM_MAXLOCK; i++)
	{
		if(i == found)
			continue;

		if (ppda->scoreboard[i])
		{
			int lock_i = scoreboard_lock[my_cpuid][i];

			/*
			 * Check acquisition in lock hierarchy.
			 * Allow exception for multiple scb locks.
			 */
			if (lock_hierarchy_number[lock_i] >=
				lock_hierarchy_number[locknum]
			    && !(locknum == lock_i && locknum == 1))
			{
				/*
				 * Also allow exception for the scb lock of
				 * the pta seg taken under the alloc lock.
				 */
				if(   (locknum == 1) /* scb lock */
				   && (lock_i  == 5) /* alloc lock */
				   && (x == &scb_lock(STOI(SRTOSID(vmker.ptasrval))))
				  )
					continue;
				PRINTF(" !!!!!!! acquiring lock %s_LOCK w/ hierachy %d \n",

					vmm_locks[locknum], lock_hierarchy_number[locknum]);
				PRINTF(" while holding lock %s_LOCK w/ hierarchy %d \n\n\n",
					vmm_locks[lock_i],lock_hierarchy_number[lock_i]);
				panic("vmm locks taken out of order");
			}
			
			if(scoreboard_vertex[lock_i][locknum] != TRUE)
			{

				scoreboard_vertex[lock_i][locknum]= TRUE;

				if(!neworder)
				{
					neworder = 1;
					PRINTF("\n ************************************* \n");
					PRINTF(" ---------- new order found ---------- \n");
				}
				PRINTF(" lock %s_LOCK >> lock %s_LOCK \n",
					vmm_locks[lock_i], vmm_locks[locknum]);


			}
		} 
	}

	if(neworder)
	{
		PRINTF("\n ---------- current locks (time order indexed)  ---------- \n");
		for (i = 0; i < VMM_MAXLOCK ; i++)
		{
			if (ppda->scoreboard[i])
			{
				int lock_i = scoreboard_lock[my_cpuid][i];
				
				PRINTF(" # %d : %s_LOCK ",
					scoreboard_order[my_cpuid][i], vmm_locks[lock_i]);
				line = scoreboard_line[my_cpuid][i];
				PRINTF("taken line: %d ", line);
				PRINTF("in file: %s\n", scoreboard_file[my_cpuid][i]);
			} 
		}
		PRINTF(" ---------- end ----------\n");
		PRINTF(" *************************\n\n\n");
	}

#endif /* _VMMDEBUG */
}

/*
 * NAME: v_descoreboard
 *
 * FUNCTION:	remove a lock from the scoreboard array.
 *
 * PARAMETERS: 	x	- lock address
 *
 * RETURN VALUE DESCRIPTION: None
 *
 */
void
#ifndef _VMMDEBUG
v_descoreboard(x)
#else  /* _VMMDEBUG */
v_descoreboard(x, LINE, FILE)
int LINE;
char *FILE;
#endif /* _VMMDEBUG */
simple_lock_t x;
{
	struct ppda *ppda = my_ppda();
	int i, hint, found;
#ifdef _VMMDEBUG
	int my_cpuid = CPUID; 

	assert(lock_depth[my_cpuid][0] > 0);
#endif

	hint = ppda->ppda_hint;
	ASSERT(hint >= 0 && hint < VMM_MAXLOCK);
	

	/* We want to try the hint first in case the last action was the
	 * corresponding lock (normal nested locks).
	 */
	if (ppda->scoreboard[hint] == x)
	{
		found = hint;
		goto descoreboard;
	}

	/* Then we try hint-1 in case the last action was an unlock
	 * (nested locks) or a lock (Cf getvmpage/getparent).
	 */
	if(hint-1 >= 0 && ppda->scoreboard[hint-1] == x)
	{
		found = hint-1;
		goto descoreboard;
	}
	
	/* Then we try hint+1 for getvmpage/getparent.
	 */
	if(hint+1 < VMM_MAXLOCK && ppda->scoreboard[hint+1] == x)
	{
		found = hint+1;
		goto descoreboard;
	}

	/* Full search, trying the lower index first
	 */
	for (i = 0; i < hint-1; i++)
		if (ppda->scoreboard[i] == x)
		{
			found = i;
			goto descoreboard;
		}
	
	for (i = hint+2; i < VMM_MAXLOCK; i++)
		if (ppda->scoreboard[i] == x)
		{
			found = i;
			goto descoreboard;
		}

	panic("descoreboard of a non scoreboarded vmm lock");

descoreboard:
	ppda->scoreboard[found] = NULL;
	ppda->ppda_hint = found;

#ifdef _VMMDEBUG	
	scoreboard_line[my_cpuid][found]= LINE <<4;
	strcpy(&scoreboard_file[my_cpuid][found], FILE);
	lock_depth[my_cpuid][0]--;
      	scoreboard_lock[my_cpuid][found]= 0;
#endif /* _VMMDEBUG */
}
	
/*
 * NAME: v_descoreboard_all
 *
 * FUNCTION:	unlock and remove all locks registered in the scoreboard array.
 *
 * PARAMETERS: 	None
 *
 * RETURN VALUE DESCRIPTION: None
 *
 */
void
#ifndef _VMMDEBUG
v_descoreboard_all()
#else  /* _VMMDEBUG */
v_descoreboard_all(LINE, FILE)
int LINE;
char *FILE;
#endif /* _VMMDEBUG */
{
	struct ppda *ppda = my_ppda();
	int i;
#ifdef _VMMDEBUG
	int my_cpuid = CPUID; 
#endif

	ppda->ppda_hint = 0;

	for (i = 0; i < VMM_MAXLOCK; i++)
		if (ppda->scoreboard[i])
		{
			simple_unlock(ppda->scoreboard[i]);
			ppda->scoreboard[i] = NULL;
#ifdef _VMMDEBUG
			scoreboard_line[my_cpuid][i]= LINE <<4;
			strcpy(&scoreboard_file[my_cpuid][i], FILE);
			lock_depth[my_cpuid][0]--;
      			scoreboard_lock[my_cpuid][i]= 0;
#endif /* _VMMDEBUG */
		} 

#ifdef _VMMDEBUG
	assert(!lock_depth[my_cpuid][0]);
	lock_sequence_number[my_cpuid][0]=0;
#endif /* _VMMDEBUG */
}

/*
 * NAME: v_descoreboard_most
 *
 * FUNCTION:	unlock and remove all locks EXCEPT the specified one
 *		registered in the scoreboard array.
 *
 * PARAMETERS: 	None
 *
 * RETURN VALUE DESCRIPTION: None
 *
 */
void
#ifndef _VMMDEBUG
v_descoreboard_most(x)
#else  /* _VMMDEBUG */
v_descoreboard_most(x, LINE, FILE)
int LINE;
char *FILE;
#endif /* _VMMDEBUG */
simple_lock_t x;
{
	struct ppda *ppda = my_ppda();
	int i, found;
#ifdef _VMMDEBUG
	int my_cpuid = CPUID; 
#endif

	ppda->ppda_hint = 0;
	found = 0;

	for (i = 0; i < VMM_MAXLOCK; i++)
		if (ppda->scoreboard[i])
		{
			if (ppda->scoreboard[i] == x)
			{
				found = 1;
				continue;
			}
			simple_unlock(ppda->scoreboard[i]);
			ppda->scoreboard[i] = NULL;
#ifdef _VMMDEBUG
			scoreboard_line[my_cpuid][i]= LINE <<4;
			strcpy(&scoreboard_file[my_cpuid][i], FILE);
			lock_depth[my_cpuid][0]--;
      			scoreboard_lock[my_cpuid][i]= 0;
#endif /* _VMMDEBUG */
		} 

	if (!found)
		panic("descoreboard_most of a non scoreboarded vmm lock");

#ifdef _VMMDEBUG
	assert(lock_depth[my_cpuid][0] == 1);
#endif /* _VMMDEBUG */
}

#ifdef _VMMDEBUG
/*
 * NAME: v_chknolock
 *
 * FUNCTION:	check that no lock remains registered in the scoreboard array.
 *
 * PARAMETERS: 	None
 *
 * RETURN VALUE DESCRIPTION: None
 *
 */
void
v_chknolock()
{
	struct ppda *ppda = my_ppda();
	int i;

	for (i = 0; i < VMM_MAXLOCK; i++)
		if (ppda->scoreboard[i])
			panic("vmm lock still scoreboarded");
	assert(!lock_depth[CPUID][0]);
	lock_sequence_number[CPUID][0]=0;
}

#endif /* _VMMDEBUG */
#endif /* _VMM_MP_EFF */

#endif /* _POWER_MP */
