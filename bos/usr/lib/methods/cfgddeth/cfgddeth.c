/*
 *   COMPONENT_NAME: DIAGDDIENT
 *
 *   FUNCTIONS: build_dds
 *              generate_minor
 *              make_special_files
 *              download_microcode
 *              query_vpd
 *              define_children
 *              get_eth_addr
 *            get_dvdr
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   cfgeth.c - Configure Method for Integrated Ethernet Adapters
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

#include "cfgdebug.h"
#include "pparms.h"

#include <ientdiag/i_entddi.h>
#include <ientdiag/i_cioddi.h>

#ifdef CFGDEBUG
#include <errno.h>
#endif

#define FROM_PARENTS		2
#define GETATT_L(A,B,C)         getatt(A,B,CAhdl,PAhdl,lname,utype,C, NULL)
#define GETATT_P(A,B,C)         getatt(A,B,CAhdl,PAhdl,pname,putype,C, NULL)

#define SIO_SLOT 0x0f   /* slot offset for SIO device (reading VPD) */

/* These attributes are hardcoded because there is no device object     */
/* entry in the database anymore.                                       */
#define REC_QUE_SIZE    30
#define STA_QUE_SIZE    5
#define RDTO            92
#define TYPE_FIELD_OFF  12
#define NET_ID_OFFSET   14

/*
 *
 *	entcfg-->sysconfig-->cioconfig
 */

extern	long	*genminor();

