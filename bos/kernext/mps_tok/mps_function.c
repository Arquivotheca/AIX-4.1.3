static char sccsid[] = "@(#)65  1.10  src/bos/kernext/mps_tok/mps_function.c, sysxmps, bos41J, 9520B_all 5/18/95 11:24:43";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: config_hp_channel
 *		modify_receive_options
 *		open_adapter
 *		read_adapter_log
 *		set_functional_address
 *		set_group_address
 *		srb_response
 *		unset_group_address
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
#include "mps_cmd.h"
#include "tr_mps_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern mps_dd_ctl_t mps_dd_ctl;
extern int mps_config();
extern struct cdt *cdt_func();

/******************************************************************************/
/*
* NAME: config_hp_channel
*
* FUNCTION:  This function tells the adapter to configure a high priority
*            transmit channel (Maunakea's transmit channel 1).
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl i		- pointer to device control structure.
*       ioa          		- I/O bus address
*       access_priority       	- Token access priority
*
*    Called From: 
*  	mps_ctl
*
*    Calls To: 
*
* RETURN:  0        = Successful completion
*          ENETDOWN = Error return
*/
/******************************************************************************/
void config_hp_channel (
  register mps_dev_ctl_t  *p_dev_ctl,
  register int         	ioa,
  uchar      	        access_priority) {

  ushort        byte_0_1 = CONFIGURE_HP_CHANNEL;
  ushort        byte_2_3 = 0x00FF;
  int 	        ipri, rc =0;

  TRACE_SYS(MPS_OTHER, "IchB", p_dev_ctl, 0, 0);
  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  PIO_PUTSRX(ioa + LAPD_I, byte_0_1);
  PIO_PUTSRX(ioa + LAPD_I, byte_2_3);
  PIO_PUTCX (ioa + LAPD,   access_priority);
  if (!WRK.pio_rc) {
  	PIO_PUTCX (ioa + LISR_SUM, SRB_CMD); /* SUM.LISR.bit5 0010 0000      */
  }

  if (WRK.pio_rc) {
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0,FALSE,FALSE,TRUE);
  }

  TRACE_SYS(MPS_OTHER, "IchE", p_dev_ctl, rc, WRK.pio_rc);
  unlock_enable(ipri, &CMD_LOCK);

} /* end of config_hp_channel */

/******************************************************************************/
/*
* NAME: modify_receive_options
*
* FUNCTION: Modify the receive channel options specified with the open command.
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input: 
*       p_dev_ctl  - pointer to device control structure.
*       rcv_op     - receive modify option
*       at_int_lvl - flag for running at interrupt or process level
*
*    Called From: 
*  	mps_ctl
*
* RETURN:  0        = Successful completion 
*          ENETDOWN = Error return
*/
/******************************************************************************/
int modify_receive_options (
register mps_dev_ctl_t       	*p_dev_ctl,
ushort  			rcv_op,
int  				at_int_lvl)
{

  mod_recv_op   parm;
  ushort        *parm_s = (ushort *)&parm;
  register int  ioa, i, x;
  ushort        id, rc = 0;
  int 	        ipri;

  TRACE_SYS(MPS_OTHER, "ImrB", p_dev_ctl, rcv_op, at_int_lvl);
  bzero(parm_s, sizeof(mod_recv_op));

  parm.cmd     = MODIFY_RECEIVE_OPTION;
  parm.retcode = 0xFF;
  parm.recv_op = rcv_op;

  parm.passwd[0] = PASSWD_0;
  parm.passwd[1] = PASSWD_1;
  parm.passwd[2] = PASSWD_2;
  parm.passwd[3] = PASSWD_3;
  parm.passwd[4] = PASSWD_4;
  parm.passwd[5] = PASSWD_5;
  parm.passwd[6] = PASSWD_6;
  parm.passwd[7] = PASSWD_7;

  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  if (p_dev_ctl->device_state != OPENED) {
  	unlock_enable(ipri, &CMD_LOCK);
	return (ENETDOWN);
  }

  /*
   * Gets access to the I/O bus to access I/O registers                  
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < sizeof(mod_recv_op)/2; i++) {
  	PIO_PUTSX(ioa + LAPD_I, *(parm_s + i));
  }
  if (!WRK.pio_rc) {
  	PIO_PUTCX(ioa + LISR_SUM, SRB_CMD); /* SUM.LISR.bit5 0010 0000*/
  }

  if (WRK.pio_rc) {
  	TRACE_BOTH(MPS_ERR, "Imr4", p_dev_ctl, WRK.pio_rc, 0);
       	mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, FALSE, FALSE, 
					TRUE);
  	BUSIO_DET(ioa);
  	unlock_enable(ipri, &CMD_LOCK);
	return (ENETDOWN);
  }

  if (!at_int_lvl) {
  	w_start(&CTLWDT);

        /*
         * go to sleep with the cmd_slock freed up/regained.
         * Note that the ctl_lock remained locked during the whole time
         */
        WRK.ctl_event = EVENT_NULL;
  	p_dev_ctl->ctl_pending = TRUE;
        e_sleep_thread(&WRK.ctl_event, &CMD_LOCK, LOCK_HANDLER);
  	rc = WRK.retcode[MODIFY_RECEIVE_OPTION];

  } else {
	io_delay(5000);
  }
  BUSIO_DET(ioa);
  TRACE_SYS(MPS_OTHER, "ImrE", p_dev_ctl, rc, at_int_lvl);
  unlock_enable(ipri, &CMD_LOCK);
  return (rc);


} /* end of modify_receive_options */

