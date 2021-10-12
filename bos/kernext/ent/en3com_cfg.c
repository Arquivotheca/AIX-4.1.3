static char sccsid[] = "@(#)38  1.11  src/bos/kernext/ent/en3com_cfg.c, sysxent, bos411, 9428A410j 5/10/94 09:52:02";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_config
 *		en3com_cfg_init
 *		en3com_cfg_remove
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/atomic_op.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/mbuf.h>
#include <sys/err_rec.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>

#include "en3com_dds.h"
#include "en3com_mac.h"
#include "en3com_hw.h"
#include "en3com.h"



extern int en3com_open();
extern int en3com_close();
extern int en3com_ctl();
extern int en3com_output();
extern int en3com_intr();
extern void en3com_tx_timeout();
extern void en3com_ctl_timeout();
extern void en3com_stimer();


/*************************************************************************/
/*  Global data structures						 */
/*************************************************************************/

/*
 * The global device driver control area. Initialize the lockl lock 
 * (cfg_lock) in the beginning of the structure to LOCK_AVAIL for 
 * synchronizing the config commands.
 */
en3com_dd_ctl_t en3com_dd_ctl = {LOCK_AVAIL};

/*
 * the initialization control flag
 */

int en3com_inited = FALSE;

/*
*  MIB status table - this table defines the MIB variable status returned
*  on MIB query operation.
*/
ethernet_all_mib_t en3com_mib_status = {
/* Generic Interface Extension Table */
        MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* ifExtnsChipSet */
        MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* ifExtnsRevWare */
        MIB_READ_ONLY,                  /* ifExtnsMulticastsTransmittedOks */
        MIB_READ_ONLY,                  /* ifExtnsBroadcastsTransmittedOks */
        MIB_READ_ONLY,                  /* ifExtnsMulticastsReceivedOks */
        MIB_READ_ONLY,                  /* ifExtnsBroadcastsReceivedOks */
        MIB_READ_ONLY,                  /* ifExtnsPromiscuous */
/* Generic Interface Test Table */
        MIB_NOT_SUPPORTED,              /* ifEXtnsTestCommunity */
        MIB_NOT_SUPPORTED,              /* ifEXtnsTestRequestId */
        MIB_NOT_SUPPORTED,              /* ifEXtnsTestType */
        MIB_NOT_SUPPORTED,              /* ifEXtnsTestResult */
        MIB_NOT_SUPPORTED,              /* ifEXtnsTestCode */
/* Generic Receive Address Table */
	MIB_READ_ONLY,			/* RcvAddrTable */
/* Ethernet-like Statistics group */
        MIB_READ_ONLY,                  /* dot3StatsAlignmentErrors */
        MIB_READ_ONLY,                  /* dot3StatsFCSErrors */
        MIB_READ_ONLY,                  /* dot3StatsSingleCollisionFrames */
        MIB_READ_ONLY,                  /* dot3StatsMultipleCollisionFrames */
        MIB_NOT_SUPPORTED,              /* dot3StatsSQETestErrors */
        MIB_NOT_SUPPORTED,              /* dot3StatsDeferredTransmissions */
        MIB_NOT_SUPPORTED,              /* dot3StatsLateCollisions */
        MIB_READ_ONLY,                  /* dot3StatsExcessiveCollisions */
        MIB_READ_ONLY,                  /* dot3StatsInternalMacTransmitErrors */
        MIB_READ_ONLY,                  /* dot3StatsCarrierSenseErrors */
        MIB_READ_ONLY,                  /* dot3StatsFrameTooLongs */
        MIB_READ_ONLY,                  /* dot3StatsInternalMacReceiveErrors */
/* Ethernet-like Collision Statistics group */
   	MIB_NOT_SUPPORTED,              /* dot3CollCount */
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,              /* dot3CollFrequencies */
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED,
        MIB_NOT_SUPPORTED
};

void en3com_cfg_remove();

/*****************************************************************************/
/*
 * NAME:     en3com_config
 *
 * FUNCTION: config entry point of the device driver
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      config method for each adapter detected.
 *
 * INPUT:
 *      cmd             - CFG_INIT, CFG_TERM, CFG_QVPD
 *      p_uio           - uio struct with ndd_config_t structure
 *
 * RETURNS:  
 *	0 - successful
 *	EINVAL - invalid parameter was passed
 *
 *	CFG_INIT:	
 *		EBUSY - device already configed
 *		EEXIST - device name in use
 *		EINVAL - invalid parameter was passed
 *		EIO - permanent I/O error
 *		ENOMEM - unable to allocate required memory
 *	CFG_TERM:	
 *		EBUSY - device in still opened
 *		ENODEV - device is not configed
 *	CFG_QVPD:	
 *		ENODEV - device is not configed
 *		EIO - permanent I/O error
 */
