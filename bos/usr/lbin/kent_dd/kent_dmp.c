static char sccsid[] = "@(#)37	1.2  src/bos/usr/lbin/kent_dd/kent_dmp.c, pcient, bos41J, 9511A_all 3/3/95 16:28:28";
/*
 *   COMPONENT_NAME: pcient
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
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _KERNEL
#include <sys/errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/dump.h>
#include "kent_proto.h"

/*****************************************************************************/
/*
 * NAME:     dmp_entry
 *
 * FUNCTION: kent dump formatter.
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
    
    	/* 
     	*  dump entry name is "dd_ctl"
     	*/
    	if (!strcmp(name, "kent_tbl")) 
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
    	 *  dump entry name is "kent_acs"
     	 */
    	if (!strcmp(name, "kent_acs")) 
	{
		if (len < sizeof(kent_acs_t))
		{
			printf("Buffer too small, len = %d, kent_acs_t = %d\n",
				len, sizeof(kent_acs_t));
			free(buf);
			return(E2BIG);
		}
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}

		fmt_acs(buf, d_ptr);
		return(0);
    	}
    
    	/* 
    	 *  dump entry name is "k_tx_dsc"
     	 */
    	if (!strcmp(name, "k_tx_dsc")) 
	{
		if (len < sizeof(kent_desc_t))
		{
			printf("Buffer too small, len = %d, kent_desc_t = %d\n",
				len, sizeof(kent_desc_t));
			free(buf);
			return(E2BIG);
		}
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}

		fmt_tx_dsc(buf, d_ptr, len);
		return(0);
    	}
    
    	/* 
    	 *  dump entry name is "k_tx_buf"
     	 */
    	if (!strcmp(name, "k_tx_buf")) 
	{
		if (len < KENT_BUFF_SZ)
		{
			printf("Buffer too small, len = %d, KENT_BUFF_SZ = %d\n",
				len, KENT_BUFF_SZ);
			free(buf);
			return(E2BIG);
		}
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}

		fmt_tx_buf(buf, d_ptr, len);
		return(0);
    	}
    
    	/* 
    	 *  dump entry name is "k_rx_dsc"
     	 */
    	if (!strcmp(name, "k_rx_dsc")) 
	{
		if (len < sizeof(kent_desc_t))
		{
			printf("Buffer too small, len = %d, kent_desc_t = %d\n",
				len, sizeof(kent_desc_t));
			free(buf);
			return(E2BIG);
		}
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}

		fmt_rx_dsc(buf, d_ptr,len);
		return(0);
    	}
    
    	/* 
    	 *  dump entry name is "k_rx_buf"
     	 */
    	if (!strcmp(name, "k_rx_buf")) 
	{
		if (len < KENT_BUFF_SZ)
		{
			printf("Buffer too small, len = %d, KENT_BUFF_SZ = %d\n",
				len, KENT_BUFF_SZ);
			free(buf);
			return(E2BIG);
		}
		if (read(kmem, buf, len) != len) 
		{
			free(buf);
	    		return(-1);
		}

		fmt_rx_buf(buf, d_ptr, len);
		return(0);
    	}
    
    
	free(buf);
	printf("Undefined Name [%s]\n",name);
	return(0);
} /* End of dmp_entry() */

/*
 * NAME:     fmt_tbl
 * FUNCTION: dumps kent_tbl structure
 * INPUTS:   kent_tbl structure
 * RETURNS:  none
 */
