static char sccsid[] = "@(#)70  1.34.2.3  src/bos/kernext/entdiag/entds.c, diagddent, bos411, 9428A410j 1/13/94 16:29:37";
/*
 * COMPONENT_NAME: DIAGDDENT - Ethernet device handler
 *
 * FUNCTIONS: xxx_init,  xxx_initdds, xxx_offl,   xxx_intr,     xxx_act,
 *            xxx_inact, xxx_ioctl,   xxx_xmit,  xxx_newtbl,  entstartblk,
 *            xxx_badext, xxx_close, xxx_halt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <net/spl.h>
#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include "ent_comio_errids.h"
#include <sys/except.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/poll.h>
#include <sys/sleep.h>

#include <sys/timer.h>

#include <sys/types.h>
#include <sys/user.h>

#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <stddef.h>

#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include "entddi.h"
#include "cioddi.h"

#include "cioddhi.h"
#include "entdshi.h"
#include "ciodds.h"
#include "cioddlo.h"
#include "entdslo.h"

#ifdef KOFF
#include "../../sys/koff/db_trace.h"
#define TR_EN	0x80000000
#else
#define db_trace(flag, exp)
#endif

dds_t *DDSPTR;


/*****************************************************************************/
/*                                                                           */
/* NAME: ent_logerr                                                          */
/*                                                                           */
/* FUNCTION: Collect information for making of error log entry.              */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Error id    - tells which error is being logged                 */
/*           Error number - tells which error code is returned               */
/*           CMD - Adapter Command Register value  (if available)            */
/*           Status - Adapter Status Register value  (if available)          */
/*                                                                           */
/*    Output: Error log entry made via errsave.                              */
/*                                                                           */
/*    Called From: entxmit, entrecv, xxx_act, xxx_offl, entexecdque          */
/*                                                                           */
/*    Calls To: bcopy, bzero, errsave                                        */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void ent_logerr(
   dds_t *dds_ptr,            /* DDS pointer - tells which adapter           */
   ulong  errid,              /* Error id    - tells which error             */
   ulong  errnum,             /* Error number - tells which error code       */
   uchar  cmd,                /* CMD - Adapter Command Register value        */
   uchar  status)             /* Status - Adapter Status Register value      */
{
   struct  error_log_def   log;
   int                     i;             /* Loop counter                    */

   db_trace(TR_EN, (dds_ptr, "****ent: error: id %x num %x cmd %x status %x"
		    , errid, errnum, cmd, status));

   TRACE4("logB", (ulong)dds_ptr, errid, errnum); /* Start of error log entry*/

   /* Initialize the log entry to zero                                       */
   bzero(&log,sizeof(struct error_log_def));

   /* Store the error id in the log entry                                    */
   log.errhead.error_id = errid;

   /* Load the device driver name into the log entry                         */
   log.errhead.resource_name[0] = DDI.ds.lname[0];
   log.errhead.resource_name[1] = DDI.ds.lname[1];
   log.errhead.resource_name[2] = DDI.ds.lname[2];
   log.errhead.resource_name[3] = DDI.ds.lname[3];
   log.errhead.resource_name[4] = ' ';
   log.errhead.resource_name[5] = ' ';
   log.errhead.resource_name[6] = ' ';
   log.errhead.resource_name[7] = ' ';

   /* Start filling in the table with data                                   */

   /* Load the error return code into the table                              */
   log.errnum = errnum;

   /* Load the adapter command & status registers if applicable              */
   log.cmd = cmd;
   log.status = status;

   /* Load POS data in the table                                             */
   for (i=0; i<8; i++)
   {
      log.pos_reg[i] = WRK.pos_reg[i];
   }

   /* Load Network address in use value into the table                       */
   for (i=0; i<ent_NADR_LENGTH; i++)
   {
      log.ent_addr[i] = WRK.ent_addr[i];
      log.ent_vpd_addr[i] = WRK.ent_vpd_addr[i];
   }

   /* Load ROS level data value into the table                               */
   for (i=0; i<ROS_LEVEL_SIZE; i++)
   {
      log.ent_vpd_rosl[i] = WRK.ent_vpd_rosl[i];
   }
   log.ent_vpd_ros_length = WRK.ent_vpd_ros_length;

   /* Load the microcode version number                                      */
   log.version_num = WRK.version_num;

   /* Load last execute command value into the table                         */
   log.exec_cmd_in_progres = WRK.exec_cmd_in_progres;

   /* Load the state of the adapter                                          */
   log.adpt_start_state = WRK.adpt_start_state;

   /* Load the slot number of where the adapter is located                   */
   log.slot = DDI.ds.slot;

   /* Load the type field displacement values                                */
   log.type_field_off = DDI.ds.type_field_off;
   log.net_id_offset  = DDI.ds.net_id_offset;


   /* log the error here                                                     */
   errsave (&log,sizeof(struct error_log_def));

   TRACE1 ("logE"); /* End of error log entry                                */

   return;

}  /* end ent_logerr                                                         */

/*****************************************************************************/
/*
 * NAME pio_retry
 *
 * FUNCTION: This routine is called when a pio rotine returns an
 *	exception.  It will retry the the PIO and do error logging
 *
 * EXECUTION ENVIRONMENT:
 *	Called by interrupt and processes level
 *	This routine is invoked by the PIO_xxxX routines
 *
 * RETURNS:
 *	0 - excetion was retried successfully
 *	exception code of last failure if not successful
 */
/*****************************************************************************/

int
pio_retry(
	dds_t	*dds_ptr,	/* tells which adapter this came from	*/
	int 	excpt_code,	/* exception code from original PIO	*/
	enum pio_func iofunc,	/* io function to retry			*/
	void 	*ioaddr,	/* io address of the exception		*/
	long	ioparam)	/* parameter to PIO routine		*/

{
	int	retry_count;		/* retry count 	*/

	TRACE5("pior", (ulong)dds_ptr, (ulong)excpt_code, (ulong)iofunc,
			(ulong)ioaddr);

	retry_count = PIO_RETRY_COUNT;

	while(1)
	{
		/* trap if not an io exception
		 */
		assert(excpt_code == EXCEPT_IO);

		/* chech if out of retries, and do error logging
		 */
		if (retry_count <= 0)
		{
			ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR1,
				(ulong)EXCEPT_IO, (uchar)0, (uchar)0);
			return(excpt_code);
		}
		else
		{
			ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR2,
				(ulong)EXCEPT_IO, (uchar)0, (uchar)0);
			retry_count--;
		}

		/* retry the pio function, return if successful
		 */
		switch (iofunc)
		{
			case PUTC:
				excpt_code = BUS_PUTCX((char *)ioaddr,
							(char)ioparam);
				break;
			case PUTS:
				excpt_code = BUS_PUTSX((short *)ioaddr,
							(short)ioparam);
				break;
			case PUTSR:
				excpt_code = BUS_PUTSRX((short *)ioaddr,
							(short)ioparam);
				break;
			case PUTL:
				excpt_code = BUS_PUTLX((long *)ioaddr,
							(long)ioparam);
				break;
			case PUTLR:
				excpt_code = BUS_PUTLRX((long *)ioaddr,
							(long)ioparam);
				break;
			case GETC:
				excpt_code = BUS_GETCX((char *)ioaddr,
							(char *)ioparam);
				break;
			case GETS:
				excpt_code = BUS_GETSX((short *)ioaddr,
							(short *)ioparam);
				break;
			case GETSR:
				excpt_code = BUS_GETSRX((short *)ioaddr,
							(short *)ioparam);
				break;
			case GETL:
				excpt_code = BUS_GETLX((long *)ioaddr,
							(long *)ioparam);
				break;
			case GETLR:
				excpt_code = BUS_GETLRX((long *)ioaddr,
							(long *)ioparam);
				break;
			defalut:
				ASSERT(0);
		}

		if (excpt_code == 0)
			return(0);

	}

}

/*****************************************************************************/
/*
 * NAME: pio_getc
 *
 * FUNCTION: get a character for io space, with exception handeling
 *	This should only be called in non-performane-critical paths
 *
 * EXECUTION ENVIORNMENT:
 *	called from process and interrupt level
 *
 * RETURNS:
 *	character value if successful
 *	-1 on failure
 */
/*****************************************************************************/

int
pio_getc (
	dds_t	*dds_ptr,	/* tells which adapter	*/
	char	*ioaddr)	/* io address to read	*/
{
	char c;
	int rc;

	/* do pio
	 */
	rc = BUS_GETCX(ioaddr, &c);

	/* call retry routine if the PIO failed
	 */
	if (rc)
	{
		rc = pio_retry(dds_ptr, rc, GETC, ioaddr, (long)&c);
		if (rc)
			return(-1);
	}

	return(c);
}

/*****************************************************************************/
/*
 * NAME: pio_putc
 *
 * FUNCTION: write a character to IO space with excption handleing
 *	and retry.
 *
 * EXECUTION ENVIORNMENT:
 *	called from process and interrupt level
 *
 * RETURNS:
 *	0 if successful
 *	-1 on failure
 */
/*****************************************************************************/
int
pio_putc (
	dds_t	*dds_ptr,	/* tells which adapter	*/
	char	*ioaddr,	/* io address to write to */
	char	byte)		/* byte to write */
{
	int rc;

	rc = BUS_PUTCX(ioaddr, byte);

	/* call retry routine if the PIO failed
	 */
	if (rc)
	{
		rc = pio_retry(dds_ptr, rc,  PUTC, ioaddr, byte);
		if (rc)
			return(-1);
	}

	return(0);
}

/*****************************************************************************/
/*
 * NAME: pio_getsr
 *
 * FUNCTION: get a little endian short form io space, with exception handeling
 *	This should only be called in non-performane-critical paths
 *
 * EXECUTION ENVIORNMENT:
 *	called from process and interrupt level
 *
 * RETURNS:
 *	character value if successful
 *	-1 on failure
 */
/*****************************************************************************/

int
pio_getsr (
	dds_t	*dds_ptr,	/* tells which adapter	*/
	short	*ioaddr)	/* io address to read	*/
{
	short s;
	int rc;

	/* do pio
	 */
	rc = BUS_GETSRX(ioaddr, &s);

	/* call retry routine if the PIO failed
	 */
	if (rc)
	{
		rc = pio_retry(dds_ptr, rc, GETSR, ioaddr, (ulong)&s);
		if (rc)
			return(-1);
	}

	return(s);
}

/*****************************************************************************/
/*
 * NAME: pio_putsr
 *
 * FUNCTION: write a little-endian short to IO space with excption handleing
 *	and retry.
 *
 * EXECUTION ENVIORNMENT:
 *	called from process and interrupt level
 *
 * RETURNS:
 *	0 if successful
 *	-1 on failure
 */
/*****************************************************************************/

int
pio_putsr (
	dds_t	*dds_ptr,	/* tells which adapter	*/
	short	*ioaddr,	/* io address to write to */
	short	s)		/* byte to write */
{
	int rc;

	rc = BUS_PUTSRX(ioaddr, s);

	/* call retry routine if the PIO failed
	 */
	if (rc)
	{
		rc = pio_retry(dds_ptr, rc,  PUTSR, ioaddr, (ulong)s);
		if (rc)
			return(-1);
	}

	return(0);
}




/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_init                                                            */
/*                                                                           */
/* FUNCTION: Initialization for the device driver (once per load)            */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: N/A                                                             */
/*                                                                           */
/*    Output: N/A                                                            */
/*                                                                           */
/*    Called From: cioconfig                                                 */
/*                                                                           */
/*    Calls To: N/A                                                          */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void xxx_init ()

{
   TRACE1 ("iniB");  /* Init of Device Driver - Once per device Driver load  */
   return;

} /* end xxx_init                                                            */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_badext                                                          */
/*                                                                           */
/* FUNCTION: Detect invalid filename extension at ddmpx time.                */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: Channel Name - String of characters for the extention.          */
/*                                                                           */
/*    Output: N/A                                                            */
/*                                                                           */
/*    Called From: ciompx                                                    */
/*                                                                           */
/*    Calls To: N/A                                                          */
/*                                                                           */
/* RETURN:  0 = Channel extension is ok                                      */
/*          1 = Channel extension is not an 'E' or 'D' etc.                  */
/*                                                                           */
/*****************************************************************************/
int xxx_badext (
   register char *channame)
{
   register int rc;

   /* Trace the test for bad extension on mpx call                           */
   TRACE3("bexB",(ulong)channame,(ulong)((channame[0] << 8)|(channame[1])));

   rc=(((channame[0] != 'E') &&        /* Exclusive use in Normal R/W Mode   */
        (channame[0] != 'D')) ||       /* Exclusive use in Diagnostic Mode   */
        (channame[1] != '\0'));        /*                                    */

   TRACE2 ("bexE", (ulong)rc); /* end test for bad extension on MPX call     */

   return (rc);
} /* end xxx_badext                                                          */






/*****************************************************************************/
/*                                                                           */
/* NAME: entdsgetvpd                                                         */
/*                                                                           */
/* FUNCTION: Read and store the adapter's Vital Product Data via POS register*/
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Vital Product Data stored in the DDS work area along           */
/*            with the status of the VPD.                                    */
/*                                                                           */
/*    Called From: xxx_initdds                                               */
/*                                                                           */
/*    Calls To: cio_gen_crc                                                  */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
static void entdsgetvpd (
   dds_t *dds_ptr)     /* DDS pointer - tells which adapter                  */
{
   int     iocc;
   int     index;      /* Loop Counter                                       */
   int     index2;     /* Loop Counter                                       */
   int     vpd_found;  /* Vital Product Data header found flag               */
   int     na_found;   /* Network Address found flag                         */
   int     rl_found;   /* ROS Level found flag                               */
   int     pn_found;   /* Part Number found flag                             */
   int     ec_found;   /* EC Number found flag                               */
   int     dd_found;   /* EC Number found flag                               */
   int     crc_valid;  /* CRC valid flag                                     */
   ushort  cal_crc;    /* Calculated value of CRC                            */
   ushort  vpd_crc;    /* Actual VPD value of CRC                            */
   uchar   tmp_pos2;   /* Temporary byte for POS register 2                  */

   /* Get access to the IOCC to access POS registers                         */
   iocc = (int)IOCC_ATT((ulong)DDI.cc.bus_id,
                                       (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

   /* Test to verify this is correct adapter type                            */
   if ((PR0_VALUE != PIO_GETC( iocc + POS_REG_0 )) ||   /* POS 0 = 0xF5  */
       (PR1_VALUE != PIO_GETC( iocc + POS_REG_1 )))     /* POS 1 = 0x8E  */
   {

      /* Not the correct adapter type or SLOT # - Leave adapter/SLOT alone   */
      /* Update the Vital Product Data Status                                */
      VPD.status = VPD_NOT_READ;

   }
   else /* 3Com ethernet adapter detected - read Vital Product Data          */
   {
      /***********************************************************************/
      /* Turn off POS parity while reading VPD - No parity checking on VPD   */
      /***********************************************************************/

      /* Read pos register two - Save this value to restore later            */
      tmp_pos2 = PIO_GETC( iocc + POS_REG_2 );

      /* Write pos register two with the parity bit turned off               */
      PIO_PUTC( iocc + POS_REG_2,(tmp_pos2 & PR2_PEN_MSK) );

      /* Initialize POS Registers 6 & 7 to zero                              */
      PIO_PUTC( iocc + POS_REG_6,PR6_VALUE );
      PIO_PUTC( iocc + POS_REG_7,PR7_VALUE );

      /***********************************************************************/
      /* NOTE: "for" loop does not handle VPD greater than 256 bytes         */
      /***********************************************************************/

      /* Get VPD from adapter for the default length                         */
      for (index=0; index < ent_VPD_LENGTH; index++)
      {
         /* Set up the correct address for the VPD read byte                 */
         PIO_PUTC( iocc + POS_REG_6,(index + 1) );

         /* Read each byte upto 7 times to get non zero value to help        */
         /* compensate for an adapter card interal bus contention problem.   */
         for (index2=0; index2 < 7; index2++)
         {

            VPD.vpd[index] = PIO_GETC( iocc + POS_REG_3 );

            /* Test if non zero then exit for loop                           */
            if (VPD.vpd[index] != 0x00)
               break;
         } /* end for loop to keep reading each byte until non zero          */
      } /* end for loop to get all of the VPD data                           */

      /* Initialize POS Registers 6 & 7 to zero                              */
      PIO_PUTC( iocc + POS_REG_6,PR6_VALUE );
      PIO_PUTC( iocc + POS_REG_7,PR7_VALUE );

      /* Restore the original value for POS Register two                     */
      PIO_PUTC( iocc + POS_REG_2,tmp_pos2 );

      /* Test some of the Fields of Vital Product Data                       */
      if ((VPD.vpd[0] == 'V') &&                  /* 'V' = Hex 56            */
          (VPD.vpd[1] == 'P') &&                  /* 'P' = Hex 50            */
          (VPD.vpd[2] == 'D') &&                  /* 'D' = Hex 44            */
          (VPD.vpd[7] == '*'))                    /* '*' = Hex 2A            */
      {

         /* Update the Vital Product Data Status - Valid Data                */
         vpd_found = TRUE;
         na_found  = FALSE;
         rl_found  = FALSE;
         pn_found  = FALSE;
         ec_found  = FALSE;
         crc_valid = FALSE;

         /* Update the Vital Product Data length                             */
         VPD.length = ((2 * ((VPD.vpd[3] << 8) | VPD.vpd[4])) + 7);

         /* Test for which length will be saved - save the smaller           */
         if (VPD.length > ent_VPD_LENGTH)
         {
            VPD.length = ent_VPD_LENGTH;

            /* Mismatch on the length - can not test crc - assume crc is good*/
            crc_valid = TRUE;

         }
         else
         {

            /* Put together the CRC value from the adapter VPD               */
            vpd_crc = ((VPD.vpd[5] << 8) | VPD.vpd[6]);

            /* One can only verify CRC if one had enough space to save it all*/
            /* Verify that the CRC is valid                                  */
            cal_crc = cio_gen_crc(&VPD.vpd[7],(VPD.length - 7));

            /* Test if the CRC compares                                      */
	    /* Log and error if the CRC is not correct */
            if (vpd_crc == cal_crc)
               crc_valid = TRUE;
	    else
         	ent_logerr(dds_ptr, ERRID_ENT_ERR4, CIO_HARD_FAIL,
                                                           vpd_crc, cal_crc);
         }


         /* Get Network Address and ROS Level and Part Number & EC Number    */
         for (index=0; index < VPD.length; index++)
         {
            /*****************************************************************/
            /*    Get Network Address                                        */
            /*****************************************************************/
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'N') &&  /* 'N' = Hex 4E            */
                (VPD.vpd[(index + 2)] == 'A') &&  /* 'A' = Hex 41            */
                (VPD.vpd[(index + 3)] ==  5 ))    /*  5  = Hex 05            */
            {

               /* Set the Network Address found flag                         */
               na_found = TRUE;

               /* Save Network Address in DDS work section                   */
               for (index2 = 0; index2 < ent_NADR_LENGTH; index2++)
               {
                  /* Store the indexed byte in the DDS Work Section          */
                  WRK.ent_vpd_addr[index2] = VPD.vpd[(index + 4 + index2)];

               } /* end for loop for storing Network Address                 */
            } /* endif test for network address header                       */

            /*****************************************************************/
            /*    Get ROS Level                                              */
            /*****************************************************************/
            /* Test for the ROS Level Header                                 */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'R') &&  /* 'R' = Hex 52            */
                (VPD.vpd[(index + 2)] == 'L') &&  /* 'L' = Hex 4C            */
                (VPD.vpd[(index + 3)] ==  4 ))    /*  4 >= Hex 04  Now = 4   */
            {

               /* Set the ROS Level found flag                               */
               rl_found = TRUE;

               /* Set the actual number of ROS ascii bytes                   */
               WRK.ent_vpd_ros_length = ((VPD.vpd[(index + 3)] * 2) - 4);

               /* Save ROS Level in DDS work section                         */
               for (index2 = 0; index2 < ROS_LEVEL_SIZE; index2++)
               {
                  /* Store the indexed byte in the DDS Work Section          */
                  WRK.ent_vpd_rosl[index2] = VPD.vpd[(index + 4 + index2)];

               } /* end for loop for storing ROS level                       */

               /* Calculate the hex value of the level to compare w/firmware */
               WRK.ent_vpd_hex_rosl = 0;

               /* Convert the ASCII Decimal digits to hex                    */
               for (index2 = 0; index2 < WRK.ent_vpd_ros_length; index2++)
               {
                  /* Test for ascii decimal digits                           */
                  if ((WRK.ent_vpd_rosl[index2] >= '0') &&
                      (WRK.ent_vpd_rosl[index2] <= '9'))
                  {
                     /* Add in the next digit and shift the decimal point    */
                     WRK.ent_vpd_hex_rosl = ((10 * WRK.ent_vpd_hex_rosl) +
                                           (WRK.ent_vpd_rosl[index2] - '0'));
                  }
               }

            } /* endif test for ROS Level header                             */

            /*****************************************************************/
            /*    Get Part Number                                            */
            /*****************************************************************/
            /* Test for the Part Number Header                               */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'P') &&  /* 'P' = Hex 50            */
                (VPD.vpd[(index + 2)] == 'N') &&  /* 'N' = Hex 4E            */
                (VPD.vpd[(index + 3)] ==  6 ))    /*  3 <= L <= 8  now = 6   */
            {

               /* Set the Part Number found flag                             */
               pn_found = TRUE;

               /* Save Part Number in DDS work section                       */
               for (index2 = 0; index2 < PN_SIZE; index2++)
               {
                  /* Store the indexed byte in the DDS Work Section          */
                  WRK.ent_vpd_pn[index2] = VPD.vpd[(index + 4 + index2)];
               } /* end for loop for storing Part number                     */
            } /* endif test for Part Number header                           */

            /*****************************************************************/
            /*    Get Engineering Change Number                              */
            /*****************************************************************/
            /* Test for the Engineering Change Number Header                 */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'E') &&  /* 'E' = Hex 45            */
                (VPD.vpd[(index + 2)] == 'C') &&  /* 'C' = Hex 43            */
                (VPD.vpd[(index + 3)] ==  5 ))    /*  3 <= L <= 8  now = 5   */
            {

               /* Set the EC Number found flag                               */
               ec_found = TRUE;

               /* Save EC Number in DDS work section                         */
               for (index2 = 0; index2 < EC_SIZE; index2++)
               {
                  /* Store the indexed byte in the DDS Work Section          */
                  WRK.ent_vpd_ec[index2] = VPD.vpd[(index + 4 + index2)];
               } /* end for loop for storing Engineering Change number       */
            } /* endif test for EC Number header                             */

            /*****************************************************************/
            /*    Get Device Driver Level Number                             */
            /*****************************************************************/
            /* Test for the Device Driver Level Header                       */
            if ((VPD.vpd[(index + 0)] == '*') &&  /* '*' = Hex 2A            */
                (VPD.vpd[(index + 1)] == 'D') &&  /* 'D' = Hex 44            */
                (VPD.vpd[(index + 2)] == 'D') &&  /* 'D' = Hex 44            */
                (VPD.vpd[(index + 3)] ==  3 ))    /*  3 >= Hex 03   now = 3  */
            {

               /* Set the EC Number found flag                               */
               dd_found = TRUE;

               /* Save DD Number in DDS work section                         */
               for (index2 = 0; index2 < DD_SIZE; index2++)
               {
                  /* Store the indexed byte in the DDS Work Section          */
                  WRK.ent_vpd_dd[index2] = VPD.vpd[(index + 4 + index2)];
               } /* end for loop for storing Device Driver level             */

               /* Calculate the hex value of the DD level                    */
               WRK.ent_vpd_hex_dd = 0;

               /* Convert the ASCII Decimal digits to hex                    */
               for (index2 = 0; index2 < DD_SIZE; index2++)
               {
                  /* Test for ascii decimal digits                           */
                  if ((WRK.ent_vpd_dd[index2] >= '0') &&
                      (WRK.ent_vpd_dd[index2] <= '9'))
                  {
                     /* Add in the next digit and shift the decimal point    */
                     WRK.ent_vpd_hex_dd = ((10 * WRK.ent_vpd_hex_dd)
                                          + (WRK.ent_vpd_dd[index2] - '0'));
                  }
               } /* end for loop for converting ASCII Decimal to hex         */
            } /* endif test for DD Number header                             */
         } /* end for loop for getting VPD fields: NA RL PN EC DD            */

         /* Test the appropriate flags to verify everything is valid         */
         if ((vpd_found == TRUE) &&        /* VPD Header found               */
             (na_found  == TRUE) &&        /* Network Address found          */
             (rl_found  == TRUE) &&        /* ROS Level found                */
             (pn_found  == TRUE) &&        /* Part Number found              */
             (ec_found  == TRUE) &&        /* EC Number found                */
             (dd_found  == TRUE) &&        /* DD Number found                */
             (crc_valid == TRUE) &&        /* CRC value is valid             */
             /* Network address does not have the multicast bit on           */
             ((WRK.ent_vpd_addr[0] & MULTI_BIT_MASK) != MULTI_BIT_MASK))
         {

            /* VPD is valid based on the tests we know to check              */
            VPD.status = VPD_VALID;

         }
         else
         {
            /* VPD failed the test - set the status                          */
            VPD.status = VPD_INVALID;

         } /* endif for test if VPD is valid                                 */
      }
      else /* Bad Vital Product Data - Set VPD status                        */
      {

         /* Update the Vital Product Data Status                             */
         VPD.status = VPD_INVALID;

      } /* endif test of some of the VPD fields                              */
   } /* endif test to verify correct adapter type                            */

   /* restore IOCC to previous value - done accessing POS Regs               */
   IOCC_DET(iocc);            /* restore IOCC                                */

   /* Test if this adapter is the internal Prototype adapter for one of      */
   /* several hardware work arounds: POS Parity Enable, DMA Word boundary.   */
   /* Look specifically for PROT0ETH or 022F9381 or 071F0927 or 071f1152     */
   /* or 071F1182 or 071F1183                                                */

   /* Set the default card type value to unknown                             */
   WRK.card_type = ACT_UNKNOWN;

   /* Test if this VPD has the Prototype value                               */
   if ((WRK.ent_vpd_pn[0] == 'P') &&
       (WRK.ent_vpd_pn[1] == 'R') &&
       (WRK.ent_vpd_pn[2] == 'O') &&
       (WRK.ent_vpd_pn[3] == 'T') &&
       (WRK.ent_vpd_pn[4] == '0') &&
       (WRK.ent_vpd_pn[5] == 'E') &&
       (WRK.ent_vpd_pn[6] == 'T') &&
       (WRK.ent_vpd_pn[7] == 'H') &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the ethernet prototype adapter                      */
      WRK.card_type = ACT_10;
   }

   /* Test if this VPD has the 022F9381 value                                */
   if ((WRK.ent_vpd_pn[0] == '0') &&
       (WRK.ent_vpd_pn[1] == '2') &&
       (WRK.ent_vpd_pn[2] == '2') &&
       (WRK.ent_vpd_pn[3] == 'F') &&
       (WRK.ent_vpd_pn[4] == '9') &&
       (WRK.ent_vpd_pn[5] == '3') &&
       (WRK.ent_vpd_pn[6] == '8') &&
       (WRK.ent_vpd_pn[7] == '1') &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the original PS2 adapter for release 1              */
      WRK.card_type = ACT_10;
   }

   /* Test if this VPD has the 058F2881 value                                */
   if ((WRK.ent_vpd_pn[0] == '0') &&
       (WRK.ent_vpd_pn[1] == '5') &&
       (WRK.ent_vpd_pn[2] == '8') &&
       (WRK.ent_vpd_pn[3] == 'F') &&
       (WRK.ent_vpd_pn[4] == '2') &&
       (WRK.ent_vpd_pn[5] == '8') &&
       (WRK.ent_vpd_pn[6] == '8') &&
       (WRK.ent_vpd_pn[7] == '1') &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the ethernet prototype adapter                      */
      WRK.card_type = ACT_20;
   }

   /* Test if this VPD has the 071F0927 value                                */
   if ((WRK.ent_vpd_pn[0] == '0') &&
       (WRK.ent_vpd_pn[1] == '7') &&
       (WRK.ent_vpd_pn[2] == '1') &&
       (WRK.ent_vpd_pn[3] == 'F') &&
       (WRK.ent_vpd_pn[4] == '0') &&
       (WRK.ent_vpd_pn[5] == '9') &&
       (WRK.ent_vpd_pn[6] == '2') &&
       (WRK.ent_vpd_pn[7] == '7') &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the ethernet prototype adapter                      */
      WRK.card_type = ACT_22;
   }

   /* Test if this VPD has the 071F1152 value                                */
   if ((WRK.ent_vpd_pn[0] == '0') &&
       (WRK.ent_vpd_pn[1] == '7') &&
       (WRK.ent_vpd_pn[2] == '1') &&
       (WRK.ent_vpd_pn[3] == 'F') &&
       (WRK.ent_vpd_pn[4] == '1') &&
       (WRK.ent_vpd_pn[5] == '1') &&
       (WRK.ent_vpd_pn[6] == '5') &&
       (WRK.ent_vpd_pn[7] == '2') &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the ethernet PS2 with embedded fixes                */
      WRK.card_type = ACT_225;
   }

   /* Test if this VPD has the 071F1182 value                                */
   if ((WRK.ent_vpd_pn[0] == '0') &&
       (WRK.ent_vpd_pn[1] == '7') &&
       (WRK.ent_vpd_pn[2] == '1') &&
       (WRK.ent_vpd_pn[3] == 'F') &&
       (WRK.ent_vpd_pn[4] == '1') &&
       (WRK.ent_vpd_pn[5] == '1') &&
       (WRK.ent_vpd_pn[6] == '8') &&
       (WRK.ent_vpd_pn[7] == '2') &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the ethernet PS2 with embedded fixes                */
      WRK.card_type = ACT_23;
   }

   /* Test if this VPD has the 071F1183 value                                */
   if ((WRK.ent_vpd_pn[0] == '0') &&
       (WRK.ent_vpd_pn[1] == '7') &&
       (WRK.ent_vpd_pn[2] == '1') &&
       (WRK.ent_vpd_pn[3] == 'F') &&
       (WRK.ent_vpd_pn[4] == '1') &&
       (WRK.ent_vpd_pn[5] == '1') &&
       (WRK.ent_vpd_pn[6] == '8') &&
       (WRK.ent_vpd_pn[7] == '3') &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the ethernet PS2 with all fixes embedded            */
      WRK.card_type = ACT_235;
   }

   /* Test if this VPD has an unknown part number - ASSUME! version 3 card   */
   if ((WRK.card_type == ACT_UNKNOWN) &&
       (VPD.status == VPD_VALID))
   {
      /* This adapter is the follow on PS2 adapter for release 1+            */
      WRK.card_type = ACT_30;
   }

   return;
} /* end entdsgetvpd                                                         */

