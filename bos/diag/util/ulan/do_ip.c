static char sccsid[] = "@(#)42  1.4  src/bos/diag/util/ulan/do_ip.c, dsalan, bos41J, 9517B_all 4/25/95 22:05:21";
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
 * (C) COPYRIGHT International Business Machines Corp. 1992,1995
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

extern int      errno;
unsigned short ipcsum(unsigned short *, unsigned short);
extern int      filedes;


/*
 * COMPONENT_NAME: LAN service aid
 *
 * FUNCTIONS: ip_write
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

/*----------------------------------------------------------------------
 * NAME: ip_write
 *
 * FUNCTION: Formulates the IP header and
 *
 *
 * NOTES: The ip_write function forms the IP packet,
 *        including hardware header info.  For ethernet, it
 *        will use the value in the location pointed to by
 *        is_802_3 to determine whether or not to formulate
 *        802.3 type packets.  It is expected that one form
 *        or another will be chosen after a successful
 *        bootp reply is received.  The 802_3 flag is
 *        ignored for Token Ring.
 *
 *        After the packet is formulated, the ip_write
 *        function will compute the header checksum, then
 *        transmit the packet out of the door.
 *
 * DATA_STRUCTURES: N/A
 *
 * RETURNS:     0       : Good
 *              -1      : Bad
 *----------------------------------------------------------------------*/

int ip_write(remote_network_address, remote_ip, data, length, protocol, code)
char            remote_network_address[6];
unsigned long   remote_ip;
char            *data;
unsigned short  length;
unsigned char   protocol;
unsigned char   code;

