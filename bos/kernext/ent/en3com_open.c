static char sccsid[] = "@(#)43  1.17  src/bos/kernext/ent/en3com_open.c, sysxent, bos411, 9431A411a 8/2/94 12:54:25";

/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_open
 *		en3com_setup
 *		en3com_tx_setup
 *		en3com_rv_setup
 *		en3com_tx_free
 *		en3com_rv_free
 *		en3com_rv_start
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
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
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


extern struct cdt *en3com_cdt_func();
extern en3com_dd_ctl_t en3com_dd_ctl;	
extern ethernet_all_mib_t en3com_mib_status;

void en3com_tx_free();
void en3com_rv_free();


/*****************************************************************************/
/*
 * NAME:     en3com_open
 *
 * FUNCTION: Ethernet driver open routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ns_alloc() service
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS:  
 *	0 - OK
 *	ENOCONNECT - unable to connect to network
 *	ENOMEM - unable to allocate required memory
 */
/*****************************************************************************/
en3com_open(
  ndd_t		*p_ndd)		/* pointer to the ndd in the dev_ctl area */

{
  en3com_dev_ctl_t   *p_dev_ctl = (en3com_dev_ctl_t *)(p_ndd->ndd_correlator);
  int rc;
  int ipri;




  /* pin the entire driver in memory */
  if (rc = pincode(en3com_open)) {
	return(ENOMEM);
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "OpnB", (ulong)p_ndd, 0, 0);

  /*
   * Set the device state and NDD flags
   */
  p_dev_ctl->device_state = OPEN_PENDING;
  p_ndd->ndd_flags = NDD_BROADCAST | NDD_SIMPLEX;
#ifdef DEBUG
  p_ndd->ndd_flags |= NDD_DEBUG;
#endif

  /* save the ndd start time for statistics */
  p_dev_ctl->ndd_stime = lbolt;


  lock_write(&DD_LOCK);
  if ((++en3com_dd_ctl.open_count) == 1) {
	en3com_cdt_add("dd_ctl", (char *)&en3com_dd_ctl, 
		sizeof(en3com_dd_ctl_t));
  	dmp_add(en3com_cdt_func);
  }
  en3com_cdt_add("dev_ctl", (char *)p_dev_ctl, 
	sizeof(en3com_dev_ctl_t));
  lock_done(&DD_LOCK);

  /*
   * Activate the adapter and allocate all the resources needed
   */
  if (en3com_start(p_dev_ctl, FALSE)) {
  	p_dev_ctl->device_state = CLOSED;

  	lock_write(&DD_LOCK);
	en3com_cdt_del("en3com_dev_ctl", (char *)p_dev_ctl, 
		sizeof(en3com_dev_ctl_t));
  	if ((--en3com_dd_ctl.open_count) == 0) {
		en3com_cdt_del("en3com_dd_ctl", (char *)&en3com_dd_ctl, 
		  sizeof(en3com_dd_ctl_t));
  		dmp_del(en3com_cdt_func);
	}
  	lock_done(&DD_LOCK);

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Opn2", ENOCONNECT, 0, 0);
	unpincode(en3com_open);
	return(ENOCONNECT);
  }
  else {
	if (rc = en3com_setup(p_dev_ctl, FALSE)) {

		/* undo any resource setup */
		en3com_cleanup(p_dev_ctl);

  		p_dev_ctl->device_state = CLOSED;

  		lock_write(&DD_LOCK);
		en3com_cdt_del("en3com_dev_ctl", (char *)p_dev_ctl, 
			sizeof(en3com_dev_ctl_t));
  		if ((--en3com_dd_ctl.open_count) == 0) {
			en3com_cdt_del("en3com_dd_ctl", (char *)&en3com_dd_ctl,
				 sizeof(en3com_dd_ctl_t));
  			dmp_del(en3com_cdt_func);
		}
  		lock_done(&DD_LOCK);

  		TRACE_BOTH(HKWD_EN3COM_ERR, "Opn3", rc, 0, 0);
		unpincode(en3com_open);
		return(rc);
	}

        /* update the device state if everything went OK */
	ipri = disable_lock(PL_IMP, &SLIH_LOCK);
	if (p_dev_ctl->device_state == OPEN_PENDING) {
		p_dev_ctl->device_state = OPENED;
		p_ndd->ndd_flags |= (NDD_RUNNING | NDD_UP);
		rc = 0;
	}
	else {
		rc = ENOCONNECT;
	}
        unlock_enable(ipri, &SLIH_LOCK);

  	TRACE_SYS(HKWD_EN3COM_OTHER, "OpnE", 0, 0, 0);
	return(rc);
  }


}
  
