static char sccsid[] = "@(#)89  1.24  src/bos/usr/lib/methods/common/ddstools.c, lftdd, bos411, 9428A410j 2/14/94 08:08:46";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: add_vpd
 *		build_depend
 *		define_child
 *		get_attrval
 *		get_conn
 *		get_devno
 *		get_instance
 *		get_vpd_descriptor
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"
#include <stdio.h>
#include <strings.h>

/*
 * NAME: get_conn
 *                                                                    
 * FUNCTION: 
 *
 *    The purpose of this routine is to obtain the connection location
 *    for a device from the customized device object class.
 *
 * RETURNS:
 *
 *    0 on success
 *    positive return code on failure 
 */
int
get_conn(lname, value)
char  *lname;      /* logical name to search for */
long  *value;      /* integer pointer for returning all integers */
{
char		crit[256];   /* used to build search criteria */
struct CuDv 	cudv;        /* will point to the customized device object */
int		rc;          /* return code */
char		*slot;

	/*
	 * Load up search criteria string and get the indicated object from ODM
	 */
	sprintf(crit, "name='%s'", lname);
	if( (rc = (int)odm_get_first(CuDv_CLASS, crit, &cudv)) == 0 )
	{
		DEBUG_1("get_conn: can't find device object %s", lname)
		return(E_NOCuDv);
	}
	else if( rc == -1 )
	{
		DEBUG_1("get_conn: error getting device object %s", lname)
		return(E_ODMGET);
	}

	/*
	 * Search the connection string for = (i.e. slot=1).  If an =
	 * was found, extract the connection from the remaining string,
	 * otherwards extract the connection from the original string.
	 */
	slot = index(cudv.connwhere, '=');
	if( slot == NULL )
		*value = atoi( cudv.connwhere ) - 1;
	else
		*value = atoi( slot + 1 ) - 1;

	return(E_OK);
}




/*
 * NAME: get_attrval
 *                                                                    
 * FUNCTION:
 *   get_attrval is used to obtain an attribute object from ODM by searching for
 *   logical name and the attribute name as a pair.  If an object is found with
 *   the given search string, the attribute value is obtained from the object
 *   depending on the values type.  If an error occurs, it is reported to the
 *	 calling routine.
 *                                                                    
 *                                                                    
 * RETURNS: The "str" parameter will contain the value of the attribute
 *			if it is of string type and "value" will contain the value
 *			if it is of integer type. "rc" contains the return code.
 *			The function itself returns a pointer to a customized attribute
 *			object.
 */

struct CuAt *
get_attrval(lname, attrname, str, value, rc)
char  *lname;      /* logical name to search for */
char  *attrname;   /* attribute name to search for */
char  *str;        /* string pointer for returning strings */
ulong *value;      /* integer pointer for returning all integers */
int		*rc;       /* return code */
{
struct CuAt 	*cuat;		/* storage to get customized attribute into */
int	   how_many;			/* argument to getattr() routine */

	*rc = E_OK;

	/*
	 * Call generic getattr routine which will search for the
	 * attribute value in customized first, then if not found,
	 * will search predefined for the attribute value.
	 */
	if( (cuat = getattr(lname, attrname, FALSE, &how_many)) == NULL )
	{
		DEBUG_1("get_attrval: can't get attribute object %s\n", attrname)
		*rc = E_NOATTR;
		return(cuat);
	}

	/*
	 * Error check for improper representation.  If we want a non string
	 * and a string is in the object, fail.
	 */
	if( str == NULL && cuat->rep[0] == 's' )
	{
		DEBUG_0("get_attrval: can't read string attribute into an integer\n")
		*rc = E_BADATTR;
		return(NULL);
	}

	/*
	 * Extract the proper value into the required pointer field for return
	 */
	if( cuat->rep[0] == 's' )
		strcpy(str, cuat->value);    /* get string into str */
	else
		/* get long into value */
		*value = (ulong)strtoul(cuat->value, (char **)NULL, 0);

	return(cuat);
}



/*
 * NAME: get_devno
 *                                                                    
 * FUNCTION:
 *                                                                    
 *    This routine is used to create a device number for the given
 *    logical name using the major/minor number form the devices
 *    customized device entry.  The devno will be created only
 *    if the device is currently available to the system.
 *
 * RETURNS:
 *
 *    0   : success
 *    positive return code on failure
 */
