static char sccsid[] = "@(#)29  1.3  src/bos/usr/ccs/lib/libcfg/POWER/bt_rs6k.c, libcfg, bos41J, bai15 4/11/95 15:48:28";
/*
 * COMPONENT_NAME: (LIBCFG) BUILD TABLES RS6K Module
 *
 * FUNCTIONS:
 *	build_lists_rs6k
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#include <sys/mdio.h>       /* For MACH_DD_IO struct                 */
#include <sys/iocc.h>       /* For NUM_TCES_PPC() and ICF_NUM_TCWS() */ 

#define _KERNSYS
#include <sys/systemcfg.h>  /* For __power_pc() and __rs6k_smp_mca() */
#undef  _KERNSYS

#include "adapter.h"
#include "bt.h"
#include "pt_extern.h"

#define MAX_IO_ADDR  0x0ffff /* All bus IO address are in the first 64K    */
#define MAX_PPC_IOCC 0x7ffff /* Power PC IOCC address space from 64 - 512K */
#define XIOBIT       0x80    /* Bit in cfgreg to identify XIO machine      */

/*------------------*/
/* Global variables */
/*------------------*/

/* Share with attributes list head defined in bt_common.c */
extern share_list_t *ShareList;

/* Platform specific variables */
static unsigned long ResvDMA67,    /* Flag to reserve DMA lvls 6,7 for RSC   */ 
                     ResvIoccMem,  /* Flag to reserve 16 bytes at 0x00fffff0 */
                     IOAddrBot,    /* Bus I/O start addresses                */ 
                     IOAddrTop,    /* Bus I/O end addresses                  */
                     MAddrBot,     /* TCE/TCW mapped memory start addresses  */ 
                     MAddrTop,     /* TCE/TCW mapped memory end addresses    */
                     BAddrBot,     /* Bus memory start addresses             */ 
                     BAddrTop;     /* Bus memory end addresses               */

/*----------------------*/
/* Forward declarations */
/*----------------------*/
int build_lists_rs6k(char *, char *, adapter_t **, attribute_t **, int);
static int get_platform_limits(char *);
static void build_attribute_list_rs6k(int *, adapter_t *, int);
static void process_bus_attr_list(int *, adapter_t *);
static void get_domain_cb(int *, attribute_t *, unsigned long *, 
                          unsigned long *, unsigned long *, unsigned long *);

#include "bt_extern.h"

/***************************************************************************/
/*                    EXTERNALLY VISIBLE FUNCTIONS                         */
/***************************************************************************/

