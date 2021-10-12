static char sccsid[] = "@(#)getmode.c	1.1 11/30/89 14:43:14";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: getdamode, find_match
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

#include	"diag/tm_input.h"
#include	"diag/bit_def.h"
#include	"diag/assign_bit.h"

/******************************************************************************/
/*									      */
/*	SET_BIT_MODE_FOR is a macro takes one parameter which is one of the   */
/*		        tm_input fields such as a system or advanced or some  */
/*		   	other tm_input modes. 				      */
/*	SET_BIT_MODE_FOR calls find_match for the tm_input and set the bit    */
/*		        associated with that mode in da_mode.		      */
/*									      */
/******************************************************************************/

#define SET_BIT_MODE_FOR(VAR) 						\
		da_mode |= find_match( tm_input->/**/VAR, tm_da_/**/VAR);


/******************************************************************************/
/*									      */
/*	getdamode is a function designed to translate the tm_input into a     */
/*		long word and return that word to the DA.		      */
/*									      */
/*	NOTE the calling function should pass an address of tm_input	      */
/*	     structure.							      */
/*									      */
/******************************************************************************/

long getdamode(tm_input)
struct	tm_input	*tm_input;
{
	long	da_mode=0;		/* Diagnostic Application Mode	*/

	/****************************************************************/
	/* 								*/
	/*  Set a disgnated bit in the da_mode word where the mode 	*/
	/*  applies. The parameter passed to the macro is the name 	*/
	/*  of the environment used by DC.				*/ 
	/* 								*/
	/****************************************************************/

   	SET_BIT_MODE_FOR(exenv)
	SET_BIT_MODE_FOR(advanced)
	SET_BIT_MODE_FOR(system)
	SET_BIT_MODE_FOR(dmode)
	SET_BIT_MODE_FOR(loopmode)
	SET_BIT_MODE_FOR(console)
	return(da_mode);
}
/************************************************************************/
/*									*/
/*	find_match is a function designed to find a match in the 	*/
/*	data base structure and the environment passed from the 	*/
/*	tm_input and return a word matching that environment		*/
/*									*/
/************************************************************************/

int find_match(env, tm_env_da_bits)
int	env;
struct	tm_env_da_bit	*tm_env_da_bits;
{
	while(     ( tm_env_da_bits->da_bit != 0 ) 
		&& ( env != tm_env_da_bits->tm_env ) )
		tm_env_da_bits++;
	if (env == tm_env_da_bits->tm_env )
		return(tm_env_da_bits->da_bit);
	else
		return(0);
}
