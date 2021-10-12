/* @(#)46       1.4  src/bos/diag/da/pcient/dpcient.h, dapcient, bos41J, 9523B_all 6/6/95 17:15:47 */
/*
 *   COMPONENT_NAME: dapcient
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		PUTVAR
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define NEW_MSG		1

#define		CAT_CLOSED		-99
#define		NO_ERROR		0
#define		ETH_ERROR		-1
#define		NOT_GOOD		-1
#define		YES			1
#define		NO			2

#define		QUIT			-1

#define		INITIALIZE_VARS		0	
#define		READ_VARS		1	

#define		FRU_NONE		-1

#define		PN_BNC_WP		"71F1168, 70F9626, 02G7433"
#define		PN_DIX_WP		"71F1167, 70F9625"
#define		PN_TP_WP		"00G2380"
#define		PN_10BASET_XCEIVER	"02G7429"
#define		PN_10BASE2_XCEIVER	"02G7435"

#define PUTVAR(A,T,V)   A = V; \
                        putdavar(da_input.dname,"A",T,&A)

enum	{ TU_ACT_NOP, TU_ACT_REPORT_FRU, TU_ACT_MENUGOAL, TU_ACT_DA_SETRC_ERROR };
enum	{ MENU_SELECT_OFFSET, MENU_SELECT_1, MENU_SELECT_2,
	  MENU_SELECT_3, MENU_SELECT_4 };

/* IMPORTANT: This order should match the selection menu for adapters */
enum	{ T2_ADAPTER, TP_ADAPTER, GENERIC_ADAPTER };

enum	{ WP_NONE, WP_BNC, WP_TP, WP_DIX };

/* IMPORTANT: this sequence should be the same as the fru_bucket sequence
 * when add a new frub, PLEASE put it in the correct order in this enum
 * and the corresponding position in msglist. Thank you. */
enum	{ FRU_101, FRU_102, FRU_104, FRU_105, FRU_106,
	  FRU_121, FRU_122, FRU_124, FRU_125, FRU_126,
	  FRU_141, FRU_142, FRU_144, FRU_145, 
	  FRU_160, FRU_161, 
	  FRU_203, FRU_204, FRU_205,
	  FRU_223,
	  FRU_243,
	  FRU_700,
	  FRU_720,
	  FRU_740 };

/* IMPORTANT: this enum SHOULD match the ORDER of tu_frus array */
enum	{ TU1_FRUS, TU2_FRUS, TU3_FRUS, TU4_FRUS, TU5_FRUS,
	  ADAPTER_FRUS_100, ELA_FRUS };

/* FRUS that required for each adapter_type */
short	tu_frus[][3] = {
	{ FRU_101, FRU_121, FRU_141},	/* TU1_FRUS */
	{ FRU_102, FRU_122, FRU_142},	/* TU2_FRUS */
	{ FRU_203, FRU_223, FRU_243},	/* TU3_FRUS */
	{ FRU_104, FRU_124, FRU_144},	/* TU4_FRUS */
	{ FRU_105, FRU_125, FRU_145},	/* TU5_FRUS */
	{ FRU_106, FRU_126, FRU_NONE},	/* 100% adapter */
	{ FRU_700, FRU_720, FRU_740},	/* ELA */
};

struct	tu_fru_group_pair
{
	short	tu_id;
	short	fru_group;
} internal_tus[] = {
			{ 01, TU1_FRUS },
			{ 02, TU2_FRUS },
			{ 03, TU3_FRUS },
			{ 04, TU4_FRUS },
			{ 05, TU5_FRUS },
			{ -1, -1 }
		};

/* transceiver frus */
enum	{ XCEIVER_FRUS_100, XCEIVER_FRUS_80 };
enum	{ XCEIVER_NONE, XCEIVER_TP, XCEIVER_T2 };
short	xceiver_frus[][3] =
	{
		{ FRU_NONE, FRU_160, FRU_161 },	/* 100% xceiver */
		{ FRU_NONE, FRU_204, FRU_205 }	/*  80% xceiver */
	};

