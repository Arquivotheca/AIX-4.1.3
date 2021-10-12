static char sccsid[] = "@(#)01  1.18  src/bos/usr/ccs/lib/libcfg/POWER/bt_rspc.c, libcfg, bos41J, 9521A_all 5/18/95 17:27:18";
/*
 * COMPONENT_NAME: (LIBCFG) BUILD TABLES ADAPTER LIST Module
 *
 * FUNCTIONS:
 *	build_lists_rspc
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

#include <string.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include "adapter.h"
#include "bt.h"
#include "pt_extern.h"

/*------------------*/
/* Global variables */
/*------------------*/

/* Share with attributes list head defined in bt_common.c */
extern share_list_t *ShareList;         

/*----------------------*/
/* Forward declarations */
/*----------------------*/
int build_lists_rspc(char *, char *, adapter_t **, attribute_t **, int);
static void sort_adapter_list(adapter_t *);
static void build_attribute_list_rspc(int *, adapter_t *, int);
static int reserve_system_resources(adapter_t **, adapter_t *);
static void mark_resid_shared_ports(int *, adapter_t *, char *);

/* bt_common.c prototypes */
#include "bt_extern.h"

/* defined in bt_isa.c bt_mca.c bt_pci.c bt_pcmcia.c */
extern void build_attribute_list_isa(int *, adapter_t *, int);
extern void build_attribute_list_mca(int *, adapter_t *, int);
extern void build_attribute_list_pci(int *, adapter_t *, int);
extern void build_attribute_list_pcmcia(int *, adapter_t *, int);

/* defined in getindex.c */
extern void assign_resid_indices(int *, adapter_t **);

/***************************************************************************/
/*                    EXTERNALLY VISIBLE FUNCTIONS                         */
/***************************************************************************/

/***************************************************************************/
/* Function Name : BUILD_LISTS_RSPC()                                      */
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
/*                                                                         */
/***************************************************************************/
int build_lists_rspc(logname, busname, adapter_list, attribute_list, flags)
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

	/*---------------------------------------------------*/
	/* Establish the residual data index for each device */
	/*---------------------------------------------------*/
	assign_resid_indices(&rc, adapter_list);
	if (rc == E_MALLOC || rc == E_DEVACCESS || rc == E_NOATTR) 
	{
		destroy_lists(adapter_list, attribute_list);
		return rc;
	}

	/*-----------------------------------------------------*/
	/* Sort the adapter list to establish resolution order */
	/*-----------------------------------------------------*/
	sort_adapter_list(*adapter_list);

	/*------------------------------------------*/
	/* Build the attribute list for each device */
	/*------------------------------------------*/

	for (adap_p = *adapter_list ; adap_p ; adap_p = adap_p->next)
	{

		/* Build this adapter's attributes list                     */
		if (flags & COMMAND_MODE_FLAG && flags & RESOLVE_MODE_FLAG)
			build_attribute_list_rspc(&rc, adap_p, TRUE);
		else
			build_attribute_list_rspc(&rc, adap_p, FALSE); 
		if (rc == E_MALLOC || rc == E_ODMGET) 
		{
			destroy_lists(adapter_list, attribute_list);
			return rc;
		}

		/*---------------------------------------------------*/
		/* Process residual data platform reserved resources */
		/*---------------------------------------------------*/

		if (!strcmp(adap_p->logname, busname))
		{
			/* Add non-odm device's residual data attrs to top level bus */
			rc = reserve_system_resources(adapter_list, adap_p);
			if (rc == E_DEVACCESS || rc == E_MALLOC || adap_p->unresolved) 
			{
				destroy_lists(adapter_list, attribute_list);
				return rc;
			}
		}

		if (adap_p->attributes == (attribute_t *)NULL) 
			continue;	

		/*-----------------------------------------------------------------*/
		/* Link tail of last attribute list to head of this attribute list */
		/*-----------------------------------------------------------------*/

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

	/* Log residual device matching summary */
	log_message(0,"\nODM logical name to ROS residual device table index :\n");
	log_resid_summary(1, *adapter_list);

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

	/*----------------------------------------------------------*/
	/* If this is a runtime invocation of busresolve(), then    */
	/* ensure there are no residual data conflicts for residual */
	/* shared I/O ports.                                        */
	/*----------------------------------------------------------*/

	if (logname)
		mark_resid_shared_ports(&rc, *adapter_list, logname);

	return rc; 
} /* end build_lists_rspc() */

