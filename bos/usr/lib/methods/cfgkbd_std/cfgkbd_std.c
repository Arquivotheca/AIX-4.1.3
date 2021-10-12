static char sccsid[] = "@(#)58  1.2  src/bos/usr/lib/methods/cfgkbd_std/cfgkbd_std.c, inputdd, bos41J, 9510A_all 3/7/95 09:55:55";
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
 * FUNCTION : This function builds the DDS for device driver
 *
 * NOTES :
 *	This function gets the values of the attributes for the keyboard
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
	static  struct	ktsmdds	*dds;  	/* pointer to kbd_dds structure	*/
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
	dds->device_class = KBD;
	/* set the device type */ 
	if ( !strcmp( dev_type, "kb101" ) )
	   dds->device_type = KS101;
	else if ( !strcmp( dev_type, "kb102" ) )
	   dds->device_type = KS102;
	else if ( !strcmp( dev_type, "kb106" ) )
	   dds->device_type = KS106;
	else if ( !strcmp( dev_type, "ps2" ) )
	   dds->device_type = KSPS2;
	else {
	   DEBUG_1("unknown dev_type %s\n", dev_type);
	   return( E_WRONGDEVICE );
	}
 
    /* copy logical name to dds */
    strcpy(dds->logical_name, lname);

	GETATT( &dds->typamatic_delay, 'i', cudv, "typamatic_delay", NULL )
	GETATT( &dds->typamatic_rate,  'i', cudv, "typamatic_rate",  NULL )
	GETATT( &dds->volume,          'c', cudv, "volume",          NULL )
	GETATT( &dds->click,           'c', cudv, "click",           NULL )

	/* "map" attr exists on 106 key or ps2 keyboard only */
	/* so, if it exists then return it else set to zero  */
	if( getatt( &dds->map, 'c', CuAt_CLASS, PdAt_CLASS,	
	      cudv.name, cudv.PdDvLn_Lvalue, "special_map",	
	      (struct attr *)NULL )  >  0  )
	  dds->map = 0;

	/* "type" attr exists on ps2 keyboards only          */
	/* so, if it exists then return it else set to zero  */
	if( getatt( &dds->type, 'c', CuAt_CLASS, PdAt_CLASS,
	      cudv.name, cudv.PdDvLn_Lvalue, "special_type",
	      (struct attr *)NULL )  >  0  )
	  dds->type = 0;
		
	*ddsptr = (caddr_t)dds;
	*dds_len = sizeof(struct ktsmdds);

#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif  /* CFGDEBUG */

	DEBUG_0("build_dds successful\n")
	return(0);
}

/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates a minor number for the keyboard
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
 * FUNCTION : This function creates a special file for the keyboard
 *
 * NOTES :
 *	Logical name of kbd device and device number are passed as
 *	parameters to this function. It checks for the validity of
 *	major and minor numbers, remove any previously created special
 *	file present, and creates a new special file with appropriate 
 *	permissions bits.
 * 
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */

int make_special_files(lname,devno)
char	*lname;			/* logical name of the device		*/
dev_t	devno;			/* device number			*/
{
	DEBUG_0(" creating special file for kbd \n")
	return(mk_sp_file(devno,lname,MKNOD_MODE));
}

/*
 * NAME     : download_microcode 
 *
 * FUNCTION : This function download microcode if applicable 
 *
 * NOTES : There is no microcode provision for kbd devices so this 
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
 * NOTES : There is no VPD for Keyboards so this routine does nothing
 *
 * RETURNS : 0
 */

int query_vpd(newobj,kmid,devno,vpd_dest)
struct  CuDv    *newobj;        /* vpd info will be put in that         */
mid_t   kmid;                   /* kernel module id                     */
dev_t   devno;                  /* device number                        */
char    *vpd_dest;              /* Destination for vpd                  */
{
	return 0;
}

/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines devices attached to this device 
 *
 * NOTES :
 *	The keyboard has no children to define so this routine does 
 *  nothing
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