/*-----------------------------------------------------------------------*/
struct fru_bucket eth_frus[] = {
  	{"", FRUB1, 0x742, 0x101, CONFIG_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x102, IO_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x104, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x105, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x106, EXT_LB_GENERIC_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
	/* ----------------- T2 Adapter SRNs ----------------------- */
  	{"", FRUB1, 0x742, 0x121, CONFIG_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x122, IO_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x124, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x125, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x126, EXT_LB_GENERIC_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
	/* ----------------- TP Adapter SRNs ----------------------- */
  	{"", FRUB1, 0x742, 0x141, CONFIG_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x142, IO_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x144, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x145, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
	/* ----------------- Generic Adapter SRNs ------------------- */
  	{"", FRUB1, 0x742, 0x160, XCEIVER_10BASE_T_FAILED,
		{
			{100, "", "", XCEIVER_10BASE_T, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x161, XCEIVER_10BASE_2_FAILED,
		{
			{100, "", "", XCEIVER_10BASE_2, NOT_IN_DB, NONEXEMPT},
		},
	},
	/* ----------------- T2 Adapter SRNs ----------------------- */
  	{"", FRUB1, 0x742, 0x203, DEVICE_CONFIG_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x204,XCEIVER_10BASE_T_FAILED,
		{
			{80, "", "", XCEIVER_10BASE_T, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x205, XCEIVER_10BASE_2_FAILED,
		{
			{80, "", "", XCEIVER_10BASE_2, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
	/* ----------------- TP Adapter SRNs ----------------------- */
  	{"", FRUB1, 0x742, 0x223, DEVICE_CONFIG_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
	/* ----------------- Generic Adapter SRNs ------------------- */
  	{"", FRUB1, 0x742, 0x243, DEVICE_CONFIG_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
	/* ----------------- ELA SRNs ------------------------------ */
  	{"", FRUB1, 0x742, 0x700, ELA_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x720, ELA_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x742, 0x740, ELA_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	}
};

/* ----------------------------------------------------------------------------*/
struct msglist what_type_adapter[]=			/* 742004 */
        {
                { 1, ADVANCED_NO_STANDBY },
                { 1, T2_OPTION },
                { 1, TP_OPTION },
                { 1, ADAPTER_UNKNOWN_OPTION },
                { 1, WHAT_TYPE_ADAPTER	},
		{	0, 0		}
        };
ASL_SCR_INFO asi_what_type_adapter[DIAG_NUM_ENTRIES(what_type_adapter)];

struct msglist t2_what_type_connector[]=		/* 742005 */
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, RJ_45_OPTION		},
                { 1, BNC_OPTION          	},
                { 1, T2_WHAT_TYPE_CONNECTOR	},
		{	0, 0			}
        };
ASL_SCR_INFO asi_t2_what_type_connector[DIAG_NUM_ENTRIES(t2_what_type_connector)];

struct msglist tp_what_type_connector[]=		/* 742006 */
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, RJ_45_OPTION		},
                { 1, DIX_OPTION          	},
                { 1, TP_WHAT_TYPE_CONNECTOR	},
		{	0, 0			}
        };
ASL_SCR_INFO asi_tp_what_type_connector[DIAG_NUM_ENTRIES(tp_what_type_connector)];

/* -------------------------------------- */
struct msglist have_wrap_plug_bnc[]=			/* 742010 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, HAVE_WRAP_PLUG_BNC		},
		{	0, 0			}
	};
ASL_SCR_INFO asi_have_wrap_plug_bnc [DIAG_NUM_ENTRIES(have_wrap_plug_bnc)];

struct msglist plug_bnc_connector[]=			/* 742011 */
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, PLUG_BNC_CONNECTOR 	},
                { 1, FINISHED                   },
                {       0, 0    		}
        };
ASL_SCR_INFO asi_plug_bnc_connector[DIAG_NUM_ENTRIES(plug_bnc_connector)];

struct msglist unplug_wrap_generic[]=			/* 742019 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_WRAP_GENERIC	},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_wrap_generic[DIAG_NUM_ENTRIES(unplug_wrap_generic)];

/* -------------------------------------- */
struct msglist have_wrap_plug_dix[]=			/* 742020 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, HAVE_WRAP_PLUG_DIX		},
		{	0, 0			}
	};
ASL_SCR_INFO asi_have_wrap_plug_dix [DIAG_NUM_ENTRIES(have_wrap_plug_dix)];

struct msglist plug_dix_connector[]=			/* 742021 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_DIX_CONNECTOR 	},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_dix_connector[DIAG_NUM_ENTRIES(plug_dix_connector)];

/* -------------------------------------- */
struct msglist have_wrap_plug_twisted[]=		/* 742030 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, HAVE_WRAP_PLUG_TWISTED	},
		{	0, 0			}
	};

ASL_SCR_INFO asi_have_wrap_plug_twisted[DIAG_NUM_ENTRIES(have_wrap_plug_twisted)];

struct msglist plug_tp_connector[]=			/* 742031 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_TP_CONNECTOR      	},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_tp_connector[DIAG_NUM_ENTRIES(plug_tp_connector)];

/* -------------------------------------- */
struct msglist xceiver_exist[]=				/* 742040 */
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, XCEIVER_EXIST		},
		{	0, 0			}
        };
ASL_SCR_INFO asi_xceiver_exist[DIAG_NUM_ENTRIES(xceiver_exist)];

/* - Confirming if Test transceiver is desired */
struct msglist test_xceiver[]=				/* 742041 */
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, YES_OPTION                 },
                { 1, NO_OPTION                  },
                { 1, TEST_XCEIVER               },
                {       0, 0    	}
        };

ASL_SCR_INFO asi_test_xceiver[DIAG_NUM_ENTRIES(test_xceiver)];

struct msglist what_type_xceiver[]=			/* 742042 */
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, BASET            		},
                { 1, BASE2            		},
                { 1, XCEIVER_UNKNOWN          	},
                { 1, WHAT_TYPE_XCEIVER		},
		{	0, 0			}
        };
