static char sccsid[] = "@(#)31  1.6  src/bos/diag/tu/ethstw/tu009.c, tu_ethi_stw, bos411, 9428A410j 10/5/92 14:54:34";
/******************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	
 *		int get_netaddr();
 *		int chk_scb();
 *		int init_tx_buf(); 
 *		int tx_data();
 *		int wrap(); 
 *		int tu009();
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*****************************************************************************
Function(s) Test Unit 009 - External Loopback Test (BNC Connector)

Module Name :  tu009.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

OTHER ROUTINES:
        extern int io_rd();	rw_io.c
	extern int io_wr();	wr_io.c
	extern int pos_rd();	rw_pos.c
	extern int pos_wr();	rw_pos.c
	extern int smem_rd();	rw_smem.c
	extern int smem_wr();	rw_smem.c
        extern int start_eth();	st_eth.c
	extern int halt_eth();	st_eth.c
	extern int init_spos();	tu004.c
	extern int get_vpd();	tu004.c

   DATA AREAS:

   TABLES: none

   MACROS: none

COMPILER / ASSEMBLER

TYPE, VERSION: AIX C Compiler, version:

OPTIONS:

NOTES: Nones.
*****************************************************************************/

/*** header files ***/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <errno.h>
#include "tu_type.h"
#include <sys/entuser.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <memory.h>

/*** external files ***/
        extern int io_rd();
	extern int io_wr();
        extern int start_eth();	
	extern int halt_eth();
	extern int pos_wr();
	extern int pos_rd();
	extern int smem_rd();
	extern int smem_wr();
	extern int init_spos();
	extern int get_vpd();

/*** constants use in this file ***/
#define SHORT_PACKET      80    /* short packet size + heading */
#define LONG_PACKET      1510    /* long packet size + heading */
#define MAX_PACKET_SIZE 1514    /* maximum packet size  + heading */
#define MIN_PACKET_SIZE   78    /* minimum packet size */
#define MAX_RETRY	  3	/* maximun number to retry */
#define VPD_MAX_RETRY	  3
#define VPD_MAX_SIZE    0xff      /* maximum length of VPD PROM */
#define TxTIMEOUT         10000   /* time out for poll  10 sec */
#define RxTIMEOUT         30000   /* time out for poll  30 sec */

/*** global constant  ***/
uchar rxbuffer[MAX_PACKET_SIZE];
uchar txbuffer[MAX_PACKET_SIZE];
struct pollfd pollblk;
struct status_block stat_blk;

/***************************************************************************
Function: int get_netaddr()

Retrieve net work address from vpd .   
*****************************************************************************/
int get_netaddr(TUTYPE *tucb_ptr)
   {
	int rc = 10; /* initialize the rc with a failure return code */
	int zero = 0;   /* set constant */
	int index, retry; 	  
	uchar   vpd[VPD_MAX_SIZE];
	uchar vpd_size, i;

	/* Initialize NIO POS6 and POS7 */
        if (rc = init_spos(tucb_ptr->mdd_fd, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"Error while init. SPOS 6 & 7.\n"));
           return (rc);
           }

	/* Get VPD */       
	retry = 0;
	do
	   {
	   rc = get_vpd(tucb_ptr->mdd_fd, vpd, tucb_ptr);
	   ++retry;
	   }
	while ( (rc) && (retry <= VPD_MAX_RETRY) );

	if (rc)
	   {
           PRINT ((tucb_ptr->msg_file,"Failed to read VPD.\n"));
	   (void) init_spos(tucb_ptr->mdd_fd, tucb_ptr);
           return (rc);
           }

	/* Reinitialize NIO POS6 and POS7 */
        if (rc = init_spos(tucb_ptr->mdd_fd, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"Error while reinit SPOS 6 & 7.\n"));
           return (rc);
           }

	/* Get vpd_size, which is vpd length plus the heading - 8 bytes. */
        /* vpd length is 2 * vpd[5], since vpd[4] is zero */
	vpd_size= (2 * vpd[5]) + 8;
 
	/* Getting the Network Address */
        for (i=8; i<vpd_size; i++)
           { /* start for i loop */
           if ((vpd[i]=='E') && (vpd[i+1]=='T') && (vpd[i+2]=='H') &&
               (vpd[i+3]=='E') && (vpd[i+4]=='R') && (vpd[i+5]=='N') &&
               (vpd[i+6]=='E') && (vpd[i+7]=='T'))
              {
	      /* Store Network address in ethi structure */
	      for (index=0; index<6; index++)
	         tucb_ptr->ethi.net_addr[index] = vpd[i+12+index];
              break;
              }

           else if (i >= (vpd_size - 10))
              {
              PRINT((tucb_ptr->msg_file,"netaddr: Ethi VPD not found.\n"));
              return (NO_ETH_VPD);
              }
           } /* end of for i loop */

	return (0); /* Getting the Network Address is successful */
   } /* end get_netaddr() */

