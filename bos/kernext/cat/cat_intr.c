static char sccsid[] = "@(#)51  1.82  src/bos/kernext/cat/cat_intr.c, sysxcat, bos41J, 9512A_all 1/27/95 09:38:56";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS:   cat_offlvl(), clean_queue(), catintr(), ginny(),
 *              resource_timeout(), handle_timer(), pscaclreq(), pscaclresp(),
 *              pscaclcon(), pscacldisc(), pscasysval(), pscaxbuf(), pscadmac(),
 *              pscaack(), pscaerr(), psca_other(), pscarsts(), pscarsta(),
 *              pscaabrt(), pscaxflu(), pscabufav(), pscaprnt(), ack_sets(),
 *              ack_strt(), ack_stop(), ack_xbuf(), err_xlst(),
 *              err_sets_strt_stop() err_other()
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
#define FNUM 4

#include <net/spl.h>
#include <sys/adspace.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/sleep.h>
#include <sys/dma.h>
#include <sys/time.h>
#include <sys/except.h>
#include <sys/poll.h>
#include <sys/errids.h>

#include "catdd.h"
#include "catproto.h"

/* Function prototypes */
static void pscaclreq(struct ca* ca, cmd_t *notify);
static void pscaclresp(struct ca *ca, cmd_t *notify);
static void pscaclcon(struct ca *ca, cmd_t *notify);
static void pscacldisc(struct ca *ca, cmd_t *notify);
static void pscasysval(struct ca *ca, cmd_t *notify);
static void pscaxbuf(struct ca *ca, cmd_t *notify);
static void pscadmac(struct ca *ca, cmd_t *notify);
static void pscaack(struct ca *ca, cmd_t *notify);
static void pscaerr(struct ca *ca, cmd_t *notify);
static void psca_other(struct ca *ca, cmd_t *notify);
static void pscarsts(struct ca *ca, cmd_t *notify);
static void pscarsta(struct ca *ca, cmd_t *notify);
static void pscaabrt(struct ca *ca, cmd_t *notify);
static void pscaxflu(struct ca *ca, cmd_t *notify);
static void pscabufav(struct ca *ca, cmd_t *notify);
static void pscaprnt(struct ca *ca, cmd_t *notify);
static void ack_sets(struct ca *ca, cmd_t *notify);
static void ack_strt(struct ca *ca, cmd_t *notify);
void ack_stop(struct ca *ca, cmd_t *notify);
static void ack_xbuf(struct ca *ca, cmd_t *notify);
static void err_xlst(struct ca *ca, cmd_t *notify);
static void err_sets_strt_stop(struct ca *ca, cmd_t *notify);
static void err_other(struct ca *ca, cmd_t *notify);

#define min(a,b)        (((a) > (b)) ? (b) : (a))
/*d51658 change next line, reverse correl and ccw */
#define NOTIFY_CCW (sc->specmode & CAT_CLAW_MOD ? notify->correl : notify->ccw)
#define LINK_ID (sc->specmode & CAT_CLAW_MOD ? notify->ccw : 0)

int resource_retries;

/*****************************************************************************
** NAME:        handle_resource_timeout
**
** FUNCTION:    Try processing a queued recv request or abort.
**
** EXECUTION ENVIRONMENT:
**              interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
**
******************************************************************************/
void handle_resource_timeout(struct ca *ca)
{
        cmd_t cmd;
        int i;

        if (ca->num_unsolicited == 0 && resource_retries++ == 10) {
                /*
                ** After max retries, give up by sending unsolicited
                ** status of unit check with intervention req'd status
                ** to each subchannel and when the notification ack's
                ** are rec'd for every SC, shutdown the adapter.
                */
                uchar sense_byte = 0x40;        /* Intervention Req'd */

                bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                cmd.command = PSCAUNST; /* Send unsolicited status */
                cmd.cmdmod = MODACK;
                cmd.data[0] = 0xE;      /* Unit Check + CE + DE */
                cmd.length = 1;
                if (cat_get_cfb(ca, &cmd.buffer)) {
                        CATDEBUG(("shutting down---cat_get_cfb() failed\n"));
                        cat_shutdown(ca);
                }
                cat_write_buf(ca, cmd.buffer, &sense_byte, 1);

                /*
                ** Send the unsolicited status to
                ** every online subchannel.
                */
                for (i=0; i<CAT_MAX_SC; i++) {
                        if (ca->sc[i]) {
                                ca->num_unsolicited++;
                                cmd.subchan = i;
                                cat_put_cmd(ca, &cmd);
                                /*i += (sc->sc_subset - 1);*/
                        }
                }
        }
        ca->flags &= ~CAT_TIMER_TRIG;
        ca->flags &= ~CAT_TIMER_ON;

        return;
} /* handle_resource_timeout() */

/*****************************************************************************
** NAME: cat_offlvl
**
** FUNCTION: off level interrupt handler.
**
** EXECUTION ENVIRONMENT:
**              interrupt only
**
**
** NOTES:
**
**    Input:
**              pointer to dds structure
**              pointer to offlevel element structure
**    Output:
**              none
**
**    Called From: cat_intr, transmit_mgr, when que_offlvl is called
**
**    Calls To:
**              CAT_WRITE1, BUSMEM_DET, clean_queue, cat_get_stat, transmit_mgr,
**
** RETURN:
**
**
******************************************************************************/
int
cat_offlvl(
struct intr *ofl_ptr)
{
        unsigned char reg;
        ulong bus;
        struct ca *ca;
        void handle_timer();

        ca = (struct ca *)(--ofl_ptr);
		ca->in_kernel = 1; /*  flag to prevent locks */

        if (ca->flags & CAT_TIMER_TRIG) {
                handle_resource_timeout(ca);
        }

        /* Handle the watchdog timer. */
        if( ca->flags & CATWDTACT ) {
                handle_timer(ca);
                ca->flags &= ~CATWDTACT;
        }

        /* Handle the new additions to the notification queue. */
        clean_queue(ca);

		ca->in_kernel = 0; /*  flag to prevent locks */

        return 0;
} /* cat_offlvl() */


/*****************************************************************************
** NAME: clean_queue
**
** FUNCTION:
**      Read all entries from the card's status fifo.
**      Handle incoming data, finished xmit's, dma complete's
**      and any unrecognized error notification.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
**    Input: pointer to dds structure
**
**    Output: none
**
**    Called From: cat_offlvl
**
**    Calls To: cat_get_stat, cat_put_rfb, cat_read_buf,
**              bcopy, async_status,
**              e_wakeup, notify_all_opens, clean_sc_queues,
**
** RETURNS: 0 : SUCCESS
**
*****************************************************************************/
void
clean_queue(struct ca *ca)
{
        cmd_t notify;                   /* adapter acknowledgement buffer */
        union {
                cmd_t n_elem;
                struct {
                        unsigned int d1;
                        unsigned int d2;
                        unsigned int d3;
                        unsigned int d4;
                } t_elem;
        } trcun;

if ( ca->locking_adap_lock != LOCK_AVAIL ) return;

CATDEBUG(("clean_queue()\n"));
        /*
        ** While we have resources and there
        ** notifications from the microcode to
        ** handle...
        */
        while(!(ca->flags & CAT_PAUSED)) {
                if (ca->flags & CAT_SAVED_NOTFY) {
                        /*
                        ** Handle the saved notification first.
                        */
                        bcopy(&ca->saved_notify, &notify, sizeof(CTLFIFO)); /* d51376*/
                        ca->flags &= ~CAT_SAVED_NOTFY;
/*d50453*/              bzero(&ca->saved_notify,sizeof(CTLFIFO));
                } else {
                        /*
                        ** Get the next notification (if any) from the ucode.
                        */
                        if (cat_get_stat(ca, &notify)) {
                                return;
                        }
                        /*
                        ** Saved notification (if any) has been handled.
                        */
                        ca->flags &= ~CAT_SAVED_NOTFY;
                        resource_retries = 0;
                }
                /*
                ** Trace the notification element.
                */
                trcun.n_elem = notify;
                DDHKWD5(HKWD_DD_CATDD, PSCA_INTR, 0, ca->dev,
                        trcun.t_elem.d1, trcun.t_elem.d2,
                        trcun.t_elem.d3, trcun.t_elem.d4);

                switch (notify.command) {
                        case PSCACLREQ: /* claw request from the host */
                                pscaclreq(ca, &notify);
                                break;
                        case PSCACLRESP:/* claw response from the host */
                                pscaclresp(ca, &notify);
                                break;
                        case PSCACLCON: /* receive a confirm from the host */
                                pscaclcon(ca, &notify);
                                break;
                        case PSCACLDISC: /* reject from the host */
                                pscacldisc(ca, &notify);
                                break;
                        case PSCASYSVAL: /* claw system validate */
                                pscasysval(ca, &notify);
                                break;
                        case PSCAXBUF:  /* Received data */
                                pscaxbuf(ca, &notify);
                                break;
                        case PSCADMAC:  /* dma transfer completed */
                                pscadmac(ca, &notify);
                                break;
                        case PSCAACK:   /* command completion */
                                pscaack(ca, &notify);
                                break;
                        case PSCAERR:   /* Error detected in command */
                                pscaerr(ca, &notify);
                                break;
                        case PSCARSTS:  /* selective reset received */
                                pscarsts(ca, &notify);
                                break;
                        case PSCARSTA:  /* system reset received */
                                pscarsta(ca, &notify);
                                break;
                         case PSCAABRT: /* microcode aborted */
                                pscaabrt(ca, &notify);
                                break;
                        case PSCAXFLU:  /* flush and cancel current transfer */
                                pscaxflu(ca, &notify);
                                break;
                        case PSCABUFAV: /* send buffer is available */
                                pscabufav(ca, &notify);
                                break;
                        case PSCAPRNT:  /* Debug data available */
                        case PSCACTCRNR:/* CTC read/not ready transition */
                        case PSCABASIC: /* basic mode set */
                        case PSCAEXT:   /* extended mode set */
                        case PSCAVTAM:  /* VTAM header mode started/stopped */
                        case PSCACMD:   /* command received from 370 */
                        case PSCAWEOF:  /* write EOF received from 370 */
                        case PSCAOFFLN: /* adapter now offline to 370 */
                        case PSCAONLN:  /* adapter now offline to 370 */
                        default:
                                psca_other(ca, &notify);
                                break;
                }
                if (ca->flags & CAT_SAVED_NOTFY) {
                        return;
                }
        }

        return;
} /* clean_queue() */


