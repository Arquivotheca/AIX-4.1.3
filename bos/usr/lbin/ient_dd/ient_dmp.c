static char sccsid[] = "@(#)66	1.2  src/bos/usr/lbin/ient_dd/ient_dmp.c, sysxient, bos41J, 9512A_all 3/14/95 14:08:20";
/*
 *   COMPONENT_NAME: sysxient
 *
 *   FUNCTIONS: dmp_entry
 *		fmt_cur_cfg
 *		fmt_dds
 *		fmt_dev
 *		fmt_devstats
 *		fmt_entstats
 *		fmt_ndd
 *		fmt_ndd_genstats
 *		fmt_tbl
 *		fmt_wrk
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
#include <stdio.h>
#include <sys/dump.h>
#include <stddef.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <malloc.h>
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
#include <sys/ndd.h>

#include <sys/cdli.h>
#include <sys/generic_mibs.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>


#include "i_entmac.h"
#include "i_entdds.h"
#include "i_enthw.h"
#include "i_ent.h"


/*****************************************************************************/
/*
 * NAME:     dmp_entry
 *
 * FUNCTION: fddi dump formatter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      main 
 *
 * INPUT:
 *      kmem		- the fd of the dump file. 
 *	name		- the character string of the dump entry name
 *      len             - length for this dump entry
 *
 * RETURNS:  
 *	0 - OK
 *	-1 - Can't read the dump file
 */
/*****************************************************************************/

dmp_entry(kmem, p_cdt_entry)
    int			kmem;
    struct cdt_entry	*p_cdt_entry;
{
	char		*buf;
    	char		*name;
    	int		len;
    	char		*d_ptr;

	int		rc;    

    	/*
     	*  Get values from the ctd_entry.
     	*/
    	name  = p_cdt_entry->d_name;
    	len   = p_cdt_entry->d_len;
    	d_ptr = p_cdt_entry->d_ptr;

	buf = (char *)malloc(len);

	if (buf == 0)
	{
		printf("No Memory \n");
		return(ENOMEM);
	}
    
    	if (!strcmp(name, "Trace")) 
	{
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}
	
		fmt_tbl(buf, d_ptr, len);
		return(0);
    	}
    
    	if (!strcmp(name, "Cfg DDS")) 
	{
		if (len < sizeof(ient_dds_t))
		{
			printf("Buffer too small, len = %d, ient_dds_t = %d\n",
				len, sizeof(ient_dds_t));
			free(buf);
			return(E2BIG);
		}
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}

		fmt_dds(buf);
		return(0);
    	}
    
    	if (!strcmp("Dev CS", name)) 
	{

		if (len < sizeof(ient_dev_ctl_t))
		{
			printf("Buffer too small, len = %d, ient_dev_ctl_t = %d\n",
				len, sizeof(ient_dev_ctl_t));
			free(buf);
			return(E2BIG);
		}
		if ((rc = read(kmem, buf, len)) != len) 
		{
			free(buf);
	    		return(-1);
		}
	
		fmt_dev(buf, d_ptr);
		return(0);
    	}
    
    	if (!strcmp("Dev Wrk", name)) 
	{

		if (len < sizeof(ient_wrk_t))
		{
			printf("Buffer too small, len = %d, ient_wrk_t = %d\n",
				len, sizeof(ient_wrk_t));
			free(buf);
			return(E2BIG);
		}
		if ((rc = read(kmem, buf, len)) != len) 
		{
			free(buf);
	    		return(-1);
		}
	
		fmt_wrk(buf, d_ptr);
		return(0);
    	}
    
	free(buf);
	printf("Undefined Name [%s]\n",name);
	return(0);
} /* End of dmp_entry() */

/*
 * NAME:     fmt_tbl
 * FUNCTION: dumps ient_trace_tbl structure
 * INPUTS:   ient_trace_tbl structure
 * RETURNS:  none
 */
fmt_tbl(p_tbl, d_ptr, len)
    	ient_trace_t	*p_tbl;
    	char		*d_ptr;
	int 		len;
{
	int 	i;
	char 	*tmpc;

    	printf("\n<< Dump of Trace (%08x) >> :\n\n", d_ptr);

    	hex_dmp("         ", &p_tbl->table[0], (len-16));
} /* End of fmt_tbl() */

