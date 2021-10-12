/* @(#)44	1.7  src/bos/diag/da/eth/eth.h, daeth, bos411, 9428A410j 9/30/93 16:06:21 */
/*
 *   COMPONENT_NAME: DAETH
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		PUTVAR
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define NEW_MSG		1

#define		NO_ERROR		0
#define		ETH_ERROR		-1
#define		NOT_GOOD		-1
#define		YES			1
#define		NO			2

#define		REPORT_FRU		-2
#define		NO_REPORT_FRU		-1

#define		NOT_CONFIG		-1
#define		QUIT			-1

#define		INITIALIZE_VARS		0	
#define		READ_VARS		1	

#define		ETHERNET_SAL    	1
#define		ETHERNET_RB  		2

#define		ETHERNET_CUSTOMER	444
#define		ETHERNET_ADVANCED	555
#define		ETHERNET_LOOPMODE	666

#define	FRU_101		0
#define	FRU_202		1
#define	FRU_203		2
#define	FRU_304		3
#define	FRU_305		4
#define	FRU_306		5
#define	FRU_307		6
#define	FRU_121		7
#define	FRU_209		8
#define	FRU_117		9
#define	FRU_319		10
#define	FRU_124		11
#define	FRU_401		12
#define	FRU_122		13
#define	FRU_402		14
#define	FRU_403		15
#define	FRU_404		16
#define	FRU_400		17
#define	FRU_405		18

#define PUTVAR(A,T,V)   A = V; \
                        putdavar(da_input.dname,"A",T,&A)


struct tu_fru_pair 
{
        short   tu;             /* test unit */
        short   fru;            /* index  into fru structure ieth_frus */
} tus_test[] = {
        { 01, FRU_101 },
        { 02, FRU_202 },
        { 03, FRU_203 },
        { 04, FRU_304 },
        { 05, FRU_305 },
        { 12, FRU_121 },
        { 06, FRU_101 },
        { -1, -1 }
 };

struct fru_bucket eth_frus[] = {
  	{"", FRUB1, 0x852, 0x101, POS_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x202,VPD_FAILED  ,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x203, IO_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x304, LAN_82596_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x305, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x306, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x307, EXT_LB_GENERIC_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x121, XCEIVER_TWISTED_FAILED,
		{
			{100, "", "", XCEIVER_10BASE_T, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x209, CONVERTER_TEST_FAILED,
		{
			{55, "", "", 0, DA_NAME, EXEMPT},
			{45, "", "", ETHERNET_CONVERTER, NOT_IN_DB, NONEXEMPT},
		},
	},

  	{"", FRUB1, 0x852, 0x117, DEVICE_CONFIG_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x319, HARD_PROBLEM,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x124, ERROR_LOG_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x401, CIRCUIT_BREAKER_TEST_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x122, XCEIVER_10BASE_2_FAILED,
		{
			{100, "", "", XCEIVER_10BASE_2, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x402, XCEIVER_10BASE_2_FAILED,
		{
			{80, "", "", XCEIVER_10BASE_2, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x403,XCEIVER_TWISTED_FAILED,
		{
			{80, "", "", XCEIVER_10BASE_T, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x404,CONVERTER_TEST_FAILED, 
		{
			{80, "", "", ETHERNET_CONVERTER, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x400,FUSE_TEST_FAILED, 
		{
			{100, "", "",ETHERNET_BAD_FUSE , NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x852, 0x405,RERUN_DIAGNOSTICS, 
		{
			{55, "", "", ETHERNET_NETWORK, NOT_IN_DB, NONEXEMPT},
			{45, "", "", 0, DA_NAME, EXEMPT},
		},
	}
};



/*
*/
struct msglist which_type_connector[]=
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, DIX_OPTION			},
                { 1, BNC_OPTION          	},
                { 1, WHICH_TYPE_CONNECTOR		},
                0
        };
ASL_SCR_INFO asi_which_type_connector[DIAG_NUM_ENTRIES(which_type_connector)];

/* Menu 852004	*/
struct msglist xceiver_exist[]=
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, XCEIVER_EXIST},
                0
        };
ASL_SCR_INFO asi_xceiver_exist[DIAG_NUM_ENTRIES(xceiver_exist)];



/* Menu 852005 */

struct msglist converter_exist[]=
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, CONVERTER_EXIST		},
                0
        };
ASL_SCR_INFO asi_converter_exist[DIAG_NUM_ENTRIES(converter_exist)];

/* - Confirming if Test transceiver is desired */
struct msglist test_xceiver[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, YES_OPTION                 },
                { 1, NO_OPTION                  },
                { 1, TEST_XCEIVER               },
                {       0, 0    }
        };

ASL_SCR_INFO asi_test_xceiver[DIAG_NUM_ENTRIES(test_xceiver)];


/* Menu 852006 */
struct msglist what_type_xceiver[]=
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, BASET            		},
                { 1, BASE2            		},
                { 1, UNKNOWN          		},
                { 1, WHICH_TYPE_XCEIVER		},
                0
        };
ASL_SCR_INFO asi_what_type_xceiver[DIAG_NUM_ENTRIES(what_type_xceiver)];

/* Menu 852007 */
struct msglist what_type_converter[]=
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, BASET            		},
                { 1, BASE2            		},
                { 1, RJ_45            		},
                { 1, UNKNOWN          		},
                { 1, WHICH_TYPE_CONVERTER	},
                0
        };
ASL_SCR_INFO asi_what_type_converter[DIAG_NUM_ENTRIES(what_type_converter)];

/* Menu 852008	*/
struct msglist have_wrap_plug_bnc[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, WRAP_PLUG_BNC	},
		{	0, 0		}
	};

