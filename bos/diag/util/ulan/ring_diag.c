static char sccsid[] = "@(#)56	1.4  src/bos/diag/util/ulan/ring_diag.c, dsalan, bos411, 9428A410j 1/3/94 13:32:34";
/*
 * COMPONENT_NAME: LAN service aid 
 *
 * FUNCTIONS:  ring_diag
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

/*----------------------------------------------------------------------*/
/*	Function:	ring_diag					*/
/*	Description:	This function will first send a MAC request	*/
/*			station request to the LAN manager		*/
/*			If the report station state from the LAN	*/
/*			manager is received. Then a NORMAL state	*/
/*			will print out from the screen.			*/
/*			REM (Ring Error Monitor) is runing for 5 minutes*/
/*			hard and soft error analysis is recorded 	*/
/*----------------------------------------------------------------------*/
int	ring_diag()
{
	int                     rc = 0;
        struct session_blk      s_start;
        struct status_block     s_status;
        char                    *read_buf;
        struct  pollfd          poll_struct;
        char                    *p_tmp;
        int                     i;
        int                     j;
        struct devinfo devinfo_s;
	cio_stat_blk_t  cio_get_stat;   /* CIO status block */

	tok_func_addr_t		tok_function_address;	



	display (LAN_STANDBY);

	/* the device must be closed in order for the device 	*/
	/* to be operated in another mode			*/
	if (filedes > 0)
		close (filedes);

	/* this net ID for Ring Error Monitor and MAC frame to 	*/
	/* be received						*/

	s_start.netid = 256;
        s_start.length = 1;
	sprintf(devname,"/dev/%s", resource_name);
        filedes = open (devname, O_RDWR);
        if (filedes == -1 )
        {
		display (RING_DIAG_ERROR);
                clean_up ();
        }
        else
        {
                rc = ioctl (filedes, CIO_START, &s_start);
                if (rc == -1)
		{
			display (RING_DIAG_ERROR);
                	clean_up ();
		}
		else
                {
                	/* set up for poll command complete */
                	poll_struct.fd = filedes;
                	poll_struct.reqevents =  POLLPRI ;
                	poll_struct.rtnevents = 0;
			check_exit_cancel();
			if
               			(( rc = poll ( &poll_struct, 1 , 30000)) > 0)
                	{
                		rc = ioctl (filedes, CIO_GET_STAT,
                			&cio_get_stat);

                		if ((rc != 0) || ( cio_get_stat.code
                			!= CIO_START_DONE)
                			|| ( cio_get_stat.option[0] != CIO_OK))
                		{
					/* start command is not completed */
					display (RING_DIAG_ERROR);
					clean_up();
                		}
			}
                       	else
                       	{
				/* start command is not completed	*/
				display (RING_DIAG_ERROR);
				clean_up();
                    	}
			check_exit_cancel();
               	}

		/* geting the adapter information	*/
		rc = ioctl(filedes, IOCINFO, &devinfo_s);
               	if ( rc != 0)
               	{
			/* Do not have device information 	*/
			/* Just quit   				*/
			display (RING_DIAG_ERROR);
                       	clean_up();
		}


		/* here we have device information			*/
		/* copy the network address				*/
               	memcpy (network_address, devinfo_s.un.token.net_addr, 6);

		tok_function_address.status = CIO_OK;	
		tok_function_address.netid = 256;
		tok_function_address.opcode = TOK_ADD;

		/* this functional address is for RING ERROR MONITOR 	*/
		/* tokenring Architecture page 3-10		*/

		tok_function_address.func_addr = 0x00000008;
        	rc = ioctl(filedes, TOK_FUNC_ADDR, &tok_function_address);
               	if ( rc != 0)
               	{
			/* cannot set Ring Error Monitor	*/
			display (RING_DIAG_ERROR);
			clean_up();
           	}
		else
		{
                	/* set up for poll command complete */
                	poll_struct.fd = filedes;
                	poll_struct.reqevents =  POLLPRI ;
                	poll_struct.rtnevents = 0;
			check_exit_cancel();
			if
               			(( rc = poll ( &poll_struct, 1 , 30000)) > 0)
                	{
                		rc = ioctl (filedes, CIO_GET_STAT,
                			&cio_get_stat);

				/*
				 * Temporary comment out	*

                		if ((rc != 0) || ( cio_get_stat.code
                			== CIO_ASYNC_STATUS)
					|| (cio_get_stat.option[0] != CIO_OK))
                		{
					display (RING_DIAG_ERROR);
					clean_up();
                		}
				*/
			}
			else
			{
				/* 
				 Ignore this error
				 Because one of the function does not work
				display (RING_DIAG_ERROR);
				clean_up();
				*/
			}
		}
	}
	check_exit_cancel();
	rc = get_station_state();
	if (rc == TRUE )
	{
		display_normal_testing();
	}
	else 
		display_ring_abnormal();
	get_all_stations();
	/* do ring_diagnostics for 5 minutes	*/
	check_exit_cancel();
	read_mac_frame();
}
/*----------------------------------------------------------------------*/
/*	Function:		get_station_state()			*/
/*	Description:		this function will send a MAC frame	*/
/*				Request Station State to the LAN	*/
/*				manager and waiting for Report Station	*/
/*				State from the LAN manager. If the	*/
/*				state of the station is normal. Then	*/
/*				a NORMAL state of the station is 	*/
/*				displayed.				*/ 
/*----------------------------------------------------------------------*/

