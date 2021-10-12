static char sccsid[] = "@(#)46	1.2  src/bos/diag/util/ulan/parse_echo_reply.c, dsalan, bos411, 9428A410j 1/3/94 13:34:26";
/*	
 *
 * COMPONENT_NAME:  LAN Service Aid
 * 
 * FUNCTIONS: 	ICMP ECHO REPLY parse frame
 *		This procedure will parse a frame that is received
 *		If it's an ICMP ECHO RELY 
 * RETURN CODE:
 *	 	ET_ECHO_REPLY
 *		BAD
 * 
 * ORIGINS: 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 *
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

/* ----------------------------------------------------------------------
 * NAME: parse_frame
 *
 * FUNCTION: Gets receive data, and determines if it's 
 *           an IP or ARP type.
 *	     This frame is our so go ahead parse if it is an ARP response.
 *	     if it is APR RESPONSE then copy the hardware address of destination
 *	     else if ARP REQUEST then answer. 
 *
 *
 * NOTES: The net_read function will poll the adapter in question, and if
 *        there is no data in the receive buffer, will return.  If there is
 *        data, it will look at each packet, and determine if it's
 *        interesting, i.e., that it's an ARP or IP type packet.  It will
 *        continue to read from the buffer until it finds something
 *        interesting or until the buffer is empty.
 * Return : ET_IP, ET_ARP, or ERROR_DETECT
 *
 *
 *------------------------------------------------------------------------*/
int parse_echo_reply( data, remote_ip, length, source_data)
unsigned char 	*data;
unsigned long 	remote_ip;
unsigned short	length;
unsigned char	*source_data;

