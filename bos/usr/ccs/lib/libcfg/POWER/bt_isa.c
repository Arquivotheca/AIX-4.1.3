static char sccsid[] = "@(#)25  1.5  src/bos/usr/ccs/lib/libcfg/POWER/bt_isa.c, libcfg, bos41J, 9520B_all 5/19/95 13:00:37";
/*
 * COMPONENT_NAME: (LIBCFG) BUILD ATTRIBUTE LIST ISA Module
 *
 * FUNCTIONS:
 *	build_attribute_list_isa
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

/*------------------*/
/* Global variables */
/*------------------*/

/*----------------------*/
/* Forward declarations */
/*----------------------*/
void build_attribute_list_isa(int *, adapter_t *, int);
static void build_attr_list_odm(int *, adapter_t *, int);
static void get_domain_cb(int *, attribute_t *, unsigned long *, 
                          unsigned long *, unsigned long *, unsigned long *);

#include "bt_extern.h"

/***************************************************************************/
/* Function Name : BUILD_ATTRIBUTE_LIST_ISA()                              */
/* Function Type : C function                                              */
/* Purpose       : Constructs a device attributes list for the specified   */
/*                 device.                                                 */
/* Arguments     :                                                         */ 
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - adapter_t structure pointer                             */
/*  incusr       - Bool include user modified values                       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void build_attribute_list_isa(rc, adap_p, incusr)
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
} /* end of build_attribute_list_isa() */