/*
 * NAME:     fmt_dev
 * FUNCTION: dumps  ient_dev_ctl_t structure
 * INPUTS:   ient_dev_ctl_t structure
 * RETURNS:  none
 */
fmt_dev(p_dev, d_ptr)
    	ient_dev_ctl_t	*p_dev;
    	char		*d_ptr;
{
	printf("\n<< Dump of IENT DEV CTL Structure (%08x) >> :\n", d_ptr);

	printf("\nIHS_SECTION - (struct intr) : \n\n");
    	hex_dmp("    ", &p_dev->ihs_section, sizeof(struct intr));

	fmt_ndd(&p_dev->ndd_section);

	printf("\nSEQ_NUMBER = %6 %d\n",p_dev->seq_number);

	printf("DEVICE_STATE = %4 ");
	switch(p_dev->device_state)
	{
		case CLOSED :
			printf("CLOSED\n");
			break;

		case OPEN_PENDING :
			printf("OPEN_PENDING\n");
			break;
		case OPENED :
			printf("OPENED\n");
			break;
		case CLOSE_PENDING :
			printf("CLOSE_PENDING\n");
			break;
		case LIMBO :
			printf("LIMBO\n");
			break;
		case DEAD :
			printf("DEAD\n");
			break;
	};

	printf("\nWDT_SECTION - (struct watchdog):\n");
    	hex_dmp("    ", &p_dev->wdt_section, sizeof(struct watchdog));

	fmt_entstats(&p_dev->entstats);
	fmt_devstats(&p_dev->devstats);
	printf("\nMIBS - (struct ethernet_all_mib_t):\n\n");
	hex_dmp("    ", &p_dev->mibs, sizeof(ethernet_all_mib_t));
} /* End of fmt_dev() */

/*
 * NAME:     fmt_dds
 * FUNCTION: dumps dds_section 
 * INPUTS:   
 * RETURNS:  none
 */
fmt_dds(p_dds)
    	ient_dds_t		*p_dds;
{
	int 	i;
    
	printf("\nDDS - (struct ient_dds):\n\n");

	printf("%4 bus_type = %8 %08x\n",p_dds->bus_type);
	printf("%4 bus_id = %10 %08x\n",p_dds->bus_id);
	printf("%4 intr_level = %6 %08x\n",p_dds->intr_level);
	printf("%4 intr_priority = %3 %08x\n",p_dds->intr_priority);
	printf("%4 xmt_que_size = %4 %d\n",p_dds->xmt_que_size);
	printf("%4 lname = %11 [%s]\n",p_dds->lname);
	printf("%4 alias = %11 [%s]\n",p_dds->alias);
	printf("%4 bus_mem_addr = %4 %08x\n",p_dds->bus_mem_addr);
	printf("%4 tcw_bus_mem_addr = %08x\n",p_dds->tcw_bus_mem_addr);
	printf("%4 tcw_bus_mem_size = %08x\n",p_dds->tcw_bus_mem_size);
	printf("%4 io_port = %9 %08x\n",p_dds->io_port);
	printf("%4 slot = %12 %08x\n",p_dds->slot);
	printf("%4 dma_arbit_lvl = %3 %08x\n",p_dds->dma_arbit_lvl);
	if (p_dds->use_alt_addr)
		printf("%4 use_alt_addr = %4 TRUE\n");
	else
		printf("%4 use_alt_addr = %4 FALSE\n");
	printf("%4 alt_addr = %8 [%02x%02x%02x%02x%02x%02x]\n",
		p_dds->alt_addr[0], p_dds->alt_addr[1], p_dds->alt_addr[2], 
		p_dds->alt_addr[3], p_dds->alt_addr[4], p_dds->alt_addr[5]);
	printf("%4 eth_addr = %8 [%02x%02x%02x%02x%02x%02x]\n",
		p_dds->eth_addr[0], p_dds->eth_addr[1], p_dds->eth_addr[2], 
		p_dds->eth_addr[3], p_dds->eth_addr[4], p_dds->eth_addr[5]);
    
} /* End of fmt_dds() */


/*
 * NAME:     fmt_wrk
 * FUNCTION: dumps wrk_section 
 * INPUTS:   
 * RETURNS:  none
 */
