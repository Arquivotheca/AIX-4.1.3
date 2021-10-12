static char sccsid[] = "@(#)50	1.7  src/bos/diag/util/ulan/saring.c, dsalan, bos411, 9428A410j 1/3/94 13:32:38";
/*
 * COMPONENT_NAME: LAN service aid 
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992 ,1993
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
#include "saring.h"
#include "ulan_msg.h"
#include "netdefs.h"
extern int errno;
extern  nl_catd diag_catopen();
void    intr_handler(int);      /* da interrupt handler                 */

/*----------------------------------------------------------------------------
 * NAME: main         
 *
 * FUNCTION:	main function of LAN Service Aid 
 *
 * RETURNS: NONE
 *---------------------------------------------------------------------------*/
int	filedes;
ASL_SCR_TYPE 	device_menutype	=	DM_TYPE_DEFAULTS;

                      
main()

{
	int 			rc = 0;		
	struct session_blk 	s_start;
	struct	pollfd		poll_struct;
	int			i;
	struct devinfo devinfo_s;
	cio_stat_blk_t  cio_get_stat;   /* CIO status block */


	prior_state = -1;
	variable_init();
	setlocale (LC_ALL, "");

        /*..............................*/
        /* General Initialization       */
        /*..............................*/

        /* initializing interrupt handler */
        (void) signal (SIGINT, intr_handler);

	monitoring = TRUE;
	rc = time (&begin_time);
	previous_time = begin_time;

        /* initialize odm       */
        init_dgodm();

	/* intializing the asl	*/
       	diag_asl_init ("NO_TYPE_AHEAD");
       	catd = diag_catopen (MF_ULAN,0);
	database_changed = FALSE;
	beacon_mac_database_changed = FALSE;
	add_beacon_mac_database= FALSE;
	ring_speed_database_changed=FALSE;
	add_ring_speed_database=FALSE;



	/* Displaying general information about this service aid	*/
	/* Menus 802-170 and 802-171					*/

	display(OPTION_PRETEST);
	display(PRETEST_INFO);

 
	/* find all LAN adapter in the system by searching thru database */
	rc = find_all_adapters();
	if (rc == 0)	/* there is no adapter to be tested */
	{
		display (NO_ADAPTER_TO_TEST);
		clean_up();
	}
	else
	{
		/* create a test list					*/
		/* and let the user choose which adapter to test	*/

		create_list ();	
		/* display a test list and wait for user input	*/
		rc = diag_display(0x802173, catd, NULL, DIAG_IO,
                   ASL_DIAG_LIST_CANCEL_EXIT_SC, &device_menutype, device_info);
		if ( rc == ASL_COMMIT)
		{
			rc = DIAG_ITEM_SELECTED(device_menutype);
			strcpy (resource_name, device_found[rc-1]);
			strcpy (device_location, location[rc-1]);
			net_type = device_type[rc-1];
		}
		else 
			clean_up();
	}


	/* this will set to the value in do_arp	*/
	/* assuming that it's not 802_3		*/
	is_802_3 = -1;


	/* setting the netid for different devices	*/

	if  (net_type == TOKEN) 
	{
		s_start.netid = 0xAA;
        	s_start.length = 1;
		sprintf(devname,"/dev/%s", resource_name);
		display (MONITORING);
		display(TOKEN_RING_SPEED);
		display(LAN_STANDBY);
		change_data_base();
	}
	else if (net_type == ETHERNET)
	{
		/* this is the TYPE field value that we are going to receive */
		s_start.netid = 0x0806;
        	s_start.length = 2;
		sprintf(devname,"/dev/%s", resource_name);
		display(LAN_STANDBY);
		monitoring = FALSE;
	} 
	else if ( net_type == FDDI)
	{
		s_start.netid = 0xAA;
        	s_start.length = 1;
		sprintf(devname,"/dev/%s", resource_name);
		display (3);
		monitoring = FALSE;
	}
	else	/* future extension	*/
	{
		display (UNIDENTIFIED_ERROR);
		clean_up ();
	}

	prior_state = get_device_status(resource_name);
	rc = unconfigure_lpp_device();

	if (prior_state != BAD)
	{
		if (!monitoring)  /* running ring monitoring */
		{
			display(IP_INPUT);
			display(LAN_STANDBY);
		}
		else	/* running ring diagnostics */ 
   		{
			open_device ();
			filedes = open (devname, O_RDWR);
			ring_diag();
			display (DONE_RING_MONITOR);
			clean_up();
		}
	}
		
	open_device ();
	errno = 0;
	filedes = open (devname, O_RDWR);
	if (filedes <= 0)	/* cannot open device	*/
	{
		switch (errno)
		{
			case EBUSY:
			case ENODEV:
			{
				if (net_type == TOKEN)
					display(TOKENRING_BUSY);
				else if (net_type == ETHERNET)
					display (ETHERNET_BUSY);
				else if (net_type == FDDI)
					display (FDDI_BUSY);
				else
					display (UNIDENTIFIED_ERROR);
			}
			break;

			default:
				display(UNIDENTIFIED_ERROR);
			break;
		}
		clean_up();
	}
	else
	{
		errno = 0;
		rc = ioctl (filedes, CIO_START, &s_start);
		if (rc == -1)	/* Start device failure	*/
		{
			if (net_type == TOKEN)
			{
				switch (errno)
				{
					case EADDRINUSE:
					{
						display(TOKENRING_BUSY);
					}
					break;
					default:
						display(UNIDENTIFIED_ERROR);
					break;
				}
				clean_up ();
			}
			else if (net_type == ETHERNET)
			{
				switch (errno)
                        	{

                                	case EADDRINUSE:
                                	{
                                        	display(DUPLICATE_NET_ID);
                                	}
                                	break;

                                	default:
                                	{
                                        	display(UNIDENTIFIED_ERROR);
                                	}
                                	break;
                        	}
				clean_up();
			}
		}
		else
		{
			check_exit_cancel();
			/* set up for polling  start device command complete */
                        poll_struct.fd = filedes;
                        poll_struct.reqevents =  POLLPRI ;
                        poll_struct.rtnevents = 0;

                        if      /* command status available */
                                (( rc = poll ( &poll_struct, 1 , ONEMIN)) > 0)
                        {
                                rc = ioctl (filedes, CIO_GET_STAT,
                                                &cio_get_stat);


                                if ((rc != 0) || ( cio_get_stat.code
                                                != CIO_START_DONE)
                                    || ( cio_get_stat.option[0] != CIO_OK))
                                {
				   if (cio_get_stat.code == CIO_ASYNC_STATUS)
				   {
					/* debugging purpose only	*/
				   }
				   display (UNIDENTIFIED_ERROR);
				   clean_up();
                                }

                        }
                        else	/* start device is not completed	*/
                        {
				display (UNIDENTIFIED_ERROR);
				clean_up();
                        }

			/* get the device information		*/
			/* and network address from the device	*/

			check_exit_cancel();
			rc = ioctl(filedes, IOCINFO, &devinfo_s);
			if ( rc != 0)
			{
				display (UNIDENTIFIED_ERROR);
				clean_up();
			}
			else
			{
				/* get the network address 		*/

				if (net_type == TOKEN)
				{
					memcpy (network_address, 
						devinfo_s.un.token.net_addr, 6);
				}
				else if (net_type == ETHERNET)
				{
					memcpy (network_address, 
					   devinfo_s.un.ethernet.net_addr, 6);
				}
				else if (net_type == FDDI)
				{
					memcpy (network_address, 
					   devinfo_s.un.fddi.netaddr, 6);
				}
			}
			
			/* start different netid for ETHERNET		*/
 			/* so we can receive different types of packet	*/

			if (net_type == ETHERNET)
				start_device();


			/* if do_arp or icmp fails, retries 3 times	*/

			
			check_exit_cancel();
			number_of_loops = 3;
			for (i=1; i<=number_of_loops; i++)
			{
				check_exit_cancel();
		
				/* first assuming that the destination 	*/
				/* station is in the same ring		*/
				rc =do_arp(&dest_net_address[0],source_ip, 
				   destination_ip);

				if ((rc != 0) && (gateway_ip != 0))
				{
					/* arping the gateway	*/
					rc =do_arp(&dest_net_address[0],
						source_ip, gateway_ip);
				}
				if ((rc != GOOD) && ( i == number_of_loops))
				{
					/* issue a warning message	*/
					display_no_network_address();
					clean_up ();
				}
				if ( rc == GOOD)
				{
					/* here we have the network address */
					/* of the destination station	    */
					/* trying icmping the destination   */
					/* station			    */
					rc = do_icmp();
					if (rc == ET_IP)
					{
						display (GOOD_TRANSMISSION);
						break;
					}
				}
			}
		}

	}
	if (net_type == TOKEN)
	{
		change_data_base();
	}
	clean_up();
}

