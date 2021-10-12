static char sccsid[] = "@(#)75	1.19  src/bos/kernext/tokdiag/tokutil.c, diagddtok, bos411, 9428A410j 10/26/93 16:25:17";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  conn_done(), tokdsgetvpd()
 *             ds_startblk(), ds_halt(),
 *             get_adap_point(), get_ring_info(), move_ring_info(),
 *             async_status(), logerr()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include "tok_comio_errids.h"
#include <sys/err_rec.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/tokuser.h>
#include <sys/adspace.h>


#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

/*****************************************************************************/
/*
 * NAME:     conn_done
 *
 * FUNCTION: notify all callers in netid table that start is complete or failed
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void
conn_done (
register dds_t *p_dds) /* pointer to dds structure */
{
	register open_elem_t *open_ptr;
	int                   saved_intr_level;
	int                   ndx;
	chan_t                chan;
	cio_stat_blk_t        stat_blk;
	int                   start_ok;

	DEBUGTRACE2 ("CONb", (ulong)p_dds); /* conn_done begin */

	DISABLE_INTERRUPTS (saved_intr_level);
	start_ok = TRUE;

	/* we need to notify every caller with netid currently in table */
	for (ndx = 0; (ndx < TOK_MAX_NETIDS); ndx++) {
		chan = CIO.netid_table[ndx].chan;
		open_ptr = CIO.open_ptr[chan-1];

		if (CIO.chan_state[chan - 1] == CHAN_OPENED) {
			ENABLE_INTERRUPTS (saved_intr_level);
			/* let ds code build status block */
			ds_startblk (p_dds, (netid_t)ndx, &stat_blk);

			/* "asynchronous" notification to caller */
			report_status (p_dds, open_ptr, &stat_blk);

			if (stat_blk.option[0] != CIO_OK)
				start_ok = FALSE;
			DISABLE_INTERRUPTS (saved_intr_level);
		}
	}

	if (start_ok)
		CIO.device_state = DEVICE_CONNECTED;
	else {
		CIO.device_state = DEVICE_NOT_CONN;
		for (ndx = 0; (ndx < TOK_MAX_NETIDS); ndx++) {
			chan = CIO.netid_table[ndx].chan;
			while ( remove_netid_for_chan(p_dds, chan) );
		}
	}

	ENABLE_INTERRUPTS (saved_intr_level);

	if (CIO.netid_table[TOK_MAC_FRAME_NETID].inuse == TRUE)
		p_dds->wrk.mac_frame_active = TRUE;
	else
		p_dds->wrk.mac_frame_active = FALSE;

	DEBUGTRACE1 ("CONe"); /* conn_done end */
	return;
} /* end conn_done */


/****************************************************************************
*
* ROUTINE NAME: tokdsgetvpd
*
* DESCRIPTIVE NAME: Ethernet Device Specific adapter get adapter's vital
*                   product data
*
* FUNCTION: Read and store the adapter's Vital Product Data via POS registers
*
* INPUT: DDS pointer - tells which adapter
*
* OUTPUT: Vital Product Data stored in the DDS work area along with the
*         status of the p_dds->vpd.
*
* RETURN:  N/A
*
* CALLED FROM: ds_initdds
*
* CALLS TO: N/A
*
*****************************************************************************
*/