/*****************************************************************************/
/*
 * NAME:     en3com_setup
 *
 * FUNCTION: Setup the system resources for the adapter to operate.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_open
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *	0 - OK
 *	ENOCONNECT - unable to activate and setup the adapter
 *	ENOMEM - unable to allocate required memory
 */
/*****************************************************************************/
en3com_setup(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to the dev_ctl area */

{


  int    ioa;
  int    iocc;
  int    i;			/* Temp variable for loop counting        */
  int    rc;                   /* Local return code                      */
  int    pio_rc = 0;
  int bus;



  TRACE_SYS(HKWD_EN3COM_OTHER, "AupB", (ulong)p_dev_ctl, 0, 0);

  if (!WRK.intr_inited) {
	/* add our interrupt routine to kernel's interrupt chain */
	if (i_init ((struct intr *)(&(IHS))) != INTR_SUCC) {
  		TRACE_BOTH(HKWD_EN3COM_ERR, "Aup0", ENOCONNECT, 0, 0);
		return(ENOCONNECT);
	}
	WRK.intr_inited = TRUE;
  }

  if (!WRK.tx_wdt_inited) {
        /* add our watchdog timer routine to kernel's list */
	while (w_init ((struct watchdog *)(&(TXWDT))));
        WRK.tx_wdt_inited = TRUE;
  }
  if (!WRK.ctl_wdt_inited) {
        /* add our watchdog timer routine to kernel's list */
	while (w_init ((struct watchdog *)(&(CTLWDT))));
        WRK.ctl_wdt_inited = TRUE;
  }



  /* clear all ndd_genstats and device stats */
  bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
  bzero (&ENTSTATS, sizeof(ENTSTATS));
  bzero (&DEVSTATS, sizeof(DEVSTATS));

  /* clear the MIB counters and initialize the ones that are constant */
  bzero (&MIB, sizeof(MIB));

  /* Save ROS level in the MIB table  */
  for (i = 0; i < WRK.vpd_ros_length; i++)
      MIB.Generic_mib.ifExtnsEntry.revware[i] = WRK.vpd_rosl[i];

  bcopy(ETH_MIB_Intel82586, (char *)&MIB.Generic_mib.ifExtnsEntry.chipset[0],
	CHIPSETLENGTH);

  /* Change the MIB support table depends on the board level */
  if (WRK.vpd_hex_rosl < 0x0E) {
       	  en3com_mib_status.Ethernet_mib.Dot3StatsEntry.s_coll_frames =
        	MIB_NOT_SUPPORTED;
          en3com_mib_status.Ethernet_mib.Dot3StatsEntry.m_coll_frames =
        	MIB_NOT_SUPPORTED;
  }
  WRK.restart_count = 0;
  WRK.promiscuous_count = 0;
  WRK.badframe_count = 0;
  WRK.otherstatus = 0;
  WRK.multi_promis_mode = FALSE;
  WRK.enable_multi = 0;
  WRK.filter_count = 0;
  WRK.multi_count = 0;
  bzero(&WRK.multi_table, sizeof(en3com_multi_t));

  p_dev_ctl->tx_pending = 0;
  p_dev_ctl->ctl_pending = FALSE;
  

  ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

  /* Enable adapter command interrupts                            */
  ENT_PUTCX( ioa + CONTROL_REG, EN_INTR_MSK );

  BUSIO_DET(ioa);

  if (pio_rc) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Aup1", ENOCONNECT, pio_rc, 0);
	return(ENOCONNECT);
  }


  /* Enable execute Mailbox command interrupts                  */
  if (en3com_cmd(p_dev_ctl, INDICAT_EN, FALSE)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Aup2", ENOCONNECT, INDICAT_EN, 0);
	return(ENOCONNECT);
  }

  /* Turn off AL-LOC to enable back-to-back receive frames if new microcode */
  if (WRK.vpd_hex_rosl >= 0xC) {
	if (en3com_cmd(p_dev_ctl, AL_LOC_OFF, FALSE)) {
  		TRACE_BOTH(HKWD_EN3COM_ERR, "Aup3", ENOCONNECT, AL_LOC_OFF, 0);
		return(ENOCONNECT);
  	}
  }

  /* Set up the Receive Packet Filter                           */
  if (en3com_cmd(p_dev_ctl, CONFIGURE, FALSE)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Aup4", ENOCONNECT, CONFIGURE, 0);
	return(ENOCONNECT);
  }

  /* Set up adapter with Network Address                        */
  if (en3com_cmd(p_dev_ctl, SET_ADDRESS, FALSE)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Aup5", ENOCONNECT, SET_ADDRESS, 0);
	return(ENOCONNECT);
  }

  /* Set up adapter the number of transmit & receive list to use.    */
  /* Need to do another report_Config to know where the list are.    */
  /* Get the receive buffer pool size from the dds. Based on that,   */
  /* determine the transmit/receive list size that we need to config */
  /* on the adapter. Also, calculate the number of TCWs we can use   */
  /* based on 2k buffer size. Normally, the TCW memory size should   */
  /* be taken into consideration, too. But with the min/max txd/rvd  */
  /* numbers set, it is not possible to exceed the current TCW       */
  /* memory range.						     */

  WRK.rvd_cnt = DDS.rv_pool_size;

  if (WRK.rvd_cnt < MIN_RVD)
	WRK.rvd_cnt = MIN_RVD;

  if (WRK.rvd_cnt > MAX_RVD)
	WRK.rvd_cnt = MAX_RVD;

  /* The adapter's total number of buffers is rv_list_cnt+ tx_list_cnt. */
  /* Make sure that we don't exceed this limit. 			*/
  /* The adapter requires at least 10 (4 in later ROS version) buffers  */
  /* in each list. 							*/
  if ((WRK.txd_cnt = WRK.rv_list_cnt + WRK.tx_list_cnt - WRK.rvd_cnt) < 
	MIN_TXD) {
	WRK.txd_cnt = MIN_TXD;
	WRK.rvd_cnt = WRK.rv_list_cnt + WRK.tx_list_cnt - WRK.txd_cnt;
  }

  if (WRK.txd_cnt > MAX_TXD)
	WRK.txd_cnt = MAX_TXD;

  /* Setup Receive TCW base memory */
  WRK.rv_tcw_base = (ulong)DDS.tcw_bus_mem_addr;

  /* Setup Transmit TCW Region managememt table base - end of receive base  */
  /* Adjust the tcw base for xmit to a page boundary if the rvd_cnt is odd. */
  /* This is very important, the xmit/recv won't work if boundary is not met */

  WRK.tx_tcw_base = WRK.rv_tcw_base + (WRK.rvd_cnt << (DMA_L2PSIZE - 1));
  if (WRK.rvd_cnt % 2)
	WRK.tx_tcw_base += DMA_PSIZE / 2;

  if (en3com_cmd(p_dev_ctl, CONFIG_LIST, FALSE)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Aup6", ENOCONNECT, CONFIG_LIST, 0);
	return(ENOCONNECT);
  }

  /* get and save the current adapter configuration again */
  if (en3com_cmd(p_dev_ctl, REPORT_CONFIG, FALSE)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Aup7", ENOCONNECT, REPORT_CONFIG, 0);
	return(ENOCONNECT);
  }

  bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr);
  if (en3com_getcfg(p_dev_ctl, bus)) {
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Aup8", ENOCONNECT, 0, 0);
	return(ENOCONNECT);
  }
  BUSMEM_DET(bus);

  /*
   * if the adapter cannot be configured as our list count says, 
   * adjust our list count to what the adapter reports
   */
  if (WRK.rv_list_cnt != WRK.rvd_cnt)
	WRK.rvd_cnt = WRK.rv_list_cnt;
  if (WRK.tx_list_cnt != WRK.txd_cnt)
	WRK.txd_cnt = WRK.tx_list_cnt;
  TRACE_SYS(HKWD_EN3COM_OTHER, "Aup9", WRK.txd_cnt, WRK.rvd_cnt, 0);


  /* Set up adapter with no filters */
  if (en3com_cmd(p_dev_ctl, SET_TYPE_NULL, FALSE)) {
  		TRACE_BOTH(HKWD_EN3COM_ERR, "Aupa", ENOCONNECT, 
			SET_TYPE_NULL, 0);
		return(ENOCONNECT);
  }


  /*
   * set up DMA channel 
   */
  if (!WRK.channel_alocd) {

	/* get dma channel id by calling d_init                             */
	WRK.dma_channel = d_init(DDS.dma_arbit_lvl,MICRO_CHANNEL_DMA,
                                  DDS.bus_id);

	if (WRK.dma_channel != DMA_FAIL) {
		d_unmask(WRK.dma_channel);

		/* Update the state of the dma_channel  */
         	WRK.channel_alocd = TRUE;
	}
	else {
		en3com_logerr(p_dev_ctl, ERRID_EN3COM_DMAFAIL, __LINE__, 
			__FILE__, WRK.dma_channel, 0, 0);
  		TRACE_BOTH(HKWD_EN3COM_ERR, "Aupb", ENOCONNECT, 
			WRK.dma_channel, 0);
		return(ENOCONNECT);
	}
  } 

  /*
   * Set up for transmit buffer descriptors 
   */
  if (rc = en3com_tx_setup(p_dev_ctl)) {

      TRACE_BOTH(HKWD_EN3COM_ERR, "Aupc", rc, 0, 0);
      return(rc);
  }


  /*
   * Set up for receive buffer descriptor
   */
  if (rc = en3com_rv_setup(p_dev_ctl, FALSE)) {

      TRACE_BOTH(HKWD_EN3COM_ERR, "Aupd", rc, 0, 0);
      return(rc);
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "AupE", 0, 0, 0);

  return(0);

} 