ASL_SCR_INFO asi_have_wrap_plug_bnc [DIAG_NUM_ENTRIES(have_wrap_plug_bnc)];

/* Menu 852009	*/
struct msglist have_wrap_plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, WRAP_PLUG_DIX	},
		{	0, 0		}
	};

ASL_SCR_INFO asi_have_wrap_plug_dix [DIAG_NUM_ENTRIES(have_wrap_plug_dix)];

/* Menu 852010	*/
struct msglist have_wrap_plug_twisted[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, WRAP_PLUG_TWISTED	},
		{	0, 0		}
	};

ASL_SCR_INFO asi_have_wrap_plug_twisted[DIAG_NUM_ENTRIES(have_wrap_plug_twisted)];

/* Menu 852011	*/
struct msglist plug_xceiver_twisted_col_test[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, PLUG_XCEIVER_T	},
		{ 1, FINISHED		},
		{	0, 0		}
	};

/* Plug transceiver 10BASE-T */
struct msglist plug_xceiver_twisted[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, STIL_UNPLUG_TRAN_T         },
                { 1, FINISHED                   },
                {       0, 0    }
        };

ASL_SCR_INFO asi_plug_xceiver_twisted[DIAG_NUM_ENTRIES(plug_xceiver_twisted)];

ASL_SCR_INFO asi_plug_xceiver_twisted_col_test [DIAG_NUM_ENTRIES(plug_xceiver_twisted_col_test)];

/* Menu 852012	*/
struct msglist isolate_xceiver[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, ISOLATE_XCEIVER	},
		{	0, 0		}
	};
ASL_SCR_INFO asi_isolate_xceiver[DIAG_NUM_ENTRIES(isolate_xceiver)];

/* Menu 852013	*/

struct msglist isolate_converter[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, ISOLATE_CONVERTER	},
		{	0, 0		}
	};
ASL_SCR_INFO asi_isolate_converter[DIAG_NUM_ENTRIES(isolate_converter)];


/* Menu 852014	*/
struct msglist xceiver_t_2_phase[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, XCEIVER_T_2_PHASE	},
		{	0, 0		}
	};
ASL_SCR_INFO asi_xceiver_t_2_phase[DIAG_NUM_ENTRIES(xceiver_t_2_phase)];