fmt_wrk(p_wrk)
    	ient_wrk_t		*p_wrk;
{
	int 	i;
    
	printf("\nWRK_SECTION - (struct ient_wrk):\n\n");
	printf("%4 wdt_setter = %7 ");
	switch (p_wrk->wdt_setter)
	{
	case WDT_INACTIVE:
		printf("WDT_INACTIVE\n");
		break;
	case WDT_CONNECT:
		printf("WDT_CONNECT\n");
		break;
	case WDT_XMIT:
		printf("WDT_XMIT\n");
		break;
	case WDT_ACT:
		printf("WDT_ACT\n");
		break;
	case WDT_CLOSE:
		printf("WDT_CLOSE\n");
		break;
	case WDT_CFG:
		printf("WDT_CFG\n");
		break;
	case WDT_IAS:
		printf("WDT_IAS\n");
		break;
	default:
		printf("UNDEFINED\n");
		break;
	};

   	printf("%4 connection_result = %08x\n",p_wrk->connection_result); 
   	printf("%4 channel_allocated = %08x\n",p_wrk->channel_allocated); 
   	printf("%4 timeout = %10 %08x\n",p_wrk->timeout); 
	printf("%4 lock_anchor\n");
	hex_dmp("        ",&p_wrk->lock_anchor,sizeof(lock_t));
	printf("\n");

	printf("%4 sleep_anchor = %5 %08x\n",p_wrk->sleep_anchor);
	printf("%4 ndd_stime = %8 %08x\n",p_wrk->ndd_stime);
   	printf("%4 dev_stime = %8 %08x\n",p_wrk->dev_stime); 
   	printf("%4 dma_channel = %6 %08x\n",p_wrk->dma_channel); 
   	printf("%4 close_event = %6 %08x\n",p_wrk->close_event); 
   	printf("%4 dump_started = %5 %08x\n",p_wrk->dump_started); 
   	printf("%4 dump_dsap = %8 %08x\n",p_wrk->dump_dsap); 
	
	printf("%4 xbuf_xd\n");
	hex_dmp("        ",&p_wrk->xbuf_xd, sizeof (struct xmem));
	printf("\n");

	printf("%4 scp_ptr = %11 %08x\n",p_wrk->scp_ptr);
	printf("%4 iscp_ptr = %10 %08x\n",p_wrk->iscp_ptr);
	printf("%4 scb_ptr = %11 %08x\n",p_wrk->scb_ptr);
	printf("%4 xcbl_ptr = %10 %08x\n",p_wrk->xcbl_ptr);
	printf("%4 acbl_ptr = %10 %08x\n",p_wrk->acbl_ptr);
	printf("%4 ncbl_ptr = %10 %08x\n",p_wrk->ncbl_ptr);
	printf("%4 scp_addr = %10 %08x\n",p_wrk->scp_addr);
	printf("%4 iscp_addr = %9 %08x\n",p_wrk->iscp_addr);
	printf("%4 scb_addr = %10 %08x\n",p_wrk->scb_addr);
	printf("%4 xcbl_addr = %9 %08x\n",p_wrk->xcbl_addr);
	printf("%4 acbl_addr = %9 %08x\n",p_wrk->acbl_addr);
	printf("%4 ncbl_addr = %9 %08x\n",p_wrk->ncbl_addr);
	printf("\n");

	printf("%4 sysmem = %11 %08x\n",p_wrk->sysmem);
	printf("%4 sysmem_end = %7 %08x\n",p_wrk->sysmem_end);
	printf("%4 asicram = %10 %08x\n",p_wrk->asicram);
	printf("%4 asicram_end = %6 %08x\n",p_wrk->asicram_end);
	printf("%4 dma_base = %9 %08x\n",p_wrk->dma_base);
	printf("%4 alloc_size = %7 %08x\n",p_wrk->alloc_size);
	printf("%4 control_pending = %2 %08x\n",p_wrk->control_pending);
	printf("%4 action_que_active = %08x\n",p_wrk->action_que_active);
	printf("\n");
	printf("%4 rfd_ptr = %10 %08x\n",p_wrk->rfd_ptr);
	printf("%4 rfd_addr = %9 %08x\n",p_wrk->rfd_addr);
	printf("%4 rbd_ptr = %10 %08x\n",p_wrk->rbd_ptr);
	printf("%4 rbd_addr = %9 %08x\n",p_wrk->rbd_addr);
	printf("%4 rbd_el_ptr = %7 %08x\n",p_wrk->rbd_el_ptr);
	printf("%4 save_bf = %10 %08x\n",p_wrk->save_bf);
	printf("%4 begin_fbl = %8 %08x\n",p_wrk->begin_fbl);
	printf("%4 end_fbl = %10 %08x\n",p_wrk->end_fbl);
	printf("%4 buffer_in = %8 %08x\n",p_wrk->buffer_in);
	printf("%4 buffer_out = %7 %08x\n",p_wrk->buffer_out);
	printf("%4 el_ptr = %11 %08x\n",p_wrk->el_ptr);
	printf("\n");

	printf("%4 recv_buffers = %5 %08x\n",p_wrk->recv_buffers);
	printf("%4 recv_buf_ptr = %5 %08x\n",p_wrk->recv_buf_ptr);
	printf("%4 recv_buf_addr = %4 %08x\n",p_wrk->recv_buf_addr);
	printf("%4 xmit_buf_ptr = %5 %08x\n",p_wrk->xmit_buf_ptr);
	printf("%4 xmit_buf_addr = %4 %08x\n",p_wrk->xmit_buf_addr);
	printf("%4 txq_first = %8 %08x\n",p_wrk->txq_first);
	printf("%4 txq_last = %9 %08x\n",p_wrk->txq_last);
	printf("%4 tbd_ptr = %10 %08x\n",p_wrk->tbd_ptr);
	printf("%4 tbd_addr = %9 %08x\n",p_wrk->tbd_addr);
	printf("%4 aram_mask = %8 %08x\n",p_wrk->aram_mask);
	printf("%4 xmits_queued = %5 %08x\n",p_wrk->xmits_queued);
	printf("%4 xmits_buffered = %3 %08x\n",p_wrk->xmits_buffered);
	printf("%4 xmits_started = %4 %08x\n",p_wrk->xmits_started);

	printf("\n");
	printf("\n%4 ent_addr = %9 [%02x%02x%02x%02x%02x%02x]\n\n",
		p_wrk->ent_addr[0], p_wrk->ent_addr[1], p_wrk->ent_addr[2], 
		p_wrk->ent_addr[3], p_wrk->ent_addr[4], p_wrk->ent_addr[5]);

	for (i=0; i<8; i++)
	{
		printf("%4 pos_reg[%d] = %7 %02x\n",i,p_wrk->pos_reg[i]);
	}
	printf("\n");

	printf("%4 promiscuous_count = %2 %d\n",p_wrk->promiscuous_count);
	printf("%4 badframe_user_count = %d\n",p_wrk->badframe_user_count);
	printf("%4 alt_count = %10 %d\n",p_wrk->alt_count);
	printf("%4 multi_count = %8 %d\n",p_wrk->multi_count);
	printf("\n");	
	
	printf("%4 alt_list = %9 %08x\n", p_wrk->alt_list);
	printf("%4 restart = %10 %04x\n",p_wrk->restart);
	printf("%4 restart_ru = %7 %04x\n",p_wrk->restart_ru);
	printf("%4 machine = %10 %08x\n", p_wrk->machine);
	printf("%4 do_ca_on_intr = %4 %08x\n",p_wrk->do_ca_on_intr);
	printf("%4 intr_registered = %2 %08x\n",p_wrk->intr_registered);
	printf("%4 timr_registered = %2 %08x\n",p_wrk->timr_registered);
	
	fmt_cur_cfg(&p_wrk->cur_cfg);

	
} /* End of fmt_wrk() */


