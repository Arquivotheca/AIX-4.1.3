static char sccsid[] = "@(#)34        1.11  src/bos/usr/lib/methods/cfgfddi/cfgfddi.c, sysxfddi, bos411, 9428A410j 6/21/94 16:16:10";
/*
 * COMPONENT_NAME: SYSXFDDI
 *
 * FUNCTIONS: build_dds, device_specific
 *	      download_microcode, query_vpd, define_children
 *            dds_dump, vpd_dump, hexdump, setattr
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <sys/types.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stddef.h>
#include <cf.h>		
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mode.h>
#include <sys/sysconfig.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/device.h>
#include <sys/buf.h>
#include <sys/watchdog.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/ndd.h>
#include "cfg_ndd.h"
#include <sys/cdli_fddiuser.h>
#include <fddi/fddibits.h>
#include <fddi/fdditypes.h>
#include "pparms.h"
#include "cfgdebug.h"
#include "ciepkg.h"     /* header file for cfgcie method, see define_children() */

#ifdef CFGDEBUG
/*
 * Local debug routines:
 */
static int
dds_dump (fddi_dds_t	*p_dds)
{
	DEBUG_0("**************************************************\n");
	DEBUG_0("       DDS values	\n");
	DEBUG_1("dds->bus_type		= [%d]\n", p_dds->bus_type);
	DEBUG_1("dds->bus_id 		= [0x%x]\n", p_dds->bus_id);
	DEBUG_1("dds->bus_intr_lvl	= [%d]\n", p_dds->bus_intr_lvl);
	DEBUG_1("dds->intr_priority	= [%d]\n", p_dds->intr_priority);
	DEBUG_1("dds->slot		= [%d]\n", p_dds->slot);
	DEBUG_1("dds->bus_io_addr	= [0x%x]\n", p_dds->bus_io_addr);
	DEBUG_1("dds->bus_mem_addr	= [0x%x]\n", p_dds->bus_mem_addr);
	DEBUG_1("dds->dma_lvl		= [%d]\n", p_dds->dma_lvl);
	DEBUG_1("dds->dma_base_addr	= [0x%x]\n", p_dds->dma_base_addr);
	DEBUG_1("dds->dma_length		= [0x%x]\n", p_dds->dma_length);
	DEBUG_1("dds->lname		= [%c", p_dds->lname[0]);
	DEBUG_1("%c", p_dds->lname[1]);
	DEBUG_1("%c", p_dds->lname[2]);
	DEBUG_1("%c", p_dds->lname[3]);
	DEBUG_1("%c]\n", p_dds->lname[4]);
	DEBUG_1("dds->alt_addr  = [%c", '0'+p_dds->alt_addr[0]);
	DEBUG_1("%c", '0' + p_dds->alt_addr[1]);
	DEBUG_1("%c", '0' + p_dds->alt_addr[2]);
	DEBUG_1("%c", '0' + p_dds->alt_addr[3]);
	DEBUG_1("%c", '0' + p_dds->alt_addr[4]);
	DEBUG_1("%c", '0' + p_dds->alt_addr[5]);
	DEBUG_1("%c]\n", '0' + p_dds->alt_addr[6]);
	{
		int i;
		DEBUG_0("dds: pmf passwd = ");
		for (i=0; i<FDDI_PASSWD_SZ; i++)
		{
			DEBUG_1("%02x",p_dds->pmf_passwd[i]);
		}
		DEBUG_0("User data =\n");
		for (i=0; i<FDDI_USR_DATA_LEN; i++)
		{
			DEBUG_1("%02x",p_dds->user_data[i]);
			if (!((i+1)%10)) 
				DEBUG_0("\n");
		}
		DEBUG_0("\n");
	}
	DEBUG_1("tx_que_sz 		= [%d]\n",p_dds->tx_que_sz);
	DEBUG_1("max t_req 		= [%d]\n",p_dds->t_req);
	DEBUG_1("tvx lower bound	= [%d]\n",p_dds->tvx);
	DEBUG_0("**************************************************\n");

	return (0);
}
static int
vpd_dump (
	ulong 	status, 
	ulong 	len, 
	uchar	*p_vpd)
{
	switch ((int) status) 
	{
		case FDDI_VPD_NOT_READ:
			DEBUG_0("\tstatus=VPD NOT READ\n")
			break;
		case FDDI_VPD_INVALID:
			DEBUG_0("\tstatus=VPD INVALID\n")
			break;
		case FDDI_VPD_VALID:
			DEBUG_0("\tstatus=VPD VALID\n")
			break;
		default:
			DEBUG_0("\tstatus=?\n")
			break;
	}
	DEBUG_2("\tDump of VPD: vpd=0x%x, len=0x%x\n", p_vpd, len)
	hexdump (p_vpd, len);
	return 0;
}
static int
hexdump (data, len)
char *data;
long len;
{
	int	i,j,k;
	char	str[18];

	fprintf(stderr,"hexdump(): length=%ld\n",len);
	i=j=k=0;
	while(i<len)
	{
		j=(int) data[i++];
		if(j>=32 && j<=126)
			str[k++]=(char) j;
		else
			str[k++]='.';
		fprintf(stderr,"%02x ",j);
		if(!(i%8))
		{
			fprintf(stderr,"  ");
			str[k++]=' ';
		}
		if(!(i%16))
		{
			str[k]='\0';
			fprintf(stderr,"     %s\n",str);
			k=0;
		}
	}
	while(i%16)
	{
		if(!(i%8))
			fprintf(stderr,"  ");
		fprintf(stderr,"   ");
		i++;
	}
	str[k]='\0';
	fprintf(stderr,"       %s\n",str);
	fflush(stderr);

	return ;
}
#endif
/*
 * Local debug routines
 */
