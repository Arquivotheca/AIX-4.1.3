static char sccsid[] = "@(#)52	1.2  src/bos/diag/util/ulan/start_device.c, dsalan, bos411, 9428A410j 1/3/94 13:34:38";
/*
 * COMPONENT_NAME: LAN service aid 
 *
 * FUNCTIONS:  start_device
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/devinfo.h>
#include <sys/ciouser.h>
#include <sys/comio.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/tokuser.h>
#include <sys/signal.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <diag/da.h>
#include <diag/diag_exit.h>
#include <diag/diag.h>
#include <diag/diago.h>
#include <diag/dcda_msg.h>
#include <diag/bit_def.h>
#include <toktst.h>
#include <locale.h>
#include "netdefs.h"
#include "saring.h"
#include "ulan_msg.h"
extern int	errno;
extern int	filedes;

/*-------------------------------------------------------------------------*/
/*      Function        : Start device                                     */
/*      Description     : This function will start the Ethernet device     */
/*                        with different net_id (or type in ethernet       */
/*                        ). So we are able to receive the different       */
/*                        types of packet                                  */
/*-------------------------------------------------------------------------*/
start_device ()
{

        struct pollfd   poll_struct;
        struct session_blk      s_start;
        static  cio_stat_blk_t  cio_get_stat;   /* CIO status block */
        struct status_block stat_blk;
        int     rc;



        if ( (is_802_3 == -1) || (is_802_3 == 1))
        {

                /* start device so type 0xAA will be set        */
                s_start.netid = 0xAA;
                s_start.length = 1;


		errno = 0;
                rc = ioctl (filedes, CIO_START, &s_start);
                if (rc == -1)
		{
			switch (errno)
			{
			
				case EADDRINUSE:
				{
					display(DUPLICATE_NET_ID);
					clean_up();
				}
				break;

				default:
				{
					display(UNIDENTIFIED_ERROR);
					clean_up();
				}
				break;
			}
		}
                else
                {
			check_exit_cancel();
                        /* set up for poll command complete */
                        poll_struct.fd = filedes;
                        poll_struct.reqevents =  POLLPRI ;
                        poll_struct.rtnevents = 0;

                        if
                         (( rc = poll ( &poll_struct, 1 , TIME_OUT)) > 0)
                        {
                                rc = ioctl (filedes, CIO_GET_STAT,
                                        &cio_get_stat);


                                if ((rc != 0) || ( cio_get_stat.code
                                        != CIO_START_DONE)
                                || ( cio_get_stat.option[0] != CIO_OK))
                                {
					display(UNIDENTIFIED_ERROR);
					clean_up();
                                }

                        }
                        else
                        {
				display(UNIDENTIFIED_ERROR);
				clean_up();
                        }
                }

		check_exit_cancel();

                /* type 0x0800 for ET_IP        */
                s_start.netid = 0x0800;
                s_start.length = 2;


                rc = ioctl (filedes, CIO_START, &s_start);
                if (rc == -1)
		{
			switch (errno)
			{
			
				case EADDRINUSE:
				{
					display(DUPLICATE_NET_ID);
					clean_up();
				}
				break;

				default:
				{
					display(UNIDENTIFIED_ERROR);
					clean_up();
				}
				break;
			}
		}
                else
                {
                        /* set up for poll command complete */
                        poll_struct.fd = filedes;
                        poll_struct.reqevents =  POLLPRI ;
                        poll_struct.rtnevents = 0;
			check_exit_cancel();

                        if
                         (( rc = poll ( &poll_struct, 1 , TIME_OUT)) > 0)
                        {
                                rc = ioctl (filedes, CIO_GET_STAT,
                                        &cio_get_stat);


                                if ((rc != 0) || ( cio_get_stat.code
                                        != CIO_START_DONE)
                                || ( cio_get_stat.option[0] != CIO_OK))
                                {
					display(UNIDENTIFIED_ERROR);
					clean_up();
                                }

                        }
                        else
                        {
				display(UNIDENTIFIED_ERROR);
				clean_up();
                        }
			check_exit_cancel();

		}
        }
} /* start_device */

