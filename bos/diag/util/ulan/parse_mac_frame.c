static char sccsid[] = "@(#)47	1.4  src/bos/diag/util/ulan/parse_mac_frame.c, dsalan, bos411, 9428A410j 1/3/94 13:32:23";
/*
 *   COMPONENT_NAME: DSALAN
 *
 *   FUNCTIONS: parse_beacon_error
 *		parse_mac_frame
 *		parse_soft_error
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
#include "ulan_msg.h"


/*----------------------------------------------------------------------*/
/*	Function:	This function will parse a mac frame		*/
/*			and determine if there is an error 		*/
/*			If it is an error reporting frame 		*/
/*			then analyze the error				*/
/*----------------------------------------------------------------------*/

int parse_mac_frame( length, data )
int	length;
char 	data[];

{
	unsigned short	major_vector_length;
	unsigned char	destination_class;
	unsigned char	command_byte;	
	unsigned short	*p_tmp;
	int		index;

	if (length < 18)
	{
		return (BAD);
	}

	destination_class = data[34] & 0xF0;


	/* the destination class is not RING ERROR MONITOR	*/
	if (destination_class != 0x60)
	{
		return (0);
	}

	p_tmp = (unsigned short *) &data[32];
	major_vector_length = *p_tmp; 
	length = major_vector_length;
	command_byte = data[35];
	index = 36;
	length -= 4;

 	while (length >=0 )
	{
		switch (command_byte)
		{
			case 0x29:	/* Report Soft Error	*/
				parse_soft_error(&data[index]);
				length -= 20;
				index += 20;
				break;
			case 0x02:	/* Beacon		*/
				parse_beacon_error(&data[index], length);
				length -= 8;
				index += 8;
				break;
			default:
				break;
		}
		length -= 1;   
		if ( length > 0)
			command_byte = data[index+1];
	}

	return (GOOD);
}

/*----------------------------------------------------------------------*/
/*	Function:	parse_soft_error				*/
/*	Description:	This function analyses the Report Soft Error	*/
/*			Subvector 	0x29				*/
/*			This Report Soft Error will include		*/
/*			0x2E	: Non-isolating Error Counts  6 bytes	*/
/*			0x2D	: Isolating Error Counts      6 bytes 	*/
/*			0x0B	: Physical location           4 bytes	*/
/*			0x02	: Nearest Active Upstream Neighbur	*/
/*				  ( 6 bytes)				*/
/*----------------------------------------------------------------------*/

parse_soft_error(data)
char	data[];
{
	int	index=0;
	int	number_of_subvectors =4;
	unsigned char	command_byte;
	while ( number_of_subvectors)
	{
		command_byte = data[index+1];
		switch (command_byte)
		{
			/* Non-Isolating Error Counts	*/
			case 0x2E:
			{
				lost_frame_errors += data[index+2];
				receiver_congestion += data[index+3];
				frame_copied_errors += data[index+4];
				frequency_errors += data[index+5];
				token_errors += data[index+6];
				index += 8;
			}
				break;

			/* Isolating Error Counts	*/
			case 0x2D:
			{
				line_errors += data[index+2];
				internal_errors += data[index+3];
				burst_errors += data[index+4];
				ac_errors += data[index+5];
				abort_del_transmit_errors += data[index+6];
				index += 8;

			}
				break;

			/* Physical location		*/
			case 0x0B:
		 	{
				index +=6;
			}
			 	break;


			/* NAUN Nearest Active Upstream Neighbor	*/
			case 0x02:
			{
			    memcpy(&NAUN_network_address[0], &data[index+2], 6);
			    index += 8;
			}
				break;
		}
		
		number_of_subvectors -= 1;
	}
	
	return ;  
}

/*----------------------------------------------------------------------*/
/*	Function:	parse_beacon_error				*/
/*	Description:	This function analyses the Beacon Subvector	*/
/*			command byte 0x02.				*/
/*----------------------------------------------------------------------*/
parse_beacon_error(data, frame_length)
char	data[];
int	frame_length;
{
	int		length;
	int		index=0;
	unsigned char	command_byte;
	int		old_index;

	length = data[index];
	command_byte = data[index+1];
	index = length;
	while (frame_length > 0)
	{
		switch (command_byte)
		{
			case 0x02:	/* NAUN	*/
			  	memcpy(&NAUN_network_address[0],
					&data[old_index+2],6);
			   	display (RING_BEACON_NO_NAUN);
			   	clean_up();
			   	break;
			default:
				break;
		}
		length = data[index];
		command_byte = data[index+1];
		old_index = index;
		index += length;
		frame_length -= length;
	}
}
