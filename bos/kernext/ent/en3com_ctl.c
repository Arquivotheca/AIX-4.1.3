static char sccsid[] = "@(#)40  1.14  src/bos/kernext/ent/en3com_ctl.c, sysxent, bos411, 9431A411a 8/2/94 12:39:44";
/*
 * COMPONENT_NAME: sysxent -- High Performance Ethernet Device Driver
 *
 * FUNCTIONS: 
 *		en3com_ctl
 *		en3com_cmd
 *		en3com_getmib
 *		en3com_getstat
 *		en3com_clrstat
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
#include <sys/dump.h>
#include <sys/mbuf.h>
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


extern ethernet_all_mib_t en3com_mib_status;
extern uchar ent_broad_adr[];



/*****************************************************************************/
/*
 * NAME:     en3com_ctl
 *
 * FUNCTION: Ethernet driver ioctl routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ndd_ctl field in the NDD on the NDD chain.
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *	cmd		- control command
 *	arg		- pointer to the argument for the control command
 *	length		- length of the argument.
 *
 * RETURNS:  
 *	0 - OK
 *	ENETUNREACH - device is currently unreachable
 *	ENETDOWN - device is down
 *	EINVAL - invalid paramter
 *	ENOMEM - unable to allocate required memory
 *	EOPNOTSUPP - operation not supported
 */
/*****************************************************************************/
en3com_ctl(
  ndd_t		*p_ndd,		/* pointer to the ndd in the dev_ctl area */
  int		cmd,		/* control command */
  caddr_t	arg,		/* argument of the control command */
  int		length)		/* length of the argument */
  

{
  en3com_dev_ctl_t   *p_dev_ctl = (en3com_dev_ctl_t *)(p_ndd->ndd_correlator);
  int rc = 0;		/* return code */
  int ipri;



  

  TRACE_SYS(HKWD_EN3COM_OTHER, "IctB", (ulong)p_ndd, cmd, (ulong)arg);
  TRACE_SYS(HKWD_EN3COM_OTHER, "IctC", length, 0, 0);

/*
 * Lock the complex lock ctl_clock to lock out other ioctl to the same device.
 */

  lock_write(&CTL_LOCK);

  switch(cmd) {

/*
 * Get the generic statistics.
 * This is for ndd_genstats + ent_genstats.
 */
	case NDD_GET_STATS:
	{

		if (length != sizeof(ent_ndd_stats_t)) {
			rc = EINVAL;
			break;
		}
         	if (rc = en3com_getstat(p_dev_ctl)) {
			break;
		}

         	/* copy statistics to user's buffer */
         	bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
         	bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t), 
			sizeof(ent_genstats_t));

		break;
	}

/*
 * Get all of the device statistics. 
 * This is for ndd_genstats + ent_genstats + en3com_stats
 */
	case NDD_GET_ALL_STATS:
	{

		if (length != sizeof(en3com_all_stats_t)) {
			rc = EINVAL;
			break;
		}
         	if (rc = en3com_getstat(p_dev_ctl)) {
			break;
		}

         	/* copy statistics to user's buffer */
         	bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
         	bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t), 
			sizeof(ent_genstats_t) + sizeof(en3com_stats_t));

		break;
	}

/*
 * Clear all of the statistics.
 */
	case NDD_CLEAR_STATS:
	{

         	rc = en3com_clrstat(p_dev_ctl);
		break;
	}

/*
 * Enable the adapter to receive all of the multicast packets on the network.
 * The driver will switch to the special promiscuous mode in order to 
 * support this function.
 */
	case NDD_ENABLE_MULTICAST:
	{

    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		WRK.enable_multi++;	/* inc the reference counter */

		/*
		 * if the adapter is not already in the promiscuous mode
		 * for any other reason, and this is the first enable
		 * multicast operation, config the adapter to promiscuous
		 * mode.
		 */
		if (WRK.enable_multi == 1) {
			p_ndd->ndd_flags |= NDD_MULTICAST;
			WRK.promiscuous_count++;
			if (WRK.promiscuous_count == 1) {
			  p_ndd->ndd_flags |= NDD_PROMISC;
			  en3com_cmd(p_dev_ctl, CONFIGURE, FALSE);
			}
		}
		break;
	}

/*
 * Disable the all multicast function. The adapter will go back to use the
 * multicast filter if this is the last disable_multicast operation. 
 * If there is no other reason to stay in the promiscuous
 * mode, the adapter will be re-configed to be out of the promiscuous mode.
 */
	case NDD_DISABLE_MULTICAST:
	{
		if (!WRK.enable_multi) {
			rc = EINVAL;
			break;
		}

		WRK.enable_multi--;	/* dec the reference counter */
		if (!WRK.enable_multi) {
			p_ndd->ndd_flags &= ~NDD_MULTICAST;
			if (WRK.promiscuous_count) {
			  WRK.promiscuous_count--;
			  if (!WRK.promiscuous_count) {
			    p_ndd->ndd_flags &= ~NDD_PROMISC;
			    en3com_cmd(p_dev_ctl, CONFIGURE, FALSE);
			  }
			}
		}
		break;
	}

