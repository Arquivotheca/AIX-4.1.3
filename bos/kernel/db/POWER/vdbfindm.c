static char sccsid[] = "@(#)73	1.15  src/bos/kernel/db/POWER/vdbfindm.c, sysdb, bos411, 9428A410j 6/5/91 09:43:43";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: findit, f_continue
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/param.h>
#include "parse.h"			/* parser structure		*/
#define FINDM
#include "vdbfindm.h"			/* find definitions		*/
#include "vdberr.h"			/* Error message stuff		*/

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern	int get_from_memory();		/* get data from memory		*/
extern  char *getterm();		/* get from the terminal	*/


/*
 * NAME: findit
 *                                                                    
 * FUNCTION: 
 *   Find data in memory.
 *   . memory may have holes in it, e.g. be paged out.
 *   . Search for either hex data or a string.
 *                                                                    
 * RETURN VALUE:
 *   the address of the match or NOTFOUND.
 */  

#define ALIGN ps->token[ALIGNP].dv
#define PAGECOUNT	10

static int paged_out;
static caddr_t memp1;

caddr_t
findit (ps, virt, interactive)
struct parse_out *ps;			/* input parameters		*/
int virt;				/* xlation on/off		*/
int interactive;			/* from parser or command	*/
{
	caddr_t memp, argp, endp, argp1, found;
	char c;
	int arglen;

	if(ps -> num_tok < 3)
	{
	    printf("usage: find <value> <start addr> <end addr> [1,2, or 4]\n");
	    return((caddr_t) NOTFOUND);
	}

	/* !!! assignment into un-used parser structure entry !!! */
	if (ps -> num_tok < 4)
		ALIGN = 1;

	if ((ALIGN != 1) && (ALIGN != 2) && (ALIGN != 4))
	{
		printf ("find: bad alignment\n");
		return ((caddr_t) NOTFOUND);
	}

	/* convert length of hex string to number of bytes */

	if (ps -> token[ARGP].tflags & HEX_VALID)
		arglen = (strlen (ps->token[ARGP].sv) + 1) / 2;
	else
		arglen = strlen (ps->token[ARGP].sv);

	/* calculate effective start address (round down to align boundary) */
	memp = (caddr_t) (ps->token[STARTP].hv & (0-ALIGN));

	/* save away end address for search */
	endp = (caddr_t) (ps -> token[ENDP].hv);

	/* calculate pointer to pattern */
	if (ps -> token[ARGP].tflags & HEX_VALID)
		argp = ((caddr_t) &(ps->token[ARGP].hv) +
			sizeof (ps->token[ARGP].hv) - arglen);
	else
		argp = ps->token[ARGP].sv;

	paged_out = 0;
	found = (caddr_t) NOTFOUND;

	while ((memp < endp) && (found == (caddr_t) NOTFOUND))
	{

		argp1 = argp;
		memp1 = memp;

		while ((argp1 < argp + arglen) && (memp1 <= endp) &&
			(found == (caddr_t) NOTFOUND))
		{
			if (!get_from_memory(memp1,virt,&c,sizeof(c)))
			{
				paged_out++;

				if (! interactive)
					return ((caddr_t) NOTFOUND);

				if (! f_continue ())
					return ((caddr_t) NOTFOUND);

				/* set memp to the start of the next page */
				memp = (caddr_t) (((int)memp+PAGESIZE) &
					(0-PAGESIZE)) - ALIGN;
			}
			else
				/* A byte was retrieved. */
				paged_out = 0;

			if (*argp1 != c)
				break;

			if (argp1 == argp+arglen-1)
				found = memp;  

			memp1++;
			argp1++;
		}
		memp += ALIGN;
	}

	return(found);
}

/*
 * NAME: f_continue
 *
 * FUNCTION: called when find encounters a paged out address
 *
 * NOTES:
 *	Determines whether find will continue searching, or will terminate
 *	the search.
 */

f_continue ()
{
	char *response;
	char errst[9];

	/* print message "Page starting at address xxxxx not in real mem" */
	sprintf (errst,"%x",memp1);
	vdbperr(page_not_in_real1, errst, page_not_in_real2);

	if (paged_out == PAGECOUNT)
	{
		while(TRUE)
		{
			vdbperr(debeenv);
			response = getterm();
			if ((*response == 'n') || (*response == 'N'))
				return(FALSE);
			if ((*response == 'y') || (*response == 'Y'))
			{
				paged_out = 0;
				return(TRUE);
			}
		}
	}
	return (TRUE);
}