/*****************************************************************************
** NAME:  cat_intr
**
** FUNCTION: second level interrupt handler.
**              validates interrupts and schedules offlevel interrupt handler
**
** EXECUTION ENVIRONMENT:
**              interrupt
**
** NOTES:
**
**    Input:  pointer to dds structure
**
**    Output:
**      INTR_SUCC       The interrupt was accepted and processed.
**      INTR_FAIL       The interrupt was not accepted.
**
**
**    Called From:  kernel
**
**    Calls To: CAT_READ1, BUSMEM_DET, cio_que_offl, CAT_WRITE1,
**
** RETURN:  0 = Good Return
**
**
*****************************************************************************/
int
catintr(struct ca *ca)
{
        ulong   status;
        uchar   int_enable;
        int     rc;
        ulong   bus;

        DDHKWD2(HKWD_DD_CATDD,DD_ENTRY_INTR,0,ca->dev,1);
        bus = CAT_MEM_ATT;
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus, XBUFSTAT, &status, sizeof(status));
        BUSMEM_DET(bus);
        if( ca->piorc ) {
                /*
                ** If this happens, the adapter has been shutdown.
                */
                i_reset(&ca->caih_struct);      /* Reset bus intr level */
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return(INTR_FAIL);
        }
        letni32(&status);
        /*
        ** verify the adapter generated this interrupt
        */
        if ((status & IRQACTIV) == 0) {
                ca->stats.ds.total_intr++;
                if (ca->flags & CATFUNC)
                        /*
                        ** When running functional ucode, an interrupt means
                        ** there's something in the fifo for me.
                        ** Add this interrupt to off-level que if there's
                        ** room.  How reassuring---i_sched() is documented
                        ** as queueing the interrupt "IF THERE IS ROOM".
                        ** It is a void function!!!
                        */
                        i_sched((struct intr *)&(ca->ofl));
                else
                        ca->stats.ds.intr_not_handled++;

                /*
                ** re-enable adapter interrupts
                */
                status = 0;
                bus = CAT_MEM_ATT;
                /* CAT_WRITE sets ca->piorc */
                CAT_WRITE(bus, RSTUCIRQ, &status, sizeof(status));
                if( ca->piorc ) {
                        i_reset(&ca->caih_struct);
                        BUSMEM_DET(bus);
                        cat_shutdown( ca );
                        ca->flags |= CATDEAD;
                        return(INTR_FAIL);
                }
                /*
                ** Re-enable generation of MCI interrupts
                */
                int_enable = 1;
                /* CAT_WRITE sets ca->piorc */
                CAT_WRITE1(bus, INTMCI, &int_enable);
                if (ca->piorc  == 0) {
                        rc = INTR_SUCC;
                }
                BUSMEM_DET(bus);
        } else {
                rc = INTR_FAIL;
        }
        i_reset(&ca->caih_struct);              /* Reset bus intr level */
        if (ca->piorc) {
                cat_shutdown(ca);
                ca->flags |= CATDEAD;
        }
        DDHKWD2(HKWD_DD_CATDD,DD_EXIT_INTR,rc,ca->dev,1);
        return (rc);
} /* catintr() */

/*****************************************************************************
** NAME: ginny
**
** FUNCTION: Watchdog timer routine
**
** EXECUTION ENVIRONMENT:
**              interrupt only
**
** NOTES:
**
**    Input:
**              pointer to offlevel element structure
**    Output:
**              nothing (void)
**    Called From:
**              "kernel" when timer runs out
**    Calls To:
**              i_sched(), w_stop()
** RETURNS: nothing (void)
**
******************************************************************************/
void
ginny( struct intr *wds_ptr )
{
        struct ca *ca;

        /*
        ** stop the timer, get a pointer to
        ** the adapter structure, indicate
        ** that a timer interrupt occurred
        ** and schedule an offlevel interrupt
        ** to handle it if necessary.
        */
        w_stop((struct watchdog *)wds_ptr);
        ca = (struct ca *)&wds_ptr[-2];
        ca->flags |= CATWDTACT;
        i_sched(&(ca->ofl));
} /* ginny() */

/*****************************************************************************
** NAME: resource_timeout
**
** FUNCTION:    Schedule an off-level interrupt to the PCA so a new
**              attempt to allocate an mbuf can be made.
**
** EXECUTION ENVIRONMENT:
**              interrupt only
**
** NOTES:
**
**    Input:
**              pointer to timer request block
**    Output:
**              none (void)
**    Called From:
**              "kernel" when timer runs out
**    Calls To:
**              i_sched()
**
** RETURNS:  nothing (void)
**
******************************************************************************/
void
resource_timeout( struct trb *trub )
{
        struct ca *ca;

        /*
        ** get a pointer to the adapter structure
        ** and schedule an offlevel interrupt
        ** to retry the first element of the status queue.
        */
        ca = (struct ca *)trub->t_func_addr;
        ca->flags |= CAT_TIMER_TRIG;
        i_sched(&(ca->ofl));
} /* resource_timeout() */

/*****************************************************************************
** NAME: handle_timer
**
** FUNCTION: Routine to handle watchdog timeouts.
**
** EXECUTION ENVIRONMENT:
**              interrupt only (off-level)
**
** NOTES:
**      Zero the timeout value of the first elements
**      value, then call the function specified to
**      handle the event with the argument pointer
**      provided.  Do this for all elements following
**      this one that have the same expiration time.
**      Finally, restart the watchdog if there are any
**      elements remaining.
**
**    Input:
**              pointer to dds structure
**    Output:
**              none (void)
**    Called From:
**              cat_offlvl()
**    Calls To:
**              w_start(), timer functions...
**
** RETURNS: nothing (void)
**
**
******************************************************************************/
void
handle_timer( struct ca *ca )
{
        timeout_t *tp;

        if( tp = ca->callout ) {
                while( (ca->flags & CATDEAD) || (tp->t_val - lbolt) < HZ ) {
                        ca->callout = tp->t_next;
                        (*tp->t_func)(ca, tp->t_arg);
                        if( (tp = ca->callout) == NULL )
                                break;
                }
        }
        if( ca->callout != NULL && !(ca->flags & CATDEAD) ) {
                ca->watch.restart = (ca->callout->t_val - lbolt) / HZ;
                w_start(&ca->watch);
        }
} /* handle_timer() */


/*****************************************************************************
** NAME:        save_notify
**
** FUNCTION:    Save the given notification for processing when the
**              necessary system resources become available.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
void save_notify(
        struct ca *ca,
        cmd_t *notify)
{
#ifdef DEBUG
cat_gen_trace("SNOT",0,0,0);
#endif
        ca->flags |= CAT_SAVED_NOTFY;
        bcopy(notify, &(ca->saved_notify), sizeof(CTLFIFO)); /* d51376*/

        if (!(ca->flags & CAT_TIMER_ON)) {
                ca->flags |= CAT_TIMER_ON;
                ca->resource_timer->timeout.it_value.tv_sec =
                        (MBUF_MS_WAIT / 1000);
                ca->resource_timer->timeout.it_value.tv_nsec =
                        (MBUF_MS_WAIT % 1000) * 1000000;
                ca->resource_timer->t_func_addr =(caddr_t)ca;
                tstart(ca->resource_timer);
        }
        return;
} /* save_notify() */


/*****************************************************************************
** NAME:        pscaclreq
**
** FUNCTION:    Handle a CLAW Connection Request.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscaclreq(
        struct ca *ca,
        cmd_t *notify)
{
        cmd_t cmd;
        char buf[3 * LK_NAME_LEN];
        sc_t *sc;
        link_t *link;
        int i;
	   open_t *openp;  /* ix29279  */

        sc = ca->sc[notify->subchan];
        ASSERT(sc);

        /*
        ** Read the application names
        */
        bzero(buf, 3 * LK_NAME_LEN);
        cat_read_buf(ca, notify->buffer, buf, 3 * LK_NAME_LEN);
        cat_ret_buf(ca, notify->buffer, CFB_TYPE);

        /*
        ** If a link structure with matching application names
        ** is found and isn't already 'confirmed' or 'responsed',
        ** confirm it, else return a disconnect response.
         */
        bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
        cmd.subchan = notify->subchan;
        cmd.ccw = notify->ccw;

        cmd.command = PSCACLDISC;
        for (i=0; i<MAX_LINKS; i++) {
                if ((link = ca->sc[notify->subchan]->links[i])
                && bcmp(buf, link->lk_WS_appl, LK_NAME_LEN) == 0
                && bcmp(buf+8, link->lk_H_appl, LK_NAME_LEN) == 0) {
                        if (link->lk_state != LK_FIRM
                        && link->lk_state != LK_RESP) {
                                cmd.command = PSCACLCON;
                                cat_get_cfb(ca, &(cmd.buffer));
                                cat_write_buf(ca,cmd.buffer,buf,3*LK_NAME_LEN);
                                cmd.length = 3 * LK_NAME_LEN;
                                link->lk_state = LK_FIRM;
                                link->lk_actual_id = notify->ccw;
                                sc->link[notify->ccw] = link;

						  /* ix29279
							get pointer to the open structure and 
							set the opened flag to true indicating
							that this open has links or subchannels which
							have been started
						   */
					       openp = link->lk_open;  /* ix29279 */
						  openp->op_sc_opened = 1; /* ix29279 */

                                if (link->lk_appl_id == 0) {
                                        sc->num_links++;
                                        link->lk_appl_id = notify->ccw;
                                        async_status(ca, link->lk_open,
                                                CIO_START_DONE, CIO_OK,
                                                notify->subchan, notify->ccw, 0);
                                } 
						  else {    /* 78660 if layer new  */
							    async_status(ca,link->lk_open,
										  CIO_START_DONE, CIO_OK,
										  notify->subchan,
										  link->lk_appl_id,0);
						  }

                        }
                        break;
                }
        }

        cat_put_cmd(ca, &cmd);
        return;
} /* pscaclreq() */


