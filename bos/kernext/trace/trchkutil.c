 static char sccsid[] = "@(#)58       1.12  src/bos/kernext/trace/trchkutil.c, systrace, bos41J, 9519A_all 5/4/95 09:18:36";
/*
 * COMPONENT_NAME: SYSTRACE   /dev/systrace pseudo-device driver
 *
 * FUNCTIONS: trcgen, trcgent, trcgenk, trcgenkt
 *            trchk_init, trcbuffull, trcbuffullgen
 *
 * ORIGINS: 27 83
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
 * LEVEL 1, 5 Years Bull Confidential Information
*/

/*
 * SYSTRACE: trcgenG, trcgentG, trcgenkG, trcgenktG, trcbuffull
 *
 * trace hook buffer overflow and generic trace routines
 * Everything is pinned.
 * trcbuffull() runs on a fixed stack.
 *
 * trc_timeoutinit() and trc_timeout() are moved to kernel/io 
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/intr.h>
#include <sys/syspest.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/trchdr.h>
#include <sys/low.h>
#ifdef _POWER_MP
#include <sys/atomic_op.h>
extern int trc_lock[]; /* declared in trcdd.c */
#endif


#undef  MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#undef MAX
#define MAX(a,b)  ( ((a) > (b))? (a): (b) )

#define ROUNDW(len) (((len) + sizeof(int) - 1) / sizeof(int)) /* char to int */

#define TG_TIMESTAMP 0x01
#define TG_USER      0x02

extern struct trb *talloc();
/* Move timeout stuffs in kernel */
/* struct trb *trc_timerp; */		/* talloc-ed once at trcinit() time */ 

/* Off-level handler for wakeup processing, its handler structure,
 * and the indicator of which channel to wake up for.
 */
extern int trc_offlevel();
static struct intr trc_handler[8] = {
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, },
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, },
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, },
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, },
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, },
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, },
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, },
{0, trc_offlevel, BUS_NONE, 0, INT_OFFL0, INTOFFL0, 0, }};


#ifdef _POWER_MP
uint trcwakechan;
#else
uchar trcwakechan;
#endif

/* Bus sreg value and address used for to-bus trace mode.
 */
extern uint trc_bussreg;
extern uint trc_busaddr;

/*
 *
 *      Routine names modified :
 *       ***********************
 *
 *      OLd _trcgen() becomes _trcgenXXX()
 *      OLd trcgen() becomes trcgenG()
 *      OLd trcgent() becomes trcgentG()
 *      OLd trcgenk() becomes trcgenkG()
 *      OLd trcgenkt() becomes trcgenktG()
*/

/* static void _trcgen(); */
static void _trcgenXXX(); 
static void _trcgenXXX_user(); 


#define TRC_ISONSPILL(chan) \
	(Trconflag[chan] != 0 && Trconflag[chan] != 4)

#define TRCCHK() \
	if(!TRC_ISONSPILL(chan) || (unsigned)chan >= TRC_NCHANNELS) \
		return;

void trcgenG(chan,hookword,d1,len,buf)
char *buf;
{

	TRCCHK();			/* set tp, return if bad channel */
	_trcgenXXX_user(TG_USER,chan,hookword,d1,len,buf);
}

void trcgentG(chan,hookword,d1,len,buf)
char *buf;
{

	TRCCHK();			/* set tp, return if bad channel */
	_trcgenXXX_user(TG_TIMESTAMP|TG_USER,chan,hookword,d1,len,buf);
}

void trcgenkG(chan,hookword,d1,len,buf)
char *buf;
{

	TRCCHK();			/* set tp, return if bad channel */
	_trcgenXXX(0,chan,hookword,d1,len,buf);
}

void trcgenktG(chan,hookword,d1,len,buf)
char *buf;
{

	TRCCHK();			/* set tp, return if bad channel */
	_trcgenXXX(TG_TIMESTAMP,chan,hookword,d1,len,buf);
}

static void _trcgenXXX(mode,chan,hookword,d1,len,buf)
char *buf;
{
	int s;
	int wlen;
	register struct trchdr *tp;
	int type;

	tp = &trchdr[chan];
	if(tp->trc_state & ST_COND && !ISEVENT(tp,hookword))
		return;
        len = MIN(GENBUFSIZE+4+4+4,len);
        len = MAX(0,len);
        wlen = ROUNDW(len);
        type = (mode & TG_TIMESTAMP) ? HKTY_VT : HKTY_V;
        hookword = (hookword & 0xFFF00000) | type | len;
        trcgenasm(hookword,d1,buf,wlen,tp,chan);
}

