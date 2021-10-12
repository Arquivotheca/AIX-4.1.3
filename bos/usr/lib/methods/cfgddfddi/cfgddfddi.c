static char sccsid[] = "@(#)29	1.5  src/bos/usr/lib/methods/cfgddfddi/cfgddfddi.c, diagddfddi, bos411, 9430C411a 7/19/94 10:53:18";
/*
 * COMPONENT_NAME: DIAGDDFDDI
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files,
 *	      download_microcode, query_vpd, define_children
 *              get_dvdr
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990,1994
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
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <fddidiag/diagddfddiuser.h>
#include <fddidiag/diagddfddidds.h>
#include <fddidiag/fddibits.h>
#include "pparms.h"
#include "cfgdebug.h"

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
	DEBUG_1("dds->rcv_que_size	= [%d]\n", p_dds->rcv_que_size);
	DEBUG_1("dds->stat_que_size	= [%d]\n", p_dds->stat_que_size);
	DEBUG_1("dds->rdto		= [%d]\n", p_dds->rdto);
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
	DEBUG_1("dds->alt_mac_smt_addr	= [%c", '0'+p_dds->alt_mac_smt_addr[0]);
	DEBUG_1("%c", '0' + p_dds->alt_mac_smt_addr[1]);
	DEBUG_1("%c", '0' + p_dds->alt_mac_smt_addr[2]);
	DEBUG_1("%c", '0' + p_dds->alt_mac_smt_addr[3]);
	DEBUG_1("%c", '0' + p_dds->alt_mac_smt_addr[4]);
	DEBUG_1("%c", '0' + p_dds->alt_mac_smt_addr[5]);
	DEBUG_1("%c]\n", '0' + p_dds->alt_mac_smt_addr[6]);
	{
		int i;
		DEBUG_0("dds: pmf passwd = ");
		for (i=0; i<FDDI_PASSWD_SZ; i++)
		{
			DEBUG_1("%02x",p_dds->pmf_passwd[i]);
		}
		DEBUG_0("\n");
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

#define GETATT_L(A,B,C)	getatt(A,B,CuAt_CLASS,PdAt_CLASS,lname,utype,C,NULL)

/*
 * NAME: setattr
 *
 * FUNCTION: To set an attribute for a device
 *
 * NOTES: This function uses getattr, and putattr, so default values wont
 *	be placed in CuAt.
 *	The value passed in should be in the same base ( e.g. "0x0" ) as
 *	the default, for putattr to recognize it correctly.
 *
 * RETURNS:  error code.  0 means no error.
 */

int
setattr( lname, aname, avalue )
char	*lname;			/* name of device attribute belongs to */
char	*aname;			/* attribute name */
char	*avalue;		/* attribute value */