/*****************************************************************************/
/*                                                                           */
/* NAME: entdssetpos                                                         */
/*                                                                           */
/* FUNCTION: Take the information stored in the DDS and set the adapter POS  */
/*           registers to proper configure adapter.                          */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter/slot                          */
/*                                                                           */
/*    Output: POS registers for the selected adapter are initialized.        */
/*                                                                           */
/*    Called From: xxx_act entvpdfix                                         */
/*                                                                           */
/*    Calls To: N/A                                                          */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void entdssetpos(dds_t *dds_ptr)
   
{
            int    iocc;
   register int    pos_index;
   register uchar  temp;

   TRACE2 ("spsB", (ulong)dds_ptr);   /* entdssetpos begin                   */

   /**************************************************************************/
   /*  Update the DDS version of the POS based on configuration              */
   /**************************************************************************/

   /* Load the DDS Work section POS array based on known Adapter Defaults    */
   WRK.pos_reg[POS_REG_0] = PR0_VALUE;   /* Default Value =  Hex F5          */
   WRK.pos_reg[POS_REG_1] = PR1_VALUE;   /* Default Value =  Hex 8E          */
   WRK.pos_reg[POS_REG_6] = PR6_VALUE;   /* Default Value =  Hex 00          */
   WRK.pos_reg[POS_REG_7] = PR7_VALUE;   /* Default Value =  Hex 00          */

   /* Load the DDS Work section POS array, based on DDS Configuration        */
   /* Configure POS reg  2 - I/O Port, RAM Address, POS Parity & Card enable */
   temp =       PR2_CDEN_MSK;                        /* OR in card enable    */

   /* Test if POS Parity enable should be turned on                          */
   /**************************************************************************/
   /* The AT form factor card must not have POS Parity enable bit turned on  */
   /* due to a hardware failure. Leave disabled if unknown.                  */
   /**************************************************************************/
   if ((WRK.pos_parity)          &&      /* Configured to be turned on.      */
       (VPD.status == VPD_VALID))        /* Qualifies the proto_card value   */
   {
      temp |= PR2_PEN_MSK;               /* OR in POS Parity enable bit      */
   }

   /**************************************************************************/
   /* I/O Address = 7280, 7290, 7680, 7690, 7A80, 7A90, 7E80 & 7E90          */
   /* OR in I/O Resgister Offset                                             */
   /**************************************************************************/
   temp |= (((ulong)(DDI.ds.io_port) & 0x00000010) >> 3) |
            (((ulong)(DDI.ds.io_port) & 0x00000C00) >> 8);

   /**************************************************************************/
   /* RAM Address = C0000, C4000, C8000, CC000, D0000, D4000, D8000 & DC000  */
   /**************************************************************************/
   temp |= (((ulong)(DDI.ds.bus_mem_addr) & 0x0001C000) >> 10); /* OR offset */
   WRK.pos_reg[POS_REG_2] = temp;

   /* Configure POS reg 3 - DMA Level and Fairness                           */
   temp = DDI.ds.dma_arbit_lvl & PR3_DMA_MSK;        /* Assign DMA Level     */

   if (WRK.dma_fair) temp |= PR3_FAIR_MSK;           /* OR in DMA Fair enable*/
   WRK.pos_reg[POS_REG_3] = temp;

   /* Configure POS reg 4 - Start Enable & BNC/DIX Transceiver select        */
   temp = PR4_WDEN_MSK;                              /* Memory Enable        */
   if (DDI.ds.bnc_select) temp |= PR4_BNC_MSK;       /* Assign BNC select    */
   if ((WRK.card_type == ACT_30) && (WRK.adpt_parity_en == 1))
      temp |= PR4_PAREN_MSK;                         /* Adapter Parity Enable*/
   if ((WRK.card_type == ACT_30) && (WRK.fdbk_intr_en == 1))
      temp |= PR4_FDBK_MSK;                          /* Feedback INTR Enable */
   WRK.pos_reg[POS_REG_4] = temp;

   /* Configure POS reg 5 - Window Size, Interrupt level & ABM Mode          */
   /**************************************************************************/
   /* ABM Mode = 0x00 - disabled, 0x01 - 16, 0x02 - 32 & 0x03 - 64           */
   /**************************************************************************/
   temp = (WRK.dma_addr_burst & 0x00000003);     /* OR in ABM Mode           */

   /**************************************************************************/
   /* Interrupt Level = 0x00 - 9, 0x01 - 10, 0x02 - 11 & 0x03 - 12           */
   /**************************************************************************/
   temp |= (((DDI.cc.intr_level & 0x0000000F) - 9) << 2);      /* OR in Intr */

   /**************************************************************************/
   /* RAM Window Size = 0x00 - 16K, 0x01 - 32K, 0x02 - 64K & 0x03 - 128K     */
   /**************************************************************************/
   temp |= ((DDI.ds.bus_mem_size & 0x00000003) << 4);      /* OR in RAM size */
   /* Turn off Channel Check and save POS Reg 5                              */
   WRK.pos_reg[POS_REG_5] = temp | PR5_CHCK_MSK;

   /**************************************************************************/
   /*  Start writing the Data to the Adapter's POS Registers                 */
   /**************************************************************************/

   /* Get access to the IOCC to access POS registers                         */
   iocc = (int)IOCC_ATT((ulong)DDI.cc.bus_id,
                                       (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

   /* Test to verify this is correct adapter type                            */
   if ((PR0_VALUE != PIO_GETC( iocc + POS_REG_0 )) ||   /* POS 0 = 0xF5      */
       (PR1_VALUE != PIO_GETC( iocc + POS_REG_1 )))     /* POS 1 = 0x8E      */
   {

      /* Not the correct adapter type or SLOT # - Leave adapter/SLOT alone   */
   }
   else /* 3Com ethernet adapter detected - configure  POS  registers        */
   {

      /* Update POS registers 6 & 7 before POS register 3 is updated         */
      PIO_PUTC( iocc + POS_REG_6,WRK.pos_reg[POS_REG_6] );
      PIO_PUTC( iocc + POS_REG_7,WRK.pos_reg[POS_REG_7] );

      /* Update POS registers 2 to 5                                         */
      for (pos_index=POS_REG_2; pos_index <= POS_REG_5; pos_index++)
      {
         PIO_PUTC( iocc + pos_index,WRK.pos_reg[pos_index] );
      }

      /* Read the current values in POS Registers 0 to 7 and save them       */
      for (pos_index=POS_REG_0; pos_index <= POS_REG_7; pos_index++)
      {
         WRK.pos_reg[pos_index]= PIO_GETC(iocc + pos_index);

      }

   } /* endif test to verify correct adapter type                            */

   /* Perform dummy read to allow adapter gate array problem to subside      */
   WRK.gate_array_fix = PIO_GETC( (caddr_t)((ulong)iocc | (ulong)0x000f0000));

   /* restore IOCC to previous value - done accessing POS Regs               */
   IOCC_DET(iocc);                /* restore IOCC                            */

   TRACE1 ("spsE");               /* entdssetpos end                         */

   return;

} /* end entdssetpos                                                         */

/*****************************************************************************/
/*                                                                           */
/* NAME: entgetconfig                                                        */
/*                                                                           */
/* FUNCTION: Get and store adapter internal configuration after "Report      */
/*           Configuration" is complete and store in the DDS.                */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Adapter internal configuration stored in DDS.                  */
/*                                                                           */
/*    Called From: xxx_offl                                                  */
/*                 entexecdque                                               */
/*                                                                           */
/*    Calls To: N/A                                                          */
/*                                                                           */
/* RETURN:  0 = OK                                                           */
/*         -1 = execute mailbox address check failed.                        */
/*         -2 = Adapter RAM size range check failed.                         */
/*                                                                           */
/*****************************************************************************/
static int entgetconfig (
   register dds_t  *dds_ptr,
   register int    bus)

{
   uint   temp_off;                /* Temp variable, execute mail box offset */
   int    rc;                      /* Temporary Return Code                  */

   TRACE2 ("cfgB", (ulong)dds_ptr);   /* entgetconfig begin                  */

   rc = 0;

   /**************************************************************************/
   /* Assumption: the caller has gotten bus access & Interrupts disabled     */
   /*             the caller has verified that report config is complete     */
   /**************************************************************************/

   /* Get & store this information in the work section                       */

   /* Get from the adapter the Base/main offset - allow for byte swapping    */
   WRK.main_offset =(PIO_GETC(bus + WRK.exec_mail_box + 2))       | /* Least */
                    (PIO_GETC(bus + WRK.exec_mail_box + 3) <<  8) | /* to    */
                    (PIO_GETC(bus + WRK.exec_mail_box + 4) << 16) | /* Most  */
                    (PIO_GETC(bus + WRK.exec_mail_box + 5) << 24);  /* Signif*/

   /* Get from the adapter the Receive Mail Box - allow for byte swapping    */
   WRK.recv_mail_box = (PIO_GETC(bus + WRK.exec_mail_box + 6)) |    /* LSB   */
                       (PIO_GETC(bus + WRK.exec_mail_box + 7) <<  8); /* MSB */

   /* Add in the main offset to get the final offset                         */
   WRK.recv_mail_box += WRK.main_offset;

   /* Get from the adapter the Transmit Mail Box - allow for byte swapping   */
   WRK.xmit_mail_box = (PIO_GETC(bus + WRK.exec_mail_box + 8)) |     /* LSB  */
                       (PIO_GETC(bus + WRK.exec_mail_box + 9) <<  8);/* MSB  */

   /* Add in the main offset to get the final offset                         */
   WRK.xmit_mail_box += WRK.main_offset;

   /* Get from the adapter the execute  Mail Box - allow for byte swapping   */
   temp_off = (PIO_GETC(bus + WRK.exec_mail_box + 10)) |       /* LSB        */
              (PIO_GETC(bus + WRK.exec_mail_box + 11) <<  8);  /* MSB        */

   /* Add in the main offset to get the final offset                         */
   temp_off += WRK.main_offset;

   /* Test to verify that the adapter gave us the right data                 */
   if (temp_off != WRK.exec_mail_box)
      rc = -1;

   /* Get the Statistics Counters Offset - allow for byte swapping           */
   WRK.stat_count_off = (PIO_GETC(bus + WRK.exec_mail_box + 12)) |     /* LSB*/
                        (PIO_GETC(bus + WRK.exec_mail_box + 13) <<  8);/* MSB*/

   /* Add in the main offset to get the final offset                         */
   WRK.stat_count_off += WRK.main_offset;

   /* Get the Total adapter RAM size     - allow for byte swapping           */
   WRK.adpt_ram_size = (PIO_GETC(bus + WRK.exec_mail_box + 14)) |     /* LSB */
                       (PIO_GETC(bus + WRK.exec_mail_box + 15) <<  8);/* MSB */

   /* Test that the adapter thinks it has some functional RAM                */
   if ((WRK.adpt_ram_size = 0) ||
       (WRK.adpt_ram_size > RAM_SIZE))
      rc = -2;

   /* Get the Buffer Descriptor Region size - allow for byte swapping        */
   WRK.buf_des_reg_size = (PIO_GETC(bus + WRK.exec_mail_box + 16)) |  /* LSB */
                       (PIO_GETC(bus + WRK.exec_mail_box + 17) <<  8);/* MSB */

   /* Get the Transmit List Start Offset    - allow for byte swapping        */
   WRK.xmit_list_off = (PIO_GETC(bus + WRK.exec_mail_box + 18)) |     /* LSB */
                       (PIO_GETC(bus + WRK.exec_mail_box + 19) <<  8);/* MSB */

   /* Add in the main offset to get the final offset                         */
   WRK.xmit_list_off += WRK.main_offset;

   /* Get the Transmit List count           - allow for byte swapping        */
   WRK.xmit_list_cnt = (PIO_GETC(bus + WRK.exec_mail_box + 20)) |     /* LSB */
                       (PIO_GETC(bus + WRK.exec_mail_box + 21) <<  8);/* MSB */

   /* Get the receive  List Start Offset    - allow for byte swapping        */
   WRK.recv_list_off = (PIO_GETC(bus + WRK.exec_mail_box + 22)) |     /* LSB */
                       (PIO_GETC(bus + WRK.exec_mail_box + 23) <<  8);/* MSB */

   /* Add in the main offset to get the final offset                         */
   WRK.recv_list_off += WRK.main_offset;

   /* Get the Receive  List count           - allow for byte swapping        */
   WRK.recv_list_cnt = (PIO_GETC(bus + WRK.exec_mail_box + 24)) |     /* LSB */
                       (PIO_GETC(bus + WRK.exec_mail_box + 25) <<  8);/* MSB */

   /* Get the Firmware Version number       - allow for byte swapping        */
   WRK.version_num   = (PIO_GETC(bus + WRK.exec_mail_box + 26)) |     /* LSB */
                       (PIO_GETC(bus + WRK.exec_mail_box + 27) <<  8);/* MSB */

   TRACE2 ("cfgE", (ulong)rc);       /* entgetconfig end                     */

   return (rc);

} /* end entgetconfig                                                        */

/*****************************************************************************/
/*                                                                           */
/* NAME: entvpdfix                                                           */
/*                                                                           */
/* FUNCTION: Work around adapter timing problem inorder to get valid VPD     */
/*           results.                                                        */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Sequence: 1) Set POS for a valid adapter configuration.                */
/*              2) Force Hard reset to adapter.                              */
/*              3) Loop read of command reg. to get execute mailbox address. */
/*              4) Read and store adapter configuration.                     */
/*              5) Verify firmware version equals 8. If not close down & exit*/
/*              6) Issue special command to try up on card processor.        */
/*              7) Read the stable version of VPD.                           */
/*              8) Free up special command.                                  */
/*              9) Disable adapter.                                          */
/*             10) Return.                                                   */
/*                                                                           */
/*    Input: DDS Pointer - Tells which adapter to run on.                    */
/*                                                                           */
/*    Output: Valid VPD data.                                                */
/*                                                                           */
/*    Called From: xxx_initdds                                               */
/*                                                                           */
/*    Calls To: entdsgetvpd entdssetpos entgetconfig                         */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void entvpdfix (
   register dds_t *dds_ptr)     /* DDS pointer - tells which adapter         */

{
   register uchar host_status_reg;
   register uchar host_command_reg;
   int    bus, ioa;
   int    index;       /* Loop Counter                                       */
   int    index2;      /* Loop Counter                                       */
   int    saved_intr_level;


   TRACE2 ("vpfB", (ulong)dds_ptr);  /* Start of VPD fix routine             */

   DISABLE_INTERRUPTS (saved_intr_level);

   /* Set the adapter's POS registers so driver can access the adapter       */
   entdssetpos(dds_ptr);

   /* Get access to the I/O bus to access I/O registers                      */
   bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );
   ioa = (int)BUSIO_ATT(  (ulong)DDI.cc.bus_id, DDI.ds.io_port );

   /* Clear any possible pending parity error interrupt */
   PIO_PUTC( ioa + STATUS_REG, 0 );

   ENABLE_INTERRUPTS (saved_intr_level);

   /* Hard Reset the adapter to force a known state                          */
   PIO_PUTC( ioa + CONTROL_REG,HARD_RST_MSK );

   /* Wait for the adapter to know that the hard reset has been set          */
   DELAYMS(10);                       /* Delay 10 milliseconds               */

   /* Start the hard reset in the adapter                                    */
   PIO_PUTC( ioa + CONTROL_REG,CONTROL_VALUE );

   /* Wait for the adapter to fully reset                                    */
   DELAYMS(1000);                     /* Delay 1 second                      */

   /* Set up for stage one of the adapter interrupts                         */
   /* Loop for 1 second waiting for hex 00 to be return                      */
   for (index2 = 0; index2 < 1000; DELAYMS(1), index2--)
   {
      /* get the interrupt status register for this card                     */
      host_status_reg = PIO_GETC( ioa + STATUS_REG );

      /* Test if command word received interrupt has occurred                */
      if (host_status_reg & CWR_MSK)
      {

         /* get the interrupt status register for this card                  */
         host_command_reg = PIO_GETC( ioa + COMMAND_REG );

         /* Test if the host command register read zero                      */
         if (host_command_reg == 0x00)
            break; /* Exit the while loop                                    */
      }
   } /* end while (loop for 1 second)                                        */

   /* Test if the host command register read zero                            */
   if (host_command_reg != 0x00)
   {
      /* restore IO bus to previous value, done accessing I/O Regs           */
      BUSIO_DET(ioa);
      BUSMEM_DET(bus);

      TRACE2 ("vpfE", (ulong)host_command_reg); /* End of VPD fix routine    */

      return;
   }

   /* Set up for stage two of the adapter interrupts                         */
   /* Loop for 1 second per byte waiting for execute mailbox to be return    */
   WRK.exec_mail_box = 0;
   for (index=0; index < 4; index++)
   {
      /* Loop for 1 second waiting for execute command to be run             */
      for (index2 = 0; index2 < 1000; DELAYMS(1), index2--)
      {
         /* get the interrupt status register for this card                  */
         host_status_reg = PIO_GETC( ioa + STATUS_REG );

         /* Test if command word received interrupt has occurred             */
         if (host_status_reg & CWR_MSK)
         {

            /* get the interrupt status register for this card               */
            host_command_reg = PIO_GETC( ioa + COMMAND_REG );

            /* Place the address byte in the proper location                 */
            WRK.exec_mail_box |= (host_command_reg << (index * 8));

            break; /* Exit the while loop                                    */

         } /* end if test if command wird received is active                 */
      } /* end while not a full second                                       */
   } /* end for loop for gathering execute mail box                          */

   TRACE2 ("vpfM", (ulong)WRK.exec_mail_box); /* EMB address from VPD fix    */

   /* Read and store the adapter configuration data                          */
   if (entgetconfig(dds_ptr, bus) != 0x00)
   {
      /* restore IO bus to previous value, done accessing I/O Regs           */
      BUSIO_DET(ioa);
      BUSMEM_DET(bus);

      TRACE2 ("vpfE", (ulong) 0xF0); /* End of VPD fix - get config failed   */

      return;
   }

   /* Test if the version has the special execute command                    */
   if (WRK.version_num < 0x0008)
   {
      /* restore IO bus to previous value, done accessing I/O Regs           */
      BUSIO_DET(ioa);
      BUSMEM_DET(bus);

      TRACE2 ("vpfE", (ulong) 0xF1); /* End of VPD fix - not version 8       */

      return;
   }

   /* Set up the special execute mailbox  command                            */
   /* Load the execute mailbox with the command to be executed               */
   PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0xC3 );
   PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 );
                                                /* Load Command Data         */
   /* Turn all bits off - the processor will turn then on                    */
   PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x00 );
   PIO_PUTC( bus + WRK.exec_mail_box + 0x03, 0x00 );

   /* Issue the execute command to the command register                      */
   PIO_PUTC( ioa + COMMAND_REG,EXECUTE_MSK );

   /* Loop for 1 second waiting for execute command to be run                */
   for (index2 = 0; index2 < 1000; DELAYMS(1), index2--)
   {
      /* Test if the 2nd word changed from 00 to FF                          */
      if ((PIO_GETC( bus + WRK.exec_mail_box + 2 ) == 0xFF) &&
          (PIO_GETC( bus + WRK.exec_mail_box + 3 ) == 0xFF))
         break;

   } /* end while (loop for 1 second)                                        */

   /* restore IO bus to previous value, done accessing I/O Regs              */
   BUSIO_DET(ioa);
   BUSMEM_DET(bus);

   /* Test if the execute command actually started working                   */
   if (index2 > 998)
   {
      TRACE2 ("vpfE", (ulong) 0xF2); /* End of VPD fix - Exec cmd failed     */

      return;
   }

   /* Read the Vital Product Data for this Adapter                           */
   entdsgetvpd(dds_ptr);

   /* Get access to the I/O bus to access I/O registers                      */
   bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );

   /* Turn all bits off - the processor will now exit loop                   */
   PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x00 );
   PIO_PUTC( bus + WRK.exec_mail_box + 0x03, 0x00 );

   /* restore IO bus to previous value, done accessing I/O Regs              */
   BUSMEM_DET(bus);

   TRACE1 ("vpfE"); /* End of VPD fix - finished ok                          */

   return;

} /* end entvpdfix                                                           */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_initdds                                                         */
/*                                                                           */
/* FUNCTION: Perform any device-specific initialization of the dds.          */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: An updated dds, the Vital Product data read and the            */
/*            network address selected.                                      */
/*                                                                           */
/*    Called From: config_init                                               */
/*                                                                           */
/*    Calls To: entdsgetvpd, entvpdfix                                       */
/*                                                                           */
/* RETURN:  0 = Good Return                                                  */
/*          EINVAL = Unable to get enough TCWs to start adapter list         */
/*                                                                           */
/*****************************************************************************/
int xxx_initdds (
   register dds_t *dds_ptr)     /* DDS pointer - tells which adapter         */

