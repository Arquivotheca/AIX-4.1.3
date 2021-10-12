static char sccsid[] = "@(#)23  1.6  src/bos/usr/ccs/lib/libcfg/POWER/bt_common.c, libcfg, bos41J, 9521A_all 5/19/95 13:07:23";
/*
 * COMPONENT_NAME: (LIBCFG) BUILD ATTRIBUTE LIST Module
 *
 * FUNCTIONS:
 *  build_adapter_list
 *  new_attribute
 *  destroy_lists
 *  get_modelcode
 *  strtovlist
 *  apply_share_attributes
 *  convert_to_list
 *  destroy_attribute_list
 *  eliminate_value_list_element
 *  process_bus_resource_attributes
 *  process_modifier_attributes
 *  setup_list_attribute
 *  sync_group_attribute
 *  sync_list_attribute
 *  verify_bus_resource_attributes
 *  destroyvlist
 *  get_bax_and_invert
 *  reserve_bax_range
 *  reserve_range
 *  build_resid_attribute_list
 *  share_algorithm_cb_234
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

#include "adapter.h"
#include "bt.h"
#include "pt_extern.h"

/*------------------*/
/* Global Variables */
/*------------------*/
share_list_t *ShareList = NULL; 

/*----------------------*/
/* Forward declarations */
/*----------------------*/

adapter_t *build_adapter_list(int *, char *, char *);
attribute_t *new_attribute(int *, adapter_t *, struct PdAt *, 
                           struct CuAt *);
int destroy_lists(adapter_t **, attribute_t **);
unsigned long get_modelcode(int *);
value_list_t *strtovlist(int *, char *);
void apply_share_attributes(int *);
void convert_to_list(int *, attribute_t *);
void destroy_attribute_list(int *, attribute_t *);
void eliminate_value_list_element(attribute_t *, int);
void process_bus_resource_attributes(int *, adapter_t *, struct PdAt *, 
                                     int, struct CuAt *, int);
void process_modifier_attributes(int *, adapter_t *, struct PdAt *, int,
                                 struct CuAt *, int);
void setup_list_attribute(int *, adapter_t *, attribute_t *, char *, 
                          bus_resource_e, unsigned long, unsigned long,
                          unsigned long, unsigned long);
void sync_group_attribute(attribute_t *);
void sync_list_attribute(int *, attribute_t *, int);
void verify_bus_resource_attributes(int *, adapter_t *, int, void());
void destroyvlist(int *, value_list_t *);
int build_resid_attribute_list(unsigned long, adapter_t *, char *);

static void get_adapter_children(int *, adapter_t *, char *, char *,
                                 adapter_t *, adapter_t *);
static adapter_t *new_adapter(int *, adapter_t *, struct CuDv *, struct PdDv *, 
                              int);
static void initialize_parent_bus_type(struct PdDv *, int, adapter_t *);
static int verify_share_attributes(int *, attribute_t *, attribute_t *, share_list_t *);
static void process_share_attribute(int *, adapter_t *, struct PdAt *);
static void process_group_attribute(int *, adapter_t *, struct PdAt *);
static void process_width_attribute(int *, adapter_t *, struct PdAt *);
static void process_priority_attribute(adapter_t *, struct PdAt *);
static void get_attribute_values(int *, struct PdAt *, struct CuAt *, 
                                 attribute_t *);
static value_range_t *strtorlist(int *, char *);
static void verify_list(int *, adapter_t *, attribute_t *, attribute_t *, 
                        void());
static void verify_range(int *, adapter_t *, attribute_t *, void());
static void set_value_limits(attribute_t *, unsigned long, unsigned long);
static void destroy_adapter_list(int *, adapter_t *);
static void destroy_group_list(int *, attribute_t *);
static void freevlist(int *, value_list_t *);
static void share_algorithm_cb_common(int *, inttbl_t *, int, attribute_t *, 
                                      unsigned long *, unsigned long*);
void share_algorithm_cb_234(int *, inttbl_t *, int, attribute_t *,
                            unsigned long *, unsigned long *);
void destroyrlist(int *, value_range_t *);
void get_bax_and_invert(int *, adapter_t *, int *, CFG_bax_descriptor_t **,
                        int *, CFG_bax_descriptor_t **);
static void sort_bax_descriptors(CFG_bax_descriptor_t *, int);

void reserve_bax_range(int *, adapter_t *, CFG_bax_descriptor_t *, int);
void reserve_range(int *, attribute_t *, unsigned long, unsigned long, char *);
static void eliminate_range_list_element(attribute_t *, value_range_t *);
static void adjust_range_list_max(attribute_t *, value_range_t *, unsigned long);
static void adjust_range_list_min(attribute_t *, value_range_t *, unsigned long);
static int add_resid_attribute(void *, int, int, adapter_t *, char *);

/***************************************************************************/
/* Function Name : GET_MODELCODE()                                         */
/* Function Type : C function                                              */
/* Purpose       : Get the modelcode from ODM database                     */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*                                                                         */
/* Return Value  : Modelcode value                                         */ 
/*                                                                         */
/***************************************************************************/
unsigned long get_modelcode(rc)
	int *rc;
{
	struct CuAt *cuatrc, cuat;

	cuatrc = odm_get_first(CuAt_CLASS, "name = sys0 AND attribute = modelcode", &cuat);
	if (cuatrc == (struct CuAt *)FAIL)
	{
		log_message(0, "odm_get failed for CuAt name = %s AND attribute = %s", "sys0", "modelcode");
		*rc = E_ODMGET;
		return 0;
	}
	if (cuatrc == (struct CuAt *)NULL)
	{
		log_message(0, "No sys0 modelcode attribute to identify model type\n");
		*rc = E_NOATTR;
		return 0;
	}

	return (strtoul(cuat.value, NULL, 0));
} /* end of get_modelcode() */

/***************************************************************************/
/* Function Name : BUILD_ADAPTER_LIST()                                    */
/* Function Type : C function                                              */
/* Purpose       : Constructs an adapter list for all adapters on the bus. */
/*                 The list is ordered first by customized device status   */
/*                 (AVAILABLE adapters followed by all DEFINED adapters)   */
/*                 and second by the order of customized device records in */
/*                 customized device database. If logname is supplied,     */
/*                 then the returned list is composed of all AVAILABLE     */
/*                 devices followed by the device specified by logname.    */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  busname      - name of the bus                                         */
/*  logname      - device logical name for single device config            */
/*                                                                         */
/* Return Value  : Head of the adapter list.                               */
/*                                                                         */
/***************************************************************************/
adapter_t *build_adapter_list(rc, busname, logname)
	int *rc;
	char *busname, *logname;
{
	struct CuDv *cudvrc, cudv;
	struct PdDv *pddvrc, pddv;
	adapter_t *adapter_list, *adap_p, available_adap, defined_adap;
	char query_str[UNIQUESIZE + 15]; /* Sized for longest criteria */

	/*----------------------------------*/
	/* Get database records for the bus */
	/*----------------------------------*/

	/* Customized Device record */ 
	sprintf(query_str, "name = %s", busname);
	cudvrc = odm_get_first(CuDv_CLASS, query_str, &cudv);
	if (cudvrc == (struct CuDv *)NULL) 
	{
		log_error(1, TRUE, busname, NULL, "No customized device entry"); 
		*rc = E_NOCuDv;
		return (adapter_t *)NULL;
	}
	if (cudvrc == (struct CuDv *)FAIL) 
	{
		log_message(0, "odm_get failed for CuDv : %s\n", query_str);
		*rc = E_ODMGET;
		return (adapter_t *)NULL;
	}

	/* The bus MUST be AVAILABLE */
	if (cudv.status != AVAILABLE)
	{
		log_error(1, TRUE, cudv.name, NULL, "Bus device status must be AVAILABLE"); 
		*rc = E_PARENTSTATE;
		return (adapter_t *)NULL;
	}

	/* Predefined Device record */
	sprintf(query_str, "uniquetype = %s", cudv.PdDvLn_Lvalue);
	pddvrc = odm_get_first(PdDv_CLASS, query_str, &pddv);
	if (pddvrc == (struct PdDv *)NULL) 
	{
		log_error(1, TRUE, busname, NULL, "No predefined device entry"); 
		*rc = E_NOPdDv;
		return (adapter_t *)NULL;
	}
	if (pddvrc == (struct PdDv *)FAIL) 
	{
		log_message(0, "odm_get failed for PdDv : %s\n", query_str);
		*rc = E_ODMGET;
		return (adapter_t *)NULL;
	}

	/*----------------------------------------*/
	/* Create an adapter node for the bus, it */ 
	/* will be the head of the returned list  */ 
	/*----------------------------------------*/

	adapter_list = new_adapter(rc, NULL, &cudv, &pddv, TRUE);
	if (*rc == E_MALLOC || *rc == E_ODMGET)
		return (adapter_t *)NULL;

	/*---------------------------------------------------*/
	/* Get all the child devices in the device hierarchy */ 
	/*---------------------------------------------------*/

	/* Init local dummy anchor structs... */
	available_adap.next = NULL;
	defined_adap.next   = NULL;

	/* ...and bring in the kids! */
	get_adapter_children(rc, adapter_list, busname, logname, &available_adap, &defined_adap);
	if (*rc == E_MALLOC || *rc == E_ODMGET)
	{
		int temprc = E_OK;

		destroy_adapter_list(&temprc, available_adap.next);
		destroy_adapter_list(&temprc, defined_adap.next);
		destroy_adapter_list(&temprc, adapter_list);
		return (adapter_t *)NULL;
	}

	/* Add DEFINED adapter list to end of AVAILABLE adapter list */
	for (adap_p = &available_adap ; adap_p->next ; adap_p = adap_p->next);
	adap_p->next = defined_adap.next;

	/* Add AVAILABLE adapter list to adapter node for busname */
	adapter_list->next = available_adap.next;

	/* Return the adapter list */
	return adapter_list;

} /* end build_adapter_list() */

/***************************************************************************/
/* Function Name : GET_ADAPTER_CHILDREN()                                  */
/* Function Type : Internal C function                                     */
/* Purpose       : Adds devices to the AVAILABLE adpater list or DEFINED   */
/*                 adapter list according to their CuDv status. When a     */
/*                 bus extending device is encountered, this routine calls */
/*                 itself to process child devices of the bus extender.    */
/*                 If logname is supplied, then only the logname device    */
/*                 is added to the DEFINED adapter list, and other DEFINED */
/*                 adapters are ignored.                                   */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  parent_bus   - Pointer to parent bus adapter structure                 */
/*  parent       - logical name of parent device                           */
/*  logname      - device logical name for single device config            */
/*  avail_adap_p - AVAILABLE device list                                   */
/*  defin_adap_p - DEFINED device list                                     */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void get_adapter_children(rc, parent_bus, parent, logname, avail_adap_p, defin_adap_p)
	int *rc;
	adapter_t *parent_bus;
	char *parent, *logname;
	adapter_t *avail_adap_p, *defin_adap_p;
{
	struct CuDv *cudv = (struct CuDv *)NULL, *cudv_p;
	struct PdDv *pddvrc, pddv;
	struct listinfo	cudvinfo; 
	adapter_t *adap_p;
	char query_str[UNIQUESIZE + 15]; /* Sized for longest criteria */
	int child_count;

	/*---------------------------*/
	/* Process each child device */
	/*---------------------------*/

	/* Get CuDv records for all of parent's children */ 

	sprintf(query_str, "parent = %s AND chgstatus != %d", parent, MISSING);
	cudv_p = cudv = odm_get_list(CuDv_CLASS, query_str, &cudvinfo, 16, 0);
	if (cudv == (struct CuDv *)FAIL) 
	{
		log_message(0, "odm_get failed for CuDv : %s\n", query_str);
		*rc = E_ODMGET;
		return;
	}

	/* Loop through CuDv recs and process each child device */

	for (child_count = cudvinfo.num ; child_count-- ; cudv_p++) 
	{

		/* If this is runtime, skip DEFINED other than logname */
		if (logname && cudv_p->status == DEFINED && strcmp(logname, cudv_p->name))
			continue;

		/* Get the Predefined Device record for this child device */ 

		sprintf(query_str, "uniquetype = %s", cudv_p->PdDvLn_Lvalue);
		pddvrc = odm_get_first(PdDv_CLASS, query_str, &pddv);
		if (pddvrc == (struct PdDv *)NULL) 
		{
			log_error(1, TRUE, cudv_p->name, NULL, "No predefined device entry"); 
			*rc = E_NOPdDv;
			continue;
		}
		if (pddvrc == (struct PdDv *)FAIL) 
		{
			odm_free_list(cudv, &cudvinfo);
			log_message(0, "odm_get failed for PdDv : %s\n", query_str);
			*rc = E_ODMGET;
			return;
		}

		/*-----------------------------------------------------------------*/ 
		/* Add new node adapter data to the end of either the AVAILABLE or */ 
		/* DEFINED lists according to the value of it's CuDv status field. */
		/*-----------------------------------------------------------------*/ 
		if (cudv_p->status == DEFINED)
			adap_p = defin_adap_p;
		else /* cudv_p->status != DEFINED */
			adap_p = avail_adap_p;
		for ( ; adap_p->next ; adap_p = adap_p->next);

		adap_p->next = new_adapter(rc, parent_bus, cudv_p, &pddv, FALSE);
		if (*rc == E_MALLOC || *rc == E_ODMGET) 
		{
			odm_free_list(cudv, &cudvinfo);
			return;
		}

		/*------------------------------------------------------------*/
		/* Make a recursive call to process a bus extender's children */
		/*------------------------------------------------------------*/
		if (pddv.bus_ext) 
		{

			if (!strcmp(pddv.class, "bus"))
				get_adapter_children(rc, adap_p->next, cudv_p->name, logname, avail_adap_p, defin_adap_p);
			else
				get_adapter_children(rc, parent_bus, cudv_p->name, logname, avail_adap_p, defin_adap_p);

			if (*rc == E_MALLOC || *rc == E_ODMGET) 
			{
				odm_free_list(cudv, &cudvinfo);
				return;
			}
		}

	} /* end for() processing each child CuDv record */

	/* Free CuDv records if needed */
	if (cudv != (struct CuDv *)NULL)
		odm_free_list(cudv, &cudvinfo);

	return;
} /* end of get_adapter_children() */

