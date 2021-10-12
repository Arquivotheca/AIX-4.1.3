static char sccsid[] = "@(#)96  1.14.1.13  src/bos/usr/ccs/lib/libcfg/POWER/br.c, libcfg, bos41J, 9521A_all 5/22/95 09:06:47";
/*
 * COMPONENT_NAME: (LIBCFG) BUSRESOLVE Module
 *
 * FUNCTIONS:
 *	busresolve
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
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include "adapter.h"
#include "bt.h"
#include "bt_extern.h"
#include "pt_extern.h"

/*------------------*/
/* Global variables */
/*------------------*/

/* The following are exported from the library for debugging purposes */
int prntflag = FALSE;
FILE *trace_file = NULL;

/* The following are exported from the library for the lsresource command */
adapter_t *AdapterList;
attribute_t *AttributeList;

/*----------------------*/
/* Forward declarations */
/*----------------------*/

int busresolve(char *, int, char *, char *, char *);
static int get_top_bus(char *, bus_type_e *, char *, char *);
static int get_cu_pd_objs(char *, struct CuDv *, struct PdDv *);
static int update_cuat(attribute_t *);
static void build_strings(adapter_t *, char *, char *, char *);

extern int build_lists_rs6k(char *, char *, adapter_t **, attribute_t **, int);
extern int build_lists_rspc(char *, char *, adapter_t **, attribute_t **, int);
extern int destroy_lists(adapter_t **, attribute_t **);
extern int resolve_lists(adapter_t **, attribute_t **);

/***************************************************************************/
/* Function Name : BUSRESOLVE()                                            */
/* Function Type : C function                                              */
/* Purpose       : Constructs the adapter and attributes lists for all of  */
/*                 the devices on the named bus.                           */
/* Global Vars   :                                                         */
/*  AdapterList    - Head of adapter list                                  */
/*  AttributeList  - Head of attributes list                               */
/* Arguments     :                                                         */ 
/*  logname        - Device logical name (if resolveing one device)        */
/*  flags          - Controls for low level busresolve command             */
/*  conf_list      - Device logical names - bus resources resolved         */
/*  not_res_list   - Device logical names - bus resources NOT resolved     */
/*  busname        - Name of the bus                                       */
/*                                                                         */
/* Return Value  :                                                         */
/*                                                                         */
/***************************************************************************/
int busresolve(logname, flags, conf_list, not_res_list, busname)
	char *logname;
	int flags;
	char *conf_list, *not_res_list, *busname;
{
	int rc = E_OK, single_device = FALSE;
	char bus[NAMESIZE];
	bus_type_e bustype;

	/*-------------------------------------------*/
	/* Check arguments and init returned strings */
	/*-------------------------------------------*/

	if ((busname == NULL || !strlen(busname)) && 
			(logname == NULL || !strlen(logname))    )
		return E_ARGS;

	if (conf_list == NULL || not_res_list == NULL)
		return E_ARGS;

	*conf_list = *not_res_list = '\0';	

	/*-------------------------------------------*/
	/* Determine the top bus in device hierarchy */
	/*-------------------------------------------*/

	rc = get_top_bus(bus, &bustype, logname, busname);
	if (rc != E_OK)
		return rc;

	if (logname)
		single_device = TRUE;

	/*--------------------------------------*/
	/* Build the adapter / attributes lists */
	/*--------------------------------------*/

	switch(bustype)
	{
		case MCA :
			rc = build_lists_rs6k(logname, bus, &AdapterList, &AttributeList, flags);
			break;
		case PCI :
			rc = build_lists_rspc(logname, bus, &AdapterList, &AttributeList, flags);
			break;
		default  :
			log_message(0, "Unsupported top level bus type detected\n");
			return E_TYPE;
	}

	if (rc != E_OK)
	{
		if (single_device)
		{
			/* Any error means device won't configure */
			destroy_lists(&AdapterList, &AttributeList);
			return rc;
		}
		else if (rc == E_NOCuDv || rc == E_NOPdDv || rc == E_BADATTR || rc == E_BUSRESOURCE) 
		{
			/* Ignore non-fatal errors */
			rc = E_OK;
		}
		else /* Terminate for fatal error */
		{
			destroy_lists(&AdapterList, &AttributeList);
			return rc;
		}
	}

	/*-----------------------------------------------------*/
	/* If we are producing the attribute summary stop here */
	/*-----------------------------------------------------*/

	if (flags & COMMAND_MODE_FLAG && !(flags & RESOLVE_MODE_FLAG))
		return rc;

	/*---------------------------*/
	/* Resolve all bus resources */
	/*---------------------------*/

	rc = resolve_lists(&AdapterList, &AttributeList);

	if (flags & COMMAND_MODE_FLAG)
	{
		build_strings(AdapterList, conf_list, not_res_list, busname);
		return rc;
	}

	if (rc != E_OK)
	{
		if (single_device)
		{
			/* Any error means device won't configure */
			destroy_lists(&AdapterList, &AttributeList);
			return rc;
		}
		else if (rc != E_BUSRESOURCE)
		{
			/* Any error other than E_BUSRESOURCE is fatal */
			destroy_lists(&AdapterList, &AttributeList);
			return rc;
		}
	}

	/* Update customized attributes database */
	rc = update_cuat(AttributeList);
	if (rc != E_OK && !single_device)
	{
		/* At boot time, clear error code and hope it wasn't the */
		/* boot device that failed. Even if it was, the machine  */
		/* is not going to boot no matter what we do...          */
		rc = E_OK;
	}

	/* Build lists of resolved and not resolved device logical names */  
	build_strings(AdapterList, conf_list, not_res_list, busname);

	destroy_lists(&AdapterList, &AttributeList);
	return rc;
} /* end busresolve */

