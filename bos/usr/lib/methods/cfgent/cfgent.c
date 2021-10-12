static char sccsid[] = "@(#)44  1.9  src/bos/usr/lib/methods/cfgent/cfgent.c, cfgmethods, bos411, 9428A410j 4/1/94 16:10:45";
/*
 * COMPONENT_NAME: (CFGMETHODS) Configure Method for ethernet adpaters
 *
 * FUNCTIONS:   build_dds
 *              download_microcode
 *              query_vpd
 *              define_children
 *              device_specific
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
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
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h>	
#include <sys/err_rec.h>

#include <sys/cdli_entuser.h>
#include <ent/en3com_dds.h>

#include "cfgdebug.h"
#include "pparms.h"
#include <sys/ciouser.h>
#include <sys/ndd.h>
#include "cfg_ndd.h"
#include "ciepkg.h"     /* header file for cfgcie method, see define_children() */

#ifdef CFGDEBUG
#include <errno.h>
#endif

extern	long	*genminor();

static  struct    attr_list *alist=NULL;   /* PdAt attribute list      */
int     how_many;               /* Used by getattr routine.            */
int     byte_cnt;               /* Byte count of attributes retrieved  */

#define GETATT(A,B,C)           getatt(alist,C,A,B,&byte_cnt)


/*
 * NAME: build_dds()
 * 
 * FUNCTION: This builds the dds for the ethernet device.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This process is invoked from the generic config device routine
 *	to build the DDS structure for an ethernet adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int build_dds(logical_name,odm_info,dds_ptr,dds_length)
char *logical_name;
ndd_cfg_odm_t *odm_info;
char **dds_ptr;
long *dds_length;
{

	en3com_dds_t   *ddi_ptr;
	struct  CuDv    cust_obj;
	struct  CuDv    bus_obj;
	struct  CuAt    *CuAt_obj;      /* Customized Attributes data struct */
	struct  PdAt    PA_obj;
	struct  attr_list   *bus_attr_list = NULL;  /* Bus Attributes list */
	char    *parentdesc,
		srchstr[256],
		utype[UNIQUESIZE],
		*ptr,
		tmp_str[5];     /* yes,no value for flag fields */
	int     i,rc;
	long    mem_width;              /* temporary variable */
	int     bnc_select;     /* Wire type returned from NVRAM    */
	int     bus_num;        /* Bus number                       */
	int     num_bus_attrs;  /* number of bus attributes     */

	DEBUG_0("build_dds(): BEGIN build_dds()\n")
	DEBUG_1("build_dds(): logical_name=%s\n",logical_name)
	*dds_length=sizeof(en3com_dds_t);
	if ( (ddi_ptr=(en3com_dds_t *) malloc(sizeof(en3com_dds_t))) == (en3com_dds_t *) NULL) {
		DEBUG_0 ("build_dds(): Malloc of dds failed\n")
		return E_MALLOC;
	}
	*dds_ptr=(uchar *) ddi_ptr;

	/* set default value for all attributes to zero */
	bzero(ddi_ptr,sizeof(en3com_dds_t));           /* zero out dds structure */

	/* Get customized object */
	sprintf(srchstr,"name = '%s'",logical_name);
	if ((rc=(int)odm_get_first(CuDv_CLASS,srchstr,&cust_obj)) == 0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;
	strcpy(utype,cust_obj.PdDvLn_Lvalue);

	/* Get slot */
	DEBUG_0("build_dds(): getting slot from CuDv\n")
	ddi_ptr->slot = atoi(cust_obj.connwhere) - 1;
	DEBUG_1("build_dds(): slot=%d\n",ddi_ptr->slot)

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/*              Regular Attributes                               */
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/*
	 *  Get attribute list
	 */
	if ((alist=get_attr_list(logical_name,utype,&how_many,16)) == NULL)
		return(E_ODMGET);


	if(rc=GETATT(&ddi_ptr->intr_level,'i',"bus_intr_lvl"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->intr_priority,'i',"intr_priority"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->xmt_que_size,'i',"xmt_que_size"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->rv_pool_size,'i',"rec_pool_size"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->bus_mem_addr,'a',"bus_mem_addr"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->tcw_bus_mem_addr,'a',"dma_bus_mem"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->io_port,'a',"bus_io_addr"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->dma_arbit_lvl,'i',"dma_lvl"))
		return(rc);
	if(rc=GETATT(tmp_str,'s',"bnc_select"))
		return(rc);
	ddi_ptr->bnc_select = (tmp_str[0] == 'd') ? 0 : 1;
	if(rc=GETATT(tmp_str,'s',"use_alt_addr"))
		return(rc);
	ddi_ptr->use_alt_addr = (tmp_str[0] == 'y') ? 1 : 0;

	if (ddi_ptr->use_alt_addr) {
		rc=GETATT(ddi_ptr->alt_addr,'b',"alt_addr");
		if(byte_cnt != 6) {
			if (rc > 0)
				return rc;
			else {
				DEBUG_1("build_dds(): getatt() bytes=%d\n",rc)
				return E_BADATTR;
			}
		}
		DEBUG_6("build_dds():alt_addr=%02x %02x %02x %02x %02x %02x\n",
		  (int) ddi_ptr->alt_addr[0],(int) ddi_ptr->alt_addr[1],
		  (int) ddi_ptr->alt_addr[2],(int) ddi_ptr->alt_addr[3],
		  (int) ddi_ptr->alt_addr[4],(int) ddi_ptr->alt_addr[5])
	}

	/* Get logical name */
	DEBUG_1("build_dds(): logical name = %s\n",logical_name)
	strcpy(ddi_ptr->lname,logical_name);
	DEBUG_1("build_dds(): logical_name = %4s\n",logical_name)

	/* create alias name, "en" appended with sequence number */
	rc=strncmp(odm_info->preobj.prefix,logical_name,
		   strlen(odm_info->preobj.prefix));

	strcpy(ddi_ptr->alias,"en");
	if (rc == 0) {
	    /* logical name created from prefix, copy only extension */
	    strcat(ddi_ptr->alias,logical_name+strlen(odm_info->preobj.prefix));
	}
	else {
	    /* logical name not created from prefix, append entire string */
	    strcat(ddi_ptr->alias,logical_name);
	}
	DEBUG_1("build_dds(): alias name is %s\n",ddi_ptr->alias)

	/* Attributes from other Object Classes/Places */

	/* Get bus memory size */
	DEBUG_1("build_dds(): uniquetype = %s\n",utype)
	sprintf(srchstr,"uniquetype = '%s' AND attribute = '%s'",utype,
							"bus_mem_addr");
	DEBUG_1("build_dds(): criteria = '%s'\n",srchstr)
	if((rc=(int)odm_get_first(PdAt_CLASS,srchstr,&PA_obj)) == 0)
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
		ddi_ptr->bus_mem_size=0;
	else if(mem_width==0x8000)
		ddi_ptr->bus_mem_size=1;
	else
		ddi_ptr->bus_mem_size=2;
	DEBUG_1("build_dds(): bus_mem_size = %ld\n",ddi_ptr->bus_mem_size)

	/* Get tcw bus memory size */
	DEBUG_1("build_dds(): uniquetype = %s\n",utype)
	sprintf(srchstr,"uniquetype = '%s' AND attribute = '%s'",utype,
							"dma_bus_mem");
	DEBUG_1("build_dds(): criteria = '%s'\n",srchstr)

	if((rc=(int)odm_get_first(PdAt_CLASS,srchstr,&PA_obj)) == 0)
		return E_NOPdOBJ;
	else if (rc == -1)
		return E_ODMGET;
	DEBUG_0("build_dds(): odm_get_obj() succeeded\n")
	DEBUG_1("build_dds(): PA_obj.width = %s\n",PA_obj.width)
	if((rc=convert_att(&ddi_ptr->tcw_bus_mem_size,'l',PA_obj.width,'n'))!=0)
		return E_BADATTR;
	DEBUG_0("build_dds(): convert_att() succeeded\n")
	DEBUG_1("build_dds(): tcw_bus_mem_size = %ld\n",ddi_ptr->tcw_bus_mem_size)

	/*
	 *  Get bus type, bus id, and bus number.
	 */
	rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj);
	if (rc) {
	    if (rc == E_PARENT)
		rc = E_NOCuDvPARENT;
	    return (rc);
	}
	if ((bus_attr_list = get_attr_list(bus_obj.name,
					   bus_obj.PdDvLn_Lvalue,
					   &num_bus_attrs,
					   4))                  == NULL)
	    return (E_ODMGET);
	if (rc = getatt(bus_attr_list, "bus_type", &ddi_ptr->bus_type, 'i'))
	    return (rc);
	if (rc = getatt(bus_attr_list, "bus_id", &ddi_ptr->bus_id, 'l'))
	    return (rc);

	ddi_ptr->bus_id |= 0x800C0060;
	DEBUG_1("build_dds(): bus_id=0x%x\n", ddi_ptr->bus_id)

	/*
	 * Get bus number.  Location = 00-00 (??-<bus><slot>)
	 * Subtract '0' (0x30) to get numberical bus value
	 */
	bus_num = bus_obj.location[3] - '0';

	/* adjust attribute bnc_select based on NVRAM */
	/* Check to see if bnc_select is set in NVRAM
	 *     If NVRAM is not set, copy database value to NVRAM
	 *     If NVRAM value is set, overwrite value in DDS
	 *         copy NVRAM value to database
	 */
	DEBUG_0("build_dds(): checking bnc_select in NVRAM\n")
	/* check to see if NVRAM has been initialized */
	if (!check_magic_num()) {
		if (!get_bnc_select(bus_num, ddi_ptr->slot, &bnc_select)) {
			DEBUG_0("build_dds(): bnc_select set in NVRAM\n")
			DEBUG_1("build_dds(): bnc_select is %x\n",bnc_select)
			/* if value is not valid (not set correctly) */
			/*    put odm value into NVRAM */
			if (bnc_select < 0) {
				DEBUG_0("build_dds(): updating bnc_select in NVRAM\n")
				put_wire_type(bus_num, ddi_ptr->slot,
							ddi_ptr->bnc_select);
			/*    else, see if odm needs updating with NVRAM value */
			} else {
				if (ddi_ptr->bnc_select != bnc_select) {
					DEBUG_0("build_dds(): resetting \
						bnc_select in ODM\n")
					ddi_ptr->bnc_select = bnc_select;

					CuAt_obj=getattr(logical_name,"bnc_select",FALSE,&how_many);
					if (!(CuAt_obj))
						return(E_ODMGET);

					if (bnc_select == 0)
						strcpy(CuAt_obj->value,"dix");
					else
						strcpy(CuAt_obj->value,"bnc");
					if (putattr(CuAt_obj)<0)
						return(E_ODMUPDATE);
				}
			}

		}
	}