fmt_tbl(p_kent_tbl, d_ptr, len)
    	kent_tbl_t	*p_kent_tbl;
    	char		*d_ptr;
	int 		len;
{
	int 	i;
	char 	*tmpc;

    	printf("\n<< Dump of kent_tbl (%08x) >> :\n\n", d_ptr);

    	printf("%4 table_lock: %08x\n", p_kent_tbl->table_lock);
	printf("%4 acs_cnt: %d%10 open_cnt: %d\n\n",p_kent_tbl->acs_cnt,
		p_kent_tbl->open_cnt);

	tmpc = (char *) p_kent_tbl;
	printf("\n%4 trace_lock: %08x\n", *(int *)&tmpc[len-4]);
    	printf("%4 trace - (struct kent_trace):");

	/* 
	 * This is dependent on the structure of the kent_tbl.  Under the 
	 * current design, the table_lock, acs_cnt, open_cnt, acs pointers, and
	 * some misc. stuff is before the trace table.  Since the trace table
	 * changes size based on the compile options, taking the len and 
	 * removing excess should allow the dump to get the whole trace no
	 * matter what is compiled
	 */

	len = len - offsetof(kent_tbl_t, trace) - offsetof(kent_trace_t, table);
    	hex_dmp("         ", &p_kent_tbl->trace.table[0], len);
} /* End of fmt_tbl() */

/* 
 * NAME: 	fmt_acs
 * FUNCTION:	dumps kent_acs_t structure
 * INPUTS:	kent_acs_t structure
 * RETURNS:	none
 */
fmt_acs(p_acs, d_ptr)
	kent_acs_t	*p_acs;
	char		*d_ptr;
{

	printf("\n<< Dump of KENT ACS Structure (%08x) >> :\n", d_ptr);

	printf("%4 next_acs = %9 %08x\n", p_acs->next_acs);
	fmt_ndd(&p_acs->ndd);
	fmt_dds(&p_acs->dds);
	fmt_tx(&p_acs->tx);
	fmt_rx(&p_acs->rx);
	fmt_dev(&p_acs->dev);
	fmt_addrs(&p_acs->addrs);
	fmt_init(&p_acs->init);
	fmt_ls(&p_acs->ls);
    	printf("%4 MIBS - (struct ethernet_all_mib_t):");
    	hex_dmp("        ", &p_acs->mibs, sizeof(ethernet_all_mib_t));
}

/* 
 * NAME: 	fmt_ndd
 * FUNCTION:	dumps ndd_t structure
 * INPUTS:	ndd_t structure
 * RETURNS:	none
 */
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
    if (p_ndd->ndd_flags & ENT_RCV_BAD_FRAME)
	printf(" ENT_RCV_BAD_FRAME");

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

/* 
 * NAME: 	fmt_ndd_genstats
 * FUNCTION:	dumps ndd_genstats structure
 * INPUTS:	ndd_genstats structure
 * RETURNS:	none
 */
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

/* 
 * NAME: 	fmt_dds
 * FUNCTION:	dumps kent_dds_t structure
 * INPUTS:	kent_dds_t structure
 * RETURNS:	none
 */
fmt_dds(p_dds)
    kent_dds_t	*p_dds;
{
	int	i;

	printf("\nDDS - (struct kent_dds):\n\n");

	printf("%4 busid = %8 %08x\n",p_dds->busid);
	printf("%4 busintr = %6 %08x\n",p_dds->busintr);
	printf("%4 busio = %8 %08x\n",p_dds->busio);
	printf("%4 md_sla = %7 %d\n",p_dds->md_sla);
	printf("%4 lname = %8 [%s]\n",p_dds->lname);
	printf("%4 alias = %8 [%s]\n",p_dds->alias);
	printf("%4 tx_que_sz = %4 %d\n",p_dds->tx_que_sz);
	printf("%4 rx_que_sz = %4 %d\n",p_dds->rx_que_sz);
	if (p_dds->use_alt_addr)
		printf("%4 use_alt_addr =  TRUE\n");
	else
		printf("%4 use_alt_addr =  FALSE\n");
	printf("%4 alt_addr = %5 [%02x%02x%02x%02x%02x%02x]\n",
		p_dds->alt_addr[0], p_dds->alt_addr[1], p_dds->alt_addr[2], 
		p_dds->alt_addr[3], p_dds->alt_addr[4], p_dds->alt_addr[5]);

}

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
    kent_tx_t	*p_tx;
{
	int	i;

	printf("\nTX - (struct kent_tx):\n\n");

	printf("%4 lock = %13 %08x\n", p_tx->lock);

	printf("%4 in_use = %11 %8d\n",p_tx->in_use);

	printf("%4 nxt_tx = %11 %8d%10 nxt_cmplt = %8 %8d\n",
		p_tx->nxt_tx, p_tx->nxt_cmplt);

	printf("%4 desc = %13 %08x%10 p_buffer = %9 %08x\n",
		p_tx->desc, p_tx->p_buffer);

	printf("%4 p_d_desc = %9 %08x\n", p_tx->p_d_desc);

	printf("%4 wdt (struct watchdog):  ");
    	hex_dmp("        ", &p_tx->wdt, sizeof(struct watchdog));
	printf("\n");
}