int get_station_state()
{
	t_mac_report_station	request_station_state;
	t_mac_report_station	response_station_state;
	unsigned char		active_monitor[6]={0xC0, 0x00, 0x00, 0x00,
						   0x00, 0x01};
	int	i, j;		/* using as loop counters	*/
	int	length;
	int	rc;
	char	*p_tmp;
	unsigned char	in_pkt[4096];
	int	return_code = BAD;



	/* Setting up the request station data and send it to active 	*/
	/* monitor							*/

	memcpy (request_station_state.dest, active_monitor,6);
	memcpy (request_station_state.source, network_address,6);
	request_station_state.ac = 0x10;
	request_station_state.fc = 0x00;
	request_station_state.major_vector_length = 0x0008;
	request_station_state.major_vector_class = 0x04;
	request_station_state.major_vector_command = 0x0F;
	request_station_state.subvector_length = 0x04;
	request_station_state.subvector_id = 0x09;
	request_station_state.data[0] = 0x00;
	request_station_state.data[1] = 0x01;

	for (i=2; i<512; i++)
		request_station_state.data[i] = 0x00;
			
	length = 22;

	/* sending a MAC Request Station State to the Active Monitor	*/
	/* the destination address is 0xC00000000001			*/

	rc = write(filedes, &request_station_state, length);

 	if ( ( rc != GOOD ) && ( rc != length))
       	{
		display_ring_abnormal();
		return (0);
	}


	poll_struct.fd = filedes;
       	poll_struct.rtnevents =0;
       	poll_struct.reqevents =  POLLIN;
       	set_timer(30);
       	alarm_timeout = FALSE;
       	while (!alarm_timeout)
       	{
		check_exit_cancel();
		/* polling for receive frame	*/
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
				length = rc;
				/* parse to check if the received frame */
				/* is the response frame from our	*/
				/* sending request			*/
				rc = parse_response_station_state(length, 
					&in_pkt[0]);
				if (rc == TRUE)
				{
					display_normal_testing();
					alarm_timeout = TRUE;
					return_code = TRUE;
				}
			}
		}
	}
	return (return_code);
}
/*----------------------------------------------------------------------*/
/*	Function:	parse_response_station_state			*/
/*	Description:	this function will parse a frame make sure that */
/*			the station will get a response from request	*/
/*			station state that is sent  			*/
/*	Return		TRUE : if response is found			*/
/*			FALSE: else					*/
/*----------------------------------------------------------------------*/

int parse_response_station_state ( length, data)
int	length;
unsigned char	data[];
{
int	i;
	int	length_tmp;
	unsigned char command_byte;
	if (net_type == TOKEN)
		command_byte = data[35];

	if ((command_byte == RESPONSE_STATION_STATE) ||
	    (command_byte == STANDBY_MONITOR_PRESENT))

	{
		return (TRUE);
	}
	else if (command_byte == 0x02)
	{
		length_tmp = data[33];
		length_tmp -= 4;
		memcpy (&BEACONING_network_address[0], &data[8],6);
		parse_beacon_error (&data[36], length_tmp);
		clean_up();
	}
	return (FALSE);
}