/***************************************************************************/
/* Function Name : BUILD_LISTS()                                           */
/* Function Type : External C function                                     */
/* Purpose       : Constructs the adapter and attributes lists for all of  */
/*                 the devices on the named bus.                           */
/* Arguments     :                                                         */ 
/*  logname        - device logical name (for single device configuration) */
/*  busname        - name of the bus                                       */
/*  flags          - controls for low level busresolve command             */
/*  adapter_list   - returned pointer to head of the adapter list          */
/*  attribute_list - returned pointer to head of the attribute list        */  
/*                                                                         */
/* Return Value (integer return code from /usr/include/cf.h) :             */
/*                                                                         */
/*  Successful termination (No devices eliminated)                         */
/*  ----------------------------------------------                         */
/*  E_OK          All lists built successfully                             */
/*                                                                         */
/*  Device termination (One or more devices eliminated)                    */
/*  ---------------------------------------------------                    */
/*  E_NOCuDv      No Customized Device data for a named device             */
/*  E_NOPdDv      No Predefined Device data for a named device             */
/*  E_BADATTR     An attribute's value could not be adjusted properly or   */
/*                could not be resonably ignored                           */
/*  E_BUSRESOURCE Not enough bus resource to satisfy device requirement    */
/*                                                                         */
/*  Fatal errors (Processing is terminated immediately)                    */
/*  ---------------------------------------------------                    */
/*  E_MALLOC      System failed to allocate virtual storage                */
/*  E_ODMGET      An ODM error occurred while retrieving data from the ODM */
/*  E_PARENTSTATE Bus is not in the avalable state                         */
/*  E_OPEN        Could not open bus special file                          */
/*  E_DEVACCESS   MDD ioctl failed                                         */
/*                                                                         */
/***************************************************************************/
int build_lists_rs6k(logname, busname, adapter_list, attribute_list, flags)
	char *logname;
	char *busname;
	adapter_t **adapter_list;
	attribute_t **attribute_list;
	int flags; 
{
	adapter_t *adap_p;
	attribute_t *attributes_tail = (attribute_t *)NULL;
	int rc = E_OK;

	/* Get some white space in the logfile */
	log_message(0,"\n");

	/* Initialize return pointers to lists */
	*adapter_list	 = (adapter_t *)NULL;
	*attribute_list = (attribute_t *)NULL; 

	/*---------------------------------*/
	/* Set up platform specific values */
	/*---------------------------------*/
	rc = get_platform_limits(busname);
	if (rc)
	{
		destroy_lists(adapter_list, attribute_list);
		return rc;
	}

	log_message(0,"\nProcessing adapters and bus resource attributes :\n");

	/*------------------------*/
	/* Build the adapter list */
	/*------------------------*/

	*adapter_list = build_adapter_list(&rc, busname, logname);
	if (rc == E_MALLOC || rc == E_ODMGET || rc == E_PARENTSTATE) 
	{
		destroy_lists(adapter_list, attribute_list);
		return rc;
	}

	/*-------------------------------------------*/
	/* Build the attribute list for each adapter */
	/*-------------------------------------------*/

	for ( adap_p = *adapter_list ; adap_p ; adap_p = adap_p->next )
	{

		/* Build this adapter's attributes list */
		if (flags & COMMAND_MODE_FLAG && flags & RESOLVE_MODE_FLAG)
			build_attribute_list_rs6k(&rc, adap_p, TRUE);
		else
			build_attribute_list_rs6k(&rc, adap_p, FALSE);
		if (rc == E_MALLOC || rc == E_ODMGET)
		{
			destroy_lists(adapter_list, attribute_list);
			return rc;
		}

		/* Process MCA bus attributes */
		if (!strcmp(adap_p->logname, busname))
		{
			process_bus_attr_list(&rc, adap_p);
			if (rc == E_MALLOC || rc == E_ODMGET) 
			{
				destroy_lists(adapter_list, attribute_list);
				return rc;
			}
		}

		if (adap_p->attributes == (attribute_t *)NULL) 
			continue;	

		/* Link tail of last attribute list to head of this attribute list */

		if (attributes_tail == (attribute_t *)NULL) 
			attributes_tail = adap_p->attributes;
		else 
			attributes_tail->next = adap_p->attributes;

		for ( ; attributes_tail->next ; ) 
			attributes_tail = attributes_tail->next; 

		/* Make the first attribute list the returned attributes list */ 

		if (*attribute_list == (attribute_t *)NULL) 
			*attribute_list = adap_p->attributes;

	} /* end for () building attribute list for each adapter */

	/* Apply any share with attributes */
	apply_share_attributes(&rc);

	/* Print a summary of the adapter/attributes lists to the log file */
	log_message(0,"\nDevice bus resource possible values :\n");
	log_resource_summary(1, *attribute_list, NULL, INTRO);

	/*---------------------------------------------------------*/
	/* If we are resolving all devices, mark them all DEFINED. */
	/* Since they are already in AVAILABLE/DEFINED order this  */
	/* should not matter as far as conflicts for the AVAILABLE */
	/* devices - but will allow adjustment, where necessary,   */
	/* for their attributes in resolve_lists().                */
	/*---------------------------------------------------------*/

	if (flags & COMMAND_MODE_FLAG && flags & RESOLVE_MODE_FLAG)
	{
		for (adap_p = *adapter_list ; adap_p ; adap_p = adap_p->next)
			adap_p->status = DEFINED;
	}

	return rc; 
} /* end build_lists() */

