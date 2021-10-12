/* @(#)50	1.9  src/bos/usr/include/POWER/execargs.h, sysproc, bos411, 9428A410j 2/3/94 19:19:45 */
/*
 * COMPONENT_NAME: (SYSPROC) Process Management 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 6, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_EXECARGS
#define _H_EXECARGS

/*
 * This include file permits ps to print the command line for
 * a process.  Given the address of the argument vector for
 * the command, the shell uses the newargs() macro to put the
 * address of the argument vector where ps can find it.
 */

#include <sys/param.h>
extern int errno;

struct	top_of_stack {
	ulong	main_reg[NGPRS];
	ulong	lr;
	char	**environ;
	int	errno;
};

typedef struct top_of_stack TopOfStack;

#define TopOfBaseStack	((struct top_of_stack *)((int)&errno		\
						- sizeof(char **)	\
						- sizeof(ulong)		\
						- sizeof(ulong[NGPRS])))

#define ARGC_value	(&(TopOfBaseStack->main_reg[ARG1]))
#define ARGS_loc	(&(TopOfBaseStack->main_reg[ARG2]))
#define ENVS_loc	(&(TopOfBaseStack->main_reg[ARG3]))
#define newargs(argv)	(*(char ***)ARGS_loc = (argv))

#endif /* _H_EXECARGS */