/******************************************************************************/
/*
* NAME: open_adapter : Ready the adapter for normal comunication.
*
* FUNCTION: 
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl - pointer to device control structure.
*       ioa       - I/O bus address
*
*    Output: 
*  Connect to the network.
*   	
*    Called From: 
*  	mps_ctl
*
* RETURN:  0 = Successful completion 
*          ENETDOWN = Error return
* 
*/
/******************************************************************************/
void  open_adapter (
register mps_dev_ctl_t  *p_dev_ctl,
register int         	ioa)
{

  struct {
  	uchar        cmd;
  	uchar        reserved6[6];
  	uchar        option2;
  	ushort       option1;
  	uchar        reserved2[2];
  	uchar        node_address[6];
  	uchar        group_address[6];
  	uchar        func_address[4];
  	ushort       trb_buffer_length;
  	uchar        rcv_channel_options;
  	uchar        number_local_addr;
  	uchar        product_id[18];
  } o_parm;

  ushort        *parm = (ushort *)&o_parm;
  ushort        id;
  register int  i, j, x, rc, ipri;

  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  TRACE_SYS(MPS_OTHER, "IopB", p_dev_ctl, 0, 0);

  bzero(parm, sizeof(o_parm));
  o_parm.cmd = OPEN_ADAPTER;

  if (DDS.attn_mac) {
  	/* set TOK ATTENTION MAC */
  	o_parm.option1 |= ATTENTION;
     	NDD.ndd_flags |= TOK_ATTENTION_MAC;
  }

  if (DDS.beacon_mac) { 
  	/* set TOK BEACON MAC */
  	o_parm.option1 |= BEACON_MAC;
     	NDD.ndd_flags |= TOK_BEACON_MAC;
  }

  /* set network speed */
  if (DDS.ring_speed == FOUR_MBS) {
  	o_parm.option2 = SPEED4;
  } else if (DDS.ring_speed == SIXTEEN_MBS) {
  	o_parm.option2 = SPEED16;
  }
  
  /* set network address */	
  if (DDS.use_alt_addr != 0) {
  	for (i = 0; i < 6; i++) {
  		o_parm.node_address[i] = WRK.mps_addr[i];
  	}
  	o_parm.number_local_addr = DDS.use_alt_addr;
  }

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < sizeof(o_parm)/2; i++) {
  	PIO_PUTSX(ioa + LAPD_I, *(parm + i));
  }
  if (!WRK.pio_rc) {
  	PIO_PUTCX(ioa + LISR_SUM, SRB_CMD);      /* SUM.LISR.bit5 0010 0000 */
  }

  if (WRK.pio_rc) {
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0,FALSE,FALSE,TRUE);
  	TRACE_BOTH(MPS_ERR, "Iop2", p_dev_ctl, WRK.pio_rc, 0);
  }

  TRACE_SYS(MPS_OTHER, "IopE", p_dev_ctl, 0, 0);
  unlock_enable(ipri, &CMD_LOCK);

} /* end of open_adapter */