/***************************************************************************/
/* Function Name : GET_PLATFORM_LIMITS()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Perform platform spcific processing :                   */
/*                 1) Determine if DMA 6,7 are reserved (RSC machines)     */
/*                 2) Establish valid memory ranges for bus memory         */
/*                 3) Determine the value for the bus_iocc_mem attribute   */ 
/* Global Vars   :                                                         */
/*  ResvDMA67    - Boolean flag to reserve DMA levels 6,7 for RSC          */
/*  ResvIoccMem  - Boolean flag to reserve IOCC space in MADDR             */ 
/*  IOAddrBot    - First valid byte of IOADDR space                        */
/*  IOAddrTop    - Last valid byte of IOADDR space                         */
/*  MAddrBot     - First valid byte of MADDR space                         */
/*  MAddrTop     - Last valid byte of MADDR space                          */
/*  BAddrBot     - First valid byte of BADDR space                         */
/*  BAddrTop     - Last valid byte of BADDR space                          */
/*                                                                         */
/* Arguments     :                                                         */
/*  busname      - Name of the bus being configured                        */
/*                                                                         */
/* Return Value (integer return code from /usr/include/cf.h) :             */
/*    E_ODMGET      An ODM error occurred while retrieving ODM data        */
/*    E_OPEN        Could not open bus special file                        */
/*    E_DEVACCESS   MDD ioctl failed                                       */
/*                                                                         */
/***************************************************************************/
static int get_platform_limits(busname)
	char *busname;
{
	char busspecialfile [32]; 
	int rc = E_OK, fd;
	MACH_DD_IO mddRecord;
	unsigned long modelcode, nmbr_tcws = 0, cfgreg = 0, tceanchor = 0;

	log_message(0, "Platform specific resource values : \n");

	/*---------------------------------------*/
	/* Need to reserve DMA levels 6,7 if RSC */
	/*---------------------------------------*/

	ResvDMA67 = FALSE;
	modelcode = get_modelcode(&rc);
	if (rc == E_ODMGET)
		return E_ODMGET;
	if (rc == E_NOATTR)
		rc = E_OK; /* Not an RSC machine, reset return code */
	else if (modelcode & 0x00000200)
		ResvDMA67 = TRUE; /* This is a RSC machine */

	/* Log DMA reserved values */
	if (ResvDMA67)
		log_message(1, "Populating tables with DMA levels 6,7 not available on this model\n"); 

	/*------------------------------------------------------------------*/
	/* Get the IOCC config register contents to determine the amount of */
	/* MADDR space and wether we are on an XIO machine or IOCC machine  */
	/*------------------------------------------------------------------*/

	sprintf(busspecialfile, "/dev/%s", busname);
	if ((fd = open(busspecialfile, O_RDWR)) == -1)
	{
		log_message(0, "Cannot open bus special file : %s\n", busspecialfile);
		return E_OPEN;
	}

	/* Get the configuration register */
	mddRecord.md_size = 1;
	mddRecord.md_incr = MV_WORD;
	mddRecord.md_data = (unsigned char *)&cfgreg;
	if (__power_pc())
		mddRecord.md_addr = IO_IOCC_PPC + IOCC_CFG_REG;
	else /* __power_rs() */
		mddRecord.md_addr = IO_IOCC + 0x10;

	if (ioctl(fd, MIOCCGET, &mddRecord) != 0) 
	{
		log_message(0, "Failed mdd ioctl\n") ;
		close(fd);
		return E_DEVACCESS;
	}

	/* Get the TCE anchor */
	if (__power_pc())
	{
		mddRecord.md_data = (unsigned char *)&tceanchor;
		mddRecord.md_addr = IO_IOCC_PPC + IOCC_TCE_ADDR;
		if (ioctl(fd, MIOCCGET, &mddRecord) != 0)
		{
			log_message(0, "Failed mdd ioctl\n") ;
			close(fd);
			return E_DEVACCESS;
		}
	}
	close(fd);

	/*---------------------------------------------------------------------*/
	/* Get bus memory address limits                                       */
	/*---------------------------------------------------------------------*/
	/*              POWER                           POWER PC               */
	/*           bus memory                        bus memory              */
	/*          -------------         4 Gb        -------------            */
	/*         |             |                   |             |           */
	/*         |   BADDR     |                   |   BADDR     |           */
	/*         |             |                   |             |           */
	/*         |             |                   |             |           */ 
	/*         |      A      |                   |      A      |           */ 
	/*          ------|------     depends on      ------|------            */ 
	/*         |      V      |     TCWs/TCEs     |      V      |           */ 
	/*         |             |                   |             |           */ 
	/*         |   MADDR     |                   |   MADDR     |           */ 
	/*         |             |                   |             |           */ 
	/*         |             |      512 Kb        =============            */ 
	/*         |             |                   |  iocc mem   |           */ 
	/*          =============        64 Kb        =============            */ 
	/*         |   IOADDR    |                   |   IOADDR    |           */ 
	/*          -------------          0          -------------            */ 
	/*                                                                     */
	/* 1) For all machines, the BUS I/O address space is allocated from    */
	/*    bus memory in the range 0K thru 64K - 1.                         */
	/* 2) For Power PC machines, the IOCC address space is assigned in     */
	/*    the range 64K thru 512K - 1, from bus memory.                    */
	/* 3) Also note that MADDR type attributes are limited to the TCW/TCE  */
	/*    mapped memory space while BADDR type attributes may be assigned  */
	/*    at any address above iocc mem and IOADDR (even though this is    */
	/*    not illustrated).                                                */
	/*---------------------------------------------------------------------*/

	/* Bus IO from 0K thru 64K - 1 */
	IOAddrBot = 0x00000000; 
	IOAddrTop = MAX_IO_ADDR;

	/* BADDRs top at 4Gb */
	BAddrTop  = 0xffffffff;

	/* Set BADDR/MADDR bot and get TCE/TCW count */
	if (__power_pc())	
	{
		MAddrBot = BAddrBot = MAX_PPC_IOCC + 1;
		nmbr_tcws = NUM_TCES_PPC(tceanchor) - SLV_TCES_PPC(cfgreg);
	}
	else /* __power_rs() */
	{
		MAddrBot = BAddrBot = MAX_IO_ADDR + 1;
		nmbr_tcws = ICF_NUM_TCWS(cfgreg);
	}

	/* Multiply TCE/TCW count by 4K to get MADDR memory size */
	MAddrTop = (nmbr_tcws << 12) - 1;

	/* Log the memory limits */
	log_message(1, "BUS I/O Addresses             = %x - %x\n", IOAddrBot, IOAddrTop);
	log_message(1, "TCE/TCW Mapped Bus Memory     = %x - %x\n", MAddrBot, MAddrTop);
	log_message(1, "non-TCE/TCW Mapped Bus Memory = %x - %x\n", BAddrBot, BAddrTop);

	/*-------------------------------------------------------*/
	/* Set flag to reserve IOCC mem and log for IOCC machine */
	/*-------------------------------------------------------*/
	if (__power_pc() || cfgreg & XIOBIT)
		ResvIoccMem = FALSE;
	else
	{
		/* IOCC machine */
		ResvIoccMem = TRUE;
		log_message(1, "Populating tables with memory %x - %x not available on this model\n", 
		            0x00fffff0, 0x00ffffff);
	}

	return E_OK;
} /* end of get_platform_limits() */