get_devno(lname, devno)
char	*lname;
dev_t	*devno;
{
char		crit[256];   /* used to build search criteria */
struct CuDv	cudv;        /* will point to the customized device object */
struct CuDvDr cudvdr;	 /* customized device driver, for major/minor # */
int		rc;          /* return code */

	*devno = -1;

	/*
	 * Load up search criteria string and get the indicated object from ODM
	 */
	sprintf(crit, "name='%s'", lname);
	rc = (int)odm_get_first(CuDv_CLASS, crit, &cudv);
	if ( rc == 0 )
	{
		DEBUG_1("get_devno: can't find device object %s\n", lname)
		return(E_NOCuDv);
	}
	else if( rc == -1 )
	{
		DEBUG_1("get_devno: error getting device object %s\n", lname)
		return(E_ODMGET);
	}

	/*
	 * make the devno from the major and minor numbers in the customized
	 * device driver object
	 */
	if( cudv.status == AVAILABLE )
	{
		sprintf(crit, "value3 = '%s' AND resource = devno", lname);
		rc = (int)odm_get_first(CuDvDr_CLASS, crit, &cudvdr);
		if( rc == 0 )
		{
			DEBUG_1("get_devno: can't find cudvdr object crit=%s\n", crit)
			return(E_NOCuOBJ);
		}
		else if( rc == -1 )
		{
			DEBUG_1("get_devno: error getting cudvdr object crit=%s\n", crit)
			return(E_ODMGET);
		}
		*devno = makedev((ulong)strtoul(cudvdr.value1, (char **)NULL, 0), 
				 (ulong)strtoul(cudvdr.value2, (char **)NULL, 0));
		return(E_OK);
	}

	DEBUG_1("get_devno: device object %s not configured\n", lname)
	return(E_DEVSTATE);
}




/*
 * NAME: build_depend
 *                                                                    
 * FUNCTION:
 *                                                                    
 *    This routine is used to create a customized dependency object
 *    and add the object into ODM.

 * RETURNS:
 *
 *    0    : success
 *   positive return code on failure
 */
build_depend(depon, dep)
char	*dep,
*depon;
{
struct CuDep	cudep;
char		*malloc();
char		crit[256];
int		rc;

	/*
	 * Set up search criteria string and search CuDep object class for
	 * the specific object matching our dependency.  If our dependency
	 * does not exist, build a new object and place it into the database.
	 */
	sprintf(crit, "name='%s' AND dependency='%s'", dep, depon);
	if( (rc = (int)odm_get_first(CuDep_CLASS, crit, &cudep))
	    == NULL )
	{
		strcpy(cudep.name, dep);
		strcpy(cudep.dependency, depon);

		if( (rc = odm_add_obj(CuDep_CLASS, &cudep)) < 0 )
		{
			DEBUG_0("Error adding object to ODM in add dependency\n")
			return(E_ODMADD);
		}
	}
	else if( rc == -1 )
	{
		DEBUG_0("Error getting customized dependency object\n")
		return(E_ODMGET);
	}

	return(E_OK);
}



/*
 * NAME: get_instance
 *
 * FUNCTION:
 *
 *    This routine is used to obtain the instance of a display.  The instance
 *    will be determined by looking at how many times the device driver
 *    for the particular display has been used in the current configuration.
 *    Each time a new display of a certain type is configured, the device
 *    driver usage field is incremented by the load command and therefore
 *    reflects the current instance of the display.
 *
 * RETURNS:
 *
 *    0 : success
 *    positive return code on success
 */
int
get_instance(lname, value)
char  *lname;      /* logical name to search for */
long  *value;      /* integer pointer for returning all integers */
{
char		crit[256];   /* used to build search criteria */
struct CuDv	cudv;        /* will point to the customized device object */
struct CuDvDr	cudvdr;      /* will point to the customized device object */
struct PdDv	pddv;        /* will point to the customized device object */
int		rc;	     /* return code from functions */

	/*
	 * Search Customized device class using logical device name
	 */
	sprintf(crit, "value3='%s'", lname);
	if( (rc = (int)odm_get_first(CuDvDr_CLASS, crit, &cudvdr)) == 0)
	{
		DEBUG_1("build_dds: can't find device object %s", lname)
		return(E_NOCuOBJ);
	}
	else if( rc == -1 )
	{
		DEBUG_1("build_dds: error getting device object %s", lname)
		return(E_ODMGET);
	}


	/*
	 * get long into value 
	 */
	*value = (ulong)strtoul(cudvdr.value2, (char **)NULL, 0);

	return(E_OK);
}



/* 
 * Search string templates 
 */
#define	SSTRING1  "type = '%s'"
#define	SSTRING2  "name = '%s'"
#define	SSTRING3  "uniquetype = '%s'"
#define	SSTRING4  "uniquetype = '%s' AND connkey = '%s'"

