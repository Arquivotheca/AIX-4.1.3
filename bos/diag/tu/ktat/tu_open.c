static char sccsid[] = "@(#)97	1.2.1.2  src/bos/diag/tu/ktat/tu_open.c, tu_ktat, bos41J, 9523A_all 5/31/95 11:09:17";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu_open
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/dma.h>
#include <odmi.h>
#include <cf.h>
#include <sys/diagex.h>
#include <sys/intr.h>
#include <sys/access.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/errno.h>
#include "kent_defs.h"
#include "kent_tu_type.h"
/*  #include "extern.h" */

diagex_dds_t		dds;
diag_struc_t		*diagex_hdl;
struct cfg_load		cfg_ld;
struct int_data         idata;
char busstring[12];

#define MDD_OPEN_FAILED 0x01000000

struct attr {
	char *attribute;
	char *value;
};

/***********************************************************
 * NAME: tu_open
 *
 * FUNCTION: Initialisation for Klickitat TUs.
 *
 * INPUT:
 *
 * OUTPUT: Error Code
 *
 ***********************************************************/

int  open_cnt = 0;
int mdd_fd;

int tu_open(char *devid, TU_TYPE *tucb_ptr)
{
  int	rc, r_data;
  FILE	*out;
  char	*path;
  int junk;
  char ipath[512];
  int base_addr;
#define intr_msk 0x085f0000

  char filename[30] = "/tmp/ktatmsg";

  /**************************
   * Initialize Message File
   **************************/
  filename[12] = devid[3];
#ifdef DEBUG
  tucb_ptr ->msg_file = freopen(filename, "w",stdout);
  DEBUG1("Opening file %s\n",filename);
#endif

/*  DEBUG1("Calling odm_initialize\n");  */

  if (rc = (int)odm_initialize())
  { 
    DEBUG1("ODM Initialization failed, rc = %d\n", rc);
    return(TU_SYS);
  }

/*  Change config state of adapter to diagnose           */
 
  DEBUG1("devid = %s\n", devid);
  if (rc = diagex_cfg_state(devid))
  {
    DEBUG1("diagex_cfg_state failed, rc=%x\n", rc);
    odm_terminate();
    return(TU_SYS);
  }

  if (rc = build_dds(devid))
  {
    DEBUG1("Build_dds failed, rc = %d\n", rc); 
    diagex_initial_state(devid);
    odm_terminate();
    return(TU_SYS);
  }

/*       open path to machine device driver                 */

  sprintf(busstring,"%s/%s","/dev",dds.parent_name);
  if((mdd_fd = open(busstring, O_RDWR)) < 0)
  {
    DEBUG1("MDD_OPEN failed\n");
    rc = MDD_OPEN_FAILED;
    return(rc);
  }

/* Initialize the adapter in case we must field an interrupt  */

/*  get base address                             */

  base_addr = dds.bus_io_addr;
  if (rc= config_reg_write(base_addr_reg, swap32(base_addr)))
  {
    cleanup(devid);
    return(rc);
  }

/* read and write cmd_stat_reg to turn off any unwanted bits  */

  if (rc= config_reg_read(cmd_stat_reg, &r_data))
  {
    cleanup(devid);
    return(rc);
  }
  if (rc= config_reg_write(cmd_stat_reg, r_data))
  {
    cleanup(devid);
    return(rc);
  }

/*  turn on ioen bit in cmd_stat_reg             */

  if (rc= config_reg_write(cmd_stat_reg, ioen_set))
  {
    cleanup(devid);
    return(rc);
  }

/*    Load interrupt handler    */

#ifdef HAVESLIH

#ifdef DIAGPATH
  sprintf(ipath, "%s/%s", (char *)getenv("DIAGX_SLIH_DIR"),"ktat_intr");
  cfg_ld.path = ipath;
#else
  cfg_ld.path = INTERRUPT_HANDLER_PATH;
#endif

/*  printf("Interrupt Handler path: %s\n", cfg_ld.path); */

  errno = 0;
  if(rc= sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld)))
  {
    return(rc);
  }
