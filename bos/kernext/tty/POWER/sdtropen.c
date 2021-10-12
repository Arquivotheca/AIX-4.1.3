#ifndef lint
static char sccsid[] = "@(#)04 1.7 src/bos/kernext/tty/POWER/sdtropen.c, sysxcommon, bos411, 9438C411a 9/19/94 16:56:25";
#endif
/*
 * COMPONENT_NAME: (sysxtty) Open disciplines for streams driver
 *
 * FUNCTIONS: dtro_open, dtro_close, dtro_input
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/device.h>                     /* for config stuff */
#include <sys/errno.h>                      /* error codes */
#include <sys/intr.h>                       /* interrupt handler stuff */
#include <sys/malloc.h>                     /* malloc services */
#include <sys/pin.h>                        /* pinning stuff */
#include <sys/sleep.h>                      /* EVENT_NULL */
#include <sys/str_tty.h>
#include "ttydrv.h"
#include "common_open.h"                    /* enum openDiscType */

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define local /* static */
typedef struct dtro * dtrop_t;

struct dtro {
    struct openDisc      commonFields;
    int                  dtro_lock;
};


/*
 * ============================================================================
 * Functions declarations
 * ============================================================================
 */
/* Common functions */

/* DTRO_OPEN open discipline functions */
local int dtro_open(caddr_t *retrieve, caddr_t driverStruct,
                    int (* ddservice) (), int status, int mode, int isFirst,
                    caddr_t eventAddr);
local int dtro_close(caddr_t retrieve, int hupcl);
local int dtro_input(caddr_t retrieve, caddr_t eventAddr, char receivedChar,
                     enum status receivedStatus);


/*
 * =============================================================================
 *                       DTRO_OPEN FUNCTIONS
 * =============================================================================
 * DTRO_OPEN functions are: dtro_open, dtro_close and dtro_input.
 *
 * In dtro_open function: Space allocation and pin are made. If successfull,
 * "retrieve" output parameter is updated.
 *
 * Return code: 0 if OK, errno error code otherwise.
 *
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       DTRO_OPEN
 * -----------------------------------------------------------------------------
 * dtropen discipline open.
 * This function looks for an existing dtro structure, allocates pin
 * memory if necessary and updates this structure.
 * Then, it raises DTR and RTS signals and waits (if not DNDELAY)
 * for DCD signal.
 *
 * Return code: 0 if OK, errno error code otherwise.
 *
 * -----------------------------------------------------------------------------
 */
