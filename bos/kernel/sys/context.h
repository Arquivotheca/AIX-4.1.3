/* @(#)71	1.8  src/bos/kernel/sys/context.h, sysproc, bos411, 9428A410j 4/2/93 13:49:36 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef	_H_CONTEXT
#define	_H_CONTEXT
#include <sys/m_param.h>
#include <sys/mstsave.h>

/* define return values for exception handlers */
#define EXCONTINUE	0
#define	EXRETURN	1
#define	EXRESUME	2

/* define exception type values; negative values are user defined */
#define	EXPGIO		0
#define EXTRAP		1
#define	EXIO		2
#define EXDLOK		3
#define EXSIG		4

/* define structure containing context for jumps within the save process,
   in the same execution mode */
struct	jmpbuf	{
	struct	mstsave	jmp_context;
};

#endif /* _H_CONTEXT */