/*****************************************************************************/
en3com_config(
  int 		cmd,		/* command being processed */
  struct uio	*p_uio)		/* pointer to uio structure */

{

  en3com_dev_ctl_t *p_dev_ctl = NULL;	/* point to dev_ctl area */
  int rc = 0;			/* return code */
  int i;			/* loop index */
  ndd_config_t  ndd_config;	/* config information */
  



	
  /*
   * Use lockl operation to serialize the execution of the config commands.
   */

  if ((rc = lockl(&CFG_LOCK, LOCK_SHORT)) != LOCK_SUCC) {
	return(EBUSY);
  }

  if (rc = uiomove((caddr_t) &ndd_config, sizeof(ndd_config_t), UIO_WRITE, 
	p_uio)) {
  	unlockl(&CFG_LOCK);
	return(rc);
  }

  if (rc = pincode(en3com_open)) {		/* pin the entire driver */
  	unlockl(&CFG_LOCK);
	return(ENOMEM);
  }
	
  if (!en3com_inited) {
  	lock_alloc(&DD_LOCK, LOCK_ALLOC_PIN, XENT_DD_LOCK, -1);
	lock_init(&DD_LOCK, TRUE);
	en3com_dd_ctl.p_dev_list = NULL;
	en3com_dd_ctl.num_devs = 0;
	en3com_dd_ctl.open_count = 0;
  	lock_alloc(&TRACE_LOCK, LOCK_ALLOC_PIN, XENT_TRACE_LOCK, -1);
	simple_lock_init(&TRACE_LOCK);
	en3com_dd_ctl.trace.next_entry = 0;
	en3com_inited = TRUE;
  }

	
  TRACE_SYS(HKWD_EN3COM_OTHER, "cfgB", cmd, (ulong)p_uio, 0);
  
			
  /*
   * find the device in the dev_list if it is there 
   */
  p_dev_ctl = en3com_dd_ctl.p_dev_list;
  while (p_dev_ctl) {
	if (p_dev_ctl->seq_number == ndd_config.seq_number) 
		break;
	p_dev_ctl = p_dev_ctl->next;
  }

  switch(cmd) {
	case CFG_INIT:
		if (p_dev_ctl) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg1", EBUSY, 0, 0);
			rc = EBUSY;
			break;
		}

