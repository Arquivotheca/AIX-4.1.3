static char sccsid[] = "@(#)59	1.22.1.23  src/bos/diag/da/memory/dmemory.c, damemory, bos41J, 9521B_all 5/24/95 15:23:05";
/*
 * COMPONENT_NAME: DAMEMORY
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <math.h>
#include <locale.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>		/* includes structures holding ipl results */

#include "diag/da.h"		/* The following header files has information */
#include "diag/diag_exit.h"	/* particular to this program. */
#include "diag/tmdefs.h"
#include "diag/tm_input.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h"
#include "diag/class_def.h"
#include "diag/modid.h"
#include "memory_msg.h"


/* fru_bucket is a structure that holds information for the diagnostic 
   program to return to the diagnostic controller when a failure is found
   that needs to be reported. (FRU means Field Replacable Unit) */

struct fru_bucket l2error[] = {

    { "", FRUB1, 0x811, 0x102, RR_BLANK,                    {
			   { 100, "L2 CACHE", " ", FF_BLANK, NOT_IN_DB, EXEMPT },
				   },
    },
};

struct fru_bucket l2simm[] = {

    { "", FRUB1, 0x940, 0x600, R_L2CACHE,		     {
			     { 100, "L2 CACHE SIMM", "", F_L2CACHE, NOT_IN_DB, EXEMPT },
				     },
    }, 
};


struct fru_bucket riser[] = {

    { "", FRUB1, 0x940, 0x600, R_L2CACHE,		     {
			     { 100, "RISER CARD", " ", F_RISER, NOT_IN_DB, EXEMPT },
				     },
    }, 
};


struct fru_bucket planar[] = {

    { "", FRUB1, 0x812, 0x900, RR_PLANAR, {
			     { 100, "", "", 0, PARENT_NAME, NONEXEMPT },
				     },
    }

};

struct fru_bucket sal_simm[] = {

    { "", FRUB1, 0x812, 0x900, R_SIMM_ERROR, {
			     { 100, "SIMM", "", 0, DA_NAME, EXEMPT },
				     },
    }

};