/*****************************************************************************/
/*
 * NAME:     en3com_tx_setup
 *
 * FUNCTION: Setup the TCWs, buffers and descriptors for transmit.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_setup
 *	en3com_exec_intr
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *
 * RETURNS:  
 *	0 - OK
 *	ENOMEM - unable to allocate required memory
 *	ENOCONNECT - unable to initialize the buffers
 */
/*****************************************************************************/
en3com_tx_setup(
  en3com_dev_ctl_t	*p_dev_ctl)  /* pointer to the device control area */

{

	int buf_size;
	int i;
	uchar *tbuf;
	uchar *tdma;
	int bus;
	ushort boffset;
	uchar tchar;
	int pio_rc = 0;




	TRACE_SYS(HKWD_EN3COM_OTHER, "AtuB", (ulong)p_dev_ctl, 0, 0);

	/*
	 * allocate space for transmit buffers
	 */

	if (!WRK.tx_buf_alocd) {
		buf_size = WRK.txd_cnt * DMA_PSIZE / 2;
		WRK.tx_buf = xmalloc(buf_size, DMA_L2PSIZE, pinned_heap);
		if (WRK.tx_buf == NULL)
		{
			en3com_logerr(p_dev_ctl, ERRID_EN3COM_NOBUFS, __LINE__,
				__FILE__, 0, 0, 0);
			TRACE_BOTH(HKWD_EN3COM_ERR, "Atu1", ENOMEM, 0, 0);
			return(ENOMEM);
		}

		/*
 	 	* attach cross-memory for the tranmit buffers
         	*/
        	WRK.txbuf_xmem.aspace_id = XMEM_INVAL;
        	if (xmattach(WRK.tx_buf, buf_size, &WRK.txbuf_xmem,
                        SYS_ADSPACE) != XMEM_SUCC)
        	{
                	xmfree(WRK.tx_buf, pinned_heap);
			en3com_logerr(p_dev_ctl, ERRID_EN3COM_NOBUFS, __LINE__,
				__FILE__, 0, 0, 0);
			TRACE_BOTH(HKWD_EN3COM_ERR, "Atu2", ENOMEM, 0, 0);
                	return(ENOMEM);
        	}

	}

	/*
         * Set up the for Transmit list
         * get the offsets for all the transmit buffer descriptors.
	 * initialize the DMA on every transmit buffer.
         */

        bus = (int)BUSMEM_ATT(DDS.bus_id, DDS.bus_mem_addr);

	tbuf = WRK.tx_buf;
	tdma = (uchar *)WRK.tx_tcw_base;
        boffset = WRK.tx_list_off;
	for (i=0; i < WRK.txd_cnt - 1; i++) {
		WRK.txd[i].next = &WRK.txd[i+1];
		WRK.txd[i].buf = tbuf;
		WRK.txd[i].dma_io = tdma;
		tbuf += DMA_PSIZE / 2;
		tdma += DMA_PSIZE / 2;
                WRK.txd[i].bd_off = boffset;
                ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC, buflo)),
                                WRK.txd[i].dma_io);
		if (!WRK.tx_buf_alocd) {
                	d_master(WRK.dma_channel, DMA_WRITE_ONLY,
                        	WRK.txd[i].buf, DMA_PSIZE/2,
                        	&WRK.txbuf_xmem, (char *)WRK.txd[i].dma_io);
		}
		WRK.txd[i].flags = BDC_INITED;
                ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
	}
	/* link the first one to the last one to create a circular list */
	WRK.txd[i].next = &WRK.txd[0];
	WRK.txd[i].buf = tbuf;
	WRK.txd[i].dma_io = tdma;
        WRK.txd[i].bd_off = boffset;
        ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC, buflo)),
                                WRK.txd[i].dma_io);
	if (!WRK.tx_buf_alocd) {
        	d_master(WRK.dma_channel, DMA_WRITE_ONLY,
                        WRK.txd[i].buf, DMA_PSIZE/2,
                        &WRK.txbuf_xmem, (char *)WRK.txd[i].dma_io);
	}
	WRK.txd[i].flags = BDC_INITED;

	WRK.txd_last = &WRK.txd[0];
	WRK.txd_first = WRK.txd_avail = &WRK.txd[1];
		

	/* read the last and first txd setup just for sanity check only */
        ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
        ASSERT(boffset == WRK.tx_list_off);

        ENT_GETCX(bus + WRK.txd[0].bd_off + offsetof(BUFDESC, control), 
		&tchar);
        ASSERT(tchar & EL_BIT_MASK);

	WRK.tx_buf_alocd = TRUE;

        BUSMEM_DET(bus);

	if (pio_rc) {
		TRACE_SYS(HKWD_EN3COM_ERR, "Atu3", ENOCONNECT, pio_rc, 0);
                return(ENOCONNECT);
        }
		

	TRACE_SYS(HKWD_EN3COM_OTHER, "AtuE", 0, 0, 0);
	return(0);

}


