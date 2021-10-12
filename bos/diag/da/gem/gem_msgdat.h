/* @(#)43       1.5  src/bos/diag/da/gem/gem_msgdat.h, dagem, bos411, 9428A410j 3/22/94 16:24:46 */
/*
 *   COMPONENT_NAME: DAGEM
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*-----------------
  Menu Nos: Displayed at the upper right hand corner 
  of each message panel. Formed by concatanating the LED no. 
  for graphics subsystem ( 0x871 ) and a serial no.
 		---------------*/

#define MENUNO_GEM_1    0x871001  /* advanced */
#define MENUNO_GEM_2    0x871002  /* looping */
#define MENUNO_GEM_3    0x871003  /* rgb     */
#define MENUNO_GEM_4    0x871004  /* red */
#define MENUNO_GEM_5    0x871005  /* green */
#define MENUNO_GEM_6    0x871006  /* blue */
#define MENUNO_GEM_7    0x871007  /* have disp?      */
#define MENUNO_GEM_8    0x871008  /* pdp  (not used) */
#define MENUNO_GEM_9    0x871009  /* taurus present  */
#define MENUNO_GEM_10   0x871010  /* taurus power    */
#define MENUNO_GEM_11   0x871011  /* rgb 7 seconds   */
#define MENUNO_GEM_12   0x871012  /* was it rgb      */

/*-----------------
  Message list structures for the various message panels.
  The constants 'GEM_NOTIFY' etc. are defined in 'dgraph_msg.h',
  which in turn is generated from 'dgraph.msg'
 		---------------*/

struct msglist gem_datitle[] =
{
  	{ GEM_NOTIFY,GEM_TITLE},
  	{ GEM_NOTIFY,GEM_STDBY},
	{ (int)0, (int)0 }
};

struct msglist rgb_screen[] =
{
	{ GEM_NOTIFY,GEM_TITLE },
	{ GEM_NOTIFY,SAY_YES   },
	{ GEM_NOTIFY,SAY_NO    },
	{ GEM_NOTIFY,DGEM_RGBQ },
	{ (int)0, (int)0 }
};

struct	msglist	havedisp_screen[]=
	{
		{ GEM_NOTIFY,GEM_TITLE },
		{ GEM_NOTIFY,SAY_YES   },
		{ GEM_NOTIFY,SAY_NO    },
		{ GEM_NOTIFY, DGEM_HAVEDISP },
		{ (int)0, (int)0 }
	};

struct	msglist	red_screen[]=
	{
		{ GEM_NOTIFY,GEM_TITLE },
		{ GEM_NOTIFY,SAY_YES   },
		{ GEM_NOTIFY,SAY_NO    },
		{ GEM_NOTIFY, DGEM_REDCUR },
		{ (int)0, (int)0 }
	};

struct	msglist	grn_screen[]=
	{
		{ GEM_NOTIFY,GEM_TITLE },
		{ GEM_NOTIFY,SAY_YES   },
		{ GEM_NOTIFY,SAY_NO    },
		{ GEM_NOTIFY, DGEM_GREENCUR },
		{ (int)0, (int)0 }
	};

struct	msglist	blue_screen[]=
	{
		{ GEM_NOTIFY,GEM_TITLE },
		{ GEM_NOTIFY,SAY_YES   },
		{ GEM_NOTIFY,SAY_NO    },
		{ GEM_NOTIFY, DGEM_BLUECUR },
		{ (int)0, (int)0 }
	};

struct  msglist taurus_present[]=
	{
		{ GEM_NOTIFY,GEM_TITLE },
		{ GEM_NOTIFY,SAY_YES   },
		{ GEM_NOTIFY,SAY_NO    },
		{ GEM_NOTIFY, TAURUS_PRESENT },
		{ (int)0, (int)0 }
	};

struct  msglist taurus_power[]=
	{
		{ GEM_NOTIFY,GEM_TITLE },
		{ GEM_NOTIFY,SAY_YES   },
		{ GEM_NOTIFY,SAY_NO    },
		{ GEM_NOTIFY, TAURUS_POWER },
		{ (int)0, (int)0 }
	};


struct msglist rgb_7_sec[] =
{
	{ GEM_NOTIFY,GEM_TITLE},
	{ GEM_NOTIFY,RGB_7_SEC},
	{ (int)0 }
};

struct  msglist was_it_rgb[]=
	{
		{ GEM_NOTIFY,GEM_TITLE },
		{ GEM_NOTIFY,SAY_YES   },
		{ GEM_NOTIFY,SAY_NO    },
		{ GEM_NOTIFY, WAS_IT_RGB },
		{ (int)0, (int)0 }
	};


/*--------------
	graphics subsystem error codes in the RAS error log.
    -----------------*/

#define	FAULT_GCP		"3213"
#define	FAULT_DRP		"3214"
#define	FAULT_SHP		"3215"
#define PARITY_ERR_HI_GCP	"3219"
#define PARITY_ERR_LO_GCP	"3222"
#define PARITY_ERR_HI_DRP	"3220"
#define PARITY_ERR_LO_DRP	"3223"
#define PARITY_ERR_HI_SHP	"3221"
#define PARITY_ERR_LO_SHP	"3224"
#define BUS_PARITY_ERR		"3230"
#define BUS_TIMEOUT		"3231"
#define CPU_BUS_TIMEOUT		"3232"
