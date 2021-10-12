static char sccsid[] = "@(#)03	1.11  src/bos/usr/bin/odme/odmehft.c, cmdodm, bos411, 9428A410j 11/23/93 09:56:58";
/*
 * COMPONENT_NAME: ODME
 *
 * FUNCTIONS: ms_input
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----------------------------------------------------------------------*/
/* FILE NAME : odmehft.c         					*/
/* DESCRIPTION : This file contains the module to 			*/
/*		 read, and process mouse input. 			*/
/*									*/
/* CONTENTS :								*/
/*		int ms_input(); read, process, and return mouse input	*/
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include "odme.h"


/*----------------------------------------------------------*/
/* MS_INPUT						    */
/* 	read input, if it is from the mouse, process it	    */
/*		otherwise just return the entered key.	    */
/*----------------------------------------------------------*/
int ms_input(wnd)
WINDOW *wnd;
{
int	c;
int 	i;
int 	j;
int	rc;
int	count;

	/*-------------------------------------------------------------*/
	/* If extended curses mode is off as it is for odme, wgetch    */
	/* will return one byte. If a multibyte character is being     */
	/* entered, multiple calls to wgetch are required. The results */
	/* are stored in a char string called mbstring, which is used  */
	/* in odmewindow by waddstr.                                   */
	/*-------------------------------------------------------------*/ 
	for (count = 0; count < MB_LEN_MAX; count++)
	{
		c = (int) getch ();

		/*-----GET KEY AND CHECK IF IT IS FROM THE MOUSE-----*/

    		if (c == KEY_LOCESC) 
		{

			/*----DELTA Y----*/

			i = (short)(ESCSTR[5]<<8) | (short)(ESCSTR[6]);	

			/*-MULTIPLY THE Y MOVEMENTS BY TWO FOR A BETTER FEEL-*/

			j = 2 * ( (short)(ESCSTR[3]<<8) | (short)(ESCSTR[4]) );
	
			/*---------------------------------------------------*/
			/* if delta y is greater than delta x		     */
			/* 	determine up or down motion		     */
			/* 	else					     */
			/* 	determine left or right motion	       	     */
			/*---------------------------------------------------*/

			if ( abs(i) > abs(j) ) 
			{
				if ( i < -10 ) 
				{
					c = KEY_DOWN;
				}
	    	 		else if ( i > 10 ) 
				{
					c = KEY_UP;
	    			}
			}
		 	else 
			{
	    			if ( j < -10 ) 
				{
					c = KEY_BTAB;
				}
	    		 	else if ( j > 10 ) 
				{
					c = KEY_TAB;
	    			}
			}

			/*-----CHECK FOR BUTTON PUSHED-----*/

	    		if ((char) ESCSTR[11] != 0)
			 	c = KEY_NWL;
		}

		mbstring[count] = c;
		rc = mblen (mbstring, count + 1);
		/*---------------------------------------------------*/
		/* If mblen returns a positive number then mbstring  */
		/* contains a valid multibyte character. A '\0'      */
		/* is appended to it so waddstr can use it in        */
		/* std_edit_keys.				     */
		/*---------------------------------------------------*/
		if (rc >= 0)
		{
			mbstring[++count] = '\0';
			return (c); /*mbstring is global */
		}

		if (count == MB_LEN_MAX)
			return (-1);

		if (IS_PADKEY(c))
		{
			mbstring[++count] = '\0';
			return (c);
		}
	}

	return (c);  /* mbstring is global */

} /* end ms_input  */
