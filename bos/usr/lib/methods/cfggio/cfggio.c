static char sccsid[] = "@(#)35   1.15  src/bos/usr/lib/methods/cfggio/cfggio.c, inputdd, bos411, 9428A410j 10/24/93 16:27:24";
/* 
 *   CFGGIO - configuration method for gio adapter
 *   
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: build_dds, define_children, download_microcode
 *		generate_minor, make_special_files,	query_vpd
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
#define MAXSLOTS 	8
/* external functions */

extern	char *malloc();
extern	long *genminor();

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
	struct	gio_dds	*dds;		/* pointer to gio_dds structure	*/
	struct	CuDv 	cusobj ;	/* Customized object 		*/
	struct	CuDv 	parobj ;	/* Parent's Customized object	*/
	struct	Class	*preatt;	/* Predefined attr class handle */
	struct	Class	*cusatt;	/* Customized attr class handle */
	char	sstring[256];		/*  search criteria string	*/
	long	bus_id;			/* bus_id of parent ( i.e. bus )*/

	/* get customized object */
	sprintf(sstring,"name = '%s'",lname);
	GETOBJ( CuDv_CLASS, sstring, &cusobj, ODM_FIRST, E_NOCuDv )

	/* get parent's customized object */
	sprintf(sstring,"name = '%s'",cusobj.parent);
	GETOBJ( CuDv_CLASS, sstring, &parobj, ODM_FIRST, E_PARENT )

	/* open predefined, and customized attribute classes */
	ODMOPEN( preatt, PdAt_CLASS )
	ODMOPEN( cusatt, CuAt_CLASS )

	/* allocate memory for gio_dds structure  */
	dds = (struct gio_dds *)malloc(sizeof(struct gio_dds));
	if(dds == NULL){
		DEBUG_0("cfggio: malloc failed\n")
		return(E_MALLOC);
	}

	/* Fill in the dds structure: */

	strncpy( dds->gd_name, lname, 16 );

	GETATT( &dds->gd_offset, 'l', cusobj, "bus_io_addr", NULL )

	GETATT( &bus_id, 'l', parobj, "bus_id", NULL )
	dds->gd_seg = bus_id | 0x800C0060;
	dds->gd_iseg = bus_id | 0x800C0080;

	dds->gd_ibase = ( ( atoi( cusobj.connwhere ) -1 ) << 16 ) | 0x00400000;

	GETATT( &dds->gd_type, 'h', parobj, "bus_type", NULL )

	dds->gd_flags = 0;

	GETATT( &dds->gd_priority, 'i', cusobj, "intr_priority", NULL )

	GETATT( &dds->bus_intr_lvl, 'i', cusobj, "bus_intr_lvl", NULL )

	dds->gd_frequency = GIO_HZ;

	dds->gd_devtype = GIO_ADAP;

	*ddsptr = (caddr_t)dds;
	*dds_len = sizeof(struct gio_dds);

#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif	CFGDEBUG

	DEBUG_0("cfggio : build_dds successful\n")
	return(0);
}

/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates minor number for gio adapter
 *
 * EXECUTION ENVIRONMENT :
 *	This function always returns 0
 *
 * NOTES :
 *	The minor number for gio adapter is always 0. This is a dummy
 *	function
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
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
 * FUNCTION : This function creates special files for gio adapter 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is used to create special files for gio adapter
 *	It is called by the generic configure method. 
 * 
 * NOTES :
 *	Logical name of gio adapter  and device number are passed as
 *	parameters to this function. It checks for the validity of
 *	major and minor numbers, remove any previously created special
 *	file if one  present, create special file and change the
 *	mode of special file. 	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 */

int make_special_files(lname,devno)
char	*lname;			/* logical name of the device		*/
dev_t	devno;			/* device number			*/
{
	DEBUG_0(" creating special file for gio \n")
	return(mk_sp_file(devno,lname,MKNOD_MODE));
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
 *	Gio adapter does not have microcode. This is a dummy
 *	function
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Always returns 0
 */

int download_microcode(lname)
char	*lname;			/* logical name of the device		*/
{
	DEBUG_0("download microde\n")
	/* null function */
	return (0);
}

/*
 * NAME     : query_vpd 
 *
 * FUNCTION : This function gets the vital product data from gio adapter
 *	      using syscnfig() system call.
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the generic configure method based
 *	on the has_vpd flag in the Predefined Device Object Class for 
 *	this device. It uses the sysconfig() system call to get the VPD
 * 	information. 
 * 
 * NOTES :
 *	Kernal module id and device number are passed as parameters to
 *	this function. It and it gets the information using sysconfig()
 * 	system call , put the value in newobj and returns newob.
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
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
	int 	rc ;		/* return code  			*/
	struct 	cfg_dd  q_vpd;	/* vpd structure to pass to sysconfig 	*/
	char	vpd_area[VPDSIZE];/* buffer to hold vpd information 	*/

	DEBUG_1("vpd devno = %d\n",devno)

	/* initialize the q_vpd structure   */
	q_vpd.kmid 	= kmid;		/* kernel module id		*/
	q_vpd.devno 	= devno;	/* device number		*/
	q_vpd.cmd 	= CFG_QVPD ;	/* command			*/
	q_vpd.ddsptr	= vpd_area ;	/* area to hold vpd information	*/
	q_vpd.ddslen	= VPDSIZE;	/* size of vpd information	*/

	/*  call sysconfig   */
	DEBUG_1("qvpd devno = %d\n",q_vpd.devno)
	rc = sysconfig(SYS_CFGDD,&q_vpd,sizeof(struct cfg_dd));
	if (rc < 0) {
		switch(errno){
		case EACCES:
			DEBUG_0("query vpd : no required privilege\n")
			return(E_VPD);

		case EFAULT:
			DEBUG_0("query vpd : i/o error\n")
			return(E_VPD);

		case EINVAL:
			DEBUG_1("query vpd : invalid kmid=%d\n",kmid)
			return(E_VPD);

		case ENODEV:
			DEBUG_1("query vpd : invalid dev number =0x%x\n",devno)
			return (E_VPD);

		default:
			DEBUG_1("query vpd : error = 0x%x\n",errno)
			return(E_VPD);
		}
	}
#ifdef	CFGDEBUG
	dump_dds(vpd_area,VPDSIZE);
#endif	CFGDEBUG
	/* store the VPD in the database */
	put_vpd(vpd_dest,vpd_area,VPDSIZE);
	DEBUG_0("query vpd : successful\n")
	return(0);
}