/*****************************************************************************
Function: int init_tx_buf()

Set up the transmit file.
The first 6 bytes of the data package must be the source address.
The next 6 bytes must be the destination address. The next 2 bytes
must be the netid; and the rest is the data.
For the ease of debugging, we fill the last 4 bytes of data frame with
0x99.

IF SUCCESS
THEN return 0
ELSE return the error code of the problem
*****************************************************************************/
int init_tx_buf (int lp_cnt, ulong p_size, WRAPTEST *wrap_op, TUTYPE *tucb_ptr)
   {
	short int i;
	int j, g;
	int rc = 10; /* initialize the rc with a failure return code */
	static uchar long_pat[] = {0xff, 0x00};
        static uchar short_pat[] = {0xaa, 0x55};
        static uchar tp_pat[] = {0x01, 0x01};
	uchar pattern[2];

	/* Select the data pattern */
        if (p_size == SHORT_PACKET || (lp_cnt%2))
            {
            pattern[0] = short_pat[0];
            pattern[1] = short_pat[1];
            }
        else
            {
            pattern[0] = long_pat[0];
            pattern[1] = long_pat[1];
            }

	/* due to bug in 82521, we force vpd and data to a know pattern */
        if (wrap_op->card_type == TWISTED_PAIR)
           {
           pattern[0] = tp_pat[0];
           pattern[1] = tp_pat[1];
           tucb_ptr->ethi.net_addr[0] = 0x08;
           tucb_ptr->ethi.net_addr[1] = 0x00;
           tucb_ptr->ethi.net_addr[2] = 0x5A;
           tucb_ptr->ethi.net_addr[3] = 0x0D;
           tucb_ptr->ethi.net_addr[4] = 0x03;
           tucb_ptr->ethi.net_addr[5] = 0xA6;
           }

	/* Fill in source and destination network addresses */
        for (i=0; i<6; i++)
           {
           txbuffer[i]     = 0xff; /* broadcast destination addr */ 
           txbuffer[i + 6] = tucb_ptr->ethi.net_addr[i];
           } /* end for loop for network id address */

        /* Put in the netid which define in exectu */ 
        txbuffer[12] = (uchar) ((tucb_ptr->ethi.netid >> 8) & 0x00ff);
        txbuffer[13] = (uchar) (tucb_ptr->ethi.netid & 0x00ff);

	/* Initialize the Transmit Buffer to a set pattern */
	g=0;
        for(i=14; i<p_size; i+=4) {
           for (j=0; j<4; j++) {
              if ((i+j) == p_size)
                  break;
              txbuffer[i+j] = pattern[g];
           }
           g=!g;
        }

	/* Make the last 4 byte of data package to be 0x99 */
	for(i=p_size-4; i<p_size; i++)
	   txbuffer[i] = 0x99;

	return (0); /* successful */
   } /* end init_tx_buf */

