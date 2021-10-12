static char sccsid[] = "@(#)82	1.1  src/bos/diag/tu/gem/dgsrgb.c, tu_gem, bos411, 9428A410j 5/30/91 12:48:25";
/*
 * COMPONENT_NAME: tu_gem ( dgsrgb )
 *
 * FUNCTIONS: rgbscreen 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "dgsmon.h"


struct rgbpat
{
/* Set interior style to solid:       */
   struct setintsty setsolid;
/* Display red color bar:            */
   struct setintci setred;
   struct fillrect3 barred;
/* Display green color bar:          */
   struct setintci setgreen;
   struct fillrect3 bargreen;
/* Display blue color bar:            */
   struct setintci setblue;
   struct fillrect3 barblu;
};


/*
 * NAME:
 *	rgbscreen
 *                                                                    
 * FUNCTION:
 * 	Displays the RGB screen needed for the visual tests.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:)
 *	Sets up the structures needed and does a FIFO write.
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	None.
 */  

void
rgbscreen(gmbase)
unsigned long  gmbase;
{
int  ssize;
static struct rgbpat dgsrgb =
{

/* Set interior style to solid:                                     */
   { 0x0008, SE_IS, SOLIDINT },

/* Display red color bar:                                           */
/* Set interior color index to red:                                 */
   { 0x0008, CE_INCI, REDI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 0., 682., 0., 1279.,
     682., 0., 1279., 1023., 0., 0., 1023., 0., 0., 682., 0. },

/* Display green color bar:                                         */
/* Set interior color index to green:                               */
   { 0x0008, CE_INCI, GREENI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 0., 341., 0., 1279.,
     341., 0., 1277., 681., 0., 0., 681., 0., 0., 0., 0. },

/* Display blue color bar:                                          */
/* Set interior color index to blue:                                */
   { 0x0008, CE_INCI, BLUEI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 0., 0., 0., 1279.,
     0., 0., 1277., 340., 0., 0., 340., 0., 0., 0., 0. },
};

ssize = sizeof(dgsrgb);
wfifo(0,&dgsrgb, ssize ,gmbase);
}