{
   int    iocc;
   int    index;       /* Loop variable for "for" loop                       */
   uchar  PR2_VALUE;   /* POS register two value                             */
   ushort temp_short1; /* Temporary 2 byte field                             */

   TRACE2 ("ddsB", (ulong)dds_ptr);  /* Initialize DDS begin                 */

   DDSPTR = dds_ptr;

   WRK.dma_fair          = 1;   /* DMA Fairness - POS 3 Bit 4                */
   WRK.pos_parity        = 1;   /* POS Parity enable - POS 2 Bit 7           */
   WRK.bus_parity        = 1;   /* Bus Parity enable - REG A Bit 0           */
   WRK.dma_addr_burst    = 0;   /* DMA Address Burst Management-POS 5 bit 0&1*/

   /* intialize the first flag counter */
   WRK.first_flag	 = 0; 
   WRK.mbox_status	 = NULL;
   /* Set the version 3 adapter card values                                  */
   WRK.adpt_parity_en    = 1;   /* Adapter Parity enable - POS 4 bit 4       */
   WRK.fdbk_intr_en      = 1;   /* Select feedback intr enable - POS 4 bit 4 */

   WRK.timr_priority     = INTCLASS2;
   WRK.offl_level        = INT_OFFL1;
   WRK.offl_priority     = INTOFFL1;
   WRK.min_packet_len    = ent_MIN_PACKET;    /* Minimum valid Packet Length */
   WRK.max_packet_len    = ent_MAX_PACKET;    /* Maximum valid Packet Length */
   WRK.channel_alocd     = FALSE;
   WRK.dma_locked        = FALSE;

   /* Update the state machine to indicate the adpt is not started           */
   WRK.adpt_start_state  = NOT_STARTED;
   WRK.adpt_state        = normal;

   /* copy the bus memory address and bus id, this is to localize memeory
    * access in interrupt handler
    */
   WRK.bus_id = DDI.cc.bus_id;
   WRK.bus_mem_addr = DDI.ds.bus_mem_addr;
   WRK.io_port = DDI.ds.io_port;

   /* Read the Vital Product Data for this Adapter                           */
   entdsgetvpd(dds_ptr);

   /* Test if the vital product data actually read ok                        */
   if (VPD.status != VPD_VALID)
   {
      /* Run the VPD fix routine to see if this helps                        */
      entvpdfix(dds_ptr);

      /* finish up this fix by turning off card enable                       */
      /* Get access to the IOCC to access POS registers                      */
      iocc = (int)IOCC_ATT((ulong)DDI.cc.bus_id,
                                       (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

      /* Read POS register 2                                                 */
      PR2_VALUE = PIO_GETC( iocc + POS_REG_2 );

      /* Disable POS parity                                                  */
      PR2_VALUE &= (~PR2_PEN_MSK);

      /* Disable Card Enable                                                 */
      PR2_VALUE &= (~PR2_CDEN_MSK);

      /* Turn off the card enable bit in POS register 2 & write the register */
      PIO_PUTC( iocc + POS_REG_2,(PR2_VALUE) );

      /* restore IOCC to previous value - done accessing POS Regs            */
      IOCC_DET(iocc);                /* restore IOCC                         */
   }

   /* Assign the configured value to the local working variable              */
   WRK.rdto = DDI.cc.rdto;

   /* Set up the DMA base address based on configured value                  */
   WRK.dma_base = (uint)DDI.ds.tcw_bus_mem_addr;

   /* Calculate the number of TCWs we can use based on 4k buffer size        */
   temp_short1 = (DDI.ds.tcw_bus_mem_size >> DMA_L2PSIZE);

   /* Temporary work around for release 1 - Use only half the allocated DMA  */
   /* space. This will inturn only allocate half the receive buffers which is*/
   /* critical for an 8 megabyte entry machine.                              */
   temp_short1 = ((DDI.ds.tcw_bus_mem_size >> DMA_L2PSIZE) / 2);

   /* Set up for how many transmit TCW's - Take half                         */
   WRK.xmit_tcw_cnt = (temp_short1 / 2);

   /* Set up for how many receive TCW's - take what is left                  */
   WRK.recv_tcw_cnt = temp_short1 - WRK.xmit_tcw_cnt;

   /* Force the receive TCW count to be even, so that full pages are used in
    * the receive buffers
    */
   if (WRK.recv_tcw_cnt % 2)
	WRK.recv_tcw_cnt--;

   /* Setup Receive TCW Region managememt table base                         */
   WRK.Recv_Tcw_Base = WRK.dma_base;

   /* Setup Transmit TCW Region managememt table base - end of receive base  */
   WRK.tx_tcw_base = WRK.Recv_Tcw_Base + (WRK.recv_tcw_cnt << DMA_L2PSIZE);

   /* Test the range of the Transmit TCW list                                */
   if (WRK.xmit_tcw_cnt > MAX_TX_TCW)
      WRK.xmit_tcw_cnt = MAX_TX_TCW;

   /* Test the range of the Receive TCW list                                 */
   if (WRK.recv_tcw_cnt > MAX_RX_TCW)
      WRK.recv_tcw_cnt = MAX_RX_TCW;

   /* Test the range for minimum value that the adapter needs                */
   if ((WRK.xmit_tcw_cnt < MIN_DES_LIST) ||
       (WRK.recv_tcw_cnt < MIN_DES_LIST))
      return(EINVAL);

   /* Test for the Prototype adapter                                         */
   if ((WRK.card_type == ACT_10) &&
       (VPD.status == VPD_VALID))
   { /* This is the internal prototype adptr - ABM = 64                      */

      WRK.dma_addr_burst  = 3;  /* DMA Address Burst Management-POS 5 bit 0&1*/

      /* POS parity enable is disabled due to hardware problems              */
      WRK.pos_parity      = 0;

      /* Fairness disabled with AT Form Factor cards                         */
      WRK.dma_fair        = 0;

      /* Test if the mbuf data offset is on a full word boundary             */
      if ((DDI.cc.rdto & 0x0003) != 0x0000)
      { /* Then not on word boundary and prototype adapter - put on boundary */

         WRK.rdto = ((DDI.cc.rdto & 0xFFFC) + 0x0004);
      }
   } /* endif test for prototype adapter                                     */

   /* Test for the Original PS2 Form Factor card                             */
   if ((WRK.card_type == ACT_20) &&
       (VPD.status == VPD_VALID))
   { /* This is the released adapter                                         */

      /* Required for first version of the PS/2 form factor card             */
      WRK.dma_addr_burst  = 1;  /* DMA Address Burst Management-POS 5 bit 0&1*/

      /* POS parity enable is disabled due to hardware problems              */
      WRK.pos_parity      = 0;

      /* Fairness enabled with updated PS2 FF cards                          */
      WRK.dma_fair        = 1;

      /* Test if the mbuf data offset is on a full word boundary             */
      if ((DDI.cc.rdto & 0x000F) != 0x0000)
      { /* Then not on "16 bit" boundary and PS2 adapter - put on boundary   */

         WRK.rdto = ((DDI.cc.rdto & 0xFFF0) + 0x0010);
      }
   } /* endif test for original PS2 form factor card                         */

   /* Test for the PS2 Form Factor card                                      */
   if ((WRK.card_type == ACT_22) &&
       (VPD.status == VPD_VALID))
   { /* This is the released adapter                                         */

      /* Required for first version of the PS/2 form factor card             */
      WRK.dma_addr_burst  = 1;  /* DMA Address Burst Management-POS 5 bit 0&1*/

      /* POS parity enable is disabled due to hardware problems              */
      WRK.pos_parity      = 0;

      /* Fairness enabled with updated PS2 FF cards                          */
      WRK.dma_fair        = 1;

      /* Test if the mbuf data offset is on a full word boundary             */
      if ((DDI.cc.rdto & 0x000F) != 0x0000)
      { /* Then not on "16 bit" boundary and PS2 adapter - put on boundary   */

         WRK.rdto = ((DDI.cc.rdto & 0xFFF0) + 0x0010);
      }
   } /* endif test for PS2 form factor card                                  */

   /* Test for the PS2 Form Factor card - parity update                      */
   if (((WRK.card_type == ACT_225)  ||
        (WRK.card_type == ACT_23)   ||
        (WRK.card_type == ACT_235)) &&
        (VPD.status    == VPD_VALID))
   { /* This is one of the released adapters                                 */

      /* Required for first version of the PS/2 form factor card             */
      WRK.dma_addr_burst  = 1;  /* DMA Address Burst Management-POS 5 bit 0&1*/

      /* POS parity enable is disabled due to hardware problems              */
      WRK.pos_parity      = 1;

      /* Fairness enabled with updated PS2 FF cards                          */
      WRK.dma_fair        = 1;

      /* Test if the mbuf data offset is on a full word boundary             */
      if ((DDI.cc.rdto & 0x000F) != 0x0000)
      { /* Then not on "16 bit" boundary and PS2 adapter - put on boundary   */

         WRK.rdto = ((DDI.cc.rdto & 0xFFF0) + 0x0010);
      }
   } /* endif test for PS2 form factor card                                  */

   /* Determine which Network Address to use, either DDS or VPD version      */
   if ((DDI.ds.use_alt_addr == 0) &&
       (VPD.status == VPD_VALID))
   { /* Use the network address that was passed in the VPD                   */

      /* Set up for loop to copy the network address                         */
      for ( index=0 ; index < ent_NADR_LENGTH ; index++)
      {
         WRK.ent_addr[index] = WRK.ent_vpd_addr[index];
      }
   }
   else
   { /* Use the network address that was passed in the DDS                   */

      /* Set up for loop to copy the network address                         */
      for ( index=0 ; index < ent_NADR_LENGTH ; index++)
      {
         WRK.ent_addr[index] = DDI.ds.alt_addr[index];
      }
   } /* endif test for which network address to use                          */

   TRACE1 ("ddsE");                  /* Initialize DDS end                   */

   /*
    * return failure if adapter state is not normal
    */
   return WRK.adpt_state == normal ? 0 : !0;
} /* end xxx_initdds routine                                                 */

/*****************************************************************************/
/*                                                                           */
/* NAME: entrdcount                                                          */
/*                                                                           */
/* FUNCTION: Read selected adapter internal counter.                         */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs under both the process thread and interrupt thread.*/
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter.                              */
/*           Counter offset - tells where in the adptr RAM to start reading. */
/*                                                                           */
/*    Output: Counter value return as a return parameter.                    */
/*                                                                           */
/*    Called From: entrdcounts                                               */
/*                                                                           */
/*    Calls To: N/A                                                          */
/*                                                                           */
/* RETURN:  The value of the counter.                                        */
/*                                                                           */
/*****************************************************************************/
static ulong entrdcount (
   register dds_t *dds_ptr,     /* DDS pointer - tells which adapter         */
   int      bus,                /* Attached memory address                   */
   ulong    count_off)          /* The offset in RAM where the counter is    */
{
   ulong counter_value;         /* Final value of the counter                */
   uchar local_byte;            /* Local value of the byte read              */
   int   i;                     /* Index counter                             */

   if (WRK.adpt_state != normal)
       return 0;

   counter_value = 0;           /* Intialize the counter value               */

   /* Read the selected 4 bytes and combine into a word in the proper order  */
   for (i = 0; i < 4; i++)
   {
      /* Read from LSB to MSB*/
      local_byte = PIO_GETC(bus + count_off + 3 - i);
      counter_value = ((counter_value << 8) | local_byte);
   }

   return(counter_value);
} /* end entrdcount subroutine                                               */

/*****************************************************************************/
/*                                                                           */
/* NAME: entrdcounts                                                         */
/*                                                                           */
/* FUNCTION: Read adapter internal counters, store these values in with the  */
/*           RAS counters and reset the internal counters.                   */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs under both the process thread and interrupt thread.*/
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Counter values are transfered to the RAS counters and the      */
/*            adapter counters are reset to zero.                            */
/*                                                                           */
/*    Called From: xxx_offl                                                  */
/*                                                                           */
/*    Calls To: entrdcount                                                   */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void entrdcounts (
   register dds_t *dds_ptr)     /* DDS pointer - tells which adapter         */
{
   int    bus;
   int    i;                    /* Loop Counter                              */
   int    j;                    /* Loop Counter                              */
   uchar  local_byte_1;         /* Local character only                      */
   uchar  local_byte_2;         /* Local character only                      */

   TRACE2 ("rdcB",(ulong)dds_ptr); /* Start of reading adapter counters      */

   /* Test if the adapter is in diagnostic mode                              */
   if  (CIO.mode != 'D')
   { /* Normal operating mode - read adapter counters                        */

      /* Get access to the I/O bus to access I/O registers                   */
      bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );

      /* Read and store four bytes of data from adapter for each counter     */
      RAS.ds.crc_error     += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off +  0));
      RAS.ds.align_error   += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off +  4));
      RAS.ds.overrun       += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off +  8));
      RAS.ds.too_short     += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 12));
      RAS.ds.too_long      += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 16));
      RAS.ds.no_resources  += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 20));
      RAS.ds.pckts_discard += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 24));
      RAS.ds.max_collision += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 28));
      RAS.ds.carrier_lost  += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 32));
      RAS.ds.underrun      += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 36));
      RAS.ds.cts_lost      += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 40));
      RAS.ds.xmit_timeouts += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 44));

      /* Set up the number of bytes to zero after reading the counters       */
      j = 48;

      /* Test if the ROS on card supports extended counters                  */
      if (WRK.ent_vpd_hex_rosl >= 0x08)
      {
         RAS.ds.host_rec_eol  += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 48));
         RAS.ds.adpt_rec_eol  += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 52));
         RAS.ds.adpt_rec_pack += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 56));
         RAS.ds.host_rec_pack += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 60));
         RAS.ds.start_recp_cmd += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 64));
	 if (WRK.first_flag >= 1)
		RAS.ds.start_recp_cmd = RAS.ds.start_recp_cmd - 0x01;

         RAS.ds.rec_dma_to    += entrdcount(dds_ptr,bus,
                                                    (WRK.stat_count_off + 68));

         /* Set up the number of bytes to zero after reading the counters    */
         j = 60;
      }

      
      /* Reset the Read/Write adapter counters to zero except the Start 
       * reception command. Reset start reception counter to 1.
       */

      for (i = 0; i < j; i++)
      {
            PIO_PUTC( bus + WRK.stat_count_off + i, 0x00 );
      }

      /* DO NOT zero out the first byte of the start recp cmd counter */
      PIO_PUTC( bus + WRK.stat_count_off + 64, 0x01);
      j = 72;
      for (i = 65; i < j; i++)
      {
            PIO_PUTC( bus + WRK.stat_count_off + i, 0x00 );
      }
     
	
      /* Test if the ROS on card supports extended state variables           */
      if (WRK.ent_vpd_hex_rosl >= 0x08)
      {

         /* Read the read only adapter counters/state machines               */
         for (i = 0; i < 5; i++)
         {

            /* Get the current value                                         */
            local_byte_1 = PIO_GETC( bus + WRK.stat_count_off + 72 + (i * 2));
            local_byte_2 = PIO_GETC( bus + WRK.stat_count_off + 73 + (i * 2));

            /* Assign the current value                                      */
            RAS.ds.reserved[i] = ((local_byte_2 << 8) | local_byte_1);

         } /* End for loop to read the read only adapter counters            */
      } /* Endif test for reading special counters on ps2 ff cards only      */

      /* restore IO bus to previous value, done accessing I/O Regs           */
      BUSMEM_DET(bus);                /* restore I/O Bus                     */

   } /* endif test for diagnostic mode                                       */

   TRACE1 ("rdcE");             /* End   of reading adapter counters         */

   return;
} /* end entrdcounts subroutine                                              */

/*****************************************************************************/
/*                                                                           */
/* NAME: ent_start_error                                                     */
/*                                                                           */
/* FUNCTION: Complete the sequence on a first start ioctl to do the clean up.*/
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the off level interrupt handler.        */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Update the start state machine, stop the watchdog timer,       */
/*            clear any remaining execute commands  and issue the            */
/*            connection done call with error.                               */
/*                                                                           */
/*    Called From: entexecdque                                               */
/*                                                                           */
/*    Calls To: cio_conn_done, w_stop, d_clear                               */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
static void ent_start_error (
   register dds_t *dds_ptr)     /* DDS pointer - tells which adapter         */

{
   /* Update the state machine to indicate the adpt is not started           */
   WRK.adpt_start_state = NOT_STARTED;
   WRK.adpt_state       = broken;

   /* Purge the rest of the queued execute command(s)                        */
   WRK.exec_next_in = WRK.exec_next_out;
   WRK.exec_entries = 0x00;

   WRK.exec_current_cmd = 0x00;
   WRK.exec_cmd_in_progres = FALSE;

    w_stop (&(WDT)); /* stop the watchdog timer                              */

    /* Reset the reason why the watchdog timer was set - Now inactive        */
    WRK.wdt_setter = WDT_INACTIVE;

    /* Free up any resourses that were allocated for the receive             */
    ent_recv_undo(dds_ptr);

    /* Clean up TX variables and TCWs etc                                    */
    ent_tx_undo(dds_ptr);

    /* Test if the dma_channel was ever allocated                            */
    if (WRK.channel_alocd == TRUE)
    {
       /* Free the DMA clannel                                               */
       d_clear (WRK.dma_channel);

       /* Turn off the flag                                                  */
       WRK.channel_alocd = FALSE;
    }

    /* Set start done with error                                             */
    WRK.connection_result = (ulong)CIO_HARD_FAIL;
    cio_conn_done(dds_ptr);

    return;

} /* end ent_start_error                                                     */

