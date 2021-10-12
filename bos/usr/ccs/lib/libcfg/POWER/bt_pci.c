static char sccsid[] = "@(#)27  1.6  src/bos/usr/ccs/lib/libcfg/POWER/bt_pci.c, libcfg, bos41J, 9520B_all 5/19/95 13:00:39";
/*
 * COMPONENT_NAME: (LIBCFG) BUILD ATTRIBUTE LIST PCI Module
 *
 * FUNCTIONS:
 *	build_attribute_list_pci
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <fcntl.h>         /* for mdd ioctl */
#include <sys/mdio.h>      /* for mdd ioctl */

#include "adapter.h"
#include "bt.h"
#include "pt_extern.h"

#define MAXISAMEM 0x00ffffff
#define MAXISAIO  0x0000ffff

/*------------------*/
/* Global variables */
/*------------------*/
extern adapter_t *AdapterList;

/*----------------------*/
/* Forward declarations */
/*----------------------*/
void build_attribute_list_isa(int *, adapter_t *, int);
static void build_attr_list_odm(int *, adapter_t *, int);
static unsigned char get_pci_interrupt_pin(adapter_t *);
static void get_domain_cb(int *, attribute_t *, unsigned long *, 
                          unsigned long *, unsigned long *, unsigned long *);
static void determine_pci_domain(int *, attribute_t *, unsigned long,
                                 unsigned long *, unsigned long *);
static void reserve_for_bridges(int *, adapter_t *);

#include "bt_extern.h"

/***************************************************************************/
/* Function Name : BUILD_ATTRIBUTE_LIST_PCI()                              */
/* Function Type : C function                                              */
/* Purpose       : Constructs a device attributes list for the specified   */
/*                 adapter.                                                */
/* Arguments     :                                                         */ 
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - adapter_t structure pointer                             */
/*  incusr       - Bool include user modified values                       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void build_attribute_list_pci(rc, adap_p, incusr)
	int *rc;
	adapter_t *adap_p;
	int incusr;
{
	char name[ATTRNAMESIZE + 1];

	/* If this is a bridge, use residual data */
	if (adap_p->bridge_type != NONE)
	{
		sprintf(name, "%s%s%d%s", adap_p->pnpid, "_", adap_p->resid_index, "_");
		*rc = build_resid_attribute_list(adap_p->resid_index, adap_p, name);
	}
	else
		build_attr_list_odm(rc, adap_p, incusr);

	return;
} /* end of build_attribute_list_pci() */

