static char sccsid[] = "@(#)49  1.16  src/bos/usr/lib/methods/cfgtok/cfgtok.c, sysxtok, bos411, 9428A410j 4/1/94 16:11:23";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: GETATT
 *		build_dds
 *		define_children
 *		download_microcode
 *		query_vpd
 *              device_specific
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <cf.h>		/* Error codes */

#include <sys/cfgodm.h>
#include <sys/comio.h>
#include <sys/cfgdb.h>
#include <sys/ndd.h>
#include <tok/tr_mon_dds.h>
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <string.h>

#ifdef CFGDEBUG
#include <errno.h>
#endif

#include "pparms.h"
#include "cfgtoolsx.h"
#include "cfgdebug.h"
#include "cfg_ndd.h"
#include "ciepkg.h"     /* header file for cfgcie method, see define_children() */

extern	long		*genminor();
static struct attr_list	*alist=NULL;/* PdAt attribute list                 */
int	how_many;		/* Used by getattr routine.            */
int	byte_cnt;		/* Byte count of attributes retrieved  */

#define GETATT(A,B,C)		getatt(alist,C,A,B,&byte_cnt)

/*
 * NAME:  build_dds()
 *
 * FUNCTION:
 *	This builds the dds for the token ring device.
 *
 * EXECUTION ENVIRONMENT:
 *	This is a common build dds routine for the configuration and
 *	change methods.  The only difference between the two is that
 *	the configuration method passes a null attribute list pointer
 *	because all variable attributes are retieved from the data base.
 *	The change method may have changed attributes passed in.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int build_dds(logical_name,odm_info,dds_ptr,dds_length)
char *logical_name;
ndd_cfg_odm_t *odm_info;
uchar **dds_ptr;
long *dds_length;
{
	tr_mon_dds_t   *ddi_ptr;               /* DDS pointer                       */
	char	*parent=NULL,
			*location=NULL;
	struct	CuDv	cust_obj;
        struct  CuDv    bus_obj;
	struct	CuAt	*CuAt_obj;	/* Customized Attributes data struct */
	struct	PdAt	PA_obj;		/* Customized Attributes data struct */
        struct attr_list *bus_attr_list = NULL; /* Bus attributes list */
	char	*parentdesc,
			srchstr[256],
			utype[UNIQUESIZE],
			*ptr,
			tmp_str[5];	/* yes,no value for flag fields      */
	int		i,rc;		/* flags for getting parent attrib   */
	int		ring_speed;	/* Ring speed returned from NVRAM    */
        int             bus_num;        /* Bus number                        */
        int             num_bus_attrs;  /* Number of bus attributes          */

	DEBUG_0("build_dds(): BEGIN build_dds()\n")

	*dds_length=sizeof(tr_mon_dds_t);
	if ( (ddi_ptr=(tr_mon_dds_t *) malloc(sizeof(tr_mon_dds_t))) == (tr_mon_dds_t *) NULL) {
		DEBUG_0 ("build_dds(): Malloc of dds failed\n")
		return E_MALLOC;
	}
	*dds_ptr=(uchar *) ddi_ptr;

	/* set default value for all attributes to zero */
	bzero(ddi_ptr,sizeof(tr_mon_dds_t));

	/* get unique type from customized OC */
	sprintf(srchstr,"name = '%s'",logical_name);
	if((rc=(int)odm_get_first(CuDv_CLASS,srchstr,&cust_obj)) == 0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;

	strcpy(utype,cust_obj.PdDvLn_Lvalue);

	/*
	 *  Get slot
	 */
	strcpy(srchstr,cust_obj.connwhere);
	DEBUG_1("build_dds(): connwhere=%s\n",srchstr)
	ddi_ptr->slot=atoi(&srchstr[strcspn(srchstr,"0123456789")]);
	ddi_ptr->slot--;
	DEBUG_1("build_dds(): slot=%d\n",ddi_ptr->slot)

	/*
	 *  Get attribute list
	 */
	if ((alist=get_attr_list(logical_name,utype,&how_many,16)) == NULL)
		return(E_ODMGET);

	/*
	 *  Regular Attributes
	 */

	if(rc=GETATT(&ddi_ptr->intr_level,'i',"bus_intr_lvl"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->intr_priority,'i',"intr_priority"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->xmt_que_size,'i',"xmt_que_size"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->io_port,'a',"bus_io_addr"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->dma_arbit_lvl,'i',"dma_lvl"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->tcw_bus_mem_addr,'i',"dma_bus_mem"))
		return(rc);
	DEBUG_1("ddi_ptr->tcw_bus_mem_addr = %x\n",ddi_ptr->tcw_bus_mem_addr)
	if(rc=GETATT(tmp_str,'s',"use_alt_addr"))
		return(rc);
	ddi_ptr->use_alt_addr = (tmp_str[0] == 'y')? 1 : 0;
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

	/*
	 *  Translated Attrributes
	 */

	if(rc=GETATT(tmp_str,'s',"attn_mac"))
		return(rc);
	ddi_ptr->attn_mac = (tmp_str[0] == 'y')? 1 : 0;
	DEBUG_1("ddi_ptr->attn_mac = %x\n",ddi_ptr->attn_mac)
	if(rc=GETATT(tmp_str,'s',"beacon_mac"))
		return(rc);
	ddi_ptr->beacon_mac = (tmp_str[0] == 'y')? 1 : 0;
	DEBUG_1("ddi_ptr->beacon_mac = %x\n",ddi_ptr->beacon_mac)

	/*
	 *  Attributes from other Object Classes/Places
	 */


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
        bus_num = bus_obj.location[3] - '0';

	/*
	 *  Get ring speed from ODM database.
	 */
	DEBUG_0("build_dds(): getting ring_speed from ODM\n")
	if (!(CuAt_obj=getattr(logical_name,"ring_speed",FALSE,&how_many)))
		return(E_ODMGET);	
	ddi_ptr->ring_speed = atoi(CuAt_obj->value);
	if (ddi_ptr->ring_speed==4)
		ddi_ptr->ring_speed=0;
	else
		ddi_ptr->ring_speed=1;

	/*
	 *  Check to see if the ring speed is set in NVRAM.  If the ring
	 *  speed is set in NVRAM then need to set ring speed to that value
	 *  and update ODM to reflect that ring speed.  If NVRAM area is
	 *  configured but the ring speed is NOT_SET then set ring_speed to
	 *  default of 4MB.
	 */
	DEBUG_0("build_dds(): checking ring_speed in NVRAM\n")
	if (!check_magic_num()) {
		if (!get_ring_speed(bus_num, ddi_ptr->slot, &ring_speed)) {
			DEBUG_0("build_dds(): ring_speed set in NVRAM\n")
			DEBUG_1("build_dds(): ring_speed is %x\n",ring_speed)
			if (ring_speed < 0) {
				put_ring_speed(bus_num, ddi_ptr->slot,
							ddi_ptr->ring_speed);
			} else {
				if (ddi_ptr->ring_speed != ring_speed) {
					DEBUG_0("build_dds(): resetting \
						ring_speed in ODM\n")
					ddi_ptr->ring_speed = ring_speed;
					if (ring_speed == 0)
						strcpy(CuAt_obj->value,"4");
					else
						strcpy(CuAt_obj->value,"16");
					if (putattr(CuAt_obj)<0)
						return(E_ODMUPDATE);
				}
			}
			
		}
	}
		
	/*
	 *  Put the logical name in the DDI for use by error logging
	 */
	strcpy(ddi_ptr->lname,logical_name);

	/* create alias name, "tr" appended with sequence number */
	rc=strncmp(odm_info->preobj.prefix,logical_name,
		   strlen(odm_info->preobj.prefix));

	strcpy(ddi_ptr->alias,"tr");
	if (rc == 0) {
	    /* logical name created from prefix, copy only extension */
	    strcat(ddi_ptr->alias,logical_name+strlen(odm_info->preobj.prefix));
	}
	else {
	    /* logical name not created from prefix, append entire string */
	    strcat(ddi_ptr->alias,logical_name);
	}
	DEBUG_1("build_dds(): alias name is %s\n",ddi_ptr->alias)

#ifdef CFGDEBUG
	hexdump(ddi_ptr,(long) sizeof(tr_mon_dds_t));
#endif

	return 0;
}

