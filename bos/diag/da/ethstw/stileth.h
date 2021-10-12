/* @(#)47	1.9  src/bos/diag/da/ethstw/stileth.h, daethstw, bos411, 9428A410j 10/28/93 13:35:38 */
/*
 *   COMPONENT_NAME: daethstw
 *
 *   FUNCTIONS: none (header file for Stilwel Ethenet Diagnostics Application)
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define		MENU_TEST_LOOP		0x887003
#define		MENU_TEST_ADVANCED	0x887002
#define		MENU_TEST_REGULAR	0x887001

#define		ETH_SOFTWARE_ERROR	0x999
#define		NO_ERROR		0
#define		ERROR			-1
#define		TESTING_LOOPMODE	3
#define		TESTING_ADVANCED_MODE	2
#define		YES			1
#define		NO			2
#define         ETHERNET_CUSTOMER       444
#define         ETHERNET_ADVANCED       555
#define         ETHERNET_LOOPMODE       666


#define		REPORT_FRU		-2
#define		NO_REPORT_FRU		-1

/* selection of menu_7[] */
#define		BASE_T			1
#define		BASE_2			2
#define		ETHERNET_UNKNOWN	3

#define		NOT_CONFIG		-1
#define		QUIT			-1

/* operation code of seth_davar_op () */
#define	SETH_INIT_OP	1
#define SETH_GET_OP	2
/*
*	FRU tables of Stilwel Ethernet Adapter Diag. 
*	Need to fill in rmsg, fmsg field later
*/
/*	indexes of seth_frus[] */
#define	FRU_101		0
#define FRU_102		1
#define FRU_103		2
#define FRU_104		3
#define FRU_105		4
#define FRU_106		5
#define FRU_107		6
#define FRU_108		7
#define FRU_109		8
#define FRU_110		9
#define FRU_111		10
#define FRU_112		11
#define FRU_113		12
#define FRU_114		13
#define FRU_115		14
#define FRU_116		15
#define FRU_117		16
#define FRU_118		17
#define FRU_120		18
#define FRU_121		19
#define FRU_122		20
#define FRU_123		21
#define FRU_124		22
#define FRU_125		23

