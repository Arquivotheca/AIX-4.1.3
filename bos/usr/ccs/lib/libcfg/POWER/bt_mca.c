static char sccsid[] = "@(#)26  1.2  src/bos/usr/ccs/lib/libcfg/POWER/bt_mca.c, libcfg, bos41J, bai15 4/11/95 15:48:24";
/*
 * COMPONENT_NAME: (LIBCFG) BUILD ATTRIBUTE LIST MCA Module
 *
 * FUNCTIONS:
 *	build_attribute_list_mca
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

/* Allowed MCA interrupts */
/* The following interrupt controller pins aren't really connnected to the        */
/* MCA bus, but the keybd/ser/par devices prevent other devices from getting      */
/* 1, 2, 13 since they are resolved first (better be) and have only one possible  */
/* value in the database...                                                       */
/* 0  = ?????                                                                     */
/* 1  = keyboard/mouse (PdAt)                                                     */
/* 2  = serial ports (PdAt)                                                       */
/* 8  = ?????                                                                     */
/* 13 = parallel port (PdAt)                                                      */
char McaInterruptTable[] = 
{
	/* 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 */
	   0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
};

/* Allowed MCA DMA levels */
char McaDmaTable[] = 
{
	/* 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 */
	   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
};

/* Low and high values for MCA I/O */
unsigned long McaIoTable[] = 
{
	0x00000000,
	0x0000ffff,
};

/* Low and high values for MCA Mapped Bus memory */
unsigned long McaMMemTable[] = 
{
	0x00010000,
	0x00ffffff,
};

/* Low and high values for MCA Mapped Bus memory */
unsigned long McaBMemTable[] = 
{
	0x00010000,
	0xffffffff,
};

/*----------------------*/
/* Forward declarations */
/*----------------------*/
void build_attribute_list_isa(int *, adapter_t *, int);
static void get_domain_cb(int *, attribute_t *, unsigned long *, 
                          unsigned long *, unsigned long *, unsigned long *);

#include "bt_extern.h"

/***************************************************************************/
/* Function Name : BUILD_ATTRIBUTE_LIST_MCA()                              */
/* Function Type : Internal C function                                     */
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
void build_attribute_list_mca(rc, adap_p, incusr)
	int *rc;
	adapter_t *adap_p;
	int incusr;
{
	struct PdAt *pdat = (struct PdAt *)NULL;
	struct CuAt *cuat = (struct CuAt *)NULL;
	struct listinfo pdatinfo, cuatinfo;
	attribute_t *int_attr, *attr_p, *attr_grp;
	value_list_t *val_p;
	unsigned long modified, element;
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

	/*---------------------------------------------------------------------------------*/
	/* MCA interrupts possible values are pruned to eliminate those used by the system */
	/* This has to be done after creating the GROUPs to keep the members in sync...    */
	/*---------------------------------------------------------------------------------*/

	for (attr_p = adap_p->attributes ; attr_p ; attr_p = attr_p->next)
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

			if (int_attr->resource != INTLVL && int_attr->resource != DMALVL)
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

				if (attr_p->resource == INTLVL && McaInterruptTable[val_p->value])
					continue;
				else if (attr_p->resource == DMALVL && McaDmaTable[val_p->value])
					continue;

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
		} /* end loop through GROUP attributes */

		/* Sync all GROUP members to the first member */
		if (attr_grp)
			sync_group_attribute(attr_grp);

	} /* end loop through attributes */

	/* Verify attribute values and set value limits */
	verify_bus_resource_attributes(rc, adap_p, incusr, get_domain_cb);

	return;
} /* end build_attribute_list_mca() */

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
			*platmin_p = McaMMemTable[LO];
			*platmax_p = McaMMemTable[HI];
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;
		case BADDR  :
			*platmin_p = McaBMemTable[LO];
			*platmax_p = McaBMemTable[HI];
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;
		case IOADDR :
			*platmin_p = McaIoTable[LO];
			*platmax_p = McaIoTable[HI];
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;
		case INTLVL :
		case NSINTLVL :
		case DMALVL :
			*platmin_p = 0;
			*platmax_p = 15;
			*dbmin_p   = 0;
			*dbmax_p   = 15;
			break;
		default     :
			log_message(0, "--- INTERNAL ERROR --- MCA BUS GET_DOMAIN_CB()\n"); 
			attr_p->adapter_ptr->unresolved = TRUE;
	}

	return;
} /* end of get_domain_cb() */