/***************************************************************************/
/* Function Name : GET_TOP_BUS()                                           */ 
/* Function Type : C function                                              */
/* Purpose       : Get the name of the topmost bus in the device hierarchy */ 
/* Arguments     :                                                         */ 
/*  bus             - returned name of the top bus                         */
/*  bustype         - returned type of the top bus (pci, mca, etc.)        */
/*  logname         - device logical name (for single device resolve)      */
/*  busname         - name of parent bus (supplied)                        */
/*                                                                         */
/* Return Value  :                                                         */
/*  E_OK           Top bus determined                                      */
/*  E_TYPE         Couldn't get proper bus type                            */
/*  E_NOCuDvPARENT Couldn't get parent device                              */ 
/*                                                                         */
/*   (from get_cu_pd_objs) :                                               */
/*  E_NOCuDv       No customized record                                    */
/*  E_NOPdDv       No predefined record                                    */
/*  E_ODMGET       ODM failure                                             */
/*                                                                         */
/***************************************************************************/
static int get_top_bus(bus, bustype, logname, busname)
	char *bus;
	bus_type_e *bustype;
	char *logname, *busname;
{
	int rc;
	struct CuDv cudv, cudv_parent;
	struct PdDv pddv, pddv_parent;

	/*-----------------------------------------------------*/
	/* Go back up the tree until the device whose class is */ 
	/* bus and whose parent's class is NOT bus is found.   */
	/*-----------------------------------------------------*/

	/* Get cudv / pddv records for the "current" device */ 
	rc = get_cu_pd_objs(logname ? logname : busname, &cudv, &pddv);
	if (rc != E_OK)
		return rc;

	for ( ; ; )
	{
		
		/* Get cudv / pddv records for the "parent" device */
		if (strlen(cudv.parent) == 0)
		{
			log_message(0, "No parent bus for : %s (Check ODM database)\n",
			            logname ? logname : busname);
			return E_NOCuDvPARENT;
		}
		rc = get_cu_pd_objs(cudv.parent, &cudv_parent, &pddv_parent);
		if (rc != E_OK)
			return rc;

		/* Check to see if this our top bus */  
		if (!strcmp(pddv.class, "bus") && strcmp(pddv_parent.class, "bus"))
		{
			strcpy(bus, cudv.name);
			if (!strcmp(pddv.type, "mca"))
				*bustype = MCA; 
			else if (!strcmp(pddv.type, "pci"))
				*bustype = PCI; 
			else if (!strcmp(pddv.type, "isa"))
				*bustype = ISA; 
			else if (!strcmp(pddv.type, "pcmcia"))
				*bustype = PCMCIA; 
			else 
			{
				*bustype = NONE;
				return E_TYPE;
			}
			break;
		}

		/* Save parent -> current */
		memcpy(&cudv, &cudv_parent, sizeof(struct CuDv));
		memcpy(&pddv, &pddv_parent, sizeof(struct PdDv));

	}

	return E_OK;
} /* end of get_top_bus() */  

