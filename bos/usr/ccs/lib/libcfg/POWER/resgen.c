static char sccsid[] = "@(#)48  1.24.1.13  src/bos/usr/ccs/lib/libcfg/POWER/resgen.c, libcfg, bos41J, 9520A_all 5/8/95 09:52:29";
/*
 *	 COMPONENT_NAME: (LIBCFG) RESOLVE BUS RESOURCES Module
 *
 *	 FUNCTIONS: 
 *		resolve_lists
 *
 *	 ORIGINS: 27
 *
 *	 IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *	 combined with the aggregated modules for this product)
 *	                  SOURCE MATERIALS
 *
 *	 (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *	 All Rights Reserved
 *	 US Government Users Restricted Rights - Use, duplication or
 *	 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <memory.h> 
#include <cf.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#include "adapter.h"
#include "bt.h"
#include "pt_extern.h"

/*------------------*/
/* Global variables */
/*------------------*/

/* Pointers into attribute list */
static attribute_t *AttrHead, *AttrTop, *AttrBot, *DefinedAttrHead;

/* Resolve_conflict() attributes stack */
static unsigned int UAttrsCount;
static attribute_t **UAttrsStack;

/* Boolean flag for shareable interrupts */
static int ShareInterrupt = FALSE;

/*----------------------*/
/* Forward declarations */
/*----------------------*/

int resolve_lists(adapter_t **, attribute_t **);
static int resolve_adapter(adapter_t *); 
static int sufficient_resource(attribute_t *);
static int terminate_adapter(attribute_t *);
static attribute_t *resolve_conflict(attribute_t *);
static attribute_t *detect_conflict(attribute_t *);
static int is_candidate(attribute_t *);
static int increment_value(attribute_t *, attribute_t *);
static int increment_list(attribute_t *, attribute_t *);
static int increment_range(attribute_t *, attribute_t *);
static void save_adapter_state(adapter_t *, adapter_t *, statedata_t *);
static void restore_adapter_state(adapter_t *, adapter_t *, statedata_t *);
static void assign_share_interrupts(int *);

/***************************************************************************/
/* Function Name : RESOLVE_LISTS()                                         */
/* Function Type : External C function                                     */
/* Purpose       : Resolves bus resource attributes for all bus attached   */
/*                 devices.                                                */
/* Global Vars   :                                                         */
/*   AttrHead        - Head of attribute list                              */
/*   DefinedAttrHead - Head of DEFINED adapter attributes                  */
/* Arguments     :                                                         */ 
/*  adapter_list   - returned pointer to head of the adapter list          */
/*  attribute_list - returned pointer to head of the attribute list        */
/*                                                                         */
/* Return Value  : Integer return code from cf.h :                         */
/*  E_OK           All bus resources resolved successfully                 */
/*  E_MALLOC       Virtual storage exhausted                               */
/*  E_BUSRESOURCE  An unresolved bus resource conflict detected            */
/*                                                                         */
/***************************************************************************/
int resolve_lists(adapter_list, attribute_list)
	adapter_t **adapter_list;
	attribute_t **attribute_list;
{
	adapter_t *adap_p, *adap_p2;
	int rc = E_OK, attrcount;
	statedata_t *state;
	attribute_t *attr_p, *attr_p2;

	/*----------------------------------------------------------*/
	/* Loop through the attributes list and count the number of */ 
	/* attributes for this configuration. Use attr_p2 to handle */
	/* GROUP attributes, counting the GROUP and all members.    */
	/*----------------------------------------------------------*/
	for (attrcount = 0, attr_p = *attribute_list ; attr_p ; attr_p = attr_p->next)
	{
		++attrcount;
		if (attr_p->resource == GROUP)
		{
			for (attr_p2 = attr_p->group_ptr ; attr_p2 ; attr_p2 = attr_p2->group_ptr)
				++attrcount;
		}
	}

	/* If there are no attributes we are done. Also prevent malloc errors */
	if (attrcount == 0)
		return E_OK;

	/* Allocate storage for attribute state data */
	state = (statedata_t *)calloc(attrcount, sizeof(statedata_t));
	if (state == NULL)
	{
		log_message(0, "resolve_lists() : Out of virtual storage\n");
		return E_MALLOC;
	}

	/* Allocate storage for the attribute stack */
	UAttrsStack = (attribute_t **)calloc(attrcount, sizeof(attribute_t *));
	if (UAttrsStack == NULL)
	{
		log_message(0, "resolve_lists() : Out of virtual storage\n");
		free(state);
		return E_MALLOC;
	}

	/* Set global AttrHead, DefinedAttrHead */
	AttrHead = DefinedAttrHead = *attribute_list;

	/*---------------------------------------------------------------*/
	/* Loop through all adapters and resolve bus resources for each. */
	/*---------------------------------------------------------------*/
	for (adap_p = *adapter_list ; adap_p ; adap_p = adap_p->next)
	{

		/* Skip devices already marked unresolved */
		if (adap_p->unresolved)
			continue;

		/* Save the state of the current resources before perturbing */
		save_adapter_state(*adapter_list, adap_p, state);		

		/* Log a message for this device */
		log_message(0, "\nEntering resolve_adapter() for device %s :\n", adap_p->logname);

		/* Attempt to resove the device's sttributes */	
		if (!resolve_adapter(adap_p))
		{
			rc = E_BUSRESOURCE;
			restore_adapter_state(*adapter_list, adap_p, state);
			adap_p->unresolved = TRUE;
		}

		/* Process any shared attributes that may have changed */
		for (adap_p2 = *adapter_list ; adap_p2 != adap_p->next ; adap_p2 = adap_p2->next)
		{
			if (adap_p2->unresolved)
				continue;

			for (attr_p = adap_p2->attributes ; 
			              attr_p && attr_p->adapter_ptr == adap_p2 ; 
			                         attr_p = attr_p->next           )
			{
				if (!attr_p->share_head)
					continue;
	
				/* Walk down shared with list */
				for (attr_p2 = attr_p->share_ptr ; attr_p2 ; attr_p2 = attr_p2->share_ptr)
				{
					/* Any attrs w/current value != head current value are 'changed' */
					if (attr_p2->current != attr_p->current)
					{
						/* set initial value (this is done on 1st increment, be consistent) */
						if (!attr_p2->changed)
						{
							if (attr_p2->reprsent == LIST)
								attr_p2->start.list.ptr = attr_p2->values.list.head;
							else /* attr_p2->reprsent == RANGE */
								attr_p2->start.range.value = attr_p2->current;
						}

						attr_p2->changed = TRUE; /* ensures CuAt is updated in br.c */
	
						/* set current value (to that of the head) */
						attr_p2->current = attr_p->current;
					}
				} /* end for () walking down share with list */
			} /* end for () walking down attr list */
		} /* end for () processing subsequent shared attributes */
	
		/* Log the current state of all processed devices to the logfile */
		log_message(0, "Returned from resolve_adapter() for device %s :\n", adap_p->logname);
		log_resource_summary(1, *attribute_list, adap_p, INTERIM);
	}

	/*--------------------------------------------*/
	/* Attempt to assign the shareable interrupts */
	/*--------------------------------------------*/
	if (ShareInterrupt)
	{
		log_message(0, "\nEntering assign_share_interrupts() :\n");
		assign_share_interrupts(&rc);
		log_message(0, "Returned from assign_share_interrupts() :\n");
	}

	/* Free temporary storage */
	free(UAttrsStack);
	free(state);

	/* Log summary of all assigned values after processing shared resources */
	log_message(0, "\nSummary of bus resource assignments :\n", adap_p->logname);
	log_resource_summary(1, *attribute_list, NULL, INTERIM);

	return rc; 
} /* end of resolve_lists() */