#ifdef CFGDEBUG
#	define VPD_DUMP(stat, len, vpd)		vpd_dump (stat, len, vpd)
#	define DDS_DUMP(p_dds)			dds_dump (p_dds)
#else
#	define VPD_DUMP(stat, len, vpd)
#	define DDS_DUMP(p_dds)
#endif

/*
 * NAME: setattr
 *
 * FUNCTION: To set an attribute for a device
 *
 * NOTES: This function uses getattr, and putattr.
 *      This function gets the attribute and writes a new value back
 *      to the database.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
setattr( lname, aname, avalue )
char    *lname;                 /* name of device attribute belongs to */
char    *aname;                 /* attribute name */
char    *avalue;                /* attribute value */

{
	struct  CuAt    *cuat;  /* customized attribute object */
	int     how_many;       /* variable needed for getattr call */

	/* routines called */
	struct  CuAt    *getattr();
	int             putattr();

	DEBUG_3("setattr(%s,%s,%s)\n",lname,aname,avalue)

	/* get current attribute setting */
	cuat = getattr(lname, aname, 0, &how_many);
	if (cuat == (struct CuAt *)NULL ) {
		DEBUG_0("ERROR: getattr() failed\n")
		return(E_NOATTR);
	}

	DEBUG_1("Value received by getattr: %s\n", cuat->value )

	/* Only update CuAt object if attr value really changed */
	if (strcmp(cuat->value,avalue)) {
		/* copy new attribute value into cuat structure */
		strcpy( cuat->value, avalue );

		if (putattr( cuat )) {
			DEBUG_0("Error: putattr() failed\n")
			return(E_ODMUPDATE);
		}
	}

	return(0);
}

static  struct    attr_list *alist=NULL;   /* PdAt attribute list      */
int     how_many;               /* Used by getattr routine.            */
int     byte_cnt;               /* Byte count of attributes retrieved  */

#define GETATT(A,B,C)           getatt(alist,C,A,B,&byte_cnt)

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

/*
 * Static data area for VPD. The VPD is read in from the adapter thru the
 *	driver the first time query_vpd() is called. From then on the
 *	VPD will be here.
 *      Since the first byte of the array is the status field, it will be
 *      initialized to VPD_NOT_READ.  This is the flag that will determine
 *      if the static area vpd is valid or not.
 */
static fddi_vpd_t       vpd_area = { FDDI_VPD_NOT_READ };

