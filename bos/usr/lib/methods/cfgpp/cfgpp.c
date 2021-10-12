#ifndef lint
static char sccsid[] = "@(#)36 1.14.1.4 src/bos/usr/lib/methods/cfgpp/cfgpp.c, cfgmethods, bos411, 9428A410j 6/28/94 11:04:46";
#endif
/*
 *   COMPONENT_NAME: (CFGMETHODS) Parallel Printer config method
 *
 *   FUNCTIONS:	build_dds
 *		generate_minor
 *		make_special_files
 *		download_microcode
 *		query_vpd
 *		define_children
 *              device_specific
 *		
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <cf.h>		/* Error codes */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/lpio.h>
#include <sys/ppdd.h>

#include "cfgcommon.h"
#include "cfgdebug.h"

/* external functions */
extern long	*genminor();


#define GETATT( DEST, TYPE, ATTR, CUDV ) {				\
		int rc;							\
		rc = getatt( DEST, TYPE, CuAt_CLASS, PdAt_CLASS,	\
			CUDV.name, CUDV.PdDvLn_Lvalue, ATTR,		\
			(struct attr *)NULL );				\
		if (rc)							\
			return(rc);					\
	}


/*
 * NAME: build_dds
 * 
 * FUNCTION: This function builds the DDS(Defined Data Structure) for the 
 *           Parallel Printer. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * This function operates as a device dependent subroutine called by the 
 * generic configure method for all devices. It is used to build the dds
 * which describes the characteristics of a device to the device driver.
 *
 * NOTES: A pointer to the DDS built and its size are returned to the generic
 *        configure method. 
 *
 * RETURNS: Returns 0 on success, >0 Error code.
 */

int build_dds(lname, ddsptr, ddslen)
char *lname;
uchar **ddsptr;
int *ddslen;
{
	static struct ppdds dds;	/* dds structure */
	char attrval[256];	/* attr value */


	sprintf(dds.name, "%s", lname);

        /*--------------------------------------------------------------*/
        /* Get pp device attributes from attributes DB and store in DDS */
        /*--------------------------------------------------------------*/
	GETATT( &dds.busy_delay, 'i', "busy_delay", cudv )
	GETATT( &dds.v_timeout, 'i', "ptop", cudv )
	GETATT( &dds.pp_lin_default, 'i', "line", cudv )
	GETATT( &dds.pp_col_default, 'i', "col", cudv )
	GETATT( &dds.pp_ind_default, 'i', "ind", cudv )

	/* check ind value: ind >= col: error */
	if (dds.pp_ind_default >= dds.pp_col_default) {
		DEBUG_0("cfgpp: build_dds: ind >= col error.\n")
		return(E_BADATTR);
	}

	GETATT( &attrval, 's', "interface", cudv )
	if (strcmp(attrval, "standard") == 0)
		dds.interface = PPIBM_PC;
	else
		dds.interface = PPCONVERGED;


	/* set up the mode for the printer  */
	dds.modes = 0 ;

	GETATT( &attrval, 's', "plot", cudv )
	if (strcmp(attrval, "yes") == 0)
		dds.modes |= PLOT;

	GETATT( &attrval, 's', "backspace", cudv )
	if (strcmp(attrval, "no") == 0)
		dds.modes |= NOBS;

	GETATT( &attrval, 's', "cr", cudv )
	if (strcmp(attrval, "no") == 0)
		dds.modes |= NOCR;

	GETATT( &attrval, 's', "form", cudv )
	if (strcmp(attrval, "no") == 0)
		dds.modes |= NOFF;

	GETATT( &attrval, 's', "lf", cudv )
	if (strcmp(attrval, "no") == 0)
		dds.modes |= NONL;

	GETATT( &attrval, 's', "addcr", cudv )
	if (strcmp(attrval, "no") == 0)
		dds.modes |= NOCL;

	GETATT( &attrval, 's', "case", cudv )
	if (strcmp(attrval, "yes") == 0)
		dds.modes |= CAPS;

	GETATT( &attrval, 's', "tabs", cudv )
	if (strcmp(attrval, "no") == 0)
		dds.modes |= NOTB;

	GETATT( &attrval, 's', "wrap", cudv )
	if (strcmp(attrval, "yes") == 0)
		dds.modes |= WRAP;

	GETATT( &attrval, 's', "mode", cudv )
	if (strcmp(attrval, "yes") == 0)
		dds.modes |= RPTERR;

	*ddsptr = (char *) &dds;
	*ddslen = sizeof(struct ppdds);

	return(0);
}


/*
 * NAME: generate_minor 
 * 
 * FUNCTION: This function generates device minor number for the Parallel 
 *           Printer. 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to generate device minor number for the specific device by calling
 *      lib function genminor.
 *
 * RETURNS: Returns 0 on SUCCESS, >0 Error code.
 */


long generate_minor(lname, major_no, minor_no)
char *lname;
long major_no;
long *minor_no;
{
	long *minorptr;

	DEBUG_0("generate minor number ...\n")

	minorptr = genminor(lname, major_no, -1, 1, 1, 1);
	if (minorptr == (long *) NULL)
		return E_MINORNO;
	else 
		*minor_no = *minorptr;
	return(0);
}



/*
 * NAME: make_special_file 
 * 
 * FUNCTION: This function creates special file(s) for the parallel printer. 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to create special file(s) for the specific device.
 *
 * (NOTES:) 2 special files will be created for the parallel printer.
 *          for examples: /dev/lp0  and /dev/pp0
 *          both of them have the same major and minor numbers.
 *
 *          Make special files for the parallel printer according to MKNOD_MOD:
 *
 *               file type is S_IFCHR: character file,
 *               file permission is 666.
 *
 * RETURNS: Returns 0 on SUCCESS, >0 Error code.
 */

#define  MKNOD_MODE (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

int make_special_files(lname, devno)
char *lname;
dev_t devno;
{
	int rc;		/* return codes go here */

	/* name of the special file is /dev/ with the logical name as postfix */
	DEBUG_1 ("making special files /dev/%s ...\n",lname)

	if ((rc=mk_sp_file(devno, lname, MKNOD_MODE)) != 0)
		return(E_MKSPECIAL);

	return(0);
}


download_microcode(logical_name)
        char    *logical_name;
{
        return(E_OK);                   /* no microcode */
}

query_vpd(newobj,kmid,devno,vpd)
        char    *newobj;
        mid_t   kmid;
        dev_t   devno;
        char    *vpd;
{
        return(E_OK);                   /* no vpd on printers */
}

define_children(logical_name,phase)
        char    *logical_name;
        int     phase;
{
        return(E_OK);                   /* terminal device */
}

/*
 * NAME     : device_specific
 *
 * FUNCTION : This function allows for device specific code to be
 *            executed.
 *
 * NOTES :
 *      This adapter does not have a special task to do, so this routine does
 *      nothing
 *
 * RETURNS : 0
 */

int device_specific()
{
        return(0);
}