/***************************************************************************/
/* Function Name : RESOLVE_ADAPTER()                                       */
/* Function Type : Internal C function                                     */
/* Purpose       : Resolve all bus resources for a single device.          */
/* Global Vars   :                                                         */
/*   AttrTop         - Adapter's first attribute to be resolved.           */
/*   AttrBot         - Adapter's last attribute to be resolved.            */
/*   DefinedAttrHead - Head of DEFINED adapter attributes                  */
/*                                                                         */
/* Arguments     :                                                         */
/*   adap_resv   - Pointer to adapter to be resolved.                      */
/*                                                                         */
/* Return Value  : Boolean TRUE if resolved, FALSE otherwise.              */ 
/*                                                                         */
/***************************************************************************/
static int resolve_adapter(adap_resv)
	adapter_t *adap_resv;
{
	attribute_t *attr_p;

	/* If there are no attributes, then we're done with this device */
	if (adap_resv->attributes == (attribute_t *)NULL)
		return TRUE;

	/* Find the first and last attributes in the adapter's attribute list */
	AttrTop = adap_resv->attributes;
	for (AttrBot = AttrTop ; AttrBot->next ; AttrBot = AttrBot->next) 
		if (AttrBot->next->adapter_ptr != adap_resv) 
			break;

	/* If this device is not DEFINED, we must not modify it's attributes */
	if (adap_resv->status != DEFINED)
	{
		DefinedAttrHead = AttrBot->next;
		return TRUE;
	}

	/* See that there is sufficient resource to configure this device */
	for (attr_p = adap_resv->attributes ; attr_p ; attr_p = attr_p->next)
	{
		if (attr_p->adapter_ptr != adap_resv)
			break;

		if (!sufficient_resource(attr_p))	
		{
			if (terminate_adapter(attr_p))
			{
				log_message(0, "Insufficient resource for : Adapter %s Attribute %s\n",
				            attr_p->adapter_ptr->logname, attr_p->name);
				return FALSE;
			}
		}
	}

	/* Loop resolveing attribute conflicts */
	while (resolve_conflict(DefinedAttrHead))
	{
		/* The failing attribute is AttrTop */

		if (terminate_adapter(AttrTop))
		{
			log_message(0, "Failed to resolve attributes for : Adapter %s Attribute %s\n",
			            AttrTop->adapter_ptr->logname, AttrTop->name);
			return FALSE;
		}
	}

	return TRUE; 
} /* end of resolve_adapter() */

/***************************************************************************/
/* Function Name : SUFFICIENT_RESOURCE()                                   */
/* Function Type : Internal C function                                     */
/* Purpose       : Determine if there is sufficient resource to allocate   */
/*                 attr_curr's bus resource requirement.                   */
/* Global Vars   :                                                         */
/*  AttrHead     - Beginning of attribute list.                            */
/*  AttrTop      - Adapter's first attribute to be resolved.               */
/*  AttrBot      - Adapter's last attribute to be resolved.                */
/*                                                                         */
/* Arguments     :                                                         */
/*   attr_p      - Attribute to be tested. May be a GROUP attribute        */
/*                                                                         */
/* Return Value  : Boolean TRUE if there is sufficient resource to         */
/*                 allocate attr_curr's bus resource requirement.          */
/*                                                                         */
/* Note          : We keep track of two "domains" the "resource domain"    */
/*                 and the "bus domain". The resource domain keeps track   */
/*                 of all allocations for that type of attribute. The bus  */
/*                 domain keeps track of the allocations made within the   */
/*                 busllim / busulim range for the target attribute. This  */
/*                 approach allows us to handle MADDR/BADDR, the two PCI   */
/*                 I/O mem spaces, PCI/ISA bus memory, and NSINTLVL/INTLVL */
/*                 with less special case code and allowing bus specific   */
/*                 modules some control over namespaces that overlap or    */
/*                 do not overlap.                                         */
/*                                                                         */
/***************************************************************************/
static int sufficient_resource(attr_p)
	attribute_t *attr_p;
{
	attribute_t *attr_p1, *attr_p2, *attr_p3;
	unsigned long avail, bus_taken, resrc_taken;
	unsigned long busllim, busulim, resrcllim, resrculim;

	/*------------------------------------------------------*/
	/* Loop through attr_p (and handle group) using attr_p1 */
	/*------------------------------------------------------*/
	if (attr_p->resource == GROUP)
		attr_p1 = attr_p->group_ptr;
	else
		attr_p1 = attr_p;
	for ( ; attr_p1 ; attr_p1 = attr_p1->group_ptr)
	{

		/* Initialize local variables */
		bus_taken = resrc_taken = 0;
		busllim = resrcllim = attr_p1->busllim;
		busulim = resrculim = attr_p1->busulim;

		/*-----------------------------------------------------------------*/
		/* Find how much resource has been taken up by previous attributes */
		/*-----------------------------------------------------------------*/
		for (attr_p2 = AttrHead ; attr_p2 && attr_p2 != attr_p ; attr_p2 = attr_p2->next)
		{
		
			/*-------------------------------------------------------*/
			/* Loop through attr_p2 (and handle group) using attr_p3 */
			/*-------------------------------------------------------*/
			if (attr_p2->resource == GROUP)
				attr_p3 = attr_p2->group_ptr;
			else
				attr_p3 = attr_p2;
			for ( ; attr_p3 ; attr_p3 = attr_p3->group_ptr)
			{

				/* Skip attrs that aren't the same resource */
				if (attr_p3->resource != attr_p1->resource)
					continue;

				/* Skip "ignorable" attrs, terminated adapts, and shareable intrs */ 
				if (attr_p3->adapter_ptr->unresolved     ||
				    attr_p3->specific_resource == INTLVL ||
				    attr_p3->ignore                         )
					continue;

				/* If the resource domain expands, keep track of it */
				if (attr_p3->busllim < resrcllim)
					resrcllim = attr_p3->busllim;
				if (attr_p3->busulim > resrculim)
					resrculim = attr_p3->busulim;

				/* Count amount of resource used in bus domain */
				if (attr_p3->busllim >= busllim && attr_p3->busulim <= busulim)
					bus_taken += attr_p3->width;

				/* Count amount of resource used in resource domain */
				resrc_taken += attr_p3->width;

			}
		}

		/* Is there enough resource in bus domain to satisfy ? */
		avail = busulim - busllim; 
		avail = (avail + 1 > avail) ? avail + 1 : avail;

		if (avail < bus_taken)
			return FALSE;

		if (avail - bus_taken < attr_p1->width)
			return FALSE;

		/* Is there enough resource in resource domain to satisfy ? */
		avail = resrculim - resrcllim; 
		avail = (avail + 1 > avail) ? avail + 1 : avail;

		if (avail < resrc_taken)
			return FALSE;

		if (avail - resrc_taken < attr_p1->width)
			return FALSE;

	}

	return TRUE;
} /* end of sufficient_resource() */

