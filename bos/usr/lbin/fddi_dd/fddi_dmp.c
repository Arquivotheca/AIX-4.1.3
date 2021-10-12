static char sccsid[] = "@(#)98	1.2  src/bos/usr/lbin/fddi_dd/fddi_dmp.c, sysxfddi, bos411, 9428A410j 4/28/94 11:00:06";
/*
 *   COMPONENT_NAME: sysxfddi
 *
 *   FUNCTIONS: dmp_entry
 *		fmt_acs
 *		fmt_adap
 *		fmt_addr
 *		fmt_addr_elem
 *		fmt_cmd
 *		fmt_dds
 *		fmt_dev
 *		fmt_ls
 *		fmt_ndd
 *		fmt_ndd_genstats
 *		fmt_rx
 *		fmt_rx_desc
 *		fmt_tbl
 *		fmt_tx
 *		fmt_tx_desc
 *		fmt_vpd
 *		print_addr
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
#include "fddiproto.h"

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

	buf = malloc(len);

	if (buf == 0)
	{
		printf("No Memory \n");
		return(ENOMEM);
	}
    
    	/* 
     	*  dump entry name is "dd_ctl"
     	*/
    	if (!strcmp(name, "fddi_tbl")) 
	{
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}
	
		fmt_tbl(buf, d_ptr, len);
		return(0);
    	}
    
    	/* 
    	 *  dump entry name is "fddi_adap_dump"
     	*/
    	if (!strcmp(name, "adapter")) 
	{
		if (len < sizeof(fddi_adap_dump_t))
		{
			printf("Buffer too small, len = %d, fddi_adap_dump_t = %d\n",
				len, sizeof(fddi_adap_dump_t));
			free(buf);
			return(E2BIG);
		}
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}

		fmt_adap(buf, d_ptr);
		return(0);
    	}
    
    	/* 
     	*  dump entry name is "fddi_acs"
     	*/
    	if (!strcmp("fddi_acs", name)) 
	{

		if (len < sizeof(fddi_acs_t))
		{
			printf("Buffer too small, len = %d, fddi_acs_t = %d\n",
				len, sizeof(fddi_acs_t));
			free(buf);
			return(E2BIG);
		}
		if ((rc = read(kmem, buf, sizeof(fddi_acs_t))) != len) 
		{
			free(buf);
	    		return(-1);
		}
	
		fmt_acs(buf, d_ptr);
		return(0);
    	}
    
	free(buf);
	printf("Undefined Name [%s]\n",name);
	return(0);
} /* End of dmp_entry() */

/*
 * NAME:     fmt_tbl
 * FUNCTION: dumps fddi_tbl structure
 * INPUTS:   fddi_tbl structure
 * RETURNS:  none
 */
fmt_tbl(p_fddi_tbl, d_ptr, len)
    	fddi_tbl_t	*p_fddi_tbl;
    	char		*d_ptr;
	int 		len;
{
	int 	i;
	char 	*tmpc;

    	printf("\n<< Dump of fddi_tbl (%08x) >> :\n\n", d_ptr);

    	printf("%4 table_lock: %08x\n", p_fddi_tbl->table_lock);
	printf("%4 acs_cnt: %d%10 open_cnt: %d\n\n",p_fddi_tbl->acs_cnt,
		p_fddi_tbl->open_cnt);

	for (i=0; i<(FDDI_MAX_ACS);)
	{
		printf("%4 acs pointer %2d: %08x%10 acs pointer %2d: %08x\n",
			i, p_fddi_tbl->p_acs[i], i+1, p_fddi_tbl->p_acs[i+1]);
		i += 2;
	}
	
	tmpc = (char *) p_fddi_tbl;
	printf("\n%4 trace_lock: %08x\n", *(int *)&tmpc[len-4]);
    	printf("%4 trace - (struct fddi_trace):");

	/* 
	 * This is dependent on the structure of the fddi_tbl.  Under the 
	 * current design, the table_lock, acs_cnt, open_cnt, acs pointers, and
	 * some misc. stuff is before the trace table.  Since the trace table
	 * changes size based on the compile options, taking the len and 
	 * removing excess should allow the dump to get the whole trace no
	 * matter what is compiled
	 */

	len = len - offsetof(fddi_tbl_t, trace) - offsetof(fddi_trace_t, table);
    	hex_dmp("         ", &p_fddi_tbl->trace.table[0], len);
} /* End of fmt_tbl() */

