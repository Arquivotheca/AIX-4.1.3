static char sccsid[] = "@(#)10  1.4  src/bos/usr/ccs/lib/libcfg/POWER/getindex.c, libcfg, bos41J, 9521A_all 5/22/95 08:59:25";
/*
 * COMPONENT_NAME: (LIBCFG) Get residual data device index module
 *
 * FUNCTIONS:
 *	assign_resid_indices
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <string.h>
#include <sys/types.h>
#include <cf.h>
#include <sys/cfgdb.h>     /* For UNIQUESIZE, LOCSIZE, ATTRNAMESIZE, etc. */
#include <sys/cfgodm.h>    /* For CuAt struct */

#include "adapter.h"
#include "pt_extern.h"

/*---------------------*/
/* Function prototypes */
/*---------------------*/
void assign_resid_indices(int *, adapter_t **);
static int get_top_pci_resid_index(CFG_DEVICE *, unsigned long, unsigned long,
                                   adapter_t *);
static void get_pci_resid_index(CFG_DEVICE *, unsigned long, unsigned long, 
                               unsigned long, adapter_t *);
static void get_isasio_resid_index(CFG_DEVICE *, unsigned long, adapter_t *);
static void get_isa_resid_index(int *, CFG_DEVICE *, unsigned long, adapter_t *); 

/***************************************************************************/
/* Function Name : ASSIGN_RESID_INDICES()                                  */
/* Function Type : C function                                              */
/* Purpose       : Assignes an appropriate residual device index to the    */
/*                 adapter structure for each device in the adapter list.  */
/* Arguments     :                                                         */ 
/*  rc             - return code from cf.h                                 */
/*  adapter_list   - Head of adapter list                                  */
/*                                                                         */
/* Return Value  : return code from cf.h                                   */
/*                                                                         */
/***************************************************************************/
void assign_resid_indices(rc, adapter_list)
	int *rc;
	adapter_t **adapter_list;
{
	adapter_t *adap_p;
	CFG_DEVICE *dev_p;
	unsigned long numdev;

	*rc = get_resid_dev(&numdev, &dev_p);
	if (*rc == E_DEVACCESS || *rc == E_MALLOC) /* fatal errors */
		return;

	for (adap_p = *adapter_list ; adap_p ; adap_p = adap_p->next)
	{
	
		adap_p->resid_index = NO_RESID_INDEX;

		/*---------------------------------------------------------*/
		/* Attrmpt to match this device with a residual data index */
		/*---------------------------------------------------------*/

		/* PCI devices */
		if (adap_p->parent_bus_type == PCI)
		{
			unsigned long busnum, devfunc;

			/* Top level PCI Bus bridge */
			if (!strncmp(adap_p->utype, "bus/sys", strlen("bus/sys")))
			{
				busnum = adap_p->bus_number;

				if (busnum == NO_BUS_NUMBER)
				{
					log_message(0, "No bus_number attribute to identify top level PCI bus\n");
					*rc = E_NOATTR;
					return;
				}

				*rc = get_top_pci_resid_index(dev_p, numdev, busnum, adap_p);
				if (*rc == E_DEVACCESS || *rc == E_MALLOC) /* fatal errors */
					return;
				if (*rc == -1)
				{
					log_message(0, "No residual data device entry for top level PCI bus\n");
					*rc = E_NOATTR; /* ??? Will find more appropriate return code in future... */
					return;
				}
			}

			else /* PCI device or PCI - (other) bus bridge */
			{
				busnum = adap_p->parent_bus->bus_number;
				
				if (busnum == NO_BUS_NUMBER)
				{
					log_error(1, TRUE, adap_p->parent_bus->logname, "bus_number", 
					          "No bus number attribute");	
					adap_p->unresolved = TRUE;
					*rc = E_BADATTR;
					continue;
				}
	
				devfunc = strtoul(adap_p->connwhere, (char **)NULL, 0);
		
				get_pci_resid_index(dev_p, numdev, busnum, devfunc, adap_p);
			}
		}

		/* ISA devices */
		else if (adap_p->parent_bus_type == ISA)
		{

			/* Integrated devices */
			if (strstr(adap_p->utype, "/isa_sio/"))
				get_isasio_resid_index(dev_p, numdev, adap_p);

			/* non - integrated devices */
			else /* strstr(adap_p->utype, "/isa/") */
			{
				get_isa_resid_index(rc, dev_p, numdev, adap_p);
				if (*rc == E_DEVACCESS || *rc == E_MALLOC)
					return;
			}
		}

		/*------------------------------------*/
		/* Assign the resid_index field value */
		/*------------------------------------*/

		/* If no resid index, proceed to next device */
		if (adap_p->resid_index == NO_RESID_INDEX)
			continue;

		/*--------------------------------------*/
		/* Gather residual data for this device */
		/*--------------------------------------*/

		/* Set resid_flags field */
		adap_p->resid_dev_flags = dev_p[adap_p->resid_index].deviceid.flags;

		/* Set bridge_type field */
		if (dev_p[adap_p->resid_index].deviceid.basetype == Bridge_Controller)
		{
			switch (dev_p[adap_p->resid_index].deviceid.subtype)
			{
				case ISA_Bridge    : 
				case EISA_Bridge   :
					adap_p->bridge_type = ISA;
					break;
				case MCA_Bridge    : 
					adap_p->bridge_type = MCA;
					break;
				case PCI_Bridge    : 
					adap_p->bridge_type = PCI;
					break;
				case PCMCIA_Bridge : 
					adap_p->bridge_type = PCMCIA;
					break;
				default            :
					/* Mark this bridge unresolved */
					log_message(0, "--- INTERNAL ERROR --- SET_BRIDGE_TYPE()\n");
					adap_p->unresolved = TRUE;
			}
		}

		/* Get the PNP ID string */
		strncpy(adap_p->pnpid, dev_p[adap_p->resid_index].pnpid, PNPIDSIZE);

	} /* end for() looping through adapter list */

	free(dev_p);
	return;
} /* end of assign_resid_indices() */ 

