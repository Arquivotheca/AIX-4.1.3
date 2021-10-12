/* @(#)19	1.3.1.2  src/bos/diag/da/fddi/dfddi.h, dafddi, bos410 5/6/93 15:01:14 */
/*
 *   COMPONENT_NAME: DAFDDI
 *
 *   FUNCTIONS: None (header file) 
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

#include "dfddi_msg.h"
#define		FRONT_ROYAL		1
#define		SCARBOROUGH		2
#define		FOXCROFT		3	
#define		NO_ERROR		0
#define		ERROR_FOUND		-1
#define		CATALOG_NAME		"dfddi.cat"
#define		FDDI_MENU_NUMBER_1	0x859001
#define		FDDI_MENU_NUMBER_2	0x859002
#define		FDDI_MENU_NUMBER_3	0x859003
#define		FDDI_MENU_NUMBER_4	0x859004
#define		FDDI_MENU_NUMBER_5	0x859005
#define		FDDI_MENU_NUMBER_6	0x859006
#define		FDDI_MENU_NUMBER_7	0x859007
#define		FDDI_MENU_NUMBER_8	0x859008
#define		FDDI_MENU_NUMBER_9	0x859009
#define		FDDI_MENU_NUMBER_10	0x859010
#define		YES			1
#define		NO			2
#define		NOT_CONFIG		-1
#define		QUIT			-1
#define		FDDI_INIT_OP		1
#define		FDDI_GET_OP		2


struct msglist wrap_yes_no[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY	},
		{ 1, FDDI_YES_OPTION			},
		{ 1, FDDI_NO_OPTION			},
		{ 1, FDDI_WRAP_PLUG_TITLE       	},
		0
	};

ASL_SCR_INFO asi_wrap_yes_no[DIAG_NUM_ENTRIES(wrap_yes_no)];

struct msglist two_wrap_yes_no[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY_WITH_EXTENDER	},
		{ 1, FDDI_YES_OPTION			},
		{ 1, FDDI_NO_OPTION			},
		{ 1, FDDI_TWO_WRAP_PLUG_TITLE       	},
		0
	};

ASL_SCR_INFO asi_two_wrap_yes_no[DIAG_NUM_ENTRIES(two_wrap_yes_no)];

struct msglist foxcroft_wrap_yes_no[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY	},
		{ 1, FDDI_YES_OPTION			},
		{ 1, FDDI_NO_OPTION			},
		{ 1, NEW_FOXCROFT_WRAP_PLUG_TITLE      },
		0
	};

ASL_SCR_INFO asi_foxcroft_wrap_yes_no[DIAG_NUM_ENTRIES(foxcroft_wrap_yes_no)];

struct msglist foxcroft_two_wrap_yes_no[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY_WITH_EXTENDER	},
		{ 1, FDDI_YES_OPTION			},
		{ 1, FDDI_NO_OPTION			},
		{ 1, NEW_FOXCROFT_TWO_WRAP_PLUG_TITLE      },
		0
	};

ASL_SCR_INFO asi_foxcroft_two_wrap_yes_no[DIAG_NUM_ENTRIES(foxcroft_two_wrap_yes_no)];

/* struct for putting wrap plug for the main adapter	*/
struct msglist wrap_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY 	},
		{ 1, FDDI_PLUG_WRAP           		},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_wrap_msg[DIAG_NUM_ENTRIES(wrap_msg)];

/* struct for putting wrap plug for the main adapter	*/
struct msglist foxcroft_wrap_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY 	},
		{ 1, NEW_FOXCROFT_PLUG_WRAP   		},
		{ 1, FDDI_FINISHED    			}, 0
	};

ASL_SCR_INFO asi_foxcroft_wrap_msg[DIAG_NUM_ENTRIES(foxcroft_wrap_msg)];

/* struct for putting wrap plug for the extender adapter	*/
struct msglist extender_wrap_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY 	},
		{ 1, FDDI_PLUG_WRAP_EXTENDER  		},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_extender_wrap_msg[DIAG_NUM_ENTRIES(extender_wrap_msg)];

/* struct for putting wrap plug for the extender adapter	*/
struct msglist foxcroft_extender_wrap_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY 	},
		{ 1, FDDI_FOXCROFT_PLUG_WRAP_EXTENDER  	},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_foxcroft_extender_wrap_msg[DIAG_NUM_ENTRIES(foxcroft_extender_wrap_msg)];

