static char sccsid[] = "@(#)33	1.8  src/bos/usr/ccs/lib/libdiag/addfrub.c, libdiag, bos41B, bai4 1/9/95 13:53:57";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	addfrub
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */                                                                   

/*
 * FUNCTION:	addfrub()
 * 
 * DESCRIPTION:
 * 
 * This function associates a Bucket of Field Replaceable Units
 * for the device currently being tested; this device is
 * identified by the tm_input object class.
 * 
 * ftype -	indicates the type of FRU Bucket being added to the
 * 		system. The following values are defined.
 * 
 * 		FRUB1	The Field Replaceable Units identify the
 * 			resource that failed, its parent, and any cables
 * 			needed to attach the resource to its parent.
 * 
 * 		FRUB2	Similar to FRUB1, but does not include the
 * 			parent resource.
 * 
 * sn -		source number of the failure.
 * 
 * rcode -	reason code associated with the failure.
 * 
 * rmsg -	message number of the reason code text.
 * 
 * frus -	an array indentifying the Field Replaceable Units
 * 		in the FRU Bucket.
 * 
 * 			struct fru {
 * 				int conf;
 * 				char fname[];
 * 				char floc[];
 * 				short fmsg;
 * 				char  fru_flag;
 * 				char  fru_exempt;
 * 			} frus[MAXFRUS];
 * 
 * 
 * conf -	confidence (probability) associated with the FRU.
 * 
 * fname -	device name or configuration database keyword associated
 * 		with the Field Replaceable Unit that is being reported.
 * 
 * floc  -	location associated with fname
 * 
 * RETURNS:
 * 
 * 	 0	on success,
 * 	-1	on failure.
 * 
 */

#include	<stdio.h>
#include	<sys/types.h>
#include 	"diag/class_def.h"
#include	"diag/da.h"

char		*get_fru_location();

 int
addfrub(frub)
struct fru_bucket *frub;

{
	int		rc=0;
	int		i;
	struct FRUs   	*T_FRUs;
	struct FRUB     *T_FRUB;
	struct listinfo c_info;
	char		buffer[132];

	/* verify this entry is not already present */
	sprintf(buffer, 
      "dname = '%s' and ftype = %d and rmsg = %d and sn = %d and rcode = %d",
		frub->dname, frub->ftype, frub->rmsg,
		frub->sn, frub->rcode);
	T_FRUB = (struct FRUB *)diag_get_list(FRUB_CLASS, 
			buffer, &c_info, 1, 1);
	if (T_FRUB == (struct FRUB *) -1)
		return(-1);

	/* get the date and time */
	getdate(buffer, sizeof(buffer));

	/* if entry is not present - then add it in */
	if (c_info.num != 1)  {

		/* allocate some struct space */
		T_FRUB = (struct FRUB *)calloc(1,sizeof(struct FRUB));
		if (T_FRUB != (struct FRUB *) -1)  {
			strcpy(T_FRUB->dname,frub->dname);
			strcpy(T_FRUB->timestamp,buffer);
			T_FRUB->ftype = frub->ftype; 
			T_FRUB->sn    = frub->sn; 
			T_FRUB->rcode = frub->rcode; 
			T_FRUB->rmsg  = frub->rmsg;  

			rc = diag_add_obj(FRUB_CLASS,T_FRUB);
			free(T_FRUB);
		}
		else
			return(-1);
	}

	for ( i=0; i < MAXFRUS && rc!=-1; i++ )  {
		if (frub->frus[i].conf > 0 )  {
			if ( strlen(frub->frus[i].floc) ) {
				sprintf(buffer, 
   "dname = '%s' and fname = '%s' and floc = '%s' and fmsg = %d and conf = %d and ftype = %d",
					frub->dname, frub->frus[i].fname, 
					frub->frus[i].floc,
					frub->frus[i].fmsg,
					frub->frus[i].conf, frub->ftype);
			}
			else {
				sprintf(buffer, 
   "dname = '%s' and fname = '%s' and fmsg = %d and conf = %d and ftype = %d",
					frub->dname, frub->frus[i].fname, 
					frub->frus[i].fmsg,
					frub->frus[i].conf, frub->ftype);
			}
			T_FRUs = (struct FRUs *)diag_get_list(FRUs_CLASS,
					buffer, &c_info, 1, 1);
			if (T_FRUs == (struct FRUs *) -1)
				return(-1);
			if (c_info.num != 1)
				if ( (rc = add_frus(&frub->frus[i], 
						    frub->ftype,
						    frub->dname)) == -1 )
					break;
		}
	}

	return((rc >= 0) ? 0 : -1);
}

add_frus( frus, ftype, dname )
fru_t          		*frus;
short			ftype;
char 			*dname;
{
	int		rc;
	struct FRUs   	*T_FRUs;
	char		*location;

	T_FRUs = (struct FRUs *) calloc(1,sizeof(struct FRUs));
	if (T_FRUs != (struct FRUs *) -1) {
		strcpy(T_FRUs->dname,dname);
		strcpy(T_FRUs->fname, frus->fname);

		/* if a location is provided - use it */
		if( strlen(frus->floc) )
			strcpy(T_FRUs->floc, frus->floc);

		/* else if device is in database - get its location */
		else if ( frus->fru_flag != NOT_IN_DB ) {
			if ( (location = get_fru_location(T_FRUs->fname)) == 
								(char *)-1)
				return(-1);
			else
				strcpy(T_FRUs->floc, location);
		}
						
		T_FRUs->ftype = ftype; 
		T_FRUs->conf = frus->conf; 
		T_FRUs->fmsg = frus->fmsg;
		rc = diag_add_obj(FRUs_CLASS,T_FRUs);
		free(T_FRUs);
	}
	else
		return(-1);
	return(rc);
}

char *
get_fru_location(fru_name)
{
	struct CuDv   		*T_CuDv;
	struct listinfo 	c_info;
	char			crit[NAMESIZE + 16];

	sprintf(crit, "name = '%s'", fru_name);
	T_CuDv = (struct CuDv *)diag_get_list(CuDv_CLASS, crit,
			&c_info, 1, 1);
	if (T_CuDv == (struct CuDv *) -1)
		return((char *)-1);
	else
		return(T_CuDv->location);
	
}