/*****************************************************************************/
/*                                                                           */
/* NAME: entexecsend                                                         */
/*                                                                           */
/* FUNCTION: Determine which execute command was passed and from that provide*/
/*           to the adapter all the data needed need to execute that command.*/
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Command - which execute command to process.                     */
/*                                                                           */
/*    Output: Execute command given to the adapter and placed in the execute */
/*            mailbox and the adapter interrupted to tell it that an execute */
/*            command has been given.                                        */
/*                                                                           */
/*    Called From: entexecque                                                */
/*                 entexecdque                                               */
/*                                                                           */
/*    Calls To: N/A                                                          */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
static void entexecsend (
   dds_t    *dds_ptr,           /* Pointer to the DDS to know which adpt etc */
   int         bus,             /* Attached memory address                   */
   int         ioa,             /* Attached I/O address                      */
   uchar       cmd)             /* Type of command for the execute mailbox   */
{
   int    index;                /* Index counters for the "for" loops        */
   int    index2;               /* Index counters for the "for" loops        */
   int    index3;               /* Index counters for the "for" loops        */
   ushort temp_length;          /* Temporary length for Pattern Match command*/
   uchar  netid_count;          /* Temporary count for Pattern Match command */
   ushort temp_count;           /* Temporary count for configure list command*/
   int    rc;                   /* Return code from pin                      */

   TRACE2 ("exsB",(ulong)cmd);  /* Execute send command begin                */

   /**************************************************************************/
   /* Assumption: the caller has disabled interrupts & gotten bus access     */
   /**************************************************************************/

   /* Set up switch with the type of execute command to be done              */
   switch (cmd)
   {


      /***********************************************************************/
      /*  Configure adapter's 82586 module        0x0000                     */
      /***********************************************************************/
      case CONFIGURE:
      {

         /* Load the execute mailbox with the command to be executed         */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x00 ); /* Configure Cmd */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 ); /* Command Data  */

         /* No Error packets, allow multicast packets, no promiscuous mode   */
         if (!CIO.promiscuous_count && !CIO.badframe_count)
             PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x00 );
         else if (CIO.promiscuous_count && !CIO.badframe_count)
             PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x01 );
         else if (!CIO.promiscuous_count && CIO.badframe_count)
             PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x04 );
         else if (CIO.promiscuous_count && CIO.badframe_count)
             PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x05 );
         PIO_PUTC( bus + WRK.exec_mail_box + 0x03, 0x00 );

         break;
      }

      /***********************************************************************/
      /*  Set Network Address                     0x0001                     */
      /***********************************************************************/
      case SET_ADDRESS:
      {

         /* Load the execute mailbox with the command to be executed         */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x01 ); /* Configure Cmd  */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 ); /* Command Data   */
         /* Store the 6 byte address in the adapter                          */
         for (index = 0; index < ent_NADR_LENGTH; index++)
         {
            /* Store the bytes in reverse order                              */
            PIO_PUTC( bus + WRK.exec_mail_box + index + 2,
                        WRK.ent_addr[index] );
         }

         break;
      }


      /***********************************************************************/
      /*  Set Multicast Address                   0x0002                     */
      /***********************************************************************/
      case SET_MULTICAST:
      {

         /* Load the execute mailbox with the command to be executed         */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x02 ); /* Configure Cmd  */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 );

         /* Load the current number of multicast Id that are being loaded    */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x02, WRK.multi_count );
         PIO_PUTC( bus + WRK.exec_mail_box + 0x03, 0x00 );

         /* Load the Multicast id's                                          */
         /* Scan the work section multicast list for valid entries           */
         index3 = 0;
         for (index = 0; index < MAX_MULTI; index++)
         {
            /* Test if the current entry is valid                            */
            if (WRK.multi_open[index] != NULL)
            { /* entry valid - copy it to the adapter                        */

               /* copy the valid entry to the adapter                        */
               for (index2 = 0; index2 < ent_NADR_LENGTH; index2++)
               {
                  /* Copy the bytes                                          */
                  PIO_PUTC( bus + WRK.exec_mail_box + 0x04 + index2 +
                    (index3 * ent_NADR_LENGTH), WRK.multi_list[index][index2]);

               }

               index3++;

            } /* end if test to verify if entry is valid                     */
         } /* end for loop for scanning the work section table               */
         break;
      }

      /***********************************************************************/
      /*  Set Receive Pattern Match Filter        0x0003                     */
      /***********************************************************************/
      case SET_TYPE:
      case SET_TYPE_BAD:
      case SET_TYPE_NULL:
      {

         /* Load the execute mailbox with the command to be executed         */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x03 ); /* Set Type Cmd   */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 ); /* Command Data   */

         /* Load Pattern Match Displacement based on first net id length     */
         temp_length = CIO.netid_table_ptr[0].length;

         /* Test if length is two bytes - Standard ethernet                  */
         if (temp_length == 0x0002)
         { /* Standard ethernet netid - update the displacement              */

            /* Type Field Displacement - byte swapped for two byte entry     */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x02,           /* LSB       */
                        (DDI.ds.type_field_off & 0x00FF ));
            PIO_PUTC( bus + WRK.exec_mail_box + 0x03,           /* MSB       */
                        ((DDI.ds.type_field_off & 0xFF00) >> 8));
         }
         else
         { /* IEEE 802.3 ethernet netid - update the displacement            */

            /* Type Field Displacement - byte swapped for two byte entry     */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x02,           /* LSB       */
                        (DDI.ds.net_id_offset & 0x00FF));
            PIO_PUTC( bus + WRK.exec_mail_box + 0x03,           /* MSB       */
                        ((DDI.ds.net_id_offset & 0xFF00) >> 8));
         }

         /* Type Field Length - 1 or 2 bytes netid length                    */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x04, temp_length );

         /* determine the number of netids that match the length             */
         netid_count = 0;

         for (index=0; index < CIO.num_netids; index++)
         {
            /* Test if the length matches                                    */
            if (temp_length == CIO.netid_table_ptr[index].length)
               netid_count++;
         }

         /* Type Field Count - number of valid netids                        */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x05, netid_count );

         /* If this is a null entry-tell the adapter to open the flood gates */
         if (cmd == SET_TYPE_NULL)
         {

            /* Type Field Count - number of valid netids - 0                 */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x05, 0 );
            break;
         }

         /* If this is a bad entry - tell the adapter to receive nothing     */
         if (cmd == SET_TYPE_BAD)
         {

            /* Type Field Displacement - byte swapped for two byte entry     */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x0C );   /* LSB - 13  */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x03, 0x00 );   /* MSB - 00  */

            /* Type Field Length -  2 bytes netid length                     */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x04, 0x02 );

            /* Type Field Count - number of valid netids is 1                */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x05, 0x01 );

            /* Store Least Significant byte in the adapter memory            */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x06, 0xFF );

            /* Store Most  Significant byte in the adapter memory            */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x07, 0xFF );

            break;
         }

         /* Type Field List                                                  */
         for (index = 0; index < CIO.num_netids; index++)
         {
            /* Test if each net id is 2 bytes long                           */
            if ((temp_length == 0x0002) &&
                (CIO.netid_table_ptr[index].length == 0x0002))
            { /* entry is a two byte value - copy both bytes                 */

               /* copy the valid entry to the adapter - NOT Byte swapped     */
               /* Store Most  Significant byte in the adapter memory         */
               PIO_PUTC( bus + WRK.exec_mail_box + 0x06 + (index * 2) + 0,
                          ((CIO.netid_table_ptr[index].netid & 0xFF00) >> 8));

               /* Store Least Significant byte in the adapter memory         */
               PIO_PUTC( bus + WRK.exec_mail_box + 0x06 + (index * 2) + 1,
                          (CIO.netid_table_ptr[index].netid & 0x00FF));

            }

            /* Test if each net id is 1 byte long                            */
            if ((temp_length == 0x0001) &&
                (CIO.netid_table_ptr[index].length == 0x0001))
            { /* entry is a one byte value - copy least significant byte     */

               /* Store Least Significant byte in the adapter memory         */
               PIO_PUTC( bus + WRK.exec_mail_box + 0x06 + index,
                          (CIO.netid_table_ptr[index].netid & 0x00FF));

            }
         } /* end for loop for scanning the work section table               */

         break;
      }

      /***********************************************************************/
      /*  Indication Enable/Disable               0x0004                     */
      /***********************************************************************/
      case INDICAT_EN:
      case INDICAT_DS:
      {

         /* Load the execute mailbox with the command to be executed         */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x04 ); /* Configure Cmd  */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 ); /* Command Data   */
         /* Determine Which indication is being set                          */
         if (cmd == INDICAT_DS)
         {
            /* Disable the indication of adapter interrupts                  */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x00 );
         }
         else
         {
            /* Enable the indication of adapter interrupts                   */
            PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x01 );
         } /* endif test for type of indication is set                       */

         break;
      }
      /***********************************************************************/
      /*  Report Configuration                    0x0006                     */
      /***********************************************************************/
      case REPORT_CONFIG:
      {

         /* Load the execute mailbox with the command to be executed         */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x06 ); /* Configure Cmd  */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 ); /* Command Data   */
         for (index = 0; index < 28; index++)
         {
            /* Initialize the return area to zero                            */
            PIO_PUTC( bus + WRK.exec_mail_box + index + 2, 0x00 );
         }
         break;
      }


      /***********************************************************************/
      /*  Configure Lists                         0x0008                     */
      /***********************************************************************/
      case CONFIG_LIST:
      {
         /* Determine the number of elements based on region size            */
         temp_count = (WRK.buf_des_reg_size / BUF_DES_SIZE);

         /* Test if the region size is big enough to match the TCW's         */
         if ((WRK.xmit_tcw_cnt + WRK.recv_tcw_cnt) > temp_count)
         {  /* Host has more TCW's than adapter has buffer descriptors       */

            /* Set up temp_count to the differnece between region & TCW      */
            temp_count -= (WRK.xmit_tcw_cnt + WRK.recv_tcw_cnt);

            /* Reduce the TCW count.  Keep the receive tcw count even so
             * an entire page is used for receive buffers.  Unpin the portion
             * of the receive buffer that will not be used.  The transmit
	     * TCW count is allready at its minimium, so only reduce the
	     * receive count
             */
	    if ((WRK.recv_tcw_cnt - temp_count) % 2)
	       temp_count++;
	    rc = unpin(WRK.recv_buf +
		(WRK.recv_tcw_cnt - temp_count) * DMA_PSIZE / 2,
		temp_count * DMA_PSIZE / 2);
	    assert(rc == 0);
            WRK.recv_tcw_cnt -= temp_count;
         }

         /* Load the execute mailbox with the command to be executed         */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x08 ); /* Configure Cmd  */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 ); /* Command Data   */
         PIO_PUTC( bus + WRK.exec_mail_box + 0x02,
                                           ((WRK.xmit_tcw_cnt & 0x00ff) >> 0));
         PIO_PUTC( bus + WRK.exec_mail_box + 0x03,
                                           ((WRK.xmit_tcw_cnt & 0xff00) >> 8));
         PIO_PUTC( bus + WRK.exec_mail_box + 0x04,
                                           ((WRK.recv_tcw_cnt & 0x00ff) >> 0));
         PIO_PUTC( bus + WRK.exec_mail_box + 0x05,
                                           ((WRK.recv_tcw_cnt & 0xff00) >> 8));

         break;
      }
      /***********************************************************************/
      /*  586 AL-LOC Off                          0x000D                     */
      /***********************************************************************/
      case AL_LOC_ON:
      {

        DTRACE2("ALOC", cmd);
        /* Load the execute mailbox with the command to be executed         */
        PIO_PUTC( bus + WRK.exec_mail_box + 0x00, 0x0D ); /* LOC Cmd  */
        PIO_PUTC( bus + WRK.exec_mail_box + 0x01, 0x00 ); /* Command Data   */
        PIO_PUTC( bus + WRK.exec_mail_box + 0x02, 0x00 ); /* Second word zero */
        WRK.mbox_status = execute;
        mail_check_status(dds_ptr);
        break;
      }

      default: /* Invalid or unsupported command sent -                      */

         TRACE1 ("exe?");
         return;

   } /* endcase statement for type of execute command                        */

   /* Issue the execute command to the command register                      */
   PIO_PUTC( ioa + COMMAND_REG,EXECUTE_MSK );

   TRACE1 ("exsE");             /* Execute send command begin                */

   return;
} /* end entexecsend subroutine                                              */

/*****************************************************************************/
/*                                                                           */
/* NAME: entexecque                                                          */
/*                                                                           */
/* FUNCTION: Queue up or actually give the adapter an execute command.       */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs under the process thread and the off level         */
/*      interrupt handler.                                                   */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Open pointer - Wake up value upon completion of command.        */
/*           Command - actual execute command to be processed.               */
/*                                                                           */
/*    Output: Command either queued or given to adapter.                     */
/*                                                                           */
/*    Called From: xxx_offl, xxx_ioctl, xxx_newtbl, xxx_close                */
/*                                                                           */
/*    Calls To:    entexecsend                                               */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
static void entexecque (
   register dds_t       *dds_ptr,  /* Pointer to the DDS to know which adptr */
   register open_elem_t *open_ptr, /* Open Pointer - unique @ to sleep on    */
            uchar       cmd)       /* Type of command for the execute mailbox*/
{
   int    saved_intr_level;
   int    ioa, bus;

   TRACE2 ("exqB",(ulong)cmd);     /* Execute command que begin              */

   bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );
   ioa = (int)BUSIO_ATT(  (ulong)DDI.cc.bus_id, DDI.ds.io_port );

   /* Disable interrupts while processing these commands                     */
   DISABLE_INTERRUPTS(saved_intr_level);

   /**************************************************************************/
   /* Assumption: the caller has gotten bus access                           */
   /**************************************************************************/

   /* Test if there is an execute command being processed by the adapter     */
   /* and Test to verify that the adapter can except an execute command      */
   if ((WRK.exec_cmd_in_progres != TRUE)                            &&
       (CRR_MSK  == (PIO_GETC( ioa + STATUS_REG )          & CRR_MSK))  &&
       (DONE_MSK == (PIO_GETC( bus + WRK.exec_mail_box + 1 ) & DONE_MSK)))
   { /* there is no execute command on the adapter - see if it can take one  */

      /* Give valid element to the adapter                                   */
      entexecsend (dds_ptr, bus, ioa, cmd);

      /* Queue element sent - save cmd & open pointer as current data        */
      WRK.exec_current_cmd = cmd;
      WRK.exec_cmd_in_progres = TRUE;
   }
   else
   { /* Command in progress - must que the current command                   */

      /* Test if there is room in the queue for this element                 */
      if (WRK.exec_entries < MAX_EXEC)
      { /* There is room so store it                                         */

         /* Store the current command in the queue                           */
         WRK.exec_que[WRK.exec_next_in] = cmd;

         /* Increment the next in pointer                                    */
         if (++WRK.exec_next_in >= MAX_EXEC)
            WRK.exec_next_in = 0;

         /* Increment the number of entries in the queue                     */
         WRK.exec_entries++;
      }
      else
      { /* Maximun exceeded - increment RAS counter                          */

         RAS.ds.exec_over_flow++;

      } /* endif test for room for que element in the execute que            */
   } /* endif Test if there is an execute command being processed by the adpt*/

   ENABLE_INTERRUPTS(saved_intr_level);
   BUSIO_DET(ioa);
   BUSMEM_DET(bus);

   TRACE1 ("exqE");                /* Execute command que end                */

   return;
} /* end entexecque routine                                                  */

