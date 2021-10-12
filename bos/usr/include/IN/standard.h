/* @(#)99	1.11  src/bos/usr/include/IN/standard.h, libIN, bos411, 9428A410j 5/11/93 10:29:24 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_STANDARD
#define _H_STANDARD

#define MAXLINE 512 /* maximum text line length (includes newline) */

#define BITSPRBYTE 8

#define MAXPOS(type) (((type)1 << (BITSPRBYTE*sizeof(type)-1)) - (type)1)
 /* yields max pos number of given type (not unsigned) */

#define UNSSHORT(n) ((unsigned short)(n))

#define UNSCHAR(c) ((unsigned char)(c))

#define ARRAYLEN(a) (sizeof(a)/sizeof(a[0]))

#ifndef NUL
#define NUL     0
#endif

#define TRUE    1
#define FALSE   0
#ifndef NULL
#define NULL    0
#endif

#define SOBUF   _sobuf          /* for setbuf(stdout, SOBUF) */
extern char     _sobuf[];

#define Reg1    register
#define Reg2    register
#define Reg3    register
#define Reg4    register
#define Reg5    register
#define Reg6    register
#define Reg7
#define Reg8
#define Reg9
#define Reg10
#define Reg11
#define Reg12

/* output generated */
#define EXITOK    UNSCHAR(0)
#define EXITWARN  UNSCHAR(1)    /* warning condition (1 - 037) */
#define EXITERROR UNSCHAR(040)  /* error occurred (040 - 077) */
#define EXITFATAL UNSCHAR(0100) /* fatal error occurred(0100-0177) */

/* specific errors */
#define EXITEXEC  UNSCHAR(255)  /* exec failed */
#define EXITBAD   UNSCHAR(254)  /* bad parameters, unopenable user file, etc*/
#define EXITIO    UNSCHAR(253)  /* io error occurred */
#define EXITFILE  UNSCHAR(252)  /* system file missing or in error */
#define EXITSIGNAL UNSCHAR(251) /* signal received */
#define EXITSPACE UNSCHAR(250)  /* program ran out of memory */
#define EXITOTHER UNSCHAR(128)  /* program-specific error conditions (128+)*/

#define EXIT_REASON(status) UNSCHAR(status)
#define EXIT_VALUE(status)  UNSCHAR((status)>>8)
/* The exit reason value is defined as follows:
 *
 * 0177 - means the child process stopped, EXIT_VALUE is the signal number
 * 0    - child exited, EXIT_VALUE is exit() code
 * >0   - signal number which caused child to terminate
 *
 */

#endif /* _H_STANDARD */