/***************************************************************************/
/* Function Name : BUILD_ATTR_LIST_ODM()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Constructs a device attributes list for the specified   */
/*                 device from ODM PdAt / CuAt info.                       */
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
	unsigned long modified, element, uppershort;
	attribute_t *int_attr, *attr_p, *attr_grp;
	value_list_t *val_p;
	int numbax, numrbax, numisa;
	CFG_bax_descriptor_t *bax = NULL, *rbax = NULL;
	CFG_isa_descriptor_t *isa_desc = NULL;
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

	/*---------------------------------------------------------------------------------*/
	/* ISA interrupts possible values are pruned to eliminate those used by the system */
	/* This has to be done after creating the GROUPs to keep the members in sync...    */
	/*---------------------------------------------------------------------------------*/

	/* Get the ISA bus bridge descriptor */
	*rc = get_isa_descriptor(adap_p->parent_bus->resid_index, 'a', &numisa, &isa_desc);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS)
		return;

	for (attr_p = adap_p->attributes ; numisa && attr_p ; attr_p = attr_p->next)
	{

		/*-------------------------------------------------------*/
		/* Loop through attr_p (and handle GROUP) using int_attr */
		/*-------------------------------------------------------*/

		if (attr_p->resource == GROUP)
		{
			int_attr = attr_p->group_ptr;
			attr_grp = attr_p;
		}
		else
		{
			int_attr = attr_p;
			attr_grp = NULL;
		}

		for ( ; int_attr ; int_attr = int_attr->group_ptr)
		{

			if (int_attr->resource != INTLVL)
				continue;

			/* Convert to a LIST if it's a RANGE so we can eliminate specific elements */
			if (int_attr->reprsent == RANGE)
			{
				convert_to_list(rc, int_attr);
				if (*rc == E_MALLOC)
					return;
			}

			modified = FALSE;
			element = 0;
			val_p = int_attr->values.list.head;
			for ( ; val_p ; val_p = val_p->next, element++)
			{

				if ((isa_desc->irq)[val_p->value] != INTR_NOT_WIRED)
					continue;

				/* else */
					modified = TRUE;

					log_error(1, FALSE, adap_p->logname, attr_p->name,
					          "List element value not allowed on this platform");
					if (attr_grp)
						eliminate_value_list_element(attr_grp, element);
					else
						eliminate_value_list_element(int_attr, element);
	
					/* Restart the scan */
					/* (just freed the node pointed to by val_p!) */
					val_p = int_attr->values.list.head;
					element = 0;

			} /* end loop through value list values */

			/* Fail if there are no values left in the list */
			if (int_attr->values.list.head == (value_list_t *)NULL)
			{
				log_error(1, TRUE, adap_p->logname, int_attr->name,
				          "No valid values in list");
				*rc = E_BADATTR;
				adap_p->unresolved = TRUE;
				return;
			}

			/* Syncronize this attribute */
			if (modified)
			{
				sync_list_attribute(rc, attr_p, modified);
				if (*rc == E_BADATTR)
					return;
			}

			/* Or in the interrupt type and controller ids */
			uppershort = (isa_desc->inttype << 24) + (isa_desc->intctlr << 16);
			int_attr->current |= uppershort;
			int_attr->valllim |= uppershort;
			int_attr->valulim |= uppershort;
			int_attr->busllim |= uppershort;
			int_attr->busulim |= uppershort;
			val_p = int_attr->values.list.head;
			for ( ; val_p ; val_p = val_p->next)
				val_p->value |= uppershort;

			/*--------------------------------------------------------*/
			/* For level triggered devices set cuat_append character. */
			/* For resid devices, we know if they are not level then  */
			/* they are edge, convert internally to non-shared. For   */
			/* non-resid devices, do nothing.                         */
			/*                                                        */
			/*  ISA interrupt sharability table                       */
			/*                                                        */
			/*                    trigger value                       */
			/*             | edge  level  don't know                  */
			/*          ---|-------------------------                 */
			/*  ODM type I |  N      I        I                       */ 
			/*  ODM type N |  N      N        N                       */
			/*             |                                          */
			/*                                                        */
			/*--------------------------------------------------------*/
			if (int_attr->trigger == LEVEL)
				int_attr->cuat_append = 'L';
			else if (int_attr->adapter_ptr->resid_index != NO_RESID_INDEX)
				int_attr->specific_resource = NSINTLVL;

			/* Overload shareing algorithm */
			int_attr->adapter_ptr->share_algorithm_cb = share_algorithm_cb_234;

		} /* end loop through GROUP attributes */

		/* Sync all GROUP members to the first member */
		if (attr_grp)
			sync_group_attribute(attr_grp);

	} /* end loop through attributes */

	/* Free ISA bus bridge descriptor */
	if (isa_desc != NULL)
		free(isa_desc);

	/* Get the bus address translation descriptors, and inverted descriptors */
	get_bax_and_invert(rc, adap_p, &numbax, &bax, &numrbax, &rbax);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS || adap_p->unresolved)
		return;

	/* Adjust for bus address translation descriptors */
	reserve_bax_range(rc, adap_p, rbax, numrbax);

	if (bax != NULL)
		free(bax);
	if (rbax != NULL)
		free(rbax);

	return;
} /* end build_attr_list_odm() */

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
	adapter_t *bus_p = attr_p->adapter_ptr->parent_bus;
	CFG_bax_descriptor_t *bax = NULL, *bax_p;
	int num;
	unsigned long min, max;

	/*----------------------------------------------------*/
	/* Get bus address translation descriptors and find   */
	/* the parent bus's upper and lower limits for bus    */
	/* I/O or bus memory if this is an I/O or memory attr */
	/*----------------------------------------------------*/
	if (attr_p->resource == ADDR)
	{
		min = 0xffffffff;
		max = 0x00000000;
		*rc = get_bax_descriptor(bus_p->resid_index, 'a', &num, &bax);
		if (*rc == E_MALLOC || *rc == E_DEVACCESS)
			return;

		for (bax_p = bax ; num-- ; bax_p++)
		{
			if (bax_p->conv != 1 /* ! bus mem */)	
				continue;

			if (min > bax_p->min)
				min = bax_p->min;
			if (max < bax_p->max)
				max = bax_p->max;
		}
	}
	else if (attr_p->resource == IOADDR)
	{
		min = 0xffffffff;
		max = 0x00000000;
		*rc = get_bax_descriptor(bus_p->resid_index, 'a', &num, &bax);
		if (*rc == E_MALLOC || *rc == E_DEVACCESS)
			return;

		for (bax_p = bax ; num-- ; bax_p++)
		{
			if (bax_p->conv != 2 /* ! bus I/O */)	
				continue;

			if (min > bax_p->min)
				min = bax_p->min;
			if (max < bax_p->max)
				max = bax_p->max;
		}
	}

	/* Set the amount of resource available */
	switch (attr_p->specific_resource)
	{
		case MADDR  :
		case BADDR  :
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			*platmin_p = min;
			*platmax_p = max;
			break;
		case IOADDR :
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			*platmin_p = min;
			*platmax_p = max;
			break;
		case INTLVL :
		case NSINTLVL :
			*platmin_p = 0;
			*platmax_p = 15;
			*dbmin_p   = 0;
			*dbmax_p   = 15;
			break;
		case DMALVL :
			*platmin_p = 0;
			*platmax_p = 7;
			*dbmin_p   = 0;
			*dbmax_p   = 15;
			break;
		default     :
			log_message(0, "--- INTERNAL ERROR --- ISA BUS GET_DOMAIN_CB()\n");
			attr_p->adapter_ptr->unresolved = TRUE;
	}

	return;
} /* end of get_domain_cb() */