/* 
 * NAME: 	fmt_rx
 * FUNCTION:	dumps kent_rx_t structure
 * INPUTS:	kent_rx_t structure
 * RETURNS:	none
 */
fmt_rx(p_rx)
    kent_rx_t	*p_rx;
{
	int	i;

	printf("\nRX - (struct kent_rx):\n\n");

	printf("%4 desc = %13 %08x%10 p_buffer = %9 %08x\n",
		p_rx->desc, p_rx->p_buffer);
	printf("%4 p_d_desc = %9 %08x%10 nxt_rx = %11 %8d\n",
		p_rx->p_d_desc,p_rx->nxt_rx);

}

/* 
 * NAME: 	fmt_dev
 * FUNCTION:	dumps kent_dev_t structure
 * INPUTS:	kent_dev_t structure
 * RETURNS:	none
 */
fmt_dev(p_dev)
    kent_dev_t	*p_dev;
{
	int	i;

	printf("\nDEV - (struct kent_dev):\n\n");

	printf("%4 State = %11 (%08x) ", p_dev->state);
	switch (p_dev->state)
	{
		case CLOSED_STATE: 
			printf("Closed State \n");
			break;
		case OPEN_STATE: 
			printf("Open State \n");
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

	printf("%4 chip revision = %4 %02x\n",p_dev->chip_rev);

	printf("%4 seq_number = %7 %08x%10 ctl_lock = %9 %08x\n",
		p_dev->seq_number, p_dev->ctl_lock);

	printf("%4 slih_lock = %8 %08x%10 dd_clock = %9 %08x\n",
		p_dev->slih_lock, p_dev->dd_clock);

	printf("%4 multi_cnt = %8 %08x%10 prom_cnt = %9 %08x\n",
		p_dev->multi_cnt, p_dev->prom_cnt);

	printf("%4 bf_cnt = %11 %08x%10 pio_rc = %11 %8d\n",
		p_dev->bf_cnt, p_dev->pio_rc);

	printf("%4 iox = %14 %08x%10 p_d_init = %9 %08x\n",
		p_dev->iox, p_dev->p_d_init);

	printf("%4 iomap: ");
	hex_dmp("         ", &p_dev->iomap, sizeof(struct io_map));
	printf("\n");

	printf("%4 dh = %15 %08x\n", p_dev->dh);

}

/* 
 * NAME: 	fmt_addrs
 * FUNCTION:	dumps kent_addrs_t structure
 * INPUTS:	kent_addrs_t structure
 * RETURNS:	none
 */
fmt_addrs(p_addr)
    kent_addrs_t	*p_addr;
{
	int	i;
	kent_addr_blk_t *p_tmp;

	printf("\nADDRS - (struct kent_addrs):\n\n");
	
	print_addr("%4 src_addr = %9 ", p_addr->src_addr);

	printf("%4 addr_cnt = %9 %8d\n",p_addr->addr_cnt);

	printf("%4 ladrf = %12 %[%02x%02x%02x%02x %02x%02x%02x%02x]\n",
		p_addr->ladrf[0],p_addr->ladrf[1],p_addr->ladrf[2],
		p_addr->ladrf[3],p_addr->ladrf[4],p_addr->ladrf[5],
		p_addr->ladrf[6],p_addr->ladrf[7]);

	printf("\n");
}

/* 
 * NAME: 	fmt_addr_blk
 * FUNCTION:	dumps kent_addr_blk_t structure
 * INPUTS:	kent_addr_blk_t structure
 * RETURNS:	none
 */
fmt_addr_blk(p_blk,num)
    	kent_addr_blk_t		*p_blk;
	int			num;
{
	int i;
	printf("\n%4 Multicast Address Block - (struct kent_addr_blk):\n\n");
	
	printf("%4 next = %13 %08x%10 prev = %13 %08x\n",
		p_blk->next, p_blk->prev);

	printf("%4 blk_cnt = %10 %8d\n",p_blk->blk_cnt);

	for (i=0; i<p_blk->blk_cnt; i++)
		fmt_addr_elem(&p_blk->addrs[i], (i+num));
	

}

/* 
 * NAME: 	fmt_addr_elem
 * FUNCTION:	dumps kent_addr_elem_t structure
 * INPUTS:	kent_addr_elem_t structure
 * RETURNS:	none
 */
fmt_addr_elem(p_addr,num)
    	kent_addr_elem_t	*p_addr;
	int			num;
{
	printf("\n%4 Multicast Address - (struct kent_addr_elem [%d]):\n\n",num);

	print_addr("%8 addr = %9 ", &p_addr->addr[0]);
	printf("%8 addr_cnt = %5 %8d\n",p_addr->addr_cnt);
	printf("\n");

}

/* 
 * NAME: 	print_addr
 * FUNCTION:	dumps a multicast address
 * INPUTS:	a multicast address
 * RETURNS:	none
 */
print_addr(p_str, p_addr)
	char *p_str, *p_addr;
{
	int i;

	printf(p_str);

	for (i=0; i<ENT_NADR_LENGTH; i++)
		printf("%02x",p_addr[i]);

	printf("\n");
	printf("\n");
}


/* 
 * NAME: 	fmt_init
 * FUNCTION:	dumps kent_init_blk_t structure
 * INPUTS:	kent_init_blk_t structure
 * RETURNS:	none
 */

fmt_init(p_init)
    	kent_init_blk_t		*p_init;
{
	printf("\n%4 Init Block - (struct kent_init_blk):\n\n");

	printf("%4 mode = %13 %08x\n", p_init->mode);
	printf("%4 padr1 = %12 %08x%10 padr2 = %12 %08x\n",
		p_init->padr1, p_init->padr2);

	printf("%4 ladrf = %12 %[%02x%02x%02x%02x %02x%02x%02x%02x]\n",
		p_init->ladrf[0],p_init->ladrf[1],p_init->ladrf[2],
		p_init->ladrf[3],p_init->ladrf[4],p_init->ladrf[5],
		p_init->ladrf[6],p_init->ladrf[7]);

	printf("%4 rdra = %13 %08x%10 tdra = %13 %08x\n",
		p_init->rdra, p_init->tdra);
}


/* 
 * NAME: 	fmt_ls
 * FUNCTION:	dumps kent_all_stats_t structure
 * INPUTS:	kent_all_stats_t structure
 * RETURNS:	none
 */
fmt_ls(p_all_stat)
	kent_all_stats_t	*p_all_stat;
{
	ent_genstats_t 	*p_stat = &p_all_stat->ent_gen_stats;
	kent_stats_t	*p_kent	= &p_all_stat->kent_stats;
	int		i;

	printf("\n%4 Statistics - (struct kent_all_stats):\n\n");

	printf("%4 device_type = %5 (%08x) ", p_stat->device_type);
	switch (p_stat->device_type)
	{
		case ENT_KEN_PCI: 
			printf("IBM PCI Ethernet (22100020) Device \n");
			break;
		default: 
			printf("Unknown Device \n");
			break;
	}; 
	printf("\n");

    	printf("\n%4 ndd_flags: %6 (");
    	if (p_stat->ndd_flags & NDD_UP) 
		printf(" UP");
    	if (p_stat->ndd_flags & NDD_BROADCAST) 
		printf(" BROADCAST");
    	if (p_stat->ndd_flags & NDD_DEBUG) 
		printf(" DEBUG");
    	if (p_stat->ndd_flags & NDD_RUNNING) 
		printf(" RUNNING");
    	if (p_stat->ndd_flags & NDD_SIMPLEX) 
		printf(" SIMPLEX");
    	if (p_stat->ndd_flags & NDD_DEAD) 
		printf(" DEAD");
    	if (p_stat->ndd_flags & NDD_LIMBO) 
		printf(" LIMBO");
	
    	printf("\n%22 ");

    	if (p_stat->ndd_flags & NDD_PROMISC) 
		printf(" PROMISC");
    	if (p_stat->ndd_flags & NDD_ALTADDRS) 
		printf(" ALTADDRS");
    	if (p_stat->ndd_flags & NDD_MULTICAST) 
		printf(" MULTICAST");
    	if (p_stat->ndd_flags & ENT_RCV_BAD_FRAME)
		printf(" ENT_RCV_BAD_FRAME");
	
    	printf(")\n\n");

	print_addr("%4 ent_nadr = %9 ", p_stat->ent_nadr);

        printf("%4 mcast_rx_ok = %6 %d\n",p_stat->mcast_rx_ok);
        printf("%4 bcast_rx_ok = %6 %d\n",p_stat->bcast_rx_ok);
        printf("%4 fcs_errs = %9 %d\n",p_stat->fcs_errs);
        printf("%4 align_errs = %7 %d\n",p_stat->align_errs);
        printf("%4 overrun = %10 %d\n",p_stat->overrun);
        printf("%4 short_frames = %5 %d\n",p_stat->short_frames);
        printf("%4 long_frames = %6 %d\n",p_stat->long_frames);
        printf("%4 no_resources = %5 %d\n",p_stat->no_resources);
        printf("%4 rx_collisions = %4 %d\n",p_stat->rx_collisions);
        printf("%4 rx_drop = %10 %d\n",p_stat->rx_drop);
        printf("%4 start_rx = %9 %d\n",p_stat->start_rx);
        printf("%4 mcast_tx_ok = %6 %d\n",p_stat->mcast_tx_ok);
        printf("%4 bcast_tx_ok = %6 %d\n",p_stat->bcast_tx_ok);
        printf("%4 carrier_sense = %4 %d\n",p_stat->carrier_sense);
        printf("%4 underrun = %9 %d\n",p_stat->underrun);
        printf("%4 cts_lost = %9 %d\n",p_stat->cts_lost);
        printf("%4 excess_collisions = %d\n",p_stat->excess_collisions);
        printf("%4 late_collisions = %2 %d\n",p_stat->excess_collisions);
        printf("%4 tx_timeouts = %6 %d\n",p_stat->tx_timeouts);
        printf("%4 sqetest = %10 %d\n",p_stat->sqetest);
        printf("%4 defer_tx = %9 %d\n",p_stat->sqetest);
        printf("%4 s_coll_frames = %4 %d\n",p_stat->s_coll_frames);
        printf("%4 m_coll_frames = %4 %d\n",p_stat->m_coll_frames);
        printf("%4 sw_txq_len = %7 %d\n",p_stat->sw_txq_len);
        printf("%4 hw_txq_len = %7 %d\n",p_stat->hw_txq_len);
        printf("%4 restart_count = %4 %d\n",p_stat->restart_count);
        printf("%4 reserved1 = %8 %d\n",p_stat->reserved1);
        printf("%4 reserved2 = %8 %d\n",p_stat->reserved2);
        printf("%4 reserved3 = %8 %d\n",p_stat->reserved3);
        printf("%4 reserved4 = %8 %d\n",p_stat->reserved4);

        for (i=0; i<16; i++)
        {
                printf("%4 coll_freq[%02d] = %4 %d\n",i,p_kent->coll_freq[i]);
        }

}

/* 
 * NAME: 	fmt_tx_dsc
 * FUNCTION:	dumps the transmit descriptors
 * INPUTS:	array of transmit descriptors
 * RETURNS:	none
 */
fmt_tx_dsc(p_tx, d_ptr, len)
    	kent_desc_t	*p_tx;
	char 		*d_ptr;
 	int		len;
{
	int	i;

	printf("\n<< Dump of KENT TX Descriptors Structure (%08x) >> :\n", d_ptr);
	for (i=0; i<(len/sizeof(kent_desc_t)); i++)
	{
		printf("%4 Descriptor #%d\n",i);
		printf("%8 addr = %13 %08x\n",p_tx[i].addr);
		printf("%8 stat1 = %12 %08x\n",p_tx[i].stat1);
		printf("%8 stat2 = %12 %08x\n",p_tx[i].stat2);
		printf("%8 stat3 = %12 %08x\n\n",p_tx[i].stat3);
	}
	
}

/* 
 * NAME: 	fmt_rx_dsc
 * FUNCTION:	dumps the receive descriptors
 * INPUTS:	array of receive descriptors
 * RETURNS:	none
 */
fmt_rx_dsc(p_rx, d_ptr, len)
    	kent_desc_t	*p_rx;
	char 		*d_ptr;
 	int		len;
{
	int	i;

	printf("\n<< Dump of KENT RX Descriptors Structure (%08x) >> :\n", d_ptr);
	for (i=0; i<(len/sizeof(kent_desc_t)); i++)
	{
		printf("%4 Descriptor #%d\n",i);
		printf("%8 addr = %13 %08x\n",p_rx[i].addr);
		printf("%8 stat1 = %12 %08x\n",p_rx[i].stat1);
		printf("%8 stat2 = %12 %08x\n",p_rx[i].stat2);
		printf("%8 stat3 = %12 %08x\n\n",p_rx[i].stat3);
	}
	
}

/* 
 * NAME: 	fmt_tx_buf
 * FUNCTION:	dumps the transmit buffers
 * INPUTS:	array of transmit buffers
 * RETURNS:	none
 */
fmt_tx_buf(p_tx, d_ptr, len)
    	char		*p_tx;
	char 		*d_ptr;
 	int		len;
{
	int	i;

	printf("\n<< Dump of KENT TX Buffers (%08x) >> :\n", d_ptr);
	for (i=0; i<(len/KENT_BUFF_SZ); i++)
	{
		printf("Buffer %d - ",i);
		hex_dmp("         ", p_tx, KENT_BUFF_SZ);
		printf("\n");
		p_tx += KENT_BUFF_SZ;
	}
	
}

/* 
 * NAME: 	fmt_rx_buf
 * FUNCTION:	dumps the receive buffers
 * INPUTS:	array of receive buffers
 * RETURNS:	none
 */
fmt_rx_buf(p_rx, d_ptr, len)
    	char		*p_rx;
	char 		*d_ptr;
 	int		len;
{
	int	i;

	printf("\n<< Dump of KENT RX Buffers (%08x) >> :\n", d_ptr);
	for (i=0; i<(len/KENT_BUFF_SZ); i++)
	{
		printf("Buffer %d - ",i);
		hex_dmp("         ", p_rx, KENT_BUFF_SZ);
		printf("\n");
		p_rx += KENT_BUFF_SZ;
	}
	
}
