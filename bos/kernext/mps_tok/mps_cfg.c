static char sccsid[] = "@(#)57  1.6  src/bos/kernext/mps_tok/mps_cfg.c, sysxmps, bos41J, 9520B_all 5/18/95 11:24:36";
/*
 *   COMPONENT_NAME: sysxmps -- Wildwood Token Ring Device Driver
 *
 *   FUNCTIONS: config_init
 *		config_remove
 *		mps_config
 *		mps_get_vpd
 *		mps_initdds
 *
 *   ORIGINS: 27
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

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/atomic_op.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/mbuf.h>
#include <sys/err_rec.h>
#include <sys/ndd.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "mps_dslo.h"
#include "mps_mac.h"
#include "mps_dds.h"
#include "mps_dd.h"
#include "tr_mps_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern int mps_open();
extern int mps_close();
extern int mps_ctl();
extern int mps_output();
extern int mps_intr();
extern void mps_tx1_timeout();
extern void mps_tx2_timeout();
extern void mps_ctl_timeout();
extern void mps_start_timeout();
extern void mps_start_recover();
extern void mps_lan_connected();

void config_remove();

/*************************************************************************/
/*  Global data structures                                               */
/*************************************************************************/

/*
 * The global device driver control area.
 * Initialize the lockl lock (cfg_lock) in the beginning of the structure
 * to LOCK_AVAIL for synchronizing the config commands.
 */
mps_dd_ctl_t mps_dd_ctl = {LOCK_AVAIL};

/*
 * The initialization control flag
 */
int mps_inited = FALSE;

/*****************************************************************************/
/*
 * NAME:     mps_config
 *
 * FUNCTION: Configure entry point of the device driver
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
 *      0      - successful
 *      EINVAL - invalid parameter was passed
 *
 *      CFG_INIT:
 *              EBUSY  - device already configured
 *              EEXIST - device name in use
 *              EINVAL - invalid parameter was passed
 *              EIO    - permanent I/O error
 *              ENOMEM - unable to allocate required memory
 *      CFG_TERM:
 *              EBUSY  - device is still opened
 *              ENODEV - device is not configured
 *      CFG_QVPD:
 *              ENODEV - device is not configured
 *              EIO    - permanent I/O error
 */
/*****************************************************************************/
mps_config(
int     cmd,                          /* command being process             */
struct uio  *p_uio)                   /* pointer to uio structure          */

{

  int rc = 0;                         /* return code                       */
  int i;                              /* loop index                        */
  ndd_config_t  ndd_config;           /* config information                */
  mps_dev_ctl_t *p_dev_ctl = NULL;    /* pointer to dev_ctl area           */
  mps_dev_ctl_t *p_dev, *p_prev;      /* temporary pointer to dev_ctl area */

  /*
   * Use lockl operation to serialize the execution of the config commands.
   */
  if ((rc = lockl(&CFG_LOCK, LOCK_SHORT)) != LOCK_SUCC) {
        return(EBUSY);
  }

  /*
   * Copys in the ndd_config_t structure 
   */
  if (rc = uiomove((caddr_t)&ndd_config,sizeof(ndd_config_t),UIO_WRITE,p_uio)) {
        unlockl(&CFG_LOCK);
  	return(EINVAL);
  }

  if (pincode(mps_config)) {       /* pin the entire driver */
        unlockl(&CFG_LOCK);
        return(ENOMEM);
  }


  /*
   * Use atomic operation to make sure that the initialization is only
   * done the first time this code is entered.
   */
  if (!mps_inited) {
        lock_alloc(&DD_LOCK, LOCK_ALLOC_PIN, MPS_DD_LOCK, -1);
        lock_init(&DD_LOCK, TRUE);
        mps_dd_ctl.p_dev_list = NULL;
        mps_dd_ctl.num_devs = 0;
        mps_dd_ctl.open_count = 0;
        lock_alloc(&TRACE_LOCK, LOCK_ALLOC_PIN, MPS_TRACE_LOCK, -1);
        simple_lock_init(&TRACE_LOCK);
        mps_dd_ctl.trace.next_entry = 0;
        mps_inited = TRUE;
  }

  /*
   * Searchs the device in the dev_list if it is there
   */
  p_dev_ctl = mps_dd_ctl.p_dev_list;
  while (p_dev_ctl) {
    if (p_dev_ctl->seq_number == ndd_config.seq_number) {
        break;
    }
    p_dev_ctl = p_dev_ctl->next;
  }