static void _trcgenXXX_user(mode,chan,hookword,d1,len,buf)
char *buf;
{
	int s;
	int wlen;
	register struct trchdr *tp;
	int type;
	char ibuf[GENBUFSIZE+4+4+4];

	tp = &trchdr[chan];
	if(tp->trc_state & ST_COND && !ISEVENT(tp,hookword))
		return;
        len = MIN(GENBUFSIZE+4+4+4,len);
        len = MAX(0,len);
        wlen = ROUNDW(len);
        type = (mode & TG_TIMESTAMP) ? HKTY_VT : HKTY_V;
        hookword = (hookword & 0xFFF00000) | type | len;

	if(copyin(buf,ibuf,len) != 0)
		return;
        trcgenasm(hookword,d1,ibuf,wlen,tp,chan);
}

/*
 * NAME:     trchk_init
 * FUNCTION: turn on trchk routines according to tmode
 *           0 means disable tracing
 * INPUTS:   tp       pointer to trchdr structure
 *           tmode    0 off
 *           tmode    1 to-memory
 *           tmode    2 conditional to-memory
 *           tmode    3 bus
 *           tmode    4 spill
 * RETURNS:  none
 *
 * This routine is called at INTMAX
 */
trchk_init(tp,tmode)
struct trchdr *tp;
{
	Trconflag[tp-trchdr] = tmode;
	mem_sync();

	/* TEMPORARY - for fetch-protect/store problem.
	 * This can be removed when we no longer need to support
	 * the hardware level with this problem.
	 *
	 * The Trconflag array must be copied to the user's
	 * kernel-shadow segment since it is readable from
	 * user-level code.
	 */
	 ukerncopy(Trconflag, sizeof(Trconflag[0]) * TRC_NCHANNELS);
	mem_sync();
}


/*
 * BUFFULL()  macros
 * goto next fsm state
 * save the inptr and overflow count
 * set the current fifo to the next fifo
 */

#define BUFFULL_A(tp,fsm) \
	FULL_WRAP(tp); \
	trcsetfsm(tp,fsm); \
	tp->trc_inptrA = tp->trc_inptr; \
	tp->trc_inptrB = tp->trc_startB; \
	tp->trc_currq = tp->trc_Bq;

#define BUFFULL_B(tp,fsm) \
	FULL_WRAP(tp); \
	trcsetfsm(tp,fsm); \
	tp->trc_inptrB = tp->trc_inptr; \
	tp->trc_inptrA = tp->trc_startA; \
	tp->trc_currq  = tp->trc_Aq;

#define BUFFULL_A_SPILL(tp,fsm) \
	FULL_WRAP(tp); \
	trchk_init(tp,4); \
	trcsetfsm(tp,fsm); \
	tp->trc_inptrA = tp->trc_inptr;

#define BUFFULL_B_SPILL(tp,fsm) \
	FULL_WRAP(tp); \
	trchk_init(tp,4); \
	trcsetfsm(tp,fsm); \
	tp->trc_inptrB = tp->trc_inptr;

#define FULL_WRAP(tp) \
if((char *)tp->trc_inptr <= (char *)tp->trc_end + TRC_OVF - 8) {\
	*tp->trc_inptr++ = HKWD_TRACE_TWRAP|HKTY_L|tp->trc_wrapcount++ & 0xFFFF; \
	*tp->trc_inptr++ = tp->trc_ovfcount; \
	tp->trc_ovfcount = 0;}

#define LOCKSPIN(addr_lock_trc) while (!(test_and_set(addr_lock_trc,1<<29)));\
					mem_isync();
	
#define UNLOCKSPIN(addr_lock_trc)	mem_sync();\
					fetch_and_and(addr_lock_trc,0);
/*
 * WAKEUP(tp)  macro
 * schedule the wakeup for off-level processing -- routine 'trc_offlevel'
 * will be invoked and will make the call to 'e_wakeup'.
 * a bit is set in 'trcwakechan' to indicate which channel (0-7) to
 * wake up on (note that we are disabled when this is done).
 */