/*
 * NAME:     fmt_cur_cfg
 * FUNCTION: dumps cur_cfg 
 * INPUTS:   
 * RETURNS:  none
 */
fmt_cur_cfg(p_cfg)
    	struct cfg		*p_cfg;
{
	int 	i;
    
	printf("\nCUR_CFG - (struct cfg (i_enthw.h)):\n\n");

	printf("%4 next_cb = %10 %08x\n",p_cfg->next_cb);
	printf("%4 byte_count = %7 %02x\n",p_cfg->byte_count);
	printf("%4 fifo_limit = %7 %02x\n",p_cfg->fifo_limit);
	printf("%4 save_bf = %10 %02x\n",p_cfg->save_bf);
	printf("%4 loopback = %9 %02x\n",p_cfg->loopback);
	printf("%4 linear_pri = %7 %02x\n",p_cfg->linear_pri);
	printf("%4 spacing = %10 %02x\n",p_cfg->spacing);
	printf("%4 slot_time_low = %4 %02x\n",p_cfg->slot_time_low);
	printf("%4 slot_time_up = %5 %02x\n",p_cfg->slot_time_up);
	printf("%4 promiscuous = %6 %02x\n",p_cfg->promiscuous);
	printf("%4 carrier_sense = %4 %02x\n",p_cfg->carrier_sense);
	printf("%4 frame_len = %8 %02x\n",p_cfg->frame_len);
	printf("%4 preamble = %9 %02x\n",p_cfg->preamble);
	printf("%4 dcr_slot = %9 %02x\n",p_cfg->dcr_slot);
	printf("%4 dcr_num = %10 %02x\n",p_cfg->byte_count);

    
} /* End of fmt_cur_cfg() */


