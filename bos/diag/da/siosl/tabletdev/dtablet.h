/* @(#)49	1.6  src/bos/diag/da/siosl/tabletdev/dtablet.h, databletsl, bos411, 9428A410j 4/1/94 17:01:01 */
/*
 * COMPONENT_NAME: DATABLET
 *
 * FUNCTIONS:   This file contains global defines, variables, and structures
 *              for the Tablet device diagnostic application.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define	TRUE		1
#define	FALSE		0

#define ID_PEN          '1'
#define CRSR4B          '2'
#define STYLUS_INPUT_DEVICE     0x08
#define PUCK_INPUT_DEVICE       0x10
#define TABSTYLUS	0x01
#define TABPUCK		0x02 
#define NIOP            0x821
#define IOP             0x227
	
#define TU10            0x10
#define TU20            0x20
#define TU30            0x30
#define TU40            0x40
#define TU60            0x60
#define TU70            0x70
#define TUC0            0xC0

int     filedes;     /* File descriptor for device driver                 */
int     fdes;        /* File descriptor for machine device driver         */
int     f_msg;       /* pointer to loop or option menu message            */
int     tu_err = 0;
int     idev_id;
int     asl_err = 1000;
uchar   inpt_dev[16];
uchar   pen[] = "pen";
uchar   puck[] = "cursor";
short	conf10 = 70;
short	conf11 = 30;

struct  objlistinfo dinfo;
struct  CuDv *cudv;

struct fru_bucket frub[] =
{
	{ "", FRUB1, 0, 0, 0,
	        {
	                { 0, "", "", 0, 0, (char) NULL },
	                { 0, "", "", 0, 0, (char) NULL },
	        },
	},
};