/*****************************************************************************
** NAME: pscaclresp
**
** FUNCTION: Handle a CLAW connection response.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void 
pscaclresp(
        struct ca *ca,
        cmd_t *notify)
{
        cmd_t cmd;
        link_t *linkp;
        subchannel_t *sc;                        /* d51658 */
        int i;

        ASSERT((sc=ca->sc[notify->subchan]));    /* d51658 */
        linkp = sc->links[notify->correl];       /* d51658 */


        /* d51658 comment
        ** check the return code, report if bad, and return.
        ** This resp does not return any recoverable error codes.
        */
/*d51658*/ if(notify->retcode) {
/*d51658*/    async_status(ca, linkp->lk_open, CIO_ASYNC_STATUS,
/*d51658*/    notify->command, notify->subchan, notify->ccw, notify->retcode);
/*d51658*/    drop_link(sc, linkp);
/*d51658*/    return;
/*d51658*/ }

		 
        /* d51658 comment added
        ** process sent a req, now getting a response from PCA
        ** NOTE: In order to guarantee the risc applications remain
        ** in sync with the host applications when the host recycles,
        ** each (WS_appl name H_appl) pair must be unique.
        **/
        for (i=0; i<MAX_LINKS; i++) {
/*d51658*/       if ((linkp = sc->links[i])    /* added the sc-> */
                && linkp->lk_state != LK_FIRM
                && linkp->lk_correl == notify->correl) {
					if (linkp->lk_state == LK_CLOSED ) {
						bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
						cmd.subchan = notify->subchan;
						cmd.ccw = notify->ccw;
						cmd.command = PSCACLDISC;
						cat_put_cmd(ca, &cmd);

						linkp->lk_state = LK_DELETE ;

					} else {

                        linkp->lk_state = LK_RESP;
                        linkp->lk_actual_id = notify->ccw;
/*d51658*/               sc->link[notify->ccw] = linkp;  /* added the sc->*/
        				linkp->lk_open->op_sc_opened = 1; /* ix40224 */
					}
                        break;
                }
        }

        if (i == MAX_LINKS) {

                /*
                ** Couldn't find a matching link, send disconnect
                */
                bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                cmd.subchan = notify->subchan;
                cmd.ccw = notify->ccw;
                cmd.command = PSCACLDISC;
                cat_put_cmd(ca, &cmd);
        }
        return;
} /* pscaclresp() */

/*****************************************************************************
** NAME: pscaclcon
**
** FUNCTION:    Handle a CLAW connection confirm.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscaclcon(
        struct ca *ca,
        cmd_t *notify)
{
        cmd_t cmd;
        char buf[3 * LK_NAME_LEN];
        link_t *link;
        sc_t *sc;

        /*
        ** Validate SC and link
        */
        ASSERT(ca->sc[notify->subchan]);
        ASSERT(notify->ccw < MAX_LINKS);

        sc = ca->sc[notify->subchan];

        /*
        ** Read WS and host application names
        */
        cat_read_buf(ca, notify->buffer, buf, 3 * LK_NAME_LEN);
        cat_ret_buf(ca, notify->buffer, CFB_TYPE);

        /*
        ** Should already have a valid link if not return.
        */
        if ((link = sc->link[notify->ccw]) == (link_t *)NULL ) {  /*d51658*/
              return;                                             /*d51658*/
        }                                                         /*d51658*/
        if( bcmp(buf, link->lk_WS_appl, LK_NAME_LEN)
            || bcmp(buf+8, link->lk_H_appl, LK_NAME_LEN)) {
           /* d51658 modified comment
           ** Application names did not match get out.
           */
                bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                cmd.subchan = notify->subchan;
                cmd.ccw = notify->ccw;
                cmd.command = PSCACLDISC;
                cat_put_cmd(ca, &cmd);
           /*  d51658 modified comment.
           ** Do have a valid openp... can give status
           */
            async_status(ca, link->lk_open, CIO_START_DONE,
                 CIO_NOT_STARTED, notify->subchan, notify->ccw, RETCSYSN);

        } else {
                link->lk_open->op_sc_opened = 1; /* ix40224 */ 

                link->lk_state = LK_FIRM;
                if (link->lk_appl_id == 0) {
                        sc->num_links++;
                        link->lk_appl_id = notify->ccw;
                        async_status(ca, link->lk_open, CIO_START_DONE, CIO_OK,
                                notify->subchan, notify->ccw, 0);
                }
                else {    /* 78660 if layer new  */
                        async_status(ca,link->lk_open,
                                    CIO_START_DONE, CIO_OK,
                                    notify->subchan,
                                    link->lk_appl_id,0);
               }

        }
        return;
} /* pscaclcon() */


/*****************************************************************************
** NAME: pscacldisc
**
** FUNCTION:    Handle a CLAW connection disconnect.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscacldisc(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        link_t *link;

        if((sc = ca->sc[notify->subchan]) == (subchannel_t *)0
        || sc->links == 0
        || (link = sc->link[notify->ccw]) == 0) {
                return;
        }

        /*   d51658 added to comment
        ** There isn't any way we can guarantee
        ** that the same link id will get allocated
        ** for this link if and when the host comes
        ** back up, so the  'lk_actual_id' and 'lk_appl_id' link fields
        ** will be used to distinguish such a discrepancy.
        ** We do not 0 out the link[] array here, it will be done
        ** in pscasysval() then we are sure there will be no more
        ** acknowledgements for a link.
        */

/* Send asynchronous notification to the application that the link
   was disconnected 
   78660 new iflayer test
*/
	if ( link->lk_state == LK_FIRM)    /* send status for confirmed links */
       async_status(ca, link->lk_open, CIO_ASYNC_STATUS,
              notify->command, notify->subchan, 0, notify->retcode);
	
    /*    sc->link[notify->ccw] = (link_t *)0;   d51658 delete */

    link->lk_state = LK_DISC;

        return;
} /* pscacldisc() */


/*****************************************************************************
** NAME: pscasysval
**
** FUNCTION:    Handle a CLAW system validate report.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscasysval(
        struct ca *ca,
        cmd_t *notify)
{
        cmd_t cmd;
        char buf[3 * LK_NAME_LEN];
        subchannel_t *sc;
        link_t *link;
        int i;
        int adapt_len,host_len; /* d51377 */


        /*
        ** MUST have valid SC and link already
        */
        sc = ca->sc[notify->subchan];
        ASSERT(sc);

        for (i=0; i<MAX_LINKS; i++) {
                if (link = sc->links[i]) {
                        break;
                }
        }
        ASSERT(link);

        /*  *d49491*
        ** Just return if bad, after returning the cntl buffer.
        */
        if (notify->retcode) {
/*d49491*/    cat_ret_buf(ca, notify->buffer, CFB_TYPE);
/*d51658*/       async_status(ca, link->lk_open, CIO_ASYNC_STATUS,
/*d51658*/              notify->command, notify->subchan, 0, notify->retcode);
                return;
        }

        /*
        ** The microcode should already validate the host
        ** and workstation names, but just to be sure...
        */
        cat_read_buf(ca, notify->buffer, buf, 3 * LK_NAME_LEN);
        cat_ret_buf(ca, notify->buffer, CFB_TYPE);
/* added d51377 */
           adapt_len = ((strlen(ca->caddp.adapter_name) > 8) ? 8 : strlen(ca->caddp.adapter_name));
           host_len = ((strlen(ca->caddp.host_name) > 8) ? 8 : strlen(ca->caddp.host_name));
        if (bcmp(buf, ca->caddp.adapter_name, adapt_len)
                || bcmp(buf+8, ca->caddp.host_name, host_len)) {
                /*
                ** Alert the application, unless this is the case
                ** where the host went down and is coming back up.
                */
                if (link->lk_state != LK_DISC) {
                        async_status(ca, link->lk_open, CIO_START_DONE,
                        CIO_NOT_STARTED, notify->subchan, 0, RETCSYSN); /* d51377*/
                } 
				     
                return;
        }
/*d51658*/ if(sc->sc_state != SC_OPEN && sc->sc_state != SC_STARTING) {
             /* d51658 comment added
             ** This is an error in sub state, report it and free it
             */
/*d51658*/     async_status(ca, link->lk_open, CIO_START_DONE,
/*d51658*/           CIO_NOT_STARTED, notify->subchan, 0,0);
/*d51658*/     return;
/*d51658*/ }

        sc->sc_state = SC_OPEN;
             /*  d51658 comment
             **  clean the link[] array here instead of when
             **  disconnect is received. This was if any notifications
             **  come in after the disc cat_intr.c can handle them.
             */
        bzero(&cmd, sizeof(CTLFIFO));                      /*d51658*/
        for (i=0; i<MAX_LINKS; i++) {                      /*d51658*/
          if(sc->link[i]) {                                /*d51658*/
              /* d51658 comment
              ** There are times if host app messes up that
              ** it will be necessary to make sure all links
              ** are disconnected.
              */
		  if ( sc->link[i]->lk_state != LK_DISC){ /* 72359 */
              cmd.subchan = notify->subchan;               /*d51658*/
              cmd.ccw = i;                                 /*d51658*/
              cmd.command = PSCACLDISC;                    /*d51658*/
              cat_put_cmd(ca, &cmd);                       /*d51658*/
              sc->link[i]->lk_state = LK_DISC;             /*d51658*/
		  } /* 72359 */
              sc->link[i]= (link_t *)0;                    /*d51658*/
          }                                                /*d51658*/
        }                                                  /*d51658*/

        /*
        ** write the host application and workstation
        ** application names to the send buffer
        */
        bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */

        /*
        ** Send a connect request to get the link id
        ** for every link.
        */
        for (i=0; i<MAX_LINKS; i++) {
                link = sc->links[i];
                if (link) {
                        link->lk_state = LK_REQ;
                        cat_get_cfb(ca, &(cmd.buffer));
                        bcopy(link->lk_WS_appl, buf, LK_NAME_LEN);
                        bcopy(link->lk_H_appl, buf+LK_NAME_LEN, LK_NAME_LEN);
                        cat_write_buf(ca, cmd.buffer, buf, 2 * LK_NAME_LEN);
                        cmd.command = PSCACLREQ;
                        cmd.subchan = notify->subchan;
                        cmd.correl = link->lk_correl;
                        cmd.length = 3 * LK_NAME_LEN;
                        cat_put_cmd(ca, &cmd);
                }
        }
        return;
} /* pscasysval() */