  TRACE_SYS(MPS_OTHER, "cfgB", p_dev_ctl, cmd, (ulong)p_uio);

  switch(cmd) {
  case CFG_INIT:

        /*
         * Make sure that we don't try to configure too many devices
         */
        if ((p_dev_ctl) || (mps_dd_ctl.num_devs >= TOK_MAX_ADAPTERS)) {
                TRACE_BOTH(MPS_ERR, "cfg1", p_dev_ctl,mps_dd_ctl.num_devs,0);
                rc = EBUSY;
                break;
        }

        /*
         *  Allocates memory for the dev_ctl structure
         */
        p_dev_ctl = (mps_dev_ctl_t *)
		xmalloc(sizeof(mps_dev_ctl_t), MEM_ALIGN, pinned_heap);

        if (!p_dev_ctl) {
                TRACE_BOTH(MPS_ERR, "cfg2", p_dev_ctl, ENOMEM, 0);
                rc = ENOMEM;
                break;
        }
        bzero(p_dev_ctl, sizeof(mps_dev_ctl_t));

        /*
         *  Initializes the locks in the dev_ctl area
         */
        lock_alloc(&TX_LOCK,   LOCK_ALLOC_PIN, MPS_TX_LOCK, 
						ndd_config.seq_number);
        lock_alloc(&CTL_LOCK,  LOCK_ALLOC_PIN, MPS_CTL_LOCK, 
						ndd_config.seq_number);
        lock_alloc(&CMD_LOCK,  LOCK_ALLOC_PIN, MPS_CMD_LOCK, 
						ndd_config.seq_number);
        lock_alloc(&SLIH_LOCK, LOCK_ALLOC_PIN, MPS_SLIH_LOCK, 
						ndd_config.seq_number);

        lock_init(&CTL_LOCK, TRUE);
        simple_lock_init(&TX_LOCK);
        simple_lock_init(&CMD_LOCK);
        simple_lock_init(&SLIH_LOCK);

        /*
         *  Adds the new dev_ctl into the dev_list
         */
        p_dev_ctl->next = mps_dd_ctl.p_dev_list;
        mps_dd_ctl.p_dev_list = p_dev_ctl;
        mps_dd_ctl.num_devs++;

        /*
         *  Copys in the dds for config manager
         */
        if (rc = copyin(ndd_config.dds, &p_dev_ctl->dds, sizeof(mps_dds_t))){
                TRACE_BOTH(MPS_ERR, "cfg3", p_dev_ctl, rc, 0);
                rc = EIO;
                config_remove(p_dev_ctl);
		break;
        }
        p_dev_ctl->seq_number = ndd_config.seq_number;

        if (rc = config_init(p_dev_ctl)) {
                TRACE_BOTH(MPS_ERR, "cfg4", p_dev_ctl, rc, 0);
                rc = EIO;
                config_remove(p_dev_ctl);
        }

         break;

  case CFG_TERM:
        /* 
	 * Checks whether the device exist 
	 */
        if (!p_dev_ctl) {
                TRACE_BOTH(MPS_ERR, "cfg5", p_dev_ctl, 0, 0);
                rc = ENODEV;
                break;
        }

        /*
         * Make sure the device is in CLOSED or DEAD state.
         * Call ns_detach and make sure that it is done without error.
         */
        if ((p_dev_ctl->device_state != CLOSED) || ns_detach(&(NDD))) {
                TRACE_BOTH(MPS_ERR, "cfg6", p_dev_ctl, 0, 0);
                rc = EBUSY;
                break;
        }
        /*
         * Removes the dev_ctl area from the dev_ctl list
         * and free the resources.
         */
        config_remove(p_dev_ctl);

        break;

  case CFG_QVPD:

        if (rc = copyout((caddr_t)&p_dev_ctl->vpd, ndd_config.p_vpd,
                        (int)ndd_config.l_vpd)) {
                TRACE_BOTH(MPS_ERR, "cfg8", p_dev_ctl, rc, 0);
                rc = EFAULT;
        }
        break;

  default:
        rc = EINVAL;
        break;

  }

  TRACE_SYS(MPS_OTHER, "cfgE", p_dev_ctl, rc, 0);