/***************************************************************************/
/* Function Name : NEW_ADAPTER()                                           */
/* Function Type : Internal C function                                     */
/* Purpose       : Allocates and initializes an instance of the adapter    */
/*                 structure.                                              */
/* Arguments     :                                                         */
/*   rc          - integer return code from cf.h                           */
/*   parent_bus  - Pointer to parent bus adapter structure                 */
/*   cudv_p      - Customized Device object                                */
/*   pddv_p      - Predefined Device object                                */
/*   top_bus     - Bool top level bus                                      */ 
/*                                                                         */
/* Return Value  : Pointer to instance of a new adapter structure.         */
/*                                                                         */
/***************************************************************************/
static adapter_t *new_adapter(rc, parent_bus, cudv_p, pddv_p, top_bus)
	int *rc;
	adapter_t *parent_bus;
	struct CuDv *cudv_p;
	struct PdDv *pddv_p;
	int top_bus;
{
	adapter_t *adap_p;

	/*------------------------------*/
	/* Create new adapter structure */
	/*------------------------------*/

	adap_p = (adapter_t *)calloc(1, sizeof(adapter_t));

	if (adap_p == (adapter_t *)NULL) 
	{
		*rc = E_MALLOC;
		log_message(0, "new_adapter() : Out of virtual storage\n");
	}

	else /* Adapter structure allocated, fill it in */ 
	{
		strncpy(adap_p->utype, cudv_p->PdDvLn_Lvalue, sizeof(adap_p->utype) - 1);
		strncpy(adap_p->logname, cudv_p->name, sizeof(adap_p->logname) - 1);
		strncpy(adap_p->devid, pddv_p->devid, sizeof(adap_p->devid) - 1);
		strncpy(adap_p->parent, cudv_p->parent, sizeof(adap_p->parent) - 1);
		strncpy(adap_p->connwhere, cudv_p->connwhere, sizeof(adap_p->connwhere) - 1);
		adap_p->status             = cudv_p->status;
		adap_p->bus_extender       = pddv_p->bus_ext;
		adap_p->base_adapter       = pddv_p->base; 
		adap_p->share_algorithm_cb = share_algorithm_cb_common;
		adap_p->resid_index        = NO_RESID_INDEX; 
		adap_p->parent_bus         = parent_bus;
		adap_p->bus_number         = NO_BUS_NUMBER;

		initialize_parent_bus_type(pddv_p, top_bus, adap_p);

		/* If this device is a bus, save the value of the bus_number attribute */
		if (!strcmp(pddv_p->class, "bus"))
		{
			struct CuAt *cuat_p;
			int howmany;

			cuat_p = getattr(adap_p->logname, "bus_number", FALSE, &howmany);
			if (howmany != 0)
			{
				adap_p->bus_number = strtoul(cuat_p->value, (char **)NULL, 0);
				free(cuat_p);
			}
		}
	}

	/* Return adapter structure */
	return adap_p;

} /* end new_adapter() */

/***************************************************************************/
/* Function Name : INITIALIZE_BRIDGE_TYPE()                                */
/* Function Type : Internal C function                                     */
/* Purpose       : Initialize the parent_bus_type field in adapter structure   */
/*                 based on ODM PdDv type or subclass. This is an initial  */
/*                 value that is actually the parent's parent_bus_type. When   */
/*                 devices are matched up with residual data, the bridges  */
/*                 themselves are identified and set according to the kind */
/*                 of bridge; the other device's parent_bus_type fields are    */
/*                 cleared (set to NONE). The initial value is needed to   */
/*                 match up devices in the residual data.                  */
/* Arguments     :                                                         */
/*  pddv_p       - PdDv object for the device                              */
/*  top_bus      - Boolean is this structure the top level bus             */
/*  adap_p       - adapter_t structure pointer                             */
/*                                                                         */
/* Return Value  : None.                                                   */
/*                                                                         */
/***************************************************************************/
static void initialize_parent_bus_type(pddv_p, top_bus, adap_p)
	struct PdDv *pddv_p;
	int top_bus;
	adapter_t *adap_p;
{

	if (top_bus)
	{

		/* Determine bus type using type */

		/* ??? Changing this to use PdCn connkey in the future... */

		if (!strcmp(pddv_p->type, "pci"))
			adap_p->parent_bus_type = PCI;
		else if (!strcmp(pddv_p->type, "isa"))
			adap_p->parent_bus_type = ISA;
		else if (!strcmp(pddv_p->type, "mca"))
			adap_p->parent_bus_type = MCA;
		else if (!strcmp(pddv_p->type, "pcmcia"))
			adap_p->parent_bus_type = PCMCIA;
	}
	else /* !top_bus */
	{

		/* Determine bus type using subclass */

		if (!strcmp(pddv_p->subclass, "pci"))
			adap_p->parent_bus_type = PCI;
		else if (!strcmp(pddv_p->subclass, "isa") || !strcmp(pddv_p->subclass, "isa_sio"))
			adap_p->parent_bus_type = ISA;
		else if (!strcmp(pddv_p->subclass, "mca") || !strcmp(pddv_p->subclass, "sio"))
			adap_p->parent_bus_type = MCA;
		else if (!strcmp(pddv_p->subclass, "pcmcia"))
			adap_p->parent_bus_type = PCMCIA;
	}

	/* If we don't know the parent bus, ensure the device is not resolved */
	if (adap_p->parent_bus_type == NONE) 
	{
		log_message(0, "--- INTERNAL ERROR --- SET_BUS_TYPE()\n");
		adap_p->unresolved = TRUE;
	}

	return;

} /* end initialize_parent_bus_type() */

/***************************************************************************/
/* Function Name : APPLY_SHARE_ATTRIBUTES()                                */
/* Function Type : C function                                              */
/* Purpose       : Processes share-with attribute information saved on the */
/*                 global ShareList. Also frees storage associated with    */
/*                 the global ShareList.                                   */
/* Global Vars   :                                                         */
/*  ShareList      - List of shared with attribute information.            */
/* Arguments     :                                                         */
/*  rc             - integer return code from cf.h                         */
/*  attribute_list - pointer to head of attributes list                    */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void apply_share_attributes(rc)
	int *rc;
{
	attribute_t *attr_p1, *attr_p2, *attr_p3; 
	share_list_t *share_p1, *share_p2;
	int found_attr;

	/*--------------------------------------------*/
	/* Loop through each shared with attribute... */
	/*--------------------------------------------*/
	for (share_p1 = ShareList ; share_p1 ; share_p1 = share_p1->next)
	{

		/* Skip this node if it's already been processed */
		if (share_p1->processed) 
			continue;

		/* Find the shared with's bus resource attribute */
		found_attr = FALSE;
		for (attr_p1 = share_p1->adapter->attributes ; 
		               attr_p1 && attr_p1->adapter_ptr == share_p1->adapter ;
		                          attr_p1 = attr_p1->next                         )
		{
				if (!strcmp(attr_p1->name, share_p1->attr_name)) 
				{
					found_attr = TRUE;
					break;
				}
		}

		/* Log the error */
		if (!found_attr)
		{
			/* Did not find share_p1 / attr_p1 match */
			log_error(1, FALSE, share_p1->adapter->logname, share_p1->shar_name,
			          "Shared with attribute's bus resource attr not found (Check ODM database)");
			continue;
		}

		/*-------------------------------------------*/
		/* ...then find and process it's partner(s). */
		/*-------------------------------------------*/
		for (share_p2 = share_p1->next ; share_p2 ; share_p2 = share_p2->next)
		{

			if (!strcmp(share_p1->shar_name, share_p2->shar_name))
			{

				/* Mark this shared with node as processed */
				share_p2->processed = TRUE;

				/* Find the shared with's bus resource attribute */
				found_attr = FALSE;
				for (attr_p2 = share_p2->adapter->attributes ; 
				               attr_p2 && attr_p2->adapter_ptr == share_p2->adapter ;
				                          attr_p2 = attr_p2->next                         )
				{
						if (!strcmp(attr_p2->name, share_p2->attr_name)  &&
						    attr_p2 != attr_p1 /* prevent cicularity! */    )
						{
							found_attr = TRUE;
							break;
						}
				}

				/* Log the error */
				if (!found_attr)
				{
					/* Did not find share_p2 / attr_p2 match */
					log_error(1, FALSE, share_p2->adapter->logname, share_p2->shar_name,
					          "Shared with attribute's bus resource attr not found (Check ODM database)");
					continue;
				}

				/* Verify the shared attributes */
				if (!verify_share_attributes(rc, attr_p1, attr_p2, share_p2))
					continue;

				/* Mark the head of this share list */
				attr_p1->share_head = TRUE;

				/* Link the bus resource attribute into the share_ptr list */
				if (attr_p1->share_ptr == NULL)
					attr_p1->share_ptr = attr_p2;
				else
				{
					attr_p3 = attr_p1->share_ptr;
					for (attr_p3 = attr_p1->share_ptr ;
					          attr_p3->share_ptr ; 
					               attr_p3 = attr_p3->share_ptr);
					attr_p3->share_ptr = attr_p2;
				}

				/* Set the ignore flag so subsequent share with attributes */
				/* are not processed by resolve_lists().                   */ 
				attr_p2->ignore = TRUE;
			}

		} /* end for() loop finding and processing partners */ 

		/* No error logged for share_p1 / share_p2 match not found, just ignore */

	} /* end for() looping through each shared with attribute */

	/*----------------------------*/
	/* Free the global share list */
	/*----------------------------*/
	for (share_p1 = ShareList ; share_p1 ; )
	{
		share_p2 = share_p1->next;
		free(share_p1);
		share_p1 = share_p2;
	}
	ShareList = NULL;

	return;
} /* end of apply_share_attributes() */

/***************************************************************************/
/* Function Name : VERIFY_SHARE_ATTRIBUTES()                               */
/* Function Type : Internal C function                                     */
/* Purpose       : Check to see that two attributes may be shared.         */  
/* Arguments     :                                                         */
/*  rc             - integer return code from cf.h                         */
/*  attr_p1        - first bus resource attribute (head of shared list)    */
/*  attr_p2        - second bus resource attribute (subsequent attribute)  */
/*  share_p2       - entry in the global ShareList associated with attr_p2 */
/*                                                                         */
/* Return Value  : Boolean TRUE if both attributes have the same resource, */
/*                 representation, and values; Also we cannot share types  */
/*                 A, N, or I where the priority class values differ.      */ 
/* Notes         :                                                         */
/*                 1) If the subsequent attribute (attr_p2) does not       */
/*                    match resource, representation, and values for the   */
/*                    head attribute (attr_p1) then its adapter is marked  */
/*                    unresolved.                                          */
/*                 2) If an attempt is made to share non-shareable resource*/
/*                    we return FALSE, but do not mark either the adapter  */
/*                    unresolved.                                          */
/*                                                                         */
/***************************************************************************/
static int verify_share_attributes(rc, attr_p1, attr_p2, share_p2)
	int *rc;
	attribute_t *attr_p1, *attr_p2;
	share_list_t *share_p2;
{
	
	/*-------------------------*/
	/* Must have same resource */
	/*-------------------------*/
	if (attr_p1->specific_resource != attr_p2->specific_resource)
	{
		log_error(1, TRUE, share_p2->adapter->logname, share_p2->shar_name, 
		          "Shared with bus resource types do not match (Check ODM database)");
		attr_p2->adapter_ptr->unresolved = TRUE;
		*rc = E_BADATTR;
		return FALSE;
	}

	/*-------------------------------*/
	/* Must have same representation */
	/*-------------------------------*/
	if (attr_p1->reprsent != attr_p2->reprsent)
	{
		log_error(1, TRUE, share_p2->adapter->logname, share_p2->shar_name, 
		          "Shared with values do not match (Check ODM database)");
		attr_p2->adapter_ptr->unresolved = TRUE;
		*rc = E_BADATTR;
		return FALSE;
	}

	/*--------------------------------*/
	/* Must have same possible values */
	/*--------------------------------*/
	if (attr_p1->reprsent == LIST)
	{
		value_list_t *val_p1 = attr_p1->values.list.head,
		             *val_p2 = attr_p2->values.list.head;

		for ( ; val_p1 && val_p2 ; val_p1 = val_p1->next, val_p2 = val_p2->next)
		{
			if (val_p1->value != val_p2->value)
			{
				log_error(1, TRUE, share_p2->adapter->logname, share_p2->shar_name, 
				          "Shared with values do not match (Check ODM database)");
				attr_p2->adapter_ptr->unresolved = TRUE;
				*rc = E_BADATTR;
				return FALSE;
			}
		}
		if (val_p1 || val_p2)
		{
			log_error(1, TRUE, share_p2->adapter->logname, share_p2->shar_name, 
			          "Shared with values do not match (Check ODM database)");
			attr_p2->adapter_ptr->unresolved = TRUE;
			*rc = E_BADATTR;
			return FALSE;
		}
	}
	else /* attr_p1->reprsent == RANGE */
	{
		value_range_t *val_p1 = attr_p1->values.range.head,
		              *val_p2 = attr_p2->values.range.head;

		for ( ; val_p1 && val_p2 ; val_p1 = val_p1->next, val_p2 = val_p2->next)
		{

			if (val_p1->lower != val_p2->lower ||
			    val_p1->upper != val_p2->upper ||
			    val_p1->incr  != val_p2->incr     )
			{
				log_error(1, TRUE, share_p2->adapter->logname, share_p2->shar_name, 
				          "Shared with values do not match (Check ODM database)");
				attr_p2->adapter_ptr->unresolved = TRUE;
				*rc = E_BADATTR;
				return FALSE;
			}
		}
		if (val_p1 || val_p2)
		{
			log_error(1, TRUE, share_p2->adapter->logname, share_p2->shar_name, 
			          "Shared with values do not match (Check ODM database)");
			attr_p2->adapter_ptr->unresolved = TRUE;
			*rc = E_BADATTR;
			return FALSE;
		}
	}

	return TRUE;
} /* end of verify_share_attributes() */

/***************************************************************************/
/* Function Name : PROCESS_BUS_RESOURCE_ATTRIBUTES()                       */
/* Function Type : Internal C function                                     */
/* Purpose       : Processes a bus resource attribute : MADDR, BADDR,      */
/*                 IOADDR, INTLVL, NSINTLVL, DMALVL.                       */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - pointer to adapter structure                            */
/*  pdat         - array of predefined attributes                          */
/*  pnum         - number of predefined attribute objects                  */
/*  cuat         - array of customized attributes                          */
/*  cnum         - number of customized attribute objects                  */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void process_bus_resource_attributes(rc, adap_p, pdat, pnum, cuat, cnum)
	int *rc;
	adapter_t *adap_p;
	struct PdAt *pdat;
	int pnum;
	struct CuAt *cuat;
	int cnum;
{
	struct PdAt *pdat_p;
	struct CuAt *cuat_p;
	int ccnt;

	/*===================================================================*/
	/* Loop through the predefined attributes and create attribute list. */
	/* Ignore SHARE, GROUP, WIDTH, and PRIORITY attributes at this time. */
	/*===================================================================*/

	for (pdat_p = pdat ; pnum-- ; pdat_p++)
	{

		/*--------------------------------------------------------*/
		/* Make sure we are working with a bus resource attribute */
		/*--------------------------------------------------------*/
		switch ((int)pdat_p->type[0])
		{
			case TMADDR    :
			case TBADDR    :
			case TIOADDR   :
			case TNSINTLVL :
			case TINTLVL   :
			case TDMALVL   :
				break;
			default        :
				continue;
		}

		/*--------------------------------------------*/
		/* Find CuAt record matching this PdAt record */
		/*--------------------------------------------*/
		for (ccnt = cnum, cuat_p = cuat ; ccnt-- ; cuat_p++)
		{
			if (!strcmp(pdat_p->attribute, cuat_p->attribute) &&
			    !strcmp(pdat_p->type, cuat_p->type)              )
				break;
		}
		cuat_p = (ccnt < 0) ? (struct CuAt *)NULL : cuat_p;
 
		/*------------------------------------------------------*/
		/* Allocate an attribute node and link it into the list */
		/*------------------------------------------------------*/

		new_attribute(rc, adap_p, pdat_p, cuat_p);
		if (*rc == E_MALLOC) 
			return;
		
	} /* end for() looping through bus resource attributes */

	return;
} /* end of process_bus_resource_attributes() */