/*****************************************************************************
** NAME: pscaxbuf
**
** FUNCTION:    Process a data received notification.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
#define LAST_BUFFER() (!(notify->cmdmod & 0x02))
#define NO_MORE_BYTES() ((residual - mbufp->m_len) == 0)
#define LAST_XFER()  (NO_MORE_BYTES() && LAST_BUFFER())

static void pscaxbuf(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        open_t *openp;
        link_t *link;
        recv_elem_t *recvp;
        struct mbuf *mbufp = (struct mbuf *)0;
        struct mbuf *tmp_mbufp = (struct mbuf *)0;
        dma_req_t *dmap;
        int residual;
        int pca_offset;
        int i;
        int j;
        int k;
        int loops;
        cmd_t cmd;
        int first_time;
	   int  mretry;  /* ix29365 */
	   int  rc;  /* ix29365 */

        CATDEBUG(("in pscaxbuf() \n"));

        /*
        ** Update statistics counters.
        */
        ca->stats.ds.recv_intr_cnt++;
        if (LAST_BUFFER()) {
                ca->stats.cc.rx_frame_lcnt++;
        }


        /*
        ** Get pointers to the subchannel, link, and open
        ** structures.  Return the receive buffer on error.
        */
        if (((sc = ca->sc[notify->subchan]) == NULL)
        || ((link = sc->link[LINK_ID]) == NULL)
        || ((openp = link->lk_open) == NULL)) {
                cat_put_rfb(ca, &(notify->buffer));
                ca->stats.cc.rx_err_cnt++;
                return;
        }

        /*  *d50453* this was moved from before set sc ptr
        ** Log bad return code, return recv buf to recv free fifo.
        ** If we ever recieve bad return code and this is not the
        ** first 41 note in the xfer, then this will be the last
        ** 41 note in the xfer, and the recv elem and all its
        ** accumulated resources must be freed.
        */
        if (notify->retcode) {
                  cat_put_rfb(ca, &(notify->buffer));
                  ca->stats.cc.rx_err_cnt++;
/*d50453*/        /* if a recv is in progress flush all of it */
/*d50453*/        if ((recvp = link->lk_recv_elem) != NULL) {
/*d50453*/           free_recv_element(ca, recvp);
/*d50453*/           /* report lost data */
/*d50453*/           async_status(ca, openp, CIO_ASYNC_STATUS,
/*d50453*/                        CIO_LOST_DATA, notify->subchan,
/*d50453*/                        notify->ccw, notify->retcode);
/*d50453*/        }
                  return;
        }


        /*  *d50453*
        ** If an overrun condition has occured  then
        ** we are waiting for acks from unit check
        ** unsolicited status we sent to channel, so
         ** we need to skip over any recv data associated with
         ** the overrun link. Otherwise we get into a save_notify loop.
         ** and will never see the ack from the unsolicited status cmd.
         */

        /*
        ** Use a receive element for this subchannel
        ** that is currently in progress or get a new one.
        */
        if ((recvp = link->lk_recv_elem) == NULL) {
                /*
                ** Save this notify and stop accepting
                ** notifications if we run out of recv elems.
                */
                if(ca->recv_free == NULL) {
                        cat_logerr(ca, ERRID_CAT_ERR7);
                        save_notify(ca, notify);
                        return;
                }

                /*
                ** If this is a kernel user and we can't get an
                ** mbuf for a PKTHDR, save the notification.
                ** We always build a packet header for kernel
                ** users since they always get an mbuf chain...
                ** ...don't bother for non-kernel processes because
                ** the mbuf chain gets copied into the user-space
                ** buffer eventually anyway.
                */
                if (openp->op_mode & DKERNEL) {
				    /* apar ix29365  retry failed m_get 5 times before reporting
					  error
				    */
				    mretry = 0;
				    do {
					   if ( (mbufp = m_gethdr(M_DONTWAIT, MT_HEADER)) == NULL){
						mretry++;
					   } else {
						 mretry=5;
 					   }	
					} while ( (mretry < 5) && (mbufp == NULL));
	


                        /*if ((mbufp = m_gethdr(M_DONTWAIT, MT_HEADER)) == NULL) { ix29365*/
                        if (mbufp == NULL) {
                                cat_logerr(ca, ERRID_CAT_ERR3);
				  save_notify(ca, notify);
				  return;

                        }
                        mbufp->m_len = 0;
                        mbufp->m_data = (caddr_t)0;
                        ++ca->mbuf_num;           /*d50453*/
                }

                /*
                ** Take an element from the "free" list.
                */
                recvp = ca->recv_free;
                if(ca->recv_free = recvp->rc_next) {
                        recvp->rc_next->rc_last = recvp->rc_last;
                }
                recvp->rc_next = NULL;
                recvp->rc_last = recvp;

                /*
                ** Add this buffer to the "active" list.
                */
                if (openp->recv_act == NULL) {
                        openp->recv_act = recvp;
                } else {
                        recvp->rc_last = openp->recv_act->rc_last;
                        recvp->rc_last->rc_next = recvp;
                        openp->recv_act->rc_last = recvp;
                }

                /*
                ** Make this the current element for this
                ** subchannel.
                */
                link->lk_recv_elem = recvp;

                /*
                ** Initialize the receive element.
                */
                recvp->rc_open = openp;
                recvp->rc_mbuf_head = mbufp;    /* null if ~kernel user */
                recvp->rc_mbuf_tail = mbufp;    /* null if ~kernel user */
                recvp->rc_state = RC_INPROGRESS;
                recvp->rc_scid = notify->subchan;
                recvp->rc_linkid = link->lk_appl_id;
                recvp->rc_count = 0;
                recvp->rc_resid = 0;
                recvp->rc_ccw = notify->ccw;
/*d50453*/ /*       recvp->rc_num_xfers = 0;  d51658 delete */
        }

        /*
        ** Gather the data into mbufs.
        */
        first_time = ca->caddp.rdto;
        residual = notify->length;
        while (residual) {
			 /* apar ix29365  retry failed m_get 5 times before reporting
			    error
			 */
			 mretry = 0;
			 do {
				if ( (mbufp = m_get(M_DONTWAIT, MT_DATA)) == NULL){
				  mretry++;
				} else {
				   mretry=5;
				}
			 } while ( (mretry < 5) && (mbufp == NULL));

                /*
                ** If we can't get an mbuf, save the notification.
                */
                /*if ((mbufp = m_get(M_DONTWAIT, MT_DATA)) == NULL) { ix29365 */
                if (mbufp == NULL) {
                        cat_logerr(ca, ERRID_CAT_ERR3);
			    save_notify(ca, notify);
			    return;
                        return;
                }
                ++ca->mbuf_num;     /*d50453*/

                /*
                ** Set the transfer parameters
                */
                if (residual + first_time > MLEN) {
					   /* apar ix29365  retry failed m_get 5 times before reporting
					   error
					   */
				    mretry = 0;
				    do {
					   rc = m_clget(mbufp);
					   if (!rc ) {
						mretry++;
					   } else {
						 mretry=5;
					   }
				    } while ( (mretry < 5) && (!rc));

                        /*
                        ** Need a cluster mbuf.
                        */
                        /*if (!m_clget(mbufp)) { ix29365 */
                        if (!rc){
                                cat_logerr(ca, ERRID_CAT_ERR3);
				save_notify(ca, notify);
                                m_free(mbufp);
                                --ca->mbuf_num;   /*d50453*/
				return;
                        }
                        mbufp->m_len = min(residual, CLBYTES);
                        mbufp->m_data += first_time;
                } else {
                        mbufp->m_len = residual;
                        mbufp->m_data += first_time;
                }
                if (first_time) {
                        first_time = 0;
                }
                pca_offset = notify->buffer + notify->length - residual;
                recvp->rc_count += mbufp->m_len;

                /*
                ** Add this mbuf to the end of
                ** the current mbuf chain.
                */
                if (recvp->rc_mbuf_tail == NULL) {
                        recvp->rc_mbuf_head = mbufp;
                        recvp->rc_mbuf_tail = mbufp;
                } else {
                        recvp->rc_mbuf_tail->m_next = mbufp;
                        recvp->rc_mbuf_tail = mbufp;
                }

    /*   *d49490*
    ** Check to see if a parity error was reported by the ucode.
    ** If there is bad parity in the cards sram it will cause the pio,
    ** or dma to fail. Since the parity error is reported to this
    ** driver by an 82 error notification that preceeds the current
    ** 41 (data received notfication) a flag must be set when the
    ** 82 error note with rc=83 is received and checked here. If that
    ** flag is set, free the recv element but do not try to read the
    ** data or save the notify.
    */
/*d49490*/    if(sc->sc_flags & CAT_BAD_PARITY) {
/*d49490*/            cat_put_rfb(ca, &(notify->buffer));
/*d49490*/            free_recv_element(ca, recvp);
/*d49490*/            sc->sc_flags &= ~CAT_BAD_PARITY;
/*d49490*/            return;
/*d49490*/    }

                /*
                ** Determine whether to use dma or pio.
                */
                if( mbufp->m_len <= PIO_THRESHOLD ) {
                        /*
                        ** If PIO fails, save the notify,
                        ** free the resources and start over.
                        */
                        if (cat_read_buf(ca, pca_offset,
                        MTOD(mbufp, caddr_t), mbufp->m_len)) {
                                                  free_recv_element(ca, recvp);
                        /* d50725        save_notify(ca, notify); */
                                    /* Above removed since will cause a loop
                                          if the notification is saved when the
                                          adapter is shutdown        */

                                return;
                        }
                        recvp->rc_resid += mbufp->m_len;
                        ca->stats.cc.rx_byte_lcnt += mbufp->m_len;

                        /*
                        ** If this is the last buffer of a buffer
                        ** chain, clear the current subchannel
                        ** receive element pointer, allocate a
                        ** DMA request element and set up a pseudo-
                        ** DMA request to wake up any processes
                        ** waiting for data from this subchannel
                        */
                        if(LAST_XFER()) {
                                /*
                                ** If there are no DMA elements
                                ** available, save the notify.
                                */
                                dmap = dma_alloc(ca, (caddr_t)recvp, 1);
                                if (dmap == NULL) {
                                        /* *d50725* comment
                                        ** take mbuf off chain
                                        */
/*d50725*/                            if(mbufp==recvp->rc_mbuf_head) {
                                           /*  *d50725* comment
                                           **  If this mbuf on only one
                                           **  on the chain.
                                           */
/*d50725*/                               recvp->rc_mbuf_head = NULL;
/*d50725*/                               recvp->rc_mbuf_tail = NULL;
/*d50725*/                            }
/*d50725*/                            else {  /* end one on chain */
/*d50725*/                              tmp_mbufp=recvp->rc_mbuf_head;
/*d50725*/                              while(tmp_mbufp->m_next != mbufp) {
/*d50725*/                                 tmp_mbufp= tmp_mbufp->m_next;
/*d50725*/                              }
/*d50725*/                              recvp->rc_mbuf_tail = tmp_mbufp;
/*d50725*/                              recvp->rc_mbuf_tail->m_next= NULL;
/*d50725*/                            }
/*d50725*/                            m_free(mbufp);
                                        --ca->mbuf_num;   /*d50453*/
                                        save_notify(ca, notify);
                                        return;
                                }

                                /*
                                ** Reset the current subchannel
                                ** receive element pointer.
                                */
                                link->lk_recv_elem = NULL;

                                /*
                                ** Set up the pseudo-DMA element.
                                */
                                dmap->p.recv_ptr = recvp;
                                dmap->dm_open = recvp->rc_open;
                                dmap->dm_end_of_list = TRUE;
                                dmap->dm_req_type = DM_PSEUDO_RECV;
                                dmap->dm_scid = recvp->rc_scid;
                                dmap->dm_state = DM_READY;
						  if ( (residual - CLBYTES) <= 0)
/*d50453*/                       dmap->dm_rfb = (caddr_t)notify->buffer;

                                /*
                                ** Initiate the pseudo-DMA request.
                                ** On error, save the notify and retry.
                                */
                		residual -= mbufp->m_len;
                                if (dma_request(ca, dmap)) {
                                        panic("dma_request() failed in pscaxbuf(DMA)\n");
                                        /*      free_recv_elem(ca, recvp);
                                        save_notify(ca, notify);
                                        return;
                                        */
                                }
                        }
				    /* d50453  must free the receive buffer
					* if the more-to-come bit is on.
					*/
				    else {
						if ((NO_MORE_BYTES() && (notify->cmdmod & 0x02))){
							cat_put_rfb(ca,&(notify->buffer));
							/* If the recieve element is freed then we will loose the
							 *	data.
							 *	free_recv_element(ca,recvp);
							 */
						}
                				residual -= mbufp->m_len;
				    }

                } else {
                        /*
                        ** If there are no DMA elements
                        ** available, save the notify.
                        */
                        if ((dmap = dma_alloc(ca, (caddr_t)recvp, 1)) == NULL) {
                                /*  *d50725*  comment
                                ** take mbuf off chain
                                */
/*d50725*/                    if(mbufp==recvp->rc_mbuf_head) {
                                   /* *d50725* comment
                                   **  If this mbuf on only one
                                   **  on the chain.
                                   */
/*d50725*/                       recvp->rc_mbuf_head = NULL;
/*d50725*/                       recvp->rc_mbuf_tail = NULL;
/*d50725*/                    }
/*d50725*/                    else {  /* end one on chain */
/*d50725*/                      tmp_mbufp=recvp->rc_mbuf_head;
/*d50725*/                      while(tmp_mbufp->m_next != mbufp) {
/*d50725*/                         tmp_mbufp= tmp_mbufp->m_next;
/*d50725*/                      }
/*d50725*/                      recvp->rc_mbuf_tail = tmp_mbufp;
/*d50725*/                      recvp->rc_mbuf_tail->m_next= NULL;
/*d50725*/                    }
                                m_free(mbufp);
                                save_notify(ca, notify);
                                return;
                        }
                        /*
                        ** If this is the last buffer of a buffer
                        ** chain, clear the current subchannel
                        ** receive element pointer.
                        */
                        if(LAST_XFER()) {
                                link->lk_recv_elem = NULL;
                        }

                        /*
                        ** Set up the DMA element.
                        */
                        dmap->p.recv_ptr = recvp;
                        dmap->dm_open = recvp->rc_open;
                        dmap->dm_end_of_list = (LAST_XFER()) ? TRUE : FALSE;
                        dmap->dm_req_type = DM_RECV;
                        dmap->dm_scid = recvp->rc_scid;
                        dmap->dm_linkid = recvp->rc_linkid;
                        dmap->dm_pca_buffer = (caddr_t)pca_offset;
                        dmap->dm_buffer = MTOD(mbufp, caddr_t);
                        dmap->dm_xmem = M_XMEMD(mbufp);
                        dmap->dm_length = mbufp->m_len;

		     	    if ( (residual - CLBYTES) <= 0)
/*d50453*/                dmap->dm_rfb = (caddr_t)notify->buffer;
                        dmap->dm_flags = DMA_READ | DMA_NOHIDE;
                        dmap->dm_state = DM_READY;

                        /*
                        ** Initiate the DMA request.
                        */
                	residual -= mbufp->m_len;
                        if (dma_request(ca, dmap)) {
                                panic("dma_request() failed in pscaxbuf(DMA)\n");
                        }
                }

        } /* END WHILE */
        return;
} /* pscaxbuf() */


