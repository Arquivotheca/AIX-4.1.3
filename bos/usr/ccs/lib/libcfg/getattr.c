static char sccsid[] = "@(#)41  1.14  src/bos/usr/ccs/lib/libcfg/getattr.c, libcfg, bos41J, 9520A_all 5/16/95 09:39:28";
/*
 * COMPONENT_NAME: (LIBCFG)  Generic library config support
 *
 * FUNCTIONS: getattr, putattr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <odmi.h>
#include <cf.h>
#include <sys/cfgodm.h>
           
#define MOREATTR        8       /* for customized attributes that
				   have no associated predefined attr */
int conferrno;

/*---------------------------------------------------------------

  getattr()

  Description: This routine is queries the Customized Attribute
  object class given a device logical name and attribute name to
  search on. If the object containing the attribute information
  exists, then this structure is returned to the calling routine.
  Otherwise, assume that the attribute took on a default value
  which is stored in the Predefined Attribute object class. This
  class is queried, and if the attribute information is found, the
  relevant information is copied into a Customized Attribute
  structure which is returned to the calling routine. If no object
  is found in Predefined Attribute object class, then NULL is
  returned indicating an error.

  Input: device logical name
	 attribute name
	 getall
	 how_many

  Returns: pointer to a structure which contains the attribute information
	   NULL if failed

  ---------------------------------------------------------------*/

