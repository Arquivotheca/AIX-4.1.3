static char sccsid[] = "@(#)04	1.3  src/bos/usr/lib/methods/cfgddent/cfgddent.c, diagddent, bos411, 9430C411a 7/19/94 10:52:21";
/*
 * COMPONENT_NAME: DIAGDDENT
 *
 * FUNCTIONS: Configure Method for ethernet adpaters
 *            build_dds
 *            define_children
 *            download_microcode
 *            generate_minor
 *            make_special_files
 *            query_vpd
 *            get_dvdr
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <stdio.h>
#include <cf.h>		/* Error codes */

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h>	
#include <sys/entuser.h>

#include <entdiag/entddi.h>
#include <entdiag/cioddi.h>

#include "cfgdebug.h"
#include "pparms.h"

#ifdef CFGDEBUG
#include <errno.h>
#endif

extern	long	*genminor();

#define GETATT_L(A,B,C)         getatt(A,B,CAhdl,PAhdl,lname,utype,C, NULL)
#define GETATT_P(A,B,C)         getatt(A,B,CAhdl,PAhdl,pname,putype,C, NULL)
#define FROM_PARENTS            2

/* These attributes are hardcoded because there is no device object     */
/* entry in the database anymore.                                       */
#define REC_QUE_SIZE    30
#define STA_QUE_SIZE    5
#define RDTO            92
#define TYPE_FIELD_OFF  12
#define NET_ID_OFFSET   14

