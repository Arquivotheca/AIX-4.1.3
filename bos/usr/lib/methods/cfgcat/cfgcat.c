static char sccsid[] = "@(#)56	1.21  src/bos/usr/lib/methods/cfgcat/cfgcat.c, sysxcat, bos411, 9428A410j 2/22/94 14:33:39";
/*
 * COMPONENT_NAME: (CFGMETH) cfgcat.c - Device-specific part of Config Method 
 *		for Parallel Channel Adapter (PCA)
 *
 * FUNCTIONS:	build_dds, generate_minor, make_special_files, 
 *		download_microcode, query_vpd, define_children, cat_dl_ucode, 
 *		cat_dl_table, get_devid
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/stat.h>
#include <sys/mdio.h>
#include <sys/ciouser.h>
#include <sys/comio.h>
 
/* Local header files */
#include "cfgdebug.h"
#include "catdd.h"
#include <sys/catuser.h>
 
/* Function prototypes */
int build_dds(char *lname, uchar **ddsptr, int *ddslen);
int define_children(char *lname, char *children, long cusdev, int phase);
int download_microcode(char *lname );
long generate_minor(char *lname, int majorno, int *minorno);
int make_special_files(char *lname, dev_t devno);
int query_vpd(struct CuDv *newobj, mid_t kmid,dev_t devno, char *vpdstr);
void methexit(char *msg, int err_code);
int cat_dl_table(int fd, char *fname, uchar type);
int cat_dl_ucode(int fd, char *fname);
static int get_devid(char *logical_name, char *devid);
/* This really should be in <sys/cfgodm.h> ... */
long *genminor(char *device_instance,long major_no,int preferred_minor,
	       int minors_in_grp,int inc_within_grp,int inc_btwn_grp);
 
 
 
#define DEF_SUBCHS	256		/* Default is to init all subchannels */
#define CLAW_CUTABLE    3               /* Claw control unit table            */
 
int claw_set=FALSE;
/*************************************************************************
** The following table is extracted from the PSCA Hardware Specification
** in the section describing the functionality of the POS registers.
** It describes the mapping between the bits in the POS register and the
** Starting shared memory address (base address).
*************************************************************************/
unsigned long cat_addr_map[] = {
			/* Bit	7 6 5 4					*/
	0x00800000, 	/*	0 0 0 0	 Early cards use this addr	*/
	0x00f40000, 	/* 	0 0 0 1	 256K slots at high end of	*/
	0x00f80000, 	/* 	0 0 1 0	     24 bit address space	*/
	0x00fc0000, 	/* 	0 0 1 1					*/
	0x01000000, 	/*	0 1 0 0	 1 Meg slots starting at 16M	*/
	0x01100000, 	/*	0 1 0 1					*/
	0x01200000, 	/*	0 1 1 0					*/
	0x01300000, 	/*	0 1 1 1					*/
	0x01400000, 	/*	1 0 0 0					*/
	0x01500000, 	/*	1 0 0 1					*/
	0x01600000, 	/*	1 0 1 0					*/
	0x01700000, 	/*	1 0 1 1					*/
	0xffc00000, 	/*	1 1 0 0	 1 Meg slots at high end of	*/
	0xffd00000, 	/*	1 1 0 1	     32 bit address space	*/
	0xffe00000, 	/*	1 1 1 0					*/
	0x00000000	/*	1 1 1 1	 Board disabled			*/
};
 
 
/*************************************************************************
** The following table is extracted from the PSCA Hardware Specification
** in the section describing the functionality of the POS registers.
** It describes the mapping between the bits in the POS register and the
** interrupt level.
*************************************************************************/
int cat_int_map[] = {
			/* Bit	2 1 0 */
	4, 		/*	0 0 0 */
	5, 		/* 	0 0 1 */
	6, 		/*	0 1 0 */
	7, 		/*	0 1 1 */
	9, 		/*	1 0 0 */
	10, 		/*	1 0 1 */
	11, 		/*	1 1 0 */
	14, 		/*	1 1 1 */
};
 