/***************************************************************************/
/* Function Name : GET_TOP_PCI_RESID_INDEX()                               */
/* Function Type : Internal C function                                     */
/* Purpose       : Finds the FIRST PCI top level bus bridge in the         */
/*                 CFG_DEVICE array has busnum matching the BusNumber      */
/*                 in the PCI Bridge Descriptor. An index into the         */
/*                 CFG_DEVICE array is returned in the index arg.          */
/* Arguments     :                                                         */ 
/*  devices    - The CFG_DEVICE array                                      */
/*  num_dev    - Number of devices in the CFG_DEVICE array                 */
/*  busnum     - PCI bus number                                            */
/*  adap_p     - Adapter structure pointer                                 */
/*                                                                         */
/* Return Value  : Return code from cf.h or -1 if not found                */
/*                                                                         */
/***************************************************************************/
static int get_top_pci_resid_index(devices, num_dev, busnum, adap_p)
	CFG_DEVICE *devices;
	unsigned long num_dev, busnum;
	adapter_t *adap_p;
{
	int rc = E_OK;
	int numpackets, i;
	CFG_pci_descriptor_t *descrip;

	for (i = 0 ; i < num_dev ; i++)
	{
		if (devices[i].deviceid.busid    == PROCESSOR_DEVICE  && 
		    devices[i].deviceid.basetype == Bridge_Controller &&
		    devices[i].deviceid.subtype  == PCI_Bridge           )
		{
			rc = get_pci_descriptor(i, 'a', &numpackets, &descrip);
			if (rc == E_DEVACCESS || rc == E_MALLOC) /* fatal error */
			{
				log_message(0, "get_top_pci_resid_index() : get_pci_descriptor() failed\n");
				return rc;
			}

			if (numpackets == 0 || rc != E_OK)
			{
				log_message(0, "get_top_pci_resid_index() : Missing PCI bridge descriptor\n");
				continue;
			}

			if (descrip->busnum == busnum)
			{
				adap_p->resid_index = i;
				free(descrip);
				return rc;
			}

			free(descrip);
		}
	}

	log_message(0, "get_top_pci_resid_index() : Residual data index not found\n");
	return -1; 
} /* end of get_top_pci_resid_index() */

/***************************************************************************/
/* Function Name : GET_PCI_RESID_INDEX()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Finds the FIRST PCI device in the CFG_DEVICE array      */
/*                 which matches the bus access field criteria supplied    */
/*                 by busnum and devfuncnum. An index into the CFG_DEVICE  */
/*                 array is returned in the index arg.                     */
/* Arguments     :                                                         */ 
/*  devices    - The CFG_DEVICE array                                      */
/*  num_dev    - Number of devices in the CFG_DEVICE array                 */
/*  busnum     - PCI bus number                                            */
/*  devfuncnum - PCI Device/Function number                                */
/*  adap_p     - Adapter structure pointer                                 */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void get_pci_resid_index(devices, num_dev, busnum, devfuncnum, adap_p)
	CFG_DEVICE *devices;
	unsigned long num_dev, busnum, devfuncnum;
	adapter_t *adap_p;
{
	unsigned long i;

	/* Find first index matching the bus access criteria */
	for (i = 0 ; i < num_dev ; i++)
	{
		if (PCI_DEVICE  == devices[i].deviceid.busid                    &&
		    busnum      == devices[i].busaccess.pciaccess.busnumber     &&
		    devfuncnum  == devices[i].busaccess.pciaccess.devfuncnumber    )
		{
			adap_p->resid_index = i;
			return;
		}
	}	

	return;
} /* end of get_pci_resid_index() */

