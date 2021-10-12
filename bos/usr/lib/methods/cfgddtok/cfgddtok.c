/*
 *   COMPONENT_NAME: DIAGDDTOK
 *
 *   FUNCTIONS: GETATT
 *		build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *              get_dvdr
 *
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
#include <sys/tokuser.h>
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/devinfo.h>

#ifdef CFGDEBUG
#include <errno.h>
#endif

#include "pparms.h"
#include "cfgtoolsx.h"
#include "cfgdebug.h"

extern	long		*genminor();
static struct attr_list	*alist=NULL;/* PdAt attribute list                 */
int	how_many;		/* Used by getattr routine.            */
int	byte_cnt;		/* Byte count of attributes retrieved  */

#define GETATT(A,B,C)		getatt(alist,C,A,B,&byte_cnt)

/* These attributes are hardcoded because there is no device object     */
/* entry in the database anymore.                                       */
#define REC_QUE_SIZE    30
#define STA_QUE_SIZE    5
#define RDTO            92
#define BUFFER_SIZE     1024

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

int 
build_dds(lname,ddsptr,ddslen)
char		*lname;
uchar		**ddsptr;
int			*ddslen;
{
	ddi_t	*ddi_ptr;		/* DDS pointer                       */
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
			attn_mac[5],	/* yes,no value for attention frames */
			beacon_mac[5],	/* yes,no value for beacon frames    */
			tmp_str[5];	/* yes,no value for flag fields      */
	int		i,rc;		/* flags for getting parent attrib   */
	int		ring_speed;	/* Ring speed returned from NVRAM    */
        int             bus_num;        /* Bus number                        */
        int             num_bus_attrs;  /* Number of bus attributes          */
	unsigned short	open_options=0x3500;	/* open_options default      */

	DEBUG_0("build_dds(): BEGIN build_dds()\n")

	*ddslen=sizeof(ddi_t);
	if ( (ddi_ptr=(ddi_t *) malloc(sizeof(ddi_t))) == (ddi_t *) NULL) {
		DEBUG_0 ("build_dds(): Malloc of dds failed\n")
		return E_MALLOC;
	}
	*ddsptr=(uchar *) ddi_ptr;

	/* set default value for all attributes to zero */
	bzero(ddi_ptr,sizeof(ddi_t));

	/* get unique type from customized OC */
	sprintf(srchstr,"name = '%s'",lname);
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
	if ((alist=get_attr_list(lname,utype,&how_many,16)) == NULL)
		return(E_ODMGET);

	/*
	 *  Regular Attributes
	 */

	if(rc=GETATT(&ddi_ptr->bus_int_lvl,'i',"bus_intr_lvl"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->intr_priority,'i',"intr_priority"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->xmt_que_size,'i',"xmt_que_size"))
		return(rc);


	/* this attribute is not read from database */
	ddi_ptr->rec_que_size = REC_QUE_SIZE;
	/* this attribute is not read from database */
	ddi_ptr->sta_que_size = STA_QUE_SIZE;
	/* this attribute is not read from database */
	ddi_ptr->rdto = RDTO;

	if(rc=GETATT(&ddi_ptr->bus_io_addr,'a',"bus_io_addr"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->dma_lvl,'i',"dma_lvl"))
		return(rc);
	if(rc=GETATT(&ddi_ptr->dma_base_addr,'i',"dma_bus_mem"))
		return(rc);
	DEBUG_1("ddi_ptr->dma_base_addr = %x\n",ddi_ptr->dma_base_addr)

	/* this attribute is not read from database */
	ddi_ptr->buf_size = BUFFER_SIZE;

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

	if(rc=GETATT(attn_mac,'s',"attn_mac"))
		return(rc);
	if(rc=GETATT(beacon_mac,'s',"beacon_mac"))
		return(rc);
	if (*attn_mac == 'y')
		open_options |= (0x01<<11);
	if (*beacon_mac == 'y')
		open_options |= (0x01<<7);
	ddi_ptr->open_options = open_options;

	/*
	 *  Attributes from other Object Classes/Places
	 */

	/*
	 *  Get tcw bus memory size
	 */
	DEBUG_1("build_dds(): uniquetype = %s\n",utype)
	sprintf(srchstr,"uniquetype = %s AND attribute = '%s'",
					utype,"dma_bus_mem");
	DEBUG_1("build_dds(): criteria = '%s'\n",srchstr)
	if((rc=(int)odm_get_first(PdAt_CLASS,srchstr,&PA_obj)) == 0)
		return E_NOPdOBJ;
	else if (rc == -1)
		return E_ODMGET;

	DEBUG_1("build_dds(): PA_obj.width = %s\n",PA_obj.width)
	if((rc=convert_att(&ddi_ptr->dma_bus_length,'l',PA_obj.width,'n'))!=0)
		return E_BADATTR;
	DEBUG_0("build_dds(): convert_att() succeeded\n")
	DEBUG_1("build_dds(): dma_bus_length = %ld\n",ddi_ptr->dma_bus_length)

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
	if (!(CuAt_obj=getattr(lname,"ring_speed",FALSE,&how_many)))
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
	strcpy(ddi_ptr->dev_name,lname);

#ifdef CFGDEBUG
	hexdump(ddi_ptr,(long) sizeof(ddi_t));
#endif

	return 0;
}

/*
 * NAME:
 *	generate_minor()
 * 
 * FUNCTION:
 *	This call genminor() to get the minor number for this logical
 *	device name.
 *
 * RETURNS:
 *		0 on Success, >0 Error code
 */