/*****************************************************************************/
/*                                                                           */
/* NAME: entexecdque                                                         */
/*                                                                           */
/* FUNCTION: Test if current execute command has completed.                  */
/*           Complete the processing of the current execute command,if needed*/
/*           and sent to the adapter the next command if one is present. Wake*/
/*           up the process that is waiting on the current command,if needed.*/
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the off level interrupt handler.        */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Completed processing of current execute command, set up        */
/*            for the next command.                                          */
/*                                                                           */
/*    Called From: xxx_offl                                                  */
/*                                                                           */
/*    Calls To: entgetconfig                                                 */
/*              entexecsend                                                  */
/*              ent_recv_start                                               */
/*              cio_conn_done                                                */
/*              ent_recv_undo                                                */
/*              ent_logerr                                                   */
/*              ent_start_error                                              */
/*              w_stop                                                       */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void entexecdque (
   register  dds_t   *dds_ptr)   /* Pointer to the DDS to know which adapter */
{
   open_elem_t *open_ptr;        /* Open Pointer-unique @ to sleep/wakeup on */
   int    saved_intr_level;
   int    bus, ioa;
   int    i;                     /* Loop counter                             */

   TRACE2 ("exdB",(ulong)dds_ptr);  /* Execute command deque begin           */

   /* Get access to the I/O bus to access I/O registers                      */
   bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );
   ioa = (int)BUSIO_ATT(  (ulong)DDI.cc.bus_id, DDI.ds.io_port );

   /* Disable interrupts while processing these commands                     */
   DISABLE_INTERRUPTS(saved_intr_level);

   /* Test if processing an execute command & that command is complete       */
   if ((WRK.exec_cmd_in_progres != FALSE) &&
       (DONE_MSK == (PIO_GETC( bus + WRK.exec_mail_box + 1 ) & DONE_MSK)))
   {

      /* Test if error bit is on and increment count                         */
      if (ERROR_MSK == (PIO_GETC( bus + WRK.exec_mail_box + 1 ) & ERROR_MSK))
      {
         RAS.ds.exec_cmd_errors++;
	 WRK.mbox_status = execute;
	 mail_check_status(dds_ptr);
         /* Test if from a first start ioctl                                 */
         if (WRK.adpt_start_state != STARTED)
         {

            /* Complete the start with hard error                            */
            ent_start_error(dds_ptr);
         }

         /* Log that an error occured                                        */
         ent_logerr(dds_ptr, ERRID_ENT_ERR3, CIO_HARD_FAIL,
                                                           (uchar)0, (uchar)0);
      }
      /***********************************************************************/
      /* Execute Mail Box Command Post interrupt processing                  */
      /***********************************************************************/

      /* Test if Report Config is the command that was processed             */

      if ((WRK.exec_current_cmd == REPORT_CONFIG) &&
          (PIO_GETC( bus + WRK.exec_mail_box ) == 0x06))
      /*  (DONE_MSK == (PIO_GETC( bus + WRK.exec_mail_box + 1 )& DONE_MSK))) */
      { /* Read and save the configuration - if error increment count        */

         /* Test for errors in reading the report config data                */
         if (entgetconfig(dds_ptr, bus) != 0x00)
         {
            RAS.ds.exec_cmd_errors++;

            /* Test if from a first start ioctl                              */
            if (WRK.adpt_start_state != STARTED)
            {

               /* Complete the start with hard error                         */
               ent_start_error(dds_ptr);

               /* Log that an error occured                                  */
               ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR3, (ulong)CIO_HARD_FAIL,
                                                           (uchar)0, (uchar)0);
            }
         }
         else
         { /* Getting adapter config data worked OK                          */

            /* Test if the VPD ROS Level matches the firmware level          */
            /* Test if from a first start ioctl                              */
            if (WRK.adpt_start_state == EXEC_CMDS_STARTED)
            {

               /* Test if the levels match                                   */
               if (WRK.ent_vpd_hex_rosl != WRK.version_num)
               {
                  /* Complete the start with hard error                      */
                  ent_start_error(dds_ptr);

                  /* Log that an error occured                               */
                  ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR1,
                                     (ulong)CIO_HARD_FAIL, (uchar)0, (uchar)0);

               } /* endif test for the ROS level match                       */
            } /* endif test for first start ioctl                            */
         } /* endif test for good report config return                       */
      } /* endif test was the last execute command a report config           */

      /* Test if SET_TYPE is the command that was processed                  */
      if (((WRK.exec_current_cmd & 0x0F) == (SET_TYPE & 0x0F)) &&
          (PIO_GETC( bus + WRK.exec_mail_box ) == 0x03))
      /*  (DONE_MSK == (PIO_GETC( bus + WRK.exec_mail_box + 1 ) & DONE_MSK)))*/
      { /* Read and save the configuration - if error increment count        */

         /* Test if this is from a first start ioctl command                 */
         if (WRK.adpt_start_state == EXEC_CMDS_STARTED)
         {

            /* Update the adapter start state variable                       */
            WRK.adpt_start_state = STARTED;
	    if (WRK.adpt_state == restart) {
		    async_notify(dds_ptr, CIO_NET_RCVRY_EXIT, 0, 0, 0);
		    WRK.adpt_state = normal;
	    }

            w_stop (&(WDT)); /* stop the watchdog timer                      */

            /* Reset the reason why the watchdog timer was set - Now inactive*/
            WRK.wdt_setter = WDT_INACTIVE;

            ent_recv_start(dds_ptr);

            /* Intialize the last netid variables to first entry          */
            WRK.last_netid = CIO.netid_table_ptr[0].netid;
            WRK.last_netid_length = CIO.netid_table_ptr[0].length;
            WRK.last_netid_index = 0;

            /* Notify the device driver that the first start is completed */
            WRK.connection_result = (ulong)CIO_OK;

            /* setup tranmit queues */
            ent_tx_start(dds_ptr);

            cio_conn_done(dds_ptr);
         }
      }

      /* Reset the command in progress flag                                  */
      WRK.exec_cmd_in_progres = FALSE;

   }

   /* Test if there is an execute command being processed by the adapter     */
   /* and Test to verify that the adapter can except an execute command      */
   /* and Test to verify that there is another command on the que            */
   if ((WRK.exec_cmd_in_progres != TRUE)                            &&
       (CRR_MSK  == (PIO_GETC( ioa + STATUS_REG )          & CRR_MSK))  &&
       (DONE_MSK == (PIO_GETC( bus + WRK.exec_mail_box + 1 ) & DONE_MSK)) &&
       (WRK.exec_entries > 0))
   { /* there is no execute command on the adapter - see if it can take one  */

      /* Give valid element to the adapter                                   */
      entexecsend (dds_ptr, bus, ioa, WRK.exec_que[WRK.exec_next_out]);

      /* Decrement the number of valid entries in the que                    */
      WRK.exec_entries--;

      /* Queue element sent - save cmd & open pointer as current data        */

      WRK.exec_current_cmd = WRK.exec_que[WRK.exec_next_out];
      WRK.exec_cmd_in_progres = TRUE;

      /* Increment the next out pointer                                      */
      if (++WRK.exec_next_out >= MAX_EXEC)
         WRK.exec_next_out = 0;

   }

   /* restore IO bus to previous value, done accessing I/O Regs              */
   BUSIO_DET(ioa);                 /* restore I/O Bus                        */
   BUSMEM_DET(bus);

   ENABLE_INTERRUPTS(saved_intr_level);

   TRACE1 ("exdE");                 /* Execute command deque end             */

   return;
} /* end entexecdque routine                                                 */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxxoffl                                                             */
/*                                                                           */
/* FUNCTION: Complete the processing of the selected type of interrupt.      */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the off level interrupt handler.        */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Off level pointer - tells which interrupt to process            */
/*                                                                           */
/*    Output: Completed processing of selected interrupt.                    */
/*                                                                           */
/*    Called From: xxx_offl                                                  */
/*                                                                           */
/*    Calls To: entexecdque                                                  */
/*              entrdcounts                                                  */
/*              ent_recv                                                     */
/*              e_wakeup                                                     */
/*              ent_tx_off_lvl                                               */
/*              cio_conn_done                                                */
/*              ent_logerr                                                   */
/*              ent_tx_time_out                                              */
/*              QUEOFFLMS                                                    */
/*              entgetconfig                                                 */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void xxxoffl (
   register   dds_t       *dds_ptr,  /* DDS pointer - tells which adapter    */
   register   offl_elem_t *offl_ptr) /*                                      */
{
   int    ioa, bus;
   int    iocc;
   ulong  rc;                      /* Return Code value                      */
   int saved_intr_level;
   int i;                          /* Loop counter                           */

   switch (offl_ptr->who_queued)
   {
      /***********************************************************************/
      /* Watch Dog Timer Interrupt has occurred - Process it                 */
      /***********************************************************************/
      case OFFL_WDT:

         TRACE2 ("offW",(ulong)dds_ptr); /* Offlevel Watch Dog Timer Intr    */

	 /*
	  * when state is abnormal, we extend the watchdog timer to permit
	  * time to try to restart the adapter.
	  */
	 if (WRK.adpt_state != normal && --WRK.doggies > 0)
		 break;

         /* Determine who set the watch dog timer                            */
	 i = WRK.wdt_setter;
	 WRK.wdt_setter = WDT_INACTIVE;
         switch (i) {
            /* WDT set by start connection - First Start                     */
            case WDT_CONNECT:

               /* Test if the adapter every did get started                  */
               if (WRK.adpt_start_state != STARTED)
               { /* Timer popped before adapter completed - set conn_done    */

                  /* Update the adapter start state variable                 */
                  WRK.adpt_start_state = STARTED;

                  /* Notify the dd that the first start is completed         */
                  WRK.connection_result = (ulong)CIO_HARD_FAIL;
                  cio_conn_done(dds_ptr);

                  /* Log that an error occured                               */
                  ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR1,
                                     (ulong)CIO_HARD_FAIL, (uchar)0, (uchar)0);
               }

               break;

            /* WDT set by start transmit                                     */
            case WDT_XMIT:
               /* Call transmit time out and cleanup routine                 */
               ent_tx_time_out(dds_ptr);

               break;

            /* WDT set by adapter close                                      */
            case WDT_CLOSE:
               TRACE2 ("slpE",(ulong)&(WRK.close_event)); /* Sleep End-close */

               /* Wake up the sleeping process                               */
               e_wakeup(&(WRK.close_event));

               break;

	   case WDT_RESTART:
	       WRK.connection_result = CIO_HARD_FAIL;
               break;

            /* Unknown reason why the watch dog timer popped - ignore it     */
            default:
               break;
         }
         break;

      /***********************************************************************/
      /* Adapter Time out interrupt has occurred - Process it                */
      /***********************************************************************/
      case OFFL_TIMO:

         TRACE2 ("offO",(ulong)dds_ptr); /* Offlevel Time Out                */

         /* Determine who set the timeout timer                              */
	 i = WRK.to_setter;
	 WRK.to_setter = TO_INACTIVE;

         switch (i) {
	     case TO_CLEANUP:
		 ent_cleanup(dds_ptr);
		 break;

	     case TO_RX_MBUF:
               ent_recv(dds_ptr, RX_P_RCVD);
               break;

            /* Timeout set by First Start                                    */
            case TO_START:
               /* Test if this time out is from the first start ioctl        */
               if (WRK.adpt_start_state != STARTED)
               { /* In process of completing the first start command         */

                  DISABLE_INTERRUPTS (saved_intr_level);

                  /* Get access to the I/O bus to access I/O registers       */
                  ioa = (int)BUSIO_ATT( (ulong)DDI.cc.bus_id, DDI.ds.io_port );

                  /* Test if it is time to turn off the hard reset           */
                  if (WRK.adpt_start_state == HARD_RESET_OFF)
                  {

                     /* Set RAM Page Register to default value               */
                     PIO_PUTC( ioa + RAM_PAGE_REG,RAM_PAGE_VALUE );

                     /* Determine if adptr is to perform I/O Parity Checking */
                     if (WRK.bus_parity)
                     { /*I/O Parity Checking configd, set the Parity Cntl Reg*/

                        /* Reset any parity err that might have occured      */
                        /* Wrt the parity cntl reg with mask to turn off bits*/
                        PIO_PUTC( ioa + PARITY_REG,PAREN_MSK );
                     }
                     else
                     { /* I/O Parity Checking not configured so DiSable it   */

                        /* Reset any parity err that might have occured      */
                        /* Wrt the parity cntl reg with mask to turn off bits*/
                        PIO_PUTC( ioa + PARITY_REG,PARDS_MSK );

                     } /* endif test to do I/O Parity Checking               */

                     /* Reenable parity interrupts */

                     /* Get access to the IOCC to access POS registers */
                     iocc = (int)IOCC_ATT((ulong)(DDI.cc.bus_id),
                                       (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

                     /* Restore POS register 4 to enable parity interrupts */
                     PIO_PUTC( iocc + POS_REG_4, WRK.pos_reg[POS_REG_4] );

                     /* restore IOCC to previous value -done accessing POS Reg*/
                     IOCC_DET(iocc);     /* restore IOCC */

                     /* Write zero to the parity bit in the status reg(2)    */
                     PIO_PUTC( ioa + STATUS_REG,~PARITY_MSK );

                     /* Enable adapter interrupts                            */
                     PIO_PUTC( ioa + CONTROL_REG,EN_INTR_MSK );

                     /* Force RAM page register to the default value         */
                     PIO_PUTC( ioa + RAM_PAGE_REG,RAM_PAGE_VALUE );

                     /* Update the adapter start state variable              */
                     WRK.adpt_start_state = SELF_TEST_ON;
                  }

                  /* Test if it is time to turn off the hard reset           */
                  if (WRK.adpt_start_state == HARD_RESET_ON)
                  {

                     /* Remove hard reset but leave adpt interrupts disabled */
                     PIO_PUTC( ioa + CONTROL_REG,CONTROL_VALUE );

                     /* Update the adapter start state variable              */
                     WRK.adpt_start_state = HARD_RESET_OFF;

                     /* Set up who set the time out timer                    */
                     WRK.to_setter = TO_START;

                     /* Set up Time Delay for 1 second (1000 miliseconds)    */
                     QUEOFFLMS(1000);

                  }

                  /* restore IO bus to previous value, done accessing I/O Reg*/
                  BUSIO_DET(ioa);                 /* restore I/O Bus         */

                  ENABLE_INTERRUPTS (saved_intr_level);

               } /* endif test for time out from the first start ioctl       */

               break;

            /* Timeout set by ????? - Do nothing                             */
            default:
               break;
         } /* end switch statement for who set the timeout timer             */

         break;

      /***********************************************************************/
      /* Adapter START Interrupt has occurred - Process it                   */
      /***********************************************************************/
      case OFFL_START:

         TRACE2 ("offS",(ulong)WRK.adpt_start_state); /* Offlevel Start intr */

         DISABLE_INTERRUPTS (saved_intr_level);

         /* Get access to the I/O bus to access I/O registers                */
         bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );

         switch (WRK.adpt_start_state)
         {

            /* Adapter self test running test if command reg is zero yet     */
            case SELF_TEST_ON:

               /* Test if adapter has successfully completed its self tests  */
               if (SELF_TESTS_OK == offl_ptr->cmd_reg)
               {

                  /* Update the adapter start state variable                 */
                  WRK.adpt_start_state = GET_EMB_1;

               }
               break;

            /* Command register contains 1st byte of execute mailbox address */
            case GET_EMB_1:

               /* Least Significant bytes comes first                        */
               WRK.exec_mail_box = offl_ptr->cmd_reg;

               /* Update the adapter start state variable                    */
               WRK.adpt_start_state = GET_EMB_2;

               break;

            /* Command register contains 2nd byte of execute mailbox address */
            case GET_EMB_2:

               /* Next to least Significant bytes comes second               */
               WRK.exec_mail_box |= (offl_ptr->cmd_reg << 8);

               /* Update the adapter start state variable                    */
               WRK.adpt_start_state = GET_EMB_3;

               break;

            /* Command register contains 3rd byte of execute mailbox address */
            case GET_EMB_3:

               /* Next to most Significant byte comes third                  */
               WRK.exec_mail_box |= (offl_ptr->cmd_reg << 16);

               /* Update the adapter start state variable                    */
               WRK.adpt_start_state = GET_EMB_4;

               break;

            /* Command register contains 4th byte of execute mailbox address */
            case GET_EMB_4:

               /* Most Significant byte comes fourth                         */
               WRK.exec_mail_box |= (offl_ptr->cmd_reg << 24);

               rc = entgetconfig (dds_ptr, bus);

               /* Test the return code for valid config data received        */
               if (rc != 0x00) {
		  /* restore IO bus to previous value, done accessing I/O Reg*/
                  BUSMEM_DET(bus);                /* restore I/O Bus         */

                  ENABLE_INTERRUPTS (saved_intr_level);

		  if (WRK.adpt_state != normal) {
			  /*
			   * mark broken, leave any watchdogs running.
			   */
			  WRK.adpt_state = broken;
		  } else {
		      w_stop (&(WDT)); /* stop the watchdog timer            */

		      /* Reset the reason why the wdt was set - Now inactive */
		      WRK.wdt_setter = WDT_INACTIVE;

		      /* Update the adapter start state variable             */
		      WRK.adpt_start_state = STARTED;

		      /* Notify the dd that the first start is completed     */
		      WRK.connection_result = (ulong)CIO_HARD_FAIL;
		      cio_conn_done(dds_ptr);
	         }
		  /* Log that an error occured                               */
		  ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR3
			     , (ulong)CIO_HARD_FAIL, (uchar)0, (uchar)0);
                  return;
               }

               /* NOTE: No more that 8 elements in the que - overflow the que*/

               /* Update the adapter start state variable                    */
               WRK.adpt_start_state = EXEC_CMDS_STARTED;

               /* Enable execute Mailbox command interrupts                  */
               entexecque(dds_ptr, NULL, (uchar)INDICAT_EN);

               /* Turn on AL-LOC on to enable back-to-back receive frames 
		  if new microcode */
               if (WRK.ent_vpd_hex_rosl >= 0x12) {
                   entexecque(dds_ptr, NULL, (uchar)AL_LOC_ON);
                   WRK.mbox_status = execute;
                   mail_check_status(dds_ptr);
		}

               /* Set up the Receive Packet Filter                           */
               /* No Error or Promiscuous packets, allow Multicast packets   */
               entexecque(dds_ptr, NULL, (uchar)CONFIGURE);

               /* Set up adapter with Network Address                        */
               entexecque(dds_ptr, NULL, (uchar)SET_ADDRESS);

               /* Set up adapter the number of transmit & receive list to use*/
               /* Need to do another report_Config to know where the list are*/
               entexecque(dds_ptr, NULL, (uchar)CONFIG_LIST);
               entexecque(dds_ptr, NULL, (uchar)REPORT_CONFIG);

               /* Set up adapter with the network IDs - issue ent_conn_done  */
               /* Note: This must be the last on in this series in order to  */
               /*       change the START state and issue cio_conn_done       */
               entexecque(dds_ptr, NULL, (uchar)SET_TYPE_BAD);

               break;

            /* Execute commands started - deque them                         */
            case EXEC_CMDS_STARTED:

               /* Check if there is another execute command to be processed  */
               entexecdque (dds_ptr);

               break;

            /* Adapter self test running test if command reg is zero yet     */
            case STARTED:

               break;

            default:

               break;

         } /* End Case select for WRK.adpt_start_state                       */

         /* restore IO bus to previous value, done accessing I/O Regs        */
         BUSMEM_DET(bus);                /* restore I/O Bus                  */

         ENABLE_INTERRUPTS (saved_intr_level);

         break;

      default:

         break;

   } /* end switch offl_ptr->who_queued                                      */

   return;

} /* end xxxoffl                                                             */

/*****************************************************************************/
/*
 * NAME: parity_error
 *
 * FUNCTION: check if a parity error occured and do error logging
 *
 * EXECUTION ENVIORNMENT:
 *	called from device interrupt level
 *
 * RETURNS:
 *	0 if no error occured
 *	1 if an error occured
 *
 */
/*****************************************************************************/

int
parity_error (
	dds_t *dds_ptr,
	char command_reg,
	char status_reg
)
{
	int 		rc;
	int 		ioa;
	char 		parity_reg;
	register struct intr *ihsptr;
	int		count;

	TRACE2("PERR", status_reg);

	ioa = (int)BUSIO_ATT((ulong)WRK.bus_id, WRK.io_port);

	/* get the data from the parity control register  (I/O Base + A)
	 */
	PIO_GETCX((char *)(ioa + PARITY_REG), &parity_reg);

	/* Test if any valid parity error has occurred
         * Version 1 card, any parity bit on
	 */
	switch (WRK.card_type) {
	    case ACT_10:
	    case ACT_20:
	    case ACT_22:
	    case ACT_225:
	    case ACT_23:
	    case ACT_235:
		parity_reg &= ANY_ERR_MSK;
		break;

	    case ACT_30:
		parity_reg &= (WRK.fdbk_intr_en
			       ? ANY_ERR3_MSK
			       : (ANY_ERR3_MSK & ~FBRT_ERR3_MSK));
		break;

	    default:
		assert(0);
	}

	if (parity_reg) {
		++RAS.ds.par_err_cnt;

		ent_logerr(dds_ptr, ERRID_ENT_ERR2
			   , parity_reg, command_reg, status_reg);

		switch (WRK.adpt_state) {
		    case normal:
			/*
			 * we have just noticed an error.
			 */
			WRK.adpt_state = error;

			/*
			 * force the board into a reset state,
			 * disable parity checking, and
			 * clear the parity bit in the status register
			 */
		      disable:
			PIO_PUTC(ioa + CONTROL_REG, HARD_RST_MSK);
			PIO_PUTC(ioa + PARITY_REG, PARDS_MSK);
			PIO_PUTC(ioa + STATUS_REG, ~PARITY_MSK);

			WRK.adpt_start_state = HARD_RESET_ON;
			break;

		    case error:
			/*
			 * we have already seen the error, but the off-level
			 * code hasn't yet made the transition to cleanup.
			 */
			break;

		    case cleanup:
		    case restart:
			/*
			 * we are in cleanup or restart mode.  since we are
			 * still having errors, we go to a hard error state.
			 */
			WRK.adpt_state = broken;
			goto disable;
		}

		rc = 1;
	} else
		rc = 0;

	BUSIO_DET(ioa);

	return(rc);
}

/*****************************************************************************/
/*
 * NAME: diag_intr
 *
 * FUNCTION: process diagnostics interrupts
 *
 * EXECUTION ENVIORNMENT: called by device interrupt handler
 *
 * RETURNS:
 *	INTR_SUCC if device generated interrupt
 */
/*****************************************************************************/