/*
 * NAME:
 *	download_microcode()
 *
 * FUNCTION:
 *	This downloads microcode to a token ring adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int download_microcode(logical_name, odm_info, cfg_k)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
{
	struct	CuDv	cust_obj;
	tok_download_t	*p_dnld_cmd;	/* download command */
	int		mcload_len,	/* length of microcode loader file */
			mcload_fd,	/* microcode loader file descriptor */
			mcode_len,	/* length of microcode file */
			mcode_fd,	/* microcode file descriptor */
			rc;
	char	*mcload,	/* pointer to microcode loader image */
			*mcode,		/* pointer to microcode image */
			mcode_filename[64],
			mcload_filename[64],
			mcode_odm_name[64],
			mcload_odm_name[64];
	ndd_config_t *ndd_config_u;

	/****************************************************************/
	/*	Get microcode filename and loader filename		*/
	/****************************************************************/

	if(rc=GETATT(mcode_odm_name,'s',"ucode"))
		return(rc);
	DEBUG_1("download_ucode(): odm ucode name = %s\n",mcode_odm_name)

	if(rc=GETATT(mcload_odm_name,'s',"lucode"))
		return(rc);
	DEBUG_1("download_ucode(): odm loader name = %s\n",mcload_odm_name)

	/* VERSIONING == look for microcode with the highest version # */
	if ((rc = findmcode(mcode_odm_name,mcode_filename,VERSIONING,NULL))
		== 0)		/* return code of 0 means an error occurred */
		return E_NOUCODE;
	DEBUG_1("download_ucode(): ucode filename = %s\n",mcode_filename)

	if ((rc = findmcode(mcload_odm_name,mcload_filename,VERSIONING,NULL)) 
		== 0)		/* return code of 0 means an error occurred */
		return E_NOUCODE;
	DEBUG_1("download_ucode(): loader filename = %s\n",mcload_filename)

	/****************************************************************/
	/*	Get and download loader file microcode			*/
	/****************************************************************/
	if( (mcload_fd=open(mcload_filename,O_RDONLY)) == -1) {
		DEBUG_0("download_microcode(): open on loader file failed\n")
		return E_NOUCODE;       /* open microcode loader file failed */
	}
	if( (mcode_fd=open(mcode_filename,O_RDONLY)) == -1) {
		DEBUG_0("download_microcode(): open on code file failed\n")
		close(mcload_fd);	/* open microcode file failed */
		return E_NOUCODE;       /* open microcode file failed */
	}
	if( (mcload_len=lseek(mcload_fd,0L,2)) == -1) {
		DEBUG_0("download_microcode(): lseek EOF loader failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if( (mcode_len=lseek(mcode_fd,0L,2)) == -1) {
		DEBUG_0("download_microcode(): lseek EOF microcode failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if(lseek(mcload_fd,0L,0)== -1) {
		DEBUG_0("download_microcode(): lseek BOF loader failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if(lseek(mcode_fd,0L,0)== -1) {
		DEBUG_0("download_microcode(): lseek BOF microcode failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if( (mcload=malloc(mcload_len)) == NULL) {
		DEBUG_0("download_microcode(): malloc for loader failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_MALLOC;
	}
	if( (mcode=malloc(mcode_len)) == NULL) {
		DEBUG_0("download_microcode(): malloc for microcode failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_MALLOC;
	}

	DEBUG_0("download_microcode(): Memory allocation complete.\n" )
	if(read(mcload_fd, mcload, mcload_len)== -1) {
		DEBUG_0("download_microcode(): read loader file failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if(read(mcode_fd, mcode, mcode_len)== -1) {
		DEBUG_0("download_microcode(): read microcode file failed\n")
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	DEBUG_0("download_microcode(): MCode loader and MCode loaded\n")
	close(mcload_fd);
	close(mcode_fd);

	if( (p_dnld_cmd=malloc(sizeof(tok_download_t))) == NULL) {
		DEBUG_0("download_microcode(): malloc for command failed\n")
		return E_MALLOC;
	}

	p_dnld_cmd->p_mcload=mcload;            /* addr of microcode loader */
	p_dnld_cmd->l_mcload=mcload_len;	/* microcode loader length */
	p_dnld_cmd->p_mcode=mcode;		/* addr of microcode */
	p_dnld_cmd->l_mcode=mcode_len;		/* microcode length */

	cfg_k->cmd = CFG_UCODE;        /* Command to download ucode */
	ndd_config_u = (ndd_config_t *) cfg_k->mdiptr;
	ndd_config_u->ucode = (caddr_t) p_dnld_cmd;

#ifdef CFGDEBUG
	DEBUG_0("download_microcode(): cfg_k struct\n")
	hexdump(cfg_k,(long) sizeof(struct cfg_kmod));
#endif
#ifdef CFGDEBUG
	DEBUG_0("download_microcode(): ndd_config_t struct\n")
	hexdump(ndd_config_u,(long) sizeof(ndd_config_t));
#endif
#ifdef CFGDEBUG
	DEBUG_0("download_microcode(): ucode struct\n")
	hexdump(ndd_config_u->ucode,(long) 32);
#endif

	/* Make the call to sysconfig: */
	if(sysconfig(SYS_CFGKMOD, cfg_k, sizeof(struct cfg_kmod))<0) {
		return E_UCODE;
	}

	DEBUG_0("download_microcode(): successfully and correctly loaded\n")

	return 0;
}

/*
 * NAME:
 *	query_vpd()
 *
 * FUNCTION:
 *	Retrieve Vital Product Data (VPD) from the adapter card in
 *	order to store it in the database from later use.
 *
 * RETURNS:
 *		0 on Success, >0 Error code
 */

int query_vpd(logical_name, odm_info, cfg_k, vpd)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
char              *vpd;

{
	tok_vpd_t vpd_area;		/* Storage area for VPD */
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
	case TOK_VPD_INVALID:
		DEBUG_0("query_vpd(): status=VPD INVALID\n")
		break;
	case TOK_VPD_VALID:
		DEBUG_0("query_vpd(): status=VPD VALID\n")
		break;
	default:
		DEBUG_0("query_vpd(): status=?\n")
		break;
	}
	DEBUG_0("query_vpd(): Dump of vpd\n")
	hexdump(vpd_area.vpd,(long) vpd_area.l_vpd);
#endif

	/* Store the VPD in the database */
	put_vpd(vpd,vpd_area.vpd,vpd_area.l_vpd);
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
 * RETURNS:
 *      Returns 0 - Success, or ODM_RUN_METHOD error.
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
     * Begin section for configuring comio emulator for token ring
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
	    DEBUG_2("cfgtok: calling %s %s\n",CFG_COMIO_EMULATOR, sstring)

	    if(odm_run_method(CFG_COMIO_EMULATOR,sstring,NULL,NULL)){
		    fprintf(stderr,"cfgtok: can't run %s\n", CFG_COMIO_EMULATOR);
		    return(E_ODMRUNMETHOD);
	    }
	}
	else {
	    /* package installed, but file missing, return error */
	    return(E_ODMRUNMETHOD);
	}
    }
    /* End section for configuring comio emulator for token ring */

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
