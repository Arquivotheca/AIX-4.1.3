static char sccsid[] = "@(#)43	1.2  src/bos/diag/util/ulan/icmp.c, dsalan, bos411, 9428A410j 1/3/94 13:34:23";
/*
 * COMPONENT_NAME: LAN service aid
 *
 * FUNCTIONS: icmp_write
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

/* ------------------------------------------------------------------------
 * NAME: icmp_write
 * 
 * FUNCTION: Formulates the ICMP header for a packet.
 *
 *
 * NOTES: The icmp_write function forms the ICMP portion of the
 *        packet.  After the packet is formulated, the
 *        icmp_write function will compute the icmp checksum,
 *        then call ip_write.  If ip_write  indicates an error,
 *        then that error value will be returned; otherwise, the
 *        icmp_write function will return a zero.
 *
 * RETURN: 		GOOD
 *		  	BAD
 *
 *
 * RETURNS: icmp_write will return a zero, or the value returned from the 
 *          ip_write function.
 *
 */

int icmp_write(
char	dest_net_address[6],
unsigned long	destination_ip,
char          *data,       /* pointer to beginning of data portion of packet */
unsigned short  length      /* number of bytes of data */
)
{
	t_icmp 		*icmp;
	int 		rc;

       /*
 	* First, set the length equal to the number of bytes of data plus 
 	* the number of bytes of icmp header - this is the length that icmp 
	* refers to.
 	*/

	length = length + sizeof(t_icmp);

	/*
 	* Set up the icmp header and pseudo header pointers:
 	* data points to the beginning of the data, so the beginning of the 
 	* icmp header is placed the appropriate number of bytes in front of 
	* that.
 	*/

	icmp = (t_icmp *) ((char *)data - sizeof(t_icmp));

       /*
 	* Fill in ICMP header
 	*/

	icmp->type = 0x08;
	icmp->code = 0x00;
	icmp->checksum = 0;

	/*
 	* Pad data to make length even (so 16 bit checksum comes out allright
 	* (Don't need to do this, since csum() takes care of it, but 
	* practicing safe sum, we do it anyway.)
 	*/

	if (length & 0x00000001)
	{
		*(((char *)icmp) + length) = 0;
	}

       /*
 	* Determine checksum using IP checksum routine.  This is a ones 
	* compliment sum (unsigned add with carry), the result of which is 
	* then complimented (inverted).  If the result is zero, change to 
	* all 1's (0xFFFF), since a zero checksum means that one wasn't 
	* computed. The checksum starts with the ICMP header 
	* (which was attached to the front of the data portion of the packet), 
	* and ends at the end of the data portion of the packet, therefore 
	* the length passed to the csum() routine 
 	* is the number of bytes of data plus the number of bytes of icmp 
	* header (which is already stored in the length variable).
 	*/


	icmp->checksum = ipcsum(((unsigned short *)icmp), 
			(unsigned short)length);


	/*
 	* Check the checksum to make sure that it's not zero.  Zero 
	* checksum implies that the checksum was not computed, so we change 
	* to all 1's, which indicates to the receiver that the checksum was 
	* computed.  Ref RFC768.
 	*/

	icmp->checksum = (icmp->checksum!=0)?icmp->checksum:0xFFFF;

       /*
 	* ICMP portion of the packet is all set, now pass it on to ip_write
 	*/


	rc = ip_write (&dest_net_address[0], destination_ip,
	 (char *) icmp, length);
	return (rc);


} /* End of icmp_write */

