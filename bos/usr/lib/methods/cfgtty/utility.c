#ifndef lint
static char sccsid[] = "@(#)25 1.14 src/bos/usr/lib/methods/cfgtty/utility.c, cfgtty, bos41J, 9521B_all 5/26/95 07:49:31";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Common functions for DDS builds
 *
 * FUNCTIONS: get_attr_list_e, steal_minor
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */
#include <errno.h>          /* standard error numbers */
#include <fcntl.h>          /* logical file system #define */

#include <sys/mode.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */

#include <sys/str_tty.h>

#include "ttycfg.h"
#include "cfgdebug.h"

#include <sys/stat.h>
#include "pparms.h"
#include <ctype.h>
/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define MKNOD_FLAGS   S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define WAG 40                           /* guess on # atributes expected */

/*
 * =============================================================================
 *                       FUNCTIONS USED BY CONFIGURATION METHOD
 * =============================================================================
 * 
 * All these functions are used by the streams based TTY configuration method.
 *
 * =============================================================================
 */

/*
 * NAME: get_attr_list_e
 *                                                                    
 * FUNCTION: Reads in all of the attributes for a device from the customized
 *	     database and predefined database and merges into one list of
 *	     attributes.
 *                                                                    
 *           Modified to support extended attributes. The original copy
 *           resides in cfgtoolsx.c    
 *                                                                    
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is linked into the device specific sections of the 
 *      various config, and change methods.
 *                                                                   
 * NOTES:
 *
 *   struct attr_list *
 *   get_attr_list_e(lname,utype,ptype,how_many,guess_num)
 *                                                                          
 *      lname = device name
 *	utype = device's uniquetype
 *	ptype = parent device's uniquetype
 *      how_many = number of attributes being returned
 *      guess_num = expected number of attributes to be returned
 *                                                                          
 *                                                                          
 * RETURNS:
 *	(struct attr_list *)NULL = error, how_many indicates error code
 *	!=NULL = successful, how_many indicates how many attributes
 *		 were returned
 */  

struct attr_list * get_attr_list_e(lname,utype,ptype,how_many,guess_num)
char    	*lname;         /* device logical name */
char		*utype;		/* device unique type */
char		*ptype;		/* device unique type */
int		*how_many;      /* number of attributes obtained */
int		guess_num;	/* expected number of attributes */
{
	struct  CuAt    *cuat,*par_cuat;
	struct  PdAt    *pdat,*par_pdat;
	struct listinfo pdat_info;
	struct listinfo par_pdat_info;
	struct listinfo cuat_info;
	struct	attr_list	*alist;
	int		i,j;
	char		criteria[256];


	sprintf(criteria, "uniquetype = %s",utype);
	pdat = (struct PdAt *)
		odm_get_list(PdAt_CLASS, criteria, &pdat_info, guess_num, 1);
	if ((int)pdat == -1) {
		*how_many = E_ODMGET;
		return(NULL);
	}

	sprintf(criteria, "uniquetype=%s AND type='E'",ptype);
	par_pdat = (struct PdAt *)
	       odm_get_list(PdAt_CLASS, criteria, &par_pdat_info, guess_num, 1);
	if ((int)pdat == -1) {
		*how_many = E_ODMGET;
		return(NULL);
	}
	if ((pdat_info.num + par_pdat_info.num) == 0) {
		*how_many = E_NOATTR;
		return(NULL);
	}

	/* malloc enough space for same number of CuAt attributes */
	alist = (struct attr_list *)
		malloc(sizeof(int) + (sizeof(struct CuAt) * (pdat_info.num + par_pdat_info.num)));
	if (alist == NULL) {
		*how_many = E_MALLOC;
		return(NULL);
	}

	alist->attr_cnt = pdat_info.num + par_pdat_info.num;

	/* copy the pdat info into the cuat structures */
	for(i=0; i<pdat_info.num; i++) {
		strcpy(alist->cuat[i].name, lname);
		strcpy(alist->cuat[i].attribute, pdat[i].attribute);
		strcpy(alist->cuat[i].value, pdat[i].deflt);
		strcpy(alist->cuat[i].type, pdat[i].type);
		strcpy(alist->cuat[i].generic, pdat[i].generic);
		strcpy(alist->cuat[i].rep, pdat[i].rep);
		alist->cuat[i].nls_index = pdat[i].nls_index;
	}
	if (pdat) free(pdat);


	/* add the extension attributes */
	if (par_pdat_info.num) {
		for(j=0; j < par_pdat_info.num; j++) {
			strcpy(alist->cuat[i].name, lname);
			strcpy(alist->cuat[i].attribute, par_pdat[j].attribute);
			strcpy(alist->cuat[i].value, par_pdat[j].deflt);
			strcpy(alist->cuat[i].type, par_pdat[j].type);
			strcpy(alist->cuat[i].generic, par_pdat[j].generic);
			strcpy(alist->cuat[i].rep, par_pdat[j].rep);
			alist->cuat[i].nls_index = par_pdat[j].nls_index;
			i++;
		}
	}
	if (par_pdat) free(par_pdat);


	*how_many = alist->attr_cnt;

	sprintf(criteria, "name = %s",lname);
	cuat = odm_get_list(CuAt_CLASS, criteria, &cuat_info, pdat_info.num, 1);
	if ((int)cuat == -1) {
		*how_many = E_ODMGET;
		return(NULL);
	}
	if (!cuat_info.num) {
		return(alist);
	}

	/* overlay the pdat data with the cuat data */
	for(j=0; j<cuat_info.num; j++) {
		for(i=0; i<*how_many; i++) {
			if (!strcmp(cuat[j].attribute,alist->cuat[i].attribute)) {
				strcpy(alist->cuat[i].value, cuat[j].value);
			}
		}
	}
	if (cuat) free(cuat);
        DEBUG_0("here\n");
#ifdef CFGDEBUG
        for (i=0; i < *how_many; i++){
        DEBUG_1("get_attr_list_e: name %s\n",alist->cuat[i].name);
        DEBUG_1("get_attr_list_e: attrib %s\n",alist->cuat[i].attribute);
        DEBUG_1("get_attr_list_e: value %s\n",alist->cuat[i].value);
        DEBUG_1("get_attr_list_e: type %s\n",alist->cuat[i].type);
        DEBUG_1("get_attr_list_e: generic %s\n",alist->cuat[i].rep);
        DEBUG_1("get_attr_list_e: nls_index %s\n\n",alist->cuat[i].nls_index);
         }
#endif

	return(alist);
}

