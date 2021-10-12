static char sccsid[] = "@(#)95  1.23  src/bos/usr/ccs/lib/libdiag/cfgdev.c, libdiag, bos41J, 9514A_all 4/4/95 14:48:57";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS:    configure_device()     get_device_state()
 *               find_method()          initial_state()
 *               get_cpu_model()        invoke_method()
 *               load_diag_kext()       unload_diag_kext()
 *
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

#include        <stdio.h>
#include        <memory.h>
#include        <cf.h>
#include        <sys/cfgdb.h>
#include	<sys/sysconfig.h>
#include	<sys/stat.h>
#include        <sys/iplcb.h>    /* ipl structures for get_cpu_model() */
#include        <sys/mdio.h>     /* ioctl() stuff for get_cpu_model()  */
#include        <sys/rosinfo.h>  /* ipl structures for get_cpu_model() */
#include	"diag/diag.h"
#include        "diag/diago.h"
#include        "diag/class_def.h"
/* Function prototype 	*/

void	freelist(short);
int	load_diag_kext(char *);
void	unload_diag_kext();

/* Location of diagnostic methods */
#define CFGDIAG_METHOD "/usr/lib/methods/cfgdiag"
#define UCFGDIAG_METHOD "/usr/lib/methods/ucfgdiag"

struct  CuDv     *find_method();
int               find_children();

/* Global diagnostics errno setup in invoke_method()	      */
/* by storing value of return code form odm_run_method() call */

int	diag_cfg_errno=0;

/* Table of diagnostic kernel extensions */

struct	cfg_load *diagex_p = NULL;
int	number_of_diagex = 0;

/*
 * NAME: configure_device()
 *
 * FUNCTION: Designed to configure the device if the device is not con_
 *           figured. Also configure all parents of device and keep a
 *           list of name and initial state. This list will be traversed
 *           by the routine initial state.
 *	     The list kept is a doubly linked list.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: -1 for failure, device previous state.
 */

struct	DvLst {
	char	name[NAMESIZE];
	int	initial_state;
	char	configure[256];
	char	unconfigure[256];
	char	stop[256];
	struct	DvLst	*next;
	struct	DvLst	*previous;
};
struct 	DvLst	*head_DvLst_ptr = (struct DvLst *)NULL;
struct 	DvLst	*tail_DvLst_ptr = (struct DvLst *)NULL;

int     configure_device(device_name)
char    *device_name;
{
        char                    crit[255], args[255], *outbuf, *errbuf;
        int                     previous_state;
        struct  CuDv            *cudv;
        struct  listinfo        cnum;
	static	struct	DvLst	*node_ptr;
	struct	DvLst	*last;
	int	state=DEFINED;


	if ( (previous_state=get_device_status(device_name)) != AVAILABLE )
	{
		/* Generate a list of all devices in the path, including */
		/* the device itself, whose state is not AVAILABLE.	 */

	        sprintf(crit, "name = %s", device_name );
     		cudv = find_method(crit, &cnum);
        	if ((cudv == (struct CuDv *) -1) || (cnum.num == 0))
               		 return( -1 );
		while (state != AVAILABLE)
		{
			node_ptr=(struct DvLst *)malloc(sizeof(struct DvLst));
			sprintf(node_ptr->name, "%s", cudv[0].name);
			sprintf(node_ptr->configure, "%s",
				cudv[0].PdDvLn->Configure);
			sprintf(node_ptr->unconfigure, "%s",
				cudv[0].PdDvLn->Unconfigure);
			sprintf(node_ptr->stop, "%s",
				cudv[0].PdDvLn->Stop);
			node_ptr->initial_state=cudv[0].status;
			node_ptr->next=(struct DvLst *)NULL;
			node_ptr->previous=(struct DvLst *)NULL;

			/* Add to beginning of list if empty list */
			if(tail_DvLst_ptr == (struct DvLst *)NULL)
			{
				tail_DvLst_ptr = node_ptr;
				head_DvLst_ptr = tail_DvLst_ptr;
			} else {
				tail_DvLst_ptr->next = node_ptr;
				node_ptr->previous = tail_DvLst_ptr;
				tail_DvLst_ptr = node_ptr;
			}
			state=cudv[0].status;
			sprintf(crit, "name = %s", cudv[0].parent);

			/* free the cudv structure from find_method */

			diag_free_list(cudv, &cnum);

     			cudv = find_method(crit, &cnum);
	        	if ((cudv == (struct CuDv *) -1) || (cnum.num == 0)){
				freelist(0);
       	        		return( -1 );
			}
			state=cudv[0].status;
		}
		/* Now walk the list from the last entry and call the */
		/* method associated with each device in the node     */
		last=tail_DvLst_ptr;
		while(last)
		{
			 if((last->initial_state == DEFINED) ||
				(last->initial_state == STOPPED))
			 {
             		 	 sprintf(args, "-l %s", last->name);
	                	 if (invoke_method( last->configure, args, &outbuf,
					&errbuf))
				 {
       			              		free(outbuf);
                       			 	free(errbuf);
						freelist(1);
	       		                	return(-1);
      		       		 }
			 }
			 last=last->previous;
       		}

	}
        return(previous_state);
}