/***************************************************************************/
/* Function Name : BUILD_ATTR_LIST_ODM()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Constructs a device attributes list for the specified   */
/*                 adapter.                                                */
/* Arguments     :                                                         */ 
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - adapter_t structure pointer                             */
/*  incusr       - Bool include user modified values                       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void build_attr_list_odm(rc, adap_p, incusr)
	int *rc;
	adapter_t *adap_p;
	int incusr;
{
	struct PdAt *pdat = (struct PdAt *)NULL;
	struct CuAt *cuat = (struct CuAt *)NULL;
	struct listinfo pdatinfo, cuatinfo;
	attribute_t *int_attr;
	unsigned long intlvl, devfunc0, uppershort;
	unsigned char intrpin;
	int numbax, numrbax, numpci, numslots;
	CFG_bax_descriptor_t *bax = NULL, *revbax = NULL;
	CFG_pci_descriptor_t *pci_desc;
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
	if (*rc == E_MALLOC || adap_p->unresolved)
	{
		odm_free_list(pdat, &pdatinfo);
		if (cuat != (struct CuAt *)NULL)
			odm_free_list(cuat, &cuatinfo);
		return;
	}

	/*-------------------------------------------------------------------*/
	/* PCI device interrupts are assigned via PCI bus bridge descriptor. */
	/* Note that GROUPed interrupt attributes aren't processed.          */ 
	/*-------------------------------------------------------------------*/

	/* Get the PCI bus bridge descriptor */
	*rc = get_pci_descriptor(adap_p->parent_bus->resid_index, 'a', &numpci, &pci_desc);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS || adap_p->unresolved)
		return;

	for (int_attr = adap_p->attributes ; numpci && int_attr ; int_attr = int_attr->next)
	{

		if (int_attr->resource != INTLVL)
			continue;

		/* Free the values list */
		if (int_attr->reprsent == LIST)
			destroyvlist(rc, int_attr->values.list.head);

		/* Convert connwhere to DevFunc for Func 0 */
		devfunc0 = (strtoul(adap_p->connwhere, (char **)NULL, 0) & 0xfffffff8); /* 7 funcs/dev */

		/* Get PCI interrupt pin value */
		intrpin = get_pci_interrupt_pin(adap_p);
	
		/*---------------------------------------------------*/
		/* Find devfunc0 in PCI bridge descriptor slot table */
		/*---------------------------------------------------*/

		numslots = pci_desc->numslots;

		for ( ; numslots-- ; )
		{
			if ((pci_desc->slotdata)[numslots].devfunc == devfunc0)
			{
				/* Found the devfunc, get interrupt level, controller, instance */
				switch (intrpin)
				{
					case 1  : 
						intlvl = (pci_desc->slotdata)[numslots].inta;
						break;
					case 2  : 
						intlvl = (pci_desc->slotdata)[numslots].intb;
						break;
					case 3  : 
						intlvl = (pci_desc->slotdata)[numslots].intc;
						break;
					case 4  : 
						intlvl = (pci_desc->slotdata)[numslots].intd;
				}
				uppershort = ((pci_desc->slotdata)[numslots].inttype << 24) + 
				             ((pci_desc->slotdata)[numslots].intctlr << 16); 
				break;
			}
		}

		/* Check to see if the devfunc was found */
		if (numslots == -1)
		{
			log_error(1, TRUE, adap_p->logname, int_attr->name,
			          "Invalid PCI connwhere value (Check ODM database)");
			free(pci_desc);
			odm_free_list(pdat, &pdatinfo);
			if (cuat != (struct CuAt *)NULL)
				odm_free_list(cuat, &cuatinfo);
			adap_p->unresolved = TRUE;
			*rc = E_BADATTR;
			return;
		}

		/* Check to see if this is a valid interrupt */
		if (intlvl == INTR_NOT_WIRED)
		{
			log_error(1, TRUE, adap_p->logname, int_attr->name,
			          "Interrupt not available in this slot at this PCI interrupt pin value");
			free(pci_desc);
			odm_free_list(pdat, &pdatinfo);
			if (cuat != (struct CuAt *)NULL)
				odm_free_list(cuat, &cuatinfo);
			adap_p->unresolved = TRUE;
			*rc = E_BUSRESOURCE;
			return;
		}

		/*-----------------------------*/
		/* Process the interrupt level */
		/*-----------------------------*/

		/* Or in the interrupt controller type and ID */
		intlvl |= uppershort;

		/* Set up new attribute value */
		setup_list_attribute(rc, adap_p, int_attr, NULL, int_attr->specific_resource, 
		                     INTR_RESOLVE(intlvl), 1, INTR_RESOLVE(intlvl), INTR_RESOLVE(intlvl));
		if (*rc == E_MALLOC)
			return;

		/*--------------------------------------------------------*/
		/* For edge triggered devices set cuat_append character.  */
		/* For all PCI devices, we know if they are edge or level */
		/* so we set edge to non-shared and leave level alone.    */
		/*                                                        */
		/*  PCI interrupt sharability table                       */
		/*                                                        */
		/*               trigger value                            */
		/*             | edge  level                              */
		/*          ---|-------------                             */
		/*  ODM type I |  N      I                                */ 
		/*  ODM type N |  N      N                                */
		/*             |                                          */
		/*                                                        */
		/*--------------------------------------------------------*/
		/* Set up for edge or level interrupt triggering */
		if (INTR_EDGE(intlvl))
		{
			int_attr->cuat_append = 'E';            /* Write ",E" in CuAt value */
			int_attr->trigger = EDGE;               /* Set triggering logic value */ 
			int_attr->specific_resource = NSINTLVL; /* Internally convert to NS */
			int_attr->changed = TRUE;               /* Assure ",E" gets in DB */
		}
		else /* level triggered */
			int_attr->trigger = LEVEL;

		/* Set changed flag so it gets in database */
		int_attr->changed = TRUE;
		/* Save the start value so we can increment */
		int_attr->start.list.ptr = int_attr->values.list.head;

		/* Save interrupt shareing algorithm */
		adap_p->share_algorithm_cb = share_algorithm_cb_234;

	}

	/* Free the PCI bus bridge descriptor */
	if (pci_desc)
		free(pci_desc);

	/* Process bus resource modifier attributes */
	process_modifier_attributes(rc,adap_p,pdat,pdatinfo.num,cuat,cuatinfo.num);
	if (*rc == E_MALLOC || adap_p->unresolved)
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
	if (*rc == E_MALLOC || *rc == E_DEVACCESS || adap_p->unresolved)
		return;

	/* Adjust address values for sibling bus bridges */
	reserve_for_bridges(rc, adap_p);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS || adap_p->unresolved)
		return;

	/* Get the bus address translation descriptors, and inverted descriptors */
	get_bax_and_invert(rc, adap_p, &numbax, &bax, &numrbax, &revbax);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS || adap_p->unresolved)
		return;

	/* Adjust for bus address translation descriptors */
	reserve_bax_range(rc, adap_p, revbax, numrbax);

	if (bax != NULL)
		free(bax);
	if (revbax != NULL)
		free(revbax);

	return;
} /* end build_attr_list_odm() */