/***************************************************************************/
/* Function Name : PROCESS_MODIFIER_ATTRIBUTES()                           */
/* Function Type : C function                                              */
/* Purpose       : Processes a bus resource modifier attribute : SHARE,    */
/*                 GROUP, WIDTH, or PRIORITY CLASS.                        */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - pointer to adapter structure                            */
/*  pdat         - array of predefined attributes                          */
/*  pnum         - number of predefined attribute objects                  */
/*  cuat         - array of customized attributes                          */
/*  cnum         - number of customized attribute objects                  */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void process_modifier_attributes(rc, adap_p, pdat, pnum, cuat, cnum)
	int *rc;
	adapter_t *adap_p;
	struct PdAt *pdat;
	int pnum;
	struct CuAt *cuat;
	int cnum;
{
	struct PdAt *pdat_p;
	struct CuAt *cuat_p;
	int ccnt, priority_done = FALSE;
	attribute_t *int_attr;

	/* Find first interrupt attribute for PRIORITY CLASS processing */
	for (int_attr = adap_p->attributes ; int_attr ; int_attr = int_attr->next)
		if (int_attr->resource == INTLVL)
			break;

	/*===================================================================*/
	/* Loop through the predefined attributes and process the "modifier" */
	/* attributes : SHARE, GROUP, WIDTH, and PRIORITY CLASS.             */
	/*===================================================================*/

	for (pdat_p = pdat ; pnum-- ; pdat_p++)
	{

		/*----------------------------------------------------*/
		/* Make sure we are working with a modifier attribute */
		/*----------------------------------------------------*/
		switch ((int)pdat_p->type[0])
		{
			case TSHARE    :
				process_share_attribute(rc, adap_p, pdat_p);
				break;
			case TGROUP    :
				process_group_attribute(rc, adap_p, pdat_p);
				break;
			case TWIDTH    :

				/* Find CuAt record matching this PdAt record */
				for (ccnt = cnum, cuat_p = cuat ; ccnt-- ; cuat_p++)
				{
					if (!strcmp(pdat_p->attribute, cuat_p->attribute) &&
					    !strcmp(pdat_p->type, cuat_p->type)              )
						break;
				}

				if (ccnt >= 0)
				{
					/* Found a CuAt record for width, apply it's value to PdAt width */
					strcpy(pdat_p->deflt, cuat_p->value);
				}

				process_width_attribute(rc, adap_p, pdat_p);
				break;
			case TPRIORITY :
				/* Process only the first PRIORITY modifier attribute */
				if (!priority_done)
				{
					process_priority_attribute(adap_p, pdat_p);
					priority_done = TRUE;
				}
				break;
			default        :
				/* Not a bus resource modifier attribute */
				continue;
		}

		/* Quit for malloc error */
		if (*rc == E_MALLOC)
			return;

	} /* end for() looping through bus resource attributes */

	/* If interrupt not assigned a priority use the default */
	if (int_attr != NULL && !priority_done)
		process_priority_attribute(adap_p, NULL);

	return;
} /* end of process_modifier_attributes() */

/***************************************************************************/
/* Function Name : PROCESS_SHARE_ATTRIBUTE()                               */
/* Function Type : Internal C function                                     */
/* Purpose       : Processes a SHARE attribute.                            */
/* Global Vars   :                                                         */
/*  ShareList    - List of shared with attribute information.              */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - pointer to adapter structure                            */
/*  pdat_p       - pointer to predefined attribute                         */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void process_share_attribute(rc, adap_p, pdat_p)
	int *rc;
	adapter_t *adap_p;
	struct PdAt *pdat_p;
{
	share_list_t *share, *share_p;

	/* Allocate a share list struct... */
	share = (share_list_t *)calloc(1, sizeof(share_list_t));
	if (share == (share_list_t *)NULL)
	{
		*rc = E_MALLOC;
		log_message(0, "process_share_attribute() : Out of virtual storage\n");
		return;
	}
	strncpy(share->shar_name, pdat_p->attribute, sizeof(share->shar_name) - 1); 
	strncpy(share->attr_name, pdat_p->values, sizeof(share->attr_name) - 1);
	share->adapter = adap_p;
	
	/* ...and link it into the share list */
	if (ShareList == (share_list_t *)NULL)
		ShareList = share;
	else
	{
		for (share_p = ShareList ; share_p->next ; share_p = share_p->next);
		share_p->next = share;
	}

	return;
} /* end of process_share_attribute() */

/***************************************************************************/
/* Function Name : PROCESS_GROUP_ATTRIBUTE()                               */
/* Function Type : Internal C function                                     */
/* Purpose       : Processes a GROUP attribute.                            */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - pointer to adapter structure                            */
/*  pdat_p       - pointer to predefined attribute                         */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void process_group_attribute(rc, adap_p, pdat_p)
	int *rc;
	adapter_t *adap_p;
	struct PdAt *pdat_p;
{
	attribute_t *attr_grp, *attr_p, *attr_p2;
	char vstr[ATTRVALSIZE+1], *tokstr = ",", *substr;
	int found, valcnt = 0, valcnt2, attrcnt = 0;
	value_list_t *val_p;

	/*----------------------------------------------------------------------*/
	/* Verify the GROUP attribute values :                                  */
	/*    1) each attribute in "values" must exist                          */
	/*    2) each attribute in "values" must be a LIST value representation */
	/*    3) each attribute in "values" must have same number of choices as */
	/*       every other attribute.                                         */
	/*    4) at least 2 members in a group                                  */
	/*----------------------------------------------------------------------*/

	vstr[ATTRVALSIZE] = '\0';
	strncpy(vstr, pdat_p->values, sizeof(vstr) - 1);
	for (substr = strtok(vstr, tokstr) ; substr ; substr = strtok(NULL, tokstr))
	{
		found = FALSE, attr_p = adap_p->attributes;
		for ( ; !found && attr_p ; attr_p = attr_p->next)
		{
			if (strcmp(substr, attr_p->name))
				continue;

			found = TRUE;
			attrcnt++;
			if (attr_p->reprsent != LIST)
			{
				log_error(1, TRUE, adap_p->logname, pdat_p->attribute,
				          "Group member not LIST representation (Check ODM database)");
				adap_p->unresolved = TRUE;
				*rc = E_BADATTR;
				return;
			}
			valcnt2 = 0;
			for (val_p = attr_p->values.list.head ; val_p ; val_p = val_p->next)
				valcnt2++;
			if (valcnt == 0)
				valcnt = valcnt2;
			else if (valcnt != valcnt2)
			{
				log_error(1, TRUE, adap_p->logname, pdat_p->attribute,
				          "Group members have different possible values (Check ODM database)");
				adap_p->unresolved = TRUE;
				*rc = E_BADATTR;
				return;
			}
		}

		if (!found)
		{
			log_error(1, TRUE, adap_p->logname, pdat_p->attribute,
			          "Group member not found (Check ODM database)");
			adap_p->unresolved = TRUE;
			*rc = E_BADATTR;
			return;
		}
	} /* end for() verifying values */

	if (attrcnt < 2)
	{
		log_error(1, FALSE, adap_p->logname, pdat_p->attribute,
		          "Group needs at least 2 members, group ignored (Check ODM database)");
		return;
	}

	/*-------------------------------------------*/
	/* Create and link in group attribute struct */ 	
	/*-------------------------------------------*/

	/* Allocate group attribute struct */
	attr_grp = new_attribute(rc, NULL, pdat_p, NULL);
	if (*rc == E_MALLOC)
		return;
	attr_grp->adapter_ptr = adap_p;

	/* Link group attribute struct at beginning of adapter's attributes list */
	attr_p = adap_p->attributes;
	adap_p->attributes = attr_grp;
	attr_grp->next = attr_p;

	/*---------------------------------------------------*/
	/* Now link the group members to the group attribute */
	/*---------------------------------------------------*/

	vstr[ATTRVALSIZE] = '\0';
	strncpy(vstr, pdat_p->values, sizeof(vstr) - 1);
	for (substr = strtok(vstr, tokstr) ; substr ; substr = strtok(NULL, tokstr))
	{
		found = FALSE, attr_p = attr_grp->next;
		for ( ; !found && attr_p ; attr_p = attr_p->next )
		{
			if (strcmp(substr, attr_p->name))
				continue;

			found = TRUE;

			/* Unlink attribute from adapter's attribute list */
			for (attr_p2 = adap_p->attributes ; ; attr_p2 = attr_p2->next) 
				if (attr_p2->next == attr_p)
					break;
			attr_p2->next = attr_p->next;
			attr_p->next = NULL;

			/* Link attribute into group list */
			for (attr_p2 = attr_grp ; ; attr_p2 = attr_p2->group_ptr)
				if (attr_p2->group_ptr == NULL)
					break;
			attr_p2->group_ptr = attr_p;

			break;
		}
	}

	return;
} /* end of process_group_attribute() */

/***************************************************************************/
/* Function Name : PROCESS_WIDTH_ATTRIBUTE()                               */
/* Function Type : Internal C function                                     */
/* Purpose       : Processes a WIDTH attribute.                            */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - pointer to adapter structure                            */
/*  pdat_p       - pointer to predefined attribute (with cus value applied)*/
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void process_width_attribute(rc, adap_p, pdat_p)
	int *rc;
	adapter_t *adap_p;
	struct PdAt *pdat_p;
{
	unsigned long width;
	attribute_t *attr_p1, *attr_p2, *attr_p3;
	int found;

	/* Get the width value */
	width = strtoul(pdat_p->deflt, (char **)NULL, 0);

	/*-----------------------------------------------*/
	/* Find the bus resource attribute named in the  */
	/* width field of the WIDTH attribute, and apply */
	/* to the width of the bus resource attribute.   */
	/*-----------------------------------------------*/

	found = FALSE, attr_p1 = adap_p->attributes;

	/* Handle first attribute in list */
	if (!strcmp(attr_p1->name, pdat_p->width))
	{
		found = TRUE;
		if (width == 0)
		{
			adap_p->attributes = attr_p1->next;   /* Relink list */
			attr_p1->next = NULL;                 /* Disconnect from list */
			destroy_attribute_list(rc, attr_p1);  /* Free attribute */
		}
		else
			attr_p1->width = width;               /* Apply width */
	}
	else /* It's not the first attribute in the list */
	{
		for ( ; !found && attr_p1 ; attr_p1 = attr_p1->next)
		{
			/*-------------------------------------------------------*/
			/* Loop through attr_p1 (and handle GROUP) using attr_p2 */
			/*-------------------------------------------------------*/
			if (attr_p1->resource == GROUP)
				attr_p2 = attr_p1->group_ptr;
			else
				attr_p2 = attr_p1->next;

			if (attr_p2 == NULL)
				break;

			if (attr_p1->resource == GROUP)
			{
				if (!strcmp(attr_p2->name, pdat_p->width))
				{
					found = TRUE;
					if (width == 0)
					{
						attr_p1->group_ptr = attr_p2->group_ptr;        /* Relink list */
						attr_p2->group_ptr = NULL;                      /* Disconnect from list */
						destroy_attribute_list(rc, attr_p2);            /* Free attribute */
					}
					else
						attr_p2->width = width;                         /* Apply width */
				}
				else /* It's not the first attribute in the list */
				{
					for ( ; !found && attr_p2->group_ptr ; attr_p2 = attr_p2->group_ptr)
					{
						if (!strcmp(attr_p2->group_ptr->name, pdat_p->width))
						{
							found = TRUE;
							if (width == 0)
							{ 
								attr_p3 = attr_p2->group_ptr;
								attr_p2->group_ptr = attr_p3->group_ptr;    /* Relink list */
								attr_p3->group_ptr = NULL;                  /* Disconnect from list */
								destroy_attribute_list(rc, attr_p3);        /* Free attribute */
							}
							else
								attr_p2->group_ptr->width = width;          /* Apply width */
						}
					}
				}
			}

			else if (!strcmp(attr_p2->name, pdat_p->width))
			{
				found = TRUE;
				if (width == 0)
				{
					attr_p1->next = attr_p2->next;        /* Relink list */
					attr_p2->next = NULL;                 /* Disconnect from list */
					destroy_attribute_list(rc, attr_p2);  /* Free attribute */
				}
				else
					attr_p2->width = width;               /* Apply width */
			}
		}
	}

	/* Log error */
	if (!found)
		log_error(1, FALSE, adap_p->logname, pdat_p->attribute,
		          "Width target attribute not found, width ignored (Check ODM database)");

	return;
} /* end of process_width_attribute() */

/***************************************************************************/
/* Function Name : PROCESS_PRIORITY_ATTRIBUTE()                            */
/* Function Type : Internal C function                                     */
/* Purpose       : Processes a PRIORITY CLASS attribute.                   */
/* Arguments     :                                                         */
/*  adap_p       - pointer to adapter structure                            */
/*  pdat_p       - pointer to predefined attribute (optional)              */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/* Note : When pdat_p parameter is NULL, interrupt attributes are assigned */
/*        the default priority class value.                                */
/*                                                                         */
/***************************************************************************/
static void process_priority_attribute(adap_p, pdat_p)
	adapter_t *adap_p;
	struct PdAt *pdat_p;
{
	unsigned long priority;
	attribute_t *attr_p1, *attr_p2;
	int found;

	/* Get priority class value (not checking) */
	if (pdat_p == NULL)
		priority = DEFAULT_PRIORITY_CLASS;
	else
		priority = strtoul(pdat_p->deflt, (char **)NULL, 0);

	/*-----------------------------------------------------*/
	/* Apply priority class value to the INTLVL attributes */
	/*-----------------------------------------------------*/
	found = FALSE;
	for (attr_p1 = adap_p->attributes ; attr_p1 ; attr_p1 = attr_p1->next) 
	{
		/*-------------------------------------------------------*/
		/* Loop through attr_p1 (and handle GROUP) using attr_p2 */
		/*-------------------------------------------------------*/
		if (attr_p1->resource == GROUP)
			attr_p2 = attr_p1->group_ptr;
		else
			attr_p2 = attr_p1;
		for ( ; attr_p2 ; attr_p2 = attr_p2->group_ptr)
		{
			if (attr_p2->resource != INTLVL)
				continue;
	
			found = TRUE;
			attr_p2->priority = priority;
		}
	}

	/* Log the error */
	if (!found)
		log_error(1, FALSE, adap_p->logname, pdat_p->attribute,
		          "No interrupt attrs, priority class attr ignored (Check ODM database)");

	return;
} /* end of process_priority_attribute() */

