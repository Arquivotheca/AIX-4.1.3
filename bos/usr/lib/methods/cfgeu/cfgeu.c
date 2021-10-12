static char sccsid[] = "@(#)04  1.13.1.4  src/bos/usr/lib/methods/cfgeu/cfgeu.c, cfgmethods, bos411, 9428A410j 1/28/94 10:10:12";
/*
 *   COMPONENT_NAME: (CFGMETHODS) Asynchronous Expansion Adapter config method
 *
 *   FUNCTIONS: build_dds
 *		def_child
 *		define_children
 *		download_microcode
 *		fquery_vpd
 *		generate_minor
 *		getxcards
 *		make_special_files
 *		query_vpd
 *		
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


#include	<stdio.h>
#include	<errno.h>
#include	<malloc.h>
#include	<cf.h>
#include	<sys/types.h>
#include	<sys/mode.h>
#include	<sys/device.h>
#include	<sys/sysconfig.h>
#include	<sys/cfgdb.h>
#include 	<sys/mdio.h>
#include	<sys/eu.h>
#include	<sys/errids.h>
#include	<sys/err_rec.h>
#include	<sys/cfgodm.h>

#include	"cfgdebug.h"

#define BUSXSIZE	8
#define S_LENGTH	256
#define C_LENGTH	10 
#define MKNOD_MODE (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define	DEVPKG_PREFIX	"devices"		/* device package prefix */

/* external functions */

extern	char	*malloc();
extern	int	*genminor();

/*
 * NAME: build_dds
 *
 * FUNCTION: Device dependant dds build for expansion unit adapter card
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded routine.
 *
 * RETURNS:	0 if succeed, or errno if fail
 */
