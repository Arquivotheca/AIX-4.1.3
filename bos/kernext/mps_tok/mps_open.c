static char sccsid[] = "@(#)70  1.12  src/bos/kernext/mps_tok/mps_open.c, sysxmps, bos41J, 9520B_all 5/18/95 11:24:33";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: cfg_pos_regs
 *		mps_act
 *		mps_open
 *		mps_setup
 *		mps_setup_clearup
 *		mps_tx_setup
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
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>

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

/*
 * used for cleanup 
 */
#define SETUP_STATUS1		1
#define SETUP_STATUS2		2
#define SETUP_STATUS3		3
#define SETUP_STATUS4		4
#define SETUP_STATUS5		5
#define SETUP_STATUS6		6
#define SETUP_STATUS7		7

extern int mps_config();
extern struct cdt *cdt_func();
extern mps_dd_ctl_t mps_dd_ctl;
extern token_ring_all_mib_t tok_mib_status;

/*****************************************************************************/
/*
 * NAME:     mps_open
 *
 * FUNCTION: Wildwood driver open routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ndd_open field in the NDD on the NDD chain.
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS: 
 *      0          - successful
 *      ENOCONNECT - unable to connect to network
 *      ENOMEM     - unable to allocate required memory
 */
/*****************************************************************************/
mps_open(
ndd_t  	*p_ndd)		/* pointer to the ndd in the dev_ctl area */

{
  mps_dev_ctl_t  *p_dev_ctl = (mps_dev_ctl_t *)(p_ndd->ndd_correlator);
  ndd_statblk_t  stat_blk;   /* status block */
  int rc, i;
  int ipri;

  /* 
   * Pins the entire driver in memory 
   */
  if (pincode(mps_config)) {
	return(ENOCONNECT);
  }

  TRACE_SYS(MPS_OTHER, "OpnB", (ulong)p_dev_ctl, 0, 0);

  if (p_dev_ctl->device_state != CLOSED) {
        unpincode(mps_config);
        return(EINVAL);
  }

  /*
   * Sets the device state 
   */
  p_dev_ctl->device_state = OPEN_SETUP;

  /* 
   * Saves the ndd start time for statistics 
   */
  WRK.ndd_stime =  WRK.dev_stime = lbolt;

 /*
  *  Setup the component dump table
  */
  lock_write(&DD_LOCK);
  if ((mps_dd_ctl.open_count) == 0) {
	mps_dd_ctl.cdt.count = 0;
        mps_dd_ctl.cdt.head._cdt_magic = DMP_MAGIC;
        strcpy(mps_dd_ctl.cdt.head._cdt_name, MPS_DD_NAME);
        mps_dd_ctl.cdt.head._cdt_len = sizeof(struct cdt_head);
        for (i = 0; i < MPS_CDT_SIZE; i++) {
                mps_dd_ctl.cdt.entry[i].d_len = 0;
                mps_dd_ctl.cdt.entry[i].d_ptr = NULL;
        }

  	cdt_add("dd_ctl",(char *)&mps_dd_ctl, sizeof(mps_dd_ctl_t));
  	dmp_add(cdt_func);
        strcpy(((char *)(&mps_dd_ctl.trace.table[0])), MPS_TRACE_TOP);
        strcpy(((char *)(&mps_dd_ctl.trace.table[MPS_TRACE_SIZE-4])),
                                                        MPS_TRACE_BOT);
  }
  mps_dd_ctl.open_count++;
  cdt_add("dev_ctl", (char *)p_dev_ctl, sizeof(mps_dev_ctl_t));
  lock_done(&DD_LOCK);

  if (!WRK.wdt_inited) {
	/*
  	 * Adds our watchdog timer routine to kernel's list 
	 */
  	while (w_init ((struct watchdog *)(&(TX1WDT))));
  	while (w_init ((struct watchdog *)(&(TX2WDT))));
  	while (w_init ((struct watchdog *)(&(CTLWDT))));
  	while (w_init ((struct watchdog *)(&(HWEWDT))));
  	while (w_init ((struct watchdog *)(&(LIMWDT))));
  	while (w_init ((struct watchdog *)(&(LANWDT))));
  	WRK.wdt_inited = TRUE;
  }

  /* 
   * Clears the counters and initialize the ones that are constant 
   */
  bzero (&NDD.ndd_genstats, sizeof(ndd_genstats_t));
  bzero (&TOKSTATS, sizeof(TOKSTATS));
  bzero (&DEVSTATS, sizeof(DEVSTATS));
  bzero (&MIB, sizeof(MIB));

  bcopy(TR_MIB_IBM16, MIB.Generic_mib.ifExtnsEntry.chipset, CHIPSETLENGTH);

  /*
   * Activates the adapter and allocate all the resources needed
   */
  if (rc = mps_act(p_dev_ctl)) {
  	p_dev_ctl->device_state = CLOSED;
  	NDD.ndd_flags &= ~NDD_RUNNING;
        TRACE_BOTH(MPS_ERR, "opn1", p_dev_ctl, rc, 0);

        lock_write(&DD_LOCK);
        cdt_del("dev_ctl", (char *)p_dev_ctl, sizeof(mps_dev_ctl_t));
        if ((mps_dd_ctl.open_count) == 1) {
  		cdt_del("dd_ctl", (char *)&mps_dd_ctl, sizeof(mps_dd_ctl_t));
                dmp_del(cdt_func);
        }
       	mps_dd_ctl.open_count--;
        lock_done(&DD_LOCK);

        /*
         * Clears the watchdog timer
         */
        if (WRK.wdt_inited) {
                w_clear (&(TX1WDT));
                w_clear (&(TX2WDT));
                w_clear (&(LANWDT));
                w_clear (&(LIMWDT));
                w_clear (&(HWEWDT));
                w_clear (&(CTLWDT));
                WRK.wdt_inited = FALSE;
        }

        unpincode(mps_config);
	return(ENOCONNECT);
  }

  MIB.Token_ring_mib.Dot5Entry.ring_state = TR_MIB_OPENED;

  /* 
   * Updates the NDD flags
   */
  NDD.ndd_flags |= (NDD_UP | NDD_BROADCAST);

  TRACE_SYS(MPS_OTHER, "OpnE", p_dev_ctl, 0, 0);
  return(0);


}