struct fru_bucket seth_frus[] = {
	/* fail from tu001; POS register test */
  	{"", FRUB1, 0x887, 0x101, POS_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu002; I/O register test */
  	{"", FRUB1, 0x887, 0x102, IO_REG_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu003; LOCAL RAM test */
  	{"", FRUB1, 0x887, 0x103, RAM_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu004; VPD test */
  	{"", FRUB1, 0x887, 0x104, VPD_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu005; 82596 test */
  	{"", FRUB1, 0x887, 0x105, LAN_82596_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu006; Internal loopback test (DIX/BNC) */
  	{"", FRUB1, 0x887, 0x106, INTERNAL_LB_FAILED,
		{
			{90, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", THICK_THIN_RISER_CARD, NOT_IN_DB,NONEXEMPT},
		},
	},
	/* fail from tu008; External loopback test DIX */
  	{"", FRUB1, 0x887, 0x107, EXT_LB_FAILED,
		{
			{75, "", "",THICK_THIN_RISER_CARD,NOT_IN_DB, NONEXEMPT},
			{25, "", "", 0, DA_NAME,NONEXEMPT},
		},
	},
	/* fail from tu009; Ext. loopback test BNC */
  	{"", FRUB1, 0x887, 0x108, EXT_LB_BNC_FAILED,
		 {
			{80, "", "", THICK_THIN_RISER_CARD, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME,NONEXEMPT},
		},
	},
	/* fail from tu00A; Ext. Lp(DIX/BNC) Parity Test */
  	{"", FRUB1, 0x887, 0x109, EXT_LB_PARITY_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu00B; Ext. Lp(DIX/BNC) Fairness Test */
  	{"", FRUB1, 0x887, 0x110, EXT_LB_FAIRNESS_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu00C; Ext. Lp(DIX/BNC) Parity & Fairness */
  	{"", FRUB1, 0x887, 0x111, EXT_LB_PARITY_FAIRNESS_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu007; Ext. Lp TP */
  	{"", FRUB1, 0x887, 0x112, EXT_LB_TP_FAILED,
		 {
			{90, "", "", TWISTED_RISERCARD, NOT_IN_DB, NONEXEMPT},
			{10, "", "", 0, DA_NAME,NONEXEMPT},
		},
	},
	/* fail from tu00D; Ext. Lp(TP) Parity */
  	{"", FRUB1, 0x887, 0x113, EXT_LB_TP_PARITY_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu00E; Ext. Lp(TP) Fairness */
  	{"", FRUB1, 0x887, 0x114, EXT_LB_TP_FAIRNESS_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu00F; Ext. Lp(TP) Fairness & Parity */
  	{"", FRUB1, 0x887, 0x115, EXT_LB_TP_PARITY_FAIRNESS_FAILED,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from tu010; Ext. Lp(TP) to Connector */
  	{"", FRUB1, 0x887, 0x116, TWISTED_WRAP_FAILED,
		 {
			{80, "", "", TWISTED_RISERCARD, NOT_IN_DB, NONEXEMPT},
			{20, "", "", 0, DA_NAME,NONEXEMPT},
		},
	},
	/* fail from configure_device(); device config. failure */
  	{"", FRUB1, 0x887, 0x117, DEVICE_CONFIG_FAILED,
		 {
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{20, "", "", SOFTWARE_FAILED, NOT_IN_DB,NONEXEMPT},
		},
	},
	/* fail from tu011 or open(); device driver indicates a hardware
	 * problem */
  	{"", FRUB1, 0x887, 0x118, HARD_PROBLEM,
		 {
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{20, "", "", THICK_THIN_RISER_CARD, NOT_IN_DB,NONEXEMPT},
		},
	},
	/* fail from tu011;  device driver indicates a hardware problem */
  	{"", FRUB1, 0x887, 0x120, RISERCARD_DETERMINATION,
		{
			{100, "", "", 0, 
						DA_NAME, EXEMPT},
		},
	},
	/* fail from transceiver test; 10 Base 2 transceiver failure */
  	{"", FRUB1, 0x887, 0x121, XCEIVER_TWISTED_FAILED,
		{
			{100, "", "", XCEIVER_TWISTED_FAILED, 
						NOT_IN_DB, EXEMPT},
		},
	},
	/* fail from transceiver test; 10 Base T transceiver failure */
  	{"", FRUB1, 0x887, 0x122, XCEIVER_10BASE_2_FAILED,
		{
			{100, "", "", XCEIVER_10BASE_2_FAILED, 
						NOT_IN_DB, EXEMPT},
		},
	},
	/* fail from tu006; Internal Lp (TP) */
  	{"", FRUB1, 0x887, 0x123, INT_LB_TP_FAILED,
		 {
			{90, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", TWISTED_RISERCARD, NOT_IN_DB,NONEXEMPT},
		},
	},
	/* Error log indicates a hardware problem */
  	{"", FRUB1, 0x887, 0x124, ERROR_LOG_FAILED,
		{
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{20, "", "", 0, PARENT_NAME,NONEXEMPT},
		},
	},
  	{"", FRUB1, 0x887, 0x125, FUSE_TEST_FAILED,
		 {
			{100, "", "", ETHERNET_BAD_FUSE, NOT_IN_DB, NONEXEMPT},
		},
	},
};

/*
*	Menus to be used by Stilwel Ethernet Adapter Diag.
*/


/* menu 887004 --- Is there transceiver attached to the Ethernet Adapter? */
struct msglist menu_4[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, XCEIVER_EXIST     		},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu4[DIAG_NUM_ENTRIES(menu_4)];

/* menu 887005 --- Test transceiver ?*/
struct msglist menu_5[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, TEST_XCEIVER     		},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu5[DIAG_NUM_ENTRIES(menu_5)];

/* menu 887006 --- Select type of transceiver */
struct msglist menu_6[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, BASET			},
		{ 1, BASE2			},
		{ 1, UNKNOWN			},
		{ 1, WHICH_TYPE_XCEIVER  	},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu6[DIAG_NUM_ENTRIES(menu_6)];

/* menu 887007 --- BNC wrap plug requirement */
struct msglist menu_7[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, WRAP_PLUG_BNC         },
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu7[DIAG_NUM_ENTRIES(menu_7)];

/* menu 761007 --- DIX wrap plug requirement */
struct msglist menu_71[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, WRAP_PLUG_DIX         },
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu71[DIAG_NUM_ENTRIES(menu_71)];

/* menu 761007 --- Twisted pair wrap plug requirement */
struct msglist menu_72[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, YES_OPTION			},
		{ 1, NO_OPTION			},
		{ 1, WRAP_PLUG_TWISTED     },
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu72[DIAG_NUM_ENTRIES(menu_72)];

/* menu 887008 --- Unplug transceiver 10BASE-T */
struct msglist menu_8[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, STIL_UNPLUG_TRAN_T		},
		{ 1, FINISHED              	},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu8[DIAG_NUM_ENTRIES(menu_8)];

/* menu 887008 --- Unplug transceiver 10BASE-2 */
struct msglist menu_81[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_XCEIVER_10BASE_2	},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu81[DIAG_NUM_ENTRIES(menu_81)];

/* menu 887009 --- plugging BNC wrap plug */
struct msglist menu_9[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, STIL_ETH_PLUG_BNC		},
		{ 1, FINISHED              	},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu9[DIAG_NUM_ENTRIES(menu_9)];

/* menu 887009 --- plugging DIX wrap plug */
struct msglist menu_91[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_DIX			},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu91[DIAG_NUM_ENTRIES(menu_91)];

/* menu 887009 --- plugging twisted pair wrap plug */
struct msglist menu_92[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, STIL_PLUG_TWISTED		},
		{ 1, FINISHED              	},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu92[DIAG_NUM_ENTRIES(menu_92)];

/* menu 887010 --- unplug  wrap plug */
struct msglist menu_100[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_WRAP_GENERIC	},
		{	0, 0	}
	};
ASL_SCR_INFO asi_menu100[DIAG_NUM_ENTRIES(menu_100)];

struct msglist menu_101[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_T_XCEIVER_BACK},
		{	0, 0	}
	};
ASL_SCR_INFO asi_menu101[DIAG_NUM_ENTRIES(menu_101)];

struct msglist menu_102[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_DIX_AND_PUT_BASE2  	},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu102[DIAG_NUM_ENTRIES(menu_102)];

struct msglist menu_11[]=
	{
		{ 1, ADVANCED_NO_STANDBY	},
		{ 1, PLUG_DIX		},
		{	0, 0	}
	};

ASL_SCR_INFO asi_menu11[DIAG_NUM_ENTRIES(menu_11)];

/* Menu 887012  */
struct msglist menu_12[]=
        {
                { 1, ADVANCED_NO_STANDBY},
                { 1, YES_OPTION         },
                { 1, NO_OPTION          },
                { 1, ISOLATE_XCEIVER    },
                {       0, 0            }
        };
ASL_SCR_INFO asi_isolate_xceiver[DIAG_NUM_ENTRIES(menu_12)];

struct msglist menu_13[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_BASE2},
                {       0, 0                    }
        };
ASL_SCR_INFO asi_menu_13[DIAG_NUM_ENTRIES(menu_13)];

struct msglist menu_14[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_TWISTED},
                {       0, 0                    }
        };
ASL_SCR_INFO asi_menu_14[DIAG_NUM_ENTRIES(menu_14)];

struct msglist menu_15[]=
        {
                { 1, ADVANCED_NO_STANDBY        },
                { 1, UNPLUG_XCEIVER_AND_PLUG_DIX},
                {       0, 0                    }
        };
ASL_SCR_INFO asi_menu_15[DIAG_NUM_ENTRIES(menu_15)];

typedef struct {
	long menu_no; /* menu number */
	struct msglist *msg_list; /* message lists */
	ASL_SCR_INFO *menu_list; /* menu lists */
	long scrtype; /* screen type */
} SETH_MENU;

/* indexes of menutbl[] */
#define	MENU_EXIST_TRAN				0
#define MENU_TEST_TRAN				1
#define MENU_WHICH_TRAN				2
#define MENU_WP_BNC				3
#define MENU_WP_DIX				4
#define MENU_WP_TWISTED				5
#define MENU_UNPLUG_T				6
#define MENU_UNPLUG_2				7
#define MENU_PLUG_BNC				8
#define MENU_PLUG_DIX				9
#define MENU_PLUG_TWISTED			10
#define MENU_UNPLUG_WP				11
#define MENU_PLUG_T				12
#define MENU_PLUG_2				13
#define MENU_UP_TX_P_DIX			14
#define MENU_ISOLATE_XCEIVER			15
#define MENU_UNPLUG_DIX_AND_PUT_BASE2 		16
#define MENU_UNPLUG_DIX_AND_PUT_TWISTED 	17
#define MENU_UNPLUG_XCEIVER_AND_PLUG_DIX	18

/* the following menu always use diag_display() */
SETH_MENU menutbl[] = {
	{ 0x887004, menu_4, asi_menu4,ASL_DIAG_LIST_CANCEL_EXIT_SC },
	{ 0x887005, menu_5, asi_menu5, ASL_DIAG_LIST_CANCEL_EXIT_SC},
	{ 0x887006, menu_6, asi_menu6 ,ASL_DIAG_LIST_CANCEL_EXIT_SC},
	{ 0x887007, menu_7, asi_menu8 ,ASL_DIAG_LIST_CANCEL_EXIT_SC},
	{ 0x887007, menu_71, asi_menu71,ASL_DIAG_LIST_CANCEL_EXIT_SC},
	{ 0x887007, menu_72, asi_menu72,ASL_DIAG_LIST_CANCEL_EXIT_SC},
	{ 0x887008, menu_8, asi_menu8 ,ASL_DIAG_ENTER_SC},
	{ 0x887008, menu_81, asi_menu81 ,ASL_DIAG_ENTER_SC},
	{ 0x887009, menu_9, asi_menu9 ,ASL_DIAG_ENTER_SC},
	{ 0x887009, menu_91, asi_menu91 ,ASL_DIAG_ENTER_SC},
	{ 0x887009, menu_92, asi_menu92 ,ASL_DIAG_ENTER_SC},
	{ 0x887010, menu_100, asi_menu100, ASL_DIAG_ENTER_SC },
	{ 0x887010, menu_101, asi_menu101, ASL_DIAG_ENTER_SC },
	{ 0x887010, menu_102, asi_menu102, ASL_DIAG_ENTER_SC },
	{ 0x887011, menu_11, asi_menu11, ASL_DIAG_ENTER_SC },
	{ 0x887012, menu_12, asi_isolate_xceiver, 
		            ASL_DIAG_LIST_CANCEL_EXIT_SC},
	{ 0x887013, menu_13, asi_menu_13, ASL_DIAG_ENTER_SC },
	{ 0x887013, menu_14, asi_menu_14, ASL_DIAG_ENTER_SC },
	{ 0x887013, menu_15, asi_menu_15, ASL_DIAG_ENTER_SC },
};

/* return code of test units */
int tu01[] = {
POS0_RD_ERR,
POS1_RD_ERR,
POS2_RD_ERR,
POS4_RD_ERR,
POS5_RD_ERR,
POS6_RD_ERR,
POS2_WR_ERR,
POS4_WR_ERR,
POS5_WR_ERR,
POS6_WR_ERR,
POS0_CMP_ERR,
POS1_CMP_ERR,
POS2_CMP_ERR,
POS4_CMP_ERR,
POS5_CMP_ERR,
POS6_CMP_ERR,
	0
};

int tu02[] = {
IO0_RD_ERR,
IO1_RD_ERR,
IO2_RD_ERR,
IO3_RD_ERR,
IO4_RD_ERR,
IO5_RD_ERR,
IO6_RD_ERR,
IO0_WR_ERR,
IO1_WR_ERR,
IO2_WR_ERR,
IO3_WR_ERR,
IO4_WR_ERR,
IO5_WR_ERR,
IO7_WR_ERR,
IO0_CMP_ERR,
IO1_CMP_ERR,
IO2_CMP_ERR,
IO3_CMP_ERR,
IO4_CMP_ERR,
IO5_CMP_ERR,
POS2_RD_ERR,
POS2_WR_ERR,
	0
};

int tu03[] = {
MEM_RD_ERR,
MEM_WR_ERR,
MEM_CMP_ERR,
POS2_RD_ERR,
POS2_WR_ERR,
IO0_RD_ERR,
IO1_RD_ERR,
START_ERR,
START_TIME_ERR,
START_BSTAT_ERR,
GET_STAT_ERR,
HALT_ERR,
HALT_TIME_ERR,
HALT_BSTAT_ERR,
	0
};

int tu04[] = {
VPD_HDR_ERR,
VPD_CRC_ERR,
VPD_LEN_ERR,
NAF_MULT_ON,
NO_ETH_VPD,
SPOS3_RD_ERR,
SPOS6_WR_ERR,
SPOS7_WR_ERR,
	0
};

int tu05[] = {
POS2_RD_ERR,
POS2_WR_ERR,
ENT_SELFTEST_ERR,
SELFTEST_FAIL,
START_ERR,
START_ERR,
GET_STAT_ERR,
START_TIME_ERR,
START_BSTAT_ERR,
HALT_ERR,
HALT_TIME_ERR,
HALT_BSTAT_ERR,
	0
};

int tu06[] = {
/* 0x9.. from wrap() */
POS2_RD_ERR,
POS2_WR_ERR,
SPOS7_WR_ERR,
SPOS6_WR_ERR,
SPOS3_RD_ERR,
NO_ETH_VPD,
ENT_POS_ERR,
ENT_CFG_ERR,
BAD_PCK_S_ERR,
WRAP_RD_ERR,
WRAP_CMP_ERR,
WRAP_WR_ERR,
STAT_NOT_AVAIL,
GET_STAT_ERR,
TX_INCOMPLETE,
CIO_NOT_OK,
WRAP_NODATA,
UNDERRUN_ERR,
OVERRUN_ERR,
START_ERR,
START_TIME_ERR,
START_BSTAT_ERR,
HALT_ERR,
HALT_TIME_ERR,
HALT_BSTAT_ERR,
	0
};

int tu07[] = { /* RISERCARD_ERR + IO6_RD_ERR + tu006 */
/* 0x9.. from wrap() */
POS2_RD_ERR,
POS2_WR_ERR,
IO6_RD_ERR,
RISERCARD_ERR,
SPOS7_WR_ERR,
SPOS6_WR_ERR,
SPOS3_RD_ERR,
NO_ETH_VPD,
ENT_POS_ERR,
ENT_CFG_ERR,
BAD_PCK_S_ERR,
WRAP_RD_ERR,
WRAP_CMP_ERR,
WRAP_WR_ERR,
STAT_NOT_AVAIL,
GET_STAT_ERR,
TX_INCOMPLETE,
CIO_NOT_OK,
WRAP_NODATA,
UNDERRUN_ERR,
OVERRUN_ERR,
START_ERR,
START_TIME_ERR,
START_BSTAT_ERR,
HALT_ERR,
HALT_TIME_ERR,
HALT_BSTAT_ERR,
	0
};

int tu08[] = {/* tu007 + BADFUSE */
POS2_RD_ERR,
POS2_WR_ERR,
IO6_RD_ERR,
RISERCARD_ERR,
BADFUSE,
SPOS7_WR_ERR,
SPOS6_WR_ERR,
SPOS3_RD_ERR,
NO_ETH_VPD,
ENT_POS_ERR,
ENT_CFG_ERR,
BAD_PCK_S_ERR,
WRAP_RD_ERR,
WRAP_CMP_ERR,
WRAP_WR_ERR,
STAT_NOT_AVAIL,
GET_STAT_ERR,
TX_INCOMPLETE,
CIO_NOT_OK,
WRAP_NODATA,
UNDERRUN_ERR,
OVERRUN_ERR,
START_ERR,
START_TIME_ERR,
START_BSTAT_ERR,
HALT_ERR,
HALT_TIME_ERR,
HALT_BSTAT_ERR,
	0
};
/* tu008,9,D,E,F is the same as tu007 */
/* tu00A,B,C is the same as tu008 */
int tu10[] = {
SPOS7_WR_ERR,
SPOS6_WR_ERR,
SPOS3_RD_ERR,
NO_ETH_VPD,
ENT_CFG_ERR,
WRAP_WR_ERR,
STAT_NOT_AVAIL,
GET_STAT_ERR,
TX_INCOMPLETE,
NO_COLLISION,
POS2_RD_ERR,
POS2_WR_ERR,
IO6_RD_ERR,
RISERCARD_ERR,
START_ERR,
START_TIME_ERR,
START_BSTAT_ERR,
HALT_ERR,
	0
};

#define TWISTED_PAIR_RISER_CARD TWISTED_PAIR
#define THICK_RISER_CARD THICK
#define THIN_RISER_CARD THIN

int tu11[] = {
RISERCARD_ERR,
BADFUSE,
IO6_RD_ERR,
TWISTED_PAIR_RISER_CARD,
THICK_RISER_CARD,
THIN_RISER_CARD,
	0
};

int *tu_rctbl[] = {
	tu01, tu02, tu03, tu04, tu05, tu06, 
	tu07, tu08, tu07, tu08, tu08, tu08,
	tu07, tu07, tu07, tu10, tu11
};
