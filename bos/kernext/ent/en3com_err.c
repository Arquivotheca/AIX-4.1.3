static char sccsid[] = "@(#)46  1.9  src/bos/kernext/ent/en3com_err.c, sysxent, bos411, 9428A410j 5/16/94 15:24:58";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_tx_timeout
 *		en3com_ctl_timeout
 *		en3com_parity_err
 *		en3com_hard_err
 *		en3com_restart
 *		en3com_hard_fail
 *		en3com_stimer
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>


#include "en3com_dds.h"
#include "en3com_mac.h"
#include "en3com_hw.h"
#include "en3com_pio.h"
#include "en3com.h"
#include "en3com_errids.h"


void en3com_hard_err();
void en3com_hard_fail();

/*****************************************************************************/
/*
 * NAME:     en3com_tx_timeout
 *
 * FUNCTION: Ethernet driver transmit watchdog timer timeout routine.
 *	     Try to recover the adapter once.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer function
 *
 * INPUT:
 *      p_wdt	- pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_tx_timeout(
  struct watchdog *p_wdt) 	/* pointer to watchdog timer structure */

{
  en3com_dev_ctl_t   *p_dev_ctl = (en3com_dev_ctl_t *)
			((ulong)p_wdt - offsetof(en3com_dev_ctl_t, tx_wdt));


  TRACE_BOTH(HKWD_EN3COM_OTHER, "ttoB", (ulong)p_wdt, 0, 0);

  en3com_logerr(p_dev_ctl, ERRID_EN3COM_TMOUT, __LINE__, __FILE__, 0, 0, 0);
  en3com_hard_err(p_dev_ctl, FALSE, TRUE, NDD_TX_TIMEOUT);

  TRACE_BOTH(HKWD_EN3COM_OTHER, "ttoE", 0, 0, 0);


}

/*****************************************************************************/
/*
 * NAME:     en3com_ctl_timeout
 *
 * FUNCTION: Ethernet driver ioctl watchdog timer timeout routine.
 *	     Set control operation status and wake up the ioctl routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer function
 *
 * INPUT:
 *      p_wdt	- pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_ctl_timeout(
  struct watchdog *p_wdt) 	/* pointer to watchdog timer structure */

{
  en3com_dev_ctl_t   *p_dev_ctl = (en3com_dev_ctl_t *)
			((ulong)p_wdt - offsetof(en3com_dev_ctl_t, ctl_wdt));


  TRACE_BOTH(HKWD_EN3COM_OTHER, "tcoB", (ulong)p_wdt, 0, 0);

  en3com_logerr(p_dev_ctl, ERRID_EN3COM_TMOUT, __LINE__, __FILE__, 0, 0, 0);
  en3com_hard_err(p_dev_ctl, FALSE, TRUE, NDD_TX_TIMEOUT);

  TRACE_BOTH(HKWD_EN3COM_OTHER, "tcoE", 0, 0, 0);


}
  
/*****************************************************************************/
/*
 * NAME:     en3com_parity_err
 *
 * FUNCTION: Ethernet driver parity error handler.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_intr 
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	ioa		- io address space access handle
 *
 * RETURNS:  
 *	0 - no parity error 
 *	non-zero parity register - parity error
 */
/*****************************************************************************/
en3com_parity_err(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int		ioa)			/* io address space access handle */


{
  uchar parity_reg;
  int pio_rc = 0;



  TRACE_SYS(HKWD_EN3COM_OTHER, "NpaB", (ulong)p_dev_ctl, ioa, 0);

  /*
   * look at the parity control register
   */
  ENT_GETCX(ioa + PARITY_REG, &parity_reg);

  if (pio_rc) {
  	en3com_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Npa1", 1, 0, 0);
	return(0);
  }
	
  TRACE_DBG(HKWD_EN3COM_OTHER, "Npa2", (int)parity_reg, WRK.card_type, 0);

  /*
   * Test if any valid parity error has occurred
   * Version 1 card, any parity bit on
   */
  switch (WRK.card_type) {
	case ADPT_10:
	case ADPT_20:
	case ADPT_22:
	case ADPT_225:
	case ADPT_23:
	case ADPT_235:
		parity_reg &= ANY_ERR_MSK;
		break;

	case ADPT_30:
		parity_reg &= ((WRK.fdbk_intr_en)
			       ? ANY_ERR3_MSK
			       : (ANY_ERR3_MSK & ~FBRT_ERR3_MSK));
		break;

	default:
		break;
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "NpaE", parity_reg, 0, 0);
  return((int)parity_reg);

}