/***************************************************************************/
/* Function Name : TERMINATE_ADAPTER                                       */
/* Function Type : Internal C function                                     */
/* Purpose       : Process an adapter whose attribute(s) apparently aren't */
/*                 resolvable                                              */ 
/* Arguments     :                                                         */
/*   attr_p      - Failing attribute.                                      */
/*                                                                         */
/* Return Value  : Boolean TRUE if adapter is terminated, FALSE if it      */
/*                 should be tried again.                                  */
/*                                                                         */
/***************************************************************************/
static int terminate_adapter(attr_p)
	attribute_t *attr_p;
{
	attribute_t *attr_p1;

	/* Postpone assignment of shareable interrupt levels */
	if (attr_p->specific_resource == INTLVL)
	{
		attr_p->specific_resource = SINTLVL;
		attr_p->ignore = TRUE;
		ShareInterrupt = TRUE;
		log_postpone(1, attr_p);
		return FALSE;
	}

	/* Process shared attributes */
	for (attr_p1 = AttrTop->adapter_ptr->attributes ; attr_p1 ; attr_p1 = attr_p1->next)
	{
		if (attr_p1->adapter_ptr != AttrTop->adapter_ptr)
			break;

		if (attr_p1->share_head && attr_p1->share_ptr != NULL)
		{
			attr_p1->share_ptr->share_head = TRUE;
			attr_p1->share_ptr->ignore = FALSE;
			attr_p1->share_head = FALSE;
		}
	}

	return TRUE;
} /* end of terminate_adapter() */

/***************************************************************************/
/* Function Name : DETECT_CONFLICT()                                       */
/* Function Type : Internal C function                                     */
/* Purpose       : Compare attr_p to each attribute in the attribute list  */
/*                 from AttrHead up to attr_p.                             */
/* Global Vars   :                                                         */
/*   AttrHead    - Beginning of attribute list.                            */
/*                                                                         */
/* Arguments     :                                                         */
/*   attr_p      - The attribute we are checking for a conflict with.      */
/*                 This attribute may be a GROUP.                          */
/*                                                                         */
/* Return Value  : Pointer to the first attribute in conflict, in a        */
/*                 forward scan of the attributes list. If there is no     */
/*                 conflict, then a NULL pointer is returned.              */ 
/*                                                                         */
/***************************************************************************/
static attribute_t *detect_conflict(attr_p)
	attribute_t *attr_p;
{
	attribute_t *attr_p1, *attr_p2, *attr_p3;
	unsigned long mask, bot1, top1, bot3, top3;

	/*------------------------------------------------------*/
	/* Loop through attr_p (and handle group) using attr_p1 */
	/*------------------------------------------------------*/
	if (attr_p->resource == GROUP)
		attr_p1 = attr_p->group_ptr;
	else 
		attr_p1 = attr_p;
	for ( ; attr_p1 != NULL ; attr_p1 = attr_p1->group_ptr)
	{
		
		/*-------------------------------------------------------------*/
		/* Loop through the attributes list from AttrHead up to attr_p */
		/* using attr_p2.                                              */
		/*-------------------------------------------------------------*/
		for (attr_p2 = AttrHead ; attr_p2 != attr_p ; attr_p2 = attr_p2->next)
		{

			if (attr_p2->adapter_ptr->unresolved || attr_p2->ignore)
				continue;

			/*-------------------------------------------------------*/
			/* Loop through attr_p2 (and handle group) using attr_p3 */
			/*-------------------------------------------------------*/
			if (attr_p2->resource == GROUP)
				attr_p3 = attr_p2->group_ptr;
			else
				attr_p3 = attr_p2;
			for ( ; attr_p3 != NULL ; attr_p3 = attr_p3->group_ptr)
			{

				/* Not the same type of resource */
				if (attr_p3->resource != attr_p1->resource)
					continue;

				/* Possible ranges don't overlap */
				if (attr_p3->busulim < attr_p1->busllim ||
				    attr_p3->busllim > attr_p1->busulim   )
					continue;

				/*-------------------------------------------------------------------------*/
				/* Check for significant bits masking; since all values not under the mask */
				/* are eliminated, if the masks are equivalent check values normally       */
				/*-------------------------------------------------------------------------*/

				if (attr_p1->addrmask != attr_p3->addrmask)
				{
					/* Get smallest mask value for addressibility */
					if (attr_p1->addrmask < attr_p2->addrmask)
						mask = attr_p1->addrmask;
					else 
						mask = attr_p2->addrmask;
	
					/* Either width greater than the mask */
					if (attr_p1->width - 1 > mask || attr_p3->width - 1 > mask)
						return attr_p3;
	
					/* Get attr_p1 bot/top values */
					bot1 = attr_p1->current;
					top1 = bot1 + attr_p1->width - 1;
	
					/* Get attr_p2 bot/top values */
					bot3 = attr_p3->current;
					top3 = bot3 + attr_p3->width - 1;
	
					if ((bot1 & mask) > (top1 & mask))
					{
						/* Case 3, both attr_p1 and attr_p3 span the end of mask window */
						if ((bot3 & mask) > (top3 & mask))
							return attr_p3;
						/* Case 1, attr_p1 spans the end of mask window */
						else 
						{
							/* Check bottom of mask window */
							bot1 = attr_p1->current & mask;
							top1 = mask;
							bot3 = attr_p3->current & mask;
							top3 = bot3 + attr_p3->width - 1;
							if (!(bot3 > top1 || top3 < bot1))
								return attr_p3;
	
							/* Check top of mask window */
							bot1 = 0;
							top1 = (attr_p1->current + attr_p1->width - 1) & mask;
							bot3 = attr_p3->current & mask;
							top3 = bot3 + attr_p3->width - 1;
							if (!(bot3 > top1 || top3 < bot1))
								return attr_p3;
						}
					}
					/* Case 2, attr_p3 spans the end of mask window */
					else if ((bot3 & mask) > (top3 & mask))
					{
						/* Check bottom of mask window */
						bot1 = attr_p1->current & mask;
						top1 = bot1 + attr_p1->width - 1;
						bot3 = attr_p3->current & mask;
						top3 = mask;
						if (!(bot3 > top1 || top3 < bot1))
							return attr_p3;
	
						/* Check top of mask window */
						bot1 = attr_p1->current & mask;
						top1 = bot1 + attr_p1->width - 1;
						bot3 = 0;
						top3 = (attr_p3->current + attr_p3->width - 1) & mask;
						if (!(bot3 > top1 || top3 < bot1))
							return attr_p3;
					}
					/* Case 4, attr_p1 and attr_p3 fall within mask window */
					else
					{
						/* Compute top / bot masked values */
						bot1 = attr_p1->current & mask;
						top1 = bot1 + attr_p1->width - 1;
						bot3 = attr_p3->current & mask;
						top3 = bot3 + attr_p3->width - 1;
	
						if (!(bot3 > top1 || top3 < bot1))
							return attr_p3;
					}
				}
				else /* Significant bit masks same, no masking needed */
				{
				bot1 = attr_p1->current;
				top1 = bot1 + attr_p1->width - 1;
				bot3 = attr_p3->current;
				top3 = bot3 + attr_p3->width - 1;
				if (!(bot3 > top1 || top3 < bot1))
					return attr_p3;
				}
			}
		}
	}

	return (attribute_t *)NULL;
} /* end of detect_conflict() */