/*****************************************************************************/
/*
 * NAME:     en3com_rv_setup
 *
 * FUNCTION: Setup the TCWs, buffers and descriptors for receive.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_setup
 *	en3com_exec_intr
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	at_int_lvl	- flag for from interrupt level
 *
 * RETURNS:  
 *	0 - OK
 *	ENOMEM - unable to allocate required memory
 *	ENOCONNECT - unable to initialize the buffers
 */
/*****************************************************************************/
en3com_rv_setup(
  en3com_dev_ctl_t	*p_dev_ctl,  /* pointer to the device control area */
  int	at_int_lvl)		     /* flag for from interrupt level */

{
	int bus, ioa;
	int mbox, i;
	int buf_size;
	ushort boffset;
	uchar *rbuf, *rdma;
	int pio_rc = 0;
	int rc = 0;


	TRACE_SYS(HKWD_EN3COM_OTHER, "AruB", (ulong)p_dev_ctl, 0, 0);

	if (!WRK.rv_buf_alocd) {
	  	/*
	   	* allocate space for receive buffers
	   	*/
	  	buf_size = WRK.rvd_cnt * DMA_PSIZE / 2;
	  	WRK.rv_buf = xmalloc(buf_size, DMA_L2PSIZE, pinned_heap);
	  	if (WRK.rv_buf == NULL)
	  	{
		  en3com_logerr(p_dev_ctl, ERRID_EN3COM_NOBUFS, __LINE__, 
			__FILE__, 0, 0, 0);
		  TRACE_BOTH(HKWD_EN3COM_ERR, "Aru1", ENOMEM, 0, 0);
		  return(ENOMEM);
	  	}

		/*
 	 	* attach cross-memory for the receive buffers
         	*/
        	WRK.rvbuf_xmem.aspace_id = XMEM_INVAL;
        	if (xmattach(WRK.rv_buf, buf_size, &WRK.rvbuf_xmem,
                        SYS_ADSPACE) != XMEM_SUCC)
        	{
                  xmfree(WRK.rv_buf, pinned_heap);
		  en3com_logerr(p_dev_ctl, ERRID_EN3COM_NOBUFS, __LINE__, 
			__FILE__, ENT_NOBUFS, 0, 0);
		  TRACE_BOTH(HKWD_EN3COM_ERR, "Aru2", ENOMEM, 0, 0);
                  return(ENOMEM);
        	}

	}
	/*
	 * initialize dma and buffer information
         */
        bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr );
        ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port );

	/*
         * Set up recv buffer descriptor pointers for all the buffer
         * descriptors
         */
	rbuf = WRK.rv_buf;
	rdma = (uchar *)WRK.rv_tcw_base;
        boffset = WRK.rv_list_off;
	for (i = 0; i < WRK.rvd_cnt - 1; i++)
	{
		WRK.rvd[i].next = &WRK.rvd[i+1];
		WRK.rvd[i].buf = rbuf;
		WRK.rvd[i].dma_io = rdma;
		rbuf += DMA_PSIZE / 2;
		rdma += DMA_PSIZE / 2;
                WRK.rvd[i].bd_off = boffset;
                ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC,buflo)),
                                        (long)WRK.rvd[i].dma_io);
		if (!WRK.rv_buf_alocd) {
                	d_master(WRK.dma_channel, DMA_READ|DMA_NOHIDE, 
				WRK.rvd[i].buf, DMA_PSIZE/2, &WRK.rvbuf_xmem, 
				WRK.rvd[i].dma_io);
		}
                ENT_PUTCX(bus + boffset + offsetof(BUFDESC,control), 0);
                ENT_PUTCX(bus + boffset + offsetof(BUFDESC, status),  0);
		WRK.rvd[i].flags = BDC_INITED;

                ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
	}

	/* link the first one to the last one to create a circular list */
	WRK.rvd[i].next = &WRK.rvd[0];
	WRK.rvd[i].buf = rbuf;
	WRK.rvd[i].dma_io = rdma;
        WRK.rvd[i].bd_off = boffset;
        ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC,buflo)),
                                        (long)WRK.rvd[i].dma_io);
	if (!WRK.rv_buf_alocd) {
        	d_master(WRK.dma_channel, DMA_READ|DMA_NOHIDE, WRK.rvd[i].buf,
                         DMA_PSIZE/2, &WRK.rvbuf_xmem, WRK.rvd[i].dma_io);
	}
        ENT_PUTSX(bus + boffset, EL_BIT_MASK);
	WRK.rvd[i].flags = BDC_INITED;

	WRK.rvd_first = &WRK.rvd[0];
	WRK.rvd_last = &WRK.rvd[i];

	/* read the last rvd setup for sanity check only */
        ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
        ASSERT(boffset == WRK.rv_list_off);

	WRK.rv_buf_alocd = TRUE;

	/*
	 * start the receiver
	 */
        rc = en3com_rv_start(p_dev_ctl, bus, ioa, at_int_lvl);

        BUSIO_DET(ioa);
        BUSMEM_DET(bus);

	if (pio_rc) {
		TRACE_SYS(HKWD_EN3COM_ERR, "Aru3", ENOCONNECT, pio_rc, 0);
                return(ENOCONNECT);
        }

	TRACE_SYS(HKWD_EN3COM_OTHER, "AruE", rc, 0, 0);
	return(rc);
}


