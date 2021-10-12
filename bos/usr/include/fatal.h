/* @(#)58	1.6  src/bos/usr/include/fatal.h, cmdsccs, bos411, 9428A410j 4/25/91 19:23:54 */
/*
 * COMPONENT_NAME: (CMDSCCS) - Source Code Control System (sccs)
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* fatal.h	5.1 - 86/12/09 - 06:04:27 */
#ifndef _H_FATAL

#define _H_FATAL        1
#include <setjmp.h>

extern	int	Fflags;
extern	char	*Ffile;
extern	int	Fvalue;
extern	int	(*Ffunc)();
extern  jmp_buf Fjmp;

# define FTLMSG		0100000
# define FTLCLN		 040000
# define FTLFUNC	 020000
# define FTLACT		    077
# define FTLJMP		     02
# define FTLEXIT	     01
# define FTLRET		      0

# define FSAVE(val)	SAVE(Fflags,old_Fflags); Fflags = val;
# define FRSTR()	RSTR(Fflags,old_Fflags);

#endif /* _H_FATAL */
