static char sccsid[] = "@(#)41	1.2  src/bos/diag/util/ulan/do_icmp.c, dsalan, bos411, 9428A410j 1/3/94 13:34:18";
/*
 * COMPONENT_NAME: LAN service aid 
 *
 * FUNCTIONS: do_icmp
 *            This function will build the icmp packet 
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

/*----------------------------------------------------------------------*/
/*      Function        : do_icmp                                       */
/*      Description     : This function will build the icmp packet      */
/*                        and call icmp_write to send out to the wire   */
/*----------------------------------------------------------------------*/

int do_icmp ()

{
	typedef struct icmp_echo 
	{
   		unsigned short identifier;
   		unsigned short seq_number;
   	} icmp_echo;
	icmp_echo *echo_req, *echo_rep;


	unsigned char		*tx_data;
	unsigned char		*rx_data;
	int			loop_counter;
	unsigned int   		length;
	unsigned char  icmp_type, icmp_code;
	int			rc;
	
	
	
/*
 * Building ICMP echo request packet.
 * ICMP echo request packet and send it out.  
 */

		/*------------------------------------------------------*/
		/*	 ICMP Echo Request				*/
		/*------------------------------------------------------*/

		/*
    	        * Set up the transmit buffer
		* that the IP and ICMP  headers are filled in correctly.
    		*/

		tx_data = &BUF[0] + TR_PAD + sizeof (t_ether) + sizeof (t_ip)
			+ sizeof (t_icmp) + sizeof (icmp_echo);

		if ( ((unsigned int)tx_data)%4 )
		{
			*(unsigned int *)&tx_data = 
				( ((unsigned int)tx_data & 0xFFFFFFFC) + 4);
		}
		echo_req = (struct icmp_echo *)( tx_data - 
					sizeof(struct icmp_echo) );

		rx_data = &BUF[0] + TR_PAD + sizeof(t_ether) + sizeof(t_ip)
		    + sizeof(t_icmp) + sizeof(struct icmp_echo);

		if ( ((unsigned int)rx_data)%4 )
		{
			*(unsigned int *)&rx_data = 
				( ((unsigned int)rx_data & 0xFFFFFFFC) + 4);
		}
		echo_rep = (struct icmp_echo *)( rx_data 
					- sizeof(struct icmp_echo) );

		/*
    		* Place ASCII '0' through 'Z' into the transmit buffer
    		*/

		for (loop_counter=0; loop_counter<('Z'-'0')+1; loop_counter++)
		{
			*tx_data++ = '0' + loop_counter;
		}

	       /*
    		* Initialize the identifier, sequence number, length, type, 
		* and code  before calling icmp_write. is_802_3 was set 
		* do_arp(), so don't diddle with.
   		*/

		echo_req->identifier = 0x1234; 
		echo_req->seq_number = 0;
		length = ('Z'-'0') + 1 + sizeof(struct icmp_echo);
		icmp_data_length = ('Z'-'0') +1;
		icmp_type = ICMP_ECHO_REQ;
		icmp_code = 0;

               /*
    		* Now send out to icmp_write
	 	*/


		rc = icmp_write(&dest_net_address[0], destination_ip, 
		    (char *)echo_req, length ) ;

		return (rc);
}
