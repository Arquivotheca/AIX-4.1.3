static char sccsid[] = "@(#)49  1.29  src/bos/kernext/cat/cat_close.c, sysxcat, bos411, 9428A410j 2/22/94 16:53:08";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catclose()
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
#define FNUM 2

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
#include <sys/types.h>
#include <sys/except.h>
#include <errno.h>

#include "catdd.h"

/*
 * Externs
 */
extern int caglobal_lock;       /* Global Lock Variable */


/*****************************************************************************
** NAME:        catclose
**
** FUNCTION:    Disable the adapter's interrupts, remove the interrupt
**              handler, unpin the code, and release the DMA channel.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              Major and minor device numbers
**    Output:
**              status code
**    Called From:
**              System call handler via close() system call.
**    Calls:
**              lockl() catget() unlockl()
**
** RETURNS:     0 - Success
**              ENXIO - invalid device
**
*****************************************************************************/
int
catclose(
        dev_t dev,              /* major and minor device number */
        chan_t chan)            /* multiplexed channel number */
{
        struct session_blk sess_blk;
        struct ca *ca;
        open_t *openp;
        int i;
        int spl;
        int j;
        int k;
        link_t *link;
        subchannel_t *sc;
        cmd_t cmd;
		int  lk_open_flag = 0;

        DDHKWD1(HKWD_DD_CATDD, DD_ENTRY_CLOSE, 0, dev);

cat_gen_trace("cls1",0,openp,ca);
        /*
        ** We do not check for CATDEAD here because we allow
        ** the user process to read any pending data and close
        ** the device.  However, we do check for CATDEAD before
        ** attempting to halt subchannels.
        */
        if ((ca = catget(minor(dev))) == NULL) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_CLOSE, ENXIO, dev);
                return ENODEV;
        }
        if (((openp = &ca->open_lst[chan])->op_flags&OP_OPENED) == 0) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_CLOSE, ENXIO, dev);
                return ENXIO;
        }

        DISABLE_INTERRUPTS(spl);

        /*
        ** If the adapter is paused (due to a SYNC_MODE-mode
        ** user having a full status queue), unpause the
        ** adapter if the closer is the culprit, or tell the
        ** user to try again.
        */
        if (ca->flags & CAT_PAUSED) {
                if ((openp->stat_free == NULL)
                && (openp->op_flags & OP_SYNC_MODE)) {
                        ENABLE_INTERRUPTS(spl);
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_CLOSE, EPERM, dev);
/*d51658*/               /* should unpause adapter and allow to close */
/*d51658*/               ca->flags &= ~CAT_PAUSED;
/*d51658  delete this     return EPERM;    */
                } else {
                        ENABLE_INTERRUPTS(spl);
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_CLOSE, EAGAIN, dev);
                        return EAGAIN;
                }
        }