/***************************************************************************/
/* Function Name : SORT_ADAPTER_LIST()                                     */
/* Function Type : Internal C function                                     */
/* Purpose       : This function sorts the adapter list to establish       */
/*                 the order of resolution for all devices. The order      */
/*                 required by architecture is :                           */
/*                    1) ISA integrated                                    */
/*                    2) ISA pluggable in residual data                    */
/*                    3) ISA pluggable not in residual data                */
/*                    4) PCMCIA                                            */
/*                    5) PCI                                               */
/*                 This routine implements the requirement by ordering     */
/*                 the devices first by the parent_bus_type enum (see      */
/*                 adapter.h) and secondly by : integrated, resid, other.  */ 
/*                 This routine is a BUBBLE sort because it maintains      */
/*                 the original order for devices that are of the same     */
/*                 ordinal sorting value.                                  */
/* Arguments     :                                                         */
/*  adap_head      - Head of adapter list                                  */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void sort_adapter_list(adap_head)
	adapter_t *adap_head;
{
	adapter_t *adap_def, *adap_p1, *adap_p2, *temp_p;
	unsigned int ord1, ord2, swap;

	/* Find head of defined list : Note that the first device, the bus, is always AVAILABLE */
	for (adap_def = adap_head ; adap_def->next ; adap_def = adap_def->next)
		if (adap_def->next->status == DEFINED)
			break;

	if (adap_def->next == NULL) /* No DEFINED devices */
		return;

	for (swap = TRUE ; swap ; )
	{

		adap_p1 = adap_def->next;
		adap_p2 = adap_p1->next;
		swap = FALSE;

		for ( ; adap_p2 ; adap_p1 = adap_p1->next, adap_p2 = adap_p2->next)
		{

			/* Compute adap1's ordinal value for sorting */
			ord1 = adap_p1->parent_bus_type << 24;
			if (adap_p1->resid_index == NO_RESID_INDEX)
				ord1 += 0x00ff0000; /* not in residual data */
			else if (!(adap_p1->resid_dev_flags & INTEGRATED))
				ord1 += 0x0000ff00; /* not integrated */

			/* Compute adap2's ordinal value for sorting */
			ord2 = adap_p2->parent_bus_type << 24;
			if (adap_p2->resid_index == NO_RESID_INDEX)
				ord2 += 0x00ff0000; /* not in residual data */
			else if (!(adap_p2->resid_dev_flags & INTEGRATED))
				ord2 += 0x0000ff00; /* not integrated */

			if (ord1 > ord2)
			{
				/* Swap elements */
				swap = TRUE;

				/* Find previous node */
				for (temp_p = adap_head ; temp_p->next != adap_p1 ; temp_p = temp_p->next);

				/* Relink adapter list */
				temp_p->next  = adap_p2;
				adap_p1->next = adap_p2->next;
				adap_p2->next = adap_p1;

				/* Unswap our compare pointers */
				temp_p        = adap_p1;
				adap_p1       = adap_p2;
				adap_p2       = temp_p;
			}
		}
	}

	return;
} /* end of sort_adapter_list() */

/***************************************************************************/
/* Function Name : BUILD_ATTRIBUTE_LIST_RSPC()                             */
/* Function Type : Internal C function                                     */
/* Purpose       : Switch to call appropriate attribute list building      */
/*                 function based on the parent's bridge type. If the      */
/*                 device IS a bridge use it's own bridge type.            */
/* Arguments     :                                                         */ 
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - adapter_t structure pointer                             */
/*  incusr       - Bool include user modified values                       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void build_attribute_list_rspc(rc, adap_p, incusr)
	int *rc;
	adapter_t *adap_p;
	int incusr;
{
	bus_type_e bustype;

	if (adap_p->unresolved)
		return;

	/* Use parent's bridge_type unless this is a bridge */
	if (adap_p->bridge_type != NONE)
		bustype = adap_p->bridge_type;
	else 
		bustype = adap_p->parent_bus->bridge_type;

	switch (bustype)
	{
		case PCI    : 
			build_attribute_list_pci(rc, adap_p, incusr);
			break;
		case ISA    : 
			build_attribute_list_isa(rc, adap_p, incusr);
			break;
		case MCA    :
			build_attribute_list_mca(rc, adap_p, incusr);
			break;
		case PCMCIA : 
			build_attribute_list_pcmcia(rc, adap_p, incusr);
			break;
		default : 
			log_message(0, "--- INTERNAL ERROR --- BUILD_ATTRIBUTE_LIST_RSPC()\n");
			adap_p->unresolved = TRUE;
	}

	return;

} /* end of build_attribute_list_rspc() */