/*****************************************************************************/
/*
 * NAME:     en3com_tx_free
 *
 * FUNCTION: Free up all the resources set up for transmit.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_setup
 *	en3com_cleanup
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_tx_free(
  en3com_dev_ctl_t  *p_dev_ctl)   /* pointer to the device control area */

{
        int rc;
	int i;
        


	TRACE_SYS(HKWD_EN3COM_OTHER, "AtfB", (ulong)p_dev_ctl, 0, 0);

	/*
         * Complete all the DMA operation.
         */
        for (i=0; i<WRK.txd_cnt; i++) {

		if (WRK.txd[i].flags & BDC_INITED) {
                  rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                        WRK.txd[i].buf, DMA_PSIZE/2, 
			&WRK.txbuf_xmem, WRK.txd[i].dma_io);

                  if (rc != DMA_SUCC) {
                        en3com_logerr(p_dev_ctl, ERRID_EN3COM_DMAFAIL, 
				__LINE__, __FILE__, WRK.dma_channel, 
				WRK.txd[i].buf, DMA_PSIZE/2);
		  	TRACE_BOTH(HKWD_EN3COM_ERR, "Atf1", rc, 0, 0);
		  }
		  WRK.txd[i].flags = 0;
		}
        }


	/*
         * free transmit data buffer
         */
        if (WRK.tx_buf) {
                rc = xmdetach((struct xmem *)&WRK.txbuf_xmem);
                assert(rc == 0);
                rc = xmfree(WRK.tx_buf, pinned_heap);
                WRK.tx_buf = NULL;
        }

	WRK.txd_first = WRK.txd_last = WRK.txd_avail = NULL;
	WRK.tx_buf_alocd = FALSE;

	TRACE_SYS(HKWD_EN3COM_OTHER, "AtfE", 0, 0, 0);
}

