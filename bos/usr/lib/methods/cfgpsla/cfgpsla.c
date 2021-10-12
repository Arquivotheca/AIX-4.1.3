static char sccsid[] = "@(#)02  1.5  src/bos/usr/lib/methods/cfgpsla/cfgpsla.c, cfgmethods, bos411, 9428A410j 8/10/90 10:22:20";
/* 
 * COMPONENT_NAME: CFGPSLA (configuration method for psla )
 *
 * FUNCTIONS : 	build_dds, generate_minor, make_special_files,
 *		download_microcode, query_vpd, define_children.
 * 
 * ORIGINS : 27 
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
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
#include "psla.h"
#include "cfgdebug.h"
 
#define  CFGDEBUG  1
 
#define	S_LENGTH	   256
#define	PSLA_BUS_OFFSET	   0x00000020
#define MKNOD_MODE (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define PORTA "gswa"
#define PORTB "gswb"

extern	char *malloc();
extern	long *genminor();

/*
 *
 *
 * NAME     : build_dds
 *
 * FUNCTION : This function builds the DDS for msla adapter
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the generic configure
 *      method to build the define data structure which defines
 * 	the attributes of the adapter.
 *
 * NOTES :
 *	This function gets the values of the attributes of msla
 *      adapter from ODM database , assigns the values to the DDS
 *      structure and returns a pointer to the dds and its size.
 *
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns  0 on success, > 0 on failure.
 *
 */

