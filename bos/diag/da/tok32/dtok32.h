/* @(#)19	1.2  src/bos/diag/da/tok32/dtok32.h, dtok32, bos411, 9438C411a 9/22/94 13:14:35 */
/*
 *   COMPONENT_NAME: DTOK32
 *
 *   FUNCTIONS:
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

#ifndef DA_TOK32
#define DA_TOK32


#include "dtok32_msg.h"
#define		GOOD		0
#define		NOT_GOOD	-1
#define		BAD 		-1
#define		QUIT		-1
#define         YES 		1
#define         NO		2
#define		TYPE_3		2
#define 	NOT_TYPE_3	1


/*
*	FRU tables of Wildwood adapter 
*	Need to fill in rmsg, fmsg field later
*/

/*	indexes of tokenring_frus[] */

#define	FRU_101		0
#define	FRU_102		1
#define FRU_103         2
#define FRU_104         3
#define FRU_105         4
#define FRU_106         5
#define FRU_107         6
#define FRU_200         7
#define FRU_300         8
#define FRU_500         9

struct tu_fru_pair 
{
        short   tu;             /* test unit */
        short   fru;            /* index into tokenring_frus[] */
} tus_test[] = {
        { 1, FRU_101 },
        { 2, FRU_102 },
        { 3, FRU_103 },
        { -1, -1 }
 };

struct cable_tu_fru_pair 
{
        short   tu;             /* test unit */
        short   fru;            /* index into tokenring_frus[] */
} cable_tus_test[] = {
        { 4, FRU_104 },
        { 5, FRU_105 },
        { 6, FRU_106 },
        { -1, -1 }
 };

struct fru_bucket tokenring_frus[] = {
	/* fail from tu001; P.O.S register test	*/ 
  	{"", FRUB1, 0x85C, 0x101, TOK_101,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},

		},
	},
	/* fail from tu002; I/O Registers test	*/ 
  	{"", FRUB1, 0x85C, 0x102, TOK_102,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
		},
	},
	/* fail from tu03, Adapter self-test failure */
  	{"", FRUB1, 0x85C, 0x103, TOK_103,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
		},
	},
	/* fail from tu04, cable test failure */
  	{"", FRUB1, 0x85C, 0x104, TOK_104,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
		},
	},
	/* fail from tu005 Token-Ring Cable Wrap Failure */
  	{"", FRUB1, 0x85C, 0x105, TOK_105,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
		},
	},
	/* fail from tu006 Token-Ring Cable Wrap Failure */
  	{"", FRUB1, 0x85C, 0x106, TOK_105,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
 			{10, " ", "",  CABLE_MSG, NOT_IN_DB, EXEMPT},
		},
	},
	/* fail from tu007, Network Initialization Test Failure */ 
  	{"", FRUB1, 0x85C, 0x107, TOK_107,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
 			{10, " ", "",  CABLE_MSG, NOT_IN_DB, EXEMPT},
		},
	},
	/* Error log analysis indicates a failure	*/ 
  	{"", FRUB1, 0x85C, 0x200, TOK_200,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
		},
	},

	/* Device driver indicates a hardware failure */
  	{"", FRUB1, 0x85C, 0x300, TOK_300,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, NONEXEMPT},
		},
	},
	/* fail from configurration	*/ 
  	{"", FRUB1, 0x85C, 0x500, TOK_500,
		{
                       {80, "", "", 0, DA_NAME, NONEXEMPT},
                       {10, "", "", 0, PARENT_NAME, NONEXEMPT},
                       {10, " ", "", TOK_SOFT_ERROR ,
                                        NOT_IN_DB, EXEMPT},

		},
	}
};



/*
*	Menus to be used by Wildwood  Adapter  
*/




struct msglist have_wrap_plug[]=
	{
		{ 1, TESTING_ADVANCED_NO_STANDBY},
		{ 1, YES_OPTION		},
		{ 1, NO_OPTION		},
/*
		{ 1, HAVE_WRAP_PLUG	},
*/
		{	0, 0		}
	};

ASL_SCR_INFO asi_have_wrap_plug [DIAG_NUM_ENTRIES(have_wrap_plug)];

/* menu xxxxxx --- plugging wrap plug */
struct msglist put_wrap_plug[]=
	{
		{ 1, TESTING_ADVANCED_NO_STANDBY},
/*
		{ 1, PLUG_WRAP		},
*/
                { 1, FINISHED		},
                0
	};