/*
 * NAME:     fmt_nd_ctl
 * FUNCTION: dumps scbdmx_ctl structure
 * INPUTS:   scbdmx_ctl structure
 * RETURNS:  none
 */
fmt_acs(p_acs, d_ptr)
    	fddi_acs_t	*p_acs;
    	char		*d_ptr;
{
	printf("\n<< Dump of FDDI ACS Structure (%08x) >> :\n", d_ptr);

	fmt_ndd(&p_acs->ndd);
	fmt_dds(&p_acs->dds);
	fmt_vpd(&p_acs->vpd);
	fmt_tx(&p_acs->tx);
	fmt_rx(&p_acs->rx);

	printf("\nLS - (struct spec_stats):\n\n");
	fmt_ls(&p_acs->ls);
	fmt_dev(&p_acs->dev);
	fmt_addr(&p_acs->addrs);
} /* End of fmt_acs() */

/*
 * NAME:     fmt_adap
 * FUNCTION: dumps adapter data
 * INPUTS:   character pointer of adapter data
 * RETURNS:  none
 */
fmt_adap(p_dump, d_ptr)
    	fddi_adap_dump_t	*p_dump;
    	char			*d_ptr;
{
	int 	i;
	short 	*p_desc;
	int	addr;
    
	printf("\n<< Dump of FDDI Adapter Data (%08x) >> :\n\n", d_ptr);
    
	for (i=0; i<8; i++)
		printf("%4 POS Register %d : %02x\n",i,p_dump->pos[i]);


	printf("\n%4 HSR Register %04x\n",p_dump->hsr);
	
	printf("%4 HCR Register %04x\n",p_dump->hcr);
	
	printf("%4 NS1 Register %04x\n",p_dump->ns1);
	
	printf("%4 NS2 Register %04x\n",p_dump->ns2);
	
	printf("%4 HMR Register %04x\n",p_dump->hmr);
	
	printf("%4 NM1 Register %04x\n",p_dump->nm1);
	
	printf("%4 NM2 Register %04x\n",p_dump->nm2);
	
	printf("%4 Alisa Control Register %04x\n",p_dump->acr);

	printf("\n%4 Transmit Descriptors \n");
	for (i=0; i<FDDI_MAX_TX_DESC; i++)
	{
		/* 
		 * This seems backwards but is correct as the structure for
		 * the adapter descriptor purposefully reversed the address 
		 */
		addr = (p_dump->tx[i].addr_lo << 16) + p_dump->tx[i].addr_hi;

		printf("%4 Desc %2d: DMA Addr %08x : Len %04x : Ctl %04x : Stat %04x\n",
			i, addr, p_dump->tx[i].cnt, p_dump->tx[i].ctl, 
			p_dump->tx[i].stat);
	}

	printf("\n%4 Receive Descriptors \n");
	for (i=0; i<FDDI_MAX_RX_DESC; i++)
	{
		addr = (p_dump->rx[i].addr_lo << 16) + p_dump->rx[i].addr_hi;

		printf("%4 Desc %2d: DMA Addr %08x : Len %04x : Ctl %04x : Stat %04x\n",
			i, addr, p_dump->rx[i].cnt, p_dump->rx[i].ctl, 
			p_dump->rx[i].stat);
	}
	
} /* End of fmt_adap() */