/*
 * NAME: define_child
 *
 * FUNCTION:
 *
 * RETURNS:
 *
 *     0 : success
 *     positive return code on failure
 */
char *
define_child(type, parent, vpd)
char		*type,
		*parent,
		*vpd;
{
char	*lname;
int	seqno;
int	child_location;
int     location_len;                   /* location string length */
char    sstring[256];                   /* search string */
struct	PdDv	pddv, par_pddv;		/* predefined device class structure */
struct  PdCn    pdcn;                   /* predefined connection class structure */
struct	CuDv	cudv, par_cudv;		/* customized device class structure */
int	rc;

        DEBUG_0 ("entering define_child()\n");

	/* 
	 * Get predefined device object for this device 
	 */
	sprintf(sstring,SSTRING1,type);
	rc = (int)odm_get_first(PdDv_CLASS, sstring, &pddv);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get predefined device object\n")
		return(E_VPD);
	}

	/* 
	 * Get customized device object for this devices parent 
	 */
	sprintf(sstring,SSTRING2, parent);
	rc = (int)odm_get_first(CuDv_CLASS, sstring, &par_cudv);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get customized device parent\n")
		return(E_VPD);
	}

	/* 
	 * Get predefined device object for this devices parent 
	 */
	sprintf(sstring,SSTRING3, par_cudv.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS, sstring, &par_pddv);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get predefined device parent\n")
		return(E_VPD);
	}

	/* 
	 * Get predefined connection object for this device 
	 */
	sprintf(sstring,SSTRING4, par_cudv.PdDvLn_Lvalue, pddv.subclass);
	rc = (int)odm_get_first(PdCn_CLASS, sstring, &pdcn);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get predefined connection object\n")
		return(E_VPD);
	}

	lname = (char *)malloc(15);
	sscanf(par_cudv.name+strlen(par_pddv.prefix), "%d", &seqno);
	sprintf(lname, "%s%d", pddv.prefix, seqno);

	/* 
	* check if device has already been defined 
	*/ 

	sprintf(sstring, "name = '%s'", lname );
	rc = (int)(odm_get_first(CuDv_CLASS, sstring, &cudv));
	if ( rc == -1 )
	{
		DEBUG_0("error in odm_get_first\n");
		return(E_VPD);
	}

	/* device was previously defined, update if necessary */
	if (rc){
		/*  update chgstatus */ 
		if (cudv.chgstatus == MISSING ) {
			cudv.chgstatus = SAME; 
			strcpy(cudv.PdDvLn_Lvalue, pddv.uniquetype);
			if ((rc = odm_change_obj(CuDv_CLASS,&cudv)) < 0) {
				DEBUG_0("odm_change_obj failed\n");
				return(E_VPD);
			}
		}
		/* update vpd if necessary */
		if ( strcmp(vpd, "") != 0 )
			add_vpd(lname, vpd);
	}
	/* device needs to be defined */
	else
	{
		strcpy(cudv.name,lname);
		cudv.status = DEFINED;
		cudv.chgstatus = pddv.chgstatus;
		strcpy(cudv.ddins,pddv.DvDr);
		if (!strcmp(par_pddv.type,"mouse"))
			cudv.location[0] = '\0';
		else /* device must be pgr, pop, mrv, mev, mde or mzb */
		{
			/*
			  Obtain location of parent and adjust it
			  for daughter card locations.
			*/
			location_len = strlen(par_cudv.location) - 1;
			sscanf(par_cudv.location + location_len,
					"%d", &child_location);
			if ( !strcmp( pddv.type, "pgr" ) ||
                             !strcmp( pddv.type, "pge" ) ||
                             !strcmp( pddv.type, "pgt" ) )

				child_location++;
			else if ( !strcmp( pddv.type, "pop" ) )
				child_location += 2;

#if 0
			/* For the future LEGA+                 */
			else if ( strcmp( pddv.type, "ppz" )  &&
#endif

			else if ( strcmp( pddv.type, "ppc" ) )
				child_location--;
			strncpy(cudv.location, par_cudv.location,
							location_len);
			cudv.location[location_len] = '\0';
			sprintf(cudv.location, "%s%d", cudv.location,
							child_location);
			/*
			 * If the card is an mev, mde or mzb for the sabine then the
			 * location code will consist of the connection location of
			 * the card concatenated to the sabine mrv location code.
			 */
			if ((!strcmp(pddv.type, "hiprfmev"))  ||
				(!strcmp(pddv.type, "hiprfmde"))  ||
				(!strcmp(pddv.type, "hiprfmzb")))
				sprintf(cudv.location, "%s-0%s",
					cudv.location, pdcn.connwhere);

			/*
			 * If the card is a ppc for Gt4 then the
			 * location code will consist of the connection
			 * location of the card concatenated to the ppr
			 * location
			 */

#if 0
			/* For the future LEGA+                 */
			if ( ( !strcmp( pddv.type, "ppz" ) )  ||
#endif

			if ( !strcmp( pddv.type, "ppc" ) )
				sprintf(cudv.location, "%s-0%s",
					cudv.location, pdcn.connwhere);

		}
		strcpy(cudv.parent, par_cudv.name);
		strcpy(cudv.connwhere, pdcn.connwhere);
		strcpy(cudv.PdDvLn_Lvalue, pddv.uniquetype);

		/* 
		 * Add customized object to customized devices object class 
	 	 */
		if( (rc = odm_add_obj(CuDv_CLASS, &cudv)) < 0 ) 
		{
			DEBUG_0("define_children: odm_add_obj of cudv failed\n")
			return(E_VPD);
		}

		DEBUG_1("define_children: %s has been defined\n", lname)
		if ( strcmp(vpd, "") != 0 )
			add_vpd(lname, vpd);
	}

        DEBUG_0("leaving define_child().\n");
  
	return(E_OK);
}


/*
 * NAME: get_vpd_descriptor
 *
 * FUNCTION: Reads a descriptor from the VPD for a device
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function is used to decode VPD which is stored in the format
 *      used by the system devices ( CPU's, Planars etc. )
 *
 * NOTES:
 *      VPD is stored as a series of descriptors, each of which
 * is encoded as follows:
 *
 * Byte 0 = '*'
 * Byte 1,2 = mnemonic          ( E.g. "TM", "Z1", etc )
 * Byte 3 = Total length / 2
 * Byte 4.. = data
 *
 *  E.g.:  Byte#     0    1    2    3    4    5    6    7    8    9
 *         Ascii    '*'  'Z'  '1'       '0'  '1'  '2'  '0'  '0'  '1'
 *         Hex       2A   5A   31   05   30   31   32   30   30   31
 *         Oct      052  132  061  005  060  061  062  060  060  061
 *
 * RETURNS:
 *
 *      A pointer to the static char array "result" is returned, with
 *      the array being empty if the descriptor was not present in the
 *      VPD passed in.
 */
char *
get_vpd_descriptor( vpd, name )
register char 	*vpd;
char 		*name;
{
static char	result[256];
register char	*res_ptr;
register int	bytecount;

        res_ptr = result;
        *res_ptr = '\0';

        vpd = index(vpd, '*');

        while( *vpd == '*' )
        {
                if( ( vpd[1] == name[0] ) && ( vpd[2] == name[1] ) )
                {
                        /* 
			 * This is the correct descriptor 
			 */
                        bytecount = ((int)vpd[3] << 1 ) - 4;

                        vpd += 4;

                        while( bytecount-- )
                                *res_ptr++ = *vpd++;

                        *res_ptr = '\0';
                }
                else
                        /* 
			 * Skip to next descriptor 
			 */
                        vpd += ( (int)vpd[3] << 1 );
        }

        return(result);
}


/*
 * NAME: add_vpd
 *
 * FUNCTION:
 *
 * RETURNS:
 *
 *    0 : success
 *    positive return code on failure
 */
int
add_vpd(lname, vpd)
char *lname;
char *vpd;
{
struct	CuVPD	cuvpd;				/* customized VPD class structure */
char	sstring[256];				/* search string */
int	rc;
	
	/* update VPD if necessary */

	sprintf(sstring,"name = '%s' AND vpd_type = '%d'", lname, 0);
	rc = (int)odm_get_first(CuVPD_CLASS, sstring, &cuvpd);
	if ( rc == -1 ) 
	{
		DEBUG_0("failed to get customized VPD \n")
		return(E_VPD);
	}

	if ( rc == 0 )
	{
		strcpy(cuvpd.name, lname);
		cuvpd.vpd_type = 0;
		strcpy(cuvpd.vpd, vpd);
		if( (rc = odm_add_obj(CuVPD_CLASS, &cuvpd)) < 0 ) 
		{
			DEBUG_0("odm_add_obj of CuVPD failed\n")
			return(E_VPD);
		}
	}
	else
	{
		if ( strcmp(cuvpd.vpd, vpd) != 0 )
		{
			strcpy(cuvpd.vpd, vpd);
			if ((rc = odm_change_obj(CuVPD_CLASS,&cuvpd)) < 0)
			{
				DEBUG_0("odm_change_obj of CuVPD failed\n")
				return(E_VPD);
			}
		}
	}

	return(E_OK);
}