int
diag_intr (
	dds_t *dds_ptr)
{
      int rc;
      uchar PR2_VALUE;
      int iocc;
      int ioa;
      uchar host_command_reg;
      uchar host_status_reg;
      uchar host_parity_reg;

      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
      /* Programmer's Note:                                                  */
      /*                                                                     */
      /* Verify that the diagnostic Test Unit has card enable active before  */
      /* doing any reads of the status, command or parity registers.         */
      /*                                                                     */
      /* Note: the case exits where this device driver will fail to service  */
      /* an adapter interrupt because the Test Unit left interrupts enabled  */
      /* but disabled the card. This will result in a HOT interrupt.         */
      /*                                                                     */
      /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

      rc = INTR_FAIL;
      /* Get access to the IOCC to access POS registers                      */
      iocc = (int)IOCC_ATT((ulong)(DDI.cc.bus_id),
                                       (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

      /* Read POS register 2                                                 */
      PR2_VALUE = PIO_GETC( iocc + POS_REG_2 );

      /* restore IOCC to previous value - done accessing POS Regs            */
      IOCC_DET(iocc);                /* restore IOCC                         */

      /* Test if Card Enable bit is active                                   */
      if ((PR2_VALUE & PR2_CDEN_MSK) == PR2_CDEN_MSK)
      { /* Card enable is active - test if valid interrupt                   */

         /* Get access to the I/O bus to access I/O registers                */
         ioa = (int)BUSIO_ATT( (ulong)DDI.cc.bus_id, DDI.ds.io_port );

         /* Loop forever processing interrupts from adapter - break to exit  */
         while (TRUE)
         {
            /* Read the interrupt status register for this card              */
            host_status_reg = PIO_GETC( ioa + STATUS_REG );

            /* Test if command word received interrupt has occurred          */
            if (host_status_reg & CWR_MSK)
            {

               /* Valid adapter interrupt - set return code to success       */
               rc = INTR_SUCC;

               /* Read the command register for this card                    */
               host_command_reg = PIO_GETC( ioa + COMMAND_REG );

               /* Test if there is room in the ring queue for this data      */
               if (WRK.num_entries < MAX_DIAG_QUEUED)
               {
                  /* Read and save the Host status reg for this adapter      */
                  WRK.io_stat_q[WRK.next_in] = host_status_reg;

                  /* Read and save the command register for this adapter     */
                  WRK.io_cmd_q[WRK.next_in] = host_command_reg;

                  /* Increment the next in pointer                           */
                  if (++WRK.next_in >= MAX_DIAG_QUEUED)
                     WRK.next_in = 0;

                  /* Increment the number of valid entries in the queue      */
                  WRK.num_entries++;

               }
               else /* No room left in diagnostic ring queue - ERROR!!!!!    */
               {
                  /* Increment the diagnostic over flow counter              */
                  RAS.ds.diag_over_flow++;

               } /* endif test for diagnostic ring queue full                */

            }
            else
            { /* this adapter is not interrupting via the status register    */

               /* Exit the while loop - no more interrupts to process        */
               break;

            } /* endif test for command word received interrupt              */
         } /* end while adapter is still interrupting                        */

         /* get the data from the parity control register  (I/O Base + A)    */
         host_parity_reg = PIO_GETC( ioa + PARITY_REG );

         /* Test if any valid parity error has occurred                      */
             /* Version 1 card, any parity bit on                            */
         if (((host_parity_reg & ANY_ERR_MSK)  && (WRK.card_type == ACT_10)) ||

             /* Version 2 card, any parity bit on                            */
             ((host_parity_reg & ANY_ERR_MSK) && (WRK.card_type == ACT_20))  ||
             ((host_parity_reg & ANY_ERR_MSK) && (WRK.card_type == ACT_22))  ||
             ((host_parity_reg & ANY_ERR_MSK) && (WRK.card_type == ACT_225)) ||
             ((host_parity_reg & ANY_ERR_MSK) && (WRK.card_type == ACT_23))  ||
             ((host_parity_reg & ANY_ERR_MSK) && (WRK.card_type == ACT_235)) ||

             /* Version 3 card, select feed back enabled                     */
             ((host_parity_reg & ANY_ERR3_MSK) && (WRK.card_type == ACT_30) &&
                                                    (WRK.fdbk_intr_en == 1)) ||

             /* Version 3 card, select feed back disabled                    */
             ((host_parity_reg & ANY_ERR3_MSK & ~FBRT_ERR3_MSK) &&
                       (WRK.card_type == ACT_30) && (WRK.fdbk_intr_en == 0)))

         {
            TRACE2 ("intP",(ulong)host_parity_reg); /* SLIH Parity error     */

            /* Valid adapter interrupt - set return code to success          */
            rc = INTR_SUCC;

            /* Increment the RAS parity error counter                        */
            RAS.ds.par_err_cnt++;

            /* Log that an error occured                                     */
            ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR2, (ulong)0,
                                            host_command_reg, host_status_reg);
            /* Reset the parity error                                        */

            /* Determine if adptr is to perform I/O Parity Checking          */
            if (WRK.bus_parity)
            { /*I/O Parity Checking configd, set the Parity Cntl Reg         */

               /* Reset any parity err that might have occured               */
               /* Wrt the parity cntl reg with mask to turn off bits         */
               PIO_PUTC( ioa + PARITY_REG,PAREN_MSK );
            }
            else
            { /* I/O Parity Checking not configured so DiSable it            */

               /* Reset any parity error that might have occured             */
               /* Wrt the parity cntl reg with mask to turn off bits         */
               PIO_PUTC( ioa + PARITY_REG,PARDS_MSK );

            } /* endif test to do I/O Parity Checking                        */

            /* Write zero to the parity bit in the status register (2)       */
            PIO_PUTC( ioa + STATUS_REG,~PARITY_MSK );

         } /* endif test for valid adapter parity error                      */

         /* restore IO bus to previous value, done accessing I/O Regs        */
         BUSIO_DET(ioa);                 /* restore I/O Bus                  */

      } /* endif test if the card enable bit is active                       */

      return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_act                                                             */
/*                                                                           */
/* FUNCTION: Start the initalization of the adapter: load POS registers and  */
/*           start the sequence for hard reset of the adapter.               */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: An initialized adapter.                                        */
/*                                                                           */
/*    Called From: cioioctl                                                  */
/*                                                                           */
/*    Calls To: entdssetpos - configure the POS registers.                   */
/*              cio_conn_done - set status to acknowledge the start.         */
/*              ent_tx_setup - sets up TCWs & control variables for transmit */
/*              ent_recv_setup - sets up TCWs & control variables for rec    */
/*              ent_logerr   - makes error log entries for fatal errors      */
/*              d_init - Get DMA channel id                                  */
/*              d_unmask - Enable the DMA channel                            */
/*              d_clear - Free the DMA channel                               */
/*              ent_tx_undo - release transmit buffer resourses              */
/*              ent_recv_undo - release receive buffer resourses             */
/*              QUEOFFLMS - Time delay in milli-seconds                      */
/*              w_start - Start the watch dog timer                          */
/*                                                                           */
/* RETURN:  0 = OK                                                           */
/*                                                                           */
/*****************************************************************************/
int xxx_act (
   register dds_t  *dds_ptr)

{
   int    ioa;
   int    iocc;
   int    temp;
   int saved_intr_level;

   int    index;                   /* Temp variable for loop counting        */
   int    index1;                  /* Temp variable for loop counting        */
   int    i;                       /* loop counter                           */
   int    rc;                      /* Local return code                      */

   TRACE2 ("actB", (ulong)dds_ptr);   /* xxx_act begin - first start ioctl   */

   /* Test if the channel failed to be clear on last close                   */
   if (WRK.channel_alocd == FALSE)
   {

      /* get dma channel id by calling d_init                                */
      WRK.dma_channel = d_init(DDI.ds.dma_arbit_lvl,MICRO_CHANNEL_DMA,
                                  DDI.cc.bus_id);

      /* Test d_init worked ok                                               */
      if (WRK.dma_channel != DMA_FAIL)
      {
         /* go ahead and enable the dma channel by callin d_unmask           */
         d_unmask(WRK.dma_channel);

         /* Update the state of the dma_channel                              */
         WRK.channel_alocd = TRUE;
      }
      else
      {

         /* Notify the device driver that the first start is completed       */
         WRK.connection_result = (ulong)EFAULT;
         cio_conn_done(dds_ptr);

         /* Log that an error occured                                        */
         ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR5, (ulong)EFAULT,
                                                           (uchar)0, (uchar)0);

         TRACE2 ("actE", (ulong)EFAULT);   /* xxx_act end                    */

         return(0);
      }
   } /* endif test if the dma channel failed to be unallocated on last close */

   DISABLE_INTERRUPTS (saved_intr_level);

   /* Set the adapter's POS registers so driver can access the adapter       */
   entdssetpos(dds_ptr);
   CIO.promiscuous_count = 0;
   CIO.badframe_count = 0;

   /* Delete the multicast table entries                                     */
   for (i=0; i < MAX_MULTI; i++)
   {
      /* Mark each entry as being invalid                                    */
      WRK.multi_open[i] = NULL;
   }

   /* Set the number of valid entries in the table to zero                   */
   WRK.multi_count = 0;

   /* Clear the RAS counters                                                 */
   bzero (&RAS, sizeof(RAS));

   /* Clear out the execute command que                                      */
   WRK.exec_next_in  = 0;                 /* Pointer to next exec input entry*/
   WRK.exec_next_out = 0;                 /* Pointer to next exec outpt entry*/
   WRK.exec_entries  = 0;                 /* Current number of valid entries */
   WRK.exec_cmd_in_progres = FALSE;       /* Command in progress in adapter  */


   /* Get access to the I/O bus to access I/O registers                      */
   ioa = (int)BUSIO_ATT( (ulong)DDI.cc.bus_id, DDI.ds.io_port );

   /* Clear any possible pending parity error interrupt */
   PIO_PUTC( ioa + STATUS_REG, 0 );

   /* Force RAM page register to the default value                           */
   PIO_PUTC( ioa + RAM_PAGE_REG,RAM_PAGE_VALUE );

   /* Force Control register to disable adapter interrupts and turn off reset*/
   PIO_PUTC( ioa + CONTROL_REG,CONTROL_VALUE );

   ENABLE_INTERRUPTS (saved_intr_level);

   /* Test if in diagnostic mode                                             */
   if (CIO.mode == 'D')
   { /* Initialize and set the I/O diagnostic command/status queue           */

      WRK.next_in        = 0;
      WRK.next_out       = 0;
      WRK.num_entries    = 0;

      /* Update the adapter start state variable                             */
      WRK.adpt_start_state = STARTED;

      /* restore IO bus to previous value, done accessing I/O Regs           */
      BUSIO_DET(ioa);               /* restore I/O Bus                       */

      /* Notify the device driver that the first start is completed          */
      WRK.connection_result = (ulong)CIO_OK;
      cio_conn_done(dds_ptr);

      TRACE1 ("actE");             /* xxx_act end                            */

      return(0);

   } /* endif test for diagnostic mode                                       */

   /* Verify that the VPD data is good                                       */
   if (VPD.status != VPD_VALID)
   { /* Vital Product Data is bad - Fail the start                           */

      /* Test if the dma_channel was ever allocated                          */
      if (WRK.channel_alocd == TRUE)
      {
         /* Free the DMA clannel                                             */
         d_clear (WRK.dma_channel);

         /* Turn off the flag                                                */
         WRK.channel_alocd = FALSE;
      }

      /* Notify the device driver that the first start is completed          */
      WRK.connection_result = (ulong)CIO_HARD_FAIL;
      cio_conn_done(dds_ptr);

      /* Log that an error occured                                           */
      ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR1, (ulong)CIO_HARD_FAIL,
                                                           (uchar)0, (uchar)0);

      TRACE2 ("actE", (ulong)CIO_HARD_FAIL);   /* xxx_act end                */

      return(0);
   }

   /* Set up for transmit - TCWs, TX variables, etc                          */
   if ((rc = ent_tx_setup(dds_ptr)) != 0)
   { /* Something went wrong - no free TCWs                                  */

      /* Clean up TX variables and TCWs etc                                  */
      ent_tx_undo(dds_ptr);

      /* Test if the dma_channel was ever allocated                          */
      if (WRK.channel_alocd == TRUE)
      {
         /* Free the DMA clannel                                             */
         d_clear (WRK.dma_channel);

         /* Turn off the flag                                                */
         WRK.channel_alocd = FALSE;
      }

      /* Notify the device driver that the first start is completed          */
      WRK.connection_result = rc;  /* rc = ENOBUFS   no TCWs                 */
      cio_conn_done(dds_ptr);

      /* Log that an error occured                                           */
      ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR5, (ulong)rc,
                                                           (uchar)0, (uchar)0);

      TRACE2 ("actE", (ulong)rc);   /* xxx_act end                           */

      return(0);
   }


   /* Set up for handling receive                                            */
   if ((rc = ent_recv_setup(dds_ptr)))
   {
      /* Free up any resourses that were allocated for the receive           */
      ent_recv_undo(dds_ptr);

      /* Clean up TX variables and TCWs etc                                  */
      ent_tx_undo(dds_ptr);

      /* Test if the dma_channel was ever allocated                          */
      if (WRK.channel_alocd == TRUE)
      {
         /* Free the DMA clannel                                             */
         d_clear (WRK.dma_channel);

         /* Turn off the flag                                                */
         WRK.channel_alocd = FALSE;
      }

      /* Notify the device driver that the first start is completed          */
      WRK.connection_result = rc;  /* rc = ENOBUFS   no TCWs                 */
      cio_conn_done(dds_ptr);

      /* Log that an error occured                                           */
      ent_logerr(dds_ptr, (ulong)ERRID_ENT_ERR5, (ulong)rc,
                                                           (uchar)0, (uchar)0);

      TRACE2 ("actE", (ulong)rc);   /* xxx_act end                           */

      return(0);
   }

   /**************************************************************************/
   /* Start the adapter HARD reset sequence - Hard Reset is edge triggered   */
   /**************************************************************************/

   /* Hard Reset the adapter to force a known state                          */
   PIO_PUTC( ioa + CONTROL_REG,HARD_RST_MSK );

   /* get access to the POS registers */
   iocc = (int)IOCC_ATT((ulong)(DDI.cc.bus_id),
                   (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

   /* Turn off parity enable in POS REG 4 */
   temp = PIO_GETC( iocc + POS_REG_4 );
   PIO_PUTC( iocc + POS_REG_4, temp & ~PR4_PAREN_MSK );

   /* restore IOCC to previous value - done accessing POS Regs            */
   IOCC_DET(iocc);                /* restore IOCC                         */

   /* Update the adapter start state variable                                */
   WRK.adpt_start_state = HARD_RESET_ON;

   /* Set up who set the time out timer                                      */
   WRK.to_setter = TO_START;

   /* Wait for the adapter to recognize hard reset - Delay .010 seconds      */
   QUEOFFLMS (10);

   /* restore IO bus to previous value, done accessing I/O Regs              */
   BUSIO_DET(ioa);                 /* restore I/O Bus                        */

   w_start (&(WDT)); /* start watchdog timer                                 */

   /* Set the reason why the watchdog timer is being set - start connection  */
   WRK.wdt_setter = WDT_CONNECT;

   TRACE1 ("actE");                /* xxx_act end                            */

   return(0);

} /* end xxx_act                                                             */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_inact                                                           */
/*                                                                           */
/* FUNCTION: Force the adapter to become inactive on the micro channel bus.  */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: A disabled adapter by turning off card enable in the POS Reg   */
/*            Freeing up transmit TCWs and associated variables etc.         */
/*                                                                           */
/*    Called From: cioclose                                                  */
/*                                                                           */
/*    Calls To: ent_tx_undo, ent_recv_undo, d_clear, w_stop, w_start,        */
/*              e_sleep, m_dereg                                             */
/*                                                                           */
/* RETURN:  0 = OK                                                           */
/*                                                                           */
/*****************************************************************************/
int xxx_inact (
   register dds_t *dds_ptr)
{
   int    iocc;
   ulong  bus, ioa;
   int saved_intr_level;
   int    i;                       /* loop counter                           */
   int    rc;                      /* Return code value                      */

   TRACE2 ("inaB", (ulong)dds_ptr);  /* xxx_inact begin - last close issued  */

   /* Test if the device driver ever got going at all                        */
   if (WRK.adpt_start_state == NOT_STARTED)
   {
      TRACE1 ("inaE");               /* xxx_inact end   - last close issued  */

      return (0);
   }

   /* Get access to the I/O bus to access I/O registers                      */
   bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );
   ioa = (int)BUSIO_ATT(  (ulong)DDI.cc.bus_id, DDI.ds.io_port );

   DISABLE_INTERRUPTS (saved_intr_level);

   /* Release the transmit TCWs and control variables, etc                   */
   if (CIO.mode != 'D')
   {

      /* Test if the device driver ever got fully going                      */
      if (WRK.adpt_start_state == STARTED)
      {
         /* Set up null event to sleep on                                    */
         WRK.close_event = EVENT_NULL;

         /* Clear the Transmit mail box status word                          */
         PIO_PUTC( bus + WRK.xmit_mail_box + 0, 0x00 );
         PIO_PUTC( bus + WRK.xmit_mail_box + 1, 0x00 );

         /* Clear the Receive mail box status word                           */
         PIO_PUTC( bus + WRK.recv_mail_box + 0, 0x00 );
         PIO_PUTC( bus + WRK.recv_mail_box + 1, 0x00 );

         /* Issue the transmit & receive abort commands to the adapter       */
         PIO_PUTC( ioa + COMMAND_REG,(RX_ABORT | TX_ABORT) );

         /* Turn off the Watch dog timer                                     */
         w_stop (&(WDT));             /* stop the watchdog timer             */

         /* Set the reason why the wdt was set - Close adapter in progress   */
         WRK.wdt_setter = WDT_CLOSE;

         /* Turn on the Watch dog timer                                      */
         w_start (&(WDT));             /* start the watchdog timer           */

         TRACE2 ("slpB",(ulong)&(WRK.close_event)); /* Sleep begin - close   */

         /* Update the state of the adapter                                  */
         WRK.adpt_start_state = SLEEPING;

         /* Sleep until adapter causes an interrupt                          */
         rc = e_sleep( &(WRK.close_event), EVENT_SIGRET);

         ENABLE_INTERRUPTS (saved_intr_level);

         /* Turn off the Watch dog timer - it did wake up                    */
         w_stop (&(WDT));             /* stop the watchdog timer             */

         /* Reset the reason why the wdt was set - Now inactive              */
         WRK.wdt_setter = WDT_INACTIVE;

         /* Disable adapter interrupts and turn off hard reset               */
         PIO_PUTC( ioa + CONTROL_REG,CONTROL_VALUE );

         /* Disable parity enable in the parity control register             */
         PIO_PUTC( ioa + PARITY_REG,PARDS_MSK );

         /* restore IO bus to previous value, done accessing I/O Regs        */
         BUSIO_DET( ioa );               /* restore I/O Bus                  */
         BUSMEM_DET( bus );              /* restore Memory Bus               */

         /* Test if the dma_channel was ever allocated                       */
         if (WRK.channel_alocd == TRUE)
         {
            /* Clean up Receive variables and TCWs etc                       */
            ent_recv_undo(dds_ptr);
         }

         /* deregister mbuf usage                                            */
         m_dereg(&WRK.mbreq);

         /* Clean up TX variables and TCWs etc                               */
         ent_tx_undo(dds_ptr);

      } /* endif test if adapter state is equal to STARTED                   */
	else {
         	ENABLE_INTERRUPTS (saved_intr_level);
	}
   }
   else
   {

      /* Disable adapter interrupts and turn off hard reset                  */
      PIO_PUTC( ioa + CONTROL_REG,CONTROL_VALUE );

      /* Disable parity enable in the parity control register                */
      PIO_PUTC( ioa + PARITY_REG,PARDS_MSK );

      /* restore IO bus to previous value, done accessing I/O Regs           */
      BUSIO_DET(ioa);               /* restore I/O Bus                       */
      BUSMEM_DET(bus);              /* restore Memory Bus                    */

      ENABLE_INTERRUPTS (saved_intr_level);
      if (WRK.channel_alocd == TRUE)
         {
           /* Clean up Receive variables and TCWs etc                       */
           ent_recv_undo(dds_ptr);
         }
         /* Clean up TX variables and TCWs etc                               */
         ent_tx_undo(dds_ptr);

   } /* endif test for diagnostic mode */

   /* Get access to the IOCC to access POS registers                         */
   iocc = (int)IOCC_ATT((ulong)DDI.cc.bus_id,
                                       (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

   /* Update POS register 2 to turn off Card enable Bit.                     */
   WRK.pos_reg[POS_REG_2] &=  (~PR2_CDEN_MSK);

   /* Update POS register 2 to turn off Parity enable bit                    */
   WRK.pos_reg[POS_REG_2] &=  (~PR2_PEN_MSK);

   /* Update POS register 2 out on the adapter                               */
   PIO_PUTC( iocc + POS_REG_2,WRK.pos_reg[POS_REG_2] );

   /* Update POS register 4 to turn off adapter Parity enable bit            */
   WRK.pos_reg[POS_REG_4] &=  (~PR4_PAREN_MSK);

   /* Update POS register 4 to turn off select feed back interrupt enable bit*/
   WRK.pos_reg[POS_REG_4] &=  (~PR4_FDBK_MSK);

   /* Update POS register 4 out on the adapter                               */
   PIO_PUTC( iocc + POS_REG_4,WRK.pos_reg[POS_REG_4] );

   /* restore IOCC to previous value - done accessing POS Regs               */
   IOCC_DET(iocc);                /* restore IOCC                            */

   /* Test if the dma_channel was ever allocated                             */
   if (WRK.channel_alocd == TRUE)
   {

      /* Free the DMA clannel                                                */
      d_clear (WRK.dma_channel);

      /* Turn off the flag                                                   */
      WRK.channel_alocd = FALSE;

   } /* end if test if dma channel was allocated                             */

   /* Update the adapter start state variable - adapter not started          */
   WRK.adpt_start_state = NOT_STARTED;

   /* Delete the multicast table entries                                     */
   for (i=0; i < MAX_MULTI; i++)
   {
      /* Mark each entry as being invalid                                    */
      WRK.multi_open[i] = NULL;
   }

   /* Set the number of valid entries in the table to zero                   */
   WRK.multi_count = 0;
   CIO.promiscuous_count = 0;
   CIO.badframe_count = 0;

   TRACE1 ("inaE");                  /* xxx_inact end   - last close issued  */

   return (0);
} /* end xxx_inact                                                           */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_ioctl                                                           */
/*                                                                           */
/* FUNCTION: Various I/O controls that include the following:                */
/*           1) Diagnostic: POS, I/O and RAM access                          */
/*           2) Update adapter multicast addresses                           */
/*           3) Diagnostic DMA and buffer facilities                         */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Open Pointer - tells who issued the command                     */
/*           op - tells which ioctl command is being issued                  */
/*           Argument - Address of the user data                             */
/*           Dev Flag - tells if user process or kernel process              */
/*           Extension - not used                                            */
/*                                                                           */
/*    Output:  Diagnostic POS, I/O and RAM access                            */
/*             Diagnostic DMA and buffer facilities                          */
/*             Updated multicast addresses                                   */
/*                                                                           */
/*    Called From: cioioctl                                                  */
/*                                                                           */
/*    Calls To:  d_master, d_complete, bcopy, copyin, copyout, xmalloc,      */
/*               xmfree, vm_cflush                                           */
/*                                                                           */
/* RETURN:  0 = OK, EACCES, EINVAL, EFAULT, ENOTREADY, EAFNOSUPPORT, ENOSPC  */
/*                                                                           */
/*****************************************************************************/
/* handle device-specific ioctl's                                            */
int xxx_ioctl (
   register dds_t  *dds_ptr,       /* tells which adapter                    */
   register open_elem_t *open_ptr, /* tells who issued the command           */
            int        op,         /* tells which ioctl cmd is being issued  */
            int        arg,        /* Address of the user data               */
            ulong      devflag,    /* tells if user or kernel process        */
            int        ext)        /* Extension - Not used                   */

{
   struct devinfo   info;          /* IOCINFO data structure                 */
   ccc_pos_acc_t    pos_data;      /* Diagnostic POS access data structure   */
   ccc_reg_acc_t     io_data;      /* Diagnostic I/O access data structure   */
   ccc_mem_acc_t    ram_data;      /* Diagnostic RAM access data structure   */
   ent_dma_buf_t     dma_buf;      /* Diagnostic DMA access data structure   */
   ent_set_multi_t set_multi;      /* Update Multicast Address data structure*/
   char          *dma_addr;        /*                                        */
   struct xmem      dp;            /* Cross memory descriptor                */
   struct xmem      att_xmem;      /* Cross memory descriptor for attach     */
   int    iocc;                    /* POS access variable                    */
   int    saved_intr_level;        /* Interrupt disable variable             */
   int    bus, ioa;                /* I/O access variables                   */
   int    i;                       /* Loop Counter                           */
   int    j;                       /* Loop Counter                           */
   int    multi_delta;             /* Multicast ioctl delta made flag        */
   int    rc;
   void   xxx_newtbl();

   TRACE5 ("iocB",(ulong)dds_ptr,(ulong)open_ptr,(ulong)op,(ulong)arg);
   TRACE3 ("ioc+",(ulong)devflag,(ulong)ext); /* IOCTL begin                 */

   switch (op)
   {

      /***********************************************************************/
      /*  ioctl - IOCINFO                                                    */
      /***********************************************************************/
      case IOCINFO:
      {
         TRACE2 ("icfB", (ulong)dds_ptr); /* Start ioctl IOCINFO             */

         /* test if there is a place for this data to be placed              */
         if (!arg) return(EFAULT);

         /* adapter does not wrap packets                                    */
         info.un.ethernet.broad_wrap = (uint)0;

         /* Update the Receive Data Transfer Offset value - RDTO             */
         info.un.ethernet.rdto = WRK.rdto;

         /* Load the network addresses                                       */
         for (i=0; i < ent_NADR_LENGTH; i++)
         {
            /* Set the network address that is obtained from VPD             */
            info.un.ethernet.haddr[i] = WRK.ent_vpd_addr[i];

            /* Set the currently used network address                        */
            info.un.ethernet.net_addr[i] = WRK.ent_addr[i];
         }

         /* Load the info types and and sub types                            */
         info.devtype    = (char)DD_NET_DH;
         info.flags      = (char)NULL;
         info.devsubtype = (char)DD_EN;

         /* Determine how to return this data to the caller                  */
         if (devflag & DKERNEL)
         {
            /* Caller is in the kernel - use bcopy                           */
            bcopy(&info, arg, sizeof(info));
         }
         else
         {
            /* Caller is a user process - use copyout                        */
            if (copyout(&info, arg, sizeof(info)) != 0)
            {
                return(EFAULT);
            }
         } /* endif test for kernel or user process caller                   */

         TRACE1 ("icfE");

         return (0);                      /* End   ioctl IOCINFO             */
      }

      /***********************************************************************/
      /*  ioctl - Query RAS Counters                                         */
      /***********************************************************************/
      case CIO_QUERY:
      {
         TRACE2 ("qryB", (ulong)dds_ptr); /* Query RAS Counters begin       */

	 WRK.first_flag++;
         entrdcounts (dds_ptr);

         TRACE1 ("qryE");                 /* Query RAS Counters end         */

         return (0);
      }

      /***********************************************************************/
      /*  Diagnostic ioctl - Access POS Registers                            */
      /***********************************************************************/
      case CCC_POS_ACC:
      {

         TRACE2 ("posB",(ulong)dds_ptr);  /* POS Reg access begin - dds_ptr  */
#if 0
         /* Get Access to the ioctl parameters - User space or Kernel space  */
         if (rc=COPYIN(devflag, arg, &pos_data, sizeof(pos_data)))
         {
            TRACE2 ("posE", (ulong)rc);   /* POS Reg access ioctl end        */
            return (rc);
         }
#endif
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy ((void *)arg, &pos_data, sizeof(pos_data));
         }
         else
         {
            /* User process copy                                             */
            if (copyin ((void *)arg, &pos_data, sizeof(pos_data)) != 0)
            {
                TRACE2 ("posE",(ulong)EFAULT); /* POS Access ioctl end       */

                return (EFAULT);
            }
         }

         /* Initialize status field in structure                             */
         pos_data.status = (ulong)CIO_OK;


         /* Test to verify diagnostic mode and started                       */
#ifdef DEBUG
         if (((CIO.mode == 'D') &&
              (WRK.adpt_start_state == STARTED)) ||
              ((pos_data.opcode == (ushort)CCC_READ_OP) &&
               (WRK.adpt_start_state == STARTED)))
#else
         if ((CIO.mode == 'D') &&
             (WRK.adpt_start_state == STARTED))
#endif
         {  /* This is diagnostic mode - test for valid ranges               */

            /* Test if valid POS register range and valid opcode             */
            if ((pos_data.pos_reg > POS_REG_7) ||
 /*             (pos_data.pos_reg < POS_REG_0) || - redundant, uchar defined */
               ((pos_data.opcode != CCC_READ_OP) &&
                (pos_data.opcode != CCC_WRITE_OP)))
            {
               pos_data.status = (ulong)CCC_BAD_RANGE;
            }
            else
            {  /* Valid ranges and opcodes - test for read or write          */

               /* Get access to the IOCC to access POS registers             */
               iocc = (int)IOCC_ATT((ulong)DDI.cc.bus_id,
                                       (ulong)(IO_IOCC + (DDI.ds.slot << 16)));

               /* Determine whether to do a read or a write                  */
               if (pos_data.opcode == (ushort)CCC_READ_OP)
               {

                  TRACE2 ("posR",(ulong)pos_data.pos_reg); /* POS Read-reg # */

                  /* Read the selected POS register and save it              */
                  pos_data.pos_val = PIO_GETC( iocc + pos_data.pos_reg );

               }
               else
               {

                  TRACE2 ("posW",(ulong)pos_data.pos_reg); /*POS Write-reg # */

                  /* Write the selected POS register with data provided      */
                  PIO_PUTC( iocc + pos_data.pos_reg,pos_data.pos_val );

               } /* end if for read/write test                               */

               /* Perform dummy read for adptr gate array problem to subside */
               WRK.gate_array_fix =
                         PIO_GETC( (caddr_t)((ulong)iocc | (ulong)0x000f0000));

               TRACE2 ("posV",(ulong)pos_data.pos_val); /* POS Access - value*/

               /* restore IOCC to previous value - done accessing adapter    */
               IOCC_DET(iocc);                 /* restore IOCC               */

            } /* endif for range test                                        */
         }
         else
         {
            pos_data.status = (ulong)CCC_NOT_DIAG_MODE;
         } /* endif for diagnostic mode test                                 */

#if 0
         /* Restore the ioctl parameters - User space or Kernel space        */
         if (rc=COPYOUT(devflag, &pos_data, arg, sizeof(pos_data)))
         {
            TRACE2 ("posE", (ulong)rc);   /* POS Reg access ioctl end        */
            return (rc);
         }
#endif

         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy (&pos_data, (void *)arg, sizeof(pos_data));

         }
         else
         {
            /* User process copy                                             */
            if (copyout (&pos_data, (void *)arg, sizeof(pos_data)) != 0)
            {
                TRACE2 ("posE",(ulong)EFAULT); /* POS Access end - error     */

                return (EFAULT);
            }
         }

         /* Test for which error to return                                   */
         if (pos_data.status == (ulong)CIO_OK)
         {
            TRACE2 ("posE",0); /* POS Register access end - ok               */

            return (0);
         }
         else
         {
            TRACE2 ("posE",(ulong)EINVAL);  /* POS Access end - error        */

            return (EINVAL);
         }
      } /* End of diagnostic POS access                                      */

      /***********************************************************************/
      /*  Diagnostic ioctl - Access I/O Registers                            */
      /***********************************************************************/
      case CCC_REG_ACC:
      {

         TRACE2 ("i/oB",(ulong)dds_ptr); /* I/O Reg access begin - dds_ptr   */

         /* Get Access to the ioctl parameters - User space or Kernel space  */
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy ((void *)arg, &io_data, sizeof(io_data));
         }
         else
         {
            /* User process copy                                             */
            if (copyin ((void *)arg, &io_data, sizeof(io_data)) != 0)
            {
                TRACE2 ("i/oE",(ulong)EFAULT); /* I/O Reg Access - error     */

                return (EFAULT);
            }
         }

         /* Initialize status field in structure                             */
         io_data.status = (ulong)CIO_OK;

         /* Test to verify diagnostic mode                                   */
         if ((CIO.mode != 'D') ||
             (WRK.adpt_start_state != STARTED))
         {
            io_data.status = (ulong)CCC_NOT_DIAG_MODE;
         }
         else
         {  /* This is diagnostic mode - test for valid ranges               */

            /* Test if valid I/O register range and valid opcode             */
            if ((io_data.io_reg > PARITY_REG) ||
 /*             (io_data.io_reg < COMMAND_REG) || redundant, defined as uchar*/
               ((io_data.opcode != CCC_READ_OP) &&
                (io_data.opcode != CCC_READ_Q_OP) &&
                (io_data.opcode != CCC_WRITE_OP)))
            {
               /* Bad parameters sent - Update status with error             */
               io_data.status = (ulong)CCC_BAD_RANGE;
            }
            else
            {  /* Valid ranges and opcodes - test for read or write          */

               if ((io_data.opcode == (ushort)CCC_READ_OP) ||
                   (io_data.opcode == (ushort)CCC_WRITE_OP))
               {
                  /* Get access to the I/O bus to access I/O registers       */
                  ioa = (int)BUSIO_ATT( (ulong)DDI.cc.bus_id, DDI.ds.io_port );

                  /* Determine whether to do a read or a write               */
                  if (io_data.opcode == (ushort)CCC_READ_OP)
                  {

                     TRACE2 ("i/oR",(ulong)io_data.io_reg); /* I/O Read-reg# */

                     /* Read the selected I/O register and save it           */
                     io_data.io_val = PIO_GETC( ioa + io_data.io_reg );

                  }
                  else
                  {

                     TRACE2 ("i/oW",(ulong)io_data.io_reg); /* I/O Write-reg#*/

                     /* Write the selected I/O register with data provided   */
                     PIO_PUTC( ioa + io_data.io_reg,io_data.io_val );

                  } /* end if for read/write test                            */

                  TRACE2 ("i/oV",(ulong)io_data.io_val);  /* I/O Value       */

                  /* restore IO bus to previous value, done accessing I/O Reg*/
                  BUSIO_DET(ioa);                 /* restore I/O Bus         */
               }
               else
               { /* this is a command/status queue read - test ranges        */

                  if ((io_data.io_reg != (uchar)COMMAND_REG) &&
                      (io_data.io_reg != (uchar)STATUS_REG))
                  {
                     /* Bad parameters sent - Update status with error       */
                     io_data.status = (ulong)CCC_BAD_RANGE;
                  }
                  else
                  {
                     /* Disable interrupts to manipulate the queue           */
                     DISABLE_INTERRUPTS (saved_intr_level);

                     /* Test if the queue is empty                           */
                     if (WRK.num_entries)
                     {
                        /* Test for which register is to be read             */
                        if (io_data.io_reg == (uchar)STATUS_REG)
                        {
                           /* Read and save the status register              */
                           io_data.io_val = WRK.io_stat_q[WRK.next_out];

                           /* Save the number of entries                     */
                           io_data.io_status = WRK.num_entries;
                        }
                        else
                        {
                           /* Read and save the command register             */
                           io_data.io_val   = WRK.io_cmd_q[WRK.next_out];

                           /* Read and save the status register              */
                           io_data.io_val_o = WRK.io_stat_q[WRK.next_out];

                           /* Increment the next out pointer                 */
                           if (++WRK.next_out >= MAX_DIAG_QUEUED)
                              WRK.next_out = 0;

                           /* Save the number of entries then decrement      */
                           io_data.io_status = WRK.num_entries--;

                        } /* endif test for which register to read           */
                     }
                     else
                     { /* Diagnostic Command Queue is empty - post it        */
                        io_data.status = (ulong)CCC_QUE_EMPTY;
                     } /* endif test for Diagnostic queue empty              */

                     /* Enable interrupts again                              */
                     ENABLE_INTERRUPTS (saved_intr_level);

                  } /* endif test for valid que registers to read            */
               } /* endif test if I/O Bus access is required                 */
            } /* endif for range test                                        */
         } /* endif for diagnostic mode test                                 */

         /* Restore the ioctl parameters - User space or Kernel space        */
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy (&io_data, (void *)arg, sizeof(io_data));
         }
         else
         {
            /* User process copy                                             */
            if (copyout (&io_data, (void *)arg, sizeof(io_data)) != 0)
            {
                TRACE2 ("i/oE",(ulong)EFAULT); /* I/O Reg access - error     */

                return (EFAULT);
            }
         }

         /* Test for which error to return                                   */
         if (io_data.status == (ulong)CIO_OK)
         {
            TRACE1 ("i/oE");    /* I/O Register ioctl access end             */
            return (0);
         }
         else
         {
            TRACE2 ("i/oE",(ulong)EINVAL); /* I/O Register ioctl access end  */
            return (EINVAL);
         }
      } /* End access I/O registers diagnostic ioctl                         */

      /***********************************************************************/
      /*  Diagnostic ioctl - Access Adapter RAM                              */
      /***********************************************************************/
      case CCC_MEM_ACC:
      {

         TRACE2 ("ramB",(ulong)dds_ptr); /* RAM Access ioctl begin - dds_ptr */

         /* Get Access to the ioctl parameters - User space or Kernel space  */
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy ((void *)arg, &ram_data, sizeof(ram_data));
         }
         else
         {
            /* User process copy                                             */
            if (copyin ((void *)arg, &ram_data, sizeof(ram_data)) != 0)
            {
                TRACE2 ("ramE",(ulong)EFAULT); /* RAM access end - error     */

                return (EFAULT);
            }
         }

         /* Initialize status field in structure                             */
         ram_data.status = (ulong)CIO_OK;

         /* Test to verify diagnostic mode and started                       */