/***************************************************************************/
/* Function Name : GET_PCI_INTERRUPT_PIN()                                 */
/* Function Type : Internal C function                                     */
/* Purpose       : Read and process PCI Interrupt Pin register.            */
/* Arguments     :                                                         */
/*  adap_p       - pointer to adapter structure                            */
/*                                                                         */
/* Return Value  : Interrupt pin register value (default to 1 for error or */
/*                 invalid value).                                         */
/*                                                                         */
/***************************************************************************/
static unsigned char get_pci_interrupt_pin(adap_p)
	adapter_t *adap_p; 
{
	int fd;
	unsigned long connwhere = strtoul(adap_p->connwhere, (char **)NULL, 0);
	unsigned char intpin;
	MACH_DD_IO mddRecord;
	char busspecialfile[50];

	/* Open device special file */
	sprintf(busspecialfile, "/dev/%s", adap_p->parent_bus->logname);
	if ((fd = open(busspecialfile, O_RDWR)) == -1)
	{
		log_message(0, "Cannot open device special file %s\n", busspecialfile);
		return 1;
	}

	/* Setup MDD struct */
	mddRecord.md_size = 1;
	mddRecord.md_sla  = connwhere; /* This is the idsel number */
	mddRecord.md_data = (unsigned char *)&intpin;
	mddRecord.md_addr = 0x0000003d;
	mddRecord.md_incr = MV_BYTE;

	/* Read the PCI config space */
	if (ioctl(fd, MIOPCFGET, &mddRecord) != 0)
	{
		log_message(0, "Failed int pin mdd ioctl PCI idsel # = %d\n", connwhere);
		close(fd);
		return 1;
	}

	close(fd);

	/* The presence of the int attr in ODM determines the requirement */
	/* for an interrupt, not the value of the interrupt pin register. */

	if (intpin == 0)	
		intpin = 1;
	if (intpin > 4)
		intpin = 1;

	return intpin;
} /* end of get_pci_interrupt_pin() */

