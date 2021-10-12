static char sccsid[] = "@(#)47  1.16  src/bos/usr/lib/methods/cfghia/cfgssla.c, cfgmethods, bos411, 9428A410j 7/21/92 10:23:09";
/* 
 * COMPONENT_NAME: (CFGMETH) CFGSSLA (configuration method for msla adapter,
	ssla mode)
 *
 * FUNCTIONS : 	build_dds, generate_minor, make_special_files,
 *		download_microcode, query_vpd, define_children.
 * 
 * ORIGINS : 27 
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
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
#include <sys/io3270.h>

#define	S_LENGTH	256
#define	SSLA_BUS_OFFSET	0x80000020
#define MKNOD_MODE (S_IFMPX|S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

/* external functions */

extern	char *malloc();
extern	long *genminor();


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
 * 	Returns  0 on success, < 0 on failure.
 */



int
build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device 	*/
char	**ddsptr;			/* pointer to dds structure 	*/
int 	*dds_len;			/* size of dds 			*/
{
	struct 	msla_dds  *dds_data; 	/* pointer to dds structure 	*/
	struct	Class	*preatt;	/* predefined attribute handle 	*/
	struct	Class	*cusatt;	/* customized attribute handle 	*/
	struct	Class	*cusdev;	/* customized device handle 	*/
	struct	CuDv	cusobj ;	/* customized object 		*/
	char	sstring[S_LENGTH];	/* search criteria string 	*/
	char	microcode[S_LENGTH];	/* search criteria string 	*/
	char	*tmpstr;		/* temp pointer to string 	*/
	int	slotno;			/* slot number 			*/
	int	rc;			/* return code 			*/
	int	i;			/* index 			*/
	static  Micro_Info micro_info;	/* micocode struct pointer	*/
	FILE *fp, *fopen();		/* file pointer			*/
	long	code_length ;		/* microcode length		*/
	long	num_bytes ;		/* number of bytes of microcode */
	uchar	*ucode_buff ;		/* pointer to ucode 		*/

	/* open the customized object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
	{	DEBUG_0("cfgssla: error in opening CuDv\n")
		return(E_ODMOPEN);
	}
	/* get customized object */
	sprintf(sstring,"name = '%s'",lname);
	rc = (int)odm_get_obj(cusdev,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("cfgssla: error in getting object\n")
		return(E_NOCuDv);
	}
	else if(rc == -1){
	/* odm error occured */
		DEBUG_0("cfgssla: get_obj failed \n")
		return(E_ODMGET);
	}
	/* open the predefined attribute class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1)
	{	DEBUG_0("cfgssla: cannot open PdAt\n")
		return(E_ODMOPEN);
	}

	/* open the customized attribute class */
	if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1)
	{	DEBUG_0("cfgssla: cannot open CuAt\n")
		return(E_ODMOPEN);
	}

	/* allocate memory for dds structure */
	dds_data = (struct msla_dds *)malloc(sizeof(struct msla_dds));
	if(dds_data == NULL){
		DEBUG_0("cfgssla: malloc failed\n")
		return(E_MALLOC);
	}
	/* get the value for the attribute num_sessions   */
	rc = getatt(&dds_data->num_of_sessions,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"num_sessions", (struct att *)NULL);
	if(rc > 0)
	{	DEBUG_0("cfgssla: error in getting num_sessions\n")
		return(rc);
	}
	
	/* get the value for the attribute buffer_size  */
	rc = getatt(&dds_data->transfer_buff_size,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"buffer_size", (struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting buffer_size\n")
		return(rc);
	}
	
	/* get the value for the attribute lower_bond */
	rc = getatt(&dds_data->lower_la,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"lower_bond", (struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting lower_bond\n")
		return(rc);
	}
	/* calculate attribute upper bound */
	dds_data->upper_la=dds_data->lower_la + dds_data->num_of_sessions - 1;
	DEBUG_1("cfgssla: upper_bond 0x%x\n",dds_data->upper_la)
	
	/* get the value for the attribute link_speed  */
	rc = getatt(&dds_data->link_speed,'i',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"link_speed", (struct att *)NULL);
	if (rc > 0){
		DEBUG_0("cfgssla: error in getting link_speed\n")
		return(rc);
	}
	/* get the value for the attribute num_5080_sess   */
	rc = getatt(&dds_data->num_5080_sess,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"num_5080_sess", (struct att *)NULL);
	if(rc > 0)
	{	DEBUG_0("cfgssla: error in getting num_5080_sess\n")
		return(rc);
	}
	
	
	/* get the value for the attribute lower_5080_bond */
	rc = getatt(&dds_data->lower_5080_la,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"lower_5080_bond", (struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting lower_5080_bond\n")
		return(rc);
	}
	/* calculate attribute upper bound */
	dds_data->upper_5080_la=dds_data->lower_5080_la + dds_data->num_5080_sess - 1;
	
	/* get the value for the attribute channel address for 5080 */
	rc = getatt(&dds_data->addr_5080_chan,'h',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"addr_5080_chan", (struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in channel address\n")
		return(rc);
	}


	/* get the value for the attribute microcode  */
	rc = getatt(microcode,'s',cusatt,preatt,lname, 
		cusobj.PdDvLn_Lvalue,"microcode", (struct att *)NULL); 
	if (rc > 0){
		DEBUG_0("cfgssla: error in getting microcode\n")
		return(rc);
	}

	/* get information from parent adapter */
	tmpstr = cusobj.parent ;
	if(*tmpstr == '\0'){
		DEBUG_0("cfgssla: error in getting parent object\n")
		return(E_NOCuDvPARENT);
	}
	
	/*get the customized object of parent to get the unique type */
	sprintf(sstring,"name = '%s'",tmpstr);
	rc = (int)odm_get_obj(cusdev,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("cfgssla: error in getting parent object\n")
		return(E_NOCuDv);
	}
	else if(rc == -1){
		/* odm error occured */
		DEBUG_0("cfgssla: get_obj failed\n")
		return(E_ODMGET);
	}
	/* get the value for the attribute bus_mem_beg   */
	rc = getatt(&dds_data->bus_mem_beg,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_mem_addr", (struct att *)NULL);
	if(rc > 0){	
		DEBUG_0("cfgssla: error in getting attribute bus_mem_addr\n")
		return(rc);
	}
	
	/* get the value for the bus_mem_size   */
	/* Is there supposed to be a call like getwidth to get this value??? - sdb 12/13 */
	dds_data->bus_mem_size = 0x10000 ; 
	
	
	/* get the value for the attribute io_port   */
	rc = getatt(&dds_data->io_port,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_io_addr",(struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting attribute bus_io_addr\n")
		return(rc);
	}
	
	/* get the value for the attribute bus_intr_lvl    */
	rc = getatt(&dds_data->bus_intr_level,'l',cusatt,preatt,
		cusobj.name,cusobj.PdDvLn_Lvalue,
		"bus_intr_lvl",(struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting bus_intr_lvl\n")
		return (rc);
	}
	/* get the value for the attribute bus_intr_pri    */
	rc = getatt(&dds_data->bus_intr_pri,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"intr_priority", (struct att *)NULL);
	if(rc > 0){

		DEBUG_0("cfgssla: error in getting attribute intr_priority\n");
		return (rc);
	}
	
	/* get the value for the attribute  dma_lvl */
	rc = getatt(&dds_data->dma_arb_level,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"dma_lvl", (struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting dma_lvl\n")
		return(rc);
	}
	strcpy(dds_data->dev_name,lname);	
	
	/* get the value for the attribute dma_bus_mem */
	rc = getatt(&dds_data->dma_base,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"dma_bus_mem", (struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting dma_bus_mem\n")
		return(rc);
	}

	/* get the value for the attribute slot_number */
	tmpstr = cusobj.connwhere;
	if(*tmpstr == '\0'){
		DEBUG_0("cfgssla: error in getting connection info\n")
		return(E_PARENT);
	}
	slotno = atoi(tmpstr);
	slotno--;
	DEBUG_1("cfghia: slotno: 0x%x\n",slotno)
	dds_data->slot_number = slotno ;

	/* get information from parent */
	tmpstr = cusobj.parent;
	if(*tmpstr == '\0'){
		DEBUG_0("cfgssla: error getting parent name \n")
		return(E_NOCuDvPARENT);
	}
	
	/*get the customized object of parent to get the unique type */
	sprintf(sstring,"name = '%s'",tmpstr);
	rc = (int)odm_get_obj(cusdev,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("cfgssla: error in getting grandparent object\n")
		return(E_NOCuDv);
	}
	else if(rc == -1){
		/* odm error occured */
		DEBUG_0("cfgssla: get_obj failed\n")
		return(E_ODMGET);
	}
	/* get the value for the attribute bus_id */
	rc = getatt(&dds_data->bus_id,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_id", (struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting bus_id\n")
		return(rc);
	}
	dds_data->bus_id |= SSLA_BUS_OFFSET;
	
	/* get the value for the attribute bus_type */
	rc = getatt(&dds_data->bus_type,'h',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_type",
		(struct att *)NULL);
	if(rc > 0){
		DEBUG_0("cfgssla: error in getting bus type\n")
		return(rc);
	}
	/* close object class */
	rc = odm_close_class(cusdev);
	if(rc < 0){
		DEBUG_0("cfgssla: error in closing object class CuDv\n")
		return(E_ODMCLOSE);
	}
	rc = odm_close_class(preatt);
	if(rc < 0){
		DEBUG_0("cfgssla: error in closing object class PdAt\n")
		return(E_ODMCLOSE);
	}
	rc = odm_close_class(cusatt);
	if(rc < 0){
		DEBUG_0("cfgssla: error in closing object class CuAt\n")
		return(E_ODMCLOSE);
	}

	/* get the microcode pointer */

	if((fp = fopen(microcode,"r")) == NULL){
		DEBUG_1("cfgssla: cannot open %s\n",microcode)
		return(E_NOUCODE);
	}
	if(fseek(fp,0,2) != 0){
		DEBUG_1("cfgssla: error in %s\n",microcode)
		return(E_UCODE);
	}
	if((code_length = ftell(fp)) == -1L){
		DEBUG_1("cfgssla: error in the size of %s\n",microcode)
		return(E_UCODE);
	}
	if((ucode_buff = (char *)malloc(code_length+1)) == NULL){
		DEBUG_0("cfgssla: malloc ucode_buff failed\n")
		return(E_MALLOC);
	}
	if(fseek(fp,0,0) != 0){
		DEBUG_1("cfgssla: error seeking in %s\n",microcode)
		return(E_UCODE);
	}
	if(fread(ucode_buff,1,code_length,fp) != code_length){
		DEBUG_1("cfgssla: error reading %s\n",microcode)
		return(E_UCODE);
	}

	/* setup values for download ioctl command */

	micro_info.size = code_length;
	micro_info.start = ucode_buff;
	micro_info.type = 0xa ;
	dds_data->micro_info = &micro_info ;
	
	*ddsptr = (caddr_t)dds_data;
	*dds_len = sizeof(struct msla_dds);

#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif	CFGDEBUG
	DEBUG_0("cfgssla: build_dds successful \n")
	return(E_OK);
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
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns 0 on success , > 0 on failure.	
 */

 
int
generate_minor(lname,majorno,minorno)
char 	*lname;			/* logical name of the device 	*/
int 	majorno;			/* major number of the device 	*/
int 	*minorno;		/* minor number of the device 	*/
{
	int	rc;			/* return code			*/
        int     *tempminorno;/*emrmult*/

	DEBUG_0("cfghia: generate minor number \n")
	/* call genminor function to generate one minor number */
	tempminorno = (int *) genminor(lname,majorno,-1,1,1,1);
	if (tempminorno)/*emrmult*/
	{
                *minorno = *tempminorno;/*emrmult*/
		DEBUG_1("cfghia: minor number = %d\n",*minorno)
		return(E_OK);
	}
	else
	{
		DEBUG_0("cfghia: error getting minor number")
		return(E_MINORNO);
	}
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
 *	Logical name of msla adapter  and device number are passed as
 *	parameters to this function. It checks for the validity of
 *	major and minor numbers, removes any previously created special
 *	file if one  present, creates special file and changes the
 *	mode of special file. 	
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns  0 on success, < 0 on failure.
 */

int
make_special_files(lname,devno)
char	*lname;			/* logical name of the device  		*/
dev_t	devno;			/* device number	       		*/
{
	struct	Class	*cusdev;	/* customized device handle 	*/
	struct	CuDv	cusobj ;	/* customized object 		*/
	struct	CuDv 	*cudvp;		/* Pointer to query results   */
	struct	objlistinfo cudv_info;	/* Results of search stats	*/
	char	sstring[S_LENGTH];	/* search criteria string 	*/
	int	rc;			/* return code 			*/
	int err=0;		/* error flag			*/
	int	i;	

	DEBUG_0("cfghia: in make_special_files\n")
	/* open the customized object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
	{	DEBUG_0("cfghia: error in opening CuDv\n")
		return(E_ODMOPEN);
	}
	/* Find all HIA adapters configured */
	sprintf(sstring,"PdDvLn = msla/msla/hia");
	cudvp = (struct CuDv *)odm_get_list(cusdev,sstring,
                 &cudv_info,1,1);
	if (cudvp == (struct CuDv *) -1)
	{
		DEBUG_0("cfghia: odm_get_list failed\n")
		err=E_ODMGET;
	}
	if (cudv_info.num > 0)		/* HIA found */
	{
		DEBUG_1("cudv_info.num = %d\n", cudv_info.num)
		/* See if HIA is available. */
		for( i = 0; i < cudv_info.num; i++ )
		{ 
			/* Criteria for next serial adapter. */
			sprintf(sstring,"name = '%s'",cudvp->name);
			cudvp++; /* increment to next name in list*/
			rc = odm_get_first(cusdev, sstring, &cusobj);
			if ( rc < 0 )
			{
				DEBUG_0("cfghia: odm_get_first failed\n")
				err = E_ODMGET;
				break;
			}
			if ( rc != NULL && cusobj.status == AVAILABLE)
			{
			/*	DEBUG_0("cfghia: An HIA device has already been configured\n")
				err = E_ALREADYDEF;emrmult*/
			}
			DEBUG_1("cusobj->name = %s\n", cusobj.name)
		}  /* for */
	}
	else 
	{
		DEBUG_0("cfghia: Nothing defined in CuDv for HIA\n")
		err = E_NOCuDv;
	}
	rc = odm_close_class(cusdev);
	if(rc < 0){
		DEBUG_0("cfgssla: error in closing object class CuDv\n")
		return(E_ODMCLOSE);
	}
	if (err )
		return(err);
	DEBUG_0("cfgssla: creating special file for ssla\n")
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
 * 	Returns  0 on success, > 0 on failure.
 */

int
query_vpd(newobj,kmid,devno,str512)
struct	CuDv	*newobj;	/* vpd info will be put in that 	*/
mid_t	kmid;			/* kernal module id             	*/
dev_t	devno;			/* device number			*/
char  	*str512;
{
	int 	rc;		/* return code  			*/
	struct 	cfg_dd  q_vpd;	/* vpd structure to pass to sysconfig 	*/
	char	*vpd_area;	/* buffer to hold vpd information 	*/
	char	*rl_vpd_area;	
	int 	*len_of_vpd;
	
	/* allocate memory to hold error message	*/
	vpd_area = (char *)malloc(VPDSIZE);
	if(vpd_area == NULL){
		DEBUG_0("cfgssla: malloc failed")
		return(E_MALLOC);
	}

	/* initialize the q_vpd structure   */
	q_vpd.kmid 	= kmid;		/* kernal module id		*/
	q_vpd.devno 	= devno;	/* device number		*/
	q_vpd.cmd 	= CFG_QVPD ;	/* command			*/
	q_vpd.ddsptr	= vpd_area ;	/* area to hold vpd information	*/
	q_vpd.ddslen	= VPDSIZE;  /* size of vpd info*/

	/*  call sysconfig   */
	rc = sysconfig(SYS_CFGDD,&q_vpd,sizeof(struct cfg_dd));
/* Added the next stmt and ifdef'd out next stuff temporarily-sdb*/
	if (rc < 0)
	{
		switch(errno){
		case EACCES:
			DEBUG_0("cfgssla: query vpd: no required privilege\n")
			return(E_VPD);

		case EFAULT:
			DEBUG_0("cfgssla: query vpd: i/o error\n")
			return(E_VPD);

		case EINVAL:
			DEBUG_1("cfgssla: query vpd: invalid kmid=%d\n",kmid)
			return(E_VPD);

		case ENODEV:
			DEBUG_1("cfgssla: query vpd: invalid dev number =0x%x\n",devno)
			return (E_VPD);

		default:
			DEBUG_1("cfgssla: query vpd: error = 0x%x\n",errno)
			return(E_VPD);
		}
	}
	/* store the VPD in the database */
	
	rl_vpd_area = vpd_area + sizeof(int);
	len_of_vpd = (int *)vpd_area;
#ifdef	CFGDEBUG
	dump_dds(vpd_area,*len_of_vpd+sizeof(int));
#endif	CFGDEBUG
	strcpy(str512,rl_vpd_area);
	DEBUG_1("cfgssla: query vpd: successful, vpd length 0x%x\n",*len_of_vpd)
	return(E_OK);
}


/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines devices attached to this device 
 *
 * EXECUTION ENVIRONMENT :
 *	This function always returns 0
 * 
 * NOTES :
 *	msla adapter has no children to define.
 * 
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Always returns 0
 */

int
define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device  		*/
int	ipl_phase;		/* ipl phase				*/
{

	DEBUG_0("cfgssla: define children\n")

	/* No children for ssla mode of msla adapter */
	return(E_OK);
}