#define VALIDATE_ATTR() \
	if (CuAt_ptr == NULL) \
		return E_NOATTR
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME: build_dds()							 */
/* 									 */
/* FUNCTION: This calls the build dds routine with a null attribute list */
/*	pointer.  This process is invoked from the generic config device */
/*	routine (cfgdevice.c) to build the DDS structure for a PCA 	 */
/*	adapter.							 */
/* 									 */
/* EXECUTION ENVIRONMENT:	Called from the process environment.	 */
/*									 */
/* RETURNS: 								 */
/*	0 - SUCCESS   					 		 */
/*	E_NOCuDv - odm_get_first() couldn't find the CuDv object	 */
/*	E_ODMGET - odm_get_first() failed				 */
/*	E_ATTRVAL - an attribute was invalid				 */
/*	E_NOATTR - getattr() couldn't find an attribute			 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int build_dds(
char *lname,
uchar **ddsptr,
int *ddslen)
{
	int i;
	int j;
	int rc;
	struct CuDv CuDv;
	struct CuAt *CuAt_ptr;
	int num_attrs;
	struct cadds *ddi_p;
	static struct cadds ddi;
	int addr;
	int intlvl;
	char sstr[128];
	char *ptrb;
	char temp[4];
	long value;
	int first_sc;	/* First Valid Subchannel (that can be started) */
 
	/* Clear the DDS structure */
	ddi_p = &ddi;
	bzero(ddi_p, sizeof(struct cadds));
	*ddsptr = (uchar *)ddi_p;
	*ddslen = sizeof(ddi);
 
	/*
	** Fill in the DDS with both "hard-coded" and ODM attribute values
	*/
 
	/*
	** Slot Number (Note: the stamped numbers on the back of the machine
	** and the numbers in ODM start with 1, whereas, internally, slots start with 0)
	*/
	sprintf(sstr,"name=%s",lname);
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&CuDv);
	if (rc == 0) {
		return(E_NOCuDv);
	}
	else if (rc == -1) {
		return(E_ODMGET);
	}
	ddi_p->slot_num = atoi(CuDv.connwhere) - 1;
 
	/* Bus ID */
	CuAt_ptr = getattr(CuDv.parent, "bus_id", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->bus_id = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	ddi_p->bus_id |= 0x800C0020;
 
	/* Bus Type */
	CuAt_ptr = getattr(CuDv.parent, "bus_type", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->bus_type = strtoul(CuAt_ptr->value, (char **)NULL, 0);
 
	/* Bus Address */
	CuAt_ptr = getattr(lname, "dma_bus_mem", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->addr_bits = -1;
	addr = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	DEBUG_1("dma_bus_mem = %X\n", addr)
	for (i=0; i<sizeof(cat_addr_map)/sizeof(cat_addr_map[0]); i++)
		if (cat_addr_map[i] == addr) {
			ddi_p->addr_bits = i;
			break;
		}
	if (ddi_p->addr_bits == -1)
		return E_ATTRVAL;		
 
	/* Bus Interrupt Level */
	CuAt_ptr = getattr(lname, "bus_intr_lvl", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->int_bits = -1;
	intlvl = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	DEBUG_1("bus_intr_lvl = %X\n", intlvl)
	for (i=0; i<sizeof(cat_int_map)/sizeof(cat_int_map[0]); i++)
		if (cat_int_map[i] == intlvl) {
			ddi_p->int_bits = i;
			break;
		}
	if (ddi_p->int_bits == -1)
		return E_ATTRVAL;		
 
	/* Interrupt Priority */
	CuAt_ptr = getattr(lname, "intr_priority", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->intr_priority = strtoul(CuAt_ptr->value, (char **)NULL, 0);
 
	/* DMA Arbitration Level */
	CuAt_ptr = getattr(lname, "dma_lvl", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->dma_lvl = strtoul(CuAt_ptr->value, (char **)NULL, 0);
 
	/* "Receive Data Transfer Offset" */
	CuAt_ptr = getattr(lname, "rdto", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->rdto = strtoul(CuAt_ptr->value, (char **)NULL, 0);
 
	/* Resource Name passed to error logging facilities */
	strncpy(ddi_p->resource_name, lname, 16);
 
	ddi_p->burst_bits = 0;				/* Burst rate */
	ddi_p->parity_opts = 0;				/* Don't check parity */
	ddi_p->fair = 0;				/* Don't be a DMA hog */
	ddi_p->dma_enable = 1;				/* Enable DMA */
	ddi_p->io_dma = 1;				/* Enable DMA */
 
	/* Channel Speed */
	CuAt_ptr = getattr(lname, "speed", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->config_params.speed = strtoul(CuAt_ptr->value, (char **)NULL, 0);
 
	/* Transmit Buffer Size (in bytes) */
	CuAt_ptr = getattr(lname, "xmitsz", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->config_params.xmitsz = strtoul(CuAt_ptr->value, (char **)NULL,0);
 
	/* Receive Buffer Size (in bytes) */
	CuAt_ptr = getattr(lname, "recvsz", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->config_params.recvsz = strtoul(CuAt_ptr->value, (char **)NULL,0);
 
	/* Number of Transmit Buffers */
	CuAt_ptr = getattr(lname, "xmitno", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->config_params.xmitno = strtoul(CuAt_ptr->value, (char **)NULL,0);
 
	/* Number of Receive Buffers */
	CuAt_ptr = getattr(lname, "recvno", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->config_params.recvno = strtoul(CuAt_ptr->value, (char **)NULL,0);
 
	/* Starting Valid Subchannel (that can be started) */
	CuAt_ptr = getattr(lname, "first_sc", 0, &num_attrs);
	VALIDATE_ATTR();
	first_sc = strtoul(CuAt_ptr->value, (char **)NULL, 0);
 
	/* Number of Valid Subchannels (that can be started...) */
	CuAt_ptr = getattr(lname, "num_sc", 0, &num_attrs);
	VALIDATE_ATTR();
	ddi_p->config_params.nosubs = strtoul(CuAt_ptr->value, (char **)NULL,0);

	/* CLAW mode HOST name */
	CuAt_ptr = getattr(lname, "host_name", 0, &num_attrs);
	VALIDATE_ATTR();
	strncpy(ddi_p->host_name, CuAt_ptr->value, 8);

	/* CLAW mode ADAPTER name */
	CuAt_ptr = getattr(lname, "adapter_name", 0, &num_attrs);
	VALIDATE_ATTR();
	strncpy(ddi_p->adapter_name, CuAt_ptr->value, 8);
 
	/* CLAW subchannel sets */
	CuAt_ptr = getattr(lname, "clawset", 0, &num_attrs);
	VALIDATE_ATTR();
	ptrb = CuAt_ptr->value;
	for (i=0; i < CAT_MAX_SC; i++)
	   ddi_p->clawset[i] = 0; 
 
	if (strcmp(ptrb, "none")) {
 
    /* First search the input value until a comma is reach, put the      */
	/* string in temp[], then use strtol() to convert the string to long */
	/* integer, do it until the end of the input value                   */
	i = 0;
        while (i || *ptrb) {
			/* 
			** Insert a "0x" prefix if there isn't one, so 
			** strtol() knows el numero es hexidecimal.
			*/
			if (i == 0) {
				temp[0] = '0';
				temp[1] = 'x';
				if (ptrb[0] == '0' && ptrb[1] == 'x') {
					ptrb += 2;
				}
				i += 2;	
			}
			if (isxdigit(*ptrb)) {
				temp[i++] = *ptrb;
			} else if (*ptrb == ',' || *ptrb == NULL) {
				if (i) {
					temp[i] = NULL;
					value = strtol(temp, (char **)NULL, 0);
					/* if odd, out-of-range or dup, return error */
					if ((value && (value % 2)) ||
						value > (CAT_MAX_SC - 2) ||
						ddi_p->clawset[value]) {
						printf("cfgcat: 'clawset' invalid\n");
						return E_BADATTR;
					}
					ddi_p->clawset[value] = 1;
					claw_set = TRUE;
					i = 0;
				}
				if (*ptrb == NULL) {
					break;
				}
			} else {
				printf("cfgcat: 'clawset' invalid\n");
				return E_BADATTR;
			}
			ptrb++;
		} /* while */
	} /* if */
 
	ddi_p->config_params.flags = 0;			/* No flags needed */
 
	/* Subchannels */
	for (i=first_sc; i<(ddi_p->config_params.nosubs + first_sc); i++) {
	   j = i - first_sc;
           if ( ddi_p->clawset[i] ) {
	     if (ddi_p->config_params.subid[j].cutype[0] != CLAW_CUTABLE){
		ddi_p->config_params.subid[j].subchan = i;
		ddi_p->config_params.subid[j].cutype[0] = CLAW_CUTABLE;
		ddi_p->config_params.subid[j].cutype[1] = 0;
		ddi_p->config_params.subid[j].cutype[2] = 0;
		
		/* CLAW works in pairs */
		i++;
		j++;
		ddi_p->config_params.subid[j].subchan = i;
		ddi_p->config_params.subid[j].cutype[0] = CLAW_CUTABLE;
		ddi_p->config_params.subid[j].cutype[1] = 0;
		ddi_p->config_params.subid[j].cutype[2] = 0;
             }
           }
	   else {
	        ddi_p->config_params.subid[j].subchan = i;
		ddi_p->config_params.subid[j].cutype[0] = 1;
		ddi_p->config_params.subid[j].cutype[1] = 2;
		ddi_p->config_params.subid[j].cutype[2] = 0;
           }
        }
	return 0;
} /* build_dds() */
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:								 */
/*	define_children()						 */
/* 									 */
/* FUNCTION:								 */
/*	There are no children.						 */
/*									 */
/* RETURN CODES:							 */
/*	Required							 */
/*		 0 = Successful						 */
/*		-1 = Error						 */
/*	Actual								 */
/*		Always return successful (0).				 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int define_children(
char *lname,
char *children,
long cusdev,
int phase)
{
	DEBUG_0("define_children(): NULL function, returning 0\n")
	return(0);
} /* define_children() */
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:								 */
/*	download_microcode()						 */
/* 									 */
/* FUNCTION:								 */
/*	Download microcode and CU tables.				 */
/*									 */
/* RETURN CODES:							 */
/*	Required							 */
/*		 0 = Successful						 */
/*		other = ODM Error Code					 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int download_microcode(
char *lname)
{
#define FNAME_MAX	128		/* Arbitrary filename maximum length */
#define CUTABLE1	"cu3088b"
#define CUTABLE2	"cu3088e"
#define CUTABLE3	"cu3172b"
	int fd;				/* special-file file descriptor */
	int rc;				/* Return code */
	char fname[FNAME_MAX];		/* special-file file name */
	char ucode[FNAME_MAX];		/* microcode filename */
	char mc_fname[FNAME_MAX]; /* microcode filename with full path */
	struct CuAt *CuAt_ptr;
	int num_attrs;
	int ucode_type;
 
	/*
	** Get the microcode type attribute to determine
	** whether to download microcode and if so, which
	** version, normal or diagnostic, or none (1, 2, or 3).
	*/
	CuAt_ptr = getattr(lname, "ucode_type", 0, &num_attrs);
	VALIDATE_ATTR();
	ucode_type = strtoul(CuAt_ptr->value, (char **)NULL, 0);
	if (ucode_type < 1 || ucode_type > 3) {
		return E_ATTRVAL;
	}
 
	/*
	** Type 3 means don't download any microcode.
	*/
	if (ucode_type == 3) {
		return 0;
	}
	
	/* Get microcode base filename from the PdDv database */
	if (rc = get_devid(lname, ucode)) {
		DEBUG_1("download_microcode(): get_devid() failed,rc==%d\n", rc)
		return rc;
	}
 
	/*
	** Type 2 means use diagnostic microcode.
	*/
	if (ucode_type == 2) {
		strcat(ucode, "d");		/* fe92d */
	}
 
	DEBUG_1("download_microcode(): odm ucode name = %s\n",ucode)
 
	/*
	** VERSIONING == look for microcode with the highest version #
	** and append the release and version numbers.
	*/
	if (!findmcode(ucode, mc_fname, VERSIONING, NULL))
		/* return code of 0 means an error occurred */
		return E_NOUCODE;
	DEBUG_1("download_microcode(): mcode filename = %s\n", mc_fname)
 
	/* Calculate the special-file name and open the file in DIAG mode */
	sprintf(fname, "/dev/%s/D", lname );
	DEBUG_1("download_microcode(): open fname = %s\n", fname)
	if ((fd = open(fname, O_RDONLY)) < 0) {
		DEBUG_2("download_microcode(): couldn't open %s, errno==%d\n", 
			fname, errno)
		return E_DEVACCESS;
	}
 
	/* Download the microcode */
	if (rc = cat_dl_ucode(fd, mc_fname)) {
		DEBUG_0("download_microcode(): couldn't download mcode\n")
		close(fd);
		return rc;
	}
	DEBUG_1("download_microcode(): %s is downloaded\n", mc_fname)
 
	/* This "if" stmt. is added with reference to DEFECT # 47983 */
	if (ucode_type == 2) {
		/* We are in DIAGNOSTICS mode. 			       */
		/* We are done with downloading DIAGNOSTICS microcode. */
		/* Hence, return.				       */
		DEBUG_0("download_microcode(): Done downloading DIAGNOSTICS microcode.\n")
		close(fd);
		return 0;
	}
	
	/* 
	 * We are not done yet.  Download the Control Unit (CU) tables
	 */
	strcpy(ucode, CUTABLE1);
	if (!findmcode(ucode, mc_fname, VERSIONING, NULL)){
		close(fd);  /* Defect 49495 */
		return E_NOUCODE;
	}
	DEBUG_1("download_microcode(): cutable filename = %s\n", mc_fname)
	if (rc = cat_dl_table(fd, mc_fname, 1)) {	/* MUST use 1! */
		DEBUG_1("download_microcode(): couldn't downld %s\n", mc_fname)
		close(fd);
		return rc;
	}
	DEBUG_1("download_microcode(): %s is downloaded\n", mc_fname)
 
	strcpy(ucode, CUTABLE2);
	if (!findmcode(ucode, mc_fname, VERSIONING, NULL)){
		close(fd); /* Defect 49495 */
		return E_NOUCODE;
	}
	DEBUG_1("download_microcode(): cutable filename = %s\n", mc_fname)
	if (rc = cat_dl_table(fd, mc_fname, 2)) {	/* MUST use 2! */
		DEBUG_1("download_microcode(): couldn't downld %s\n", mc_fname)
		close(fd);
		return rc;
	}
	DEBUG_1("download_microcode(): %s is downloaded\n", mc_fname)
 
        if (claw_set) {
		strcpy(ucode, CUTABLE3);
		if (!findmcode(ucode, mc_fname, VERSIONING, NULL)){
			close(fd); /* Defect 49495 */
			return E_NOUCODE;
		}

		DEBUG_1("download_microcode(): cutable filename = %s\n", 
			mc_fname)
	        if (rc = cat_dl_table(fd, mc_fname, 3)) {  /* MUST use 3! */
			DEBUG_1("download_microcode(): couldn't downld %s\n", 
				mc_fname)
		        close(fd);
		        return rc;
	        }
		DEBUG_1("download_microcode(): %s is downloaded\n", mc_fname)
	}
 
	close(fd);
	return 0;
} /* download_microcode() */
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME: generate_minor()						 */
/* 							  	 	 */
/* FUNCTION:								 */
/*	This calls genminor() to get the minor number for this logical   */
/*	device name.							 */
/*									 */
/* RETURN CODES:							 */
/*	   = minor number						 */
/*	-1 = Error							 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
long generate_minor(
char *lname,
int majorno,
int *minorno)
{
	int *minor_list;
 
	DEBUG_0("generate_minor(): BEGIN generate_minor()\n")
	DEBUG_2("generate_minor(): lname=%s, major=%ld\n", lname, majorno)
	if (minor_list = genminor(lname, majorno, -1, 1, 1, 1)) {
		*minorno = *minor_list;
		return 0;
	} else
		return -1;
} /* generate_minor() */
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:								 */
/*	make_special_files()						 */
/* 									 */
/* FUNCTION:								 */
/*	This creates 1 special file with the same name as the logical	 */
/*	device name.							 */
/*									 */
/* RETURN CODES:							 */
/*	 0 = Successful							 */
/*	-1 = Error							 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int make_special_files(
char *lname,
dev_t devno)
{
#define FLAGS (S_IFMPX | S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
	char *special_file;
 
	if ((special_file = malloc(strlen(lname)+4)) == NULL)
		return ENOMEM;
	sprintf(special_file, "%s", lname );
	return mk_sp_file(devno, special_file, FLAGS);
} /* make_special_files() */
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:								 */
/*	query_vpd()							 */
/* 									 */
/* FUNCTION:								 */
/*	Retrieve Vital Product Data (VPD) from the adapter card in	 */
/*	order to store it in the database from later use.		 */
/*									 */
/* RETURN CODES:							 */
/*	 0 = Successful							 */
/*	-1 = Error							 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int query_vpd(
struct CuDv *newobj,
mid_t kmid,		/* Kernel module I.D. for Dev Driver */
dev_t devno,		/* Concatenated Major & Minor No.s */
char *vpdstr)		/* String to store vpd in */
{
	struct	cfg_dd	cfg;			/* dd config structure */
	/* struct	vital_product_data vpd_area;*/	/* Storage area for VPD */
	char vpd_area[VPDSIZE];			/* Storage area for VPD */
	ulong	ul;
 
	DEBUG_0("query_vpd(): BEGIN query_vpd()\n")
	/* Initialize the cfg structure for a call to sysconfig(), which */
	/* will in turn call the ddconfig() section of the appropriate	 */
	/* device driver.						 */
	cfg.kmid = kmid;		/* the kmid of the device driver */
	cfg.devno = devno; 		/* concatenated Major#, and Minor# */
	cfg.cmd = CFG_QVPD;		/* Command to read VPD */
	cfg.ddsptr = vpd_area;		/* Storage space for VPD */
	cfg.ddslen = sizeof(vpd_area);	/* Size of VPD storage area */
 
	/* Make the call to sysconfig: */
	if(sysconfig(SYS_CFGDD, &cfg, sizeof(cfg)) < 0) {
		DEBUG_0("query_vpd(): Dump of vpd_area\n")
		/*hexdump(vpd_area, (long)(((vpd_area[3] << 8) | vpd_area[4]) * 2)); */
		switch(errno) {
			case EINVAL:
				DEBUG_1("query_vpd(): invalid kmid = %d\n", kmid)
				break;
			case EACCES:
				DEBUG_0("query_vpd(): not privileged\n")
				break;
			case EFAULT:
				DEBUG_0("query_vpd(): i/o error\n")
				break;
			case ENODEV:
				DEBUG_1("query_vpd(): invalid devno = 0x%x\n",devno)
				break;
			default:
				DEBUG_1("query_vpd(): error = 0x%x\n",errno)
				break;
		}
		return(-1);
	}
 
	/* Store the VPD in the CuVPD object */
	put_vpd(vpdstr, &vpd_area[3], (((vpd_area[3] << 8) | vpd_area[4]) * 2) - 3);
 
	return(0);
} /* query_vpd() */
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:								 */
/*	cat_dl_table							 */
/* 									 */
/* FUNCTION:								 */
/*	Download the CU tables to the adapter.				 */	
/*									 */
/* RETURN CODES:							 */
/*	 0 = Successful							 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int 
cat_dl_table(
int fd,
char *fname,
uchar type)
{
	struct cat_cu_load	cmd;
	int			tbl_fd;		/* cu table file descriptor */
 
	cmd.cu_type = type;
	cmd.overwrite = 1;
 
	if ((tbl_fd = open(fname, O_RDONLY)) < 0) {
		DEBUG_1("cat_dl_table(): couldn't open %s\n", fname);
		return(-1);
	}
 
	if ((cmd.length = lseek(tbl_fd, 0L, 2)) == -1) {
		DEBUG_1("cat_dl_table(): couldn't seek to end in %s\n", fname);
		close(tbl_fd);
		return(-1);
	}
 
	if (lseek(tbl_fd, 0L, 0) == -1) {
		DEBUG_1("cat_dl_table(): couldn't seek to 0 in %s\n", fname);
		close(tbl_fd);
		return(-1);
	}
 
	if ((cmd.cu_addr = malloc(cmd.length)) == NULL) {
		DEBUG_0("cat_dl_table(): couldn't malloc\n");
		close(tbl_fd);
		return(-1);
	}
 
	if (read(tbl_fd, cmd.cu_addr, cmd.length) == -1) {
		DEBUG_1("cat_dl_table(): couldn't read %s\n", fname);
		free(cmd.cu_addr);
		close(tbl_fd);
		return(-1);
	}
 
	if (ioctl(fd, CAT_CU_LOAD, &cmd) == -1) {
		DEBUG_1("cat_dl_table(): CAT_CU_LOAD failed, errno==%d\n", 
			errno);
		free(cmd.cu_addr);
		close(tbl_fd);
		return(-1);
	}
	/* The "else if" part is added with reference to DEFECT # 52818. */
	else if (cmd.status != DNLD_SUCC) {
		DEBUG_1("cat_dl_table(): download failed errno==%d\n", errno); 
		free(cmd.cu_addr);
		close(tbl_fd);
		return E_NOUCODE;
	}
 
	free(cmd.cu_addr);
	close(tbl_fd);
	return(0);
} /* cat_dl_table() */
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:	cat_dl_ucode					 	 */
/* 									 */
/* FUNCTION:	Download microcode to adapter.				 */
/*									 */
/* RETURN CODES:							 */
/*	0 = Successful							 */
/*	E_NOUCODE = error reading ucode file				 */
/*	E_UCODE = ioctl() failed					 */
/*	E_MALLOC = malloc() failed					 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int
cat_dl_ucode(
int fd,
char *fname)
{
	struct cat_dnld		cmd;		/* download command */
	int			mcode_fd;	/* microcode file descriptor */
 
	DEBUG_0("in cat_dl_ucode():\n"); 
	/* Open the file that contains the microcode */
	if ((mcode_fd = open(fname, O_RDONLY)) < 0) {
		DEBUG_1("cat_dl_ucode(): open failed errno==%d\n", errno); 
		return E_NOUCODE;
	}
 
	/* Get the microcode length in bytes */
	if ((cmd.mcode_len = lseek(mcode_fd, 0L, 2)) == -1) {
		DEBUG_1("cat_dl_ucode(): lseek/len failed errno==%d\n", errno); 
		close(mcode_fd);
		return E_NOUCODE;
	}
 
	/* Allocate memory to hold the microcode */
	if ((cmd.mcode_ptr = malloc(cmd.mcode_len)) == NULL) {
		DEBUG_1("cat_dl_ucode(): malloc failed errno==%d\n", errno); 
		close(mcode_fd);
		return E_MALLOC;
	}
 
	/* Read the microcode into the command structure */
	if (lseek(mcode_fd, 0L, 0) == -1) {
		DEBUG_1("cat_dl_ucode(): lseek/res failed errno==%d\n", errno); 
		close(mcode_fd);
		return E_NOUCODE;
	}
	if (read(mcode_fd, cmd.mcode_ptr, cmd.mcode_len) == -1) {
		DEBUG_1("cat_dl_ucode(): read failed errno==%d\n", errno); 
		free(cmd.mcode_ptr);
		close(mcode_fd);
		return E_NOUCODE;
	}
 
	/* Call driver to download microcode */
	if (ioctl(fd, CAT_DNLD, &cmd) == -1) {
		DEBUG_1("cat_dl_ucode(): ioctl failed errno==%d\n", errno); 
		free(cmd.mcode_ptr);
		close(mcode_fd);
		return E_UCODE;
	} 
	/* The "else if" part is added with reference to DEFECT # 52818. */
	else if (cmd.status != DNLD_SUCC) {
		DEBUG_1("cat_dl_ucode(): download failed errno==%d\n", errno); 
		free(cmd.mcode_ptr);
		close(mcode_fd);
		return E_NOUCODE;
	} 
 
	free(cmd.mcode_ptr);
	close(mcode_fd);
	DEBUG_0("exitting cat_dl_ucode():\n");
	return 0;
} /* cat_dl_ucode() */
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* NAME:	get_devid					 	 */
/* 									 */
/* FUNCTION:	Get the device ID from the PdDv class of ODM.		 */
/*									 */
/* RETURN CODES:							 */
/*	0 = Successful							 */
/*	E_ODMINIT = couldn't initialize ODM				 */
/*	E_ODMOPEN = couldn't open an ODM object class			 */
/*	E_ODMCLOSE = couldn't close an ODM object class			 */
/*	E_NOCuDv = no CuDv entry with given name			 */
/*	E_ODMGET = odm_get_first() failed				 */
/*	E_NOPdDv = no PdDv entry with given name			 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static int
get_devid(
char *logical_name,
char *devid)
{
        struct Class *cusdev;           /* customized devices class ptr */
        struct Class *predev;           /* predefined devices class ptr */
        struct CuDv cusobj;             /* customized device object storage */
        struct PdDv preobj;             /* predefined device object storage */
	char sstring[256];		/* search criteria string */
	int devid_value;		/* numeric version of device ID */
	int rc;				/* return code */
 
        /* Start up odm */
        if (odm_initialize() == -1) {
                /* initialization failed */
                DEBUG_0("cfgcat.c/get_devid(): odm_initialize() failed\n")
		odm_terminate();
                return(E_ODMINIT);
        }
 
        /* Open customized devices object class */
        if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
                DEBUG_0("cfgcat.c/get_devid(): open class CuDv failed\n");
		odm_terminate();
                return(E_ODMOPEN);
        }
 
        /* Search for customized object with this logical name */
        sprintf(sstring, "name = '%s'", logical_name);
        rc = (int)odm_get_first(cusdev,sstring,&cusobj);
        if (rc==0) {
                /* No CuDv object with this name */
                DEBUG_1("cfgcat.c/get_devid(): failed to get CuDv obj for %s\n",
			logical_name);
		odm_close_class(CuDv_CLASS);
		odm_terminate();
                return(E_NOCuDv);
        }
        else if (rc==-1) {
                /* ODM failure */
                DEBUG_0("cfgcat.c/get_devid(): ODM failed getting CuDv object");
		odm_close_class(CuDv_CLASS);
		odm_terminate();
                return(E_ODMGET);
        }
 
	/* Close the CuDv object class */
        /* Close predefined device object class */
        if (odm_close_class(CuDv_CLASS) == -1) {
                DEBUG_0("cfgcat.c/get_devid(): close object class CuDv failed");
                return(E_ODMCLOSE);
	}
 
        /* Open predefined devices object class */
        if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
                DEBUG_0("cfgcat.c/get_devid(): open class PdDv failed\n");
		odm_terminate();
                return(E_ODMOPEN);
        }
 
        /* Get predefined device object for this logical name */
        sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
        rc = (int)odm_get_first(predev, sstring, &preobj);
        if (rc==0) {
                /* No PdDv object for this device */
                DEBUG_0("cfgcat.c/get_devid(): failed to find PdDv object\n");
		odm_terminate();
		odm_close_class(PdDv_CLASS);
                return(E_NOPdDv);
        }
        else if (rc==-1) {
                /* ODM failure */
                DEBUG_0("cfgcat.c/get_devid(): ODM failed getting PdDv object");
		odm_terminate();
		odm_close_class(PdDv_CLASS);
                return(E_ODMGET);
        }
 
        /* Close predefined device object class */
        if (odm_close_class(PdDv_CLASS) == -1) {
                DEBUG_0("cfgcat.c/get_devid(): close object class PdDv failed");
                return(E_ODMCLOSE);
        }
 
	/* Terminate access to ODM */
	odm_terminate();
 
	/* */
	devid_value = strtoul(preobj.devid, (char **)NULL, 0);
	devid_value = ((devid_value << 8)&0xFF00)|((devid_value >> 8)&0xFF);
	sprintf(devid, "%x", devid_value);
	
	return 0;
} /* get_devid() */