/***************************************************************************/
/* Function Name : GET_ISASIO_RESID_INDEX()                                */
/* Function Type : Internal C function                                     */
/* Purpose       : Finds the FIRST ISA device in the CFG_DEVICE array      */
/*                 which matches the PNPID and serial number specified by  */
/*                 the connwhere string. An index into the CFG_DEVICE      */
/*                 array is returned in the index arg.                     */
/*                 This function is intended to be use for integrated ISA  */
/*                 devices.                                                */                   
/* Arguments     :                                                         */ 
/*  devices    - The CFG_DEVICE array                                      */
/*  num_dev    - Number of devices in the CFG_DEVICE array                 */
/*  adap_p     - Adapter structure pointer                                 */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void get_isasio_resid_index(devices, num_dev, adap_p)
	CFG_DEVICE *devices;
	unsigned long num_dev;
	adapter_t *adap_p;
{
	unsigned long i;
	char catstr[PNPIDSIZE + 8 + 1]; /* PNPID string + serial number + null */

	/* Loop through the residal data device table */ 
	for (i = 0 ; i < num_dev ; i++)
	{

		/* Residual device must be integrated */
		if (!(devices[i].deviceid.flags & INTEGRATED))
			continue;
			
		/* Residual device must be an ISA device */
		if (ISA_DEVICE     != devices[i].deviceid.busid &&
		    EISA_DEVICE    != devices[i].deviceid.busid &&
		    PNP_ISA_DEVICE != devices[i].deviceid.busid    )
			continue;

		/* Concatenate the residual dev's pnpid and serialnum */
		sprintf(catstr, "%s%x", devices[i].pnpid, devices[i].deviceid.serialnum);

		/* Compare residual dev's pnpid/serialnum to adapter's connwhere */
		if (!strcmp(adap_p->connwhere, catstr))
		{
			/* Matched residual device to ODM device */
			adap_p->resid_index = i;
			return;
		}
	}	

	return;
} /* end of get_isasio_resid_index() */

/***************************************************************************/
/* Function Name : GET_ISA_RESID_INDEX()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Finds the FIRST ISA device in the CFG_DEVICE array      */
/*                 whose FIRST I/O address matches an I/O address value    */
/*                 in the ODM AND whose PNP ID matches the devid for the   */
/*                 device represented by the adapter structure.            */
/*                 This function is intended to be used for non-integrated */
/*                 ISA devices.                                            */
/* Arguments     :                                                         */ 
/*  rc         - pointer to int return code from cf.h                      */
/*  devices    - The CFG_DEVICE array                                      */
/*  num_dev    - Number of devices in the CFG_DEVICE array                 */
/*  adap_p     - Adapter structure pointer                                 */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void get_isa_resid_index(rc, devices, num_dev, adap_p)
	int *rc;
	CFG_DEVICE *devices;
	unsigned long num_dev;
	adapter_t *adap_p;
{
	unsigned long i;
	int how_many, localrc, num, j;
	struct CuAt *cuat;
	CFG_iopack_t *iop;

	/* Get I/O addresses for this adapter */
	cuat = getattr(adap_p->logname, NULL, TRUE, &how_many);
	if (cuat == (struct CuAt *)NULL || how_many == 0)
		return;

	/* Loop through the residal data device table */ 
	for (i = 0 ; i < num_dev ; i++)
	{

		/* Residual device must be an ISA device */
		if (ISA_DEVICE     != devices[i].deviceid.busid &&
		    EISA_DEVICE    != devices[i].deviceid.busid &&
		    PNP_ISA_DEVICE != devices[i].deviceid.busid    )
			continue;

		/* Residual device must be non-integrated */
		if (devices[i].deviceid.flags & INTEGRATED)
			continue;
			
		/* Residual device pnpid must match devid */
		if (strcmp(adap_p->devid, devices[i].pnpid))
			continue;

		/* Get I/O packets for this residual device */
		localrc = get_io_packets(i, 'a', &num, &iop);
		if (localrc == E_DEVACCESS || localrc == E_MALLOC)
		{
			*rc = localrc;
			free(cuat);
			return;
		}
		if (num == 0)
		{
			free(cuat);
			return;
		}

		/* Search for a matching ODM I/O attribute */ 
		for (j = how_many ; j-- ; )
		{

			/* Attribute must be type I/O */
			if (cuat[j].type[0] != 'O')
				continue;

			/* Compare ODM value to first I/O packet */
			if (strtoul(cuat[j].value, (char **)NULL, 0) == iop->min)
			{
				/* Matched residual device to ODM device */
				adap_p->resid_index = i;
				free(cuat);
				free(iop);
				return;
			}
		}
	}	

	free(iop);
	free(cuat);
	return;
} /* end of get_isa_resid_index() */