/*
 * Enable the promiscuous mode. If this is the first promiscuous on operation
 * and the adapter is not in promiscuous mode already, the driver will config
 * the adapter to run in the promiscuous mode.
 */
	case NDD_PROMISCUOUS_ON:
	{
    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		WRK.promiscuous_count++;	/* inc the reference counter */
		if (WRK.promiscuous_count == 1) { 
			p_ndd->ndd_flags |= NDD_PROMISC;
			en3com_cmd(p_dev_ctl, CONFIGURE, FALSE);
		}
		break;
	}

/*
 * Disable the promiscuous mode. If this is the last promiscuous off operation
 * and there is no other reason to stay in the promiscuous mode, the  adapter 
 * will be re-configed to be out of the promiscuous mode.
 */
	case NDD_PROMISCUOUS_OFF:
	{
		if (!WRK.promiscuous_count) {
			rc = EINVAL;
			break;
		}

		WRK.promiscuous_count--;	/* dev the reference counter */
		if (!WRK.promiscuous_count) {
			p_ndd->ndd_flags &= ~NDD_PROMISC;
			en3com_cmd(p_dev_ctl, CONFIGURE, FALSE);
		}
		break;
	}

/*
 * Add a filter. Since the demuxer is managing all the type filtering, the
 * adapter is running with no filters all the time, this operation doesn't
 * mean a whole lot to the driver. Increment a reference count and return.
 */
	case NDD_ADD_FILTER:
	{
		
    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}
		if (length != sizeof(ns_8022_t)) {
			rc = EINVAL;
			break;
		}

		WRK.filter_count++;	/* inc the reference counter */

		break;
	}

/*
 * Delete a filter. The driver doesn't need to do anything, decrement the
 * reference count and return.
 */
	case NDD_DEL_FILTER:
	{

		if (length != sizeof(ns_8022_t)) {
			rc = EINVAL;
			break;
		}
		if (!WRK.filter_count) {
			rc = EINVAL;
			break;
		}

		WRK.filter_count--;	/* dec the reference counter */

		break;
	}

/*
 * Query the MIB support status on the driver.
 */
	case NDD_MIB_QUERY:
	{

    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		if (length != sizeof(ethernet_all_mib_t)) {
			rc = EINVAL;
			break;
		}

         	/* copy status to user's buffer */
         	bcopy(&en3com_mib_status, arg, sizeof(ethernet_all_mib_t));
		break;

	}

/*
 * Get all MIB values.
 */
	case NDD_MIB_GET:
	{

		if (length != sizeof(ethernet_all_mib_t)) {
			rc = EINVAL;
			break;
		}

         	if (rc = en3com_getmib(p_dev_ctl)) {
			break;
		}

         	/* copy mibs to user's buffer */
         	bcopy(&MIB, arg, sizeof(ethernet_all_mib_t));

		break;
	}

/*
 * Get receive address table (mainly for MIB variables). 
 * The receive address table is consists of all the addresses that the
 * adapter is armed to receive packets with. It includes the host
 * network address, the broadcast address and the currently registered
 * multicast addresses.
 */
	case NDD_MIB_ADDR:
	{

		ndd_mib_addr_t  *p_table = (ndd_mib_addr_t *)arg;
		ndd_mib_addr_elem_t  *p_elem;
		en3com_multi_t *p_multi = &WRK.multi_table;
		int elem_len = 
			sizeof(ndd_mib_addr_elem_t) + ENT_NADR_LENGTH - 2;
		int count = 0;
		int i;


		
    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		if (length < sizeof(ndd_mib_addr_t)) {
			rc = EINVAL;
			break;
		}

		length -= sizeof(u_int);   /* room for count field */
		arg += 4;

		/* copy the specific network address in use first */
		if (length >= elem_len) {
			p_elem = (ndd_mib_addr_elem_t *)arg;
			p_elem->status = NDD_MIB_VOLATILE;
			p_elem->addresslen = ENT_NADR_LENGTH;
			COPY_NADR(WRK.net_addr, p_elem->address);
			length -= elem_len;
			arg += elem_len;
			count++;
		}
		else {
         		p_table->count = 0;
			rc = E2BIG;
			break;
		}
		
		/* copy the broadcast address */
		if (length >= elem_len) {
			p_elem = (ndd_mib_addr_elem_t *)arg;
			p_elem->status = NDD_MIB_NONVOLATILE;
			p_elem->addresslen = ENT_NADR_LENGTH;
			COPY_NADR(ent_broad_adr, p_elem->address);
			length -= elem_len;
			arg += elem_len;
			count++;
		}
		else {
         		p_table->count = 1;
			rc = E2BIG;
			break;
		}
			
		
		/* copy multicast addresses */
		while (p_multi) {
			for (i=0; i < p_multi->in_use; i++) {
			  if (length >= elem_len) {
				p_elem = (ndd_mib_addr_elem_t *)arg;
				p_elem->status = NDD_MIB_VOLATILE;
				p_elem->addresslen = ENT_NADR_LENGTH;
				COPY_NADR(p_multi->m_slot[i].m_addr, 
					p_elem->address);
				length -= elem_len;
				arg += elem_len;
				count++;
			  }
			  else {
				rc = E2BIG;  /* user table is not big enough */
				break;
			  }
			}
			if (i < p_multi->in_use) 
			  break;

			p_multi = p_multi->next;
		}
				
         	/* put the final count into the buffer */
         	p_table->count = count;

		break;
	}

