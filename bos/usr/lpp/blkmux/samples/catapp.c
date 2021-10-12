static char sccsid[] = "@(#)21	1.3  src/bos/usr/lpp/blkmux/samples/catapp.c, sysxcat, bos411, 9428A410j 10/25/91 09:47:10";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: 
 *	Channel device driver application interface sample code
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
*/

/* -------------------------------------------------------------------- */
/* Include files needed 						*/
/* -------------------------------------------------------------------- */
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <sys/comio.h>
#include <sys/catuser.h>

extern errno;

typedef struct {
     uchar data[DATASIZE];
}XMIT_DATA, RECV_DATA;;

XMIT_DATA     writbuf;          /* write buffer */
RECV_DATA     readbuf;          /* read buffer  */


/*
 * NAME: start_device
 *
 * FUNCTION: Start the CAT device by issueing a CIO_START ioctl.
 * 	Poll for the CIO_START_DONE status block.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * INPUTS:
 *	fd - file descriptor of the CAT device
 *
 * RETURNS:
 *	0 	- start of the device was successful
 *	others	- start of the device failed 
 */
int
start_device( int fd )
{
	struct session_blk   session;   /* input block for CIO_START */
	struct cat_set_sub   startsub;   
	int rc=0;
	int result=0;


        startsub.sb.netid = $netid;     /* set subchannel to use  */
	startsub.sb.length = 0;         /* not used in CAT driver */
	startsub.sb.status = 0;         /* exception code         */
	startsub.specmode = $mode;      /* No special / CLAW      */ 
	startsub.subset = $num;         /* number of subchannel   */
					/* will be used           */
	startsub.set_default = $default;/* use default? TRUE/FALSE */

	if (!startsub.set_default)
	{
	    startsub.shrtbusy = $busy_status;
	    startsub.startde = $startde;/* unsolicate device end ? */
					/* TRUE / FALSE            */
        }
		  
        /* if startsub.specmode = CAT_CLAW_MOD */
	startsub.claw_blk.WS_appl[] = $name1;
	startsub.claw_blk.H_appl[] = $name2;
	startsub.claw_blk.WS_adap[] = $name3;
	startsub.claw_blk.H_name[] = $name4;

        /* issue CIO_START commond */
	rc = ioctl(fd, CIO_START, &startsub);
	if ( rc < 0 )
	{
		/*
		 * the CIO_START failed
		 */
		return(EIO); 
	} 
	else
	{   /* wait for a status block by polling for it: */
  	
	   pollblk.fd = fd;
	   pollblk.reqevents = POLLPRI;	/* wait for exception */
	   pollblk.rtnevents = 0;
	   result = poll( &pollblk, 1, STATUS_TIMEOUT );	
	   if (result < 0) 
	   {
		close(fd);
		return(EIO);
	   }
	   if (result == 0) 
	   {
		/*
		 * wait for status timed out
		 */
		return(EIO); 
	   }
	   else 
	   {
		/*
		 * Status block is now available.  Issue a 
		 * CIO_GET_STAT ioctl to get status info
		 */
		result = ioctl( fd, CIO_GET_STAT, &status );
		if (result < 0) 
		{
			/* 
			 * ioctl failed 
			 */
			return(EIO);
		}
		if (status.code == CIO_START_DONE)
		{
			if (status.option[0] == CIO_OK)
			{
			   /*
			    * for CLAW mode, check status.option[2]
			    * for CLAW linkid        
			    */
				return(0);
			}
			else   
			{
				/* 
				 * the start failed
				 * check status.option[]
				 */
				 ..
				 ..
				return(EIO);
			}
   

		 }
	    }
        }
} /* end start_device() */

/* -------------------------------------------------------------------- */
/* Halting the CAT device driver via CIO_HALT ioctl			*/
/* -------------------------------------------------------------------- */

/*
 * NAME: halt_device
 *
 * FUNCTION: Halt the CAT device by issueing a CIO_HALT ioctl.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * INPUTS:
 *	fd - file descriptor of the CAT device
 *
 * RETURNS:
 *	void
 */
void
halt_device(int fd)
{
        struct session_blk   session;
	struct cat_set_sub   haltsub;
	int rc=0;

	haltsub.sb.netid = $netid;           /* subchannel to halt */
	haltsub.claw_blk.linkid = $linkid;   /* CLAW linkid */
	haltsub.claw_blk.WS_appl[] = $name1; /* CLAW workstation name */ 
	haltsub.claw_blk.H_appl[] = $name2;  /* CLAW Host application */
	haltsub.claw_blk.WS_adap[] = $name3; /* CLAW workstation appl */
	haltsub.claw_blk.H_name[] = $name4;  /* CLAW Host name        */

	/* 
	 * Halt the netid we registered with the CIO_START command 
	 */
	rc = ioctl(fd, CIO_HALT, &haltsub);
	if (rc) 
	    fprintf(stderr, "\n\t ERROR returned halting: %d - ",errno);
	return;
} /* end halt device */



