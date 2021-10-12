static char sccsid[] = "@(#)38	1.9  src/bos/usr/ccs/lib/libdiag/frus.c, libdiag, bos41J, 9521A_all 5/23/95 10:23:02";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: insert_frub, get_parent_fru, adjust_frus
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	"diag/class_def.h"
#include	"diag/da.h"
#include	"diag/tm_input.h"
#include	"diag/diagresid.h"

/*
 * NAME: (insert_frub) Get fru's from the odm, adjust
 *
 * FUNCTIONS: Designed to get FRU's from the odm, adjust
 * 	      the FRU percentage if necessary.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment wich is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: 0 passed, -1 failure.
 */
 
#define get_cstm(VAR) 							   \
{									   \
		sprintf(crit, "name = %s",/**/VAR );			   \
		cudv = (struct CuDv *)diag_get_list(CuDv_CLASS,crit,   \
				&obj_info,1,2);	   			   \
		if ( ( cudv == (struct CuDv *) -1  ) 			   \
			|| ( cudv == (struct CuDv *) NULL) )		   \
			return( -1 );					   \
}

short	not_found = 0;
int	insert_frub(tm_input,frub)
struct	tm_input	*tm_input;
struct	fru_bucket	*frub;
{
	int			i;
	char			crit[40];
	char	new_parent[16];
	struct	listinfo	obj_info;
	struct	CuDv		*cudv;
	struct	CuAt		*cuat_ptr;
	int	adjust_frus_needed=0;
	int	bc;

	/* get dname's entry in database and put led value into frub->sn */
	get_cstm(tm_input->dname);
	frub->sn = cudv->PdDvLn->led;
	if( cudv->PdDvLn->led == 0 )
		/* No LED number in PdDv so check for type Z */
		/* attribute                                 */
		if ((cuat_ptr = (struct CuAt *)getattr(cudv->name,
		       "led",0,&bc)) != (struct CuAt *)NULL)
			   frub->sn = (short)strtoul(cuat_ptr->value,NULL,0);

	for ( i=0; i<MAXFRUS; i++ )
		switch (frub->frus[i].fru_flag)
		{
			case DA_NAME:
				get_cstm(tm_input->dname);
				if(cudv->PdDvLn-> fru == SELF_FRU )
				{
					strncpy(frub->frus[i].fname,
						tm_input->dname, NAMESIZE );
					strcpy(frub->frus[i].floc,
					       cudv->location);
					break;
				}
				if(cudv->PdDvLn-> fru == PARENT_FRU )
				{
					if(get_parent_fru(i,frub,cudv->parent)
								!=0)
					{
						/* see if this is a ttydevice */
						/* attribute like dials/lpfks */
						/* if so, use the new parent  */
						/* name found.		      */
						if(found_new_parent
							     (tm_input->dname,
							     new_parent))
							if(get_parent_fru(i,
								  frub,
								  new_parent)
								      != 0)
								return(-1);
						else
							return(-1);
					}
					break;
				}
				if( cudv->PdDvLn-> fru == 3 )
				{
					if(determine_fru(tm_input->dname,
						cudv->parent,
						i, frub, cudv->location,
						&adjust_frus_needed) != 0)
							return(-1);
							
				}
				else
					return(-1);
				break;

			case PARENT_NAME:

				if ( *tm_input->parent == '\0')
				{
					/* If device does not have parent */
					/* and a parent is found for the  */
					/* device having the ttydevice    */
					/* attribute, use the new parent  */
					if(found_new_parent(tm_input->dname,
						   new_parent))
					{
						get_cstm(new_parent);
					} else
						return ( -1 );
				} else
					get_cstm(tm_input-> dname);

				if ( cudv->PdDvLn->fru == PARENT_FRU )
				{ 
					get_cstm(cudv->parent);
					break;
				}
				if (get_parent_fru(i,frub,cudv->parent) != 0)
				{
					/* see if this is a ttydevice */
					/* attribute like dials/lpfks */
					/* if so, use the new parent  */
					/* name found.		      */
					if(found_new_parent(tm_input->dname,
						     new_parent))
						if(get_parent_fru(i,frub,
							  new_parent) != 0)
							return(-1);

				}
				break;
			case NO_FRU_LOCATION:
				strcpy(frub->frus[i].floc,tm_input->dnameloc);
				break;

			default:
				break;
		}
	if ((not_found == TRUE) || (adjust_frus_needed == TRUE))
		(void)adjust_frus( frub ) ;
	return(0);
}

/*
 * NAME: found_new_parent
 *
 * FUNCTION: search for ttydevice attribute, then take the value
 *           found as the new device name, then the parent
 *	     of the new device  as the parent.
 *
 * RETURNS: 1 if found
 *          0 if not found
 *
 */