/*****************************************************************************/
/*                                                                           
* NAME: mps_act                                                             
*                                                                           
* FUNCTION: Start the initalization of the adapter: load POS registers and  
*           start the sequence for open the adapter.               
*                                                                           
* EXECUTION ENVIRONMENT:                                                    
*      This routine runs only under the process thread.                     
*                                                                           
* NOTES:                                                                    
*    Input: 
*      p_dev_ctl       - pointer to the dev_ctl area.
*
*    Output: 
*  An adapter open.                                               
*
*    Called From: 
*  mps_open                                                  
*
*    Calls To: 
*  cfg_pos_regs - configure the POS registers.                  
*       mps_setup - Setup the ctl, tx and rx memory area
*       d_init - Get DMA channel id                                  
*       d_unmask - Enable the DMA channel                            
*       d_clear - Free the DMA channel                               
*       w_start - Start the watch dog timer                          
*                                                                           
* RETURN:  
*      0 - OK
*      ENOCONNECT - unable to activate and setup the adapter
*      ENOMEM - unable to allocate required memory
*
*/
/*****************************************************************************/
int mps_act (
register mps_dev_ctl_t  *p_dev_ctl)

{
  int    ioa;
  int    cntl_reg;
  ushort data;
  int    i;                       /* loop counter                      */
  int    rc;                      /* Local return code                 */

  TRACE_SYS(MPS_OTHER, "ActB", p_dev_ctl, 0, 0);
  /* 
   * Set up dma channel 
   */
  if (!WRK.channel_alocd) {
  	/* 
	 * Gets dma channel id by calling d_init                      
         */
  	WRK.dma_channel = d_init(DDS.dma_lvl,MICRO_CHANNEL_DMA, DDS.bus_id);

  	/* 
	 * Checks if d_init worked                                      
	 */
  	if (WRK.dma_channel != DMA_FAIL) {
  		d_unmask(WRK.dma_channel); /* enable the dma channel  */
  		WRK.channel_alocd = TRUE; /* Update the dma_chnl state*/
  	} else {
  		mps_logerr(p_dev_ctl, ERRID_MPS_MEM_ERR, __LINE__, 
  			   __FILE__, TOK_DMA_FAIL, 0, 0);
  		TRACE_BOTH(MPS_ERR, "Act1", p_dev_ctl, ENOCONNECT, 0);
  		return(ENOCONNECT);
  	}
  }

  /* 
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

  /* 
   * Soft reset the adapter to force a known state                     
   */
  PIO_PUTSRX( ioa + BCtl, SOFT_RST_MSK);
  io_delay(1000);
  PIO_PUTSRX( ioa + BCtl, 0x0000);
  WRK.hard_err = FALSE;
  /*
   * set channel check bit in basic control register to override a logic
   * error
   */
  PIO_PUTSRX( ioa + BCtl, CHCK_DISABLE);
  if (WRK.pio_rc) {
        d_clear (WRK.dma_channel);   /* Free the DMA clannel         */
        WRK.channel_alocd = FALSE;   /* Turn off the flag            */
  	BUSIO_DET(ioa);              /* restore I/O Bus              */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,0,FALSE,FALSE,FALSE);
  	TRACE_BOTH(MPS_ERR, "Act2", p_dev_ctl, WRK.pio_rc, 0);
  	return(ENOCONNECT);
  }

  /* 
   * Sets the adapter's POS registers so driver can access the adapter  
   */
  if (rc = cfg_pos_regs(p_dev_ctl)) {
  	TRACE_BOTH(MPS_ERR, "Act4", p_dev_ctl, rc, 0);
  	BUSIO_DET(ioa);              /* restore I/O Bus              */
        d_clear (WRK.dma_channel);   /* Free the DMA clannel           */
        WRK.channel_alocd = FALSE;   /* Turn off the flag              */
  	return(ENOCONNECT);
  }

  /*********************************************************************/
  /* Set up for Control block ,transmit and receive memory eare        */
  /*********************************************************************/
  if (rc = mps_setup(p_dev_ctl))
  {  /* Something went wrong                                           */
  	TRACE_BOTH(MPS_ERR, "Act5", p_dev_ctl, ENOMEM, 0);
  	BUSIO_DET(ioa);              /* restore I/O Bus                */
        d_clear (WRK.dma_channel);   /* Free the DMA clannel           */
        WRK.channel_alocd = FALSE;   /* Turn off the flag              */
  	return(ENOMEM);
  }
  WRK.setup = TRUE;

  PIO_PUTSRX( ioa + MISRMask, MISR_MSK); /* setup the MISR mask register*/
  if (WRK.pio_rc) 
  {
  	BUSIO_DET(ioa);              /* restore I/O Bus                 */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,0,FALSE,FALSE,FALSE);

       /* 
  	* Frees up any resourses that were allocated for the 
  	* receive & transmit
  	*/
        recv_cleanup(p_dev_ctl, MAX_RX_LIST);
        mps_tx1_undo(p_dev_ctl);
        mps_tx2_undo(p_dev_ctl);
        WRK.setup = FALSE;
        d_clear (WRK.dma_channel);   /* Free the DMA clannel	        */
        WRK.channel_alocd = FALSE;   /* Turn off the flag      	        */
  	TRACE_BOTH(MPS_ERR, "Act6", p_dev_ctl, WRK.pio_rc, 0);
  	return(ENOCONNECT);
  }

  if (!WRK.intr_inited) {
	/*
  	 * Adds our interrupt routine to kernel's interrupt chain 
	 */
    	if (rc = i_init ((struct intr *)(&(IHS)))) {
  		TRACE_BOTH(MPS_ERR, "Act7", p_dev_ctl, rc, 0);
  		BUSIO_DET(ioa);              /* restore I/O Bus        */

 		/* 
  		* Frees up any resourses that were allocated for the 
  		* receive & transmit
  		*/
        	recv_cleanup(p_dev_ctl, MAX_RX_LIST);
        	mps_tx1_undo(p_dev_ctl);
        	mps_tx2_undo(p_dev_ctl);
        	WRK.setup = FALSE;
        	d_clear (WRK.dma_channel);   /* Free the DMA clannel   */
        	WRK.channel_alocd = FALSE;   /* Turn off the flag      */
        	return(ENOCONNECT);
    	}
  	WRK.intr_inited = TRUE;
  }

  /*********************************************************************/
  /* 
   * Starts the adapter init sequence - SISR_Mask is edge triggered
   * The response of this will be in INITIALIZATION_COMPLETE in        
   * srb_response function.
   */
  /*********************************************************************/
  PIO_PUTSRX( ioa + SISRMask, SISR_MSK);
  if (WRK.pio_rc) 
  {
  	TRACE_BOTH(MPS_ERR, "Act8", p_dev_ctl, rc, 0);
  	BUSIO_DET(ioa);              /* restore I/O Bus                */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,0,FALSE,FALSE,FALSE);
 
       /* 
  	* Frees up any resourses that were allocated for the 
  	* receive & transmit
  	*/
        recv_cleanup(p_dev_ctl, MAX_RX_LIST);
        mps_tx1_undo(p_dev_ctl);
        mps_tx2_undo(p_dev_ctl);
        WRK.setup = FALSE;
        d_clear (WRK.dma_channel);   /* Free the DMA clannel	       */
        WRK.channel_alocd = FALSE;   /* Turn off the flag      	       */
        i_clear(&(IHS));
        WRK.intr_inited = FALSE;
  	return(ENOCONNECT);
  }

  w_start (&(HWEWDT));               /* start watchdog timer           */

  WRK.ctl_event = EVENT_NULL;
  p_dev_ctl->open_pending = TRUE;
  e_sleep(&WRK.ctl_event, EVENT_SHORT);

  if ((p_dev_ctl->device_state == OPENED) ||
	(p_dev_ctl->device_state == LIMBO)) {
  	TRACE_BOTH(MPS_ERR, "ActE", p_dev_ctl, p_dev_ctl->device_state, 0);
  	BUSIO_DET(ioa);              /* restore I/O Bus                */
        NDD.ndd_flags &= ~(NDD_RUNNING | NDD_LIMBO);
        if (p_dev_ctl->device_state == LIMBO) {
                NDD.ndd_flags |= NDD_LIMBO;
        } else {
                NDD.ndd_flags |= NDD_RUNNING;
        }
  	return (0);
  } else {
  	TRACE_BOTH(MPS_ERR, "Act9", p_dev_ctl, p_dev_ctl->device_state, 0);
  	BUSIO_DET(ioa);              /* restore I/O Bus                */
 
       /* 
  	* Frees up any resourses that were allocated for the 
  	* receive & transmit
  	*/
        recv_cleanup(p_dev_ctl, MAX_RX_LIST);
        mps_tx1_undo(p_dev_ctl);
        mps_tx2_undo(p_dev_ctl);
        WRK.setup = FALSE;
        d_clear (WRK.dma_channel);   /* Free the DMA clannel	       */
        WRK.channel_alocd = FALSE;   /* Turn off the flag      	       */
        i_clear(&(IHS));
        WRK.intr_inited = FALSE;
  	return(ENOCONNECT);
  }

} /* end mps_act                                                             */