/***************************************************************************/
/* Function Name : RESOLVE_CONFLICT()                                      */
/* Function Type : Internal C function                                     */
/* Purpose       : Resolve any conflicts for an adapter's attributes.      */ 
/* Global Vars   :                                                         */
/*   AttrTop     - Adapter's first attribute to be resolved.               */
/*   AttrBot     - Adapter's last attribute to be resolved.                */
/*   AttrHead    - Beginning of attribute list.                            */
/*   UAttrsStack - Stack of pointers to attribute being resolved and all   */
/*                 attributes in conflict.                                 */ 
/*   UAttrsCount - Valid array elements for UAttrsStack.                   */
/*                                                                         */
/* Arguments     :                                                         */
/*   attr_curr   - The current attribute at "this" level of recursion      */
/*                 Initially, this is the DefinedAttrHead in the attribute */
/*                 list.                                                   */ 
/*                                                                         */
/* Return Value  : The attribute being resolved, while an unresolved       */
/*                 conflict exists, otherwise NULL.                        */ 
/*                                                                         */
/***************************************************************************/
static attribute_t *resolve_conflict(attr_curr)
	attribute_t *attr_curr; 
{
	int i; 
	attribute_t *attr_resv; /* Attr being resolved */
	attribute_t *attr_conf; /* Attr in conflict reported by detect_conflict() */

	UAttrsCount = 0;

	if (attr_curr->adapter_ptr->unresolved || attr_curr->ignore)
	{
		if (attr_curr == AttrBot) /* We are done */
			return NULL;
		else /* Wind up the stack one level */
		{
			if (attr_curr == AttrTop) /* Proceed to next attribute */ 
				AttrTop = AttrTop->next;
			attr_resv = resolve_conflict(attr_curr->next);
			return attr_resv; 
		}
	}
 
	attr_resv = NULL;
	do
	{

		if ((attr_conf = detect_conflict(attr_curr)) == NULL)
		{

			/* No conflict at this level, log the OK value */
			log_ok_value(1, attr_curr);

			if (attr_curr == AttrBot) /* We are done */
				return (NULL);
			else /* Wind up the stack one level */
			{
				if (attr_curr == AttrTop) /* Proceed to next attribute */ 
					AttrTop = AttrTop->next;
				
				attr_resv = resolve_conflict(attr_curr->next);
				if (attr_resv)
				{

					/* See if this attribute may resolve the conflict */
					i = is_candidate(attr_curr);
					if (!i)
						return (attr_resv);

					/* Add the current attribute to the stack */
					UAttrsStack[UAttrsCount++] = attr_curr;
				}
				else
					return (NULL);
			}
		}

		/* Log the conflict detected */
		else
			log_conflict(1, attr_curr, attr_conf);

	} while (!increment_value(attr_curr, attr_conf)); 

	/* Falling through the do while() loop means attr_curr wrapped */

	/* Log the wrapped attribute */
	log_wrap(1, attr_curr);

	if (attr_resv)
		return (attr_resv);
	else
	{
		/* Add the attribute being resolved to the stack */
		UAttrsStack[UAttrsCount++] = attr_curr;
		return (attr_curr);
	}
} /* end of resolve_conflict() */