void tokdsgetvpd (
   dds_t *p_dds)
{
   int    index;
   int    index2;
   int    vpd_found;   /* Vital Product Data header found flag               */
   int    na_found;    /* Network Address found flag                         */
   int    rl_found;    /* ROS Level found flag                               */
   int    crc_valid;   /* CRC valid flag                                     */
   int    pio_attachment;
   ushort cal_crc;     /* Calculated value of CRC                            */
   ushort vpd_crc;     /* Actual VPD value of CRC                            */

    DEBUGTRACE2 ("vpdB", (ulong)p_dds);

   /* Get access to the IOCC to access POS registers                         */
    pio_attachment = attach_iocc( p_dds );

   /* Test to verify this is correct adapter type                            */
   if ((PR0_VALUE != pio_read( p_dds, POS_REG_0)) ||   /* POS 0 = 0xF5  */
       (PR1_VALUE != pio_read( p_dds, POS_REG_1)))     /* POS 1 = 0x8E  */
   {

      /* Not the correct adapter type or SLOT # - Leave adapter/SLOT alone   */
      /* Update the Vital Product Data Status                                */
      p_dds->vpd.status = TOK_VPD_NOT_READ;

   }
   else /* adapter detected - read Vital Product Data          */
   {

      /* Initialize POS Registers 6 & 7 to zero                              */
        pio_write( p_dds, POS_REG_6, 0 );
        pio_write( p_dds, POS_REG_7, 0 );

      /***********************************************************************/
      /* NOTE: "for" loop does not handle VPD greater than 256 bytes         */
      /***********************************************************************/

      /* Get VPD from adapter for the default length                         */
      for (index=0; index < TOK_VPD_LENGTH; index++)
      {
         /* Set up the correct address for the VPD read byte                 */
         pio_write( p_dds, POS_REG_6, (index + 1));
         p_dds->vpd.vpd[index] = pio_read( p_dds, POS_REG_3 );
      }

      /* Initialize POS Registers 6 & 7 to zero                              */
        pio_write( p_dds, POS_REG_6, 0 );
        pio_write( p_dds, POS_REG_7, 0 );

      /* Test some of the Fields of Vital Product Data                       */
      if ((p_dds->vpd.vpd[0] == 'V') && /* 'V' = Hex 56            */
          (p_dds->vpd.vpd[1] == 'P') && /* 'P' = Hex 50            */
          (p_dds->vpd.vpd[2] == 'D') && /* 'D' = Hex 44            */
          (p_dds->vpd.vpd[7] == '*'))   /* '*' = Hex 2A            */
      {

         /* Update the Vital Product Data Status - Valid Data                */
         vpd_found = TRUE;
         na_found  = FALSE;
         rl_found  = FALSE;
         crc_valid = FALSE;

         /* Update the Vital Product Data length                             */
         p_dds->vpd.l_vpd =
                ((2 * ((p_dds->vpd.vpd[3] << 8) |
                        p_dds->vpd.vpd[4])) + 7);

         /* Test for which length will be saved - save the smaller           */
         if (p_dds->vpd.l_vpd > TOK_VPD_LENGTH)
         {
            p_dds->vpd.l_vpd = TOK_VPD_LENGTH;

            /* Mismatch on the length - can not test crc - assume crc is good*/
            crc_valid = TRUE;

         }
         else
         {

            /* Put together the CRC value from the adapter VPD               */
            vpd_crc = ((p_dds->vpd.vpd[5] << 8) |
                        p_dds->vpd.vpd[6]);


            /* One can only verify CRC if one had enough space to save it all*/
            /* Verify that the CRC is valid                                  */
            cal_crc = cio_gen_crc(&p_dds->vpd.vpd[7],
                                (p_dds->vpd.l_vpd - 7));


            /* Test if the CRC compares                                      */
            if (vpd_crc == cal_crc)
               crc_valid = TRUE;

         }

         /* Get Network Address and ROS Level                                */
         for (index=0; index < p_dds->vpd.l_vpd; index++)
         {
            /* Test for the Network Address Header                           */
            if ((p_dds->vpd.vpd[(index + 0)] == '*') &&
                (p_dds->vpd.vpd[(index + 1)] == 'N') &&
                (p_dds->vpd.vpd[(index + 2)] == 'A') &&
                (p_dds->vpd.vpd[(index + 3)] ==  5 ))
            {


               /* Set the Network Address found flag                         */
               na_found = TRUE;

               /* Save Network Address in DDS work section                   */
               for (index2 = 0; index2 < TOK_NADR_LENGTH; index2++)
               {
                  /* Store the indexed byte in the DDS Work Section          */
                   p_dds->wrk.tok_vpd_addr[index2] =
                        p_dds->vpd.vpd[(index + 4 + index2)];


               }
            }
            else /* NetWork Address not found - Do Nothing                   */
            {

            } /* endif test for network address header                       */

            /* Test for the ROS Level Header                                 */
            if ((p_dds->vpd.vpd[(index + 0)] == '*') &&
                (p_dds->vpd.vpd[(index + 1)] == 'R') &&
                (p_dds->vpd.vpd[(index + 2)] == 'L'))
            {


               /* Set the ROS Level found flag                               */
               rl_found = TRUE;

               /* Save ROS Level in DDS work section                         */
               for (index2 = 0; index2 < ROS_LEVEL_SIZE; index2++)
               {
                  /* Store the indexed byte in the DDS Work Section          */
                  p_dds->wrk.tok_vpd_rosl[index2]
                        = p_dds->vpd.vpd[(index + 4 + index2)];

               }
            }
            else /* ROS Level Header not found - Do Nothing                  */
            {
            } /* endif test for ROS Level header                             */
         }

         /* Test the appropriate flags to verify everything is valid */
         if ((vpd_found == TRUE) &&        /* VPD Header found */
             (na_found  == TRUE) &&        /* Network Address found */
             (rl_found  == TRUE) &&     /* ROS Level found */
             (crc_valid == TRUE))       /*  CRC value is valid */
         {

            /* VPD is valid based on the test we know to check               */
            p_dds->vpd.status = TOK_VPD_VALID;
         }
         else
         {
           /* VPD failed the test - set the status                         */
            p_dds->vpd.status = TOK_VPD_INVALID;


         } /* endif for test if VPD is valid                                 */
      }
      else /* Bad Vital Product Data - Set VPD status                        */
      {

         /* Update the Vital Product Data Status                             */
         p_dds->vpd.status = TOK_VPD_INVALID;

      } /* endif test of some of the VPD fields                              */
   } /* endif test to verify correct adapter type                            */

   /* restore IOCC to previous value - done accessing POS Regs               */
        detach_iocc( p_dds, pio_attachment );

    DEBUGTRACE1 ("vpdE");
   return;
} /* end tokdsgetvpd */