/***************************************************************************/
/* Function Name : GET_DOMAIN_CB()                                         */
/* Function Type : Callback C function                                     */
/* Purpose       : This callback function provides the platform min/max    */ 
/*                 values and database min/max values given a pointer to   */
/*                 an attribute struct.                                    */
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h                      */
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

	/*-------------------------------------------------------------*/
	/* Set the amount of resource available for this attr's domain */
	/*-------------------------------------------------------------*/
	switch (attr_p->specific_resource)
	{
		case MADDR  :
		case BADDR  :
			determine_pci_domain(rc, attr_p, MAXISAMEM, platmin_p, platmax_p);
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;

		case IOADDR :
			determine_pci_domain(rc, attr_p, MAXISAIO, platmin_p, platmax_p);
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;

		case INTLVL :
		case NSINTLVL :
			*platmin_p = 0x00000000;
			*platmax_p = 0xffffffff;
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;

		case DMALVL :
			log_error(1, TRUE, attr_p->adapter_ptr->logname, attr_p->name,
			          "PCI device with DMA Level attribute (Check ODM database)");
			attr_p->adapter_ptr->unresolved = TRUE;
			break;

		default     :
			log_message(0, "--- INTERNAL ERROR --- PCI BUS GET_DOMAIN_CB()\n");
			attr_p->adapter_ptr->unresolved = TRUE;
	}

	return;
} /* end of get_domain_cb() */

/***************************************************************************/
/* Function Name : DETERMINE_PCI_DOMAIN()                                  */
/* Function Type : Internal C function                                     */
/* Purpose       : Determine attribute's domain space, above or below the  */
/*                 boundry value, and return the min and max domain values */
/*                 according to the bus address translation descriptors.   */
/*                 This function determines if a PCI device's resource     */
/*                 must be located in ISA resource domain space.           */
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h (reserved)           */
/*  attr_p         - Pointer to attribute struct                           */
/*  ulimit         - Upper limit used to determine attribute's domain      */
/*  min_p          - Returned minimum value in domain                      */
/*  max_p          - Returned maximum value in domain                      */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void determine_pci_domain(rc, attr_p, boundry, min_p, max_p)
	int *rc;
	attribute_t *attr_p;
	unsigned long boundry, *min_p, *max_p;
{
	adapter_t *bus_p = attr_p->adapter_ptr->parent_bus; 
	CFG_bax_descriptor_t *bax = NULL, *bax_p;
	int num, lo_avail, lo_space, hi_avail;
	unsigned long lo_min, lo_max, hi_min, hi_max;

	/*----------------------------------------------------*/
	/* Get bus address translation descriptors and find   */
	/* the parent bus's upper and lower limits for bus    */
	/* I/O or bus memory above and below the boundry      */
	/*----------------------------------------------------*/

	*rc = get_bax_descriptor(bus_p->resid_index, 'a', &num, &bax);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS) /* Fatal errors */
		return;

	lo_min   = hi_min = 0xffffffff;
	lo_max   = hi_max = 0x00000000;
	lo_avail = FALSE;
	hi_avail = FALSE;

	for (bax_p = bax ; num-- ; bax_p++)
	{
		if ((bax_p->conv != 1 /* ! bus mem */ && attr_p->resource == ADDR)  ||
		    (bax_p->conv != 2 /* ! bus I/O */ && attr_p->resource == IOADDR)  )	
			continue;

		if (bax_p->max > boundry)
		{
			hi_avail = TRUE;
			if (hi_min > bax_p->min)
				hi_min = bax_p->min;
			if (hi_max < bax_p->max)
				hi_max = bax_p->max;
		}
		else
		{
			lo_avail = TRUE;
			if (lo_min > bax_p->min)
				lo_min = bax_p->min;
			if (lo_max < bax_p->max)
				lo_max = bax_p->max;
		}
	}

	/*--------------------------------------------------------------------*/
	/* Enforce the seperation of PCI/ISA spaces at the given boundry and  */
	/* allow a single descriptor to describe BOTH the low and high spaces */
	/*--------------------------------------------------------------------*/
	if (hi_avail == TRUE && lo_avail == FALSE && hi_min <= boundry)
	{
		/* Single descriptor spans the boundry, split it up */
		lo_avail = TRUE;
		lo_min   = hi_min;
		lo_max   = boundry;
		hi_min   = boundry + 1;
	}
	else
	{
		/* Two or more descriptors, but insure no crossing the boundry */ 
		if (hi_min <= boundry)
			hi_min = boundry + 1;
		if (lo_max > boundry)
			lo_max = boundry;
	}

	/*-------------------------------------------------*/
	/* Now determine how to set the attribute's domain */ 
	/*-------------------------------------------------*/

	if (attr_p->user)
	{
		/* Figure out if current lies in ISA space */
		if (attr_p->current > boundry)
			*min_p = hi_min, *max_p = hi_max;
		else
			*min_p = lo_min, *min_p = lo_max;
	}
	else /* not user settable */
	{
		/*---------------------------------------------------*/
		/* Eliminate possible values in lo resource domain   */
		/* unless all possible values are within that domain */
		/*---------------------------------------------------*/

		lo_space = FALSE;

		if (!hi_avail)
			lo_space = TRUE; /* Must use ISA/PCI lower range */

		else if (lo_avail)
		{
			lo_space = TRUE;
			switch (attr_p->reprsent)
			{
				case RANGE : 
				{
					value_range_t *val_p = attr_p->values.range.head;

					for ( ; val_p ; val_p = val_p->next )
						if ((val_p->upper - attr_p->width - 1) > boundry)
							break;

					if (val_p)
						lo_space = FALSE;
				}
					break;
				case LIST  :
				{
					value_list_t *val_p = attr_p->values.list.head;

					for ( ; val_p ; val_p = val_p->next )
						if (val_p->value > boundry)
							break;

					if (val_p)
						lo_space = FALSE;
				}
			}
		}

		/* Now set non-user domain used to validate values */
		if (lo_space)
			*min_p = lo_min, *max_p = lo_max;
		else
			*min_p = hi_min, *max_p = hi_max;
	}

	return;	
} /* end of determine_pci_domain() */