#ifdef _POWER_MP
#define WAKEUP(tp) \
	fetch_and_or((int *)&trcwakechan, ( 1 <<( (tp-trchdr)+24 ) )); \
	mem_sync(); \
        i_sched( &trc_handler[tp-trchdr] );

#else
#define WAKEUP(tp) \
	trcwakechan |= 1 << (tp-trchdr); \
	i_sched( &trc_handler[tp-trchdr] );
#endif


/*
 * NAME:        trcbuffull
 * FUNCTION:    handle buffer full conditions for the 3 to-memory trace modes
 * INPUTS:      tp  pointer to a trchdr structure (1 of 8)
 * RETURNS:     none
 * CALLS:       none
 * ENVIRONMENT: All interrupts disabled
 *              Fixed stack is supplied by the calling assembler routine
 *
 * Note.  The actual wakeup is not done here in order to reduce path length
 *        and because of problems associated with calling process management
 *	  routines that might end up making trace hook calls.
 *	  Instead, the wakeup is scheduled to be done off-level.
 *
 * trcbuffull() is called by the trchook routine when the buffer it is tracing
 *  into has reached its limit.
 * It will switch to a new buffer with the BUFFULL() macro.
 * If the new buffer is not ready (as determined by the fsm_state), trcbuffull
 *  will temporarily turn off tracing with trchk_init . This makes
 *  the trchook routine return to its caller without storing into the
 *  trace buffer.
 */

extern int trcbuffullgen();
trcbuffull(tp)
register struct trchdr *tp;
{
        trcbuffullgen(tp,tp->trc_inptr);
}

/* a specific trcbuffull for trcgenasm() */
/* the problem is that trc_inptr is update before the test if the buffer */
/* is full. This is necessary to be sure that just one processor empty */
/*  the buffer. So, it need the trc_inptr before the update ie gptr */

#define GEN_BUFFULL(tp,gptr,fsm,inptr_full,inptr,start,end,size) \
        GEN_FULL_WRAP(tp,gptr); \
        trcsetfsm(tp,fsm); \
        compare_and_swap ((int *)&(inptr_full), (int *)&(inptr_full), (int)gptr); \
        compare_and_swap ((int *)&(inptr), (int *)&(inptr), (int)start); \
        compare_and_swap ((int *)&(tp->trc_inptr), (int *)&(tp->trc_inptr), \
       	(int)MAX (tp->trc_inptr, end)); /*allways trc_inptr > trc_end*/\
        compare_and_swap ((int *)&(tp->trc_start), (int *)&(tp->trc_start), (int)start); \
        compare_and_swap ((int *)&(tp->trc_size), (int *)&(tp->trc_size), (int)size); \
        compare_and_swap ((int *)&(tp->trc_end), (int *)&(tp->trc_end), (int)end); \
        compare_and_swap ((int *)&(tp->trc_inptr), (int *)&(tp->trc_inptr), (int)inptr);

#define GEN_BUFFULL_A(tp,gptr,fsm) \
	 GEN_BUFFULL(tp,gptr,fsm,tp->trc_inptrA,tp->trc_inptrB,tp->trc_startB,\
			tp->trc_endB,tp->trc_sizeB)

#define GEN_BUFFULL_B(tp,gptr,fsm) \
	GEN_BUFFULL(tp,gptr,fsm,tp->trc_inptrB,tp->trc_inptrA,tp->trc_startA,\
		tp->trc_endA,tp->trc_sizeA)


#define GEN_BUFFULL_SPILL(tp,gptr,fsm,inptr) \
        GEN_FULL_WRAP(tp,gptr); \
        trchk_init(tp,4); \
        trcsetfsm(tp,fsm); \
        compare_and_swap ((int *)&(inptr), (int *)&(inptr), (int )gptr);

#define GEN_BUFFULL_A_SPILL(tp,gptr,fsm) \
	GEN_BUFFULL_SPILL(tp,gptr,fsm, tp->trc_inptrA)

#define GEN_BUFFULL_B_SPILL(tp,gptr,fsm) \
	GEN_BUFFULL_SPILL(tp,gptr,fsm, tp->trc_inptrB)