/*****************************************************************************/
/* build a start done status block                                           */
/*****************************************************************************/
void
ds_startblk (
dds_t          *p_dds,
netid_t        netid,
struct status_block *sta_blk_ptr)
{
	DEBUGTRACE4 ("sblb", (ulong)p_dds, (ulong)netid, (ulong)sta_blk_ptr);

	sta_blk_ptr->code = (ulong)CIO_START_DONE;
	sta_blk_ptr->option[0] = p_dds->wrk.start_done_blk.option[0];
	sta_blk_ptr->option[1] = netid;
	sta_blk_ptr->option[2] = p_dds->wrk.start_done_blk.option[2];
	sta_blk_ptr->option[3] = p_dds->wrk.start_done_blk.option[3];

	DEBUGTRACE1 ("sble");
	return;
}

/*****************************************************************************/
/* process a halt                                                            */
/*****************************************************************************/
void ds_halt(
   register dds_t              *p_dds,
   register open_elem_t        *open_ptr,
   register struct session_blk *sess_blk_ptr)
{
/*    cio_stat_blk_t stat_blk; */

   struct status_block stat_blk;

   DEBUGTRACE4 ("hltb", (ulong)p_dds, (ulong)open_ptr, (ulong)sess_blk_ptr);

   stat_blk.code = (ulong)CIO_HALT_DONE;
   stat_blk.option[0] = (ulong)CIO_OK;
   stat_blk.option[1] = sess_blk_ptr->netid;
   stat_blk.option[2] = NULL;
   stat_blk.option[3] = NULL;

   report_status (p_dds, open_ptr, &stat_blk);

   DEBUGTRACE1 ("hlte");
   return;
} /* end ds_halt */