/***************************************************************************/
/* Function Name : RESERVE_FOR_BRIDGES()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : This function adjusts bus memory and I/O attributes to  */
/*                 accomodate the ranges used by peer PCI bus extenders.   */
/* Global Vars   :                                                         */
/*  AdapterList    - Head of adapter list                                  */ 
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h                      */
/*  adap_p         - Pointer to adapter to be adjusted                     */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void reserve_for_bridges(rc, adap_p)
	int *rc;
	adapter_t *adap_p;
{
	adapter_t *adap_p2;
	CFG_bax_descriptor_t *packets;
	int numpackets;

	/* If this device is a bus, return */
	if (!strncmp(adap_p->utype, "bus/", strlen("bus/")))
		return;

	/* Loop through the adapter list looking for sibling bridges */
	for (adap_p2 = AdapterList ; adap_p2 ; adap_p2 = adap_p2->next)
	{

		/* Must have same parent bus as adapter */
		if (adap_p2->parent_bus != adap_p->parent_bus)
			continue;

		/* Must be a bus extender */
		if (!adap_p2->bus_extender)
			continue;

		/* Must have a residual data index */
		if (adap_p2->resid_index == NO_RESID_INDEX)
			continue;

		/* Get bridge address translation descriptors for sibling bridges */
		*rc = get_bax_descriptor(adap_p2->resid_index, 'a', &numpackets, &packets);
		if (*rc == E_MALLOC || *rc == E_DEVACCESS) /* Fatal errors */
			return;

		/* Reserve ranges described, [reserve_bax_range() skips negative decode] */
		reserve_bax_range(rc, adap_p, packets, numpackets);

		/* Free the packets */
		if (packets != NULL)
			free(packets);

	}

	return;
} /* end of reserve_for_bridges() */





