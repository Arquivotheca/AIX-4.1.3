/* @(#)31	1.3  src/bos/diag/da/pcitok/dtokpci.h, datokpci, bos41J, 9516A_all 4/18/95 10:19:49  */
/*
 *   COMPONENT_NAME: datokpci
 *
 *   FUNCTIONS:
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

#ifndef DA_TOKPCI
#define DA_TOKPCI


#include "dtokpci_msg.h"
#define		GOOD		0
#define		NOT_GOOD	-1
#define		BAD 		-1
#define		QUIT		-1
#define         YES 		1
#define         NO		2
#define		TYPE_3		2
#define 	NOT_TYPE_3	1

#define 	LOOPMODE	(da_input.dmode == LOOPMODE_INLM)
#define		EXITLM		(da_input.dmode == LOOPMODE_EXITLM)
#define		NOTLM		(da_input.dmode == LOOPMODE_NOTLM)
#define		CATALOG		MF_DTOKPCI

/*
*	FRU tables of token-ring pci adapter 
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
#define FRU_700         7

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
	/* fail from tu001; Config register test	- TU 1*/ 
  	{"", FRUB1, 0x750, 0x200, TOK_200,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},

		},
	},
	/* fail from tu002; I/O Registers test - TU 2	*/ 
  	{"", FRUB1, 0x750, 0x201, TOK_201,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},
		},
	},
	/* fail from tu03, Adapter self-test failure - TU 3*/
  	{"", FRUB1, 0x750, 0x202, TOK_202,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},
		},
	},
	/* fail from tu04, Connect Test - TU 4*/
  	{"", FRUB1, 0x750, 0x203, TOK_203,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},
		},
	},
	/* fail from tu005 Token-Ring internal wrap  Failure - TU 5*/
  	{"", FRUB1, 0x750, 0x204, TOK_204,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},
		},
	},
	/* fail from tu006 Token-Ring external Wrap Failure - TU 6*/
  	{"", FRUB1, 0x750, 0x300, TOK_300,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},
 			{10, " ", "",  CABLE_MSG, NOT_IN_DB, EXEMPT},
		},
	},
	/* fail from tu007, Network Initialization Test Failure */ 
  	{"", FRUB1, 0x750, 0x400, TOK_400,
		{
			{70, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},
 			{10, " ", "",  CABLE_MSG, NOT_IN_DB, EXEMPT},
 			{10, " ", "", TOK_NET_MSG, NOT_IN_DB, EXEMPT},
		},
	},
	/* Error log analysis indicates a failure	*/ 
  	{"", FRUB1, 0x750, 0x700, TOK_700,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
 			{10, "", "",  0, PARENT_NAME, EXEMPT},
		},
	},

};



/* This message will ask the user which type of cable is tested */
struct msglist which_type_of_cable[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, NONTELE_CABLE			},
                { 1, TELE_CABLE				},
                { 1, NO_CABLE				},
                { 1, WIRING_TYPE			},
                { 0,0},
        };
ASL_SCR_INFO asi_which_type_of_cable[DIAG_NUM_ENTRIES(which_type_of_cable)];

/* this message will ask the user if he wants to test cable     */

struct msglist testing_cable_yes_no[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, CABLE_REQUIRE		},
                { 0,0},
        };
ASL_SCR_INFO asi_testing_cable_yes_no[DIAG_NUM_ENTRIES(testing_cable_yes_no)];

/* this message asks the user if the Token-Ring network connected	*/
struct msglist network_connected[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, YES_OPTION     	  	},
                { 1, NO_OPTION          	},
                { 1, NETWORK_CONNECTED},
                { 0,0},
        };
ASL_SCR_INFO asi_network_connected[DIAG_NUM_ENTRIES(network_connected)];

/* this message will ask the user to plug the cable to be tested in */

struct msglist plug_cable_type_3[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, PLUG_CABLE_TYPE_3		},
                { 1, FINISHED			},
                { 0,0},
        };
ASL_SCR_INFO asi_plug_cable_type_3[DIAG_NUM_ENTRIES(plug_cable_type_3)];

/* Ask the user to plug the regular Token-Ring cable	*/
struct msglist plug_cable[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, PLUG_CABLE				},
                { 1, FINISHED				},
                { 0,0},
        };
ASL_SCR_INFO asi_plug_cable[DIAG_NUM_ENTRIES(plug_cable)];

/* Ask the user unplug the cable from the network but leave the cable 	*/
/* hooked to the adapter						*/
struct msglist unplug_cable[]=
        {
                { 1, TESTING_ADVANCED_NO_STANDBY	},
                { 1, UNPLUG_CABLE			},
                { 1, FINISHED				},
                { 0,0},
        };
ASL_SCR_INFO asi_unplug_cable[DIAG_NUM_ENTRIES(unplug_cable)];



struct msglist put_cable_back[]=
	{
		{ 1, TESTING_ADVANCED_NO_STANDBY	},
		{ 1, PUT_CABLE_BACK		},
                { 1, FINISHED			},
                { 0,0},
	};
ASL_SCR_INFO asi_put_cable_back[DIAG_NUM_ENTRIES(put_cable_back)];


#endif