/******************************************************************************/
/*
* NAME: read_adapter_log
*
* FUNCTION: Read and reset the adapter error counters.
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl   - pointer to device control structure.
*       at_int_lvl  - flag for running at interrupt or process level
*
*    Called From: 
*  	mps_ctl
*
*    Calls To: 
*
* RETURN:  0        = Successful completion 
*          ENETDOWN = Error return
*/
/******************************************************************************/
int read_adapter_log (
register mps_dev_ctl_t  *p_dev_ctl,
int  			at_int_lvl)
{

  register int ioa;
  ushort       byte_0_1 =  READ_LOG;
  ushort       byte_2_3 = 0x00FF;
  int 	       ipri, rc = 0;

  TRACE_SYS(MPS_OTHER, "IrlB", p_dev_ctl, 0, 0);
  if (p_dev_ctl->device_state != OPENED) {
	return (ENETDOWN);
  }
  ipri = disable_lock(PL_IMP, &CMD_LOCK);

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);
  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  PIO_PUTSRX(ioa + LAPD_I, byte_0_1);
  PIO_PUTSRX(ioa + LAPD_I, byte_2_3);
  if (!WRK.pio_rc) {
  	PIO_PUTCX(ioa + LISR_SUM, SRB_CMD); /* SUM.LISR.bit5 0010 0000*/
  }

  if (WRK.pio_rc) {
  	TRACE_BOTH(MPS_ERR, "Irl1", p_dev_ctl, WRK.pio_rc, 0);
  	BUSIO_DET(ioa);                      /* restore I/O Bus       */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,
			at_int_lvl, FALSE, FALSE, TRUE);
  	unlock_enable(ipri, &CMD_LOCK);
	return (ENETDOWN);
  }

  if (!at_int_lvl) {
  	w_start(&CTLWDT);

        /*
         * go to sleep with the cmd_slock freed up/regained.
         * Note that the ctl_lock remained locked during the whole time
         */
        WRK.ctl_event = EVENT_NULL;
  	p_dev_ctl->ctl_pending = TRUE;
        e_sleep_thread(&WRK.ctl_event, &CMD_LOCK, LOCK_HANDLER);
	rc = WRK.retcode[READ_LOG];

  } else {
	io_delay(5000);
  }

  BUSIO_DET(ioa);
  TRACE_SYS(MPS_OTHER, "IrlE", p_dev_ctl, rc, at_int_lvl);
  unlock_enable(ipri, &CMD_LOCK);
  return(rc);

} /* end of read_adapter_log */

/******************************************************************************/
/*
* NAME: set_functional_address
*
* FUNCTION: Set the functional address for which the adapter will receive   
*           messages. 
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl   - pointer to device control structure.
*       addr	    - address to be set
*       at_int_lvl  - flag for running at interrupt or process level
*
*    Called From: 
*  	mps_ctl
*
* RETURN:  0        = Successful completion 
*          ENETDOWN = Error return
*/
/******************************************************************************/
int set_functional_address (
register mps_dev_ctl_t  *p_dev_ctl, 
uchar     		addr[4],
int  			at_int_lvl)
{
  f_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  register int  ioa, i, x;
  ushort        id;
  int 	        ipri, rc = 0;

  TRACE_SYS(MPS_OTHER, "IsfB", p_dev_ctl, 0, 0);
  bzero(parm_s, sizeof(f_address));

  parm.cmd     = SET_FUNCTIONAL_ADDRESS;
  parm.retcode = 0xFF;
  for (i = 0; i < 4; i++)
  	parm.f_addr[i] = addr[i];

  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  /*
   * Gets access to the I/O bus to access I/O registers                  
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < sizeof(f_address)/2; i++) {
  	PIO_PUTSX(ioa + LAPD_I, *(parm_s + i));
  }
  if (!WRK.pio_rc) {
  	PIO_PUTCX(ioa + LISR_SUM, SRB_CMD);/* SUM.LISR.bit5 0010 0000 */
  }

  if (WRK.pio_rc) {
  	TRACE_BOTH(MPS_ERR, "Isf1", p_dev_ctl, WRK.pio_rc, 0);
  	BUSIO_DET(ioa);                      /* restore I/O Bus       */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, at_int_lvl, 
				FALSE, TRUE);
  	unlock_enable(ipri, &CMD_LOCK);
	return (ENETDOWN);
  }

  if (!at_int_lvl) {
  	w_start(&CTLWDT);

        /*
         * go to sleep with the cmd_slock freed up/regained.
         * Note that the ctl_lock remained locked during the whole time
         */

        WRK.ctl_event = EVENT_NULL;
  	p_dev_ctl->ctl_pending = TRUE;
        e_sleep_thread(&WRK.ctl_event, &CMD_LOCK, LOCK_HANDLER);
  	rc = WRK.retcode[SET_FUNCTIONAL_ADDRESS];

  } else {
	io_delay(5000);
  }

  BUSIO_DET(ioa);              /* restore I/O Bus       */
  TRACE_SYS(MPS_OTHER, "IsfE", p_dev_ctl, rc, at_int_lvl);
  unlock_enable(ipri, &CMD_LOCK);
  return (rc);

} /* end of set_functional_address */