/*
 * NAME: initial_state()
 *
 * FUNCTION: Designed to put the device in it's initial state before
 *           configuration. Also put all parents back into the
 *	     initial state by going through the head_DvLst_ptr list.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: -1 for failure, 0 for success
 */

int     initial_state(state,device_name)
int     state;          /* previous state of the device.                  */
char    *device_name;
{
        char                    crit[255], args[255], *outbuf, *errbuf;
        struct  CuDv            *cudv;
        struct  listinfo        cnum;
        int                     rc = 0;
	struct	DvLst	*top, *current_p;

	top=head_DvLst_ptr;

	/* First search the device name passed in.	*/

	while (top)
		if(!strcmp(top->name, device_name))
			break;
		else
			top=top->next;

	/* Traverse the doubly linked list and put the device back */
	/* to its initial state. Start from top which could be     */
	/* in the middle of the list.				   */

	while(top)
	{

                sprintf( args, "-l %s", top->name );

                if (top->initial_state == STOPPED)
                        rc = invoke_method( top->stop,
                                             args, &outbuf, &errbuf );
                else
                        rc = invoke_method( top->unconfigure,
                                             args, &outbuf, &errbuf );

                free(outbuf);
                free(errbuf);
		current_p=top;
		top=top->next;

		/* If able to put the device back to its initial state */
		/* then free the entry in the list.		       */
		if(rc==0)
		{
			if(current_p == head_DvLst_ptr)
			{
				head_DvLst_ptr=current_p->next;
				if(head_DvLst_ptr)
					head_DvLst_ptr->previous = 
						(struct DvLst *)NULL;
			} else {
				current_p->previous->next=current_p->next;
				if(current_p != tail_DvLst_ptr)
					current_p->next->previous =
						 current_p->previous;
				else
					tail_DvLst_ptr=current_p->previous;
			}
			if(current_p == tail_DvLst_ptr)
				tail_DvLst_ptr = (struct DvLst *)NULL;
			free(current_p);
		} else
			break;
        }
	if(rc)
		freelist(1);
        return ( !rc ? 0 : -1 );
}

/*
 * NAME: freelist()
 *
 * FUNCTIONS: free the doubly linked list created by configure_device
 *            Used in case of errors.
 *	      A flag unconfigure is passed in and indicates that
 *	      the device needs to be unconfigured before freeing th
 *	      entry.
 *
 * RETURNS: none
 *
 */

void freelist(short unconfigure)
{
	struct	DvLst *index, *top;
        char    args[255], *outbuf, *errbuf;
	int	rc;

	top=head_DvLst_ptr;
	while(top)
	{
		/* Need to unconfigure before free the entry */
		if(unconfigure)
		{
	                sprintf( args, "-l %s", top->name );
                	if (top->initial_state == STOPPED)
                       		 rc = invoke_method( top->stop,
                                             args, &outbuf, &errbuf );
	                else
       		                 rc = invoke_method( top->unconfigure,
                                             args, &outbuf, &errbuf );

               		free(outbuf);
	                free(errbuf);
		}
		index=top->next;
		free(top);
		top=index;
	}
	head_DvLst_ptr = (struct DvLst *)NULL;
	tail_DvLst_ptr = (struct DvLst *)NULL;
}
/*
 * NAME: get_device_status()
 *
 * FUNCTION: Designed to check the configuration status of the
 *           requested device.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: -1 for failure, device current state.
 */

