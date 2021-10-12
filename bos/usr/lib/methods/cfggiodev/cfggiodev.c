static char sccsid[] = "@(#)44   1.6  src/bos/usr/lib/methods/cfggiodev/cfggiodev.c, inputdd, bos41J, 9516B_all 4/18/95 15:12:07";
/*
 *   CFGGIODEV - configuration method for gio device
 *   
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: build_dds, define_children, download_microcode
 *		generate_minor, make_special_files, query_vpd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/mode.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/gio.h>
#include "pparms.h"
#include <cf.h>
#include "cfgdebug.h"

#define MKNOD_MODE (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

/* external functions */
extern	char *malloc();
extern	long *genminor();
extern	char *strchr();

dev_t	gio_dev_devno;
char	gio_dev_type[20];

#define GETOBJ( CLASS, SSTRING, OBJADDR, SEARCHFLAG, NOTFOUND ) {	\
		int rc;							\
		DEBUG_3(						\
		"GETOBJ: SSTRING='%s', OBJADDR = '%s', Address=%ld\n ",	\
				SSTRING, #OBJADDR, (long)OBJADDR )	\
		rc=(int)odm_get_obj(CLASS,SSTRING,OBJADDR,SEARCHFLAG);	\
		if( rc == 0 ) {						\
			DEBUG_2("GETOBJ: Object '%s' not found in %s\n",\
				SSTRING, #CLASS )			\
			return NOTFOUND;				\
		}							\
		else if( rc == -1 ) {					\
			DEBUG_0("ODM ERROR\n")				\
			return E_ODMGET;				\
		}							\
	}

#define ODMOPEN( HANDLE, CLASS ) {					\
		if( ( int )( HANDLE = odm_open_class( CLASS ) ) == -1 ){\
			DEBUG_1("Error opening %s\n", #CLASS )		\
			return E_ODMOPEN;				\
		}							\
	}	

#define ODMCLOSE( CLASS ) {						\
		if( odm_close_class( CLASS ) == -1 )			\
			return E_ODMCLOSE;				\
	}

#define GETATT( DEST, TYPE, CUSOBJ, ATTR, NEWATTRS ) {			\
		int rc;							\
		rc = getatt( DEST, TYPE, CuAt_CLASS, PdAt_CLASS,	\
			CUSOBJ.name, CUSOBJ.PdDvLn_Lvalue, ATTR,	\
			(struct attr *)NEWATTRS );			\
		if( rc > 0 )						\
			return rc;					\
	}

/*
 * NAME     : build_dds 
 *
 * FUNCTION : This function builds the DDS for gio adapter 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the generic configure method to 
 *	build the define data structure which defines the attributes
 *	of the adapter.	
 * 
 * NOTES :
 *	This function gets the values of the attributes of gio adapter
 *	from ODM database , assigns the values to the DDS structure
 * 	and returns a pointer to the dds and its size.
 *
 * RECOVERY OPERATION:
 * 
 * DATA STRUCTURES:
 *	
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */

int build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device	*/
char	**ddsptr;			/* pointer to dds structure	*/
int 	*dds_len;			/* size of dds structure	*/
{
	struct	gio_dds	*dds;	/* pointer to gio_dds structure	*/
	struct	CuDv 	cusobj ;	/* Customized object for device */
	struct	CuDv 	parobj ;	/* Customized object for parent */
	struct	CuDv 	busobj ;	/* Customized object for bus	*/
	struct	Class	*preatt;	/* Predefined attr class handle */
	struct	Class	*cusatt;	/* Customized attr class handle */
	char	sstring[256];		/*  search criteria string	*/
	long	bus_id;			/* bus_id of parent ( i.e. bus )*/
	char	*strrchr();

	DEBUG_0("build_dds:\n")

	ODMOPEN( preatt, PdAt_CLASS )	/* Open predefined attributes	*/
	ODMOPEN( cusatt, CuAt_CLASS )	/* Open customized attributes	*/

	/* Read Customized object of device */
	sprintf( sstring, "name = '%s'", lname );
	GETOBJ( CuDv_CLASS, sstring, &cusobj, ODM_FIRST, E_NOCuDv )

	/* Save type for later usage */
	strcpy( gio_dev_type, strrchr( cusobj.PdDvLn_Lvalue, '/' )+1);

	/* Read Customized object of parent */
	sprintf( sstring, "name = '%s'", cusobj.parent );
	GETOBJ( CuDv_CLASS, sstring, &parobj, ODM_FIRST, E_PARENT )

	/* Find Customized object of bus */
	if(Get_Parent_Bus(CuDv_CLASS, cusobj.parent, &busobj)) {
		DEBUG_0("error getting bus object \n");
		return(E_PARENT);
	}

	/* Allocate memory for gio_dds structure  */
	dds = (struct gio_dds *)malloc(sizeof(struct gio_dds));
	if(dds == NULL){
		DEBUG_0("cfggiodev: malloc failed\n")
		return(E_MALLOC);
	}

	strncpy( dds->gd_name, lname, 16 );

	GETATT( &dds->gd_offset, 'l', parobj, "bus_io_addr", NULL )
	if( cusobj.connwhere[0] == '2' )
		dds->gd_offset |= 8;

	GETATT( &bus_id, 'l', busobj, "bus_id", NULL )
	dds->gd_seg = bus_id | 0x800C0060;
	dds->gd_iseg = bus_id | 0x800C0080;
	
	dds->gd_ibase = ((atoi(parobj.connwhere)-1)<<16) | 0x00400000;

	GETATT( &dds->gd_type, 'h', busobj, "bus_type", NULL )

	dds->gd_flags = 0;

	GETATT( &dds->gd_priority, 'i', parobj, "intr_priority", NULL )

	GETATT( &dds->bus_intr_lvl, 'i', parobj, "bus_intr_lvl", NULL )

	dds->gd_frequency = GIO_HZ;

	if( lname[0] == 'd' )
		dds->gd_devtype = GIO_DIALS;
	else
		dds->gd_devtype = GIO_LPFK;

	ODMCLOSE( PdAt_CLASS )
	ODMCLOSE( CuAt_CLASS )

	*ddsptr = (caddr_t)dds;
	*dds_len = sizeof(struct gio_dds);

#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif	CFGDEBUG

	DEBUG_0("cfggiodev : build_dds successful\n")
	return(0);
}

/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates a minor number for the gio device
 *
 * EXECUTION ENVIRONMENT :
 *	This function always returns 0
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
	/* Null function */
	if((minorno = genminor(lname,majorno,-1,1,1,1)) == (long *)NULL )
		return E_MINORNO;
	*minordest = *minorno;

	return 0;
}

/*
 * NAME     : make_special_files 
 *
 * FUNCTION : This function creates special files for gio devices
 *
 * NOTES :
 *	Logical name of gio device and device number are passed as
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
	DEBUG_0(" creating special file for gio \n")
	gio_dev_devno = devno;
	return(mk_sp_file(devno,lname,MKNOD_MODE));
}

/*
 * NAME     : download_microcode 
 *
 * FUNCTION : This function download microcode if applicable 
 *
 * NOTES : There is no microcode provision for gio devices, however
 *	this routine is used to establish that the correct device is
 *	indeed present.
 *
 * RETURNS :
 *	0		if all OK
 * 	E_NODETECT	if no device is present,
 *	E_WRONGDEVICE	if the wrong kind of GIO device is present
 *	E_DEVACCESS	if another type of error occurs
 */

int download_microcode(lname)
char	*lname;			/* logical name of the device		*/
{
	char	devname[30];
	int	fd;


	sprintf( devname, "/dev/%s", lname );

	if( ( fd = open( devname, O_RDONLY ) ) == -1 )
	{
		if( errno == ENXIO ) return E_NODETECT;
		if( errno == EINVAL ) return E_WRONGDEVICE;
		return E_DEVACCESS;
	}

	close( fd );

	return (0);
}

/*
 * NAME     : query_vpd 
 *
 * FUNCTION : This function is used to get device specific VPD.
 *
 * NOTES : There is no VPD for dials, or LPF Keys.
 *
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
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
 *	There are no children devices of gio devices.
 *
 * EXECUTION ENVIRONMENT :
 *	This function always returns 0
 * 
 * NOTES :
 *	Gio devices have no children to define.
 *
 * RETURNS :
 * 	0 for success.
 */

int define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device 		*/
int	ipl_phase;		/* ipl phase				*/
{
        return 0;
}