#define GEN_FULL_WRAP(tp,gptr) \
if((char *)gptr <= (char *)tp->trc_end + TRC_OVF - 12) {\
        *gptr = HKWD_TRACE_TWRAP|HKTY_L|tp->trc_wrapcount++ & 0xFFFF; \
	fetch_and_add((int *)&gptr,4); \
        *gptr = tp->trc_ovfcount; \
	fetch_and_add((int *)&gptr,4); \
        *gptr = thread_self(); \
	fetch_and_add((int *)&gptr,4); \
        tp->trc_ovfcount = 0;}

extern struct timestruc_t c_time;

trcbuffullgen(tp,gptr)
register struct trchdr *tp;
int *gptr;
{
#ifdef _POWER_MP
        LOCKSPIN(&(trc_lock[tp - trchdr]))
#endif
        switch(tp->trc_mode) {
        case MD_OFF:
                ASSERT(tp->trc_mode != MD_OFF);
                trcfail(tp);
                break;
        case MD_CIRCULAR:
        case MD_CIRCULAR_COND:
                switch(tp->trc_fsm) {
                case FSM_A_BLOCK: GEN_BUFFULL_A(tp,gptr,FSM_B_BLOCK); break
;
                case FSM_B_BLOCK: GEN_BUFFULL_B(tp,gptr,FSM_A_BLOCK); break
;
                default: ASSERT(0 == MD_CIRCULAR); trcfail(tp);
                }
                break;
        case MD_ALTERNATE:
        case MD_ALTERNATE_COND:
                switch(tp->trc_fsm) {
                case FSM_A_BLOCK: GEN_BUFFULL_A(tp,gptr,FSM_B_A); WAKEUP(tp); break;
                case FSM_B_BLOCK: GEN_BUFFULL_B(tp,gptr,FSM_A_B); WAKEUP(tp); break;
                case FSM_A_B:     GEN_BUFFULL_A_SPILL(tp,gptr,FSM_SPILL_B);
   break;
                case FSM_B_A:     GEN_BUFFULL_B_SPILL(tp,gptr,FSM_SPILL_A);
   break;
                default: ASSERT(0 == MD_ALTERNATE); trcfail(tp);
                }
                break;
        case MD_SINGLE:
        case MD_SINGLE_COND:
                switch(tp->trc_fsm) {
                case FSM_A_BLOCK:
			tp->trc_state &= ~ST_TRCON;
 			curtime(&c_time);
                        *gptr++ = HKWD_TRACE_TRCOFF | HKTY_LT;
			*gptr++ = c_time.tv_sec;
 			*gptr++ = thread_self();
                        *gptr++ = TIMESTAMP;
                        GEN_BUFFULL_A_SPILL(tp,gptr,FSM_SPILL_A);
                        WAKEUP(tp);
                        break;
                default:
                        ASSERT(0 == MD_SINGLE);
                        trcfail(tp);
                }
                break;
        default:
                ASSERT(tp->trc_mode == MD_OFF);
                trcfail(tp);
        }
#ifdef _POWER_MP
        UNLOCKSPIN(&(trc_lock[tp - trchdr]))
#endif
}


/*
 * NAME:        trcfail
 * FUNCTION:    handle trace failures such as inconsistent state change.
 */
trcfail(failtp)
register struct trchdr *failtp;
{
	/* Disable trace.
	 */
	trchk_init(failtp,0);
}


