static char sccsid[] = "@(#)01  1.13  src/bos/usr/lib/methods/cfgdlc/cfggendlc.c, dlccfg, bos411, 9428A410j 10/19/93 09:36:32";
/*
 * COMPONENT_NAME: (DLCCFG) DLC Configuration
 *
 * FUNCTIONS: build_dds, generate_number, make_special_files,
 *              query_vpd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/***************************************************************************
 * DLCCFG:      build_dds, generate_number, make_special_files,
 *              query_vpd
 *
 **************************************************************************/


#include <stdio.h>
#include <errno.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include        <sys/types.h>
#include <sys/mode.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <string.h>
#include "dlcdds.h"
#include "cfgdebug.h"

/* multiplex character special file with rw for all */
#define MKNOD_MODE (S_IFMPX|S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)


/* external functions */

extern  char *malloc();
extern  int *genminor();

extern int              odm_terminate();
extern struct Class    *odm_open_class();
extern int              odm_close_class();
extern void            *odm_get_obj();




/***************************************************************************
 * NAME     : build_dds
 *
 * FUNCTION : This function builds the DDS for dlc adapter
 *
 * EXECUTION ENVIRONMENT :
 *      This function is called by the generic configure method to
 *      build the define data structure which defines the attributes
 *      of the adapter.
 *
 * NOTES :
 *      This function gets the values of the attributes of dlc adapter
 *      from ODM database , assigns the values to the DDS structure
 *      and returns a pointer to the dds and its size.
 *
 * RETURNS :
 *      Returns  0 on success, errno on failure.
 **************************************************************************/


int build_dds(lname,ddsptr,dds_len, devno)
char    *lname;                         /* logical name of the device */
char    **ddsptr;                       /* pointer to dds structure  */
long    *dds_len;                       /* size of dds structure */
dev_t   devno;                          /* device number */
{
	struct dlcconfig  *dlc_dds;     /* pointer to dds structure */
	struct  Class   *cusdev;        /* customized device handle     */
	struct  Class   *preatt;        /* predefined attribute handle  */
	struct  Class   *cusatt;        /* customized attribute handle  */
	struct  CuDv    cusobj ;        /* customized object            */

	char    sstring[512];      /* string to hold search criteria */
	int rc;                         /* return code goes here */
	int     i;                      /* index                        */



	/*
	 * Get Customized Device Object
	 */
	sprintf(sstring,"name = '%s'",lname);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	if(rc == 0)
	{
		DEBUG_0("cfgdlc : CuDv object not found\n" )
		return(E_NOCuDv);
	}
	else if ( rc == -1 )
	{
	/* odm error occured */
		DEBUG_0("cfgdlc : get_obj from CuDv failed\n")
		return(E_ODMGET);
	}



	/* open predefined attribute class */
	preatt = odm_open_class(PdAt_CLASS);
	if((int) preatt == -1)
	{
		DEBUG_0("cfgdlc :cannot open PdAt\n")
		return(E_ODMOPEN);
	}

	/* open customized attribute class */
	cusatt = odm_open_class(CuAt_CLASS);
	if ((int) cusatt == -1)
	{
		DEBUG_0("cfgdlc :cannot open CuAt\n")
		return(E_ODMOPEN);
	}



	/* allocate memory for dlc_dds structure  */
	dlc_dds=(struct dlcconfig *)malloc(sizeof(struct dlcconfig));
	if (dlc_dds == NULL)
	{
		DEBUG_0("cfgdlc: malloc failed\n")
		return(E_MALLOC);
	}

	/* get value of attribute dlc_queue_depth  */
	rc = getatt(&dlc_dds->maxq,'i', cusatt,preatt, lname,
		cusobj.PdDvLn_Lvalue, "queue_depth",
		(struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgdlc : error getting queue depth \n")
		return rc;
	}

	rc = odm_close_class(cusatt);
	if(rc == -1)
	{
		DEBUG_0("cfgdlc: error in closing object class CuAt\n")
		return(E_ODMCLOSE);
	}

	rc = odm_close_class(preatt);
	if(rc == -1)
	{
		DEBUG_0("cfgdlc: error in closing object class PdAt\n")
		return(E_ODMCLOSE);
	}

	rc = odm_terminate();

	/* Establish other attributes */
	/* store the dev_t address */
	dlc_dds->dev = devno;

	*ddsptr = (caddr_t) dlc_dds;            /* Store address of struct*/
	*dds_len = sizeof( struct dlcconfig );  /* Store size of structure*/

#ifdef  CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif  CFGDEBUG

	DEBUG_0("cfgdlc : build_dds successful \n")
	return(0);
}

/**************************************************************************/
/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates minor number for dlc adapter
 *
 * EXECUTION ENVIRONMENT :
 *      This function always returns 0
 *
 * NOTES :
 *      The minor number for dlc adapter is always 0. This is a dummy
 *      function
 * RETURNS :
 *      Returns 0 on success ,  errno 0 on failure.
 *      Stores minor number via last parameter
 */

int
generate_minor(lname,majorno,minordest)
char    *lname;
int     majorno;
long    *minordest;             /* Place to store minor number          */
{
	long    *tmp;           /* return code                          */

	DEBUG_0("generate minor number \n")

	/* call genminor function to generate one minor number */
	tmp = genminor(lname,majorno,-1,1,1,1);

	if( tmp == (long *)NULL )
		return E_MINORNO;

	*minordest = *tmp;

	return 0;

}


/***************************************************************************/

/*
 * NAME     : make_special_files
 *
 * FUNCTION : This function creates special files for dlc adapter
 *
 * EXECUTION ENVIRONMENT :
 *      This function is used to create special files for dlc adapter
 *      It is called by the generic configure method.
 *
 * NOTES :
 *      Logical name of dlc adapter  and device number are passed as
 *      parameters to this function. It checks for the validity of
 *      major and minor numbers, remove any previously created special
 *      file if one  present, create special file and change the
 *      mode of special file.
 *
 * RETURNS :
 *      Returns  0 on success, errno on failure.
 */

int make_special_files(lname,devno)
char    *lname;                 /* logical name of the device  */
dev_t   devno;                  /* device number               */
{

	DEBUG_0("creating special file for 3270\n")
	return(mk_sp_file(devno,lname,MKNOD_MODE));
}

/***************************************************************************/
/*
 * NAME     : query_vpd
 *
 * FUNCTION : This function gets the vital product data from dlc adapter
 *            using syscnfig() system call.
 *
 * EXECUTION ENVIRONMENT :
 *      This function is called by the generic configure method based
 *      on the has_vpd flag in the Predefined Device Object Class for
 *      this device. It uses the sysconfig() system call to get the VPD
 *      information.
 *
 * NOTES :
 *      Kernal module id and device number are passed as parameters to
 *      this function. It and it gets the information using sysconfig()
 *      system call , put the value in newobj and returns newob.
 *
 * RETURNS :
 *      Always returns  0
 */

int
query_vpd(cusobjptr,kmid,devno,vpd)
struct CuDv *cusobjptr;         /* Customized Device object pointer     */
mid_t   kmid;                   /* Kernel module I.D. for Dev Driver    */
dev_t   devno;                  /* Concatenated Major & Minor No.s      */
char    *vpd;                   /* Returned VPD string                  */
{
	/* no vpd for DLC, null function */
	return(0);
}