int query_vpd(logical_name, odm_info, cfg_k, vpd)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;
char              *vpd;
{
	ndd_config_t    *ndd_config_u;  /* pointer to ndd cfg structure */

	DEBUG_0("query_vpd(): BEGIN query_vpd()\n")

	if (vpd_area.status == FDDI_VPD_NOT_READ ) {

		/*
		 * Initialize the cfg structure for a call to sysconfig()
		 */
		cfg_k->cmd = CFG_QVPD;             /* Command to read VPD */
		ndd_config_u = (ndd_config_t *) cfg_k->mdiptr;
		ndd_config_u->p_vpd = (caddr_t) &vpd_area;   /* Storage space for VPD */
		ndd_config_u->l_vpd = sizeof(vpd_area);      /* Size of VPD storage area */

		/*
		 * Make the call to sysconfig:
		 */
		if (sysconfig(SYS_CFGKMOD, cfg_k, sizeof(struct cfg_kmod))<0) {
			DEBUG_0("query_vpd(): Primary card VPD:\n");
			VPD_DUMP (vpd_area.status, vpd_area.l_vpd, vpd_area.vpd);
			DEBUG_1("query(): error = 0x%x\n",errno);
			return E_VPD;
		}
		DEBUG_0("query_vpd(): Primary card VPD:\n");
		VPD_DUMP (vpd_area.status, vpd_area.l_vpd, vpd_area.vpd);
		DEBUG_0("query_vpd(): Xtender card VPD:\n");
		VPD_DUMP (vpd_area.xc_status, vpd_area.l_xcvpd, vpd_area.xcvpd);
	}

	/* check return status on VPD */
	if (vpd_area.status == FDDI_VPD_VALID) {
		/* Store the VPD in the database */
		put_vpd (vpd, vpd_area.vpd, vpd_area.l_vpd);
		DEBUG_0("Leaving query vpd\n");
	}
	else {
		VPD_DUMP (vpd_area.status, vpd_area.l_vpd, vpd_area.vpd);
		return E_VPD;
	}

	return(0);
}
/*
 * NAME:
 *
 *	define_children()
 *
 * FUNCTION:
 *
 *	Define the child of a FDDI if extender card is present. The 
 *	only child FDDI can have is one extender card. This child is
 *	detected by the CFG_QVPD function. If the extender card's VPD
 *	is VALID (actually anything except NOT READ) then the child
 *	will be defined.
 *
 * RETURNS: 0 on success, or E_* Error code if failure.
 */

#define FDDIX_UTYPE	"adapter/fddi/fddix"

int define_children(logical_name, odm_info, phase)
char *logical_name;
ndd_cfg_odm_t *odm_info;
int  phase;