/*
*  FUNCTION:   get_adap_point()
*
*          This function will read the Token-Ring adapter's internal
*          pointers for the following:
*              - Microcode Level
*              - Adapter Addresses
*                  Node
*                  Group
*                  Functional
*              - Adapter Parameters
*                  This is what is return via
*                  the TOK_RING_INFO ioctl.
*              - MAC Buffer
*
*          This function should be successfully issued before any
*          READ ADAPTER SCB command can be issued to the adapter.
*
*  INPUT:      p_dds   - pointer to DDS
*
*
*/
int get_adap_point(dds_t *p_dds)
{  /* begin function */


        int     pio_attachment;
        pio_attachment = attach_bus( p_dds );
   /*
   *   Get the pointer to the
   *   microcode level
   */

        pio_write( p_dds, ADDRESS_REG, P_A_UCODE_LEVEL );
   p_dds->wrk.p_a_ucode_lvl = pio_read( p_dds, DATA_REG );

   /*
   *   Get the pointer to the
   *   Adapter Addresses
   */

        pio_write( p_dds, ADDRESS_REG, P_A_ADDRS );
   p_dds->wrk.p_a_addrs = pio_read( p_dds, DATA_REG );



   /*
   *   Get the pointer to the
   *   Adapter parmeters...for TOK_RING_INFO ioctl
   */

        pio_write( p_dds, ADDRESS_REG, P_A_PARMS );
        p_dds->wrk.p_a_parms = pio_read( p_dds, DATA_REG );


   /*
   *   Get the pointer to the
   *   MAC Buffer
   */

        pio_write( p_dds, ADDRESS_REG, P_A_MAC_BUF );
        p_dds->wrk.p_a_mac_buf = pio_read( p_dds, DATA_REG );


        detach_iocc( p_dds, pio_attachment );
   return(0);


}  /* end function get_adap_point() */
/*
*  FUNCTION:   get_ring_info()
*
*          This function will issue a READ ADAPTER command via
*          the issue_scb_command() routine.
*
*          This routine assumes that the adapter's pointer to the Adapter
*          Parameters has already been read and that it has been stored
*          in the p_a_parms variable.  It also assumes that the ACA has been
*          set up and that the bus address to put the Ring Information has
*          been calculated and is stored in the p_d_ring_info variable.
*
*
*  INPUT:      p_dds - pointer to DDS
* 
*  OUTPUT:
*	0 - successful 
*	ENETDOWN - unable to get ring info
*/
int get_ring_info(dds_t *p_dds)
{  /* begin function */

  int rc;
  tok_ring_info_t    tmpinfo;

  /*
   *   To read the adapter parameters (Token-Ring Information, it
   *   consists of 2 parts: The read adapter buffer and the SCB command.
   *   The adapter buffer in this routine is the tmpinfo variable that
   *   will be moved to the ACA_RING_INFO_BASE location in the ACA.
   *
   *   The adapter buffer is:
   *           ----------------
   *           |  data count  |
   *           |--------------|
   *           |  data addr   |
   *           |--------------|
   *           |    data      |
   *           |    area      |
   *           |     :        |
   *           |     :        |
   *           |--------------|
   *
   *   where the data count is the number of bytes to be read and the
   *   data addr is the adapter address to be read from.
   *
   *   The data will be stored in the adapter buffer starting at the
   *   data count location.
   *
   *   Since the tok_ring_info_t structure is what is to be returned,
   *   and this is what we are really after, we use the tok_ring_info_t
   *   structure for convienence and make it look like the
   *   adapter buffer by using the first entry in the tok_ring_info_t
   *   to our advantage.
   *
   *   We want to read sizeof(tok_ring_info_t) bytes.  The adapter addr.
   *   where this info is stored at is in the p_a_parms variable.
   *
   */

	DEBUGTRACE2("GRIb", (ulong)p_dds);

	/* if there has been a fatal error
	 * return that we cannot get the ring information
	 */
	if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
		(p_dds->wrk.adap_state == OPEN_STATE) )
		return (ENETDOWN);


	tmpinfo.adap_phys_addr[0] = (sizeof(tok_ring_info_t));
	tmpinfo.adap_phys_addr[1] = p_dds->wrk.p_a_parms;

	rc = d_kmove (&tmpinfo,
			p_dds->wrk.p_d_ring_info,
			sizeof(tok_ring_info_t),
			p_dds->wrk.dma_chnl_id,
			p_dds->ddi.bus_id,
			DMA_WRITE_ONLY);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
	   bcopy (&tmpinfo,
		p_dds->wrk.p_ring_info,
		sizeof(tok_ring_info_t));

	/* give it to adapter 
	 */
	initiate_scb_command(p_dds, ADAP_READ_CMD,
                          p_dds->wrk.p_d_ring_info );



	DEBUGTRACE2("GRIe", (ulong)p_dds);
	return(0);
}  /* end function get_ring_info() */


/*
*  FUNCTION:   move_ring_info()
*
*          This function will move the Token-Ring Information (adapter
*          parameters) from the ACA to the Token-Ring Information
*          section of the DDS.
*
*          It assumes that the ACA has been set up and that the
*          bus address to put the Ring Information has
*          been calculated and is stored in the p_d_ring_info variable.
*
*
*  INPUT:      p_dds - pointer to DDS
*
*
*/
int move_ring_info(dds_t *p_dds)
{  /* begin function */

int rc;

       rc = d_kmove (&(p_dds->wrk.ring_info),
                     p_dds->wrk.p_d_ring_info,
                     sizeof(tok_ring_info_t),
                     p_dds->wrk.dma_chnl_id,
                     p_dds->ddi.bus_id,
                     DMA_READ);

	if (rc == EINVAL)
	{	/* IOCC is NOT buffered */
                 bcopy (p_dds->wrk.p_ring_info,
       		     &(p_dds->wrk.ring_info),
                     sizeof(tok_ring_info_t));
		rc = 0;
	}

return(rc);

}  /* end function move_ring_info() */