struct CuAt 
*getattr(devname, attrname, getall, how_many)
char	*devname;
char 	*attrname;
int	getall;
int 	*how_many;
{
	struct CuDv	cudv;		/* CuDv object */
	struct CuDv	parcudv;	/* parent CuDv object */
	struct CuAt     *newobj;        /* pointer to CuAt object */
	struct CuAt     *cuatptr;       /* pointer to CuAt object */
	struct CuAt	*cuat;		/* pointer to CuAt object */	
	struct CuAt	*newcuat;	/* pointer to CuAt object */
	struct PdAt	*pdobj;		/* pointer to PdAt object */
	struct PdAt	*pdat;		/* temporary holder for PdAt object */
	struct PdAt	*parpdat;	/* temporary holder for parent PdAt object */
	struct PdAt	*total;		/* temporary holder for combined PdAt object list*/
	struct listinfo pdat_info;	/* info about PdAt retrieved objects */
	struct listinfo par_pdat_info;	/* info about parent PdAt retrieved objects */
	struct listinfo cuat_info;	/* info about CuAt retrieved objects */
	struct listinfo cudv_info;	/* info about CuDv retrieved objects */
	struct listinfo par_cudv_info;	/* info about CuDv retrieved objects */
	char   criteria[MAX_CRITELEM_LEN];/* search criteria string */
	register int    i, j;           /* loop counters */
	int    num;
	int    rc;

	*how_many = 0;

	/* look for the specified device in CuDv */

	sprintf(criteria, "name = '%s'", devname);
 	rc = (int)odm_get_first(CuDv_CLASS, criteria, &cudv);
	if (rc == -1)
	{
		conferrno = E_ODMGET;
		return(NULL);
	}
	else if (rc == 0) 
 	{
		conferrno = E_NOCuDv;
		return(NULL);
	}
	
	/* get the customized attr info */

	if (getall)
	{
		num = MOREATTR;         /* Prepare for multiple attr's */
		sprintf(criteria, "name = '%s'", devname);
	}
	else {
		num = 1;                /* Only expect one attribute */
		sprintf(criteria, "name = '%s' AND attribute = '%s'",
				     devname, attrname);
	}

	cuat = get_CuAt_list(CuAt_CLASS, criteria, &cuat_info, num, 1);
	if ((int) cuat == -1)
	{
		conferrno = E_ODMGET;
		return(NULL);
	}


	if ( (!getall) && (cuat_info.num) )
	{  /* cuat was found - return it */
	   *how_many = cuat_info.num;
	   return(cuat);
	}

	/* get the pdat info */

	if (getall)
	{
		num = cuat_info.num + MOREATTR; /* May be more PdAt attrs */
		sprintf(criteria, "uniquetype = '%s' AND type != 'E'", cudv.PdDvLn_Lvalue);
	}
	else
		/* num is still 1 */
		sprintf(criteria, "uniquetype = '%s' AND attribute = '%s' AND type != 'E'",
				     cudv.PdDvLn_Lvalue, attrname);

	pdat = get_PdAt_list(PdAt_CLASS, criteria, &pdat_info, num, 1);
	if ((int) pdat == -1)
	{
		if(cuat) odm_free_list(cuat, &cuat_info);
		conferrno = E_ODMGET;
		return(NULL);
	}

	
        /* if attr not found then look for an extension attribute under the */
	/* parent or if getall, but only if cudv has a parent */
       /* if (((!pdat_info.num)  || (getall)) && (cudv.parent[0] != NULL)){*/ 
        if (((!pdat_info.num)  || (getall)) && (strcmp(cudv.parent,""))){ 

		/* get parent object */
		sprintf(criteria, "name = '%s'",cudv.parent);
        	rc = (int)odm_get_first(CuDv_CLASS, criteria, &parcudv);
        	if (rc == -1)
        	{
			if(cuat) odm_free_list(cuat, &cuat_info);
			if(pdat) odm_free_list(pdat, &pdat_info);
                	conferrno = E_ODMGET;
                	return(NULL);
        	}
        	else if (rc == 0)
        	{
			if(cuat) odm_free_list(cuat, &cuat_info);
			if(pdat) odm_free_list(pdat, &pdat_info);
                	conferrno = E_NOCuDv;
                	return(NULL);
        	}

		/*  get extension attributes */
		if (getall)
		{
			num = cuat_info.num + MOREATTR; /* May be more PdAt attrs */
			sprintf(criteria, "uniquetype = '%s' AND type = 'E'", parcudv.PdDvLn_Lvalue);
		}
		else
		{
			/* num is still 1 */
			sprintf(criteria, "uniquetype = '%s' AND attribute = '%s' AND type = 'E'",
					     parcudv.PdDvLn_Lvalue, attrname);
		}
        	parpdat = get_PdAt_list(PdAt_CLASS,criteria,&par_pdat_info,num,1);
        	if((int)parpdat == -1)
		{
			if(cuat) odm_free_list(cuat, &cuat_info);
			if(pdat) odm_free_list(pdat, &pdat_info);
			conferrno = E_ODMGET;
			return(NULL);
		}

                /* if not found and !getall then fail*/
		/* or if getall and neither list found */
		if (((!par_pdat_info.num) && (!getall))
			|| ((!par_pdat_info.num) && (!pdat_info.num)))
		{
			if(cuat) odm_free_list(cuat, &cuat_info);
			if(pdat) odm_free_list(pdat, &pdat_info);
			if(parpdat) odm_free_list(parpdat, &par_pdat_info);
			conferrno = E_NOPdOBJ;
			return(NULL);
		}

		/* if getall and extension attributes found*/
		if (getall && par_pdat_info.num && pdat_info.num) {
 
                        /* merge list */
			total = (struct PdAt *) malloc(sizeof(struct PdAt)
				 * (pdat_info.num + par_pdat_info.num));

	        	if ((int)total == NULL) 
			{
				if(cuat) odm_free_list(cuat, &cuat_info);
				if(pdat) odm_free_list(pdat, &pdat_info);
				if(parpdat)
					odm_free_list(parpdat, &par_pdat_info);
				conferrno = E_MALLOC;
				return(NULL);
			}

			
 		       	/* copy attributes into a single list */
              		memcpy(&total[0],&pdat[0],
				sizeof(struct PdAt) * pdat_info.num);
			
              		memcpy(&total[pdat_info.num],&parpdat[0],
				sizeof(struct PdAt) * par_pdat_info.num);

			free(pdat);
			pdat = total;
			pdat_info.num += par_pdat_info.num;

			/* free the parent list */
			if(parpdat)odm_free_list( parpdat, &par_pdat_info );

		} 
		else if (par_pdat_info.num) 
		{
			pdat = parpdat;
			pdat_info = par_pdat_info;
        	} 

	}
	/* if no PdAt object and device has no parent, fail */
	if (!pdat_info.num) {
		if(cuat) odm_free_list(cuat, &cuat_info);
		if(pdat) odm_free_list(pdat, &pdat_info);
		if(parpdat) odm_free_list(parpdat, &par_pdat_info);
		conferrno = E_NOPdOBJ;
		return(NULL);
	}

	/* copy all the PdAt information to a CuAt structure
	   for every predefined attribute found */

	/* malloc enough space */
	if ((newcuat = (struct CuAt *) malloc((sizeof(struct CuAt)) *
		pdat_info.num)) == NULL)
	{
	   conferrno = E_MALLOC;
	   if(cuat) odm_free_list(cuat, &cuat_info);
           if(pdat) odm_free_list(pdat, &pdat_info);
	   return(NULL);
	}

	/* copy the pdat info into the cuat struct */

	for(i=0,pdobj=pdat,newobj=newcuat; 
	    i<pdat_info.num; 
	    i++,pdobj++,newobj++)
	{
	  (*how_many)++;

	  /* init the cuat structs with pdat info */
	  strcpy(newobj->name, devname);
	  strcpy(newobj->attribute,pdobj->attribute);
	  strcpy(newobj->value, pdobj->deflt);
	  strcpy(newobj->type, pdobj->type);
	  strcpy(newobj->generic, pdobj->generic);
	  strcpy(newobj->rep, pdobj->rep);
	  newobj->nls_index = pdobj->nls_index;

	  /* check for a cuat object for this attr */ 
	  for(cuatptr=cuat,j=0; j < cuat_info.num; j++,cuatptr++)
	  {
	     if (!strcmp(pdobj->attribute,cuatptr->attribute))
	     {  /* same attr - use the info from CuAt */
	        strcpy(newobj->value, cuatptr->value);
	        strcpy(newobj->type, cuatptr->type);
	        strcpy(newobj->generic, cuatptr->generic);
	        strcpy(newobj->rep, cuatptr->rep);
	        newobj->nls_index = cuatptr->nls_index;
	     }
	  }
	} /* end of for */
		
	/* free up the attr lists */
	if(cuat)odm_free_list( cuat, &cuat_info );
	if(pdat)odm_free_list( pdat, &pdat_info );

	/* return the attr info */
	return(newcuat);

} /* end of getattr */	