long 
generate_minor(lname,majorno,minorno)
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
 *	This creates 1 special file with the same name as the logical
 *	device name.
 *
 * RETURNS:
 *		0 on Success, >0 Error code
 */
int 
make_special_files(lname,devno)
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
 *	This downloads microcode to a token ring adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int 
download_microcode(lname)
char	*lname;
{
	struct	CuDv	cust_obj;
	tok_download_t	*p_dnld_cmd;	/* download command */
	int		mcload_len,	/* length of microcode loader file */
			mcload_fd,	/* microcode loader file descriptor */
			mcode_len,	/* length of microcode file */
			mcode_fd,	/* microcode file descriptor */
			pd0,		/* 1st port opened file descripter */
			rc;
	char	*mcload,	/* pointer to microcode loader image */
			*mcode,		/* pointer to microcode image */
			buf[20],
			mcode_filename[64],
			mcload_filename[64],
			mcode_odm_name[64],
			mcload_odm_name[64];

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
	/*	Open device						*/
	/****************************************************************/
	sprintf(buf, "/dev/%s/D",lname);
	if( (pd0=open(buf,O_RDWR | O_NDELAY)) == -1) {
		DEBUG_1("download_microcode(): open on %s failed\n", buf)
		return E_OPEN;
	}

	/****************************************************************/
	/*	Get and download loader file microcode			*/
	/****************************************************************/
	if( (mcload_fd=open(mcload_filename,O_RDONLY)) == -1) {
		DEBUG_0("download_microcode(): open on loader file failed\n")
		close(pd0);
		return E_NOUCODE;       /* open microcode loader file failed */
	}
	if( (mcode_fd=open(mcode_filename,O_RDONLY)) == -1) {
		DEBUG_0("download_microcode(): open on code file failed\n")
		close(pd0);
		close(mcload_fd);	/* open microcode file failed */
		return E_NOUCODE;       /* open microcode file failed */
	}
	if( (mcload_len=lseek(mcload_fd,0L,2)) == -1) {
		DEBUG_0("download_microcode(): lseek EOF loader failed\n")
		close(pd0);
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if( (mcode_len=lseek(mcode_fd,0L,2)) == -1) {
		DEBUG_0("download_microcode(): lseek EOF microcode failed\n")
		close(pd0);
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if(lseek(mcload_fd,0L,0)== -1) {
		DEBUG_0("download_microcode(): lseek BOF loader failed\n")
		close(pd0);
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if(lseek(mcode_fd,0L,0)== -1) {
		DEBUG_0("download_microcode(): lseek BOF microcode failed\n")
		close(pd0);
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if( (mcload=malloc(mcload_len)) == NULL) {
		DEBUG_0("download_microcode(): malloc for loader failed\n")
		close(pd0);
		close(mcload_fd);
		close(mcode_fd);
		return E_MALLOC;
	}
	if( (mcode=malloc(mcode_len)) == NULL) {
		DEBUG_0("download_microcode(): malloc for microcode failed\n")
		close(pd0);
		close(mcload_fd);
		close(mcode_fd);
		return E_MALLOC;
	}

	DEBUG_0("download_microcode(): Memory allocation complete.\n" )
	if(read(mcload_fd, mcload, mcload_len)== -1) {
		DEBUG_0("download_microcode(): read loader file failed\n")
		close(pd0);
		close(mcload_fd);
		close(mcode_fd);
		return E_NOUCODE;
	}
	if(read(mcode_fd, mcode, mcode_len)== -1) {
		DEBUG_0("download_microcode(): read microcode file failed\n")
		close(pd0);
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

	p_dnld_cmd->p_mcload=mcload;		/* addr of microcode loader */
	p_dnld_cmd->l_mcload=mcload_len;	/* microcode loader length */
	p_dnld_cmd->p_mcode=mcode;		/* addr of microcode */
	p_dnld_cmd->l_mcode=mcode_len;		/* microcode length */
	if(ioctl(pd0, TOK_DOWNLOAD_UCODE, p_dnld_cmd) == -1) {
		close(pd0);
		return E_UCODE;
	}
	DEBUG_0("download_microcode(): successfully and correctly loaded\n")

	close(pd0);
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
int 
query_vpd(newobj,kmid,devno,vpdstr)
struct	CuDv	*newobj;
mid_t	kmid;				/* Kernel module I.D. for Dev Driver */
dev_t	devno;				/* Concatenated Major & Minor No.s */
char	*vpdstr;			/* String to store VPD in */
{
	struct	cfg_dd	cfg;		/* dd config structure */
	tok_vpd_t vpd_area;		/* Storage area for VPD */
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
	case TOK_VPD_NOT_READ:
		DEBUG_0("query_vpd(): status=VPD NOT READ\n")
		break;
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
	put_vpd(vpdstr,vpd_area.vpd,vpd_area.l_vpd);
	return(0);
}

/*
 * NAME:
 *      define_children()
 *
 * FUNCTION:
 *      There are no children.
 *
 * RETURNS:
 *      Always returns 0 - Success.
 */
int
define_children(lname,cusdev,phase,children)
char    *lname,*children;
long    cusdev;
int     phase;
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
 *	Always returns 0 - Success.
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
			"/usr/lib/drivers/tokdiag");
		strcpy(dvdr_name, driver_name);
	} else
		strcpy(dvdr_name,"/usr/lib/drivers/tokdiag");
	return(0);
}