#ifdef DEBUG
         if (((CIO.mode == 'D') &&
              (WRK.adpt_start_state == STARTED)) ||
              ((ram_data.opcode == (ushort)CCC_READ_OP) &&
               (WRK.adpt_start_state == STARTED)))
#else
         if ((CIO.mode == 'D') &&
             (WRK.adpt_start_state == STARTED))
#endif
         {  /* This is diagnostic mode or read - test for valid ranges       */

            /* Trace RAM offset and length                                   */
            TRACE3 ("ramO",(ulong)ram_data.ram_offset,(ulong)ram_data.length);

            /* Test if valid RAM Address range and valid opcode              */
            if ((ram_data.ram_offset > (RAM_SIZE - 1)) ||
                (ram_data.ram_offset + ram_data.length > (RAM_SIZE - 1)) ||
               ((ram_data.opcode != (ushort)CCC_READ_OP) &&
                (ram_data.opcode != (ushort)CCC_WRITE_OP)))
            {
               /* Bad parameters sent - Update status with error             */
               ram_data.status = (ulong)CCC_BAD_RANGE;

            }
            else
            {  /* Valid ranges and opcodes - test for read or write          */


               /* Get access to the I/O bus to access on card RAM            */
               bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id,
                                             DDI.ds.bus_mem_addr );

               /* Determine whether to do a read or a write                  */
               if (ram_data.opcode == (ushort)CCC_READ_OP)
               {

                  TRACE1 ("ramR");    /* RAM Access Read                     */

                  /* Read the selected RAM space and save it                 */
                  /* Determine user space or Kernel space                    */
                  if (devflag & DKERNEL)
                  {
                     /* Kernel Process copy                                  */
                     for (i = 0; i < ram_data.length; i++)
                        ram_data.buffer[i] =
                            PIO_GETC( bus + ram_data.ram_offset + i );
                  }
                  else
                  {
                     /* User process copy                                    */
                     for (i = 0; i < ram_data.length; i++)

                        if (subyte( &ram_data.buffer[i],
                                PIO_GETC( bus + ram_data.ram_offset + i )) < 0)
                        {
                            TRACE4 ("ramM",
                            (ulong)bus + ram_data.ram_offset + i,
                            (ulong)&ram_data.buffer[i],
                            (ulong)ram_data.length);

                            TRACE2 ("ramE",(ulong)EFAULT); /* RAM Access end */

                            return ( (int)EFAULT);
                        }

                  } /* endif test for which type of process made the call    */

               }
               else
               {

                  TRACE1 ("ramW");   /* RAM access Write                     */

                  /* Write the selected RAM space with data provided         */
                  /* Determine user space or Kernel space                    */
                  if (devflag & DKERNEL)
                  {
                     /* Kernel Process copy                                  */
                     for (i = 0; i < ram_data.length; i++)
                        PIO_PUTC( bus + ram_data.ram_offset + i,
                                        ram_data.buffer[i] );
                  }
                  else
                  {
                     /* User process copy                                    */
                     for (i = 0; i < ram_data.length; i++)
                     {
                        if (( rc = fubyte( &ram_data.buffer[i] )) < 0)
                        {
                           TRACE2 ("ramE",(ulong)EFAULT);   /* Access error  */
                           return (EFAULT);
                        }
                        else
                        {
                           PIO_PUTC(bus + ram_data.ram_offset + i, (uchar)rc);
                        }

                     } /* end for loop for process copy                      */

                  } /* endif test for user space or kernel space             */

               } /* end if for read/write test                               */

               /* restore IO bus to previous value, done accessing I/O Regs  */
               BUSMEM_DET(bus);                /* restore I/O Bus            */

            } /* endif for range test                                        */

         }
         else
         {
            ram_data.status = (ulong)CCC_NOT_DIAG_MODE;
         } /* endif for diagnostic mode test                                 */

         /* Restore the ioctl parameters - User space or Kernel space        */
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy (&ram_data, (void *)arg, sizeof(ram_data));

         }
         else
         {
            /* User process copy                                             */
            if (copyout (&ram_data, (void *)arg, sizeof(ram_data)) != 0)
            {
                TRACE4 ("ramU", (ulong)&ram_data, (ulong)arg,
                (ulong)sizeof(ram_data));

                TRACE2 ("ramE",(ulong)EFAULT);  /* RAM Access end - error    */

                return ( (int)EFAULT);

            } /* end if test for copyout working correctly                   */

         } /* end if test for which type of process                          */

         /* Test for which error to return                                   */
         if (ram_data.status == (ulong)CIO_OK)
         {
            TRACE1 ("ramE");      /* RAM Access ioctl end                    */

            return (0);
         }
         else
         {
            TRACE2 ("ramE",(ulong)EINVAL); /* RAM Access ioctl end - error   */

            return ( (int)EINVAL);

         } /* endif test for which type of return code to send               */

      } /* End access adapter RAM ioctl                                      */

      /***********************************************************************/
      /*  Diagnostic ioctl - DMA Facility                                    */
      /***********************************************************************/
      case ENT_LOCK_DMA:
      case ENT_UNLOCK_DMA:
      {
         dp.aspace_id   = (int)XMEM_GLOBAL;
         dp.subspace_id = (int)NULL;

         TRACE2 ("dmaB", (ulong)dds_ptr); /* DMA Ioctl begin - dds_ptr       */

         /* Test to verify diagnostic mode                                   */
         if ((CIO.mode != 'D') ||
             (WRK.adpt_start_state != STARTED))
         {
            TRACE2 ("dmaE", (ulong)EINVAL); /* DMA ioctl end - error         */
            return ( (int)EINVAL);
         }

         /* Get Access to the ioctl parameters - User space or Kernel space  */
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy ((void *)arg, &dma_buf, sizeof(dma_buf));
         }
         else
         {
            /* User process copy                                             */
            if (copyin ((void *)arg, &dma_buf, sizeof(dma_buf)) != 0)
            {
               TRACE2 ("dmaE",(ulong)EFAULT);/* DMA ioctl end - error        */
               return ( (int)EFAULT);
            }
         }

         TRACE4 ("dmaI",(ulong)dma_buf.p_bus,   /* DMA input data - DMA @    */
                        (ulong)dma_buf.p_user,  /* User space address        */
                        (ulong)dma_buf.length); /* DMA Length                */


         /********************************************************************/
         /* Test if lock of user Buffer                                      */
         /********************************************************************/
         if (op == ENT_LOCK_DMA)
         {
            TRACE2 ("dmaL", (ulong)dma_buf.p_user); /* DMA Allocate - buffer */

            /*  If there is already a dma buffer locked -- return an error   */
            if (WRK.dma_locked)
            {
                TRACE2 ("dmaE", (ulong)EINVAL);
                return( (int)EINVAL );
            }

            /*  Attempt to attach the user buffer for dma access --          */
            /*  this "locks" the user buffer so that the driver can          */
            /*  access it while not executing under the process.             */
            WRK.dma_xatt.aspace_id   = (int)XMEM_INVAL;
            WRK.dma_xatt.subspace_id = (int)NULL;

            if ((xmattach( (char *)dma_buf.p_user, (int)dma_buf.length,
                            &WRK.dma_xatt, (int)USER_ADSPACE)) !=
                                                                (int)XMEM_SUCC)
            {
                TRACE2 ("dmaE", (ulong)EFAULT); /* DMA ioctl end             */
                return( (int)EFAULT );
            }

            /* Pin the buffer                                                */
            rc = pin( dma_buf.p_user, dma_buf.length);

            /* Check for results of pinning the buffer                       */
            if (rc != PIN_SUCC)
            {
                TRACE2 ("dmaE", (ulong)rc);    /* DMA ioctl end - pin failed */
                return( (int)rc );
            }

            WRK.dma_locked = TRUE;             /* mark as locked             */

            /* Set up for initializing a block mode DMA Transfer             */
            dma_buf.p_bus = (ulong)(WRK.dma_base);

            /* Initializes a block mode DMA transfer for a DMA master        */
            d_master (WRK.dma_channel, 0, (char *)dma_buf.p_user,
                              (int)dma_buf.length, &dp, (char *)dma_buf.p_bus);

            TRACE4 ("dmaO",
                        (ulong)dma_buf.p_bus,   /* DMA Output data - DMA @   */
                        (ulong)dma_buf.p_user,  /* User space address        */
                        (ulong)dma_buf.length); /* DMA Length                */
         }

         /********************************************************************/
         /* Test if unlock DMA Buffer                                        */
         /********************************************************************/
         if (op == ENT_UNLOCK_DMA)
         {
            TRACE2 ("dmaU",(ulong)dma_buf.p_user); /* DMA Unlock - buffer @  */

            /*  If there is no dma buffer to unlock, return an error.        */
            if (!WRK.dma_locked)
            {
                TRACE2 ("dmaE",(ulong)EINVAL);
                return( (int)EINVAL );
            }

            if (d_complete(WRK.dma_channel, 0, (char *)dma_buf.p_user,
                            (int)dma_buf.length, &dp,
                            (char *)dma_buf.p_bus) != DMA_SUCC)
            {

               TRACE2 ("dmaE",(ulong)EFAULT);   /* DMA ioctl end             */
               return ( (int)EFAULT);
            }

            /* Unpin the buffer                                              */
            if ((rc = unpin( dma_buf.p_user, dma_buf.length)) != UNPIN_SUCC)
            {
                TRACE2 ("dmaE",(ulong)rc);   /* DMA ioctl end - unpin failed */
                return( (int)rc );
            }

            /*  Attempt to deattach the previously attached buffer;          */
            /*  this "unlocks" the user buffer.                              */

            /* WRK.dma_xatt.aspace_id   = XMEM_INVAL;                        */
            /* WRK.dma_xatt.subspace_id = NULL;                              */

            if (xmdetach( &WRK.dma_xatt ) != XMEM_SUCC)
            {
                TRACE2 ("dmaE",(ulong)EFAULT); /* DMA ioctl end              */
                return( (int)EFAULT );
            }

            WRK.dma_locked = FALSE;            /* mark as unlocked           */

            TRACE4 ("dmaO",
                        (ulong)dma_buf.p_bus,   /* DMA Output data - DMA @   */
                        (ulong)dma_buf.p_user,  /* User space address        */
                        (ulong)dma_buf.length); /* DMA Length                */
         }

         /* Restore the ioctl parameters - User space or Kernel space        */
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy (&dma_buf, (void *)arg, sizeof(dma_buf));

         }
         else
         {
            /* User process copy                                             */
            if (copyout (&dma_buf, (void *)arg, sizeof(dma_buf)) != 0)
            {
                TRACE2 ("dmaE",(ulong)EFAULT);   /* DMA ioctl end            */

                return ((int)EFAULT);
            }
         }

         TRACE1 ("dmaE");                        /* DMA ioctl end            */

         return ( (int)0);
      }

      /***********************************************************************/
      /*             ioctl - Set ethernet Multicast address                  */
      /***********************************************************************/
      case ENT_SET_MULTI:
      {

         TRACE2 ("mulB",(ulong)dds_ptr); /* Set Multicast IOCTL begin        */

         /* Test if in Diagnostic mode                                       */
         if (CIO.mode == 'D')             /* Test NOT in diagnostic mode     */
         {

           TRACE2 ("mulE",(ulong)EACCES); /* Set Multicast IOCTL end         */

           return ((int)EACCES);
         }

         /* Test to make sure the adapter has started                        */
         if (WRK.adpt_start_state != STARTED)  /* Test if interface started  */
         {

           TRACE2 ("mulE",(ulong)ENOTREADY); /* Set Multicast IOCTL end      */

           return ((int)ENOTREADY);
         }

         /* Get Access to the ioctl parameters - User space or Kernel space  */
         if (devflag & DKERNEL)
         {
            /* Kernel Process copy                                           */
            bcopy ((void *)arg, &set_multi, sizeof(set_multi));
         }
         else
         {
            /* User process copy                                             */
            if (copyin ((void *)arg, &set_multi, sizeof(set_multi)) != 0)
            {
                TRACE2 ("mulE",(ulong)EFAULT); /* Set Multicast IOCTL end    */
                return ((int)EFAULT);
            }
         }

         /* Test if valid opcode sent                                        */
         if ((set_multi.opcode != (ushort)ENT_ADD) &&
              (set_multi.opcode != (ushort)ENT_DEL))
         {

            TRACE2 ("mulE",(ulong)EINVAL); /* Set Multicast IOCTL end        */

            return ((int)EINVAL);
         }

         /* Test if multicast bit is set                                     */
         if  ((set_multi.multi_addr[0] & MULTI_BIT_MASK) != MULTI_BIT_MASK)
         {

            TRACE2 ("mulE",(ulong)EAFNOSUPPORT); /* Multicast IOCTL end      */

            return ((int)EAFNOSUPPORT);
         }
         else
         {  /* Valid ranges and opcodes - test for add or delete address     */

            /* Initialize flag to indicate if a change to adapter is needed  */
            multi_delta = FALSE;

            /* Disable Interrupts                                            */
            DISABLE_INTERRUPTS (saved_intr_level);

            /* Determine whether to do an add or a delete                    */
            if (set_multi.opcode == (ushort)ENT_ADD)
            {

               TRACE1 ("mulA");   /* Set Multicast ioctl ADD address         */

               /* Test if there is room to add one more to list              */
               if (WRK.multi_count >= MAX_MULTI)
               {

                  /* Enable interrupts again                                 */
                  ENABLE_INTERRUPTS (saved_intr_level);

                  TRACE2 ("mulE",(ulong)ENOSPC); /* Set Multicast IOCTL end  */

                  return ((int)ENOSPC);
               }

               /* Find the next open spot in the stored multicast array      */
               for (i=0; i < MAX_MULTI; i++)
               {

                  /* Test if this entry is available to be loaded with new @ */
                  if (WRK.multi_open[i] == NULL)
                  { /* Update the mutlicast table                            */

                     /* Copy the current address to the work section table   */
                     for (j=0; j < ent_NADR_LENGTH; j++)
                     {
                        WRK.multi_list[i][j] = set_multi.multi_addr[j];
                     }

                     /* Make this entry valid                                */
                     WRK.multi_open[i] = open_ptr;

                     /* Increment the number of valid entries in the table   */
                     WRK.multi_count++;

                     /* Delta change required for this adapter               */
                     multi_delta = TRUE;

                     /* Done with this for loop                              */
                     break;

                  } /* Endif test for empty element found                    */
               } /* end for loop looking for empty element                   */
            }
            else
            { /* Delete multicast address was decoded - Do it!               */

               TRACE1 ("mulD");   /* Set Multicast ioctl DELETE address      */

               /* Match the current address to table and invalidate entry    */
               /* Set up for loop to test all addresses in the table         */
               for (i=0; multi_delta != TRUE && i < MAX_MULTI; i++)
               {

                  /* Test if the array entry is a valid address              */
                  if (WRK.multi_open[i] == open_ptr)
                  {

                     /* Set up for loop to scan each byte of multicast addres*/
                     for (j=0; j < ent_NADR_LENGTH; j++)
                     {

                        /* Test if the individual address byte match         */
                        if (WRK.multi_list[i][j] == set_multi.multi_addr[j])
                        {

                           /* Test if this is last byte of address - this    */
                           /*   implies that complete address compare        */
                           if (j == (ent_NADR_LENGTH - 1))
                           {

                              /* Invalidate the entry                        */
                              WRK.multi_open[i] = NULL;
                              WRK.multi_count--;

                              /* Delta change required for this adapter      */
                              multi_delta = TRUE;

                              /* Break out of this for loop - address found  */
                              break;
                           } /* endif test if is the last byte of address    */
                        }
                        else
                        {
                           /* Not a match - get out of this for loop         */
                           break;

                        } /* endif test address byte match                   */
                     } /* end for loop for address byte match                */
                  } /* endif test for valid array entry address              */
               } /* end for loop for complete address match                  */
            } /* end if for add/delete test                                  */

            /* Enable interrupts again                                       */
            ENABLE_INTERRUPTS (saved_intr_level);

            /* Test if a delta needs to be given to the adapter              */
            if (multi_delta == TRUE)
            {

               /* Give the updated multicast table to the adapter            */
               entexecque(dds_ptr, NULL, (uchar)SET_MULTICAST);

               TRACE1 ("mulE");    /* Set Multicast IOCTL end                */

               return ( (int)0);
            }
            else
            { /* Something went wrong with the add or delete - report error  */

               TRACE2 ("mulE", (ulong)EFAULT); /* Set Multicast IOCTL end    */

               return ( (int)EFAULT);

            } /* endif  test to delta change required                        */

         } /* endif for range test                                           */

      } /* end case entry for set multicast ioctl                            */


      /***********************************************************************/
      /*             ioctl - Set ethernet Multicast address                  */
      /***********************************************************************/
      case ENT_PROMISCUOUS_ON:
      {

         TRACE2 ("pmon",(ulong)dds_ptr); /* Set Multicast IOCTL begin     */

         /* Test if in Diagnostic mode                                       */
         if (CIO.mode == 'D')             /* Test NOT in diagnostic mode     */
         {

           TRACE2 ("pmon",(ulong)EACCES); /* Set Multicast IOCTL end      */

           return ((int)EACCES);
         }

         /* Test to make sure the adapter has started                        */
         if (WRK.adpt_start_state != STARTED)  /* Test if interface started  */
         {

           TRACE2 ("pmon",(ulong)ENOTREADY); /* Set Multicast IOCTL end   */

           return ((int)ENOTREADY);
         }
       
         if (!(devflag & DKERNEL))
         {
           TRACE1 ("pmon");
           return ((int)EACCES);
         }

         /* Disable Interrupts                                            */
         DISABLE_INTERRUPTS (saved_intr_level);

         open_ptr->prom_on_cnt++;
         CIO.promiscuous_count++;

         /* Enable interrupts again                                       */
         ENABLE_INTERRUPTS (saved_intr_level);

         if (CIO.promiscuous_count == 1) {
		/* disable the type filter on the adapter 		  */
		/* and turn on the promiscuous mode			  */
		xxx_newtbl(dds_ptr);		
                entexecque (dds_ptr, NULL, (uchar)CONFIGURE);
	 }


         TRACE1 ("pmon");
         break;
      }

      /***********************************************************************/
      /*             ioctl - Set ethernet Multicast address                  */
      /***********************************************************************/
      case ENT_PROMISCUOUS_OFF:
      {

         TRACE2 ("pmof",(ulong)dds_ptr); /* Set Multicast IOCTL begin     */

         /* Test if in Diagnostic mode                                       */
         if (CIO.mode == 'D')             /* Test NOT in diagnostic mode     */
         {

           TRACE2 ("pmof",(ulong)EACCES); /* Set Multicast IOCTL end      */

           return ((int)EACCES);
         }

         /* Test to make sure the adapter has started                        */
         if (WRK.adpt_start_state != STARTED)  /* Test if interface started  */
         {

           TRACE2 ("pmof",(ulong)ENOTREADY); /* Set Multicast IOCTL end   */

           return ((int)ENOTREADY);
         }

         if (!(devflag & DKERNEL))
         {
           TRACE1 ("pmof");
           return ((int)EACCES);
         }


         if (!open_ptr->prom_on_cnt)
                return ((int)EINVAL);

         /* Disable Interrupts                                            */
         DISABLE_INTERRUPTS (saved_intr_level);

         open_ptr->prom_on_cnt--;
         CIO.promiscuous_count--;

         /* Enable interrupts again                                       */
         ENABLE_INTERRUPTS (saved_intr_level);

         if (CIO.promiscuous_count == 0) {
		/* re-set the type filter on the adapter 		  */
		/* and turn off the promiscuous mode			  */
		xxx_newtbl(dds_ptr);		
                entexecque (dds_ptr, NULL, (uchar)CONFIGURE);
 	 }
         TRACE1 ("pmof");
         break;
      }


      /***********************************************************************/
      /*             ioctl - Set ethernet Multicast address                  */
      /***********************************************************************/
      case ENT_BADFRAME_ON:
      {

         TRACE2 ("bfon",(ulong)dds_ptr); /* Set Multicast IOCTL begin    */

         /* Test if in Diagnostic mode                                       */
         if (CIO.mode == 'D')             /* Test NOT in diagnostic mode     */
         {

           TRACE2 ("bfon",(ulong)EACCES); /* Set Multicast IOCTL end      */

           return ((int)EACCES);
         }

         /* Test to make sure the adapter has started                        */
         if (WRK.adpt_start_state != STARTED)  /* Test if interface started  */
         {

           TRACE2 ("bfon",(ulong)ENOTREADY); /* Set Multicast IOCTL end   */

           return ((int)ENOTREADY);
         }

         if (!(devflag & DKERNEL))
         {
           TRACE1 ("bfon");
           return ((int)EACCES);
         }

         /* Disable Interrupts                                            */
         DISABLE_INTERRUPTS (saved_intr_level);

         open_ptr->badframe_on_cnt++;
         CIO.badframe_count++;

         /* Enable interrupts again                                       */
         ENABLE_INTERRUPTS (saved_intr_level);

         if (CIO.badframe_count == 1) {
		/* disable the type filter on the adapter 		  */
		/* and turn on the bad frame  mode			  */
		xxx_newtbl(dds_ptr);		
                entexecque (dds_ptr, NULL, (uchar)CONFIGURE);
	 }

         TRACE1 ("bfon");
         break;
      }

      /***********************************************************************/
      /*             ioctl - Set ethernet Multicast address                  */
      /***********************************************************************/
      case ENT_BADFRAME_OFF:
      {

         TRACE2 ("bfof",(ulong)dds_ptr); /* Set Multicast IOCTL begin   */

         /* Test if in Diagnostic mode                                       */
         if (CIO.mode == 'D')             /* Test NOT in diagnostic mode     */
         {

           TRACE2 ("bfof",(ulong)EACCES); /* Set Multicast IOCTL end    */

           return ((int)EACCES);
         }

         /* Test to make sure the adapter has started                        */
         if (WRK.adpt_start_state != STARTED)  /* Test if interface started  */
         {

           TRACE2 ("bfof",(ulong)ENOTREADY); /* Set Multicast IOCTL end */

           return ((int)ENOTREADY);
         }

         if (!(devflag & DKERNEL))
         {
           TRACE1 ("bfof");
           return ((int)EACCES);
         }


         if (!open_ptr->badframe_on_cnt)
                return ((int)EINVAL);

         /* Disable Interrupts                                            */
         DISABLE_INTERRUPTS (saved_intr_level);

         open_ptr->badframe_on_cnt--;
         CIO.badframe_count--;

         /* Enable interrupts again                                       */
         ENABLE_INTERRUPTS (saved_intr_level);

         if (CIO.badframe_count == 0) {
		/* re-set the type filter on the adapter 		  */
		/* and turn off the bad frame  mode			  */
		xxx_newtbl(dds_ptr);		
                entexecque (dds_ptr, NULL, (uchar)CONFIGURE);
	 }

         TRACE1 ("bfof");
         break;
      }


      default:
      {
         TRACE2 ("iocE", (ulong)EINVAL);  /* IOCTL end                       */

         return ( (int)EINVAL);
      }
   } /* end ioctl switch statement                                           */

   TRACE1 ("iocE");                  /* IOCTL end                            */

   return ( (int)0);

} /* end xxx_ioctl                                                           */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_newtbl                                                          */
/*                                                                           */
/* FUNCTION: Queue up an execute command to update the netid table.          */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*                                                                           */
/*    Output: Execute command Set Type queued.                               */
/*                                                                           */
/*    Called From: cioclose, cioioctl, cio_conn_done                         */
/*                                                                           */
/*    Calls To: entexecque                                                   */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void xxx_newtbl (
   register dds_t     *dds_ptr) /* tells which adapter                       */
{
   ulong  bus;
   ulong  byte_1_type;          /* Flag to tell if a 1 byte type field found */
   ulong  byte_2_type;          /* Flag to tell if a 2 byte type field found */
   int    i;                    /* Loop counter                              */

   /* 3Com ethernet card needs this to re-load adapter's netid table         */

   TRACE2 ("newB",(ulong)dds_ptr); /* Net Id New Table begin - dds_ptr       */

   /* If not in diagnostic mode then update the netid table on adapter       */
   if  ((CIO.mode != 'D') &&
       ((WRK.adpt_start_state == EXEC_CMDS_STARTED) ||
        (WRK.adpt_start_state == STARTED)))
   {

      /* Test if there are any valid entries in the table                    */
      if (CIO.num_netids > 0)
      {
         /* Initialize the flags before starting                             */
         byte_1_type = FALSE;            /* IEEE 802.3 netid type            */
         byte_2_type = FALSE;            /* Standard ether netid type        */

         /* Scan the netid table for which types of netids                   */
         for (i=0; i < CIO.num_netids; i++)
         {
            /* Test if one byte netid found                                  */
            if (CIO.netid_table_ptr[i].length == 0x0001)
               byte_1_type = TRUE;       /* IEEE 802.3 netid type            */

            /* Test if one byte netid found                                  */
            if (CIO.netid_table_ptr[i].length == 0x0002)
               byte_2_type = TRUE;       /* Standard ethernet netid type     */

         }

         /* Test if both types of netid are being used                       */
	 /* or the promiscuous mode is enabled                               */
         if (((byte_1_type == TRUE) && (byte_2_type == TRUE)) ||
		CIO.promiscuous_count || CIO.badframe_count)
         {
            /* Tell the adapter to not do ANY pattern matching               */
            entexecque(dds_ptr, NULL, (uchar)SET_TYPE_NULL);
         }
         else
         {
            /* Update the list of netids on the adpater                      */
            entexecque(dds_ptr, NULL, (uchar)SET_TYPE);
         }
      }
      else
      {
	 /* if the promiscuous mode is enabled, 
	    just don't do pattern matching on the adapter */
	 if (CIO.promiscuous_count || CIO.badframe_count)
           entexecque(dds_ptr, NULL, (uchar)SET_TYPE_NULL);
	 else
         /* Tell the adapter not to receive ANY packets                      */
           entexecque(dds_ptr, NULL, (uchar)SET_TYPE_BAD);
      }

   }

   TRACE1 ("newE");                 /* Net Id New Table end                  */

   return;
}

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_startblk                                                        */
/*                                                                           */
/* FUNCTION: Build a start done status block.                                */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Netid element pointer - tells which netid                       */
/*           Status block pointer - tells where to build status block        */
/*                                                                           */
/*    Output: An updated status block that contains the current              */
/*            burned-in address.                                             */
/*                                                                           */
/*    Called From: cio_conn_done, cioioctl                                   */
/*                                                                           */
/*    Calls To: bcopy                                                        */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void xxx_startblk (
   dds_t               *dds_ptr,
   netid_elem_t        *netid_elem_ptr,
   struct status_block *sta_blk_ptr)
{
   TRACE4 ("sblB",(ulong)dds_ptr, (ulong)netid_elem_ptr,
   (ulong)sta_blk_ptr); /* xxx_startblk Begin                                */