/*****************************************************************************/
/*
 * NAME:     en3com_rv_free
 *
 * FUNCTION: Free up all the resources for receive descriptors.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *	en3com_cleanup
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
en3com_rv_free(
	en3com_dev_ctl_t  *p_dev_ctl)	/* pointer to device control area */
{
	int i;
	int rc;

	

	TRACE_SYS(HKWD_EN3COM_OTHER, "ArfB", (ulong)p_dev_ctl, 0, 0);

	/*
	 * Free up the DMA buffer resources
	 */
	for (i = 0; i < WRK.rvd_cnt; i++) {
		if (WRK.rvd[i].flags & BDC_INITED) {
			/*
			 * Finish the DMA processing of the buffer
			 */
			rc = d_complete (WRK.dma_channel, DMA_READ|DMA_NOHIDE,
				WRK.rvd[i].buf, DMA_PSIZE/2,
				&WRK.rvbuf_xmem, WRK.rvd[i].dma_io);
			if (rc != DMA_SUCC) {
                        	en3com_logerr(p_dev_ctl, ERRID_EN3COM_DMAFAIL, 
					__LINE__, __FILE__, WRK.dma_channel, 
					WRK.rvd[i].buf, DMA_PSIZE/2);
		  		TRACE_BOTH(HKWD_EN3COM_ERR, "Arf1", rc, 0, 0);
			}
			WRK.rvd[i].flags = 0;
		}
	}

	/*
	 * free the receive buffer area if it was allocated
	 */
	if (WRK.rv_buf)
	{
                rc = xmdetach((struct xmem *)&WRK.rvbuf_xmem);
 		assert(rc == 0);
  		rc = xmfree(WRK.rv_buf, pinned_heap);
		assert(rc == 0);
 		WRK.rv_buf = NULL;
	}

	WRK.rvd_first = WRK.rvd_last = NULL;
	WRK.rv_buf_alocd = FALSE;

	TRACE_SYS(HKWD_EN3COM_OTHER, "ArfE", 0, 0, 0);

} 