{
	struct	CuAt	*cuat;	/* customized attribute object */
	int	how_many;	/* variable needed for getattr call */

	/* routines called */
	struct	CuAt	*getattr();
	int		putattr();

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

/*
 * NAME: generate_minor()
 *
 * FUNCTION:
 *	This calls genminor() to get the minor number for this logical
 *	device name.
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
long 
generate_minor (
	char	*lname,
	long	majorno,
	long	*minorno)
{
	long 	*minorptr;

	DEBUG_0("generate_minor(): BEGIN generate_minor()\n")
	DEBUG_2("generate_minor(): lname=%s, major=%ld\n",lname, majorno)
	minorptr = genminor(lname,majorno,-1,1,1,1);
	if (minorptr == (long *) NULL)
	{
		return E_MINORNO;
	}
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
int 
make_special_files (
	char	*lname,
	dev_t	devno)
{
	long	create_flags;

	DEBUG_0("make_special_files(): BEGIN make_special_file()\n")
	create_flags = ( S_IFMPX | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
	return (mk_sp_file (devno, lname, create_flags));
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

/*
 * Static data area for VPD. The VPD is read in from the adapter thru the
 *	driver the first time query_vpd() is called. From then on the
 *	VPD will be here.
 */
static fddi_vpd_t 	vpd_area = { 0 };

int 
query_vpd (
	struct CuDv	*newobj,	/* NOT USED */
	mid_t		kmid,		/* Kernel module I.D. for Dev Driver */
	dev_t		devno,		/* Concatenated Major & Minor No.s */
	char		*vpdstr)	/* String to store vpd in */
{
	struct cfg_dd	cfg;		/* dd config structure */

	DEBUG_0("query_vpd(): BEGIN query_vpd()\n")

	/* 
	 * Initialize the cfg structure for a call to sysconfig()
	 */ 
	cfg.kmid = kmid;		/* the kmid of the ddriver */
	cfg.devno = devno; 		/* Major#, and Minor# */
	cfg.cmd = CFG_QVPD;		/* Command to read VPD */
	cfg.ddsptr = (char*) &vpd_area;	/* Storage space for VPD */
	cfg.ddslen = sizeof(vpd_area);	/* Size of VPD storage area */

	/* 
	 * Make the call to sysconfig: 
	 */
	if (sysconfig(SYS_CFGDD, &cfg, sizeof(cfg)) < 0) 
	{
		DEBUG_0("query_vpd(): Primary card VPD:\n");
		VPD_DUMP (vpd_area.status, vpd_area.l_vpd, vpd_area.vpd); 
		DEBUG_1("query(): error = 0x%x\n",errno);
		return E_VPD;
	}
	DEBUG_0("query_vpd(): Primary card VPD:\n");
	VPD_DUMP (vpd_area.status, vpd_area.l_vpd, vpd_area.vpd); 
	DEBUG_0("query_vpd(): Xtender card VPD:\n");
	VPD_DUMP (vpd_area.xc_status, vpd_area.l_xcvpd, vpd_area.xcvpd);


	/* Store the VPD in the database */
	bcopy (&vpd_area, vpdstr, sizeof(vpd_area));
	DEBUG_0("Leaving query vpd\n");

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

int 
define_children (
	char	*lname,			/* logical name of parent device */
	int	phase)			/* ipl phase */
{
	struct cfg_dd	cfg;		/* dd config structure */
	struct stat	st;		/* for getting devno of 'lname' */
	fddi_vpd_t 	vpd_area;	/* Storage area for VPD */
	struct CuDv 	cusobj;         /* customized device object storage */
	struct PdDv 	preobj;         /* predefined device object storage */
	struct CuVPD 	vpdobj;         /* customized vpd object */
	char		bufp[128];	/* search string and filename buffer */
	char		*stdp;		/* stdout from child's define method */
	int		rc;		/* return code */
	int 		i;
	char		card_ident;
	char		tb[100];


	DEBUG_2("define_children(): BEGIN: lname=%s, phase=%d\n", lname, phase)

	/*
	 * Get the parent's VPD 
	 */
	sprintf (bufp, "/dev/%s", lname);
	if (stat (bufp, &st))
	{
		DEBUG_1("stat failed on %s\n", bufp);
		return (0);
	}
	if (query_vpd (NULL, 0, st.st_rdev, &vpd_area))
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
#endif
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
	sprintf(bufp, "parent = '%s' and connwhere = 0", lname);
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
				setattr(lname, "title", "38");
			else if (card_ident == 'S')
				setattr(lname, "title", "36");
			return (0);
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
			setattr(lname, "title", "37");
		else if (card_ident == 'S') 
			setattr(lname, "title", "39");
		return (0);
	}
	else 
		if (vpd_area.xc_status == FDDI_VPD_NOT_READ)
		{
			if (card_ident == 'F')
				setattr(lname, "title", "36");
			else if (card_ident == 'S') 
				setattr(lname, "title", "38");
			return (0);
		}
	/*
	 * Child is not defined so define it
	 */
	sprintf (bufp, "-c %s -s %s -t %s -p %s -w 0",
			preobj.class, 
			preobj.subclass,
			preobj.type, 
			lname); 		
	rc = odm_run_method (preobj.Define, bufp, &stdp, NULL);
	if (rc != 0)
	{
		DEBUG_1("run method FAILED: define %s\n", bufp);
		return (E_ODMRUNMETHOD);
	}
	DEBUG_1("define_children: new child is %s\n", stdp);

	sprintf(bufp, "parent = '%s' and connwhere = 0", lname);
	rc = (int) odm_get_first (CuDv_CLASS, bufp, &cusobj);
	if (rc == -1) 
	{
		DEBUG_1("define_children: ODM fail on CuDv %s\n", bufp);
		return (E_ODMGET);
	}

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
		memcpy (vpdobj.vpd, vpd_area.xcvpd, vpd_area.l_xcvpd);
		if (odm_add_obj (CuVPD_CLASS, &vpdobj) == -1) 
		{
			DEBUG_0("adding CuVPD failed\n")
			return (E_ODMADD);
		}
		DEBUG_0("Successfully added new VPD object\n");
	}
	else if (memcmp (vpdobj.vpd, vpd_area.xcvpd, vpd_area.l_xcvpd))
	{
		DEBUG_0("define_children: Updating VPD object\n");

		memcpy (vpdobj.vpd, vpd_area.xcvpd, vpd_area.l_xcvpd);
		if (odm_change_obj (CuVPD_CLASS, &vpdobj) == -1) 
		{
			DEBUG_0("ODM failure updating CuVPD\n");
			return (E_ODMUPDATE);
		}
		DEBUG_0("Successfully updated VPD object\n");
	}
	if (card_ident == 'F')
		setattr(lname, "title", "37");
	else if (card_ident == 'S') 
		setattr(lname, "title", "39");
	return(0);
}