/* Menu 852016	*/
struct msglist plug_xceiver_10base_2[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_XCEIVER_10BASE_2	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_xceiver_10base_2[DIAG_NUM_ENTRIES(plug_xceiver_10base_2)];


/* Menu 852017	*/
struct msglist plug_converter[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_CONVERTER		},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_converter[DIAG_NUM_ENTRIES(plug_converter)];


/* Menu 852018	*/
struct msglist plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_DIX      		},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_dix[DIAG_NUM_ENTRIES(plug_dix)];

/* Menu 852018	*/
struct msglist plug_bnc_connector[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, STIL_ETH_PLUG_BNC          },
                { 1, FINISHED                   },
                {       0, 0    }
        };
ASL_SCR_INFO asi_plug_bnc_connector[DIAG_NUM_ENTRIES(plug_bnc_connector)];

/* Menu 852019	*/
struct msglist unplug_wrap_generic[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_WRAP_GENERIC	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_wrap_generic[DIAG_NUM_ENTRIES(unplug_wrap_generic)];

/* Menu 852020	*/
struct msglist unplug_dix_and_put_base2[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_AND_PUT_BASE2	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_and_put_base2[DIAG_NUM_ENTRIES(unplug_dix_and_put_base2)];


/* Menu 852021	*/
struct msglist unplug_t_wrap_and_put_converter_back[]=
	{
		{ 1, ADVANCED_NO_STANDBY			},
		{ 1, UNPLUG_T_WRAP_AND_PUT_CONVERTER_BACK 	},
		{	0, 0					}
	};
ASL_SCR_INFO asi_unplug_t_wrap_and_put_converter_back[DIAG_NUM_ENTRIES(unplug_t_wrap_and_put_converter_back)];



/* Menu 852022	*/
struct msglist plug_t_xceiver_back[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_T_XCEIVER_BACK	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_t_xceiver_back[DIAG_NUM_ENTRIES(plug_t_xceiver_back)];

/* Menu 852024	*/
struct msglist unplug_xceiver_and_plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_XCEIVER_AND_PLUG_DIX},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_xceiver_and_plug_dix[DIAG_NUM_ENTRIES(unplug_xceiver_and_plug_dix)];


/* Menu 852025	*/
struct msglist unplug_converter_and_plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_CONVERTER_AND_PLUG_DIX},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_converter_and_plug_dix[DIAG_NUM_ENTRIES(unplug_converter_and_plug_dix)];


/* Menu 852026	*/
struct msglist unplug_dix_wrap_and_put_xceiver_base2[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_BASE2},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_wrap_and_put_xceiver_base2[DIAG_NUM_ENTRIES(unplug_dix_wrap_and_put_xceiver_base2)];

/* Menu 852027	*/
struct msglist unplug_dix_wrap_and_put_xceiver_twisted[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_TWISTED},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_wrap_and_put_xceiver_twisted[DIAG_NUM_ENTRIES(unplug_dix_wrap_and_put_xceiver_twisted)];


/* Menu 852028	*/
struct msglist unplug_dix_wrap_and_put_converter[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_WRAP_AND_PUT_CONVERTER},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_wrap_and_put_converter[DIAG_NUM_ENTRIES(unplug_dix_wrap_and_put_converter)];

/* Menu 852030	*/
struct msglist plug_network_cable_only[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_NETWORK_CABLE_ONLY	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_network_cable_only[DIAG_NUM_ENTRIES(plug_network_cable_only)];

/* menu 852015 */
struct msglist unplug_wrap_cable_for_isolation[]=
	{
		{ 1, ADVANCED_NO_STANDBY		},
		{ 1, UNPLUG_WRAP_CABLE_FOR_ISOLATION	},
		{	0, 0				}
	};
ASL_SCR_INFO asi_unplug_wrap_cable_for_isolation[DIAG_NUM_ENTRIES(unplug_wrap_cable_for_isolation)];

