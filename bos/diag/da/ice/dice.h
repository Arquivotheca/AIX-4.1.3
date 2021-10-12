/* @(#)49	1.3  src/bos/diag/da/ice/dice.h, daice, bos41J, 9515A_all 4/11/95 10:59:58  */
/*
 *   COMPONENT_NAME: DAICE
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
#ifndef _H_DICE_H
#define _H_DICE_H

#include    "hga_tu.h"
#define		NO_ERROR		0
#define		ERROR_FOUND		-1
#define		YES			1
#define		NO			0
#define		QUIT			-1
#define 	OTHER 	(0x9999)
#define 	ASL_INITIALIZED	1
#define 	ASL_NOT_INITIALIZED	-1

#define		READ_KBD		ASL_OK

/* Defines used for values that are often looked at */
#define		CONSOLE_INSTALLED 	(da_input.console == CONSOLE_TRUE)
#define		ELA_MODE		(da_input.dmode == DMODE_ELA)
#define		PD_MODE			(da_input.dmode == DMODE_PD)
#define		REPAIR_MODE		(da_input.dmode == DMODE_REPAIR)
#define		ADVANCED		( da_input.advanced == ADVANCED_TRUE )
#define		SYSTEM			( da_input.system == SYSTEM_TRUE )
#define   	IPL				(da_input.exenv == EXENV_IPL)
#define   	CONCURRENT		(da_input.exenv == EXENV_CONC)

/* defines to check to see if it is in a particular mode */
#define		NOTLM			(da_input.loopmode == LOOPMODE_NOTLM)
#define		INLM			(da_input.loopmode == LOOPMODE_INLM)
#define		EXITLM			(da_input.loopmode == LOOPMODE_EXITLM)
#define		ENTERLM			(da_input.loopmode == LOOPMODE_ENTERLM)


/*
 *
 *
 * Below is where the device specific stuff starts
 * 
 * 
 */
#include "dice_msg.h"

/* TEST UNIT STRUCTURES */
#define RED_TU red_tu
#define GREEN_TU green_tu
#define BLUE_TU blue_tu
#define TU_ORD_IPL tu_ipl
#define TU_ORD_ADV tu_adv
#define TU_OPEN_DEX  tu_open
#define TU_CLOSE_DEX tu_close

int tu_adv[] = { 
	 	 VRAM_TEST,			/* TU 4 */
  		 LINES_DRAW_TEST,		/* TU 10 */
		 WINDOWING_TEST, 		/* TU 11 */
		 GM_TEST,		/* TU 44 */
	 	 RED_SCREEN,			/* TU 31 */
		 GREEN_SCREEN,		/* TU 32 */
		 BLUE_SCREEN,		/* TU 33 */
		 (int) NULL};  


int tu_ipl[] = { VRAM_TEST, WINDOWING_TEST, (int) NULL}; 
				
int tu_open[] = {TU_OPEN,(int) NULL};
int tu_close[] = {TU_CLOSE,(int) NULL};
int tu_config[] = {CARD_CONFIG_LEVEL,(int) NULL};
int red_tu[] = {RED_SCREEN,(int) NULL};
int blue_tu[] = {BLUE_SCREEN,(int) NULL};
int green_tu[] = {GREEN_SCREEN,(int) NULL};

#define ERRIDS errids
int errids[] = {(int) NULL};

/* 
 *  MENU defines 
 */
#define CATALOG					MF_DICE
#define MSG_SET					ICE_MSG
/* #define UTIL_MSG_SET				UICE_MSG */
#define ADVANCED_MSG_ID			ADVANCED_MODE_MENU
#define CUSTOMER_MSG_ID			CUSTOMER_MODE_MENU
#define LOOPMODE_MSG_ID			LOOPMODE_MODE_MENU


/*
 * Set up menus to be displayed 
 */
struct msglist havedisp[] =
{
	{ICE_MSG, TITLE},
	{ICE_MSG, YES_OPTION},
	{ICE_MSG, NO_OPTION},
	{ICE_MSG, HAVEDISP},
	{(int)NULL,(int)NULL}
};

struct msglist color_screen[] =
{
	{ICE_MSG, TITLE},
	{ICE_MSG, YES_OPTION},
	{ICE_MSG, NO_OPTION},
	{ICE_MSG, COLOR_SCREEN},
	{(int)NULL,(int)NULL}
};

struct msglist builtin[] =
{
	{ICE_MSG, TITLE},
	{ICE_MSG, YES_OPTION},
	{ICE_MSG, NO_OPTION},
	{ICE_MSG, DISP_PD},
	{(int)NULL,(int)NULL}
};

struct msglist f3_exit[] =
{
	{ICE_MSG, TITLE},
	{ICE_MSG, F3_EXIT},
	{(int)NULL,(int)NULL}
};


#define DEFAULT_CARD_TYPE S3_928_ID

/* device ID's */
#define S3_928_ID      (0x3353b088)

/* Defines for the FFC */
#define FFC_S3 0x81C

/* 
 * * FRU structures ** 
 */
struct fru_bucket frus[] = {
    { "", FRUB1, FFC_S3, 0x100, ICE_102,     /* 2 - Adapter test failure */
        {
            {90, "", "", 0,  DA_NAME, EXEMPT},
            {10, "", "", 0,  PARENT_NAME, EXEMPT},
        },
    },
    { "", FRUB1, FFC_S3, 0x200, ICE_103,     /* 3 - Display test failure */
        {
            {65, "", "", 0,  DA_NAME, EXEMPT},
            {35, "Display", "", DISPLAY,  NOT_IN_DB, EXEMPT},
        },
    },
    { "", FRUB1, FFC_S3, 0x101, ICE_103,     /* 4 - Display test failure */
        {
            { 100, "Display", "", DISPLAY,  NOT_IN_DB, EXEMPT},
        },
    },
};  /* end of ICE_frus */

#define CHAR_TO_INT(x) (x[0]<<24 | x[1]<<16 | x[2]<<8 | x[3])

#endif