/*****************************************************************************/
/*
 * NAME:     en3com_hard_err
 *
 * FUNCTION: Ethernet driver hardware error handler.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_intr 
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	from_slih	- flag for enterring from process or interrupt level
 *	restart		- flag for restarting the adapter or not
 *	err_code	- error code for status blocks
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_hard_err(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int		from_slih, 		/* flag for process or interrupt */
  int		restart,		/* flag for restart the adapter */
  int		err_code)		/* error reason code */


{
  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);
  struct mbuf	*p_mbuf;
  int ipri_slih;
  int ipri;



  TRACE_BOTH(HKWD_EN3COM_OTHER, "NheB", (ulong)p_dev_ctl, from_slih, restart);
  TRACE_BOTH(HKWD_EN3COM_OTHER, "NheC", err_code, 0, 0);
  
  /*
   * Stop the watchdog timers
   */
  w_stop(&(CTLWDT));
  w_stop(&(TXWDT));

  /* get locks */
  if (!from_slih) {
	ipri_slih = disable_lock(PL_IMP, &SLIH_LOCK);
  }
  
  ipri = disable_lock(PL_IMP, &TX_LOCK);
  disable_lock(PL_IMP, &CMD_LOCK);

  /* wakeup outstanding ioctl event */
  if (p_dev_ctl->ctl_pending) {
  	e_wakeup((int *)&p_dev_ctl->ctl_event);
  }

  /* free up the mbufs sitting in the txq */
  if (p_dev_ctl->txq_len) {
	while (p_mbuf = p_dev_ctl->txq_first) {
		p_dev_ctl->txq_first = p_mbuf->m_nextpkt;
		m_freem(p_mbuf);
  		p_ndd->ndd_genstats.ndd_oerrors++;
	}
	p_dev_ctl->txq_len = 0;
	p_dev_ctl->txq_last = NULL;
  }
  /* all the pending transmit packets will be accounted as errors */
  p_ndd->ndd_genstats.ndd_oerrors += p_dev_ctl->tx_pending;
  
  if (restart && p_dev_ctl->device_state == OPENED) {

	p_dev_ctl->device_state = LIMBO;
  	p_ndd->ndd_flags &= ~NDD_RUNNING;
  	p_ndd->ndd_flags |= NDD_LIMBO;
	if (en3com_restart(p_dev_ctl, err_code)) {
		en3com_hard_fail(p_dev_ctl, err_code);
  		p_dev_ctl->ctl_status = ENETDOWN;
	}
	else {
  		p_dev_ctl->ctl_status = 0;
	}
  }
  else {
	en3com_hard_fail(p_dev_ctl, err_code);
  	p_dev_ctl->ctl_status = ENETDOWN;
  }

  unlock_enable(PL_IMP, &CMD_LOCK);
  unlock_enable(ipri, &TX_LOCK);

  if (!from_slih) {
	unlock_enable(ipri_slih, &SLIH_LOCK);
  }


  TRACE_BOTH(HKWD_EN3COM_OTHER, "NheE", 0, 0, 0);
}

/*****************************************************************************/
/*
 * NAME:     en3com_restart
 *
 * FUNCTION: Ethernet driver network recovery handler.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_hard_err 
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	err_code	- error code for status blocks
 *
 * RETURNS:  
 *	0 - OK
 *      EIO - PIO error occurred during the start
 *      ENOCONNECT - adapter error occurred during the start
 */
/*****************************************************************************/
en3com_restart(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int	err_code)			/* error reason code */