/*****************************************************************************
*                                                                           
* NAME: cfg_pos_regs                                                        
*                                                                           
* FUNCTION: Take the information stored in the DDS and set the adapter POS  
*           registers to proper configure.                                  
*                                                                           
* EXECUTION ENVIRONMENT:                                                    
*      This routine runs only under the process thread.                     
*                                                                           
* NOTES:                                                                    
*    Input: 
*      p_dev_ctl       - pointer to the dev_ctl area.
*
*    Output: 
*      POS registers for the selected adapter are initialized.        
*
*    Called From: 
*  mps_act      
*                                             
* RETURN:   
*      0 - OK
*      1 - ERROR
*                                                                           
*****************************************************************************/
int cfg_pos_regs (mps_dev_ctl_t  *p_dev_ctl)
{
  ndd_t  *p_ndd = &(NDD);
  int    iocc, pio_rc;
  int    rc = 0;
  uchar  value1, value2;

  TRACE_SYS(MPS_OTHER, "PosB", p_dev_ctl, 0, 0);
  /* 
   * Gets access to the IOCC to access POS registers                   
   */
  iocc = (int)IOCC_ATT((ulong)DDS.bus_id, (ulong)(IO_IOCC + (DDS.slot << 16)));

 /*
  *   POS2:
  * set PIO addr, card enable and config flag 
  * +-----+-----+-----+-----+-----+-----+-----+-----+
  * |conf |                 PIO               |card |
  * |flag |                 addr              |enabl|
  * +-----+-----+-----+-----+-----+-----+-----+-----+
  *    7     6     5     4     3     2     1     0
  * 
  * For I/O address mapped
  * 	pos2.bit6 = MC.I/O.addr.bit15
  * 	pos2.bit5 = MC.I/O.addr.bit14
  * 	pos2.bit4 = MC.I/O.addr.bit13
  * 	pos2.bit3 = MC.I/O.addr.bit12
  * 	pos2.bit2 = MC.I/O.addr.bit11
  * 	pos2.bit1 = MC.I/O.addr.bit10
  */

  WRK.pos_reg[2] = CONF_FLAG | CARD_ENABLE | (DDS.io_base_addr >> 9);

 /*
  *   POS3:
  *      Ring Speed:
  *              0 = 4Mb
  *              1 = 16Mb
  *
  *      Auto sennse
  *              0 = no
  *              1 = yes
  *
  * +-----+-----+-----+-----+-----+-----+-----+-----+
  * |Ring |Auto |   Intr    |     MCROM       |Rom  |
  * |Speed|Sense|   level   |   BASE ADDR     |enabl|
  * +-----+-----+-----+-----+-----+-----+-----+-----+
  *    7     6     5     4     3     2     1     0
  * For ROM address mapped
  * 	pos3.bit3 = MC.MEM.addr.bit16
  * 	pos3.bit2 = MC.MEM.addr.bit15
  * 	pos3.bit1 = MC.MEM.addr.bit14
  */

 /* 
  * Sets interrupt level 
  */
  if (DDS.intr_level == 11) {
	WRK.pos_reg[3] = 0x30;
  } else if (DDS.intr_level == 10) {
	WRK.pos_reg[3] = 0x20;
  } else if (DDS.intr_level == 3)  {
	WRK.pos_reg[3] = 0x10;
 /*
  * The interrupt level 2 on PS/2 machine was really interrupt level
  * on RS/6000 machine.
  */
  } else if (DDS.intr_level == 9)  { 
	WRK.pos_reg[3] = 0x00;
  } else { /* unknow interrupt level */
  	TRACE_BOTH(MPS_ERR, "Pos1", p_dev_ctl, DDS.intr_level, 0);
	return(1); 
  }

 /* 
  * Sets ring speed 
  */
 if (DDS.ring_speed == SIXTEEN_MBS) {
  	WRK.pos_reg[3] |= 0x80;
  	MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_SIXTEENMEGABIT;
	p_ndd->ndd_flags |= TOK_RING_SPEED_16;
  } else  if (DDS.ring_speed == FOUR_MBS) {
  	MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_FOURMEGABIT;
	p_ndd->ndd_flags |= TOK_RING_SPEED_4;
  } else {
  	WRK.pos_reg[3] |= 0x40; /* autosence */
  	MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_UNKNOWN;
  }

 /* 
  * Sets ROM address 
  */
 WRK.pos_reg[3] |= (((DDS.mem_base_addr >> 14) & 0x7) << 1);

 /*
  *   POS4:
  * proto type:
  *          0 = Token Ring
  *
  * Media type:
  *          0 = STP
  *
  * set Media type, protocol flag, pre-Empt ,bus master enabler
  *  +-----+-----+-----+-----+-----+-----+-----+-----+
  *  |Media|proto|  SCB intr | pre | MC  |selec| BM  |
  *  |Type |type |    level  |empt |parit|Fback|enabl|
  *  +-----+-----+-----+-----+-----+-----+-----+-----+
  *      7        6     5     4     3     2     1     0
  */
  WRK.pos_reg[4] = 0x05;

 /*
  *   POS5:
  * set arbitration level, fairness enable, 
  *  MC Parity enable,
  * +-----+-----+-----+-----+-----+-----+-----+-----+
  * |Chan check |Strea|Fairn|   MC BUS Arbit-       |
  * |& stat. bit|data |disab|      ration level     |
  * +-----+-----+-----+-----+-----+-----+-----+-----+
  *    7     6     5     4     3     2     1     0
  *
  */
  WRK.pos_reg[5] = 0xE0 | DDS.dma_lvl;

  /*
   * Sets the new value for POS Registers                       
   */
  MPS_PUTPOS( iocc + POS5, WRK.pos_reg[5]);
  MPS_PUTPOS( iocc + POS4, WRK.pos_reg[4]);
  MPS_PUTPOS( iocc + POS3, WRK.pos_reg[3]);
  MPS_PUTPOS( iocc + POS2, WRK.pos_reg[2]);

  /*
   * Restores IOCC to previous value - done accessing POS Regs          
   */
  IOCC_DET(iocc);            /* restore IOCC                           */

  TRACE_SYS(MPS_OTHER, "PosE", p_dev_ctl, rc, 0);
  return (rc);
} /* end cfg_pos_regs                                                        */