/***************************************************************************/
/* Function Name : PROCESS_BUS_ATTR_LIST()                                 */
/* Function Type : Internal C function                                     */
/* Purpose       : Sets up platform sensitive attributes for MCA bus.      */
/* Arguments     :                                                         */ 
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - adapter_t structure pointer                             */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void process_bus_attr_list(rc, adap_p)
	int *rc;
	adapter_t *adap_p;
{
	attribute_t *attr_p, *attr_p2;
	char attr_name[ATTRNAMESIZE];

	/*---------------------------------------------------*/
	/* Process the bus_iocc_mem attribute - 16 bytes are */
	/* reserved at address 0x00fffff0 for IOCC machines  */
	/*---------------------------------------------------*/

	if (ResvIoccMem)
	{
		/* Find attribute and set width to 16 */
		for (attr_p = adap_p->attributes ; attr_p ; attr_p = attr_p->next)
		{
			if (strcmp(attr_p->name, "bus_iocc_mem"))
				continue;

			attr_p->width = 16;
			break;
		}
	}
	else /* Not an IOCC machine */
	{
		/* Remove bus_iocc_mem attribute from the list so we don't reserve memory */
		if (!strcmp(adap_p->attributes->name, "bus_iocc_mem"))
		{
			attr_p = adap_p->attributes;
			adap_p->attributes = adap_p->attributes->next;
			attr_p->next = NULL;
			destroy_attribute_list(rc, attr_p);
		}
		else
		{
			for (attr_p = adap_p->attributes ; attr_p->next ; attr_p = attr_p->next)
			{
				if (strcmp(attr_p->next->name, "bus_iocc_mem"))
					continue;

				attr_p2 = attr_p->next;
				attr_p->next = attr_p2->next;
				attr_p2->next = NULL;
				destroy_attribute_list(rc, attr_p2);
				break;
			}
		}
	}

	/*--------------------------------*/
	/* Reserve DMA levels 6,7 for RSC */
	/*--------------------------------*/

	if (ResvDMA67)
	{
		/* Add bus_resv_dma attribute to end of list */

		attr_p = new_attribute(rc, adap_p, NULL, NULL);
		if (*rc == E_MALLOC)
			return;

		sprintf(attr_name, "bus_resv_dma");
		setup_list_attribute(rc, adap_p, attr_p, attr_name, DMALVL, 6, 2, 0, 15);
		if (*rc == E_MALLOC)
			return;
	}

	return;
} /* end of process_bus_attr_list() */

