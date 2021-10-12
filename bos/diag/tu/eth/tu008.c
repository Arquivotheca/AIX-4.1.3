static char sccsid[] = "@(#)86  1.4.1.3  src/bos/diag/tu/eth/tu008.c, tu_eth, bos411, 9428A410j 6/11/92 10:17:47";
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: lswap, swap, init_rx_buf, setup_pos, wrap, tu008
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
Function(s) Test Unit 008 - External Wrap Test (BNC Connector)
*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include <sys/sysdma.h>
#include <errno.h>
#include "ethtst.h"

#define DFLT_NUM_PACKETS        4	/* default no. of packets to wrap */
#define DFLT_PACKET_SIZE       68	/* default packet size */
#define MAX_PACKET_SIZE      1514	/* maximum packet size */
#define DFLT_ABM             0x01	/* default ABM
					(address burst management) */


#ifdef debugg 
extern void detrace();
#endif

#if 0

/*****************************************************************************

brief_delay

Function to perform "nothing" loop for delaying hopefully around 1/2 a second.

*****************************************************************************/

brief_delay (n)
   long n;
   {
	long i;
	static long ch;

	for (i = 0; i < 500000; i++)
		ch = i;

	return(i);
   }
#endif	/* end if-zero */

/*****************************************************************************

lswap

This routine swaps the Hi and Low words in a 32 bit value.

*****************************************************************************/

long lswap (num)
   long num;
   {
	num = ((num << 16) & 0xFFFF0000) | ((num >> 16) & 0x0000FFFF);
	return(num);
   }

/*****************************************************************************

swap

This routine swaps the Hi and Low bytes

*****************************************************************************/

short swap (num)
   short num;
   {
	num = ((num << 8) & 0xFF00) | ((num >> 8) & 0x00FF);
	return(num);
   }

/*****************************************************************************

init_rx_buf

*****************************************************************************/

void init_rx_buf (rbuf, pattern_byte, p_size, tucb_ptr)
   unsigned char rbuf[];
   unsigned char pattern_byte;
   unsigned long p_size;
   TUTYPE *tucb_ptr;
   {
	register int i;

	/*
	 * First, fill in source and destination
	 * network addresses.
	 */
	for (i = 0; i < 6; i++)
	   {
		rbuf[i]     = tucb_ptr->eth_htx_s.net_addr[i];
		rbuf[i + 6] = tucb_ptr->eth_htx_s.net_addr[i];
	   }
	
	/*
	 * Next, put in the "type" (i.e., the destination "netid")
	 * that was initialized at START time.
	 */
	rbuf[12] = (unsigned char) ((tucb_ptr->eth_htx_s.netid >> 8) & 0x00ff);
	rbuf[13] = (unsigned char) (tucb_ptr->eth_htx_s.netid & 0x00ff);

	/*
	 * Initialize the Receive Buffer to a set pattern
	 */
	memset(&rbuf[14], pattern_byte, p_size - 14);
#ifdef debugg
	detrace(0,"init_rx_buf:  set %d (dec) bytes to 0x%02x\n\n",
			p_size - 14, pattern_byte);
#endif
   }

/*****************************************************************************

setup_pos

*****************************************************************************/