/* ix29279
   Check if any subchannels or links have been established for this 
   open.  If so then they may have to be terminated, otherwise
   don't mess with subchannels and links.
   The op_sc_opened field is set to TRUE when a subchannel is started
   or a link is established in claw mode 
*/
if ( openp->op_sc_opened !=  0 ) {
        /*
        ** For Claw mode, disconnect all links that belong
        ** to this open after cleaning out all transmission
        ** queues. If there are no more active links, close the
        ** subchannel.  For 3088 mode, stop all subchannels started
        ** by this open and clean out any transmit requests in-progress
        ** for this open.  Set the subchannel state to SC_CLOSING.  When
        ** the interrupt gets the PSCA ack it will change it to
        ** SC_CLOSED
        */
        for (i=0; i<MAX_SUBCHAN; i++) {
			lk_open_flag = 0;
           if (((sc = ca->sc[i]) != NULL) && !(ca->flags & CATDEAD)) {
               if (sc->specmode & CAT_CLAW_MOD) {
                      /*
                      ** both read and write sc share the same struct
                      ** so when the read is visited first, the links
                      ** are disconnected
                      */
                      for (j=0; j<MAX_LINKS; j++) {
                          if ((link = sc->links[j])
                                  && link->lk_open == openp) {

cat_gen_trace("cls2",openp,link->lk_open,link->lk_state);

                                        if (link->lk_state == LK_RESP ||
                                             link->lk_state == LK_FIRM ){

										lk_open_flag = 1;
										clean_sc_queues(ca, i, j);
/*d51376*/								bzero(&(cmd),sizeof(CTLFIFO));
										cmd.command = PSCACLDISC;
										cmd.cmdmod = 0; /* No ack */
 										cmd.subchan = i;
 										cmd.ccw = link->lk_actual_id;
										cat_put_cmd(ca, &cmd);
/*d51658*/								/* free the link */
										sc->link[link->lk_actual_id] = (link_t *)0;
										ENABLE_INTERRUPTS(spl);
										LOCK(ca->lcklvl);
/*d51658*/								drop_link(sc, link);
										UNLOCK(ca->lcklvl);
										DISABLE_INTERRUPTS(spl);
		
   } else {
                                             if ( link->lk_state == LK_NEW ||
                                                   link->lk_state == LK_DISC ) {
                                                       lk_open_flag = 1;
                                                       if ( link->lk_actual_id )
                                                   sc->link[link->lk_actual_id]= (link_t *)0;
											ENABLE_INTERRUPTS(spl);
											LOCK(ca->lcklvl);
											drop_link(sc,link);
											UNLOCK(ca->lcklvl);
											DISABLE_INTERRUPTS(spl);
								} else {

/* The link is pending to the microcode and can not be dropped
   until the notification has occurred.  The microcode notification will
   initiate the processing of dropping the link.

	 At that time if there are no more active links or pending links
	 for the subchannel then the subchannel should be halted.
*/
										link->lk_state = LK_CLOSED;
										link->lk_open = 0;
										} 
									}
									/* one link per subchannel per open */
									break; 
							
						}
					}
/*
 * SAB check for links pending associated with other opens on this subchannel
 */
                    for (j=0; j<MAX_LINKS;j++){
                        if ( sc->links[j] )
                            break;
                    }


                   /*
                   ** Send stop when no more links.
                   */
/*d51658*/          if ( j>= MAX_LINKS && lk_open_flag && sc->num_links <= 0 &&
/*d51658*/                  (sc->sc_state == SC_OPEN||sc->sc_state == SC_STARTING)) {                            /*
                           ** Reset and stop the SC
                           */
                           sess_blk.length = 1;
                           sess_blk.netid = i;
						   ENABLE_INTERRUPTS(spl);
                           do_stopsub(ca, &sess_blk);
						   DISABLE_INTERRUPTS(spl);
                           /*
                           ** Write SC stopped
                           ** as part of group
                           */
                           i++;
                   }

               } else if ((link = sc->link[0]) &&
                   (link->lk_open == openp)) { /* 3088 mode */
                   clean_sc_queues(ca, i, 0);
                   link->lk_open = NULL;
                   if (sc->sc_state != SC_CLOSING
                           && !(ca->flags & CATDEAD)) {
                           sess_blk.length = 1;
                           sess_blk.netid = i;
                           sess_blk.status = 0;
						   ENABLE_INTERRUPTS(spl);
                           do_stopsub(ca, &sess_blk);
						   DISABLE_INTERRUPTS(spl);
                   }
               }
           }
        }
} /* ix29279 */
        /*
        ** catmpx() will free the open structure
        ** and associated resources.
        */
        ca->flags &= ~CATDIAG;
        if (ca->flags & CAT_CE_OPEN) {
                ca->flags &= ~CAT_CE_OPEN;
        }
        ENABLE_INTERRUPTS(spl);
        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_CLOSE, 0, dev);
        return 0;
} /* catclose() */
