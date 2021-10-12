/* @(#)28	1.19  src/bos/kernel/sys/POWER/ppda.h, sysml, bos411, 9438C411a 9/23/94 10:53:11 */
/*
 *   COMPONENT_NAME: SYSML
 *
 *   FUNCTIONS: Per processor data area
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_PPDA
#define _H_PPDA

/*
 * Warning:
 *	Do not change this file without also updating ppda.m4
 */

#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/proc.h>
#include <sys/processor.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/lock_def.h>
#include <sys/watchdog.h>
#include <sys/mpc.h>
#include <sys/buf.h>
#ifdef _KERNSYS
#include <sys/inline.h>
#endif /* _KERNSYS */

/*
 * Timer handling ppda structure.
 */
struct ppda_timer {
  struct trb         *t_free;          /* Free list                          */
  struct trb         *t_active;        /* Active list                        */
  unsigned int       t_freecnt;        /* Number on free list                */
  struct trb         *trb_called;      /* Trb in use if != NULL              */
  Simple_lock        trb_lock;       /* A simple lock to protect trb handling*/
  struct trb         *systimer;        /* This processors systimer           */
  int                ticks_its;        /* Tick counter within one second     */
  struct timestruc_t ref_time;         /* Ideal time for next systimer       */
  long               time_delta;       /* Unapplied correction               */
  int                time_adjusted;    /* adjtime or settimer called         */ 
  struct watchdog    wtimer;           /* Watchdog chain root                */
  struct watchdog    *w_called;        /* Watchdog in use if != NULL         */
};

/*
 * Some timer reference macros.
 */ 
#define MY_TIMER() (&(PPDA->ppda_timer))
#define TIMER(p)   (&(GET_PPDA((cpu_t)p)->ppda_timer))

/*
 * cache alignment filler 
 */
#define PPDA_DCACHE	32

#define	i_prilvl	prilvl		/* This is here due a compiler bug
					 * when fixed this can be removed */

struct ppda {
	uint 		alsave[16];	/* save area for the alignment handler*/
	struct mstsave 	*_csa;		/* current save area */
	struct mstsave 	*mstack;	/* next available mst */
	struct thread 	*fpowner;	/* current floating point owner */
	struct thread	*_curthread;	/* currently dispatched thread */

	long	syscall;		/* count of sys calls/processor */

	uint 	save[8];		/* flih scratch save area */
	void 	*intr;			/* power PC interrupt registers */
	/*
	 * i_softis & i_softpri must stay in the same word
	 */
	ushort	i_softis;		/* HW assisted software intr flags */
	ushort	i_softpri;		/* SW managed model priority flags */

	uint	prilvl[INTTIMER];	/* Array of priorities to levels   */
					/* When MAX_LVL_PER_PRI goes beyond*/
					/* 32 then this will have to become*/
					/* a 2 dimensional arrary. PRILVL_LEN*/
					/* should handle it                */
	uint	dar;			/* saved value fo DAR */
	uint	dsisr;			/* saved value of DSISR */
	uint	dsi_flag;		/* flag used by DSI flih */
	uint	dssave[8];		/* save area for dsi flih */

	/*
	 * The schedtail array contains a list of request scheduled for
	 * off-level processing. A request is added to it by i_sched and
	 * a request is * processed from it by i_softint.
	 * There is one entry in it for each
	 * off-level interrupt priority managed by i_sched/i_softint. The
	 * index is the interrupt priority - INTOFFL0.
	 */
	struct intr *schedtail[INTOFFL3-INTOFFL0+1];

	/*
	 * cpuid (short), stackfix (char), lru (char) must stay as positioned
	 * and in the same aligned word for atomic access via fetch_and_noop()
	 * (see getstackfix()).
	 */
	cpu_t	cpuid;			/* logical cpu number */ 
	char	stackfix;		/* set during disabled crit sections */
	char	lru;			/* set during VMM critical sections */

	/*
	 * VMM flags.
	 */ 
	union {
		int _vmflags;
		struct {
			char 	_sio;		/* flag: start i/o if set */
			char	_reservation;	/* free frame reservation */
			char	_hint;		/* hint in the scoreboard */
			char	_vmrsvd;	/* reserved		  */
		} s1;
	} u1;
	int	no_vwait;		/* flag: no v_wait if set */

	/*
	 * VMM lock scoreboard area.
	 */
#define VMM_MAXLOCK	8
	simple_lock_t scoreboard[VMM_MAXLOCK];

	uint	mfrr_pend;		/* pending mfrr hw ints */
	uint	mfrr_lock;
	uint	mpc_pend;		/* pending mpc services */

	struct buf *iodonelist; 	/* per-cpu iodone list */
	struct ppda_timer ppda_timer;   /* timer fields */

       	struct thread *affinity;        /* affinity pointer */

	struct mstsave *kdb_csa;	/* address of current kdb save area */
	struct mstsave *kdb_mstack;    	/* next kdb area in mstsave pool */
	uint kdb_flih_save[9];		/* flih save area */

	uint	TB_ref_u;		/* ref. of Time Base value: upper */
	uint	TB_ref_l;		/* ref. of Time Base value: lower */
	uint	sec_ref;		/* ref. of Time Base value: seconds */
	uint	nsec_ref;		/* ref. of Time Base value: nanosecs */

	char	_ficd;			/* finish interrupt call dispatcher */
	char	_rsvd[3];
	void	*decompress;		/* filesystem decompression buffer */
	int	ppda_qio;		/* VMM queued I/O pages */
	int	cs_sync;		/* Synchronous MPC indicator */
	uint	pmapstk;		/* address of V=R pmap stack */
	char	filler[PPDA_DCACHE];	/* cache alignment */
};

#define ppda_vmflags		u1._vmflags
#define ppda_sio		u1.s1._sio
#define ppda_reservation	u1.s1._reservation
#define ppda_hint		u1.s1._hint
#define ppda_no_vwait		no_vwait

extern struct ppda ppda[];

#endif /* _H_PPDA */