int setup_pos (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned char p0;
	unsigned char cval;
	unsigned long status;

	extern unsigned char pos_rd();
	extern int pos_wr();

       /*
	* Switch the window on, set the ABM boundary, select connector
	*/
	p0 = pos_rd(fdes, 4, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Read failed on POS4\n");
#endif
		return(POS4_RD_ERR);
	   }

	/*
	 * bit 0  = Open RAM window, bit 1  = Select Connector
	 * Revision 3 Additions
	 * bit 7  = Select Feedback, bit 4  = Card Parity
	 */
	if (tucb_ptr->eth_htx_s.wrap_type == EXTERNAL_BNC)
		p0 |= 0x03;	/* BNC and Open RAM Window	*/
	else
	   {
		p0 |= 0x01;	/* DIX only	*/
		p0 &= 0xfd;
	   }


	/*
	 * Revision 3 additions.  Always bit 7 (Select Feedback) and only
	 * if requested, bit 4 (Card Parity)  
	 */
	if (tucb_ptr->eth_htx_s.adapter_type == PS3)
	   {
		p0 |= 0x80;	/* Select Feedback	*/
		if (tucb_ptr->eth_htx_s.parity == ENABLE)
			p0 |= 0x10;	/* Card Parity	*/
	   }	


	if (pos_wr(fdes, 4, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Failed to write a %x to POS4\n", p0);
#endif
		return(POS4_WR_ERR);
	   }

	/*
	 * set up Address Burst Management (ABM).
	 * This programs the adapter (when acting as bus master)
	 * to give up the bus upon crossing a particular
	 * address boundary.
	 */
	p0 = pos_rd(fdes, 5, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Read failed on POS5\n");
#endif
		return(POS5_RD_ERR);
	   }

	/*
	 * First, let's zero out the ABM bits.
	 */
	p0 &= 0xfc;

	/*
	 * If card type is AT, then MUST set ABM to 64K.
	 * so set corresponding bits to dec. value 3.
	 */
	if (tucb_ptr->eth_htx_s.adapter_type == AT)
		p0 |= 0x03;
	else
	   {
		/*
		 * if running with HTX, pull ABM
		 * value as specified from rule file
		 */
		if (tucb_ptr->eth_htx_s.htx_sp != NULL)
		   {
			cval = (unsigned char) tucb_ptr->eth_htx_s.abm & 0x03;
			p0 |= cval;
		   }
		else
			/*
			 * Not running HTX, so use default value.
			 */
			p0 |= (0x03 & DFLT_ABM);
	   }

	/*
	 * Insure that channel check is NOT asserted
	 */
	p0 |= 0x80;

	if (pos_wr(fdes, 5, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Failed to write a %x to POS5\n", p0);
#endif
		return(POS5_WR_ERR);
	   }
	
	/*
	 * setup POS3 for enabling or disabling fairness.
	 */
	p0 = pos_rd(fdes, 3, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Read failed on POS3\n");
#endif
		return(POS3_RD_ERR);
	   }
	if (tucb_ptr->eth_htx_s.fairness == ENABLE)
		p0 |= 0x10;
	else
		p0 &= 0xef;
	if (pos_wr(fdes, 3, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Failed to write a %x to POS3\n", p0);
#endif
		return(POS3_WR_ERR);
	   }
	
	/*
	 * setup POS2 for enabling or disabling parity.
	 */
	p0 = pos_rd(fdes, 2, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Read failed on POS2\n");
#endif
		return(POS2_RD_ERR);
	   }
	if (tucb_ptr->eth_htx_s.parity == ENABLE)
		p0 |= 0x80;
	else
		p0 &= 0x7f;
	if (pos_wr(fdes, 2, p0, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"Wrap Failed -- Failed to write a %x to POS2\n", p0);
#endif
		return(POS2_WR_ERR);
	   }

	
	return(0);
   }

/*****************************************************************************

wrap

This is the Internal/External wrap Test.
The microchannel sets up a transmit and receive buffer in host memory.
The 82586 is put into loopback mode, a transmit command is issued and
The test data is checked in the receive buffer. This is repeated for
different test patterns.

*****************************************************************************/

int wrap (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int i, j, k, rc;

	static unsigned char *rxbuffer;
	static unsigned char test_patt[] = { 0x55, 0xAA, 0xFF, 0x33 };
	static int num_test_bytes = sizeof(test_patt) / sizeof(unsigned char);
	unsigned char test_patt_byte;

	struct mailbox rx_mb_s, tx_mb_s;
	struct buffer_descriptor rx_bd_s, tx_bd_s, former_bd_s;

	unsigned char cval;
	unsigned short sval;
	unsigned long status;
	unsigned long packet_size;
	unsigned long user_buf_size;
	unsigned long num_packets;

	unsigned long dma_base_add;
	struct htx_data *htx_sp;

	extern int hard_reset();
	extern int get_netadd();
	extern int hio_ctrl_wr();
	extern unsigned char hio_parity_rd();
	extern unsigned char hio_status_rd_q();
	extern int hio_parity_wr();
	extern int set_netadd();
	extern int set_indication_enable();
	extern int set_filter();
	extern int disable_match_filter();
	extern int set_loopback();
	extern int access_dma();
	extern int setup_rx();
	extern int setup_tx();
	extern int init_tx_bd();
	extern int rx_complete_status();
	extern int tx_complete_status();
	extern int smem_wr();
	extern int hio_cmd_wr();
	extern int mktu_rc();
	extern char *malloc();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->eth_htx_s.htx_sp;

	/*
	 * determine whether we are running with HTX.  IF so,
	 * then we'll use the packet_size specified from the
	 * tucb_ptr which may have been set in the rule file.
	 * ELSE, we'll use the default size specified in a macro.
	 * Also, we'll grab the number of packets we'll wrap.
         * If diagnostics passes packet size in r1, use it
	 */
	if (htx_sp != NULL)
	   {
		packet_size = (unsigned long) tucb_ptr->eth_htx_s.packet_size;
		num_packets = (unsigned long) tucb_ptr->eth_htx_s.num_packets;
	   }
	else
	   {
                if (tucb_ptr->header.r1 == 0)
                 {
                  packet_size = (unsigned long) DFLT_PACKET_SIZE;
                  num_packets = (unsigned long) DFLT_NUM_PACKETS;
                 }
                else
                 {
                  packet_size = (unsigned long) tucb_ptr->header.r1;
                  num_packets = (unsigned long) DFLT_NUM_PACKETS;
                 }
	   }

	/*
	 * First, calculate needed user buffer size.  Since the
	 * DMA scheme works by locking the user memory which
	 * gets malloc'd in "access_dma" we must be VERY careful here.
	 * A "lock" of user memory can only be performed on a PAGE
	 * of memory.  To insure that we do NOT have variables in
	 * the same PAGE as the malloc'd space BEFORE or AFTER, we'll
	 * also grab enough space by allocating two PAGES.  That
	 * way we can guarantee ourselves through calculation that
	 * we can secure a PAGE within this area on a PAGE boundary.
	 * 
	 * Please see "a_dma.c" for TONS of documentation/notes on
	 * how this works.
	 */
	user_buf_size = DMA_PSIZE * 2;

	if ((DMA_PSIZE / 2) < MAX_PACKET_SIZE)
	   {
#ifdef debugg
		detrace(0,"wrap:  DMA_PSIZE is %d, 0x%x bytes\n",
				DMA_PSIZE, DMA_PSIZE);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, BAD_DMA_S_ERR));
	   }
	
	if (packet_size > MAX_PACKET_SIZE)
	   {
#ifdef debugg
		detrace(0,"wrap:  Need to specify smaller packet size\n");
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			BAD_PCK_S_ERR));
	   }
	
	/*
	 * Now, allocate local user space for the transmit and receive packet.
	 */
	rxbuffer = (unsigned char *) malloc(packet_size);
	if (rxbuffer == NULL)
	   {
#ifdef debugg
		detrace(0,"wrap:  malloc for rxbuffer %d bytes failed\n",
				packet_size);
#endif
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			TRBUF_MAL_ERR));
	   }

       /*
	* Do a hard_reset
	*/
	if (rc = hard_reset(fdes, tucb_ptr))
	   {
		free(rxbuffer);
		/*
		 * mask out the top two bytes which
		 * may have error code info since it
		 * is also called as a separate test unit.
		 */
		rc &= 0x0000ffff;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
	/*
	 * Check firmware version.  If adapter is AT, then doesn't support
	 * parity so drop out if attempt is made to run this with parity.
	 */
	if ((tucb_ptr->eth_htx_s.adapter_type == AT) &&
	    (tucb_ptr->eth_htx_s.parity == ENABLE))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
			ATYPE_PAR_ERR));
	   }
	
	/*
	 * REMEMBER!!!  You must have called hard_reset() FIRST
	 * before calling get_netadd() which grabs the
	 * network address from the VPD.  This is because
	 * of the VPD problem requiring the PAUSE/RESUME stuff
	 * (10/03/89).  I could've put the hard_reset() in 
	 * get_netadd() too, but since we need one for the wrap
	 * we don't want to call another one which slows us down.
	 */
	if (rc = get_netadd(fdes, tucb_ptr))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	/*
	 *  Disable the Interrupts to directly access the command register
	 */
	if (hio_ctrl_wr(fdes, 0x00, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"wrap:  Disable interrupts Failed\n");
#endif
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RCTL_WR_ERR));
	   }
	
	/*
	 * enable/disable parity as desired...
	 *
	 * NOTE that the enable/disable of parity in the POS register
	 * is handled in the setup_pos() function call later.
	 */
	cval = hio_parity_rd(fdes, &status, tucb_ptr);
	if (status)
	   {
#ifdef debugg
		detrace(0,"wrap:  Reading parity reg. Failed\n");
#endif
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RPAR_RD_ERR));
	   }
	if (tucb_ptr->eth_htx_s.parity == ENABLE)
		cval |= 0x01;
	else
		cval &= 0xfe;
	if (hio_parity_wr(fdes, cval, &status, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"wrap:  Writing parity reg. Failed\n");
#endif
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RPAR_WR_ERR));
	   }

	/*
	 * set the enable indication
	 */
	if (rc = set_indication_enable(fdes, tucb_ptr))
	   {
#ifdef debugg
		detrace(0,"wrap:  Enable indication failed\n");
#endif
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
#ifdef debugg
	detrace(0,"wrap:  Go set network address\n");
#endif
	/*
	 * make sure adapter responds to our network address since
	 * we issued a hard reset
	 */
	if (rc = set_netadd(fdes, tucb_ptr))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
#ifdef debugg
	detrace(0,"wrap:  Go set filter\n");
#endif
	/*
	 * set up receive filter so that we receive packets
	 * sent to us.
	 *
	 * NOTE that the filter does NOT have to be set in promiscuous mode
	 * when specifying external wrap as per the hardware spec!
	 */
	if (htx_sp != NULL)
		sval = tucb_ptr->eth_htx_s.rcv_filter;
	else
		sval = 0x0002;

	if (rc = set_filter(fdes, sval, tucb_ptr))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
#ifdef debugg
	detrace(0,"wrap:  Go disable match filter\n");
#endif
	/*
	 * let's make sure that no pattern matching is going on, too.
	 */
	if (rc = disable_match_filter(fdes, tucb_ptr))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	/*
	 * set up loopback type in adapter - either INTERNAL or EXTERNAL
	 */
	if (tucb_ptr->eth_htx_s.wrap_type == INTERNAL)
		sval = 0x4000;
	else
		sval = 0x8000;
	
	if (rc = set_loopback(fdes, sval, tucb_ptr))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
	/*
	 * set up POS registers as needed:
	 *
	 *        connector type (enable BNC if specified)
	 *        enable memory window
	 *        setup address burst management
	 *        enable/disable fairness
	 *        enable/disable parity
	 */
	if (rc = setup_pos(fdes, tucb_ptr))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }

	/*
	 * go initialize the Transmit Mailbox 
	 */
	if (rc = setup_tx(fdes, &tx_mb_s, tucb_ptr))
	   {
		free(rxbuffer);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
   	   }