/*************************************************************************
Function: int chk_scb ();

This fuction will read the data from local memory to determine whether
the POLL timeout for receiver buffer causes by overrun or not.
*****************************************************************************/
int chk_scb (TUTYPE *tucb_ptr)
   {
	int rc = 10;	/* initialize rc with the failure return code */
	uchar rd_data;
	int start_addr; 

	struct
	   {
	   int crc; 		/* crc errors */
	   int alignment; 	/* alignment errors */
	   int resource;	/* resource errors */
	   int overrun;		/* overrun errors */
	   int rcvcdt;		/* rcvcdt errors */
	   int short_fr;	/* short frame errors */
	   } errors;

        /* read Hi byte of address */
	if (io_rd(tucb_ptr->mdd_fd, IO1, &rd_data))
           PRINT((tucb_ptr->msg_file,"scb: Unable to write IO Register 1\n"));

	start_addr = rd_data;

	/* read Lower byte of address */
	if (io_rd(tucb_ptr->mdd_fd, IO0, &rd_data))
	    PRINT((tucb_ptr->msg_file,"scb: Unable to write IO Register 0\n"));

	start_addr = (start_addr << 8) | rd_data;    

	/* If start_addr is 0 then RAM offset is 0x0000.
	   If start_addr is 1 then RAM offset is 0x1000.
	   So we need to shift to the left 12 places. */
	start_addr = (start_addr << 12);

	start_addr += 0x20; /* start address for errors in scb */ 

	if (smem_rd(tucb_ptr->mdd_fd, start_addr, 6, &errors))
	   PRINT((tucb_ptr->msg_file,"scb: Failed to read overrun word\n")); 

 	if (errors.overrun)
	   return (1); /* overrun occurs */
	else
	   {
	   PRINT((tucb_ptr->msg_file,"scb: crc = %#x\n",errors.crc)); 
	   PRINT((tucb_ptr->msg_file,"scb: alignment=%#x\n",errors.alignment)); 
	   PRINT((tucb_ptr->msg_file,"scb: resource = %#x\n",errors.resource)); 
	   PRINT((tucb_ptr->msg_file,"scb: overrun = %#x\n",errors.overrun)); 
	   PRINT((tucb_ptr->msg_file,"scb: rcvcdt = %#x\n",errors.rcvcdt)); 
	   PRINT((tucb_ptr->msg_file,"scb: short_fr = %#x\n",errors.short_fr)); 
	   return (0); /* no overrun */
	   }
   } /* end of chk_scb */

/*****************************************************************************
Function: int tx_data ();

This function will execute the write command to transmit data and check 
the transmit status.   
*****************************************************************************/
int tx_data(int fdes, ulong p_size, TUTYPE *tucb_ptr)
   {
	int rc = 10; /* initialize the rc with a failure return code */
	int err = 0; /* return code from poll */
	int i;

        struct write_extension ext;

        /* set write extension */
        ext.status = 0;
        ext.flag = CIO_ACK_TX_DONE;
        ext.write_id = 1;
        ext.netid = tucb_ptr->ethi.netid;

	/* transmit data */
        rc = writex(fdes, &txbuffer, p_size, &ext);
        if (rc <= 0 || rc != p_size)
           {
           PRINT((tucb_ptr->msg_file,"Unable to transmit data package"));
           PRINT((tucb_ptr->msg_file,"rc = %d\n",rc));
           return (WRAP_WR_ERR);
           }

	/** Delay before polling for write complete */
	for (i=0;i<0x7fff;i++);

        /* Poll for status */
        pollblk.fd = fdes;
        pollblk.reqevents = POLLPRI;
        pollblk.rtnevents = 0;

        err = poll(&pollblk, 1, TxTIMEOUT);
        if (err <= 0)
           {
           PRINT((tucb_ptr->msg_file,"wrap: No status available \n"));
           PRINT ((tucb_ptr->msg_file,"wrap: errno=%d: err=%d\n", errno, err));
           return (STAT_NOT_AVAIL);
           }

	/* Get the transmit status */
        if (rc = ioctl(fdes,CIO_GET_STAT, &stat_blk) < 0)
           {
           PRINT((tucb_ptr->msg_file,"wrap: CIO_GET_STAT failed.\n"));
           PRINT((tucb_ptr->msg_file,"wrap: rc = %d\n",rc));
           PRINT ((tucb_ptr->msg_file,"wrap: errno = %d\n",errno));
           return (GET_STAT_ERR);
           }

	/* Check for transmit complete */
        if (( stat_blk.code & 0x0000ffff) != CIO_TX_DONE)
           {
           PRINT((tucb_ptr->msg_file,"wrap: code %ld\n",stat_blk.code));
	   PRINT((tucb_ptr->msg_file,"wrap: opt[3]=%#x\n", stat_blk.option[3]));
           return (TX_INCOMPLETE);
           }

	/* Check for successfully transmitted */
        if (stat_blk.option[0] != CIO_OK)
	   {
	   if (stat_blk.option[3] == ENT_TX_UNDERRUN)
	      return (UNDERRUN_ERR);
	   else
	      {
	      PRINT((tucb_ptr->msg_file,"wrap: opt[3]=%#x\n", rc));
              return (CIO_NOT_OK);
	      }
	   }

	/* Wait for recieve data by polling for it */
	pollblk.fd = fdes;
	pollblk.reqevents = POLLIN;
	pollblk.rtnevents = 0;

	err = poll(&pollblk, 1, RxTIMEOUT);
	if (err <= 0)
	   {
	   if (rc = chk_scb (tucb_ptr))
	      rc = OVERRUN_ERR; 
           else
	      {
              PRINT((tucb_ptr->msg_file,"wrap: errno=%d: err=%d\n",errno,err));
	      PRINT((tucb_ptr->msg_file,"Wrap: Poll failed and not overrun\n"));
	      rc = WRAP_NODATA;
	      }
	   return (rc);
	   }

        return(0);
   } /* tx_func end */