/******************************************************************************/
/*
* NAME: set_group_address
*
* FUNCTION: Set a group address for which the adapter will receive messages. 
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input: 
*       p_dev_ctl 	- pointer to device control structure.
*       addr		- address to be set
*       at_int_lvl      - flag for running at interrupt or process level
*
*    Called From: 
*  	re_multi_add
*
* RETURN:  0        = Successful completion 
*          ENETDOWN = Error return
*/
/******************************************************************************/
int set_group_address (
register mps_dev_ctl_t  *p_dev_ctl, 
uchar  			*addr,
int  			at_int_lvl)
{

  g_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  register int  ioa, i, x;
  ushort        id;
  int 	        ipri, rc = 0;

  TRACE_SYS(MPS_OTHER, "IrgB", p_dev_ctl, 0, 0);
  bzero(parm_s, sizeof(g_address));

  parm.cmd     = SET_GROUP_ADDRESS;
  parm.retcode = 0xFF;
  parm.num     = 0;
  parm.type    = 0;

  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  for (i = 0; i < 6; i++) {
  	parm.g_addr[i] = *(addr + i);
  }

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < sizeof(g_address)/2; i++) {
  	PIO_PUTSX(ioa + LAPD_I, *(parm_s + i));
  }
  if (!WRK.pio_rc) {
  	PIO_PUTCX(ioa + LISR_SUM, SRB_CMD);/* SUM.LISR.bit5 0010 0000 */
  }

  if (WRK.pio_rc) {
  	TRACE_BOTH(MPS_ERR, "Irg1", p_dev_ctl, WRK.pio_rc, 0);
  	BUSIO_DET(ioa);                      /* restore I/O Bus       */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, at_int_lvl, 
			FALSE, TRUE);
        unlock_enable(ipri, &CMD_LOCK);
	return (ENETDOWN);
  }


  if (!at_int_lvl) {
  	w_start(&CTLWDT);

        /*
         * go to sleep with the cmd_slock freed up/regained.
         * Note that the ctl_lock remained locked during the whole time
         */

        WRK.ctl_event = EVENT_NULL;
  	p_dev_ctl->ctl_pending = TRUE;
        e_sleep_thread(&WRK.ctl_event, &CMD_LOCK, LOCK_HANDLER);
 	rc = WRK.retcode[SET_GROUP_ADDRESS];

  } else {
	io_delay(5000);
  }

  BUSIO_DET(ioa);
  TRACE_SYS(MPS_OTHER, "IrgE", p_dev_ctl, rc ,at_int_lvl);
  unlock_enable(ipri, &CMD_LOCK);
  return(rc);
} /* end of set_group_address */