#ifdef debugg
	detrace(0,"wrap:  tx mb status is at:   0x%04x\n", tx_mb_s.a_status);
#endif
	

/* -- START THE LOOP -- */

#ifdef debugg 
	detrace(0,"wrap:  Ready to run through test patterns?\n");
#endif 
	/*
	 * Cycle through the test patterns
	 */
	for (i = 0; i < num_packets; i++)
	   {

		/*
		 * initialize read/transmit buffer (packet)
		 * with net addresses, netid, and pattern
		 * data.
		 */
		test_patt_byte = test_patt[i % num_test_bytes];
#ifdef debugg
		detrace(0,"wrap:  initialize rxbuffer with patt 0x%02x\n",
					test_patt_byte);
#endif
		init_rx_buf(rxbuffer, test_patt_byte, packet_size, tucb_ptr);

		/*
		 * alright, time for the ugly stuff:
		 *    Go and set up all your mailbox pointers,
		 *    allocate DMA buffers,
		 *    transmit/receive lists pointers AND more pointers
		 */

		/*
		 * First, we need to initialize a kernel DMA buffer -
		 * because the adapter cannot read/write to our own
		 * user space.  So, let's allocate one big DMA buffer
		 * and we'll use the first half for writing (transmit)
		 * and the second half for reading (receive).
		 *
		 * The DMA base address of the buf is returned through the
		 * pointer, "dma_base_add".
		 */

		if (rc = access_dma(fdes, LOCK_DMA, rxbuffer, user_buf_size,
				packet_size, &dma_base_add, tucb_ptr))
		   {
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }
		
		/*
		 * go set up the Receive Mailbox and Buffer Descriptors.
		 * Note we'll always use the same buffer descriptor in
		 * the Receive List so we can go ahead and set up 
		 * the DMA buffer address in here.
		 */
		if (rc = setup_rx(fdes, &rx_mb_s, &rx_bd_s,
				dma_base_add, tucb_ptr))
		   {
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				packet_size, user_buf_size,
				&dma_base_add, tucb_ptr);
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }

#ifdef debugg
		detrace(0,"wrap:  done DMA write, init_tx_bd\n");
#endif
	       /*
		* Set up the Transmit Buffer Descriptor
		*/
		if (rc = init_tx_bd(fdes, dma_base_add, packet_size,
				&tx_mb_s, &tx_bd_s, &former_bd_s, tucb_ptr))
		   {
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }

		/*
		 * Clear the status of
		 * the Receive Mailbox
		 * in preparation to receiving transmission.
		 */
		sval = 0x0000;
		sval = swap(sval);
		if (rc = smem_wr(fdes, (unsigned int) rx_mb_s.a_status,
			2, (unsigned char *) &sval, &status, tucb_ptr))
		   {
#ifdef debugg
    detrace(0,"wrap:  Writing to clear RX MB, sval = %x\n",sval);
#endif
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				RXMB_WR_ERR));
		   }
		

#ifdef debugg
		detrace(0,"wrap:  Issuing the Start Packet Reception\n");
#endif
		/*
		 * Write a 0x08 to the Command Register, to issue the
		 * start packet reception
		 */
		if (hio_cmd_wr(fdes, 0x08, &status, tucb_ptr))
		   {
#ifdef debugg
	   detrace(0,"wrap:  Command Register Write Failed\n");
#endif
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				RCMD_WR_ERR));
		   }
		
		if (cmd_recd(fdes, tucb_ptr))
		   {
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				REC_NOT_ERR));
		   }
