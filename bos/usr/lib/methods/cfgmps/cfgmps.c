static char sccsid[] = "@(#)61  1.6  src/bos/usr/lib/methods/cfgmps/cfgmps.c, sysxmps, bos411, 9437C411a 9/15/94 21:16:42";
/*
 *   COMPONENT_NAME: SYSXMPS
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
 *   (C) COPYRIGHT International Business Machines Corp. 1994
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
#include <values.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ndd_var.h>
#include <sys/ndd.h>
#include <sys/stat.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/cdli_tokuser.h>
#include <mps_tok/mps_dds.h>
#include <locale.h>
#include <nl_types.h>

#ifdef CFGDEBUG
#include <errno.h>
#endif

#include "pparms.h"
#include "cfgtoolsx.h"
#include "cfgdebug.h"
#include "cfg_ndd.h"

extern	long		*genminor();
static struct attr_list	*alist=NULL;/* PdAt attribute list                 */
int	how_many;		/* Used by getattr routine.            */
int	byte_cnt;		/* Byte count of attributes retrieved  */

#define GETATT(A,B,C)		getatt(alist,C,A,B,&byte_cnt)

/*
 * Define the statistic table for mapping all token-ring statistics.
 * The type name has to be stable_t so we can share the global
 * statistics table stable
 */

typedef struct {
        tok_ndd_stats_t tok_ndd;
        tr_mps_stats_t mps;
} stable_t;

stable_t        stable;         /* statistic table */

typedef struct {
        u_int   bufferlen;
        caddr_t bufferptr;
} io_block_t;

io_block_t io_block;            /* table for socket ioctl */

#define TOKS stable.tok_ndd.tok_gen_stats

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

/* DDS structure to be a static declaration */
static  mps_dds_t       ddi_ptr;        /* DDS structure */