/*****************************************************************************
Function: int wrap()

This is the Internal/External wrap Test.
The 82596 is put into loopback mode, a transmit command is issued and
The test data is checked in the receive buffer. This is repeated for
different test patterns.
*****************************************************************************/

int wrap (int fdes, WRAPTEST *wrap_op, TUTYPE *tucb_ptr)
   {
        int rc = 10; /* initialize rc with a failure return code */ 
	int i, result;
	int index; 
	int pckt_lp;
	uchar loop_byte;
	int retry;

	struct cfg cfg_setup;

	long packet_size;
	long num_pckt;
	ulong posregs = 0x0000c000;

	/** initialize counter ***/
	tucb_ptr->counter.bad_other = 0;
	tucb_ptr->counter.good_read = 0;
	tucb_ptr->counter.bad_read = 0;
	tucb_ptr->counter.good_write= 0;
	tucb_ptr->counter.bad_write = 0;
	tucb_ptr->counter.byte_read = 0;
	tucb_ptr->counter.byte_write = 0;

	/********************************************************
         * Setup the configuration
         * Most of the parameter, except loopback and promiscuous,
         * are assigned with the default value.
         ********************************************************/

        loop_byte = (wrap_op->wrap_type << 6) | (0x2e);

  	cfg_setup.csc                = 0x00;
        cfg_setup.next_cb            = 0xffffffff;
        cfg_setup.loopback           = loop_byte;
        cfg_setup.save_bf            = 0x40;
        cfg_setup.fifo_limit         = 0xcf;
        cfg_setup.byte_count         = 0x0e;
        cfg_setup.slot_time_up       = 0xf7; 
        cfg_setup.slot_time_low      = 0xff;
        cfg_setup.spacing            = 0x60;
        cfg_setup.linear_pri         = 0x00;
        cfg_setup.preamble           = 0xdf; 
        cfg_setup.frame_len          = 0x40;
        cfg_setup.carrier_sense      = 0x00;
        cfg_setup.promiscuous        = 0x00;
        cfg_setup.dcr_num            = 0x00;
        cfg_setup.dcr_slot           = 0x00;

	/* setup pos register */

	if (wrap_op->fairness == ENABLE)
	   posregs |= 0x80000; /* enable fairness */ 
	else
	   posregs &= 0xfff7ffff; /* disable fairness */

		/* setup parity */
	posregs |= 0x3f000000; /* enable parity */
	if (wrap_op->parity == DISABLE)
	   posregs &= 0xfdffffff; /* disable parity */

	/* Now send posregs to the device driver */
	if (ioctl(fdes, ENT_POS, &posregs) < 0)
	  {
	  PRINT((tucb_ptr->msg_file,"wrap: ioctl the ENT_POS failed\n"));
          return (ENT_POS_ERR);
	  }

	/* get network address */
        if (rc = get_netaddr(tucb_ptr))
           {
           tucb_ptr->counter.bad_other++;
           if (result = halt_eth(fdes, tucb_ptr))
               PRINT((tucb_ptr->msg_file,"wrap: raltrc = %d\n", result));
           return (rc);
           }

	/*** start the ethernet device ***/
        if (rc = start_eth(fdes, tucb_ptr))
           {
	   tucb_ptr->counter.bad_other++;
           PRINT((tucb_ptr->msg_file,"wrap: Unable to start the device\n"));
	   if (result = halt_eth(fdes, tucb_ptr))    
                PRINT((tucb_ptr->msg_file,"wrap: raltrc = %d\n", result));
           return(rc);
           }

        /*** Send the config structure to the device driver *****/
          if (ioctl(fdes, ENT_CFG, &cfg_setup) < 0)
             {
	     tucb_ptr->counter.bad_other++;
	     PRINT((tucb_ptr->msg_file,"wrap: ioctl the ENT_CFG failed\n"));
	     if (result = halt_eth(fdes, tucb_ptr))    
                PRINT((tucb_ptr->msg_file,"wrap: raltrc = %d\n", result));
             return (ENT_CFG_ERR);
             }

	if (tucb_ptr->header.mfg == INVOKED_BY_HTX)
           num_pckt = tucb_ptr->ethi.num_packet;
        else
           num_pckt = 2;
	
	for (pckt_lp=1; pckt_lp <= num_pckt; pckt_lp++)
	   { /* for loop to transmit all the packages */
	   if (pckt_lp == 1 && tucb_ptr->header.mfg != INVOKED_BY_HTX)
              packet_size = SHORT_PACKET;
	   else if (pckt_lp == 2 && tucb_ptr->header.mfg != INVOKED_BY_HTX)
              packet_size = LONG_PACKET;
	   else
	      packet_size = tucb_ptr->ethi.packet_size;
	
	   /* Make sure the packet size is not exceed the limitation */
	   if (packet_size > MAX_PACKET_SIZE)
	      {
              PRINT((tucb_ptr->msg_file,"wrap: Packet size is too big.\n"));
	      if (result = halt_eth(fdes, tucb_ptr))
                PRINT((tucb_ptr->msg_file,"wrap: raltrc = %d\n", result));
              return (BAD_PCK_S_ERR);
	      }

	  /* Make sure the packet size is not smaller than the Minimum size */
	  if (packet_size < MIN_PACKET_SIZE)
              {
              PRINT((tucb_ptr->msg_file,"wrap: Packet size is too small.\n"));
	      if (result = halt_eth(fdes, tucb_ptr))
                PRINT((tucb_ptr->msg_file,"wrap: raltrc = %d\n", result));
              return (BAD_PCK_S_ERR);
              }

	  /* initialize transmit buffer with zero */
          memset(&txbuffer, 0, packet_size);

          /* setup data to transmit */
          if (rc = init_tx_buf(pckt_lp, packet_size, wrap_op, tucb_ptr))
             {
             PRINT((tucb_ptr->msg_file,"wrap: init_tx_buf failed.\n"));
             return (rc);
             }

	  retry = 0;	/* initalize retry */
	  rc = tx_data(fdes, packet_size, tucb_ptr);
	  while ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
		((rc==UNDERRUN_ERR) || (rc==OVERRUN_ERR)) &&
		 (retry <= MAX_RETRY))
	     {
     	     (void) read(fdes, &rxbuffer, packet_size);
     	     /* delay waiting for bus to clear */
	     sleep (2);
	     retry += 1; /* increase number of retry */
	     rc = 10; /* reinitialize the return code */ 
	     rc = tx_data(fdes, packet_size, tucb_ptr);
	     } /* end while */

	  if (rc) 
	     { /* trans. fail.  It's not underrun or retry >3 */
	     if ((rc==CIO_NOT_OK) || (rc==UNDERRUN_ERR))
	        tucb_ptr->counter.bad_write++;
    	     else
	        tucb_ptr->counter.bad_read++;
             PRINT((tucb_ptr->msg_file,"wrap: package # %d failed\n",pckt_lp)); 
             PRINT((tucb_ptr->msg_file,"wrap: p_size # %d\n",packet_size)); 
	     (void) read(fdes, &rxbuffer, packet_size);
	     if (result = halt_eth(fdes, tucb_ptr))
                PRINT((tucb_ptr->msg_file,"wrap: haltrc = %d\n", result));
	     return (rc);
	     } 

	   tucb_ptr->counter.good_write++;
           tucb_ptr->counter.byte_write += packet_size;

	  /* initialize receive buffer with zero */
          memset(&rxbuffer, 0, packet_size);

	   /* Read back the sent package */
    	   rc = read(fdes, &rxbuffer, packet_size);
	   if ((rc <= 0) || (rc != packet_size))
	      {
	      tucb_ptr->counter.bad_read++;
	      PRINT((tucb_ptr->msg_file,"Read: rc = %d\n",rc));
	      PRINT((tucb_ptr->msg_file,"Wrap: Unable to receive data\n"));
	      if (result = halt_eth(fdes, tucb_ptr))
                 PRINT((tucb_ptr->msg_file,"wrap: raltrc = %d\n", result));
	      return (WRAP_RD_ERR);
	      }

	   tucb_ptr->counter.good_read++;
           tucb_ptr->counter.byte_read += packet_size;

	   /********************************************
	    * okay, compare the buffer with the pattern
	    ********************************************/
	   for (i=14; i<packet_size; i++)
		{ /* compare data */
		if (rxbuffer[i] != txbuffer[i])
		   {
		   tucb_ptr->counter.bad_other++;
		   PRINT((tucb_ptr->msg_file,"wrap: compare failed at %d\n",i));
		   PRINT((tucb_ptr->msg_file,"txbuffer\n"));
                   for(index=0;index<packet_size;index++)
                      {
                      if ((index>0) && ((index%32)==0))
                         PRINT((tucb_ptr->msg_file,"\n"));
                      PRINT((tucb_ptr->msg_file,"%.2x",txbuffer[index]));
                      }
                   PRINT((tucb_ptr->msg_file,"\nrxbuffer\n"));
                   for(index=0;index<packet_size;index++)
                      {
                      if ((index>0) && ((index%32)==0))
                         PRINT((tucb_ptr->msg_file,"\n"));
                      PRINT((tucb_ptr->msg_file,"%.2x",rxbuffer[index]));
                      }
                   PRINT((tucb_ptr->msg_file,"\n"));
		   if (result = halt_eth(fdes, tucb_ptr))
                      PRINT((tucb_ptr->msg_file,"wrap: raltrc = %d\n", result));
		   return (WRAP_CMP_ERR);
		   } /* end if */
		} /* end for, compare data */

	   }   /* End for loop, pckt_lp */
	
	/************************
	 * Done. Now, we can HALT 
	 ***********************/

	if (rc = halt_eth(fdes, tucb_ptr))
           {
	   tucb_ptr->counter.bad_other++;
           PRINT((tucb_ptr->msg_file,"wrap: Unable to halt the device\n"));
           PRINT((tucb_ptr->msg_file,"wrap: rc = %d\n",rc));
           return(rc);
           }

	return(0); /* transmit is successfully completed */
  } /* end of wrap() */