int     get_device_status(device_name)
char    *device_name;
{
        char                    crit[100];
        struct  CuDv            *cudv;
        struct  listinfo        obj_info;
        int                     state;

        sprintf( crit, "name = %s", device_name );
        cudv = (struct CuDv *)diag_get_list( CuDv_CLASS, crit,
			&obj_info, 1, 1);
        if ((cudv == (struct CuDv *) -1) || (obj_info.num == 0))
                return( -1 );

        state = cudv[0].status;

        diag_free_list( cudv, &obj_info );
        return( state );
}

/*
 * NAME: find_method()
 *
 * FUNCTION: Designed to find config method for a specific device, if
 *           that device does not have a config method then trace back
 *           the parents until finds a config method.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: pointer of the CuDv structure.
 */

struct  CuDv    *
find_method(name, obj_info)
char                    *name;
struct  listinfo        *obj_info;
{
        char            crit[100];
        struct  CuDv    *cud2;
        int             found=FALSE;

        strcpy(crit,name);
        while (!found)
        {
                cud2 = (struct CuDv *)diag_get_list(CuDv_CLASS,
				crit,obj_info,1,2);

                if ((cud2 == (struct CuDv *) -1) || (obj_info->num == 0))
                        return((struct CuDv *)-1);

                if(strlen(cud2->PdDvLn->Configure)>0)
                        found=TRUE;
                else
                        sprintf(crit, "name = %s",cud2[0].parent);
        }
        return(cud2);
}

/*
 * NAME: invoke_method
 *
 * FUNCTION:  Invoke the specifed method and retry in a ODM LOCK occurs.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  method return code
 */

invoke_method( method, args, inbuf, erbuf)
char    *method;
char    *args;
char    **inbuf;
char    **erbuf;
{
        int     rc;
        int     count = 0;
	char	*mounted_cdrfs;
	struct	stat	tmpbuf;
	char	progs[255];

	/* If the method passed in does not exist, then use the CDRFS_DIR */
	/* value to setup the path to the mount point for excution from   */
	/* mounted CDROM. Note that for mounted CDROM, there is no links  */
	/* between /etc and /usr/lib, therefore any method that requires  */
	/* to be executed from the mounted CDROM will need to have its    */
	/* Configure, Define, Unconfigure field in the PdDv setup to have */
	/* /usr/lib/methods in the path.			          */

	rc=stat(method, &tmpbuf);
	if((rc != 0) || (tmpbuf.st_size == 0))
		if((mounted_cdrfs=(char *)getenv("CDRFS_DIR")) != (char *)NULL)
			sprintf(progs, "%s%s", mounted_cdrfs, method);
		else
			sprintf(progs, "%s", method);
	else
		sprintf(progs, "%s", method);
		
        do {
                rc = odm_run_method(progs, args, inbuf, erbuf);
                if ( rc == E_ODMLOCK )
                        sleep(1);
        } while ( count++ != 120 && rc == E_ODMLOCK );

	/* Store return code in here for caller to examine, if needed */
	diag_cfg_errno = rc;
        return (rc);
}

/*
 * NAME: get_cpu_model
 *
 * FUNCTION:  Return the 4 byte CPU model id from the s1 ipl ros structure.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  CPU Model number if successful, -1 otherwise.
 */
 unsigned int
 get_cpu_model(aix_model_code)
 int    *aix_model_code;
 {
        IPL_DIRECTORY *iplcb_dir;   /* iplcb.h */
        IPL_INFO      *iplcb_info;  /* rosinfo.h */
        MACH_DD_IO    mdd;          /* mdio.h */
        int           fdes;
        unsigned int  model;
        struct CuAt   *cuat;
        int           howmany;


        /* Search ODM Customized Device Attribute File for the modelcode */

        cuat = (struct CuAt *)getattr("sys0", "modelcode", FALSE, &howmany);
        if(cuat == (struct CuAt *)NULL)
                return(-1);
        else
                *aix_model_code = strtol(cuat->value, (char **)NULL, 16);

        /* Open the NVRAM dd and get the iplcb version of model */

        if ((fdes = open("/dev/nvram",0)) < 0)
                return(-1);

        iplcb_dir = (IPL_DIRECTORY *)malloc(sizeof(IPL_DIRECTORY));
        iplcb_info = (IPL_INFO *)malloc(sizeof(IPL_INFO));
        if ( (iplcb_dir == ((IPL_DIRECTORY *)NULL)) ||
             (iplcb_info == ((IPL_INFO *)NULL)) )
                return(-1);

        mdd.md_incr = MV_BYTE;
        mdd.md_addr = 128;
        mdd.md_data = (char *)iplcb_dir;
        mdd.md_size = sizeof(*iplcb_dir);
        if (ioctl(fdes, MIOIPLCB, &mdd))
                return(-1);

        /* s1 */
        mdd.md_incr = MV_BYTE;
        mdd.md_addr = iplcb_dir->ipl_info_offset;
        mdd.md_data = (char *)iplcb_info;
        mdd.md_size = sizeof(*iplcb_info);
        if (ioctl(fdes, MIOIPLCB, &mdd))
                return(-1);

        if ( fdes > 0 )
                close(fdes);

	model = iplcb_info->model;
	free(iplcb_dir);
	free(iplcb_info);

        return(model);
 }






