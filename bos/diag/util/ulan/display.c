static char sccsid[] = "@(#)40	1.5  src/bos/diag/util/ulan/display.c, dsalan, bos41J, 9523B_all 6/6/95 16:16:29";
/*
 * COMPONENT_NAME: LAN service aid 
 *
 * FUNCTIONS:  display
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
#include <asl.h>
#include <curses.h> 
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
#include "message.h"
extern 	ASL_SCR_TYPE	device_menutype;

/*......................................................................*/
/*
 * NAME: 	display 
 *
 * FUNCTION: display the different menus. 
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS:
 */
/*......................................................................*/

display (msg_number)
int 	msg_number;

{
	int	rc;
	int 	input;
	char	buffer[80];
	int	return_code = BAD;
	ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

	switch (msg_number)
	{
	   case TOKEN_RING_SPEED:
		/* display the menu ask the user to choose the ring speed */
	   {
		rc = diag_display(0x802175, catd, warning,
                                DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes, asi_warning);
                sprintf(msgstr, asi_warning[0].text,
                                resource_name, device_location);
                free (asi_warning[0].text);
                asi_warning[0].text = msgstr;

                /* set the cursor points to UNKNOWN */
                menutypes.cur_index = 3;
                rc = diag_display(0x802175, catd, NULL, DIAG_IO,
                      ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,asi_warning);

		if ((check_asl_stat (rc)) == QUIT) 
			clean_up();

                rc = DIAG_ITEM_SELECTED (menutypes);
                if      /* if user does not have the wrap NTF */
                   ( rc == SIXTEEN)
                {
			ring_speed = 16;
                }
		else if ( rc == FOUR)
		{
			ring_speed =4;
		}
		else
			clean_up();
		

	   }
	   break;

	   case IP_INPUT:
		/* ask the user to input Internet addresses	*/
	   {
		while (return_code == BAD)
		{
			return_code = get_ip_values();
		}
			
	   }
	   break;

	   case LAN_STANDBY:
		/* stanby menu		*/
	   {
		rc = diag_msg_nw ( 0x802174, catd, 1, STANDBY, 
					resource_name, device_location);
	   }
	   break;

	   case RING_DIAGNOSTICS: 
	   {
		rc = diag_msg_nw ( 0x899002, catd, 1, RING_DIAGNOSTICS, 
					resource_name, device_location);
	   }
	   break;


	   case ETHERNET_BUSY:
		/* display the menu to inform the user that		*/
		/* Service Aid cannot be run because the device is busy	*/
	   {
 		rc = diag_display(0x802271, catd,
                        ethernet_busy, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_ethernet_busy);
                sprintf(msgstr, asi_ethernet_busy[0].text);
                free (asi_ethernet_busy[0].text);
                asi_ethernet_busy[0].text = msgstr;
                sprintf(msgstr1, asi_ethernet_busy[1].text,
                                resource_name, device_location);
                free (asi_ethernet_busy[1].text);
                asi_ethernet_busy[1].text = msgstr1;
                sprintf(msgstr2, asi_ethernet_busy[2].text);
                free (asi_ethernet_busy[2].text);
                asi_ethernet_busy[2].text = msgstr2;
                sprintf(msgstr3, asi_ethernet_busy[3].text);
                free (asi_ethernet_busy[3].text);
                asi_ethernet_busy[3].text = msgstr3;

		menutypes.max_index=3;
                rc = diag_display(0x802271, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_ethernet_busy);

                if (rc != ASL_ENTER)
                        clean_up();

	   }
	   break; 

	   case TOKENRING_BUSY:
		/* display the menu to inform the user that		*/
		/* Service Aid cannot be run because the device is busy	*/
	   {
 		rc = diag_display(0x802272, catd,
                        tokenring_busy, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_tokenring_busy);
                sprintf(msgstr, asi_tokenring_busy[0].text);
                free (asi_tokenring_busy[0].text);
                asi_tokenring_busy[0].text = msgstr;
                sprintf(msgstr1, asi_tokenring_busy[1].text,
                                resource_name, device_location);
                free (asi_tokenring_busy[1].text);
                asi_tokenring_busy[1].text = msgstr1;
                sprintf(msgstr2, asi_tokenring_busy[2].text);
                free (asi_tokenring_busy[2].text);
                asi_tokenring_busy[2].text = msgstr2;
                sprintf(msgstr3, asi_tokenring_busy[3].text);
                free (asi_tokenring_busy[3].text);
                asi_tokenring_busy[3].text = msgstr3;

		menutypes.max_index=3;
                rc = diag_display(0x802272, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_tokenring_busy);

                if (rc != ASL_ENTER)
                        clean_up();

		
	   }
	   break; 
	   case FDDI_BUSY:
		/* display the menu to inform the user that		*/
		/* Service Aid cannot be run because the device is busy	*/
	   {
 		rc = diag_display(0x802273, catd,
                        fddi_busy, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_fddi_busy);
                sprintf(msgstr, asi_fddi_busy[0].text);
                free (asi_fddi_busy[0].text);
                asi_fddi_busy[0].text = msgstr;
                sprintf(msgstr1, asi_fddi_busy[1].text,
                                resource_name, device_location);
                free (asi_fddi_busy[1].text);
                asi_fddi_busy[1].text = msgstr1;
                sprintf(msgstr2, asi_fddi_busy[2].text);
                free (asi_fddi_busy[2].text);
                asi_fddi_busy[2].text = msgstr2;
                sprintf(msgstr3, asi_fddi_busy[3].text);
                free (asi_fddi_busy[3].text);
                asi_fddi_busy[3].text = msgstr3;

		menutypes.max_index=3;
                rc = diag_display(0x802273, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_fddi_busy);

                if (rc != ASL_ENTER)
                        clean_up();

		
	   }
	   break; 
	
	   case GOOD_TRANSMISSION:
		/* display the menu to tell the user the result of	*/
		/* of good data transmission				*/
	   {
 		rc = diag_display(0x802473, catd,
                        transmission_successful, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_transmission_successful);
                sprintf(msgstr, asi_transmission_successful[0].text,
                                resource_name, device_location);
                free (asi_transmission_successful[0].text);
                asi_transmission_successful[0].text = msgstr;
                sprintf(msgstr1, asi_transmission_successful[1].text,
				source_ip_decimal, destination_ip_decimal);
                free (asi_transmission_successful[1].text);
                asi_transmission_successful[1].text = msgstr1;
                sprintf(msgstr2, asi_transmission_successful[2].text);
                free (asi_transmission_successful[2].text);
                asi_transmission_successful[2].text = msgstr2;

		menutypes.max_index=2;
                rc = diag_display(0x802473, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_transmission_successful);

                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case BAD_TRANSMISSION:
		/* display the menu to inform the user about unsuccessful */
		/* data transmission					  */
	   {
 		rc = diag_display(0x802474, catd,
                        bad_transmission, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_bad_transmission);
                sprintf(msgstr, asi_bad_transmission[0].text,
                                resource_name, device_location);
                free (asi_bad_transmission[0].text);
                asi_bad_transmission[0].text = msgstr;
                sprintf(msgstr1, asi_bad_transmission[1].text,
				source_ip_decimal, destination_ip_decimal);
                free (asi_bad_transmission[1].text);
                asi_bad_transmission[1].text = msgstr1;
                sprintf(msgstr2, asi_bad_transmission[2].text);
                free (asi_bad_transmission[2].text);
                asi_bad_transmission[2].text = msgstr2;

		menutypes.max_index=3;
                rc = diag_display(0x802474, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_bad_transmission);

                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case NO_ADAPTER_TO_TEST:
		/* There is no LAN adapter to test	*/
	   {
 		rc = diag_display(0x802172, catd,
                        no_adapter_to_test, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_no_adapter_to_test);
                sprintf(msgstr, asi_no_adapter_to_test[0].text);
                free (asi_no_adapter_to_test[0].text);
                asi_no_adapter_to_test[0].text = msgstr;
		menutypes.max_index=1;
                rc = diag_display(0x802172, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_no_adapter_to_test);

	   }
	   break;

	   case DUPLICATE_NET_ID:
		/* Inform the user about the Duplicate Net ID	*/
	   {
 		rc = diag_display(0x802002, catd,
                        duplicate_net_id, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_duplicate_net_id);
                sprintf(msgstr, asi_duplicate_net_id[0].text,
                                resource_name, device_location);
                free (asi_duplicate_net_id[0].text);
                asi_duplicate_net_id[0].text = msgstr;
		menutypes.max_index=1;
                rc = diag_display(0x802001, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_duplicate_net_id);

	   }
	   break;

	   case UNIDENTIFIED_ERROR:
		/* Generic miscellaneous error		*/
	   {
 		rc = diag_display(0x802274, catd,
                        unidentified_error, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_unidentified_error);
                sprintf(msgstr, asi_unidentified_error[0].text,
                                resource_name, device_location);
                free (asi_unidentified_error[0].text);
                asi_unidentified_error[0].text = msgstr;
		menutypes.max_index=1;
                rc = diag_display(0x802274, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_unidentified_error);
	   }
	   break;
	
	   case NETWORK_DOWN:
	   {
 		rc = diag_display(0x802377, catd,
                        network_down, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_network_down);
                sprintf(msgstr, asi_network_down[0].text,
                                resource_name, device_location);
                free (asi_network_down[0].text);
                asi_network_down[0].text = msgstr;
		menutypes.max_index=2;
                rc = diag_display(0x802377, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_network_down);
                if (rc != ASL_ENTER)
                        clean_up();

	   }
	   break;
	
	   case HARDWARE_ERROR:
	   {
 		rc = diag_display(0x802172, catd,
                        hardware_error, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_hardware_error);
                sprintf(msgstr, asi_hardware_error[0].text,
                                resource_name, device_location);
                free (asi_hardware_error[0].text);
                asi_hardware_error[0].text = msgstr;
		menutypes.max_index=1;
                rc = diag_display(0x802172, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_hardware_error);

	   }
	   break;

	   /* display the normal state in ring diagnostics	*/
	   case RING_NORMAL_AND_TESTING:
	   {
		rc = diag_msg_nw ( 0x802470, catd, 1, 
			RING_NORMAL_AND_TESTING, resource_name, 
			device_location, minute_remain, second_remain);
	   }
	   break;

	   /* display the abnormal state in ring diagnostics	*/
	   case RING_ABNORMAL:
	   {
		rc = diag_msg_nw ( 0x802470, catd, 1, 
			RING_ABNORMAL, resource_name, 
			device_location, minute_remain, second_remain);
	   }
	   break;

	   case RING_BEACON_NO_NAUN:
		/* display the beaconing state		*/
	   {
 		rc = diag_display(0x802471, catd,
                        ring_state_beacon_no_naun, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_ring_state_beacon_no_naun);
                sprintf(msgstr, asi_ring_state_beacon_no_naun[0].text,
                     resource_name, device_location,NAUN_network_address[0],
		     NAUN_network_address[1], NAUN_network_address[2],
		     NAUN_network_address[3], NAUN_network_address[4],
		     NAUN_network_address[5],
		     BEACONING_network_address[0], BEACONING_network_address[1],
		     BEACONING_network_address[2], BEACONING_network_address[3],
		     BEACONING_network_address[4], 
		     BEACONING_network_address[5]);
                free (asi_ring_state_beacon_no_naun[0].text);
                asi_ring_state_beacon_no_naun[0].text = msgstr;
		menutypes.max_index=3;
                rc = diag_display(0x802471, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_ring_state_beacon_no_naun);
                if (rc != ASL_ENTER)
                        clean_up();

	   }
	   break;

	   case RING_BEACON_WITH_NAUN:
	   {
 		rc = diag_display(0x802472, catd,
                        ring_state_beacon_with_naun, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_ring_state_beacon_with_naun);
                sprintf(msgstr, asi_ring_state_beacon_with_naun[0].text,
                     resource_name, device_location,NAUN_network_address,
		     NAUN_ip);
                free (asi_ring_state_beacon_with_naun[0].text);
                asi_ring_state_beacon_with_naun[0].text = msgstr;
		menutypes.max_index=3;
                rc = diag_display(0x802472, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_ring_state_beacon_with_naun);
                if (rc != ASL_ENTER)
                        clean_up();

	   }
	   break;

	   /* here a beaconing has been captured and NAUN will be	*/
	   /* displayed							*/
	   case NAUN:
		break;

	   case PRETEST_INFO:
	   {
 		rc = diag_display(0x802171, catd,
                        pretest_info, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_pretest_info);
                rc = diag_display(0x802171, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_pretest_info);

                if (rc != ASL_ENTER)
                        clean_up();

	   }
	   break;

	   case OPTION_PRETEST:
	   {
 		rc = diag_display(0x802170, catd,
                        option_pretest, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_option_pretest);
                rc = diag_display(0x802170, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_option_pretest);

                if (rc != ASL_ENTER)
                        clean_up();

	   }
	   break;
	
	   /* This menu will ask the user to choose		*/
	   /* Connectivity testing or monitor the ring		*/
	   case MONITORING:
	   {
		rc = diag_display(0x802174, catd, connectivity_or_monitor,
                                DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes, asi_connectivity_or_monitor);
                sprintf(msgstr, asi_connectivity_or_monitor[0].text,
                                resource_name, device_location);
                free (asi_connectivity_or_monitor[0].text);
                asi_connectivity_or_monitor[0].text = msgstr;

                /* set the cursor points to UNKNOWN */
                menutypes.cur_index = 2;
                menutypes.max_index = 3;
                rc = diag_display(0x802174, catd, NULL, DIAG_IO,
                      ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
			asi_connectivity_or_monitor);

		if ((check_asl_stat (rc)) == QUIT) 
			clean_up();

                rc = DIAG_ITEM_SELECTED (menutypes);
		if ( rc == 2) 
			monitoring = TRUE;
		else
			monitoring = FALSE;
	   }
	   break;

	   case FUNCTION_FAILURE:
	   {
 		rc = diag_display(0x802275, catd,
                        function_failure, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_function_failure);
                sprintf(msgstr, asi_function_failure[0].text,
                                resource_name, device_location);
                free (asi_function_failure[0].text);
                asi_function_failure[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802275, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_function_failure);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case SIGNAL_LOSS:
	   {
 		rc = diag_display(0x802276, catd,
                        signal_loss, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_signal_loss);
                sprintf(msgstr, asi_signal_loss[0].text,
                                resource_name, device_location);
                free (asi_signal_loss[0].text);
                asi_signal_loss[0].text = msgstr;
		menutypes.max_index=5;
                rc = diag_display(0x802276, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_signal_loss);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case WIRE_FAULT:
	   {
 		rc = diag_display(0x802277, catd,
                        wire_fault, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_wire_fault);
                sprintf(msgstr, asi_wire_fault[0].text,
                                resource_name, device_location);
                free (asi_wire_fault[0].text);
                asi_wire_fault[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802277, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_wire_fault);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case FREQUENCY_ERROR:
	   {
 		rc = diag_display(0x802278, catd,
                        frequency_error, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_frequency_error);
                sprintf(msgstr, asi_frequency_error[0].text,
                                resource_name, device_location);
                free (asi_frequency_error[0].text);
                asi_frequency_error[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802278, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_frequency_error);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case TIME_OUT_ERROR:
	   {
 		rc = diag_display(0x802279, catd,
                        time_out, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_time_out);
                sprintf(msgstr, asi_time_out[0].text,
                                resource_name, device_location);
                free (asi_time_out[0].text);
                asi_time_out[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802279, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_time_out);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case WARNING_1:
	   {
		if (ring_speed == 16 ) input = 4;
		else input = 16;
		rc = diag_display(0x802280, catd, warning_1,
                                DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes, asi_warning_1);
                sprintf(msgstr, asi_warning_1[0].text,
                                resource_name, device_location);
                free (asi_warning_1[0].text);
                asi_warning_1[0].text = msgstr;

                sprintf(msgstr1, asi_warning_1[3].text,
                                ring_speed, input);
                free (asi_warning_1[3].text);
                asi_warning_1[3].text = msgstr1;

                /* set the cursor points to NO */
                menutypes.cur_index = 2;
                menutypes.max_index = 3;
                rc = diag_display(0x802280, catd, NULL, DIAG_IO,
                      ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,asi_warning_1);

		if ((check_asl_stat (rc)) == QUIT) 
			clean_up();

                rc = DIAG_ITEM_SELECTED (menutypes);

                if      /* if user chooses NO, exit	*/ 
                   ( rc == 2)
                {
			clean_up();
                }
		

	   }
	   break;

	   case RING_FAILURE:
	   {
 		rc = diag_display(0x802370, catd,
                        ring_failure, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_ring_failure);
                sprintf(msgstr, asi_ring_failure[0].text,
                                resource_name, device_location);
                free (asi_ring_failure[0].text);
                asi_ring_failure[0].text = msgstr;
		menutypes.max_index=5;
                rc = diag_display(0x802370, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_ring_failure);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case RING_BEACONING:
	   {
 		rc = diag_display(0x802371, catd,
                        ring_beaconing, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_ring_beaconing);
                sprintf(msgstr, asi_ring_beaconing[0].text,
                                resource_name, device_location);
                free (asi_ring_beaconing[0].text);
                asi_ring_beaconing[0].text = msgstr;
		menutypes.max_index=5;
                rc = diag_display(0x802371, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_ring_beaconing);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case RING_DIAG_ERROR:
	   {
 		rc = diag_display(0x802476, catd,
                        ring_diag_error, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_ring_diag_error);
                sprintf(msgstr, asi_ring_diag_error[0].text,
                                resource_name, device_location);
                free (asi_ring_diag_error[0].text);
                asi_ring_diag_error[0].text = msgstr;
		menutypes.max_index=2;
                rc = diag_display(0x802476, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_ring_diag_error);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case DUPLICATE_ADDRESS:
	   {
 		rc = diag_display(0x802372, catd,
                        duplicate_address, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_duplicate_address);
                sprintf(msgstr, asi_duplicate_address[0].text,
                                resource_name, device_location);
                free (asi_duplicate_address[0].text);
                asi_duplicate_address[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802372, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_duplicate_address);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case REQUEST_PARAMETERS:
	   {
 		rc = diag_display(0x802373, catd,
                        request_parameters, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_request_parameters);
                sprintf(msgstr, asi_request_parameters[0].text,
                                resource_name, device_location);
                free (asi_request_parameters[0].text);
                asi_request_parameters[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802373, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_request_parameters);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;

	   case REMOVE_RECEIVED:
	   {
 		rc = diag_display(0x802374, catd,
                        remove_received, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_remove_received);
                sprintf(msgstr, asi_remove_received[0].text,
                                resource_name, device_location);
                free (asi_remove_received[0].text);
                asi_remove_received[0].text = msgstr;
		menutypes.max_index=5;
                rc = diag_display(0x802374, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_remove_received);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;


	   case IMPL_FORCE_RECEIVED:
	   {
 		rc = diag_display(0x802375, catd,
                        impl_force_received, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_impl_force_received);
                sprintf(msgstr, asi_impl_force_received[0].text,
                                resource_name, device_location);
                free (asi_impl_force_received[0].text);
                asi_impl_force_received[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802375, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_impl_force_received);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;


	   case DUPLICATE_MODIFIER_ADDRESS:
	   {
 		rc = diag_display(0x802376, catd,
                        duplicate_modifier_address, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_duplicate_modifier_address);
                sprintf(msgstr, asi_duplicate_modifier_address[0].text,
                                resource_name, device_location);
                free (asi_duplicate_modifier_address[0].text);
                asi_duplicate_modifier_address[0].text = msgstr;
		menutypes.max_index=4;
                rc = diag_display(0x802376, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_duplicate_modifier_address);
                if (rc != ASL_ENTER)
                        clean_up();


	   }
	   break;
	  case SOURCE_IP_ERROR:

		rc = diag_display(0x802178, catd,
               		source_ip_error, DIAG_MSGONLY,
                   	ASL_DIAG_KEYS_ENTER_SC,
                       	&menutypes, asi_source_ip_error);
		sprintf(msgstr, asi_source_ip_error[0].text,
			resource_name, device_location);
		asi_source_ip_error[0].text = msgstr;
		sprintf(msgstr1, asi_source_ip_error[1].text);
		asi_source_ip_error[1].text = msgstr1;
		sprintf(msgstr2, asi_source_ip_error[2].text);
		asi_source_ip_error[2].text = msgstr2;

		rc = diag_display(0x802178, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_source_ip_error);

		if (rc != ASL_ENTER) 
			clean_up();
	  break;

	  case DESTINATION_IP_ERROR:
		rc = diag_display(0x802177, catd,
               		destination_ip_error, DIAG_MSGONLY,
                   	ASL_DIAG_KEYS_ENTER_SC,
                       	&menutypes, asi_destination_ip_error);
		sprintf(msgstr, asi_destination_ip_error[0].text,
			resource_name, device_location);
		asi_destination_ip_error[0].text = msgstr;
		sprintf(msgstr1, asi_destination_ip_error[1].text);
		asi_destination_ip_error[1].text = msgstr1;
		sprintf(msgstr2, asi_destination_ip_error[2].text);
		asi_destination_ip_error[2].text = msgstr2;

		rc = diag_display(0x802177, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_destination_ip_error);

		if (rc != ASL_ENTER) 
			clean_up();
	  break;
	
	  case GATEWAY_IP_ERROR:
		rc = diag_display(0x802179, catd,
               		gateway_ip_error, DIAG_MSGONLY,
                   	ASL_DIAG_KEYS_ENTER_SC,
                       	&menutypes, asi_gateway_ip_error);
		sprintf(msgstr, asi_gateway_ip_error[0].text,
			resource_name, device_location);
		asi_gateway_ip_error[0].text = msgstr;
		sprintf(msgstr1, asi_gateway_ip_error[1].text);
		asi_gateway_ip_error[1].text = msgstr1;
		sprintf(msgstr2, asi_gateway_ip_error[2].text);
		asi_gateway_ip_error[2].text = msgstr2;

		rc = diag_display(0x802179, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_gateway_ip_error);

		if (rc != ASL_ENTER) 
			clean_up();
	  break;
	   case DONE_RING_MONITOR:
	   {
 		rc = diag_display(0x802172, catd,
                        done_ring_monitor, DIAG_MSGONLY,
                        ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_done_ring_monitor);
                sprintf(msgstr, asi_done_ring_monitor[0].text,
                                resource_name, device_location);
                free (asi_done_ring_monitor[0].text);
                asi_done_ring_monitor[0].text = msgstr;
		menutypes.max_index=1;
                rc = diag_display(0x802172, catd, NULL, DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC, &menutypes,
			asi_done_ring_monitor);

	   }

	   default:
		break;

	}
}

/*......................................................................*/
/*      Function:       check  asl     return code                      */
/*......................................................................*/
int check_asl_stat ( rc)
int     rc;
{

        int     return_code =0;
        switch ( rc)
        {
                case ASL_CANCEL:
                        DA_SETRC_USER (DA_USER_QUIT);
                        return_code = QUIT;
                        break;

                case ASL_EXIT:
                        DA_SETRC_USER (DA_USER_EXIT);
                        return_code = QUIT;
                        break;
                case ASL_OK:
                case ASL_ENTER:
                case ASL_COMMIT:
                        break;
                default:
                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                        return_code = QUIT;
                        break;

        }
        return (return_code);
} /* check_asl_stat     */

/*......................................................................*/
/* 	Function A							*/
/*......................................................................*/
display_no_network_address ()
{
	int	rc;
	int     line = 0;
	ASL_SCR_INFO	*no_network_address_info;


	no_network_address_info = (ASL_SCR_INFO *)
			 calloc(1, 2*sizeof(ASL_SCR_INFO));
	line =1;
	if (gateway_ip != 0)
	{
		no_network_address_info[0].text = 
		     (char *) diag_cat_gets(catd, 1, NO_NETWORK_ADDRESS_W_GW);

		sprintf (msgstr, no_network_address_info[0].text, 
		     resource_name, device_location);
     		free (no_network_address_info[0].text);
		no_network_address_info[0].text = msgstr;
	}
	else
	{
		no_network_address_info[0].text = 
			(char *) diag_cat_gets(catd, 1, NO_NETWORK_ADDRESS);

		sprintf (msgstr, no_network_address_info[0].text, 
			resource_name, device_location);
     		free (no_network_address_info[0].text);
		no_network_address_info[0].text = msgstr;
	}

	no_network_address_info[line].text = (char *) diag_cat_gets(catd, 1,
	    SA_ENTER);
	device_menutype.max_index = line;
	rc = diag_display(0x802378, catd, NULL, DIAG_IO,
                   ASL_DIAG_KEYS_ENTER_SC, &device_menutype, 
				no_network_address_info);
}
/*......................................................................*/
/*......................................................................*/


int get_ip_values()
{
	int	entry=0;
	int	rc;
	int	i,j;
	int	return_code=GOOD;
	char	*tmp;
	char	*opstring;
	char 	src_ip[]="               ";
	char 	dest_ip[]="               ";
	char 	gateway[]="               ";
	char	*entered_src, *entered_dest, *entered_gateway;
	long	menu_number = 0x802271;

        static struct msglist menulist[] = {
                { 1, INPUT_INTERNET_TITLE },
                { 1, SOURCE_IP },
                { 1, DESTINATION_IP },
                { 1, GATEWAY_IP },
                { 1, INPUT_INTERNET_PROMPT },
                { (int )NULL, (int )NULL}
	};
	static ASL_SCR_INFO menuinfo[DIAG_NUM_ENTRIES(menulist)];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        rc = diag_display(NULL,catd,menulist,DIAG_MSGONLY,
                                NULL,&menutype,menuinfo);

	/* Fill in title with device and location */
	opstring = (char *)malloc(strlen(menuinfo[0].text) + 
			strlen(resource_name) + strlen(device_location) + 5);
        sprintf(opstring, menuinfo[0].text, resource_name, device_location);
        free(menuinfo[0].text);
        menuinfo[0].text = opstring;

	/* Initialize all global data */
	for (i=1; i<4; i++)
	{
      		menuinfo[i].op_type = ASL_LIST_ENTRY; 
		menuinfo[i].entry_type = ASL_TEXT_ENTRY;
      		menuinfo[i].changed = ASL_NO; 
      		menuinfo[i].cur_value_index = 0; 
      		menuinfo[i].default_value_index = 0; 
	}

	/* set up source ip data */
      	menuinfo[1].disp_values = src_ip; 
      	menuinfo[1].entry_size = strlen(src_ip)+2; 
	entered_src = (char *)malloc( strlen (src_ip) + 1 );
	strcpy(entered_src,src_ip);
      	menuinfo[1].data_value = entered_src; 

	/* set up dest ip data */
      	menuinfo[2].disp_values = dest_ip; 
      	menuinfo[2].entry_size = strlen(dest_ip)+2; 
	entered_dest = (char *)malloc( strlen (dest_ip) + 1 );
	strcpy(entered_dest,dest_ip);
      	menuinfo[2].data_value = entered_dest; 

	/* set up gateway ip data - not required */
      	menuinfo[3].disp_values = gateway; 
      	menuinfo[3].entry_size = strlen(gateway)+2; 
	entered_gateway = (char *)malloc( strlen (gateway) + 1 );
	strcpy(entered_gateway,gateway);
      	menuinfo[3].data_value = entered_gateway; 

        rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                        ASL_DIAG_DIALOGUE_SC, &menutype, menuinfo );

    	if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
		clean_up ();


	strcpy (source_ip_decimal, menuinfo[1].data_value);
	strcpy (destination_ip_decimal, menuinfo[2].data_value);
	source_ip = inet_addr (menuinfo[1].data_value);
	destination_ip = inet_addr (menuinfo[2].data_value);
	gateway_ip = inet_addr (menuinfo[3].data_value);
	if ((source_ip == -1) || (source_ip == 0))
	{
		display (SOURCE_IP_ERROR);
		return_code = BAD;

	}
	if ((destination_ip == -1) || (destination_ip == 0))
	{
		display (DESTINATION_IP_ERROR);
		return_code = BAD;
	}
	if (gateway_ip == -1) 
	{
		display (GATEWAY_IP_ERROR);
		return_code = BAD;
	}
	return (return_code);
}
/*......................................................................*/
/*	Function : 	check_exit_cancel				*/
/*	Description:	This function will check if the F3 or F10	*/
/*			is pressed					*/
/*......................................................................*/
check_exit_cancel( )
{
	int	rc;
	rc = diag_asl_read (ASL_DIAG_LIST_CANCEL_EXIT_SC,0,0);

        switch ( rc)
        {
                case ASL_CANCEL:
		{
			clean_up();
                        break;
		}

                case ASL_EXIT:
		{
			clean_up();
                        break;
		}
                case ASL_OK:
                case ASL_ENTER:
                case ASL_COMMIT:
                default:
                        break;

        }
} /* check_exit_cancel */