/*
 * NAME: build_dds()
 * 
 * FUNCTION: This calls the ethernet build dds routine with a null 
 *	     attribute list pointer.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This process is invoked from the generic config device routine
 *	to build the DDS structure for an ethernet adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int 
build_dds(lname,ddsptr,ddslen)
char	*lname;
uchar	**ddsptr;
int	*ddslen;
{
	ddi_t   *ddi_ptr;
	struct  Class   *CDhdl,         /* Customized Devices OC handle */
			*CAhdl,         /* Customized Attribute OC handle */
			*PAhdl;         /* Predefined Attribute OC handle */
	struct  CuDv    cust_obj;
	struct  CuDv    bus_obj;
	struct  CuAt    *CuAt_obj;      /* Customized Attributes data struct */
	struct  PdAt    PA_obj;
	char    *parentdesc,pname[NAMESIZE],
		srchstr[256],
		utype[UNIQUESIZE],
		putype[UNIQUESIZE],
		*ptr,
		tmp_str[5];     /* yes,no value for flag fields */
	int     i,rc;
	long    mem_width;              /* temporary variable */
	int     bnc_select;     /* Wire type returned from NVRAM    */
	int     bus_num;        /* Bus number                       */
	int     how_many;               /* Used by getattr routine. */

	DEBUG_0("build_dds(): BEGIN build_dds()\n")
	*ddslen=sizeof(ddi_t);
	if ( (ddi_ptr=(ddi_t *) malloc(sizeof(ddi_t))) == (ddi_t *) NULL) {
		DEBUG_0 ("build_dds(): Malloc of dds failed\n")
		return E_MALLOC;
	}
	*ddsptr=(uchar *) ddi_ptr;

	/* set default value for all attributes to zero */
	bzero(ddi_ptr,sizeof(ddi_t));           /* zero out dds structure */

	/* open Customized Attribute OC */
	if((int)(CAhdl=odm_open_class(CuAt_CLASS))== -1) {
		DEBUG_0 ("build_dds(): failure to open CuAt\n")
		return E_ODMOPEN;
	}
	/* open Predefined Attribute OC */
	if((int)(PAhdl=odm_open_class(PdAt_CLASS))== -1) {
		DEBUG_0 ("build_dds(): failure to open PdAt\n")
		return E_ODMOPEN;
	}
	/* open Customized Devices OC */
	if((int)(CDhdl=odm_open_class(CuDv_CLASS))== -1)   {
		DEBUG_0 ("build_dds(): failure to open CuDv\n")
		return E_ODMOPEN;
	}
	/* Get customized object */
	sprintf(srchstr,"name = '%s'",lname);
	if ((rc=(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST)) == 0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;
	strcpy(utype,cust_obj.PdDvLn_Lvalue);

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/*              Regular Attributes                               */
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	if(rc=GETATT_L(&DDI_CC.intr_level,'i',"bus_intr_lvl"))   {
		DEBUG_0 ("build_dds(): failure to read bus_intr_lvl\n")
		return(rc);
	}
	if(rc=GETATT_L(&DDI_CC.intr_priority,'i',"intr_priority")) {
		DEBUG_0 ("build_dds(): failure to read intr_priority\n")
		return(rc);
	}
	if(rc=GETATT_L(&DDI_CC.xmt_que_size,'i',"xmt_que_size"))  {
		DEBUG_0 ("build_dds(): failure to read xmt_que_size\n")
		return(rc);
	}

	/* this attribute is not read from database */
	DDI_CC.rec_que_size = REC_QUE_SIZE;
	/* this attribute is not read from database */
	DDI_CC.sta_que_size = STA_QUE_SIZE;
	/* this attribute is not read from database */
	DDI_CC.rdto = RDTO;

	if(rc=GETATT_L(&DDI_DS.bus_mem_addr,'a',"bus_mem_addr"))    {
		DEBUG_0 ("build_dds(): failure to read bus_mem_addr\n")
		return(rc);
	}
	if(rc=GETATT_L(&DDI_DS.tcw_bus_mem_addr,'a',"dma_bus_mem")) {
		DEBUG_0 ("build_dds(): failure to read dma_bus_mem\n")
		return(rc);
	}
	if(rc=GETATT_L(&DDI_DS.io_port,'a',"bus_io_addr"))          {
		DEBUG_0 ("build_dds(): failure to read bus_io_addr\n")
		return(rc);
	}
	if(rc=GETATT_L(&DDI_DS.dma_arbit_lvl,'i',"dma_lvl"))        {
		DEBUG_0 ("build_dds(): failure to read dma_lvl\n")
		return(rc);
	}
	if(rc=GETATT_L(tmp_str,'s',"bnc_select"))                   {
		DEBUG_0 ("build_dds(): failure to read bnc_select\n")
		return(rc);
	}
	DDI_DS.bnc_select = (tmp_str[0] == 'd') ? 0 : 1;
	if(rc=GETATT_L(tmp_str,'s',"use_alt_addr"))  {
		DEBUG_0 ("build_dds(): failure to read use_alt_addr\n")
		return(rc);
	}
	DDI_DS.use_alt_addr = (tmp_str[0] == 'y') ? 1 : 0;
	if (DDI_DS.use_alt_addr) {
		if((rc=GETATT_L(DDI_DS.alt_addr,'b',"alt_addr")) != -6) {
			if (rc > 0)  {
				DEBUG_0 ("build_dds(): failure to read alt_addr\n")
				return(rc);
			}
			else {
				DEBUG_1("build_dds(): getatt() bytes=%d\n",rc)
				return E_BADATTR;
			}
		}
		DEBUG_6("build_dds(): alt_addr=%02x %02x %02x %02x %02x %02x\n",
		  (int) DDI_DS.alt_addr[0],(int) DDI_DS.alt_addr[1],
		  (int) DDI_DS.alt_addr[2],(int) DDI_DS.alt_addr[3],
		  (int) DDI_DS.alt_addr[4],(int) DDI_DS.alt_addr[5])
	}

	/* this attribute is not read from database */
	DDI_DS.type_field_off = TYPE_FIELD_OFF;
	/* this attribute is not read from database */
	DDI_DS.net_id_offset = NET_ID_OFFSET;

	/* Attributes from other Object Classes/Places */

	/* Get logical name */
	DEBUG_1("build_dds(): logical name = %s\n",lname)
	strncpy(DDI_DS.lname,lname,4);
	DEBUG_1("build_dds(): lname = %4s\n",lname)

	/* Get bus memory size */
	DEBUG_1("build_dds(): uniquetype = %s\n",utype)
	sprintf(srchstr,"uniquetype = '%s' AND attribute = '%s'",utype,
							"bus_mem_addr");
	DEBUG_1("build_dds(): criteria = '%s'\n",srchstr)
	if((rc=(int)odm_get_obj(PAhdl,srchstr,&PA_obj,ODM_FIRST)) == 0)
		return E_NOPdOBJ;
	else if (rc == -1)
		return E_ODMGET;
	DEBUG_0("build_dds(): odm_get_obj() succeeded\n")

	DEBUG_1("build_dds(): PA_obj.width = %s\n",PA_obj.width)
	if((rc=convert_att(&mem_width,'l',PA_obj.width,'n')) != 0)
		return E_BADATTR;
	DEBUG_0("build_dds(): convert_att() succeeded\n")
	DEBUG_1("build_dds(): width = %ld\n",mem_width)
	if(mem_width==0x4000)
		DDI_DS.bus_mem_size=0;
	else if(mem_width==0x8000)
		DDI_DS.bus_mem_size=1;
	else
		DDI_DS.bus_mem_size=2;
	DEBUG_1("build_dds(): bus_mem_size = %ld\n",DDI_DS.bus_mem_size)

	/* Get tcw bus memory size */
	DEBUG_1("build_dds(): uniquetype = %s\n",utype)
	sprintf(srchstr,"uniquetype = '%s' AND attribute = '%s'",utype,
							"dma_bus_mem");
	DEBUG_1("build_dds(): criteria = '%s'\n",srchstr)
	if((rc=(int)odm_get_obj(PAhdl,srchstr,&PA_obj,ODM_FIRST)) == 0)
		return E_NOPdOBJ;
	else if (rc == -1)
		return E_ODMGET;
	DEBUG_0("build_dds(): odm_get_obj() succeeded\n")
	DEBUG_1("build_dds(): PA_obj.width = %s\n",PA_obj.width)
	if((rc=convert_att(&DDI_DS.tcw_bus_mem_size,'l',PA_obj.width,'n'))!=0)
		return E_BADATTR;
	DEBUG_0("build_dds(): convert_att() succeeded\n")
	DEBUG_1("build_dds(): tcw_bus_mem_size = %ld\n",DDI_DS.tcw_bus_mem_size)

	/*
	 *  Get bus type, bus id, and bus number.
	 */
	rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj);
	if (rc) {
	    if (rc == E_PARENT)
		rc = E_NOCuDvPARENT;
	    return (rc);
	}
	strcpy(pname,bus_obj.name);
	strcpy(putype,bus_obj.PdDvLn_Lvalue);
	if(rc=GETATT_P(&DDI_CC.bus_type,'i',"bus_type")) {
		DEBUG_0("build_dds(): failure to read bus_type\n")
		return(rc);
	}
	DEBUG_1("build_dds(): bus_type=%d\n", DDI_CC.bus_type)
	if(rc=GETATT_P(&DDI_CC.bus_id,'i',"bus_id")) {
		DEBUG_0("build_dds(): failure to read bus_id\n")
		return(rc);
	}
	DDI_CC.bus_id |= 0x800C0060;
	DEBUG_1("build_dds(): bus_id=0x%x\n", DDI_CC.bus_id)

	/*
	 * Get bus number.  Location = 00-00 (??-<bus><slot>)
	 * Subtract '0' (0x30) to get numberical bus value
	 */
	bus_num = bus_obj.location[3] - '0';

	/* Get slot */
	DEBUG_0("build_dds(): getting slot from CuDv\n")
	sprintf(srchstr,"name = '%s'",lname);
	if((rc=(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST))==0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;
	strcpy(srchstr,cust_obj.connwhere);
	DEBUG_1("build_dds(): connwhere=%s\n",srchstr)

	DDI_DS.slot=atoi(&srchstr[strcspn(srchstr,"0123456789")]);
	DDI_DS.slot--;
	DEBUG_1("build_dds(): slot=%d\n",DDI_DS.slot)


#ifdef CFGDEBUG
	hexdump(ddi_ptr,(long) sizeof(ddi_t));
#endif

	if (odm_close_class(PdAt_CLASS) < 0)
		return E_ODMCLOSE;
	if (odm_close_class(CuAt_CLASS) < 0)
		return E_ODMCLOSE;
	if (odm_close_class(CuDv_CLASS) < 0)
		return E_ODMCLOSE;
	return 0;
}