/******************************************************************************/
/*
* NAME: unset_group_address
*
* FUNCTION: Reset a previously set group address. 
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input: 
*       p_dev_ctl 	- pointer to device control structure.
*       addr		- address to be set
*
*    Called From: 
*  	mps_clt
*
* RETURN:  0        = Successful completion 
*          ENETDOWN = Error return
*/
/******************************************************************************/
int unset_group_address (
register mps_dev_ctl_t  *p_dev_ctl, 
uchar  			*addr)
{

  g_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  register int  ioa, i, x;
  ushort        id;
  int 	        ipri;

  TRACE_SYS(MPS_OTHER, "IrgB", p_dev_ctl, 0, 0);
  bzero(parm_s, sizeof(g_address));

  parm.cmd     = RESET_GROUP_ADDRESS;
  parm.retcode = 0xFF;
  parm.num     = 0;
  parm.type    = 0;

  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  for (i = 0; i < 6; i++) {
  	parm.g_addr[i] = *(addr + i);
  }

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < sizeof(g_address)/2; i++) {
  	PIO_PUTSX(ioa + LAPD_I, *(parm_s + i));
  }

  if (!WRK.pio_rc) {
  	PIO_PUTCX(ioa + LISR_SUM, SRB_CMD);      /* SUM.LISR.bit5 0010 0000 */
  }

  if (WRK.pio_rc) {
  	TRACE_BOTH(MPS_ERR, "Irg4", p_dev_ctl, WRK.pio_rc, 0);
  	BUSIO_DET(ioa);                      /* restore I/O Bus       */
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
				FALSE, FALSE, TRUE);
        unlock_enable(ipri, &CMD_LOCK);
	return (ENETDOWN);
  }

  w_start(&CTLWDT);

  /*
   * go to sleep with the cmd_slock freed up/regained.
   * Note that the ctl_lock remained locked during the whole time
   */
  p_dev_ctl->ctl_pending = TRUE;
  WRK.ctl_event = EVENT_NULL;
  e_sleep_thread(&WRK.ctl_event, &CMD_LOCK, LOCK_HANDLER);

  TRACE_SYS(MPS_OTHER, "IrgE", p_dev_ctl, WRK.retcode[RESET_GROUP_ADDRESS], 0);
  BUSIO_DET(ioa);                      /* restore I/O Bus       */
  unlock_enable(ipri, &CMD_LOCK);
  return (WRK.retcode[RESET_GROUP_ADDRESS]);
} /* end of unset_group_address */