/*****************************************************************************/
/*
 * NAME:     en3com_rv_start
 *
 * FUNCTION: Setup the receive mailbox and start the receiver.
 *
 * EXECUTION ENVIRONMENT: process or interrupt.
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_rv_setup
 * 	en3com_rv_intr
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	bus		- bus address access handle
 *	ioa		- io address access handle
 *	got_cmdlock	- flag indicating the caller has got the command lock
 *
 * RETURNS:  
 *	0 - OK
 *	ENOCONNECT - unable to start the receiver
 */
/*****************************************************************************/
en3com_rv_start(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to the device control area */
  int		bus,		/* bus address access handle */
  int		ioa,		/* io address access handle */
  int		got_cmdlock)	/* flag indicating the caller has got the
				   command lock */

{
	int mbox;
	int pio_rc = 0;
	int ipri;
	int i;
	uchar host_status_reg;



	TRACE_SYS(HKWD_EN3COM_OTHER, "ArsB", (ulong)p_dev_ctl, bus, ioa);

	/*
         * setup mailbox for receive
         */
        mbox = bus + WRK.rv_mail_box;
        ENT_PUTSRX(mbox + offsetof( RECVMBOX, status ), 0 );
        ENT_PUTSRX(mbox + offsetof( RECVMBOX, rv_list ), 
		WRK.rvd_first->bd_off);

	/*
         * Issue receive command
         */
	if (!got_cmdlock) {
		ipri = disable_lock(PL_IMP, &CMD_LOCK);

		/* get the status register for this card                */
		for (i = 0; i <= CRR_DELAY; i++) {

      		        ENT_GETCX(ioa + STATUS_REG, &host_status_reg);

      			/* Test if command reg ready */
      			if (host_status_reg & CRR_MSK) {
				break;
			}
			if (i < CRR_DELAY)
				I_DELAYMS(p_dev_ctl, 1);
		}
		if (i > CRR_DELAY) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "Ars1", host_status_reg, 
				0, 0);
			return(ENOCONNECT);
		}

        	ENT_PUTCX(ioa + COMMAND_REG, RX_START );

		unlock_enable(ipri, &CMD_LOCK);
	}
	else {
		/* get the status register for this card                */
		for (i = 0; i <= CRR_DELAY; i++) {

      		        ENT_GETCX(ioa + STATUS_REG, &host_status_reg);

      			/* Test if command reg ready */
      			if (host_status_reg & CRR_MSK) {
				break;
			}
			if (i < CRR_DELAY)
				I_DELAYMS(p_dev_ctl, 1);
		}
		if (i > CRR_DELAY) {
  			TRACE_SYS(HKWD_EN3COM_ERR, "Ars2", host_status_reg, 
				0, 0);
			return(ENOCONNECT);
		}
        	ENT_PUTCX(ioa + COMMAND_REG, RX_START );
	}

  	if (pio_rc) {
  		TRACE_SYS(HKWD_EN3COM_ERR, "Ars3", pio_rc, 0, 0);
		return(ENOCONNECT);
  	}

	TRACE_SYS(HKWD_EN3COM_OTHER, "ArsE", 0, 0, 0);
	return(0);
}