ASL_SCR_INFO asi_what_type_xceiver[DIAG_NUM_ENTRIES(what_type_xceiver)];

/* -------------------------------------- */
struct msglist plug_xceiver_twisted[]=		/* 742050 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_XCEIVER_T		},
		{ 1, FINISHED			},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_xceiver_twisted [DIAG_NUM_ENTRIES(plug_xceiver_twisted)];

struct msglist plug_t_xceiver_back[]=			/* 742051 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_T_XCEIVER_BACK	},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_t_xceiver_back[DIAG_NUM_ENTRIES(plug_t_xceiver_back)];

struct msglist isolate_xceiver[]=			/* 742052 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, ISOLATE_XCEIVER		},
		{	0, 0			}
	};
ASL_SCR_INFO asi_isolate_xceiver[DIAG_NUM_ENTRIES(isolate_xceiver)];

struct msglist unplug_xceiver_and_plug_dix[]=		/* 742053 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_XCEIVER_AND_PLUG_DIX},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_xceiver_and_plug_dix[DIAG_NUM_ENTRIES(unplug_xceiver_and_plug_dix)];

struct msglist unplug_dix_and_put_tp_xceiver_back[]=	/* 742054 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_AND_PUT_TP_XCEIVER_BACK	},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_and_put_tp_xceiver_back[DIAG_NUM_ENTRIES(unplug_dix_and_put_tp_xceiver_back)];

/* -------------------------------------- */
struct msglist plug_xceiver_10base_2[]=			/* 742060 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_XCEIVER_10BASE_2	},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_xceiver_10base_2[DIAG_NUM_ENTRIES(plug_xceiver_10base_2)];

struct msglist plug_generic_xceiver_back[]=		/* 742061 */
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_GENERIC_XCEIVER_BACK	},
                { 1, FINISHED                   },
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_generic_xceiver_back[DIAG_NUM_ENTRIES(plug_generic_xceiver_back)];

struct msglist unplug_dix_and_put_generic_xceiver_back[]=	/* 742064 */
	{
		{ 1, ADVANCED_NO_STANDBY			},
		{ 1, UNPLUG_DIX_AND_PUT_GENERIC_XCEIVER_BACK	},
                { 1, FINISHED                   },
		{	0, 0					}
	};
ASL_SCR_INFO asi_unplug_dix_and_put_generic_xceiver_back[DIAG_NUM_ENTRIES(unplug_dix_and_put_generic_xceiver_back)];