int build_dds(logical_name,odm_info,dds_ptr,dds_length)
char *logical_name;
ndd_cfg_odm_t *odm_info;
uchar **dds_ptr;
long *dds_length;
{
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
	*dds_length=sizeof(mps_dds_t);
	*dds_ptr=(uchar *) &ddi_ptr;

	/* set default value for all attributes to zero */
	bzero(&ddi_ptr,sizeof(mps_dds_t));

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
	ddi_ptr.slot=atoi(&srchstr[strcspn(srchstr,"0123456789")]);
	ddi_ptr.slot--;
	DEBUG_1("build_dds(): slot=%d\n",ddi_ptr.slot)

	/*
	 *  Get attribute list
	 */
	alist=(struct attr_list*)get_attr_list(logical_name,utype,&how_many,16);
	if (alist == NULL)
		return(E_ODMGET);

	/*
	 *  Regular Attributes
	 */

	if(rc=GETATT(&ddi_ptr.intr_level,'i',"bus_intr_lvl"))
		return(rc);
	if(rc=GETATT(&ddi_ptr.intr_priority,'i',"intr_priority"))
		return(rc);
	if(rc=GETATT(&ddi_ptr.xmt_que_size,'i',"xmt_que_size"))
		return(rc);
	if(rc=GETATT(&ddi_ptr.io_base_addr,'a',"bus_io_addr"))
		return(rc);
	if(rc=GETATT(&ddi_ptr.dma_lvl,'i',"dma_lvl"))
		return(rc);
	if(rc=GETATT(&ddi_ptr.dma_base_addr,'i',"dma_bus_mem"))
		return(rc);
	DEBUG_1("ddi_ptr.dma_base_addr = %x\n",ddi_ptr.dma_base_addr)
	if(rc=GETATT(&ddi_ptr.mem_base_addr,'i',"bus_mem_addr"))
		return(rc);
	DEBUG_1("ddi_ptr.mem_base_addr = %x\n",ddi_ptr.mem_base_addr)
	/*
	 * If the use_alt_addr attribute is "no", then the use_alt_addr
	 * DDS value will be set to zero.  If the use_alt_addr attribute
	 * is "yes", the the use_alt_addr DDS value will be set to 1
	 */
	if(rc=GETATT(tmp_str,'s',"use_alt_addr"))
		return(rc);
	if (tmp_str[0] == 'y') {
		ddi_ptr.use_alt_addr = 1;
	}
	else {
		/* no alternate address, set DDS value to 0 */
		ddi_ptr.use_alt_addr = 0;
	}
	DEBUG_1("ddi_ptr.use_alt_addr = %x\n",ddi_ptr.use_alt_addr)
	if (ddi_ptr.use_alt_addr) {
		rc=GETATT(ddi_ptr.alt_addr,'b',"alt_addr");
		if(byte_cnt != 6) {
			if (rc > 0)
				return rc;
			else {
				DEBUG_1("build_dds(): getatt() bytes=%d\n",rc)
				return E_BADATTR;
			}
		}
		DEBUG_6("build_dds():alt_addr=%02x %02x %02x %02x %02x %02x\n",
		  (int) ddi_ptr.alt_addr[0],(int) ddi_ptr.alt_addr[1],
		  (int) ddi_ptr.alt_addr[2],(int) ddi_ptr.alt_addr[3],
		  (int) ddi_ptr.alt_addr[4],(int) ddi_ptr.alt_addr[5])
	}

	if(rc=GETATT(tmp_str,'s',"attn_mac"))
		return(rc);
	ddi_ptr.attn_mac = (tmp_str[0] == 'y')? 1 : 0;
	DEBUG_1("ddi_ptr.attn_mac = %x\n",ddi_ptr.attn_mac)
	if(rc=GETATT(tmp_str,'s',"beacon_mac"))
		return(rc);
	ddi_ptr.beacon_mac = (tmp_str[0] == 'y')? 1 : 0;
	DEBUG_1("ddi_ptr.beacon_mac = %x\n",ddi_ptr.beacon_mac)
	if(rc=GETATT(tmp_str,'s',"priority_tx"))
		return(rc);
	ddi_ptr.priority_tx = (tmp_str[0] == 'y')? 1 : 0;
	DEBUG_1("ddi_ptr.priority_tx = %x\n",ddi_ptr.priority_tx)

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
	if((rc=convert_att(&ddi_ptr.dma_bus_length,'l',PA_obj.width,'n'))!=0)
		return E_BADATTR;
	DEBUG_0("build_dds(): convert_att() succeeded\n")
	DEBUG_1("build_dds(): dma_bus_length = %ld\n",ddi_ptr.dma_bus_length)

	/*
	 *  Get bus memory size
	 */
	DEBUG_1("build_dds(): uniquetype = %s\n",utype)
	sprintf(srchstr,"uniquetype = %s AND attribute = '%s'",
					utype,"bus_mem_addr");
	DEBUG_1("build_dds(): criteria = '%s'\n",srchstr)
	if((rc=(int)odm_get_first(PdAt_CLASS,srchstr,&PA_obj)) == 0)
		return E_NOPdOBJ;
	else if (rc == -1)
		return E_ODMGET;

	DEBUG_1("build_dds(): PA_obj.width = %s\n",PA_obj.width)
	if((rc=convert_att(&ddi_ptr.mem_bus_size,'l',PA_obj.width,'n'))!=0)
		return E_BADATTR;
	DEBUG_0("build_dds(): convert_att() succeeded\n")
	DEBUG_1("build_dds(): mem_bus_size = %ld\n",ddi_ptr.mem_bus_size)

	/*
	 *  Get bus type, bus id, and bus number.
	 */
        rc = Get_Parent_Bus(CuDv_CLASS, cust_obj.parent, &bus_obj); 
        if (rc) {
            if (rc == E_PARENT) 
                rc = E_NOCuDvPARENT;
            return (rc);
        }
        bus_attr_list = (struct attr_list *)get_attr_list(bus_obj.name, 
                                           bus_obj.PdDvLn_Lvalue,
                                           &num_bus_attrs, 
                                           4);
        if (bus_attr_list == NULL)  
            return (E_ODMGET);   
	if (rc = getatt(bus_attr_list, "bus_type", &ddi_ptr.bus_type, 'i'))
            return (rc);
	if (rc = getatt(bus_attr_list, "bus_id", &ddi_ptr.bus_id, 'l'))
            return (rc);  
	ddi_ptr.bus_id |= 0x800C0020;
        bus_num = bus_obj.location[3] - '0';

	/*
	 *  Get ring speed from ODM database.
	 */
	DEBUG_0("build_dds(): getting ring_speed from ODM")
	if (!(CuAt_obj=getattr(logical_name,"ring_speed",FALSE,&how_many)))
		return(E_ODMGET);	
	DEBUG_1(" : value of %s\n",CuAt_obj->value)
	if (strcmp(CuAt_obj->value,"4") == 0) {
		ddi_ptr.ring_speed=0;
	}
	else {
	    if (strcmp(CuAt_obj->value,"16") == 0) {
		    ddi_ptr.ring_speed=1;
	    }
	    else {
		    /* ring speed = autosense */
		    ddi_ptr.ring_speed=2;
	    }
	}

	/*
	 *  Check to see if the ring speed is set in NVRAM.  If the ring
	 *  speed is set in NVRAM then need to set ring speed to that value
	 *  and update ODM to reflect that ring speed.  If NVRAM area is
	 *  configured but the ring speed is NOT_SET then set ring_speed to
	 *  default of 4MB.
	 */

	 DEBUG_0("build_dds(): checking ring_speed in NVRAM\n")
	 if (!check_magic_num()) {
		 if (!get_ring_speed(bus_num, ddi_ptr.slot, &ring_speed)) {
			 /* ring speed value from NVRAM was read ok */
			 DEBUG_0("build_dds(): ring_speed set in NVRAM\n")
			 DEBUG_1("build_dds(): ring_speed is %x\n",ring_speed)
			 /* ring speed value set to -1 if not 4 or 16 */
			 if (ring_speed < 0) {
				 put_ring_speed(bus_num, ddi_ptr.slot,
							 ddi_ptr.ring_speed);
			 } else {
				 if (ddi_ptr.ring_speed != ring_speed) {
					 DEBUG_0("build_dds(): resetting \
						 ring_speed in ODM\n")
					 ddi_ptr.ring_speed = ring_speed;
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
	strcpy(ddi_ptr.dev_name,logical_name);

	/* create alias name, "tr" appended with sequence number */
	rc=strncmp(odm_info->preobj.prefix,logical_name,
		   strlen(odm_info->preobj.prefix));

	strcpy(ddi_ptr.alias,"tr");
	if (rc == 0) {
	    /* logical name created from prefix, copy only extension */
	    strcat(ddi_ptr.alias,logical_name+strlen(odm_info->preobj.prefix));
	}
	else {
	    /* logical name not created from prefix, append entire string */
	    strcat(ddi_ptr.alias,logical_name);
	}
	DEBUG_1("build_dds(): alias name is %s\n",ddi_ptr.alias)

#ifdef CFGDEBUG
	hexdump(&ddi_ptr,(long) sizeof(mps_dds_t));
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
	DEBUG_0("download_microcode(): NULL function, returning 0\n")
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
 * RETURNS:
 *		0 on Success, >0 Error code
 */

int query_vpd(logical_name, odm_info, cfg_k, vpd)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
char              *vpd;

{
	mps_vpd_t vpd_area;             /* Storage area for VPD */
	ulong	ul;
	ndd_config_t *ndd_config_u;
	char    *p_vpd;         /* temporary pointer to copy vpd */
	char    *p_adap_vpd;    /* pointer to adapters vpd */

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

	/* Store the VPD in the database */
	/*     convert to proper format */
	p_vpd = vpd;
	p_adap_vpd = vpd_area.vpd;

	/*    copy in Network Address */
	strncpy(p_vpd,"*NA",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x05;  /* length = 10 characters */
	p_vpd = p_vpd + 1;
	memcpy(p_vpd,p_adap_vpd+0x10,6);
	p_vpd = p_vpd + 6;

	/*    copy in Part Number */
	strncpy(p_vpd,"*PN",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x06;  /* length = 12 characters */
	p_vpd = p_vpd + 1;
	memcpy(p_vpd,p_adap_vpd+0x20,8);
	p_vpd = p_vpd + 8;

	/*    copy in EC Level */
	strncpy(p_vpd,"*EC",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x06;  /* length = 12 characters */
	p_vpd = p_vpd + 1;
	memcpy(p_vpd,p_adap_vpd+0x28,8);
	p_vpd = p_vpd + 8;

	/*    copy in Card Serial Number */
	strncpy(p_vpd,"*SN",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x06;  /* length = 12 characters */
	p_vpd = p_vpd + 1;
	memcpy(p_vpd,p_adap_vpd+0x30,8);
	p_vpd = p_vpd + 8;

	/*    copy in Card FRU */
	strncpy(p_vpd,"*FN",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x06;  /* length = 12 characters */
	p_vpd = p_vpd + 1;
	memcpy(p_vpd,p_adap_vpd+0x40,8);
	p_vpd = p_vpd + 8;

	/*    copy in Manufacturer */
	strncpy(p_vpd,"*MF",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x05;  /* length = 10 characters */
	p_vpd = p_vpd + 1;
	memcpy(p_vpd,p_adap_vpd+0x48,6);
	p_vpd = p_vpd + 6;

	/*    copy in a Brief Description */
	strncpy(p_vpd,"*DS",3);
	p_vpd = p_vpd + 3;
	*p_vpd = 0x13;  /* length = 26 characters */
	p_vpd = p_vpd + 1;

	switch (*(p_adap_vpd+0x50) & 0xf0) {     /* check on Family Version */
	    case 0xf0:
		strncpy(p_vpd,"MCA -   ",8);
		break;
	    case 0xe0:
		strncpy(p_vpd,"ISA -   ",8);
		break;
	    case 0xd0:
		strncpy(p_vpd,"EISA -  ",8);
		break;
	    default:
		strncpy(p_vpd,"UNKNOWN:",8);
		break;
	}
	p_vpd = p_vpd + 8;
	switch (*(p_adap_vpd+0x55) & 0x0f) {      /* check on Protocol supported */
	    case 0x0f:
		strncpy(p_vpd,"Token Ring  32",14);
		break;
	    case 0x0e:
		strncpy(p_vpd,"Ethernet    32",14);
		break;
	    case 0x0d:
		strncpy(p_vpd,"TR and Eth  32",14);
		break;
	    default:
		strncpy(p_vpd,"UNKNOWN   ",14);
		break;
	}
	p_vpd = p_vpd + 14;

	DEBUG_0("query_vpd(): Dump of vpd to put in database\n")

#ifdef CFGDEBUG
	hexdump(vpd,94);
#endif

	return(0);
}

/*
 * NAME:
 *	define_children()
 *
 * FUNCTION:
 *	There are no children.
 *
 * RETURNS:
 *	Always returns 0 - Success.
 */

int define_children(logical_name, odm_info, phase)
char *logical_name;
ndd_cfg_odm_t *odm_info;
int  phase;

{
	DEBUG_0("define_children(): NULL function, returning 0\n")
	return(0);
}

/*
 * NAME: device_specific
 *
 * FUNCTION:
 *	This get the network ring speed.
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
 * RETURNS: Update cfg_ring_speed in ODM.
 */

int device_specific(logical_name, odm_info, cfg_k)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
{
  int fd;
  struct sockaddr_ndd sa;
  struct  CuAt    *CuAt_obj;      /* Customized Attributes data struct */
  struct  PdAt    PA_obj;         /* Customized Attributes data struct */

  /*
   *  Get ring speed from ODM database.
   */
  if (!(CuAt_obj=getattr(logical_name,"ring_speed",FALSE,&how_many))) {
  	return(E_ODMGET);	
  }

  if (strcmp(CuAt_obj->value,"autosense") != 0) {
	return(0);
  }

  if ((fd = socket(AF_NDD, SOCK_DGRAM, 0)) < 0) {
	return(0);
  }

  sa.sndd_family = AF_NDD;
  sa.sndd_len = sizeof(struct sockaddr_ndd);
  sa.sndd_filterlen = 0;
  bcopy(logical_name, sa.sndd_nddname, sizeof(sa.sndd_nddname));

  if (connect(fd, (const struct sockaddr*)&sa, sizeof(sa))) {
        close(fd);
	return(0);
  }

  io_block.bufferlen = sizeof(tok_ndd_stats_t);
  io_block.bufferptr = (caddr_t)&stable;

  if (ioctl(fd, NDD_GET_STATS, (caddr_t)&io_block)) {
        close(fd);
	return(0);
  }
  close(fd);

 /*
  *  Get ring speed from ODM database.
  */
  DEBUG_0("build_dds(): getting ring_speed from ODM")
  if (!(CuAt_obj=getattr(logical_name,"cfg_ring_speed",FALSE,&how_many))) {
                return(E_ODMGET);
  }

  if ((strcmp(CuAt_obj->value,"4")==0)&&(TOKS.ndd_flags & TOK_RING_SPEED_16)) {
 	strcpy(CuAt_obj->value,"16");
  }

  if ((strcmp(CuAt_obj->value,"16")==0)&&(TOKS.ndd_flags & TOK_RING_SPEED_4)) {
 	strcpy(CuAt_obj->value,"4");
  }

  if (putattr(CuAt_obj) < 0) {
	return(E_ODMUPDATE);
  }

  return(0);


}