/***************************************************************************/
/* Function Name : GET_CU_PD_OBJS()                                        */ 
/* Function Type : Internal C function                                     */
/* Purpose       : Get the customized and predefined device recs given the */
/*                 device logical name.                                    */ 
/* Arguments     :                                                         */ 
/*  name            - logical name for device                              */
/*  cudv            - pointer to storage for CuDv record                   */
/*  pddv            - pointer to storage for PdDv record                   */
/*                                                                         */
/* Return Value  :                                                         */
/*  E_OK           Got predefined and customized objects OK                */
/*  E_NOCuDv       No customized record                                    */
/*  E_NOPdDv       No predefined record                                    */
/*  E_ODMGET       ODM failure                                             */
/*                                                                         */
/***************************************************************************/
static int get_cu_pd_objs(name, cudv, pddv)
	char *name;
	struct CuDv *cudv;
	struct PdDv *pddv;
{
	struct CuDv *cudvrc;
	struct PdDv *pddvrc;
	char query_str[UNIQUESIZE + 15];

	/* Get CuDv record */
	sprintf(query_str, "name = %s", name);
	cudvrc = odm_get_first(CuDv_CLASS, query_str, cudv);
	if (cudvrc == (struct CuDv *)NULL)
	{
		log_message(0, "odm_get failed for CuDv : %s\n", query_str);
		return E_NOCuDv;
	}
	if (cudvrc == (struct CuDv *)FAIL)
	{
		log_message(0, "odm_get failed for CuDv : %s\n", query_str);
		return E_ODMGET;
	}

	/* Get PdDv record */
	sprintf(query_str, "uniquetype = %s", cudv->PdDvLn_Lvalue);
	pddvrc = odm_get_first(PdDv_CLASS, query_str, pddv);
	if (pddvrc == (struct PdDv *)NULL)
	{
		log_message(0, "odm_get failed for PdDv : %s\n", query_str);
		return E_NOPdDv;
	}
	if (pddvrc == (struct PdDv *)FAIL)
	{
		log_message(0, "odm_get failed for PdDv : %s\n", query_str);
		return E_ODMGET;
	}

	return E_OK;
} /* end of get_cu_pd_objs() */

/***************************************************************************/
/* Function Name : UPDATE_CUAT()                                           */ 
/* Function Type : Internal C function                                     */
/* Purpose       : Store new values for the customized attributes database */
/* Arguments     :                                                         */ 
/*  attribute_list  - Pointer to head of attributes list                   */
/*                                                                         */
/* Return Value  :                                                         */
/*  E_OK          All attributes updated successfully                      */
/*  E_ODMGET                                             getattr()         */
/*  E_NOATTR      A PdAt/CuAt obj could not be           getattr()         */
/*  E_BADATTR     Multiple PdAt objects in ODM           getattr()         */
/*  E_ODMUPDATE   Could not update an attribute          putattr()         */
/*                                                                         */
/* Notes :                                                                 */
/*  1) This function always attempts to update all attributes, but if a    */
/*     failure occurs, the device is marked unresolved so it won't get     */
/*     into the resolved list.                                             */
/*                                                                         */
/***************************************************************************/
static int update_cuat(attribute_list)
	attribute_t *attribute_list;
{
	attribute_t *attr_p, *attr_p1;
	struct CuAt *cuat_p;
	int how_many, rc = E_OK, error_detect = FALSE;
	char append_str[3]; /* Sized for one char plus comma */

	log_message(0, "\nUpdating customized attributes database : \n\n");

	for (attr_p = attribute_list ; attr_p ; attr_p = attr_p->next)
	{

		/* Use attr_p1 to handle GROUP attributes */ 

		if (attr_p->resource == GROUP)
			attr_p1 = attr_p->group_ptr;
		else
			attr_p1 = attr_p;
		for ( ; attr_p1 ; attr_p1 = attr_p1->group_ptr) 
		{

			/* Skip unmodified attributes, unresolved devices */

			if (attr_p1->adapter_ptr->unresolved || !attr_p1->changed)
				continue;

			/*-------------------------------------*/
			/* Get the CuAt struct (using getattr) */
			/*-------------------------------------*/

			cuat_p = getattr(attr_p1->adapter_ptr->logname, 
			                 attr_p1->name, FALSE, &how_many);
			if (cuat_p == (struct CuAt *)NULL)
			{
				error_detect = TRUE;
				log_error(1, TRUE, attr_p1->adapter_ptr->logname, attr_p1->name,
									"Attribute could not be found in ODM");
				attr_p1->adapter_ptr->unresolved = TRUE;
				rc = E_NOATTR;
				continue;
			}
			else if (cuat_p == (struct CuAt *)FAIL)
			{
				error_detect = TRUE;
				log_error(1, TRUE, attr_p1->adapter_ptr->logname, attr_p1->name,
									"getattr() failed");
				attr_p1->adapter_ptr->unresolved = TRUE;
				rc = E_ODMGET;
				continue;
			}
			else if (how_many != 1)
			{
				error_detect = TRUE;
				log_error(1, TRUE, attr_p1->adapter_ptr->logname, attr_p1->name,
									"Multiple attributes in ODM");
				attr_p1->adapter_ptr->unresolved = TRUE;
				rc = E_INVATTR; /* Could use a better error message - multiple objects */
				continue;
			}

			/*---------------------------------------------------*/
			/* Update the CuAt object to reflect the new value   */
			/* Note putattr() takes care of all cases :          */
			/*   - add CuAt obj for non-default current value    */
			/*   - update CuAt for new non-default current value */
			/*   - delete CuAt for new default current value     */
			/*---------------------------------------------------*/

			switch (attr_p1->specific_resource)
			{
				case IOADDR   :
				case MADDR    :
				case BADDR    :
					sprintf(cuat_p->value, "0x%x", attr_p1->current);
					break; 
				case INTLVL   :
				case NSINTLVL :
					/* Mask out interrupt controller type and id for RSPC */
					sprintf(cuat_p->value, "%lu", INTR_NUMBER(attr_p1->current));

					/* Append CuAt value string if needed */
					if (attr_p1->cuat_append != '\0')
					{
						sprintf(append_str, ",%c", attr_p1->cuat_append);
						strcat(cuat_p->value, append_str);
					}
					break; 
				case DMALVL   :
					sprintf(cuat_p->value, "%lu", attr_p1->current);
					break; 
				default       :
					attr_p1->adapter_ptr->unresolved = TRUE;
					error_detect = TRUE;
					log_message(0, "--- INTERNAL ERROR --- UPDATE_CUAT()\n");
			}

			rc = putattr(cuat_p);
			if (rc != E_OK)
			{
				error_detect = TRUE;
				log_error(1, TRUE, attr_p1->adapter_ptr->logname, attr_p1->name,
									"putattr() failed");
				attr_p1->adapter_ptr->unresolved = TRUE;
				rc = E_ODMUPDATE;
			}

		} /* end for () looping through grouped attribute list */
	} /* end for () looping through attribute list */

	if (!error_detect)
		log_message(1, "No errors detected while updateing CuAt DB\n");
		
	return rc;
} /* end of update_cuat() */