{
	fddi_vpd_t      temp_vpd_area;  /* Temporary storage area for VPD */
	struct CuDv 	cusobj;         /* customized device object storage */
	struct PdDv 	preobj;         /* predefined device object storage */
	struct CuVPD 	vpdobj;         /* customized vpd object */
	char		bufp[128];	/* search string and filename buffer */
	char		child_name[20];
	char		*stdp;		/* stdout from child's define method */
	int		rc;		/* return code */
	int 		i;
	char		card_ident;
	char		tb[100];
	struct cfg_kmod cfg_k;          /* sysconfig struct */
	ndd_config_t    ndd_config;     /* ndd struct */
	int             rtn_value;      /* return code from this routine */


	DEBUG_2("define_children(): BEGIN: logical_name=%s, phase=%d\n", logical_name, phase)

	/*
	 * First try to configure the comio emulator if installed
	 * Save the return code to use only if child device configuring
	 * is successful (return error for config of daughter card first)
	 * (This had to be done this way because routine has multiple
	 * successful returns scattered throughout the code.)
	 */
	/* define_emulator to return a zero (sucess) or error */
	rtn_value = define_emulator (logical_name);

	/*
	 * Get the parent's VPD 
	 */
	/************************************************
	  Call loadext() to query kernel module ID
	 ************************************************/
	DEBUG_1("define_children: Querying the driver: %s\n",
						  odm_info->preobj.DvDr)
	cfg_k.kmid = loadext(odm_info->preobj.DvDr,FALSE,TRUE);
	if (cfg_k.kmid == NULL) {
		/* error querying device driver */
		DEBUG_0("define_children: error querying driver\n")
		err_exit(E_LOADEXT);
	}
	DEBUG_1("define_children: kmid of driver = %x\n",cfg_k.kmid)

	/* get instance number for this device */
	ndd_config.seq_number = lsinst(logical_name);
	if (ndd_config.seq_number == -1) {
	    DEBUG_0("define_children: error getting instance number\n")
	    err_exit(E_INSTNUM);
	}
	DEBUG_1("define_children: instance number = %d\n",ndd_config.seq_number);

	cfg_k.mdiptr = (caddr_t) &ndd_config;
	cfg_k.mdilen = sizeof(ndd_config_t);

	/* make sure VPD has been read and copied into static area */
	if (query_vpd (logical_name, odm_info, &cfg_k, &temp_vpd_area.vpd))
	{
		DEBUG_0("define_child(): failed query_vpd ");
		return (E_VPD);
	}

	for (i=0; i<FDDI_VPD_LENGTH; i++)
	{
		if (vpd_area.vpd[i] == 'D' &&
			vpd_area.vpd[i+1] == 'S')
		{
			card_ident = vpd_area.vpd[i+4];
#ifdef CFGDEBUG
sprintf(tb,">>>>>>>>>>>>>>>>>>>> card ident - %c",card_ident);
DEBUG_0(tb);
#endif CFGDEBUG
			break;
		}
	}
			
	/* 
	 * Get child's predefined 
	 */
	sprintf(bufp, "uniquetype = '%s'", FDDIX_UTYPE);
	rc = (int) odm_get_first (PdDv_CLASS, bufp, &preobj);
	if (rc == -1) 
	{
		DEBUG_1("define_children: ODM fail on PdDv %s\n", bufp);
		return (E_ODMGET);
	}
	if (rc == 0)
	{
		DEBUG_1("define_children: PdDv Not Found: %s\n", bufp);
		return (E_NOPdDv);
	}
	/*
	 *	Get child's Customized
	 */
	sprintf(bufp, "parent = '%s' and connwhere = 0", logical_name);
	rc = (int) odm_get_first (CuDv_CLASS, bufp, &cusobj);
	if (rc == -1) 
	{
		DEBUG_1("define_children: ODM fail on CuDv %s\n", bufp);
		return (E_ODMGET);
	}
	if (rc != 0)
	{
		/*
	 	* Check the VPD to see if child is present in the machine
	 	*/
		if (vpd_area.xc_status == FDDI_VPD_NOT_READ)
		{
			if (cusobj.chgstatus == SAME)
			{
				cusobj.chgstatus = MISSING;
				rc = odm_change_obj (CuDv_CLASS, &cusobj);
				if (rc == -1)
				{
					return (E_ODMUPDATE);
				}
			}
			/*
		 	* Child is not in machine so we are done 
		 	* 	(diagnostics will take care of marking this 
		 	*	device as missing if it is in the defined state)
		 	*/
			DEBUG_0("define_children: No Child in machine\n");
			if (card_ident == 'F')
				setattr(logical_name, "title", "36");
			else if (card_ident == 'S')
				setattr(logical_name, "title", "38");
			return (rtn_value);
		}
		/*
		 * Child is already defined so just update the chgstatus if
		 *	marked MISSING and then finish.
		 */
		if (cusobj.chgstatus == MISSING)
		{
			cusobj.chgstatus = SAME;
			rc = odm_change_obj (CuDv_CLASS, &cusobj);
			if (rc == -1)
			{
				return (E_ODMUPDATE);
			}
		}
		if (card_ident == 'F')
			setattr(logical_name, "title", "37");
		else if (card_ident == 'S') 
			setattr(logical_name, "title", "39");
		return (rtn_value);
	}
	else 
		if (vpd_area.xc_status == FDDI_VPD_NOT_READ)
		{
			if (card_ident == 'F')
				setattr(logical_name, "title", "36");
			else if (card_ident == 'S') 
				setattr(logical_name, "title", "38");
			return (rtn_value);
		}
	/*
	 * Child is not defined so define it
	 */

	sprintf(child_name, "%s%s",preobj.type,&logical_name[4]);
	sprintf (bufp, "-c %s -s %s -t %s -p %s -l %s -w 0",
			preobj.class, 
			preobj.subclass,
			preobj.type, 
			logical_name,
			child_name);
	rc = odm_run_method (preobj.Define, bufp, &stdp, NULL);
	if (rc != 0)
	{
		DEBUG_1("run method FAILED: define %s\n", bufp);
		return (E_ODMRUNMETHOD);
	}

	strcpy(stdp,child_name);
	DEBUG_1("define_children: new child is %s\n", stdp);

	sprintf(bufp, "parent = '%s' and connwhere = 0", logical_name);
	rc = (int) odm_get_first (CuDv_CLASS, bufp, &cusobj);
	if (rc == -1) 
	{
		DEBUG_1("define_children: ODM fail on CuDv %s\n", bufp);
		return (E_ODMGET);
	}

	/*
	 * overriding standard values, making fddi0 the parent device
	 * but indicating slot as if on bus (one less than parents)
	 */
	cusobj.location[4] -= 1;
	cusobj.location[5] = 0x00;

	rc = odm_change_obj (CuDv_CLASS, &cusobj);
	if (rc == -1)
	{
		return (E_ODMUPDATE);
	}

	/*
	 * Get extender card's VPD 
	 */
	sprintf(bufp, "name = '%s' and vpd_type = '%d'", stdp, HW_VPD);
	rc = (int) odm_get_first (CuVPD_CLASS, bufp, &vpdobj);
	if (rc == -1) 
	{
		DEBUG_1("ODM failure getting CuVPD object: %s\n", bufp);
		return (E_ODMGET);
	}
	if (rc == 0) 
	{
		DEBUG_0("define_children: Adding VPD object\n");

		strcpy (vpdobj.name, stdp);
		vpdobj.vpd_type = HW_VPD;
		put_vpd (vpdobj.vpd, vpd_area.xcvpd, vpd_area.l_xcvpd);
		if (odm_add_obj (CuVPD_CLASS, &vpdobj) == -1) 
		{
			DEBUG_0("adding CuVPD failed\n")
			return (E_ODMADD);
		}
		DEBUG_0("Successfully added new VPD object\n");
	}
	else
	{
		DEBUG_0("define_children: Updating VPD object\n");

		put_vpd (vpdobj.vpd, vpd_area.xcvpd, vpd_area.l_xcvpd);
		if (odm_change_obj (CuVPD_CLASS, &vpdobj) == -1) 
		{
			DEBUG_0("ODM failure updating CuVPD\n");
			return (E_ODMUPDATE);
		}
		DEBUG_0("Successfully updated VPD object\n");
	}
	if (card_ident == 'F')
		setattr(logical_name, "title", "37");
	else if (card_ident == 'S') 
		setattr(logical_name, "title", "39");

	return (rtn_value);
}