#ifdef CFGDEBUG
	hexdump(ddi_ptr,(long) sizeof(en3com_dds_t));
#endif

	return 0;



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

int download_microcode(logical_name, odm_info, cfg_k)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
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
 *      order to store it in the database for later use.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int query_vpd(logical_name, odm_info, cfg_k, vpd)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
char              *vpd;
{
	struct	vital_product_data vpd_area;	/* Storage area for VPD */
	ulong	ul;
	ndd_config_t *ndd_config_u;

	DEBUG_0("query_vpd(): BEGIN query_vpd()\n")

	/* Initialize the cfg structure for a call to sysconfig(), which */
	/* will in turn call the ddconfig() section of the appropriate	 */
	/* device driver.						 */

	cfg_k->cmd = CFG_QVPD;             /* Command to read VPD */
	ndd_config_u = (ndd_config_t *) cfg_k->mdiptr;
	ndd_config_u->p_vpd = (caddr_t) &vpd_area;   /* Storage space for VPD */
	ndd_config_u->l_vpd = sizeof(vpd_area);      /* Size of VPD storage area */

	/* Make the call to sysconfig: */
	if(sysconfig(SYS_CFGKMOD, cfg_k, sizeof(struct cfg_kmod))<0) {
#ifdef CFGDEBUG
		DEBUG_0("query_vpd(): Dump of vpd_area\n")
		hexdump(&vpd_area,(long) sizeof(vpd_area));
		switch(errno) {
		case EINVAL:
			DEBUG_1("query(): invalid kmid = %d\n",cfg_k->kmid)
			break;
		case EACCES:
			DEBUG_0("query(): not privileged\n")
			break;
		case EFAULT:
			DEBUG_0("query(): i/o error\n")
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
	put_vpd(vpd,vpd_area.vpd,vpd_area.length);

	return(0);
}

/*
 * NAME:
 *	define_children()
 *
 * FUNCTION:
 *	There are no children.
 *      The comio emulator device will be verified if installed, and if so
 *      its configure routine will be called.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int define_children(logical_name, odm_info, phase)
char *logical_name;
ndd_cfg_odm_t *odm_info;
int  phase;
{

    int rtn;
    struct stat stat_buf;
    char    sstring[256];           /* parameter temp space */

    DEBUG_0("define_children(): BEGIN routine\n")

    /*
     * Begin section for configuring comio emulator for ethernet
     * if it has been installed.  If configuration method for
     * emulator is present (code has been installed), execute it.
     * If file has not been installed, continue with no error.
     */
    sprintf(sstring,"lslpp -l %s > /dev/null 2>&1",CFG_EMULATOR_LPP);
    rtn = system(sstring);
    if (rtn == 0) {     /* directory exists */

	rtn = stat(CFG_COMIO_EMULATOR,&stat_buf);
	if (rtn == 0) {     /* file exists */

	    /* call specified method with parameters */
	    sprintf( sstring, " -l %s ", logical_name);
	    DEBUG_2("cfgent: calling %s %s\n",CFG_COMIO_EMULATOR, sstring)

	    if(odm_run_method(CFG_COMIO_EMULATOR,sstring,NULL,NULL)){
		    fprintf(stderr,"cfgent: can't run %s\n", CFG_COMIO_EMULATOR);
		    return(E_ODMRUNMETHOD);
	    }
	}
	else {
	    /* package installed, but file missing, return error */
	    return(E_ODMRUNMETHOD);
	}
    }
    /* End section for configuring comio emulator for ethernet */
	return(0);
}


/*
 * NAME: device_specific
 *
 * FUNCTION:This is a null function for the time being and returns success
 * always
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function operates as a device dependent subroutine called by the
 *      generic configure method for all devices. For the time being, as
 *      there is not any vpd, this a null function.
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: Returns 0 (success), if called.
 */

int device_specific(logical_name, odm_info, cfg_k)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
{
	DEBUG_1("device_specific: NULL subroutine for %s\n",logical_name)
	return(0);

}