/*
 * NAME: diagex_cfg_state
 *
 * FUNCTION:  Sets the status field of the device to DIAGNOSE state.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This function will unconfigure the device and its children if
 *	  neccessary in order to set the device into the DIAGNOSE state. 
 *	  The original states will be saved. 
 *
 * RETURNS: 
 *		-1      Software error
 *		0       Good
 *		1       Child cannot be unconfigured
 *		2       Device cannot be unconfigured
 *		3       Device cannot be put into diagnose state
 */

struct	ChildLst {
	struct  CuDv    *device;
	struct	ChildLst	*next;
	struct	ChildLst	*previous;
};
struct 	ChildLst	*head_ChildLst_ptr = (struct ChildLst *)NULL;
struct 	ChildLst	*tail_ChildLst_ptr = (struct ChildLst *)NULL;
struct  ChildLst 	*child_ptr = (struct ChildLst *)NULL;

struct	CuDv	*diagex_cudv, *diagex_parent_cudv;
struct	listinfo	diagex_dev_num, diagex_parent_num;


int diagex_cfg_state ( char *device_name )
{
	char	*outbuf, *errbuf;
	char	crit[64], args[64];
	struct	ChildLst  *last;
	int	i;

	sprintf ( crit, "name = %s", device_name );
	diagex_cudv = find_method( crit, &diagex_dev_num );
	if ( diagex_cudv == (struct CuDv *) -1 || diagex_dev_num.num == 0 )
		return (-1);

	/* If the device is already in the DIAGNOSE state - then return */
	if ( diagex_cudv->status == DIAGNOSE ) 
		return (0);

	/* Check the devices' parents state. The parent must be AVAILABLE
         * If not, configure the parent					*/
	diagex_parent_cudv = (struct CuDv *)NULL;
	if ( get_device_status(diagex_cudv->parent) != AVAILABLE )  {
		sprintf ( crit, "name = %s", diagex_cudv->parent );
		diagex_parent_cudv = find_method( crit, &diagex_parent_num );
		if ( diagex_parent_cudv==(struct CuDv *) -1 || diagex_parent_num.num==0 )
			return (-1);
		sprintf ( args, "-l %s", diagex_parent_cudv->name );
		if ( invoke_method ( diagex_parent_cudv->PdDvLn->Configure,
						args, &outbuf, &errbuf)) {
			free (outbuf);
			free (errbuf);
			return (3);
		}
	}
	
	/* If the device is in the AVAILABLE state, then
         * all of the children devices must be put into the DEFINED state.
         * This has to happen before the device itself can be Unconfigured */
	if ( diagex_cudv->status == AVAILABLE ) {
		sprintf ( crit, "parent = %s", device_name );
		if (find_children( crit ))
			return(-1);
               /* Now walk the list from the last entry and call the */
                /* method associated with each device in the node     */
                last=tail_ChildLst_ptr;
                while(last)
                {
                         sprintf(args, "-l %s", last->device->name);
                         if (invoke_method( last->device->PdDvLn->Unconfigure, 
						args, &outbuf, &errbuf)){
                                free(outbuf);
                                free(errbuf);
                                return(1);
                         }
                         last=last->previous;
                }

		/* Unconfigure the device	*/
		sprintf ( args, "-l %s", diagex_cudv->name );
		if ( invoke_method ( diagex_cudv->PdDvLn->Unconfigure,
				args, &outbuf, &errbuf)) {
			free (outbuf);
			free (errbuf);
			child_initial_state();
			return (2);
		}
	}
	 
	/* Put device in DIAGNOSE state	*/
	sprintf ( args, "-l %s", diagex_cudv->name );
	if ( invoke_method ( CFGDIAG_METHOD, args, &outbuf, &errbuf)) {
		free (outbuf);
		free (errbuf);
		return (3);
	}
	return (0);
}