  /* 
   * if we are about to be unloaded, free locks 
   */
  if (!mps_dd_ctl.num_devs) {
    	lock_free(&DD_LOCK);
    	lock_free(&TRACE_LOCK);
    	mps_inited = FALSE;
  }

  unpincode(mps_config);               /* unpin the entire driver */
  unlockl(&CFG_LOCK);
  return (rc);

}


/*****************************************************************************/
/*
 * NAME:     config_init
 *
 * FUNCTION: Perform CFG_INIT function.  Initialize the device control
 *           table and get the adapter VPD data.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_config
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *
 * RETURNS:
 *      0      - successful
 *      EEXIST - device name in use (from ns_attach)
 *      EINVAL - invalid parameter was passed
 *      EIO    - permanent I/O error
 */
/*****************************************************************************/
config_init (
mps_dev_ctl_t       *p_dev_ctl)  /* pointer to the device control area */

{

  int rc;       /* return code */

  TRACE_SYS(MPS_OTHER, "cinB", (ulong)p_dev_ctl, 0, 0);

  /* 
   * Initializes the device & open states 
   */
  p_dev_ctl->device_state = CLOSED;

  /* 
   * Set up the interrupt control structure section 
   */
  IHS.next          = (struct intr *) NULL;
  IHS.handler       = mps_intr;
  IHS.bus_type      = DDS.bus_type;
  IHS.flags         = INTR_MPSAFE;   	/* MP safe driver */
  IHS.level         = DDS.intr_level;
  ASSERT (DDS.intr_priority == PL_IMP);
  IHS.priority      = PL_IMP;           /* MP priority */
  IHS.bid           = DDS.bus_id;

  /* 
   * Save the mps_dd_ctl address 
   */
  p_dev_ctl->ctl_correlator = (int)&mps_dd_ctl;

  /* 
   * Set up the watchdog timer control structure section 
   */
  HWEWDT.func        = mps_start_timeout;
  HWEWDT.restart     = 100;
  HWEWDT.count       = 0;

  LIMWDT.func        = mps_start_recover;
  LIMWDT.restart     = 30;
  LIMWDT.count       = 0;

  LANWDT.func        = mps_lan_connected;
  LANWDT.restart     = 30;
  LANWDT.count       = 0;

  TX1WDT.func        = mps_tx1_timeout;
  TX1WDT.restart     = 10;
  TX1WDT.count       = 0;
  WRK.tx1wdt_setter  = INACTIVE;

  TX2WDT.func        = mps_tx2_timeout;
  TX2WDT.restart     = 10;
  TX2WDT.count       = 0;
  p_dev_ctl->tx_dev  = p_dev_ctl;
  WRK.tx2wdt_setter  = INACTIVE;

  CTLWDT.func        = mps_ctl_timeout;
  CTLWDT.restart     = 10;
  CTLWDT.count       = 0;
  p_dev_ctl->ctl_dev = p_dev_ctl;
 
  /*
   * Performs device-specific initialization and set POS registers.     
   * if this routine returns non-zero, the device can't be configured 
   */
  if (rc = mps_initdds(p_dev_ctl)) {
        mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__, __FILE__,
                         NDD_UCODE_FAIL,rc,0);
        TRACE_BOTH(MPS_ERR, "cin1", p_dev_ctl, rc, 0);
        return(rc);
  }
  NDD.ndd_alias      = DDS.alias;          /* point to the alias name
                                                contained in the dds       */
  NDD.ndd_name       = DDS.dev_name;       /* point to the name contained
                                                in the dds                 */
  NDD.ndd_correlator = (caddr_t)p_dev_ctl; /* put the dev_ctl address here */
  NDD.ndd_addrlen    = CTOK_NADR_LENGTH;
  NDD.ndd_hdrlen     = MPS_HDR_LEN;
  NDD.ndd_physaddr   = WRK.mps_addr;       /* point to network address which
                                                will be determined later   */
  NDD.ndd_mintu      = CTOK_MIN_PACKET;

  /*
   * The NDD.ndd.mtu set here only correct when user does not select the
   * autosence.  If autosence is selected then the value of NDD.ndd.mtu
   * will be reset to the correct one after successfully open to the network
   */
  if (DDS.ring_speed == 1)  {
  	NDD.ndd_mtu        = CTOK_16M_MAX_PACKET;
  } else {
  	NDD.ndd_mtu        = CTOK_4M_MAX_PACKET;
  }
  NDD.ndd_type       = NDD_ISO88025;
  NDD.ndd_flags      = NDD_BROADCAST;