/******************************************************************************/
/*
* NAME: srb_response
*
* FUNCTION: process the System Request Block(SRB) command response. 
*
* EXECUTION ENVIRONMENT: interrupt only
*
* NOTES:
*    Input: 
*       p_dev_ctl - pointer to device control structure.
*       ioa       - I/O bus address
*
*    Called From: 
*  	mps_intr
*
*/
/******************************************************************************/
srb_response(
register mps_dev_ctl_t  *p_dev_ctl,
int  			ioa)
{

  ndd_t        *p_ndd = &(NDD);
  ushort       open_error_code;
  ushort       command;
  ushort       data[10],i,j;
  ushort       func_addr_offset;
  ushort	Mcode_level_offset;
  ndd_statblk_t  stat_blk;   /* status block */
  int rc;
  int ipri;
  uchar init_status_h;

  TRACE_SYS(MPS_OTHER, "IsrB", p_dev_ctl, 0, 0);
  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i=0; i < 10; i++) {
  	PIO_GETSX(ioa + LAPD_I, &data[i]);
  }

  if (WRK.pio_rc) {
  	TRACE_BOTH(MPS_ERR, "Isr1", p_dev_ctl, WRK.pio_rc, 0);
        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, TRUE,FALSE,TRUE);
	return (ENETDOWN);
  }


  command = data[0] >> 8;
  WRK.retcode[command] = data[1] >> 8;

  if (p_dev_ctl->ctl_pending == TRUE) { 
       	w_stop(&CTLWDT);
  }

  switch (command) {
  case INITIALIZATION_COMPLETE :
        /*
         * Initializes status
         */
        if (p_dev_ctl->device_state == LIMBO) {
                p_dev_ctl->device_state = LIMBO_EXIT_PENDING;
        } else {
                p_dev_ctl->device_state = OPEN_PENDING;
        }

        TRACE_SYS(MPS_OTHER,"Isr2",p_dev_ctl,p_dev_ctl->device_state,
                                                        WRK.retcode[command]);
  	WRK.retcode[command] = data[3];
  	if (!WRK.retcode[command]) {

                func_addr_offset = data[6];
		Mcode_level_offset = data[5];
		/*
  		 * Sets the ring speed in MIB for autosense 
		 */
		init_status_h = data[0] & 0x00FF;
  		if (DDS.ring_speed == AUTOSENSE) {
  			if (init_status_h & SPEED_16) {
  				NDD.ndd_mtu = CTOK_16M_MAX_PACKET;
                          	MIB.Token_ring_mib.Dot5Entry.ring_speed =
                                                TR_MIB_SIXTEENMEGABIT;
  				p_ndd->ndd_flags |= TOK_RING_SPEED_16;
  			} else {
  				NDD.ndd_mtu = CTOK_4M_MAX_PACKET;
                          	MIB.Token_ring_mib.Dot5Entry.ring_speed =
                                                TR_MIB_FOURMEGABIT;
  				p_ndd->ndd_flags |= TOK_RING_SPEED_4;
  			}
		}

		/*
  		 * Address of adapter parameters 
		 */
  		WRK.parms_addr = data[7];

		/*
  		 * Gets the function address 
		 */
  		PIO_PUTSRX(ioa + LAPE, 0x00);
  		PIO_PUTSRX(ioa + LAPA, func_addr_offset);
  		for (i=0; i < 7; i++) {
  			PIO_GETSX(ioa + LAPD_I, &data[i]);
  		}

		/*
  		 * Sets functional address in MIB
		 */
  		FUNCTIONAL.functional[0] = 0xc0;
  		FUNCTIONAL.functional[1] = 0x00;
  		for (i=2, j=5 ; i < 6; j++) {
  			FUNCTIONAL.functional[i++] |= data[j] >> 8;
  			FUNCTIONAL.functional[i++] |= data[j] & 0xff;
  		}

		/*
  		 * Gets the Micro code level 
		 */
  		PIO_PUTSRX(ioa + LAPE, 0x00);
  		PIO_PUTSRX(ioa + LAPA, Mcode_level_offset);
  		for (i=0; i < 5; i++) {
  			PIO_GETSX(ioa + LAPD_I, &data[i]);
  		}

                if (WRK.pio_rc) {
                        TRACE_BOTH(MPS_ERR, "Isr3", p_dev_ctl, WRK.pio_rc, 0);
                        mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0,
                                                TRUE, FALSE, TRUE);
                        break;
                }

		/*
  		 * Sets Micro code level in MIB
		 */
  		for (i=0, j=0 ; j < 5; j++) {
  			MIB.Generic_mib.ifExtnsEntry.revware[i++]=data[j] >> 8;
  			MIB.Generic_mib.ifExtnsEntry.revware[i++]=data[j]& 0xff;
  		}

  		MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_NO_STATUS;
		
		if (DDS.priority_tx) {
			/* config high priority channel */
			config_hp_channel(p_dev_ctl,ioa,TOKEN_ACCESS_PRIORITY);
		} else {
  			open_adapter(p_dev_ctl, ioa);
		}

  	} else {
  		TRACE_BOTH(MPS_ERR,"Isr4",p_dev_ctl,p_dev_ctl->device_state,
							WRK.retcode[command]);
		mps_bug_out(p_dev_ctl, NDD_CMD_FAIL, command, 
				WRK.retcode[command], TRUE, FALSE, TRUE);
  		mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__, 
  			   __FILE__, command, WRK.retcode[command], 0);

  		WRK.retcode[command] = ENETDOWN;
  	}
  	break;

  case CONFIGURE_HP_CHANNEL :
  	TRACE_SYS(MPS_OTHER,"Isr5",p_dev_ctl,p_dev_ctl->device_state,
							WRK.retcode[command]);
  	if (WRK.retcode[command] == OPERATION_SUCCESSFULLY) {
  		open_adapter(p_dev_ctl, ioa);

  	} else {
  		mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__, 
  			   __FILE__, command, WRK.retcode[command], 0);
  		WRK.retcode[command] = ENETDOWN;
          	enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_CMD_FAIL, command, 0);
  	}
  	break;

  case OPEN_ADAPTER :
	/*
  	 * Stops the watchdog timers 
	 */
  	w_stop(&(HWEWDT));

  	if (WRK.retcode[command] == OPERATION_SUCCESSFULLY) {

  		TRACE_SYS(MPS_OTHER,"Isr6",p_dev_ctl,p_dev_ctl->device_state,0);
		/*
  		 * Gets the adapter block address 
		 */
  		WRK.asb_address = data[4];
  		WRK.srb_address = data[5];
  		WRK.arb_address = data[6];
  		WRK.trb_address = data[8];

		/*
  		 * Enables the Rx & TX channel 
		 */
  		PIO_PUTSRX(ioa + BMCtl_rum, CHNL_ENABLE);
  		/* give RX buffer descriptor address to adapter   */
  		PIO_PUTLRX(ioa+RxBDA_L, WRK.recv_list[0]);
  		PIO_PUTLRX(ioa+RxLBDA_L,WRK.recv_list[MAX_RX_LIST - 1]);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(MPS_ERR, "Isr7", p_dev_ctl, WRK.pio_rc, 0);
        		mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
						TRUE, FALSE, TRUE);
  			break;
  		}


		/*
  		 * Checks if promiscouus mode need to be set
		 */
                if (WRK.promiscuous_count) { 
                          p_ndd->ndd_flags |= NDD_PROMISC;
                          MIB.Generic_mib.ifExtnsEntry.promiscuous = TRUE;
                          modify_receive_options (p_dev_ctl, PROMIS_ON, TRUE);
                }

		/*
  		 * Sets the functional address 
		 */
               set_functional_address(p_dev_ctl,&FUNCTIONAL.functional[2],TRUE);
		/*
  		 * Sets the group address 
		 */
  		if (WRK.multi_count) {
                          re_multi_add( p_dev_ctl, p_ndd);
		}

  		MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_NOPROBLEM;
                if (NDD.ndd_flags & NDD_LIMBO) {
                        w_start(&(LANWDT)); /* start the lan connected timer */
                } else {
                        /*
                         * Updates the adapter device state & NDD flag
                         * and then wakeup the open process (in mps_act)
                         */
                        p_dev_ctl->device_state = OPENED;
                        NDD.ndd_flags |= NDD_RUNNING;
                        e_wakeup((int *)&WRK.ctl_event);
                        p_dev_ctl->open_pending = FALSE;
                }

  	} else if (WRK.retcode[command] == COMMAND_CANCELLED) {
  		open_error_code = data[3];
  		TRACE_SYS(MPS_ERR,"Isr8",p_dev_ctl,p_dev_ctl->device_state,
							open_error_code);

  		switch (open_error_code & 0xF) {
  		case function_failure :
  		case wire_fault :
                        if (!(NDD.ndd_flags & NDD_LIMBO)) {
       				mps_logerr(p_dev_ctl, ERRID_MPS_WIRE_FAULT, 
  				   	   __LINE__, __FILE__, TOK_WIRE_FAULT, 
  				   	   open_error_code, 0);
			}
			enter_limbo (p_dev_ctl, TRUE, TRUE, 30, TOK_WIRE_FAULT,
					 command, open_error_code);
  			break;

  		case signal_los :
  		case time_out :
  		case ring_failure :
  		case ring_beaconing :
  		case request_parameter :
  			mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_OPEN,
  				   __LINE__, __FILE__, TOK_ADAP_OPEN, 
  				   open_error_code, 0);
			enter_limbo (p_dev_ctl, TRUE, TRUE, 30, TOK_WIRE_FAULT,
					 command, open_error_code);
  			break;

  		case remove_received :
  			mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_OPEN,
  				   __LINE__, __FILE__, TOK_RMV_ADAP, 
  				   open_error_code, 0);
			enter_limbo (p_dev_ctl, TRUE, TRUE, 30, TOK_WIRE_FAULT,
					 command, open_error_code);
  			break;

  		case duplicate_node_address :
			mps_bug_out(p_dev_ctl, TOK_DUP_ADDR, open_error_code,
					0, TRUE, FALSE, TRUE);
          		mps_logerr(p_dev_ctl, ERRID_MPS_DUP_ADDR, __LINE__, 
				   __FILE__, TOK_DUP_ADDR, open_error_code, 0);

  			break;

  		case ring_speed_mismatch :
  			if (DDS.ring_speed == AUTOSENSE) {
  				if (p_ndd->ndd_flags & TOK_RING_SPEED_16) {
  					NDD.ndd_mtu = CTOK_4M_MAX_PACKET;
					DDS.ring_speed = FOUR_MBS;
  					p_ndd->ndd_flags |= TOK_RING_SPEED_4;
                          		MIB.Token_ring_mib.Dot5Entry.ring_speed=
                                                TR_MIB_FOURMEGABIT;
  				} else {
  					NDD.ndd_mtu = CTOK_16M_MAX_PACKET;
					DDS.ring_speed = SIXTEEN_MBS;
  					p_ndd->ndd_flags |= TOK_RING_SPEED_16;
                          		MIB.Token_ring_mib.Dot5Entry.ring_speed=
                                                TR_MIB_SIXTEENMEGABIT;
  				}
  				open_adapter(p_dev_ctl, ioa);
			} else {
				mps_bug_out(p_dev_ctl, TOK_RING_SPEED, 
						open_error_code, 0, 
						TRUE, FALSE, TRUE);
          			mps_logerr(p_dev_ctl, ERRID_MPS_RING_SPEED, 
  					__LINE__, __FILE__, TOK_RING_SPEED, 
  					open_error_code, 0);
			}

  			break;
  		case no_monitor_RPL :
  		case monitor_failed_RPL :
  			break;
  		default :
  			mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_OPEN,
  				   __LINE__, __FILE__, TOK_ADAP_OPEN, 
  				   open_error_code, 0);
  			break;
  		}
  	} else if (WRK.retcode[command] == INVALID_CONFIGURATION) {
                if (!(NDD.ndd_flags & NDD_LIMBO)) {
          		mps_logerr(p_dev_ctl, ERRID_MPS_WIRE_FAULT, __LINE__, 
				   __FILE__, TOK_WIRE_FAULT, 0, 
				   WRK.retcode[command]); 
		}
          	enter_limbo(p_dev_ctl, TRUE, TRUE, 30,NDD_CMD_FAIL, command,0);
  	} else {
          	mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_OPEN, __LINE__, __FILE__, 
			   TOK_ADAP_OPEN, WRK.retcode[command], 0); 
		mps_bug_out(p_dev_ctl, TOK_DUP_ADDR, TOK_ADAP_OPEN, 
				WRK.retcode[command], TRUE, FALSE, TRUE);
          }	
  	break;
  case READ_LOG :
  	TRACE_SYS(MPS_OTHER,"Isr9",p_dev_ctl,p_dev_ctl->device_state,
							WRK.retcode[command]);
  	if (WRK.retcode[command] == OPERATION_SUCCESSFULLY) {
		/*
  		 * Adapter error counter 
		 */
  		bcopy((uchar *)&data[3], (uchar *)&WRK.adap_log, 
  		    sizeof(WRK.adap_log));

		/*
  		 * Updates the counter 
		 */
  		TOKSTATS.line_errs    +=WRK.adap_log.line_error;
  		TOKSTATS.burst_errs   +=WRK.adap_log.burst_error;
  		TOKSTATS.int_errs     +=WRK.adap_log.internal_error;
  		TOKSTATS.lostframes   +=WRK.adap_log.lost_frame;
  		TOKSTATS.rx_congestion+=WRK.adap_log.receive_congestion;
  		TOKSTATS.framecopies  +=WRK.adap_log.frame_copied_error;
  		TOKSTATS.token_errs   +=WRK.adap_log.token_error;
  	} else {
  		bzero((uchar *)&WRK.adap_log, sizeof(WRK.adap_log));
  		mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__, 
  		    	   __FILE__, command, WRK.retcode[command], 0);
  		WRK.retcode[command] = ENETDOWN;
          	enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_CMD_FAIL, command, 0);
  	}
  	break;

  case MODIFY_RECEIVE_OPTION :
  case RESET_GROUP_ADDRESS :
        TRACE_SYS(MPS_OTHER,"Isra",p_dev_ctl,p_dev_ctl->device_state,
                                                        WRK.retcode[command]);
        if (WRK.retcode[command] != OPERATION_SUCCESSFULLY) {
                mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__,
                           __FILE__, command, WRK.retcode[command], 0);

                WRK.retcode[command] = ENETDOWN;
                enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_CMD_FAIL, command, 0);
        }
        break;

  case SET_FUNCTIONAL_ADDRESS :
  case SET_GROUP_ADDRESS :
        TRACE_SYS(MPS_OTHER,"Isrb",p_dev_ctl,p_dev_ctl->device_state,
                                                        WRK.retcode[command]);
        if (WRK.retcode[command] != OPERATION_SUCCESSFULLY) {
                mps_logerr(p_dev_ctl, ERRID_MPS_ADAP_CHECK, __LINE__,
                           __FILE__, command, WRK.retcode[command], 0);

                if (WRK.retcode[command] != NOT_SET_GROUP) {
                        WRK.retcode[command] = ENETDOWN;
                        enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_CMD_FAIL,
                                        command, 0);
                }
        }
        break;

  default :
  	TRACE_BOTH(MPS_ERR,"Isrc",p_dev_ctl,p_dev_ctl->device_state, 0);
        enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_CMD_FAIL, command, 0);

  	break;
  } /* end of switch of command to adapter                             */

  if (p_dev_ctl->ctl_pending) { 
      	ipri = disable_lock(PL_IMP, &CMD_LOCK);
  	e_wakeup((int *)&WRK.ctl_event);
	p_dev_ctl->ctl_pending = 0;
        unlock_enable(ipri, &CMD_LOCK);
  }
  TRACE_SYS(MPS_OTHER, "IsrE", p_dev_ctl, command, WRK.retcode[command]);
} /* end of srb_response */
