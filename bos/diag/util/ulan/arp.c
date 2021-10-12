static char sccsid[] = "@(#)38	1.3  src/bos/diag/util/ulan/arp.c, dsalan, bos411, 9428A410j 1/3/94 13:31:48";
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
extern int filedes;




void rbcopy ();
void timeout(int);
void set_timer();
extern int errno;
struct  sigaction  invec;       /* interrupt handler structure */
/* This array allows efficient bit reversal for FDDI arp responses */

u_char bit_reverse[] = {
   0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0,
   0x30, 0xb0, 0x70, 0xf0, 0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
   0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8, 0x04, 0x84, 0x44, 0xc4,
   0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
   0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc,
   0x3c, 0xbc, 0x7c, 0xfc, 0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
   0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 0x0a, 0x8a, 0x4a, 0xca,
   0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
   0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6,
   0x36, 0xb6, 0x76, 0xf6, 0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
   0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 0x01, 0x81, 0x41, 0xc1,
   0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
   0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9,
   0x39, 0xb9, 0x79, 0xf9, 0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
   0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 0x0d, 0x8d, 0x4d, 0xcd,
   0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
   0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3,
   0x33, 0xb3, 0x73, 0xf3, 0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
   0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 0x07, 0x87, 0x47, 0xc7,
   0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
   0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf,
   0x3f, 0xbf, 0x7f, 0xff};




/*-----------------------------------------------------------------------------
 * NAME: do_arp
 *
 * FUNCTION: Resolves IP address with HW address using the Address Resolution
 *           Protocol (ARP).
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
 * RETURNS: ET_ARP for a good ARP call
 -----------------------------------------------------------------------------*/

int do_arp (find_network_address, local_ip, remote_ip)
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

	/*--------------------------------------------------------------*/
	/*						| arp_req	*/
	/*--------------------------------------------------------------*/

	arp_req = (t_arp *)(arp_tx_buff + TR_PAD);
	if (net_type == FDDI)
		arp_req = (t_arp *)(arp_tx_buff + TR_PAD + 2);

	arp_req->protocol = ET_IP;
	arp_req->hlen = sizeof(HW_ADDR);
	arp_req->plen = sizeof(IP_ADDR);
	arp_req->opcode = ARP_REQ;

	/*
 	 * Byte copy the IP and HW addresses into the ARP request structure, 
	 * since the addresses lie across word boundaries.
 	 */


	memcpy (arp_req->sender_ip, &source_ip, 4);
	memcpy (arp_req->target_ip, &remote_ip, 4);
	memcpy (arp_req->sender_hw, network_address,6);


	/* for ARP frame the network address in ICMP needs to be reversed */
	/* this is just for FDDI					  */

	if (net_type == FDDI)
	{
		reversebit_copy((caddr_t)network_address, 
				(caddr_t) arp_req->sender_hw, 6);

	}

	/* set the target hardware to all 0s' since it is not known yet	*/
	memset (arp_req->target_hw, 0x00, 6);

	/*
 	 * Make the remote HW address all 1's, i.e., the broadcast address.
 	 * Preparing to broadcast to all stations
 	 */

	memset (remote_ip_hw->hw, 0xFF, 6);


	if (    is_802_3 == -1 || is_802_3 == 1 || (net_type == TOKEN) )
	{
		/*
 		 * Create 802.2 type packet
 		 */
	/*--------------------------------------------------------------------*/
	/*					              | LLC | ARP_REQ */
	/*--------------------------------------------------------------------*/

		/*	LLC  Portion	      				*/
		/*------------------------------------------------------------*/
		/*DSAP | SSAP | CONTROL | PROD ID (3 BYTES) | TYPE ( 2 BYTES) */
		/*------------------------------------------------------------*/

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
		llc->type = ET_ARP;
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
			memcpy (eth_802_3->dest, remote_ip_hw->hw,
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
						if (rc_tmp == ET_ARP)
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

		memcpy (token->dest,remote_ip_hw->hw, sizeof(HW_ADDR));
		memcpy ( token->source, network_address, sizeof(HW_ADDR));

		/* Have routing info, so set MSB of source to 1,
		   and copy RC fields and routing information  */

		if (net_type == TOKEN)
			token->source[0] |= 0x80;

		memcpy(( (char *)token + sizeof(t_802_5) ),
			(char *) &(remote_ip_hw->route.rcf), rcl);

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


                        rcl = 2;   /* Routing control length = 2 */

                        remote_ip_hw->route.rcf = 0x8240;

                        token = (t_802_5 *)( (char *)llc - sizeof(t_802_5) - rcl );

                        /* Copy the HW addresses and routing info to the MAC 
			   header, and set AC and FC fields, then shoot it 
			   out the door */

                        memcpy(token->dest,remote_ip_hw->hw, sizeof(HW_ADDR));
                        memcpy(token->source, network_address, sizeof(HW_ADDR));

                        /* Have routing info, so set MSB of source to 1,
                           and copy RC fields and routing information  */

                        token->source[0] |= 0x80;

			for (i=0; i<8; i++)
				remote_ip_hw->route.seg[i] = 0x00;

			memcpy(( (char *)token + sizeof(t_802_5) ),
				(char *) &(remote_ip_hw->route.rcf), rcl);

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
						if (rc_tmp == ET_ARP)
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


/*
 * NAME: rbcopy
 *
 * FUNCTION: Copies data from one location to another.
 *
 * EXECUTION ENVIRONMENT: Runs in the ROS execution environment (real machine
 *                        mode).
 *
 * NOTES: This functions mimics the bcopy function of the
 *        standard C library.  That is, s1 is the source
 *        pointer, s2 is the destination pointer, and n is the
 *        number of bytes to copy from s1 to s2.
 *
 * RECOVERY OPERATION: N/A
 *
 * DATA STRUCTURES: N/A
 *
 * RETURNS: N/A
 *
 */

void 
rbcopy(
char *s1,
char *s2,
int n
)
{
	if ( (unsigned int)s1 + n <= (unsigned int)s2 )
	{
		while (n-- > 0)
		{
			*s2++ = *s1++;
		}
	}
	else /* This should cover the case where the two pointers would overlap */
	{
		s2 = s2 + n - 1;
		s1 = s1 + n - 1;
		while (n-- > 0)
		{
			*s2-- = *s1--;
		}
	}
}

/*
 * NAME: set_timer ()
 *
 * FUNCTION: Designed to set up alarm timer
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void set_timer(timer)
int	timer;
{
        invec.sa_handler = timeout;
        alarm_timeout = FALSE;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );
        alarm(timer);
}
/*------------------------------------------------------------------------
 * NAME: timeout()
 *
 * FUNCTION: Designed to handle timeout interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 *------------------------------------------------------------------------*/

void timeout(int sig)
{
        alarm(0);
        alarm_timeout = TRUE;
}
/*------------------------------------------------------------------------
 *	Function :	reversebit_copy					  *
 *	Description :	this function will reverse bit copy for FDDI	  *
 *------------------------------------------------------------------------*/


reversebit_copy(src, dst, size)
caddr_t src, dst;
int size;
{
        for(;size>0;size--)
                *dst++ = bit_reverse[*src++];
}
