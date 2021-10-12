/* @(#)64	1.1  src/bos/diag/da/artic/display.h, daartic, bos411, 9428A410j 4/1/94 15:17:13 */
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "dmultp_msg.h"
/*--------------------------------------------------------------*/
/*      Messages and menus                                      */
/*--------------------------------------------------------------*/


struct msglist do_you_have_78_pin_wrap_plug[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION_MPQP         	},
                { 1, NO_OPTION_MPQP		},
                { 1, DO_YOU_HAVE_78_PIN_WRAP_PLUG	},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_do_you_have_78_pin_wrap_plug[DIAG_NUM_ENTRIES(do_you_have_78_pin_wrap_plug)];

/* menu xxxxxx --- plugging wrap plug */
struct msglist put_78_pin_wrap_plug[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, INSERT_78_PIN_WRAP_PLUG	},
                { 1, MPQP_FINISHED           	},
                0
        };

ASL_SCR_INFO asi_put_78_pin_wrap_plug [DIAG_NUM_ENTRIES(put_78_pin_wrap_plug)];


struct msglist generic_wrap_plug[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION_MPQP         	},
                { 1, NO_OPTION_MPQP		},
                { 1, GENERIC_WRAP_PLUG		},
                {       0, 0            	}

	};

ASL_SCR_INFO 	asi_generic_wrap_plug[DIAG_NUM_ENTRIES(generic_wrap_plug)];

struct msglist no_port_to_run[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, NO_PORT_TO_RUN         	},
                { 1, MPQP_FINISHED           	},
                0
        };

ASL_SCR_INFO asi_no_port_to_run[DIAG_NUM_ENTRIES(no_port_to_run)];

struct msglist which_interface_to_test_port0[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION_MPQP         	},
                { 1, NO_OPTION_MPQP		},
                { 1, DO_YOU_HAVE_78_PIN_WRAP_PLUG	},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_which_interface_to_test_port0[DIAG_NUM_ENTRIES(which_interface_to_test_port0)];

struct msglist cable_test_for_each_port[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION_MPQP         	},
                { 1, NO_OPTION_MPQP		},
                { 1, CABLE_TEST_FOR_EACH_PORT   },
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_cable_test_for_each_port[DIAG_NUM_ENTRIES(cable_test_for_each_port)];

struct msglist remove_78_pin_wrap_plug[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, REMOVE_78_PIN_WRAP_PLUG	},
                { 1, MPQP_FINISHED           	},
                0
        };

ASL_SCR_INFO asi_remove_78_pin_wrap_plug [DIAG_NUM_ENTRIES(remove_78_pin_wrap_plug)];

struct msglist port_0_menu[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, P0_V35},
                { 1, P0_232},
                { 1, P0_X21},
                { 1, P0_422},
                { 1, INTERFACE_CABLE_MESSAGE},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_port_0_menu[DIAG_NUM_ENTRIES(port_0_menu)];


struct msglist put_generic_wrap_plug[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, REMOVE_CABLE		}, 
		{ 1, INSERT_WRAP_PLUG		},
                { 1, MPQP_FINISHED           	},
                0
        };

ASL_SCR_INFO asi_put_generic_wrap_plug[DIAG_NUM_ENTRIES(put_generic_wrap_plug)];

struct msglist remove_generic_wrap_plug[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, REMOVE_WRAP_PLUG		}, 
                { 1, RECONNECT_CABLE         	},
                { 1, MPQP_FINISHED           	},
                0
        };

ASL_SCR_INFO asi_remove_generic_wrap_plug[DIAG_NUM_ENTRIES(remove_generic_wrap_plug)];

struct msglist port_1_menu[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, P1_232},
                { 1, P1_V35},
                { 1, INTERFACE_CABLE_MESSAGE},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_port_1_menu[DIAG_NUM_ENTRIES(port_1_menu)];

struct msglist port_2_menu[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, P2_232},
                { 1, P2_422},
                { 1, INTERFACE_CABLE_MESSAGE},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_port_2_menu[DIAG_NUM_ENTRIES(port_2_menu)];

struct msglist port_3_menu[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, P3_232},
                { 1, INTERFACE_CABLE_MESSAGE},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_port_3_menu[DIAG_NUM_ENTRIES(port_3_menu)];