		/*
		 * Make sure that we don't try to config too many devices 
		 */
		if (en3com_dd_ctl.num_devs >= EN3COM_MAX_ADAPTERS) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg2", EBUSY, 0, 0);
			rc = EBUSY;
			break;
		}

		/*
		 * Allocate memory for the dev_ctl structure 
		 */
		p_dev_ctl = xmalloc(sizeof(en3com_dev_ctl_t), MEM_ALIGN, 
			pinned_heap);
		
		if (!p_dev_ctl) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg3", ENOMEM, 0, 0);
			rc = ENOMEM;
			break;
		}
		bzero(p_dev_ctl, sizeof(en3com_dev_ctl_t));
			
		/*
		 *  Initialize the locks in the dev_ctl area
		 */
  		lock_alloc(&CTL_LOCK, LOCK_ALLOC_PIN, XENT_CTL_LOCK, 
			ndd_config.seq_number);
  		lock_alloc(&CMD_LOCK, LOCK_ALLOC_PIN, XENT_CMD_LOCK, 
			ndd_config.seq_number);
  		lock_alloc(&TX_LOCK, LOCK_ALLOC_PIN, XENT_TX_LOCK, 
			ndd_config.seq_number);
  		lock_alloc(&SLIH_LOCK, LOCK_ALLOC_PIN, XENT_SLIH_LOCK, 
			ndd_config.seq_number);

		lock_init(&CTL_LOCK, TRUE);
		simple_lock_init(&CMD_LOCK);
		simple_lock_init(&TX_LOCK);
		simple_lock_init(&SLIH_LOCK);
		
		/*
		 * Add the new dev_ctl into the dev_list 
		 */
		p_dev_ctl->next = en3com_dd_ctl.p_dev_list;
		en3com_dd_ctl.p_dev_list = p_dev_ctl;
		en3com_dd_ctl.num_devs++;

		/*
		 *  Copy in the dds for config manager
		 */
		if (copyin(ndd_config.dds, &p_dev_ctl->dds, 
			sizeof(en3com_dds_t))) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg4", EIO, 0, 0);
                        rc = EIO;
			en3com_cfg_remove(p_dev_ctl);
			break;
		}
		p_dev_ctl->seq_number = ndd_config.seq_number;

		/*
         	 *  Setup the component dump table
         	 */
		if (en3com_dd_ctl.num_devs == 1) {
		  en3com_dd_ctl.cdt.count = 0;
		  en3com_dd_ctl.cdt.head._cdt_magic = DMP_MAGIC;
		  strcpy(en3com_dd_ctl.cdt.head._cdt_name, EN3COM_DD_NAME);
		  en3com_dd_ctl.cdt.head._cdt_len = sizeof(struct cdt_head);
		  for (i = 0; i < EN3COM_CDT_SIZE; i++) {
		  	en3com_dd_ctl.cdt.entry[i].d_len = 0;
		  	en3com_dd_ctl.cdt.entry[i].d_ptr = NULL;
		  }
		}

		if (rc = en3com_cfg_init(p_dev_ctl)) {
			en3com_cfg_remove(p_dev_ctl);
		}
		
		break;

	case CFG_TERM:
		/* Does the device exist? */
		if (!p_dev_ctl) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg5", ENODEV, 0, 0);
			rc = ENODEV;
			break;
		}
		
		/*
		 * Make sure the device is in CLOSED state.
		 * Call ns_detach and make sure that it is done
		 * without error.
		 */
		if (p_dev_ctl->device_state != CLOSED || ns_detach(&(NDD))) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg6", EBUSY, 0, 0);
			rc = EBUSY;
			break;
		}
		/*
		 * Remove the dev_ctl area from the dev_ctl list
		 * and free the resouces.
		 */
		en3com_cfg_remove(p_dev_ctl);
		break;

 	case CFG_QVPD:
		/* Does the device exist? */
		if (!p_dev_ctl) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg7", ENODEV, 0, 0);
			rc = ENODEV;
			break;
		}

                if (copyout((caddr_t)&p_dev_ctl->vpd, ndd_config.p_vpd, 
			(int)ndd_config.l_vpd)) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "cfg8", EIO, 0, 0);
                        rc = EIO;
		}
		break;

	default:
  		TRACE_SYS(HKWD_EN3COM_ERR, "cfg9", EINVAL, 0, 0);
		rc = EINVAL;


  }
  /* if we are about to be unloaded, free locks */
  if (!en3com_dd_ctl.num_devs) {
	lock_free(&DD_LOCK);
	lock_free(&TRACE_LOCK);
	en3com_inited = FALSE;
  }
  TRACE_SYS(HKWD_EN3COM_OTHER, "cfgE", rc, 0, 0);

  unpincode(en3com_open);		/* unpin the entire driver */
  unlockl(&CFG_LOCK);
  return (rc);

}


/*****************************************************************************/
/*
 * NAME:     en3com_cfg_init
 *
 * FUNCTION: perform CFG_INIT function. Initialize the device control 
 *	     table and get the adapter VPD data.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_config
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *
 * RETURNS:  
 *	0 - OK
 *	EEXIST - device name in use (from ns_attach)
 *	EINVAL - invalid parameter was passed
 *	EIO - permanent I/O error
 */
/*****************************************************************************/
en3com_cfg_init (
  en3com_dev_ctl_t       *p_dev_ctl)  /* pointer to the device control area */