#ifdef debugg
		detrace(0,"wrap:  Receive enabled, start transmit\n");
#endif

		/*
		 * Start transmission by clearing the EL bit in
		 * the presently pointed to transmit buffer descriptor
		 * whose info we saved off in "former_bd_s".
		 *
		 */
		cval = 0x00;
		if (rc = smem_wr(fdes,
			(unsigned int) former_bd_s.a_ctrl_status + 1,
			1, (unsigned char *) &cval, &status, tucb_ptr))
		   {
#ifdef debugg
    detrace(0,"wrap:  Writing to clear former bd, sval = %x\n",sval);
#endif
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
			
			if (htx_sp != NULL)
				(htx_sp->bad_writes)++;
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				TXBD_CT_WR_ERR));
		   }

#ifdef debugg
		detrace(0,"wrap:  TX begun, get compl. status\n");
#endif
		/*
		 * Go poll the transmit buffer descriptor
		 * for completion bit to indicate transmit done.
		 */
		if (rc = tx_complete_status(fdes, &tx_bd_s, tucb_ptr))
		   {
#ifdef debugg
			detrace(0,"wrap:  tx_complete status failed\n");
#endif
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
			if (htx_sp != NULL)
				(htx_sp->bad_writes)++;
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }
		if (htx_sp != NULL)
		   {
			(htx_sp->good_writes)++;
			htx_sp->bytes_writ += packet_size;
		   }
		
		/*
		 * Go poll the receive buffer descriptor
		 * for completion bit to indicate receive done.
		 */
		if (rc = rx_complete_status(fdes, &rx_bd_s, tucb_ptr))
		   {
			if (rc == 0x8888)
			{
				(void) access_dma(fdes, FREE_DMA, rxbuffer,
					user_buf_size, packet_size,
					&dma_base_add, tucb_ptr);
				(htx_sp->bad_reads)++;
				free(rxbuffer);
				return(0);
			}
#ifdef debugg
			detrace(0,"wrap:  rx_complete status failed\n");
#endif
			(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
			if (htx_sp != NULL)
				(htx_sp->bad_reads)++;
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }
		
		if (htx_sp != NULL)
		   {
			(htx_sp->good_reads)++;
			htx_sp->bytes_read += packet_size;
		   }

#ifdef debugg
		detrace(0,"wrap:  GOT packet!  read from DMA buf\n");
#endif

		/*
		 * We got a read, so let's init our own
		 * buffer with zeroes first.
		 */
		memset(rxbuffer, 0, packet_size);

		/*
		 * Now, we need to move the received data
		 * from the DMA buffer into our own
		 * user space.
		 *
		 * NOTE that if UNLOCK_DMA fails, we do NOT need
		 * to call FREE_DMA because the failed call will
		 * clean up for us.
		 */
		 if (rc = access_dma(fdes, UNLOCK_DMA, rxbuffer,
			user_buf_size, packet_size, &dma_base_add, tucb_ptr))
		   {
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
		   }
		
#ifdef debugg
		detrace(0,"wrap:  Compare buffer\n");
#endif
		/*
		 * okay, compare the buffer with the pattern
		 */
		for (j = 14; j < packet_size; j++)
			if (rxbuffer[j] != test_patt_byte)
			   {
				if (htx_sp != NULL)
					(htx_sp->bad_others)++;
#ifdef debugg
				detrace(0,"wrap:  compare failed\n");
				detrace(0,"       offset j = %d\n", j);
				detrace(0,"     rxbuffer j = 0x%02x\n",
							rxbuffer[j]);
				detrace(1,"    test_patt i = 0x%02x\n",
							test_patt_byte);

#endif
				free(rxbuffer);
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, WRAP_CMP_ERR));
			   }

	/*	
	 * Read Status register queue to check for Micro channel 
	 * exceptions or parity errors.
	 */

	rc = hio_status_rd_q(fdes, &status, tucb_ptr);
	if (status) 
 	   {
		/*
		 * Check status indicating the failure of reading
		 * status queue. If queue is empty discard else post
		 * error.
		 */

		if (status == CCC_QUE_EMPTY)
		    break;  	
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,QSTA_RD_ERR));
	    }

	if (rc & 0x80)
	{
		if(htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,ADAP_PAR_ERR));
	}