/*****************************************************************************/
/*
 * NAME:     fmt_tx
 *
 * FUNCTION: The format routine for the tx structure.
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
fmt_tx(p_tx)
    fddi_acs_tx_t	*p_tx;
{
	int	i;

	printf("\nTX - (struct fddi_acs_tx):\n\n");

	printf("%4 lock = %13 %08x\n", p_tx->lock);

	printf("%4 hdw_in_use = %7 %8d\n",p_tx->hdw_in_use);
	printf("%4 hdw_nxt_req = %6 %8d%10 hdw_nxt_cmplt = %4 %8d\n",
		p_tx->hdw_nxt_req, p_tx->hdw_nxt_cmplt);

	printf("%4 p_d_base = %9 %08x%10 p_d_sf = %11 %08x\n",
		p_tx->p_d_base, p_tx->p_d_sf);

	printf("%4 p_sf_cache = %7 %08x\n",p_tx->p_sf_cache);
	printf("%4 p_sw_que = %9 %08x%10 sw_in_use = %8 %8d\n",
		p_tx->p_sw_que, p_tx->sw_in_use);

	printf("%4 xmd (struct xmem): ");
    	hex_dmp("        ", &p_tx->xmd, sizeof(struct xmem));
	printf("\n");

	printf("%4 wdt (struct watchdog):  ");
    	hex_dmp("        ", &p_tx->wdt, sizeof(struct watchdog));
	printf("\n");

	for (i=0; i< FDDI_MAX_TX_DESC; i++)
		fmt_tx_desc(&p_tx->desc[i], i);
}


/*****************************************************************************/
/*
 * NAME:     fmt_tx_desc
 *
 * FUNCTION: The format routine for the tx_desc structure.
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
fmt_tx_desc(p_tx, num)
    	fddi_tx_desc_t	*p_tx;
 	int		num;
{
	int	i;
	int	addr;

	printf("\n%4 TX Descriptor #%d - (struct fddi_tx_desc):\n\n",num);
	
	/* 
	 * This seems backwards but is correct as the structure for
	 * the adapter descriptor purposefully reversed the address 
	 */
	addr = (p_tx->adap.addr_lo << 16) + p_tx->adap.addr_hi;

	printf("%8 Desc %2d: DMA Addr %08x : Len %04x : Ctl %04x : Stat %04x\n",
		i, addr, p_tx->adap.cnt, p_tx->adap.ctl, 
		p_tx->adap.stat);

	printf("%8 offset = %11 %04x%10 p_mbuf = %11 %08x\n",
		p_tx->offset, p_tx->p_mbuf);

	printf("%8 p_dump_buf = %3 %08x%10 dump_len = %9 %8d\n",
		p_tx->p_dump_buf, p_tx->dump_len);

	printf("%8 p_d_addr = %5 %08x%10 p_d_sf = %11 %08x\n",
		p_tx->p_d_addr, p_tx->p_d_sf);

	printf("%8 p_sf = %9 %08x%10 eof_jump = %9 %8d\n",
		p_tx->p_sf, p_tx->eof_jump);
}