/***************************************************************************/
/* Function Name : BUILD_ATTRIBUTE_LIST_RS6K()                             */
/* Function Type : Internal C function                                     */
/* Purpose       : Constructs a device attributes list for all devices     */
/*                 attached to the specified adapter.                      */
/* Arguments     :                                                         */ 
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - adapter_t structure pointer                             */
/*  incusr       - Bool include user modified values                       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void build_attribute_list_rs6k(rc, adap_p, incusr)
	int *rc;
	adapter_t *adap_p;
	int incusr;
{
	struct PdAt *pdat = (struct PdAt *)NULL;
	struct CuAt *cuat = (struct CuAt *)NULL;
	struct listinfo pdatinfo, cuatinfo;
	char query_str[UNIQUESIZE + 28]; /* Sized for longest criteria */

	/* Get bus resource Predefined Attributes for this device */
	sprintf(query_str, "uniquetype = %s AND type != R", adap_p->utype);
	pdat = odm_get_list(PdAt_CLASS, query_str, &pdatinfo, 16, 0);
	if (pdat == (struct PdAt *)NULL) 
		return;
	if (pdat == (struct PdAt *)FAIL) 
	{
		log_message(0, "odm_get failed for PdAt : %s\n", query_str); 
		*rc = E_ODMGET;
		return;
	}

	/* Get bus resource Customized Attributes for this device */
	sprintf(query_str, "name = %s AND type != R", adap_p->logname);
	cuat = odm_get_list(CuAt_CLASS, query_str, &cuatinfo, 16, 0);
	if (cuat == (struct CuAt *)FAIL) 
	{
		odm_free_list(pdat, &pdatinfo);
		log_message(0, "odm_get failed for CuAt : %s\n", query_str); 
		*rc = E_ODMGET;
		return;
	}

	/* Process bus resource attributes */
	process_bus_resource_attributes(rc,adap_p,pdat,pdatinfo.num,cuat,cuatinfo.num);
	if (*rc == E_MALLOC)
	{
		odm_free_list(pdat, &pdatinfo);
		if (cuat != (struct CuAt *)NULL)
			odm_free_list(cuat, &cuatinfo);
		return;
	}

	/* Process bus resource modifier attributes */
	process_modifier_attributes(rc,adap_p,pdat,pdatinfo.num,cuat,cuatinfo.num);
	if (*rc == E_MALLOC)
	{
		odm_free_list(pdat, &pdatinfo);
		if (cuat != (struct CuAt *)NULL)
			odm_free_list(cuat, &cuatinfo);
		return;
	}

	odm_free_list(pdat, &pdatinfo);
	if (cuat != (struct CuAt *)NULL)
		odm_free_list(cuat, &cuatinfo);

	/* Verify attribute values and set value limits */
	verify_bus_resource_attributes(rc, adap_p, incusr, get_domain_cb);

	return;
} /* end build_attribute_list_rs6k() */

