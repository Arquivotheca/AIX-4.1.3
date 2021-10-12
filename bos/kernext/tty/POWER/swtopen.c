/* @(#)06 1.7 src/bos/kernext/tty/POWER/swtopen.c, sysxcommon, bos41J, 9516B_all 4/18/95 08:47:26 */
/*
 *  
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *  
 * FUNCTIONS:
 *    wto_open, wto_close, wto_input, wto_start, wto_closeit, wto_dsrfell,
 *    wto_dsrrose.
 *  
 * ORIGINS: 27, 83
 *  
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/sleep.h>
#include <sys/stream.h>      /* For redefinition of timeout by pse */
#include <sys/str_tty.h>
#include "ttydrv.h"
#include "common_open.h"                    /* enum openDiscType */

#define local /* static */

#define DSRDROP_TIMER 2      /* 20 ms */
#define DSRUP_TIMER 3        /* 30 ms */
#define CLOSE_TIMER 2400     /* 24s/output  */

typedef struct wt *wtp_t;
struct wt {
    struct openDisc      commonFields;
    int wto_event;			/* for sleep/wakeup */
    int wto_status;			/* current modem status lines */
    int wto_control;			/* current modem control lines */
    int wto_clstmr;	                /* close timer */
    int wto_dsrdrop;	                /* DSR drop -> DTR drop timer */
    int wto_dsrup;		        /* DSR deglitch timer */
    uint wto_output : 1;		/* on output path */
    uint wto_local : 1;			/* local DTR mode */
    uint wto_CDsleep : 1;		/* sleeping for CD */
    uint wto_close : 1;			/* sleeping for close */
    uint wto_dsrfalling : 1;		/* dtr timer is going */
    uint wto_dsrrising : 1;		/* dsr timer is going */
};

local int wto_open(caddr_t *retrieve, caddr_t driverStruct,
                    int (* ddservice) (), int status, int mode, int isFirst,
                    caddr_t eventAddr);
local int wto_close(caddr_t retrieve);
local int wto_input(caddr_t retrieve, caddr_t eventAddr, char receivedChar,
                     enum status receivedStatus);
static int wto_start(wtp_t wt);
static void wto_closeit(wtp_t wt);
static void wto_dsrfell(wtp_t wt);
static void wto_dsrrose(wtp_t wt);


local int
wto_open(caddr_t * getRetrieve, caddr_t driverStruct, int (* ddservice) (),
         int status, int mode, int isFirst, caddr_t eventAddr)
{
    register wtp_t current_wto = (wtp_t) (*getRetrieve);
    register int return_code = 0;
    register int old_intr;
    register int err = 0;

    if (!current_wto) {
        /* Space allocation */
        if (!(current_wto = (wtp_t) pinmalloc(sizeof(*current_wto)))) {
            *getRetrieve = NULL;
            return(ENOMEM);
        };
        /* structure initialisation */
        bzero(current_wto, sizeof(*current_wto));
        current_wto->commonFields.driverStruct = driverStruct;
        current_wto->commonFields.ddservice = ddservice;
        current_wto->wto_event = EVENT_NULL;
    }; /* End if (!current_wto) */
    
    switch (status) {
      case OPEN_LOCAL:
        current_wto->wto_local = 1;
        break;
        /* Decision not yet taken for this paramter interpretation */
      case OPEN_REMOTE:
        current_wto->wto_local = 0;
        break;
      default:
        pinfree(current_wto);
        *getRetrieve = NULL;
        return(EINVAL);
    }
	
    /* Output parameters updating */
    *getRetrieve = (caddr_t) current_wto;
    *(int *)(*getRetrieve) = 1;


    if (return_code = ddservice(driverStruct, TS_SCONTROL, (TSDTR|TSRTS))) {
        return(return_code);
    };

    old_intr = i_disable(INT_TTY);
    ddservice(driverStruct, TS_GSTATUS, &current_wto->wto_status);
    ddservice(driverStruct, TS_GCONTROL, &current_wto->wto_control);

    while (!(mode & DNDELAY) && !current_wto->wto_local &&
           !(current_wto->wto_status & TSCD)) {
        current_wto->wto_CDsleep = 1;
        if (OPEN_SLEEP(&current_wto->wto_event)) {
	    err = EINTR;
	    break;
        }
    }
    if (!err)
        err = wto_start(current_wto);
    i_enable(old_intr);

    return(err);
}

