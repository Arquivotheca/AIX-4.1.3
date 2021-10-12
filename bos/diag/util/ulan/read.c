static char sccsid[] = "@(#)49	1.2  src/bos/diag/util/ulan/read.c, dsalan, bos411, 9428A410j 1/3/94 13:34:32";
/*
 * COMPONENT_NAME: LAN service aid
 *
 * FUNCTIONS:
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
int	filedes;

extern int errno;

/*
 * COMPONENT_NAME: LAN service aid
 *
 * FUNCTIONS: read_frame
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*----------------------------------------------------------------------*/
/*	Function:	This function will read a frame			*/
/*			The expected frame will be ICMP ECHO REPLY	*/
/*			If the parsed frame is ICMP ECHO REPLY		*/
/*			Then ET_IP wil be returned			*/
/*----------------------------------------------------------------------*/

int read_frame ()

{
	int		counter;
	unsigned char	*p_tmp;
	struct pollfd	poll_struct;
	unsigned char	*read_buf;
	int		rc, rc_tmp;
	int		i,j;
	unsigned char	in_pkt[4096];
	int		return_code=-1;
	

		/* setting up a poll for reading a frame	*/

		poll_struct.fd = filedes;
		poll_struct.rtnevents =0;
		poll_struct.reqevents =  POLLIN;
		set_timer(30);
                alarm_timeout = FALSE;
                while (!alarm_timeout)
		{
			check_exit_cancel();

			/* if there is a receive frame to read	*/
			if ((rc = poll (&poll_struct, 1,TIME_OUT)) > 0)
			{
				errno =0;
				rc = read (filedes, in_pkt, 4096);
				if (rc == -1)
				{
					/* do nothing	*/
				}
				else
				{
					/* compared the destination address */
					/* if rc_tmp == 0: it is our frame  */

					if (net_type == ETHERNET)
					{
						rc_tmp = memcmp (in_pkt[0], 
							network_address[0], 6);
					}
					else if ( net_type == TOKEN)
					{
						rc_tmp = memcmp (in_pkt[2], 
							network_address[0], 6);
					}
					else if ( net_type == FDDI)
					{
						rc_tmp = memcmp (in_pkt[4], 
							network_address[0], 6);
					}

					/* this is our frame 		*/
					if (rc_tmp == 0)
					{
						/* parse if this frame	    */
						/* frame is ICMP ECHO REPLY */
						/* to our ICMP ECHO REQUEST */
						rc_tmp = parse_echo_reply 
							 (&in_pkt[0], 
							  destination_ip,rc);
						if (rc_tmp == ET_IP)
						{
							return_code = ET_IP;
							break;
						}
					}
					
				}
		}
	}
	return (return_code);
} /* read_frame */

/*----------------------------------------------------------------------*/
/*	Function:	read_mac_frame				  	*/	
/*	Description:	This function will read a MAC frame		*/
/*			and analize the hard error (BEACON)		*/
/*			or soft error					*/
/*----------------------------------------------------------------------*/
read_mac_frame ()

{
	int		counter;
	unsigned char	*p_tmp;
	struct pollfd	poll_struct;
	unsigned char	*read_buf;
	int		rc, rc_tmp;
	int		i,j;
	unsigned char	in_pkt[4096];
	int		length;
	int		time_expired=FALSE;
	long		timer;
	

		poll_struct.fd = filedes;
		poll_struct.rtnevents =0;
		poll_struct.reqevents =  POLLIN;
		second_remain = time_allowed %60;
		minute_remain = time_allowed / 60;
                while (time_allowed >=0)
		{
			check_exit_cancel();
			if ((second_remain == 0) && ( minute_remain == 0))
				break;
			rc = get_station_state();
			if (rc == TRUE)
				display_normal_testing ();
			else
				display_ring_abnormal();


			if ((rc = poll (&poll_struct, 1,1000)) > 0)
			{
				errno =0;
				rc = read (filedes, in_pkt, 4096);
				if (rc == -1)
				{
					/* do nothing	*/
				}
				else
				{
				     length = rc;
				     parse_mac_frame(length, &in_pkt[0]);
				}
			}
		}
} /* read_mac_frame */