/***************************************************************************/
/* Function Name : NEW_ATTRIBUTE()                                         */
/* Function Type : C function                                              */
/* Purpose       : Allocates and initializes an instance of the attribute  */
/*                 structure.                                              */ 
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - pointer to adapter structure to attach to (optional)    */
/*  pdat_p       - pointer to Predefined Attributes object (optional)      */
/*  cuat_p       - pointer to Customized Attributes object (optional)      */
/*                                                                         */
/* Return Value  : Pointer to instance of new attribute structure          */
/*                                                                         */
/***************************************************************************/
attribute_t *new_attribute(rc, adap_p, pdat_p, cuat_p)
	int *rc;
	adapter_t *adap_p;
	struct PdAt *pdat_p;
	struct CuAt *cuat_p;
{
	attribute_t *attr_p = NULL, *attr_p2;

	/*----------------------------*/
	/* Create attribute structure */
	/*----------------------------*/

	if (adap_p != NULL)
	{
		if (adap_p->attributes == NULL) /* First attribute in list */
			adap_p->attributes = attr_p = (attribute_t *)calloc(1, sizeof(attribute_t));
		else /* Add to end of list */
		{
			for (attr_p2 = adap_p->attributes ; attr_p2->next ; attr_p2 = attr_p2->next);
			attr_p2->next = attr_p = (attribute_t *)calloc(1, sizeof(attribute_t));
		}
	}
	else /* Not linking into adapter's attribute list */
		attr_p = (attribute_t *)calloc(1, sizeof(attribute_t));

	/* Check malloc */
	if (attr_p == (attribute_t *)NULL)
	{
		*rc = E_MALLOC;
		log_message(0, "new_attribute() : Out of virtual storage\n");
		return (attribute_t *)NULL;
	}

	/* Link back to adapter struct */
	attr_p->adapter_ptr = adap_p;

	/*----------------------------------------------------*/
	/* Fill in attribute structure (if PdAt rec supplied) */
	/*----------------------------------------------------*/

	if (pdat_p != NULL)
	{	

		/* Add the name of the attribute */
		strncpy(attr_p->name, pdat_p->attribute, sizeof(attr_p->name) - 1);

		/*-------------------------------------------------*/
		/* Establish attribute type for the new attribute. */ 
		/*-------------------------------------------------*/ 
		switch ((int)pdat_p->type[0])
		{
			case TGROUP:
				attr_p->resource = GROUP; 
				attr_p->specific_resource = GROUP;
				return attr_p;
			case TBADDR:
				attr_p->resource = ADDR;
				attr_p->specific_resource = BADDR;		
				break; 
			case TMADDR:
				attr_p->resource = ADDR;
				if (adap_p->parent_bus_type == MCA)
					attr_p->specific_resource = MADDR;		
				else /* No distinction between M and B for PCI/ISA/PCMCIA */
					attr_p->specific_resource = BADDR;
				break; 
			case TIOADDR:
				attr_p->resource = IOADDR;	 
				attr_p->specific_resource = IOADDR;	
				break; 
			case TINTLVL:
				attr_p->resource = INTLVL; 
				attr_p->specific_resource = INTLVL; 
				break; 
			case TNSINTLVL:
				attr_p->resource = INTLVL; 
				attr_p->specific_resource = NSINTLVL; 
				break; 
			case TDMALVL:
				attr_p->resource = DMALVL;
				attr_p->specific_resource = DMALVL;
		}

		/* Now obtain attribute values */
		get_attribute_values(rc, pdat_p, cuat_p, attr_p);

		/* Process significant bits in PdAt type field */
		if (strlen(pdat_p->type) > 1 && pdat_p->type[1] != '.')
		{
			unsigned long sigbits = strtoul(&(pdat_p->type[1]), (char **)NULL, 0);

			if (sigbits == 0 || sigbits > 32)
			{
				log_error(1, TRUE, attr_p->adapter_ptr->logname, attr_p->name,
				          "Invalid significant bits in PdAt record type field");
				*rc = E_BADATTR; 
				attr_p->adapter_ptr->unresolved = TRUE;
				return attr_p;
			}
			else
			{
				attr_p->addrmask = 0xffffffff >> (32 - sigbits);
				reserve_range(rc, attr_p, attr_p->addrmask, 0xffffffff,
				              "processing PdAt type field significant bits");
			}
		}
		else
			attr_p->addrmask = 0xffffffff;

	} /* end processing PdAt record */

	return attr_p;

} /* end new_attribute() */

/***************************************************************************/
/* Function Name : GET_ATTRIBUTE_VALUES()                                  */
/* Function Type : Internal C function                                     */
/* Purpose       : Get device attribute values                             */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  pdat_p       - Predefined attribute record                             */
/*  cuat_p       - Customized attribute record                             */
/*  attr_p       - Attribute structure pointer                             */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void get_attribute_values(rc, pdat_p, cuat_p, attr_p)
	int *rc;
	struct PdAt *pdat_p;
	struct CuAt *cuat_p;
	attribute_t *attr_p;
{
	unsigned long *curr  = &(attr_p->current),
	              *width = &(attr_p->width);

	/*------------------------------------------------------*/
	/* Get the current value - either customized or default */
	/*------------------------------------------------------*/

	*curr = strtoul(((cuat_p) ? cuat_p->value : pdat_p->deflt), (char**)NULL, 0);

	/*----------------------------------------*/
	/* Get the width and make sure it's not 0 */
	/*----------------------------------------*/
 
	*width = (*width = strtoul(pdat_p->width, (char **)NULL, 0)) ? *width : 1;

	/*-------------------------*/
	/* Get the possible values */
	/*-------------------------*/

	if (!strlen(pdat_p->values)) 
	{

		/*----------------------------------------------------------------*/
		/* If the PdAt values are NULL, get value from the CuAt object.   */
		/* If the CuAt object does not exist, the bus specific modules    */
		/* may decide how to handle it. If the bus specific modules do    */
		/* not handle, error trapped in verify_bus_resource_attributes(). */ 
		/* This condition is ID'd by the NULLVALS representation enum.    */
		/*----------------------------------------------------------------*/

		if (!cuat_p)
			attr_p->reprsent = NULLVALS;
		else
		{
			attr_p->reprsent            = LIST;
			attr_p->values.list.head    = strtovlist(rc, cuat_p->value);
			attr_p->values.list.current = attr_p->values.list.head;
		}

	}
	else if (strchr(pdat_p->values, RANGESEP))
	{
		attr_p->reprsent             = RANGE;
		attr_p->values.range.head    = strtorlist(rc, pdat_p->values);
		attr_p->values.range.current = attr_p->values.range.head;
	}
	else /* no RANGESEP in pdat_p->values */
	{
		attr_p->reprsent            = LIST;
		attr_p->values.list.head    = strtovlist(rc, pdat_p->values);
		attr_p->values.list.current = attr_p->values.list.head;
	}

	/* If NOT including all values and user modifiable mark as such */
	if (strchr(pdat_p->generic, 'U'))
		attr_p->user = TRUE;

	/* Set edge / level interrupt trigger for interrupt attributes */
	if (attr_p->resource == INTLVL)
	{
		if (strstr(cuat_p->value,",E"))
			attr_p->trigger = EDGE;
		if (strstr(cuat_p->value,",L"))
			attr_p->trigger = LEVEL;
	}

	return;
} /* end get_attribute_values */

/***************************************************************************/
/* Function Name : STRTORLIST()                                            */
/* Function Type : Internal C function                                     */
/* Purpose       : Convert "l-u,i" string to RANGE list element            */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  str          - string to be converted                                  */
/*                                                                         */
/* Return Value  : Range list element                                      */
/*                                                                         */
/***************************************************************************/
static value_range_t *strtorlist(rc, str)
	int *rc;
	char *str;
{
	value_range_t *rlist_p;

	/* Allocate node */
	rlist_p = (value_range_t *)calloc(1, sizeof(value_range_t));
	if (rlist_p == (value_range_t *)NULL)
	{
		*rc = E_MALLOC;
		log_message(0, "strtorlist() : Out of virtual storage\n");
		return rlist_p;
	}

	/* Parse lower-upper,increment string for numeric values */ 
	rlist_p->lower = strtoul(str, (char **)NULL, 0);
	rlist_p->upper = strtoul(strchr(str, RANGESEP) + 1, (char **)NULL, 0); 
	rlist_p->incr = (str = strchr(str, ',')) ? strtoul(str + 1, (char **)NULL, 0) : 1; 

	return rlist_p;
} /* end of strtouli() */

/***************************************************************************/
/* Function Name : STRTOVLIST()                                            */
/* Function Type : C function                                              */
/* Purpose       : Convert string to list of unsigned longs                */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  str          - string to be converted                                  */
/*                                                                         */
/* Return Value  : Head of a values list                                   */
/*                                                                         */   
/***************************************************************************/
value_list_t *strtovlist(rc, str)
	int *rc;
	char *str;
{
	char *str_p; 
	value_list_t *head_p = (value_list_t *)NULL, *vlist_p;

	/*--------------------------*/
	/* Parse values from string */
	/*--------------------------*/

	for ( str_p = str ; str_p != (char *)1 ; str_p = strchr(str_p, ',') + 1 )
	{
		
		/* Allocate node */
		if (head_p == (value_list_t *)NULL)
			head_p = vlist_p = (value_list_t *)calloc(1, sizeof(value_list_t));
		else	 
			vlist_p->next = (value_list_t *)calloc(1, sizeof(value_list_t)), 
			vlist_p = vlist_p->next;
		if (vlist_p == (value_list_t *)NULL)
		{
			*rc = E_MALLOC;
			log_message(0, "strtovlist() : Out of virtual storage\n");
			break;
		}

		/* Get value */
		vlist_p->value = strtoul(str_p, (char **)NULL, 0);

	} /* end for () */
 
	return head_p; 
} /* end of strtovlist() */

/***************************************************************************/
/* Function Name : SETUP_LIST_ATTRIBUTE()                                  */
/* Function Type : C function                                              */
/* Purpose       : An alternative to passing PdAt / CuAt to new_attribute()*/
/*                 to get the struct filled out - this is handy for those  */
/*                 nasty reserved values or special attributes which turn  */
/*                 out to be a list of a single value.                     */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  attr_p       - attribute_t structure pointer                           */
/*  name         - attribute name (OPTIONAL)                               */
/*  resrc        - resource type                                           */
/*  val          - current value                                           */
/*  wid          - width                                                   */
/*  bll          - bus values domain lower limit                           */
/*  bul          - bus values domain upper limit                           */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void setup_list_attribute(rc, adap_p, attr_p, name, resrc, val, wid, bll, bul)
	int *rc;
	adapter_t *adap_p;
	attribute_t *attr_p;
	char *name;
	bus_resource_e resrc;
	unsigned long val, wid, bll, bul; 
{

	/* Fill out struct according to args */
	if (name != NULL)
		strcpy(attr_p->name, name);
	attr_p->reprsent = LIST;
	attr_p->width    = wid;
	attr_p->busllim  = bll;
	attr_p->busulim  = bul;
	attr_p->addrmask = -1;
	attr_p->values.list.head = (value_list_t *)calloc(1, sizeof(value_list_t));
	if (attr_p->values.list.head == NULL)
	{
		*rc = E_MALLOC;
		log_message(0, "setup_list_attribute() : Out of virtual storage\n");
		return;
	}
	attr_p->values.list.current = attr_p->values.list.head;
	attr_p->adapter_ptr = adap_p;

	if (resrc == NSINTLVL)
		attr_p->resource = INTLVL;
	else if (resrc == MADDR || resrc == BADDR)
		attr_p->resource = ADDR;
	else
		attr_p->resource = resrc;

	attr_p->specific_resource = resrc;

	attr_p->current = val;
	attr_p->valllim = val;
	attr_p->valulim = val + (wid - 1);
	attr_p->values.list.head->value = val;

	attr_p->start.list.ptr = attr_p->values.list.head;

	return;
} /* end of setup_list_attribute() */

/***************************************************************************/
/* Function Name : VERIFY_BUS_RESOURCE_ATTRIBUTES()                        */
/* Function Type : Internal C function                                     */
/* Purpose       : Check to see that the attribute values are specified    */
/*                 properly and are adjusted to the limits which are       */
/*                 valid for the platform on which we are running.         */
/* Arguments     :                                                         */
/*  rc            - Integer return code from cf.h                          */
/*  adap_p        - Pointer to adapter structure for which attributes are  */
/*                  to be verified                                         */
/*  incusr        - Bool include user modified values                      */
/*  get_domain_cb - Bus specific callback for resource domain information  */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void verify_bus_resource_attributes(rc, adap_p, incusr, get_domain_cb)
	int *rc;
	adapter_t *adap_p;
	int incusr;
	void get_domain_cb();
{
	attribute_t *attr_p1, *attr_p2, *attr_grp;
	
	if (adap_p->unresolved)
		return;

	for (attr_p1 = adap_p->attributes ;
	          attr_p1 && attr_p1->adapter_ptr == adap_p ;
	                     attr_p1 = attr_p1->next           )
	{
		/*-------------------------------------------------------*/
		/* Loop through attr_p1 (and handle GROUP) using attr_p2 */
		/*-------------------------------------------------------*/
		if (attr_p1->resource == GROUP)
		{
			attr_p2  = attr_p1->group_ptr;
			attr_grp = attr_p1;
		}
		else
		{
			attr_p2  = attr_p1;
			attr_grp = NULL;
		}
		for ( ; attr_p2 ; attr_p2 = attr_p2->group_ptr)
		{

			/*------------------------------------------------------------*/
			/* Null values are allowed in the database but must have been */
			/* taken care of by now. The bus specific module did not or   */
			/* could not handle this condition.                           */
			/*------------------------------------------------------------*/

			if (attr_p2->reprsent == NULLVALS)
			{
				log_error(1, TRUE, attr_p2->adapter_ptr->logname, attr_p2->name,
				          "NULL PdAt values field with no CuAt object");
				*rc = E_BADATTR; 
				attr_p2->adapter_ptr->unresolved = TRUE;
				return;
			}

			/*------------------------------------------------*/
			/* Interrupt and DMA attributes must be width = 1 */
			/*------------------------------------------------*/

			if (attr_p2->resource == INTLVL && attr_p2->width != 1)
			{
				log_error(1, TRUE, adap_p->logname, attr_p2->name,
				          "Interrupt attribute must use width of 1 (Check ODM database)");
				*rc = E_BADATTR;
				attr_p2->adapter_ptr->unresolved = TRUE;
				return;
			}

			if (attr_p2->resource == DMALVL && attr_p2->width != 1)
			{
				log_error(1, TRUE, adap_p->logname, attr_p2->name,
				          "DMA arb level attribute must use width of 1 (Check ODM database)");
				*rc = E_BADATTR;
				attr_p2->adapter_ptr->unresolved = TRUE;
				return;
			}

			/*-------------------------*/
			/* Verify attribute values */
			/*-------------------------*/

			if (attr_p2->reprsent == LIST)
				verify_list(rc, adap_p, attr_p2, attr_grp, get_domain_cb);
			else /* attr_p2->reprsent == RANGE */
				verify_range(rc, adap_p, attr_p2, get_domain_cb);

			if (adap_p->unresolved || *rc == E_MALLOC || *rc == E_DEVACCESS)
				return;

			/*--------------------------------------------------------*/
			/* Process user attributes by converting to a list of one */
			/*--------------------------------------------------------*/

			if (!incusr && attr_p2->user)
			{
				char str[25]; 

				if (attr_p2->reprsent == LIST)
					destroyvlist(rc, attr_p2->values.list.head); /* free the full list */
				else /* attr_p2->reprsent == RANGE */
					destroyrlist(rc, attr_p2->values.range.head); /* free the full list */

				/* Convert to list of one value */
				attr_p2->reprsent = LIST;
				sprintf(str, "%lu", attr_p2->current);
				attr_p2->values.list.head = strtovlist(rc, str);
				attr_p2->values.list.current = attr_p2->values.list.head;
			}

		}

		/* Sync all GROUP members to the first member */
		if (attr_grp)
			sync_group_attribute(attr_grp);

	} /* end for() checking all attributes */

	return;
} /* end of verify_bus_resource_attributes() */