int found_new_parent(char *devname,
	char *new_parent)
{
        struct listinfo cuat_info;
        struct listinfo cudv_info;
	char	criteria[512];
	struct  CuAt            *cuat_obj;
	struct  CuDv            *cudv;
	int	i, found=0;

	sprintf(criteria, "name = %s and attribute = ttydevice",
			devname);
        cuat_obj = (struct CuAt *)diag_get_list(CuAt_CLASS, 
		criteria, &cuat_info, 1, 1 );
	if( (cuat_obj == (struct CuAt *)-1)
		|| ( cuat_obj == (struct CuAt *) NULL) )
		return (0);

	if(cuat_info.num > 0)
	{
		sprintf(criteria, "name = %s",cuat_obj->value);
		cudv = (struct CuDv *)diag_get_list(CuDv_CLASS,criteria,
				&cudv_info,1,2);
		if ( ( cudv != (struct CuDv *) -1  )
			&& ( cudv != (struct CuDv *) NULL) )
		{
			found=1;
			strcpy(new_parent, cudv->parent);
			diag_free_list(cudv, &cudv_info);
		}
	}
	diag_free_list(cuat_obj, &cuat_info);
	return(found);
}


/*
 * NAME: (fru_in_frub) see if fru is already filled in the frub
 *
 * FUNCTION: Search a frub, to see if the fru name is already filled
 *		in.
 *
 * RETURNS: 0 Not found
 *          1 found
 *
 */

int fru_in_frub(char *dname,
struct	fru_bucket	*frub)
{
	int	index;

	for ( index = 0; index < MAXFRUS; index++ )
		if(!strcmp(frub->frus[index].fname, dname))
			return(1);

	return(0);
}

/*
 * NAME: (determine_fru) see if fru is integrated. If so use
 *		parent as fru if possible.
 *
 * FUNCTIONS: fru determination based on integrated bit.
 *
 * RETURNS: -1 failure, 0 passed.
 *
 */

int determine_fru(char *dname, 
	char *parent,
	int  frub_index,
	struct	fru_bucket *frub,
	char *location,
	int *frus_need_adjustment)
{
	long	flags;

	*frus_need_adjustment=0;

	/* First see if device is integrated */
 	if( (diag_get_device_flag(dname, &flags)) != -1);
	{
		if( flags & INTEGRATED )
		{
			if(get_parent_fru(frub_index,frub, parent) !=0)
				return(-1);
			*frus_need_adjustment=1;
		} else /* no adjustment needed */
		{
			strncpy(frub->frus[frub_index].fname, dname, NAMESIZE );
			strcpy(frub->frus[frub_index].floc, location);

		}
	}
	return(0);
}

/*
 * NAME: (get_parent_fru) get parent device FRU name.
 *
 * FUNCTIONS: Designed to get parent FRU's from the odm
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: 0 passed, -1 failure.
 */
 

int get_parent_fru(index, frub, parent)
int			index;
struct	fru_bucket	*frub;
char			*parent;
{
	char			crit[40];
	struct	listinfo	obj_info;
	struct	CuDv		*cudv;

	get_cstm(parent);

	while ( *frub-> frus[index].fname == '\0')
	{
		switch ( cudv->PdDvLn->fru )
		{
			case PARENT_FRU :
				get_cstm(cudv-> parent);
				break;
			case SELF_FRU :
				if(!fru_in_frub(cudv->name, frub))
				{
					strncpy(frub->frus[index].fname,
						cudv->name, NAMESIZE );
					strcpy(frub->frus[index].floc,
						cudv->location);
				} else
					return(0);
				break;
			default:
				not_found = TRUE;
				return(0);			
		}

	}
	return (0);
}

/*
 * NAME: (adjust_frus) adjust FRU's percentage if necessary.
 *
 * FUNCTIONS: Designed to adjust FRU's percentage if FRUs was
 *	      not found in the data base.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment wich is described
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS: 0 passed, -1 failure.
 */
 
int	adjust_frus( frub )
struct	fru_bucket	*frub;
{
	int	index;
	int	new_conf = 0;
	int	exempt_index = -1 ;
	
	/* Assumption: First fru found has fru_exempt flag    */
	/* set to NONEXEMPT. Save the index to this fru.      */
	/* Take the confidence level of the next EXEMPT fru   */
	/* add it to the first one, then clear the confidence */
	/* level of the EXEMPT fru.			      */
	/* For example: fru[0] = scsi0, NONEXEMPT, 80%        */
	/*              fru[1] = ioplanar0 EXEMPT, 20%        */
	/* After adjusting the frus:			      */
	/*              fru[0] = ioplanar0, NONEXEMPT, 100%   */
	/*              fru[1] = ioplanar0, EXEMPT, 0%        */
	/* I've put the name in for clarity, actually, the    */
	/* fru names are going to be both ioplanar0, coming   */
	/* into this routine, as a result of determine_fru.   */

	for ( index = 0; index < MAXFRUS; index++ )
	{
		if ( ( frub-> frus[index].fru_exempt == NONEXEMPT ) && 
			( exempt_index == -1 ) )
			exempt_index = index;

		if ( ( frub-> frus[index].fru_exempt == EXEMPT ) &&
			( *frub-> frus[index].fname == '\0' ) )
		{
			new_conf += frub-> frus[index].conf ;
			frub-> frus[index].conf = 0 ;
		}
	}
	if (exempt_index != -1)
		frub-> frus[exempt_index].conf += new_conf ;
	return(0);
}