/*
 * Add an asynchronous status filter. The 3com driver only track the
 * NDD_BAD_PKTS status filter. If this is the first time the bad packets
 * status is added, the driver will config the adapter to start receiving
 * bad packets on the network.
 */
	case NDD_ADD_STATUS:
	{
		
		ns_com_status_t *p_stat = (ns_com_status_t *)arg;


    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		if (length < sizeof(ns_com_status_t)) {
			rc = EINVAL;
			break;
		}

		if (p_stat->filtertype == NS_STATUS_MASK) {
			if (p_stat->mask & NDD_BAD_PKTS) {
				WRK.badframe_count++; /* inc reference count */
				if (WRK.badframe_count == 1) {
					p_ndd->ndd_flags |= ENT_RCV_BAD_FRAME;
					en3com_cmd(p_dev_ctl, CONFIGURE, FALSE);
				}
			}
			else {
				WRK.otherstatus++; /* inc reference count */
			}
		}
		else {
			rc = EINVAL;
		}
		break;
	}

/*
 * Delete an asynchronous status filter. The 3com driver only track the
 * NDD_BAD_PKTS status filter. If this is the last time the bad packets
 * status is deleted, the driver will config the adapter to stop receiving
 * bad packets on the network.
 */
	case NDD_DEL_STATUS:
	{
		
		ns_com_status_t *p_stat = (ns_com_status_t *)arg;


		if (length < sizeof(ns_com_status_t)) {
			rc = EINVAL;
			break;
		}
		if (p_stat->filtertype == NS_STATUS_MASK) {
			if (p_stat->mask == NDD_BAD_PKTS) {
				if (!WRK.badframe_count) {
					rc = EINVAL;
					break;
				}

				WRK.badframe_count--; /* dec reference count */
				if (!WRK.badframe_count) {
					p_ndd->ndd_flags &= ~ENT_RCV_BAD_FRAME;
					en3com_cmd(p_dev_ctl, CONFIGURE,
						FALSE);
				}
			}
			else {
				if (!WRK.otherstatus) {
					rc = EINVAL;
					break;
				}
				WRK.otherstatus--; /* dec reference count */
			}
				
		}
		else {
			rc = EINVAL;
		}
		break;
	}

/*
 * Add a multicast address to the multicast filter. The 3com adapter only
 * support a filter of 10 multicast addresses. When there are less than
 * 10 multicast addresses in the table, update the adapter's multicast
 * filter as required. When there are more than 10 multicast addresses in
 * the table, the driver will enable the promiscuous mode on the adapter 
 * in order to get all of the multicast packets on the network.
 * The driver will keep the multicast address in its table and expand the
 * table when more space is needed.
 */
	case NDD_ENABLE_ADDRESS:
	{
		
    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		if (length != ENT_NADR_LENGTH) {
			rc = EINVAL;
			break;
		}

		/* verify that it is a valid multicast address */
		if ((*((char *)arg) & MULTI_BIT_MASK) &&
			(!SAME_NADR(arg, ent_broad_adr))) {
		  if (rc = en3com_multi_add(p_dev_ctl, arg)) {
			 /* if can't add it to the table */
			break;
		  }

		  p_ndd->ndd_flags |= NDD_ALTADDRS;
		  if (WRK.multi_count > MAX_MULTI) {
			if (!WRK.multi_promis_mode) {
				WRK.multi_promis_mode = TRUE;
				WRK.promiscuous_count++; 
				if (WRK.promiscuous_count == 1) {
		  		  p_ndd->ndd_flags |= NDD_PROMISC;
				  en3com_cmd(p_dev_ctl, CONFIGURE, FALSE);
				}
			}
		  }
		  else {
			en3com_cmd(p_dev_ctl, SET_MULTICAST, FALSE);
		  }
		}
		else {
		  rc = EINVAL;
		}
			
		break;
	}

