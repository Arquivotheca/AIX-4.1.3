static char sccsid[] = "@(#)391.5 src/bos/usr/lpp/bosinst/rda/extract.c, bosinst, bos411, 9428A410j 94/03/23 09:33:18";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System installation
 *
 * FUNCTIONS: extract
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 /*********************************************************************/
/* 
 ******************************************************************
 * NAME     : extract
 * 
 * FUNCTION : generate device and attribute stanza files from
 * 	      CuDv and CuAt.
 * 
 * NOTES    :
 * 
 * RETURN CODE : 
 *        0  : success
 *       -1  : failure
 * 
 ****************************************************************** 
 */
#include <stdio.h>
#include <sys/types.h>
#include <cf.h>
#include   <sys/cfgdb.h>
#include   <sys/cfgodm.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

/* Local header files */
/* #include "cfgdebug.h" */
#include "dback.h"

#define OLD_CD_EST 50
#define OLD_CA_EST 10

char	st_ftype[] = "w" ;		/* file type used at open of stanzas */

/* 
 *-------------------------------------------------------
 * The following strings are the search string formats 
 * used to query the databases.
 *-------------------------------------------------------
 */
static char	cuat_sstr[] = "name=%s and generic like *U*" ;

/* 
 *-------------------------------------------------------
 * The following strings (*_form) are used to generate
 * the stanza files
 *-------------------------------------------------------
 */
char    head_form[]  = "\n%s:\n" ;
char    location_form[] = "\tlocation = %s\n" ;
char    paren_form[] = "\tparent = %s\n" ;
char    cwhere_form[]= "\tconnwhere = %s\n" ;
char	pddvln_form[]= "\tPdDvLn = %s\n" ;

char    attr_form[]  = "\t%s = %s\n" ;

int
extract (old_obj_path, dvc_sstr, dv_fname, at_fname)
  char	*old_obj_path ;
  char  *dvc_sstr ;
  char	*dv_fname ;
  char  *at_fname ;
{
    FILE	*dv_file ;
    FILE	*at_file ;


    struct listinfo  old_cd_info ;	/* info for list of old cudv objs. */
    struct listinfo  old_ca_info ;	/* info for list of old cuat objs. */
    struct CuDv	 *old_cd_lst ;		/* ptr to list of old CuDv objects */
    struct CuAt  *old_ca_lst ;		/* ptr to list of old CuAt objects */
    struct CuDv	 *old_cd ;		/* ptr to old CuDv obj being used  */
    struct CuAt	 *old_ca ;		/* ptr to old CuAt obj being used  */
    char	*cur_obj_path ;		/* Path of ODMDIR. */

    int	    rc ;			/* return codes go here 	*/
    int	    i  ;			/* loop control			*/
    int     j  ;			/* loop control			*/
    int	    attr_only;			/* Attribute only flag		*/

    char    sstr[64] ;			/* search criteria string 	*/
    struct Class *CuDv_class;		/* class ptrs for odm stuff */
    struct Class *CuAt_class;		/* class ptrs for odm stuff */

/* BEGIN extract */

    if ((cur_obj_path = getenv("ODMDIR")) == (char *)NULL)
    {
	cur_obj_path = "/etc/objrepos" ;
    }
    dv_file = fopen(dv_fname, st_ftype) ;
    at_file = fopen(at_fname, st_ftype) ;
    if ((dv_file == NULL) ||(at_file == NULL))
    {
	DEBUG_0("extract: error opening stanza files\n")
	return(-1) ;
    }
    DEBUG_0("extract: Init complete, beginning extraction\n");

    /* Get a list of ALL devices in the old cudv database. */

    odm_set_path(old_obj_path) ;
    CuDv_class = odm_mount_class("CuDv");
    CuAt_class = odm_mount_class("CuAt");
    old_cd_lst = odm_get_list(CuDv_class, dvc_sstr, &old_cd_info,
			      OLD_CD_EST, 1) ;
    if ((int)old_cd_lst != -1)
    {
	DEBUG_1("extract: Got %d devices from old CuDv\n", old_cd_info.num)

	if (old_cd_info.num > 0)
	{
	    /* got some objs from old cudv, loop thru them and gen stanza */
	    for (i = 0; i < old_cd_info.num; i++)
	    {
		old_cd = old_cd_lst + i ;
		DEBUG_1("extract: Working with old device '%s'\n", 
			old_cd->name) 
					/* Check for exclusions */

		fprintf(dv_file, head_form, old_cd->name);
		fprintf(dv_file, location_form, old_cd->location) ;
		fprintf(dv_file, paren_form, old_cd->parent) ;
		fprintf(dv_file, cwhere_form, old_cd->connwhere) ;
		fprintf(dv_file, pddvln_form, old_cd->PdDvLn_Lvalue) ;

		/* get list of customized attributes */
		sprintf(sstr, cuat_sstr, old_cd->name) ;
		old_ca_lst = odm_get_list(CuAt_class, sstr, 
					  &old_ca_info, OLD_CA_EST, 1) ;
		if ((int)old_ca_lst != -1)
		{
		    DEBUG_1("extract: Got %d attrs from old CuAt\n",
			    old_ca_info.num)
			if (old_ca_info.num > 0)
			{
						/* Put out header */
			    
			    fprintf(at_file, head_form, old_cd->name) ;
			    for (j = 0; j < old_ca_info.num; j++)
			    {
				old_ca = old_ca_lst + j ;
				 		/* Create stanza for attr */
				fprintf(at_file, attr_form, old_ca->attribute,
					old_ca->value);
			    } /* END for loop through old CuAt objs */

				odm_free_list(old_ca_lst, &old_ca_info) ;
			}
			else
			{
			    /* no attributes customized for this device */
			    DEBUG_0(
			    "extract: No customized attributes for dvc\n")
			}
		}
		else
		{
		    /* ODM error getting old cuat list */
		    DEBUG_0("extract: unable to get old cuat list\n")
		}
	    } /* END loop thru old_cd_list */
	    odm_free_list(old_cd_lst, &old_cd_info) ;
	}
    }

    /* close the stanza files */
    fclose(dv_file) ;
    fclose(at_file) ;

    return(0) ;
} /* END extract */