/*
 * NAME: generate_minor()
 *
 * FUNCTION:
 *	This calls genminor() to get the minor number for this logical
 *	device name.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
long generate_minor(lname,majorno,minorno)
char	*lname;
long	majorno;
long	*minorno;
{
	long *minorptr;

	DEBUG_0("generate_minor(): BEGIN generate_minor()\n")
	DEBUG_2("generate_minor(): lname=%s, major=%ld\n",lname,majorno)
	minorptr = genminor(lname,majorno,-1,1,1,1);
	if (minorptr == (long *) NULL)
		return E_MINORNO;
	else
		*minorno = *minorptr;
	return 0;
}

/*
 * NAME:
 *	make_special_files()
 * 
 * FUNCTION:
 *	This creates a special file with the same name as the logical
 *	device name.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int make_special_files(lname,devno)
char	*lname;
dev_t	devno;
{
	long	create_flags;

	DEBUG_0("make_special_files(): BEGIN make_special_file()\n")
	create_flags = ( S_IFMPX | S_IFCHR | S_IRUSR | S_IWUSR | \
                         S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
	return(mk_sp_file(devno,lname,create_flags));
}

/*
 * NAME:
 *	download_microcode()
 * 
 * FUNCTION:
 *	There is no microcode.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int download_microcode(lname)
char	*lname;
{
	DEBUG_0("download_microcode(): NULL function returning 0\n")
	return(0);
}

/*
 * NAME:
 *	query_vpd()
 * 
 * FUNCTION:
 *	Retrieve Vital Product Data (VPD) from the adapter card in
 *	order to store it in the database from later use.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int query_vpd(newobj,kmid,devno,vpdstr)
struct	CuDv	*newobj;
mid_t	kmid;				/* Kernel module I.D. for Dev Driver */
dev_t	devno;				/* Concatenated Major & Minor No.s */
char	*vpdstr;			/* String to store vpd in */
{
	struct	cfg_dd	cfg;			/* dd config structure */
	struct	vital_product_data vpd_area;	/* Storage area for VPD */
	ulong	ul;

	DEBUG_0("query_vpd(): BEGIN query_vpd()\n")

	/* Initialize the cfg structure for a call to sysconfig(), which */
	/* will in turn call the ddconfig() section of the appropriate	 */
	/* device driver.						 */

	cfg.kmid = kmid;		/* the kmid of the device driver */
	cfg.devno = devno; 		/* concatenated Major#, and Minor# */
	cfg.cmd = CFG_QVPD;		/* Command to read VPD */
	cfg.ddsptr = (char*) &vpd_area;		/* Storage space for VPD */
	cfg.ddslen = sizeof(vpd_area);		/* Size of VPD storage area */

	/* Make the call to sysconfig: */
	if(sysconfig(SYS_CFGDD, &cfg, sizeof(cfg))<0) {
#ifdef CFGDEBUG
		DEBUG_0("query_vpd(): Dump of vpd_area\n")
		hexdump(&vpd_area,(long) sizeof(vpd_area));
		switch(errno) {
		case EINVAL:
			DEBUG_1("query(): invalid kmid = %d\n",kmid)
			break;
		case EACCES:
			DEBUG_0("query(): not privileged\n")
			break;
		case EFAULT:
			DEBUG_0("query(): i/o error\n")
			break;
		case ENODEV:
			DEBUG_1("query(): invalid devno = 0x%x\n",devno)
			break;
		default:
			DEBUG_1("query(): error = 0x%x\n",errno)
			break;
		}
#endif
		return E_VPD;
	}

#ifdef CFGDEBUG
	DEBUG_1("query_vpd(): status=%d     ",(int) vpd_area.status)
	switch((int) vpd_area.status) {
	case VPD_NOT_READ:
		DEBUG_0("query_vpd(): status=VPD NOT READ\n")
		break;
	case VPD_NOT_AVAIL:
		DEBUG_0("query_vpd(): status=VPD NOT AVAIL\n")
		break;
	case VPD_INVALID:
		DEBUG_0("query_vpd(): status=VPD INVALID\n")
		break;
	case VPD_VALID:
		DEBUG_0("query_vpd(): status=VPD VALID\n")
		break;
	default:
		DEBUG_0("query_vpd(): status=?\n")
		break;
	}
	DEBUG_0("query_vpd(): Dump of vpd\n")
	hexdump(vpd_area.vpd,(long) vpd_area.length);
#endif

	/* Store the VPD in the database */
	put_vpd(vpdstr,vpd_area.vpd,vpd_area.length);

	return(0);
}

/*
 * NAME:
 *	define_children()
 *
 * FUNCTION:
 *	There are no children.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int define_children(lname,cusdev,phase,children)
char	*lname,*children;
long	cusdev;
int	phase;
{
	DEBUG_0("define_children(): NULL function, returning 0\n")
	return(0);
}


/*
 * NAME:
 *      get_dvdr()
 *
 * FUNCTION:
 *      Returns name of diagnostic driver to be loaded.
 *
 * RETURNS:
 *      Always returns 0 - Success.
 */
int
get_dvdr(dvdr_name)
char    *dvdr_name;
{
	char	*mountpoint;
	char	driver_name[255];

	DEBUG_0("get_dvdr(): \n")
        if((mountpoint = (char *)getenv("CDRFS_DIR")) != (char *)NULL){
		sprintf(driver_name, "%s%s", mountpoint,
			"/usr/lib/drivers/entdiag");
		strcpy(dvdr_name, driver_name);
	} else
		strcpy(dvdr_name,"/usr/lib/drivers/entdiag");

	return(0);
}