/*   printf("kmid from queryload is %d\n", cfg_ld.kmid);  */
  
  if(cfg_ld.kmid)
  {
    printf("Interrupt Handler already loaded\n");
  }
  else
    if (rc=sysconfig(SYS_KLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld)))
    {
      diagex_initial_state(devid);
      odm_terminate();
      DEBUG1("SYS_KLOAD of interrupt handler failed: %d\n", errno);
      return(TU_SYS);
    }

/*  printf("%s loaded, kmid = %d\n", cfg_ld.path, cfg_ld.kmid);  */
  dds.kmid = (int) cfg_ld.kmid;
  dds.intr_flags = 0;
  dds.data_ptr = (uchar *) &idata;
  dds.d_count = sizeof(idata);

#endif
/* =======================> */
/*
  prt_dds(&dds);
  scanf("%d", &junk);
*/
/* Open diagex handle for Klickitat  */

  if (rc = diag_open(&dds, &diagex_hdl))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif

    DEBUG1("diag_open failed, rc=0x%08x\n", rc);
    DEBUG1("errno=%d\n", errno);

    diagex_initial_state(devid);
    odm_terminate();
    close(mdd_fd);
    return(TU_SYS);
  }

  if (rc = odm_terminate())
  {
    diag_close(diagex_hdl);
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    close(mdd_fd);
    diagex_initial_state(devid);
    return(TU_SYS);
  }

/*  issue s_reset to adapter                     */

  if (rc = io_read(IOLONG, reset_reg, &r_data))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }
  sleep(1);
/*  turn on the software style bits in bcr20 to allow 32 bits   */

  if (rc = io_write(IOLONG, rap, bcr20))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }
  if (rc = io_write(IOLONG, bdp, software_style))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }

/* read and write csr0 and leave the stop bit on  */

  if (rc = io_write(IOLONG, rap, csr0))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }
  if (rc = io_read(IOLONG, rdp, &r_data))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }
  r_data = r_data | csr0_stop;
  if (rc = io_write(IOLONG, rdp, r_data))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }

/*  set the interrupt mask in csr3   */

  if (rc = io_write(IOLONG, rap, csr3))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }
  if (rc = io_write(IOLONG, rdp, intr_msk))
  {
#ifdef HAVESLIH
    sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
#endif
    cleanup(devid);
    return(rc);
  }
 
  open_cnt++;
  return(0);
}

/***********************************************************
* Name: cleanup
*         
* Function: cleans up if card initializtion fails
*   
************************************************************/

cleanup(char *devid)
{
    diagex_initial_state(devid);
    odm_terminate();
    close(mdd_fd);  
}


/************************************************************
 * NAME: build_dds
 *
 * FUNCTION: Builds the DDS (Defined Data Structure) for the
 *           Klickitat Adapter
 *
 * RETURNS: 0 - success
 *         >0 - failure
 ***********************************************************/
int build_dds(char *logical_name)
{
	int		rc = 0;		/* used for return codes */
	char		sstring[512];	/* working string	*/
	char		*lname;		/* pointer to device name */
	char		*pname;		/* pointer to parent name */
	char		*ut;		/* pointer to device's uniquetype */
	char		*pt;		/* pointer to parent's uniquetype */
	struct PdAt	pdatobj;

	struct cfg_dd	cfg;		/* sysconfig command structure */

	struct Class	*cusdev;	/* customized devices class ptr */
	struct CuDv 	cusobj;		/* customized device object storage */
	struct CuDv 	parobj;		/* customized device object storage */


	/*
	 * Validate Parameters
	 */
	/* logical name must be specified */
	if (logical_name == NULL) {
/*		VERBOSE((OUT,"build_dds: no logical name specified\n")); */
/*    printf("Build_dds failed, no logical name specifies\n");  */

		return(E_LNAME);
	}

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
/*                printf("build_dds: odm_initialize() failed\n");      */
		return(E_ODMINIT);
	}

	/* lock the database */
	if (odm_lock("/etc/objrepos/config_lock",0) == -1) {
/*                printf("build_dds: odm_lock() failed\n");  */
		return(err_exit(E_ODMLOCK));
	}