/*
 * NAME: main
 *
 * FUNCTION: Use the CAT device driver to transmit and receive data
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *	This example does the following:
 *
 *		1.  open the CAT device driver via the /dev/cat0 special file
 *		2.  Start a subchannel, start_device() routine
 *		3.  write data to S/370 host
 *		4.  poll for receive data
 *		5.  read data into buffer
 *		6.  poll for status
 *		8.  Once received data has been read, halt the netid 
 *		    (halt_device() routine) and then close the device.
 *
 *
 */
main()
{

int		fd;	  /* file descriptor */
int		rc;
int		result;
uchar		*p_pack;
struct pollfd	pollblk;
cat_write_ext_t wrt_ext;;      /* write extension structure */
struct read_extension rd_ext;  /* read extension structure */

	/*
	 * open the device driver for reading and writing of
	 * data in Non-blocking mode
	 */
	fd = open("/dev/cat0", O_RDWR | O_NONBLOCK);
	if ( fd < 0 )
	{
		/* 
		 * the open failed
		 */
		return;
	}

	if (start_device(fd))
	{	
		/*
		 * the start of the device failed
		 */
		return;
	}
/* 
 * The device is now opened and started.  We can now do work
 * on the device.  (i.e. write and read)
 */

	
	wrt_ext.cio_ext.flag = 0;          /* clear flags */
	wrt_ext.cio_ext.status   = 0;	 /* clear status */
	wrt_ext.cio_ext.netid = netid;
	/* if CLAW mode */
	wrt_ext.cio_ext.write_id = linkid; /* CLAW linkid */
	wrt_ext.attn_int = 1;

	/* 
	 * here we would build the packet, making sure to 
	 * fill in the FC, destination address, source address, 
	 * and routing information (if any).
	 */

	if ( (rc=writex(fd, $writbuf, DATASIZE, &wrt_ext) < 0)
	{
	   fprintf(stderr, "ERROR: %d, errno);
	   /* or see status block for reason */
        }
	if (rc < 0)
	{
		/* 
		 * the write failed.  logic to handle the
		 * device being in Network Recovery Mode could
		 * be added here if the rc was ENETUNREACH.
		 */
		halt_device(fd);
		close(fd);
		return;
	}



	pollblk.fd = fd;   /* The file desc returned when the open was issued */
	pollblk.reqevents = POLLIN;	/* wait for receive data */
	pollblk.rtnevents = 0;

	poll( &pollblk, 1, READ_TIMEOUT );

	/* 
	 * since the poll has finished there should be data waiting for us, 
	 * issue the read command to get it. 
	 */

	rc = readx(fd, &readbuf, DATASIZE, &rd_ext);
	if (rc < 0)
	{
		/* 
		 * the read failed
		 */
		halt_device(fd);
		close(fd);
		return;
	}
	else if (rc == 0)
	{
		/* 
		 * no data on read
		 */
		halt_device(fd);
		close(fd);
		return;
	}

        else
	{
           /*
	    * Data avaliable!
	    * A user started more than one subchannel on an 
	    * open should check the "rd_ext.netid" field to determine
	    * which subchannel the received data is from.
	    */
	 }

        /* wait for status block by polling for it */

	pollblk.fd = fd;
	pollblk.reqevents = POLLPRI;	/* wait for exception */
	pollblk.rtnevents = 0;
	result = poll( &pollblk, 1, STATUS_TIMEOUT);
	if (result < 0) 
	{
		halt_device(fd);
		close(fd);
		return;
	}
	if (result == 0) 
	{
		/*
		 * wait for status timed out
		 */
		halt_device(fd);
		close(fd);
		return;
	} 
	else 
	{
		/*
		 * Status block is now available.  Issue a 
		 * CIO_GET_STAT ioctl to get status info
		 */
		result = ioctl( fd, CIO_GET_STAT, &status );
		if (result < 0) 
		{
			/* 
			 * ioctl failed 
			 */
			halt_device(fd);
			close(fd);
			return;
		}
		if (status.code == CIO_TX_DONE)
		{
			if (status.option[0] != CIO_OK)
			{
				/* 
				 * the write failed
				 */
				halt_device(fd);
				close(fd);
				return;
			}

		}
		else
		{
			/*
			 * figure out what type
			 * of status we got (i.e. CIO_ASYNC_STATUS,
			 * CIO_HARD_FAIL, CIO_NET_RCVRY_ENTER, etc.)
			 */
			...
		}
	}

	 */
	halt_device(fd);
	close(fd);
} /* end main */