/***************************************************************************/
/* Function Name : RESERVE_SYSTEM_RESOURCES()                              */
/* Function Type : Internal C function                                     */
/* Purpose       : Adds an attribute for each bus resource packet in the   */
/*                 IPL ROS allocated heap data section for devices which   */
/*                 are not in the adapter list (and therefore not in the   */
/*                 ODM).                                                   */
/* Arguments     :                                                         */
/*  adapter_list - Head of the adapter list                                */
/*  adap_p       - Adapter structure to which attributes are added         */
/*                                                                         */
/* Return Value  : integer return code from cf.h                           */
/*                                                                         */
/***************************************************************************/
static int reserve_system_resources(adapter_list, adap_p)
	adapter_t **adapter_list, *adap_p;
{
	int rc = E_OK;
	adapter_t *adap_p2;
	CFG_DEVICE *devices;
	unsigned long numdev, found;
	char pid_index_str[ATTRNAMESIZE + 1];

	/* Get the device table */
	rc = get_resid_dev(&numdev, &devices);
	if (rc == E_DEVACCESS || rc == E_MALLOC) /* fatal errors */
		return rc;

	/*-----------------------------------------------------------------*/
	/* Loop through the device table finding devices in adapter list.  */
	/* If the device is not found, add its bus resources to the adap_p */
	/* adapter struct's attribute list.                                */
	/*-----------------------------------------------------------------*/
	for ( ; numdev-- ; ) 
	{

		found = FALSE;
		for (adap_p2 = *adapter_list ; adap_p2 && !found ; adap_p2 = adap_p2->next)
			if (numdev == adap_p2->resid_index) 
				found = TRUE;

		if (!found)
		{
			sprintf(pid_index_str, "%s%s%d%s", devices[numdev].pnpid, "_", numdev, "_");
			rc = build_resid_attribute_list(numdev, adap_p, pid_index_str);
		}

		if (rc == E_DEVACCESS || rc == E_MALLOC || adap_p->unresolved)
			break;
	}

	free(devices);
	return rc;
} /* end of reserve_system_resources() */

/***************************************************************************/
/* Function Name : MARK_RESID_SHARED_PORTS()                               */
/* Function Type : Internal C function                                     */
/* Purpose       : Prevents conflicts from occurring due to residual data  */
/*                 shared I/O ports.                                       */
/* Arguments     :                                                         */
/*  rc             - Pointer to int return code from cf.h                  */
/*  adap_head      - Head of adapter list                                  */
/*  logname        - Device logical name for runtime busresolve            */
/*                                                                         */
/* Return Value  : None                                                    */
/* Note          : This routine is called on resolve of single device, and */
/*                 any error code is a fatal error, so we allow this       */
/*                 routine to write over the value stored in rc.           */
/*                                                                         */
/***************************************************************************/
static void mark_resid_shared_ports(rc, adap_head, logname)
	int *rc;
	adapter_t *adap_head;
	char *logname;
{
	adapter_t *adap_p;
	attribute_t *attr_p;
	int num;
	CFG_iopack_t *iop;

	/* Find the logname device */
	for (adap_p = adap_head ; adap_p ; adap_p = adap_p->next)
		if (!strcmp(logname, adap_p->logname))
			break;

	if (!adap_p)
		return;

	/* If this device is in resid data, eliminate any resid I/O port conflicts */
	if (adap_p->resid_index != NO_RESID_INDEX)
	{

		/* Get resid I/O packets */
		*rc = get_io_packets(adap_p->resid_index, 'a', &num, &iop);

		/* Loop through resid I/O packets, non-ODM resid data looking for conflict */
		for ( ; num-- ; )
			for (attr_p = adap_head->attributes ; attr_p ; attr_p = attr_p->next) 
			{

				/* bus0's attributes are all non-odm resid data */
				if (attr_p->adapter_ptr != adap_head)
					break; /* from inner loop */

				/* Looking only for I/O ports */
				if (attr_p->resource != IOADDR)
					continue;

				/* If a resid conflict is found, mark bus0's resid attr as ignored */
				if (attr_p->current == iop[num].min)
					attr_p->ignore = TRUE;
			}
	}

	return;
} /* end of mark_resid_shared_ports() */