/*        printf("ODM initialized and locked\n");      */

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
/*                printf("build_dds: open class CuDv failed\n");      */
		return(err_exit(E_ODMOPEN));
	}

	/* search for customized object with this logical name */
	sprintf (sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc == 0) {
		/* No CuDv object with this name */
/*              printf("build_dds: no CuDv with %s\n", sstring);    */
		return(err_exit(E_NOCuDv));
	} else if (rc == -1) {
		/* ODM failure */
/*                printf("build_dds: ODM failure getting CuDv object");  */
		return(err_exit(E_ODMGET));
	}

	sprintf (sstring, "name = '%s'", cusobj.parent);
	rc = (int)odm_get_first(cusdev,sstring,&parobj);
	if (rc == 0) {
		/* Parent device not in CuDv */
/*                printf("build_dds: no parent CuDv with %s\n", sstring);   */
		return(err_exit(E_NOCuDvPARENT));
	} else if (rc == -1) {
		/* ODM failure */
/*                 printf(("build_dds: failed to get parent CuDv object\n")); */
		return(err_exit(E_ODMGET));
	}
	/* parent must be available to continue */
	if (parobj.status != AVAILABLE) {
/*                printf(("build_dds: parent is not AVAILABLE"));  */
		return(err_exit(E_PARENTSTATE));
	}

	/* DDS must be cleared */
	memset(&dds, 0, sizeof(dds));

	/* save some strings for short hand */
	lname = cusobj.name;
	ut = cusobj.PdDvLn_Lvalue;
	pname = cusobj.parent;
	pt = parobj.PdDvLn_Lvalue;
	strcpy(dds.parent_name,pname);
	strcpy(dds.device_name,lname);

/*      printf("getad: pname = %s pt = %s \n",pname,pt);    */
	if ((int)(odm_open_class(CuAt_CLASS)) == -1) {
/*              printf(("bld_dds: can not open CuAt\n"));    */
		return(E_ODMOPEN);
	}
	if ((int)(odm_open_class(PdAt_CLASS)) == -1) {
/*              printf("bld_dds: can not open PdAt\n");   */
		return(E_ODMOPEN);
	}

	/* get bus attributes */
	if ((rc=getatt(&dds.bus_id,'l',CuAt_CLASS,
	    PdAt_CLASS,pname,pt,"bus_id",NULL))>0)
		return(rc);
/*	if ((rc=getatt(&dds.bus_type,'i',CuAt_CLASS,
	    PdAt_CLASS,pname,pt,"bus_type",NULL))>0)
		return(rc);
*/
	/* get device attributes */
	if ((rc=getatt(&dds.bus_io_addr,'l',CuAt_CLASS,
	    PdAt_CLASS,lname,ut,"busio",NULL))>0)
 		return(rc);

	sprintf(sstring, "uniquetype = %s AND attribute = busio",ut);
	if ((rc = (int)odm_get_first(PdAt_CLASS, sstring, &pdatobj)) == 0)
		return(E_NOCuDv);
	else if (rc == -1)
		return(E_ODMGET);

	dds.bus_io_length =
	    (ulong)strtol(pdatobj.width,(char **) NULL,0);

	if ((rc=getatt(&dds.bus_intr_lvl,'i',CuAt_CLASS,
	    PdAt_CLASS,lname,ut,"busintr",NULL))>0)
		return(rc);
	if ((rc=getatt(&dds.intr_priority,'i',CuAt_CLASS,
	    PdAt_CLASS,lname,ut,"intr_priority",NULL))>0)
		return(rc);

	dds.slot_num = atoi(cusobj.connwhere);
        dds.bus_type = BUS_BID;
        dds.maxmaster = 64;
        dds.dma_flags = DMA_MASTER;

/* 
#ifdef CFGDEBUG
	hexdump(*dds_data_ptr,(long)*dds_len);
#endif CFGDEBUG	*/
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);

	odm_terminate();

	return(0);
}

