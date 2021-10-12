static char sccsid[] = "@(#)50	1.4  src/bos/usr/ccs/lib/libsrc/bldview.c, libsrc, bos411, 9428A410j 10/18/90 15:08:20";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	bldview
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


#include <stdio.h>
#include <odmi.h>
#include "srcopt.h"
#include "srcodm.h"

/*
** IDENTIFICATION:
**    Name:	bldview
**    Title:	Build Add/Update Object View
** PURPOSE:
**    To build the object view used on add/read/update of an object record.
** 
** SYNTAX:
**    bldview(argview,fieldview,bufaddr,dbaddr)
**    Parameters:
**       i struct argview argview[]
**       i struct fieldview **fieldview
**	 o char *bufaddr
**	 o char *dbaddr
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**	Count the number of fields that had values inputed.
**	Allocate storage for the object view to be created.
**	Copy data from argview into  the object view allocated.
**	return the address of the object veiw allocated in fieldview.
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**
** RETURNS:
**    number of fields to be acted apon
**    -1 if a field had been assiged to be update more that once
**    -2 if memory could not be allocated
**/
int bldview(argview,fieldview,bufaddr,dbaddr)
struct argview argview[];
struct fieldview **fieldview;
char *bufaddr;
char *dbaddr;
{
	int i;
	int numviews;
	struct fieldview *ptrview;
	char *malloc();

	/* count the number of fields that are to be changed/added */
	for(numviews=1, i=0;argview[i].size != 0;i++) 
	{

		/* new value for this field ? */
		if(argview[i].newval>(char)0 && argview[i].view==VIEWFIELD)
			/* new val assigned more than once? */
			if(argview[i].newval > 1)
				/* yes. it is illegal */
				return(-1);
			else
				/* valid field, count it */
				numviews++;
	}

	/* allocate mem for object view */
	ptrview=(struct fieldview *)malloc((unsigned)(sizeof(struct fieldview)*numviews));
	/* did we run out of memory? */
	if(ptrview == NULL)
		return(-2);

	/* init the memory allocated */
	memset(ptrview,0,(unsigned)(sizeof(struct fieldview)*numviews));

	/* return pointer to allocated storage */
	*fieldview=ptrview;

	/* assign values to the objects view */
	for(i=0;argview[i].size != 0;i++) 
	{
		/* field marked for update/add */
		if(argview[i].newval>(char)0 && argview[i].view==VIEWFIELD)
		{
			/* cpy field info to fieldview */
			ptrview->size=argview[i].size;
			ptrview->c_addr=argview[i].bufaddr;
			ptrview->db_addr=dbaddr+(argview[i].bufaddr-bufaddr);
			ptrview++;
		}
	}

	return(numviews);
}