/*
 * NAME: build_dds()
 * 
 * FUNCTION: 	Builds the dds for the ethernet device.
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

	ddi_t	*ddi_ptr;
	struct	Class	*CDhdl,		/* Customized Devices OC handle */
			*CAhdl,		/* Customized Attribute OC handle */
			*PAhdl;		/* Predefined Attribute OC handle */
	struct	CuDv	cust_obj;
	struct  CuAt    *cuat;    	/* customized attributes */

	struct	PdAt	PA_obj;
	struct  attr    *attrs;
	char	*parentdesc,pname[NAMESIZE],
		srchstr[256],
		utype[UNIQUESIZE],
		putype[UNIQUESIZE],
		*ptr,
		tmp_str[5];	/* yes,no value for flag fields */
	int	i,rc,
		cc,bt,bi;	/* flags for getting parent attributes */
	long	l;		/* temporary variable */
	ulong	atts_found;
	ulong   *how_many;

	/* declarations for get vpd for network address */
	char    busdev[32];
	int     fd;             /* machine device driver file descriptor */


	DEBUG_0("build_dds(): BEGIN build_dds()\n")
	*ddslen=sizeof(ddi_t);
	if ( (ddi_ptr=(ddi_t *) malloc(sizeof(ddi_t))) == (ddi_t *) NULL) {
		DEBUG_0 ("build_dds(): Malloc of dds failed\n")
		return E_MALLOC;
	}
	*ddsptr=(uchar *) ddi_ptr;

	/* set default value for all attributes to zero */
	bzero(ddi_ptr,sizeof(ddi_t));		/* zero out dds structure */

	/* open Customized Attribute OC */
	if((int)(CAhdl=odm_open_class(CuAt_CLASS))== -1)
		return E_ODMOPEN;

	/* open Predefined Attribute OC */
	if((int)(PAhdl=odm_open_class(PdAt_CLASS))== -1)
		return E_ODMOPEN;

	/* open Customized Devices OC */
	if((int)(CDhdl=odm_open_class(CuDv_CLASS))== -1)
		return E_ODMOPEN;

	/* Get customized object */
	sprintf(srchstr,"name = '%s'",lname);
	if ((rc=(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST)) == 0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;


	DDI_DS.slot = atoi(cust_obj.connwhere) - 1;


	strcpy(utype,cust_obj.PdDvLn_Lvalue);
	strcpy(pname,cust_obj.parent);
	if ((rc=(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST)) == 0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;


        strcpy(pname,cust_obj.parent);


       /*
        * If parent is an SIO, need to get its parent (the bus)
        * in order to configure the bus parms correctly
        */

        if (!strncmp(utype, "adapter/sio", 11))
        {
            strcpy(pname,cust_obj.parent);
            sprintf(srchstr,"name = '%s'",pname);
	    if ((rc=(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST)) == 0)
		return E_NOCuDv;
	    else if (rc == -1)
		return E_ODMGET;

        }
	strcpy(putype,cust_obj.PdDvLn_Lvalue);


	/*
	 *  New and improved methods to get regular attributes.
	 */

	cuat = getattr( lname, NULL, TRUE, &how_many );


	atts_found = 0xE;		/* 13 attributes to be found */
	for ( i=0; i < how_many; i++, cuat++ )
	{

		if ( ( strcmp ( cuat->attribute, "bus_intr_lvl" ) == 0 ))
		{
			convert_att(&DDI_CC.intr_level,'i', &cuat->value,'n');
			atts_found--;
			continue;
		}

		if ( ( strcmp ( cuat->attribute, "intr_priority" )  == 0))
		{
			convert_att(&DDI_CC.intr_priority,'i', &cuat->value,'n');
			atts_found--;
			continue;
		}

		if ( ( strcmp ( cuat->attribute, "xmt_que_size" )  == 0))
		{
			convert_att(&DDI_CC.xmt_que_size,'i', &cuat->value,'n');
			atts_found--;
			continue;
		}

		if ( ( strcmp ( cuat->attribute, "dma_bus_mem" )  == 0))
		{
			convert_att(&DDI_DS.tcw_bus_mem_addr,'i', &cuat->value,'n');
			atts_found--;
			continue;
		}

		if ( ( strcmp ( cuat->attribute, "bus_mem_addr" )  == 0))
		{
			convert_att(&DDI_DS.bus_mem_addr,'i', &cuat->value,'n');
			atts_found--;
			continue;
		}


		if ( ( strcmp ( cuat->attribute, "bus_io_addr" )  == 0))
		{
			convert_att(&DDI_DS.io_port,'i', &cuat->value,'n');
			atts_found--;
			continue;
		}

		if ( ( strcmp ( cuat->attribute, "dma_lvl" )  == 0))
		{
			convert_att(&DDI_DS.dma_arbit_lvl,'i', &cuat->value,'n');
			atts_found--;
			continue;
		}

		if ( ( strcmp ( cuat->attribute, "use_alt_addr" )  == 0))
		{
			convert_att(tmp_str,'s', &cuat->value,'s');
			DDI_DS.use_alt_addr = (tmp_str[0] == 'y') ? 1 : 0;
			atts_found--;
			continue;
		}

		if ( ( strcmp ( cuat->attribute, "alt_addr" )  == 0))
		{
			convert_att(&DDI_DS.alt_addr,'b', &cuat->value,'s');
			continue;
		}

	}

/*	if ( atts_found )
		return ( E_NOATTR ); */

	/* this attribute is not read from database */
	DDI_CC.rec_que_size = REC_QUE_SIZE;
	/* this attribute is not read from database */
	DDI_CC.sta_que_size = STA_QUE_SIZE;
	/* this attribute is not read from database */
	DDI_CC.rdto = RDTO;
	/* this attribute is not read from database */
	DDI_DS.type_field_off = TYPE_FIELD_OFF;
	/* this attribute is not read from database */
	DDI_DS.net_id_offset = NET_ID_OFFSET;


	DEBUG_6("build_dds(): alt_addr=%02x %02x %02x %02x %02x %02x\n",
	  (int) DDI_DS.alt_addr[0],(int) DDI_DS.alt_addr[1],
	  (int) DDI_DS.alt_addr[2],(int) DDI_DS.alt_addr[3],
	  (int) DDI_DS.alt_addr[4],(int) DDI_DS.alt_addr[5])
	

	/* Get logical name */
	DEBUG_1("build_dds(): logical name = %s\n",lname)
	strncpy(DDI_DS.lname,lname,4);
	DEBUG_1("build_dds(): lname = %4s\n",lname)


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

	/* Get bus attributes */
	cc=bt=bi=0;
	DEBUG_1("build_dds(): parent=%s\n",cust_obj.parent)
		

	DEBUG_0("build_dds(): getting parent from CuDv\n")
	strcpy(pname,cust_obj.parent);
	


	while(cc<FROM_PARENTS) {
		DEBUG_1("build_dds(): parent name=%s\n",pname)
		if(!*pname)
			return E_NOCuOBJ;
		sprintf(srchstr,"name = '%s'",pname);
		if((rc=(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST))==0)
			return E_NOCuDvPARENT;
		else if (rc == -1)
			return E_ODMGET;
		strcpy(putype,cust_obj.PdDvLn_Lvalue);
		if(!bt)
			if(!GETATT_P(&DDI_CC.bus_type,'i',"bus_type")) {
				bt=1;
				cc++;
				DEBUG_1("build_dds(): bus_type=%d\n",
					DDI_CC.bus_type)
			}
		if(!bi)
			if(!GETATT_P(&DDI_CC.bus_id,'i',"bus_id")) {
				bi=1;
				cc++;
				DDI_CC.bus_id |= 0x820C0060;
				DEBUG_1("build_dds(): bus_id=0x%x\n",
					DDI_CC.bus_id)
			}
		if(cc<FROM_PARENTS) {
			strcpy(pname,cust_obj.parent);
		}
	}
	DEBUG_1("build_dds(): parent name=%s\n",pname)

	/* Get slot ( location )*/
	DEBUG_0("build_dds(): getting connection info from CuDv\n")
	sprintf(srchstr,"name = '%s'",lname);
	if((rc=(int)odm_get_obj(CDhdl,srchstr,&cust_obj,ODM_FIRST))==0)
		return E_NOCuDv;
	else if (rc == -1)
		return E_ODMGET;
	strcpy(srchstr,cust_obj.connwhere);
	DEBUG_1("build_dds(): connwhere=%s\n",srchstr)

       /*
	* Call device specific subroutine to get
	* network address from the system hardware
	*/

	sprintf(busdev, "/dev/%s",pname) ;

	if (0 > (fd = open(busdev, O_RDWR)) )
	{
	    perror("[busquery]open()");
	    fprintf(stderr, "Unable to open %s\n", busdev) ;
	    return(E_DEVACCESS);
	}
	else
	{
	    /* if uniquetype is for type 1 (deskside) box */
	    DEBUG_1("build_dds(): checking unqiuetype of %s\n",utype)
	    if (!strncmp(utype, "adapter/sio/ient_1", 18)) {
		/* need to read vpd from SIO registers, */
		/* not from the ethernet adapter registers */
		rc = get_eth_addr ( fd, DDI_DS.eth_addr, DDI_DS.slot, SIO_SLOT );
	    }
	    else {
		/* if uniquetype is for type 2 (RSC) box */
		if (!strncmp(utype, "adapter/sio/ient_2", 18)) {
		    /* need to read vpd from pos registers of */
		    /* adapter in the slot indicated */
		    rc = get_eth_addr ( fd, DDI_DS.eth_addr, DDI_DS.slot, DDI_DS.slot );
		}
		else {
		    /* if uniquetype is for type 6 (601) box */
		    if (!strncmp(utype, "adapter/sio/ient_6", 18)) {
			/* need to read the VPD from the ipl control block */
			rc = get_eth601_addr ( fd, DDI_DS.eth_addr, DDI_DS.slot );
		    }
		    else {
			/* error, unknown box */
			rc = E_WRONGDEVICE;
		    }
		}
	    }
	    if (rc != 0) {
		/* error in reading VPD or ethernet address */
		DEBUG_1("build_dds(): return from get vpd = %d\n",rc)
		return(rc);
	    }

	    DEBUG_1("build_dds(): get ethernet address rc=%d\n",rc)
	    DEBUG_6("build_dds(): eth_addr=%02x %02x %02x %02x %02x %02x\n",
	      (int) DDI_DS.eth_addr[0],(int) DDI_DS.eth_addr[1],
	      (int) DDI_DS.eth_addr[2],(int) DDI_DS.eth_addr[3],
	      (int) DDI_DS.eth_addr[4],(int) DDI_DS.eth_addr[5])
	    close(fd);
	}


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
 *      search_eth_addr()
 *
 * FUNCTION:
 *      Searchs for and copies out the ethernet network address from
 *      the vpd string.
 *
 * DESCRIPTION:
 *      Called by the device specific routine get_data_power()
 *
 * INPUT:
 *      pointer to the vpd string
 *      length of vps
 *      pointer to storage to put ethernet address
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */
int search_eth_addr(vpd, l_vpdlen, p_addr)
char    vpd[];
int     l_vpdlen;
char    p_addr[];
{
	int i, j;
	int na_found;

	DEBUG_0("search_eth_addr(): start of routine\n")
	DEBUG_1("search_eth_addr(): vpd length = %d\n",l_vpdlen)

#ifdef CFGDEBUG
	hexdump(vpd,(long) l_vpdlen);
#endif

	/*
	 *  find the network address
	 */
	i = na_found = 0;
	while ( i < ( l_vpdlen - 2 )) {
		if ((vpd[i] == '*' ) && (vpd[i + 1] == 'N' ) &&
					    (vpd[i + 2] == 'A' )) {
			/* put Network Address in DDS  */
			i += 4;
			for (j = 0; j < ent_NADR_LENGTH; j++,i++)
				p_addr[ j ] = vpd[ i ];

			na_found = 1;
			break;
		}
		i++;
	}

	if (na_found == 0) {
	    DEBUG_0("search_eth_addr(): no address found in VPD\n")
	    return (6);
	}
	else {
	    return(0);
	}
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
	char    *mountpoint;
	char    driver_name[255];

	DEBUG_0("get_dvdr(): \n")
	if((mountpoint = (char *)getenv("CDRFS_DIR")) != (char *)NULL){
		sprintf(driver_name, "%s%s", mountpoint,
			"/usr/lib/drivers/ethdiag");
		strcpy(dvdr_name, driver_name);
	} else
		strcpy(dvdr_name,"/usr/lib/drivers/ethdiag");

	return(0);
}