{

  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);
  int ioa, rc; 
  int pio_rc = 0;



  TRACE_SYS(HKWD_EN3COM_OTHER, "NrsB", (ulong)p_dev_ctl, err_code, 0);

  /* pass a status block to demuxer */
  bzero(&stat_blk, sizeof(ndd_statblk_t));
  stat_blk.code = NDD_LIMBO_ENTER;
  stat_blk.option[0] = err_code;
  (*(p_ndd->nd_status))(p_ndd, &stat_blk);

  if (WRK.tx_buf_alocd) {
	/*
         * Cleanup the DMA errors just in case.
         */
	if (WRK.txd[0].flags & BDC_INITED) {
        	rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                        WRK.txd[0].buf, DMA_PSIZE/2, 
			&WRK.txbuf_xmem, WRK.txd[0].dma_io);

        	if (rc != DMA_SUCC) {
                        en3com_logerr(p_dev_ctl, ERRID_EN3COM_DMAFAIL, 
				__LINE__, __FILE__, WRK.dma_channel, 
				WRK.txd[0].buf, DMA_PSIZE/2);
  			TRACE_BOTH(HKWD_EN3COM_ERR, "Nrs0", rc, 0, 0);
		}
        }
  }


  /* Set POS registers. Enable the card */
  if (en3com_setpos(p_dev_ctl)) {
	BUSIO_DET(ioa);

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Nrs1", EIO, 0, 0);
	return(EIO);
  }

  /* save the device start time for statistics */
  p_dev_ctl->dev_stime = lbolt;

  /* set the error recovery initial state */
  p_dev_ctl->limbo_state = LIMBO_RESET;

  /* Get access to the I/O bus to access I/O registers                      */
  ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

  /*
   * Clear any possible pending interrupt,
   * such as parity error.
   */
  ENT_PUTCX(ioa + STATUS_REG, 0);

  /* Hard Reset the adapter to force a known state                          */
  ENT_PUTCX(ioa + CONTROL_REG, HARD_RST_MSK);

  BUSIO_DET(ioa);

  if (pio_rc) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Nrs2", EIO, 0, 0);
	return(EIO);
  }

  /* set system timer to delay 10 ms. */
  STIMER_MS(10);

  TRACE_SYS(HKWD_EN3COM_OTHER, "NrsE", 0, 0, 0);
  return(0);

}
/*****************************************************************************/
/*
 * NAME:     en3com_hard_fail
 *
 * FUNCTION: Set the adapter to DEAD state, notify user and cleanup any
 *	     outstanding transmit/ioctl requests.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_hard_err 
 *	en3com_stimer
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	err_code	- error code for status blocks
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_hard_fail(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int		err_code)		/* error reason code */


{
  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);



  TRACE_BOTH(HKWD_EN3COM_OTHER, "NhfB", (ulong)p_dev_ctl, err_code, 0);
  
  p_dev_ctl->device_state = DEAD;
  p_ndd->ndd_flags &= ~NDD_RUNNING;
  p_ndd->ndd_flags |= NDD_DEAD;
  en3com_logerr(p_dev_ctl, ERRID_EN3COM_FAIL, __LINE__, __FILE__, 0, 0, 0);

  /* de-activate the adapter */
  en3com_stop(p_dev_ctl);

  /* pass a status block to demuxer */
  bzero(&stat_blk, sizeof(ndd_statblk_t));
  stat_blk.code = NDD_HARD_FAIL;
  stat_blk.option[0] = err_code;
  (*(p_ndd->nd_status))(p_ndd, &stat_blk);

  TRACE_BOTH(HKWD_EN3COM_OTHER, "NhfE", 0, 0, 0);

}
/*****************************************************************************/
/*
 * NAME:     en3com_stimer
 *
 * FUNCTION: Timeout function for the system timer used for error recovery.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	system timer
 *
 * INPUT:
 *      p_systimer	- pointer to the timer structure.
 *
 * RETURNS:  
 * 	none.
 */
/*****************************************************************************/
void
en3com_stimer(
  struct trb *p_systimer)	/* pointer to the timer structure */

{

  en3com_dev_ctl_t	*p_dev_ctl = (en3com_dev_ctl_t *)
					(p_systimer->func_data);
  int   bus, ioa;
  int   i;
  uchar host_status_reg;
  uchar host_command_reg;
  int	pio_rc = 0;	/* pio exception code */
  int   rc = 0;		/* error code */
  int   ipri;


  TRACE_SYS(HKWD_EN3COM_OTHER, "tstB", (ulong)p_systimer, (ulong)p_dev_ctl, 0);

  /* get the slih lock */
  ipri = disable_lock(PL_IMP, &SLIH_LOCK);

  /*
   * if the device state is not LIMBO, the device may be closed or dead, or
   * there is something wrong, don't continue the restart procedure
   */
  if (p_dev_ctl->device_state != LIMBO) {
	unlock_enable(ipri, &SLIH_LOCK);
  	TRACE_BOTH(HKWD_EN3COM_ERR, "tst0", p_dev_ctl->device_state, 0, 0);
        return;
  }

  switch (p_dev_ctl->limbo_state) {

	case LIMBO_RESET:
  		ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);
  		/* reset the control register */
  		ENT_PUTCX(ioa + CONTROL_REG, CONTROL_VALUE);
		BUSIO_DET(ioa);