/*****************************************************************************
*                                                                           
* NAME: mps_setup                                                        
*                
* FUNCTION: Setup the system resources for the adapter to operate.
*                                                                           
* EXECUTION ENVIRONMENT:                                                    
*      This routine runs only under the process thread.                     
*                                                                           
* NOTES:                                                                    
*    Input: 
*      p_dev_ctl       - pointer to the dev_ctl area.
*
*    Called From: 
*  mps_act      
*                                             
*    Called To: 
*  recv_cleanup
*  mps_tx_setup      
*                                             
* RETURN:   
*      0 - OK
*      ENOCONNECT - unable to activate and setup the adapter
*      ENOMEM - unable to allocate required memory
*                                                                           
****************************************************************************/
int mps_setup (mps_dev_ctl_t *p_dev_ctl)
{
  int      rc, i;
  uchar   *tx_list_base;

  TRACE_SYS(MPS_OTHER, "PspB", p_dev_ctl, 0, 0);
  /***********************************************************************/
  /* Allocates a table for multi_address table                           */
  /***********************************************************************/
  WRK.new_multi = (mps_multi_t *)
		xmalloc(sizeof(mps_multi_t), MEM_ALIGN, pinned_heap);
  WRK.free_multi = NULL;

  /* 
   * Checks if memory allocation was successful 
   */
  if (WRK.new_multi == NULL) {
  	TRACE_BOTH(MPS_ERR, "Psp1", p_dev_ctl, 0, 0);
        mps_logerr(p_dev_ctl, ERRID_MPS_MEM_ERR, __LINE__, __FILE__, 0, 0, 0);


  	return(ENOMEM);     /* No! return errno */
  }  /* end if no memory available */

  /*********************************************************************/
  /* Set up for receive                                                */
  /*********************************************************************/
  /* 
   * Gets two pages of memory for Rx descriptor area.  
   * Each Rx descriptor needs to be in 256 bytes boundary. 
   */
  WRK.rx_p_mem_block = (uchar *)xmalloc(RX_DESCRIPTOR_SIZE, DMA_L2PSIZE, 
								pinned_heap);

  /* 
   * Checks if memory allocation was successful 
   */
  if (WRK.rx_p_mem_block == NULL) {
  	TRACE_BOTH(MPS_ERR, "Psp2", p_dev_ctl, 0, 0);
        mps_logerr(p_dev_ctl, ERRID_MPS_MEM_ERR, __LINE__, __FILE__, 0, 0, 0);
	xmfree(WRK.new_multi, pinned_heap);
  	return(ENOMEM);     
  }  /* end if no memory available */

  /* 
   * Set up cross memory descriptor 
   */
  WRK.rx_mem_block.aspace_id   = XMEM_GLOBAL;
  WRK.rx_mem_block.subspace_id = NULL;

  /*  
   * Set up DMA for the adapter control area via d_master 
   */
  d_master(WRK.dma_channel, DMA_READ | DMA_NOHIDE, WRK.rx_p_mem_block,
      	RX_DESCRIPTOR_SIZE, &(WRK.rx_mem_block), (uchar *)DDS.dma_base_addr);

  if ((rc = recv_buf_setup(p_dev_ctl, FALSE))) {
  	/* 
	 * Free up any resourses that were allocated for the receive 
         */

  	TRACE_BOTH(MPS_ERR, "Psp3", p_dev_ctl, rc, 0);
	mps_setup_clearup (p_dev_ctl, SETUP_STATUS2);
        mps_logerr(p_dev_ctl, ERRID_MPS_MEM_ERR, __LINE__, __FILE__, rc, 0, 0);
  	return(ENOMEM);     /* No! return errno */
  }

  /********************************************************************/
  /* Set up for transmit channel 1 -  TX variables, etc               */
  /********************************************************************/
  /* Gets page size chunk of memory */
  if (DDS.priority_tx) {
  	WRK.tx1_p_mem_block = (uchar *) xmalloc(TX1_DESCRIPTOR_SIZE, PGSHIFT, 
								pinned_heap);

  	/* 
   	 * Checks if memory allocation was successful 
    	 */
  	if (WRK.tx1_p_mem_block == NULL) {
        	TRACE_BOTH(MPS_ERR, "Psp4", p_dev_ctl, 0, 0);
		mps_setup_clearup (p_dev_ctl, SETUP_STATUS3);
        	mps_logerr(p_dev_ctl,ERRID_MPS_MEM_ERR,__LINE__,__FILE__,0,0,0);
  		return(ENOMEM);     /* No! return errno */
  	}  /* end if no memory available */

  	/* 
  	 * Set up cross memory descriptor 
  	 */
  	WRK.tx1_mem_block.aspace_id   = XMEM_GLOBAL;
  	WRK.tx1_mem_block.subspace_id = NULL;

  	tx_list_base = (uchar *)(DDS.dma_base_addr + RX_DESCRIPTOR_SIZE);
  	/*
  	 *  Set up DMA for the adapter control area via d_master 
  	 */
  	d_master(WRK.dma_channel, DMA_READ | DMA_NOHIDE, WRK.tx1_p_mem_block,
      		TX1_DESCRIPTOR_SIZE, &(WRK.tx1_mem_block), tx_list_base);
  }

  /***************************************************************************/
  /* Reserve DMA address range for Tx1                                       */
  /***************************************************************************/
  /*
   * Initializes the dma address arrays
   */
  tx_list_base = (uchar *)(DDS.dma_base_addr + TX1_DMA_ADDR_OFFSET);
  for (i = 0; i < MAX_TX_LIST; i++) {
        /*
         * Set up cross memory descriptor
         */
        WRK.tx1_dma_xmemp[i].aspace_id   = XMEM_GLOBAL;
        WRK.tx1_dma_xmemp[i].subspace_id = NULL;

        WRK.tx1_dma_addr[i] = tx_list_base + ((PAGESIZE * 5) * i);
  } /* end of for */

  /*********************************************************************/
  /* Set up for transmit channel 2 -  TX variables, etc                */
  /*********************************************************************/
  /* Gets page size chunk of memory */
  WRK.tx2_p_mem_block = (uchar *) xmalloc(TX2_DESCRIPTOR_SIZE, PGSHIFT, 
								pinned_heap);

  /* 
   * Checks if memory allocation was successful 
   */
  if (WRK.tx2_p_mem_block == NULL) {
  	TRACE_BOTH(MPS_ERR, "Psp6", p_dev_ctl, 0, 0);
	mps_setup_clearup (p_dev_ctl, SETUP_STATUS5);
        mps_logerr(p_dev_ctl, ERRID_MPS_MEM_ERR, __LINE__, __FILE__, rc, 0, 0);
  	return(ENOMEM);     /* No! return errno */
  }  /* end if no memory available */

  /* 
   * Set up cross memory descriptor 
   */
  WRK.tx2_mem_block.aspace_id   = XMEM_GLOBAL;
  WRK.tx2_mem_block.subspace_id = NULL;

  tx_list_base = (uchar *)(DDS.dma_base_addr + TX1_DESCRIPTOR_SIZE +
					       RX_DESCRIPTOR_SIZE);
  /*
   *  Set up DMA for the adapter control area via d_master 
   */
  d_master(WRK.dma_channel, DMA_READ | DMA_NOHIDE, WRK.tx2_p_mem_block,
      		TX2_DESCRIPTOR_SIZE, &(WRK.tx2_mem_block), tx_list_base);

  /*********************************************************************/
  /* Allocated mbuf for transmit 				       */
  /*********************************************************************/
  if ((rc = mps_tx_setup(p_dev_ctl)) != 0)
  { /* Something went wrong, clean up TX & RX variables                */
  	/* 
	 * Frees up any resourses that were allocated for the 
         * receive & transmit
  	 */
  	TRACE_BOTH(MPS_ERR, "Psp8", p_dev_ctl, rc, 0);
	mps_setup_clearup (p_dev_ctl, SETUP_STATUS7);
        mps_logerr(p_dev_ctl, ERRID_MPS_MEM_ERR, __LINE__, __FILE__, rc, 0, 0);
  	return(ENOMEM);     /* No! return errno */
  }

  /***************************************************************************/
  /* Reserve DMA address range for Tx2                                       */
  /***************************************************************************/
  /*
   * Initializes the dma address arrays
   */
  tx_list_base = (uchar *)(DDS.dma_base_addr + TX2_DMA_ADDR_OFFSET);
  for (i = 0; i < MAX_TX_LIST; i++) {
        /*
         * Set up cross memory descriptor
         */
        WRK.tx2_dma_xmemp[i].aspace_id   = XMEM_GLOBAL;
        WRK.tx2_dma_xmemp[i].subspace_id = NULL;

        WRK.tx2_dma_addr[i] = tx_list_base + ((PAGESIZE * 5) * i);
  } /* end of for */

  TRACE_SYS(MPS_OTHER, "StuE", p_dev_ctl, 0, 0);
  return(0);
}