/***************************************************************************/
/* Function Name : GET_DOMAIN_CB()                                         */
/* Function Type : Callback C function                                     */
/* Purpose       : This callback function provides the platform min/max    */
/*                 values and database min/max values given a pointer to   */
/*                 an attribute struct.                                    */
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h (reserved)           */
/*  attr_p         - Bus resource attribute                                */
/*  platmin_p      - Minimum value in platform domain                      */
/*  platmax_p      - Maximum value in platform domain                      */
/*  dbmin_p        - Minimum valid value in database PdAt                  */
/*  dbmax_p        - Maximum value value in database PdAt                  */
/*                                                                         */
/* Return Value  : None (reserved)                                         */
/* Notes         :                                                         */
/*                 1) Values for the limits should always conform to :     */
/*                                                                         */
/*                        dbmin <= platmin <= platmax <= dbmax             */
/*                                                                         */
/*                 2) The PdAt possible values are always pruned to be     */
/*                    within both the database and platform limits :       */
/*                                                                         */
/*            platmin <= PdAt value <= (PdAt value + width) <= platmax     */
/*                                                                         */
/*                3)  The difference between the platform and database     */
/*                    minimum and maximum values is in the error message   */
/*                    written to the log file "BUS.out". If PdAt values    */
/*                    are outside the database limits the message states   */
/*                    that the PdAt record is coded incorrectly :          */
/*                                                                         */
/*                  PdAt value < dbmin  ||  dbmax < PdAt value + width     */
/*                                                                         */
/***************************************************************************/
static void get_domain_cb(rc, attr_p, platmin_p, platmax_p, dbmin_p, dbmax_p)
	int *rc;
	attribute_t *attr_p;
	unsigned long *platmin_p, *platmax_p, *dbmin_p, *dbmax_p;
{

	/* Set the amount of resource available */
	switch (attr_p->specific_resource)
	{
		case MADDR  :
			*platmin_p = MAddrBot;
			*platmax_p = MAddrTop;
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;
		case BADDR  :
			*platmin_p = BAddrBot;
			*platmax_p = BAddrTop;
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;
		case IOADDR :
			*platmin_p = IOAddrBot;
			*platmax_p = IOAddrTop;
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0x0000ffff;
			break;
		case INTLVL :
		case NSINTLVL :
			*platmin_p = 0;
			*platmax_p = 15;
			*dbmin_p   = 0;
			*dbmax_p   = 15;
			break;
		case DMALVL :
			if (__rs6k_smp_mca()) /* reserve arb 0 for early SMP */
			{
				*platmin_p = 1;
				*platmax_p = 15;
			}
			else
			{
				*platmin_p = 0;
				*platmax_p = 15;
			}
			*dbmin_p   = 0;
			*dbmax_p   = 15;
			break;
		default     :
			log_message(0, "--- INTERNAL ERROR --- RISC SYSTEM/6000 GET_DOMAIN_CB()\n");
			*rc = E_BADATTR;
			attr_p->adapter_ptr->unresolved = TRUE;
	}

	return;
} /* end of get_domain_cb() */