/***************************************************************************/
/* Function Name : SYNC_GROUP_ATTRIBUTE()                                  */
/* Function Type : C function                                              */
/* Purpose       : Syncronize a GROUP attribute                            */
/* Arguments     :                                                         */ 
/*  attr_grp       - Group attribute to sync                               */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void sync_group_attribute(attr_grp)
	attribute_t *attr_grp;
{
	attribute_t *attr_p;
	value_list_t *val_p;
	int element;

	/* Find the starting element for the first GROUP member... */
	attr_p = attr_grp->group_ptr;
	val_p = attr_p->values.list.head;
	element = 0;
	for ( ; val_p ; val_p = val_p->next, element++)
		if (val_p->value == attr_p->current)
			break;

	if (!val_p)
	{
		element = 0;
		/* Modify current value... */
		attr_p->values.list.current = attr_p->values.list.head;
		attr_p->current             = attr_p->values.list.head->value;

		/* ...set changed flag and save start pointer */
		attr_p->changed             = TRUE;
		attr_p->start.list.ptr      = attr_p->values.list.current;
	}

	/* ...and sync each GROUP member to the first */
	for (attr_p = attr_p->group_ptr; attr_p; attr_p = attr_p->group_ptr)
	{
		int elem = element;

		for (val_p = attr_p->values.list.head ; elem-- ; val_p = val_p->next);
		if (val_p != attr_p->values.list.current)
		{
			/* Modify current value... */
			attr_p->values.list.current = val_p;
			attr_p->current             = val_p->value;

			/* ...set changed flag and save start pointer */
			attr_p->changed             = TRUE;
			attr_p->start.list.ptr      = attr_p->values.list.current;
		}
	}

	return;
}

/***************************************************************************/
/* Function Name : CONVERT_TO_LIST()                                       */
/* Function Type : C function                                              */
/* Purpose       : Convert a RANGE attribute to a LIST attribute           */
/* Arguments     :                                                         */ 
/*  rc             - integer return code from cf.h                         */
/*  attr_p         - Range attribute to convert                            */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void convert_to_list(rc, attr_p)
	int *rc;
	attribute_t *attr_p;
{
	unsigned long lower, upper, incr, val;
	value_list_t *head_p = (value_list_t *)NULL, *vall_p;
	value_range_t *valr_p;

	/*-------------------------*/ 
	/* Loop through RANGE list */
	/*-------------------------*/ 
	for (valr_p = attr_p->values.range.head ; valr_p ; valr_p = valr_p->next)
	{

		lower = valr_p->lower;
		incr  = valr_p->incr;
		upper = valr_p->upper;
	
		/*----------------------*/
		/* Build the value list */
		/*----------------------*/
		for (val = lower ; val <= upper ; val += incr)
		{
			if (head_p == (value_list_t *)NULL)	
				head_p = vall_p = (value_list_t *)calloc(1, sizeof(value_list_t));
			else
			{
				vall_p->next = (value_list_t *)calloc(1, sizeof(value_list_t));
				vall_p = vall_p->next;
			}
	
			if (vall_p == (value_list_t *)NULL)
			{
				destroyvlist(rc, head_p);
				*rc = E_MALLOC;
				log_message(0, "convert_to_list() : Out of virtual storage\n");
				return;
			}
			vall_p->value = val;
		}
	}

	/*------------------------------*/
	/* Get pointer to current value */
	/*------------------------------*/
	for (vall_p = head_p ; vall_p ; vall_p = vall_p->next)
		if (vall_p->value == attr_p->current)
			break;
	if (!vall_p)
	{
		vall_p = head_p;
		attr_p->current = vall_p->value;
	}

	/* Free the RANGE list */
	destroyrlist(rc, attr_p->values.range.head);

	/*----------------------------------------*/
	/* Attach the value list to the attribute */
	/*----------------------------------------*/
	attr_p->values.list.head    = head_p;  
	attr_p->values.list.current = vall_p;
	attr_p->reprsent            = LIST;

	return;
} /* end convert_to_list() */

/***************************************************************************/
/* Function Name : VERIFY_LIST()                                           */
/* Function Type : Internal C function                                     */
/* Purpose       : Verify LIST attribute values                            */
/* Arguments     :                                                         */
/*  rc             - integer return code from cf.h                         */
/*  adap_p         - pointer to adapter struct                             */
/*  attr_p         - pointer to attributes struct                          */
/*  attr_grp       - pointer to GROUP node if attr_p is a GROUP member     */
/*  get_domain_cb  - Bus specific callback for resource domain information */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void verify_list(rc, adap_p, attr_p, attr_grp, get_domain_cb)
	int *rc;
	adapter_t *adap_p;
	attribute_t *attr_p;
	attribute_t *attr_grp;
	void get_domain_cb();
{
	value_list_t *val_p;
	int element;
	int plat_adjusted = FALSE, db_adjusted = FALSE;
	unsigned long platmin, platmax, dbmin, dbmax;

	/* Use the callback provided by the bus type module */
	get_domain_cb(rc, attr_p, &platmin, &platmax, &dbmin, &dbmax);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS)
		return;

	attr_p->busllim = platmin;
	attr_p->busulim = platmax;

	/* Fail if the width is greater than the domain of allowable values */
	if (attr_p->width - 1 > dbmax - dbmin)
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Width greater than available resource (Check ODM database)");
		*rc = E_BADATTR;
		adap_p->unresolved = TRUE;
		return;
	}
	if (attr_p->width - 1 > platmax - platmin)	
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Width greater than available resource on this platform");
		*rc = E_BUSRESOURCE;
		adap_p->unresolved = TRUE;
		return;
	}

	/* Sync this attribute */
	sync_list_attribute(rc, attr_p, FALSE);

	/*---------------------------------------------------------------------------------*/
	/* Loop through the elements, and discard those outside the domain of valid values */
	/*---------------------------------------------------------------------------------*/
	for (val_p = attr_p->values.list.head, element = 0 ; val_p ; )
	{

		/* Check element against domain minimum value */
		if (val_p->value < dbmin  || 
				val_p->value < platmin  )
		{
			if (val_p->value < dbmin)
			{
				log_error(1, FALSE, adap_p->logname, attr_p->name,
				          "List element value too low (Check ODM database)");
				db_adjusted = TRUE;
			}
			else
			{
				log_error(1, FALSE, adap_p->logname, attr_p->name,
				          "List element value too low on this platform");
				plat_adjusted = TRUE;
			}

			if (attr_grp)
				eliminate_value_list_element(attr_grp, element);
			else 
				eliminate_value_list_element(attr_p, element);

			/* Restart the scan */
			val_p = attr_p->values.list.head;
			element = 0;
			continue;
		}

		/* Check element against domain maximium value */
		if (val_p->value + (attr_p->width - 1) > dbmax  ||
				val_p->value + (attr_p->width - 1) > platmax  )
		{
			if (val_p->value + (attr_p->width - 1) > dbmax)
			{
				log_error(1, FALSE, adap_p->logname, attr_p->name,
				          "List element value too high (Check ODM database)");
				db_adjusted = TRUE;
			}
			else
			{
				log_error(1, FALSE, adap_p->logname, attr_p->name,
				          "List element value too high on this platform");
				plat_adjusted = TRUE;
			}

			if (attr_grp)
				eliminate_value_list_element(attr_grp, element);
			else 
				eliminate_value_list_element(attr_p, element);

			/* Restart the scan */
			val_p = attr_p->values.list.head;
			element = 0;
			continue;
		}

		/* Increment loop invariant and element count */
		val_p = val_p->next;
		element++;

	} /* end for() checking value list values */

	/* Fail if there are no values left in the list */
	if (attr_p->values.list.head == (value_list_t *)NULL)
	{
		if (!plat_adjusted)
			log_error(1, TRUE, adap_p->logname, attr_p->name, 
			          "No valid values in list (Check ODM database)");
		else
			log_error(1, TRUE, adap_p->logname, attr_p->name, 
				  "No valid values in list");
		*rc = E_BADATTR;
		adap_p->unresolved = TRUE;
		return;
	}

	set_value_limits(attr_p, platmin, platmax);

	/* Sync this attribute */
	sync_list_attribute(rc, attr_p, (plat_adjusted || db_adjusted));

	return;
} /* end of verify_list() */

/***************************************************************************/
/* Function Name : ELIMINATE_VALUE_LIST_ELEMENT()                          */
/* Function Type : C function                                              */
/* Purpose       : Delete an element of the value list.                    */
/* Arguments     :                                                         */
/*  attr_p       - attribute struct pointer                                */
/*  element      - position in values list to be deleted (0 is first)      */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void eliminate_value_list_element(attr_p, element)
	attribute_t *attr_p;	
	int element;
{
	attribute_t *attr_p1;
	value_list_t *val_p1, *val_p2;
	int elem;

	/* Handle GROUP type using attr_p1 */ 
	if (attr_p->resource == GROUP)
		attr_p1 = attr_p->group_ptr; 
	else
		attr_p1 = attr_p;
	for ( ; attr_p1 ; attr_p1 = attr_p1->group_ptr)
	{

		/* Find the target element */
		for (val_p1 = attr_p1->values.list.head, elem = element ; elem-- ; )
			val_p1 = val_p1->next;

		/* Unlink the target element from the list */
		if (val_p1 == attr_p1->values.list.head)
			attr_p1->values.list.head = val_p1->next;
		else
		{
			for (val_p2 = attr_p1->values.list.head ; val_p2 ; val_p2 = val_p2->next)
				if (val_p2->next == val_p1) 
					break;

			val_p2->next = val_p1->next;
		}

		/* Free the target element */
		free(val_p1);
	}

	return;
} /* end of eliminate_value_list_element() */

/***************************************************************************/
/* Function Name : SYNC_LIST_ATTRIBUTE()                                   */
/* Function Type : C function                                              */
/* Purpose       : Call at least once to setup pointers, and also after    */
/*                 modifying the possible values.                          */
/*                 1) Set current value                                    */
/*                 2) Set current value pointer                            */
/*                 3) Set start pointer for wrap check                     */
/* Arguments     :                                                         */
/*  rc             - integer return code from cf.h                         */
/*  attr_p         - attribute struct pointer                              */
/*  modified       - Boolean attr poss vals have been modified             */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void sync_list_attribute(rc, attr_p, modified)
	int *rc;
	attribute_t *attr_p;
	int modified;
{
	value_list_t *val_p;

	/* Find the current value in the list */ 
	for (val_p = attr_p->values.list.head ; val_p ; val_p = val_p->next)
		if (val_p->value == attr_p->current)
			break;

	/* Set current element pointer */
	attr_p->values.list.current = val_p;

	/* Handle no match found for current value and list */
	if (!val_p)
	{
		if (attr_p->user)
		{
			log_error(1, TRUE, attr_p->adapter_ptr->logname, attr_p->name,
			          "User value not in list (Check ODM database)");
			*rc = E_BADATTR;
			attr_p->adapter_ptr->unresolved = TRUE;
			return;
		}
		/* else */
			if (modified)
				log_error(1, FALSE, attr_p->adapter_ptr->logname, attr_p->name,
				          "Default value not in list, using first element\n");
			else
				log_error(1, FALSE, attr_p->adapter_ptr->logname, attr_p->name,
				          "Default value not in list, using first element (Check ODM database)");

			/* Modify current value... */
			attr_p->values.list.current = attr_p->values.list.head;
			attr_p->current             = attr_p->values.list.head->value;
			/* ...set changed flag and save start pointer */ 
			attr_p->changed             = TRUE;
			attr_p->start.list.ptr      = attr_p->values.list.current;
	}

	return;
} /* end of sync_list_attribute() */