{
	char       	*loc_data;
	t_ip    	*ip; 
	t_icmp  	*icmp; 
	t_802_2 	*p802_2;
	t_ether        	*eth;
	int            	rc;
	int            	loc_802_2;
	unsigned int   	route_pad;
	unsigned short 	type, loc_rcf, ril=0;
	int		i;
	unsigned char	*p_tmp;
	unsigned short	rec_rcf, rec_rcl;
	unsigned char	cmp_data;


/*
 * A. Something interesting is received from the network, i.e., an IP
 *    packet or ARP reply.  If this occurs, the function will return either
 *    ET_IP or ET_ARP.
 * B. There is nothing in the network buffer.  The funtion will
 *    return a length of zero if there is nothing in the receive buffers.
 *    This function will then return a zero.
 */

	loc_802_2 = 0;


   /*
    * Dig the type field out of the header.  If ethernet, it's right after the
    * source hardware address.  If 802.3 or token ring, it's buried in the LLC
    * portion of the header.  
    * Figure out if it's something we recognize - first, if 802.2, check
    * DSAP=SSAP=0xAA, if not, discard, else check control byte for UI frames
    * (0x03).  If not UI frames, check for XID or Test frames (0xAF or
    * 0xE3).  If XID or Test, answer, get next frame.  If UI or plain ethernet,
    * check type field for something we recognize, i.e, type = IP (0x0800) or 
    * ARP (0x0806).  After we accept the packet, if ethernet, look at the 802.3
    * flag, and if -1, modify it to represent what we got (0 if regular
    * ethernet, 1 if 802.3).
    */

   	if ( net_type == ETHERNET)
   	{
      	       /*
       		* Find out if 802.3. If it is, then set loc_802_2 flag.  Can
       		* tell if it's 802.3 by looking at the first two bytes after the
       		* hardware addresses.  All packets that travel on the ethernet
       		* wires are less than 1514 bytes long.  Since the 802.3 packet
       		* has the length of the packet in the first two bytes after the
       		* hardware addresses, and the ethernet packet has 
		* the protocol type
       		* in those two bytes, all we have to do is check to see that
       		* what is contained in those two bytes is less than or equal to
       		* the maximum ethernet packet length.  Since the lowest type
       		* value defined is 0x0800, and it is greater than the max packet
       		* size, all we have to check is that the type field is less than
       		* 0x0800.  If true, then we have 802.3, else regular ethernet.
       		*/

      		eth = (t_ether *)data;
      		if ( eth->type < 0x0800 )
        	{
         		loc_802_2 = 1;
         	}
      		else
         	{
         		loc_802_2 = 0;
         	}
      	}

   	if ((net_type == FDDI) ||  (net_type == TOKEN) || (loc_802_2 == 1) )
      	{
      		if ( net_type == TOKEN)
         	{
         		/* Test high order bit for routing info */
         		if (((t_802_5 *)data)->source[0] & 0x80)
            		{
            			loc_rcf = *((unsigned short *)
						(data + sizeof(t_802_5)));
            			ril = (loc_rcf & RC_LTH) >> 8;
            		}
         		else
            		{
            			ril = 0;
            		}
         		route_pad = RC_PAD;
		}
      		else if ( net_type == FDDI )
         	{
         		/* Test high order bit for routing info */
         		if (((t_fddi *)data)->source[0] & 0x80)
            		{
            			loc_rcf = *((unsigned short *)
						(data + sizeof(t_fddi)));
            			ril = (loc_rcf & RC_LTH) >> 8;
            		}
         		else
            		{
            			ril = 0;
            		}
			if (ril)
         			route_pad = RC_PAD;
			else
				route_pad = 0;
		}
      		else  /* 802.3 ether */
    		{
         		route_pad = ril = 0;
   		}

      		/*
       		 * Set pointer to beginning of LLC fields.
       		*/

      		p802_2 = (t_802_2 *)(data + sizeof(t_802_5) + route_pad);

		if (net_type == FDDI)
      		p802_2 = (t_802_2 *)(data + sizeof(t_fddi) + route_pad);


       	       /*
       		* Check DSAP and SSAP fields.  If they're not 0xAA, 
		* drop this packet, look for the next one.
       		*/

      		if ( (p802_2->dsap != 0xAA) || (p802_2->ssap != 0xAA) )
		{
         		/* reset loc_802_2 */
         		loc_802_2 = 0;
			return (ERROR_DETECT);
    		}

               /*
       		* Check control field.  If it's not 0xAF, 0xE3, or 0x03 
		* drop this packet, look for the next one.  If 0xAF, then 
		* respond to XID and get next packet.  If 0xE3, respond to 
		* Test and get next packet.  If 0x03, set loc_type and place 
		* a pointer to the beginning of the data (IP header 
       		* or ARP data).
       		*/

  		if ( p802_2->control == 0xAF )
		{
         		/* Someday, answer the XID packet, then continue */
         		/* For now, just ignore (continue) */
			return (ERROR_DETECT);
        	}
        	else
       		{
         		if ( p802_2->control == 0xE3 )
            		{
            			/* Someday, answer the TEST packet, 
				   then continue */

            			return (ERROR_DETECT); 
            		}
         		else
            		{
            			if ( p802_2->control != 0x03 )
               			{
               				/* Don't know what it is, 
					 * so drop and continue */
               				return (ERROR_DETECT);
               			}
            		}
		}

      		/*
       		* If it was anything but an UI frame, we never would have 
		* got this far, so set loc_type and place pointer.
       		*/

      		type = p802_2->type;
      		loc_data = (char *)(p802_2 + 1);
      	}
   	else
      	{
      		/*
       		* This is a plain old ethernet packet.  Move the loc_data 
		* pointer to the
       		* beginning of the data, and mark the type.
       		*/

      		type = eth->type;
      		loc_data = (char *)(eth + 1);
      	}

   	/*
    	 * Process based on type.  If type is IP, return -IP.  If type is ARP,
    	 * determine if request or reply.  If request, and know our IP address
    	 * (local_ip_hw->ip != 0), then reply, and continue.  If it's an ARP 
	 * reply,
    	 * return -ARP.  If it's neither, continue.
    	 */

   	if ( (type != ET_IP) && (type != ET_ARP) )
      	{
      		return (ERROR_DETECT);
      	}

   	if ( type == ET_ARP)
      	{
      		is_802_3 = loc_802_2;
      		return ( ET_ARP);
      	}

   	/* 
    	 * Got an got an IP frame 
    	 */
   
   	ip = (t_ip *)loc_data;
	/* this where the data before sending ICMP ECHO REQUEST	*/
	p_tmp =  ( (char *)ip + sizeof (t_ip) + sizeof (t_icmp) + 4 );

   	if ((ip->ver_IHL  == IP_VERIHL) &&( ip->service == IP_SERVICE) &&
	    ( ip->dest == source_ip) && 
		(ip->source == remote_ip) && ( ip-> proto == IP_ICMP))
 
      	{
		icmp = (t_icmp *) (ip - ( ip->len *4));
		if (icmp->type == ICMP_ECHO_REPLY)
		{
			for (i=0; i<icmp_data_length; i++)
			{
				cmp_data = '0'+i;
				if (*p_tmp != cmp_data)
				{
					return (-1);
				}
				++p_tmp;
			}
			return (ET_IP);
		}

      	}
}