int
dtro_open(caddr_t * getRetrieve, caddr_t driverStruct,
              int (* ddservice) (), int status, int mode, int isFirst, caddr_t eventAddr)
{
    register dtrop_t current_dtro = (dtrop_t) (* getRetrieve);
    register int return_code;
    register int old_pri;
    int signal_temp;

    /* Initialisation */
    return_code = 0;

    /* Space allocation */
    if (!current_dtro) {
        if (!(current_dtro = (dtrop_t) he_alloc(sizeof(*current_dtro),
                                                BPRI_MED))) {
            *getRetrieve = NULL;
            return(ENOMEM);
        };
        /* structure initialisation */
        bzero(current_dtro, sizeof(*current_dtro));
        current_dtro->commonFields.driverStruct = driverStruct;
        current_dtro->commonFields.ddservice = ddservice;
        
        current_dtro->dtro_lock = SIMPLE_LOCK_AVAIL;
        simple_lock_init(& current_dtro->dtro_lock);
    
        /* Output parameters updating */
        *getRetrieve = (caddr_t) current_dtro;
	*(int *)(*getRetrieve) = 0;
    }; /* End if (!current_dtro) */
	
    old_pri = disable_lock(1, &( current_dtro->dtro_lock));

    if (return_code = ddservice(driverStruct, TS_GCONTROL, &signal_temp)) {
        unlock_enable(old_pri, &(current_dtro->dtro_lock));
        return(return_code);
    };
    if (signal_temp != (signal_temp | (TSDTR|TSRTS))) {
        if (return_code = ddservice(driverStruct,
                                    TS_SCONTROL, signal_temp|TSDTR|TSRTS)) {
            unlock_enable(old_pri, &(current_dtro->dtro_lock));
            return(return_code);
        };
    }; /* End if (signal_temp != (signal_temp | (TSDTR|TSRTS))) */
    
    /* If the sleep is interrupted, the close will get called to clean */
    /* things up.  Also, the port can not be close when there is at */
    /* least one open pending so the port can be closed while we are */
    /* sleeping.  The only other possible bad thing is a wakeup from */
    /* someplace beside carrier detect but I can't imagin how that can */
    /* happen. */
    
    /* No need for while loop in sleeping, since we cannot exit
     * e_block_thread() */
    
    if ((status == OPEN_REMOTE) && !(mode & DNDELAY)) {
        /* priority level may be modified */
        if (!(return_code = ddservice(driverStruct, TS_GSTATUS,
                                      &signal_temp))) {
            if (!(signal_temp & TSCD)) {
                e_assert_wait(eventAddr, INTERRUPTIBLE);
                unlock_enable(old_pri, &(current_dtro->dtro_lock));
                /* Get the lock in both exit cases from e_block_thread
                   since will decrement inopen field.
                 */
                if (e_block_thread() != THREAD_AWAKENED) {
                    /* exit because of reciept of a signal */
                    old_pri = disable_lock(1, &( current_dtro->dtro_lock));
                    return_code = EINTR;
                }
                else {
                    old_pri = disable_lock(1, &( current_dtro->dtro_lock));
                    return_code = 0;
                };
            }; /* End if (signal_temp &TSCD) */
        } 
        else {
           return_code = EINTR;
        } /* End if (!(return_code = ddservice(...) */
    }; /* End if (!(mode & DNDELAY)) */
    
    unlock_enable(old_pri, &(current_dtro->dtro_lock));

    return(return_code);
/* 
 * WARNING! missing here: cleaning the dtro struct if it is the first
 * open and it fails.
 */
} /* End int dtro_open(...) */

/*
 * -----------------------------------------------------------------------------
 *                       DTRO_CLOSE
 * -----------------------------------------------------------------------------
 * dtropen discipline close.
 * This function drops DTR and RTS signals (if no current open in progress).
 *
 * Return code: 0 if OK, errno error code otherwise.
 *
 * -----------------------------------------------------------------------------
 */
int
dtro_close(caddr_t retrieve, int hupcl)
{
    register int return_code = 0;
    register dtrop_t current_dtro = NULL;

    /* Happens this really when no open in progress? */
    if (hupcl) {
        int signal_temp;
        if (!(return_code = ((openDiscp_t)retrieve)->ddservice(
                                       ((openDiscp_t)retrieve)->driverStruct,
                                       TS_GCONTROL, &signal_temp))) {
            signal_temp &= ~(TSDTR|TSRTS);
            return_code = ((openDiscp_t)retrieve)->ddservice(
                                       ((openDiscp_t)retrieve)->driverStruct,
                                       TS_SCONTROL, signal_temp);
        }; /* End if (!(return_code = ...)) */
    }; /* End if (hupcl) */
    
    /* Free the dtro structure. */
    he_free(retrieve);
    return(return_code);
} /* End int dtro_close(...) */

/*
 * -----------------------------------------------------------------------------
 *                       DTRO_INPUT
 * -----------------------------------------------------------------------------
 * dtropen discipline input.
 * This function wakes up sleeping open function only if open is in progress.
 *
 * Return code: Always 0.
 *
 * -----------------------------------------------------------------------------
 */
int dtro_input(caddr_t retrieve, caddr_t eventAddr,
               char receivedChar, enum status receivedStatus)
{
    int old_pri = disable_lock(1, &(((dtrop_t)retrieve)->dtro_lock));

    if (receivedStatus == cd_on) {
        OPEN_WAKEUP(eventAddr);
    };
    unlock_enable(old_pri, &(((dtrop_t)retrieve)->dtro_lock));
    return(0);
} /* End int dtro_input(...) */