/***************************************************************************/
/* Function Name : VERIFY_RANGE()                                          */
/* Function Type : Internal C function                                     */
/* Purpose       : Verify RANGE attribute values.                          */
/* Arguments     :                                                         */
/*  rc             - integer return code from cf.h                         */
/*  adap_p         - pointer to adapter struct                             */
/*  attr_p         - pointer to attributes struct                          */
/*  get_domain_cb  - Bus specific callback for resource domain information */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/* Notes : Must be called only for range lists of one element!             */
/*                                                                         */
/***************************************************************************/
static void verify_range(rc, adap_p, attr_p, get_domain_cb)
	int *rc;
	adapter_t *adap_p;
	attribute_t *attr_p;
	void get_domain_cb();
{
	unsigned long width = attr_p->width;
	unsigned long platmin, platmax, dbmin, dbmax;

	/* Use the callback provided by the bus type module */
	get_domain_cb(rc, attr_p, &platmin, &platmax, &dbmin, &dbmax);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS)
			return;

	attr_p->busllim = platmin;
	attr_p->busulim = platmax;

	/* Fail if the width is greater than the domain of allowable values */
	if (width - 1 > dbmax - dbmin)
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Width greater than available resource (Check ODM database)");
		*rc = E_BADATTR;
		adap_p->unresolved = TRUE;
		return;
	}
	if (width - 1 > platmax - platmin) 
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Width greater than available resource on this platform");
		*rc = E_BUSRESOURCE;
		adap_p->unresolved = TRUE;
		return;
	}
	
	/* Fail if the upper value is less than the lower value */
	if (attr_p->values.range.head->upper < attr_p->values.range.head->lower)
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Upper value less than lower value (Check ODM database)");
		adap_p->unresolved = TRUE;
		*rc = E_BADATTR;
		return;
	}

	/* Fail if increment value is zero */
	if (attr_p->values.range.head->incr == 0)
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Increment value of zero is invalid (Check ODM database)");
		adap_p->unresolved = TRUE;
		*rc = E_BADATTR;
		return;
	}

	/* Fail if the upper value is less than the minimum (plus width) */
	if (attr_p->values.range.head->upper < dbmin + (width - 1))
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Range outside allowable values (Check ODM database)");
		*rc = E_BADATTR;
		adap_p->unresolved = TRUE;
		return;
	}
	if (attr_p->values.range.head->upper < platmin + (width - 1)) 
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Range outside allowable values on this platform");
		*rc = E_BUSRESOURCE;
		adap_p->unresolved = TRUE;
		return;
	}

	/* Fail if the lower value is greater than the maximum (minus width) */
	if (attr_p->values.range.head->lower > dbmax - (width - 1))
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Range outside allowable values (Check ODM database)");
		*rc = E_BADATTR;
		adap_p->unresolved = TRUE;
		return;
	}
	if (attr_p->values.range.head->lower > platmax - (width - 1))
	{
		log_error(1, TRUE, adap_p->logname, attr_p->name,
		          "Range outside allowable values on this platform");
		*rc = E_BUSRESOURCE;
		adap_p->unresolved = TRUE;
		return;
	}

	/* Handle current value not within the given range */
	if (attr_p->current > attr_p->values.range.head->upper || 
	    attr_p->current < attr_p->values.range.head->lower   )
	{
		if (attr_p->user)
		{
			log_error(1, TRUE, adap_p->logname, attr_p->name,
			          "User value not within specified range (Check ODM database)");
			*rc = E_BADATTR;
			adap_p->unresolved = TRUE;
			return;
		}
		/* else */
			log_error(1, FALSE, adap_p->logname, attr_p->name,
			          "Default value not within specified range (Check ODM database)");
	}

	/* Handle current value not on incr boundry */
	if ((attr_p->current - attr_p->values.range.head->lower) % attr_p->values.range.head->incr)
	{
		if (attr_p->user)
		{
			log_error(1, TRUE, adap_p->logname, attr_p->name,
			          "User value not on an increment boundry (Check ODM database)");
			*rc = E_BADATTR;
			adap_p->unresolved = TRUE;
			return;
		}
		/* else */
			log_error(1, FALSE, adap_p->logname, attr_p->name,
		 	          "Default value not on an increment boundry (Check ODM database)");
	}

	/* Warn if the increment value is too large for the range */
	if (attr_p->values.range.head->incr > 
			attr_p->values.range.head->upper - attr_p->values.range.head->lower) 
		log_error(1, FALSE, adap_p->logname, attr_p->name,
		          "Increment value greater than range (Check ODM database)");

	/* Set upper and lower limits to the db limits */
	set_value_limits(attr_p, dbmin, dbmax);
	attr_p->values.range.head->upper = attr_p->valulim - (width - 1);
	attr_p->values.range.head->lower = attr_p->valllim;

	/* Warn if lower value less than platform minimum */
	/* (No warning for db minimum)                    */
	if (attr_p->values.range.head->lower < platmin)  
		log_error(1, FALSE, adap_p->logname, attr_p->name,
		          "Lower value too low on this platform");

	/* Warn if upper value greater than platform maximum */ 
	/* (No warning for db maximum)                       */
	if (attr_p->values.range.head->upper > platmax - (width - 1))
		log_error(1, FALSE, adap_p->logname, attr_p->name,
		          "Upper value too high on this platform");

	/* Set upper and lower limits to the platform limits */
	set_value_limits(attr_p, platmin, platmax);

	/* Adjust upper value to upper limit - width */
	if (attr_p->values.range.head->upper != attr_p->valulim - (width - 1))
		attr_p->values.range.head->upper = attr_p->valulim - (width - 1);

	/* Adjust lower value to lower platform limit */
	if (attr_p->values.range.head->lower < attr_p->valllim)
		attr_p->values.range.head->lower = attr_p->valllim;

	/* Readjust current if not within platform limits or on increment boundry */
	if (attr_p->current > attr_p->values.range.head->upper                                    || 
	    attr_p->current < attr_p->values.range.head->lower                                    || 
	    (attr_p->current - attr_p->values.range.head->lower) % attr_p->values.range.head->incr  )
	{
		/* Modify current value... */
		attr_p->current = attr_p->valllim;
		/* ...set changed flag and save start value */ 
		attr_p->changed = TRUE;
		attr_p->start.range.value = attr_p->current;
	}

	return;
} /* end of verify_range() */

/***************************************************************************/
/* Function Name : SET_VALUE_LIMITS()                                      */
/* Function Type : Internal C function                                     */
/* Purpose       : Compute the overall minimum and maximum values for an   */
/*                 attribute and store values in attribute structure in the*/
/*                 valllim and valulim fields.                             */
/* Arguments     :                                                         */
/*  attr_p       - attribute for which to compute limits                   */
/*  min          - minimum allowable value for resource                    */
/*  max          - maximum allowable value for resource                    */
/*                                                                         */
/* Return Value  : None                                                    */
/* Note          : The valulim includes the width; that is to say that the */
/*                 valulim is the maximum value which attr_p->current may  */
/*                 take on plus the attribute's width, adjusted for the    */
/*                 domain of possible values for the resource type. For    */
/*                 example, the valulim would be the last byte of memory   */
/*                 used by a memory attribute if attr_p->current was at    */
/*                 its numerically largest valid setting.                  */
/*                                                                         */
/* Notes : Must be called only for range lists of one element!             */
/*                                                                         */
/***************************************************************************/
static void set_value_limits(attr_p, min, max)
	attribute_t *attr_p;
	unsigned long min, max;
{
	unsigned long valllim, valulim, wid = attr_p->width - 1;

	if (attr_p->reprsent == LIST)
	{
		unsigned long value;
		value_list_t *val_p;

		valllim = 0xffffffff, valulim = 0x00000000;

		for (val_p = attr_p->values.list.head ; val_p ; val_p = val_p->next)
		{
			value = val_p->value;
			if (value + wid >= value)
				valllim = ((value < valllim) ? value : valllim),
				value = value + wid,
				valulim = ((value > valulim) ? value : valulim);
		}
	}
	else /* attr_p->reprsent == RANGE */
	{
		unsigned long n, incr = attr_p->values.range.head->incr;

		valllim = attr_p->values.range.head->lower;
		valulim = attr_p->values.range.head->upper;

		/* Check and correct lower value below domain */
		if (valllim < min)
		{
			/* compute number of increments between min and llim */
			n = (min - valllim) / incr;

			/* if llim is not on a boundry, add 1 more increment */
			if ((min - valllim) % incr)
				n++;

			valllim = valllim + n * incr;
		}

		/* Check and correct upper value exceeding domain */
		if (max < valulim)
			valulim = max;
		if (max - valulim < wid)
			valulim = max - wid;

		/* Check and correct upper value not on boundry */
		if ((valulim - valllim) % incr)
		{
			n = (valulim - valllim) / incr; /* Compute int incs in range */
			valulim = valllim + n * incr;
		}

		valulim = valulim + wid; 
	}

	/* Set the upper and lower limits */
	attr_p->valllim = valllim;
	attr_p->valulim = valulim;

	return;
} /* end of set_value_limits() */

/***************************************************************************/
/* Function Name : DESTROY_LISTS()                                         */
/* Function Type : C function                                              */
/* Purpose       : Frees the adapter and/or attributes lists               */
/* Arguments     :                                                         */
/*  rc             - integer return code from cf.h                         */
/*  adapter_list   - pointer to pointer to head of the adapter list        */
/*  attribute_list - pointer to pointer to head of the attribute list      */
/*                                                                         */
/* Return Value  : Integer return code from cf.h :                         */
/*  E_OK          List(s) freed successfully.                              */
/*                                                                         */
/***************************************************************************/
int destroy_lists(adapter_list, attribute_list)
	adapter_t **adapter_list;
	attribute_t **attribute_list;
{
	int rc = E_OK;

	if (adapter_list != (adapter_t **)NULL && 
	    *adapter_list != (adapter_t *)NULL   )   
	{
		destroy_adapter_list(&rc, *adapter_list);
		*adapter_list = (adapter_t *)NULL;
	}
	if (attribute_list != (attribute_t **)NULL &&
	    *attribute_list != (attribute_t *)NULL   )
		*attribute_list = (attribute_t *)NULL;

	return rc;
} /* end destroy_lists() */

/***************************************************************************/
/* Function Name : DESTROY_ADAPTER_LIST()                                  */
/* Function Type : Internal C function                                     */
/* Purpose       : Frees all storage associated with the adapter list.     */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  adap_p       - pointer to head of adapter list                         */
/*                                                                         */
/* Return Value  : None                                                    */
/* Notes         : 1) Recursion terminates when adapter->next is NULL.     */
/*                 2) This function frees the adapter's attributes list    */ 
/*                                                                         */
/***************************************************************************/
static void destroy_adapter_list(rc, adap_p)
	int *rc;
	adapter_t *adap_p;
{

	/* Walk down the list... */
	if (adap_p->next != (adapter_t *)NULL)
		destroy_adapter_list(rc, adap_p->next);

	/* ...free off an attributes list... */ 
	if (adap_p->attributes != NULL)
		destroy_attribute_list(rc, adap_p->attributes); 

	/* ...and free adapter nodes on the way back out */
	free(adap_p);

} /* end destroy_adapter_list() */

/***************************************************************************/
/* Function Name : DESTROY_ATTRIBUTE_LIST()                                */
/* Function Type : C function                                              */
/* Purpose       : Frees the entire attributes list.                       */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  attr_p       - pointer to head of attribute list to free               */
/*                                                                         */
/* Return Value  : None                                                    */
/* Notes         : Recursion terminates when next member is NULL.          */
/*                                                                         */
/***************************************************************************/
void destroy_attribute_list(rc, attr_p)
	int *rc;
	attribute_t *attr_p;
{

	/* Walk down the list... */
	if (attr_p->next) 
		destroy_attribute_list(rc, attr_p->next);

	/* ...and on the way back out... */

	if (attr_p->resource == GROUP)
		/* ...free a GROUP list... */
		destroy_group_list(rc, attr_p->group_ptr);

	else if (attr_p->reprsent == LIST)
		/* ...or free a values list... */
		destroyvlist(rc, attr_p->values.list.head);

	else /* attr_p->reprsent == RANGE */
		/* ...or free a range list... */
		destroyrlist(rc, attr_p->values.range.head);

	/* ...and free the attribute structure itself */
	free(attr_p);

	return;

} /* end destroy_attribute_list() */

/***************************************************************************/
/* Function Name : DESTROY_GROUP_LIST()                                    */
/* Function Type : Internal C function                                     */
/* Purpose       : Frees a GROUP attributes list.                          */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  attr_p       - pointer to head of GROUP attribute list to free         */
/*                                                                         */
/* Return Value  : None                                                    */
/* Notes         : 1) Recursion terminates when group_ptr member is NULL.  */
/*                 2) GROUP attributes are always LIST representation.     */ 
/*                                                                         */
/***************************************************************************/
static void destroy_group_list(rc, attr_p)
	int *rc;
	attribute_t *attr_p;
{

	/* Walk down the list... */
	if (attr_p->group_ptr) 
		destroy_group_list(rc, attr_p->group_ptr);

	/* ...free the values list... */
	destroyvlist(rc, attr_p->values.list.head);

	/* ...and free the attribute structures on the way back out */
	free(attr_p);

	return;

} /* end destroy_group_list() */

/***************************************************************************/
/* Function Name : DESTROYVLIST()                                          */
/* Function Type : C function                                              */
/* Purpose       : Free the unsigned long list                             */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  vlist_p      - pointer to list node                                    */
/*                                                                         */
/* Return Value  : None                                                    */
/* Notes         : Recursion terminates when next member is NULL or        */
/*                 points to the beginning of list.                        */
/*                                                                         */
/***************************************************************************/
static void freevlist();
static value_list_t *vhead_p;

void destroyvlist(rc, vlist_p)
	int *rc;
	value_list_t *vlist_p;
{
	vhead_p = vlist_p;
	freevlist(rc, vlist_p);

	return;
} /* end of destroyvlist() */

static void freevlist(rc, vlist_p)
	int *rc;
	value_list_t *vlist_p;
{

	/* Walk down the list... */
	if (vlist_p->next && vlist_p->next != vhead_p)
		freevlist(rc, vlist_p->next);

	/* ...and free nodes on the way back out */
	free(vlist_p);

	return;
} /* end of freevlist() */

/***************************************************************************/
/* Function Name : DESTROYRLIST()                                          */
/* Function Type : C function                                              */
/* Purpose       : Free the range list                                     */
/* Arguments     :                                                         */
/*  rc           - integer return code from cf.h                           */
/*  rlist_p      - pointer to range list node                              */
/*                                                                         */
/* Return Value  : None                                                    */
/* Notes         : Recursion terminates when next member is NULL or        */
/*                 points to the beginning of list.                        */
/*                                                                         */
/***************************************************************************/
static void freerlist();
static value_range_t *rhead_p;

void destroyrlist(rc, rlist_p)
	int *rc;
	value_range_t *rlist_p;
{
	rhead_p = rlist_p;
	freerlist(rc, rlist_p);

	return;
} /* end of destroyrlist() */

static void freerlist(rc, rlist_p)
	int *rc;
	value_range_t *rlist_p;
{

	/* Walk down the list... */
	if (rlist_p->next && rlist_p->next != rhead_p)
		freerlist(rc, rlist_p->next);

	/* ...and free nodes on the way back out */
	free(rlist_p);

	return;
} /* end of freerlist() */

/***************************************************************************/
/* Function Name : SHARE_ALGORITHM_CB_COMMON()                             */
/* Function Type : Callback C function                                     */
/* Purpose       : This callback function provides the default algorithm   */
/*                 to share interrupt attribute values.                    */ 
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h (reserved)           */
/*  inttbl         - Pointer to the interrupt table                        */
/*  intsiz         - Number of entries in the interrupt table              */
/*  attr_p         - Pointer to the attribute struct                       */
/*  index_p        - Interrupt table index to share (returned)             */
/*  found_p        - Boolean suitable index found (returned)               */
/*                                                                         */
/* Return Value  : None                                                    */
/* Notes         : This routine attempts to distribute the interrupts to a */
/*                 degree, so that they are not all assigned to the first  */
/*                 available sharable interrupt level found.               */
/*                                                                         */
/***************************************************************************/
static void share_algorithm_cb_common(rc, inttbl, intsiz, attr_p, index_p, found_p)
	int *rc;
	inttbl_t *inttbl;
	int intsiz;
	attribute_t *attr_p;
	unsigned long *index_p, *found_p;
{
	inttbl_t *inttbl_p;
	unsigned i, j, minused = 0xffffffff;

	/*-------------------------------------------------------*/
	/* Search for shareable (and unassigned) bus interrupts  */
	/* Note that INTLVL attributes which are members of a    */
	/* GROUP must have been assigned previously, they cannot */
	/* be assigned here.                                     */
	/*-------------------------------------------------------*/
	if (attr_p->reprsent == LIST)
	{
		value_list_t *val_p = attr_p->values.list.head; 

		for ( ; val_p ; val_p = val_p->next)
		{
			for (inttbl_p = inttbl, i = 0, j = intsiz ; j-- ; inttbl_p++, i++)
			{
				if (inttbl_p->intnumber == val_p->value     &&
				    inttbl_p->priority  == attr_p->priority &&
				    inttbl_p->usecount  <  minused             )
				{
					minused = inttbl_p->usecount + 1;
					*index_p = i;
					*found_p = TRUE;
				}
			}
		}
	}
	else /* attr_p->reprsent == RANGE */
	{
		value_range_t *val_p = attr_p->values.range.head;
		unsigned long value, incr;

		for (val_p = attr_p->values.range.head ; val_p ; val_p = val_p->next)
		{

			value = val_p->lower;
			incr  = val_p->incr; 

			for ( ; value <= val_p->upper ; value += incr )
			{
				for (inttbl_p = inttbl, i = 0, j = intsiz ; j-- ; inttbl_p++, i++)
				{
					if (inttbl_p->intnumber == value            &&
					    inttbl_p->priority  == attr_p->priority &&
					    inttbl_p->usecount  <  minused             )
					{
						minused = inttbl_p->usecount + 1;
						*index_p = i;
						*found_p = TRUE;
					}
				}
			}
		}
	}

	return;
} /* end of share_algorithm_cb_common() */