/*****************************************************************************
** NAME: pscadmac
**
** FUNCTION:    Process a DMA complete notification.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscadmac(
        struct ca *ca,
        cmd_t *notify)
{
        open_t *openp;
        recv_elem_t *recvp;
        xmit_elem_t *xmitp;
        dma_req_t *nextp;
        ulong saddr;
        ulong bus;
        int reg;
        int rc;
        int spl;
        int i;
        cio_read_ext_t rd_ext;
        struct mbuf *m_ptr;
        dma_req_t *dmap;

        /*
        ** Get the DMA element from the head of the
        ** request queue.
        */
        dmap  = ca->dma_act;

        ASSERT(dmap);

        /*
        ** Cleanup after the DMA
        */
        d_complete(ca->dma_channel, dmap->dm_flags,
                dmap->dm_buffer, dmap->dm_dmalen,
                dmap->dm_xmem, NULL);

        /*
        ** The only time an element should be in DM_ABORTED
        ** state is when free_xmit_element() or free_recv_element()
        ** tried to extract the DMA element that was in
        ** DM_STARTED state, otherwise the routine can just
        ** pluck the element off the queue.
        */
        if (dmap->dm_state == DM_ABORTED) {
	  		 dmap->dm_state = DM_DONE;     /*d50453*/
                if (dmap->dm_req_type & DM_XMIT) {
                        xmitp = dmap->p.xmit_ptr;
                        free_dma_request(ca, dmap);
                        free_xmit_element(ca, xmitp);
                } else { /* HAS to be DM_RECV */
                        recvp = dmap->p.recv_ptr;
                        free_dma_request(ca, dmap);
                        free_recv_element(ca, recvp);
                }

                /* Start processing the next dma request */
                if (ca->dma_act) {
                        rc = dma_request(ca, NULL);
                        ASSERT(rc == 0);
                }
                return;
        }

        switch (dmap->dm_req_type) {

        case DM_XMIT:
                ca->stats.ds.xmit_dma_completes++;
                xmitp = dmap->p.xmit_ptr;
                if (dmap->dm_end_of_list) {
                        free_dma_request(ca, dmap);
                        rc = send_elem(ca, xmitp);
                        ASSERT(rc == 0);
                } else {
                        free_dma_request(ca, dmap);
                }
                break;

        case DM_RECV:
                ca->stats.ds.recv_dma_completes++;
                recvp = dmap->p.recv_ptr;
                ASSERT(recvp);
                recvp->rc_resid += dmap->dm_length;
                ca->stats.cc.rx_byte_lcnt += dmap->dm_length;
                if (dmap->dm_end_of_list) {
                        free_dma_request(ca, dmap);
                        /*
                        ** Get the open ptr.
                        */
                        openp = recvp->rc_open;
                        ASSERT(openp);

                        /*
                        ** Now we have a valid receive element.
                        */
                        recvp->rc_state = RC_COMPLETE;
                        if (openp->op_mode & DKERNEL) {
                                /*
                                ** Fill in the read extension fields.
                                */
                                rd_ext.status = CIO_OK;
                                rd_ext.netid = recvp->rc_scid;
                                rd_ext.sessid = recvp->rc_linkid;

                                /*
                                ** Notify the kernel user of data received.
                                */
                                m_ptr = recvp->rc_mbuf_head;

                                /*
                                ** Set the packet length in the header
                                ** (created in pscaxbuf()) so TCP/IP
                                ** doesn't have to calculate it.
                                */
                                m_ptr->m_pkthdr.len = recvp->rc_resid;
                                recvp->rc_mbuf_head = NULL;
                                (*(openp->op_rcv_fn))(openp->op_open_id,
                                        &(rd_ext), m_ptr);

                                /*
                                ** Notify the SYNC_MODE-mode user via the status queue.
                                */
                                if (openp->op_flags & OP_SYNC_MODE) {
                                        async_status(ca, openp, CIO_ASYNC_STATUS,
                                                CIO_OK, notify->subchan, recvp->rc_ccw, 0);
                                /* d50453w  decrement the number of
                                ** mbufs held by driver.
                                */
/*d50453     */                 ca->mbuf_num-=num_mbufs(recvp->rc_mbuf_head);
                                }

                                free_recv_element(ca, recvp);
                        } else {
                                /*
                                ** User-mode process:
                                ** If user is blocked on read, do a wakeup.
                                */
                                if (openp->op_rcv_event != EVENT_NULL)
                                        e_wakeup(&openp->op_rcv_event);

                                /*
                                ** Notify the SYNC_MODE-mode user via the status queue.
                                */
                                if (openp->op_flags & OP_SYNC_MODE) {
                                        async_status(ca, openp, CIO_ASYNC_STATUS,
                                                CIO_OK, notify->subchan, recvp->rc_ccw, 0);
                                }

                                /*
                                ** Notify the user vi poll/select mechanism.
                                */
                                if (openp->op_select & POLLIN) {
                                        selnotify((int)ca->dev, openp->op_chan, POLLIN);
                                }

                                /*
                                ** Free the receive element in catread()...
                                */
                        }
                } else {
                        free_dma_request(ca, dmap);
                }
                break;

        default:
                panic("pscadmac()--unknown dmap->dm_req_type\n");
                break;
        }


        /*
        ** Start processing the next dma request.
        */
        if (ca->dma_act) {
                rc = dma_request(ca, NULL);
                ASSERT(rc == 0);
        }

        return;
} /* pscadmac() */