/*
 * NAME:
 *	build_dds()
 *
 * FUNCTION:
 *	This builds the dds for the fddi device.
 *
 * EXECUTION ENVIRONMENT:
 *	This is a common build dds routine for the configuration and
 *	change methods.  The only difference between the two is that
 *	the configuration methods passes a null attribute list pointer
 *	because all variable attributes are retrieved from the database.
 *	The change method may have changed attributes passed in.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int build_dds(logical_name,odm_info,dds_ptr,dds_length)
char *logical_name;             /* logical name of device */
ndd_cfg_odm_t *odm_info;        /* ptr to odm information already retrieved */
uchar **dds_ptr;                /* pass by ref ptr to dds */
long *dds_length;               /* pass by ref ptr to dds length */
{
	int		i;			/* counter */
	int		rc;			/* return code */
	fddi_dds_t	*p_dds;			/* the dds pointer */
	struct CuDv	cusobj;			/* customized struct */
	struct CuDv	busobj;			/* customized struct */
	struct PdAt	preobj;			/* predefined struct */
	char		yes_no[5];		/* yes/no buffer */
	char		srchstr[256];		/* generic search buffer */
	char		utype[UNIQUESIZE];	/* connecting CuDv and PdDv */
	struct  attr_list   *bus_attr_list = NULL;  /* Bus Attributes list */
	int     num_bus_attrs;  /* number of bus attributes     */

	/* 
	 * get memory for the dds
	 */
	p_dds = (fddi_dds_t *) malloc(sizeof(fddi_dds_t));
	if (p_dds == NULL) 
	{
		DEBUG_0 ("dds: Malloc of dds failed\n");
		return E_MALLOC;
	}

	*dds_length = sizeof(fddi_dds_t);
DEBUG_1("The sizeof the dds = %d\n",*dds_length);
	*dds_ptr = (uchar *) p_dds;

	/* set default value for all attributes to zero */
	bzero (p_dds, sizeof (fddi_dds_t));

	/* Get customized object
	sprintf(srchstr,"name = '%s'",logical_name);
	if ((rc = (int) odm_get_first (CuDv_CLASS, srchstr, &cusobj)) == 0)
	{
		return E_NOCuDv;
	}
	if (rc == -1)
	{
		return E_ODMGET;
	}
	/* copy the uniquetype name from the customized database */
	strcpy (utype, odm_info->cusobj.PdDvLn_Lvalue);

	/* 
	 *	REGULAR ATTRIBUTES:
	 *              get the effective attribute values
	 *              (CuAt:PdAt)
	 */

	if ((alist=get_attr_list(logical_name,utype,&how_many,16)) == NULL)
		return(E_ODMGET);

	rc = GETATT (&p_dds->bus_intr_lvl,    'i', "bus_intr_lvl");
	if (rc)
	{
		DEBUG_0("failed getting bus_intr_lvl");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->bus_intr_lvl, rc);
		return (rc);
	}
	rc = GETATT (&p_dds->intr_priority,   'i', "intr_priority");
	if (rc)
	{
		DEBUG_0("failed getting intr_priority");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->intr_priority, rc);
		return (rc);
	}

	rc = GETATT (&p_dds->tx_que_sz,       'i', "tx_que_size");
	if (rc)
	{
		DEBUG_0("failed getting tx_que_size ");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->tx_que_sz, rc);
		return (rc);
	}
		
	rc = GETATT (&p_dds->bus_io_addr,     'a', "bus_io_addr");
	if (rc)
	{
		DEBUG_0("failed getting bus_io_addr ");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->bus_io_addr, rc);
		return (rc);
	}
	rc = GETATT (&p_dds->bus_mem_addr,    'a', "bus_mem_addr");
	if (rc)
	{
		DEBUG_0("failed getting bus_mem_addr ");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->bus_mem_addr, rc);
		return (rc);
	}
	rc = GETATT (&p_dds->dma_lvl,                 'i', "dma_lvl");
	if (rc)
	{
		DEBUG_0("failed getting dma_lvl");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->dma_lvl, rc);
		return (rc);
	}
	rc = GETATT (&p_dds->dma_base_addr,   'a', "dma_base_addr");
	if (rc)
	{
		DEBUG_0("failed getting dma_base_addr");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->dma_base_addr, rc);
		return (rc);
	}

	yes_no[0] = NULL;
	GETATT (&yes_no[0], 's', "use_alt_addr");
	p_dds->use_alt_addr = (yes_no[0] == 'y') ? 1 : 0;
	if (p_dds->use_alt_addr)
	{
		rc = GETATT(p_dds->alt_addr,'b',"alt_addr");
		/* 
		 * do address check: only 6 byte addresses are valid 
		 */
		if (byte_cnt != 6)
		{
			DEBUG_0("failed getting the alt_addr\n");
			DEBUG_1("dds: len of alt_addr = %d\n", byte_cnt);
			return (E_INVATTR);
		}
	}
	rc = GETATT(p_dds->pmf_passwd,'b',"pmf_passwd");
	if ((rc < 0) || (rc > 8))
	{
		DEBUG_1("failed getting the pmf_passwd, rc = %d\n",rc);
		return(E_INVATTR);
	}

	rc = GETATT(&p_dds->t_req,'i',"t_req");
	if (rc)
	{
		DEBUG_0("failed getting t_req ");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->t_req, rc);
		return (rc);
	}

	rc = GETATT(&p_dds->tvx,'i',"tvx");
	if (rc)
	{
		DEBUG_0("failed getting tvx ");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->tvx, rc);
		return (rc);
	}

	/* 
	 * do address check: only 8 byte addresses are valid 
	 */

	rc = GETATT(p_dds->user_data,'s',"user_data");
	DEBUG_0("User data =\n");
	for (i=0; i<FDDI_USR_DATA_LEN; i++)
	{
		DEBUG_1("%02x",p_dds->user_data[i]);
		if (!((i+1)%10))
			DEBUG_0("\n");
	}
	DEBUG_0("\n");

	if (rc != 0) 
	{
		DEBUG_1("failed getting the user_data, rc = %d\n",rc);
		return (E_INVATTR);
	}

	/* 
	 * Get slot 
	 */
	p_dds->slot = atoi(odm_info->cusobj.connwhere) - 1;
	DEBUG_1("slot indicated as - %d\n",p_dds->slot);

	/* 
	 * Get bus memory size and dma length
	 */
        sprintf (srchstr,
		"uniquetype = '%s' AND attribute = 'dma_base_addr'", utype);
        if ((rc = odm_get_first (PdAt_CLASS, srchstr, &preobj)) == 0)
	{
                return E_NOPdOBJ;
	}
        else if (rc == -1)
	{
                return E_ODMGET;
	}
        if ((rc = convert_att(&p_dds->dma_length,'l',preobj.width,'n')) != 0)
	{
                return E_BADATTR;
	}
	if (p_dds->dma_length < FDDI_MIN_DMA_SPACE)
	{
		DEBUG_1("dds: dma_length too small 0x%x\n", p_dds->dma_length);
		return E_INVATTR;
	}
	/* 
	 * Get logical name 
	 */
	strcpy (p_dds->lname, logical_name);

	/* create alias name, "fi" appended with sequence number */
	rc=strncmp(odm_info->preobj.prefix,logical_name,
		   strlen(odm_info->preobj.prefix));

	strcpy(p_dds->alias,"fi");
	if (rc == 0) {
	    /* logical name created from prefix, copy only extension */
	    strcat(p_dds->alias,logical_name+strlen(odm_info->preobj.prefix));
	}
	else {
	    /* logical name not created from prefix, append entire string */
	    strcat(p_dds->alias,logical_name);
	}
	DEBUG_1("build_dds(): alias name is %s\n",p_dds->alias)

	/*
	 * Get bus type and bus id
	 */
	rc = (int) Get_Parent_Bus (CuDv_CLASS, logical_name, &busobj);
	if (rc)
	{
		if (rc == E_PARENT)
		    rc = E_NOCuDvPARENT;
		DEBUG_1 ("dds: failed getting parent bus for %s\n", logical_name);
		return (rc);
	}
	DEBUG_2 ("dds: reading bus %s with uniquetype = %s\n",
		       busobj.name, busobj.PdDvLn_Lvalue );

	if ((bus_attr_list = get_attr_list(busobj.name,
					   busobj.PdDvLn_Lvalue,
					   &num_bus_attrs,
					   4))                  == NULL)
	    return (E_ODMGET);
	if (rc = getatt(bus_attr_list, "bus_type", &p_dds->bus_type, 'i')) {
	    DEBUG_1("dds: failed bus_type getatt (%s)\n", busobj.name);
	    return (rc);
	}
	if (rc = getatt(bus_attr_list, "bus_id", &p_dds->bus_id, 'l')) {
	    DEBUG_1("dds: failed bus_id getatt (%s)\n", busobj.name);
	    return (rc);
	}
	p_dds->bus_id |= 0x800C0020;
	DEBUG_1("build_dds(): bus_id=0x%x\n", p_dds->bus_id)

	DDS_DUMP (p_dds);
	DEBUG_0("dds: END\n")

	return 0;
}