/*
 * NAME: getatt
 *
 * FUNCTION: Reads an attribute from the customized database, predefined
 *	database, or change list.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is linked into the device specific sections of the
 *	various config, and change methods. No global variables are used.
 *
 * NOTES:
 *
 * int
 *	getatt(dest_addr,dest_type,cuat_oc,pdat_oc,lname,utype,att_name,newatt)
 *
 *	dest_addr = pointer to the destination field.
 *	dest_type = The data type which the attribute is to be converted to
 *	's' = string	rep=s
 *	'b' = byte sequence	rep=s,	e.g. "0x56FFE67.."
 *	'l' = long	rep=n
 *	'i' = int	rep=n
 *	'h' = short (half)	rep=n
 *	'c' = char	rep=n,or s
 *	'a' = address	rep=n
 *	cuat_oc	= Customized Attribute Object Class.
 *	pdat_oc	= Predefined Attribute Object Class.
 *	lname	= Device logical name. (or parent's logical name)
 *	utype	= Device uniquetype. (or parent's uniquetype)
 *	att_name	= attribute name to retrieve from the Customized
 *	Attribute Object Class.
 *	newatt	= New attributes to be scanned before reading database
 *
 *
 * RETURNS:
 *	0	= Successful
 *	<0 = Successful (for byte sequence only, = -ve no. of bytes)
 *	>0 = errno (E_NOATTR = attribute not found)
 *
 */

int getatt(dest_addr,dest_type,cuat_oc,pdat_oc,lname,utype,att_name,newatt)
void		*dest_addr;	/* Address of destination	*/
char		dest_type;	/* Destination type	*/
struct Class	*cuat_oc;	/* handle for Customized Attribute OC	*/
struct Class	*pdat_oc;	/* handle for Predefined Attribute OC	*/
char		*lname;		/* device logical name	*/
char		*utype;		/* device unique type	*/
char		*att_name;	/* attribute name	*/
struct attr	*newatt;	/* List of new attributes	*/
{
	struct CuAt	cuat_obj;
	struct PdAt	pdat_obj;
	struct attr	*att_changed();
	struct attr	*att_ptr;
	int		convert_seq();
	int		rc;
	char		srchstr[100];
	char		*val_ptr;
	char		rep;
	int		mm;

	/*
	 * Note: We need an entry from customized, or predefined even if
	 * an entry from newatt is going to be used because there is no
	 * representation (rep) in newatt
	 */

/*      printf("getatt(): Attempting to get attribute %s for device %s\n",
	    att_name, lname);  */
	/* SEARCH FOR ENTRY IN CUSTOMIZED ATTRIBUTE CLASS */

	sprintf(srchstr, "name = '%s' AND attribute = '%s'", lname, att_name);

	if (cuat_oc == (struct Class *)NULL)
		rc = 0;
	else
		rc = (int)odm_get_obj(cuat_oc, srchstr, &cuat_obj, TRUE);

	if (rc == 0) {
		/* OBJECT NOT FOUND, SEARCH IN PREDEFINED ATTRIBUTE CLASS */
		sprintf(srchstr, "uniquetype = '%s' AND attribute = '%s'",
		    utype, att_name);

		if ((rc=(int)odm_get_obj(pdat_oc, srchstr, &pdat_obj, TRUE)) == 0) {
			return(E_NOATTR);
		} else if (rc == -1) {
			return(E_ODMGET);
		}
		/* USE THE PREDEFINED ENTRY (for now) */
		val_ptr = pdat_obj.deflt;
		mm = strcspn(pdat_obj.rep,"sn");
		rep = pdat_obj.rep[strcspn(pdat_obj.rep,"sn")];
	} else if (rc == -1) {
		return(E_ODMGET);
	} else {
		/* USE THE CUSTOMIZED ENTRY (for now) */
		val_ptr = cuat_obj.value;
		rep = cuat_obj.rep[strcspn(cuat_obj.rep,"sn")];
	}

	/* CHECK TO SEE IF THIS ATTRIBUTE IS IN CHANGED LIST */

	if ((att_ptr = att_changed(newatt,att_name))!=NULL)
		val_ptr = att_ptr->value;