/*****************************************************************************/
/*
 * NAME:     fmt_rx
 *
 * FUNCTION: The format routine for the rx structure.
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
fmt_rx(p_rx)
    fddi_acs_rx_t	*p_rx;
{
	int	i;

	printf("\nRX - (struct fddi_acs_rx):\n\n");

	printf("%4 nxt_rx = %11 %8d\n",p_rx->nxt_rx);
	printf("%4 arm_val.cnt = %10 %04x\n",p_rx->arm_val.cnt);
	printf("%4 arm_val.ctl = %10 %04x\n",p_rx->arm_val.ctl);
	printf("%4 arm_val.stat = %9 %04x\n",p_rx->arm_val.stat);

	printf("%4 xmd (struct xmem): ");
    	hex_dmp("        ", &p_rx->xmd, sizeof(struct xmem));
	printf("\n");

	printf("%4 p_rx_cache = %7 %08x%10 l_adj_buf = %8 %8d\n",
		p_rx->p_rx_cache, p_rx->l_adj_buf);

	for (i=0; i< FDDI_MAX_RX_DESC; i++)
		fmt_rx_desc(&p_rx->desc[i], i);
}


/*****************************************************************************/
/*
 * NAME:     fmt_rx_desc
 *
 * FUNCTION: The format routine for the rx_desc structure.
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
fmt_rx_desc(p_rx, num)
    	fddi_rx_desc_t	*p_rx;
 	int		num;
{
	int	i;

	printf("\n%4 RX Descriptor #%d - (struct fddi_rx_desc):\n\n",num);
	
	printf("%8 offset = %11 %04x\n",
		p_rx->offset);

	printf("%8 p_buf = %8 %08x%10 p_d_addr = %9 %08x\n",
		p_rx->p_buf, p_rx->p_d_addr);

}

/*****************************************************************************/
/*
 * NAME:     fmt_dev
 *
 * FUNCTION: The format routine for the dev structure.
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
fmt_dev(p_dev)
    fddi_acs_dev_t	*p_dev;
{
	int	i;

	printf("\nDEV - (struct fddi_acs_dev):\n\n");

	printf("%4 State = %11 (%08x) ", p_dev->state);
	switch (p_dev->state)
	{
		case NULL_STATE: 
			printf("Null State \n");
			break;
		case DNLDING_STATE: 
			printf("Dnlding State \n");
			break;
		case DNLD_STATE: 
			printf("Dnld State \n");
			break;
		case INIT_STATE: 
			printf("Init State \n");
			break;
		case OPENING_STATE: 
			printf("Opening State \n");
			break;
		case OPEN_STATE: 
			printf("Open State \n");
			break;
		case LIMBO_STATE: 
			printf("Limbo State \n");
			break;
		case LIMBO_RECOVERY_STATE: 
			printf("Limbo Recovery State \n");
			break;
		case CLOSING_STATE: 
			printf("Closing State \n");
			break;
		case DEAD_STATE: 
			printf("Dead State \n");
			break;
		default: 
			printf("Unknown State \n");
			break;
	}; 

	printf("\n");

	printf("%4 rop = %14 %8d%10 stuck = %12 %8d\n",p_dev->rop,p_dev->stuck);
	printf("%4 dma_channel = %6 %08x%10 hsr_events = %11 %04x\n",
		p_dev->dma_channel, p_dev->hsr_events);

	fmt_cmd(&p_dev->pri_blk);
	fmt_cmd(&p_dev->cmd_blk);
	printf("%4 p_cmd_prog = %8 %08x%10 pri_que = %10 %08x\n",
		p_dev->p_cmd_prog, p_dev->pri_que);

	printf("%4 cmd_wdt (struct watchdog) :");
    	hex_dmp("        ", &p_dev->cmd_wdt, sizeof(struct watchdog));

	printf("%4 cmd_status = %7 %08x\n", p_dev->cmd_status);

	printf("%4 cmd_event = %8 %08x%10 cmd_wait_event = %3 %08x\n",
		p_dev->cmd_event, p_dev->cmd_wait_event);
	
    	printf("\n%4 dev.ls_buf.genstats - (struct ndd_genstats):\n");
	fmt_ndd_genstats(&p_dev->ls_buf.genstats);

	printf("\n%4 p_dev->ls_buf.fddistats - (struct spec_stats):\n\n");
	fmt_ls(&p_dev->ls_buf.fddistats);

	printf("\n%4 cmd_lock (Simple_lock) ");
    	hex_dmp("        ", &p_dev->cmd_lock, sizeof(Simple_lock));
	printf("\n");

	printf("%4 slih_lock (Simple_lock) :");
    	hex_dmp("        ", &p_dev->slih_lock, sizeof(Simple_lock));
	printf("\n");
	
	printf("%4 limbo_wdt (struct watchdog) :");
    	hex_dmp("        ", &p_dev->limbo_wdt, sizeof(struct watchdog));
	printf("\n");

	printf("%4 icr (stuct fddi_icr_cmd) :");
    	hex_dmp("        ", &p_dev->icr, sizeof(struct fddi_icr_cmd));
	printf("\n");

	printf("%4 p_d_kbuf = %9 %08x%10 dma_status = %7 %08x\n",
		p_dev->p_d_kbuf, p_dev->dma_status);

	printf("%4 dma_xmd (struct xmem) :");
    	hex_dmp("        ", &p_dev->dma_xmd, sizeof(struct xmem));
	printf("\n");

	printf("%4 dnld_wdt (struct watchdog) :");
	hex_dmp("         ", &p_dev->dnld_wdt, sizeof(struct watchdog));
	printf("\n");
	
	printf("%4 smt_control = %10 %04x%10 multi_cnt = %8 %8d\n",
		p_dev->smt_control, p_dev->multi_cnt);

	printf("%4 prom_cnt = %9 %8d%10 bea_cnt = %10 %8d\n",
		p_dev->prom_cnt, p_dev->bea_cnt);

	printf("%4 smt_cnt = %10 %8d%10 nsa_cnt = %10 %8d\n",
		p_dev->smt_cnt, p_dev->nsa_cnt);

	printf("%4 bf_cnt = %11 %8d%10 thresh_rtt = %7 %8d\n",
		p_dev->bf_cnt, p_dev->thresh_rtt);

	printf("%4 thresh_trc = %7 %8d%10 thresh_sbf = %7 %8d\n",
		p_dev->thresh_trc, p_dev->thresh_sbf);

	printf("\n");
	for (i=0; i<8; i++)
		printf("%4 pos %d = %18 %02x\n",i,p_dev->pos[i]);
	printf("\n");

	printf("%4 mcerr = %12 %8d%10 pio_rc = %11 %8d\n",
		p_dev->mcerr, p_dev->pio_rc);
	
	printf("%4 iox = %14 %8d\n",p_dev->iox);
	
	printf("%4 stime (time_t) :");
    	hex_dmp("        ", &p_dev->stime, sizeof(time_t));
	printf("\n");

	for (i=0; i<FDDI_STEST_CNT; i++)
		printf("%4 stest %2d = %13 %04x\n",i,p_dev->stest[i]);
	printf("\n");

	print_addr("%4 vpd_addr = %9 ", &p_dev->vpd_addr[0]);

	printf("%4 smt_event_mask = %3 %08x%10 smt_error_mask = %3 %08x\n",
		p_dev->smt_event_mask, p_dev->smt_error_mask);

	printf("%4 addr_index = %7 %8d%10 attach_class = %5 %8d\n",
		p_dev->addr_index, p_dev->attach_class);

	printf("%4 card_type = %8 %8d\n",p_dev->card_type);
}

/*****************************************************************************/
/*
 * NAME:     fmt_addrs
 *
 * FUNCTION: The format routine for the acs_addrs structure.
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
fmt_addr(p_addr)
    fddi_acs_addrs_t	*p_addr;
{
	int	i;
	int	len;

	printf("\nADDRS - (struct fddi_acs_addrs):\n\n");
	
	print_addr("%4 src_addr = %9 ", p_addr->src_addr);

	printf("%4 hdw_addr_cnt = %5 %8d\n",p_addr->hdw_addr_cnt);

	for (i=0; i<p_addr->hdw_addr_cnt; i++)
		fmt_addr_elem(&p_addr->hdw_addrs[i], i);

	printf("%4 sw_addrs = %9 %08x%10 sw_addr_cnt = %6 %8d\n",
		p_addr->sw_addrs, p_addr->sw_addr_cnt);

	printf("\n");
}

/*****************************************************************************/
/*
 * NAME:     fmt_addr_elem
 *
 * FUNCTION: The format routine for the addr_elem structure.
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
fmt_addr_elem(p_addr,num)
    	fddi_addr_elem_t	*p_addr;
	int			num;
{
	printf("\n%4 Hardware Address - (struct fddi_addr_elem [%d]):\n\n", num);

	print_addr("%8 addr = %9 ", &p_addr->addr[0]);
	printf("%8 addr_cnt = %5 %8d\n",p_addr->addr_cnt);
	printf("\n");

}
/*****************************************************************************/
/*
 * NAME:     fmt_cmd
 *
 * FUNCTION: The format routine for the cmd structure.
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
fmt_cmd(p_cmd)
    fddi_cmd_t	*p_cmd;
{
	int	i;
	int	len;

	printf("\n%4CMD - (struct fddi_cmd [%08x]):\n\n", p_cmd);

	printf("%8 cmd_code = %9 %04x\n",p_cmd->cmd_code);
	printf("%8 ctl = %14 %04x%10 stat = %13 %08x\n",
		p_cmd->ctl, p_cmd->stat); 
	printf("%8 cpb_len = %6 %d\n",p_cmd->cpb_len);

	printf("%8 cpb :");
    	hex_dmp("            ", &p_cmd->cpb[0], FDDI_CPB_SIZE);
	printf("\n");

}
/*****************************************************************************/
/*
 * NAME:     fmt_dds
 *
 * FUNCTION: The format routine for the dds structure.
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
fmt_dds(p_dds)
    fddi_dds_t	*p_dds;
{
	int	i;

	printf("\nDDS - (struct fddi_dds):\n\n");

	printf("%4 bus_type = %5 %08x\n",p_dds->bus_type);
	printf("%4 bus_id = %7 %08x\n",p_dds->bus_id);
	printf("%4 bus_intr_lvl =  %08x\n",p_dds->bus_intr_lvl);
	printf("%4 intr_priority = %08x\n",p_dds->intr_priority);
	printf("%4 slot = %9 %d\n",p_dds->slot);
	printf("%4 bus_io_addr =   %08x\n",p_dds->bus_io_addr);
	printf("%4 bus_mem_addr =  %08x\n",p_dds->bus_mem_addr);
	printf("%4 dma_lvl = %6 %08x\n",p_dds->dma_lvl);
	printf("%4 dma_base_addr = %08x\n",p_dds->dma_base_addr);
	printf("%4 dma_length = %3 %08x\n",p_dds->dma_length);
	printf("%4 lname = %8 [%s]\n",p_dds->lname);
	printf("%4 alias = %8 [%s]\n",p_dds->alias);
	if (p_dds->use_alt_addr)
		printf("%4 use_alt_addr =  TRUE\n");
	else
		printf("%4 use_alt_addr =  FALSE\n");
	printf("%4 alt_addr = %5 [%02x%02x%02x%02x%02x%02x]\n",
		p_dds->alt_addr[0], p_dds->alt_addr[1], p_dds->alt_addr[2], 
		p_dds->alt_addr[3], p_dds->alt_addr[4], p_dds->alt_addr[5]);

	printf("%4 tvx = %10 %08x\n",p_dds->tvx);
	printf("%4 t_req = %8 %08x\n",p_dds->t_req);

	printf("%4 pmf_passwd = %3 [");
	for (i=0; i<FDDI_PASSWD_SZ; i++)
		printf("%02x",p_dds->pmf_passwd[i]);
	printf("]\n");
	
	printf("%4 user_data : ");
    	hex_dmp("          ", &p_dds->user_data[0], FDDI_USR_DATA_LEN);

	printf("%4 tx_que_sz = %4 %d\n",p_dds->tx_que_sz);
}

/*****************************************************************************/
/*
 * NAME:     fmt_ls
 *
 * FUNCTION: The format routine for the spec_stats structure.
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
fmt_ls(p_ls)
    fddi_spec_stats_t	*p_ls;
{
	int	i;
	uint	tmp1, tmp2;

	tmp1 = (p_ls->smt_error_hi << 16) + p_ls->smt_error_lo;
	tmp1 = (p_ls->smt_event_hi << 16) + p_ls->smt_event_lo;
	printf("%8 smt_error = %4 %08x%10 smt_event = %8 %08x\n",tmp1, tmp2);
	printf("%8 cpv = %14 %04x%10 port_event = %7 %08x\n",
		p_ls->cpv,p_ls->port_event);

	tmp1 = (p_ls->setcount_hi << 16) + p_ls->setcount_lo;
	printf("%8 setcount = %5 %08x%10 aci_code = %13 %04x\n",
		tmp1,p_ls->aci_code);

	printf("%8 pframe_cnt = %7 %04x\n",p_ls->pframe_cnt);

	printf("\n%8 ecm_sm = %11 %04x\n",p_ls->ecm_sm);
	printf("%8 pcm_a_sm = %9 %04x%10 pcm_b_sm = %13 %04x\n",
		p_ls->pcm_a_sm, p_ls->pcm_b_sm);

	printf("%8 cfm_a_sm = %9 %04x%10 cfm_b_sm = %13 %04x\n",
		p_ls->cfm_a_sm, p_ls->cfm_b_sm);

	printf("%8 cf_sm = %12 %04x%10 mac_cfm_sm = %11 %04x\n",
		p_ls->cf_sm, p_ls->mac_cfm_sm);

	printf("%8 rmt_sm = %11 %4x\n", p_ls->rmt_sm);

	printf("%8 mcast_tx_ok = %2 %8d%10 bcast_tx_ok = %6 %8d\n",
		p_ls->mcast_tx_ok, p_ls->bcast_tx_ok);

	printf("%8 mcast_rx_ok = %2 %8d%10 bcast_rx_ok = %6 %8d\n",
		p_ls->mcast_rx_ok, p_ls->bcast_rx_ok);

	printf("%8 ndd_flags = %4 %08x\n", p_ls->ndd_flags);

}

/*****************************************************************************/
/*
 * NAME:     fmt_vpd
 *
 * FUNCTION: The format routine for the vpd structure.
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
fmt_vpd(p_vpd)
    fddi_vpd_t	*p_vpd;
{
	int	i;

	printf("\nVPD - (struct fddi_vpd):\n");
	
	printf("%4 vpd status = %d\n",p_vpd->status);
	printf("%4 vpd length = %d\n",p_vpd->l_vpd);

	if (p_vpd->l_vpd < FDDI_VPD_LENGTH) 
		i = p_vpd->l_vpd;
	else
		i = FDDI_VPD_LENGTH;
	printf("%4 vpd :");
    	hex_dmp("        ", &p_vpd->vpd[0], i);

	printf("\n%4 extender vpd status = %d\n",p_vpd->xc_status);
	printf("%4 extender vpd length = %d\n",p_vpd->l_xcvpd);

	if (p_vpd->l_xcvpd < FDDI_XCVPD_LENGTH) 
		i = p_vpd->l_xcvpd; 
	else 
		i = FDDI_XCVPD_LENGTH;

	printf("%4 extender vpd :");
    	hex_dmp("        ", &p_vpd->xcvpd[0], i);
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
    printf("\nNDD - (struct ndd):\n\n");
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
    if (p_ndd->ndd_flags & CFDDI_NDD_LLC_DOWN)
	printf(" LLC_DOWN");
    if (p_ndd->ndd_flags & CFDDI_NDD_BEACON)
	printf(" BEACON");
    if (p_ndd->ndd_flags & CFDDI_NDD_SMT)
	printf(" SMT");
    if (p_ndd->ndd_flags & CFDDI_NDD_NSA)
	printf(" NSA");
    if (p_ndd->ndd_flags & CFDDI_NDD_BF)
	printf(" BF");
    if (p_ndd->ndd_flags & CFDDI_NDD_DAC)
	printf(" DAC");

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

print_addr(p_str, p_addr)
	char *p_str, *p_addr;
{
	int i;

	printf(p_str);

	for (i=0; i<CFDDI_NADR_LENGTH; i++)
		printf("%02x",p_addr[i]);

	printf("\n");
	printf("\n");
};