/***************************************************************************/
/* Function Name : BUILD_STRINGS()                                         */ 
/* Function Type : Internal C function                                     */
/* Purpose       : Build the strings of resolved and unresolved device     */
/*                 logical names.                                          */ 
/* Arguments     :                                                         */ 
/*  adapter_list  - Head of the adapters list                              */ 
/*  resv_list     - Device logical names resolved                          */
/*  not_resv_list - Device logical names NOT resolved                      */
/*  busname       - Device logical name for the bus                        */
/*                                                                         */
/* Return Value  : None                                                    */
/*                                                                         */
/***************************************************************************/
static void build_strings(adapter_list, resv_list, not_resv_list, busname)
	adapter_t *adapter_list;
	char *resv_list, *not_resv_list, *busname;
{
	adapter_t *adap_p;
	
	*resv_list = '\0';
	*not_resv_list = '\0';

	for (adap_p = adapter_list ; adap_p ; adap_p = adap_p->next)
	{

		/*-------------------------------------------*/
		/* Skip bus extenders because they already   */
		/* had their config methods run in sync.c    */
		/* and we don't want to have cfgmgr run them */
		/* again!                                    */
		/*-------------------------------------------*/
		if (adap_p->bus_extender)
			continue;
		
		/* Don't add the bus into either list */
		if (!strcmp(adap_p->logname, busname))
			continue;

		if (adap_p->unresolved)  
		{
			/* Put it in the "not resolved list" */
			strcat(not_resv_list, adap_p->logname);
			strcat(not_resv_list, ",");
		}
		else
		{
			/* Put it in the "resolved list" */
			strcat(resv_list, adap_p->logname);
			strcat(resv_list, ",");
		}
	}

	/* Remove trailing comma from both strings */
	if (strlen(resv_list) != 0)
		resv_list[strlen(resv_list) - 1] = '\0';
	if (strlen(not_resv_list) != 0)
		not_resv_list[strlen(not_resv_list) - 1] = '\0';

	/* Log the returned lists */
	log_message(0, "\nReturned lists : \n\n");
	log_message(1, "conf_list    : \"%s\"\n", resv_list);
	log_message(1, "not_res_list : \"%s\"\n", not_resv_list);

	return;
} /* end of build_strings() */

