/* @(#)17       1.6  src/bos/kernext/rcm/inc/rcmhsc.h, rcm, bos411, 9433B411a 8/16/94 18:11:01 */
/* COMPONENT_NAME: (rcm) Rendering Context Manager Heavy Switch Controller
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
	
#ifndef _H_RCMHSC
#define _H_RCMHSC

#include <sys/rcm.h>

/*
 *  Maximum outstanding HSC requests.
 *
 *  Currently there will be one or two requests per active device.
 */
#define MAX_HSC_QES     16

/*
    events known to the HSC
*/
#define HSC_WAIT_INIT       0x80000000    /* waiting on initialization */
#define HSC_WAIT_Q          0x40000000    /* waiting on queue element */


/* HSC Q element
    The HSC q element contains all the information needed to
    complete a heavy rendering context switch.
*/

typedef struct _hscQE {
    int         command;        /* command */
    gscDevPtr   pDev;           /* pointer to device */
    rcxPtr      pNew, pOld;     /* pointers to new and old rcxs */
    /***************************** pNew will contain a pointer to a gscdma
	                           structure if command==HSC_DMA_CLEANUP */
    int         seq;            /* sequence number to aid in synchronization
	                           between HSC and RCM */
} hscQE, *hscQEPtr;

/* HSC commnands */

#define HSC_COM_SWITCH      1   /* switch contexts */
#define HSC_COM_TERM        2   /* terminate the hsc */
#define HSC_DMA_CLEANUP     3   /* off-level DMA cleanup */


/* HSC Q
    The HSC q contains all the requests to perform a heavy rendering
    context switch.
*/

typedef struct _hscQ {
    long        head;                 /* head ptr of q */
    long        tail;                 /* tail ptr of q */
    hscQE       qe[MAX_HSC_QES];      /* space for q elements */
} hscQ, *hscQPtr;


/* HSC state
    The HSC state contains information about the HSC and the HSC q
*/

typedef struct _hscState {
    pid_t       pid;            /* process id of the HSC */
    pid_t       i_pid;          /* process id for initializer of HSC */
    int         flags;          /* status flags */
    hscQ        Q;              /* the hsc q */
} hscState, *hscStatePtr;

/* flags values */
#define HSC_INIT        1       /* HSC initialized */


/* return codes for HSC functions */

#define HSC_NO_INIT     25      /* return code - not initialized */
#define HSC_Q_OVRFLW    26      /* q has overflowed */
#define HSC_Q_SUCC      0       /* qe queued successfully */


/*
	entry points
*/

extern long rcmhsc();
extern int hsc_enq();

#endif  /* _H_RCMHSC */