/*
 * Delete a multicast address from the multicast filter. The 3com adapter only
 * support a filter of 10 multicast addresses. When there are more than 10
 * multicast addresses in the table, the adapter will stay in the promiscuous
 * mode as mentioned above. When there are less than 10 multicast addresses 
 * left in the table, the driver will update the adapter's multicast filter
 * as required and disable the promiscuous mode on the adapter if there is no
 * other reason to stay in promiscuous mode. The driver will maintain its
 * multicast table and shrink the table when spaces freed up.
 */
	case NDD_DISABLE_ADDRESS:
	{

		if (length != ENT_NADR_LENGTH) {
			rc = EINVAL;
			break;
		}

		/* verify that it is a valid multicast address */
		if ((*((char *)arg) & MULTI_BIT_MASK) &&
			(!SAME_NADR(arg, ent_broad_adr))) {

		  /* if found and deleted it from the table */
		  if (!(rc = en3com_multi_del(p_dev_ctl, arg))) {
			if (WRK.multi_count <= MAX_MULTI) {
				if (WRK.multi_promis_mode) {
				  WRK.multi_promis_mode = FALSE;
				  if (WRK.promiscuous_count) {
				    WRK.promiscuous_count--;
				    if (!WRK.promiscuous_count) {
				      p_ndd->ndd_flags &= ~NDD_PROMISC;
				      en3com_cmd(p_dev_ctl, CONFIGURE, FALSE);
				    }
				  }
				}
				if (!WRK.multi_count)
				  p_ndd->ndd_flags &= ~NDD_ALTADDRS;
				en3com_cmd(p_dev_ctl, SET_MULTICAST, FALSE);
			}
		  }
		}
		else 
		  rc = EINVAL;
				
		break;
	}

	default:
		rc = EOPNOTSUPP;
  		TRACE_SYS(HKWD_EN3COM_ERR, "Ict3", rc, cmd, 0);
		break;

		
  }

  lock_done(&CTL_LOCK);
  TRACE_SYS(HKWD_EN3COM_OTHER, "IctE", rc, 0, 0);
  return(rc);


}

/*****************************************************************************/
/*
 * NAME:     en3com_cmd
 *
 * FUNCTION: Issue control commands to the adapter
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_ctl
 *	en3com_setup
 *	en3com_stimer
 *	en3com_exec_intr
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *	cmd		- adapter control command
 *	at_int_lvl	- flag for running at interrupt or process level
 *
 * RETURNS:  
 *	0 - OK
 *	ENETUNREACH - device is currently unreachable
 *	ENETDOWN - device is down
 *	EOPNOTSUPP - operation not supported
 */
/*****************************************************************************/
en3com_cmd(
  en3com_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int			cmd, 		/* adapter control command */
  int 			at_int_lvl)	/* flag for intr/proc level */
  