/*****************************************************************************
** NAME: ack_sets
**
** FUNCTION:    Handle an acknowledgement of a set subchannel parameters
**              command.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void ack_sets(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        open_t *openp;
        link_t *link;
        cmd_t cmd;

        /* if this flag is set the a start was attempted while the */
        /* channel was hung and there is no scstructure allocated  */
        /* for this subchannel.  d52590                         */
        if(ca->flags & CAT_CHAN_BLOCK) return;    /*d52590*/
        /*
        ** If any of these ASSERT, the ucode or the driver must be bad
        */
        sc = ca->sc[notify->subchan];
	   if (sc == NULL ) return;
        ASSERT(sc);

        link = sc->links[0];
        ASSERT(link);

        openp = link->lk_open;
        ASSERT(openp);

        if (notify->retcode == RETGOOD) {
                /*
                ** first check that this subchannel is being started
                ** get the open pointer that owns this subchannel
                */
                /*
                ** Ensure the subchannel is in the correct state
                */
                if (sc->sc_state != SC_CLOSED) {
                        async_status(ca, openp, CIO_START_DONE,
                                CIO_NETID_INV, notify->subchan,0,0);
                        return;
                }

                /*
                ** Start the subchannel
                */
                bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                cmd.command = PSCASTRT;
                cmd.cmdmod = MODACK;
                cmd.length = 0;
                cmd.subchan = notify->subchan;
/*d51658*/       cmd.correl = link->lk_correl;

                if (cat_put_cmd(ca, &cmd)) {
                        if (sc->sc_state != SC_CLOSING ) {
                                SET_GROUP( notify->subchan, SC_CLOSED );
                        }
                        async_status(ca, openp, CIO_START_DONE,
                                CIO_NOT_STARTED, notify->subchan,0,0);
                }
                sc->sc_state = SC_SETUP;
        } else {
                /* leave it in a closed state */
                if (sc->sc_state != SC_CLOSING ) {
                        SET_GROUP( notify->subchan, SC_CLOSED );
                }
                async_status(ca, openp, CIO_START_DONE,
                        CIO_NOT_STARTED, notify->subchan, 0, notify->retcode);
        }
        return;
} /* ack_sets() */


/*****************************************************************************
** NAME: ack_strt
**
** FUNCTION:    Handle the acknowledgement of a start command.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void ack_strt(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        open_t *openp;
        link_t *link;
        int i;

        /*
        ** get the subchannel pointer
        */
        sc = ca->sc[notify->subchan];
        ASSERT(sc);

        link = sc->links[0];
        ASSERT(link);

        openp = link->lk_open;
        ASSERT(openp);

        /*
        ** Ensure the subchannel is in the correct state.
        */
        if (sc->sc_state != SC_SETUP) {
                async_status(ca, openp, CIO_START_DONE,
                        CIO_NETID_INV, notify->subchan, 0, 0);
                return;
        }

        /*
        ** the subchannel is in the correct state for this response,
        ** so now check the return code
        */
        if (notify->retcode != RETGOOD) {
                SET_GROUP(notify->subchan, SC_CLOSED);
                async_status(ca, openp, CIO_START_DONE, CIO_NOT_STARTED,
                        notify->subchan, 0, notify->retcode);
                return;
        }

        ca->tot_subchan += sc->sc_subset;
        openp->op_num_scopen += sc->sc_subset;
	   openp->op_sc_opened = 1;   /* indicate that subchannels/links have been started */
							/* ix29279  */

        /*
        ** CLAW mode SC's don't become STARTED until the
        ** SYSVAL, CLRESP, and CLCONN are received,
        ** while other SC types are now STARTED
        */
        if (sc->specmode & CAT_CLAW_MOD) {
                /*
                ** Second SC in pair points to first,
                ** so don't bother with SET_GROUP()
                */
                SET_GROUP(notify->subchan, SC_STARTING);
                return;
        } else {
                SET_GROUP(notify->subchan, SC_OPEN);
                async_status(ca, openp, CIO_START_DONE,
                        CIO_OK, notify->subchan, 0, 0);
        }

        return;
} /* ack_strt() */


/*****************************************************************************
** NAME: ack_stop
**
** FUNCTION:    Handle the acknowledgement to a stop subchannel command.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
void ack_stop(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        int i;
        int j;
/*d49940*/ int lastsc;

/*d52590  When channel is down don't process this stop */
        if(ca->flags & CAT_CHAN_BLOCK) return;  /*d52590 */

        if ((sc = ca->sc[notify->subchan]) == NULL) {
                return;
        }

        /*
        ** Wakeup the stopping process.
        */
#ifndef AIXV3
        if (sc->sc_stop_ack != EVENT_NULL ) {
			e_wakeup(&sc->sc_stop_ack);
		}
#else
        if (sc->sc_stop_ack) {
                e_post(CAT_STOP_EVENT, sc->sc_stop_ack);
        }
#endif  
	   
/* ix31884 */
/* Can't free memory on the interrupt level.
 * The sleeping process will free it when the post
 * wakes it up.
 * Just in case we will mark the state as DELETE
 * so that it can be freed elsewhere
 */
	SET_GROUP(sc,SC_DELETE);

        return;
} /* ack_stop() */



/*****************************************************************************
** NAME: ack_xbuf
**
** FUNCTION:    Handle the acknowledgement to a data transmit command.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void ack_xbuf(
        struct ca *ca,
        cmd_t *notify)
{
        open_t *openp;
        subchannel_t *sc;
        link_t *link;
        xmit_elem_t *xmitp;
        dma_req_t *dmap;

        /* if this flag is set the a start was attempted while the */
        /* channel was hung and there is no scstructure allocated  */
        /* for this subchannel.  d52590                          */
        if(ca->flags & CAT_CHAN_BLOCK) return;    /*d52590*/

        /*
        ** Get the SC, link and open pointers.
        */
        if ((sc = ca->sc[notify->subchan]) == NULL
        || (link = sc->link[(sc->specmode & CAT_CLAW_MOD) ? notify->correl : 0]) == NULL
        || (openp = link->lk_open) == NULL) {
                return;
        }

        /*
        ** Update statistics counter.
        */
        ca->stats.ds.xmit_intr_cnt++;

        /*
        ** Find the transmit element
        */
        for (xmitp = ca->xmit_act; xmitp ; xmitp = xmitp->xm_next) {
                if (xmitp->xm_cmd.correl == notify->correl
                && openp == xmitp->xm_open) {
                        break;
                }
        }
        if (xmitp == NULL) {
                return;
        }

        /*
        ** Keep track of how many buffers are available
        ** and release the command element.
        */
        if (ca->xmit_bufs_avail < ca->caddp.config_params.xmitno) {
                ca->xmit_bufs_avail += xmitp->xm_cmd.data[0];
        }
        xmitp->xm_cmd.command = 0;

        /*
        ** Now, check the return code.
        */
        if (notify->retcode != RETGOOD) {
                ca->stats.cc.tx_err_cnt++;
        }

        if (xmitp->xm_ack & CIO_ACK_TX_DONE) {
                if (notify->retcode != RETGOOD) {
                        async_status(ca, openp, CIO_TX_DONE,
                                CIO_LOST_DATA, notify->subchan,
                                notify->correl, notify->retcode);
                } else {
                        /*
                        ** Notify, calling process if requested.
                        */
                        async_status(ca, openp, CIO_TX_DONE, CIO_OK,
                                notify->subchan, notify->correl, 0);
                }
        }

        /*
        ** Update statistics counters.
        */
        if (++ca->stats.cc.tx_frame_lcnt == 0) {
                ca->stats.cc.tx_frame_mcnt++;
        }
        if ((ca->stats.cc.tx_byte_lcnt += xmitp->xm_length) <
                xmitp->xm_length) {
                ca->stats.cc.tx_byte_mcnt++;
        }

        /*
        ** Wakeup anyone affected by this transfer
        ** The e_wakeup() at the end of the PSCAACK or
        ** PSCAERR case will wakeup anyone waiting on
        ** a PSCA buffer.
        */
        if (openp->op_select & POLLOUT) {
                selnotify((int)ca->dev, openp->op_chan, POLLOUT);
        }

        /*
        ** If the kernel user wants us to free the mbufs,
        ** do so now.
        */
        if (xmitp->xm_mbuf
                && (openp->op_mode & DKERNEL)
                && ((xmitp->xm_ack & CIO_NOFREE_MBUF) == 0)) {
                ca->mbuf_num -= num_mbufs(xmitp->xm_mbuf);   /*d50453*/
                m_freem(xmitp->xm_mbuf);
        }
/*d50453   add starts here */
        else if (xmitp->xm_mbuf && (openp->op_mode & DKERNEL)) {
           /*
           **  Even if Kernal mode process wants to free his own
           **  mbufs, I will decrement the count of mbufs held by
           **  the driver.
           */
           ca->mbuf_num -= num_mbufs(xmitp->xm_mbuf);
        }
/*d50453   add ends here */


        free_xmit_element(ca, xmitp);

        return;
} /* ack_xbuf() */


/*****************************************************************************
** NAME: pscaack
**
** FUNCTION:    Handle an acknowledgement command.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscaack(
        struct ca *ca,
        cmd_t *notify)
{
        switch (notify->origcmd) {
                case PSCASETS :
                        ack_sets(ca, notify);
                        break;
                case PSCASTRT :
                        ack_strt(ca, notify);
                        break;
                case PSCASTOP:
                        ack_stop(ca, notify);
                        break;
                case PSCAXBUF:
                case PSCAXLST:
                        ack_xbuf(ca, notify);
                        break;
                case PSCASUSP:
                case PSCARSTS:
                case PSCARSTA:
                        break;
                case PSCAUNST:  /* Sent unsolicited status */
                        /*
                        ** We retried the max. # of times once
                        ** we ran out of resources (mbufs, dma
                        ** elems, or recv. elems) and sent the
                        ** host unsolicited status.  Now we
                        ** know the adapter sent it so shut down.
                        */
                        if (ca->num_unsolicited) {
                                if (ca->num_unsolicited == 1) {
                                        notify_all_opens(ca, CIO_NOMBUF);
                                        cat_shutdown(ca);
                                }
                                ca->num_unsolicited--;
                        }
                        break;
                case PSCASETA:
                case PSCACDBA:
                case PSCALDCU:
                default:
                        bcopy(notify, &(ca->ca_cmd), sizeof(CTLFIFO));
                        if (ca->ca_cmd.cmd_event != EVENT_NULL) {
                                e_wakeup(&(ca->ca_cmd.cmd_event));
                        }
                        break;
        }
        return;
} /* pscaack() */