int build_dds(lname,dds_data_ptr,dds_len)
char	*lname;				/* logical name			*/
char	**dds_data_ptr;			/* pointer to dds pointer 	*/
int	*dds_len;			/* pointer to length of dds 	*/
{
	struct	eu_dds	*dds;		/* dds for eu from eu.h	*/
	struct	CuDv	cusobj;		/* customized device object	*/
	struct	PdDv	preobj;		/* predefined device object	*/
	char	sstring[S_LENGTH];	/* search criterion		*/
	char	*tmpstr;		/* temporary pointer		*/
	int	rc;			/* return code			*/
	int	tmp_int;		/* temporary variable 		*/
        struct  Class *cusdev;          /* CuDv class                   */

	DEBUG_0( "build_dds()\n" )

	dds = (struct eu_dds *)malloc(sizeof(struct eu_dds));
	if(dds == NULL){
		DEBUG_0("Cannot malloc space for struct eu_dds\n")
		return E_MALLOC;
	}

        if((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1){
            DEBUG_0("build_dds: can not open CuDv\n")
            return E_ODMOPEN;
        }

	/* put device name in dds */
	strcpy(dds->eu_name,lname);

	/* Read CuDv for eu */
	sprintf(sstring, "name = '%s'", lname);

	DEBUG_1("Performing odm_get_obj(CuDv,%s)\n",sstring)
	rc = (int)odm_get_obj(cusdev,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_1("build_dds():Record not found in CuDv:%s\n",sstring)
		return E_NOCuDv;
	}
	else if(rc == -1){
		DEBUG_0( "build_dds() for eu: Error Reading CuDv\n")
		return E_ODMGET;
	}
	DEBUG_1("odm_get_obj(CuDv,%s) succeeded\n", sstring)

	/* Read PdDv for eu */

	sprintf( sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	DEBUG_1("Performing odm_get_obj(CuDv,%s)\n", sstring)
	rc = (int)odm_get_obj(PdDv_CLASS,sstring,&preobj,TRUE) ;
	if(rc == 0){
		DEBUG_1("build_dds():Record not found in PdDv:%s\n",sstring)
		return E_NOPdDv;
	}
	else if (rc == -1){
		DEBUG_0( "build_dds() for eu: Error Reading PdDv\n")
		return E_ODMGET;
	}
	DEBUG_1("odm_get_obj(PdDv,%s) succeeded\n",sstring)
	
	/* Get the device id from predefined object class */
	
	dds->eu_id = (short)strtoul(preobj.devid,(char **)NULL,16);

	if(( rc = getatt(&tmp_int,'i',CuAt_CLASS,PdAt_CLASS,lname,
		cusobj.PdDvLn_Lvalue,"eu_delay", (struct attr *)NULL))>0)
		return rc;

	dds->eu_delay = (uchar)tmp_int;

	/* Establish other attributes */

	/* get value of eu_slot ( slot number) */
	if((int)(dds->eu_slot = (uchar)(atoi(cusobj.connwhere))-1) < 0 ) {
	/* connection information is not available */
		DEBUG_0("cfgeu: error in getting connection info\n")
		return E_INVCONNECT;
	}

       /*
        * Get the parent bus so that we can set some attributes 
        * based on the bus we are attached to (either directly
        * or indirectly).
        */
        if ((rc = Get_Parent_Bus(cusdev, cusobj.parent, &cusobj)) != 0)
        {
            DEBUG_1("cfgeu: Unable to get parent bus object; rc = %d\n",
                     rc)
            if (rc == E_PARENT)
            {
                rc = E_NOCuDvPARENT ;
            }
            odm_close_class(cusdev);
            return (rc) ;
        }
        odm_close_class(cusdev);

	/* get value of attribute eu_nseg (bus id from parent) */
	rc = getatt(&dds->eu_nseg,'l',CuAt_CLASS,PdAt_CLASS,cusobj.name,
		cusobj.PdDvLn_Lvalue, "bus_id", (struct attr *)NULL);
	if(rc > 0){
		DEBUG_0("cfgeu: error in getting bus_id of parent\n")
		return rc;
	}
	
        dds->eu_iseg = dds->eu_nseg | 0x800C0080 ;

	*dds_data_ptr = (caddr_t)dds;		/* Store address of struct*/
	*dds_len = sizeof( struct eu_dds );	/* Store size of structure*/

#ifdef CFGDEBUG
	dump_dds(*dds_data_ptr,*dds_len);
#endif CFGDEBUG

	return 0;
}

/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates minor number for expansion unit 
 *
 * EXECUTION ENVIRONMENT :
 *	This function calls the genminor function to generate the
 *	first available minor number. 
 * 
 * NOTES :
 *	The minor number for eu adapter is always 0. 
 *
 * RETURNS :
 * 	Returns 0 on success, errno on failure.
 */
int generate_minor(logical_name,majorno,minordest)
char 	*logical_name;			/* logical name of the device	*/
long 	majorno;			/* major number of the device 	*/
long	*minordest;			/* destination for minor no.	*/
{
	long	*ptr;

	DEBUG_0("generate minor\n")

	/* Null function */
	if(( ptr = genminor(logical_name,majorno,-1,1,1,1)) == (long *)NULL)
		return E_MINORNO;

	*minordest = *ptr;

	return 0;
}

/*
 * NAME     : make_special_files 
 *
 * FUNCTION : This function creates special files for expansion unit. 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is used to create special files for expansion unit. 
 *	It is called by the generic configure method. 
 * 
 * NOTES :
 *	Logical name of exp unit and device number are passed as
 *	parameters to this function. It checks for the validity of
 *	major and minor numbers, remove any previously created special
 *	file if one  present, create special file and change the
 *	mode of special file. 	
 *	
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */
int make_special_files(logical_name,devno)
char	*logical_name;			/* logical name of the device  */
dev_t	devno;				/* device number	       */
{

	DEBUG_0(" creating special file for eu \n")

	return mk_sp_file(devno,logical_name,MKNOD_MODE);
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
 *	eu does not have microcode. This is a dummy
 *	function
 * 
 * RETURNS :
 * 	Always returns 0
 */
int download_microcode(logical_name)
char	*logical_name;			/* logical name of the device	*/
{

	DEBUG_0("download microde\n")
	
	/* null function */
	return 0;
}

/*
 * NAME     : query_vpd 
 *
 * FUNCTION : This function gets the vital product data from eu adapter
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
 *	this function. It gets the vpd information using sysconfig()
 * 	system call , put the value in newobj and returns newob.
 *
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */
int query_vpd(newobj,kmid,devno,vpd_dest)
struct	CuDv	*newobj;		/* vpd info will be put in that	*/
mid_t	kmid;				/* kernal module id  		*/
dev_t	devno;				/* device number   		*/
char	*vpd_dest;
{
	char	bufstr[VPDSIZE];	/* temporary buffer		*/
	long	majorno,
		minorno;		/* major and minor numbers 	*/
	int	rc;			/* return code 			*/

	rc = fquery_vpd(bufstr,kmid,devno);
	if(rc > 0){
		DEBUG_0("error in getting vpd0\n")
		return rc;
	}

#ifdef	CFGDEBUG
	dump_dds(bufstr,VPDSIZE);
#endif	CFGDEBUG

	/* store the VPD in the database */
	/* (N.B. The VPD may be preceded by an 8-byte header: */
	if( strncmp( &bufstr[8], "VPD", 3 ) == 0 )
		put_vpd(vpd_dest,&bufstr[8],VPDSIZE);
	else
		put_vpd(vpd_dest,bufstr,VPDSIZE);
	majorno = major(devno);
	minorno = 0xff;
	devno = makedev(majorno,minorno);
	rc = fquery_vpd(bufstr,kmid,devno);
	if(rc > 0){
		DEBUG_0("error in getting vpd1\n")
		return rc;
	}
#ifdef	CFGDEBUG
	dump_dds(bufstr,VPDSIZE);
#endif	CFGDEBUG
	return 0;
}

/*
 * NAME     : fquery_vpd 
 *
 * FUNCTION : This function is called by query_vpd.It gets the vital 
 * 	product information of the device.
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by query_vpd function.
 *	It uses the sysconfig() system call to get the VPD
 * 	information. 
 * 
 * NOTES :
 *	Kernal module id and device number are passed as parameters to
 *	this function. It gets the vpd information using sysconfig()
 * 	system call , puts the value in newobj and returns newobj.
 *
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */
int fquery_vpd(newobj,kmid,devno)
char	*newobj;			/* vpd info will be put in that */
mid_t	kmid;				/* kernal module id             */
dev_t	devno;				/* device number		*/
{
	struct 	cfg_dd  q_vpd;		/* vpd structure  to pass to	*/
					/* sysconfig system call	*/
	
	DEBUG_1("vpd devno = %d\n",devno)

	/* initialize the q_vpd structure   */

	q_vpd.kmid 	= kmid;			/* kernal module id 	*/
	q_vpd.devno 	= devno;		/* device number   	*/
	q_vpd.cmd 	= CFG_QVPD ;		/* command	   	*/
	q_vpd.ddsptr	= newobj;		/* area to hold vpd 	*/
	q_vpd.ddslen	= VPDSIZE;		/* size of vpd info   	*/

		/*  call sysconfig   */
	DEBUG_1("qvpd devno = %d\n",q_vpd.devno)
	if ( sysconfig(SYS_CFGDD,&q_vpd,sizeof(struct cfg_dd)) < 0 )
	{
		switch(errno){
		case EACCES:
			DEBUG_0("query vpd : no required privilege\n")
			return E_VPD;
		case EFAULT:
			DEBUG_0("query vpd : i/o error\n")
			return E_VPD;
		case EINVAL:
			DEBUG_1("query vpd : invalid kmid=%d\n",kmid)
			return E_VPD;
		case ENODEV:
			DEBUG_1("query vpd : invalid dev number=0x%x\n",devno)
			return E_VPD;
		default:
			DEBUG_1("query vpd : error = 0x%x\n",errno)
			return E_VPD;
		}
	}
	DEBUG_0("query vpd : successful\n")
	return 0;
}

/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines devices attached to this device 
 *
 * NOTES :
 *	This function talks to the expansion unit device driver. It
 *	queries for the card id in each slot and invokes the define
 * 	methods based on the card id. 
 *
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */
int define_children(logical_name,ipl_phase)
char	*logical_name;			/* logical name of the device  	*/
int	ipl_phase;			/* ipl phase			*/
{
	ushort	xcardtbl[BUSXSIZE];  	/* table to hold device id 	*/
	int	rc;			/* return code 	  		*/
	int	i;			/* index 			*/


		DEBUG_0("define children\n");

		rc = getxcards(xcardtbl,logical_name,BUSXSIZE);
		if(rc > 0){
			DEBUG_0("error in getting card id\n")
			return rc;
		}
#ifdef CFGDEBUG
		for(i=0;i<BUSXSIZE;i++){
			DEBUG_2("slot = %d, devid=%x\n",i,xcardtbl[i])
		}
#endif
		rc = def_child(xcardtbl,logical_name);
		if(rc > 0){
			DEBUG_0("error in defining children\n");
			return rc;
		}
		DEBUG_0("define children successful\n")
	return 0;
}

/*
 * NAME     : def_child 
 *
 * FUNCTION : This function is called by define_children function.
 *
 * EXECUTION ENVIRONMENT :
 *	This function invokes the define method for children 
 * 	based on the device id of the device.	
 * 
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */
int allpkg = 0;

int def_child(xcardtbl,pname)
ushort	xcardtbl[] ;			/* table to hold device id 	*/
char	*pname ;			/* name of the parent	    	*/
{
	struct	PdDv	preobj ;	/* predefined device object 	*/
	struct	CuDv	cusobj ;	/* customized device object 	*/
	char	sstring[S_LENGTH];	/* search criteria 		*/
	char	constr[C_LENGTH];	/* string to hold connection 
						information		*/
	char	*out_p ;		/* temp pointer to string	*/
	int	slottbl[BUSXSIZE];	/* table to check available 
						slots 			*/
	int	slot;			/* variable to hold slot no  	*/
	int	rc;			/* return code 			*/

	if(!strcmp(getenv("DEV_PKGNAME"),"ALL"))
		allpkg = 1;

	for(slot=0; slot < BUSXSIZE ; slot++) {

		if(xcardtbl[slot] == 0xffff)
			continue;

		/* search Predefined devices for object with this device_id */
		sprintf(sstring,"devid = '0x%x'",xcardtbl[slot]);
		rc = (int)odm_get_obj(PdDv_CLASS,sstring,&preobj,TRUE);
		if(rc == 0) {
			struct cfgeu_err {
				struct	err_rec0 err_info;
				int	slot;
				} err_data;

			if( !allpkg ) 
			fprintf(stdout,":%s.mca.%02x%02x ", DEVPKG_PREFIX,
				xcardtbl[slot] & 0xff,xcardtbl[slot] >> 8 );	
			/* device not found in data base. Put an entry into
			   the error log to record this event */
			err_data.err_info.error_id = ERRID_EU_BAD_ADPT;
			strcpy(err_data.err_info.resource_name,pname);
			err_data.slot = slot + 1;

			rc = errlog((char *)&err_data,
				sizeof(struct cfgeu_err));
#ifdef CFGDEBUG
			if (rc == -1)
				DEBUG_1("cfgeu: errlog failed, errid=%x\n",
					ERRID_EU_BAD_ADPT)
			else
				DEBUG_1("cfgeu: errlog good, errid=%x\n",
					ERRID_EU_BAD_ADPT)
#endif
			continue;
		}
		else if (rc == -1) {
			/* odm error occurred */
			DEBUG_1( "cfgeu: get_obj failed, crit=%s",sstring)
			return E_ODMGET;
		}
		/* search Customized object with
			PdDvLn = preobj.uniquetype and
			parent = pname and
			connwhere = slot number       */
		if(allpkg)
			fprintf(stdout,":%s.mca.%02x%02x ", DEVPKG_PREFIX,
				xcardtbl[slot] & 0xff ,xcardtbl[slot] >> 8);	
		sprintf(sstring,
		"PdDvLn = '%s' AND parent = '%s' AND connwhere = '%d'",
		preobj.uniquetype,pname,slot+1) ;

		rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
		if(rc == 0) {
			/* objects not found  so define it */
			sprintf(sstring,
				"-c %s -s %s -t %s -p %s -w '%d'",
				preobj.class,preobj.subclass,
				preobj.type,pname,slot+1);
			if(odm_run_method(preobj.Define,sstring,&out_p,NULL)){
				DEBUG_1("can't run %s\n",preobj.Define)
				return E_ODMRUNMETHOD;
			}
			fprintf(stdout,"%s ",out_p);
		}
		else if (rc == -1) {
			/* odm error occurred */
			DEBUG_1( "cfgeu: get_obj failed, crit=%s",sstring)
			return E_ODMGET;
		}
		else {
			if ( cusobj.chgstatus == MISSING ) {
				cusobj.chgstatus = SAME ;
				rc = (int)odm_change_obj(CuDv_CLASS,&cusobj) ;
				if (rc == -1){
					/* change object failed */
					DEBUG_1("cfgeu:change_obj failed, crit=%s",
						sstring)
					return E_ODMUPDATE;
				}
			}
			fprintf(stdout,"%s ",cusobj.name);
		}
	}
	return 0;
}

/*
 * NAME     : getxcards 
 *
 * FUNCTION : This function is called by defines_children to query
 *	  devices attached to in each slot of this device 
 *
 * EXECUTION ENVIRONMENT :
 *	This function talks to the expansion unit device driver. It
 *	queries for the card id in each slot and invokes the define
 * 	methods based on the card id. 
 * 
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */
int getxcards(cardtbl, lname, busize)
ushort *cardtbl;			/* table containing card info	*/
char	*lname;				/* logical name of eu		*/
int busize;				/* number of slots available	*/
{
	int eufd;			/* file descriptor		*/
	uchar slot;			/* slot 			*/
	struct euidcb euid;		/* struct required for ioctl	*/
	char	sp_file[20];		/* special file name 		*/

	sprintf (sp_file,"/dev/%s",lname);
	eufd = open(sp_file, O_RDWR);
	if(eufd < 0) {
		DEBUG_0("error in opening the xbus\n")
		return E_OPEN;
	}
	for (slot=0; slot < busize; slot++) {
		euid.slot = slot;
		if (ioctl(eufd, EU_GETID, &euid) < 0) {
			DEBUG_1("error in getting card id in slot=%d\n",slot)
			return E_DEVACCESS;
		}
		cardtbl[slot] = euid.id;
	}
	close(eufd);
	return 0;
}
