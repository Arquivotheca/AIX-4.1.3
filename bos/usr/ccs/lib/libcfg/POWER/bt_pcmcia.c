static char sccsid[] = "@(#)28  1.4  src/bos/usr/ccs/lib/libcfg/POWER/bt_pcmcia.c, libcfg, bos41J, 9520B_all 5/19/95 13:00:42";
/*
 * COMPONENT_NAME: (LIBCFG) BUILD ATTRIBUTE LIST PCMCIA Module
 *
 * FUNCTIONS:
 *	build_attribute_list_pcmcia
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

/* Allowed PCMCIA interrupts */
/* 0 = system timer                  */
/* 2 = interrupt controller cascade  */
/* 8 = real-time clock               */
/* 11 = Power management             */
/* 13 = integrated NCR810 scsi       */
/* 15 = level, d_init w/o pgm'ble edge/level parm */ 
char PcmciaInterruptTable[] = 
{
	/* 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 */
	   0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0,
};

/* Low and high values for PCMCIA I/O */
unsigned long PcmciaIoTable[] =
{
	0x00000000,
	0x0000ffff,
};

/* Low and high values for PCMCIA Bus memory */
unsigned long PcmciaMemTable[] = 
{
	0x00000000,
	0x00ffffff,
};


/*----------------------*/
/* Forward declarations */
/*----------------------*/
void build_attribute_list_isa(int *, adapter_t *, int);
static void get_domain_cb(int *, attribute_t *, unsigned long *, 
                          unsigned long *, unsigned long *, unsigned long *);

#include "bt_extern.h"

/***************************************************************************/
/* Function Name : BUILD_ATTRIBUTE_LIST_PCMCIA()                           */
/* Function Type : Internal C function                                     */
/* Purpose       : Constructs a device attributes list for ithe specified  */
/*                 device.                                                 */
/* Arguments     :                                                         */ 
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - adapter_t structure pointer                             */
/*  incusr       - Bool include user modified values                       */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void build_attribute_list_pcmcia(rc, adap_p, incusr)
	int *rc;
	adapter_t *adap_p;
	int incusr;
{
	struct PdAt *pdat = (struct PdAt *)NULL;
	struct CuAt *cuat = (struct CuAt *)NULL;
	struct listinfo pdatinfo, cuatinfo;
	attribute_t *int_attr;
	unsigned int i;
	char str[12 * 16]; /* sized for max string */
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

	/*-------------------------------------------------------------------------------*/
	/* PCMCIA bus interrupt possible values are defined by the PcmciaInterruptTable. */
	/*-------------------------------------------------------------------------------*/

	for (int_attr = adap_p->attributes ; int_attr ; int_attr = int_attr->next)
	{

		if (int_attr->resource != INTLVL)
			continue;

		/* Free the values list */
		if (int_attr->reprsent == LIST)
			destroyvlist(rc, int_attr->values.list.head);
		else if (int_attr->reprsent == RANGE)
			destroyrlist(rc, int_attr->values.range.head);

		/* Construct new list */ 
		int_attr->reprsent = LIST;
		str[0] = '\0';
		for (i = 0 ; i < 16 ; i++)
			if (PcmciaInterruptTable[i])
				/* Or in AT interrupt controller ID 0x01000000 */
				sprintf(str + strlen(str), "%lu,", 0x01000000 + i); 

		str[(strlen(str) ? strlen(str) - 1 : 0)] = '\0';
		int_attr->values.list.head = strtovlist(rc, str);
		if (*rc == E_MALLOC)
			return;

		/* Or in AT interrupt controller ID 0x01000000 into current */
		int_attr->current = int_attr->current + 0x01000000;
		sync_list_attribute(rc, int_attr, TRUE);

		/*--------------------------------------------------------*/
		/* For all PCMCIA devices, we don't know if they are edge */
		/* or level, so share based on the I and N. This means do */
		/* nothing to modify the attribute type for PCMCIA.       */
		/*                                                        */
		/*  PCMCIA interrupt sharability table                    */
		/*                                                        */
		/*               trigger value                            */
		/*             |  don't know                              */
		/*          ---|--------------                            */
		/*  ODM type I |       I                                  */ 
		/*  ODM type N |       N                                  */
		/*             |                                          */
		/*                                                        */
		/*--------------------------------------------------------*/

		/* Overload shareing algorithm */
		int_attr->adapter_ptr->share_algorithm_cb = share_algorithm_cb_234;
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

	return;
} /* end build_attribute_list_pcmcia() */

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
		case BADDR  :
			*platmin_p = PcmciaMemTable[LO];
			*platmax_p = PcmciaMemTable[HI];
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;
		case IOADDR :
			*platmin_p = PcmciaIoTable[LO];
			*platmax_p = PcmciaIoTable[HI];
			*dbmin_p   = 0x00000000;
			*dbmax_p   = 0xffffffff;
			break;
		case INTLVL :
		case NSINTLVL :
			/* Or in the AT interrupt controller ID 0x01000000 */
			*platmin_p = 0x01000000 + 0;
			*platmax_p = 0x01000000 + 15;
			*dbmin_p   = 0x01000000 + 0;
			*dbmax_p   = 0x01000000 + 15;
			break;
		default     :
			log_message(0, "--- INTERNAL ERROR --- PCMCIA BUS GET_DOMAIN_CB()\n");
			attr_p->adapter_ptr->unresolved = TRUE;
	}

	return;
} /* end of get_domain_cb() */