ASL_SCR_INFO asi_put_wrap_plug [DIAG_NUM_ENTRIES(put_wrap_plug)];

/* menu XXXXXX --- unplug  wrap plug */
struct msglist unplug_wrap[]=
	{
		{ 1, TESTING_ADVANCED_NO_STANDBY	},
/*
		{ 1, UNPLUG_WRAP		},
*/
                { 1, FINISHED			},
                0
	};
ASL_SCR_INFO asi_unplug_wrap[DIAG_NUM_ENTRIES(unplug_wrap)];
/* This message will ask the user which type of cable is tested */
struct msglist which_type_of_cable[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, NONTELE_CABLE			},
                { 1, TELE_CABLE				},
                { 1, WIRING_TYPE			},
                0
        };
ASL_SCR_INFO asi_which_type_of_cable[DIAG_NUM_ENTRIES(which_type_of_cable)];

/* this message will ask the user if he wants to test cable     */

struct msglist testing_cable_yes_no[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, CABLE_REQUIRE		},
                0
        };
ASL_SCR_INFO asi_testing_cable_yes_no[DIAG_NUM_ENTRIES(testing_cable_yes_no)];

/* this message asks the user if the Token-Ring network connected	*/
struct msglist network_connected[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, NETWORK_CONNECTED},
                0
        };
ASL_SCR_INFO asi_network_connected[DIAG_NUM_ENTRIES(network_connected)];

/* this message will ask the user to plug the cable to be tested in */

struct msglist plug_cable_type_3[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, PLUG_CABLE_TYPE_3		},
                { 1, FINISHED			},
                0
        };
ASL_SCR_INFO asi_plug_cable_type_3[DIAG_NUM_ENTRIES(plug_cable_type_3)];

/* Ask the user to plug the regular Token-Ring cable	*/
struct msglist plug_cable[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, PLUG_CABLE				},
                { 1, FINISHED				},
                0
        };
ASL_SCR_INFO asi_plug_cable[DIAG_NUM_ENTRIES(plug_cable)];

/* Ask the user unplug the cable from the network but leave the cable 	*/
/* hooked to the adapter						*/
struct msglist unplug_cable[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, UNPLUG_CABLE			},
                { 1, FINISHED				},
                0
        };
ASL_SCR_INFO asi_unplug_cable[DIAG_NUM_ENTRIES(unplug_cable)];



struct msglist put_cable_back[]=
	{
		{ 1, TESTING_ADVANCED_NO_STANDBY	},
		{ 1, PUT_CABLE_BACK		},
                { 1, FINISHED			},
                0
	};
ASL_SCR_INFO asi_put_cable_back[DIAG_NUM_ENTRIES(put_cable_back)];


/* return code of test units */

int tu001[] = { 0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0 };

int tu002[] = { 0x2000, 0x2100, 0x2200, 0x2300, 0x2400, 0x2500, 0x2600,
		0x2700, 0x2800, 0x2900, 
		0x3000, 0x3100, 0x3200, 0x3300, 0x3400, 0x3500, 0x3600, 
		0x3700, 0x3800, 0x3900, 
		0x4000, 0x4100, 0x4200, 0x4300, 0x4400, 0x4500, 0x4600, 
		0x4700, 0x4800, 0x4900, 
		0x5000, 0x5100, 0x5200, 0x5300, 0x5400, 0x5500, 0x5600, 
		0x5700, 0x5800, 0x5900, 
		0x6000, 0x6100, 0x6200, 0x6300, 0x6400, 0x6500, 0x6600, 
		0x6700, 0x6800, 0x6900, 
		0x7000, 0x7100, 0x7200, 0x7300, 0x7400, 0x7500, 0x7600, 
		0x7700, 0x7800, 0x7900, 
		0 };

int tu003[] = { 0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2D, 0x2E, 
		0x30, 0x32, 0x48, 0 };

int tu004[] = { 0x07, 0x51, 0x62, 0xD01, 0};
	

int tu005[] = { 0x07, 0x51, 0x62, 0xC07, 0xC08, 0xC0E, 0xD01,0xD02,  0};

int tu006[] = { 0x07, 0x51, 0x62, 0xC07, 0xC08, 0xC0E, 0xD01,0xD02,  0};

int tu007[] = { 0x07, 0x51, 0x62, 0xC07, 0xC08, 0xC0E, 
	        0xC10, 0xC11, 0xC13, 0xC14, 0xC15, 0xC17, 0xD01,0xD02,  0};



int *tu_rctbl[] = { 	tu001, tu002, tu003 };


#endif

