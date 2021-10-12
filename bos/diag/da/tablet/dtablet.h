/* @(#)16       1.10  src/bos/diag/da/tablet/dtablet.h, datablet, bos411, 9428A410j 5/20/94 16:02:52 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define ID_PEN          '1'
#define CRSR4B          '2'
#define STYLUS_INPUT_DEVICE	0x08
#define PUCK_INPUT_DEVICE	0x10
#define TABSTYLUS	0x01
#define TABPUCK		0x02
#define NIOP            0x821
#define IOP             0x227
	
#define TU10            0x10
#define TU20            0x20
#define TU30            0x30
#define TU40            0x40
#define TU50            0x50
#define TU60            0x60
#define TU70            0x70

#define CANCEL_KEY_ENTERED      7
#define EXIT_KEY_ENTERED        8

#define M11		0x926
#define M12		0x927

#define ATU_YES		1	/* user select yes choice */
#define ATU_NO		2	/* user select no choice  */
#define TU_FAIL		1	/* action fail */
#define TU_SUCCESS	0	/* action succeed */

int     fdes;           /* File descriptor for machine device driver	*/
int	filedes;	/* File descriptor for device driver		*/ 
int	filedes_k;	/* File descriptor for keyboard device		*/
int     f_msg;          /* pointer to loop or option menu message	*/
int     tu_err = 0;
int     idev_id;
int     asl_err = 1000;
uchar   inpt_dev[16];
uchar   pen[] = "pen";
uchar   puck[] = "cursor";

struct  objlistinfo dinfo;
struct  CuDv *cudv,*cudv_k;

struct fru_bucket frub[] =
{
	{ "", FRUB1, 0, 0, 0,
	        {
	                { 0, "", "", 0, 0, (char) NULL },
	                { 0, "", "", 0, 0, (char) NULL },
	        },
	},
};

struct tucb_d
{
	nl_catd catd;
	long	ad_mode;
	int	tabtype;
	char	kbd_fd[20];
};