/***************************************************************************/
/* Function Name : IS_CANDIDATE()                                          */
/* Function Type : Internal C function                                     */
/* Purpose       : Determine if attr_curr is a candidate for resolving the */
/*                 attribute conflict.                                     */
/* Global Vars   :                                                         */
/*   UAttrsStack - Array of pointers to attribute being resolved and all   */
/*                 candidate attributes.                                   */
/*   UAttrsCount - Valid array elements for UAttrsStack.                   */
/*                                                                         */
/* Arguments     :                                                         */
/*   attr_curr   - The current attribute at "this" level of recursion.     */
/*                 This attribute may be a GROUP.                          */
/*                                                                         */
/* Return Value  : Boolean TRUE if attr_curr is a candidate to resolve     */
/*                 the conflict.                                           */ 
/* Notes :                                                                 */
/*                 An attribute is a candidate to resolve the conflict if  */ 
/*                 all of the following are true :                         */
/*                                                                         */
/*                     1) It's possible values are not identical to the    */
/*                        possible values of the last attribute on the top */
/*                        of UAttrsStack (last attribute wrapped).         */
/*                     2) It's bus resource and current value conflict     */
/*                        with any of the possible values for any of the   */
/*                        UAttrsStack attributes.                          */
/*                                                                         */
/***************************************************************************/
static int is_candidate(attr_curr)
	attribute_t *attr_curr;
{
	int i, identical;
	attribute_t **stack_pp, *top_stack_p, *attr_p1, *attr_p2;
	unsigned long top_p1;

	/*==========================================================================*/
	/* Check to see if attr_curr is "identical" to the last attribute wrapped.  */
	/* We discount the possibility that attr_curr might be a "subset" of the    */
	/* last attribute processed, or have equivalent effective possible values   */
	/* (different ordering, etc.).                                              */
	/*==========================================================================*/

	identical = TRUE; /* Assume they are identical, and try to disprove it */
	top_stack_p = UAttrsStack[UAttrsCount - 1];

	/*----------------------------------------------------*/
	/* Use attr_p1 and attr_p2 to handle GROUP attributes */
	/*----------------------------------------------------*/
	if (attr_curr->resource == GROUP)
	{
		if (top_stack_p->resource == GROUP)
		{
			attr_p1 = attr_curr->group_ptr;
			attr_p2 = top_stack_p->group_ptr;
		}
		else 
			identical = FALSE;
	}
	else /* attr_curr->resource != GROUP */
	{
		if (top_stack_p->resource == GROUP)
			identical = FALSE;
		else
		{
			attr_p1 = attr_curr;
			attr_p2 = top_stack_p;
		}
	}

	/*--------------------------------------------------------------------*/
	/* Loop through attr_p1 and attr_p2 trying to disprove their identity */
	/*--------------------------------------------------------------------*/
	for ( ; attr_p1 && attr_p2 && identical ; attr_p1 = attr_p1->group_ptr, attr_p2 = attr_p2->group_ptr)
	{

		if (attr_p1->resource != attr_p2->resource || 
		    attr_p1->reprsent != attr_p2->reprsent ||
		    attr_p1->width    != attr_p2->width       )
		{
			identical = FALSE;
			break;
		}

		if (attr_p1->reprsent == LIST)
		{
			value_list_t *val_p1, *val_p2;

			/* Loop through LIST elements looking for differences */
			val_p1 = attr_p1->values.list.head;
			val_p2 = attr_p2->values.list.head;
			for ( ; val_p1 && val_p2 ; val_p1 = val_p1->next, val_p2 = val_p2->next)
				if (val_p1->value != val_p2->value)
					break;

			if (val_p1 || val_p2)
				identical = FALSE;
		}
		else /* attr_p2->reprsent == RANGE */
		{
			value_range_t *val_p1, *val_p2;

			/* Loop through RANGE elements looking for differences */
			val_p1 = attr_p1->values.range.head;
			val_p2 = attr_p2->values.range.head;

			for ( ; val_p1 && val_p2 ; val_p1 = val_p1->next, val_p2 = val_p2->next)
				if (val_p1->upper != val_p2->upper ||
				    val_p1->lower != val_p2->lower ||
				    val_p1->incr  != val_p2->incr     )
					break;

			if (val_p1 || val_p2)
				identical = FALSE;
		} 

	} /* end for loop to disprove identity of attr_p1 and attr_p2 */

	if (identical)
		return FALSE;

	/*=====================================================================*/
	/* Check to see if attr_curr's current value conflicts with any of the */
	/* possible values for any of the attributes on UAttrsStack.           */
	/*=====================================================================*/

	/*---------------------------------------------------------*/
	/* Loop through attr_curr (and handle group) using attr_p1 */
	/*---------------------------------------------------------*/
	if (attr_curr->resource == GROUP)
		attr_p1 = attr_curr->group_ptr;
	else
		attr_p1 = attr_curr;
	for ( ; attr_p1 ; attr_p1 = attr_p1->group_ptr)
	{

		top_p1 = attr_p1->current + attr_p1->width - 1;

		/*---------------------------------------------*/
		/* Loop through all the UAttrsStack attributes */
		/*---------------------------------------------*/
		for (stack_pp = UAttrsStack, i = UAttrsCount ; i-- ; stack_pp++)
		{

			/*--------------------------------------------------------*/
			/* Loop through stack_pp (and handle group) using attr_p2 */
			/*--------------------------------------------------------*/
			if ((*stack_pp)->resource == GROUP)
				attr_p2 = (*stack_pp)->group_ptr;
			else
				attr_p2 = *stack_pp;
			for ( ; attr_p2 ; attr_p2 = attr_p2->group_ptr)
			{

				if (attr_p1->resource != attr_p2->resource)
					continue;

				if (attr_p2->reprsent == LIST)
				{
					value_list_t *val_p = attr_p2->values.list.head;
					unsigned long top_p2;
	
					/*------------------------------------------------------*/
					/* Loop through each LIST element looking for conflicts */
					/*------------------------------------------------------*/
	
					for ( ; val_p ; val_p = val_p->next)
					{
						top_p2 = val_p->value + attr_p2->width - 1;
						if (!(val_p->value > top_p1 || top_p2 < attr_p1->current))
							return TRUE;
					}
				}
	
				/*-----------------------------------------------------------------------*/
				/* For a RANGE value representation, we are checking to see if the       */
				/* current attribute's (attr_p1) current value falls anywhere in         */ 
				/* the range of possible values for the UAttrsStack attribute (attr_p2). */
				/* We discount the possibility of width > increment (hence unused "gaps" */
				/* within range of possible UAttrsStack attribute values) for this test. */
				/*-----------------------------------------------------------------------*/
		
				else /* attr_p2->reprsent == RANGE */
				{
					value_range_t *val_p = attr_p2->values.range.head;

					for ( ; val_p ; val_p = val_p->next)
						if (!(attr_p1->current > (val_p->upper + attr_p2->width - 1) || top_p1 < val_p->lower))
							return TRUE;
				}

			} /* end for() loop through attr_p2 (to handle GROUP) */
		} /* end for() loop through UAttrsStack */
	} /* end for() loop through attr_p1 (to handle GROUP) */
	
	return FALSE;
} /* end of is_candidate() */