/*****************************************************************************
** NAME: err_xlst
**
** FUNCTION:    Handle an error from a transmit.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void err_xlst(
        struct ca *ca,
        cmd_t *notify)
{
        xmit_elem_t *xmitp;
        subchannel_t *sc;
        link_t *link;
        open_t *openp;

        if ((sc = ca->sc[notify->subchan]) == NULL) {
                return;
        }

        if ((link = sc->link[LINK_ID]) == NULL) {     /* d51658 */
                return;
        }

        if ((openp = link->lk_open) == NULL) {
                return;
        }

        if (notify->correl != 0) {
                /*
                ** Find the transmit element
                */
                for (xmitp = ca->xmit_act; xmitp; xmitp = xmitp->xm_next) {
                        if (xmitp->xm_cmd.correl == notify->correl) {
                                break;
                        }
                }
                if (xmitp != NULL) {
                        free_xmit_element(ca, xmitp);
                }
        }
        async_status(ca, openp, CIO_TX_DONE,
                CIO_HARD_FAIL, notify->subchan, notify->correl, notify->retcode);
        cat_logerr(ca, ERRID_CAT_ERR5);

        return;
} /* err_xlst() */

/*****************************************************************************
** NAME: err_con_resp   d51658 added new routine
**
** FUNCTION:    Handle an error from a claw connection confirm or claw,
**              connection response command.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/

static void err_con_resp(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        link_t *link;
        open_t *openp;
        int i;
        cmd_t cmd; /* 72359 */
        char buf[3 * LK_NAME_LEN]; /* 72359 */
	   struct session_blk sess_blk; /* 72359 */


        if ((sc = ca->sc[notify->subchan]) == NULL) return;

        if (notify->origcmd == PSCACLCON) {
            if ((link = sc->link[notify->ccw]) == NULL) {
                    return;
            }
        }
        else {    /* this is connection response use correlator. */
		  if ((link = sc->link[notify->correl]) == NULL) {
				return;
		  }
        }

        if ((openp = link->lk_open) == NULL) {
                return;
        }


 	   /* 72359  
 	    * If we got the error notification on a Confirm command 
         * which is issued after we have been assigned a link by
	    * the microcode, the microcode has been told 
	    * to disconnect the link.  The link structure shouldb
	    * be in the disconnected LK_DISC state so we
	    * assume that the validate is still good, and issue
	    * the connection request to re-start the connection
	    * sequence.
 	    */

       /* d72359  originally drop_link(sc,link); always, however
        *  if the link was disconnected previously (race condition)
        *  failure then just issue the connection request to
        *  recover the link.
        */

	   
	if (  (notify->origcmd == PSCACLCON ) &&
		 (notify->retcode == RETCBLID  )  &&
		 (link->lk_state == LK_DISC    ) ) {
           /* Issue a connection request since the race condition hangs everyone */
		 /* Issue the request to get a new link id since the other one was     */
 		 /* invalid												 */
				    bzero(&cmd,sizeof(cmd_t));
                        link->lk_state = LK_REQ;
                        cat_get_cfb(ca, &(cmd.buffer));
                        bcopy(link->lk_WS_appl, buf, LK_NAME_LEN);
                        bcopy(link->lk_H_appl, buf+LK_NAME_LEN, LK_NAME_LEN);
                        cat_write_buf(ca, cmd.buffer, buf, 2 * LK_NAME_LEN);
                        cmd.command = PSCACLREQ;
                        cmd.subchan = notify->subchan;
                        cmd.correl = link->lk_correl;
                        cmd.length = 3 * LK_NAME_LEN;
                        cat_put_cmd(ca, &cmd);
				    return;
	} else {
	   /* d72359 The confirm must have realy failed.  
	    * The application must check the status of the CIO_START 
	    */

        async_status (ca, openp, CIO_START_DONE,
             CIO_NOT_STARTED , notify->subchan, NOTIFY_CCW,notify->retcode);
        /*
        ** This code will be hit if the user tries to start a
        ** subchannel in the wrong mode, to allow him to change
        ** his mode setting and restart, we need to free the
        ** sc struct and pointer and link struct and pointer.
        */

	   /* d72359
	    * If this is the last link for this subchannel
	    * halt the subchannel after freeing the links
	    * otherwise just free the link since another
	    * validate from the host could still establish
	    * a connection
	    */

	   if ( sc->num_links > 1 ){
		 /* just free the current link, it has no valid link_id */
	      /* drop_link(sc,link); */
		 /*
		  * ix31884 
 		  * Mark the link as deleted and free later
		  *
		  */
		   link->lk_state = LK_DELETE;

	   } else {
		 /* ix31884  Mark the subchannel as the DELETE state
		  * so as not to use it in the future.
            * It will be freed in the user processes environment
            *
		  * free_sc_links(ca,sc);
		  */
		  SET_GROUP(sc,SC_DELETE);

	   }	
		 
	}

        return;
} /* err_con_resp() */

/*****************************************************************************
** NAME: err_sets_strt_stop
**
** FUNCTION:    Handle an error from a set subchannel parameters, start,
**              or stop subchannel command.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void err_sets_strt_stop(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        link_t *link;
        open_t *openp;
        int i;

#ifdef DEBUG
cat_gen_trace("ESSS",0,0,0);
#endif

        if ((sc = ca->sc[notify->subchan]) == NULL) return;


        if ((sc->specmode & CAT_CLAW_MOD) && (notify->origcmd != PSCASTOP)) {
                /* Links not yet set up..use correlator index into links. */
/*d51658*/if ((link = sc->links[notify->correl]) == NULL) {
                    return;
            }
        }
        else {
            if ((link = sc->link[LINK_ID]) == NULL) {
                    return;
            }
        }


        if ((openp = link->lk_open) == NULL) {
                return;
        }


        if (notify->origcmd == PSCASTOP) {
#ifndef AIXV3
                if (sc->sc_stop_ack != EVENT_NULL) {
                    cat_gen_trace("PSTE",0,0,0);
/*d51658 */          e_wakeup(&sc->sc_stop_ack);
                }
#else

                if (sc->sc_stop_ack) {
#ifdef DEBUG
			    cat_gen_trace("PSTE",0,0,0);
#endif
/*d51658*/         e_post(CAT_STOP_EVENT,sc->sc_stop_ack);
                }
#endif
                /*
                ** tot_subchan will not have been incrmented yet if this
                ** error occured on sets or start.
                */
                ca->tot_subchan--;
                openp->op_num_scopen--;
                async_status (ca, openp, CIO_HALT_DONE,
                CIO_NETID_INV , notify->subchan, NOTIFY_CCW, notify->retcode);
        } else {
                async_status (ca, openp, CIO_START_DONE,
                CIO_NETID_INV , notify->subchan, NOTIFY_CCW, notify->retcode);
        }

        /*
        ** This code will be hit if the user tries to start a
        ** subchannel in the wrong mode, to allow him to change
        ** his mode setting and restart, we need to free the
        ** sc struct and pointer and link struct and pointer.
        */

/* ix31884
 * Don't free the links this is run on kernel level.
 *
 *       free_sc_links(ca, sc);    /* free up the links and sc */
 
	SET_GROUP(sc,SC_DELETE);

        return;
} /* err_sets_strt_stop() */


/*****************************************************************************
** NAME:        err_other
**
** FUNCTION:    handle an error notification (other than
**              one for write, start, stop, or setsub).
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void err_other(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        link_t *link;
        open_t *openp;


#ifdef DEBUG
cat_gen_trace("EOTH",0,0,0);
#endif

/*d52590 add this next section   start */
         if (notify->retcode == RETHWACT) {
            /* set the channel blocked flag so starts are disabled */
            /* if you allow a start attemp with ucode in blocked state */
            /* an assert(sc) will result.                              */
            ca->flags |= CAT_CHAN_BLOCK;
         }
         if (notify->retcode == RETHWREL) {
            /* reset the channel blocked flag so starts are enabled */
            ca->flags &= ~CAT_CHAN_BLOCK;
         }
/*d52590 add this next section   end   */

/*d49490* moved the check for bad parity till after openp obtained */

        if ((sc = ca->sc[notify->subchan]) == NULL) {
                return;
        }

        link = sc->links[(sc->specmode & CAT_CLAW_MOD) ? (notify->ccw >> 3) : 0];
        if (link == NULL) {
                return;
        }

        if ((openp = link->lk_open) == NULL) {
                return;
        }

        /*d49490
        ** Log the parity error detected with rec'd data.
        ** and set flag in sc so that pscaxbuf will not try
        ** to read the data with bad parity.
         */
/*d49490*/        if (notify->retcode == RETCNMIP) {
/*d49490*/                ca->stats.cc.rx_err_cnt++;
/*d49490*/                cat_logerr(ca, ERRID_CAT_ERR8);
/*d49490*/                sc->sc_flags |= CAT_BAD_PARITY;
/*d49490*/        }

        async_status(ca, openp, CIO_ASYNC_STATUS,
                notify->command, notify->subchan, NOTIFY_CCW, notify->retcode);

        return;
} /* err_other() */


/*****************************************************************************
** NAME:        err_seta
**
** FUNCTION:    handle an error notification
**              for the set adapter parameters.
**              do_setadap is sleeping waiting for
**              completion of the operation, just wake it up.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void err_seta(
	struct ca *ca,
	cmd_t *notify)
{
		bcopy(notify, &(ca->ca_cmd), sizeof(CTLFIFO));
	     if (ca->ca_cmd.cmd_event != EVENT_NULL) {
			 e_wakeup(&(ca->ca_cmd.cmd_event));
		}	
}



/*****************************************************************************
** NAME: pscaerr
**
** FUNCTION:    Handle an error notification.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscaerr(
        struct ca *ca,
        cmd_t *notify)
{
        switch (notify->origcmd) {
                case PSCASTRT:
                case PSCASETS:
                case PSCASTOP:
                        err_sets_strt_stop(ca, notify);
                        break;
                case PSCAXLST:
                        err_xlst(ca, notify);
                        break;
			 case PSCASETA:
				err_seta(ca, notify);
				break;
/*d51658*/       case PSCACLRESP:
/*d51658*/       case PSCACLCON:
/*d51658*/               err_con_resp(ca, notify);
/*d51658*/               break;
                default:
                        err_other(ca, notify);
                        break;
        }
        return;
} /* pscaerr() */


