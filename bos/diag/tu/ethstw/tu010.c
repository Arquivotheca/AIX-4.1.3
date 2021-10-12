static char sccsid[] = "@(#)38  1.5  src/bos/diag/tu/ethstw/tu010.c, tu_ethi_stw, bos411, 9428A410j 4/23/93 11:01:21";
/******************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	
 *		int col_wrap(); 
 *		int col_chk();
 *		int tu010();
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*****************************************************************************
Function(s) Test Unit 009 - External Loopback Test (BNC Connector)

Module Name :  tu010.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int io_rd();		rw_io.c
	extern int pos_rd();		rw_pos.c
	extern int pos_wr();		rw_pos.c
        extern int start_eth();		st_eth.c
	extern int halt_eth();  	st_eth.c
	extern int get_netaddr(); 	tu009.c

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
#include <sys/entuser.h>
#include "tu_type.h"
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/types.h>
#include <memory.h>

/*** external files ***/
        extern int io_rd();
        extern int start_eth();	
	extern int halt_eth();
	extern int pos_wr();
	extern int pos_rd();
	extern int get_netaddr();

/*** constants use in this file ***/
#define SHORT_PACKET  	  78    /* short packet size */
#define TIMEOUT		10000   /* poll timeout 10 sec */
/*** global constant ***/
uchar txbuffer[SHORT_PACKET];
uchar rxbuffer[SHORT_PACKET];

/*****************************************************************************
Function: int col_wrap()

This is the External wrap Test.
The microchannel sets up a transmit and receive buffer.
The 82596 is put into loopback mode, a transmit command is issued and
The test data is checked in the receive buffer. This is repeated for
different test patterns.
*****************************************************************************/
int col_wrap (int fdes, WRAPTEST *wrap_op, TUTYPE *tucb_ptr)
   {
	int i, rc=0, counter;
	uchar loop_byte;

	struct cfg cfg_setup;
	struct pollfd pollblk;
	struct status_block stat_blk;
	struct write_extension ext;

	static uchar short_pat[]= {0x33, 0xcc};

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

	/* Send the config structure to the device driver */
        if (ioctl(fdes, ENT_CFG, &cfg_setup))
	   {
	   PRINT((tucb_ptr->msg_file,"col_wrap: errno = %d",errno));
           PRINT((tucb_ptr->msg_file,"col_wrap: ioctl the ENT_CFG failed\n"));
           return (ENT_CFG_ERR);
           }

	/* get the network address from VPD */
	if (rc = get_netaddr(tucb_ptr))
           return (rc);

	/* fill in transmit buffer */
	for (i=0; i<6; i++) /* fill network address */
           {
           txbuffer[i]     = 0xff; /* default dest. address */
           txbuffer[i + 6] = tucb_ptr->ethi.net_addr[i];
           } /* end for loop for network id address */

	/* fill in netid */
	txbuffer[12] = (uchar) ((tucb_ptr->ethi.netid >> 8) & 0x00ff);
        txbuffer[13] = (uchar) (tucb_ptr->ethi.netid & 0x00ff);

	/* fill in data */
	counter = 0;
        for(i=14; i<(SHORT_PACKET-4); i++)
           {
           txbuffer[i]   = short_pat[0];
           txbuffer[i+4] = short_pat[1];
           counter +=1;
           if (counter == 4)
                {
                counter = 0;
                i += 4;
                }
           }

  	/******************
    	 * Sent a package
	 ******************/

	 /* set write extension */
	 ext.status = 0;
	 ext.flag = CIO_ACK_TX_DONE;
	 ext.write_id = 1;
	 ext.netid = tucb_ptr->ethi.netid;

	 rc = writex(fdes, &txbuffer, SHORT_PACKET, &ext);
    	 if (rc <= 0)
	    {
	    tucb_ptr->counter.bad_write++;
	    PRINT ((tucb_ptr->msg_file,"Unable to transmit data package"));
	    PRINT((tucb_ptr->msg_file,"rc = %d\n",rc));
	    return (WRAP_WR_ERR);
	    }	
	   
	/* delay before poll for write complete */
	for(i=0;i<0x7fff;i++);

   	/* Poll for status */ 
        pollblk.fd = fdes;
        pollblk.reqevents = POLLPRI;
        pollblk.rtnevents = POLLNVAL;

	rc = (poll(&pollblk, 1, TIMEOUT ));
        if (rc <= 0) 
          {
          PRINT((tucb_ptr->msg_file,"col_wrap: No status available \n"));
          return (STAT_NOT_AVAIL);
          }

	if (rc = ioctl(fdes, CIO_GET_STAT, &stat_blk) < 0)	
	   {
	   PRINT((tucb_ptr->msg_file,"col_wrap: CIO_GET_STAT failed.\n"));
	   PRINT((tucb_ptr->msg_file,"col_wrap: rc = %d\n",rc));
	   return (GET_STAT_ERR);
	   }

	if ((stat_blk.code & 0x0000ffff) != CIO_TX_DONE)
	   {
	   PRINT((tucb_ptr->msg_file,"col_wrap: code %ld\n",stat_blk.code));
	   return (TX_INCOMPLETE);
	   }
	
	if (stat_blk.option[0] == CIO_OK)
	   PRINT((tucb_ptr->msg_file,"col_wrap: CIO_OK\n"));

	if (stat_blk.option[3] != ENT_TX_MAX_COLLNS)
	   {
	   PRINT((tucb_ptr->msg_file,"col_wrap: option[3] = %ld\n",stat_blk.option[3]));
	   return (NO_COLLISION);
	   }

	return(0); /* test pass if its gets to this point */
  } /* end of col_wrap() */