/***************************************************************************/
/* Function Name : SHARE_ALGORITHM_CB_234()                                */
/* Function Type : Callback C function                                     */
/* Purpose       : This callback function provides the algorithm to share  */ 
/*                 a interrupt attribute values with priority classes 2,   */
/*                 3, or 4.                                                */  
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h (reserved)           */
/*  inttbl         - Pointer to the interrupt table                        */
/*  intsiz         - Number of entries in the interrupt table              */
/*  attr_p         - Pointer to the attribute struct                       */
/*  index_p        - Interrupt table index to be shared (returned)         */
/*  found_p        - Boolean suitable index_p found_p                      */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void share_algorithm_cb_234(rc, inttbl, intsiz, attr_p, index_p, found_p)
	int *rc;
	inttbl_t *inttbl;
	int intsiz;
	attribute_t *attr_p;
	unsigned long *index_p, *found_p;
{
	inttbl_t *inttbl_p;
	unsigned i, j, minused = 0xffffffff;

	/*-------------------------------------------------------*/
	/* Search for shareable (and unassigned) bus interrupts  */
	/* Note that INTLVL attributes which are members of a    */
	/* GROUP must have been assigned previously, they cannot */
	/* be assigned here.                                     */
	/*-------------------------------------------------------*/
	if (attr_p->reprsent == LIST)
	{
		value_list_t *val_p = attr_p->values.list.head; 

		for ( ; val_p ; val_p = val_p->next)
		{
			for (inttbl_p = inttbl, i = 0, j = intsiz ; j-- ; inttbl_p++, i++)
			{
				if (inttbl_p->intnumber == val_p->value     &&
				    inttbl_p->usecount  <  minused             )
				{
					if (inttbl_p->priority == attr_p->priority            || 
					    (inttbl_p->priority != 1 && attr_p->priority != 1)  )
					{
						minused = inttbl_p->usecount + 1;
						*index_p = i;
						*found_p = TRUE;
					}
				}
			}
		}
	}
	else /* attr_p->reprsent == RANGE */
	{
		value_range_t *val_p = attr_p->values.range.head;
		unsigned long value, incr;

		for (val_p = attr_p->values.range.head ; val_p ; val_p = val_p->next)
		{

			value = val_p->lower;
			incr  = val_p->incr; 

			for ( ; value <= val_p->upper ; value += incr )
			{
				for (inttbl_p = inttbl, i = 0, j = intsiz ; j-- ; inttbl_p++, i++)
				{
					if (inttbl_p->intnumber == value            &&
					    inttbl_p->usecount  <  minused             )
					{
						if (inttbl_p->priority == attr_p->priority            || 
						    (inttbl_p->priority != 1 && attr_p->priority != 1)  )
						{
							minused = inttbl_p->usecount + 1;
							*index_p = i;
							*found_p = TRUE;
						}
					}
				}
			}
		}
	}

	return;
} /* end of share_algorithm_cb_234() */

/***************************************************************************/
/* Function Name : GET_BAX_AND_INVERT()                                    */
/* Function Type : C function                                              */
/* Purpose       : Gets bridge address translation descriptors and the     */
/*                 inverted bridge address translation descriptors for     */
/*                 the specified adapter. "Inverted" bridge translation    */
/*                 descriptors are descriptors which describe where bus    */
/*                 resource allocations may NOT be made.                   */
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h                      */
/*  adap_p         - Pointer to adapter to be adjusted                     */
/*  num            - Number of regular bax packets                         */
/*  pack           - Regular bax packets                                   */
/*  numr           - Number of inverted bax packets                        */
/*  packr          - Inverted bax packets                                  */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void get_bax_and_invert(rc, adap_p, num, pack, numr, packr)
	int *rc;
	adapter_t *adap_p;
	int *num;
	CFG_bax_descriptor_t **pack;
	int *numr;
	CFG_bax_descriptor_t **packr;
{
	CFG_bax_descriptor_t *pack_p, *pack_p2;
	int ocount, bcount, pcount, count;

	/* Get bridge address translation descriptors for parent bus */
	*rc = get_bax_descriptor(adap_p->parent_bus->resid_index, 'a', num, pack);
	if (*rc == E_MALLOC || *rc == E_DEVACCESS) /* Fatal errors */
		return;

	/*-----------------------------------------*/
	/* Malloc storage for inverted descriptors */
	/*-----------------------------------------*/

	/* Count descriptors */
	ocount = bcount = 0;
	for (pack_p = *pack, pcount = *num ; pcount-- ; pack_p++)
	{
		if (pack_p->conv == 1 /* bus mem */)
			bcount++;
		if (pack_p->conv == 2 /* bus I/O */)
			ocount++;
	}

	/* Malloc storage for maximum number of inverted descriptors */
	*numr = bcount + 1 + ocount + 1; /* Max number of descriptors needed */
	*packr = (CFG_bax_descriptor_t *)calloc(*numr, sizeof(CFG_bax_descriptor_t));
	if (*packr == NULL)
	{
		*rc = E_MALLOC;
		log_message(0, "get_bax_and_invert() : Out of virtual storage\n");
		return;
	}

	/*-----------------------------------*/
	/* Fill out the inverted descriptors */
	/*-----------------------------------*/

	/* Sort the descriptors in ascending order */
	sort_bax_descriptors(*pack, *num);

	/* Fill out the inverted bus mem packets */
	*numr = 0;
	pack_p2 = *packr;

	pack_p2->min = 0x00000000;
	count        = 0;
	for (pack_p = *pack, pcount = *num ; pcount-- ; pack_p++)
	{
		if (pack_p->conv != 1 /* bus mem */)
			continue;

		if (pack_p->min == 0x00000000 && 
		    pack_p->max == 0xffffffff    )
			break;

		pack_p2->flags = pack_p->flags;
		pack_p2->type  = pack_p->type;
		pack_p2->conv  = pack_p->conv;

		if (pack_p->min == pack_p2->min)
			pack_p2->min = pack_p->max + 1;
		else 
		{
			pack_p2->max = pack_p->min - 1;
			pack_p2++;
			*numr = *numr + 1;
			pack_p2->flags = pack_p->flags;
			pack_p2->type  = pack_p->type;
			pack_p2->conv  = pack_p->conv;
			pack_p2->min   = pack_p->max + 1;
		}
		
		if (pack_p->max == 0xffffffff)
			break;

		if (++count == bcount)
		{
			pack_p2->max = 0xffffffff;
			pack_p2++;
			*numr = *numr + 1;
			break;
		}
	}

	/* Fill out the inverted bus I/O packets */
	pack_p2->min = 0x00000000;
	count        = 0;
	for (pack_p = *pack, pcount = *num ; pcount-- ; pack_p++)
	{
		if (pack_p->conv != 2 /* bus I/O */)
			continue;

		if (pack_p->min == 0x00000000 && 
		    pack_p->max == 0xffffffff    )
			break;

		pack_p2->flags = pack_p->flags;
		pack_p2->type  = pack_p->type;
		pack_p2->conv  = pack_p->conv;

		if (pack_p->min == pack_p2->min)
			pack_p2->min = pack_p->max + 1;
		else 
		{
			pack_p2->max = pack_p->min - 1;
			pack_p2++;
			*numr = *numr + 1;
			pack_p2->flags = pack_p->flags;
			pack_p2->type  = pack_p->type;
			pack_p2->conv  = pack_p->conv;
			pack_p2->min   = pack_p->max + 1;
		}
		
		if (pack_p->max == 0xffffffff)
			break;

		if (++count == ocount)
		{
			pack_p2->max = 0xffffffff;
			pack_p2++;
			*numr = *numr + 1;
			break;
		}
	}

	return;
} /* end of get_bax_and_invert() */

/***************************************************************************/
/* Function Name : SORT_BAX_DESCRIPTORS()                                  */
/* Function Type : Internal C function                                     */
/* Purpose       : This function sorts the bridge address translation      */
/*                 descriptors into ascending order by their starting      */
/*                 address. Each address conversion type is ordered        */
/*                 by starting address, but the table is not sorted by     */
/*                 conversion type.                                        */
/* Arguments     :                                                         */
/*  packets        - Bus address translation packets                       */
/*  numpackets     - Number of bus address translation packets             */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void sort_bax_descriptors(packets, numpackets)
	CFG_bax_descriptor_t *packets;
	int numpackets;
{
	CFG_bax_descriptor_t temp;
	int i, j;

	for (i = 0 ; i < numpackets - 1 ; i++)
		for (j = i + 1; j < numpackets ; j++)
		{
			if (packets[j].conv != packets[i].conv)
				continue;

			if (packets[i].min > packets[j].min)
			{
				memcpy(&temp, &(packets[j]), sizeof(CFG_bax_descriptor_t));
				memcpy(&(packets[j]), &(packets[i]), sizeof(CFG_bax_descriptor_t));
				memcpy(&(packets[i]), &temp, sizeof(CFG_bax_descriptor_t));
			}
		}

	return;
} /* end of sort_bax_descriptors() */

/***************************************************************************/
/* Function Name : RESERVE_BAX_RANGE()                                     */
/* Function Type : C function                                              */
/* Purpose       : This function adjusts bus memory and I/O attributes to  */
/*                 accomodate the ranges specified by the input packets.   */
/*                 Note that the input packets describe ranges which       */
/*                 CANNOT be used.                                         */
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h (reserved)           */
/*  adap_p         - Pointer to adapter to be adjusted                     */
/*  bax            - Packets describing where NOT to allocate resource     */
/*  numbax         - Number of packets                                     */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void reserve_bax_range(rc, adap_p, bax, numbax)
	int *rc;
	adapter_t *adap_p;
	CFG_bax_descriptor_t *bax;
	int numbax;
{
	attribute_t *attr_p = adap_p->attributes;
	CFG_bax_descriptor_t *bax_p;
	int num;
	char *reason_str = "processing platform bus bridge descriptors";

	/* If this device is a bus, return */
	if (!strncmp(adap_p->utype, "bus/", strlen("bus/")))
		return;

	/* Loop through the adapter's attributes, looking for type M/B/O */
	for ( ; attr_p && attr_p->adapter_ptr == adap_p ; attr_p = attr_p->next)
	{

		if (attr_p->resource != ADDR && attr_p->resource != IOADDR)
			continue;

		/* Find correct packet and reserve range */
		for (bax_p = bax, num = numbax ; num-- ; bax_p++)
		{

			/* If this packet is negative decode, don't have to exclude it */
			if (!(bax_p->flags & 0x00000001))
				continue;

			switch (attr_p->resource)
			{
				case ADDR   : 
					if (bax_p->conv == 1 /* bus mem */)
						reserve_range(rc, attr_p, bax_p->min, bax_p->max, reason_str);
					break;
				case IOADDR : 
					if (bax_p->conv == 2 /* bus I/O */)
						reserve_range(rc, attr_p, bax_p->min, bax_p->max, reason_str);
			}
		}
	}

	return;
} /* end of reserve_bax_range() */