  		if (pio_rc) {
			rc = EIO;
  			TRACE_BOTH(HKWD_EN3COM_ERR, "tst1", EIO, pio_rc, 0);
			break;
  		}

		p_dev_ctl->limbo_state = LIMBO_RESET_DONE;

  		/* set system timer to delay 2 seconds for fully reset */
		STIMER_MS(2000);

		break;
	
	case LIMBO_RESET_DONE:
  		ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);
  		/* Force RAM page register to the default value */
  		ENT_PUTCX(ioa + RAM_PAGE_REG, RAM_PAGE_VALUE);

  		/* perform I/O parity checking */
  		ENT_PUTCX( ioa + PARITY_REG, PAREN_MSK);

   		/* check the self test status */
      		/* get the interrupt status register */ 
      		ENT_GETCX(ioa + STATUS_REG, &host_status_reg);
      		if (host_status_reg & CWR_MSK) {

         		/* get the host command register */
         		ENT_GETCX(ioa + COMMAND_REG, &host_command_reg);
         		if (host_command_reg == SELF_TESTS_OK) {
			  if (pio_rc) {
				BUSIO_DET(ioa);
				rc = EIO;
  				TRACE_BOTH(HKWD_EN3COM_ERR, "tst2", EIO, 
					pio_rc, 0);
				break;
			  }

			  BUSIO_DET(ioa);
			  p_dev_ctl->limbo_state = LIMBO_GET_MBOX_1;

                	  /* set system timer to delay 10 ms. */
			  STIMER_MS(10);

			  break;
			}
		}
		BUSIO_DET(ioa);
		rc = EIO;
  		TRACE_BOTH(HKWD_EN3COM_ERR, "tst3", EIO, pio_rc, 0);
		break;

	case LIMBO_GET_MBOX_1:
  		/* Get the 4 bytes execute mailbox offset  */
  		WRK.exec_mail_box = 0;

  		ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

         	/* get the interrupt status register */
         	ENT_GETCX(ioa + STATUS_REG, &host_status_reg);
         	if (host_status_reg & CWR_MSK) {

            		/* get the host command register */
            		ENT_GETCX(ioa + COMMAND_REG, &host_command_reg);
            		/* Place the value in the proper order */
            		WRK.exec_mail_box = host_command_reg;

                        if (pio_rc) {
                                BUSIO_DET(ioa);
                                rc = EIO;
  				TRACE_BOTH(HKWD_EN3COM_ERR, "tst4", EIO, 
					pio_rc, 0);
                                break;
                        }

                        BUSIO_DET(ioa);
                        p_dev_ctl->limbo_state = LIMBO_GET_MBOX_2;

                        /* set system timer to delay 10 ms. */
                        STIMER_MS(10);

                        break;

		}
		BUSIO_DET(ioa);
		rc = EIO;
  		TRACE_BOTH(HKWD_EN3COM_ERR, "tst5", EIO, pio_rc, 0);
            	break; 

	case LIMBO_GET_MBOX_2:
  		ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

         	/* get the interrupt status register */
         	ENT_GETCX(ioa + STATUS_REG, &host_status_reg);
         	if (host_status_reg & CWR_MSK) {

            		/* get the host command register */
            		ENT_GETCX(ioa + COMMAND_REG, &host_command_reg);
            		/* Place the value in the proper order */
            		WRK.exec_mail_box |= (host_command_reg << 8);

                        if (pio_rc) {
                                BUSIO_DET(ioa);
                                rc = EIO;
  				TRACE_BOTH(HKWD_EN3COM_ERR, "tst6", EIO, 
					pio_rc, 0);
                                break;
                        }
                        BUSIO_DET(ioa);
                        p_dev_ctl->limbo_state = LIMBO_GET_MBOX_3;

                        /* set system timer to delay 10 ms. */
                        STIMER_MS(10);

                        break;

		}
		BUSIO_DET(ioa);
		rc = EIO;
  		TRACE_BOTH(HKWD_EN3COM_ERR, "tst7", EIO, pio_rc, 0);
            	break; 

	case LIMBO_GET_MBOX_3:
  		ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

         	/* get the interrupt status register */
         	ENT_GETCX(ioa + STATUS_REG, &host_status_reg);
         	if (host_status_reg & CWR_MSK) {

            		/* get the host command register */
            		ENT_GETCX(ioa + COMMAND_REG, &host_command_reg);
            		/* Place the value in the proper order */
            		WRK.exec_mail_box |= (host_command_reg << 16);

                        if (pio_rc) {
                                BUSIO_DET(ioa);
                                rc = EIO;
  				TRACE_BOTH(HKWD_EN3COM_ERR, "tst8", EIO, 
					pio_rc, 0);
                                break;
                        }
                        BUSIO_DET(ioa);
                        p_dev_ctl->limbo_state = LIMBO_GET_MBOX_4;

                        /* set system timer to delay 10 ms. */
                        STIMER_MS(10);

                        break;

		}
		BUSIO_DET(ioa);
		rc = EIO;
  		TRACE_BOTH(HKWD_EN3COM_ERR, "tst9", EIO, pio_rc, 0);
            	break; 

	case LIMBO_GET_MBOX_4:
  		bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr);
  		ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

         	/* get the interrupt status register */
         	ENT_GETCX(ioa + STATUS_REG, &host_status_reg);
         	if (host_status_reg & CWR_MSK) {

            		/* get the host command register */
            		ENT_GETCX(ioa + COMMAND_REG, &host_command_reg);
            		/* Place the value in the proper order */
            		WRK.exec_mail_box |= (host_command_reg << 24);

                        if (pio_rc) {
                                BUSIO_DET(ioa);
				BUSMEM_DET(bus);
                                rc = EIO;
  				TRACE_BOTH(HKWD_EN3COM_ERR, "tsta", EIO, 
					pio_rc, 0);
                                break;
                        }

  			/* get and save the adapter initial configuration */
  			if (en3com_getcfg(p_dev_ctl, bus)) {
        			BUSIO_DET(ioa);
        			BUSMEM_DET(bus);

				rc = ENOCONNECT;
        			TRACE_BOTH(HKWD_EN3COM_ERR, "tstb", ENOCONNECT, 
				  0, 0);
				break;
			}
			
			/* more setup work */

  			/* initialize the MIB variables */
  			bzero (&ENTSTATS, sizeof(ENTSTATS));
  			bzero (&DEVSTATS, sizeof(DEVSTATS));
  			bzero (&MIB, sizeof(MIB));

  			/* Save ROS level in the MIB table  */
  			for (i = 0; i < WRK.vpd_ros_length; i++)
      				MIB.Generic_mib.ifExtnsEntry.revware[i] = 
					WRK.vpd_rosl[i];

  			bcopy(ETH_MIB_Intel82586, 
			  (char *)&MIB.Generic_mib.ifExtnsEntry.chipset[0], 
			  CHIPSETLENGTH);

  			WRK.restart_count++;

  			p_dev_ctl->tx_pending = 0;
  			p_dev_ctl->ctl_pending = FALSE;

  			/* Enable adapter command interrupts */
  			ENT_PUTCX( ioa + CONTROL_REG, EN_INTR_MSK );

  			if (pio_rc) {
                                BUSIO_DET(ioa);
				BUSMEM_DET(bus);
                                rc = EIO;
  				TRACE_BOTH(HKWD_EN3COM_ERR, "tstb", EIO, 
					pio_rc, 0);
                                break;
  			}

                        BUSIO_DET(ioa);
			BUSMEM_DET(bus);

  			/* Enable execute Mailbox command interrupts   */
  			if (en3com_cmd(p_dev_ctl, INDICAT_EN, TRUE)) {
        			TRACE_BOTH(HKWD_EN3COM_ERR, "tstc", ENOCONNECT, 
					INDICAT_EN, 0);
				rc = ENOCONNECT;
				break;
			}

                        p_dev_ctl->limbo_state = LIMBO_SET_EN;

                        /* set system timer to delay 10 ms. */
                        STIMER_MS(10);

                        break;

		}
		BUSIO_DET(ioa);
        	BUSMEM_DET(bus);
		rc = EIO;
  		TRACE_BOTH(HKWD_EN3COM_ERR, "tstd", EIO, pio_rc, 0);
            	break; 

	default:
		rc = ENOCONNECT;
  		TRACE_BOTH(HKWD_EN3COM_ERR, "tste", ENOCONNECT, 
			p_dev_ctl->limbo_state, 0);
            	break; 
		
  }

  if (rc) {
        en3com_hard_fail(p_dev_ctl, NDD_ADAP_CHECK);
  }

  unlock_enable(ipri, &SLIH_LOCK);

  TRACE_SYS(HKWD_EN3COM_OTHER, "tstE", 0, 0, 0);

} 