/*****************************************************************************
*
* NAME: mps_setup_clearup
*
* FUNCTION: Frees up any resource that were allocated for the receive &
*	    transemit.
*
* EXECUTION ENVIRONMENT:
*      This routine runs only under the process thread.
*
* NOTES:
*    Input:
*      p_dev_ctl       - pointer to the dev_ctl area.
*      flag	       - state to cleanup. 
*
*    Called From:
*  	mps_setup
*
****************************************************************************/
mps_setup_clearup (mps_dev_ctl_t *p_dev_ctl, 
        	   int	          flag)
{
  int rc = 0;

  TRACE_SYS(MPS_OTHER, "PscB", p_dev_ctl, flag, 0);

 /* 
  * Frees up any resourses that were allocated for the receive & transmit
  */
  switch(flag) {

  case SETUP_STATUS7:
        /* fails allocating memory for Tx pool descriptor */
  	rc = d_complete(WRK.dma_channel, DMA_READ | DMA_NOHIDE, 
			WRK.tx2_p_mem_block, PAGESIZE, &(WRK.tx2_mem_block), 
  		   	(uchar *)DDS.dma_base_addr + RX_DESCRIPTOR_SIZE + 
			TX1_DESCRIPTOR_SIZE);
        if (rc != DMA_SUCC) {
        	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, __FILE__, 
		           WRK.dma_channel, WRK.tx2_p_mem_block, rc);
        } 

  case SETUP_STATUS6:
	/* fails setting up cross memory descriptor for Tx2 */
  	xmfree(WRK.tx2_p_mem_block, pinned_heap);

  case SETUP_STATUS5: 
        if (DDS.priority_tx) {
        	/* fails allocating memory for Tx2 descriptor */
  		rc = d_complete(WRK.dma_channel, DMA_READ | DMA_NOHIDE, 
  		   	WRK.tx1_p_mem_block, PAGESIZE, &(WRK.tx1_mem_block), 
  		   	(uchar *)DDS.dma_base_addr + RX_DESCRIPTOR_SIZE);
        	if (rc != DMA_SUCC) {
        		mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__, 
			   __FILE__, WRK.dma_channel, WRK.tx1_p_mem_block, rc);
        	} 
        } 

  case SETUP_STATUS4: 
        if (DDS.priority_tx) {
		/* fails setting up cross memory descriptor for Tx1 */
  		xmfree(WRK.tx1_p_mem_block, pinned_heap);
        } 

  case SETUP_STATUS3: 
        /* fails allocating memory for Tx1 descriptor */
  	recv_buf_undo(p_dev_ctl, MAX_RX_LIST);

  case SETUP_STATUS2:
        /* fails recv_buf_setup */
  	rc = d_complete(WRK.dma_channel, DMA_READ | DMA_NOHIDE, 
			WRK.rx_p_mem_block, RX_DESCRIPTOR_SIZE, 
			&(WRK.rx_mem_block), (uchar *)DDS.dma_base_addr);
  	if (rc != DMA_SUCC) {
        	mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL, __LINE__,
                   	__FILE__, WRK.dma_channel, WRK.rx_p_mem_block, rc);
  	}

  case SETUP_STATUS1: 
	/* fails setting up cross memory descriptor for Rx */
  	xmfree(WRK.rx_p_mem_block, pinned_heap);
  	xmfree(WRK.new_multi, pinned_heap);
	break;
  }

  TRACE_SYS(MPS_OTHER, "PscE", p_dev_ctl, flag, 0);

}