/***************************************************************************/
/* Function Name : RESERVE_RANGE()                                         */
/* Function Type : Internal C function                                     */
/* Purpose       : This function removes the input range specified by the  */
/*                 min and max arguments from the possible values of the   */
/*                 supplied attribute.                                     */
/* Arguments     :                                                         */
/*  rc             - Pointer to return code from cf.h                      */
/*  attr_p         - Pointer to attribute to be adjusted                   */
/*  min            - Minimum value of range to be excluded                 */
/*  max            - Maximum value of range to be excluded                 */
/*  reason_str     - Log string describing reason for value elim or adjust */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
void reserve_range(rc, attr_p, min, max, reason_str)
	int *rc;
	attribute_t *attr_p;
	unsigned long min, max;
	char *reason_str;
{
	char str[256];
	
	if (attr_p->reprsent == LIST) /* Process a value list */
	{	
		value_list_t *val_p = attr_p->values.list.head;
		int i = 0;

		/* Find and eliminate any elements in conflict */
		for ( ; val_p ; )
		{

			if (val_p->value > max || val_p->value + attr_p->width - 1 < min)
			{
				/* No conflict, increment loop invariant and continue */
				i++;
				val_p = val_p->next;
				continue;
			}

			/* Log message */
			sprintf(str, "%s%s", "Adjusting values while ", reason_str);
			log_error(1, FALSE, attr_p->adapter_ptr->logname, attr_p->name, str);

			/* Eliminate the conflicting element */
			eliminate_value_list_element(attr_p, i);

			/* Restart the scan, val_p was just freed! */
			i = 0;  
			val_p = attr_p->values.list.head;
		}

		/* Fail if there are no values left in the value list */
		if (attr_p->values.list.head == (value_list_t *)NULL)
		{
			sprintf(str, "%s%s", "No valid values after ", reason_str);
			log_error(1, TRUE, attr_p->adapter_ptr->logname, attr_p->name, str);
			*rc = E_BUSRESOURCE;
			attr_p->adapter_ptr->unresolved = TRUE;
			return;
		}

		/* Set up current pointer and value */
		for (val_p = attr_p->values.list.head ; val_p ; val_p = val_p->next)
		{
			if (attr_p->current == val_p->value)
			{
				attr_p->values.list.current = val_p;
				break;
			}
		}
		if (val_p == NULL)
		{
			attr_p->values.list.current = attr_p->values.list.head;
			attr_p->current = attr_p->values.list.current->value;
			attr_p->changed = TRUE;
			attr_p->start.list.ptr = attr_p->values.list.current;
		}
	} /* end processing LIST type */

	else /* attr_p->reprsent == RANGE */
	{
		value_range_t *val_p = attr_p->values.range.head;

		/* Find and adjust or eliminate any elements in conflict */
		for ( ; val_p ; )
		{

			/* Element is OK */
			if (val_p->lower > max || val_p->upper + attr_p->width - 1 < min)
			{
				val_p = val_p->next;
				continue;
			}

			/*-------------------------------------------------------------*/
			/* The range list element conflicts with the given range to be */
			/* reserved. Determine how to rectify this conflict.           */
			/*-------------------------------------------------------------*/

			/* Element needs to be removed entirely */
			if (val_p->lower + attr_p->width - 1 >= min && val_p->upper <= max)
			{
				value_range_t *next = val_p->next;

				/* Log message */
				sprintf(str, "%s%s", "Adjusting values while ", reason_str);
				log_error(1, FALSE, attr_p->adapter_ptr->logname, attr_p->name, str);

				eliminate_range_list_element(attr_p, val_p);
				val_p = next;
				continue;
			}

			/* Element needs to be split into two elements */
			if (val_p->lower < min && val_p->upper + attr_p->width - 1 > max)
			{
				value_range_t *new, *next = val_p->next;			

				/* Log message */
				sprintf(str, "%s%s", "Adjusting values while ", reason_str);
				log_error(1, FALSE, attr_p->adapter_ptr->logname, attr_p->name, str);

				/* Insert new element */
				new = (value_range_t *)calloc(1, sizeof(value_range_t));
				if (new == (value_range_t *)NULL)
				{
					*rc = E_MALLOC;
					log_message(0, "reserve_range() : Out of virtual storage\n");
					return;
				}
				new->next   = val_p->next;
				new->upper  = val_p->upper;
				new->lower  = val_p->lower;
				new->incr   = val_p->incr;
				val_p->next = new;

				/* Adjust upper value of val_p */
				adjust_range_list_max(attr_p, val_p, min);
				
				/* Check to see if there is enough range to have at least 1 value */
				if ((val_p->upper + attr_p->width - 1) - val_p->lower < attr_p->width - 1)
					eliminate_range_list_element(attr_p, val_p);

				/* Adjust lower value of new */
				adjust_range_list_min(attr_p, new, max);

				/* Check to see if there is enough range to have at least 1 value */
				if ((new->upper + attr_p->width - 1) - new->lower < attr_p->width - 1)
					eliminate_range_list_element(attr_p, new);
				
				val_p = next;
				continue;
			}

			/* Element needs to have lower value adjusted */
			if (min <= val_p->lower && max >= val_p->lower)
			{

				/* Log message */
				sprintf(str, "%s%s", "Adjusting values while ", reason_str);
				log_error(1, FALSE, attr_p->adapter_ptr->logname, attr_p->name, str);

				adjust_range_list_min(attr_p, val_p, max);

				/* Check to see if there is enough range to have at least 1 value */
				if ((val_p->upper + attr_p->width - 1) - val_p->lower < attr_p->width - 1)
				{
					value_range_t *next = val_p->next;

					eliminate_range_list_element(attr_p, val_p);
					val_p = next;
					continue;
				}
				val_p = val_p->next;
				continue;
			}

			/* Element needs to have upper value adjusted */
			if (max >= val_p->upper + attr_p->width - 1 &&
			    min <= val_p->upper + attr_p->width - 1    )
			{

				/* Log message */
				sprintf(str, "%s%s", "Adjusting values while ", reason_str);
				log_error(1, FALSE, attr_p->adapter_ptr->logname, attr_p->name, str);

				adjust_range_list_max(attr_p, val_p, min);

				/* Check to see if there is enough range to have at least 1 value */
				if ((val_p->upper + attr_p->width - 1) - val_p->lower < attr_p->width - 1)
				{
					value_range_t *next = val_p->next;

					eliminate_range_list_element(attr_p, val_p);
					val_p = next;
					continue;
				}
				val_p = val_p->next;
				continue;
			}
		}

		/* Fail if there are no values left in the range list */
		if (attr_p->values.range.head == (value_range_t *)NULL)
		{
			sprintf(str, "%s%s", "No valid values after ", reason_str);
			log_error(1, TRUE, attr_p->adapter_ptr->logname, attr_p->name, str);
			*rc = E_BUSRESOURCE;
			attr_p->adapter_ptr->unresolved = TRUE;
			return;
		}

		/* Set up current pointer and value */
		for (val_p = attr_p->values.range.head ; val_p ; val_p = val_p->next)
		{
			if (attr_p->current >= val_p->lower && attr_p->current <= val_p->upper)
			{
				attr_p->values.range.current = val_p;
				break;
			}
		}
		if (val_p == NULL)
		{
			attr_p->values.range.current = attr_p->values.range.head;
			attr_p->current = attr_p->values.range.current->lower;
			attr_p->changed = TRUE;
			attr_p->start.range.value = attr_p->current;
		}
	} /* end processing RANGE type */

	return;
} /* end of reserve_range() */

/***************************************************************************/
/* Function Name : ELIMINATE_RANGE_LIST_ELEMENT()                          */
/* Function Type : Internal C function                                     */
/* Purpose       : This frees a range list element.                        */
/* Arguments     :                                                         */
/*  attr_p         - Pointer to attribute                                  */
/*  val_p          - Pointer to range list element                         */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void eliminate_range_list_element(attr_p, val_p)
	attribute_t *attr_p;
	value_range_t *val_p;
{
	value_range_t *prev = attr_p->values.range.head, *next = val_p->next;

	/* Relink list */
	if (prev == val_p)
		attr_p->values.range.head = next;
	else
	{
		for ( ; prev ; prev = prev->next)
			if (prev->next == val_p)
				break;
		
		prev->next = next;
	}

	/* Free element */
	free(val_p);
	
	return;
} /* end of eliminate_range_list_element() */

/***************************************************************************/
/* Function Name : ADJUST_RANGE_LIST_MIN()                                 */
/* Function Type : Internal C function                                     */
/* Purpose       : Adjust a range list element minimum value.              */
/* Arguments     :                                                         */
/*  attr_p         - Pointer to attribute                                  */
/*  val_p          - Pointer to range list element                         */
/*  min            - Minium non-conflicting value                          */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void adjust_range_list_min(attr_p, val_p, min)
	attribute_t *attr_p;
	value_range_t *val_p;
	unsigned long min;
{
	unsigned long n;

	/* Compute number of increments between lower and (min + 1) */
	n = ((min + 1) - val_p->lower) / val_p->incr;

	/* If computed address is not on a boundry, add 1 more increment */
	if (((min + 1) - val_p->lower) % val_p->incr)
		n++;

	/* Set range element lower value */ 
	val_p->lower = val_p->lower + n * val_p->incr;

	return;
} /* end of adjust_range_list_min() */

/***************************************************************************/
/* Function Name : ADJUST_RANGE_LIST_MAX()                                 */
/* Function Type : Internal C function                                     */
/* Purpose       : Adjust a range list element maximum value.              */
/* Arguments     :                                                         */
/*  attr_p         - Pointer to attribute                                  */
/*  val_p          - Pointer to range list element                         */
/*  max            - Maximum non-conflicting value                         */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void adjust_range_list_max(attr_p, val_p, max)
	attribute_t *attr_p;
	value_range_t *val_p;
	unsigned long max;
{
	unsigned long n;

	/* Set upper value not conflicting with max */
	val_p->upper = max - attr_p->width;

	/* Check and correct upper value not on boundry */
	if ((val_p->upper - val_p->lower) % val_p->incr)
	{
		n = (val_p->upper - val_p->lower) / val_p->incr;
		val_p->upper = val_p->lower + n * val_p->incr;
	}

	return;
} /* end of adjust_range_list_max() */

/***************************************************************************/
/*              RESIDUAL DATA BUS RESOURCE PACKET FUNCTIONS                */
/***************************************************************************/

static int ResidAttrCount;

/***************************************************************************/
/* Function Name : BUILD_RESID_ATTRIBUTE_LIST()                            */
/* Function Type : Internal C function                                     */
/* Purpose       : Creates an attribute list attached to adap_p for the    */
/*                 device in the CFG_DEVICE array at the index supplied.   */
/* Arguments     :                                                         */ 
/*  index        - Index into CFG_DEVICE array                             */
/*  adap_p       - adapter_t structure pointer                             */
/*  pid          - PNP ID / resid device index string                      */
/*                                                                         */
/* Return Value  : return code from cf.h                                   */
/*                                                                         */
/***************************************************************************/
int build_resid_attribute_list(index, adap_p, pid)
	unsigned long index;
	adapter_t *adap_p;
	char *pid;
{
	int rc = E_OK, num, tristate = TRUE;
	CFG_irqpack_t *irqp;
	CFG_dmapack_t *dmap;
	CFG_iopack_t *iop;
	CFG_mempack_t *memp;

	ResidAttrCount = 0;

	if (adap_p->resid_index     != NO_RESID_INDEX &&
	    adap_p->resid_dev_flags &  NO_TRI_STATE      )
		tristate = FALSE;

	/*--------------------------*/
	/* Add interrupt attributes */
	/*--------------------------*/
	if (!tristate) /* If tristated, don't add to list */ 
	{
		rc = get_irq_packets(index, 'a', &num, &irqp);
		if (rc == E_DEVACCESS || rc == E_MALLOC)
			return rc;
		if (num > 0)
		{
			rc = add_resid_attribute(irqp, num, IRQPACKS, adap_p, pid);
			free(irqp);
			if (rc != E_OK)
				return rc;
		}
	}

	/*--------------------------*/
	/* Add dma level attributes */
	/*--------------------------*/
	if (!tristate) /* If tristated, don't add to list */
	{
		rc = get_dma_packets(index, 'a', &num, &dmap);
		if (rc == E_DEVACCESS || rc == E_MALLOC)
			return rc;
		if (num > 0)
		{
			rc = add_resid_attribute(dmap, num, DMAPACKS, adap_p, pid);
			free(dmap);
			if (rc != E_OK)
				return rc;
		}
	}

	/*------------------------*/
	/* Add bus I/O attributes */
	/*------------------------*/
	rc = get_io_packets(index, 'a', &num, &iop);
	if (rc == E_DEVACCESS || rc == E_MALLOC)
		return rc;
	if (num > 0)
	{
		rc = add_resid_attribute(iop, num, IOPACKS, adap_p, pid);
		free(iop);
		if (rc != E_OK)
			return rc;
	}

	/*---------------------------*/
	/* Add bus memory attributes */
	/*---------------------------*/
	rc = get_mem_packets(index, 'a', &num, &memp);
	if (rc == E_DEVACCESS || rc == E_MALLOC)
		return rc;
	if (num > 0)
	{
		rc = add_resid_attribute(memp, num, MEMPACKS, adap_p, pid);
		free(memp);
		if (rc != E_OK)
			return rc;
	}

	return rc;
} /* end of build_resid_attribute_list() */

/***************************************************************************/
/* Function Name : ADD_RESID_ATTRIBUTE()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Scans an array of packets and creates attributes which  */
/*                 are attached to adap_p for each packet.                 */
/* Arguments     :                                                         */ 
/*  pack         - Generic pointer                                         */
/*  numpack      - Number of packets                                       */
/*  tag          - Identifies packet type                                  */
/*  adap_p       - adapter_t structure pointer                             */
/*  pid          - PNP ID / resid device index string                      */
/*                                                                         */
/* Return Value  : return code from cf.h                                   */
/*                                                                         */
/***************************************************************************/
static int add_resid_attribute(pack, numpack, tag, adap_p, pid)
	void *pack;
	int numpack;
	int tag;
	adapter_t *adap_p;
	char *pid;
{
	unsigned long j;
	CFG_irqpack_t *irqpack = (CFG_irqpack_t *)pack;
	CFG_dmapack_t *dmapack = (CFG_dmapack_t *)pack;
	CFG_iopack_t  *iopack  = (CFG_iopack_t *)pack;
	CFG_mempack_t *mempack = (CFG_mempack_t *)pack;
	int rc = E_OK;
	attribute_t *attr_p;
	char attr_count_str[4];
	char attr_name[ATTRNAMESIZE + 1];
	bus_resource_e resrc;
	unsigned long val, wid, bll, bul, addrmask;

	/* Add an attribute for each packet */ 
	for ( j = numpack ; j-- ; )
	{

		/* Process packet fields to create attribute */

		switch (tag)
		{
			case IRQPACKS : 
				if (irqpack[j].flags == HIGH_EDGE || irqpack[j].flags == LOW_EDGE)
					resrc = NSINTLVL;
				else /* irqpack[j].flags == HIGH_LEVEL || irqpack[j].flags == LOW_LEVEL */
					resrc = INTLVL;
				val   = irqpack[j].value + (irqpack[j].inttype << 24) + (irqpack[j].intctlr << 16);
				wid   = 1;
				bll   = irqpack[j].value + (irqpack[j].inttype << 24) + (irqpack[j].intctlr << 16);
				bul   = irqpack[j].value + (irqpack[j].inttype << 24) + (irqpack[j].intctlr << 16);
				break;
			case DMAPACKS :
				resrc = DMALVL;
				val   = dmapack[j].value;
				wid   = 1;
				bll   = dmapack[j].value;
				bul   = dmapack[j].value;
				break;
			case IOPACKS  :
				resrc   = IOADDR;
				val     = iopack[j].min; 
				wid     = iopack[j].width;
				bll     = iopack[j].min;
				bul     = iopack[j].min + (iopack[j].width - 1);
				addrmask = 0xffffffff >> (32 - iopack[j].sigbits);
				break;
			case MEMPACKS :
				resrc   = BADDR;
				val     = mempack[j].min;
				wid     = mempack[j].width; 
				bll     = mempack[j].min; 
				bul     = mempack[j].min + (mempack[j].width - 1);
				addrmask = 0xffffffff >> (32 - mempack[j].sigbits);
				break;
			default       :
				log_message(0, "--- INTERNAL ERROR --- ADD_RESID_ATTRIBUTE()\n");
				continue;
		}

		/* Create attribute name by concatenating attribute count to pid string */
		sprintf(attr_count_str, "%d", ResidAttrCount);
		strcpy(attr_name, pid);
		strcat(attr_name, attr_count_str);
		ResidAttrCount++;

		/* Add attribute to adap_p's attribute list */
		attr_p = new_attribute(&rc, adap_p, NULL, NULL);
		if (rc == E_MALLOC)
			return rc;

		setup_list_attribute(&rc, adap_p, attr_p, attr_name, resrc, val, wid, bll, bul);
		if (rc == E_MALLOC)
			return rc;

		switch (tag)
		{
			case IRQPACKS : 
				if (irqpack[j].flags == HIGH_EDGE || irqpack[j].flags == LOW_EDGE)
					attr_p->trigger  = EDGE;
				else /* irqpack[j].flags == HIGH_LEVEL || irqpack[j].flags == LOW_LEVEL */
					attr_p->trigger  = LEVEL;
			case DMAPACKS :
				attr_p->addrmask = -1; /* for IRQPACKS too! */
				break;
			case IOPACKS  :
			case MEMPACKS :
				/* Issue log message for incorrect significant bits masking */
				if (val + (wid - 1) > addrmask)
				{
					log_error(1, TRUE, "Resid Data", attr_name,
					          "Specified value and width not under significant bits mask");
					adap_p->unresolved = TRUE;
					rc = E_BADATTR;
					continue;
				}
				attr_p->addrmask = addrmask;
		}
		

	} /* end for() looping through packets */

	return rc;
} /* end of add_resid_attribute() */


