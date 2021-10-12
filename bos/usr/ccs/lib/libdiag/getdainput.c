static char sccsid[] = "@(#)40	1.7  src/bos/usr/ccs/lib/libdiag/getdainput.c, libdiag, bos41B, bai4 1/9/95 13:54:11";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	getdainput
 *            	clrdainput
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */                                                                   

/* NAME: getdainput
 *
 * FUNCTION: This unit returns the input for the diagnostic application
 * 		from the TM_input Object Class.
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	0  - get successful
 *	-1 - get unsuccessful
 */

#include <sys/processor.h>
#include "diag/tm_input.h"
#include "diag/class_def.h"		/* object class data structures	*/ 
#include "diag/tmdefs.h"

int clrdainput(void);			/* function prototype */

getdainput(tm_input)
struct	tm_input	*tm_input;
{

	int rc;
	struct	TMInput	*T_TMInput;
	struct listinfo v_info;
	char	crit[64];
	pid_t	pid;

	/* get pid of this process, and search TMInput for object */
	/* if not found, search for object with pid = 0 	  */
	pid = getpid();
	sprintf( crit, "pid = %d", pid); 
	T_TMInput = (struct TMInput *)diag_get_list( TMInput_CLASS, crit,
			&v_info, 1, 1 );

	if ( T_TMInput == (struct TMInput *)-1 || v_info.num == 0 ) {
		sprintf( crit, "pid = 0" );
		T_TMInput = (struct TMInput *)diag_get_list(TMInput_CLASS,
			crit, &v_info, 1,1);
	}

	if ( T_TMInput != (struct TMInput *)-1 && v_info.num == 1 )  {
		if(T_TMInput->cpuid > 0) { /* asked for processor bondage */
			(void)bindprocessor(BINDPROCESS, getpid(),
						T_TMInput->cpuid - 1);
			/* return code does not need to be checked */
			/* an error is not fatal here */
		}
		tm_input->exenv = T_TMInput->exenv;
		tm_input->advanced = T_TMInput->advanced;
		tm_input->system = T_TMInput->system;
		tm_input->dmode = T_TMInput->dmode;

		strncpy(tm_input->date,T_TMInput->date, sizeof(tm_input->date));

		tm_input->loopmode = T_TMInput->loopmode;
		tm_input->lcount = T_TMInput->lcount;
		tm_input->lerrors = T_TMInput->lerrors;
		tm_input->console = T_TMInput->console;

		strcpy(tm_input->parent,T_TMInput->parent);
		strcpy(tm_input->parentloc,T_TMInput->parentloc);

		strcpy(tm_input->dname,T_TMInput->dname);
		strcpy(tm_input->dnameloc,T_TMInput->dnameloc);

		strcpy(tm_input->child1,T_TMInput->child1);
		tm_input->state1 = T_TMInput->state1;
		strcpy(tm_input->childloc1,T_TMInput->childloc1);

		strcpy(tm_input->child2,T_TMInput->child2);
		tm_input->state2 = T_TMInput->state2;
		strcpy(tm_input->childloc2,T_TMInput->childloc2);

		/* If the execution environment is system exerciser AND	    *
		 * the console is FALSE, then we explicitly clear TMInput.  *
		 * Else, TMInput is cleared by the controller or by the DA. */
		if ((tm_input->exenv == EXENV_SYSX) &&
		    (tm_input->console == FALSE ))
			odm_rm_obj(TMInput_CLASS, crit);

		return(0);	
	}
	return(-1);
}
/*  */
/* NAME: clrdainput
 *
 * FUNCTION: This function clears any TMInput object that
 *	     may exist in the data base.
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	0  - get successful
 *	-1 - get unsuccessful
 */

int clrdainput(void)
{
	return(diag_rm_obj(TMInput_CLASS, ""));
}