{


  int bus, ioa;
  int i;
  int pio_rc = 0;
  int ipri;
  short tmp_stat;
  uchar host_status_reg;



  TRACE_SYS(HKWD_EN3COM_OTHER, "IcmB", (ulong)p_dev_ctl, cmd, at_int_lvl);

  /*
   * Get the cmd_lock and check the device state, make sure that the 
   * error recovery has not occurred.
   */
  if (!at_int_lvl) {
    ipri = disable_lock(PL_IMP, &CMD_LOCK);
    if (p_dev_ctl->device_state != OPENED && 
	p_dev_ctl->device_state != OPEN_PENDING) {
	if (p_dev_ctl->device_state == DEAD) {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Icm1", ENETDOWN, 
			p_dev_ctl->device_state, 0);
		return(ENETDOWN);
	}
	else {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Icm2", ENETUNREACH, 
			p_dev_ctl->device_state, 0);
		return(ENETUNREACH);
	}
    }
  }

  bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr);
  ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

  switch (cmd) {

      /***********************************************************************/
      /*  Configure adapter's 82586 module        0x0000                     */
      /***********************************************************************/
      case CONFIGURE:
      {

	 short parm = 0;

         /* Load the execute mailbox with the command to be executed         */
         ENT_PUTSRX( bus + WRK.exec_mail_box, (short)CONFIGURE); 

	 if (WRK.promiscuous_count || WRK.enable_multi || WRK.multi_promis_mode)
		parm |= PROMIS_ON;
	 if (WRK.badframe_count)
		parm |= SAVE_BP;
         ENT_PUTSRX( bus + WRK.exec_mail_box + 2, parm );

         break;
      }

      /***********************************************************************/
      /*  Set Network Address                     0x0001                     */
      /***********************************************************************/
      case SET_ADDRESS:
      {
	 int  i;

         /* Load the execute mailbox with the command to be executed         */
         ENT_PUTSRX( bus + WRK.exec_mail_box, (short)SET_ADDRESS ); 

         /* Store the address in the adapter                          */
         for (i = 0; i < ENT_NADR_LENGTH; i++) {
            ENT_PUTCX( bus + WRK.exec_mail_box + 2 + i, WRK.net_addr[i]);
         }

         break;
      }


      /***********************************************************************/
      /*  Set Multicast Address                   0x0002                     */
      /***********************************************************************/
      case SET_MULTICAST:
      {

	 int  i;
	 char *p;
	 int  start_offset;
	 en3com_multi_t *p_multi;


         /* Load the execute mailbox with the command to be executed         */
         ENT_PUTSRX( bus + WRK.exec_mail_box, (short)SET_MULTICAST);

         /* Load the current count of multicast addresses */
         ENT_PUTSRX( bus + WRK.exec_mail_box + 2, WRK.multi_count);

         /* Load the Multicast addresses from the multi_table chain */
	 start_offset = bus + WRK.exec_mail_box + 4;
	 p_multi = &WRK.multi_table;
	 while (p_multi) {
         	for (i = 0; i < p_multi->in_use; i++) {
         		p = p_multi->m_slot[i].m_addr; 
			/* copy the address to the adapter */
			ENT_PUTLX(start_offset, *((ulong *)p));
			ENT_PUTSX(start_offset + 4, 
				*(ushort *)((uchar *)p + 4));
			start_offset += 6;
		}
		p_multi = p_multi->next;
	 }

         break;
      }

      /***********************************************************************/
      /*  Set Receive Pattern Match Filter        0x0003                     */
      /***********************************************************************/
      case SET_TYPE_NULL:
      {

	 int count;
	 int i;


         /* Load the execute mailbox with the command to be executed         */
         ENT_PUTSRX( bus + WRK.exec_mail_box, (short)SET_TYPE ); 

         /* Set Count to 0 */
         ENT_PUTCX( bus + WRK.exec_mail_box + 5, 0 );
         break;

      }

      /***********************************************************************/
      /*  Indication Enable/Disable               0x0004                     */
      /***********************************************************************/
      case INDICAT_EN:
      case INDICAT_DS:
      {

         /* Load the execute mailbox with the command to be executed         */
         ENT_PUTSRX( bus + WRK.exec_mail_box, (short)INDICAT_DS); 

         /* Determine Which indication is being set                          */
         if (cmd == INDICAT_DS) {
            /* Disable the indication of adapter interrupts                  */
            ENT_PUTCX( bus + WRK.exec_mail_box + 2, 0x00 );
         }
         else {
            /* Enable the indication of adapter interrupts                   */
            ENT_PUTCX( bus + WRK.exec_mail_box + 2, 0x01 );
         } 

         break;
      }
      /***********************************************************************/
      /*  Report Configuration                    0x0006                     */
      /***********************************************************************/
      case REPORT_CONFIG:
      {

	 int i;

         /* Load the execute mailbox with the command to be executed         */
         ENT_PUTSRX( bus + WRK.exec_mail_box, (short)REPORT_CONFIG); 

         for (i = 0; i < CONFIG_TBL_SIZE; i++) {
            /* Initialize the return area to zero                            */
            ENT_PUTSX( bus + WRK.exec_mail_box + 2 + i, (short)0 );
         }
         break;
      }


      /***********************************************************************/
      /*  Configure Lists                         0x0008                     */
      /***********************************************************************/
      case CONFIG_LIST:
      {

         /* Load the execute mailbox with the command to be executed         */
         ENT_PUTSRX( bus + WRK.exec_mail_box, (short)CONFIG_LIST);
         ENT_PUTSRX( bus + WRK.exec_mail_box + 2, WRK.txd_cnt);
         ENT_PUTSRX( bus + WRK.exec_mail_box + 4, WRK.rvd_cnt);

         break;
      }
      /***********************************************************************/
      /*  586 AL-LOC Off                          0x000D                     */
      /***********************************************************************/
      case AL_LOC_OFF:
      {

        /* Load the execute mailbox with the command to be executed         */
        ENT_PUTSRX( bus + WRK.exec_mail_box, (short)AL_LOC_OFF); 
        ENT_PUTSX( bus + WRK.exec_mail_box + 2, (short)0 ); 
        break;
      }

      default: /* Invalid or unsupported command */

   	BUSIO_DET(ioa);
   	BUSMEM_DET(bus);
	if (!at_int_lvl)
        	unlock_enable(ipri, &CMD_LOCK);

  	TRACE_SYS(HKWD_EN3COM_ERR, "Icm3", EOPNOTSUPP, cmd, 0);

        return(EOPNOTSUPP);

  } 

  /* Issue the execute command to the command register                      */
  /* check the status register for this card                */
  for (i = 0; i <= CRR_DELAY; i++) {

	ENT_GETCX(ioa + STATUS_REG, &host_status_reg);

	/* Test if command reg ready */
	if (host_status_reg & CRR_MSK) {
		break;
	}
	if (i < CRR_DELAY)
		I_DELAYMS(p_dev_ctl, 1);	/* delay 1 ms */
  }

  if (i > CRR_DELAY) {
   	BUSIO_DET(ioa);
   	BUSMEM_DET(bus);
	if (!at_int_lvl) {
        	unlock_enable(ipri, &CMD_LOCK);
		en3com_hard_err(p_dev_ctl, FALSE, FALSE, NDD_PIO_FAIL);
	}

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Icm4", ENETDOWN, host_status_reg, 0);
        return(ENETDOWN);
	
  }

  ENT_PUTCX( ioa + COMMAND_REG, EXECUTE_MSK );
  
  if (pio_rc) {
   	BUSIO_DET(ioa);
   	BUSMEM_DET(bus);
	if (!at_int_lvl) {
        	unlock_enable(ipri, &CMD_LOCK);
		en3com_hard_err(p_dev_ctl, FALSE, FALSE, NDD_PIO_FAIL);
	}

  	TRACE_BOTH(HKWD_EN3COM_ERR, "Icm5", ENETDOWN, pio_rc, 0);
        return(ENETDOWN);
	
  }
	
  /*
   * if we are running in process level code, go to sleep and let the 
   * interrupt handler * to check the exec command status and wake us up.
   */
  if (!at_int_lvl) {

  	p_dev_ctl->ctl_pending = TRUE;

  	/* set null event to sleep on */
  	p_dev_ctl->ctl_event = EVENT_NULL;

  	/* start the ioctl timer */
  	w_start(&(CTLWDT));

  	/*
   	* go to sleep with the cmd_slock freed up/regained. 
   	* Note that the ctl_slock remained locked during the whole time
   	*/

  	e_sleep_thread(&p_dev_ctl->ctl_event, &CMD_LOCK, LOCK_HANDLER);

  	p_dev_ctl->ctl_pending = FALSE;

		
  }
  BUSIO_DET(ioa);
  BUSMEM_DET(bus);

  if (!at_int_lvl)
  	unlock_enable(ipri, &CMD_LOCK);

  TRACE_SYS(HKWD_EN3COM_OTHER, "IcmE", (ulong)p_dev_ctl->ctl_status, 0, 0);
  return(p_dev_ctl->ctl_status);


}