#ifdef DEBUG
  NDD.ndd_flags	    |= NDD_DEBUG;
#endif
  NDD.ndd_open       = mps_open;
  NDD.ndd_output     = mps_output;
  NDD.ndd_ctl        = mps_ctl;
  NDD.ndd_close      = mps_close;
  NDD.ndd_specstats  = (caddr_t)&(TOKSTATS);
  NDD.ndd_speclen    = sizeof(TOKSTATS);

  if (rc = ns_attach(&NDD)) {
        TRACE_BOTH(MPS_ERR, "cin2", p_dev_ctl, rc, 0);
        return(rc);
  }
  TRACE_SYS(MPS_OTHER, "cinE", p_dev_ctl, 0, 0);
  return(0);

}

/*****************************************************************************/
/*
 * NAME:     config_remove
 *
 * FUNCTION: Remove the device resources that have been allocated during
 *           CFG_INIT configuration time.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_config
 *
 * INPUT:
 *      p_dev_ctl       - address of a pointer to the dev control structure
 *
 * RETURNS:
 *  none.
 */
/*****************************************************************************/
void config_remove(
mps_dev_ctl_t  *p_dev_ctl)      /* pointer to the dev_ctl area */

{

  mps_dev_ctl_t *p_dev, *p_prev;

  TRACE_SYS(MPS_OTHER, "crmB", (ulong)p_dev_ctl, 0, 0);

  /*
   * Removes the dev_ctl area from the dev_ctl list
   */
  p_dev = mps_dd_ctl.p_dev_list;
  p_prev = NULL;
  while (p_dev) {
        if (p_dev == p_dev_ctl) {
                if (p_prev) {
                        p_prev->next = p_dev->next;
                }
                else {
                        mps_dd_ctl.p_dev_list = p_dev->next;
                }
                break;
        }
        p_prev = p_dev;
        p_dev = p_dev->next;
  }

  /* 
   * Frees the locks in the dev_ctl area 
   */
  lock_free(&TX_LOCK);
  lock_free(&CTL_LOCK);
  lock_free(&CMD_LOCK);
  lock_free(&SLIH_LOCK);

  mps_dd_ctl.num_devs--;
  xmfree(p_dev_ctl, pinned_heap);       /* Frees the dev_ctl area */

  TRACE_SYS(MPS_OTHER, "crmE", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
* NAME: mps_initdds
*
* FUNCTION: Perform any device-specific initialization of the dds.
*
* EXECUTION ENVIRONMENT:
*      This routine runs only under the process thread.
*
* NOTES:
*    Input: p_dev_ctl - pointer to device control structure.
*
*    Output: An updated of device_specific structure, the Vital Product
*            data and selected network address.
*
*    Called From: config_init
*
*    Calls To: mps_get_vpd
*
* RETURN:  0      - Successful
*          EINVAL - Unable to get enough TCWs to start adapter list
*/
/*****************************************************************************/
int mps_initdds ( register mps_dev_ctl_t *p_dev_ctl)
{
  int    bus;
  int    index;
  int    rc;

  TRACE_SYS(MPS_OTHER, "cdsB", p_dev_ctl, 0, 0);

  /* 
   * Reads the vital product data for this adapter 
   */
  if ((rc = mps_get_vpd(p_dev_ctl) != VPD_VALID)) {
  	TRACE_BOTH(MPS_ERR, "cds1", p_dev_ctl, rc, 0);
        return (EINVAL);
  }

  WRK.intr_inited    = FALSE;
  WRK.wdt_inited     = FALSE;
  WRK.setup          = FALSE;
  WRK.iocc           = TRUE;    /* When WRK.iocc = TRUE then machine is not 
				 * cache consistent.
				 */
  /*
   * Multi address 
   */
  WRK.multi_last   = &WRK.multi_table;
  WRK.free_multi   = NULL;
  WRK.new_multi    = NULL;
  WRK.multi_count  = 0;

  /*
   * Initializes ioctl waiting event 
   */
  WRK.ctl_event = EVENT_NULL;

  /*
   * Initializes receive buffer array index frame status 
   */
  WRK.read_index = 0;
  WRK.mhead      = NULL;
  WRK.mtail      = NULL;

  WRK.ndd_limbo = FALSE;

  /*
   * Functional address 
   */
  FUNCTIONAL.functional[0] = 0xC0;
  FUNCTIONAL.functional[1] = 0x00;
  FUNCTIONAL.functional[2] = 0x00;
  FUNCTIONAL.functional[3] = 0x00;
  FUNCTIONAL.functional[4] = 0x00;
  FUNCTIONAL.functional[5] = 0x00;

  MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_NOPROBLEM;

  /* 
   * Determines which network address to use, either DDS or VPD version 
   */
  if (DDS.use_alt_addr == 0) { 
	/* Use the network address that was passed in the VPD */
	/*
         * Copys the network address 
  	 */
        COPY_NADR (&VPD.vpd[0x10], WRK.mps_addr);
  } else { /* Use the network address that was passed in the DDS */

        /* 
	 * Copys the network address 
	 */
        COPY_NADR (DDS.alt_addr, WRK.mps_addr);
  } /* endif for which network address to use */

  TRACE_SYS(MPS_OTHER, "cdsE", p_dev_ctl, 0, 0);
  return (0);
}

/*****************************************************************************/
/*
* NAME: mps_get_vpd
*
* FUNCTION: Read and store the adapter's Vital Product Data via POS register
*
* EXECUTION ENVIRONMENT:
*      This routine runs only under the process thread.
*
* NOTES:
*    Input: p_dev_ctl - pointer to device control structure.
*
*    Output: Vital Product Data stored in the p_dev_ctl work area along
*            with the status of the VPD.
*
*    Called From: mps_initdds
*
* RETURN:
*  VPD_NOT_READ
*  VPD_VALID
*
* Note : The adapter is NOT NECESSARY to DISABLE after read VPD.  Enable adapter
*         does not means adapter is in operation mode.
*
*/
/*****************************************************************************/
int mps_get_vpd (register mps_dev_ctl_t *p_dev_ctl)
{
  int     iocc, ioa, i, pio_rc;
  ushort  data;

  TRACE_SYS(MPS_OTHER, "cvpB", p_dev_ctl, 0, 0);
  /*
   * Gets access to IOCC to access the POS registers 
   */
  iocc = (int)IOCC_ATT((ulong)DDS.bus_id, (ulong)(IO_IOCC + (DDS.slot << 16)));

  MPS_GETPOS( iocc + POS0, &WRK.pos_reg[0] );
  MPS_GETPOS( iocc + POS1, &WRK.pos_reg[1] );

  VPD.status = VPD_NOT_READ;
  if ((PR0_VALUE == WRK.pos_reg[0]) &&   /* POS 0 = 0xA2      */
      (PR1_VALUE == WRK.pos_reg[1])) {   /* POS 1 = 0x8F      */
       /* MPS adapter detected - read Vital Product Data               */

       /*
        * Enables the adapter 
        *   POS2:
        * set PIO addr, card enable and config flag
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        * |conf |                 PIO               |card |
        * |flag |                 addr              |enabl|
        * +-----+-----+-----+-----+-----+-----+-----+-----+
        *    7     6     5     4     3     2     1     0
        */

        MPS_PUTPOS(iocc+POS2,CONF_FLAG | CARD_ENABLE | (DDS.io_base_addr >> 9));

	/*
         * Gets the vital product data from the adapter 
	 */
        ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
        PIO_PUTSRX(ioa + LAPE, SEG1);
        PIO_PUTSRX(ioa + LAPA, VPD_ADDR);
        for (i = 0; i < MAX_MPS_VPD_LEN/2 ;) {
                PIO_GETSX(ioa + LAPD_I, &data);
                VPD.vpd[i++] = (uchar)(data >> 8);
                VPD.vpd[i++] = (uchar)(data & 0xFF);
        }

        BUSIO_DET(ioa);      /* restore I/O Bus */
  	if (WRK.pio_rc) {
  		TRACE_BOTH(MPS_ERR, "cvp1", p_dev_ctl, WRK.pio_rc, 0);
                mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 0, 0, 0);
	} else {
		/*
        	 * Updates the VPD status 
		 */
        	VPD.length = MAX_MPS_VPD_LEN;
        	VPD.status = VPD_VALID;
	}
  }

  IOCC_DET(iocc);            /* restore IOCC    */

  TRACE_SYS(MPS_OTHER, "cvpE", p_dev_ctl, VPD.status, 0);
  return (VPD.status);
} /* end mps_get_vpd */

