/* @(#)44	1.4  src/bos/diag/util/ulan/message.h, dsalan, bos411, 9428A410j 4/1/94 14:18:55 */
/*
 *   COMPONENT_NAME: DSALAN
 *
 *   FUNCTIONS: 
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

#include "ulan_msg.h"
/*--------------------------------------------------------------*/
/*      Messages and menus                                      */
/*--------------------------------------------------------------*/

char msgstr[512];
char msgstr1[512];
char msgstr2[512];
char msgstr3[512];


struct msglist warning[]=
        {
                { 1, RING_SELECTION             },
                { 1, SIXTEEN_MEG_RING           },
                { 1, FOUR_MEG_RING              },
                { 1, UNKNOWN                    },
                { 1, WARNING                    },
        };

struct msglist source_ip_error[]=
        {
                { 1, LAN_SELECTION             },
                { 1, SOURCE_IP_ERROR            },
		{ 1, SA_ENTER			},
        };
		

struct msglist destination_ip_error[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, DESTINATION_IP_ERROR	},
		{ 1, SA_ENTER			},
        };
		
struct msglist gateway_ip_error[]=
        {
                { 1, LAN_SELECTION             },
                { 1, GATEWAY_IP_ERROR            },
		{ 1, SA_ENTER			},
        };
		
struct msglist transmission_successful[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, GOOD_TRANSMISSION         	},
		{ 1, SA_ENTER		 	},
        };
		

struct msglist ethernet_busy[]=
        {
                { 1, ADAPTER_BUSY              	},
                { 1, ETHERNET_ADAP             	},
                { 1, NEW_FREE_RESOURCE  	},
		{ 1, SA_ENTER		 	},
        };
		
struct msglist tokenring_busy[]=
        {
                { 1, ADAPTER_BUSY              	},
                { 1, TOKENRING_ADAP            	},
                { 1, NEW_FREE_RESOURCE        	},
		{ 1, SA_ENTER		 	},
        };

struct msglist fddi_busy[]=
        {
                { 1, ADAPTER_BUSY              	},
                { 1, FDDI_ADAP            	},
                { 1, NEW_FREE_RESOURCE          },
		{ 1, SA_ENTER		 	},
        };

struct msglist no_adapter_to_test[]=
        {
                { 1, NO_ADAPTER_TO_TEST        	},
		{ 1, SA_ENTER		 	},
        };

struct msglist duplicate_net_id[]=
        {
                { 1, DUPLICATE_NET_ID          	},
		{ 1, SA_ENTER		 	},
        };

struct msglist unidentified_error[]=
        {
                { 1, UNIDENTIFIED_ERROR         },
		{ 1, SA_ENTER		 	},
        };

struct msglist network_down[]=
        {
                { 1, LAN_SELECTION		},
                { 1, NETWORK_DOWN               },
		{ 1, SA_ENTER		 	},
        };

struct msglist hardware_error[]=
        {
                { 1, HARDWARE_ERROR             },
		{ 1, SA_ENTER		 	},
        };

struct msglist pretest_info[]=
        {
                { 1, LAN_SELECTION_NO_DEVICE 	},
                { 1, PRETEST_INFO		},
		{ 1, SA_ENTER		 	},
        };

struct msglist option_pretest[]=
        {
                { 1, LAN_SELECTION_NO_DEVICE 	},
                { 1, OPTION_PRETEST		},
		{ 1, SA_ENTER		 	},
        };

struct msglist connectivity_or_monitor[]=
        {
                { 1, SELECTION_WITH_OPTION	},
                { 1, CONNECTIVITY		},
                { 1, MONITORING   		},
                { 1, SELECT                     },
        };
struct msglist function_failure[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, FUNCTION_FAILURE		},
                { 1, ENSURE_1        		},
		{ 1, SA_ENTER		 	},
        };

struct msglist signal_loss[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, SIGNAL_LOSS		},
                { 1, ENSURE_1        		},
		{ 1, ACTION_1			},
		{ 1, SA_ENTER		 	},
        };

struct msglist wire_fault[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, WIRE_FAULT			},
                { 1, ENSURE_1        		},
		{ 1, SA_ENTER		 	},
        };

struct msglist frequency_error[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, FREQUENCY_ERROR		},
                { 1, ENSURE_1        		},
		{ 1, SA_ENTER		 	},
        };

struct msglist time_out[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, TIME_OUT_ERROR		},
                { 1, ENSURE_1        		},
		{ 1, SA_ENTER		 	},
        };

struct msglist warning_1[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, YES_OPTION			},
                { 1, NO_OPTION			},
                { 1, WARNING_1                  },
        };

