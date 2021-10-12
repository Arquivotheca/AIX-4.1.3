/* @(#)35	1.4.1.1  src/bos/diag/da/ethsal/ethsal.h, daethsal, bos411, 9428A410j 10/18/93 13:35:28 */
/*
 *   COMPONENT_NAME: DAETHSAL
 *
 *   FUNCTIONS: N/A
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define NEW_MSG		1

#define		NO_ERROR		0
#define		ETH_ERROR		-1
#define		NOT_GOOD		-1
#define		CATALOG_NAME	 	MF_DSALETH
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
        { 02, FRU_202 },
        { 03, FRU_203 },
        { 04, FRU_304 },
        { 05, FRU_305 },
        { 06, FRU_306 },
        { -1, -1 }
 };

struct fru_bucket ieth_frus[] = {
  	{"", FRUB1, 0x887, 0x101, POS_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x202,VPD_FAILED  ,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x203, IO_REG_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x304, LAN_82596_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x305, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x306, INTERNAL_LB_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x307, EXT_LB_GENERIC_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x121, XCEIVER_TWISTED_FAILED,
		{
			{100, "", "", XCEIVER_10BASE_T, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x209, CONVERTER_TEST_FAILED,
		{
			{55, "", "", 0, DA_NAME, EXEMPT},
			{45, "", "", ETHERNET_CONVERTER, NOT_IN_DB, NONEXEMPT},
		},
	},

  	{"", FRUB1, 0x887, 0x117, DEVICE_CONFIG_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x319, HARD_PROBLEM,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x124, ERROR_LOG_FAILED,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x401, CIRCUIT_BREAKER_TEST_FAILED,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x122, XCEIVER_10BASE_2_FAILED,
		{
			{100, "", "", XCEIVER_10BASE_2, NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x402, XCEIVER_10BASE_2_FAILED,
		{
			{80, "", "", XCEIVER_10BASE_2, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x403,XCEIVER_TWISTED_FAILED,
		{
			{80, "", "", XCEIVER_10BASE_T, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x404,CONVERTER_TEST_FAILED, 
		{
			{80, "", "", ETHERNET_CONVERTER, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME, EXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x400,FUSE_TEST_FAILED, 
		{
			{100, "", "",ETHERNET_BAD_FUSE , NOT_IN_DB, NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x405,RERUN_DIAGNOSTICS, 
		{
			{55, "", "", ETHERNET_NETWORK, NOT_IN_DB, NONEXEMPT},
			{45, "", "", 0, DA_NAME, EXEMPT},
		},
	}
};



/*
*/

/* Menu 887004	*/
struct msglist xceiver_exist[]=
        {
                { 1, ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, XCEIVER_EXIST},
                0
        };
ASL_SCR_INFO asi_xceiver_exist[DIAG_NUM_ENTRIES(xceiver_exist)];



/* Menu 887005 */

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

/* - Confirming if Test transceiver or convert is desired */
struct msglist test_converter[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, YES_OPTION                 },
                { 1, NO_OPTION                  },
                { 1, TEST_CONVERTER             },
                {       0, 0    }
        };

ASL_SCR_INFO asi_test_converter[DIAG_NUM_ENTRIES(test_converter)];



/* Menu 887006 */
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

/* Menu 887007 */
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

/* Menu 887008	*/
struct msglist have_wrap_plug_bnc[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, WRAP_PLUG_BNC	},
		{	0, 0		}
	};

ASL_SCR_INFO asi_have_wrap_plug_bnc [DIAG_NUM_ENTRIES(have_wrap_plug_bnc)];

/* Menu 887009	*/
struct msglist have_wrap_plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, WRAP_PLUG_DIX	},
		{	0, 0		}
	};

ASL_SCR_INFO asi_have_wrap_plug_dix [DIAG_NUM_ENTRIES(have_wrap_plug_dix)];

/* Menu 887010	*/
struct msglist have_wrap_plug_twisted[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, WRAP_PLUG_TWISTED	},
		{	0, 0		}
	};

ASL_SCR_INFO asi_have_wrap_plug_twisted[DIAG_NUM_ENTRIES(have_wrap_plug_twisted)];

/* Menu 887011	*/
struct msglist plug_xceiver_twisted_col_test[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, PLUG_XCEIVER_T	},
		{ 1, FINISHED		},
		{	0, 0		}
	};

ASL_SCR_INFO asi_plug_xceiver_twisted_col_test [DIAG_NUM_ENTRIES(plug_xceiver_twisted_col_test)];

/* Menu 887012	*/
struct msglist isolate_xceiver[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, ISOLATE_XCEIVER	},
		{	0, 0		}
	};
ASL_SCR_INFO asi_isolate_xceiver[DIAG_NUM_ENTRIES(isolate_xceiver)];

/* Menu 887013	*/

struct msglist isolate_converter[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
		{ 1, ISOLATE_CONVERTER	},
		{	0, 0		}
	};