   sta_blk_ptr->code = (ulong)CIO_START_DONE;

   sta_blk_ptr->option[0] = WRK.connection_result;/* set before cio_conn_done*/

   sta_blk_ptr->option[1] = netid_elem_ptr->netid;

   sta_blk_ptr->option[3] = 0x00000000;

   bcopy (&(WRK.ent_addr[0]), &(sta_blk_ptr->option[2]), ent_NADR_LENGTH);

   TRACE1 ("sblE"); /* xxx_startblk End                                      */

   return;

} /* end xxx_startblk                                                        */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_halt                                                            */
/*                                                                           */
/* FUNCTION: Build a halt done status block.                                 */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Open pointer - tells which open                                 */
/*           Session Block pointer - tells which session status block        */
/*                                                                           */
/*    Output: An updated status block that contains the current status       */
/*                                                                           */
/*    Called From: cioioctl                                                  */
/*                                                                           */
/*    Calls To: cio_report_status                                            */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void xxx_halt(
   register dds_t          *dds_ptr,      /* tells which adapter             */
   register open_elem_t    *open_ptr,     /* tells which open                */
   register cio_sess_blk_t *sess_blk_ptr) /* tells which session status block*/
{
   cio_stat_blk_t stat_blk;

   TRACE4 ("hltB", (ulong)dds_ptr, (ulong)open_ptr,
   (ulong)sess_blk_ptr);   /* xxx_halt begin                                 */

   stat_blk.code = (ulong)CIO_HALT_DONE;
   stat_blk.option[0] = (ulong)CIO_OK;
   stat_blk.option[1] = sess_blk_ptr->netid;
   cio_report_status (dds_ptr, open_ptr, &stat_blk);

   TRACE1 ("hltE");        /* xxx_halt end                                   */

   return;
} /* end xxx_halt                                                            */

/*****************************************************************************/
/*                                                                           */
/* NAME: xxx_close                                                           */
/*                                                                           */
/* FUNCTION: Delete multicast entries for this open, if they exits.          */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Open pointer - tells which open                                 */
/*                                                                           */
/*    Output: An updated multicast address table.                            */
/*                                                                           */
/*    Called From: cioclose                                                  */
/*                                                                           */
/*    Calls To: entexecque                                                   */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void xxx_close(
   register dds_t       *dds_ptr,  /* tells which adapter                    */
   register open_elem_t *open_ptr) /*  tells which open                      */
{
   int    saved_intr_level;
   int    bus;
   int    i;                       /* Loop Counter                           */
   int    multi_delta = FALSE;     /* Multicast ioctl delta made flag        */
   int    setconfig = FALSE;

   TRACE3 ("cloB", (ulong)dds_ptr, (ulong)open_ptr); /* xxx_close begin      */

   /* Test to verify that the adapter has started                            */
   if (WRK.adpt_start_state == STARTED)
   {

      /* Disable Interrupts                                               */
      DISABLE_INTERRUPTS (saved_intr_level);

      /* Test if there is any valid multicast addresses left                 */
      if (WRK.multi_count > 0)
      {
         /* Initialize flag to indicate if a change to adapter is needed     */
         multi_delta = FALSE;

         /* Match the current address to table and invalidate entry          */
         /* Set up for loop to test all addresses in the table               */
         for (i=0; i < MAX_MULTI; i++)
         {

            /* Test if the array entry is a valid address                    */
            if (WRK.multi_open[i] == open_ptr)
            {

               /* Invalidate the entry                                       */
               WRK.multi_open[i] = NULL;
               WRK.multi_count--;

               /* Delta change required for this adapter                     */
               multi_delta = TRUE;

            } /* endif test for valid array entry address                    */
         } /* end for loop for complete address match                        */
      } /* endif test for any valid multicast addresses left                 */
      if (open_ptr->prom_on_cnt) {
        if (!(CIO.promiscuous_count -= open_ptr->prom_on_cnt)) {
            setconfig = TRUE;
        }
      }

      /* test if there are bad frame mode outstanding on this open */
      if (open_ptr->badframe_on_cnt) {
        if (!(CIO.badframe_count -= open_ptr->badframe_on_cnt))
                setconfig = TRUE;
      }


      /* Enable interrupts again                                          */
      ENABLE_INTERRUPTS (saved_intr_level);

      if (setconfig == TRUE)
      {

	    /* re-set the type filter on the adapter 		  */
	    /* and re-configure the adapter			  */
 	    xxx_newtbl(dds_ptr);		
            entexecque(dds_ptr, NULL, (uchar)CONFIGURE);
      }

      /* Test if a delta needs to be given to the adapter                 */
      if (multi_delta == TRUE)
      {

            /* Give the updated multicast table to the adapter               */
            entexecque(dds_ptr, NULL, (uchar)SET_MULTICAST);

      } /* endif  test to delta change required                           */

   } /* endif test for if adapter has started                                */

   TRACE1 ("cloE");           /* xxx_close end                               */

   return;
} /* end xxx_close                                                           */


/****************************************************************************
 * ent_cleanup -	start cleanup processing for this adapter
 *
 * Input:
 *	dds_ptr	-	the DDS pointer
 ****************************************************************************/
ent_cleanup(dds_t *dds_ptr) {
    int    iocc, i, rc, saved_intr_level;
    ulong  bus, ioa;
    cio_stat_blk_t  stat_blk;

    db_trace(TR_EN, (dds_ptr, "ent: cleanup!"));

    TRACE1("clnB");
    if (WRK.adpt_start_state == NOT_STARTED)
	return;

    bus = BUSMEM_ATT((ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr);
    ioa = BUSIO_ATT( (ulong)DDI.cc.bus_id, DDI.ds.io_port);

    DISABLE_INTERRUPTS(saved_intr_level);

    /*
     * let users if the device know what's going on, in case they care.
     */
    async_notify(dds_ptr, CIO_NET_RCVRY_ENTER, 0, 0, 0);

    /*
     * cleanup up host side xmit resources.
     */
    if (WRK.channel_alocd == TRUE) {
	    register recv_des_t *rd;
	    register struct xmit_elem *xel;

	    /*
	     * free up transmit DMA resources
	     */
	    for (; ((xel = &WRK.xmit_queue[WRK.tx_list_next_out])
		    && xel->in_use == TRUE
		    && WRK.tx_list_next_out != WRK.tx_list_next_in)
		 ; ) {
		    rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY
				    , xel->tx_ds->sys_addr
				    , (size_t)xel->bytes
				    , &WRK.xbuf_xd
				    , xel->tx_ds->io_addr);

		    if (rc != DMA_SUCC)
			    ent_logerr(dds_ptr
				       , ERRID_ENT_ERR5, EFAULT, 0, 0);

		    ent_xmit_done(dds_ptr, xel, CIO_OK);
		    xel->in_use = FALSE;
		    --WRK.tx_tcw_use_count;

		    XMITQ_INC(WRK.tx_list_next_out);
	    }

	    /*
	     * flush any other pending xmit pkts.
	     */
	    for (; ((xel = &WRK.xmit_queue[WRK.tx_list_next_in])
		    && WRK.tx_list_next_buf != WRK.tx_list_next_in)
		 ; ) {

		    ent_xmit_done(dds_ptr, xel, CIO_OK);

		    XMITQ_INC(WRK.tx_list_next_in);
	    }

	    ASSERT(WRK.tx_tcw_use_count == 0);
	    ASSERT(WRK.tx_list_next_in == WRK.tx_list_next_buf);

	    WRK.tx_list_next_in  = 0;
	    WRK.tx_list_next_buf = 0;
	    WRK.tx_list_next_out = 0;
	    WRK.tx_tcw_use_count = 0;
	    WRK.xmits_queued     = 0;
    }

    /*
     * cleanup any host side receive resources
     */
    if (WRK.channel_alocd == TRUE) {
	    register recv_des_t *rd;
	    register struct xmit_elem *xel;

	    /*
	     * free up receive DMA resources
	     */
	    for (rd=&WRK.recv_list[i=0]; i < WRK.recv_tcw_cnt; ++i, ++rd) {
		    if (rd->rbufd) {
			    d_complete(WRK.dma_channel, DMA_READ|DMA_NOHIDE
				       , rd->buf, DMA_PSIZE/2
				       , &WRK.rbuf_xd, rd->rdma);
			    rd->rbufd = 0;
		    }
	    }

	    WRK.Recv_Index  = 0;
	    WRK.Recv_El_off = 0;
    }

    PIO_PUTC(ioa + CONTROL_REG, HARD_RST_MSK);

    if (WRK.wdt_setter == WDT_INACTIVE) {
	    /*
	     * no watchdog timer running. we need to make sure one
	     * is running to watch the restart.
	     */
	    WRK.wdt_setter = WDT_RESTART;
	    w_start(&WDT);
    }

    ASSERT(WRK.adpt_start_state == HARD_RESET_ON);

    /*
     * make state transition to restart, set OFFL timer for 1ms
     *  If the timer is running already, we will just use its tick, but
     *  subvert it to our purposes.
     */
    if (WRK.to_setter == TO_INACTIVE)
	    QUEOFFLMS(1);

    WRK.adpt_state = restart;
    WRK.to_setter  = TO_START;

    BUSIO_DET(ioa);               /* restore I/O Bus                       */
    BUSMEM_DET(bus);              /* restore Memory Bus                    */

    ENABLE_INTERRUPTS(saved_intr_level);
    TRACE1("clnX");
}

/*
 * async_notify -	provide asynchronous notification
 *
 * Input:
 *	dds_ptr		-	^ to DDS!
 *	opt[0-3]	-	optional data
 */
async_notify(dds_ptr, opt0, opt1, opt2, opt3)
dds_t *dds_ptr;
ulong opt0, opt1, opt2, opt3; {
    register i;
    cio_stat_blk_t  stat_blk;

    stat_blk.code      = CIO_ASYNC_STATUS;
    stat_blk.option[0] = opt0;
    stat_blk.option[1] = opt1;
    stat_blk.option[2] = opt2;
    stat_blk.option[3] = opt3;

    for (i = 0; i < CIO.num_allocates; ++i) {
	    register open_elem_t *oe;

	    /*
	     * pass async status to all open channels
	     */
	    if (CIO.chan_state[i] == CHAN_OPENED) {
		    /*
		     * devices open by kernel processes can be directly
		     * notified, otherwise we must queue.
		     */
		    cio_report_status(dds_ptr, CIO.open_ptr[i], &stat_blk);
	    }
    }
}

/*
 * mail_check_status -	Check the status/error bit in the mailboxes
 *
 * Input:
 *	dds_ptr	-	the  DDS pointer
 */

int mail_check_status(
   register  dds_t   *dds_ptr)   /* Pointer to the DDS to know which adapter */
{
   int    saved_intr_level;
   int    bus, ioa;

   TRACE2("mcsB", WRK.mbox_status);           

   /* Get access to the I/O bus to access I/O registers */
   bus = (int)BUSMEM_ATT( (ulong)DDI.cc.bus_id, DDI.ds.bus_mem_addr );
   ioa = (int)BUSIO_ATT(  (ulong)DDI.cc.bus_id, DDI.ds.io_port );

   switch (WRK.mbox_status)
	  {
	  case execute:
		/* Test if processing an execute command & 
	   	 * that command is complete
	         */
		if ((WRK.exec_cmd_in_progres != FALSE) && (DONE_MSK ==
			(PIO_GETC( bus + WRK.exec_mail_box + 1 ) & DONE_MSK)))
			{
      			/* Test if error bit is on and increment count */
      			if (ERROR_MSK == (PIO_GETC( bus + 
				WRK.exec_mail_box + 1 ) & ERROR_MSK))
      				{
       			  	RAS.ds.exec_cmd_errors++;
       			  	/* Log that an error occured */
       			  	ent_logerr(dds_ptr, ERRID_ENT_ERR4,
					WRK.mbox_status, (uchar)0, (uchar)0);
                                                       
      				}
    			}
		break; 
	case recv:
   		/* Test if processing a recv command &
		 *  that command is complete
	         */
   		if ((WRK.recv_cmd_in_progres != FALSE) && (DONE_MSK ==
       			 (PIO_GETC( bus + WRK.recv_mail_box + 1 ) & DONE_MSK)))
   		  	 { 
      			 /* Test if error bit is on and increment count */
      			if (ERROR_MSK == (PIO_GETC( bus + 
				WRK.recv_mail_box + 1 ) & ERROR_MSK))
      				{  
       			  	/* Log that an error occured */
       			  	ent_logerr(dds_ptr, ERRID_ENT_ERR4, 
					WRK.mbox_status, (uchar)0, (uchar)0);
      				} 
    			} 
		break; 
	case xmit:
   		/* Test if processing an transmit command & 
		 * that command is complete
	         */
   		if ((WRK.xmit_cmd_in_progres != FALSE) && (DONE_MSK == 
       			(PIO_GETC( bus + WRK.xmit_mail_box + 1 ) & DONE_MSK)))
   			{
      			/* Test if error bit is on and increment count */
      			if (ERROR_MSK == (PIO_GETC( bus + 
				WRK.xmit_mail_box + 1 ) & ERROR_MSK))
      				{
       			  	/* Log that an error occured */
       			  	ent_logerr(dds_ptr, ERRID_ENT_ERR4,
					WRK.mbox_status, (uchar)0, (uchar)0);
      				}
    			}
		break; 
	}
      	BUSIO_DET(ioa);
      	BUSMEM_DET(bus);

   TRACE1 ("mcsX");           
   return;
 }