struct fru_bucket simm1_pegasus[] = {
    /* MEMCARD_MR2_8 (MR2 CARD, 8 Mb MD2 SIMM) */
    { "", FRUB1, 0x716, 0x147, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_MD2, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MR4_32 (MR4 CARD, 32 Mb MD4 SIMM) */
    { "", FRUB1, 0x716, 0x167, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_MD4, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_8 (MRE CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x148, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_8 (NFx CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x149, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_16 (MRE CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x158, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT},
				     },
    }, 
    /* MEMCARD_MRE_32 (MRE CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x168, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT},
				     },
    }, 
    /* MEMCARD_NFx_16 (NFx CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x159, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT},
				     },
    }, 
    /* MEMCARD_NFx_32 (NFx CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x169, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT},
				     },
    }, 
};

struct fru_bucket simm2_pegasus[] = {
    /* MEMCARD_MR2_8 (MR2 CARD, 8 Mb MD2 SIMM) */
    { "", FRUB1, 0x716, 0x347, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_MD2, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_MD2, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MR4_32 (MR4 CARD, 32 Mb MD4 SIMM) */
    { "", FRUB1, 0x716, 0x367, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_MD4, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_MD4, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_8 (MRE CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x348, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_8 (NFx CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x349, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_16 (MRE CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x358, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_32 (MRE CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x368, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_16 (NFx CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x359, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_32 (NFx CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x369, R_CARD2SIMMS,		     {
			     { 60, "", "", 0, DA_NAME, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
			     { 20, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
};

struct fru_bucket simm3_pegasus[] = {
    /* MEMCARD_MR2_8 (MR2 CARD, 8 Mb MD2 SIMM) */
    { "", FRUB1, 0x716, 0x447, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_MD2, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_MD2, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_MD2, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MR4_32 (MR4 CARD, 32 Mb MD4 SIMM) */
    { "", FRUB1, 0x716, 0x467, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_MD4, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_MD4, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_MD4, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_8 (MRE CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x448, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_8 (NFx CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x449, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_8MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_16 (MRE CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x458, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_32 (MRE CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x468, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_16 (NFx CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x459, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_16MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_32 (NFx CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x469, R_CARD3SIMMS,		     {
			     { 70, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
			     { 10, "SIMM", "", F_SIMM_32MB, NOT_IN_DB, EXEMPT },
				     },
    }, 
};

struct fru_bucket simm4_pegasus[] = {
    /* MEMCARD_MR2_8 (MR2 CARD, 8 Mb MD2 SIMM) */
    { "", FRUB1, 0x716, 0x247, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
    /* MEMCARD_MR4_32 (MR4 CARD, 32 Mb MD4 SIMM) */
    { "", FRUB1, 0x716, 0x267, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_8 (MRE CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x248, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_8 (NFx CARD, 8 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x249, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_16 (MRE CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x258, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
    /* MEMCARD_MRE_32 (MRE CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x268, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_16 (NFx CARD, 16 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x259, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
    /* MEMCARD_NFx_32 (NFx CARD, 32 Mb std SIMM) */
    { "", FRUB1, 0x716, 0x269, R_CARD_ERROR,		     {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    }, 
};

struct fru_bucket newsimm[] = {

    { "", FRUB1, 0x940, 0x100, R_SIMM_ERROR,		     {
			     { 100, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT },
				     },
    }, 
};

struct fru_bucket newcard[] = {

    { "", FRUB1, 0x940, 0x200, R_CARD_ERROR, {
			     { 90, "", "", 0, DA_NAME, EXEMPT },
			     { 10, "", "", 0, PARENT_NAME, NONEXEMPT },
				     },
    },
};


struct fru_bucket newcard_2simms[]=
{
	{"", FRUB1, 0x940, 0x300, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT},
		},
	},
};


struct fru_bucket newcard_3simms[]=
{
	{"", FRUB1, 0x940, 0x400, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM, NOT_IN_DB, EXEMPT},
		},
	},
};

struct fru_bucket newtwocards[] = {

    { "", FRUB1, 0x940, 0x500, R_CARD_ERROR, {
			     { 50, "", "", 0, DA_NAME, EXEMPT },
			     { 50, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
};

struct fru_bucket simm[] = {

    { "", FRUB1, 0, 0, 0,		     {
			     { 0, "INVALID ERROR", "", 0, 0, 0 },
				     },
    }, 
    { "", FRUB1, 0x812, 0x173, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_1, NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x174, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_2, NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x187, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_4, NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x177, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_8S15,NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x176, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_4S15,NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x172, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_4S3,NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x182, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_8S3,NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x155, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_4S25,NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x175, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_1S3,NOT_IN_DB, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x179, R_SIMM_ERROR, {
			     { 100, "SIMM", "", F_SIMM_2S3,NOT_IN_DB, EXEMPT },
				     },
    },
};

struct fru_bucket card[] = {

    { "", FRUB1, 0, 0, 0,     	     {
			     { 0, "INVALID ERROR", "", 0, 0, 0 },
				     },
    },
    { "", FRUB1, 0x812, 0x171, R_CARD_ERROR, {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x171, R_CARD_ERROR, {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x180, R_CARD_ERROR, {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x178, R_CARD_ERROR, {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x156, R_CARD_ERROR, {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x157, R_CARD_ERROR, {
			     { 100, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
};


struct fru_bucket card_2simms[]=
{
	{"", FRUB1, 0x812, 0x190, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_1, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_1, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x200, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_2, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_2, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x210, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x220, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_8S15, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_8S15, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x230, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4S15, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4S15, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x240, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_8S3, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_8S3, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x250, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4S3, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4S3, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x260, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4S25, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_4S25, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x270, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_1S3, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_1S3, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x280, R_CARD2SIMMS,
		{
		    {60, "", "", 0, DA_NAME, EXEMPT},
		    {20, "SIMM", "", F_SIMM_2S3, NOT_IN_DB, EXEMPT},
		    {20, "SIMM", "", F_SIMM_2S3, NOT_IN_DB, EXEMPT},
		},
	},
};


struct fru_bucket card_3simms[]=
{
	{"", FRUB1, 0x812, 0x195, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_1, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_1, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_1, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x205, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_2, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_2, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_2, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x215, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x225, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_8S15, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_8S15, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_8S15, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x235, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S15, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S15, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S15, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x245, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_8S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_8S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_8S3, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x255, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S3, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x265, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S25, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S25, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_4S25, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x275, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_1S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_1S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_1S3, NOT_IN_DB, EXEMPT},
		},
	},
	
	{"", FRUB1, 0x812, 0x285, R_CARD3SIMMS,
		{
		    {70, "", "", 0, DA_NAME, EXEMPT},
		    {10, "SIMM", "", F_SIMM_2S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_2S3, NOT_IN_DB, EXEMPT},
		    {10, "SIMM", "", F_SIMM_2S3, NOT_IN_DB, EXEMPT},
		},
	},
	
};

struct fru_bucket twocards[] = {

    { "", FRUB1, 0x812, 0x300, R_CARD_ERROR, {
			     { 50, "", "", 0, DA_NAME, EXEMPT },
			     { 50, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x400, R_CARD_ERROR, {
			     { 50, "", "", 0, DA_NAME, EXEMPT },
			     { 50, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x500, R_CARD_ERROR, {
			     { 50, "", "", 0, DA_NAME, EXEMPT },
			     { 50, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x600, R_CARD_ERROR, {
			     { 50, "", "", 0, DA_NAME, EXEMPT },
			     { 50, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x700, R_CARD_ERROR, {
			     { 50, "", "", 0, DA_NAME, EXEMPT },
			     { 50, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
    { "", FRUB1, 0x812, 0x800, R_CARD_ERROR, {
			     { 50, "", "", 0, DA_NAME, EXEMPT },
			     { 50, "", "", 0, DA_NAME, EXEMPT },
				     },
    },
};

struct msglist test_or_not[] = {
				{MEM_TEST_NOTEST, MEM_TEST_TITLE},
				{MEM_TEST_NOTEST, MEM_TEST_YES},
				{MEM_TEST_NOTEST, MEM_TEST_NO},
				{MEM_TEST_NOTEST, MEM_TEST_ACTION},
				(int)NULL
			       };


extern	getdainput();

static ASL_SCR_INFO meminfo[DIAG_NUM_ENTRIES(test_or_not)];
static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

struct	tm_input	tm_input;	/* info. form dc to da */
struct	fru_bucket	temp_frub;	/* used for substitution */
extern  nl_catd diag_catopen(char *, int);
nl_catd	fdes;				/* file descriptor for catalog file */
int     fd;				/* file descriptor for nvram */
int	s_slot;				/* slot for salmon simm */
int	newfru=0;
char    sal_slot_table[] = { 'A','B','C','D','E','F','G','H' };
char    buffer[132];			/* temp. buffer for substitution */
char    buffer1[132];			/* temp. buffer for substitution */
char	MRB_name[][4] = { "MR2", "MR4", "MRE", "NFx" };
/* these defs are indexes in pegasus fru_bucket tables */
/* the numbers in names (8 16 32) are SIMM sizes, not card sizes! */
#define MEMCARD_MR2_8	0	/* also MR2 index in MRB_name */
#define MEMCARD_MR4_32	1	/* also MR4 index in MRB_name */
#define MEMCARD_MRE_8	2	/* also MRE index in MRB_name */
#define MEMCARD_NFx_8	3	/* also NFx index in MRB_name */
#define MEMCARD_MRE_16	4
#define MEMCARD_MRE_32	5
#define MEMCARD_NFx_16	6
#define MEMCARD_NFx_32	7

struct	CuDv		*T_CuDv;
struct	CuDv		*L_CuDv;
struct	CuVPD		*T_CuVPD;
struct	listinfo	c_info,vpd_info;
struct	CuAt		*cuat_size;
struct	CuAt		*cuat_ssize;
struct	CuAt		*cuat_type;
struct	CuAt		*cuat_cardec;
struct	CuAt		*cuat_realmem;

#define MEMDEBUG 0			/* debug flag to simulate failures */	
#define CARD_SIZE(a) ((((a & 0x0000ffff) ^ 0x0000ffff) +1) / 16)
#define SIMM_SIZE(a) (CARD_SIZE(a) / 8)
#define LAST_16_BITS(a) (a & 0x0000ffff)
#define MEG_1 "1"
#define MEG_2 "2"
#define MEG_4 "4"
#define MEG_8 "8"
#define MEG_16 "16"
#define MEG_32 "32"
#define MEG_64 "64"
#define MEG_128 "128"
#define MEG_256 "256"
#define MEG_512 "512"
#define MEG_1024 "1024"
#define U1     "U1"
#define S1_5   "S1.5"
#define S2_5   "S2.5"
#define S3     "S3"
#define S4     "S4"
#define S5     "S5"
#define S6     "S6"
#define S7     "S7"

#define DEBUG 0

#if DEBUG
FILE *membug;
#endif

#if DEBUG
char *tbuf;
#endif

IPL_DIRECTORY  *iplcb_dir;
IPL_INFO       *iplcb_info;
MACH_DD_IO      l;
MACH_DD_IO      m;
MACH_DD_IO      n;
MACH_DD_IO      o;
MACH_DD_IO      p;
MACH_DD_IO      pc;
MACH_DD_IO      l2;
MACH_DD_IO      proc;

RAM_DATA	*iplcb_ram_data;
RAM46_DATA	*iplcb_ram46_data;
MEM_DATA	*iplcb_mem_data,*mem_defect_list;
PROCESSOR_DATA	*iplcb_proc_data;
L2_DATA		*iplcb_l2_data;

int		rsc_type;

main()
{

    int             i,ii;		/* loop counter */
    int             j, jj;		/* loop counter */
    int             rc;			/* user's response to screen */
    int             level;		/* machine level 0.9 or 1.0 */
    int             jcount, count;	/* scan counter through table */
    int             simm_table[9][9];	/* showing location of bad simms */
    int             temp_table[9][9];	/* showing location of bad simms */
    int             simm_or_card[9][9];	/* showing either simm or card is bad */
    int             bitcount;		/* no. of bits turned 1 in bit map */
    int             badsimmcount;	/* no. of bad simms in simm table */
    int             x;			/* size of bad simm or card */
    int             offset_in_byte;	/* size of ipl control block */
    int             bad_mem_in_K;	/* bad memory found during IPL */
    int             bad_ratio;		/* bad memory size/entire memory size */
    int		    bytes_per_bit;
    int             ext_size;
    int             bad_simm_present;
    int             bitmap_size;
    int             model;		/* machine model		*/

    char            slot[9][2];		/* labelling memory slots */
    char           *bitmap_ptr;		/* pointer to bit map */
    char            Z;			/* temporary character for analysis */
    char            tstr[NAMESIZE];	/* temp. string for substitution */
    char           *memname;		/* used for substitution */
    char           *temp_str;		/* used for substitution */
    char           *tmp_str;		/* used for substitution */
    char           *beg_addr;
    char           *end_addr;


    setlocale(LC_ALL,"");

    iplcb_dir = (IPL_DIRECTORY *) malloc (sizeof(IPL_DIRECTORY));
    iplcb_info = (IPL_INFO *) malloc (sizeof(IPL_INFO));
    iplcb_ram_data = (RAM_DATA *) malloc (sizeof(RAM_DATA));
    iplcb_ram46_data = (RAM46_DATA *) malloc (sizeof(RAM46_DATA));
    iplcb_mem_data = (MEM_DATA *) malloc (8*sizeof(MEM_DATA));
    iplcb_l2_data = (L2_DATA *) malloc (sizeof(L2_DATA));
    iplcb_proc_data = (PROCESSOR_DATA *) malloc (sizeof(PROCESSOR_DATA));

    /* Initialize default return status */
    DA_SETRC_STATUS(DA_STATUS_GOOD);
    DA_SETRC_USER(DA_USER_NOKEY);
    DA_SETRC_ERROR(DA_ERROR_NONE);
    DA_SETRC_TESTS(DA_TEST_SHR);
    DA_SETRC_MORE(DA_MORE_NOCONT);

    if ((fd = open("/dev/nvram", 0)) < 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }
    l.md_incr = MV_BYTE;
    l.md_addr = 128;
    l.md_data = (char *) iplcb_dir;
    l.md_size = sizeof(*iplcb_dir); 
    if (ioctl(fd, MIOIPLCB, &l)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* Processor info configuration table */
    proc.md_incr = MV_BYTE;
    proc.md_addr = iplcb_dir->processor_info_offset;
    proc.md_data = (char *) iplcb_proc_data;
    proc.md_size = sizeof(*iplcb_proc_data); 
    if (ioctl(fd, MIOIPLCB, &proc)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* uniform memory configuration table */
/* these data are valid also for PEGASUS */
    p.md_incr = MV_BYTE;
    p.md_addr = iplcb_dir->mem_data_offset;
    p.md_data = (char *) iplcb_mem_data;
    p.md_size = 8*sizeof(*iplcb_mem_data); 
    if (ioctl(fd, MIOIPLCB, &p)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* s13 - salmon information */
    o.md_incr = MV_BYTE;
    o.md_addr = iplcb_dir->ram_post_results_offset;
    o.md_data = (char *) iplcb_ram_data;
    o.md_size = sizeof(*iplcb_ram_data); 
    if (ioctl(fd, MIOIPLCB, &o)) {
       	DA_SETRC_ERROR(DA_ERROR_OTHER);
       	DA_EXIT();
    }

/* s? - power pc information */
    pc.md_incr = MV_BYTE;
    pc.md_addr = iplcb_dir->ram_post_results_offset;
    pc.md_data = (char *) iplcb_ram46_data;
    pc.md_size = sizeof(*iplcb_ram46_data); 
    if (ioctl(fd, MIOIPLCB, &pc)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

/* s1 */
    m.md_incr = MV_BYTE;
    m.md_addr = iplcb_dir->ipl_info_offset;
    m.md_data = (char *) iplcb_info;
    m.md_size = sizeof(*iplcb_info); 
    if (ioctl(fd, MIOIPLCB, &m)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

    bitmap_ptr = (char *) malloc(iplcb_dir->bit_map_size);
    n.md_incr = MV_BYTE;
    n.md_addr = iplcb_dir->bit_map_offset;
    n.md_data = (char *)bitmap_ptr;
    n.md_size = iplcb_dir->bit_map_size; 
    if (ioctl(fd, MIOIPLCB, &n)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

    if (init_dgodm() != 0) {
        term_dgodm();
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

    rc = getdainput(&tm_input);
    if (rc != 0) {
        term_dgodm();
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

    if (tm_input.console == CONSOLE_TRUE)
        diag_asl_init(ASL_INIT_DEFAULT);

    fdes = diag_catopen(MF_MEMORY,0);

    /* If Salmon, use different analysis and clean up */
    rsc_type = salmon();

    if ((rs2()==2) || (rsc_type==2)) {
            /* L2 cache configuration table */
            l2.md_incr = MV_BYTE;
            l2.md_addr = iplcb_dir->l2_data_offset;
            l2.md_data = (char *) iplcb_l2_data;
            l2.md_size = sizeof(*iplcb_l2_data); 
            if (ioctl(fd, MIOIPLCB, &l2)) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                DA_EXIT();
            }
    }

    temp_str = (char *) malloc(1024);
    tmp_str = (char *) malloc(1024);

    /* This part will analyze the bit map and determine how many blocks (pages) 
       have been marked bad during ros ipl */

/*
    bitmap_ptr = (char *) iplcb_ptr;		
    size_in_byte = (iplcb_ptr->s0.bit_map_size);
    offset_in_byte = (iplcb_ptr->s0.bit_map_offset);
    bitmap_ptr += offset_in_byte;	
    */

    bitcount = 0;
    /* iplcb_dir->bit_map_size is the maximum bitmap size */
    /* get_bitmap_size return the significant bitmap size */
    /* from the CuAt memory objects */
    bitmap_size = get_bitmap_size();
    if (bitmap_size == 0)		/* conversion aborted ...	*/
	bitmap_size = iplcb_dir->bit_map_size;
/*    for (x = 0; x < iplcb_dir->bit_map_size; ++x) {	*/
    for (x = 0; x < bitmap_size; ++x) {
        Z = *bitmap_ptr;
        for (i = 0; i < 8; ++i)
            bitcount += (((Z >> i) & 1) ? 1 : 0);
        ++bitmap_ptr;
    }

#if DEBUG
membug=fopen("/tmp/mem.debug","a+");
tbuf = (char *) malloc(1024);
sprintf(tbuf,"bit map size = %d\n",iplcb_dir->bit_map_size);
diag_asl_msg(tbuf);
fprintf(membug,tbuf);
fflush(membug);
sprintf(tbuf,"model = %x\n",iplcb_info->model);
diag_asl_msg(tbuf);
fprintf(membug,tbuf);
fflush(membug);
sprintf(tbuf,"bitcount = %d\n",bitcount);
diag_asl_msg(tbuf);
fprintf(membug,tbuf);
fflush(membug);
free(tbuf);
#endif

    /* Every bit that is marked 1 represents 16K of memory is bad */
    bad_mem_in_K = 16 * bitcount;
    bad_ratio = bad_mem_in_K * 100 * 1024 / iplcb_info->ram_size;

    /* Memory PRE-TEST */
    if (tm_input.exenv == EXENV_IPL) {
	if ((iplcb_info->ram_size == 12582912) || 
            ((iplcb_info->ram_size - (bitcount*16*1024)) > 12582912))
                clean_up();
    }

#if DEBUG
diag_asl_msg("Start analysing\n");
printf("Start analysing\n");
fprintf(membug,"Start analysing\n");
fflush(membug);
#endif

    /* Display "stand by" screen and do analysis */
    if (tm_input.console == CONSOLE_TRUE) {
        switch (tm_input.advanced) {
            case ADVANCED_TRUE:
                if (tm_input.loopmode != LOOPMODE_NOTLM)
                   rc = diag_msg_nw(0x812004,fdes,ALOOP,ALTITLE,tm_input.lcount,
                             	 tm_input.lerrors);
                else
                   rc = diag_msg_nw(0x812003, fdes, ADVANCED, ATITLE);
                break;
            case ADVANCED_FALSE:
                if (tm_input.loopmode != LOOPMODE_NOTLM)
                   rc = diag_msg_nw(0x812002,fdes,LOOP,LTITLE,tm_input.lcount,
                             	 tm_input.lerrors);
                else
                   rc = diag_msg_nw(0x812001, fdes, REGULAR, RTITLE);
                break;
            default:
                break;
        }
        sleep(1);
	rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }

    check_missing();

    if (IsPowerPC_SMP(get_cpu_model(&model)))
	get_pegasus_defect_list();

    if ((rs2()==2) || (rsc_type==2)) {
        check_l2cache();
        check_missL2();
    }

    if (rsc_type == 1) 
       check_salmon();

    if (rsc_type == 2)
        check_powerpc();

    if ((rs2()==1) || (rs2()==2))
        check_newmemory();

/*
sprintf(tmp_str,"cache_line_size = %d\n", iplcb_info->cache_line_size);
diag_asl_msg(tmp_str);
free(tmp_str);
tmp_str = (char *) malloc(1024);
sprintf(tmp_str,"IO planar level = %d\n",iplcb_info->IO_planar_level_reg);
diag_asl_msg(tmp_str);
*/


    if (bitcount == 0)  		/* Memory is good */
        clean_up();

    /* Display "stand by" screen and do analysis */
    if (tm_input.console == CONSOLE_TRUE) {
        switch (tm_input.advanced) {
            case ADVANCED_TRUE:
                if (tm_input.loopmode != LOOPMODE_NOTLM)
                   rc = diag_msg_nw(0x812004,fdes,ALOOP,ALTITLE,tm_input.lcount,
                             	 tm_input.lerrors);
                else
                   rc = diag_msg_nw(0x812003, fdes, ADVANCED, ATITLE);
                break;
            case ADVANCED_FALSE:
                if (tm_input.loopmode != LOOPMODE_NOTLM)
                   rc = diag_msg_nw(0x812002,fdes,LOOP,LTITLE,tm_input.lcount,
                             	 tm_input.lerrors);
                else
                   rc = diag_msg_nw(0x812001, fdes, REGULAR, RTITLE);
                break;
            default:
                break;
        }
        sleep(1);
	rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }

    /* Using contents of cache_line_size to determine level of machine */
    if (iplcb_info->cache_line_size == 64)
        level = 0;			/* level = 0 means a 0.9 machine */
    else if (iplcb_info->cache_line_size == 128)
        level = 1;    			/* level = 1 means a 1.0 machine */
    else {            			/* some other models covered by other tests */
    };


    /* Assigning slot identification according to layout in ipl control block */
    /* 1.0 machine */
    slot[1][1] = 'H';
    slot[2][1] = 'F';
    slot[3][1] = 'G';
    slot[4][1] = 'E';
    slot[5][1] = 'D';
    slot[6][1] = 'B';
    slot[7][1] = 'C';
    slot[8][1] = 'A';

    /* 0.9 machine */
    slot[1][0] = 'H';
    slot[2][0] = 'D';
    slot[3][0] = 'F';
    slot[4][0] = 'B';
    slot[5][0] = 'G';
    slot[6][0] = 'C';
    slot[7][0] = 'E';
    slot[8][0] = 'A';

    /* For Llano, model number = 49, slot D is marked B, slot H is marked C */
    if ((iplcb_info->IO_planar_level_reg & 0x80000000) == 0x80000000) {
        slot[2][0] = 'B';
        slot[1][0] = 'C';
    }

    /* Initializing simm_table to all zeros */
    for (i = 1; i <= 8; ++i)
        for (j = 1; j <= 8; ++j)
            simm_table[i][j] = 0;

    count = 0;
    for (i = 1; i <= 8; ++i) {
        for (j = 1; j <= 4; ++j) {
            switch (iplcb_info->SIMM_INFO.memcd_errinfo[count]) {
                case 0x0f:{
                    if (j == 1)
                        simm_table[i][8] = 1;
                    if (j == 2)
                        simm_table[i][4] = 1;
                    if (j == 3)
                        simm_table[i][6] = 1;
                    if (j == 4)
                        simm_table[i][2] = 1;
                    break;
                }
                case 0xf0:{
                    if (j == 1)
                        simm_table[i][7] = 1;
                    if (j == 2)
                        simm_table[i][3] = 1;
                    if (j == 3)
                        simm_table[i][5] = 1;
                    if (j == 4)
                        simm_table[i][1] = 1;
                    break;
                }
                case 0xff:{
                    if (j == 1) {
                        simm_table[i][7] = 1;
                        simm_table[i][8] = 1;
                    }
                    if (j == 2) {
                        simm_table[i][3] = 1;
                        simm_table[i][4] = 1;
                    }
                    if (j == 3) {
                        simm_table[i][5] = 1;
                        simm_table[i][6] = 1;
                    }
                    if (j == 4) {
                        simm_table[i][1] = 1;
                        simm_table[i][2] = 1;
                    }
                    break;
                }
            	default:
		    break;
            } /* end switch */
            count += 1;
        }
    }

    /* Rearranging the simm locations - design change */
    /*  ---------------------------      -----------------------------
        |   1                 2   |      |   5                    1  |
        |   3                 4   |      |   6                    2  |
        |   5                 6   |  to  |   7                    3  |
        |   7                 8   |      |   8                    4  |
        ---------       -----------      ------------        ---------
                 |||||||                             ||||||||      
    */

    for (i=0; i<=8; ++i) {
        temp_table[i][1] = simm_table[i][2];
        temp_table[i][2] = simm_table[i][4];
        temp_table[i][3] = simm_table[i][6];
        temp_table[i][4] = simm_table[i][8];
        temp_table[i][5] = simm_table[i][1];
        temp_table[i][6] = simm_table[i][3];
        temp_table[i][7] = simm_table[i][5];
        temp_table[i][8] = simm_table[i][7];
    }

    /* Restoring the simm_table contents which will be used later in program */
    for (i=0; i<=8; ++i)
        for (j=0; j<=8; ++j)
            simm_table[i][j] = temp_table[i][j];

    /* Checking if an incomplete pair of cards is present, i.e. bad bit map
       and good simm table */

    badsimmcount = 0;

    /* Initializing simm_or_card array */
    for (i = 1; i <= 8; ++i)
        for (j = 1; j <= 8; ++j) {
            simm_or_card[i][j] = 0;
	    badsimmcount += simm_table[i][j];
        }

    /* Display "test or no test screen" */
    if ((tm_input.system != SYSTEM_TRUE)&&(bad_ratio != 0)&&(badsimmcount !=0))
    {
        if ((tm_input.console == CONSOLE_TRUE) && 
	    (tm_input.loopmode == LOOPMODE_NOTLM)) {
            rc = diag_display(NULL,fdes,test_or_not,DIAG_MSGONLY,NULL,&menutype,
			      meminfo);
            sprintf(temp_str, meminfo[0].text, bitcount, bad_ratio);
            free(meminfo[0].text);
            meminfo[0].text = temp_str;
            rc = diag_display(0x812000,fdes,NULL,DIAG_IO,
			      ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutype,meminfo);
            check_rc(rc);
            if (rc == ASL_COMMIT)
                switch (DIAG_ITEM_SELECTED(menutype)) {
                    case 2: break;    		/* continue executing code */
                    case 1: clean_up();	  	/*  user ignores the problem */
                            break;
                    default:{
                            /* clean_up(); */
                            break;
                    };
                }
        }
    }
    free(temp_str);

    /* Initializing tstr array to all blanks */
    for (i = 0; i < (NAMESIZE - 1); ++i)
        tstr[i] = ' ';

    /* Deciding to report card or simm failure. If 1 simm is bad, report simm.
       If 2 or 3 simms are bad, report card and simms.
       If 4 or more simms are bad, report base card only. */
    for (i = 1; i <= 8; ++i) {
        for (j = 1; j <= 8; ++j) {
            simm_or_card[i][1] += simm_table[i][j];
            if (simm_table[i][j] == 1)
                simm_or_card[i][2] = j;
        }

    #if MEMDEBUG
        if (i == 8) {
            simm_or_card[i][1] = 3;
            level = 1;
        }
        if (i == 7) {
            simm_or_card[i][1] = 1;
            simm_or_card[i][2] = 4;
        }
    #endif

        if (simm_or_card[i][1] >= 4) {
            if (level == 0) {				/* 0.9 machine */
		if (iplcb_info->cre[(i-1)*2] != (uint) 0xf0000000)
                    x = CARD_SIZE(iplcb_info->cre[(i-1)*2]) ; 
		if (iplcb_info->cre[(i*2)-1] != (uint) 0xf0000000)
		    x += CARD_SIZE(iplcb_info->cre[(i*2)-1]);
	    }
            else {					/* 1.0 machine */
                if (iplcb_info->cre[((i - 1) * 4) % 16] != (uint) 0xf0000000) 
                    x = CARD_SIZE(iplcb_info->cre[((i - 1) * 4) % 16]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+1] != (uint) 0xf0000000) 
                    x += CARD_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 1]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+2] != (uint) 0xf0000000) 
                    x += CARD_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 2]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+3] != (uint) 0xf0000000) 
                    x += CARD_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 3]) ;
                x = x / 2;
            }
	    /* end if */

    #if MEMDEBUG
            x = 8;
    #endif

            if (((get_type(slot[i][level]))==5) ||
                ((get_type(slot[i][level]))==6) ||
                ((get_type(slot[i][level]))==7) ||
                ((get_type(slot[i][level]))==8) ||
                ((get_type(slot[i][level]))==4 && x == 128)) {
	       x = x / 8; /* use simm size instead of card size to build srn */
	       temp_frub = newcard[0];
	       temp_frub.rcode += 16 * (log(x)/0.69314718 + 1);
	       /* so that 1 meg simm will have a value of 16 or 10(in hex) to be
	          added to the rcode. 2 meg will be 20(hex) and so on */
	       temp_frub.rcode += get_type(slot[i][level]);
	    }
   	    if (newfru == 0) {
                switch (x) {
                    case 8:
                    case 16:
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = card[6];
                        else
                            temp_frub = card[1];
                        break;
                    case 32:
                        if ((get_type(slot[i][level]))==0)
                            temp_frub = card[3];
                        else if ((get_type(slot[i][level]))==3)
                            temp_frub = card[5];
                        else if ((get_type(slot[i][level]))==4)
                            temp_frub = card[6];
                        else /* S1.5 base card */
                            temp_frub = card[4];
                        break;
                    case 64: 
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = card[6];
                        else /* S1.5 base card */
                            temp_frub = card[4];
                        break;
                    default:{
                            DA_SETRC_ERROR(DA_ERROR_OTHER);
                            clean_up();
                    }
                } /* end switch */
	    }

            sprintf(buffer,"parent = %s and connwhere = %c and chgstatus != 3", 
                    tm_input.parent, slot[i][level]);
            T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
            if (c_info.num == 0) {
            }
            else 
                strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                        sizeof(temp_frub.frus[0].fname));

            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
            addfrub(&temp_frub);
            DA_SETRC_STATUS(DA_STATUS_BAD);
        }
	else if (simm_or_card[i][1] == 1) {		/* 1 simm bad in slot */
            if (level == 0) {				/* 0.9 machine */
		if (iplcb_info->cre[(i-1)*2] != (uint) 0xf0000000)
                    x = SIMM_SIZE(iplcb_info->cre[(i-1)*2]) ; 
		if (iplcb_info->cre[(i*2)-1] != (uint) 0xf0000000)
		    x += SIMM_SIZE(iplcb_info->cre[(i*2)-1]);
	    }
            else {					/* 1.0 machine */
                if (iplcb_info->cre[((i - 1) * 4) % 16] != (uint) 0xf0000000) 
                    x = SIMM_SIZE(iplcb_info->cre[((i - 1) * 4) % 16]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+1] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 1]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+2] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 2]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+3] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 3]) ;
                x = x / 2;
            }
	    /* end if */

    #if MEMDEBUG
            x = 2;
    #endif

            if (((get_type(slot[i][level]))==5) ||
                ((get_type(slot[i][level]))==6) ||
                ((get_type(slot[i][level]))==7) ||
                ((get_type(slot[i][level]))==8) ||
                ((get_type(slot[i][level]))==4 && x==16)) {
	       temp_frub = newsimm[0];
	       temp_frub.rcode += 16 * (log(x)/0.69314718 + 1);
	       /* so that 1 meg simm will have a value of 16 or 10(in hex) to be
	          added to the rcode. 2 meg will be 20(hex) and so on */
	       temp_frub.rcode += get_type(slot[i][level]);
	    }

	    if (newfru == 0) {
                switch (x) {
                    case 1:
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = simm[9];
                        else /*S1*/
                            temp_frub = simm[1];
                        break;
                    case 2:
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = simm[10];
                        else /*S1*/
                            temp_frub = simm[2];
                        break;
                    case 4:
                        if ((get_type(slot[i][level]))==0)
                            temp_frub = simm[3];
                        else if ((get_type(slot[i][level]))==2)/*S15*/
                            temp_frub = simm[5];
                        else if ((get_type(slot[i][level]))==3)/*S25*/
                            temp_frub = simm[8];
                        else /* S3 */
                            temp_frub = simm[6];
                        break;
                    case 8:
                        if ((get_type(slot[i][level]))==2)  /*S15*/
                            temp_frub = simm[4];
                        else /* S3 */
                            temp_frub = simm[7];
                        break;
                    default:{
                            DA_SETRC_ERROR(DA_ERROR_OTHER);
                            clean_up();
                    }
                } /* end switch */
	    }
            sprintf(temp_frub.frus[0].floc, "00-0%c-00-0%d", slot[i][level],
                    simm_or_card[i][2]);
            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
            addfrub(&temp_frub);
            DA_SETRC_STATUS(DA_STATUS_BAD);
        } /* end 1 simm bad */

	else if (simm_or_card[i][1] == 2) {		/* 2 simms bad in slot */
            if (level == 0) {				/* 0.9 machine */
		if (iplcb_info->cre[(i-1)*2] != (uint) 0xf0000000)
                    x = SIMM_SIZE(iplcb_info->cre[(i-1)*2]) ; 
		if (iplcb_info->cre[(i*2)-1] != (uint) 0xf0000000)
		    x += SIMM_SIZE(iplcb_info->cre[(i*2)-1]);
	    }
            else {					/* 1.0 machine */
                if (iplcb_info->cre[((i - 1) * 4) % 16] != (uint) 0xf0000000) 
                    x = SIMM_SIZE(iplcb_info->cre[((i - 1) * 4) % 16]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+1] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 1]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+2] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 2]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+3] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 3]) ;
                x = x / 2;
            }
	    /* end if */

            if (((get_type(slot[i][level]))==5) ||
                ((get_type(slot[i][level]))==6) ||
                ((get_type(slot[i][level]))==7) ||
                ((get_type(slot[i][level]))==8) ||
                ((get_type(slot[i][level]))==4 && x==16)) { /* this is for S3 128meg card */
	       temp_frub = newcard_2simms[0];
	       temp_frub.rcode += 16 * (log(x)/0.69314718 + 1);
	       /* so that 1 meg simm will have a value of 16 or 10(in hex) to be
	          added to the rcode. 2 meg will be 20(hex) and so on */
	       temp_frub.rcode += get_type(slot[i][level]);
	    }

 	    if (newfru == 0) {
                switch (x) {
                    case 1:
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = card_2simms[8];
                        else /*S1*/
                            temp_frub = card_2simms[0];
                        break;
                    case 2:
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = card_2simms[9];
                        else /*S1*/
                            temp_frub = card_2simms[1];
                        break;
                    case 4:
                        if ((get_type(slot[i][level]))==0)
                            temp_frub = card_2simms[2];
                        else if ((get_type(slot[i][level]))==2)/*S15*/
                            temp_frub = card_2simms[4];
                        else if ((get_type(slot[i][level]))==3)/*S25*/
                            temp_frub = card_2simms[7];
                        else /* S3 */
                            temp_frub = card_2simms[6];
                        break;
                    case 8:
                        if ((get_type(slot[i][level]))==2)  /*S15*/
                            temp_frub = card_2simms[3];
                        else /* S3 */
                            temp_frub = card_2simms[5];
                        break;
                    default:{
                            DA_SETRC_ERROR(DA_ERROR_OTHER);
                            clean_up();
                    }
                } /* end switch */
	    }

            sprintf(buffer,"parent = %s and connwhere = %c and chgstatus != 3", 
                    tm_input.parent, slot[i][level]);
            T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
            if (c_info.num == 0) {
            }
            else 
                strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                        sizeof(temp_frub.frus[0].fname));

            jcount = 1;
            for (jj=1; jj<=8; ++jj)
                if (simm_table[i][jj] == 1) {
                    sprintf(temp_frub.frus[jcount].floc, "00-0%c-00-0%d", 
                            slot[i][level], jj);
                    jcount += 1;
                }
            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
            addfrub(&temp_frub);
            DA_SETRC_STATUS(DA_STATUS_BAD);
        } /* end 2 bad simms */

	else if (simm_or_card[i][1] == 3) {		/* 3 simms bad in slot */
            if (level == 0) {				/* 0.9 machine */
		if (iplcb_info->cre[(i-1)*2] != (uint) 0xf0000000)
                    x = SIMM_SIZE(iplcb_info->cre[(i-1)*2]) ; 
		if (iplcb_info->cre[(i*2)-1] != (uint) 0xf0000000)
		    x += SIMM_SIZE(iplcb_info->cre[(i*2)-1]);
	    }
            else {					/* 1.0 machine */
                if (iplcb_info->cre[((i - 1) * 4) % 16] != (uint) 0xf0000000) 
                    x = SIMM_SIZE(iplcb_info->cre[((i - 1) * 4) % 16]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+1] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 1]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+2] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 2]) ;
                if (iplcb_info->cre[(((i - 1) * 4) % 16)+3] != (uint) 0xf0000000) 
                    x += SIMM_SIZE(iplcb_info->cre[(((i - 1) * 4) % 16) + 3]) ;
                x = x / 2;
            }
	    /* end if */

            if (((get_type(slot[i][level]))==5) ||
                ((get_type(slot[i][level]))==6) ||
                ((get_type(slot[i][level]))==7) ||
                ((get_type(slot[i][level]))==8) ||
                ((get_type(slot[i][level]))==4 && x==16)) { /* this is for S3 128meg card */
	       temp_frub = newcard_3simms[0];
	       temp_frub.rcode += 16 * (log(x)/0.69314718 + 1);
	       /* so that 1 meg simm will have a value of 16 or 10(in hex) to be
	          added to the rcode. 2 meg will be 20(hex) and so on */
	       temp_frub.rcode += get_type(slot[i][level]);
	    }

	    if (newfru == 0) {
                switch (x) {
                    case 1:
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = card_3simms[8];
                        else if ((get_type(slot[i][level]))==2)/*S15*/
                            temp_frub = card_3simms[0];
                        break;
                    case 2:
                        if ((get_type(slot[i][level]))==4)
                            temp_frub = card_3simms[9];
                        else if ((get_type(slot[i][level]))==2)/*S15*/
                            temp_frub = card_3simms[1];
                        break;
                    case 4:
                        if ((get_type(slot[i][level]))==0)
                            temp_frub = card_3simms[2];
                        else if ((get_type(slot[i][level]))==2)/*S15*/
                            temp_frub = card_3simms[4];
                        else if ((get_type(slot[i][level]))==3)/*S25*/
                            temp_frub = card_3simms[7];
                        else /* S3 */
                            temp_frub = card_3simms[6];
                        break;
                    case 8:
                        if ((get_type(slot[i][level]))==2)  /*S15*/
                            temp_frub = card_3simms[3];
                        else /* S3 */
                            temp_frub = card_3simms[5];
                        break;
                    default:{
                            DA_SETRC_ERROR(DA_ERROR_OTHER);
                            clean_up();
                    }
                } /* end switch */
	    }

            sprintf(buffer,"parent = %s and connwhere = %c and chgstatus != 3", 
                    tm_input.parent, slot[i][level]);
            T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
            if (c_info.num == 0) {
            }
            else 
                strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                        sizeof(temp_frub.frus[0].fname));

            jcount = 1;
            for (jj=1; jj<=8; ++jj)
                if (simm_table[i][jj] == 1) {
                    sprintf(temp_frub.frus[jcount].floc, "00-0%c-00-0%d", 
                            slot[i][level], jj);
                    jcount += 1;
                }
            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
            addfrub(&temp_frub);
            DA_SETRC_STATUS(DA_STATUS_BAD);
        } /* end 3 bad simms */

	else {    /* no failure detected */
        }
    } /* end for "i" loop */

    clean_up();

}   /* main end */

/*
* NAME: check_rc
*                                                                    
* FUNCTION: Checks if user has entered the Esc or Cancel key.
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: Returns the input parameter to this routine - the user's input
*/

int 
check_rc(rc)
    int		rc;			/* User's input to the screen */
{
    if (rc == ASL_CANCEL) {
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_QUIT);
        DA_SETRC_TESTS(DA_TEST_SHR);
        DA_EXIT();
    }
    if (rc == ASL_EXIT) {
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_EXIT);
        DA_SETRC_TESTS(DA_TEST_SHR);
        DA_EXIT();
    }
    return (rc);
}

/*
* NAME: check_type
*                                                                    
* FUNCTION: Checks the type of a card or simm for the new uniform memory structure
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: a value that indicates a particular type
*/

check_type(type_val)
{

if ((type_val>=0x10)&&(type_val<=0x1f))
	return (3); /*S2*/

if ((type_val>=0x20)&&(type_val<=0x2f))
	return (4); /*S3*/

if ((type_val>=0x30)&&(type_val<=0x3f))
	return (5); /*S4*/

if ((type_val>=0x40)&&(type_val<=0x4f))
	return (6); /*S5*/

if ((type_val>=0x50)&&(type_val<=0x5f))
	return (7); /*S6*/

if ((type_val>=0x60)&&(type_val<=0x6f))
	return (8); /*S7*/

}

/*
* NAME: get_type
*                                                                    
* FUNCTION: Checks the type (e.g. S1, S1.5, U1 etc) of a simm (memory card)
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: the string in the "type" field in the attribute.
*/

get_type(mem_slot)
char mem_slot;
{
    int  how_many;

    if (tm_input.dmode == DMODE_MS1)
        sprintf(buffer,"parent=%s and connwhere=%c and chgstatus=3", 
                tm_input.parent, mem_slot);
    else
        sprintf(buffer,"parent=%s and connwhere=%c and chgstatus != 3", 
                tm_input.parent, mem_slot);
    T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
    cuat_type = (struct CuAt *)getattr(T_CuDv->name, "type", FALSE, &how_many);
    cuat_cardec = (struct CuAt *)getattr(T_CuDv->name, "cardec", FALSE, &how_many);
    cuat_ssize = (struct CuAt *)getattr(T_CuDv->name, "size", FALSE, &how_many);

    if  ( (strcmp(cuat_cardec->value,"0x20")==0)
        ||(strcmp(cuat_cardec->value,"0x21")==0)
        ||(strcmp(cuat_cardec->value,"0x22")==0)
        ||(strcmp(cuat_cardec->value,"0x23")==0)
        ||(strcmp(cuat_cardec->value,"0x24")==0)
        ||(strcmp(cuat_cardec->value,"0x25")==0)
        ||(strcmp(cuat_cardec->value,"0x26")==0)
        ||(strcmp(cuat_cardec->value,"0x27")==0)
        ||(strcmp(cuat_cardec->value,"0x28")==0)
        ||(strcmp(cuat_cardec->value,"0x29")==0)
        ||(strcmp(cuat_cardec->value,"0x2a")==0)
        ||(strcmp(cuat_cardec->value,"0x2b")==0)
        ||(strcmp(cuat_cardec->value,"0x2c")==0)
        ||(strcmp(cuat_cardec->value,"0x2d")==0)
        ||(strcmp(cuat_cardec->value,"0x2e")==0)
        ||(strcmp(cuat_cardec->value,"0x2f")==0) ) {
	    if (strcmp(cuat_ssize->value,MEG_128) == 0) /* special case to include s3 128 */
   	        newfru = 1;
	    return(4); /*S3*/
	    }
    else if  ( (strcmp(cuat_cardec->value,"0x30")==0)
        ||(strcmp(cuat_cardec->value,"0x31")==0)
        ||(strcmp(cuat_cardec->value,"0x32")==0)
        ||(strcmp(cuat_cardec->value,"0x33")==0)
        ||(strcmp(cuat_cardec->value,"0x34")==0)
        ||(strcmp(cuat_cardec->value,"0x35")==0)
        ||(strcmp(cuat_cardec->value,"0x36")==0)
        ||(strcmp(cuat_cardec->value,"0x37")==0)
        ||(strcmp(cuat_cardec->value,"0x38")==0)
        ||(strcmp(cuat_cardec->value,"0x39")==0)
        ||(strcmp(cuat_cardec->value,"0x3a")==0)
        ||(strcmp(cuat_cardec->value,"0x3b")==0)
        ||(strcmp(cuat_cardec->value,"0x3c")==0)
        ||(strcmp(cuat_cardec->value,"0x3d")==0)
        ||(strcmp(cuat_cardec->value,"0x3e")==0)
        ||(strcmp(cuat_cardec->value,"0x3f")==0) ) {
	    newfru = 1;
	    return(5); /*s4*/
	}
    else if  ( (strcmp(cuat_cardec->value,"0x40")==0)
        ||(strcmp(cuat_cardec->value,"0x41")==0)
        ||(strcmp(cuat_cardec->value,"0x42")==0)
        ||(strcmp(cuat_cardec->value,"0x43")==0)
        ||(strcmp(cuat_cardec->value,"0x44")==0)
        ||(strcmp(cuat_cardec->value,"0x45")==0)
        ||(strcmp(cuat_cardec->value,"0x46")==0)
        ||(strcmp(cuat_cardec->value,"0x47")==0)
        ||(strcmp(cuat_cardec->value,"0x48")==0)
        ||(strcmp(cuat_cardec->value,"0x49")==0)
        ||(strcmp(cuat_cardec->value,"0x4a")==0)
        ||(strcmp(cuat_cardec->value,"0x4b")==0)
        ||(strcmp(cuat_cardec->value,"0x4c")==0)
        ||(strcmp(cuat_cardec->value,"0x4d")==0)
        ||(strcmp(cuat_cardec->value,"0x4e")==0)
        ||(strcmp(cuat_cardec->value,"0x4f")==0) ) {
	    newfru = 1;
	    return(6); /*s5*/
        }
    else if  ( (strcmp(cuat_cardec->value,"0x50")==0)
        ||(strcmp(cuat_cardec->value,"0x51")==0)
        ||(strcmp(cuat_cardec->value,"0x52")==0)
        ||(strcmp(cuat_cardec->value,"0x53")==0)
        ||(strcmp(cuat_cardec->value,"0x54")==0)
        ||(strcmp(cuat_cardec->value,"0x55")==0)
        ||(strcmp(cuat_cardec->value,"0x56")==0)
        ||(strcmp(cuat_cardec->value,"0x57")==0)
        ||(strcmp(cuat_cardec->value,"0x58")==0)
        ||(strcmp(cuat_cardec->value,"0x59")==0)
        ||(strcmp(cuat_cardec->value,"0x5a")==0)
        ||(strcmp(cuat_cardec->value,"0x5b")==0)
        ||(strcmp(cuat_cardec->value,"0x5c")==0)
        ||(strcmp(cuat_cardec->value,"0x5d")==0)
        ||(strcmp(cuat_cardec->value,"0x5e")==0)
        ||(strcmp(cuat_cardec->value,"0x5f")==0) ) {
	    newfru = 1;
	    return(7); /*s6*/
        }
    else if  ( (strcmp(cuat_cardec->value,"0x60")==0)
        ||(strcmp(cuat_cardec->value,"0x61")==0)
        ||(strcmp(cuat_cardec->value,"0x62")==0)
        ||(strcmp(cuat_cardec->value,"0x63")==0)
        ||(strcmp(cuat_cardec->value,"0x64")==0)
        ||(strcmp(cuat_cardec->value,"0x65")==0)
        ||(strcmp(cuat_cardec->value,"0x66")==0)
        ||(strcmp(cuat_cardec->value,"0x67")==0)
        ||(strcmp(cuat_cardec->value,"0x68")==0)
        ||(strcmp(cuat_cardec->value,"0x69")==0)
        ||(strcmp(cuat_cardec->value,"0x6a")==0)
        ||(strcmp(cuat_cardec->value,"0x6b")==0)
        ||(strcmp(cuat_cardec->value,"0x6c")==0)
        ||(strcmp(cuat_cardec->value,"0x6d")==0)
        ||(strcmp(cuat_cardec->value,"0x6e")==0)
        ||(strcmp(cuat_cardec->value,"0x6f")==0) ) {
	    newfru = 1;
	    return(8); /*s7*/
        }
    else if  ( (strcmp(cuat_cardec->value,"0x10")==0)
        ||(strcmp(cuat_cardec->value,"0x11")==0)
        ||(strcmp(cuat_cardec->value,"0x12")==0)
        ||(strcmp(cuat_cardec->value,"0x13")==0)
        ||(strcmp(cuat_cardec->value,"0x14")==0)
        ||(strcmp(cuat_cardec->value,"0x15")==0)
        ||(strcmp(cuat_cardec->value,"0x16")==0)
        ||(strcmp(cuat_cardec->value,"0x17")==0)
        ||(strcmp(cuat_cardec->value,"0x18")==0)
        ||(strcmp(cuat_cardec->value,"0x19")==0)
        ||(strcmp(cuat_cardec->value,"0x1a")==0)
        ||(strcmp(cuat_cardec->value,"0x1b")==0)
        ||(strcmp(cuat_cardec->value,"0x1c")==0)
        ||(strcmp(cuat_cardec->value,"0x1d")==0)
        ||(strcmp(cuat_cardec->value,"0x1e")==0)
        ||(strcmp(cuat_cardec->value,"0x1f")==0) )
	    return(3); /*s2.5*/
    else if (((strcmp(cuat_cardec->value,"0x0")==0)
            ||(strcmp(cuat_cardec->value,"0x1")==0)
            ||(strcmp(cuat_cardec->value,"0x2")==0)
            ||(strcmp(cuat_cardec->value,"0x3")==0)
            ||(strcmp(cuat_cardec->value,"0x4")==0)
            ||(strcmp(cuat_cardec->value,"0x5")==0)
            ||(strcmp(cuat_cardec->value,"0x6")==0)
            ||(strcmp(cuat_cardec->value,"0x7")==0)
            ||(strcmp(cuat_cardec->value,"0x8")==0)
            ||(strcmp(cuat_cardec->value,"0x9")==0)
            ||(strcmp(cuat_cardec->value,"0xa")==0)
            ||(strcmp(cuat_cardec->value,"0xb")==0)
            ||(strcmp(cuat_cardec->value,"0xc")==0)
            ||(strcmp(cuat_cardec->value,"0xd")==0)
            ||(strcmp(cuat_cardec->value,"0xe")==0)
            ||(strcmp(cuat_cardec->value,"0xf")==0))
	    &&(strcmp(cuat_type->value,"0x1c")==0) ) /* U1 */
	        return(0); /*u1*/
    else /* S1.5 */
	        return(2);
    
}

/*
* NAME: check_missing
*                                                                    
* FUNCTION: Checks if diag. controller pass missing card to this da.
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: NONE
*/

int
check_missing()
{
    char sslot;
    int how_many;
    int si_size;
    int i,model;

    if (tm_input.dmode == DMODE_MS1) {

        sslot = tm_input.dnameloc[4];
        sprintf (buffer,"parent=%s and connwhere=%c and chgstatus=3", tm_input.parent,sslot);
        T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
	if (T_CuDv == (struct CuDv *)NULL) return(0);

	cuat_size = (struct CuAt *)getattr(T_CuDv->name, "size", FALSE, &how_many);

	if (IsPowerPC_SMP(get_cpu_model(&model))) {
            char fruname[NAMESIZE];
            i = get_board_type(sslot, fruname);
            temp_frub = simm4_pegasus[i];
            strncpy(temp_frub.frus[0].fname, fruname, NAMESIZE);
            strncpy(temp_frub.dname,tm_input.dname,sizeof(temp_frub.dname));
	    addfrub(&temp_frub);
            DA_SETRC_STATUS(DA_STATUS_BAD);
            clean_up();
	}	/* check missing in PEGASUS		*/

	if ((salmon()==1) || (salmon()==2)) {

	    if (strcmp(cuat_size->value,MEG_1) == 0) /* MEG_1,2,4 are for salmon only */
	        si_size = 1;
	    if (strcmp(cuat_size->value,MEG_2) == 0)
	        si_size = 2;
	    if (strcmp(cuat_size->value,MEG_4) == 0)
	        si_size = 4;
	    if (strcmp(cuat_size->value,MEG_8) == 0) 
	        si_size = 8;
	    if (strcmp(cuat_size->value,MEG_16) == 0)
	        si_size = 16;
	    if (strcmp(cuat_size->value,MEG_32) == 0)
	        si_size = 32;
	    if (strcmp(cuat_size->value,MEG_64) == 0)
	        si_size = 64;
	    if (strcmp(cuat_size->value,MEG_128) == 0)
	        si_size = 128;
	    if (strcmp(cuat_size->value,MEG_256) == 0)
	        si_size = 256;

	    temp_frub = sal_simm[0];
	    temp_frub.rcode += si_size;

	    sprintf(temp_frub.frus[0].floc,"00-0%c",sslot);
            strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                    sizeof(temp_frub.frus[0].fname));
            strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));

	    addfrub(&temp_frub);
	    DA_SETRC_STATUS(DA_STATUS_BAD);
	    clean_up();
	}

        if (((get_type(sslot))==5) ||
            ((get_type(sslot))==6) ||
            ((get_type(sslot))==7) ||
            ((get_type(sslot))==8) ||
            ((get_type(sslot))==4 && (strcmp(cuat_size->value,MEG_128) == 0))) {

	    if (strcmp(cuat_size->value,MEG_8) == 0)
		si_size = 16; /* 8 meg card means 1 meg simm, use same srn number 
				 calculation, 1x16=16 */
	    if (strcmp(cuat_size->value,MEG_16) == 0)
	        si_size = 32;
	    if (strcmp(cuat_size->value,MEG_32) == 0)
		si_size = 48;
	    if (strcmp(cuat_size->value,MEG_64) == 0)
	        si_size = 64;
	    if (strcmp(cuat_size->value,MEG_128) == 0)
		si_size = 80;
	    if (strcmp(cuat_size->value,MEG_256) == 0)
	        si_size = 96;
	    if (strcmp(cuat_size->value,MEG_512) == 0)
	        si_size = 112;
	    if (strcmp(cuat_size->value,MEG_1024) == 0)
	        si_size = 128;
	    temp_frub = newcard[0];
	    temp_frub.rcode += get_type(sslot);
	    temp_frub.rcode += si_size;
	    }

	if (newfru == 0) {
            if (strcmp(cuat_size->value,MEG_8) == 0) {
                if ((get_type(sslot))==4)
                    temp_frub = card[6];
                else /* S1.5 base card */
                    temp_frub = card[1];
            }
            else if (strcmp(cuat_size->value,MEG_16) == 0) {
                if ((get_type(sslot))==4)
                    temp_frub = card[6];
                else /* S1.5 base card */
                    temp_frub = card[2];
            }
            else if (strcmp(cuat_size->value,MEG_32) == 0) {
                if ((get_type(sslot))==0)
                    temp_frub = card[3];
                else if ((get_type(sslot))==3)
                    temp_frub = card[5];
                else if ((get_type(sslot))==4)
                    temp_frub = card[6];
                else /* S1.5 base card */
                    temp_frub = card[4];
            }
            else if (strcmp(cuat_size->value,MEG_64) == 0) {
                if ((get_type(sslot))==4)
                    temp_frub = card[6];
                else /* S1.5 base card */
                    temp_frub = card[4];
            }
            else
                return(1);
	}

        strncpy (temp_frub.frus[0].fname,T_CuDv[0].name,
                 sizeof(temp_frub.frus[0].fname));
        strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
        addfrub(&temp_frub);
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
    }
    else
        return(0);
}

/*
* NAME: check_missL2
*                                                                    
* FUNCTION: Checks if diag. controller pass missing L2 cache to this da.
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: NONE
*/

int
check_missL2()
{

    if (tm_input.dmode == DMODE_MS1)
    {
	sprintf (buffer1,"name=%s and chgstatus=3", tm_input.dname);
	L_CuDv = get_CuDv_list(CuDv_CLASS,buffer1,&c_info,1,1);
	if (L_CuDv != (struct CuDv *)NULL) {
	    temp_frub = l2error[0];
            strncpy(temp_frub.dname,tm_input.dname,sizeof(temp_frub.dname));
	    temp_frub.sn=0x811;
	    addfrub(&temp_frub);
            DA_SETRC_STATUS(DA_STATUS_BAD);
            clean_up();
	}
    }
    else
        return(0);
}




/*
* NAME: clean_up
*                                                                    
* FUNCTION: Calls check_missing and get ready to quit the application.
*                                                                    
* EXECUTION ENVIRONMENT:
*	This routine is called by the main program.
*
* RETURNS: NONE
*/

clean_up()

{
    catclose(fdes);
    if (fd > 0) 
        close(fd);
    term_dgodm();
    if (tm_input.console == CONSOLE_TRUE)
        diag_asl_quit();
#if DEBUG
fflush(membug);
fclose(membug);
#endif
free(iplcb_mem_data);
    DA_EXIT();

}

int
check_newmemory() /* checking for new uniform memory configuration - RS2 for now */
{
int i,j,k, holdval, mem_offset,x;

holdval = iplcb_mem_data->num_of_structs;
mem_offset = iplcb_dir->mem_data_offset;
for (k=0; k<holdval; ++k) {
    if (iplcb_mem_data->state == IS_BAD) {
        if (iplcb_mem_data->card_or_simm_indicator == CARD) /*something is bad*/
        {
	switch (iplcb_mem_data->num_of_bad_simms) {
	    case 1 :  /* 1 simm bad */
	        temp_frub = newsimm[0];
	        sprintf(temp_frub.frus[0].floc,"00-0%c-00-0%c",
			iplcb_mem_data->location[0][3],iplcb_mem_data->location[1][3]);
		break;
	    case 2 :  /* Base card and 2 simms bad*/
	        temp_frub = newcard_2simms[0];
	        sprintf(temp_frub.frus[0].floc,"00-0%c",iplcb_mem_data->location[0][3]);
	        sprintf(temp_frub.frus[1].floc,"00-0%c-00-0%c",
			iplcb_mem_data->location[0][3],iplcb_mem_data->location[1][3]);
	        sprintf(temp_frub.frus[2].floc,"00-0%c-00-0%c",
			iplcb_mem_data->location[0][3],iplcb_mem_data->location[2][3]);
		break;
	    case 3 :  /* Base card and 3 simms bad */
	        temp_frub = newcard_3simms[0];
	        sprintf(temp_frub.frus[0].floc,"00-0%c",iplcb_mem_data->location[0][3]);
	        sprintf(temp_frub.frus[1].floc,"00-0%c-00-0%c",
			iplcb_mem_data->location[0][3],iplcb_mem_data->location[1][3]);
	        sprintf(temp_frub.frus[2].floc,"00-0%c-00-0%c",
			iplcb_mem_data->location[0][3],iplcb_mem_data->location[2][3]);
	        sprintf(temp_frub.frus[3].floc,"00-0%c-00-0%c",
			iplcb_mem_data->location[0][3],iplcb_mem_data->location[3][3]);
		break;
	    case 0 :  /* Base card */ /* 0 means more than 4 are bad - number reset */
	    case 4 :  
	    case 5 :  
	    case 6 :  
	    case 7 :  
	    case 8 :  
	    default:
	        temp_frub = newcard[0];
	        sprintf(temp_frub.frus[0].floc,"00-0%c",iplcb_mem_data->location[0][3]);
		break;
	}
    }
    temp_frub.rcode += check_type(iplcb_mem_data->EC_level);
    x = iplcb_mem_data->card_or_SIMM_size / 8;
    temp_frub.rcode += 16 * (log(x) / 0.69314718 + 1 );
    strncpy(temp_frub.dname,tm_input.dname,sizeof(temp_frub.dname));
    temp_frub.sn=0x940;

    /* Add fru bucket */
    insert_frub(&tm_input, &temp_frub);
    temp_frub.sn=0x940;
    addfrub(&temp_frub);
    DA_SETRC_STATUS(DA_STATUS_BAD);

#if DEBUG
tbuf = (char *) malloc(4096);
diag_asl_msg("k = %d\n",k);
diag_asl_msg("iplcb pointer = %d\n",iplcb_mem_data);
    sprintf(tbuf,"num of structs = %d\n", iplcb_mem_data->num_of_structs);
    fprintf(membug,tbuf);
    fflush(membug);
    sprintf(tbuf,"struct size = %d\n", iplcb_mem_data->struct_size);
    fprintf(membug,tbuf);
    fflush(membug);
    sprintf(tbuf,"card state = %d\n", iplcb_mem_data->state);
    fprintf(membug,tbuf);
    fflush(membug);
    sprintf(tbuf,"num of bad simms = %d\n", iplcb_mem_data->num_of_bad_simms);
    fprintf(membug,tbuf);
    fflush(membug);
    sprintf(tbuf,"entry indicator = %d\n", iplcb_mem_data->card_or_simm_indicator);
    fprintf(membug,tbuf);
    fflush(membug);
    sprintf(tbuf,"card or simm size= %d\n", iplcb_mem_data->card_or_SIMM_size);
    fprintf(membug,tbuf);
    fflush(membug);
    sprintf(tbuf,"EC level in hex = %x\n", iplcb_mem_data->EC_level);
    fprintf(membug,tbuf);
    fflush(membug);
    sprintf(tbuf,"PD bits in hex = %x\n", iplcb_mem_data->PD_bits);
    fprintf(membug,tbuf);
    fflush(membug);
    
    for (i=0; i<5; ++i){
        for (j=0; j<4; ++j) {
    	sprintf(tbuf,"location [%d] [%d] = %x\n", i, j, iplcb_mem_data->location[i][j]);
    	fprintf(membug,tbuf);
    	fflush(membug);
        }
        sprintf(tbuf,"\n\n");
        fprintf(membug,tbuf);
        fflush(membug);
    }
#endif


    }

    mem_offset += iplcb_mem_data->struct_size;

/* uniform memory configuration table */
    p.md_incr = MV_BYTE;
    p.md_addr = mem_offset;
    p.md_data = (char *) iplcb_mem_data;
    p.md_size = sizeof(*iplcb_mem_data); 
    if (ioctl(fd, MIOIPLCB, &p)) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

#if DEBUG
sprintf(tbuf,"\n\n");
fprintf(membug,tbuf);
fflush(membug);
free(tbuf);
#endif
}

clean_up();
	
}

int
check_powerpc()
{

#if DEBUG
char *tbuf;
#endif
int i, sum, x;


	sum = 0;
	for (i=0; i<8; ++i) {

#if DEBUG
diag_asl_msg("In check_powerpc routine");
diag_asl_msg("bad simm report [%d] = %d\n",i,iplcb_ram46_data->bad_simm_report[i]);
diag_asl_msg("simm size [%d] = %d\n",i,iplcb_ram46_data->simm_size[i]);
diag_asl_msg("sum = %d\n", sum);
#endif

	    if (iplcb_ram46_data->bad_simm_report[i] != 0)
		sum += 1;
	}

	if (sum > 2) {
	    /* Call up system planar 100% bad */
	    insert_frub(&tm_input, &planar[0]);
            strncpy(planar[0].dname,tm_input.dname, sizeof(planar[0].dname));
	    addfrub(&planar[0]);
	    DA_SETRC_STATUS(DA_STATUS_BAD);
	    clean_up();
	}

	for (i=0; i<8; ++i) {
#if DEBUG
tbuf = (char *) malloc(1024);
sprintf(tbuf,"bad simm report [%d] = %d\n",i,iplcb_ram46_data->bad_simm_report[i]);
diag_asl_msg(tbuf);
sprintf(tbuf,"simm size [%d] = %d\n",i,iplcb_ram46_data->simm_size[i]);
diag_asl_msg(tbuf);
free(tbuf);
iplcb_ram46_data->bad_simm_report[2] = 1;
#endif
	    if (iplcb_ram46_data->bad_simm_report[i] != 0) {
		s_slot = sal_slot_table[i];

		temp_frub = sal_simm[0];
		temp_frub.rcode += iplcb_ram46_data->simm_size[i];

		sprintf(temp_frub.frus[0].floc,"00-0%c",s_slot);
                sprintf(buffer,"parent = %s and connwhere = %c and chgstatus != 3", 
                        tm_input.parent, s_slot);
                T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
                strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                        sizeof(temp_frub.frus[0].fname));
                strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));

		addfrub(&temp_frub);
		DA_SETRC_STATUS(DA_STATUS_BAD);
            }
	}
	clean_up();
}

int
check_l2cache()
{
int i;

/*
diag_asl_msg("L2 cache size = %d\n",iplcb_proc_data->L2_cache_size);
diag_asl_msg("In routine\n");
iplcb_proc_data->L2_cache_size = 1;
iplcb_l2_data->type[0]='R';
iplcb_l2_data->location[0][0] = '0';
iplcb_l2_data->location[0][1] = '0';
iplcb_l2_data->location[0][2] = '0';
iplcb_l2_data->location[0][3] = 'R';
*/


        for (i=0; i<16; ++i) {
	    if (iplcb_l2_data->type[i] == '0') 
		return(0);
	    else
	    {
	        if (iplcb_l2_data->type[i] == 'P') {
	    	    /* Call up system planar 100% bad */
	    	    insert_frub(&tm_input, &planar[0]);
            	    strncpy(planar[0].dname,tm_input.dname, sizeof(planar[0].dname));
	    	    addfrub(&planar[0]);
	    	    DA_SETRC_STATUS(DA_STATUS_BAD);
		    return(-1);
		}
		else if (iplcb_l2_data->type[i] == 'R') {
		    temp_frub = riser[0];
	    	    /* Call up riser card 100% bad */
                    strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
                    addfrub(&temp_frub);
                    DA_SETRC_STATUS(DA_STATUS_BAD);
		    return(-1);
		}
		else if (iplcb_l2_data->type[i] == 'L') {
		    temp_frub = l2simm[0];
		    temp_frub.rcode += iplcb_l2_data->size[i]/256; /*assume smallest is 256 K*/
		    if (iplcb_l2_data->location[i][0] != ' ')
                        sprintf(temp_frub.frus[0].floc, "0%c-0%c-0%c-0%c", 
			    iplcb_l2_data->location[i][0], iplcb_l2_data->location[i][1],
			    iplcb_l2_data->location[i][2], iplcb_l2_data->location[i][3]);
                    strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));
                    addfrub(&temp_frub);
                    DA_SETRC_STATUS(DA_STATUS_BAD);
		    return(-1);
		}
	    }
        }
    return(0);
}

int
check_salmon()
{

#if DEBUG
char *tbuf;
#endif
int i, sum, x;


	sum = 0;
	for (i=0; i<8; ++i) {

#if DEBUG
diag_asl_msg("In check_salmon routine");
diag_asl_msg("bad simm report [%d] = %d\n",i,iplcb_ram_data->bad_simm_report[i]);
diag_asl_msg("simm size [%d] = %d\n",i,iplcb_ram_data->simm_size[i]);
diag_asl_msg("sum = %d\n", sum);
#endif

	    if (iplcb_ram_data->bad_simm_report[i] != 0)
		sum += 1;
	}

	if (sum > 2) {
	    /* Call up system planar 100% bad */
	    insert_frub(&tm_input, &planar[0]);
            strncpy(planar[0].dname,tm_input.dname, sizeof(planar[0].dname));
	    addfrub(&planar[0]);
	    DA_SETRC_STATUS(DA_STATUS_BAD);
	    clean_up();
	}

	for (i=0; i<8; ++i) {
#if DEBUG
tbuf = (char *) malloc(1024);
sprintf(tbuf,"bad simm report [%d] = %d\n",i,iplcb_ram_data->bad_simm_report[i]);
diag_asl_msg(tbuf);
sprintf(tbuf,"simm size [%d] = %d\n",i,iplcb_ram_data->simm_size[i]);
diag_asl_msg(tbuf);
free(tbuf);
iplcb_ram_data->bad_simm_report[2] = 1;
#endif
	    if (iplcb_ram_data->bad_simm_report[i] != 0) {
		s_slot = sal_slot_table[i];

		temp_frub = sal_simm[0];
		temp_frub.rcode += iplcb_ram_data->simm_size[i];

		sprintf(temp_frub.frus[0].floc,"00-0%c",s_slot);
                sprintf(buffer,"parent = %s and connwhere = %c and chgstatus != 3", 
                        tm_input.parent, s_slot);
                T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
                strncpy(temp_frub.frus[0].fname,T_CuDv[0].name,
                        sizeof(temp_frub.frus[0].fname));
                strncpy(temp_frub.dname,tm_input.dname, sizeof(temp_frub.dname));

		addfrub(&temp_frub);
		DA_SETRC_STATUS(DA_STATUS_BAD);
            }
	}
	clean_up();
}

int
salmon()
{
int	cuat_mod;
int	ipl_mod;
#if DEBUG
int	x;
char    *tbuf;
#endif

ipl_mod = get_cpu_model(&cuat_mod);
#if DEBUG
tbuf = (char *) malloc(1024);
sprintf(tbuf,"ipl_mod = %x\n",ipl_mod);
diag_asl_msg(tbuf);
sprintf(tbuf,"cuat_mod = %x\n",cuat_mod);
diag_asl_msg(tbuf);
x = IS_RSC(cuat_mod);
sprintf(tbuf,"x = %d\n",x);
diag_asl_msg(tbuf);
free (tbuf);
#endif
 
/* 02010041 and 02010045 - salmon (45 is in the field, 41 early internals)
   02010043        - cabeza
   08010046        - rainbow 3 - POWER PC
   02010047        - chaparral */

ipl_mod &= 0xff000000; /* masking off 3 bytes */
#if DEBUG
diag_asl_msg("ipl_mod after masking is = %x\n", ipl_mod);
#endif

if (ipl_mod == 0x02000000) 
    return(1);
else if (ipl_mod == 0x08000000)
    return(2);
else
    return(0);
}

int
rs2()
{
int	cuat_mod;
int	ipl_mod;
int	ocs_mod;
#if DEBUG
int	x;
char    *tbuf;
#endif

ipl_mod = get_cpu_model(&cuat_mod);
#if DEBUG
tbuf = (char *) malloc(1024);
sprintf(tbuf,"ipl_mod = %x\n",ipl_mod);
diag_asl_msg(tbuf);
sprintf(tbuf,"cuat_mod = %x\n",cuat_mod);
diag_asl_msg(tbuf);
x = IS_RSC(cuat_mod);
sprintf(tbuf,"x = %d\n",x);
diag_asl_msg(tbuf);
free (tbuf);
#endif

 
/* 0x04000070	     - Hawthorn
   0x04xxxx71        - Sioux
   0x04020080        - Silverbell

   If it is RS2 with no L2 cache structure, return 1
   If it is RS2 with L2 cache structure, return 2
   else return 0 */

ocs_mod = ipl_mod & 0xff0000ff; /* masking off middle 2 bytes */
ipl_mod &= 0xff000000; /* masking off last 3 bytes */

if ((ocs_mod == 0x04000070) || (ocs_mod == 0x04000071) ||
    (ocs_mod == 0x04000079) || (ocs_mod == 0x04000080))
{
    newfru = 1;
    return(1);
}
else if (ipl_mod == 0x04000000)
{
    newfru = 1;
    return(2);
}
else
{
    return(0);
}
}
	/* page	*/
/************************************************************************/
/*									*/
/* NAME: get_pegasus_defect_list					*/
/*									*/
/* FUNCTION: checks the iplcb_mem_data array.				*/
/*		if (error) then calls get_error(0)			*/
/*		if there is an error, fills the frub with MRB data	*/
/*			and calls addfrub				*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine is called by the main program.			*/
/*									*/
/* INPUT DATA: MEM_DATA structure					*/
/*									*/
/*		uint struct_size;   size of this structure (in bytes)	*/
/*		enum card_state state;					*/
/*		uint num_of_bad_simms;					*/
/*		enum entry_indicator card_or_simm_indicator;		*/
/*		uint card_or_SIMM_size;					*/
/*		uint EC_level;						*/
/*		uint PD_bits;						*/
/*		char location[5][4];					*/
/*									*/
/* OUTPUT DATA: frub structure						*/
/*	typedef struct							*/
/*									*/
/*	int	conf;			   probability of failure	*/
/*	char	fname[NAMESIZE];	   FRU name			*/
/*	char	floc[LOCSIZE];		   location of fname		*/
/*	short	fmsg; 			   text message number for fname*/
/*	char    fru_flag;		   flag used by DA		*/
/*	char	fru_exempt;						*/
/*	}fru_t;								*/
/*									*/
/*	struct	fru_bucket                                              */
/*									*/
/*	char	dname[NAMESIZE];	   device name			*/
/*	short   ftype;	   FRU bucket type added to the system		*/
/*	short	sn;	   Source number of the failure			*/
/*	short	rcode;	   reason code for the failure			*/
/*	short	rmsg;	   message 					*/
/*	fru_t 	frus[MAXFRUS];						*/
/*									*/
/*									*/
/* RETURNS: NONE							*/
/*									*/
/************************************************************************/
get_pegasus_defect_list()
{
    int	i,j;

    mem_defect_list = iplcb_mem_data;

#if MEMDEBUG
mem_defect_list->state = IS_BAD;
mem_defect_list->num_of_bad_simms = 2;
sprintf(mem_defect_list->location[1],"%s","0014");
sprintf(mem_defect_list->location[2],"%s","0002");
#endif

    for (i=0,j=mem_defect_list->num_of_structs; i<j; i++,mem_defect_list++) {
	if (mem_defect_list->state == IS_BAD) {
		get_error(mem_defect_list->num_of_bad_simms);
		addfrub(&temp_frub);
		DA_SETRC_STATUS(DA_STATUS_BAD);
	}
    }
    clean_up();
}
	/* page	*/
/************************************************************************/
/*									*/
/* NAME: get_error							*/
/*									*/
/* PARAMETERS : error count						*/
/*									*/
/* FUNCTION: for all simms or card in error				*/
/*		computes the error count 				*/
/*		assign a frub according to the error count 		*/
/*		put error location into the frub (3 at most)		*/
/*		case 1 error  : fill-in simm_pegasus			*/
/*		case 2 errors : fill-in simm2_pegasus with simm & card	*/
/*		case 3 errors : fill-in simm3_pegasus with simm & card	*/
/*		case 0 or >=4 errors : fill-in simm4_pegasus with card	*/
/*	The MRB type is stored in CuVPD, to change it into an index,	*/
/*		this type is compared to a reference MRB_name[].	*/
/*		The SIMM type is this index modulo 2 (MR2,MR4,...)	*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine is called by the get_pegasus_defect_list routine.	*/
/*									*/
/* INPUT DATA: MEM_DATA structure					*/
/*									*/
/*		uint struct_size;   size of this structure (in bytes)	*/
/*		enum card_state state;					*/
/*		uint num_of_bad_simms;					*/
/*		enum entry_indicator card_or_simm_indicator;		*/
/*		uint card_or_SIMM_size;					*/
/*		uint EC_level;						*/
/*		uint PD_bits;						*/
/*		char location[5][4] =					*/
/*			card location; 					*/
/*			simm location = [01 - 15] or nil		*/
/*			simm location = [01 - 15] or nil		*/
/*			simm location = [01 - 15] or nil		*/
/*									*/
/* OUTPUT DATA: frub structure						*/
/*									*/
/*									*/
/* RETURNS: NONE							*/
/************************************************************************/
get_error(num)
int	num;
{
    int	i,n;
    char fruname[NAMESIZE];

    n = get_board_type(mem_defect_list->location[0][3], fruname);

    switch(num)	{			/* assign the right frub	*/
	case 1 :
		temp_frub = simm1_pegasus[n];
		sprintf(temp_frub.frus[0].floc, "00-0%c-00-00",
				mem_defect_list->location[0][3]);
		strncpy(&temp_frub.frus[0].floc[9],
				&mem_defect_list->location[num][2],2);
		break;
	case 2 :
		temp_frub = simm2_pegasus[n];
                strncpy(temp_frub.frus[0].fname, fruname, NAMESIZE);
		for (i=num; i>0; i--) {
			sprintf(temp_frub.frus[i].floc,"00-0%c-00-00",
				mem_defect_list->location[0][3]);
			strncpy(&temp_frub.frus[i].floc[9],
				&mem_defect_list->location[i][2],2);
		}
		break;
	case 3 :
		temp_frub = simm3_pegasus[n];
                strncpy(temp_frub.frus[0].fname, fruname, NAMESIZE);
		for (i=num; i>0; i--) {
			sprintf(temp_frub.frus[i].floc,"00-0%c-00-00",
				mem_defect_list->location[0][3]);
			strncpy(&temp_frub.frus[i].floc[9],
				&mem_defect_list->location[i][2],2);
		}
		break;
	default:
		temp_frub = simm4_pegasus[n];
                strncpy(temp_frub.frus[0].fname, fruname, NAMESIZE);
		break;
    }
    strncpy(temp_frub.dname,tm_input.dname,sizeof(temp_frub.dname));
}
	/* page	*/
/************************************************************************/
/*									*/
/* NAME: get_bitmap_size						*/
/*									*/
/* PARAMETERS : 							*/
/*									*/
/* FUNCTION: gets the realmem value from CuAt object			*/
/*		computes the effective size of the bitmap 		*/
/*		knowing that a single states for 16k ram 		*/
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine is called from the main.				*/
/*									*/
/* INPUT DATA: CuAt object Class					*/
/*									*/
/*	char name[16];							*/
/*	char attribute[16];						*/
/*	char value[256];						*/
/*	char type[8];							*/
/*	char generic[8];						*/
/*	char rep[8];							*/
/*	short nls_index;						*/
/*									*/
/*									*/
/* RETURNS: bitmap real size (bytes)					*/
/*				 					*/
/************************************************************************/
int
get_bitmap_size()
{
    int	how_many;
    int	realmem,i;
    char mem[8] ;

    for (i=0,realmem=0; i<8; i++) {
	sprintf(mem,"mem%d",i);
	cuat_realmem = (struct CuAt *)getattr(mem, "size", FALSE, &how_many);
	if (cuat_realmem != (struct CuAt *)NULL && how_many != 0)
		realmem += atoi(cuat_realmem->value);
    }
    return(realmem*8);
}
	/* page	*/
/************************************************************************/
/*									*/
/* NAME: get_board_type							*/
/*									*/
/* PARAMETERS : board location (char)					*/
/*              buffer where memory card CuDv name                      */
/*									*/
/* FUNCTION: computes the index to use in fru_bucket tables             */
/*           and fills-in cudvname with the memory card CuDv name       */
/*									*/
/* EXECUTION ENVIRONMENT:						*/
/*	This routine is called from routines.				*/
/*									*/
/* INPUT DATA: board location as char					*/
/*									*/
/* RETURNS: index in fru_bucket tables                                  */
/*				 					*/
/************************************************************************/
int
get_board_type(location, cudvname)
char	location;
char	*cudvname;
{
    int banksize;
    int	n;
    char *board_type;

    sprintf(buffer,"parent = %s and connwhere = %c ",tm_input.parent,location);
    T_CuDv = get_CuDv_list(CuDv_CLASS,buffer,&c_info,1,1);
    if (T_CuDv == (struct CuDv *)NULL || T_CuDv == (struct CuDv *)(-1)) {
       	DA_SETRC_ERROR(DA_ERROR_OTHER);
       	DA_EXIT();
    }

    sprintf(buffer, "name = %s", T_CuDv[0].name);
    strncpy(cudvname, T_CuDv[0].name, NAMESIZE);
    odm_free_list(T_CuDv, &c_info);

    T_CuVPD = get_CuVPD_list(CuVPD_CLASS,buffer,&vpd_info,1,1);
    if (T_CuVPD == (struct CuVPD *)NULL || T_CuVPD == (struct CuVPD *)(-1)) {
       	DA_SETRC_ERROR(DA_ERROR_OTHER);
       	DA_EXIT();
    }

    board_type = strstr(T_CuVPD->vpd, "*FN");

    for (n = 0; board_type && n < sizeof(MRB_name)/sizeof(MRB_name[0]); ++n) {
	if (strncmp(MRB_name[n], board_type + 4, 3))
		continue;

	if (n == MEMCARD_MR2_8 || n == MEMCARD_MR4_32) {
		odm_free_list(T_CuVPD, &vpd_info);
		return(n);
	}

	if ((n == MEMCARD_MRE_8 || n == MEMCARD_NFx_8) &&
	    (board_type = strstr(T_CuVPD->vpd, "*DS\004BANK")) != NULL &&
	    (board_type = strstr(board_type, "*SZ")) != NULL &&
	    sscanf(board_type + 4, "%04d", &banksize) == 1) {

		odm_free_list(T_CuVPD, &vpd_info);
		switch(banksize) {
		case 32:
			return(n == MEMCARD_MRE_8 ?
				MEMCARD_MRE_8 : MEMCARD_NFx_8);
		case 64:
			return(n == MEMCARD_MRE_8 ?
				MEMCARD_MRE_16 : MEMCARD_NFx_16);
		case 128:
			return(n == MEMCARD_MRE_8 ?
				MEMCARD_MRE_32 : MEMCARD_NFx_32);
		default:
			break;
		} /* end of switch(banksize) */
	} /* end of if (n == MEMCARD_MRE_8... */
	break;
    } /* end of for (n = 0... */

    /* should never go here, except if garbage in CuVPD */
    DA_SETRC_ERROR(DA_ERROR_OTHER);
    DA_EXIT();
    /* NOT REACHED */
} /* end of get_board_type function*/