/*****************************************************************************/
/*
 * NAME:     fmt_entstats
 *
 * FUNCTION: The format routine for the ent_genstats_t structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_entstats(p_ls)
    ent_genstats_t	*p_ls;
{
	int	i;
	printf("\nENTSTATS - (struct ent_genstats_t):\n\n");
	
	printf("%4 device_type = %6 %d\n",p_ls->device_type);
	printf("%4 dev_elapsed_time =  %d\n",p_ls->dev_elapsed_time);

    	printf("\n%4 ndd_flags: %9 (");
    	if (p_ls->ndd_flags & NDD_UP) 
		printf(" UP");
    	if (p_ls->ndd_flags & NDD_BROADCAST) 
		printf(" BROADCAST");
    	if (p_ls->ndd_flags & NDD_DEBUG) 
		printf(" DEBUG");
    	if (p_ls->ndd_flags & NDD_RUNNING) 
		printf(" RUNNING");
    	if (p_ls->ndd_flags & NDD_SIMPLEX) 
		printf(" SIMPLEX");
    	if (p_ls->ndd_flags & NDD_DEAD) 
		printf(" DEAD");
    	if (p_ls->ndd_flags & NDD_LIMBO) 
		printf(" LIMBO");
	
    	printf("\n%25 ");
	
    	if (p_ls->ndd_flags & NDD_PROMISC) 
		printf(" PROMISC");
    	if (p_ls->ndd_flags & NDD_ALTADDRS) 
		printf(" ALTADDRS");
    	if (p_ls->ndd_flags & NDD_MULTICAST) 
		printf(" MULTICAST");
    	if (p_ls->ndd_flags & ENT_RCV_BAD_FRAME)
		printf(" RCV_BAD_FRAME");
    	printf(")\n\n");

	printf("%4 ent_nadr = %9 [%02x%02x%02x%02x%02x%02x]\n",
		p_ls->ent_nadr[0], p_ls->ent_nadr[1], p_ls->ent_nadr[2], 
		p_ls->ent_nadr[3], p_ls->ent_nadr[4], p_ls->ent_nadr[5]);

	printf("%4 mcast_rx_ok = %6 %d\n",p_ls->mcast_rx_ok);
	printf("%4 bcast_rx_ok = %6 %d\n",p_ls->bcast_rx_ok);
	printf("%4 fcs_errs = %9 %d\n",p_ls->fcs_errs);
	printf("%4 align_errs = %7 %d\n",p_ls->align_errs);
	printf("%4 overrun = %10 %d\n",p_ls->overrun);
	printf("%4 short_frames = %5 %d\n",p_ls->short_frames);
	printf("%4 long_frames = %6 %d\n",p_ls->long_frames);
	printf("%4 no_resources = %5 %d\n",p_ls->no_resources);
	printf("%4 rx_collisions = %4 %d\n",p_ls->rx_collisions);
	printf("%4 rx_drop = %10 %d\n",p_ls->rx_drop);
	printf("%4 start_rx = %9 %d\n",p_ls->start_rx);
	printf("%4 mcast_tx_ok = %6 %d\n",p_ls->mcast_tx_ok);
	printf("%4 bcast_tx_ok = %6 %d\n",p_ls->bcast_tx_ok);
	printf("%4 carrier_sense = %4 %d\n",p_ls->carrier_sense);
	printf("%4 underrun = %9 %d\n",p_ls->underrun);
	printf("%4 cts_lost = %9 %d\n",p_ls->cts_lost);
	printf("%4 excess_collisions = %d\n",p_ls->excess_collisions);
	printf("%4 late_collisions = %2 %d\n",p_ls->excess_collisions);
	printf("%4 tx_timeouts = %6 %d\n",p_ls->tx_timeouts);
	printf("%4 sqetest = %10 %d\n",p_ls->sqetest);
	printf("%4 defer_tx = %9 %d\n",p_ls->sqetest);
	printf("%4 s_coll_frames = %4 %d\n",p_ls->s_coll_frames);
	printf("%4 m_coll_frames = %4 %d\n",p_ls->m_coll_frames);
	printf("%4 sw_txq_len = %7 %d\n",p_ls->sw_txq_len);
	printf("%4 hw_txq_len = %7 %d\n",p_ls->hw_txq_len);
	printf("%4 restart_count = %4 %d\n",p_ls->restart_count);
	printf("%4 reserved1 = %8 %d\n",p_ls->reserved1);
	printf("%4 reserved2 = %8 %d\n",p_ls->reserved2);
	printf("%4 reserved3 = %8 %d\n",p_ls->reserved3);
	printf("%4 reserved4 = %8 %d\n",p_ls->reserved4);
}


/*****************************************************************************/
/*
 * NAME:     fmt_devstats
 *
 * FUNCTION: The format routine for the ient_stats_t structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_devstats(p_ls)
    ient_stats_t	*p_ls;
{
	int	i;
	printf("\nDEVSTATS - (struct ient_stats_t):\n\n");
	
	for (i=0; i<16; i++)
	{
		printf("%4 coll_freq[%02d] = %4 %d\n",i,p_ls->coll_freq[i]);
	}
}


/*****************************************************************************/
/*
 * NAME:     fmt_ndd
 *
 * FUNCTION: The format routine for the ndd structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_ndd(p_ndd)
    ndd_t	*p_ndd;
{
    printf("\nNDD_SECTION - (struct ndd):\n\n");
    printf("%4 ndd_next: %9 %08x%10 ndd_refcnt: %7 %08x\n", 
	   p_ndd->ndd_next, p_ndd->ndd_refcnt);
    printf("%4 ndd_name: %9 %08x\n", p_ndd->ndd_name);
    
    printf("\n%4 ndd_flags: %6 (");
    if (p_ndd->ndd_flags & NDD_UP) 
	printf(" UP");
    if (p_ndd->ndd_flags & NDD_BROADCAST) 
	printf(" BROADCAST");
    if (p_ndd->ndd_flags & NDD_DEBUG) 
	printf(" DEBUG");
    if (p_ndd->ndd_flags & NDD_RUNNING) 
	printf(" RUNNING");
    if (p_ndd->ndd_flags & NDD_SIMPLEX) 
	printf(" SIMPLEX");
    if (p_ndd->ndd_flags & NDD_DEAD) 
	printf(" DEAD");
    if (p_ndd->ndd_flags & NDD_LIMBO) 
	printf(" LIMBO");

    printf("\n%22 ");

    if (p_ndd->ndd_flags & NDD_PROMISC) 
	printf(" PROMISC");
    if (p_ndd->ndd_flags & NDD_ALTADDRS) 
	printf(" ALTADDRS");
    if (p_ndd->ndd_flags & NDD_MULTICAST) 
	printf(" MULTICAST");
    if (p_ndd->ndd_flags & ENT_RCV_BAD_FRAME)
	printf(" RCV_BAD_FRAME");

    printf(")\n\n");
    
    printf("%4 ndd_correlator: %3 %08x\n", p_ndd->ndd_correlator);
    printf("\n%4 ndd_open: %4 %08x%5 ndd_close: %3 %08x%5 ndd_output: %2 %08x\n",
	   p_ndd->ndd_open, p_ndd->ndd_close, p_ndd->ndd_output);
    printf("%4 ndd_ctl: %5 %08x%5 nd_receive: %2 %08x%5 nd_status: %3 %08x\n", 
	   p_ndd->ndd_ctl, p_ndd->nd_receive, p_ndd->nd_status);
    printf("\n%4 ndd_mtu: %10 %08x%10 ndd_mintu: %8 %08x\n", p_ndd->ndd_mtu, 
	   p_ndd->ndd_mintu);
    printf("%4 ndd_type: %9 %08x%10 ndd_addrlen: %6 %08x\n",
	   p_ndd->ndd_type, p_ndd->ndd_addrlen);
    printf("%4 ndd_hdrlen: %7 %08x%10 ndd_physaddr: %5 %08x\n",
	   p_ndd->ndd_hdrlen, p_ndd->ndd_physaddr);
    printf("%4 ndd_demuxer: %6 %08x%10 ndd_nsdemux: %6 %08x\n",
	   p_ndd->ndd_demuxer, p_ndd->ndd_nsdemux);
    printf("%4 ndd_specdemux: %4 %08x%10 ndd_demuxsource: %2 %08x\n",
	   p_ndd->ndd_specdemux, p_ndd->ndd_demuxsource);
    printf("%4 ndd_trace: %8 %08x%10 ndd_trace_arg: %4 %08x\n",
	   p_ndd->ndd_trace, p_ndd->ndd_trace_arg);
    printf("%4 ndd_lock: %9 %08x\n", p_ndd->ndd_lock);
    printf("%4 ndd_reserved:");
    hex_dmp("        ", &p_ndd->ndd_reserved, sizeof(p_ndd->ndd_reserved));

    printf("\n%4 ndd_specstats: %4 %08x%10 ndd_speclen: %6 %08x\n",
	   p_ndd->ndd_specstats, p_ndd->ndd_speclen);
    
    printf("\n%4 ndd.ndd_genstats - (struct ndd_genstats):\n");
    fmt_ndd_genstats(&p_ndd->ndd_genstats);
}

/*****************************************************************************/
/*
 * NAME:     fmt_ndd_genstats
 *
 * FUNCTION: The format routine for the ndd_genstats structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_ndd_genstats(p_ndd)
    ndd_genstats_t	*p_ndd;
{
    printf("\n%8 ipackets_msw: %1 %08x%10 ipackets_lsw: %5 %08x\n",
	   p_ndd->ndd_ipackets_msw, 
	   p_ndd->ndd_ipackets_lsw);
    printf("%8 ibytes_msw: %3 %08x%10 ibytes_lsw: %7 %08x\n",
	   p_ndd->ndd_ibytes_msw, 
	   p_ndd->ndd_ibytes_lsw);
    printf("%8 recvintr_msw: %1 %08x%10 recvintr_lsw: %5 %08x\n",
	   p_ndd->ndd_recvintr_msw, 
	   p_ndd->ndd_recvintr_lsw);
    printf("%8 ierrors: %6 %08x\n",
	   p_ndd->ndd_ierrors);
    printf("%8 opackets_msw: %1 %08x%10 opackets_lsw: %5 %08x\n",
	   p_ndd->ndd_opackets_msw, 
	   p_ndd->ndd_opackets_lsw);
    printf("%8 obytes_msw: %3 %08x%10 obytes_lsw: %7 %08x\n",
	   p_ndd->ndd_obytes_msw, 
	   p_ndd->ndd_obytes_lsw);
    printf("%8 xmitintr_msw: %1 %08x%10 xmitintr_lsw: %5 %08x\n",
	   p_ndd->ndd_xmitintr_msw, 
	   p_ndd->ndd_xmitintr_lsw);
    printf("%8 oerrors: %6 %08x\n", p_ndd->ndd_oerrors);
    printf("%8 xmitque_max: %2 %08x%10 xmitque_ovf: %6 %08x\n",
	   p_ndd->ndd_xmitque_max, 
	   p_ndd->ndd_xmitque_ovf);
    printf("%8 nobufs: %7 %08x\n",
	   p_ndd->ndd_nobufs);
}
