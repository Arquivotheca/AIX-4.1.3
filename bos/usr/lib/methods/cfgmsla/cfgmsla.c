static char sccsid[] = "@(#)09	1.7  src/bos/usr/lib/methods/cfgmsla/cfgmsla.c, cfgmethods, bos411, 9428A410j 6/15/90 16:49:18";

/* 
 * COMPONENT_NAME: (CFGMETH) CFGMSLA (configuration method for msla adapter)
 *
 * FUNCTIONS : 	build_dds, generate_minor, make_special_files,
 *		download_microcode, query_vpd, define_children.
 * 
 * ORIGINS : 27 
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <sys/cfgdb.h>

#include <sys/mode.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <string.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"


#define	S_LENGTH	256

/* external functions */

extern	char *malloc();
extern	int *genminor();


/*
 * NAME     : build_dds 
 *
 * FUNCTION : This function builds the DDS for msla adapter 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the generic configure method to 
 *	build the define data structure which defines the attributes
 *	of the adapter.	
 * 
 * NOTES :
 *	This function gets the values of the attributes of msla adapter
 *	from ODM database , assigns the values to the DDS structure
 * 	and returns a pointer to the dds and its size.
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */



int
build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device 	*/
char	**ddsptr;			/* pointer to dds structure 	*/
int 	*dds_len;			/* size of dds 			*/
{
	return(E_OK);
}


/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates minor number for msla adapter
 *
 * EXECUTION ENVIRONMENT :
 *	This function returns 0 
 * 
 * NOTES :
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns 0 
 */

 
int
generate_minor(lname,majorno,minorno)
char 	*lname;				/* logical name of the device 	*/
long 	majorno;				/* major number of the device 	*/
long 	*minorno;			/* minor number of the device 	*/
{

	return(E_OK);

}




/*
 * NAME     : make_special_files 
 *
 * FUNCTION : This function creates special files for msla adapter 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is used to create special files for msla adapter
 *	It is called by the generic configure method. 
 * 
 * NOTES :
 *	For msla adapter there is no special file
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Always returns  0 .
 */

int
make_special_files(lname,devno)
char	*lname;			/* logical name of the device  		*/
dev_t	devno;			/* device number	       		*/
{


	return(E_OK);
}




/*
 * NAME     : download_microcode 
 *
 * FUNCTION : This function download microcode if applicable 
 *
 * EXECUTION ENVIRONMENT :
 *	This function always returns 0
 * 
 * NOTES :
 *	msla adapter does not have microcode. This is a dummy
 *	function
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Always returns 0
 */

int
download_microcode(lname)
char	*lname;			/* logical name of the device */
{
	return (E_OK);
}

/*
 * NAME     : query_vpd 
 *
 * FUNCTION : This function gets the vital product data from msla adapter
 *	      using sysconfig() system call.
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the generic configure method based
 *	on the has_vpd flag in the Predefined Device Object Class for 
 *	this device. It uses the sysconfig() system call to get the VPD
 * 	information. 
 * 
 * NOTES :
 *	Kernal module id and device number are passed as parameters to
 *	this function. It gets the information using sysconfig()
 * 	system call , put the value in newobj and returns newob.
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns 0 
 */

int
query_vpd(newobj,kmid,devno)
struct	CuDv	*newobj;	/* vpd info will be put in that 	*/
mid_t	kmid;			/* kernal module id             	*/
dev_t	devno;			/* device number			*/
{
	return(E_OK);
}


/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines devices attached to this device 
 *
 * EXECUTION ENVIRONMENT :
 *	This function returns 0 on success, < 0 on failure.
 * 
 * NOTES :
 *	msla adapter has two children to define.
 * 
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns 0 on success, > 0 on failure.
 */

int
define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device  		*/
int	ipl_phase;		/* ipl phase				*/
{
	struct	Class	*predev ;	/* predefined devices class ptr */
	struct	Class	*cusdev ;	/* customized devices class ptr */
	struct	PdDv	preobj ;	/* predefined device object 	*/
	struct	CuDv	cusobj ;	/* customized device object 	*/
	char	sstring[S_LENGTH];	/* search criteria 		*/
	int	rc;			/* return code 			*/
	char	*out_p;

	DEBUG_0("define children\n")

	/* open customized devices object class (CusDevices) */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
	{
		/* open failed */
		DEBUG_0("cfgmsla: open class CuDv failed\n")
		return(E_ODMOPEN);
	}
	/* search CusDevices for customized object with this logical 
	   name as parent */
	sprintf(sstring, "parent = '%s'", lname);
	if ((rc = (int)odm_get_obj(cusdev, sstring, &cusobj, TRUE)) == 0) {
		/* odm objects not found */
		DEBUG_0("cfgmsla: object not found\n")
		return(E_FINDCHILD);
	} 
	else if (rc == -1)
	{	DEBUG_1("cfgmsla: couldn't get child of %s\n",lname)
		return(E_ODMGET);
	}
	DEBUG_1("cfgmsla: name of child %s\n",cusobj.name)
	fprintf(stdout,"%s ",cusobj.name);
	
	rc = odm_close_class(cusdev);
	if(rc < 0){
		/* error closing object class */
		DEBUG_0("cfgmsla: close object class CuDv failed\n")
		return(E_ODMCLOSE);
	}
	return(E_OK);
}