/*
 * -----------------------------------------------------------------------------
 *                       STEAL_MINOR
 * -----------------------------------------------------------------------------
 *
 * This function is called by any of the tty/lp specific minor number creation
 * routines when a genminor fails.
 *
 * This function determines if another tty or lp already having the requested
 * minor number was the reason for the failure. If this is the case it attempts
 * to "steal" the minor number by performing an odm_change of the CuDvDr
 * object. If this is successful, then the current tty now owns the minor #.
 *
 * Really the only case where this can happen (and work) is when there are two
 * devices defined for a given port and the device that the system first tries
 * to configure at reboot is not the one that had the entry in  CuDvDr when
 * the system was shutdown.
 *
 * This function returns 0 if successful. In addition, it sets the minP
 * parameter to point to the minor number requested. An odmerrno is returned
 * if the function fails.
 *
 * -----------------------------------------------------------------------------
 */
int steal_minor(char *lname, long major, long minor, long **minP)
{
   char   sstring[256];
   struct CuDvDr dvdrobj;
   static long min;
   int    rc;

   /* find the object that currently has this devno */
   sprintf(sstring,"resource = 'devno' and value1 = '%ld' and value2 = '%ld'",
           major,minor);
   if ((rc = (int)odm_get_obj(CuDvDr_CLASS,sstring,&dvdrobj,ODM_FIRST)) == 0) {

        /* No object */
        DEBUG_1("steal_minor: CuDvDr crit=%s found no objects.\n", sstring);
        return E_NOCuOBJ;

    } else if (rc == -1) {

        /* odm error occurred */
        DEBUG_1("steal_minor: get_obj failed, crit = %s.\n",sstring);
        return E_ODMGET;

    }

    DEBUG_1 ("steal_minor: Conflict is with %s\n",dvdrobj.value3)

    /* determine if the device is a tty or serial printer */
    if ((!strncmp(dvdrobj.value3,"tty",3)) ||
        (!strncmp(dvdrobj.value3,"lp",2)) ) {

        /* the device is a tty/printer; attempt to steal the minor number */
        strcpy (dvdrobj.value3,lname);
        if ((rc=odm_change_obj(CuDvDr_CLASS,&dvdrobj))<0) {
            /* error changing CuDvDr */
            DEBUG_0 ("steal_minor: odm_change_obj failed\n");
            return E_ODMUPDATE;
        }

        /*
         * successfully stole the minor number
         * set minP to point to the minor number for caller to use
         * and return with successful return code
         */
        min = minor;
        *minP = &min;
        return(0);

    }

    /* minor number held by non-tty/non-lp device; return error */
    return E_MINORNO;

}



