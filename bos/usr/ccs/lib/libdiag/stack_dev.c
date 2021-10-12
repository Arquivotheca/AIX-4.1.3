static char sccsid[] = "@(#)51	1.1.1.3  src/bos/usr/ccs/lib/libdiag/stack_dev.c, libdiag, bos41B, bai4 1/10/95 16:39:29";
/*
 * COMPONENT_NAME: (LIBDIAG) Diagnostic Library
 *
 * FUNCTIONS: 		stack_device_name
 *			remove_new_device
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "odmi.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#define STACK_SIZE 8

/* GLOBAL VARIABLES */
typedef struct 	new_dname_s { 
	char	*name[STACK_SIZE];
	struct	new_dname_s *next;
} dname_t;
int		num_dname_stack = 0;
dname_t		*dname, *top_dname;

/* CALLED FUNCTIONS */

/*  */
/* NAME: stack_device_name
 *
 * FUNCTION: This function creates a stack of device names that were 'defined'
 * 	by the diag initialization process. This stack will be processed when
 *	the controller exits by 'undefining' the devices. 
 * 
 * NOTES: 
 *
 * RETURNS: NONE
 *
 */

stack_device_name( device_name )
char		*device_name;
{

	/* if first time through, alloc space for some names */	
	if ( num_dname_stack == 0 ) {
		dname=(struct new_dname_s *)calloc(1,sizeof(struct new_dname_s));

		top_dname = dname;
		dname->next = NULL;
	}

	/* check to see if we need more space */
	else if ( !(num_dname_stack % STACK_SIZE) ) {
		num_dname_stack = 0;
		dname->next = (struct new_dname_s *)
					calloc(1,sizeof(struct new_dname_s));
		dname = dname->next;
		dname->next = NULL;
	}

	dname->name[num_dname_stack] = (char *)malloc(strlen(device_name)+1);
	strcpy(dname->name[num_dname_stack++],device_name);

}
/*  */
/* NAME: remove_new_device
 *
 * FUNCTION: This function executes a 'rmdev' command on each device in
 *	the stack top_dname. 
 * 
 * NOTES: 
 *
 * RETURNS: NONE
 *
 */

remove_new_device()
{
	int rc;
	struct  CuDv	*cudv;
	char    args[255];
	struct  listinfo info;
	char	*outbuf, *errbuf, ucfg_args[128];

	num_dname_stack = 0;
	dname = top_dname;

	while ( strlen(dname->name[num_dname_stack]) ) {
		sprintf(args, "name = %s", dname->name[num_dname_stack]);
		cudv = (struct CuDv *)diag_get_list(CuDv_CLASS, args,
				&info, 1, 2);
		if (cudv != (struct CuDv *) -1 && cudv != (struct CuDv *)NULL){
			/* First unconfigure, then undefine */
			sprintf(ucfg_args, "-l %s ", 
			       dname->name[num_dname_stack++]);
			if(cudv->status == AVAILABLE){
				rc = invoke_method(cudv->PdDvLn->Unconfigure,
					ucfg_args, &outbuf, &errbuf);
				free ( outbuf );
				free ( errbuf );
			}
			rc = invoke_method(cudv->PdDvLn->Undefine,
				ucfg_args, &outbuf, &errbuf);
			free ( outbuf );
			free ( errbuf );
		} else
			num_dname_stack++;

		if ( !(num_dname_stack % STACK_SIZE) ) {
			num_dname_stack = 0;
			dname = dname->next;
		}
	}
}
