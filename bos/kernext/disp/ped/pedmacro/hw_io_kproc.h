/* @(#)97       1.1  src/bos/kernext/disp/ped/pedmacro/hw_io_kproc.h, pedmacro, bos411, 9428A410j 3/24/94 13:59:21 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS:      definition of mid-level graphics adapter I/O kproc xface
 *
 * ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_MID_IO_KPROC
#define _H_MID_IO_KPROC

#include <sys/types.h>
#include <sys/lockl.h>                  /* to define lock_t */
#include <keytab.h>     /* define kdd_data for vt.h */
#include <sys/hft.h>    /* define COLORPAL, FONTPAL & keystroke for vt.h */
#include <vt.h>                  	/* to define vtmstruc */

/* ---------------------------------------------------------------------
   IO_kproc Q element

    The io_k_proc q element contains all the information needed to
    complete each of the request queued to the kernel process.

    The request codes are found in ped/inc/mid_io_kproc_xface.h.
   --------------------------------------------------------------------- */

typedef struct _mid_iok_qe_t {

    struct vtmstruc *   pVT ;           /* pointer to VT structure */
    int                 req_code;       /* request code */

} mid_iok_qe_t, *pmid_iok_qe_t;



/* ---------------------------------------------------------------------
   PCB stack definitions
   --------------------------------------------------------------------- */

#define MID_PCB_STACK_TOP               (MID_IO_Q.pcb_stack.top)
#define MID_PCB_STACK_IN                (MID_IO_Q.pcb_stack.in)
#define MID_PCB_STACK_OUT               (MID_IO_Q.pcb_stack.out)
#define MID_PCB_STACK_BOTTOM            (MID_IO_Q.pcb_stack.bottom)


#define MID_PCB_STACK_IS_EMPTY         (MID_PCB_STACK_IN == MID_PCB_STACK_OUT)
#define MID_PCB_STACK_IS_NOT_EMPTY     (MID_PCB_STACK_IN != MID_PCB_STACK_OUT)

#define MID_PCB_STACK_LENGTH    200

#define MID_PCB_ADAPTER_FIFO_LENGTH      8      /* I/O limited to 8 words */


/* ---------------------------------------------------------------------
   PCB stack structure
   --------------------------------------------------------------------- */

typedef struct _mid_pcb_stack_data_t
{
	ulong *         top ;
	ulong *         bottom ;
	ulong *         in ;
	ulong *         out ;

	int             unstack_event_word ;
	lock_t          pcb_ind_lockword;
	ulong           spare1 ;
	ulong           spare2 ;

	ulong           stack[MID_PCB_STACK_LENGTH] ;

} mid_pcb_stack_data_t, *pmid_pcb_stack_data_t ;



/* ---------------------------------------------------------------------
    The queue management structure

    This structure contains all the actual request Q as well as in and out
    pointers.

    For various and differing reasons, there can only be one outstanding
    request of each type.  The maximum number of outstanding requests is
    therefore less than: */
#define MID_MAX_IO_Q     8
   /* ------------------------------------------------------------------ */

typedef struct _mid_iok_Q_t {
    pid_t               pid;                /* pid of the k-proc */
    pid_t               i_pid;              /* pid of initializer of kproc */
    ulong               flags;              /* status flags */
#       define MID_IOK_INIT             1L>>31          /* k-proc init */
#       define MID_IOK_FUNCTIONAL       1L>>30          /* k-proc functional */
#       define MID_IOK_MOM	        1L>>29          /* active VT in MOM */
#       define MID_IOK_PCB_IO_NQed      1L>>27          /* PCB req nq-ed */
#       define MID_IOK_FLUSH_NQed       1L>>26          /* FLUSH req nq-ed */
    ulong               flush_sleep;              /* flush sleep word */

    ushort              in;                 /* index of next request in */
    ushort              out;                /* index of next request out */

    mid_iok_qe_t        qe [MID_MAX_IO_Q];    /* space for q elements */

    mid_pcb_stack_data_t        pcb_stack ;             /* PCB stack data */

} mid_iok_Q_t, *pmid_iok_Q_t;



/* ---------------------------------------------------------------------

    Events known to this kernel process:

    These are the events on the e_wait / e_post interface.  
   --------------------------------------------------------------------- */

#define MID_IOK_WAIT_INIT       0x80000000    /* waiting on initialization */
#define MID_IOK_WAIT_Q          0x40000000    /* waiting on queue element */



#endif /* _H_MID_IO_KPROC */