	/* CONVERT THE DATA TYPE TO THE DESTINATION TYPE */

	return(convert_att(dest_addr, dest_type, val_ptr, rep));
}

/***********************************************************
* NAME: prt_dds
*
* FUNCTION: Print a DDS for debugging purposes
*
*/
prt_dds(ds)
diagex_dds_t		*ds;
{
	printf("\ndevice_name = \t'%s'\n", ds->device_name);
	printf("parent_name = \t'%s'\n", ds->parent_name);
	printf("slot_num = \t%d\n", ds->slot_num);
	printf("bus_intr_lvl = \t%d\n", ds->bus_intr_lvl);
	printf("intr_priority =\t%d\n", ds->intr_priority);
	printf("intr_flags = \t%d\n", ds->intr_flags);
	printf("dma_lvl = \t%d\n", ds->dma_lvl);
	printf("dma_chan_id = \t%d\n", ds->dma_chan_id);
	printf("bus_io_addr = \t0x%x\n", ds->bus_io_addr);
	printf("bus_io_length =\t0x%x\n", ds->bus_io_length);
	printf("bus_mem_addr = \t0x%x\n", ds->bus_mem_addr);
	printf("bus_mem_length=\t0x%x\n", ds->bus_mem_length);
	printf("dma_bus_mem = \t0x%x\n", ds->dma_bus_mem);
	printf("dma_bus_length=\t0x%x\n", ds->dma_bus_length);
	printf("dma_flags = \t0x%x\n", ds->dma_flags);
	printf("bus_id = \t0x%x\n", ds->bus_id);
	printf("bus_type = \t%d\n", ds->bus_type);
	printf("kmid = \t\t%d\n", ds->kmid);
	printf("data_prt = \t\t%d\n", ds->data_ptr);
	printf("maxmaster = \t%d\n\n", ds->maxmaster);
}

/*
 * NAME: dump_dds
 *
 * FUNCTION: Display a DDS for debugging purposes
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is ONLY AVAILABLE IF COMPILED WITH DEBUG DEFINED
 *
 * RETURNS: NONE
 */

dump_dds(dds, dds_len)
char *dds;
int dds_len;
{
#ifdef CFGDEBUG
	hexdump(dds, (long)dds_len);
#endif
	;
}


/*
 * NAME: hexdump1
 *
 * FUNCTION: Display an array of type char in ASCII, and HEX.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is ONLY AVAILABLE IF COMPILED WITH DEBUG DEFINED
 *
 * RETURNS: NONE
 */


hexdump1(data,len)
char *data;
long len;
{

#ifdef CFGDEBUG

	int	i,j,k;
	char	str[18];

	fprintf(stderr,"hexdump(): length=%ld\n",len);
	i=j=k=0;
	while(i<len) {
		j=(int) data[i++];
		if(j>=32 && j<=126)
			str[k++]=(char) j;
		else
			str[k++]='.';
		fprintf(stderr,"%02x ",j);
		if(!(i%8)) {
			fprintf(stderr,"	");
			str[k++]=' ';
		}
		if(!(i%16)) {
			str[k]='\0';
			fprintf(stderr,"	%s\n\n",str);
			k=0;
		}
	}
	while(i%16) {
		if(!(i%8))
			fprintf(stderr,"	");
		fprintf(stderr,"   ");
		i++;
	}
	str[k]='\0';
	fprintf(stderr,"	%s\n\n",str);
	fflush(stderr);
#endif
	;
}


/*
 * NAME: att_changed
 *
 * FUNCTION: Searches for an attribute in the new_attributes list
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Routines calling att_changed should include pparms.h for
 *	the definition of struct attr.
 *	This routine uses no global variables.
 *
 * NOTES:
 *
 *	if the list of changed attributes (at) is a NULL pointer, the
 *	routine accepts that there are no parameters in the list.
 *	Generally, the list consists of a sequence of attributes with
 *	the last attribute having a name of NULL.
 */
struct attr *att_changed(at,attname)
struct attr *at;
char	*attname;
{
	struct attr *p = at;

	if (at != NULL)
		while(p->attribute != NULL) {
			if(strcmp(p->attribute,attname) == 0)
				return(p);
			p++;
		}
	return((struct attr *)NULL);
}


