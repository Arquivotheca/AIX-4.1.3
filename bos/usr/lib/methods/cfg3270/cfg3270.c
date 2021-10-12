static char sccsid[] = "@(#)32	1.10  src/bos/usr/lib/methods/cfg3270/cfg3270.c, cfgmethods, bos411, 9428A410j 6/14/91 11:14:17";
/* 
 * COMPONENT_NAME: (CFGMETH) CFG3270 (configuration method for 3270 adapter)
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
#include <sys/cfgdb.h>

#include <sys/mode.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <string.h>
#include <sys/io3270.h>

#include <sys/cfgodm.h>
#include "cfgdebug.h"
#include <cf.h>

#define BUSID_OFFSET	0x800c0060
#define MKNOD_MODE (S_IFMPX|S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

/*
 *	EBCDIC char table.
 *	Index with ascii value to get its equivalent ebcdic code
 */
unsigned char eb_tab[256] = {
	0x00,0x01,0x02,0x03,0x37,0x2d,0x2e,0x2f,
	0x16,0x05,0x25,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x3c,0x3d,0x32,0x26,
	0x18,0x19,0x3f,0x27,0x1c,0x1d,0x1e,0x1f,
	0x40,0x5a,0x7f,0x7b,0x5b,0x6c,0x50,0x7d,
	0x4d,0x5d,0x5c,0x4e,0x6b,0x60,0x4b,0x61,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,
	0xf8,0xf9,0x7a,0x5e,0x4c,0x7e,0x6e,0x6f,
	0x7c,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
	0xc8,0xc9,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,
	0xd7,0xd8,0xd9,0xe2,0xe3,0xe4,0xe5,0xe6,
	0xe7,0xe8,0xe9,0xad,0xe0,0xbd,0x9a,0x6d,
	0x79,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
	0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x96,
	0x97,0x98,0x99,0xa2,0xa3,0xa4,0xa5,0xa6,
	0xa7,0xa8,0xa9,0xc0,0x4f,0xd0,0x5f,0x07,
	0x20,0x21,0x22,0x23,0x24,0x15,0x06,0x17,
	0x28,0x29,0x2a,0x2b,0x2c,0x09,0x0a,0x1b,
	0x30,0x31,0x1a,0x33,0x34,0x35,0x36,0x08,
	0x38,0x39,0x3a,0x3b,0x04,0x14,0x3e,0xe1,
	0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,
	0x49,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
	0x58,0x59,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x70,0x71,0x72,0x73,0x74,0x75,
	0x76,0x77,0x78,0x80,0x8a,0x8b,0x8c,0x8d,
	0x8e,0x8f,0x90,0x6a,0x9b,0x9c,0x9d,0x9e,
	0x9f,0xa0,0xaa,0xab,0xac,0x4a,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,
	0xb8,0xb9,0xba,0xbb,0xbc,0xa1,0xbe,0xbf,
	0xca,0xcb,0xcc,0xcd,0xce,0xcf,0xda,0xdb,
	0xdc,0xdd,0xde,0xdf,0xea,0xeb,0xec,0xed,
	0xee,0xef,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};
#define EBCDIC(X) (eb_tab[X &0xff])


/* external functions */
extern	char *malloc();
extern	long *genminor();

/*
 * NAME     : build_dds 
 *
 * FUNCTION : This function builds the DDS for 3270 adapter 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the generic configure method to 
 *	build the define data structure which defines the attributes
 *	of the adapter.	
 * 
 * NOTES :
 *	This function gets the values of the attributes of 3270 adapter
 *	from ODM database , assigns the values to the DDS structure
 * 	and returns a pointer to the dds and its size.
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */



int
build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device	*/
char	**ddsptr;			/* pointer to dds structure 	*/
int 	*dds_len;			/* size of dds 			*/
{
	c327_dds  *dds_data; 		/* pointer to dds structure 	*/
	struct	Class	*cusdev;	/* customized device handle 	*/
	struct	Class	*preatt;	/* predefined attribute handle 	*/
	struct	Class	*cusatt;	/* customized attribute handle 	*/
	struct	Class	*cuvpd;		/* vpd handle			*/
	struct	CuDv	cusobj ;	/* customized object 		*/
	struct	CuVPD	vpdobj ;	/* Customized VPD for sysunit0	*/
	char	sstring[256];		/* search criteria string 	*/
	char	*tmpstr;		/* temp pointer to string 	*/
	int	slotno;			/* slot number 			*/
	short	printer_addr;		/* printer address		*/
	int	num_of_printers;	/* number of sessions		*/
	int	rc;			/* return code 			*/
	int	i;			/* index 			*/
	int	printer_no;		/* Printer 0..7			*/


	num_of_printers = 0;		/* initialize number of sessions */

	/* get customized object */
	sprintf(sstring,"name = '%s'",lname);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("cfg3270 : CuDv object not found\n" )
		return(E_NOCuDv);
	}
	else if ( rc == -1 ) {
	/* odm error occured */
		DEBUG_0("cfg3270 : get_obj from CuDv failed\n")
		return(E_ODMGET);
	}

	/* open the predefined attribute class */
	preatt = odm_open_class(PdAt_CLASS);
	if(preatt == NULL){
		DEBUG_0("cfg3270 :cannot open PdAt\n")
		return(E_ODMOPEN);
	}

	/* open the customized attribute class */
	cusatt = odm_open_class(CuAt_CLASS);
	if(cusatt == NULL){
		DEBUG_0("cfg3270 :cannot open CuAt\n")
		return(E_ODMOPEN);
	}

	/* allocate memory for dds structure */
	dds_data = (c327_dds *)malloc(sizeof(c327_dds));
	if(dds_data == NULL){
		DEBUG_0("cfg3270: malloc failed\n")
		return(E_MALLOC);
	}
	
	if(( rc = getatt(&dds_data->bus_mem_beg,'l',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"bus_mem_addr", (struct attr *)NULL)) >0)
		return rc;
	
	dds_data->bus_mem_size = 0x2000 ;
	
	if(( rc = getatt(&dds_data->io_port,'l',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue,"bus_io_addr",(struct attr *)NULL)) >0)
		return rc;
	
	if(( rc = getatt(&dds_data->bus_intr_lvl,'i',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue, "bus_intr_lvl",(struct attr *)NULL)) >0)
		return rc;

	if(( rc = getatt(&dds_data->intr_priority,'i',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue, "intr_priority",(struct attr *)NULL)) >0)
		return rc;

	for( printer_no=0; printer_no<8; printer_no++ ) {

		sprintf( sstring, "printer%d_addr", printer_no+1 );
		
		if(( rc = getatt(&printer_addr,'h',cusatt,preatt,lname,
			cusobj.PdDvLn_Lvalue,sstring, (struct attr *)NULL)) >0)
			return rc;
		if( printer_addr == 0x100)
			dds_data->printer_addr[printer_no]= -1 ;
		else {
			dds_data->printer_addr[printer_no] = printer_addr ;
			num_of_printers++ ;
		}
	}

	if(( rc = getatt(&dds_data->num_sessions,'i',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue, "num_sessions", (struct attr *)NULL)) >0)
		return rc;

	/* check whether number of sessions >= number of printers */
	if( dds_data->num_sessions  < num_of_printers){
		DEBUG_0("cdf3270 : invalid data, for number of printers\n")
		return E_BADATTR;
	}

	if((rc = getatt(&dds_data->buffer_size,'i',cusatt,preatt,lname,
		cusobj.PdDvLn_Lvalue, "buffer_size", (struct attr *)NULL)) >0)
		return rc;

	/* Note device driver uses slot numbers 0,1.. connwhere is 1,2.. */
	slotno = atoi(cusobj.connwhere) - 1;
	dds_data->slot_number = slotno ;

	/* get information from parent */
	if(cusobj.parent[0] == '\0'){
		DEBUG_0("cfg3270 : error getting parent name \n")
		return E_PARENT;
	}
	
	/* get customized object of parent  */
	sprintf(sstring,"name = '%s'",cusobj.parent);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("cfg3270 : parent object not found\n")
		return E_NOCuDvPARENT;
	}
	else if(rc == -1){
		/* odm error occured */
		DEBUG_0("cfg3270 : get_obj failed \n")
		return E_ODMGET;
	}

	if((rc = getatt(&dds_data->bus_id,'l',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_id", (struct attr *)NULL))>0)
		return rc;

	dds_data->bus_id |= BUSID_OFFSET;
	
	if((rc = getatt(&dds_data->bus_type,'h',cusatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_type", (struct attr *)NULL))>0)
		return rc;
	
	/* get customized object of system unit */
	sprintf(sstring,"name = '%s'","sysunit0");
	rc = (int)odm_get_obj(CuVPD_CLASS,sstring,&vpdobj,TRUE);
	if( rc == 0 )
		vpdobj.vpd[0] = '\0';
	if(rc == -1){
	/* odm error occured */
		DEBUG_0("cfg3270 : get_obj failed \n")
		return E_ODMGET;
	}
	/* get the value of machine_type number and model number    */
	tmpstr = read_descriptor(vpdobj.vpd,"TM");
	DEBUG_1("cfg3270: machine type & model no = %s\n",tmpstr)
	if(tmpstr == NULL){
		/* get vpd object of rack  */
		sprintf(sstring,"name = '%s'","rack0");
		rc = (int)odm_get_obj(CuVPD_CLASS,sstring,&vpdobj,TRUE);
		if( rc == 0 )
			vpdobj.vpd[0] = '\0';
		if(rc == -1){
		/* odm error occured */
			DEBUG_0("cfg3270 : get_obj failed \n")
			return E_ODMGET;
		}
		/* get the value of machine_type number and model number    */
		tmpstr = read_descriptor(vpdobj.vpd,"TM");
		DEBUG_1("cfg3270: machine type & model no = %s\n",tmpstr)
		if(tmpstr[0] == '\0'){
			DEBUG_0("cfg3270: error in getting machine type\n")
/*
			return(E_VPD);
*/
		}
	}
	ascii_2_ebcdic(tmpstr,dds_data->machine_type_number,4);
	tmpstr += 4;
	ascii_2_ebcdic(tmpstr,dds_data->model_number,3);

	/* get the value of customer_id     */
	dds_data->customer_id = 0xE1 ;
	
	/* get the value of plant manufactured */
	tmpstr = read_descriptor(vpdobj.vpd,"MF");
	DEBUG_1("cfg3270: plant manufactured = %s\n",tmpstr)
	if(tmpstr[0] == '\0'){
		DEBUG_0("cfg3270 : error in getting plant_manufactured\n")
/*
		return(E_VPD);
*/
	}
	ascii_2_ebcdic(tmpstr,dds_data->plant_manufactured,2);

	/* get the value for the attribute serial_no     */
	tmpstr = read_descriptor(vpdobj.vpd,"SN");
	DEBUG_1("cfg3270: serial no = %s\n",tmpstr)
	if(tmpstr[0] == '\0' ) {
		DEBUG_0("cfg3270 : error in getting serial_no\n")
/*
		return(E_VPD);
*/
	}
	ascii_2_ebcdic(tmpstr,dds_data->serial_no,7);
	
	/* get VPD object of system planar  */
	sprintf(sstring,"name = '%s'","sysplanar0");
	rc = (int)odm_get_obj(CuVPD_CLASS,sstring,&vpdobj,TRUE);
	if(rc == 0)
		vpdobj.vpd[0] = '\0';
	else if(rc == -1){
	/* odm error occured */
		DEBUG_0("cfg3270 : get_obj failed \n")
		return(E_ODMGET);
	}
	/* get the value for the attribute software_release_level  */
	tmpstr = read_descriptor(vpdobj.vpd,"RL");
	DEBUG_1("cfg3270: software release level = %s\n",tmpstr)
	if(tmpstr[0] == '\0'){
		DEBUG_0("cfg3270 : error in getting software_release_level\n")
/*
		return(E_VPD);
*/
	}

	ascii_2_ebcdic(tmpstr,dds_data->software_release_level,3);

	/* get the value for the attribute ec_level    */
	tmpstr = read_descriptor(vpdobj.vpd,"EC");
	DEBUG_1("cfg3270: ec level = %s\n",tmpstr)
	if(tmpstr[0] == '\0') {
		DEBUG_0("cfg3270 : error in getting ec_level\n")
/*
		return(E_VPD);
*/
	}

	ascii_2_ebcdic(tmpstr,dds_data->ec_level,16);

	/* put the logical name in dev_name */
	strcpy(dds_data->dev_name,lname);

	/* close object classes */
	
	if(odm_close_class(preatt)) {
		DEBUG_0("cfg3270: error in closing object class PdAt\n")
		return(E_ODMCLOSE);
	}
	if(odm_close_class(cusatt)<0) {
		DEBUG_0("cfg3270: error in closing object class CuAt\n")
		return(E_ODMCLOSE);
	}
	
	*ddsptr = (caddr_t)dds_data;
	*dds_len = sizeof(c327_dds);

#ifdef	CFGDEBUG
	dump_dds(*ddsptr,*dds_len);
#endif	CFGDEBUG

	DEBUG_0("cfg3270 : build_dds successful \n")
	return(0);
}


/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates minor number for 3270 adapter
 *
 * EXECUTION ENVIRONMENT :
 *	This function  returns a value between 0 and 3 on success.
 * 
 * NOTES :
 *	The minor number for 3270 adapter can be between 0 and 3.
 *	This function calls the genminor function to generate a 
 *	minor number 
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns 0 on success, errno on error
 *	Stores minor number via last parameter
 */

 
int
generate_minor(lname,majorno,minordest)
char 	*lname;			/* logical name of the device 		*/
long	majorno;		/* major number of the device 		*/
long	*minordest;		/* Place to store minor number		*/
{
	long	*tmp;		/* return code 				*/

	DEBUG_0("generate minor number \n")
	/* call genminor function to generate one minor number */
	tmp = genminor(lname,majorno,-1,1,1,1);

	if( tmp == (long *)NULL )
		return E_MINORNO;

	*minordest = *tmp;
	return 0;
}




/*
 * NAME     : make_special_files 
 *
 * FUNCTION : This function creates special files for 3270 adapter 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is used to create special files for 3270 adapter
 *	It is called by the generic configure method. 
 * 
 * NOTES :
 *	Logical name of 3270 adapter  and device number are passed as
 *	parameters to this function. It checks for the validity of
 *	major and minor numbers, remove any previously created special
 *	file if one  present, create special file and change the
 *	mode of special file. 	
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Returns  0 on success, errno on failure.
 */

int
make_special_files(lname,devno)
char	*lname;			/* logical name of the device  		*/
dev_t	devno;			/* device number	       		*/
{
	DEBUG_0("creating special file for 3270\n")
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
 *	3270 adapter does not have microcode. This is a dummy
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
char	*lname;			/* logical name of the device 		*/
{

	DEBUG_0("download microde\n")
	
	/* null function */
	return (0);
}



/*
 * NAME     : query_vpd 
 *
 * FUNCTION : This function gets the vital product data if applicable.
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the generic configure method based
 *	on the has_vpd flag in the Predefined Device Object Class for 
 *	this device. 
 * 
 * NOTES :
 *	3270 adapter does not have VPD. This is a null function.
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 * 	Always returns 0 .
 */


int
query_vpd(newobj,kmid,devno,vpd_dest)
struct	CuDv	*newobj;		/* vpd info will be put in that */
mid_t	kmid;				/* kernal module id             */
dev_t	devno;				/* device number		*/
char	*vpd_dest;
{
	/* no vpd for 3270 adapter, null function */
	return(0);
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
 *	3270 adapter has no children to define.
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

	DEBUG_0("define children\n")

	/* No children for 3270 adapter */
	return(0);
}


/*
 * NAME     : ascii_2_ebcdic 
 *
 * FUNCTION : This function converts ascii to ebcdic characters. 
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the build_dds function to  
 *	convert ascii to ebcdic.
 * 
 * NOTES :
 *	
 * 
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 *
 */


ascii_2_ebcdic(str1,str2,n)
char	*str1 ;			/* string to hold ascii data 		*/
char	*str2 ;			/* string to hold ebcdic data		*/
int	n;			/* Number of chars to copy (max)	*/
{
	int	i ;
	for(i = 0; i < n && *str1 != '\0' ; *str2++ = EBCDIC(*str1++) , i++);
	*str2 = '\0' ;
}