/* struct for putting wrap plug for the main adapter and extender adapter    */
struct msglist foxcroft_wrap_msg_all[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY_WITH_EXTENDER 	},
		{ 1, NEW_FOXCROFT_PLUG_WRAP_ALL	},
		{ 1, FDDI_FINISHED    			}, 0
	};

ASL_SCR_INFO asi_foxcroft_wrap_msg_all[DIAG_NUM_ENTRIES(foxcroft_wrap_msg_all)];

/* struct for putting wrap plug for the main adapter and extender adapter    */
struct msglist fddi_wrap_msg_all[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY_WITH_EXTENDER 	},
		{ 1, FDDI_PLUG_WRAP_ALL	},
		{ 1, FDDI_FINISHED    			}, 0
	};

ASL_SCR_INFO asi_fddi_wrap_msg_all[DIAG_NUM_ENTRIES(fddi_wrap_msg_all)];


/* struct for asking the user to unplug the wrap and plug the cables in */
/* for the fddi main adapter						*/

struct msglist unplug_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY	},
		{ 1, FDDI_UNPLUG_WRAP         		},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_unplug_msg[DIAG_NUM_ENTRIES(unplug_msg)];

struct msglist foxcroft_unplug_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY	},
		{ 1, FDDI_FOXCROFT_UNPLUG_WRAP  	},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_foxcroft_unplug_msg[DIAG_NUM_ENTRIES(foxcroft_unplug_msg)];

/* struct for asking the user to unplug the wrap and plug the cables in */
/* for the fddi extender adapter					*/

struct msglist extender_unplug_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY	},
		{ 1, FDDI_UNPLUG_WRAP_EXTENDER 		},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_extender_unplug_msg[DIAG_NUM_ENTRIES(extender_unplug_msg)];

/* struct for asking the user to unplug the wrap and plug the cables in */
/* for the fddi  main and extender adapter				*/

struct msglist all_unplug_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY_WITH_EXTENDER	},
		{ 1, FDDI_UNPLUG_WRAP_ALL      		},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_all_unplug_msg[DIAG_NUM_ENTRIES(all_unplug_msg)];

struct msglist foxcroft_all_unplug_msg[]=
	{
		{ 1, TESTING_ADVANCED_MODE_NO_STANDBY_WITH_EXTENDER	},
		{ 1, FOXCROFT_UNPLUG_WRAP_ALL      		},
		{ 1, FDDI_FINISHED    			},
		0
	};

ASL_SCR_INFO asi_foxcroft_all_unplug_msg[DIAG_NUM_ENTRIES(foxcroft_all_unplug_msg)];

/* Field Report Unit	section		*/

#define		FRU_101		0
#define		FRU_102		1
#define		FRU_103		2
#define		FRU_104		3
#define		FRU_106		4
#define		FRU_107		5
#define		FRU_108		6
#define		FRU_109		7
#define		FRU_110		8
#define		FRU_111		9
#define		FRU_112		10
#define		FRU_113		11
#define		FRU_114		12
#define		FRU_115		13
#define		FRU_116		14
#define		FRU_117		15
#define		FRU_118		16
#define		FRU_121		17
#define		FRU_122		18
#define		FRU_123		19
#define		FRU_124		20
#define		FRU_125		21
#define		FRU_126		22
#define		FRU_127		23
#define		FRU_128		24
#define		FRU_129		25
#define		FRU_130		26
#define		FRU_131		27
#define		FRU_132		28
#define		FRU_133		29
#define		FRU_134		30
#define		FRU_135		31
#define		FRU_150		32
#define		FRU_151		33
#define		FRU_120		34
#define		FRU_999		35
#define		FRU_150E	35
#define		FRU_134E	36
#define		FRU_127E	37
#define		FRU_133E	38
#define		FRU_135E	39
#define		FRU_130E	40

/* struct for FRU buckets		*/

