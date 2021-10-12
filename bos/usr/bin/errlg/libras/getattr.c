static char sccsid[] = "@(#)64  1.4  src/bos/usr/bin/errlg/libras/getattr.c, cmderrlg, bos411, 9428A410j 2/3/94 14:27:12";

/*
 *   COMPONENT_NAME: CMDERRLG
 *
 *   FUNCTIONS: ras_getattr 
 *		ras_putattr
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <odmi.h>
#include <stdio.h>
#include <string.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <errlg/SWservAt.h>

#define NUMATTRS	10  /* number of attributes you expect to find */

static char criteria[MAX_CRITELEM_LEN]; /* search criteria string */

/*
 * NAME: ras_getattr
 *
 * FUNCTION: This function queries the software service aids object class
 *           (SWservAt) given an attribute name to search on.  If the object
 *	     containing the attribute information exists, then a pointer to 
 * 	     this structure is returned to the calling routine.  If no object
 *           is found, then NULL is returned.  
 *
 * EXECUTION ENVIRONMENT:
 *	     Preemptable		: Yes
 *	     Runs on Fixed Stack	: No
 *	     May Page Fault		: Yes
 *	     Serialization		: none 
 *
 * INPUTS: 
 *  	     attribute name	: The attribute name to search on.
 *	     getall		: If this is true, get all attributes that 
 *			          match the search criteria.  Otherwise,
 *				  just get the first attribute.	
 *	     how_many		: This will hold the number of attributes 
 *				  that were actually retrieved. 
 *
 * RETURNS:
 *	NULL	object not found 
 *	pointer to structure which contains attribute information
 */

struct SWservAt *ras_getattr(attrname, getall, how_many)
char *attrname;
int getall;
int *how_many;
{
	struct 	listinfo SWservAt_info;		/* info about retrieved objects */
	struct 	SWservAt *SWservp;		/* pointer to retrieved object  */
	struct  SWservAt *p;			/* temporary pointer to retrieved object */
	int 	num_expected;			/* the number of attributes you expect to find */ 
	int	i;				


	*how_many = 0;

	/* If getall is true, then we should retrieve all objects that match the 
	   search criteria.  Otherwise, just retrieve the first object. */
	if (getall) 
		num_expected = NUMATTRS;
	else
		num_expected = 1;

	sprintf(criteria, "attribute = '%s'", attrname); 

	SWservp = get_SWservAt_list(SWservAt_CLASS, criteria, &SWservAt_info, num_expected, 1);

	if ((SWservp == -1) || (SWservp == NULL)) /* object not there */ 
		return (NULL);
	else					  /* object is there  */ 
		{
		*how_many = SWservAt_info.num;
		p = SWservp;

		/* Check to see if the attribute's value field is null.  If it is, copy 
		   the default value of the attribute to the current value and return.  
		   This way, the caller always gets something returned in the value
		   field of the structure as long as the default value is there. */ 

		for (i = 0; i < SWservAt_info.num; i++, p++)
			if (strcmp(p->value,"\0") == 0)
				p->value = stracpy(p->deflt);
		return (SWservp);
		}
}

/*
 * NAME: ras_putattr
 *
 * FUNCTION: This function updates the software service aids object class 
 *           (SWservAt) given an attribute name to search on.  If the object 
 *	     containing the attribute information exists, then the value field  
 * 	     of the attribute is updated according to the structure passed in. 
 *	     If there is no object found that matches the search criteria, then
 *	     -1 is returned.	
 *
 * EXECUTION ENVIRONMENT:
 *	     Preemptable		: Yes
 *	     Runs on Fixed Stack	: No
 *	     May Page Fault		: Yes
 *	     Serialization		: none 
 *
 * INPUTS: 
 *	pointer to structure which contains attribute information	
 *
 * RETURNS:
 *	-1      failure	
 *	 0	success	
 */

int ras_putattr(SWservobj)
struct SWservAt *SWservobj;
{
struct SWservAt *SWservp;	/* pointer to retrieved object */
struct listinfo SWservAt_info;	/* info about retrieved object */

	/* We need to search on both attribute name and default because of
	   alog.  There will be multiple attributes of alog_type in the object
	   class. This assumes that the default value is known by the caller, 
	   or is retrieved with a call to ras_getattr(). */
	sprintf(criteria, "attribute = '%s' AND deflt = '%s'", SWservobj->attribute, SWservobj->deflt);
 
	if (((SWservp = get_SWservAt_list(SWservAt_CLASS, criteria, &SWservAt_info, 1, 1)) == -1) ||
	    ((SWservp == NULL)))			/* object is not there */
		return (-1);
	else
		{					/* object is there, so change it */
		SWservp->value = stracpy(SWservobj->value);
		if (odm_change_obj(SWservAt_CLASS,SWservp)) 
			{
			odm_free_list(SWservp,&SWservAt_info);
			return (-1);
			}
		odm_free_list(SWservp,&SWservAt_info);
		return (0);
		}
}
