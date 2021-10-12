static char sccsid[] = "@(#)62	1.1  src/bos/usr/lib/methods/cfgtablet_std/cfgtablet_std.c, inputdd, bos41J, 9509A_all 2/14/95 12:56:40";
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: build_dds
 *		define_children
 *		device_specific
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cfgktsm.h"

/*
 * NAME     : build_dds 
 *
 * FUNCTION : This function builds the DDS for the device driver
 *
 * NOTES :
 *	This function gets the values of the attributes for the tablet
 *	from the ODM database , assigns the values to the DDS structure
 * 	and returns a pointer to the dds and its size.
 *
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */

int build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device	*/
char	**ddsptr;			/* pointer to dds structure	*/
int 	*dds_len;			/* size of dds structure	*/
{
	struct	ktsmdds	*dds;	  	/* pointer to tab_dds structure	*/
	char	dev_type[20];

	DEBUG_0("building dds\n")

	/* Save type for later usage */
	strcpy( dev_type, strrchr( cudv.PdDvLn_Lvalue, '/' )+1);

	/* Allocate memory for dds structure  */
	dds = (struct ktsmdds *)malloc(sizeof(struct ktsmdds));
	if(dds == NULL){
		DEBUG_0("malloc failed\n")
		return(E_MALLOC);
	}

	dds->ipl_phase = ipl_phase;
	dds->device_class = TABLET;
	/* set the device type */ 
	if ( !strcmp( dev_type, "6093_m11" ) )
	   dds->device_type = TAB6093M11;
	else if ( !strcmp( dev_type, "6093_m12" ) )
	   dds->device_type = TAB6093M12;
	else {
	   DEBUG_1("unknown dev_type %s\n", dev_type);
	   return( E_WRONGDEVICE );
	}
 
	*ddsptr = (caddr_t)dds;
	*dds_len = sizeof(struct ktsmdds);

#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif	/* CFGDEBUG */

	DEBUG_0("cfgkdd : build_dds successful\n")
	return(0);
}
 
/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates a minor number for the tablet
 *
 * RETURNS :
 * 	0 = success
 *	>0 = errno.
 */

int generate_minor(lname,majorno,minordest)
char 	*lname;			/* logical name of device 		*/
long	majorno;		/* major number of the device		*/
long	*minordest;
{
	long	*minorno;
	DEBUG_0("generate minor\n")
	if((minorno = genminor(lname,majorno,-1,1,1,1)) == (long *)NULL )
		return E_MINORNO;
	*minordest = *minorno;
    free(minorno);

	return 0;
}

/*
 * NAME     : make_special_files 
 *
 * FUNCTION : This function creates a special file for the tablet
 *
 * NOTES :
 *	Logical name of tablet device and device number are passed as
 *	parameters to this function. It checks for the validity of
 *	major and minor numbers, remove any previously created special
 *	file present, and creates a new special file with appropriate 
 *	permission bits.
 * 
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */

int make_special_files(lname,devno)
char	*lname;			/* logical name of the device		*/
dev_t	devno;			/* device number			*/
{
	DEBUG_0(" creating special file for tablet\n")
	return(mk_sp_file(devno,lname,MKNOD_MODE));
}

/*
 * NAME     : download_microcode 
 *
 * FUNCTION : This function download microcode if applicable 
 *
 * NOTES : There is no microcode provision for tablet devices so this 
 *	routine does nothing                                          
 *
 * RETURNS : 0
 */

int download_microcode(lname)
char	*lname;			/* logical name of the device		*/
{
	return (0);
}

/*
 * NAME     : query_vpd 
 *
 * FUNCTION : This function is used to get device specific VPD.
 *
 * NOTES : There is no VPD returned for tablets.
 *
 * RETURNS : 0
 */

int query_vpd(newobj,kmid,devno,vpd_dest)
struct	CuDv	*newobj;	/* vpd info will be put in that 	*/
mid_t	kmid;			/* kernel module id             	*/
dev_t	devno;			/* device number			*/
char	*vpd_dest;		/* Destination for vpd			*/
{
	return 0;
}

/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines devices attached to this device 
 *
 * NOTES :
 *	The tablet has no children so this routine does nothing
 *
 * RETURNS : 0
 */

int define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device 		*/
int	ipl_phase;		/* ipl phase				*/
{
	return 0;
}

/*
 * NAME     : device_specific
 *
 * FUNCTION : This function allows for device specific code to be
 *            executed.
 *
 * NOTES :
 *      This device does not have a special task to do so this routine does
 *      nothing
 *
 * RETURNS : 0
 */

int device_specific()
{
	return 0;
}