/*****************************************************************************
col_chk ()- Function to check if there is any collision after wrap test.
*****************************************************************************/
int col_chk (int fdes, int mdd_fdes, TUTYPE *tucb_ptr)
   {
	uchar rd_data;
	ulong init_collision, final_collision;
	int errc, rc = 0; /* return codes */
	int result;
	WRAPTEST wrap_op;
	
	wrap_op.wrap_type = EXT_DISABLE;
	wrap_op.fairness = ENABLE;
	wrap_op.parity = ENABLE;

	/**********************************
         * Enable adapter, address and data
         * parity by reset POS2.
         **********************************/

        if (rc = pos_rd(mdd_fdes, POS2, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"col_chk: Unable to read POS2\n"));
          return (POS2_RD_ERR);
          }

        rd_data |= 0x3f;
        if (rc = pos_wr(mdd_fdes, POS2, &rd_data))
          {
          PRINT((tucb_ptr->msg_file,"col_chk: Unable to write to POS2\n"));
          return (POS2_WR_ERR);
          }

	/***********************
	 * check the riser cards
	 ***********************/

	if (io_rd(mdd_fdes, IO6, &rd_data))
          {
          PRINT ((tucb_ptr->msg_file,"col_chk: Unable to read I/O reg 6\n"));
          return (IO6_RD_ERR);
          }

	rd_data &= 0x03;
	if (rd_data != TP_CARD)
          {
          PRINT((tucb_ptr->msg_file,"col_chk: Riser card is not Twisted Pair\n"));
          return (RISERCARD_ERR);
          }
	
	/*** start the ethernet device ***/

        if (rc = start_eth(fdes, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"col_chk: Unable to start the device\n"));
	   if (result = halt_eth(fdes, tucb_ptr))
              PRINT((tucb_ptr->msg_file,"col_chk: halt rc = %d\n", result));
           return(rc);
           }

	if (rc = col_wrap(fdes, &wrap_op, tucb_ptr))
           {
	   /*  in collision there is nothing to read */
	   /*  (void) read(fdes, &rxbuffer, SHORT_PACKET);  */
           if (result = halt_eth(fdes, tucb_ptr))
                PRINT((tucb_ptr->msg_file,"col_chk: halt rc = %d\n", result));
           return (rc);
           }

	/* the collision test is successful */
        /*  in collision there is nothing to read */
	/* (void) read(fdes, &rxbuffer, SHORT_PACKET); */
	if (rc = halt_eth(fdes, tucb_ptr))
           {
           PRINT((tucb_ptr->msg_file,"col_chk: Unable to halt the device\n"));
           PRINT((tucb_ptr->msg_file,"col_chk: rc = %d\n",rc));
           return(rc);
           }

	return(0); /* Collision Test successful */ 
   } /* end of col_chk() */

/**********************************************************************
 * TU010();
 **********************************************************************/
int tu010 (int fdes, TUTYPE *tucb_ptr) 
   {
        int rc = 0; /* return code */

	rc = col_chk (fdes, tucb_ptr->mdd_fd, tucb_ptr);

        return (rc);
   } /* end of TU010() */