/*****************************************************************************
Function: int tu009()

 External Loopback Test (BNC Connector)
*****************************************************************************/
int tu009 (int fdes, TUTYPE *tucb_ptr) 
   {
	uchar rd_data;
        int rc = 10; /* initialize rc with a failure return code */ 
        WRAPTEST wrap_op;

	wrap_op.wrap_type = EXT_DISABLE;
	wrap_op.fairness = DISABLE;	
	wrap_op.parity = DISABLE;
	wrap_op.card_type = THIN;

	/**********************************
         * Enable adapter, address and data
         * parity by reset POS2.
         **********************************/

        if (rc = pos_rd(tucb_ptr->mdd_fd, POS2, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu009: Unable to read POS2\n"));
          return (POS2_RD_ERR);
          }

        rd_data |= 0x3f;
        if (rc = pos_wr(tucb_ptr->mdd_fd, POS2, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"tu009: Unable to write to POS2\n"));
          return (POS2_WR_ERR);
          }

	/**************************
         * check the riser cards
         **************************/
        if (rc = io_rd(tucb_ptr->mdd_fd, IO6, &rd_data))
	  {
          PRINT((tucb_ptr->msg_file,"tu009: Unable to read from I/O reg 6\n"));
          return (IO6_RD_ERR);
          }

	rd_data &= 0x03;
        if (rd_data != TN_CARD)
           {
           PRINT((tucb_ptr->msg_file,"The riser card is not THIN.\n"));
           return (RISERCARD_ERR);
           }

	rc = wrap(fdes, &wrap_op, tucb_ptr);

        return (rc);
   } /* end of tu009() */