/*
 * NAME:
 *	download_microcode()
 *
 * FUNCTION:
 *	This downloads microcode to a fddi adapter.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

/*
 * Location of the EC level minus the seven bytes at the beginning.
 *	(see put_vpd() for more detail)
 */

int download_microcode(logical_name, odm_info, cfg_k)
char              *logical_name;
ndd_cfg_odm_t     *odm_info;
struct  cfg_kmod  *cfg_k;

{
	fddi_dnld_t  	cmd;			/* download command struct */
	fddi_vpd_t  	vpd;			/* VPD for ucode card level */
	int		rc;			/* return codes */
	int		len;			/* len of mcode image */
	int		fd;			/* mcode image and device */
	char		*mcode;			/* ptr to mcode image */
	char		mcname[64];		/* mc file name */
	char		bufp[64];		/* char buffer for search */
	ndd_config_t    *ndd_config_u;

	{
		/*
		 * Get micro code file name
		 *	1. get base ucode name from cardid in predefined.
		 *	2. get the EC level from VPD
		 *	3. get path and VERSION from findmcode() call
		 */
		struct CuDv	cusobj;		/* to get base mcode name */
		struct stat     st;             /* to get devno of 'logical_name' */
		char	utype[UNIQUESIZE];	/* connect from CuDv to PdDv */
		char		mcbase[16];	/* microcode name with level */
		int		i;
		struct  CuAt    *cuat;  /* customized attribute object */
		int     how_many;       /* variable needed for getattr call */
		/* subroutines called */
		struct  CuAt    *getattr();
		
		/*
		 * 1. get base ucode name
		 */
DEBUG_0("Entering download microcode\n");
		/*
		sprintf(bufp,"name = '%s'",logical_name);
		if((rc = (int) odm_get_first (CuDv_CLASS, bufp, &cusobj)) == 0)
		{
			DEBUG_0("failed getting the microcode name\n");
			return E_NOCuDv;
		}
		if (rc == -1)
		{
			DEBUG_0("failed getting the odm call to get the mcode name\n");
			return E_ODMGET;
		}
		*/
		strcpy (utype, odm_info->cusobj.PdDvLn_Lvalue);

		/* get current attribute setting */
		cuat = getattr(logical_name, "mcode", 0, &how_many);
		if (cuat == (struct CuAt *)NULL ) {
			DEBUG_1("failed getting the mcode attr (rc = %d)\n",rc);
			return(E_NOATTR);
		}

		DEBUG_1("Value received by getattr: %s\n", cuat->value )
		strcpy( mcbase, cuat->value );

		DEBUG_1("mc: base mcode file = %s\n", mcbase)

		/*
		 * 2. Get the EC LEVEL from the VPD
		 */
		/*
		 *  This section of code has been removed.
		 *  Instead of getting EC LEVEL from the VPD on
		 *  the adapter (VPD may not always be what is
		 *  to be used), the EC LEVEL has been hardcoded
		 *  to .02. .  See following code.
		 *
		 */

		sprintf(bufp, "%s.%c%c", mcbase, 
				'0', 
				'2');

		/*
		 * 3. Finally, get the version number via findmcode().
		 */
		rc = findmcode (bufp, mcname, VERSIONING, NULL);
		if (rc == 0)
		{
			DEBUG_1("mc: findmcode failed with rc = %d\n", rc);
			fprintf(stderr,
				"cfgfddi: findmcode failed bufp: %s\n", bufp);
			fflush(stderr);
			DEBUG_1("mc: findmcode failed mcname: %s\n", mcname);
			return (E_NOUCODE);
		}
		rc = 0;
		DEBUG_1("mc: full mcode file = %s\n", mcname)
	}
	/*	
	 * Open microcode file and get mcode image 		
	 */
	if ((fd = open (mcname, O_RDONLY)) == -1) 
	{
		DEBUG_0("mc: open on code file failed\n")
		return E_NOUCODE;
	}

