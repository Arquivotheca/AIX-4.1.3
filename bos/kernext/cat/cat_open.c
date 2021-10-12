static char sccsid[] = "@(#)54  1.20  src/bos/kernext/cat/cat_open.c, sysxcat, bos411, 9434A411a 8/18/94 15:59:49";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catopen()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define FNUM 7

#include <sys/dma.h>
#include <sys/device.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/comio.h>
#include <sys/malloc.h>
#include <sys/devinfo.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <errno.h>
#include <sys/except.h>

#include "catdd.h"

void free_open_struct();


/*****************************************************************************
** NAME:        catopen
**
** FUNCTION:
**              Saves the open mode.
**
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              device number, read/write/kernel mode, multiplex channel
**              number, extension structure (kernel users only)
**    Output:
**              success/fail code
**    Called From:
**              system call handler via open()
**    Calls:
**              lockl() unlockl() catget()
**
** RETURNS:     0 - Success
**              EINTR - failed due to signal while trying to get global lock
**              ENODEV - no such adapter
**              EINTR - failed due to signal while trying to get write lock
**
*****************************************************************************/
int
catopen(
        dev_t dev,              /* major/minor device number */
        ulong mode,             /* defined in <sys/device.h> */
        chan_t chan,            /* channel number (index into open list) */
        struct kopen_ext *ext)  /* extension value for DKERNEL users */
{
        struct ca *ca;
        open_t *openp;
        recv_elem_t *recvp;
        stat_elem_t *statp;
        int i;
        int rc = 0;

        DDHKWD4(HKWD_DD_CATDD,DD_ENTRY_OPEN,0,dev,mode,chan,ext);
        F_TRACE(1);

        if ((chan < 0)
         || (chan > CAT_MAX_OPENS)
         || ((ca = catget(minor(dev))) == NULL)
         || (((openp = &ca->open_lst[chan])->op_flags & OP_OPENED) == 0))
                rc = ENXIO;
        else if (lockl(&ca->adap_lock, LOCK_SIGRET) != LOCK_SUCC) {
                rc = EINTR;
        } else {
                openp->op_rcv_event = EVENT_NULL;
                openp->op_chan = chan;

                /*
                ** Allocate free status queue elements
                ** The status elements form a doubly-linked ring with
                ** a NULL next pointer terminating the ring.
                */
                if ((statp = KMALLOC(stat_elem_t)) == NULL) {
                        free_open_struct(ca, openp);
                        unlockl(&ca->adap_lock);
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_OPEN, ENOMEM, dev);
                        cat_logerr(ca, ERRID_CAT_ERR4);
                        return ENOMEM;
                }
                bzero(statp, sizeof(stat_elem_t));
                statp->stat_last = statp;
                openp->stat_free = statp;
                i = ca->caddp.config_params.recvno * 2;
                while (--i > 0) {
                        if ((statp = KMALLOC(stat_elem_t)) == NULL) {
                                free_open_struct(ca, openp);
                                unlockl(&ca->adap_lock);
                                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_OPEN, ENOMEM, dev);
                                cat_logerr(ca, ERRID_CAT_ERR4);
                                return ENOMEM;
                        }
                        bzero(statp, sizeof(stat_elem_t));
                        statp->stat_last = openp->stat_free->stat_last;
                        statp->stat_last->stat_next = statp;
                        openp->stat_free->stat_last = statp;
                }

                /*
                ** Save the open mode
                */
                openp->op_mode = mode;

                /*
                ** Save the extension parameters
                */
                if( ext != NULL ) {
                        openp->op_open_id = ext->open_id;
                        openp->op_rcv_fn = ext->rx_fn;
                        openp->op_xmit_fn = ext->tx_fn;
                        openp->op_stat_fn = ext->stat_fn;
                }
                unlockl(&ca->adap_lock);
        }
        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_OPEN, rc, dev);
        return rc;
} /* catopen() */


/*****************************************************************************
** NAME:        free_open_struct
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:     0 - Success
**
*****************************************************************************/
void
free_open_struct(
        struct ca *ca,
        open_t *openp)
{
        recv_elem_t *recvp;
	   recv_elem_t *next_recvp;    /*d50453*/
        stat_elem_t *statp;
        int subchannel;
        int linkid;
        int spl;
        /*
        ** Free all of the active receive elements
        */
/* d50454 new start */
		recvp = openp->recv_act;   /* start with first on active list */

		while (recvp)  {             /*d50453*/
			next_recvp = recvp->rc_next;  /* get next on active list */
			free_recv_element(ca, recvp);
			recvp = next_recvp;
		}
/* d50454 new complete */

/* DISABLE_INTERRUPTS(spl); */
	 	LOCK(ca->lcklvl);
        /*
        ** Free all of the active status elements
        */
        while (statp = openp->stat_act) {
                openp->stat_act = statp->stat_next;
                KFREE(statp);
        }

        /*
        ** Now free all of the free status elements
        */
        while (statp = openp->stat_free) {
                openp->stat_free = statp->stat_next;
                KFREE(statp);
        }

        /* ** If this was the last open for this adapter
        ** de-allocate the data structures.
        */
        if (--ca->num_opens == 0) {
                UNLOCK(ca->lcklvl);
                cat_term_dev(ca);
                LOCK(ca->lcklvl);
        }

        bzero(openp, sizeof(open_t));

	 	UNLOCK(ca->lcklvl);
/* ENABLE_INTERRUPTS(spl); */
} /* free_open_struct() */