struct msglist cable_note[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION_MPQP         	},
                { 1, NO_OPTION_MPQP		},
                { 1, CABLE_NOTE			},
                {       0, 0            	}

	};

ASL_SCR_INFO 	asi_cable_note[DIAG_NUM_ENTRIES(cable_note)];


struct msglist port_0_menu_cable[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, CABLE_V35},
                { 1, CABLE_232},
                { 1, CABLE_X21},
                { 1, CABLE_422},
                { 1, CABLE_SELECTION_MESSAGE},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_port_0_menu_cable[DIAG_NUM_ENTRIES(port_0_menu_cable)];

struct msglist port_1_menu_cable[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, CABLE_232},
                { 1, CABLE_V35},
                { 1, CABLE_SELECTION_MESSAGE},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_port_1_menu_cable[DIAG_NUM_ENTRIES(port_1_menu_cable)];

struct msglist port_2_menu_cable[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, CABLE_232},
                { 1, CABLE_422},
                { 1, CABLE_SELECTION_MESSAGE},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_port_2_menu_cable[DIAG_NUM_ENTRIES(port_2_menu_cable)];

struct msglist port_3_menu_cable[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, CABLE_232			},
                { 1, CABLE_SELECTION_MESSAGE	},
                {       0, 0            	}

	};

ASL_SCR_INFO 	asi_port_3_menu_cable[DIAG_NUM_ENTRIES(port_3_menu_cable)];

struct msglist put_generic_cable[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, CONNECT_CABLE		}, 
                { 1, MPQP_FINISHED           	},
                0
        };

ASL_SCR_INFO asi_put_generic_cable[DIAG_NUM_ENTRIES(put_generic_cable)];

struct msglist remove_generic_wrap_plug_from_cable[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY		},
                { 1, REMOVE_WRAP_PLUG_FROM_CABLE	}, 
                { 1, RECONNECT_CABLE         		},
                { 1, MPQP_FINISHED           		},
                0
        };

ASL_SCR_INFO asi_remove_generic_wrap_plug_from_cable[DIAG_NUM_ENTRIES(remove_generic_wrap_plug_from_cable)];

/* This message is for asking the user to remove wrap plug from the	*/
/* tested cable and remove the tested cable from the interface cable	*/
/* insert the wrap plug into the port					*/
struct msglist cable_isolate[]=
        {
                { 1, MPQP_ADVANCED_NO_STANDBY		},
                { 1, CABLE_ISOLATE			},
                { 1, MPQP_FINISHED           		},
                0
        };

ASL_SCR_INFO asi_cable_isolate[DIAG_NUM_ENTRIES(cable_isolate)];

struct msglist cable_menu[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, CABLE_PORT_0_V35},
                { 1, CABLE_PORT_0_232},
                { 1, CABLE_PORT_0_X21},
                { 1, CABLE_PORT_0_422},
                { 1, CABLE_PORT_1_232},
                { 1, CABLE_PORT_1_V35},
                { 1, CABLE_PORT_2_232},
                { 1, CABLE_PORT_2_422},
                { 1, CABLE_PORT_3_232},
                { 1, CABLE_SELECTION_MESSAGE},
                {       0, 0            }

	};
ASL_SCR_INFO asi_cable_menu[DIAG_NUM_ENTRIES(cable_menu)];

struct msglist ports_menu[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, P0_V35},
                { 1, P0_232},
                { 1, P0_X21},
                { 1, P0_422},
                { 1, P1_232},
                { 1, P1_V35},
                { 1, P2_232},
                { 1, P2_422},
                { 1, P3_232},
                { 1, INTERFACE_CABLE_MESSAGE},
                {       0, 0            }

	};
ASL_SCR_INFO asi_ports_menu[DIAG_NUM_ENTRIES(ports_menu)];

struct msglist individual_port_connector[]=
	{
                { 1, MPQP_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION_MPQP         	},
                { 1, NO_OPTION_MPQP		},
                { 1, INDIVIDUAL_PORT_CONNECTOR	},
                {       0, 0            }

	};

ASL_SCR_INFO 	asi_individual_port_connector[DIAG_NUM_ENTRIES(individual_port_connector)];