#ifdef debugg
		detrace(0,"wrap:  clear status in rx bd\n");
#endif


		/*
		 * Clear the ctrl/status (and set EL) in the
		 * Receive Buffer Descriptor
		 * in preparation to receiving another packet.
		 */
		sval = 0x4000;
		sval = swap(sval);
		if (rc = smem_wr(fdes, (unsigned int) rx_bd_s.a_ctrl_status,
			2, (unsigned char *) &sval, &status, tucb_ptr))
		   {
#ifdef debugg
    detrace(1,"wrap:  Writing to clear RX BD, sval = %x\n",sval);
#endif
			free(rxbuffer);
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				RXBD_ST_WR_ERR));
		   }


	   }   /* End for loop, looping on test patterns */

#ifdef debugg
	detrace(0,"wrap:  Done for now, time for a Hot Tub party\n");
#endif
	(void) access_dma(fdes, FREE_DMA, rxbuffer,
				user_buf_size, packet_size,
				&dma_base_add, tucb_ptr);
	/*
	 * free up malloc'd user space for TX/RX packet data
	 */
	free(rxbuffer);
	return(0);
  }

/*****************************************************************************

tu008

*****************************************************************************/

int tu008 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	/* Verify that the DIX Connector has been removed for	*/
	/* the BNC Wrap test					*/
	tucb_ptr->eth_htx_s.wrap_type = EXTERNAL_DIX;
	tucb_ptr->eth_htx_s.fairness = DISABLE;
	tucb_ptr->eth_htx_s.parity = DISABLE;

	if ((wrap(fdes, tucb_ptr) & 0x0000ffff) != TXBD_NOK_ERR)
		return(mktu_rc(tucb_ptr->header.tu,LOG_ERR,DIX_ON_BNC));

	if ((tucb_ptr != NULL) && ( tucb_ptr ->header.mfg == INVOKED_BY_HTX ))
		(tucb_ptr->eth_htx_s.htx_sp->bad_writes)--;
	tucb_ptr->eth_htx_s.wrap_type = EXTERNAL_BNC;
	tucb_ptr->eth_htx_s.fairness = DISABLE;
	tucb_ptr->eth_htx_s.parity = DISABLE;
	return(wrap(fdes, tucb_ptr));
   }