/*****************************************************************************/
/*
 * NAME:     en3com_getmib
 *
 * FUNCTION: Gather the current statistics from the adapter to the MIB table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_ctl
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * RETURNS:  
 *	0 - OK
 *	ENETDOWN - device is down
 *	ENETUNREACH - device is currently unreachable
 */
/*****************************************************************************/
en3com_getmib(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to device control area */
  
{
  int   ipri;
  int   bus;
  int   offset;
  int   pio_rc = 0;
  int   temp;


  TRACE_SYS(HKWD_EN3COM_OTHER, "IgmB", (ulong)p_dev_ctl, 0, 0);

  /*
   * Get the cmd_lock and check the device state, make sure that the 
   * error recovery has not occurred.
   */
  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  if (p_dev_ctl->device_state != OPENED) {
	if (p_dev_ctl->device_state == DEAD) {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Igm1", ENETDOWN, 
			p_dev_ctl->device_state, 0);
		return(ENETDOWN);
	}
	else {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Igm2", ENETUNREACH, 
			p_dev_ctl->device_state, 0);
		return(ENETUNREACH);
	}
  }

  /* Get access to the I/O bus to access I/O registers                   */
  bus = (int)BUSMEM_ATT( (ulong)DDS.bus_id, DDS.bus_mem_addr );
  offset = bus + WRK.stat_count_off;

  /*
   * Read four bytes of counters from adapter. 
   * Update the MIB table.
   */

  ENT_GETLRX(offset + RV_CRC, &MIB.Ethernet_mib.Dot3StatsEntry.fcs_errs);
  ENT_GETLRX(offset + RV_ALIGN, &MIB.Ethernet_mib.Dot3StatsEntry.align_errs);
  ENT_GETLRX(offset + RV_OVERRUN, &MIB.Ethernet_mib.Dot3StatsEntry.mac_rx_errs);
  ENT_GETLRX(offset + RV_SHORT, &temp);
  MIB.Ethernet_mib.Dot3StatsEntry.mac_rx_errs += temp;
  ENT_GETLRX(offset + RV_RSC, &temp);
  MIB.Ethernet_mib.Dot3StatsEntry.mac_rx_errs += temp;

  ENT_GETLRX(offset + RV_LONG, &MIB.Ethernet_mib.Dot3StatsEntry.long_frames);
  ENT_GETLRX(offset + TX_MAX_COLL, 
	&MIB.Ethernet_mib.Dot3StatsEntry.excess_collisions); 
  ENT_GETLRX(offset + TX_NO_CS, 
	&MIB.Ethernet_mib.Dot3StatsEntry.carriers_sense);
  ENT_GETLRX(offset + TX_UNDERRUN, 
	&MIB.Ethernet_mib.Dot3StatsEntry.mac_tx_errs);
  ENT_GETLRX(offset + TX_CLS, &temp);
  MIB.Ethernet_mib.Dot3StatsEntry.mac_tx_errs += temp;
  ENT_GETLRX(offset + TX_TMOUT, &temp);
  MIB.Ethernet_mib.Dot3StatsEntry.mac_tx_errs += temp;


  /* Test if the ROS on card supports extended counters                  */
  if (WRK.vpd_hex_rosl >= 0x0E) {
	ENT_GETLRX(offset + TX_ONE_COLL, 
		&MIB.Ethernet_mib.Dot3StatsEntry.s_coll_frames); 
  	ENT_GETLRX(offset + TX_MULTI_COLL,
       		&MIB.Ethernet_mib.Dot3StatsEntry.m_coll_frames); 
  }

  BUSMEM_DET(bus);                /* restore I/O Bus                     */

  MIB.Generic_mib.RcvAddrTable = WRK.multi_count + 2;
  MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok = ENTSTATS.mcast_tx_ok;
  MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok = ENTSTATS.bcast_tx_ok;
  MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok = ENTSTATS.mcast_rx_ok;
  MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok = ENTSTATS.bcast_rx_ok;
  MIB.Generic_mib.ifExtnsEntry.promiscuous = 
	(WRK.promiscuous_count) ? PROMTRUE : PROMFALSE;


  unlock_enable(ipri, &CMD_LOCK);
  
  if (pio_rc) {
	en3com_hard_err(p_dev_ctl, FALSE, FALSE, NDD_PIO_FAIL);
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Igm3", ENETDOWN, pio_rc, 0);
	return(ENETDOWN);
  }
	

  TRACE_SYS(HKWD_EN3COM_OTHER, "IgmE", 0, 0, 0);
  return(0);



} 
/*****************************************************************************/
/*
 * NAME:     en3com_getstat
 *
 * FUNCTION: Gather the current statistics from the adapter to the device
 *	     stats table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_ctl
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * RETURNS:  
 *	0 - OK
 *	ENETDOWN - device is down
 *	ENETUNREACH - device is currently unreachable
 */