/* These attributes are hardcoded for the build_dds routine because     */
/* there is no device object entry in the database anymore.             */
#define REC_QUE_SIZE    30
#define STA_QUE_SIZE    5
#define RDTO            92
#define PASS_BCON_FRAMES   0
#define PASS_SMT_FRAMES   0
#define PASS_NSA_FRAMES   0
#define USE_ALT_ADDR    0

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
int 
build_dds (
	char		*lname,			/* logical name */
	uchar		**ddsptr,		/* pass by ref ptr to dds */
	int		*ddslen)		/* pass by ref ptr to dds len */
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

	/* 
	 * get memory for the dds
	 */
	p_dds = (fddi_dds_t *) malloc(sizeof(fddi_dds_t));
	if (p_dds == NULL) 
	{
		DEBUG_0 ("dds: Malloc of dds failed\n");
		return E_MALLOC;
	}

	*ddslen = sizeof(fddi_dds_t);
DEBUG_1("The sizeof the dds = %d\n",*ddslen);
	*ddsptr = (uchar *) p_dds;

	/* set default value for all attributes to zero */
	bzero (p_dds, sizeof (fddi_dds_t));

	/* Get customized object */
	sprintf(srchstr,"name = '%s'",lname);
	if ((rc = (int) odm_get_first (CuDv_CLASS, srchstr, &cusobj)) == 0)
	{
		return E_NOCuDv;
	}
	if (rc == -1)
	{
		return E_ODMGET;
	}
	/* copy the uniquetype name from the predefined database */
	strcpy (utype, cusobj.PdDvLn_Lvalue);

	/* 
	 *	REGULAR ATTRIBUTES:
	 *		get values from the customized database first
	 *		and if not there go to the predefined database.
	 */
	rc = GETATT_L (&p_dds->bus_intr_lvl, 	'i', "bus_intr_lvl");
	if (rc)
	{
		DEBUG_0("failed getting bus_intr_lvl");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->bus_intr_lvl, rc);
		return (rc);
	}
	rc = GETATT_L (&p_dds->intr_priority, 	'i', "intr_priority");
	if (rc)
	{
		DEBUG_0("failed getting intr_priority");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->intr_priority, rc);
		return (rc);
	}
	p_dds->rcv_que_size = REC_QUE_SIZE;
	p_dds->stat_que_size = STA_QUE_SIZE;

	GETATT_L (&p_dds->tx_que_sz, 		'i', "tx_que_size");

	if ((p_dds->tx_que_sz < 3) || (p_dds->tx_que_sz > 150))
	{
		DEBUG_0("The tx que must be at least as big as the number of gather locations\n");
		return E_INVATTR;
	}
		
	p_dds->rdto = RDTO;
	if (p_dds->rdto > FDDI_MAX_RDTO)
	{
		DEBUG_1("dds: rdto too big [%d]\n", p_dds->rdto);
		return E_INVATTR;
	}
	rc = GETATT_L (&p_dds->bus_io_addr, 	'a', "bus_io_addr");
	if (rc)
	{
		DEBUG_0("failed getting bus_io_addr");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->bus_io_addr, rc);
		return (rc);
	}
	rc = GETATT_L (&p_dds->bus_mem_addr, 	'a', "bus_mem_addr");
	if (rc)
	{
		DEBUG_0("failed getting bus_mem_addr");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->bus_mem_addr, rc);
		return (rc);
	}
	rc = GETATT_L (&p_dds->dma_lvl, 		'i', "dma_lvl");
	if (rc)
	{
		DEBUG_0("failed getting dma_lvl");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->dma_lvl, rc);
		return (rc);
	}
	rc = GETATT_L (&p_dds->dma_base_addr, 	'a', "dma_base_addr");
	if (rc)
	{
		DEBUG_0("failed getting dma_base_addr");
		DEBUG_2("=[%d], rc =[%d]\n", p_dds->dma_base_addr, rc);
		return (rc);
	}
	p_dds->pass_bcon_frames = PASS_BCON_FRAMES;
	p_dds->pass_smt_frames = PASS_SMT_FRAMES;
	p_dds->pass_nsa_frames = PASS_NSA_FRAMES;

	p_dds->use_alt_mac_smt_addr = USE_ALT_ADDR;
	if (p_dds->use_alt_mac_smt_addr) 
	{
		rc = GETATT_L(p_dds->alt_mac_smt_addr,'b',"alt_mac_smt");
		/* 
		 * do address check: only 6 byte addresses are valid 
		 */
		if (rc != -6) 
		{
			DEBUG_0("failed getting the alt_mac_smt_addr\n");
			DEBUG_1("dds: len of mac smt = %d\n", rc);
			return (E_INVATTR);
		}
	}
	rc = GETATT_L(p_dds->pmf_passwd,'b',"pmf_passwd");
	if ((rc < -8) || (rc > 0))
	{
		DEBUG_1("failed getting the pmf_passwd, rc = %d\n",rc);
		return(E_INVATTR);
	}

	rc = GETATT_L(&p_dds->t_req,'i',"t_req");
	if ((p_dds->t_req > 165002240) && (p_dds->t_req < 160))
	{
		DEBUG_1("failed getting the t_req, rc = %d\n",rc);
		return(E_INVATTR);
	}

	rc = GETATT_L(&p_dds->tvx,'i',"tvx");
	if ((p_dds->tvx > 5202000) && (p_dds->tvx < 80))
	{
		DEBUG_1("failed getting the tvx, rc = %d\n",rc);
		return(E_INVATTR);
	}

	if (p_dds->tvx >= p_dds->t_req)
	{
		DEBUG_2("tvx (%d) is greater than or equal to t_req (%d)\n",
			p_dds->tvx, p_dds->t_req);
		return(E_INVATTR);
	}

	/* 
	 * do address check: only 8 byte addresses are valid 
	 */

	rc = GETATT_L(p_dds->user_data,'s',"user_data");

	if (rc != 0) 
	{
		DEBUG_1("failed getting the user_data, rc = %d\n",rc);
		return (E_INVATTR);
	}

	/* 
	 * Get slot 
	 */
	p_dds->slot = atoi(cusobj.connwhere) - 1;

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
	strcpy (p_dds->lname, lname);

	/*
	 * Get bus type and bus id
	 */
	rc = (int) Get_Parent_Bus (CuDv_CLASS, lname, &busobj);
	if (rc)
	{
		DEBUG_1 ("dds: failed getting parent bus for %s\n", lname);
		return (rc);
	}
	rc = getatt (&p_dds->bus_id, 'i', CuAt_CLASS, PdAt_CLASS, 
		busobj.name, busobj.PdDvLn_Lvalue, "bus_id" , NULL);
	p_dds->bus_id |= 0x800C0020;
	if (rc > 0)
	{
		DEBUG_1("dds: failed bus_id getatt (%s)\n", busobj.name);
		return (rc);
	}
	rc = getatt (&p_dds->bus_type, 'i', CuAt_CLASS, PdAt_CLASS, 
		busobj.name, busobj.PdDvLn_Lvalue, "bus_type" , NULL);
	if (rc > 0)
	{
		DEBUG_1("dds: failed bus_type getatt (%s)\n", busobj.name);
		return (rc);
	}
	/*DDS_DUMP (p_dds); */
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

