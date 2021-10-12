static char sccsid[] = "@(#)48	1.4  src/bos/diag/util/ulan/rarp.c, dsalan, bos411, 9428A410j 1/3/94 13:32:28";
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
extern  nl_catd diag_catopen();
extern	int	filedes;




void timeout(int);
void set_timer();
void rbcopy();
extern int errno;
struct  sigaction  invec;       /* interrupt handler structure */

/*-----------------------------------------------------------------------------
 * NAME: do_rarp
 *
 * FUNCTION:  Resolve IP address with known network address
 *         
 *
 *
 * NOTES: do_arp will formulate an ARP packet using the values passed to
 *        it, and call the IP function .  If the network type
 *        is ethernet, and the is_802_3 flag is -1, it will send both
 *        802.3 and regular ethernet packets out. 
 *        If no valid ARP reply is received a -1 is return. 
 *        If a valid ARP reply is received, the HW address corresponding to the
 *        IP address being ARPed will be placed into 
 *        structure, as well Token Ring routing info, and a zero will
 *        be returned.
 *
 *
 -----------------------------------------------------------------------------*/

int do_rarp (find_network_address, local_ip, remote_ip)
unsigned char	find_network_address[6];
unsigned long	local_ip;
unsigned long	remote_ip;
{

	char arp_tx_buff[150];

	ARP_ENTRY	*remote_ip_hw;
	t_arp          *arp_req, *arp_rep;
	t_ether        *eth;
	t_802_2        *llc;
	t_802_3        *eth_802_3;
	t_802_5        *token;
	unsigned short length, rec_length;
	int            lctr;                             /* Loop counter */
	int            ret_code;
	int            rec_flag;
	unsigned int   rcl, rec_rcl, rec_llc;
	unsigned int   route_pad;
	unsigned short rec_rcf;
	char           *temp;
	IP_ADDR        temp_ip;
	unsigned int timeout;
	int		counter;
	unsigned char	*p_tmp;
	unsigned char	*p_fddi;
	unsigned char	*p_tmp_fddi;
	struct pollfd	poll_struct;
	unsigned char	*read_buf;
	int		rc, rc_tmp;
	int		i,j;
	unsigned char	in_pkt[4096];
	int		return_code=BAD;
	struct status_block stat_blk;
        struct write_extension ext;
 	struct session_blk      s_start;
	static  cio_stat_blk_t  cio_get_stat;   /* CIO status block */




	remote_ip_hw = ( ARP_ENTRY *) malloc (sizeof (ARP_ENTRY));

	remote_ip_hw->ip = remote_ip;

	/*
     	 * Set up the ARP request packet.
 	 */

	arp_req = (t_arp *)(arp_tx_buff + TR_PAD);
	if (net_type == FDDI)
		arp_req = (t_arp *)(arp_tx_buff + TR_PAD + 2);

	arp_req->protocol = ET_IP;
	arp_req->hlen = sizeof(HW_ADDR);
	arp_req->plen = sizeof(IP_ADDR);
	arp_req->opcode = RARP_REQ;

	/*
 	 * Byte copy the IP and HW addresses into the ARP request structure, 
	 * since the addresses lie across word boundaries.
 	 */

	p_tmp = (unsigned char *) &source_ip;
	for (counter =0; counter <=3; counter++)
	{
		arp_req->sender_ip[counter] = *p_tmp;
		++p_tmp;
	}

	for (counter =0; counter <=3; counter++)
	{
		arp_req->target_ip[counter] = 0x00;
		++p_tmp;
	}

	memcpy (arp_req->sender_hw, network_address,6);
	memcpy (arp_req->target_hw, find_network_address,6);


	/* for ARP frame the network address in ICMP needs to be reversed */
	/* this is just for FDDI					  */
	if (net_type == FDDI)
	{
		reversebit_copy((caddr_t)network_address, 
				(caddr_t) arp_req->sender_hw, 6);

	}


	memcpy(remote_ip_hw->hw, find_network_address, 6);


	if (    is_802_3 == -1 || is_802_3 == 1 || (net_type == TOKEN) )
	{
		/*
 		 * Create 802.2 type packet
 		 */

		p_tmp = (char *)( arp_req ) ;
		llc = (t_802_2 *) ( p_tmp -8);
		llc->dsap = LLC_K1;
		llc->ssap = LLC_K1;
		llc->control = LLC_CONTROL; 
		/* =0x03, since we never send XID or test */
		for (lctr=0; lctr<3; lctr++)
		{
			llc->prot_id[lctr] = LLC_K2;
		}
		llc->type = ET_RARP;
	}

	if (net_type == ETHERNET)
	{
   	/* 
    	 * First check is_802_3.  If -1, have to send out both 802_3 and
    	 * ethernet packets; otherwise, send out the type specified.
    	 * Build the 802.3 packet first, send it out, then move the hardware 
    	 * addresses over 8 bytes to create an ethernet packet, and send it out.
    	 */

		if ( (is_802_3 == -1) || (is_802_3 == 1) )
		{
			arp_req->hardware = HARDWARE_TOKEN;
			eth_802_3 = (t_802_3 *)((char *) llc - sizeof(t_802_3));

      			/*
       			 * We know that the length of the ARP packet is less 
			 * than the minimum length of an ethernet packet, so 
			 * set the packet length to min length
       			 * and zero out the remainder of the packet.
       			 */
			temp = (char *)(arp_req + 1);
			for (lctr=0; lctr<( ET_MINLEN - ARP_SIZE ); lctr++)
			{
				*(temp + lctr) = 0x00;
			}

			eth_802_3->length = ARP_SIZE + sizeof(t_802_2); 
			eth_802_3->length = ET_MINLEN; 

			memcpy(eth_802_3->source, network_address,
				sizeof(HW_ADDR));
			rbcopy(remote_ip_hw->hw, eth_802_3->dest, 
				sizeof(HW_ADDR));
			length = eth_802_3->length + sizeof(t_802_3);


			errno = 0;
      			ret_code = write ( filedes, (char *)eth_802_3, length);
			if (( ret_code == BAD ) || ( ret_code != length))
			{
				switch (errno)
				{
					case ENETDOWN:
					case ENETUNREACH:
						if (net_type == TOKEN)
						{
							display (NETWORK_DOWN);
							clean_up();
						}
					break;
				
					default:
					break;
				}
				return (BAD);
			}




			if ( is_802_3 == -1 )
			{
	 		/*
          		 * Now formulate the normal ethernet packet
          		 */
				arp_req->hardware = HARDWARE_ETHER;
				eth = (t_ether *)( (char *) arp_req  
					- sizeof(t_ether) );
				memcpy (eth->src, network_address, 
					sizeof (HW_ADDR));
				memcpy (eth->dst, remote_ip_hw->hw, 
					sizeof (HW_ADDR));
				length = ET_MINLEN;
				errno = 0;
      				ret_code = write (filedes, (char *)eth, length);
				if ((ret_code == BAD ) || ( ret_code != length))
				{
					switch (errno)
					{
						case ENETDOWN:
						case ENETUNREACH:
							if (net_type == TOKEN)
							{
							  display(NETWORK_DOWN);
							  clean_up();
							}
						break;
				
						default:
						break;
					}
					return (BAD);
				}
			}
		}
		poll_struct.fd = filedes;
		poll_struct.rtnevents =0;
		poll_struct.reqevents =  POLLIN;
		set_timer(30);
		alarm_timeout = FALSE;
		while (!alarm_timeout)
		{
			check_exit_cancel();
			if ((rc = poll (&poll_struct, 1,TIME_OUT)) > 0)
			{
				errno =0;
				rc = read (filedes, in_pkt, 1200);
				if (rc == -1)
				{
					return (BAD);
				}
				else
				{
					
					rc_tmp = memcmp (in_pkt[0], 
						network_address[0], 6);
					if (rc_tmp == 0)
					/* this is our packet	*/
					{
						alarm_timeout = TRUE;	
						
			
						p_tmp = (unsigned char *)&in_pkt[0];
						rc_tmp = parse_frame (p_tmp, remote_ip);
						if (rc_tmp == ET_RARP)
						{
							for (rc_tmp =0; rc_tmp <= 5; rc_tmp++)
							{
									if (net_type == TOKEN)
									{
										*find_network_address =	
										in_pkt[8+rc_tmp];
									}
									else if (net_type == FDDI)
									{
										*find_network_address =	
										in_pkt[11+rc_tmp];
									}
									else
									{
										*find_network_address =	
										in_pkt[6+rc_tmp];
									}
									find_network_address++;
							}
							return_code= GOOD;
							break;
						}
					

					}
				}
					
			}
			else
			{
				return (NO_READ_DATA);
			}

				
		}
	}

	else	/* THIS IS THE TOKENRING PART				*/
	{
		/*
    		 * Do the Token Ring thing.  Since we know this is a broadcast,
		 *  we must make room for the Routing info, and must set the 
		 * MSB of the source HW address to a 1.
    		 */

		arp_req->hardware = HARDWARE_TOKEN;
	

		rcl = 2;   /* Routing control length = 2 */

		/* at this release 3.2.1 routing information in ARP frame */
		/* has not been ARP reponsed yet. So two types of frame	  */
		/* have been developed for FDDI				  */
		/* this one does not have routing information		  */

		if (net_type == FDDI)
		{
			arp_req->hardware = 0x01;
			rcl = 0;
		}
		/* routing control field		*/
		/* Broadcast Indicator field 
			BBB : 0x100 : All routes Broadcast */
		/* Length Bits Field			*/
		/*    LLLLL : 0x0010	2 routing control bytpes	 */
		/*  DFFFrrrr 						 */
		/*  D = 0x0	: Routing information from left to right */
		/*  FFF: Largest Frame bit 0x100 as many as 8144 bytes   */
		/*       in the information field			 */
		/*  rrrr : Reserve bits					 */
		/* Consult Tokenring Network Architecture SC30-3374   	 */
		/* Page 2-7 -> 2-11					 */

		remote_ip_hw->route.rcf = 0x8240;   
		                                   
		token = (t_802_5 *)( (char *)llc - sizeof(t_802_5) - rcl );

		/* Copy the HW addresses and routing info to the MAC header, 
	           and set AC and FC fields, then shoot it out the door */

		rbcopy(remote_ip_hw->hw, token->dest, sizeof(HW_ADDR));
		memcpy ( token->source, network_address, sizeof(HW_ADDR));

		/* Have routing info, so set MSB of source to 1,
		   and copy RC fields and routing information  */

		if (net_type == TOKEN)
			token->source[0] |= 0x80;

		rbcopy( (char *) &(remote_ip_hw->route.rcf), ( (char *)token
		    + sizeof(t_802_5) ), rcl);

		token->ac = 0x10;   
		token->fc = 0x40;  
		if (net_type == FDDI)
			token->fc = 0x50;


		/* Send it out the appropriate network */
		length =  ARP_SIZE + sizeof(t_802_2) + rcl + sizeof(t_802_5);

		p_tmp = (unsigned char *) token ;
		if (net_type == FDDI)
		{
			length = length+2;
			p_fddi =  p_tmp -2;
			p_tmp = p_fddi;
			for (i=0; i<3; i++)
			{
				*p_tmp = 0x00;
				++p_tmp;
			}
		}
		p_tmp = (unsigned char *) token;
		if (net_type == FDDI)
			p_tmp = p_fddi;
		errno = 0;
		ret_code = write(filedes, p_tmp, length);
		if (( ret_code == BAD ) || ( ret_code != length))
		{
			switch (errno)
			{
				case ENETDOWN:
				case ENETUNREACH:
				if (net_type == TOKEN)
				{
					display (NETWORK_DOWN);
					clean_up();
				}
				break;
			
				default:
				break;
			}
			return (BAD);
		}

		/* sending a frame with Routing information	*/
		/* This is the second FDDI frame with routing information. For future 	*/


		if (net_type == FDDI)
		{
                        arp_req->hardware = HARDWARE_TOKEN;


                        rcl = 18;   /* Routing control length = 2 */

                        remote_ip_hw->route.rcf = 0x8240;

                        token = (t_802_5 *)( (char *)llc - sizeof(t_802_5) - rcl );

                        /* Copy the HW addresses and routing info to the MAC 
			   header, and set AC and FC fields, then shoot it 
			   out the door */

                        rbcopy(remote_ip_hw->hw, token->dest, sizeof(HW_ADDR));
                        memcpy ( token->source, network_address, sizeof(HW_ADDR));

                        /* Have routing info, so set MSB of source to 1,
                           and copy RC fields and routing information  */

                        token->source[0] |= 0x80;

			for (i=0; i<8; i++)
				remote_ip_hw->route.seg[i] = 0x00;

                        rbcopy( (char *) &(remote_ip_hw->route.rcf), ( (char *)token
                            + sizeof(t_802_5) ), rcl);

                        token->fc = 0x50;


                        /* Send it out the appropriate network */
                        length =  ARP_SIZE + sizeof(t_802_2) + rcl + sizeof(t_802_5);

                        p_tmp = (unsigned char *) token ;
                        if (net_type == FDDI)
                        {
                                length = length+2;
                        	p_fddi = (unsigned char *) token -2  ;
                                p_tmp =  (char *) p_fddi;
                                for (i=0; i<3; i++)
                                {
                                        *p_tmp = 0x00;
                                        ++p_tmp;
                                }
                                p_tmp = (char *) p_fddi;
                        }

                        p_tmp = (char *)p_fddi;
                        errno = 0;
                        ret_code = write(filedes, p_tmp, length);
                        if (( ret_code == BAD ) || ( ret_code != length))
                        {
                                switch (errno)
                                {
                                        case ENETDOWN:
                                        case ENETUNREACH:
                                        if (net_type == TOKEN)
                                        {
                                                display (NETWORK_DOWN);
                                                clean_up();
                                        }
                                        break;

                                        default:
                                        break;
                                }
                                return (BAD);
                        }
		}

		set_timer (30);
		alarm_timeout = FALSE;
		poll_struct.fd = filedes;
		poll_struct.rtnevents =0;
		poll_struct.reqevents =  POLLIN;
		while (!alarm_timeout)
		{
			check_exit_cancel();
			if ((rc = poll (&poll_struct, 1,TIME_OUT)) > 0)
			{
				errno =0;
				rc = read (filedes, in_pkt, 4096);
				if (rc == -1)
				{
					return (BAD);
				}
				else
				{

					if (net_type == FDDI)
					{
						rc_tmp = memcmp (in_pkt[4], 
						   network_address[0], 6);
					}
					else
						rc_tmp = memcmp (in_pkt[2], 
						   network_address[0], 6);
					if (rc_tmp == 0)
					/* this is our packet	*/
					{
						p_tmp = (unsigned char *)&in_pkt[0];
						rc_tmp = parse_frame (p_tmp, remote_ip);
						if (rc_tmp == ET_RARP)
						{
							if (net_type == TOKEN)
							{
							   for (rc_tmp =0; rc_tmp <= 5; rc_tmp++)
							   {
									*find_network_address =	
									in_pkt[8+rc_tmp];
									find_network_address++;
							    }
							}
							else if (net_type == FDDI)
							{
							   for (rc_tmp =0; rc_tmp <= 5; rc_tmp++)
							   {
									*find_network_address =	
									in_pkt[10+rc_tmp];
									find_network_address++;
							    }
							}
							alarm_timeout = TRUE;
							return_code= GOOD;
							break;
						}
					

					}
				}
					
			}

				
		}
	}

	return (return_code);

}	/* do_arp	*/