struct fru_bucket	fddi_frus[] = {
	{ "", FRUB1, 0x859, 0x101, FDDI_MSG_101,
		{
			{90, "", "", 0,  DA_NAME, NONEXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x102, FDDI_MSG_102,
		{
			{90, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x103, FDDI_MSG_103,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x104, FDDI_MSG_104,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x106, FDDI_MSG_106,
		{
			{90, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x107, FDDI_MSG_107,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x108, FDDI_MSG_108,
		{
			{90, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x109, FDDI_MSG_109,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x110, FDDI_MSG_110,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x111, FDDI_MSG_111,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x112, FDDI_MSG_112,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x113, FDDI_MSG_113,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x114, FDDI_MSG_114,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x115, FDDI_MSG_115,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x116, FDDI_MSG_116,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x117, FDDI_MSG_117,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x118, FDDI_MSG_118,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x121, FDDI_MSG_121,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x122, FDDI_MSG_122,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x123, FDDI_MSG_123,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x124, FDDI_MSG_124,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x125, FDDI_MSG_125,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x126, FDDI_MSG_126,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x127, FDDI_MSG_127,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x128, FDDI_MSG_128,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x129, FDDI_MSG_129,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x130, FDDI_MSG_130,
		{
			{80, "", "", 0, CHILD_NAME, NONEXEMPT},
			{15, "", "", 0, DA_NAME, NONEXEMPT},
			{05, "", "", FDDI_CROSSOVER_CABLE, NOT_IN_DB, EXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x131, FDDI_MSG_131,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x132, FDDI_MSG_132,
		{
			{80, "", "", 0, CHILD_NAME, NONEXEMPT},
			{15, "", "", 0, DA_NAME, NONEXEMPT},
			{05, "", "", FDDI_CROSSOVER_CABLE, NOT_IN_DB, EXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x133, FDDI_MSG_133,
		{
			{100, "", "", 0, CHILD_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x134,FDDI_OPEN_FAIL, 
		{
			{70, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", 0, CHILD_NAME, NONEXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
			{10, "", "", FDDI_SOFTWARE_ERROR, NOT_IN_DB, EXEMPT},
		},
	},
	/* This is label 135, even though message is 132	*/
	{ "", FRUB1, 0x859, 0x132, FDDI_MSG_132,
		{
			{80, "", "", 0, CHILD_NAME, NONEXEMPT},
			{20, "", "", FDDI_CROSSOVER_CABLE, NOT_IN_DB, EXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x150, FDDI_CONFIG_FAIL,
		{
			{70, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", 0, CHILD_NAME, NONEXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
			{10, "", "", FDDI_SOFTWARE_ERROR, NOT_IN_DB, EXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x151, FDDI_OPEN_FAIL,
		{
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{20, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},
	{ "", FRUB1, 0x859, 0x120, FDDI_OPEN_FAIL,
		{
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{20, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	},
	/* this FRU is 150E 		index 35		*/
	{ "", FRUB1, 0x859, 0x150, FDDI_CONFIG_FAIL,
		{
			{70, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", FDDI_EXTENDER_CARD, NOT_IN_DB, EXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
			{10, "", "", FDDI_SOFTWARE_ERROR, NOT_IN_DB, EXEMPT},
		},
	},
	/* this FRU is 134E      	index 36		*/
	{ "", FRUB1, 0x859, 0x134, FDDI_OPEN_FAIL,
		{
			{70, "", "", 0, DA_NAME, NONEXEMPT},
			{10, "", "", FDDI_EXTENDER_CARD, NOT_IN_DB, EXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
			{10, "", "", FDDI_SOFTWARE_ERROR, NOT_IN_DB, EXEMPT},
		},
	},
	
	/* this fru is 127E index 37 when rc = 0x1b0250bc	*/
	/* the fru is 859-137				        */
	{ "", FRUB1, 0x859, 0x137, FDDI_MSG_127,
		{
			{40, "", "", 0, DA_NAME, NONEXEMPT},
			{40, "", "", FDDI_EXTENDER_CARD, NOT_IN_DB, EXEMPT},
			{20, "", "", FDDI_CROSSOVER_CABLE, NOT_IN_DB, EXEMPT},
		},
	},
	/* this fru is for 133E where data base does not show fddix	*/
	{ "", FRUB1, 0x859, 0x133, FDDI_MSG_133,
		{
			{100, "", "", FDDI_EXTENDER_CARD, NOT_IN_DB, EXEMPT},
		},
	},
	/* this fru is for 135E where data base does not show fddix	*/
	/* It is label 135, it is called as 132				*/
	{ "", FRUB1, 0x859, 0x132, FDDI_MSG_132,
		{
			{80, "", "", FDDI_EXTENDER_CARD, NOT_IN_DB, EXEMPT},
			{20, "", "", FDDI_CROSSOVER_CABLE, NOT_IN_DB, EXEMPT},
		},
	},
	/* this fru is for 130E where data base does not show fddix	*/
	{ "", FRUB1, 0x859, 0x130, FDDI_MSG_130,
		{
			{80, "", "", FDDI_EXTENDER_CARD, NOT_IN_DB, EXEMPT},
			{15, "", "", 0, DA_NAME, NONEXEMPT},
			{05, "", "", FDDI_CROSSOVER_CABLE, NOT_IN_DB, EXEMPT},
		},
	},
};

/* this FRU is for the old Front Royal card		*/
/* The system configuration consists of base card only	*/
struct fru_bucket	frub170 = 
{ "", FRUB1, 0x859, 0x170, FDDI_OPEN_FAIL,
	{
		{100, "", "", 0,  DA_NAME, NONEXEMPT},
	},
};

/* this FRU is for old Front Royal card				*/
/* The system configuration consists of base and extender card	*/

struct fru_bucket	frub180 = 
{ "", FRUB1, 0x859, 0x180, FDDI_OPEN_FAIL,
	{
		{100, "", "", 0,  DA_NAME, NONEXEMPT},
		{100, "", "", 0, CHILD_NAME, NONEXEMPT},
		{100, "", "",  FDDI_CROSSOVER_CABLE, NOT_IN_DB,
					NONEXEMPT},
	},
};

struct tu_fru_pair {
	short 	tu;	/* test unit	*/
	short	fru;	/* index to fddi_frus */
} group_tests_A[] = {
	{ 3,  FRU_103},
	{ 1,  FRU_101},
	{ 6,  FRU_106},
	{ 7,  FRU_107},
	{ 1,  FRU_101},
	{ 8,  FRU_108},
	{ 9,  FRU_109},
	{ 10, FRU_110},
	{ 11, FRU_111},
	{ 12, FRU_112},
	{ 13, FRU_113},
	{ 14, FRU_114},
	{ 15, FRU_115},
	{ 16, FRU_116},
	{ 2,  FRU_102},
	{ 20, FRU_120},
	{ 21, FRU_121},
	{ 20, FRU_120},
	{ 18, FRU_118},
	{ 3,  FRU_103},
	{ 22, FRU_122},
	{ 23, FRU_123},
	{ 24, FRU_124},
	{ 25, FRU_125},
	{ 26, FRU_126},
	{ 27, FRU_127},
	{ 28, FRU_128},
	{ 29, FRU_129},
	{-1,  -1},
},  /* group_tests_A 	*/

group_tests_B[] = {
	{1,  FRU_101},
	{ 20, FRU_120},
	{18, FRU_118},
	{2,  FRU_102},
	{3,  FRU_103},
	{31, FRU_131},
	{4,  FRU_104},
	{-1, -1},
};  /* group_tests_B	*/



/* return code for test units	*/
int	tu01[] = 
	{
		0x1020082, 0x1020092, 0x1020102, 0,
	};
   
int	tu02[] = 
	{
		0x2020082, 0x2020092, 0x2020102, 0,
	};

int	tu03[] = 
	{
		0x3025400, 0x3025401, 0,
	};
int	tu04[] = 
	{
		0x4025700, 0x4025701, 0,
	};
int	tu06[] = 
	{
		0x6020110, 0x6020111, 0x6020112, 0x6020113, 0x6020114, 
		0x6020115, 0x6020116, 0x6020117, 0x6020120, 0x6020121, 
		0x6020122, 0x6020123, 0x6020124, 0x6020125, 0x6020126, 
		0x6020127, 0x6020130, 0x6020131, 0x6020132, 0x6020133, 
		0x6020134, 0x6020135, 0x6020136, 0x6020137, 0x6020140, 
		0x6020141, 0x6020901, 0,
	};
int	tu07[] =
	{
		0x7020082, 0x7020092, 0x7020102, 0x7020300, 0x7020301, 
		0x7020302, 0x7026300, 0x7026301, 0,
	};

int	tu08[] =
	{
		0x8022001, 0x8022002, 0x8022003, 0x8026310, 0x8026311,
		0x8020310, 0x8020311, 0x8020312, 0x8020335, 0x8020336,
		0x8020337, 0x8020338, 0,
	};

int	tu09[] =
	{
		0x9022001, 0x9022002, 0x9022003, 0x9026310, 0x9026311,
		0x9020320, 0x9020321, 0x9020312, 0x9020322, 0x9020335,
		0x9020336, 0x9020337, 0x9020338, 0,
	};	
		
int	tu10[]=
	{
		0xA022001, 0XA022002, 0XA022003, 0XA026310, 0XA026311,
		0XA020330, 0XA020331, 0XA020312, 0XA020322, 0XA020332,
		0XA020335, 0XA020336, 0XA020337, 0XA020338, 0,
	};

int	tu11 [] =
	{
		0xb022001, 0xb022002, 0xb022003, 0xb026340, 0xb026341,
		0xb020340, 0xb020341, 0xb020342, 0,
	};

int	tu12[] =
	{
		0xc022001, 0xc022002, 0xc022003, 0xc026340, 0xc026341,
		0xc020350, 0xc020351, 0xc020342, 0xc020352, 0,
	};

int	tu13[] =
	{
		0xd022001, 0xd022002, 0xd022003, 0xd026340, 0xd026341,
		0xd020360, 0xd020361, 0xd020342, 0xd020352, 0xd020362, 0,
	 };

int	tu14[] =
	{
		0xe022001, 0xe022002, 0xe022003, 0xe026370, 0xe026371,
		0xe020370, 0xe020371, 0xe020372, 0,
	};

int	tu15[] =
	{
		0xf022001, 0xf022002, 0xf022003, 0xf026370, 0xf026371,
		0xf020380, 0xf020381, 0xf020372, 0xf020382, 0,
	};

int	tu16[] =
	{
		0x10022001, 0x10022002, 0x10022003, 0x10026370, 0x10026371,
		0x10020390, 0x10020391, 0x10020372, 0x10020382, 0x10020392, 0,
	};
		

int	tu17[] =
	{
		0x11020400, 0x11025800, 0x11025801, 0x11025700, 0x11025701,
		0x11020300, 0,
	};

int	tu18[] =
	{
		0x12020503, 0,
	};

int	tu21[] =
	{
		0x15020803, 0,
	};

int	tu22[] =
	{
		0x1602000A, 0x16020901, 0x16020300, 0,
	};

int	tu23[] =
	{
		0x1702100A,0x17020901, 0x17020300, 0,
	};

int	tu24[] =
	{
		0x1802200A, 0x18020901, 0x18020300, 0,
	};

int	tu25[] =
	{
		0x1902300A, 0x19020901, 0x19020300, 0,
	};

int	tu26[]=
	{
		0x1a02400A, 0x1a020901, 0x1a020300, 0,
	};

int	tu27[] =
	{
		0x1b02500A, 0x1b020901, 0x1b020300, 0x1b0250bc, 0,
	};

int	tu28[] =
	{
		0x1c02600A, 0x1c020901, 0x1c020300, 0,
	};

int	tu29[] =
	{
		0x1d02700A, 0x1d020901, 0x1d020300, 0,
	};

int	tu30[] =
	{
		0x1e020901, 0x1e020300, 0x1e02800A, 0x1e0280BC, 0x1E0280EA, 0,
	};

int	tu31[] =
	{
		0x1f020901, 0x1f020300, 0x1f029001, 0x1f02900A, 0
	};

int	tu32[] =
	{
		0x20020901, 0x20020300, 0x2002A0BC, 0x2002A0EA, 0,
	};
	
int	tu33[] =
	{
		0x21020a00, 0x21020a01, 0x21025900, 0x21025901, 0x21020300, 0,
	};

int	tu34[] =
	{
		0x22020950, 0x22020951, 0x22020952, 0x22020953, 0,
	};
int	tu35[] =
	{
		0x23020b00, 0x22020b01,0x23020b03, 0x23020b05, 0,
	};

int	*tu_rc_table[] = {
	tu01, tu02, tu03, tu04, tu04, tu06, tu07, tu08, tu09, tu10,
	tu11, tu12, tu13, tu14, tu15, tu16, tu17, tu18, tu18, tu18,
	tu21, tu22, tu23, tu24, tu25, tu26, tu27, tu28, tu29, tu30, 
	tu31, tu32, tu33, tu34, tu35
	}; 

/* @(#)19	1.3.1.3  src/bos/diag/da/fddi/dfddi.h, dafddi, bos411, 9428A410j 9/30/93 16:25:53 */
