static char sccsid[] = "@(#)97	1.7  src/bos/usr/ccs/lib/libblas/bl_err.c, libblas, bos411, 9428A410j 11/9/93 14:38:37";
/*
 * COMPONENT_NAME: (LIBBLAS) Basic Linear Algebra Subroutine Library
 *
 * FUNCTIONS: Write fatal and warning messages to stderr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>			/* includes... */
#include <string.h>
#include <locale.h>

#include  <nl_types.h>
nl_catd catd;
#include "bl_err_msg.h"
#define MSGSTR(C,D)      catgets(catd,MS_BLAS_ERROR,C,D)


/* length of routine name as defined in xerbla.f */
#define NAME_LENGTH 6

/*
 * NAME: blas_fatal
 *                                                                    
 * FUNCTION: Issues fatal messages to stderr
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                  
 *      Called by xerbla, the blas error handling routine.
 *      Called because the msg facility doesn't work under
 *      Fortran.                                                 
 *
 * (RECOVERY OPERATION:) 
 *	
 *
 * (DATA STRUCTURES:) None.
 *
 * RETURNS: (NONE)
 */  


  void blas_fatal
		   
#ifdef _NO_PROTO

  (routine_name, error_parm_number)

char * routine_name;	/* name of failing routine */
int * error_parm_number;   /* which parm was invalid */

#else

(char * routine_name,
 int * error_parm_number)

#endif

  {
  
  /* convert FORTRAN string to C null-terminated string */

  char c_name[NAME_LENGTH + 1];

  (void) strncpy (c_name, routine_name, (size_t) NAME_LENGTH);
  c_name[NAME_LENGTH] = '\0';
  
  catd = catopen(MF_BL_ERR, NL_CAT_LOCALE);

  (void) fprintf (stderr, 
		  MSGSTR(FORT_FATAL1,
			 "libblas: On entry to %s parameter number %d had an illegal value\n"),
		  c_name, *error_parm_number);

  }
     