{
        int             i;
        t_ip            *ip;
        t_ether         *eth;
        t_802_2         *llc;
        t_802_3         *eth_802_3;
        t_802_5         *token;
        int             rc;
        unsigned int    rcl;
        int             broadcast_flag;
        ARP_ENTRY       remote_ip_hw;
        unsigned short  *rcf;
        unsigned short  tmp_rcf;
        char            *p_tmp;



        remote_ip_hw.ip = remote_ip;
        for (i=0; i<= 5; i++)
                remote_ip_hw.hw[i] = remote_network_address[i];

       /*
        * Setup the ip pointer to point to the front of the IP header, and start
        * filling in the header.
        */

        ip = (t_ip *)(data - sizeof(t_ip));

        ip->ver_IHL  = IP_VERIHL;
        ip->service  = IP_SERVICE;

        /* 0 means that this is not a fragmented packet but can be */
        ip->fragment = 0;
        ip->time     = IP_TIME;
        ip->proto    = IP_ICMP;
        ip->csum     = 0;
        ip->len      = length + sizeof (t_ip);
        ip->id       = 0;
        ip->source   = source_ip;
        ip->dest     = remote_ip;

       /*
        * Compute IP checksum - checksum for the IP header only
        */

        ip->csum = ipcsum((unsigned short *)ip, (unsigned short)sizeof(t_ip));

        /*
        * IP portion of the header is filled in, now have to deal with the
        * data link  and LLC layers.  There are three possible formats -
        * token ring, 802.3  ethernet or plain ethernet.
        */

        if (net_type == ETHERNET)
        {
                if (is_802_3)      /* Formulate an ethernet 802.3 packet */
                {
                        llc = (t_802_2 *)((char *) ip - sizeof(t_802_2));
                        llc->dsap = LLC_K1;
                        llc->ssap = LLC_K1;
                        llc->control = LLC_CONTROL;
                        /* =0x03, since we never send XID or test */

                        for (rc=0; rc<3; rc++)
                        {
                                llc->prot_id[rc] = LLC_K2;
                        }
                        llc->type = ET_IP;
                        eth_802_3 = (t_802_3 *)((char *) llc - sizeof(t_802_3));
                        eth_802_3->length = length + sizeof(t_ip)
                                + sizeof(t_802_2);
                        memcpy((char *)eth_802_3->source,
                                (char *)network_address, 6);
                        memcpy ((char *)eth_802_3->dest,
                                (char *)remote_ip_hw.hw, 6);
                        data = (char *)eth_802_3;
                        length = length + sizeof(t_ip) + sizeof(t_802_2)
                                        + sizeof(t_802_3);
                }
                else /* Formulate a regular ethernet packet */
                {
                        eth = (t_ether *) ((char*) ip - sizeof(t_ether));
                        eth->type = ET_IP;
                        memcpy ((char *)eth->src,(char *)network_address, 6);
                        memcpy ((char *)eth->dst,(char *)remote_ip_hw.hw, 6);
                        data = (char *)eth;
                        length = length + sizeof(t_ip) + sizeof(t_ether);
                }

                /*
                 * Check the length - if less than ET_MINLEN (60 bytes),
                 * change to ET_MINLEN and pad with zeros
                 */

                if (length < ET_MINLEN)
                {
                        for (rc=length; rc<(ET_MINLEN); rc++)
                        {
                                *(data+rc) = 0x00;
                        }
                        length = ET_MINLEN;
                }
                /* Send it out the appropriate network */
                rc = write(filedes, data, length);

                if ( ( rc != GOOD ) && ( rc != length))
                {
                        return (rc);
                }

        }
        else /* Formulate a Token Ring packet - implies we have LLC */
        {
                /* First, set up 802.2 stuff, since it's the same whether
                 * we need routing info or not */

                llc = (t_802_2 *)((char *) ip - sizeof(t_802_2));
                llc->dsap = LLC_K1;
                llc->ssap = LLC_K1;

                /* =0x03, since we never send XID or test */
                llc->control = LLC_CONTROL;
                for (rc=0; rc<3; rc++)
                {
                        llc->prot_id[rc] = LLC_K2;
                }
                llc->type = ET_IP;


               /* Now set up MAC stuff.  The format of this info will be
                * dependent upon whether or not token (bridge) routing info
                * is needed.  If routing info is required (the RC field is not
                * blank, or sending an all routes broadcast), then we have to
                * make room for the RC field and the RI fields (up to 18
                * bytes total).  Otherwise, we're on the same ring, so no
                * RC or RI fields are required. */

               /* Check for broadcast first.  If a broadcast, then we know
                * that there is no routing info for the destination.
                * However, we place the 2 byte RC field into our MAC header,
                * so we can gather routing info from the reply.  Also when
                * sending packets with routing info, we must make the high
                * order bit of the source (our) hardware address a 1 - this
                * tells the receiving end that there is routing info in the
                * packet. */

                broadcast_flag = 1;
                for (rc=0;rc<6;rc++)
                {
                        if (network_address[rc] != 0xFF)
                           /* Not a broadcast if true */
                        {
                                broadcast_flag = 0;
                                break;
                        }
                }
                if (broadcast_flag)
                {
                        rcl = 2;
                        /* rcl = 2 bytes, for sizeof (rcf) (No RI info yet) */

                        remote_ip_hw.route.rcf = 0x8210;

                        /* All routes broadcast, dir = 0, max
                         * frame = 1 (1500 bytes), but maybe
                         * should be all 1's */
                }
                else
                {
                       /* Not a broadcast, implies we know who it is we are
                        * sending to, thus implies that they have an ARP entry,
                        * which may have routing info associated with it.
                        * So, check for routing info in the ARP entry, and
                        * if found, set the routing control length (rcl)
                        * appropriately, otherwise set to zero.
                        * NOTE - this check assumes that the routine that
                        * stuffed the ARP entry for the destination cleared
                        * the RC field if there was no
                        *  routing data */

                        rcl = remote_rcl;
                        /* rcl = 0 if rcf = 0, otherwise
                                                rcl is the number of bytes of
                                                routing info (including 2 bytes
                                                for rcf */
                }

               /* Place the token pointer to the beginning of the MAC
                * portion of the packet, taking into account the routing info,
                * if any */

                token =(t_802_5 *)((char *) llc - sizeof(t_802_5) - remote_rcl);

               /* Copy the HW addresses and routing info to the MAC header,
                * and set AC and FC fields, then shoot it out the door */

                memcpy((char *)token->dest, (char *)remote_ip_hw.hw, 6);
                memcpy((char *)token->source,(char *) &network_address[0], 6);
                token->ac = 0x10;
                token->fc = 0x40;

                if (net_type == FDDI)
                {
                        token->ac = 0x00;
                        token->fc = 0x50;
                }

               /*
                * And first bit of dest address to ensure it's off.  This is
                * done since the first bit of the destinination address is
                * always zero, but the destination address may have been
                * obtained from copying a source address that had the first
                * (routing) bit on.  If so, then this action
                * will turn that bit off.
                */

                if ( !broadcast_flag )
                {
                        token->dest[0] &= 0x7F;
                }
                if (rcl)
                {
                       /* Have routing info, so set MSB of source HW address
                        *to 1 */

                        token->source[0] |= 0x80;

                        rcf = (unsigned short *) &remote_route_info[0];
                        tmp_rcf = *rcf;
                        *rcf = tmp_rcf^RC_DIR_MASK;

                        /* Now copy routing info */
                        memcpy ((((char *)token) + sizeof(t_802_5)),
                                &remote_route_info[0], rcl);
                }

                if (net_type == FDDI)
                {
                        p_tmp = (char *) token -2;
                        token = (t_802_5 *) p_tmp;

                        *p_tmp = (unsigned char )0x00;
                        p_tmp++;
                        *p_tmp = (unsigned char )0x00;
                        length += 2;

                }
                /* Send it out the appropriate network */
                data = (char *) token;
                length = length + sizeof(t_ip) + sizeof(t_802_2)
                         + sizeof (TOK_ROUTE) + sizeof(t_802_5);


                data = (char *) token;
                rc = write(filedes, data, length);

                if ( ( rc != GOOD ) && ( rc != length))
                {
                        return (rc);
                }
        }
        rc = read_frame ();
        return (rc);
}  /* End of ip_write  */

/*----------------------------------------------------------------------
 * NAME: ipcsum
 *
 * FUNCTION: Computes an IP type checksum.
 *
 * EXECUTION ENVIRONMENT: Runs in the ROS execution environment (real machine
 *                        mode).
 *
 * NOTES: Computes a 16 bit checksum for the data specified.
 *
 * RECOVERY OPERATION: N/A
 *
 * DATA STRUCTURES: N/A
 *
 * RETURNS: The checksum computed is returned.
 *
 ---------------------------------------------------------------------*/
unsigned short
ipcsum (
        unsigned short *data,  /* pointer to beginning of data buffer */
        unsigned short length  /* number of bytes */
       )

{
        register int sum = 0;
        while (length >1)
        {
                sum += * data++;
                length -= 2;
        }
        if (length == 1)    /* Add in last byte */
        {
                sum += (*(unsigned char *)data) << 8;
        }
        while (sum >>  16)
        {
                sum = (sum & 0xFFFF) + (sum >> 16);
        }
        sum = ~sum;
        return (sum);
} /* ipcsum     */