struct msglist ring_failure[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, RING_FAILURE		},
                { 1, ENSURE_1        		},
		{ 1, ACTION_1			},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist ring_beaconing[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, RING_BEACONING		},
                { 1, ENSURE_2        		},
		{ 1, ACTION_1			},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist duplicate_address[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, DUPLICATE_ADDRESS		},
                { 1, ACTION_2        		},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist request_parameters[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, REQUEST_PARAMETERS		},
                { 1, ENSURE_1        		},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist remove_received[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, REMOVE_RECEIVED		},
                { 1, ENSURE_1        		},
		{ 1, ACTION_1			},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist impl_force_received[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, IMPL_FORCE_RECEIVED	},
                { 1, ENSURE_1        		},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist duplicate_modifier_address[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, OPEN_ERROR                	},
                { 1, DUPLICATE_MODIFIER_ADDRESS	},
                { 1, ACTION_2        		},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist ring_state_beacon_no_naun[]=
        {
                { 1, RING_BEACON_NO_NAUN},
                { 1, ENSURE_1        		},
		{ 1, ACTION_1			},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist ring_state_beacon_with_naun[]=
        {
                { 1, RING_BEACON_WITH_NAUN},
                { 1, ENSURE_1        		},
		{ 1, ACTION_1			},
		{ 1, SA_ENTER		 	},
		0
        };
struct msglist bad_transmission[]=
        {
                { 1, LAN_SELECTION             	},
                { 1, BAD_TRANSMISSION         	},
		{ 1, ENSURE_3			},
		{ 1, SA_ENTER		 	},
        };
		
struct msglist ring_diag_error[]=
        {
                { 1, RING_DIAG_ERROR		},
                { 1, ENSURE_2        		},
		{ 1, SA_ENTER		 	},
		0
        };

struct msglist done_ring_monitor[]=
        {
                { 1, DONE_RING_MONITOR          },
		{ 1, SA_ENTER		 	},
        };

ASL_SCR_INFO 	asi_warning[DIAG_NUM_ENTRIES(warning)];
ASL_SCR_INFO 	asi_source_ip_error[DIAG_NUM_ENTRIES(source_ip_error)];
ASL_SCR_INFO 	asi_destination_ip_error[DIAG_NUM_ENTRIES(destination_ip_error)];
ASL_SCR_INFO 	asi_gateway_ip_error[DIAG_NUM_ENTRIES(gateway_ip_error)];
ASL_SCR_INFO 	asi_transmission_successful[DIAG_NUM_ENTRIES(transmission_successful)];

ASL_SCR_INFO 	asi_ethernet_busy[DIAG_NUM_ENTRIES(ethernet_busy)];
ASL_SCR_INFO 	asi_tokenring_busy[DIAG_NUM_ENTRIES(tokenring_busy)];
ASL_SCR_INFO 	asi_fddi_busy[DIAG_NUM_ENTRIES(fddi_busy)];
ASL_SCR_INFO 	asi_no_adapter_to_test[DIAG_NUM_ENTRIES(no_adapter_to_test)];
ASL_SCR_INFO 	asi_duplicate_net_id[DIAG_NUM_ENTRIES(duplicate_net_id)];
ASL_SCR_INFO 	asi_unidentified_error[DIAG_NUM_ENTRIES(unidentified_error)];
ASL_SCR_INFO 	asi_network_down[DIAG_NUM_ENTRIES(network_down)];
ASL_SCR_INFO 	asi_hardware_error[DIAG_NUM_ENTRIES(hardware_error)];
ASL_SCR_INFO 	asi_pretest_info[DIAG_NUM_ENTRIES(pretest_info)];
ASL_SCR_INFO 	asi_option_pretest[DIAG_NUM_ENTRIES(option_pretest)];
ASL_SCR_INFO 	asi_connectivity_or_monitor[DIAG_NUM_ENTRIES(connectivity_or_monitor)];
ASL_SCR_INFO 	asi_function_failure[DIAG_NUM_ENTRIES(function_failure)];
ASL_SCR_INFO 	asi_signal_loss[DIAG_NUM_ENTRIES(signal_loss)];
ASL_SCR_INFO 	asi_wire_fault[DIAG_NUM_ENTRIES(wire_fault)];
ASL_SCR_INFO 	asi_frequency_error[DIAG_NUM_ENTRIES(frequency_error)];
ASL_SCR_INFO 	asi_time_out[DIAG_NUM_ENTRIES(time_out)];
ASL_SCR_INFO 	asi_warning_1[DIAG_NUM_ENTRIES(warning_1)];
ASL_SCR_INFO 	asi_ring_failure[DIAG_NUM_ENTRIES(ring_failure)];
ASL_SCR_INFO 	asi_ring_beaconing[DIAG_NUM_ENTRIES(ring_beaconing)];
ASL_SCR_INFO 	asi_duplicate_address[DIAG_NUM_ENTRIES(duplicate_address)];
ASL_SCR_INFO 	asi_request_parameters[DIAG_NUM_ENTRIES(request_parameters)];
ASL_SCR_INFO 	asi_remove_received[DIAG_NUM_ENTRIES(remove_received)];
ASL_SCR_INFO 	asi_impl_force_received[DIAG_NUM_ENTRIES(impl_force_received)];
ASL_SCR_INFO 	asi_duplicate_modifier_address[DIAG_NUM_ENTRIES(duplicate_modifier_address)];
ASL_SCR_INFO 	asi_ring_state_beacon_no_naun[DIAG_NUM_ENTRIES(ring_state_beacon_no_naun)];
ASL_SCR_INFO 	asi_ring_state_beacon_with_naun[DIAG_NUM_ENTRIES(ring_state_beacon_with_naun)];
ASL_SCR_INFO 	asi_bad_transmission[DIAG_NUM_ENTRIES(bad_transmission)];
ASL_SCR_INFO 	asi_ring_diag_error[DIAG_NUM_ENTRIES(ring_diag_error)];
ASL_SCR_INFO 	asi_done_ring_monitor[DIAG_NUM_ENTRIES(done_ring_monitor)];