/*****************************************************************************/
en3com_getstat(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to device control area */
  
{
  int   ipri;
  int   bus;
  int   offset;
  int   i;
  int   pio_rc = 0;


  TRACE_SYS(HKWD_EN3COM_OTHER, "IgsB", (ulong)p_dev_ctl, 0, 0);

  /*
   * Get the cmd_lock and check the device state, make sure that the 
   * error recovery has not occurred.
   */
  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  if (p_dev_ctl->device_state != OPENED) {
	if (p_dev_ctl->device_state == DEAD) {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Igs1", ENETDOWN, 
			p_dev_ctl->device_state, 0);
		return(ENETDOWN);
	}
	else {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Igs2", ENETUNREACH, 
			p_dev_ctl->device_state, 0);
		return(ENETUNREACH);
	}
  }

  NDD.ndd_genstats.ndd_elapsed_time = ELAPSED_TIME(p_dev_ctl->ndd_stime);
  ENTSTATS.dev_elapsed_time = ELAPSED_TIME(p_dev_ctl->dev_stime);

  /* Get access to the I/O bus to access I/O registers                   */
  bus = (int)BUSMEM_ATT( (ulong)DDS.bus_id, DDS.bus_mem_addr );
  offset = bus + WRK.stat_count_off;

  /*
   * Read four bytes of counters from adapter. 
   * Update the device specific stats table.
   */

  ENT_GETLRX(offset + RV_CRC, &ENTSTATS.fcs_errs);
  ENT_GETLRX(offset + RV_ALIGN, &ENTSTATS.align_errs);
  ENT_GETLRX(offset + RV_OVERRUN, &ENTSTATS.overrun);
  ENT_GETLRX(offset + RV_SHORT, &ENTSTATS.short_frames);
  ENT_GETLRX(offset + RV_LONG, &ENTSTATS.long_frames);
  ENT_GETLRX(offset + RV_RSC, &ENTSTATS.no_resources);
  ENT_GETLRX(offset + RV_DISCARD, &ENTSTATS.rx_drop);
  ENT_GETLRX(offset + ADPT_RV_START, &ENTSTATS.start_rx);
  ENT_GETLRX(offset + TX_NO_CS, &ENTSTATS.carrier_sense);
  ENT_GETLRX(offset + TX_UNDERRUN, &ENTSTATS.underrun);
  ENT_GETLRX(offset + TX_CLS, &ENTSTATS.cts_lost);
  ENT_GETLRX(offset + TX_MAX_COLL, &ENTSTATS.excess_collisions);
  ENT_GETLRX(offset + TX_TMOUT, &ENTSTATS.tx_timeouts);

  /* Test if the ROS on card supports extended counters                  */
  if (WRK.vpd_hex_rosl >= 0x0E) {
  	ENT_GETLRX(offset + TX_ONE_COLL, &ENTSTATS.s_coll_frames);
  	ENT_GETLRX(offset + TX_MULTI_COLL, &ENTSTATS.m_coll_frames);
	ENT_GETLRX(offset + ADPT_RV_EL, &DEVSTATS.host_rcv_eol);
        ENT_GETLRX(offset + ADPT_RSC_586, &DEVSTATS.adpt_rcv_eol);
        ENT_GETLRX(offset + ADPT_RVPKTS_OK, &DEVSTATS.adpt_rcv_pack);
        ENT_GETLRX(offset + ADPT_DMA_TMOUT, &DEVSTATS.rcv_dma_to);

        /* Read 5 read only adapter counters/state machines               */
	for (i = 0; i < EN3COM_USE; i ++) {
            ENT_GETSRX(offset + ADPT_3COM_STAT + i * 2, &DEVSTATS.reserved[i]);
        } 
  }

  BUSMEM_DET(bus);                /* restore I/O Bus                     */

  /* get the rest of the statistics */
  /* ierrors is all the hardware input errors that result in dropping    */
  /* packets. If bad packet is armed, the received bad packets should    */
  /* not be included in the ierrors because they are not dropped by h/w. */
  NDD.ndd_genstats.ndd_ierrors = ENTSTATS.fcs_errs + ENTSTATS.align_errs +
	ENTSTATS.overrun + ENTSTATS.short_frames + ENTSTATS.long_frames +
	ENTSTATS.no_resources + ENTSTATS.rx_drop - 
	NDD.ndd_genstats.ndd_ibadpackets;
  NDD.ndd_genstats.ndd_xmitque_cur = p_dev_ctl->txq_len + p_dev_ctl->tx_pending;

  ENTSTATS.ndd_flags = NDD.ndd_flags;
  for (i = 0; i < ENT_NADR_LENGTH; i++) {
	ENTSTATS.ent_nadr[i] = WRK.net_addr[i];
  }
  ENTSTATS.sw_txq_len = p_dev_ctl->txq_len;	
  ENTSTATS.hw_txq_len = p_dev_ctl->tx_pending; 
  ENTSTATS.device_type = ENT_3COM;
  ENTSTATS.restart_count = WRK.restart_count;
  DEVSTATS.multi_promis_mode = WRK.multi_promis_mode;
  DEVSTATS.rv_pool_size = WRK.rvd_cnt;
  DEVSTATS.tx_pool_size = WRK.txd_cnt;

  unlock_enable(ipri, &CMD_LOCK);
  
  if (pio_rc) {
	en3com_hard_err(p_dev_ctl, FALSE, FALSE, NDD_PIO_FAIL);
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Igs3", ENETDOWN, pio_rc, 0);
	return(ENETDOWN);
  }
	
  TRACE_SYS(HKWD_EN3COM_OTHER, "IgsE", 0, 0, 0);
  return(0);



} 
/*****************************************************************************/
/*
 * NAME:     en3com_clrstat
 *
 * FUNCTION: Clear all the network statistics and device statistics. 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      en3com_ctl
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * RETURNS:  
 *	0 - OK
 *	ENETDOWN - device is down
 *	ENETUNREACH - device is currently unreachable
 */
