static char sccsid[] = "@(#)53	1.3  src/bos/diag/util/ulan/parse_frame.c, dsalan, bos411, 9428A410j 1/3/94 13:34:29";
/*
 *   COMPONENT_NAME: DSALAN
 *
 *   FUNCTIONS: parse_frame
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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




/* 
 * NAME: parse_frame
 *
 * FUNCTION: Gets a received data and determines if it is  
 *           an IP or ARP type.
 *	     This frame is our so go ahead parse if it is an ARP response.
 *	     if it is APR RESPONSE then copy the hardware address of destination
 *	     else if ARP REQUEST then answer. 
 *
 * Return:	good : ET_IP
 *		bad:   Anything else
 *
 *
 */

int parse_frame( data, remote_ip )
unsigned char *data;
unsigned long remote_ip;

{
	char           *loc_data;
	t_arp          *arp;
	t_802_2        *p802_2;
	t_ether        *eth;
	int            rc;
	int            loc_802_2;
	unsigned int   route_pad;
	unsigned short type, loc_rcf, ril=0;
	ARP_ENTRY      arp_reply_addr;
	IP_ADDR        temp_ip;
	int		i;
	unsigned char	*p_tmp;
	unsigned short	rec_rcf, rec_rcl;


/*
 * A. Something interesting is received from the network DIR, i.e., an IP
 *    packet or ARP reply.  If this occurs, the function will return either
 *    -ET_IP or -ARP_REP.
 * B. There is nothing in the network buffer.  The network DIR will
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

   	if ( (net_type == TOKEN) || (loc_802_2 == 1) )
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
      		else  /* 802.3 ether */
    		{
         		route_pad = ril = 0;
   		}

      		/*
       		 * Set pointer to beginning of LLC fields.
       		*/

      		p802_2 = (t_802_2 *)(data + sizeof(t_802_5) + route_pad);

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
   	else if ( net_type == FDDI) 
      	{
         	/* Test high order bit for routing info */
         	if (((t_fddi *)data)->source[0] & 0x80)
            	{
            		loc_rcf = *((unsigned short *)
					(data + sizeof(t_802_5)));
            		ril = (loc_rcf & RC_LTH) >> 8;
            	}
         	else
            	{
            		ril = 0;
            	}
		if ( ril == 0)
			route_pad = 0;
		else
         		route_pad = RC_PAD;

      		/*
       		 * Set pointer to beginning of LLC fields.
       		*/

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

   	if ( type == ET_IP )
      	{
      		is_802_3 = loc_802_2;
      		return ( ET_IP);
      	}

   	/* 
    	 * Got an ARP.  Check if reply.
    	 */
   
   	arp = (t_arp *)loc_data;

   	if ( arp->opcode == ARP_REP )
      	{
		p_tmp = &arp->sender_ip[0];
		rc = memcmp (p_tmp, &remote_ip, 4);
   		if (rc == 0)
		{
			p_tmp = data;
			if (ril != 0)
			{
			rec_rcf = *((unsigned short *)(data + sizeof(t_802_5)));
         		rec_rcl = (rec_rcf & RC_LTH) >> 8;
			remote_rcl = rec_rcl;
			remote_rcf = rec_rcf;
			memset (remote_route_info,0, remote_rcl);
			memcpy (remote_route_info, data 
				+ sizeof (t_802_5),remote_rcl);
			}
      			is_802_3 = loc_802_2;
      			return (ET_ARP);
		}
		else 
			return (NO_DATA);
      	}
   	else
      	{
      		if ( arp->opcode != ARP_REQ )
         	{
                       /* Got a RARP packet, or other garbage.  
			* Drop it and continue. */

         		return (ERROR_DETECT);
         	}
      	}

   	/*
    	* Got an ARP request.  Byte copy the target IP address into a local 
    	* variable so we can do an integer compare.  Check our local IP 
    	* address, and if can answer, do so, else drop it and continue.
    	*/

   	memcpy((char *)&temp_ip,arp->target_ip, sizeof(HW_ADDR));

   	if ( (source_ip == 0) 
       		|| (source_ip != temp_ip ) )
      	{
        	/*
       		* Can't answer, don't know our IP address or it's not 
		* addressed to us. Drop it and continue.
       		*/

      		return (ERROR_DETECT);
      	}

   	if ( arp->protocol != ET_IP )
      	{
      		/*
       		* We only do IP protocol, so if this request isn't for IP, 
		* drop it,
       		* continue.
       		*/
      		return (ERROR_DETECT);
      	}

   	if ( net_type == TOKEN) 
      	{
      		if ( arp->hardware != HARDWARE_TOKEN )
         	{
         		/*
          		 * The hardware type wasn't right, so toss it, continue.
          		 */
         		return (ERROR_DETECT);
         	}
      	}
  	 else
      	{
      		if ( (loc_802_2 && (arp->hardware != HARDWARE_TOKEN))
          		|| (!loc_802_2 && (arp->hardware != HARDWARE_ETHER)) )
         	{
         		/*
          		 * The hardware type wasn't right, so toss it, continue.
          		 */
			         		return (ERROR_DETECT);
         	}
      	}

   	if ( (arp->hlen != sizeof(HW_ADDR)) || (arp->plen != sizeof(IP_ADDR)) )
      	{
      		/*
       		 * Sender is hosed - throw it away.
       		 */
      		return (ERROR_DETECT);
      	}


   /*
    * Everything's okay.  Place the sender_hw in the target_hw and the
    * sender_ip in the target_ip, place the local_ip_hw->hw into the
    * sender_hw and the local_ip_hw->ip into sender_ip.  Change opcode to
    * ARP_REP, and set up the MAC level stuff - for ethernet or 802.3,
    * use the packet laying in data buffer, and just place the target.hw
    * value into the destination field, and the sender.hw value into the
    * source field, and let it rip.  For Token, have to use routing
    * info, if any, or strip off the rc stuff if the request didn't pass
    * through any bridges.  If routing info, have to flip the direction
    * bit, and set the MSB of the source (us).  After letting it fly,
    * continue.  Ignore errors from the network write routines.
    */

   /*
    * First, fill in the arp_reply_addr structure.  If ril<=2, make the 
    * rcf field zero, else copy the data starting with the rcf, length = ril
    * into the ROUTE portion of the ARP_ENTRY structure.
    */
   	memcpy ((char *)&(arp_reply_addr.ip), arp->sender_ip,sizeof(IP_ADDR));
   	memcpy (arp_reply_addr.hw,arp->sender_hw, sizeof(HW_ADDR));
   	if ( ril <= 2 )
      	{
      		arp_reply_addr.route.rcf = 0x0000;
      	}
   	else
      	{
      		memcpy ( (char *)&(arp_reply_addr.route.rcf), 
			(char *) (data + sizeof(t_802_5)),ril);
      		arp_reply_addr.route.rcf = arp_reply_addr.route.rcf^RC_DIR_MASK;
      		arp_reply_addr.route.rcf = arp_reply_addr.route.rcf&RC_BC_MASK;
      	}
/*
   	rc = answer_arp(ipl_cb_ptr, local_ip_hw, &arp_reply_addr, net_type, 
                   loc_802_2);
*/
   	if (rc)
      	{  /* Error from network DIR */
      		return(rc);
      	}
}
