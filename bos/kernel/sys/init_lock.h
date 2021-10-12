/* @(#)86	1.9  src/bos/kernel/sys/init_lock.h, sysproc, bos411, 9428A410j 5/19/94 08:17:32 */

#ifndef _H_INIT_LOCK
#define _H_INIT_LOCK

/*
 * COMPONENT_NAME: SYSPROC 
 *
 * ORIGIN: 83
 *
 *
 * Copyright International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

/* the following defines need to be verified and tuned */

#define FAMILY_LOCK_SIZE        (MAX_FAMILY*sizeof(struct lock_data_instrumented))      /* size of family_lock_statistics table */
#define POOL_INITIAL_SIZE       (4*LOCK_PER_PAGE*sizeof(struct lock_data_instrumented))      /* initial pools size */
#define INITIAL_LOCK_NUMBER     4*LOCK_PER_PAGE /* number of lock initially allocated */

#ifdef  _POWER_MP
#include <sys/types.h>
#include <sys/overlay.h>
#include <sys/lock_def.h>

/* Instrumented overlay areas */
extern int simple_lock_instr_ppc[];
extern int simple_unlock_instr_ppc[];
extern int simple_lock_try_instr_ppc[];
extern int rsimple_lock_instr_ppc[];
extern int disable_lock_instr_ppc[];
extern int unlock_enable_instr_ppc[];
extern int lockl_instr_ppc[];
extern int unlockl_instr_ppc[];

/* Instrumented branch functions */
void simple_lock_init_instr();
void slock(), slock_instr_ppc();
void sunlock(), sunlock_instr_ppc();
boolean_t lock_mine(), lock_mine_instr_ppc();
void lock_init_instr();
void lock_write_instr_ppc();
void lock_read_instr_ppc();
void lock_done_instr_ppc();
boolean_t lock_read_to_write_instr_ppc();
void lock_write_to_read_instr_ppc();
boolean_t lock_try_write_instr_ppc();
boolean_t lock_try_read_instr_ppc();
boolean_t lock_try_read_to_write_instr_ppc();
void lock_set_recursive_instr();
void lock_clear_recursive_instr();
int lock_islocked_instr();
void lock_alloc_instr();
void lock_free_instr();
int klockl(), klockl_instr();
void kunlockl(), kunlockl_instr();

/* This table defines instrumented functions that are overlays.  Only functions
 * that are performance critical are reached with this mechanism.
 * The files overlay.h and overlay.m4 must be carefully
 * updated when adding functions to this table.
 */

struct  {
        int *ov_addr;           /* target address of overlay */
        int size;               /* maximum size of overlay */
        int *addr_instr;        /* instrumentation function name */
        } lock_overlay_data [] =
        {
		{(int *)SIMPLE_LOCK_ADDR, SIMPLE_LOCK_SIZE, &simple_lock_instr_ppc[0]},
                {(int *)SIMPLE_UNLOCK_ADDR, SIMPLE_UNLOCK_SIZE, &simple_unlock_instr_ppc[0]},
                {(int *)SIMPLE_LOCK_TRY_ADDR, SIMPLE_LOCK_TRY_SIZE, &simple_lock_try_instr_ppc[0]},
		{(int *)RSIMPLE_LOCK_ADDR, RSIMPLE_LOCK_SIZE, &rsimple_lock_instr_ppc[0]},
                {(int *)I_DISABLE_ADDR, I_DISABLE_SIZE+DISABLE_LOCK_SIZE, &disable_lock_instr_ppc[0]},
                {(int *)I_ENABLE_ADDR, I_ENABLE_SIZE+UNLOCK_ENABLE_SIZE, &unlock_enable_instr_ppc[0]},
		{(int *)LOCKL_ADDR, LOCKL_SIZE, &lockl_instr_ppc[0]},
		{(int *)UNLOCKL_ADDR, UNLOCKL_SIZE, &unlockl_instr_ppc[0]}
	};
#define LOCK_OVERLAY_NUMBER (sizeof(lock_overlay_data)/sizeof(lock_overlay_data[0]))

struct  {
                int     (*ext_name)();
                int     (*instr_name)();
        } lock_branch_data [] =
        {       {simple_lock_init,	simple_lock_init_instr},
                {slock,			slock_instr_ppc},
                {sunlock,		sunlock_instr_ppc},
	        {lock_mine,             lock_mine_instr_ppc},
                {lock_init,		lock_init_instr},
                {lock_write,		lock_write_instr_ppc},
                {lock_read,		lock_read_instr_ppc},
                {lock_done,		lock_done_instr_ppc},
                {lock_read_to_write,	lock_read_to_write_instr_ppc},
                {lock_write_to_read,	lock_write_to_read_instr_ppc},
                {lock_try_write,	lock_try_write_instr_ppc},
                {lock_try_read,		lock_try_read_instr_ppc},
                {lock_try_read_to_write,lock_try_read_to_write_instr_ppc},
                {lock_set_recursive,	lock_set_recursive_instr},
                {lock_clear_recursive,	lock_clear_recursive_instr},
                {lock_islocked,		lock_islocked_instr},
                {lock_alloc,		lock_alloc_instr},
                {lock_free,		lock_free_instr},
                {klockl,                klockl_instr},
                {kunlockl,              kunlockl_instr}
        };

#define LOCK_PRIM_NUMBER (sizeof(lock_branch_data)/sizeof(lock_branch_data[0]))

#endif /* _POWER_MP */

#endif /* _H_INIT_LOCK */