local int
wto_close(caddr_t retreive)
{
    int err = 0;
    int ospeed;
    wtp_t wt  = (wtp_t) retreive;
    int (* ddservice) () = wt->commonFields.ddservice;
    caddr_t driverStruct = wt->commonFields.driverStruct;

    ddservice(driverStruct, TS_GBAUD, &ospeed);

    if (ospeed) {
	wt->wto_close = 1;
        wt->wto_clstmr = timeout(wto_closeit, wt, CLOSE_TIMER/ospeed);
	while (wt->wto_close) {
            if (OPEN_SLEEP(&wt->wto_event)) {
		err = EINTR;
                break;
            }
        }    
        if ((err) && (wt->wto_clstmr)) {
            untimeout(wt->wto_clstmr);
        }
    }    

    if (wt->wto_dsrdrop) {
        untimeout(wt->wto_dsrdrop);
        wt->wto_dsrfalling = 0;
    }
    if (wt->wto_dsrup) {
        untimeout(wt->wto_dsrup);
        wt->wto_dsrrising = 0;
    }
    /* Free the wto structure. */
    pinfree(retreive);

    return(err);
}


/* do remote pacing */
local int 
wto_input(caddr_t retrieve, caddr_t eventAddr,
               char receivedChar, enum status s)
{

    register wtp_t wt = (wtp_t) retrieve;
    int err;
    int (* ddservice) () = wt->commonFields.ddservice;
    caddr_t driverStruct = wt->commonFields.driverStruct;

    switch (s) {
    case cts_on:
	wt->wto_status |= TSCTS;
	wto_start(wt);
	break;

    case cts_off:
	wt->wto_status &= ~TSCTS;
        ddservice(driverStruct, TS_PROC, T_SUSPEND);
	if (wt->wto_control & TSRTS) {
            ddservice(driverStruct, TS_SCONTROL, 0);
        }
	break;
    case dsr_on:
	if (wt->wto_dsrfalling) {
	    wt->wto_dsrfalling = 0;
	    untimeout(wt->wto_dsrdrop);
	    wt->wto_dsrdrop = 0;
	}
	if (!wt->wto_dsrrising) {
	    wt->wto_dsrrising = 1;
	    wt->wto_dsrup = timeout(wto_dsrrose, wt, DSRUP_TIMER);
	}
	return(0);
    case dsr_off:
	wt->wto_status &= ~TSDSR;
        ddservice(driverStruct, TS_PROC, T_SUSPEND);
	if (!wt->wto_dsrfalling) {
	    wt->wto_dsrfalling = 1;
	    wt->wto_dsrdrop = timeout(wto_dsrfell, wt, DSRDROP_TIMER);
	}
	if (wt->wto_dsrrising) {
	    wt->wto_dsrrising = 0;
	    untimeout(wt->wto_dsrup);
	    wt->wto_dsrup = 0;
	}
	return(0);
    case ri_on:
	wt->wto_status |= TSRI;
	/* ###6 */
	if (!(wt->wto_control & TSDTR)) {
	    /* Just in case -- what the 'ell-man */
            ddservice(driverStruct, TS_GCONTROL, &wt->wto_control);
	    wt->wto_control |= TSDTR;
            ddservice(driverStruct, TS_SCONTROL, wt->wto_control);
	}
	break;
    case ri_off:
	wt->wto_status &= ~TSRI;
	break;
    case cd_on:
	wt->wto_status |= TSCD;
	if (wt->wto_CDsleep) {
	    wt->wto_CDsleep = 0;
            OPEN_WAKEUP(&wt->wto_event);
	}
	break;
    case cd_off:
	wt->wto_status &= ~TSCD;
	break;
    }
    return(0);
}

/* 
 * If everything is o.k. we permit data output, otherwise we suspend it
 */
static int
wto_start(wtp_t wt)
{
    int (* ddservice) () = wt->commonFields.ddservice;
    caddr_t driverStruct = wt->commonFields.driverStruct;
    if ((wt->wto_status & (TSCTS|TSDSR)) == (TSCTS|TSDSR) &&
	(wt->wto_control & (TSRTS|TSDTR)) == (TSRTS|TSDTR)) {
        ddservice(driverStruct, TS_PROC, T_RESUME);
    } else
        ddservice(driverStruct, TS_PROC, T_SUSPEND);
    return(0);
}

/* Timer routine to wait 2 char times at close time. */
static void
wto_closeit(wtp_t wt)
{
    wt->wto_close = 0;
    wt->wto_clstmr = 0;
    OPEN_WAKEUP(&wt->wto_event);
    return;
}

/* Timer routine to drop DTR when DSR dropps for more than 20 msec. */
static void
wto_dsrfell(wtp_t wt)
{

    int (* ddservice) () = wt->commonFields.ddservice;
    caddr_t driverStruct = wt->commonFields.driverStruct;

    ddservice(driverStruct, TS_GCONTROL, &wt->wto_control);
    wt->wto_control &= ~TSDTR;
    ddservice(driverStruct, TS_SCONTROL, wt->wto_control);
    wt->wto_dsrdrop = 0;
    return;
}

/* Timer routine to raise DSR after 30msec of deglitch time */
static void
wto_dsrrose(wtp_t wt)
{
    wt->wto_status |= TSDSR;
    wt->wto_dsrup = 0;
    (void)wto_start(wt);
    return;
}