/*---------------------------------------------------------------

  putattr()

  Description: 

  Input: cuobj

  Returns: 0 if successful
	   -1 if failed
 
  ----------------------------------------------------------------*/

int
putattr(cuobj)
struct CuAt *cuobj;
{
	struct CuDv	cudv;		/* CuDv object */
	struct CuDv	parcudv;	/* CuDv object */
	struct CuAt	cuat;		/* CuAt object */
	struct PdAt	pdat;		/* PdAt object */
	char   criteria[MAX_CRITELEM_LEN];	/* search criteria string */
	char   cuat_crit[MAX_CRITELEM_LEN];	/* search criteria string */
	int    rc;
 	int    found_cuat;
 
	
	sprintf(cuat_crit, "name = '%s' AND attribute = '%s'", 
				cuobj->name, cuobj->attribute);

 	found_cuat = (int)odm_get_first(CuAt_CLASS, cuat_crit, &cuat);
	if (found_cuat == -1)
	{
		conferrno = E_ODMGET;
		return(-1);
	}
	
	/* need to query CuDv for the uniquetype */

	sprintf(criteria, "name = '%s'", cuobj->name);
 	rc = (int)odm_get_first(CuDv_CLASS, criteria, &cudv);
	if (rc == -1)
	{
		conferrno = E_ODMGET;
		return(-1);
	}
	else if (rc == 0)
 	{
		conferrno = E_NOCuDv;
		return(-1);
	}
		
	sprintf(criteria, "uniquetype = '%s' AND attribute = '%s'", 
			cudv.PdDvLn_Lvalue, cuobj->attribute);
		
 	rc = (int)odm_get_first(PdAt_CLASS, criteria, &pdat);
	if (rc == -1)
	{
		conferrno = E_ODMGET;
		return(-1);
	}
	/* If pdat not found, check for attribute under the */
	/* parents PdAt.                                    */
	else if (rc == 0 && (strcmp(cudv.parent,"")))    
 	{
		sprintf(criteria, "name = '%s'", cudv.parent);
 		rc = (int)odm_get_first(CuDv_CLASS, criteria, &parcudv);
		if (rc == -1)
		{
			conferrno = E_ODMGET;
			return(-1);
		}
		else if (rc == 0)
 		{
			conferrno = E_NOCuDv;
			return(-1);
		}
		sprintf(criteria, "uniquetype = '%s' AND attribute = '%s' AND type = 'E'", 
				parcudv.PdDvLn_Lvalue, cuobj->attribute);
		
 		rc = (int)odm_get_first(PdAt_CLASS, criteria, &pdat);
		if (rc == -1)
		{
			conferrno = E_ODMGET;
			return(-1);
		}
		else if (rc == 0)
		{
			conferrno = E_NOPdOBJ;
			return(-1);
		}
	}
 
	if (strcmp(pdat.deflt, cuobj->value))
	{
		if (found_cuat)		
		{

	/* if the values are not the same, check if there was an  
	   object returned for the cuat_crit; if so, then just
           want to update object */
 
			strcpy(cuat.value, cuobj->value);
			if (odm_change_obj(CuAt_CLASS, &cuat) == -1)
			{
				conferrno = E_ODMUPDATE;		
				return(-1);
			}
		}

	/* need to add a new object to CuAt since the attribute
           value is being changed to a non-default value */

		else 	if (odm_add_obj(CuAt_CLASS, cuobj) == -1)
			{
				conferrno = E_ODMADD;		
				return(-1);
			}
	}

	/* if the attribute value in cuobj is the same as the
 	   default value in the Predefined Attribute object class,
           i.e., the value is being changed back to the default,
           then the customized object is deleted */

	else	if (found_cuat)
			if (odm_rm_obj(CuAt_CLASS, cuat_crit) == -1)
			{
				conferrno = E_ODMDELETE;
				return(-1);
			}
		
	return(0);

} /* end of putattr */