/*----------------------------------------------------------------------*/
/*									*/
/* Funtion:	get_all_stations					*/
/* Description: this function will get all the stations			*/
/*		networks address in the local ring by sending		*/
/*		the test command					*/
/*----------------------------------------------------------------------*/

get_all_stations ()
{
	t_send_stations_test	t_test;
	int		rc;
	int		i, j;
	int		length;
	unsigned char	*p_tmp;
	unsigned char	in_pkt[4096];

	check_exit_cancel();

	/* Setting up a test MAC frame and send out to the wire	*/

	t_test.ac = 0x10;
	t_test.fc = 0x40;
	memset (t_test.dest, 0xFF, 6);
	memcpy (t_test.source, network_address, 6);
	t_test.dsap = 0xAA;
	t_test.ssap = 0xAA;
	
	t_test.command_byte = 0xF3;

	length = sizeof (t_send_stations_test);
	

	/* send the test command to the network		*/
	rc = write(filedes, &t_test, sizeof (t_send_stations_test) );
	
 	if ( ( rc != GOOD ) && ( rc != length))
       	{
		display_ring_abnormal();
		return (ERROR);
	}

	/* polling for receive frame		*/

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
                       	rc = read (filedes, in_pkt, 4096);
			if (rc == -1)
                       	{
				display (RING_DIAG_ERROR);
				return (ERROR);
			}
                       	else
			{
				length = rc;
				add_local_stations( &in_pkt[8]);
			}
		}
	}

}

/*----------------------------------------------------------------------*/
/*	Function:	add_local_stations				*/
/*	Description:	This function will add a station ip in		*/
/*			the local ring stations structure		*/
/*----------------------------------------------------------------------*/

add_local_stations ( new_station_address)
char	*new_station_address;
{
	int	i;
	int	rc;

	check_exit_cancel();
	for (i =0; i< number_of_local_stations; i++)
	{
		rc = memcmp ( new_station_address, 
				station_ids[i].network_address, 6);
		if (rc == 0)
			return (0);
	}
	memcpy ( station_ids[number_of_local_stations].network_address,
		new_station_address, 6);
	number_of_local_stations++;

	do_rarp (new_station_address, source_ip, destination_ip);

	return (0);

}
	

/*----------------------------------------------------------------------*/
/*	Function:	display_normal_testing ()			*/
/*	Description:	This function will display if the elapsed time 	*/
/*			From the previous display is > 15 second	*/
/*			If it is, then display time remaining		*/
/*----------------------------------------------------------------------*/
display_normal_testing()
{
        long            temp;
	long		time_loop;


	check_exit_cancel();
	/* time allowed is set to 5 minustes or 300 seconds */
	if (time_allowed > 0)
	{
        	time (&current_time) ;
		time_loop = current_time - previous_time;
		time_loop = time_loop % 86400L;
		if ( time_loop >= 15 )
		{
			/* calculate the time remaining to run	*/
        		second_remain = (time_allowed - time_loop) %60;
        		minute_remain = (time_allowed - time_loop) /60;
		
			if (second_remain >=0)
				display (RING_NORMAL_AND_TESTING);
			/* update the previous time	*/
			previous_time = current_time;
			time_allowed = time_allowed - time_loop ;
		}
	}

} 
/*----------------------------------------------------------------------*/
/*	Function:	display_ring_abnormal()			        */
/*	Description:	This function will display if the elapsed time 	*/
/*			From the previous display is > 15 second	*/
/*			If it is, then display time remaining		*/
/*----------------------------------------------------------------------*/
display_ring_abnormal()
{
        long            temp;
	long		time_loop;


	check_exit_cancel();
	/* time allowed is set to 5 minustes or 300 seconds */
	if (time_allowed > 0)
	{
        	time (&current_time) ;
		time_loop = current_time - previous_time;
		time_loop = time_loop % 86400L;
		if ( time_loop >= 15 )
		{
			/* calculate the time remaining to run	*/
        		second_remain = (time_allowed - time_loop) %60;
        		minute_remain = (time_allowed - time_loop) /60;
		
			if (second_remain >=0)
				display (RING_ABNORMAL);
			/* update the previous time	*/
			previous_time = current_time;
			time_allowed = time_allowed - time_loop ;
		}
	}

} 