/***************************************************************************/
/* Function Name : INCREMENT_VALUE()                                       */
/* Function Type : Internal C function                                     */
/* Purpose       : Increment the current attribute to the next value, or   */
/*                 next non-conflicting value if attr_conf is supplied.    */
/* Arguments     :                                                         */
/*   attr_curr   - The attribute to be incremented (if attr_curr is a      */
/*                 GROUP, then attr_conf is ignored and all the members    */
/*                 in the group are incremented to thier next LIST value). */
/*   attr_conf   - An optional parameter describing an attribute for which */
/*                 no conflict will exist after incrementing attr_curr.    */
/*                 This attribute should NEVER be a GROUP.                 */ 
/*                                                                         */
/* Return Value  : Boolean TRUE if the attr_curr wrapped during this       */
/*                 incrementing phase; attr_curr will be set to it's start */
/*                 value in this case. FALSE otherwise.                    */ 
/*                                                                         */
/***************************************************************************/
static int increment_value(attr_curr, attr_conf)
	attribute_t *attr_curr, *attr_conf;
{
	attribute_t *attr_p;
	int wrapped;
	unsigned long oldval;

	/*------------------------------------------------------------*/
	/* Loop through the attr_curr (and handle group) using attr_p */
	/*------------------------------------------------------------*/
	if (attr_curr->resource == GROUP)
	{
		attr_p = attr_curr->group_ptr;
		attr_conf = NULL;
	}
	else
		attr_p = attr_curr;
	for ( ; attr_p ; attr_p = attr_p->group_ptr)
	{

		oldval = attr_p->current;

		/* If this is the first increment, save the starting value */
		if (!attr_p->changed)
		{
			attr_p->changed = TRUE;
			if (attr_p->reprsent == LIST)
				attr_p->start.list.ptr = attr_p->values.list.current;
			else /* attr_p->reprsent == RANGE */ 
				attr_p->start.range.value = attr_p->current;
		}
	
		/* Increment the attribute */
		if (attr_p->reprsent == LIST)
			wrapped = increment_list(attr_p, attr_conf);
		else /* attr_p->reprsent == RANGE */
			wrapped = increment_range(attr_p, attr_conf);

		/* Log the attribute increment */
		log_increment(1, attr_p, oldval);
	}

	return wrapped;
} /* end of increment_value() */

/***************************************************************************/
/* Function Name : INCREMENT_LIST()                                        */
/* Function Type : Internal C function                                     */
/* Purpose       : Increment a LIST representation to the next value       */
/*                 or next non-conflicting value with attr_conf.           */
/* Arguments     :                                                         */
/*   attr_curr   - The attribute to be incremented.                        */
/*   attr_conf   - An optional parameter describing an attribute for which */
/*                 no conflict will exist after incrementing attr_curr.    */
/*                                                                         */
/* Return Value  : Boolean TRUE if the attr_curr wrapped during this       */
/*                 incrementing phase; attr_curr will be set to it's start */
/*                 value in this case. FALSE otherwise.                    */
/*                                                                         */
/***************************************************************************/
static int increment_list(attr_curr, attr_conf)
	attribute_t *attr_curr, *attr_conf;
{
	valuespec_u *vals_curr = &(attr_curr->values);

	if (attr_conf == NULL)
	{
		/* Increment the attribute one time */
		if (vals_curr->list.current->next == NULL)
			vals_curr->list.current = vals_curr->list.head;
		else
			vals_curr->list.current = vals_curr->list.current->next;
	}

	else
	{ /* Increment the attribute until no conflict (and at least once) */
		unsigned long top = attr_conf->current + attr_conf->width - 1;
		value_list_t *val_p = vals_curr->list.current;

		for (val_p = val_p->next ;  ; val_p = val_p->next)
		{
			/* End of list */
			if (val_p == NULL)
				val_p = vals_curr->list.head;

			/* Check for wrap */
			if (val_p == attr_curr->start.list.ptr) 
				break;

			/* Check to see if the conflict is solved */
			if (val_p->value > top || 
			    val_p->value + attr_curr->width - 1 < attr_conf->current) 
				break;
		}

		/* Set the values list pointer */
		vals_curr->list.current = val_p;
	}

	/* Set current value for the attribute */
	attr_curr->current = vals_curr->list.current->value;
 
	/* Return TRUE for attribute wrapped condition */
	if (attr_curr->start.list.ptr == vals_curr->list.current)
		return TRUE;
	else
		return FALSE;
} /* end of increment_list() */