/****************************************************************************/
/*
* NAME: mps_tx_setup
*
* FUNCTION: Set up the transmit list and Initialize the transmit list
*           variables.
*
* EXECUTION ENVIRONMENT:
*
*      This routine runs only under the process thread.
*
* NOTES:
*
*    Input:
*  p_dev_ctl - pointer to device contril structure
*
*    Output:
*  Transmit list set up, transmit list variables initialized.
*
*    Called From:
*  mps_act
*
* RETURN:  0       - Successful completion
*          ENOBUFS - No Bus Address space available
*/
/****************************************************************************/

int mps_tx_setup(mps_dev_ctl_t  *p_dev_ctl)
{
  int                   rc,ioa, pio_rc;
  uchar                 *addr_p;
  int                   i;            /* Loop Counter                        */

  TRACE_SYS(MPS_TX, "TsuB", p_dev_ctl, 0, 0);

  /*********************************************************************/
  /*
   *  Creates the transmit chain by initializing the pointer to TX1
   *  channel list.  Create both DMA and virtual address lists.
   *  Allocated 256 bytes long for each Tx descriptor
   */
  /*********************************************************************/
  WRK.tx1_list[0] = (tx_list_t *)(DDS.dma_base_addr + RX_DESCRIPTOR_SIZE);
  WRK.tx1_vadr[0] = (tx_list_t *)(WRK.tx1_p_mem_block);
  for (i = 1; i < MAX_TX_LIST; i++) {
        WRK.tx1_list[i] = (tx_list_t *)((int)WRK.tx1_list[i - 1] + 256);
        WRK.tx1_vadr[i] = (tx_list_t *)((int)WRK.tx1_vadr[i - 1] + 256);
  } /* end of for */

  /*
   * Initializes the dma & mbuf pointers arrays
   */
  addr_p = (uchar *)(DDS.dma_base_addr + TX1_BUF_OFFSET);
  for (i = 0; ((i < MAX_TX_LIST) && (DDS.priority_tx)); i++) {

        WRK.tx1_buf[i] = (uchar *)
                xmalloc(H_PAGE, PGSHIFT, pinned_heap);
        if (WRK.tx1_buf[i] == NULL) {
                /*
                 * Fails to allocated a tx buffer memory
                 */
                int j;

                for (j = 0; j < i; j++) {
                        rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                                   WRK.tx1_buf[j], H_PAGE,
                                   &WRK.tx1_xmemp[j], WRK.tx1_addr[j]);
                        if (rc != DMA_SUCC) {
                                mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL,
                                           __LINE__, __FILE__, WRK.dma_channel,
                                           WRK.tx1_buf[j], rc);
                        }

                        xmfree(WRK.tx1_buf[j], pinned_heap);
                        WRK.tx1_buf[j] = NULL;
                        WRK.tx1_addr[j] = NULL;
                }
                NDD.ndd_genstats.ndd_nobufs++;
                TRACE_BOTH(MPS_ERR, "Tsu1", p_dev_ctl, i, 0);
                return(ENOBUFS);
        }

        /*
         * Set up cross memory descriptor
         */
        WRK.tx1_xmemp[i].aspace_id   = XMEM_GLOBAL;
        WRK.tx1_xmemp[i].subspace_id = NULL;

        WRK.tx1_addr[i] = addr_p + (PAGESIZE * i);
        /*
         * Set up the DMA channel for block mode DMA transfer
         */
        d_master(WRK.dma_channel, DMA_WRITE_ONLY, WRK.tx1_buf[i],
                H_PAGE, &WRK.tx1_xmemp[i], WRK.tx1_addr[i]);

  } /* end of for */

  /*********************************************************************/
  /*
   *  Create the transmit chain by initializing the pointer to TX2 channel
   *  list.  Create both DMA and virtual address lists.
   *  Allocated 256 bytes long for each Tx descriptor
   */
  /*********************************************************************/
  WRK.tx2_list[0] = (tx_list_t *)(DDS.dma_base_addr + RX_DESCRIPTOR_SIZE +
                        TX1_DESCRIPTOR_SIZE);
  WRK.tx2_vadr[0] = (tx_list_t *)(WRK.tx2_p_mem_block);
  for (i = 1; i < MAX_TX_LIST; i++) {
        WRK.tx2_list[i] = (tx_list_t *)((int)WRK.tx2_list[i - 1] + 256);
        WRK.tx2_vadr[i] = (tx_list_t *)((int)WRK.tx2_vadr[i - 1] + 256);
  } /* end of for */

  /*
   * Initializes the dma & mbuf pointers arrays
   */
  addr_p = (uchar *)(DDS.dma_base_addr + TX2_BUF_OFFSET);
  for (i = 0; i < MAX_TX_LIST; i++) {
        WRK.tx2_buf[i] = (uchar *)
			xmalloc(H_PAGE, PGSHIFT, pinned_heap);
        if (WRK.tx2_buf[i] == NULL) {
                /*
                 * Fails to allocated a tx buffer memory
                 */
                int j;

                /*
                 * Frees up the TX2 buffer resources
                 */
                for (j = 0; j < i; j++) {
                        rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                                        WRK.tx2_buf[j], H_PAGE,
                                        &WRK.tx2_xmemp[j], WRK.tx2_addr[j]);
                        if (rc != DMA_SUCC) {
                                mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL,
                                           __LINE__, __FILE__, WRK.dma_channel,
                                           WRK.tx2_buf[i], rc);
                        }

                        xmfree(WRK.tx2_buf[j], pinned_heap);
                        WRK.tx2_buf[j] = NULL;
                        WRK.tx2_addr[j] = NULL;
                }

                /*
                 * Frees up the TX1 buffer resources
                 */
                for (i = 0; ((i < MAX_TX_LIST) && (DDS.priority_tx)); i++) {
                        rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                                        WRK.tx1_buf[i], H_PAGE,
                                        &WRK.tx1_xmemp[i], WRK.tx1_addr[i]);
                        if (rc != DMA_SUCC) {
                                mps_logerr(p_dev_ctl, ERRID_MPS_DMAFAIL,
                                           __LINE__, __FILE__, WRK.dma_channel,
                                           WRK.tx1_buf[i], rc);
                        }

                        xmfree(WRK.tx1_buf[i], pinned_heap);
                        WRK.tx1_buf[i] = NULL;
                        WRK.tx1_addr[i] = NULL;
                }

                TRACE_BOTH(MPS_ERR, "Tsu3", p_dev_ctl, ENOBUFS, 0);
                NDD.ndd_genstats.ndd_nobufs++;
                return(ENOBUFS);
        }

        /*
         * Set up cross memory descriptor
         */
        WRK.tx2_xmemp[i].aspace_id   = XMEM_GLOBAL;
        WRK.tx2_xmemp[i].subspace_id = NULL;

        WRK.tx2_addr[i] = addr_p + (PAGESIZE * i);
        /*
         * Set up the DMA channel for block mode DMA transfer
         */
        d_master(WRK.dma_channel, DMA_WRITE_ONLY, WRK.tx2_buf[i],
                 H_PAGE, &WRK.tx2_xmemp[i], WRK.tx2_addr[i]);

  } /* end of for */

  /*
   * Set up variable for Transmit list
   */
  WRK.tx1_elem_next_out = 0;
  WRK.tx1_elem_next_in  = 0;
  WRK.tx1_frame_pending = 0;
  WRK.tx1_buf_next_out  = 0;
  WRK.tx1_buf_next_in   = 0;
  WRK.tx1_buf_use_count = 0;
  WRK.tx1_dma_next_out  = 0;
  WRK.tx1_dma_next_in   = 0;
  WRK.tx1_dma_use_count = 0;
  WRK.tx1_retry         = MAX_TX_LIST;
  p_dev_ctl->txq1_len   = 0;
  p_dev_ctl->txq1_first = NULL;
  p_dev_ctl->txq1_last  = NULL;

  WRK.tx2_elem_next_out = 0;
  WRK.tx2_elem_next_in  = 0;
  WRK.tx2_frame_pending = 0;
  WRK.tx2_buf_next_out  = 0;
  WRK.tx2_buf_next_in   = 0;
  WRK.tx2_buf_use_count = 0;
  WRK.tx2_dma_next_out  = 0;
  WRK.tx2_dma_next_in   = 0;
  WRK.tx2_dma_use_count = 0;
  WRK.tx2_retry         = MAX_TX_LIST;
  p_dev_ctl->txq2_len   = 0;
  p_dev_ctl->txq2_first = NULL;
  p_dev_ctl->txq2_last  = NULL;

  WRK.t_len = 0;

  TRACE_SYS(MPS_TX, "TsuE", p_dev_ctl, 0, 0);
  return(0);
}  /* end function mps_tx_setup                                              */