/*****************************************************************************/
en3com_clrstat(
  en3com_dev_ctl_t	*p_dev_ctl)	/* pointer to device control area */
  
{
  int   ipri;
  int   bus;
  int   offset;
  int   i;
  int   pio_rc = 0;


  TRACE_SYS(HKWD_EN3COM_OTHER, "IclB", (ulong)p_dev_ctl, 0, 0);

  /*
   * Get the cmd_lock and check the device state, make sure that the 
   * error recovery has not occurred.
   */
  ipri = disable_lock(PL_IMP, &CMD_LOCK);
  if (p_dev_ctl->device_state != OPENED) {
  	if (p_dev_ctl->device_state == DEAD) {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Icl1", ENETDOWN, 
			p_dev_ctl->device_state, 0);
		return(ENETDOWN);
	}
	else {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_EN3COM_ERR, "Icl2", ENETUNREACH, 
			p_dev_ctl->device_state, 0);
		return(ENETUNREACH);
	}
  }

  /* reset the start time for both ndd and device */
  p_dev_ctl->ndd_stime = p_dev_ctl->dev_stime = lbolt;

  /* clear all ndd_genstats */
  bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));

  /* Get access to the I/O bus to access I/O registers                   */
  bus = (int)BUSMEM_ATT( (ulong)DDS.bus_id, DDS.bus_mem_addr );
  offset = bus + WRK.stat_count_off;

  /*
   * Clear all the counters on the adapter. 
   */

  for (i = 0; i < ADPT_3COM_STAT; i+=4) {
	ENT_PUTLRX(offset + i, 0);
  }

  /* skip the 3com reserved stats and continue */
  if (WRK.vpd_hex_rosl >= 0x0E) {
    for (i = ADPT_TXPKTS_DN; i < 0x82; i+=4) {
	ENT_PUTLRX(offset + i, 0);
    }
    ENT_PUTSRX(offset + i, 0);
  }

  WRK.restart_count = 0; 	  /* reset the restart count, too */
  ENTSTATS.mcast_rx_ok =  ENTSTATS.bcast_rx_ok =  ENTSTATS.mcast_tx_ok =  
  	ENTSTATS.bcast_tx_ok = 0;

  BUSMEM_DET(bus);                /* restore I/O Bus                     */
  
  unlock_enable(ipri, &CMD_LOCK);

  if (pio_rc) {
	en3com_hard_err(p_dev_ctl, FALSE, FALSE, NDD_PIO_FAIL);
  	TRACE_BOTH(HKWD_EN3COM_ERR, "Icl3", ENETDOWN, pio_rc, 0);
	return(ENETDOWN);
  }

  TRACE_SYS(HKWD_EN3COM_OTHER, "IclE", 0, 0, 0);
  return(0);



} 