/***************************************************************************/
/* Function Name : INCREMENT_RANGE()                                       */
/* Function Type : Internal C function                                     */
/* Purpose       : Increment a RANGE representation to the next value      */
/*                 or next non-conflicting value with attr_conf.           */
/* Arguments     :                                                         */
/*   attr_curr   - The attribute to be incremented.                        */
/*   attr_conf   - An optional parameter describing an attribute for which */
/*                 no conflict will exist after incrementing attr_curr.    */
/*                                                                         */
/* Return Value  : Boolean TRUE if the attr_curr wrapped during this       */
/*                 incrementing phase; attr_curr will be set to it's start */
/*                 value in this case. FALSE otherwise.                    */
/*                                                                         */
/***************************************************************************/
static int increment_range(attr_curr, attr_conf)
	attribute_t *attr_curr, *attr_conf;
{
	value_range_t *val_p = attr_curr->values.range.current, *val_p2;

	/* Always increment by at least one increment */
	if (attr_curr->current > val_p->upper - val_p->incr)
	{
		if (val_p->next)
			attr_curr->values.range.current = val_p->next;
		else
			attr_curr->values.range.current = attr_curr->values.range.head;

		val_p = attr_curr->values.range.current;
		attr_curr->current = val_p->lower;
	}
	else
		attr_curr->current += val_p->incr;
	
	/* Check for wrap */
	if (attr_curr->current == attr_curr->start.range.value) 
		return TRUE;

	if (attr_conf != NULL) 
	{
		unsigned long conf_top = attr_conf->current + attr_conf->width - 1;
		unsigned long curr_bot = attr_curr->current;
		unsigned long incr = val_p->incr;
		unsigned long inc;

		/* Check to see if the single increment above solved the conflict */
		if (conf_top < curr_bot                                  || 
		    attr_conf->current > curr_bot + attr_curr->width - 1   )
			return FALSE;

		/* Compute the "inc" value that solves this particular conflict */ 
		inc = (conf_top - curr_bot) + (incr - ((conf_top - curr_bot) % incr));

		/*-----------------------------------------------------------*/
		/* If the "inc" value fits in the permissible range for this */
		/* attribute, then apply it. If not, then see if there are   */
		/* more range list elements to try. If not, then determine   */
		/* how to reset the attribute to either the lower end of the */
		/* range or its starting value and indicate we wrapped.      */
		/*-----------------------------------------------------------*/
		if (val_p->upper > conf_top)
		{
			/* Check for wrap where start is between current and curr_bot + inc */ 
			if (curr_bot < attr_curr->start.range.value        &&
			    attr_curr->start.range.value <= curr_bot + inc    )
			{
				attr_curr->current = attr_curr->start.range.value;
				val_p2 = attr_curr->values.range.head;
				for ( ; val_p2 ; val_p2 = val_p2->next)
				{
					if (attr_curr->current >= val_p2->lower &&
					    attr_curr->current <= val_p2->upper    )
						attr_curr->values.range.current = val_p2;
				}
			}

			/* OK to increment this attribute to curr_bot + inc */
			else 
				attr_curr->current = curr_bot + inc;

			/* Return TRUE for attribute wrapped condition */
			if (attr_curr->current == attr_curr->start.range.value)
				return TRUE;
			else
				return FALSE;
		}
		else if (val_p->next) /* Go to next range element at least curr_bot + inc */
		{
			for ( val_p2 = val_p->next ; val_p2 ; val_p2 = val_p2->next)
			{
				if (val_p2->lower >= curr_bot + inc) 
				{
					attr_curr->values.range.current = val_p2;
					attr_curr->current = val_p2->lower;
					break;
				}
				if (curr_bot + inc <= val_p2->upper)
				{
					attr_curr->current = curr_bot + inc;
					attr_curr->values.range.current = val_p2;
					break;
				}
			}

			if (!val_p2)	
			{
				attr_curr->current = attr_curr->start.range.value;
				val_p2 = attr_curr->values.range.head;
				for ( ; val_p2 ; val_p2 = val_p2->next)
				{
					if (attr_curr->current >= val_p2->lower &&
					    attr_curr->current <= val_p2->upper    )
						attr_curr->values.range.current = val_p2;
				}
			}

			/* Return TRUE for attribute wrapped condition */
			if (attr_curr->current == attr_curr->start.range.value)
				return TRUE;
			else
				return FALSE;
		}
		else /* Can't increment this attribute above conf_top */
		{
			/* In this case we can't increment past, so wrap */
			if (curr_bot < attr_curr->start.range.value)
			{
				attr_curr->current = attr_curr->start.range.value;
				val_p2 = attr_curr->values.range.head;
				for ( ; val_p2 ; val_p2 = val_p2->next)
				{
					if (attr_curr->current >= val_p2->lower &&
					    attr_curr->current <= val_p2->upper    )
						attr_curr->values.range.current = val_p2;
				}
			}

			/* In this case we still have a conflict with attr_conf, so wrap */
			else if (val_p->lower + attr_curr->width - 1 >= 
		 	         attr_conf->current                      )
			{
				attr_curr->current = attr_curr->start.range.value;
				val_p2 = attr_curr->values.range.head;
				for ( ; val_p2 ; val_p2 = val_p2->next)
				{
					if (attr_curr->current >= val_p2->lower &&
					    attr_curr->current <= val_p2->upper    )
						attr_curr->values.range.current = val_p2;
				}
			}

			/* OK to set this attribute to lower end of range */
			else
			{
				attr_curr->current = attr_curr->values.range.head->lower;
				attr_curr->values.range.current = attr_curr->values.range.head;
			}
		}
	} /* end processing attr_conf argument */

	/* Return TRUE for attribute wrapped condition */
	if (attr_curr->current == attr_curr->start.range.value)
		return TRUE;
	else
		return FALSE;
} /* end of increment_range() */

/***************************************************************************/
/* Function Name : SAVE_ADAPTER_STATE()                                    */
/* Function Type : Internal C function                                     */
/* Purpose       : Save the adapters' attributes state                     */
/* Arguments     :                                                         */
/*   adap_p1     - First adapter in adapter list to be saved.              */
/*   adap_p2     - Last adapter in adapter list to be saved.               */
/*   stdat_p     - Pointer to save area.                                   */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void save_adapter_state(adap_p1, adap_p2, stdat_p)
	adapter_t *adap_p1, *adap_p2;
	statedata_t *stdat_p;
{
	adapter_t *adap_p;
	attribute_t *attr_p, *attr_p2;

	/* Save the state data */
	for (adap_p = adap_p1 ; adap_p != adap_p2->next ; adap_p = adap_p->next)
		for (attr_p = adap_p->attributes ; 
		             attr_p && attr_p->adapter_ptr == adap_p ; 
		                     attr_p = attr_p->next                )
		{

			stdat_p->changed = attr_p->changed;

			if (attr_p->reprsent == LIST)
			{
				stdat_p->start.list.ptr   = attr_p->start.list.ptr;
				stdat_p->current.list.ptr = attr_p->values.list.current;	 
			}
			else /* attr_p->reprsent == RANGE */
			{
				stdat_p->start.range.value   = attr_p->start.range.value;
				stdat_p->current.range.value = attr_p->current;
			}

			stdat_p++;

			if (attr_p->resource == GROUP)
			{
				attr_p2 = attr_p->group_ptr;
				for ( ; attr_p2 ; attr_p2 = attr_p2->group_ptr, stdat_p++)
				{
					/* GROUP members must be LIST representation */
					stdat_p->changed          = attr_p2->changed;
					stdat_p->start.list.ptr   = attr_p2->start.list.ptr;
					stdat_p->current.list.ptr = attr_p2->values.list.current;
				}
			}
		}

	return;
} /* end of save_adapter_state() */

