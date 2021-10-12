/* @(#)96	1.3.1.1  src/bos/diag/da/corvette/dacorv.h, dascsi, bos41J, 9511A_all 2/28/95 16:48:22 */
/*
 *   COMPONENT_NAME: DASCSI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
struct	fru_bucket	frub[]=
{
/* 0 */
	{ "",	FRUB1,	0x890,	0x101,	RESET_FAILED,
		{ 100,	"",	"",	0,	DA_NAME,	EXEMPT	},
	},
/* 1 */
	{ "",	FRUB1,	0x890,	0x102,	REGISTER_FAILED,
		{ 100,	"",	"",	0,	DA_NAME,	EXEMPT	},
	},
/* 2 */
	{ "",	FRUB1,	0x890,	0x103,	IMMEDIATE_PACE,
		{ 100,	"",	"",	0,	DA_NAME,	EXEMPT	},
	},
/* 3 */
	{ "",	FRUB1,	0x890,	0x104,	SCB_FAILED,
		{ 100,	"",	"",	0,	DA_NAME,	EXEMPT	},
	},
/* 4 */
	{ "",	FRUB1,	0x890,	0x105,	ADDRESS_LINE,
		{ 100,	"",	"",	0,	DA_NAME,	EXEMPT	},
	},
/* 5 */
	{ "",	FRUB1,	0x890,	0x106,	PTC_INTERNAL,
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	},
/* 6 */
	{ "",	FRUB1,	0x890,	0x107,	PTC_EXTERNAL,
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	},
/* 7 */
	{ "",	FRUB1,	0x890,	0x108,	INT_BUS,
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	},
/* 8 */
	{ "",	FRUB1,	0x890,	0x109,	EXT_BUS,
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	},
/* 9 */
	{ "",	FRUB1,	0x890,	0x110,	ADDRESS_LINE,
	   {
		{ 95,	"",	"",	0,DA_NAME,	EXEMPT},
		{ 5,	"",	"",	0,	PARENT_NAME,	EXEMPT	},
	   }
	},
/* 10 */
	{ "",	FRUB1,	0x890,	0x150,	ELA_MCODE,
		{ 100,	"",	"",	0,	DA_NAME,	EXEMPT	},
	},

/* 11 */
	{ "",	FRUB1,	0x890,	0x155,	ELA_TIMEOUT,
	   {
		{ 95,	"",	"",	0,DA_NAME,	EXEMPT},
		{ 5,	"",	"",	0,	PARENT_NAME,	EXEMPT	},
	   }
	},

/* 12 */
	{ "",	FRUB1,	0x890,	0x160,	ELA_RESET,
	   {
		{ 95,	"",	"",	0,	DA_NAME,	EXEMPT	},
		{ 5,	"Cable","",CABLE_TERM,	NOT_IN_DB,	NONEXEMPT},
	   }
	},

/* 13 */
	{ "",	FRUB1,	0x890,	0x165,	ELA_DMA,
	   {
		{ 90,	"",	"",	0,	DA_NAME,	EXEMPT	},
		{ 10,	"",	"",	0,PARENT_NAME,	EXEMPT},
	   }
	},


/* 14 */
	{ "",	FRUB1,	0x890,	0x170,	ELA_MCODE,
	   {
		{ 100,	"",	"",	0,	DA_NAME,	EXEMPT	},
	   }
	},

/* 15 */
	{ "",	FRUB1,	0x890,	0x175,	ELA_PTC_INT,
	   {
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	   }
	},

/* 16 */
	{ "",	FRUB1,	0x890,	0x180,	ELA_PTC_EXT,
	   {
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	   }
	},

/* 17 */
	{ "",	FRUB1,	0x890,	0x185,	ELA_INT_BUS,
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	},
/* 18 */
	{ "",	FRUB1,	0x890,	0x190,	ELA_EXT_BUS,
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	},
/* 19 */
	{ "",	FRUB1,	0x890,	0x195,	ELA_FAILED,
		{ 100,	"","",SCSI_SUBSYSTEM,NO_FRU_LOCATION, NONEXEMPT	},
	},
};

#define	SE	5
#define	DE	6
#define	INTEGRATED	7
#define TURBO_DE	98
#define	UNKOWN_ADAPT	99
#define	MCODE	0
#define	DA_CORV	1
#define DE_TURBO_LL	80