ASL_SCR_INFO asi_isolate_converter[DIAG_NUM_ENTRIES(isolate_converter)];


/* Menu 887014	*/
struct msglist xceiver_t_2_phase[]=
	{
		{ 1, ADVANCED_NO_STANDBY},
		{ 1, XCEIVER_T_2_PHASE	},
		{	0, 0		}
	};
ASL_SCR_INFO asi_xceiver_t_2_phase[DIAG_NUM_ENTRIES(xceiver_t_2_phase)];

/* Menu 887016	*/
struct msglist plug_xceiver_10base_2[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_XCEIVER_10BASE_2	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_xceiver_10base_2[DIAG_NUM_ENTRIES(plug_xceiver_10base_2)];


/* Menu 887017	*/
struct msglist plug_converter[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_CONVERTER		},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_converter[DIAG_NUM_ENTRIES(plug_converter)];


/* Menu 887018	*/
struct msglist plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_DIX      		},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_dix[DIAG_NUM_ENTRIES(plug_dix)];

/* Menu 887019	*/
struct msglist unplug_wrap_generic[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_WRAP_GENERIC	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_wrap_generic[DIAG_NUM_ENTRIES(unplug_wrap_generic)];

/* Menu 887020	*/
struct msglist unplug_dix_and_put_base2[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_AND_PUT_BASE2	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_and_put_base2[DIAG_NUM_ENTRIES(unplug_dix_and_put_base2)];


/* Menu 887021	*/
struct msglist unplug_t_wrap_and_put_converter_back[]=
	{
		{ 1, ADVANCED_NO_STANDBY			},
		{ 1, UNPLUG_T_WRAP_AND_PUT_CONVERTER_BACK 	},
		{	0, 0					}
	};
ASL_SCR_INFO asi_unplug_t_wrap_and_put_converter_back[DIAG_NUM_ENTRIES(unplug_t_wrap_and_put_converter_back)];



/* Menu 887022	*/
struct msglist plug_t_xceiver_back[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_T_XCEIVER_BACK	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_t_xceiver_back[DIAG_NUM_ENTRIES(plug_t_xceiver_back)];

/* Menu 887024	*/
struct msglist unplug_xceiver_and_plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_XCEIVER_AND_PLUG_DIX},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_xceiver_and_plug_dix[DIAG_NUM_ENTRIES(unplug_xceiver_and_plug_dix)];


/* Menu 887025	*/
struct msglist unplug_converter_and_plug_dix[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_CONVERTER_AND_PLUG_DIX},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_converter_and_plug_dix[DIAG_NUM_ENTRIES(unplug_converter_and_plug_dix)];


/* Menu 887026	*/
struct msglist unplug_dix_wrap_and_put_xceiver_base2[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_BASE2},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_wrap_and_put_xceiver_base2[DIAG_NUM_ENTRIES(unplug_dix_wrap_and_put_xceiver_base2)];

/* Menu 887027	*/
struct msglist unplug_dix_wrap_and_put_xceiver_twisted[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_TWISTED},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_wrap_and_put_xceiver_twisted[DIAG_NUM_ENTRIES(unplug_dix_wrap_and_put_xceiver_twisted)];


/* Menu 887028	*/
struct msglist unplug_dix_wrap_and_put_converter[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_WRAP_AND_PUT_CONVERTER},
		{	0, 0			}
	};
ASL_SCR_INFO asi_unplug_dix_wrap_and_put_converter[DIAG_NUM_ENTRIES(unplug_dix_wrap_and_put_converter)];

/* Menu 887030	*/
struct msglist plug_network_cable_only[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_NETWORK_CABLE_ONLY	},
		{	0, 0			}
	};
ASL_SCR_INFO asi_plug_network_cable_only[DIAG_NUM_ENTRIES(plug_network_cable_only)];

/* menu 887015 */
struct msglist unplug_wrap_cable_for_isolation[]=
	{
		{ 1, ADVANCED_NO_STANDBY		},
		{ 1, UNPLUG_WRAP_CABLE_FOR_ISOLATION	},
		{	0, 0				}
	};
ASL_SCR_INFO asi_unplug_wrap_cable_for_isolation[DIAG_NUM_ENTRIES(unplug_wrap_cable_for_isolation)];

/* return code of test units */
int tu001[] = { 100,102, 102, 103, 0};


int tu002[] = { 200,202, 203, 204,205,206,0};

int tu003[] = { 300,301,302,303,304,0};

int tu004[] = { 400,401,0};

int tu005[] = { 500,501,502,503,504,505,506,507,508,509,510,511,512,0};

int tu006[] = { 500,501,502,503,504,505,506,507,508,509,510,511,512,0};

int tu007[] = { 500,501,502,503,504,505,506,507,508,509,510,511,512,0};


int *tu_rctbl[] = { tu001, tu002, tu003, tu004, tu005, tu006 };