/***************************************************************************/
/* Function Name : RESTORE_ADAPTER_STATE()                                 */
/* Function Type : Internal C function                                     */
/* Purpose       : Restore the adapters' attributes state to what has been */
/*                 previously saved using save_adapter_state().            */
/* Arguments     :                                                         */
/*   adap_p1     - First adapter in adapter list to be restored.           */
/*   adap_p2     - Last adapter in adapter list to be restored.            */
/*   stdat_p     - Pointer to save area.                                   */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void restore_adapter_state(adap_p1, adap_p2, stdat_p)
	adapter_t *adap_p1, *adap_p2;
	statedata_t *stdat_p;
{
	adapter_t *adap_p;
	attribute_t *attr_p, *attr_p2;

	for (adap_p = adap_p1 ; adap_p != adap_p2->next ; adap_p = adap_p->next)
		for ( attr_p = adap_p->attributes ; 
		              attr_p && attr_p->adapter_ptr == adap_p ; 
	 	                      attr_p = attr_p->next              ) 
		{
			attr_p->changed = stdat_p->changed;

			if (attr_p->reprsent == LIST)
			{
				attr_p->values.list.current = stdat_p->current.list.ptr; 
				attr_p->start.list.ptr      = stdat_p->start.list.ptr;
				attr_p->current             = stdat_p->current.list.ptr->value;
			}
			else /* attr_p->reprsent == RANGE */
			{
				value_range_t *val_p = attr_p->values.range.head;

				attr_p->start.range.value   = stdat_p->start.range.value;
				attr_p->current             = stdat_p->current.range.value; 

				for ( ; val_p ; val_p = val_p->next)
				{
					if (attr_p->current >= val_p->lower &&
					    attr_p->current <= val_p->upper    )
						attr_p->values.range.current = val_p;
				}
			}

			stdat_p++;

			if (attr_p->resource == GROUP)
			{
				attr_p2 = attr_p->group_ptr;
				for ( ; attr_p2 ; attr_p2 = attr_p2->group_ptr, stdat_p++)
				{
					/* GROUP members must be LIST representation */
					attr_p2->changed             = stdat_p->changed;
					attr_p2->values.list.current = stdat_p->current.list.ptr;
					attr_p2->start.list.ptr      = stdat_p->start.list.ptr;
					attr_p2->current             = stdat_p->current.list.ptr->value;
				}
			}
		}

	return;
} /* end of restore_adapter_state() */

/***************************************************************************/
/* Function Name : ASSIGN_SHARE_INTERRUPTS()                               */
/* Function Type : Internal C function                                     */
/* Purpose       : Assign shareable interrupts where unique bus interrupt  */
/*                 level could not be resolved. Attempt to distribute the  */
/*                 shared interrupts to a degree, so that they are not all */
/*                 assigned the same level, if possible.                   */
/* Arguments     :                                                         */
/*   rc          - Pointer to int return code, value from cf.h.            */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/

#define INT_TBL_INCR 16 /* Initial and increment size of interrupt table */

static void assign_share_interrupts(rc)
	int *rc;
{
	attribute_t *attr_p1, *attr_p2;
	int found, index, table_size, table_count;
	inttbl_t *inttbl, *inttbl_p;

	/* Malloc the interrupt table */
	table_size = INT_TBL_INCR;
	inttbl = (inttbl_t *)calloc(1, table_size * sizeof(inttbl_t));
	if (inttbl == (inttbl_t *)NULL)
	{
		log_message(0, "assign_share_interrupts() : Out of virtual storage\n");
		*rc = E_MALLOC;
		return;
	}

	/*-----------------------------------------------------------------*/
	/* Initialize the interrupt table with shareable interrupt entries */
	/*-----------------------------------------------------------------*/
	for (attr_p1 = AttrHead ; attr_p1 ; attr_p1 = attr_p1->next)
	{
		if (attr_p1->resource == GROUP)
			attr_p2 = attr_p1->group_ptr;
		else
			attr_p2 = attr_p1;
		for ( ; attr_p2 ; attr_p2 = attr_p2->group_ptr)
		{
			if (attr_p2->resource != INTLVL || attr_p2->ignore)
				continue;

			if (attr_p2->specific_resource == INTLVL)
			{

				/* Search for the entry */
				inttbl_p = inttbl;
				for (index = 0 ; index < table_size && inttbl_p->usecount ; index++, inttbl_p++)
					if (inttbl_p->intnumber == attr_p2->current)
						break;
	
				/* See if table needs to be realloc'ed (don't use realloc, see AIX doc) */
				if (index == table_size)
				{
					inttbl_t *table2;
					int table2_size;

					table2_size = table_size + INT_TBL_INCR;
					table2 = (inttbl_t *)calloc(1, table2_size * sizeof(inttbl_t));
					if (table2 == (inttbl_t *)NULL)
					{
						free(inttbl);
						log_message(0, "assign_share_interrupts() : Out of virtual storage\n");
						*rc = E_MALLOC;
						return;
					}
					memcpy(table2, inttbl, table_size * sizeof(inttbl_t));
					free(inttbl);
					inttbl_p = table2 + table_size; /* Point to next avail element */
					table_size = table2_size;
					inttbl = table2;
				}

				/* Otherwise, if entry was found inttbl_p->usecount will be non zero */
				else if (inttbl_p->usecount)
				{
					inttbl_p->usecount = inttbl_p->usecount + 1;
					continue;
				}
				
				/* Create a new entry in the table */
				inttbl_p->intnumber = attr_p2->current; /* Upper two bytes for int type & ID */
				inttbl_p->priority  = attr_p2->priority;
				inttbl_p->trigger   = attr_p2->trigger;
				inttbl_p->usecount  = inttbl_p->usecount + 1;
			}
		}
	}

	/* Count the number of valid entries for the callbacks */
	inttbl_p = inttbl;
	table_count = 0;
	for ( ; table_size-- && inttbl_p->usecount ; inttbl_p++, table_count++);

	/*----------------------------------------------------------*/
	/* Use the bus specific callback to determine if interrupts */
	/* may be shared.                                           */
	/*----------------------------------------------------------*/
	for (attr_p1 = AttrHead ; attr_p1 ; attr_p1 = attr_p1->next)
	{
		if (attr_p1->specific_resource != SINTLVL ||
		    attr_p1->adapter_ptr->unresolved         )
			continue;

		found = FALSE;

		/* Use the callback */
		if (attr_p1->adapter_ptr->share_algorithm_cb != NULL)
		{
			adapter_t *adap_p = attr_p1->adapter_ptr;
			adap_p->share_algorithm_cb(rc, inttbl, table_count, attr_p1, &index, &found);
		}

		/* Now set the shareable interrupt value */
		if (found)
		{
			if (attr_p1->current != inttbl[index].intnumber)
			{
				attr_p1->changed = TRUE;   /* s/b set by increment, but be robust */
				attr_p1->current = inttbl[index].intnumber;
			}
			attr_p1->ignore = FALSE;
			attr_p1->specific_resource = INTLVL;
			inttbl[index].usecount++;
			log_share(1, attr_p1);
		}
		else /* !found */
		{
			log_error(1, TRUE, attr_p1->adapter_ptr->logname, attr_p1->name, 
			          "Could not match sharable bus interrupt level");
			attr_p1->adapter_ptr->unresolved = TRUE;
			*rc = E_BUSRESOURCE;
		}
		
	} /* end for() searching for shareable interrupt attributes */

	/* Free the interrupt table */
	free(inttbl);

	return;
} /* end of assign_share_interrupts() */