/*
 * NAME: diagex_initial_state
 *
 * FUNCTION:  Restores initial state to device and children.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This function will restore the device and its children back to
 *	  their original state. Must be used after the diagex_cfg_state function.
 *
 * RETURNS: 
 *		0       Good
 *		4       Device cannot be restored to defined state. 
 *		5       Device cannot be restored to configured state.
 *		6       Child cannot be restored to original state.
 */

int diagex_initial_state ( char *device_name )
{
	char	*outbuf, *errbuf;
	char	args[64];

	/* Put the device back into DEFINE state */
	sprintf ( args, "-l %s", device_name );
	if ( invoke_method ( UCFGDIAG_METHOD, args, &outbuf, &errbuf)) {
		free (outbuf);
		free (errbuf);
		return (4);
	}

	/* If the device was previously in the AVAILABLE state,
         * run its Config method to configure the device	*/
	if ( diagex_cudv->status == AVAILABLE ) {
		if ( invoke_method ( diagex_cudv->PdDvLn->Configure,
				args, &outbuf, &errbuf)) {
			free (outbuf);
			free (errbuf);
			return (5);
		}
	}

	/* If the parent was previously in the DEFINED state, 
	 * run its UnConfigure method                     	*/
	if ( diagex_parent_cudv && (diagex_parent_cudv->status == DEFINED) ) {
		sprintf ( args, "-l %s", diagex_parent_cudv->name );
		if ( invoke_method ( diagex_parent_cudv->PdDvLn->Unconfigure,
				args, &outbuf, &errbuf)) {
			free (outbuf);
			free (errbuf);
			return (5);
		}
	}

	return ( child_initial_state() );
}

/*
 * NAME: child_initial_state
 *
 * FUNCTION:  Restores children devices to initial state.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This function will restore the children back to
 *	  their original state. Must be used after the diagex_cfg_state function.
 *
 * RETURNS: 
 *		0       Good
 *		6       Child cannot be restored to original state.
 */

int child_initial_state()
{
	int 	rc = 0;
	char	*outbuf, *errbuf;
	char	args[64];
	struct	ChildLst  *head;
	
        /* Now walk the list from the first entry and call the */
        /* method associated with each device in the node     */
        head=head_ChildLst_ptr;
        while(head)
        {
        	sprintf(args, "-l %s", head->device->name);
                if (invoke_method( head->device->PdDvLn->Configure,
                                   args, &outbuf, &errbuf)){
                	free(outbuf);
                        free(errbuf);
                        rc = 6;
                }
                head=head->next;
        }

	return (rc);
}



/*
 * NAME: find_children
 *
 * FUNCTION:  Returns pointer to all children of specified device.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This function searches the CuDv for all children to "crit".
 *
 * RETURNS: 
 */
int
find_children( crit )
char *crit;
{
	struct	CuDv	*cudv;
	struct listinfo child_info;
	char		newcrit[64];
	int i;

	cudv = (struct CuDv *)diag_get_list( CuDv_CLASS, crit,
			&child_info, 1, 2 );
	if ( cudv == (struct CuDv *) -1 )
		return ( -1 );
	for ( i=0; i < child_info.num; i++ ) {
		if ( cudv[i].status == AVAILABLE ) {
			child_ptr = (struct ChildLst *)malloc(sizeof(struct ChildLst));
			child_ptr->next = (struct ChildLst *)NULL;
			child_ptr->previous = (struct ChildLst *)NULL;
        		/* Add to beginning of list if empty list */
        		if(tail_ChildLst_ptr == (struct ChildLst *)NULL) {
        			tail_ChildLst_ptr = child_ptr;
                		head_ChildLst_ptr = child_ptr;
        		} else {
        			tail_ChildLst_ptr->next = child_ptr;
                		child_ptr->previous = tail_ChildLst_ptr;
                		tail_ChildLst_ptr = child_ptr;
        		}
			child_ptr->device = &cudv[i];
			sprintf ( newcrit, "parent = %s", cudv[i].name );
			if ( find_children (newcrit) )
				return( -1 );
		}
		
	}
	return(0);
}