	if ((len = lseek(fd,0L,2)) == -1) 
	{
		DEBUG_0("mc: lseek EOF microcode failed\n")
		close(fd);
		return E_NOUCODE;
	}

	if (lseek(fd,0L,0) == -1) 
	{
		DEBUG_0("mc: lseek BOF microcode failed\n")
		close(fd);
		return E_NOUCODE;
	}

	if ((mcode = malloc (len)) == NULL) 
	{
		DEBUG_0("mc: malloc for microcode failed\n")
		close(fd);
		return E_MALLOC;
	}

	DEBUG_0("mc: Memory allocation complete.\n" )
	if (read(fd, mcode, len)== -1) 
	{
		DEBUG_0("mc: read microcode file failed\n")
		close(fd);
		free (mcode);
		return E_NOUCODE;
	}
	close(fd);

	/* set values in microcode command structure */
	cmd.p_mcode = mcode;
	cmd.l_mcode = len;

	cfg_k->cmd = CFG_UCODE;        /* Command to download ucode */
	ndd_config_u = (ndd_config_t *) cfg_k->mdiptr;
	ndd_config_u->ucode = (caddr_t) &cmd;

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
	hexdump(ndd_config_u->ucode,(long) sizeof(fddi_dnld_t));
#endif

	/* Make the call to sysconfig: */
	if(sysconfig(SYS_CFGKMOD, cfg_k, sizeof(struct cfg_kmod))<0) {
		DEBUG_0("mc: failed download \n");
		free (mcode);
		return E_UCODE;
	}