{

  int rc;		/* return code */


  TRACE_SYS(HKWD_EN3COM_OTHER, "cinB", (ulong)p_dev_ctl, 0, 0);


  p_dev_ctl->device_state = CLOSED;

  /* set up the interrupt control structure section */
  IHS.next = (struct intr *) NULL;
  IHS.handler = en3com_intr;
  IHS.bus_type = DDS.bus_type;
  IHS.flags = INTR_MPSAFE;	/* MP safe driver */
  IHS.level = DDS.intr_level;
  IHS.priority = PL_IMP;	/* MP priority */
  IHS.bid = DDS.bus_id;

  /* save the en3com_dd_ctl address */
  p_dev_ctl->ctl_correlator = (int)&en3com_dd_ctl;

  /* set up the watchdog timer control structure section */
  TXWDT.func = en3com_tx_timeout;
  TXWDT.restart = 10;
  TXWDT.count = 0;

  CTLWDT.func = en3com_ctl_timeout;
  CTLWDT.restart = 10;
  CTLWDT.count = 0;

  /* set up the system timer for error recovery */
  p_dev_ctl->systimer = (struct trb *)talloc(); /* get timer structure */
  p_dev_ctl->systimer->flags &= ~(T_ABSOLUTE);  /* want incremental timer */
  p_dev_ctl->systimer->ipri = PL_IMP;	/* MP priority   */
  p_dev_ctl->systimer->func = (void (*)())en3com_stimer;  
  p_dev_ctl->systimer->func_data = (ulong)p_dev_ctl;   


  NDD.ndd_name = DDS.lname; /* point to the name contained in the dds */
  NDD.ndd_alias = DDS.alias; /* point to the alias contained in the dds */


  /* save the dev_ctl address in the NDD correlator field */
  NDD.ndd_correlator = (caddr_t)p_dev_ctl;  
  NDD.ndd_addrlen = ENT_NADR_LENGTH;
  NDD.ndd_hdrlen = ENT_HDR_LEN;
  NDD.ndd_physaddr = WRK.net_addr;	/* point to network address which will 
					   be determined later */
  NDD.ndd_mtu = ENT_MAX_MTU;
  NDD.ndd_mintu = ENT_MIN_MTU;
  NDD.ndd_type = NDD_ISO88023;
  NDD.ndd_flags = NDD_BROADCAST | NDD_SIMPLEX;
#ifdef DEBUG
  NDD.ndd_flags |= NDD_DEBUG;
#endif
  NDD.ndd_open = en3com_open;
  NDD.ndd_output = en3com_output;
  NDD.ndd_ctl = en3com_ctl;
  NDD.ndd_close = en3com_close;
  NDD.ndd_specstats = (caddr_t)&(ENTSTATS);
  NDD.ndd_speclen = sizeof(ENTSTATS);

  /* perform device-specific initialization */
  /* if this routine returns non-zero, the device can't be configured */
  if (rc = en3com_init(p_dev_ctl)) {
  	TRACE_SYS(HKWD_EN3COM_ERR, "cin1", rc, 0, 0);
	return(rc);
  }

  /* add the device to the NDD chain */
  if (rc = ns_attach(&NDD)) {
  	TRACE_SYS(HKWD_EN3COM_ERR, "cin2", rc, 0, 0);
	return(rc);
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "cinE", 0, 0, 0);
  return(0);

}

/*****************************************************************************/
/*
 * NAME:     en3com_cfg_remove
 *
 * FUNCTION: remove the device resources that has been allocated during
 *	     CFG_INIT configuration time.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_config
 *
 * INPUT:
 *      p_dev_ctl	- address of a pointer to the dev control structure
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
en3com_cfg_remove(
  en3com_dev_ctl_t	*p_dev_ctl)	/* point to the dev_ctl area */

{

  en3com_dev_ctl_t *p_dev, *p_prev;


  TRACE_SYS(HKWD_EN3COM_OTHER, "crmB", (ulong)p_dev_ctl, 0, 0);

  /*
   * Remove the dev_ctl area from the dev_ctl list
   */
  p_dev = en3com_dd_ctl.p_dev_list;
  p_prev = NULL;
  while (p_dev) {
	if (p_dev == p_dev_ctl) {
		if (p_prev) {
			p_prev->next = p_dev->next;
		}
		else {
			en3com_dd_ctl.p_dev_list = p_dev->next;
		}
		break;
	}
	p_prev = p_dev;
	p_dev = p_dev->next;
  }

  /* free the locks in the dev_ctl area */
  lock_free(&CTL_LOCK);
  lock_free(&CMD_LOCK);
  lock_free(&TX_LOCK);
  lock_free(&SLIH_LOCK);

  /* give back the system timer */
  tfree(p_dev_ctl->systimer);

  en3com_dd_ctl.num_devs--;
  xmfree(p_dev_ctl, pinned_heap);	/* free the dev_ctl area */

					
  TRACE_SYS(HKWD_EN3COM_OTHER, "crmE", 0, 0, 0);


}
