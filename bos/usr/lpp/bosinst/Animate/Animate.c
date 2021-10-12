static char sccsid[] = "@(#) 33 1.1 src/bos/usr/lpp/bosinst/Animate/Animate.c, bosinst, bos411, 9428A410j 93/06/30 16:56:39";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: Animate
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * NAME: Animate
 *
 * FUNCTION: Displays a sequence of characters on the same spot on the
 *	screen
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This process is started by DisplayMenu.  The animation
 *      sequence is continued unitil a kill signal is recieved.
 *
 * RETURNS: NONE
 */

#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
    char *beg;			/* ptr to begining of animation string 	*/
    char *ptr;			/* ptr to current animation character   */

    /* check arg count */
    if (argc != 2)
    {
         printf("Usage: Animate string\n");
         exit(0);
    }

    /* Set the ptrs */
    beg = ptr = argv[1];
    printf("%c\010", *ptr++);
    fflush(stdout);
    while (1)
    {
	sleep(1);
	if (!*ptr)
	    ptr = beg;
	printf("%c\010", *ptr++);
	fflush(stdout);
    }
}

        
