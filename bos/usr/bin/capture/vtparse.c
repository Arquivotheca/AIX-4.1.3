static char sccsid[] = "@(#)73	1.4  src/bos/usr/bin/capture/vtparse.c, cmdsh, bos411, 9428A410j 3/24/94 18:34:34";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: ERR_RET, vtparse
 *
 * ORIGINS: 10, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* define to use faster MACROS instead of functions for performance */
#define _ILS_MACROS

#include <sys/types.h>
#include <ctype.h>

#include "vtparse.h"

#define ERR_RET()	rv.v_type = V_STRING;\
			rv.v_data.string = S_string;\
			rv.v_len = cp - S_string;\
			*rrv = rv;\
 			state = s_start;\
			return -1;

/*
 * vtparse.c
 *
 * Routine called after an escape character is read.  This routine
 * parses VT-100 sequences.  This routine returns a structure of
 * type "vcmd_t" which is described above.  The element of the union
 * "v_data" is a pointer (either int or char) to a *static* area of 
 * memory within this function, so if you want to save a value in the union,
 * you must copy that value to another safe place.
 */

int vtparse(ch, rrv)
int ch;				/* char input */
vcmd_t *rrv;			/* return structure */
{
	static short	semicolon_seen, 
			close_paren, 	/* just too keep track */
			cmdx_p; 	/* whether we got a ? or not */
	static char	S_string[64]; 	/* holds rejected characters */
	static int	S_args[16]; 	/* holds arguments to cmds */
	static vcmd_t	rv;		/* return value */
	static char	*cp;		/* tmp char star */
	static int	acc; 		/* number accumulator */

	static enum {
		s_start,		/* 0 */
		s_after_bracket,	/* 1 */
		s_after_pigpen,		/* 2 */
		s_indigit,		/* 3 */
		s_after_paren		/* 4 */
		} state = s_start; 	/* state variable */

	cp = S_string;

	/* 
	 *	Use a for loop to allow one case to throw out to another
	 *	case with a continue statement.
	 */

	for(;;) 
	{	if(state != s_start) *cp++ = ch;

		switch(state)
		{  case s_start:
			semicolon_seen    = FALSE;
			close_paren       = FALSE;
			cmdx_p            = FALSE;
			rv.v_data.args    = S_args;
			rv.v_data.args[0] = 0;
			rv.v_data.args[1] = 0;
			rv.v_data.args[2] = 0;
			rv.v_data.args[3] = 0;
			rv.v_len          = 0;
			
			acc   = 0;
			cp    = S_string;
			*cp++ = ch;

			switch (ch)
			{  case '[':
				state = s_after_bracket; 
				break;
			   case '#':
				state = s_after_pigpen; 
				break;
			   case '(':
				state = s_after_paren; 
				break;
			   case ')':
				state = s_after_paren;
				close_paren = TRUE;
				break;
			   default:		/* check for SCMD1 */
				if(strchr(SCMD1STR, ch)) {
					rv.v_type         = V_SCMD1;
					rv.v_len          = 1;
					rv.v_data.args[0] = ch;
					*rrv = rv;
					state = s_start;
					return(0);	/* done processing */
				}
				ERR_RET();	/* else, reject the string */
			}
			return(1);	/* continue processing */


		   case s_after_bracket:
			if(ch == '?') {		/* Possibly CMDX */
				cmdx_p = TRUE;	/* keep same state */
				return 1;
			}
			if(ch == ';') { 	/* keep the same state */
				semicolon_seen = TRUE;
				return 1;
			}
			if(isdigit(ch)) {
				acc = ch - '0';	/* account 1st digit */
				state = s_indigit;
				return 1;
			}
			if(ch >= 0x40) {	/* good cmd character */
						/* GOOD, we got a CMD */
				rv.v_data.args[rv.v_len] = (int) ch;
				rv.v_type = cmdx_p ? V_CMDX : V_CMD;
				*rrv = rv;
				state = s_start;
				return 0;	/* done processing */
			}
			ERR_RET();
		
		case s_after_paren:
			if(isalnum(ch)) {
				rv.v_data.args[0] = ch;
				rv.v_type = close_paren ? V_SCMD4 :V_SCMD3;
				rv.v_len = 1;
				*rrv = rv;
				state = s_start;
				return 0;	/* done processing */
			}

		case s_after_pigpen:
			if(isdigit(ch)) {
				rv.v_data.args[0] = ch;
				rv.v_type = V_SCMD2;
				rv.v_len = 1;
				*rrv = rv;
				state = s_start;
				return 0;	/* done processing */
			}
			ERR_RET();
		
		case s_indigit:
			if(!isdigit(ch)) {
				if (semicolon_seen && rv.v_len == 0)
					rv.v_len++;
				rv.v_data.args[rv.v_len++] = acc;
				acc = 0;
				state = s_after_bracket;
	/* *throw* */		continue;
			}
			acc *= 10;		/* keep the same state */
			acc += ch - '0';
			return 1;		/* still processing */
		}
	}
}