/*
 * NAME: async_status
 *
 * FUNCTION:
 *      This function is given an status block structure for input
 *      and then takes that status block and reports this status block
 *      as asynchronous status information for all currently attached users.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the OFLV.
 *
 * NOTES:
 *
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

void async_status( dds_t *p_dds,
                   struct status_block *p_sb)

{ /* begin function async_status */

   register int i;     /* index into the open element array */


for ( i=0; i < p_dds->cio.num_allocates; ++i)
{
   /*
   *   If this channel is open, post asynchronous status
   *   to it.
   */
   if ( p_dds->cio.chan_state[i] == CHAN_OPENED)
       report_status(p_dds, p_dds->cio.open_ptr[i], p_sb);
} /* end for loop */

return;
} /* end function async_status */

/*
*
* ROUTINE NAME: logerr
*
* DESCRIPTIVE NAME: Token-Ring error logging routine.
*
* FUNCTION: Collect information for making of error log entry.
*
* INPUT: DDS pointer - tells which adapter
*        Error id    - tells which error is being logged
*        Error number - tells which error code is returned
*
* OUTPUT: Error log entry made via errsave.
*
* RETURN:  N/A
*
* CALLED FROM:
*
* CALLS TO: bcopy, bzero, errsave
*/

void logerr( dds_t *p_dds,
              ulong  errid)
{
   tok_error_log_data_t    log;
   register int            i;             /* Loop counter */


   DEBUGTRACE4 ("logB", (ulong)p_dds, errid, 0); /* Start of error log entry*/

   /* Initialize the log entry to zero */
   bzero(&log,sizeof(tok_error_log_data_t));

   /* Store the error id in the log entry */
   log.errhead.error_id = errid;

   /* Load the device driver name into the log entry */
   strncpy(log.errhead.resource_name, p_dds->ddi.dev_name,
           (size_t)sizeof(log.errhead.resource_name));

   /*
   *   Start filling in the log with data
   */

   log.adap_state = p_dds->wrk.adap_state;
   log.limbo = p_dds->wrk.limbo;
   log.rr_entry = p_dds->wrk.rr_entry;
   log.limcycle = p_dds->wrk.limcycle;
   log.soft_chaos = p_dds->wrk.soft_chaos;
   log.hard_chaos = p_dds->wrk.hard_chaos;
   log.piox = p_dds->wrk.piox;
   log.ring_status = p_dds->wrk.ring_status;
   log.afoot = p_dds->wrk.afoot;
   log.footprint = p_dds->wrk.footprint;
   log.pio_attachment = p_dds->wrk.pio_attachment;
   log.pio_errors = p_dds->wrk.pio_errors;
   log.mcerr = p_dds->wrk.mcerr;
   log.ac_blk = p_dds->wrk.ac_blk;
   log.tx_cstat = p_dds->wrk.tx_cstat;
   log.rcv_cstat = p_dds->wrk.rcv_cstat;

   log.slot = p_dds->ddi.slot;


   /* Load POS data in the table */
   for (i=0; i<8; i++)
   {
      log.cfg_pos[i] = p_dds->wrk.cfg_pos[i];
   }

   /* Load Network address in use value into the table */
   for (i=0; i<TOK_NADR_LENGTH; i++)
   {
      log.tok_addr[i] = p_dds->wrk.tok_addr[i];
      log.tok_vpd_addr[i] = p_dds->wrk.tok_vpd_addr[i];
   }

   /* Load ROS level data value into the table */
   for (i=0; i<ROS_LEVEL_SIZE; i++)
   {
      log.tok_vpd_rosl[i] = p_dds->wrk.tok_vpd_rosl[i];
   }

   /* Load the microcode version number */
   for (i=0; i<MICROCODE_LEVEL_SIZE; i++)
   {
      log.tok_vpd_mclvl[i] = p_dds->wrk.tok_vpd_mclvl[i];
   }

       /*
       *   Put the Token-Ring Information into
       *   Error log structure
       */

   bcopy(  &(p_dds->wrk.ring_info),
           &(log.ring_info), sizeof(tok_ring_info_t) );


   /* log the error here */
   errsave (&log,sizeof(tok_error_log_data_t) );


   return;

}  /* end logerr */