/*****************************************************************************
** NAME: psca_other
**
** FUNCTION:    Handle any notification that isn't uniquely handled.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void psca_other(struct ca *ca, cmd_t *notify)
{
        subchannel_t *sc;
        link_t *link;
        open_t *openp;

        if ((sc = ca->sc[notify->subchan]) == NULL) {
                /*
                ** this will always happen for offline and online
                */
                notify_all_opens(ca, notify->command);
                return;
        }

        /*
        ** links not yet set up  for Claw
        */
        if (sc->specmode & CAT_CLAW_MOD) {
                return;
        }

        if ((link = sc->links[0]) == NULL) {
                return;
        }

        if (openp = link->lk_open) {
                async_status(ca, openp, CIO_ASYNC_STATUS,
                        notify->command, notify->subchan, notify->ccw, notify->retcode);
        }
        return;
} /* psca_other() */


/*****************************************************************************
** NAME: pscarsts
**
** FUNCTION:    Handle a selective reset to a subchannel (subchannel group).
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscarsts(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        open_t *openp[MAX_LINKS];   /*d51658*/
        link_t *link;
        cmd_t cmd;
        int   i,j;            /*d51658*/


        /*
        ** If the reset came in after we sent the
        ** PSCASTOP, don't bother responding---it
        ** will just result in an error.
        */
        if ((sc = ca->sc[notify->subchan]) == NULL
        || sc->sc_state == SC_CLOSED
        || sc->sc_state == SC_CLOSING) {   /* closing added d50453 */
                goto reset_comp;
        }


        if (sc->specmode & CAT_CLAW_MOD) {
                /* d51658 added comment.                               */
                /* I need to check links[0] here since the reset note */
                /* does not include a link id with it because it is   */
                /* for all link_ids on this subchannel.               */
                if ((sc->sc_state != SC_OPEN) ||
                        ((link = sc->links[notify->ccw]) == NULL)) {
                        goto reset_comp;
                }
        } else if ((link = sc->links[0]) == NULL) {
                goto reset_comp;
        }

/* d51658 replace the rest of this routine ....................*/
/* we may be dealing with more than one open and multiple links on the  */
/* subchannel and all link xmits must be flushed and all opens notified */
/* clean_sc_queues need be run only once in claw or non-claw, it will   */
/* get all the xmints on the subchannel, but the async_status must      */
/* be sent (one time) to each open with a link on this subchannel.      */

        if (sc->specmode & CAT_CLAW_MOD) {
                clean_sc_queues(ca, sc->sc_id, -1);
                for (j=0; j<MAX_LINKS; j++) {
                   openp[j]=(open_t *)0;
                   if( (link=sc->links[j]) != NULL) {
                        if ((openp[j] = link->lk_open) == NULL) continue;
                        else {
                           for(i=0; i<MAX_LINKS; i++) {
                              if((openp[i]==openp[j]) && (i!=j)) break;
                           }
                           if(i!=MAX_LINKS) continue;
                        }
                        async_status(ca, openp[j], CIO_ASYNC_STATUS, CAT_RESET_SUB,
                           notify->subchan, notify->ccw, notify->retcode);
                   }
                }

        } else {
                clean_sc_queues(ca, sc->sc_id, 0);
                async_status(ca, link->lk_open, CIO_ASYNC_STATUS, CAT_RESET_SUB,
                notify->subchan, notify->ccw, notify->retcode);
        }

        /*   d50453 moved from beginning
        ** If reset-flush mode was specified for this subchannel,
        ** then the adapter is waiting in holdoff mode for acknowledgement
        ** and we need to reset the subchannel to release it.
        **
        */
reset_comp:
        if (sc->specmode & CAT_FLUSHX_MOD) {
                bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                cmd.command = PSCARSTS;
                cmd.subchan = notify->subchan;
                cat_put_cmd(ca, &(cmd));
        }

        return;

} /* pscarsts() */


/*****************************************************************************
** NAME: pscarsta
**
** FUNCTION:    Handle a system reset.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscarsta(struct ca *ca, cmd_t *notify)
{
        subchannel_t *sc;
        open_t *openp;
        link_t *link;
        cmd_t cmd;
        int x;
        int spl;
        int i;

        /*
        ** If RESTFLSH mode set for the adapter, issue a system
        ** reset all command to clear the hold off condition.
        */
        DISABLE_INTERRUPTS(spl);
        x = CAT_MEM_ATT;
        CAT_READ1(x, 0x74, &i);         /* Check the adapter flags */
        BUSMEM_DET(x);
        ENABLE_INTERRUPTS(spl);
        if (ca->piorc) {
                cat_shutdown(ca);
                ca->flags |= CATDEAD;
                return;
        }
        if (i & 2) {
                bzero(&cmd, sizeof(CTLFIFO)); /* d51376 */
                cmd.command = PSCARSTA;
                cat_put_cmd(ca, &(cmd));
        } else {
                /*
                ** If reset-flush mode was specified for any subchannel,
                ** then the adapter is waiting in holdoff mode for
                ** acknowledgement and we need to issue a reset subchannel
                ** command to release it.
                */
                /* re-use 'i'... */
                for (i=0; i<MAX_SUBCHAN; i++) {
                        if (ca->sc[i] &&
                                (ca->sc[i]->specmode & CAT_FLUSHX_MOD)) {
                                bzero(&cmd,sizeof(CTLFIFO)); /* d51376 */
                                cmd.command = PSCARSTS;
                                cmd.subchan = i;
                                cat_put_cmd(ca, &(cmd));
                        }
                }
        }

        /*
        ** Release xmit elems for each active subchannel.
        */
        clean_sc_queues(ca, -1, 0);
        notify_all_opens(ca, CAT_RESET_ALL);
        return;
} /* pscarsta() */


/*****************************************************************************
** NAME: pscaabrt
**
** FUNCTION:    Handle a microcode abort.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscaabrt(struct ca *ca, cmd_t *notify)
{
        subchannel_t *sc;
        open_t *openp;
        link_t *link;

        ca->flags |= CATDEAD;
        notify_all_opens(ca, CIO_BAD_MICROCODE);

        /*
        ** NOTE: don't clear out received data
        */
        clean_sc_queues(ca, -1 ,0);
        cat_logerr(ca, ERRID_CAT_ERR1);

        return;
} /* pscaabrt() */


/*****************************************************************************
** NAME: pscaxflu
**
** FUNCTION:    Handle a flush current transfer.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscaxflu(
        struct ca *ca,
        cmd_t *notify)
{
        subchannel_t *sc;
        open_t *openp;
        link_t *link;
        recv_elem_t *recvp;

#ifdef DEBUG
cat_gen_trace("XFLU",0,0,0);
#endif

        /*
        ** get the subchannel for this transfer
        */
        if ((sc = ca->sc[notify->subchan]) == NULL)
                return;

        if ((link= sc->link[LINK_ID]) == NULL)  /* d51658 */
                return;

        if ((openp = link->lk_open) == NULL) {
                return;
        }

        /*
        ** now find a receive element for this subchannel
        ** that is currently in progress
        */
        recvp = link->lk_recv_elem;
        if( recvp == NULL )
                return;

#if 1
        async_status(ca, openp, CIO_ASYNC_STATUS,
         notify->command, notify->subchan, notify->ccw, notify->retcode);


/*         if(notify->chanstat != 0x4a) {  d51658 */
            /*
            ** Free the receive element and associated resources
            */
            free_recv_element(ca, recvp);
            link->lk_recv_elem = NULL;

#endif

        return;
} /* pscaxflu() */


/*****************************************************************************
** NAME: pscabufav
**
** FUNCTION:    Send buffer now available.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
static void pscabufav(struct ca *ca, cmd_t *notify)
{
        open_t *openp;
        int i;

        if (ca->flags & CATXMITOWED) {
                for (i = 0; i < ca->num_opens; i++) {
                        openp = &ca->open_lst[i];
                        if ((openp->op_flags&OP_OPENED)
                        && (openp->op_flags&XMIT_OWED)) {
                                (*(openp->op_xmit_fn))(openp->op_open_id);
                                openp->op_flags &= ~XMIT_OWED;
                        }
                }
                ca->flags &= ~CATXMITOWED;
        }
        return;
} /* pscabufav() */


/*****************************************************************************
** NAME: num_mbufs
**
** FUNCTION:    Return count of mbufs in a chain.
**
** EXECUTION ENVIRONMENT: process or interrupt (off-level)
**
** NOTES:
**
** RETURNS: number of mbufs
**
*****************************************************************************/
int num_mbufs(struct mbuf *mbufp)
{
        int num_mbufs = 0;
        struct mbuf *mp;

        for (mp=mbufp; mp; mp=mp->m_next) {
                num_mbufs++;
        }

        return num_mbufs;
} /* num_mbufs() */

/* d50453 new function */
/*****************************************************************************
** NAME:        send_overrun
**
** FUNCTION:    Send an overrun sense byte to channel when mubfs are
**              depleted.
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**
** RETURNS: nothing (void)
**
*****************************************************************************/
send_overrun(
        struct ca *ca,
        cmd_t *notify)
{

        cmd_t cmd;
        uchar sense_byte = 0x04;        /* Overrun */

        bzero(&cmd, sizeof(CTLFIFO));
        cmd.command = PSCAUNST; /* Send unsolicited status */
        cmd.cmdmod = MODACK;
        cmd.data[0] = 0xE;      /* Unit Check + CE + DE */
        cmd.length = 1;
        if (cat_get_cfb(ca, &cmd.buffer)) {
                CATDEBUG(("shutting down---cat_get_cfb() failed\n"));
                cat_shutdown(ca);
        }
        cat_write_buf(ca, cmd.buffer, &sense_byte, 1);

        /*
        ** Send the unsolicited status to
        ** the offending subchannel.
        */
        ca->num_unsolicited++;
        cmd.subchan = notify->subchan;
        cat_put_cmd(ca, &cmd);

        return;
} /* send_overrun() */