/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines devices attached to this device 
 *
 * NOTES :
 *	The Gio adapter may have one Dials and/or one LPF Keys attached.
 *	Any children found which are already defined will have their names
 *	printed, and mill be marked as SAME. Newly discovered devices
 *	will be define'd and their names printed also.
 * 
 * RETURNS :
 * 	0 for success, >0 = errno.
 */

int define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device 		*/
int	ipl_phase;		/* ipl phase				*/
{

	struct PdDv	dev_pddv;	/* PdDv for LPFKeys or Dials */
	struct CuDv	dev_cudv,	/* CuDv for LPFKeys or Dials */
			gio_cudv;	/* CuDv for GIO card */
	int		cardno;		/* Number stipped from lname of gio */
	int		filedes=-1;	/* File descriptor for gio card */
	char		gio_name[20];	/* File name (and path) for gio */
	char		dev_lname[20];	/* lname for LPFKeys or Dials */
	char		dev_loc[20];	/* location for LPFKeys or Dials */
	char		sstring[100];	
	struct gioqueryid g_query;
	int		rc,
			i,
			devid,
			port;
	
	sprintf( sstring, "name = %s", lname );
	GETOBJ( CuDv_CLASS, sstring, &gio_cudv, ODM_FIRST, E_NOCuDv )

	/* Determine the "number" of the card */
	cardno = atoi( &lname[strcspn(lname,"0123456789")] );

	DEBUG_1("Card number = %d\n", (int)cardno )

	/* Query the gio card to see what devices are attached */

	sprintf( gio_name, "/dev/%s", lname );
	if( ( filedes = open(gio_name,O_RDWR) ) == NULL )
	{
		DEBUG_1("Open of %s failed\n",gio_name)
		return E_DEVACCESS;
	}

	/* set up query structure */
	
	memset(&g_query,0,sizeof(g_query));


	if ( (rc = ioctl(filedes,GIOQUERYID,&g_query)) !=0) {
		DEBUG_1("ioctl error %d\n",errno)
		close(filedes);
		return E_DEVACCESS;
	}

	close(filedes);

	for( port=0; port<2; port++ ) {
		DEBUG_1("PORT #%d\n",port)

                switch ( port ) {
                case 0:
		      if( g_query.port0_id == giolpfkid )
		     	strcpy( sstring, "uniquetype = lpfk/gio/lpfkeys" );
		      else if( g_query.port0_id == giodialsid )
			strcpy( sstring, "uniquetype = dial/gio/dials" );
		      else {
                            DEBUG_2("PORT #%d port0.id=%d\n",port,g_query.port0_id)
                            continue;	/* Nothing on this port */
                           }
                      break;
                case 1:
		      if( g_query.port1_id == giolpfkid )
		     	strcpy( sstring, "uniquetype = lpfk/gio/lpfkeys" );
		      else if( g_query.port1_id == giodialsid )
			strcpy( sstring, "uniquetype = dial/gio/dials" );
		      else {	/* Nothing on this port */
                            DEBUG_2("PORT #%d port1.id=%d\n",port,g_query.port1_id)
                            continue;
                            }
                      break;
                 }

		GETOBJ( PdDv_CLASS, sstring, &dev_pddv, ODM_FIRST, E_NOPdDv )

		sprintf( sstring,
			"PdDvLn = %s AND parent = %s AND connwhere = %d",
			dev_pddv.uniquetype, lname, port+1 );

		DEBUG_1("Reading from CuDv where %s\n", sstring )
		
		rc = odm_get_obj( CuDv_CLASS, sstring, &dev_cudv, ODM_FIRST);
		if( rc == -1 )
		{
			DEBUG_0("odm_get_obj failed\n")
			return E_ODMGET;
		}
		if( rc == 0 ) {
			sprintf( sstring, "-c %s -s %s -t %s -p %s -w %d",
				dev_pddv.class, dev_pddv.subclass,
				dev_pddv.type, lname, port+1 );
			
			DEBUG_2( "Running %s %s\n", dev_pddv.Define, sstring )
			if( odm_run_method( dev_pddv.Define, sstring, NULL, NULL)) {
				DEBUG_2( "Can't run %s %s\n", dev_pddv.Define,
					sstring )
				return E_ODMRUNMETHOD;
			}

		} else {

			/* The device already exists */
			DEBUG_0("Device already exists\n")

			if( dev_cudv.chgstatus != SAME ) {
				dev_cudv.chgstatus = SAME;
				rc = odm_change_obj( CuDv_CLASS, &dev_cudv );
				if( rc == -1 )
				{
					DEBUG_0("odm_change_obj failed\n")
					return E_ODMUPDATE;
				}
			}
			printf("%s ", dev_cudv.name );
		}
	}
	return 0;
}