/*
 * NAME: load_diag_kext
 *
 * FUNCTION:  Load all kernel extensions needed to run diagnostics
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: Only load the kernel extension if the kmid is zero.
 *
 * RETURNS:  0 or -1
 */
int
load_diag_kext(char *uniquetype)
{
	struct	PDiagAtt *pdiagatt;
	struct 	listinfo p_info;
	int	diag_source;
	char	mounted_cdrfs[128];
 	char	diagexdd[255];
	int	i,rc,bc;

        diag_source=diag_exec_source(&mounted_cdrfs);
	if(diagex_p == NULL)
	{ /* Allocate and setup table */
		pdiagatt = (struct PDiagAtt *)diag_get_list(PDiagAtt_CLASS,
			"attribute=diag_kext", &p_info, MAX_EXPECT, 1);
		number_of_diagex = p_info.num + 1;
		diagex_p = (struct cfg_load *)calloc(number_of_diagex,
				sizeof(struct cfg_load));
		if(diag_source)
			sprintf(diagexdd, "%s%s", mounted_cdrfs,
				"/usr/lib/drivers/diagex");
		else
			sprintf(diagexdd, "%s", "/usr/lib/drivers/diagex");

		/* First entry always contains the default diagnostics */
		/* kernel extension. Since this is the first time      */
		/* it needs to be loaded.			       */

		diagex_p[0].path = (char *)calloc(1, strlen(diagexdd)+1);
		strcpy(diagex_p[0].path, diagexdd);
		rc=sysconfig(SYS_SINGLELOAD, (void *)
				&diagex_p[0], (int)sizeof(diagex_p[0]));
		if(rc)
		   return(-1);

		/* Now setup the table to contain the rest of the other */
		/* diagnostic kernel extensions.			*/

		for(i=1; i<number_of_diagex;i++){
			if(diag_source)
				sprintf(diagexdd, "%s%s%s",
				mounted_cdrfs, "/usr/lib/drivers/",
				pdiagatt[i-1].value);
			else
				sprintf(diagexdd, "%s%s",
				"/usr/lib/drivers/", pdiagatt[i-1].value);
		        diagex_p[i].path = (char *)calloc(1, strlen(diagexdd)+1);
			strcpy(diagex_p[i].path, diagexdd);
		}
		if(p_info.num > 0)
			diag_free_list(pdiagatt, &p_info);
	}
	/* Now see if the device needs to have another diagnostic kernel */
	/* extension loaded.						 */

	rc=get_diag_att(uniquetype, "diag_kext", 's', &bc, diagexdd);
	if(rc == 0) /* Device has the attribute, therefore needs extra diagex */
		for(i=1; i<number_of_diagex; i++)
			if(strstr(diagex_p[i].path, diagexdd) != (char *)NULL)
				if(diagex_p[i].kmid == 0)
				{
					rc=sysconfig(SYS_SINGLELOAD, (void *)
						&diagex_p[i],
						(int)sizeof(diagex_p[i]));
					if(rc)
					   return(-1);
				}
	return(0);
}

/*
 * NAME: unload_diag_kext
 *
 * FUNCTION:  unload all kernel extensions needed to run diagnostics
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: Only unload the kernel extension if the kmid is non zero.
 *
 * RETURNS: 
 */
void
unload_diag_kext()
{

	int	i, rc;
	char	cmd[256];

	if(diagex_p != NULL)
	{
		for(i=0; i<number_of_diagex; i++)
		{
			if(diagex_p[i].kmid != 0)
				rc=sysconfig(SYS_KULOAD, (void *)
					&diagex_p[i], (int)sizeof(diagex_p[i]));
			if(diagex_p[i].path != NULL)
				free(diagex_p[i].path);
		}
		number_of_diagex=0;
		free(diagex_p);
		diagex_p = NULL;
	}
}