/*
 * NAME: convert_att
 *
 * FUNCTION: This routine converts attributes into different data types
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Generally this routine is called by getatt(), but it is available
 *	to other procedures which need to convert data which may not also
 *	be represented in the database.
 *	No global variable are used, so this may be dynamically linked.
 *
 * RETURNS:
 *
 *	0 = Successful
 *	<0 = Successful (for byte sequence only, = -ve no. of bytes)
 *	>0 = errno
 */
int convert_att(dest_addr, dest_type, val_ptr, rep)
void		*dest_addr;	/* Address of destination	*/
char		dest_type;	/* Destination type	*/
char		*val_ptr;	/* Address of source	*/
char		rep;		/* Representation of source ('s', or 'n')	*/
{

	if (rep == 's') {
		switch (dest_type) {
		case 's':
			strcpy((char *)dest_addr, val_ptr);
			break;
		case 'c':
			*(char *)dest_addr = *val_ptr;
			break;
		case 'b':
			return(convert_seq(val_ptr, (char *)dest_addr));
		default:
/*			VERBOSE((OUT,"dest_type is %c, should be s, c, or b\n",
			    dest_type)); */
			return(E_BADATTR);
		}
	} else if (rep == 'n') {
		switch (dest_type) {
		case 'l':
			*(long *)dest_addr =
			    strtoul(val_ptr, (char **)NULL, 0);
			break;
		case 'i':
			*(int *)dest_addr =
			    (int)strtoul(val_ptr, (char **)NULL, 0);
			break;
		case 'h':
			*(short *)dest_addr =
			    (short)strtoul(val_ptr, (char **)NULL, 0);
			break;
		case 'c':
			*(char *)dest_addr =
			    (char)strtoul(val_ptr, (char **)NULL, 0);
			break;
		case 'a':
			*(void **)dest_addr =
			    (void *)strtoul(val_ptr, (char **)NULL, 0);
			break;
		default:
/*			VERBOSE((OUT,
			    "dest_type is %c, should be l,i,h,c, or a\n",
			    dest_type)); */
			return(E_BADATTR);
		}
	} else {
/*		VERBOSE((OUT,
		    "Rep field in attribute is %c, should be s, or n\n",
		    rep)); */
		return(E_BADATTR);
	}
	return(0);
}


/*
 * NAME: convert_seq
 *
 * FUNCTION: Converts a hex-style string to a sequence of bytes
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine uses no global variables
 *
 * NOTES:
 *
 *	The string to be converted is of the form
 *	"0xFFAAEE5A567456724650789789ABDEF678"	(for example)
 *	This would put the code FF into the first byte, AA into the second,
 *	etc.
 *
 * RETURNS: No of bytes, or -3 if error.
 *
 */
int convert_seq(source, dest)
char *source;
uchar *dest;
{
	char		byte_val[5];	/* e.g. "0x5F\0" */
	int		byte_count = 0;
	uchar		tmp_val;
	char		*end_ptr;

	strcpy(byte_val, "0x00");

	if (*source == '\0') 		/* Accept empty string as legal */
		return(0);

	if (*source++ != '0')
		return(E_BADATTR);
	if (tolower(*source++) != 'x')
		return(E_BADATTR);

	while ((byte_val[2] = *source) && (byte_val[3] = *(source+1))) {
		source += 2;

		/*
		 * Be careful not to store illegal bytes in case the
		 * destination is of exact size, and the source has
		 * trailing blanks.
		 */
		tmp_val = (uchar) strtoul(byte_val, &end_ptr, 0);
		if (end_ptr != &byte_val[4])
			break;
		*dest++ = tmp_val;
		byte_count++;
	}

	return(-byte_count);
}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.	Used to
 *	back out on an error.
 *
 * NOTES:
 *
 *	err_exit(exitcode)
 *	exitcode = The error exit code.
 *
 * RETURNS:
 *	None
 */
err_exit(exitcode)
char	exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	return(exitcode);
}