	DEBUG_0("download_microcode(): successfully and correctly loaded\n")

	free (mcode);
DEBUG_0("Leaving download microcode\n");
	return 0;
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

/*
 * NAME: define_emulator
 *
 * FUNCTION:
 *      The comio emulator device will be verified if installed, and if so
 *      its configure routine will be called.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function operates as a device dependent subroutine called by the
 *      define_children subroutine in this module.
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: Returns 0 (success)
 */

int define_emulator(logical_name)
char              *logical_name;
{

    /* used for configuring comio emulator if installed */
    int rtn;
    struct stat stat_buf;
    char    sstring[256];           /* parameter temp space */


    /*
     * Begin section for configuring comio emulator for fddi
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
	    DEBUG_2("cfgfddi: calling %s %s\n",CFG_COMIO_EMULATOR, sstring)

	    if(odm_run_method(CFG_COMIO_EMULATOR,sstring,NULL,NULL)){
		    fprintf(stderr,"cfgfddi: can't run %s\n", CFG_COMIO_EMULATOR);
		    return(E_ODMRUNMETHOD);
	    }
	}
	else {
	    /* package installed, but file missing, return error */
	    return(E_ODMRUNMETHOD);
	}
    }
    /* End section for configuring comio emulator for fddi */

    return(0);

}