int build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device 	*/
char	**ddsptr;			/* pointer to dds structure 	*/
int 	*dds_len;			/* size of dds 			*/
{
 
	struct 	psladd  *dds_data; 	/* pointer to psladd structure 	*/
	struct	Class	*preatt;	/* predefined attribute handle 	*/
	struct	Class	*cusatt;	/* customized attribute handle 	*/
	struct	CuDv	cusobj ;	/* customized object 		*/
	char	sstring[S_LENGTH];	/* search criteria string 	*/
	char	microcode[S_LENGTH];	/* search criteria string 	*/
	char	*tmpstr;		/* temp pointer to string 	*/
	int	slotno;			/* slot number 			*/
	int	rc;			/* return code 			*/
	int	i;			/* index 			*/
	FILE	*fp, *fopen();		/* file pointer			*/
	long	code_length ;		/* microcode length		*/
	long	num_bytes ;		/* number of bytes of microcode */
  	uchar   *ucode_buff;		/* pointer to ucode             */
 
	/* get customized object */
	sprintf(sstring,"name = '%s'",lname);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	if(rc == 0)
	{
		DEBUG_0("cfgpsla : CuDv object not found\n")
		return E_NOCuDv;
	}
	else if(rc == -1)
	{
		/* odm error occured */
		DEBUG_0("cfgpsla : get_obj failed \n")
		return E_ODMGET;
	}
 
	/* open the predefined attribute class */
	preatt = odm_open_class(PdAt_CLASS);
	if((int)preatt == -1)
	{
		DEBUG_0("cfgpsla :cannot open PdAt\n")
		return E_ODMOPEN;
	}
 
	/* open the customized attribute class */
	cusatt = odm_open_class(CuAt_CLASS);
	if((int)cusatt == -1)
	{
		DEBUG_0("cfgpsla :cannot open CuAt\n")
		return E_ODMOPEN;
	}
 
	/* allocate memory for psladd structure */
	dds_data = (struct psladd *)malloc(sizeof(struct psladd));
	if(dds_data == NULL)
	{
		DEBUG_0("cfgpsla: malloc failed\n")
		return E_MALLOC;
	}
 
	/* get the value for the attribute microcode  */
	rc = getatt(microcode,'s',cusatt,preatt,lname, 
		cusobj.PdDvLn_Lvalue,"microcode", (struct att *)NULL); 
	if (rc > 0){
		DEBUG_0("cfgpsla: error in getting microcode\n")
		return rc;
	}
 
	/* get information from parent adapter */
	tmpstr = cusobj.parent ;
	if(tmpstr[0] == '\0')
	{
		DEBUG_0("cfgpsla : No parent is defined in CuDv object\n");
		return E_PARENT;
	}
 
	/*get the customized object of parent to get the unique type */
	sprintf(sstring,"name = '%s'",tmpstr);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	if(rc == 0)
	{
		DEBUG_0("cfgpsla: parent object not found\n")
		return E_NOCuDvPARENT;
	}
	else if(rc == -1){
		/* odm error occured */
		DEBUG_0("cfgpsla : get_obj failed\n")
		return E_ODMGET;
	}
 
	/* get the value for the attribute bus_mem_beg   */
	rc = getatt(&dds_data->start_busmem,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_mem_addr", (struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgpsla : error in getting attribute bus_mem_addr\n")
		return rc;
	}
	
	/* get the value for the attribute io_port   */
	rc = getatt(&dds_data->start_busio,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_io_addr",(struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgpsla :error in getting attribute bus_io_addr\n")
		return rc;
	}
	
	/* get the value for the attribute bus_intr_level    */
	rc = getatt(&dds_data->intr_level,'l',cusatt,preatt,
		cusobj.name,cusobj.PdDvLn_Lvalue,
		"bus_intr_lvl",(struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgpsla : error in getting bus_intr_level\n")
		return rc;
	}
 
	/* get the value for the attribute bus_intr_pri    */
	rc = getatt(&dds_data->intr_priority,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"intr_priority", (struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgpsla : error in getting attr intr_priority\n")
		return rc;
	}
 
	/* get the value for the dma bus memory addr */
	rc = getatt(&dds_data->dma_bus_addr,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"dma_bus_mem", (struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgpsla : error in getting dma_bus_mem\n")
		return rc;
	}
 
	/* get the value for the attribute  dma_arb_level */
	rc = getatt(&dds_data->dma_level,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"dma_lvl", (struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgpsla : error in getting dma_arb_level\n")
		return rc;
	}
 
	/* get the value for the attribute slot_number */
	tmpstr = cusobj.connwhere;
	if(tmpstr[0] == '\0')
	{
		DEBUG_0("cfgpsla : error in getting connection info\n")
		return E_INVCONNECT;
	}
 
	slotno = atoi(tmpstr)-1;
	dds_data->slot_number = slotno ;
 
	/* get information from parent adapter */
	tmpstr = cusobj.parent ;
	if(tmpstr[0] == '\0' )
	{
		DEBUG_0("cfgpsla : parent not defined\n");
		return E_PARENT;
	}
 
	/*get the customized object of parent to get the unique type */
	sprintf(sstring,"name = '%s'",tmpstr);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("cfgpsla: error in getting grandparent object\n")
		return E_NOCuDvPARENT;
	}
	else if(rc == -1){
		/* odm error occured */
		DEBUG_0("cfgpsla: get_obj failed\n")
		return E_ODMGET;
	}
 
	/* get the value for the attribute bus_id */
	rc = getatt(&dds_data->bus_id,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_id", (struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfgpsla : error in getting bus_id\n")
		return rc;
	}
 
	dds_data->bus_id |= PSLA_BUS_OFFSET;
	
	/* close object class */
 
	rc = odm_close_class(preatt);
	if(rc == -1)
	{
		DEBUG_0("cfgpsla: error in closing object class PdAt\n")
		return E_ODMCLOSE;
	}
 
	rc = odm_close_class(cusatt);
	if(rc == -1)
	{
		DEBUG_0("cfgpsla: error in closing object class CuAt\n")
		return E_ODMCLOSE;
	}
 
	/* Get Microcode file descriptor     */
	if( (dds_data->ucode_fd = open(microcode, O_RDONLY)) == -1 )
	{
		/* If microcode 8787P.00.01 is not present is because  */
		/* the adapter is meant to be configured as SSLA.      */
		/* However for diagnostics purposes we need it to con- */
		/* figure as PSLA, but we will let the device driver   */
		/* know that operational microcode is not present. And */
		/* we will continue with the configuration process.    */
		dds_data->ucode_len = 0;
	}
	else
	{
		dds_data->ucode_len = lseek(dds_data->ucode_fd, 0, SEEK_END);
		lseek(dds_data->ucode_fd, 0, SEEK_SET);
	}
 
	/* setup values for download ioctl command */
 
	*ddsptr = (caddr_t)dds_data;
	*dds_len = sizeof(struct psladd);
 
#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif	CFGDEBUG
	DEBUG_0("cfgpsla : build_dds successful \n")
 
	return 0;
}
 
/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates minor number for msla adapter
 *
 * EXECUTION ENVIRONMENT :
 *	This function  returns a value between 0 and 3 on success.
 * 
 * NOTES :
 *	The minor number for msla adapter can be between 0 and 3.
 *	This function calls the genminor function to generate a 
 *	minor number 
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns 0 on success ,  > 0 on failure.	
 */
 
int generate_minor(lname,majorno,minordest)
char 	*lname;				/* logical name of the device 	*/
long 	majorno;			/* major number of the device 	*/
long	*minordest;			/* place to store minor no	*/
{
	long	*rc;			/* return code			*/
 
	DEBUG_0("generate minor number \n")

	/* call genminor function to generate one minor number */
	/* start at minor 0 reserve 3 minors increment by 1 */
	/* increment between groups..don'tcare */
 
	if( ( rc = genminor(lname,majorno,0,3,1,1) ) == (long *)NULL )
		return E_MINORNO;

	*minordest = *rc;

	return 0;
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
 * 	0 on success, >0 on failure
 */
 
int make_special_files(lname,devno)
char	*lname;			/* logical name of the device  		*/
dev_t	devno;			/* device number	       		*/
{
 int rc;
	DEBUG_0("cfgpsla: creating special file for lname \n")
	rc =  	(mk_sp_file(devno,lname,MKNOD_MODE));
	if (rc)
	         return rc;
 
	DEBUG_0("cfgpsla: creating gswa special file for psla\n")
	rc =  	(mk_sp_file(devno+1,PORTA,MKNOD_MODE));
	if (rc)
	         return rc;
 
	DEBUG_0("cfgpsla: creating gswb special file for psla\n")
	rc =  	(mk_sp_file(devno+2,PORTB,MKNOD_MODE));
	if (rc)
	         return rc;
	return 0;
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
 
int download_microcode(lname)
char	*lname;			/* logical name of the device */
{
	return 0;
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
 * 	Returns  0 on success, > 0 on failure.
 */
 
int query_vpd(newobj,kmid,devno,vpd_dest)
struct	CuDv	*newobj;	/* vpd info will be put in that 	*/
mid_t	kmid;			/* kernal module id             	*/
dev_t	devno;			/* device number			*/
char	*vpd_dest;		/* place to store vpd			*/
{
	int 	rc ,i;		/* return code  			*/
	struct 	cfg_dd  q_vpd;	/* vpd structure to pass to sysconfig 	*/
	char	vpd_area[VPDSIZE];/* buffer to hold vpd information 	*/
	
	DEBUG_1("vpd devno = %d\n",devno)
	
	/* initialize the q_vpd structure   */
	q_vpd.kmid 	 = kmid;	/* kernal module id		*/
	q_vpd.devno  = devno;	        /* device number		*/
	q_vpd.cmd 	 = CFG_QVPD ;	/* command			*/
	q_vpd.ddsptr = (char *)vpd_area;/* area to hold vpd information	*/
	q_vpd.ddslen = VPDSIZE-1;        /* size of vpd information	*/
 
	/*  call sysconfig   */

	DEBUG_1("qvpd devno = %d\n",q_vpd.devno)
	rc = sysconfig(SYS_CFGDD,&q_vpd,sizeof(struct cfg_dd));
	if(rc < 0)
	{
	    return E_SYSCONFIG;
	}
	for ( i=0; i<256; i++) {
		if( vpd_area[i] == '\0') {
			vpd_dest[i] = ' ';
		}
		else
			vpd_dest[i] = vpd_area[i];
	}

	/* store the VPD in the database */
	put_vpd(vpd_dest,vpd_area,VPDSIZE);
 
	return 0;
 
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
 *	psla adapter has no children to define.
 * 
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns 0
 */
 
int define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device  		*/
int	ipl_phase;		/* ipl phase				*/
{
 
	return 0;
}