int 
download_microcode (
	char		*lname)			/* logical device name */
{
	fddi_dwnld_t  	cmd;			/* download command struct */
	fddi_vpd_t  	vpd;			/* VPD for ucode card level */
	int		rc;			/* return codes */
	int		len;			/* len of mcode image */
	int		fd;			/* mcode image and device */
	char		*mcode;			/* ptr to mcode image */
	char		mcname[64];		/* mc file name */
	char		bufp[64];		/* char buffer for search */
	char            mcbase[64];     /* microcode name with level */

DEBUG_0("Entering download microcode\n");

	sprintf(mcbase,"/usr/lib/microcode/8ef4m.02");
	DEBUG_1("mc: base mcode file = %s\n", mcbase)

	/*
	 * get the version number via findmcode().
	 */
	rc = findmcode (mcbase, mcname, VERSIONING, NULL);
	if (rc == 0)
	{
		DEBUG_1("mc: findmcode failed with rc = %d\n", rc);
		fprintf(stderr,
			"cfgfddi: findmcode failed mcbase: %s\n", mcbase);
		fflush(stderr);
		DEBUG_1("mc: findmcode failed mcname: %s\n", mcname);
		return (E_NOUCODE);
	}
	rc = 0;
	DEBUG_1("mc: full mcode file = %s\n", mcname)

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

	/* 
	 * Open fddi device and download
	 */
	sprintf (bufp, "/dev/%s/C", lname);
	if ((fd = open (bufp, O_RDWR | O_NDELAY)) == -1) 
	{
		DEBUG_2("mc: open on %s failed, errno = 0x%x\n", bufp, errno)
		free (mcode);
		return E_OPEN;
	}

	DEBUG_2("mc: successfully opened [%s], fd = %d\n", bufp, fd)
	if ((rc = ioctl(fd, FDDI_DWNLD_MCODE, &cmd)) < 0) 
	{
		DEBUG_1("mc: failed download %d\n", rc);
		switch (cmd.status)
		{
			case CIO_TIMEOUT:
				DEBUG_0 ("status = CIO_TIMEOUT\n");
				break;
			case CIO_INV_CMD:
				DEBUG_0 ("status = CIO_INV_CMD\n");
				break;
			case CIO_BAD_RANGE:
				DEBUG_0 ("status = CIO_BAD_RANGE\n");
				break;
			case CIO_HARD_FAIL:
				DEBUG_0 ("status = CIO_HARD_FAIL\n");
				break;
			case FDDI_NOT_DIAG_MODE:
				DEBUG_0 ("status = FDDI_NOT_DIAG_MODE\n");
				break;
			default:
				DEBUG_1 ("status unknown = [%d]\n", cmd.status);
				break;
		}
		rc = E_UCODE;
	}
	close(fd);
	free (mcode);
DEBUG_1("Leaving download microcode, rc = %d\n",rc);
	return rc;
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
			"/usr/lib/drivers/fddidiag");
		strcpy(dvdr_name, driver_name);
	} else
		strcpy(dvdr_name,"/usr/lib/drivers/fddidiag");

	return(0);
}