/*------------------------------------------------------------------------
 * NAME: intr_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * RETURNS: NONE
 *------------------------------------------------------------------------*/

void intr_handler(int sig)
{
        clean_up();
}
open_device ()
{
	char	diag_device_name[64];

	errno = 0;
	sprintf(diag_device_name,"/dev/%s/D", resource_name);
	filedes = open (diag_device_name, O_RDWR);
	if (filedes <= 0)	/* cannot open device	*/
	{
		switch (errno)
		{
			case EBUSY:
			case ENODEV:
			{
				if (net_type == TOKEN)
					display(TOKENRING_BUSY);
				else if (net_type == ETHERNET)
					display (ETHERNET_BUSY);
				else if (net_type == FDDI)
					display (FDDI_BUSY);
				else
					display (UNIDENTIFIED_ERROR);
			}
			break;

			default:
				display(UNIDENTIFIED_ERROR);
			break;
		}
		clean_up();
	}
	else 
		close (filedes);
}
/*------------------------------------------------------------------------
 * NAME: variable_init
 *
 * FUNCTION: Initial gobal variables
 *
 * RETURNS: NONE
 *------------------------------------------------------------------------*/
variable_init()
{

	database_changed=FALSE;
	add_ring_speed_database=FALSE;
	ring_speed_database_changed=FALSE;
	beacon_mac_database_changed=FALSE;
	add_beacon_mac_database=FALSE;
	monitoring=FALSE;
	number_of_loops=1;
	line_errors=0; 
	internal_errors=0;
	burst_errors=0;
	ac_errors=0;
	abort_del_transmit_errors=0;
	lost_frame_errors=0;
	receiver_congestion=0;
	frame_copied_errors=0;
	frequency_errors=0;
	token_errors=0;
	number_of_local_stations=0;
	time_allowed=300;
	time_run=0;
	previous_time=0;

}