/*
State diagrams for buffer control for the CIRCULAR, ALTERNATE,
and SINGLE trace modes.

1. The bracketed words with arrows going into and out of are FSM states.
   (FSM = finite state machine)

2. The words next to the lines are the events:
   trcon2     trcon ioctl
   trcoff2    trcoff ioctl
   trcstop    trcstop ioctl
   BUFFULL_A  A buffer becoming full
   BUFFULL_B  B buffer becoming full
   EMPTYCHK_A A buffer becoming empty
   EMPTYCHK_B B buffer becoming empty

   The "event" ST_TRCSTOP is not an event. It is a global flag that
   records that the 'trcstop' event has occurred. When the SPILL_BLOCK state
   is entered, if the ST_TRCSTOP flag is on, then the SPILL_EOF state is
   entered right away. This special flag avoids the need to have a separate
   path for trcstop's and trcoff's. Except in the SPILL_BLOCK state, a
   'trcstop' is a 'trcoff' with the ST_TRCSTOP flag set.

3. The naming convention for the states breaks the name into a
   write description and a read description.

   The write descriptions are:
   A       trchk routines write to buffer A
   B       trchk routines write to buffer B
   SPILL   trchk routines don't write anything

   The read descriptions are:
   A       trcread() routine reads from buffer A
   B       trcread() routine reads from buffer B
   BLOCK   trcread() routine is blocked, waiting for a buffer to become ready
   EOF     trcread() routine returns EOF
   
   In addition, there is the description 'OFF' that is used to signify that
   a trcoff event has occurred, which puts the state machine in a different
   path. The 'OFF' path is used to synchronize when the buffers can be
   re-enabled by trcon. If a trcon ioctl is issued when in a state that does
   not have a trcon event associated with it (eg. CIRCULAR SPILL_B_OFF), a
   EBUSY errno is returned.
   

CIRCULAR MODE
                            BUFFULL_B
         +--------------------<---------------------+
         |                                          |
         |                  BUFFULL_A               |
         +----->[A_BLOCK]------------>[B_BLOCK]->---+
         |          |                     |
         |          |trcoff2              |trcoff2
         |          v                     v
         |    [SPILL_B_OFF]         [SPILL_A_OFF2]
         ^          |                     |
         |          |EMPTYCHK_B           |EMPTYCHK_A
         |          v                     v
         |    [SPILL_A_OFF]         [SPILL_B_OFF2]
         |          |                     |
         |          |EMPTYCHK_A           |EMPTYCHK_B
         |          +--------+  +---------+
         |                   |  |
         |                   v  v
         |               [SPILL_BLOCK]
         |                   |  |
         |    trcon2         |  |trcstop/ST_TRCSTOP
         +---------<---------+  |
                                |
                                v
                          [SPILL_EOF]

SINGLE-SHOT MODE

         +----->[A_BLOCK]----->-----------+
         |          |                     |
         |          |BUFFULL_A_SPILL      |trcstop/ST_TRCSTOP
         |          v                     v
         |    [SPILL_A]           	[SPILL_A_OFF]
         |          |                     |
         |          |EMPTYCHK_A_START     |EMPTYCHK_A
         |          +--------+            |
         |                   |            |
         |                   v            |
         |                   |            |
         |                   |            |
         +---------<---------+            |
                                          |
					  v
                          		[SPILL_EOF]

ALTERNATE MODE

+-------------------------------------------<---------------------------------+
|        EMPTYCHK_B       BUFFULL_A           EMPTYCHK_A       BUFFULL_B      |
|              +-->[A_BLOCK]--->-+                  +-->[B_BLOCK]--->-+       |
|              |        |        |                  |        |        |       |
+--->[A_B]-->--|        |trcoff2 |------>-[B_A]-->--|        |trcoff2 |-->----+
^      |       |        |        |      ^   |       |        |        |       ^
|      | BUFFULL_A_SPILL| EMPTYCHK_B_ON |   | BUFFULL_B_SPILL| EMPTYCHK_B_ON  |
|      v       +-->[SPILL_B]--->-+      |   v       +-->[SPILL_A]--->-+       |
|      |             |  |               |   |             |  |                |
|      |trcoff2      |trcoff2           |   |trcoff2      |trcoff2            |
|      |             |  |               |   |             |  |                |
|      +---->-----+  |  |               |   +---->-----+  |  |                |
|                 |  |  |               |              |  |  |                |
|                 v  v  |               |              v  v  |                |
|              [SPILL_B_OFF]            |           [SPILL_A_OFF2]            |
|       trcon2    |  |  |               |    trcon2    |  |  |                |
|<----------------+  |EMPTYCHK_B        |<-------------+  |EMPTYCHK_A         |
|                    v  v               |                 v  v                |
|              [SPILL_A_OFF]            |           [SPILL_B_OFF2]            |
|                    |  |               |                 |  |                |
|                    |  |trcon2         |                 |  |trcon2          |
|                    |  +---->----------+                 |  +--------->------+
|                    |                                    |
|                    |EMPTYCHK_A                          |EMPTYCHK_B
^                    +------->--------+     +---<---------+
|                                     |     |
|                                     v     v
|                                  [SPILL_BLOCK]
|                                      |   |
|                         trcon2       |   |trcstop/ST_TRCSTOP
+------------<-------------------------+   |
                                           |
                                           v
                                    [SPILL_EOF]

The states are shown enclosed in brackets.
The events that cause state transitions are shown as directed lines connecting
   the states.
The name of the routine (or macro) called by the event is shown for each event
   as the label above the line.

*/

