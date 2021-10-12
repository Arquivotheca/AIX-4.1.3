static char sccsid[] = "@(#)45	1.9.2.3  src/bos/kernext/scsi/vscsiddt.c, sysxscsi, bos41J, 9514A_all 3/21/95 13:31:30";
/*
 *   COMPONENT_NAME: SYSXSCSI
 *
 *   FUNCTIONS: vsc_alloc_b_link_pool
 *		vsc_alloc_cmdpool
 *		vsc_alloc_scsi
 *		vsc_alloc_shared
 *		vsc_alloc_tgt
 *		vsc_bld_vsc_buf
 *		vsc_close
 *		vsc_config
 *		vsc_dealloc_tgt
 *		vsc_fail_open
 *		vsc_free_scsi
 *		vsc_free_shared
 *		vsc_init_cancel_cmd_elem
 *		vsc_init_dev
 *		vsc_inquiry
 *		vsc_ioctl
 *		vsc_open
 *		vsc_read_blk
 *		vsc_register_async
 *		vsc_start_unit
 *		vsc_test_unit_rdy
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 *
 * COMPONENT:   SYSXSCSI 
 *                      
 * NAME:	vscsidd.c
 *                      
 * FUNCTION:	IBM SCSI Protocol Driver Source File
 *                                                 
 *		This protocol driver is the interface between a 
 *		SCSI device driver and the adapter device driver.  
 *		It executes commands from multiple drivers which 
 *		contain generic SCSI device commands, and manages 
 *		the execution of those commands. Several ioctls 
 *		are supported to provide for system and device
 *		management.
 */


/*
 *  GENERAL NOTES
 *  -------------
 *
 *  The protocol driver needs several attributes of the parent adapter 
 *  in order to function properly. These are passed to the protocol 
 *  via the DDS structure at CFG_INIT time.
 *
 *  SCSI id 
 *  The SCSI id of the virtual device is an attribute of the
 *  parent adapter. At SCIOSTART, the protocol driver can ensure 
 *  that the started device's scsi id 
 *  does not conflict with the adapter's SCSI id.
 *
 *  WIDE ENABLE 
 *  If the adapter's wide enable attribute is disabled(8 bit SCSI bus),
 *  then at SCIOSTART, the SCSI id of the device is checked to ensure
 *  that it is not greater then 7.
 *
 *  INTERRUPT PRIORITY
 *  It is necessary for the protocol driver to know the adapter driver's
 *  interrupt priority so it can serialize critical sections of code
 *  without interruption from the adapter driver.
 *
 *  PARENT NAME
 *  It is necessary for the protocol driver to know the parent adapter's
 *  logical name. This is needed for the NS services to establish a channel
 *  between the protocol and adapter drivers.
 *
 *
 *
 *  SOME FLAGS AND COUNTERS 
 *  vsc_info.num_of_cfgs
 *	- used to determine the first CONFIG_INIT to the protocol 
 *	  driver and the last CONFIG_TERM to the protocol driver.
 *  	  operations such as for devswadd and devswdel occur at this time.
 *	  vsc_ctrl is a static global structure.
 *
 *
 *  shared->num_of_cfgs
 *	- shared is a structure associated with a group of virtual
 *	  devices that have a common parent adapter.
 *	  used to determine first config_init call and the last config_term
 *	  call for a virtual device pair.
 *	  used for xmfree of shared struct.
 *
 *
 *  DEVICE REGISTRATION
 *	adapters may choose to address destination devices
 *	in different ways. In order the to give the adapter driver
 *	the oppurtunity to register the device with the adapter, a NDD_ADD_DEV
 *	ioctl will be issued to the adapter driver at SCIOSTART and a 
 *      NDD_DEL_DEV ioctl at SCIOSTOP. the adapter may or may not transform 
 *      this combination before communicating with the device.
 */

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/scb_user.h>
#include <sys/scsi.h>
#include <sys/scsi_scb.h> 
#include <sys/vscsidd.h>
#include <sys/errids.h>
#include <sys/lockname.h>
#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#endif

/* END OF INCLUDED SYSTEM FILES  */



/*****************************************************************************/
/* Global device driver data areas                                           */
/*****************************************************************************/

/* array containing pointer to shared scsi bus information (1 per bus)       */
struct scsi_shared *shared_ptrs[MAX_ADAPTERS] = {NULL};

/* array containing pointer to scsi bus information (two per bus)            */
extern struct scsi_info   *vsc_scsi_ptrs[];

/* global device driver component dump table pointer                         */
extern struct vsc_cdt_tab *vsc_cdt;

#ifdef VSC_TRACE
extern uint vsc_trace_tab[];
extern uint *vsc_trace_ptr;
#endif VSC_TRACE


/*****************************************************************************/
/* End of global device driver data areas                                    */
/*****************************************************************************/


/*
 * NAME:	vsc_config 
 *
 * FUNCTION: 
 *		protocol driver's configuration routine
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:
 *
 *		If op is INIT, the driver is added to the devsw table and
 *		necessary structures are allocated and initialized.
 *
 *		If op is TERM, the driver is removed from the devsw table
 *  		and resources released.
 *
 *
 * EXTERNAL CALLS:
 *		devswadd            devswdel
 *              lockl               unlockl
 *              bcopy               uiomove
 *		ns_alloc
 *		
 * INPUT:
 *		devno	- major/minor number
 *		op 	- option code(INIT, TERM)
 *		uiop 	- pointer to uio structure
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		ENOMEM	- error allocating resources
 *		EINVAL	- invalid input parameter
 *		EIO	- kernel service failure or invalid operation
 */
int
vsc_config(
	dev_t devno,
	int op,
	struct uio * uiop)
{
struct	devsw		vsc_dsw;		/* dev switch structure   */
struct	scsi_info	*scsi;			/* virtual scsi structure */
struct	scsi_shared	*shared;		/* shared device struct   */
struct  vscsi_ddi       local_ddi;              /* ddi for vscsidd        */
char    ret_code,                               /* function return code   */
        rc;                                     /* local return code      */
int     i_hash;                                 /* index to hash table    */
                                                /* of scsi_info ptrs      */
extern int nodev();                             /* needed for devswadd    */



    /* setup default return code */
    ret_code = 0;
    /* lock on the global lockword */
    (void) lockl(&(vsc_info.vsc_lock), LOCK_SHORT);
 
    /* search for scsi_info structure in configured list */
    /* build the hash index */
    i_hash = minor(devno) & SCSI_HASH;
    scsi = vsc_scsi_ptrs[i_hash];
    while (scsi != NULL) {
        if (scsi->devno == devno)
            break;
        scsi = scsi->next;
    }   /* endwhile */

    switch (op) {

   /************************************************************************/
   /*      handle request to add a virtual device defenition here          */
   /************************************************************************/

    case   CFG_INIT:

        /* NOTE: the DDS passed in at CFG_INIT includes  */
        /* parent's logical name and devno.              */

        /* if this device already configured, return error */
        if (scsi != NULL) {
             ret_code = EIO;
             break;
        }

        if ( vsc_info.num_cfgs == 0 ) {
        /*
         *  driver has yet to be configured,
         *  configure the driver 
         *
         * initialize devswitch structure(devsw) with
         * driver entry points - vsc_open, vsc_close,
         * vsc_ioctl, vsc_strategy, vsc_config, 
         * and vsc_dump.  all others set to nodev or 0
         */

            vsc_dsw.d_open = (int (*) ()) vsc_open;
            vsc_dsw.d_close = (int (*) ()) vsc_close;
            vsc_dsw.d_read = nodev;
            vsc_dsw.d_write = nodev;
            vsc_dsw.d_ioctl = (int (*) ()) vsc_ioctl;
            vsc_dsw.d_strategy = (int (*) ()) vsc_strategy;
            vsc_dsw.d_ttys = 0;
            vsc_dsw.d_select = nodev;
            vsc_dsw.d_config = (int (*) ()) vsc_config;
            vsc_dsw.d_print = nodev;
            vsc_dsw.d_dump = (int (*) ()) vsc_dump;
            vsc_dsw.d_mpx = nodev;
            vsc_dsw.d_revoke = nodev;
            vsc_dsw.d_dsdptr = 0;
            vsc_dsw.d_selptr = 0;
#ifdef _POWER_MP
            vsc_dsw.d_opts = DEV_MPSAFE;
#else
            vsc_dsw.d_opts = 0;
#endif

	    /* add device driver to the devsw table */
            rc = devswadd(devno, &vsc_dsw);
            if ( rc != 0 ) {
	        ret_code = EIO; /* devswadd failed */
                break;
            }
#ifdef VSC_TRACE
            /* initialize internal trace table pointer */
            vsc_trace_ptr = &vsc_trace_tab[0] + 1;
#endif VSC_TRACE
        } /* end if num_cfgs = 0 */

        /*  copy ddi into adapter structure */
        rc = uiomove ((caddr_t) (&local_ddi), 
                      (int) sizeof(struct vscsi_ddi), 
                      UIO_WRITE,   /* from uio space to kernel space */
                      (struct uio *) uiop);
        if (rc != 0) {    /* if copy unsuccessful */
           if (vsc_info.num_cfgs == 0)  {
               (void) devswdel(devno); /* clean up */
           }
           ret_code = EIO;
           break;
        }

        /* allocate scsi_info struct memory and put it in  */
        /* the table of scsi_info_structs                  */
        scsi = vsc_alloc_scsi(devno);

        /* NULL means unsuccesful allocation */
        if (scsi == NULL) {
            if (vsc_info.num_cfgs == 0)  {
                (void) devswdel(devno); /* clean up */
            }
            ret_code = ENOMEM;
            break;
        }

        /*
         *  determine if we need a shared structure.
         *  a shared struct is necessary if sibling
         *  device has yet to be configured.
         *  a shared structure is shared between
         *  sibling device. it contains things
         *  common to all siblings like the parent
         *  adapter's ndd structure and adapter status.
         */

        /* search for shared structure in configured list */
        /* build the hash index */
        i_hash = minor(local_ddi.parent_unit_no) & SCSI_HASH;
        shared = shared_ptrs[i_hash];
        while (shared != NULL) {
            if (shared->p_unit_no == local_ddi.parent_unit_no) {
                break;
            }
            shared = shared->next;
        }   /* endwhile */

        if (shared == NULL) {  /* need to allocated scsi_shared struct */
            /* allocate scsi_shared struct memory and put it in  */
            /* the table of scsi_shared_structs                  */
            shared = vsc_alloc_shared(local_ddi.parent_unit_no);
 
            /* NULL means unsuccessful allocation */
            if (shared == NULL) {
                if (vsc_info.num_cfgs == 0)  {
                    (void) devswdel(devno); /* clean up */
                }
                /* must free memory for scsi_info and remove pointer   */
                /* to it in the table of scsi_info structures          */
                vsc_free_scsi(scsi);
                ret_code = ENOMEM;
                break;
            } /* end if shared==NULL */

            /* initialize the ndd scb_filter structures for initiator and */
            /* target mode */
            shared->im_filter.filtertype = NDD_SCSI;
            shared->im_filter.entity_info.entity_number = 
                                                 local_ddi.sc_im_entity_id;
            /* SCSI initiators never require unsolicited data receive buffers*/
            shared->im_filter.entity_info.num_buffers1 = 0;
            shared->im_filter.entity_info.buffer_size1 = 0;

        } /* end if shared==NULL */

	/* initialize target mode structure if working on an external bus */
	if (local_ddi.location)
	{
            shared->tm_filter.filtertype = NDD_SCSI;
            shared->tm_filter.entity_info.entity_number = 
                                                 local_ddi.sc_tm_entity_id;
            /* SCSI targets requirements for  unsolicited data receive */
            /* buffers is included in the ddi */
            shared->tm_filter.entity_info.num_buffers1 = local_ddi.num_tm_bufs;
            shared->tm_filter.entity_info.buffer_size1 = SC_TM_BUFSIZE;
	}

        /* now shared struct is known to be allocated */
        /* set pointer in scsi_info struct to scsi_shared struct */
        scsi->shared = shared;
        /* increment the shared number of configs to know when to     */
        /* free the shared structure */
        shared->num_cfgs++;
        /* initialize the ndd ns_com_status structure passed to the  */
        /* ns_add_status network service during open */

        /* the mask field of the status_filter struct will be used to pass   */
        /* a correlator to the adapter device driver, so that when async     */
        /* status is received, the adapter driver will return the correlator */
        /* allowing this device driver to quickly acess the proper scsi_info */
        /* struct for the SCSI bus receiving async status.  This correlator  */
        /* will be the address of the scsi_info struct with the least sig    */
        /* bit indicating the bus number on the adapter for which aync       */
        /* status is being registered.                                       */

        /* copy local ddi to scsi_info structure */
        bcopy((char *) &local_ddi, (char *) &scsi->ddi, 
	      sizeof(struct vscsi_ddi));

        scsi->status_filter.filtertype = NDD_SCSI;
        scsi->status_filter.mask = (uint)scsi | scsi->ddi.location;
        /* scsi->status_filter.sid = 0;   not used */
       
        /* increment the global count of configured scsi_info */
        vsc_info.num_cfgs++; 
           
	/* temporary performance enhancement.				*/
	/* To reduce the number of opens issued to the adapter,		*/
	/* the adapter is opened here at CFG_INIT time and left open	*/
	/* until a CFG_TERM is received.				*/
        if (shared->num_cfgs == 1 ) {
            /* call ns_alloc to get ndd structure for parent adapter */
            /* this causes the ns services to issue an open to the   */
            /* adapter driver.                                       */
            rc = ns_alloc(scsi->ddi.parent_lname, &scsi->shared->ndd);
            if (rc != 0) {
        	vsc_info.num_cfgs--; 
        	scsi->shared->num_cfgs--;
                if (vsc_info.num_cfgs == 0)  {
                    (void) devswdel(devno); /* clean up */
                }

        	if (scsi->shared->num_cfgs == 0) {
            	    vsc_free_shared(scsi->shared);
                }
        
		/* free the scsi struct */
		vsc_free_scsi(scsi);
                ret_code = ENOMEM;
                break;
            }
	}
           
        break;

/************************************************************************/
/*      handle request to terminate an adapter here                     */
/************************************************************************/
    case   CFG_TERM:

        if (scsi == NULL) {  /* already terminated */
            ret_code = 0;
            break;
        }
        /*  if no occurrences of open devices left unconfigure device. */
        if (scsi->opened) {
            ret_code = EBUSY;
            break;
        }
 
        scsi->shared->num_cfgs--; /* decrement number of cfgs of shared */
        /* determine if shared structure needs to be freed */
        if (scsi->shared->num_cfgs == 0) {
	    /* issue the ns_free call to reliquish access to the ndd */
            (void) ns_free(scsi->shared->ndd);
            vsc_free_shared(scsi->shared);
        }
        vsc_free_scsi(scsi);    /* free the scsi_info struct     */
        vsc_info.num_cfgs--;    /* decrement global config count */

        if (vsc_info.num_cfgs == 0)  {
            (void) devswdel(devno); /* clean up */
        }

        break;

/************************************************************************/
/*      handle invalid config parameter here                            */
/************************************************************************/

    default:
        ret_code = EINVAL;
        break;
    }  /* end switch (op) */

    unlockl(&(vsc_info.vsc_lock));
    return(ret_code);

}  /* end vsc_config */

/*
 * NAME:     vsc_alloc_scsi
 *
 * FUNCTION:
 *              This internal routine performs action required to obtain 
 *              the required memory to hold needed adapter structures.  Then,
 *              if successful, it save the pointer to the scsi_info structure
 *              in the global pointer array, and returns the pointer to the 
 *              caller as a return value.
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *              xmalloc          bzero     
 *
 * INPUT:
 *              devno   - major/minor number
 *
 * RETURNS:
 *              NULL if unsuccessful completion (no resources have been  
 *              allocated)
 *              non_NULL indicates successful completion and return value
 *              is a pointer to the scsi_info struct.
 */

struct scsi_info *
vsc_alloc_scsi(
         dev_t devno)
{
    struct scsi_info *scsi, /* structure returned to caller if successful */
                 *tmp_scsi; /* structures used to locate proper position  */
                            /* in scsi_info array if hashing is neccesary */
    int           i_hash;   /* index into global scsi_info ptr table      */
    
    /* malloc scsi_info struct from pinned heap */
    scsi = (struct scsi_info *) xmalloc((uint) sizeof(struct scsi_info), 
                                        8, pinned_heap);
    /* check the results from xmalloc */
    if (scsi == NULL) {
        return(scsi);
    }
    /* initially, clear the scsi_info structre */
    bzero((char *)scsi, sizeof(struct scsi_info));

    /* add the pointer to this scsi_info struct to the global pointer table */
    i_hash = minor(devno) & SCSI_HASH;       /* get hash pointer       */
    if (vsc_scsi_ptrs[i_hash] == NULL) {     /* if no chain here       */
        vsc_scsi_ptrs[i_hash] = scsi;        /* store in pointer table */
    } 
    else {  /* points to a chain */ 
        tmp_scsi = vsc_scsi_ptrs[i_hash];    /* point at first one   */
        while(tmp_scsi->next != NULL) {      /* loop until end found */
            tmp_scsi = tmp_scsi->next;       /* look at next struct  */
        }
        tmp_scsi->next = scsi;               /* add at end of chain  */
    }
    scsi->next = NULL;   /* mark new end of chain */ 
    scsi->devno = devno; /* store the scsi devno  */

    return(scsi);

} /* end vsc_alloc_scsi */


/*
 * NAME:     vsc_alloc_shared
 *
 * FUNCTION:
 *              This internal routine performs action required to obtain
 *              the required memory to hold needed scsi_shared structures.  
 *              There will be one scsi_shared structure per adapter shared
 *              by all the virtual scsi ports which may be supported by that
 *              adapters.  If successful, it save the pointer to the 
 *              scsi_shared structure in the global pointer array, and returns 
 *              the pointer to the caller as a return value.
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *              xmalloc          bzero
 *
 * INPUT:
 *              p_unit_no   - unit number of parent adapter
 *
 * RETURNS:
 *              NULL if unsuccessful completion (no resources have been
 *              allocated)
 *              non_NULL indicates successful completion and return value
 *              is a pointer to the scsi_shared struct.
 */

struct scsi_shared *
vsc_alloc_shared(
         uint p_unit_no)
{
    struct scsi_shared *shared, /* structure returned to caller if successful*/
                 *temp_shared;  /* structures used to locate proper position */
                                /* in scsi_shared array if hashing is        */
                                /* neccesary                                 */
    int           i_hash;       /* index into global scsi_shared ptr table   */

    /* malloc scsi_shared struct space, cache line alligned from pinned heap */
    shared = (struct scsi_shared *) xmalloc((uint) sizeof(struct scsi_shared),
                                        8, pinned_heap);
    /* check the results from xmalloc */
    if (shared == NULL) {
        return(shared);
    }

    /* initially, clear the scsi_shared structre */
    bzero((char *)shared, sizeof(struct scsi_shared));

    /* add the pointer to this scsi_shared struct to the global pointer table*/
    i_hash = minor(p_unit_no) & SCSI_HASH;   /* get hash pointer       */
    if (shared_ptrs[i_hash] == NULL) {     /* if no chain here       */
        shared_ptrs[i_hash] = shared;      /* store in pointer table */
    }
    else {  /* points to a chain */
        temp_shared = shared_ptrs[i_hash];   /* point at first one   */
        while(temp_shared->next != NULL)     /* loop until end found */
            temp_shared = temp_shared->next; /* look at next struct  */
        temp_shared->next = shared;          /* add at end of chain  */
    }
    shared->next = NULL;       /* mark new end of chain    */
    shared->p_unit_no = p_unit_no; /* store the parents devno  */

    return(shared);

} /* end vsc_alloc_shared */


/*
 * NAME:     vsc_free_scsi
 *
 * FUNCTION:
 *              This routine performs the required action to remove a
 *              scsi_info struct from the global pointer table and free the
 *              pinned memory used for that structure
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES:       This routine is used during error recover of CFG_INIT and
 *              normal path of CFG_TERM
 *
 * EXTERNAL CALLS:
 *              xmfree
 *
 * INPUT:
 *              scsi - pointer to the scsi_info to be freed
 *
 * RETURNS:
 *              none
 */
void
vsc_free_scsi(
         struct scsi_info * scsi)
{
    struct scsi_info *tptr; /* temp scsi_info ptr to walk vsc_scsi_ptrs    */
                            /* table of scsi_info structs                  */
    int i_hash;             /* index into vsc_scsi_ptrs table of scsi_info */
                            /* structs                                     */

    /* It is already known that scsi is conatained in the vsc_scsi_ptrs */
    /* table. Find and remove it from the table                         */
    i_hash = minor(scsi->devno) & SCSI_HASH;
    if(vsc_scsi_ptrs[i_hash] == scsi) {
        vsc_scsi_ptrs[i_hash] = scsi->next; /* remove scsi from table */
    }
    else {
        tptr = vsc_scsi_ptrs[i_hash];  /* starting pointer */
        while (tptr->next !=NULL) {    /* follow chain     */
            if (tptr->next == scsi) {
                tptr->next = scsi->next; /* remove scsi from table */
                break;
            }
            tptr = tptr->next;  /* look at next element */
        } /* end while */
    } /* end else */

    /* free the pinned memory for this scsi_info struct */
    (void) xmfree((void *) scsi, pinned_heap);

} /* end vsc_free_scsi */


/*
 * NAME:     vsc_free_shared
 *
 * FUNCTION:
 *              This routine performs the required action to remove a
 *              scsi_shared struct from the global pointer table and free the
 *              pinned memory used for that structure
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES:       This routine is used during error recover of CFG_INIT and
 *              normal path of CFG_TERM
 *
 * EXTERNAL CALLS:
 *              xmfree
 *
 * INPUT:
 *              shared - pointer to the scsi_shared to be freed
 *
 * RETURNS:
 *              none
 */
void
vsc_free_shared(
         struct scsi_shared *shared)
{
    struct scsi_shared *tptr; /* temp scsi_shared ptr to walk shared_ptrs    */
                              /* table of scsi_shared structs */
    int i_hash;               /* index into shared_ptrs table of scsi_shared */
                              /* structs */

    /* it is already known that shared is conatained in the shared_ptrs     */
    /* table. Find and remove it from the table */
    i_hash = minor(shared->p_unit_no) & SCSI_HASH;
    if(shared_ptrs[i_hash] == shared) {
        shared_ptrs[i_hash] = shared->next; /* remove shared from table */
    }
    else {
        tptr = shared_ptrs[i_hash];    /* starting pointer */
        while (tptr->next !=NULL) {    /* follow chain     */
            if (tptr->next == shared) {
                tptr->next = shared->next; /* remove shared from table */
                break;
            }
            tptr = tptr->next;  /* look at next element */
        } /* end while */
    } /* end else */

    /* free the pinned memory for this scsi_shared struct */
    (void) xmfree((void *) shared, pinned_heap);

} /* end vsc_free_shared */


/*
 * NAME:     vsc_open
 *
 * FUNCTION: 
 *		This function opens the virtual SCSI device and makes it ready.
 *              It allocates and initializes necessary resources to perform
 *              the open.
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:
 *		This function uses the NS services to establish a channel with 
 *		the adapter driver.  A failed open function is used to back out
 *              of an open when an error occurs.  Diagnostic mode open is not
 *              supported by this device.
 *
 *
 * EXTERNAL CALLS:
 *		lockl             unlockl
 *              pincode           unpincode
 *              privcheck         e_sleep
 *		ns_add_filter     xmalloc
 *              bzero		  w_init
 *
 * INPUT:
 *		devno	- major/minor number
 *		devflag - indicates kernel or user caller
 *		chan 	- not used
 *		ext 	- defines mode of operation
 *			  0 - normal mode
 *			  1 - diagnostic mode
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EPERM   - access requires CTL_DEV authority
 *              EINVAL  - diagnostic mode open not supported
 *		ENOMEM  - resource allocation failed
 *		EIO	- kernel service failure or invalid operation
 */

int
vsc_open(
         dev_t devno,
         ulong devflag,
         int chan,
         int ext)
{
    struct  scsi_info   *scsi;       /* pointer to the scsi_info struct for  */
                                     /* the given devno                      */
    struct  ns_user     scsi_nsuser; /* a struct passed to network services  */
                                     /* with the ns_add_filter call          */
    struct  ns_statuser scsi_stat;   /* a struct passed to network services  */
                                     /* with the ns_add_status call          */
    int     rc,                      /* locally used return code             */
            ret_code,                /* return code for this function        */
            undo_level,              /* indicates where opened failed        */
            i_hash;                  /* index to hash table of scsi_info ptrs*/


    ret_code = 0;             /* setup default return code       */
    undo_level = 0;           /* init undo level for failed open */

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_OPEN, ret_code, devno, devflag,
            chan, ext, 0);

    /* lock on the global lockword */
    (void) lockl(&(vsc_info.vsc_lock), LOCK_SHORT);
 
    /* search for scsi_info structure in configured list */
    /* build the hash index */
    i_hash = minor(devno) & SCSI_HASH;
    scsi = vsc_scsi_ptrs[i_hash];
    while (scsi != NULL) {
        if (scsi->devno == devno)
            break;
        scsi = scsi->next;
    }   /* endwhile */

    if (scsi == NULL) {   /* structure not yet initialized */
        ret_code = EIO;
        vsc_fail_open(undo_level, NULL, ret_code, devno);
        return(ret_code);
    }

    if (ext & SC_DIAGNOSTIC) { /* diagnostic mode not is not supported */
        ret_code = EINVAL;
        vsc_fail_open(undo_level, scsi, ret_code, devno);
        return(ret_code);
    }

    /* if normal mode requested, and not called from kernel process, */
    /* then dev_config authority must be set or open is not allowed  */

    if ((privcheck(DEV_CONFIG) != 0) && (!(devflag & DKERNEL))) {  
            ret_code = EPERM;
            vsc_fail_open(undo_level, scsi, ret_code, devno);
            return (ret_code);
    }

    if (!scsi->opened) {   /* first open to this vscsi device */

        if (vsc_info.num_open_scsi == 0) {

            /* here, this is first time any adapter is open, so execute */
            /* things which affect the driver globally.                 */

            rc = pincode(vsc_recv); /* pin the driver */
            if (rc != 0) {          /* pin failed     */
                ret_code = EIO;
                vsc_fail_open(undo_level, scsi, ret_code, devno);
                return(ret_code);
            }
            undo_level = 5;

            /* allocate and set up the component dump table entry */
            vsc_cdt = (struct vsc_cdt_tab *) xmalloc(
                                    (uint) sizeof(struct vsc_cdt_tab),
                                                 (uint) 2, pinned_heap);

            /* error in dump table memory allocation */
            if (vsc_cdt == NULL)  {
                    ret_code = ENOMEM;
                    vsc_fail_open(undo_level, scsi, ret_code, devno);
                    return (ret_code);
            }
            undo_level = 10;

            /* initialize the storage for the dump table */
            bzero((char *) vsc_cdt, sizeof(struct vsc_cdt_tab));
            rc = dmp_add(vsc_cdt_func);
            if (rc != 0) {
                ret_code = EIO;
                vsc_fail_open(undo_level, scsi, ret_code, devno);
                return (ret_code);
            }
            undo_level = 15;
        }  /* end first open of any vscsi device */
 
        /* if this is the first open to a device on this adapter */
        if (scsi->shared->num_of_opens == 0) {
            /* initialize the ns_user struct for the initiator ns_add_filter */
            scsi_nsuser.isr = (void (*) ()) vsc_recv; 
            scsi_nsuser.isr_data = NULL;
            scsi_nsuser.protoq = NULL;
            scsi_nsuser.netisr = 0;
            scsi_nsuser.pkt_format = NS_PROTO;
            scsi_nsuser.ifp = NULL;
            /* issue the ns_add_filter call for this SCSI initiator instance */
            rc = ns_add_filter(scsi->shared->ndd, &scsi->shared->im_filter, 
                               sizeof(struct scb_filter), &scsi_nsuser);
            if (rc != 0) {
                ret_code = EIO;
                vsc_fail_open(undo_level, scsi, ret_code, devno);
                return (ret_code);
            }
         
        } /* end if shared num_opens == 0 */

        undo_level = 25;

        /* init ns_statuser structure passed to the ns_add_status call */
        scsi_stat.isr = (void (*) ()) vsc_async_stat;

        /* scsi_stat.isr_data = 0;  not used */

        rc = ns_add_status(scsi->shared->ndd, &scsi->status_filter, 
                          sizeof(struct ns_com_status), &scsi_stat);
        if (rc != 0) {
            ret_code = EIO;
            vsc_fail_open(undo_level, scsi, ret_code, devno);
            return(ret_code);
        }
        undo_level = 30;
        /* allocate the pool of command info elements */
        rc = vsc_alloc_cmdpool(scsi);
        if (rc != 0) {
            ret_code = ENOMEM;
            vsc_fail_open(undo_level, scsi, ret_code, devno);
            return (ret_code);
        }
        /* initialize the scsi_info structure */
        ASSERT(scsi->scsi_lock == 0 || scsi->scsi_lock== LOCK_AVAIL);
        scsi->scsi_lock = LOCK_AVAIL;
#ifdef _POWER_MP
        lock_alloc(&scsi->ioctl_lock,LOCK_ALLOC_PIN, VSC_IOCTL_LOCK_CLASS,
                   minor(devno));
        simple_lock_init(&scsi->ioctl_lock);
#endif
        scsi->ioctl_event = EVENT_NULL;
        /* initialize command delay after reset timer */
        scsi->cdar_wdog.dog.next = NULL;
        scsi->cdar_wdog.dog.prev = NULL;
        scsi->cdar_wdog.dog.func = vsc_watchdog;
        scsi->cdar_wdog.dog.count = 0;
        scsi->cdar_wdog.dog.restart = scsi->ddi.cmd_delay;
        scsi->cdar_wdog.timer_id = SCSI_CDAR_TMR;
        scsi->cdar_wdog.save_time = 0; 
        scsi->cdar_wdog.scsi = scsi; 
#ifdef _POWER_MP
        while(w_init(&scsi->cdar_wdog.dog));
#else
        w_init(&scsi->cdar_wdog.dog);
#endif

        /* initialize SCSI bus reset timer */
        scsi->reset_wdog.dog.next = NULL;
        scsi->reset_wdog.dog.prev = NULL;
        scsi->reset_wdog.dog.func = vsc_watchdog;
        scsi->reset_wdog.dog.count = 0;
        scsi->reset_wdog.dog.restart = RESET_CMD_T_O;
        scsi->reset_wdog.timer_id = SCSI_RESET_TMR;
        scsi->reset_wdog.save_time = 0; 
        scsi->reset_wdog.scsi = scsi; 
#ifdef _POWER_MP
        while(w_init(&scsi->reset_wdog.dog));
#else
        w_init(&scsi->reset_wdog.dog);
#endif

        /* initialize the SCSI bus reset cmd_elem */
        vsc_init_cancel_cmd_elem(scsi, &scsi->reset_cmd_elem);
        scsi->reset_cmd_elem.request.scsi_cdb.media_flags |= 
                                             (VSC_SCSI_RESET | VSC_RESUME);
        scsi->reset_cmd_elem.cmd_type = INTR_LVL_RESET;
        /* mark this virtual scsi instance as open */
        scsi->opened = TRUE;
        /* increment the shared number of opens counter */
        scsi->shared->num_of_opens++;
        /* increment the global count of open scsi instances */
        vsc_info.num_open_scsi++; 
    } /* end if !scsi->open */     
            
    unlockl(&(vsc_info.vsc_lock));
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
    return(ret_code);

} /* end vsc_open */


/*
 * NAME:     vsc_fail_open
 *
 * FUNCTION:
 *              This function does the appropriate error recovery to back out
 *              of a failed open.  When a system call fails during open, this
 *              routine is involked with an undo level indicating at what
 *              point the open failed.
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES:
 *              This function uses the NS services to relinquish a channel 
 *              with the adapter driver.
 *
 *
 * EXTERNAL CALLS:
 *              w_clear               dmp_del
 *              xmfree                unpincode
 *              unlockl
 * 
 * INPUT:
 *              undo_level - indicates at which point the open failed
 *              scsi       - pointer to the scsi_info struct which is to be
 *                           closed
 *
 * RETURNS:
 *              NONE
 */

void
vsc_fail_open(
        int undo_level,
        struct scsi_info *scsi,
        int ret_code, 
        dev_t devno) 
{

    switch (undo_level) {
              
    case 35 : /* clear the timers, free ioctl mp lock and dealloc */
              /*  the cmd_pool */
#ifdef _POWER_MP
        lock_free(&scsi->ioctl_lock);
        while(w_clear(&scsi->cdar_wdog.dog));
        while(w_clear(&scsi->reset_wdog.dog));
#else
        w_clear(&scsi->cdar_wdog.dog);
        w_clear(&scsi->reset_wdog.dog);
#endif
        /* decrement the number of shared opens */
        scsi->shared->num_of_opens--;
        /* free the command element pool */
        (void) xmfree((void *) scsi->cmd_pool, pinned_heap);
         
    case 30 : /* issue the ns_del_status call to delete the status filter */
        (void) ns_del_status(scsi->shared->ndd, &scsi->status_filter,
                                 sizeof(struct ns_com_status));

    case 25 : /* issue ns_del_filter call */
        if (scsi->shared->num_of_opens == 0) { 
            (void) ns_del_filter(scsi->shared->ndd, &scsi->shared->im_filter, 
                         sizeof(struct scb_filter));
        }
    case 15 : /* issue dmp_del call to unregister for dump */ 
        if (vsc_info.num_open_scsi == 0) {
            (void) dmp_del(vsc_cdt_func);
        }
    case 10 : /* free component dump table */
        if (vsc_info.num_open_scsi == 0) {
            (void) xmfree((void *) vsc_cdt, pinned_heap);
        }
    case 5  : /* unpin this device driver */
        if (vsc_info.num_open_scsi == 0) {
            (void) unpincode(vsc_recv);
            if(vsc_info.top_pinned) {
                (void) unpincode(vsc_open);
                vsc_info.top_pinned = FALSE;
            }
        }
    default : 
        break;
    } /* end switch */

    if (undo_level == 35) { /* this is called from close */
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
    }
    else {  /* this is a failed open */
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
    }
    unlockl(&(vsc_info.vsc_lock));

} /* end vsc_fail_open */

/*
 * NAME:     vsc_alloc_cmdpool
 *
 * FUNCTION:
 *              Allocates the pool of command elements for all virtual SCSI
 *              instance sharing the same physical adapter.  Command elemements
 *              are the interface structure passed from the protocol layer to
 *              the adapter driver.
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES:
 *
 * EXTERNAL CALLS:
 *               xmalloc                  bzero
 *
 * INPUT:
 *		scsi	  - scsi_info structure ptr for the device to alloc
 *                          the command pool.
 *
 * RETURNS:
 *              0 for good completion,  ERRNO on error
 *              ENOMEM     - xmalloc failure 
 *
 */
int
vsc_alloc_cmdpool(
        struct scsi_info *scsi)
{
    int fullwords,                /* number of full words of cmd_elements to */
                                  /* allocate in cmd_pool                    */
        resid_bits,               /* number of bits in the last word of      */
                                  /* free_cmd_list for given num_cmd_elems   */
        i, j;                     /* for loop variables                      */

    scsi->cmd_pool = (struct cmd_elem *)xmalloc(sizeof(struct cmd_elem) * 
                                  scsi->ddi.num_cmd_elems, 12,pinned_heap);
    if (scsi->cmd_pool == NULL) 
        return(ENOMEM);

    /* zero out command element pool */
    bzero ((char *)scsi->cmd_pool, 
            sizeof(struct cmd_elem) * scsi->ddi.num_cmd_elems);

    fullwords = scsi->ddi.num_cmd_elems / VSC_BITS_PER_WORD;
    resid_bits = scsi->ddi.num_cmd_elems % VSC_BITS_PER_WORD;

    /* initialize words in free cmd list */
    for (i = 0; i < fullwords; i++)
        scsi->free_cmd_list[i] = 0xffffffff;

    /* initialize bits in last word of free cmd list */
    for (j = 0; j < resid_bits; j++)
        scsi->free_cmd_list[i] |= (0x01 << ((VSC_BITS_PER_WORD - 1) - j));


    /* initialize necessary fields of the cmd_info structures */
    for (i = 0; i < scsi->ddi.num_cmd_elems; i++) {
        /* setup the scsi_info pointer in the cmd_elem */
        scsi->cmd_pool[i].scsi = scsi;
        /* set the cmd_state to INACTIVE; this is a no-op because of bzero */
        /* scsi->cmd_pool[i].cmd_state = INACTIVE */
        /* set the tag number corresponding to this cmd_elem's position  */
        /* in the free_cmd_list */
        scsi->cmd_pool[i].tag = i;
        /* all of these cmd_elems are used for strategy i/o cmds */
        scsi->cmd_pool[i].cmd_type = DEV_STRAT_CMD;
        /* init the control element block struct in the cmd_elem */
        scsi->cmd_pool[i].ctl_elem.num_pd_info = 1;
        /* scsi->cmd_pool[i].ctl_elem.key = 0x0; */
        /* scsi->cmd_pool[i].ctl_elem.status = 0x0; */
        scsi->cmd_pool[i].ctl_elem.reply_elem =
                  (caddr_t)&scsi->cmd_pool[i].reply;
        scsi->cmd_pool[i].ctl_elem.reply_elem_len =
                  sizeof(struct rpl_element_def);
        scsi->cmd_pool[i].ctl_elem.ctl_elem =
                  (caddr_t)&scsi->cmd_pool[i].request;
        scsi->cmd_pool[i].ctl_elem.pd_info =
                  &scsi->cmd_pool[i].pd_info1;

        /* init the pd_info struct in the cmd_elem */
        scsi->cmd_pool[i].pd_info1.next = NULL;
        scsi->cmd_pool[i].pd_info1.buf_type = P_BUF;

        /* init the request element struct in the cmd_elem     */
        /* start with the ctl_elem_hdr of the request element  */
        /* scsi->cmd_pool[i].request.header.format = 0x0;      */
        scsi->cmd_pool[i].request.header.length =
                  sizeof(struct ctl_element_def);
        scsi->cmd_pool[i].request.header.options = 0x46;
        /* scsi->cmd_pool[i].request.header.reserved = 0x0;    */
        /* scsi->cmd_pool[i].request.header.src_unit = 0x0;    */
        /* scsi->cmd_pool[i].request.header.src_entity = 0x0;  */
        /* scsi->cmd_pool[i].request.header.dest_unit = 0x0;   */
        /* scsi->cmd_pool[i].request.header.dest_entity = 0x0; */
        scsi->cmd_pool[i].request.header.correlation_id =
                              (uint)&scsi->cmd_pool[i].ctl_elem;

        /* init the type 2 parameter descriptor in the request element */
        /*scsi->cmd_pool[i].request.type2_pd.desc_number = 0x0;*/
        scsi->cmd_pool[i].request.type2_pd.ctl_info = 0x8280;
        /* the destination address of the type2 pd in the request element  */
        /* specifies which of the SCSI busses the device for which this    */
        /* command is for resides */
        scsi->cmd_pool[i].request.type2_pd.word1 = scsi->ddi.location;
        /*scsi->cmd_pool[i].request.type2_pd.word2 = 0x0;      */
        /*scsi->cmd_pool[i].request.type2_pd.word3 = 0x0;      */

        /* init the type 1 parameter descriptor in the request element */
        /*scsi->cmd_pool[i].request.type1_pd.desc_number = 0x0;*/
        scsi->cmd_pool[i].request.type1_pd.ctl_info = 0x0180;
        scsi->cmd_pool[i].request.type1_pd.word1 =
                                  sizeof(struct ctl_element_def);
        /*scsi->cmd_pool[i].request.type1_pd.word2 = 0x0;      */
        /*scsi->cmd_pool[i].request.type1_pd.word3 = 0x0;      */

        /* init the scsi_cdb struct in the request element     */
        /*scsi->cmd_pool[i].request.scsi_cdb.next_addr1 = 0x0; */
        /*scsi->cmd_pool[i].request.scsi_cdb.next_addr2 = 0x0; */
        /*scsi->cmd_pool[i].request.scsi_cdb.scsi_extra = 0x0; */

    } /* end for */

    return(0);

} /* end vsc_alloc_cmd_pool */

/*
 * NAME:     vsc_init_cancel_cmd_elem
 *
 * FUNCTION:
 *           This routine, given a pointer to a cmd_elem will initialize the
 *           cmd_elem for use as a cancel (abort, BDR, bus reset, resume)
 *           command type cmd_elem.  cmd_elems used for general send SCSI
 *           commands require different initialization and is done in
 *           vsc_alloc_cmd_pool.
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES:    This function assumes the cmd_elem has been bzeroed before this
 *           function is called.
 *       
 *
 * EXTERNAL CALLS:
 *
 * INPUT:
 *              scsi      - pointer to scsi_info struct associated with the
 *                          given cmd_elem
 *              cmd       - pointer to the cmd_elem to be initialized
 *
 * RETURNS:
 *
 *              NONE
 */

void
vsc_init_cancel_cmd_elem(
    struct scsi_info *scsi,
    struct cmd_elem *cmd)
{
  
    cmd->scsi = scsi;  /* set the scsi pointer in cmd_elem */
    /* set the cmd state to indicate cmd_elem not acive */
    /* cmd->cmd_state = INACTIVE; */
 
    /*
     * init the control element block struct in the command element
     * this command element is dedicated for building cancel request elements
     */
    cmd->ctl_elem.flags = CANCEL | EXPEDITE;
    /*
     * cmd->ctl_elem.num_pd_info = 0;
     * cmd->ctl_elem.key = 0x0;
     * cmd->ctl_elem.status = 0x0;
     */
    cmd->ctl_elem.reply_elem = (caddr_t)&cmd->reply;
    cmd->ctl_elem.reply_elem_len = sizeof(struct rpl_element_def);
    cmd->ctl_elem.ctl_elem = (caddr_t)&cmd->request;
    /* cancel requests do not require a pd_info struct */
    cmd->ctl_elem.pd_info = NULL;

    /*
     * init the request element struct in the command element
     * start with the ctl_elem_hdr of the request element
     */
    cmd->request.header.length = sizeof(struct ctl_element_def);
    cmd->request.header.options = 0x46;
    /*
     * cmd->request.header.format = 0x0;
     * cmd->request.header.reserved = 0x0;
     * cmd->request.header.src_unit = 0x0;
     * cmd->request.header.src_entity = 0x0;
     * cmd->request.header.dest_unit = 0x0;
     * cmd->request.header.dest_entity = 0x0;
     */

    cmd->request.header.correlation_id = (uint)&cmd->ctl_elem;

    /* init the type 2 parameter descriptor in the request element */
    /* cmd->request.type2_pd.desc_number = 0x0;                        */
    cmd->request.type2_pd.ctl_info = 0x8280;
    /* the destination address of the type2 pd in the request element  */
    /* specifies which of the SCSI busses the device for which this    */
    /* command is for resides */
    cmd->request.type2_pd.word1 = scsi->ddi.location;
    /*
     * cmd->request.type2_pd.word2 = 0x0;
     * cmd->request.type2_pd.word3 = 0x0;
     */

    /* init the type 1 parameter descriptor in the request element */
    cmd->request.type1_pd.ctl_info = 0x180;
    cmd->request.type1_pd.word1 = sizeof(struct ctl_element_def);
    /*
     * cmd->request.type1_pd.desc_number = 0x0;
     * cmd->request.type1_pd.word2 =  0x0;
     * cmd->request.type1_pd.word3 =  0x0;
     */

    /*
     * init the scsi_cdb struct in the request element
     * cmd->request.scsi_cdb.next_addr1 = 0x0;
     * cmd->request.scsi_cdb.next_addr2 = 0x0;
     * cmd->request.scsi_cdb.scsi_cmd_blk = 0x0;
     * cmd->request.scsi_cdb.scsi_extra = 0x0;
     * cmd->request.scsi_cdb.scsi_data_length = 0x0;
     */


} /* end vsc_init_cancel_cmd_elem */


/*
 * NAME:     vsc_close
 *
 * FUNCTION: 
 *		This function closes an instance of a virtual SCSI bus and
 *              releases any resources that were allocated at the time of
 *              the open.
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:
 *              This function uses the vsc_clear_dev and vsc_stop_tgt functions
 *              to stop any devices which may be active on this SCSI bus during
 *              the close.
 *              This function makes use of the vsc_fail_open function for much
 *              of the close processing (close and failed open are somewhat
 *              synonymous).
 *    
 *              
 *
 * EXTERNAL CALLS:
 *		lockl               unlockl
 *              bzero
 *		
 * INPUT:
 *		devno	- major/minor number
 *		chan 	- not used
 *		ext 	- not used
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		EIO	-  invalid operation
 *
 */
int
vsc_close(
	dev_t devno,
	int   chan)
{

    struct  scsi_info   *scsi;       /* pointer to the scsi_info struct for  */
                                     /* the given devno                      */
    int     i,                       /* for loop variable                    */
            ret_code,                /* return code for this function        */
            i_hash;                  /* index to hash table of scsi_info ptrs*/
    struct  sc_stop_tgt stop_tgt;    /* used to clean up open target devices */


    ret_code = 0;
    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_CLOSE, ret_code, devno);
    /* lock on the global lockword */
    (void) lockl(&(vsc_info.vsc_lock), LOCK_SHORT);
 
    /* search for scsi_info structure in configured list */
    /* build the hash index */
    i_hash = minor(devno) & SCSI_HASH;
    scsi = vsc_scsi_ptrs[i_hash];
    while (scsi != NULL) {
        if (scsi->devno == devno)
            break;
        scsi = scsi->next;
    }   /* endwhile */
 
    if ((scsi == NULL) || (!scsi->opened)) {
        /* error, device not configured or opened */
        ret_code = EIO;   
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
        unlockl(&(vsc_info.vsc_lock));
        return(ret_code);
    }
   
    /* check all initiator devices on this virtual SCSI bus and close those */
    /* that are still open */
    for (i = 0; i < DEVPOINTERS; i++) {
        if (scsi->dev[i] != NULL) {  /* device still open */
            (void) vsc_clear_dev(scsi, i);
        }
    }  /* end for */ 

    /* check all target devices on this virtual SCSI bus and close those */
    /* that are still open */
    /* zero out the stop target structure */
    bzero((caddr_t) & stop_tgt, sizeof(struct sc_stop_tgt));
    for (i = 0; i < TMPOINTERS; i++) {
        if (scsi->tm[i] != NULL) {  /* device still open */
            stop_tgt.id = i;
            (void) vsc_dealloc_tgt(scsi, (int) &stop_tgt, DKERNEL);
        }
    }  /* end for */ 

    /* mark this virtual SCSI instance as closed */
    scsi->opened = FALSE;
    /* decrement global count of open  SCSI instances */
    vsc_info.num_open_scsi--;
    /* call the fail open routine to do the remainder of the close processing */
    /* vsc_fail_open will release the lock ! */
    vsc_fail_open(35, scsi, ret_code, devno);

    return(ret_code);

}  /* end vsc_close */


/*
 * NAME:	vsc_ioctl 
 *
 * FUNCTION:    SCSI protocol driver IOCTL routine
 *              This function performs certain specific commands as indicated
 *              by the IOCTL option
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:
 *      supported IOCTL commands are :
 *   
 *      IOCINFO		- Returns information about the adapter.
 *      SCIOSTART	- Open a SCSI ID/LUN.                 
 *   +  SCIOSTOP	- Close a SCSI ID/LUN.               
 *   +  SCIOHALT	- Halt active command and fail queue of device.
 *   +  SCIOINQU	- Issue a SCSI Device Inquiry command.
 *   +  SCIORESET	- Send a Bus Device Reset msg to a SCSI device.
 *	SCIODIAG	- No operation. Returns ENXIO.
 *      SCIOTRAM	- No operation. Returns ENXIO. 
 *      SCIODNLD	- No operation. Returns ENXIO. 
 *   +  SCIOSTUNIT	- Issue a SCSI Start Unit command.   
 *   +  SCIOTUR		- Issue a SCSI Test Unit Ready command.
 *   +  SCIOREAD	- Issue a SCSI Read command (6-byte). 
 *   +  SCIOEVENT	- Register a device for async event notification
 *	SCIOSTARTTGT	- Open a SCSI target device on this bus
 *	SCIOSTOPTGT	- Close a SCSI target device on this bus
 *	SCIOGTHW	- Returns 0 if gathered write is supported 
 *                                                               
 *   +	These ioctl options require an SCIOSTART before they are run.
 *
 * EXTERNAL CALLS:
 *              lockl                  unlockl
 *              copyout                bcopy
 *              pincode
 *		
 * INPUT:
 *		devno	- major/minor number
 *		cmd	- indicates which operation to execute
 *		arg 	- pointer to the argument(s) that ioctl work with
 *		devflag - indicates kernel or user caller
 *		chan 	- not used
 *		ext 	- not used
 *
 * RETURNS:  
 *              (see also the prologs of the individual ioctl functions)
 *		0 for good completion,  ERRNO on error
 *		EIO	- kernel service failure or invalid operation
 *		EINVAL	- invalid input paramenter
 *		ENXIO	- operation not supported
 *
 */

int
vsc_ioctl(
	dev_t	devno,
	int	cmd,
	int	arg,
	ulong	devflag,
	int	chan,
	int	ext)
{
    struct  scsi_info   *scsi;       /* pointer to the scsi_info struct for  */
                                     /* the given devno                      */
    struct  devinfo     scinfo;      /* structure for IOCINFO ioctl          */
    int     rc,                      /* locally used return code             */
            ret_code,                /* return code for this function        */
            i_hash;                  /* index to hash table of scsi_info ptrs*/
   int (*addr)() = vsc_ioctl;

    ret_code = 0; /* default to no errors found */

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_IOCTL, ret_code, devno, cmd, devflag,
            chan, ext);

    /* lock on the global lockword */
    (void) lockl(&(vsc_info.vsc_lock), LOCK_SHORT);
 
    /* search for scsi_info structure in configured list */
    /* build the hash index */
    i_hash = minor(devno) & SCSI_HASH;
    scsi = vsc_scsi_ptrs[i_hash];
    while (scsi != NULL) {
        if (scsi->devno == devno)
            break;
        scsi = scsi->next;
    }   /* endwhile */

    if ((scsi == NULL) || (!scsi->opened)) {
        /* error, device not configured or opened */
        ret_code = EIO;   
        unlockl(&(vsc_info.vsc_lock));
        return(ret_code);
    }
 
    /* lock the scsi_info struct lock to serialize ioctls per virtual */
    /* SCSI bus instance */
    (void) lockl(&(scsi->scsi_lock), LOCK_SHORT);

    /* release the global lock */
    unlockl(&(vsc_info.vsc_lock));
    
    /* validate operation and process it */
    switch (cmd) {

    case IOCINFO:       
        /*  return device information. Note: max transfer was given to us */
        /*  in the parent adapter's ndd structure.                        */

        scinfo.devtype = DD_BUS;   /* say this is a bus device */
        scinfo.flags = 0;
        scinfo.devsubtype = DS_SCSI;
        scinfo.un.scsi.card_scsi_id = (char)scsi->ddi.bus_scsi_id;
        scinfo.un.scsi.max_transfer = scsi->shared->ndd->ndd_mtu;
        if (!(devflag & DKERNEL)) {  /* for a user process */
            rc = copyout((char *) &scinfo, (char *) arg, 
			 sizeof(struct devinfo));
            if (rc != 0) {
                ret_code = EFAULT;
            }
        } 
        else {  
            /* for a kernel process */
            bcopy((char *) &scinfo, (char *) arg, sizeof(struct devinfo));
        } 

        break;

    case SCIOSTART:   /* start a device */
        ret_code = vsc_init_dev(scsi, INDEX(arg >> 8, arg));
        break;

    case SCIOSTOP:    /* stop a device */
        ret_code = vsc_clear_dev(scsi, INDEX(arg >> 8, arg));
        break;

    case SCIOHALT:    /* issue SCSI abort message to a device */
        ret_code = vsc_halt_dev(scsi, INDEX(arg >> 8, arg));
        break;

    case SCIOINQU:    /* issue a SCSI device inquiry cmd */
        ret_code = vsc_inquiry(scsi, arg, devno, devflag);
        break;

    case SCIORESET:   /* issue a SCSI bus device reset message to a device */
        ret_code = vsc_issue_bdr(scsi, INDEX(arg >> 8, arg));
        break;

    case SCIODIAG:    /* not supported by this driver */
        ret_code = ENXIO;
        break;

    case SCIOTRAM:    /* not supported by this driver */
        ret_code = ENXIO;
        break;

    case SCIODNLD:    /*  microcode download- not supported by this driver */
        ret_code = ENXIO;
        break;

    case SCIOSTUNIT:  /* issue a SCSI start unit command */
        ret_code = vsc_start_unit(scsi, arg, devno, devflag);
        break;

    case SCIOTUR:     /* issue a SCSI test unit ready command */
        ret_code = vsc_test_unit_rdy(scsi, arg, devno, devflag);
        break;

    case SCIOREAD:    /* issue a SCSI read command (6-byte) */
        ret_code = vsc_read_blk(scsi, arg, devno, devflag);
        break;

    case SCIOEVENT:   /* register a devices asynchronous event */
                      /* notification function */
        ret_code = vsc_register_async(scsi, arg, devflag);
        break;

    case SCIOSTARTTGT:/* allocate resources for target mode device */
        ret_code = vsc_alloc_tgt(scsi, arg, devflag);
        break;

    case SCIOSTOPTGT: /* deallocate resources for target device */
        ret_code = vsc_dealloc_tgt(scsi, arg, devflag);
        break;

    case SCIOGTHW:    /* returns 0 if gathered write is supported */
        ret_code = 0; 
        break;
      /* private undocumented ioctl to allow users to call future ioctls */
      /* directly, bypassing the need for the ioctl kernel services      */
      case 901:

        if (!(devflag & DKERNEL)) {     /* for a user process */
            rc = copyout(&addr, (char *) arg, sizeof(addr));
            if (rc != 0)
                ret_code = EFAULT;
        }
        else {  /* for a kernel process */
            /* s, d, l */
            bcopy(&addr, (char *) arg, sizeof(addr));
        }
        if (!(vsc_info.top_pinned)) {
            rc = pincode(vsc_open); /* pin the top part of the driver */
            if (rc != 0) {
                ret_code = EIO;
            }
            else {
                vsc_info.top_pinned = TRUE;
            }
        }
       
        break;


    default:          /* catch unknown ioctls here */
        ret_code = EINVAL;
        break;

    }   /* end switch */

    unlockl(&(scsi->scsi_lock)); /* release the scsi_info struct lock */

    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);

    return(ret_code);

}  /* end vsc_ioctl */


/*
 * NAME:	vsc_init_dev 
 *
 * FUNCTION:   
 *              Allocates resources for a device.  This function performs 
 *              actions required to ready a device information structure 
 *              for use.
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOSTART)
 *
 * EXTERNAL CALLS:
 *              xmalloc                  w_init
 *              bzero
 *		
 * INPUT:
 *		scsi	  - scsi_info structure ptr for this device
 *		dev_index - index to device info structure 
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *		ENOMEM	- xmalloc failed to allocated memory
 *		EINVAL	- invalid argument 
 *		EIO	- ndd ioctl failure 
 */
int
vsc_init_dev(
	struct	scsi_info *scsi,
        int dev_index)
{
    int  rc,                           /* locally used return code          */
         ret_code,                     /* return code for this function     */
         arg;                          /* argument for adapter ndd ioctl    */

    ret_code = 0;       /* set default return code */
 
   /*
    * validate input paramters :
    * check the passed scsi id is different from the card
    * check the device is not already started (opened)
    * check wide enabled and SCSI id do not conflict
    */
    if ((SID(dev_index) == scsi->ddi.bus_scsi_id) || 
        (scsi->dev[dev_index] != NULL) || 
        ((SID(dev_index) > 7) && (!scsi->ddi.wide_enabled)))  {
        ret_code = EINVAL; 
        return(ret_code);
    }

  /*
   * build the argument to the adapter device registration ioctl
   * it consists of a SCSI id/lun combination as created by the INDEX
   * macro, and the location of the bus on the adapter is stored in the
   * two most signifigant bits of the word : bits 0 - 4 are lun
   * bits 5-8 are id and bits 14-15 are bus
   */
   arg = dev_index | (scsi->ddi.location  << 14);
 
   /* issue the adapter driver device registration ioctl */
   rc = (scsi->shared->ndd->ndd_ctl) (scsi->shared->ndd, NDD_ADD_DEV, arg);
   if (rc != 0) {     /* device registration failed */
       ret_code = EIO;
       return(ret_code);
   }
   
    /* allocate memory for the dev_info structure for this device */
    scsi->dev[dev_index] = (struct dev_info *) xmalloc((int) sizeof
                            (struct dev_info), 4, pinned_heap);

    if(scsi->dev[dev_index] ==  NULL) {  /* xmalloc failed */
        ret_code = ENOMEM; 
        return(ret_code);
    }

    /* zero out dev_info struct */
    bzero((char *)scsi->dev[dev_index], sizeof(struct dev_info));

    /* initialize pointer  fields in the dev_info struct ; all other fields */
    /* not referenced below have initial state values of zero               */
    scsi->dev[dev_index]->head_act = NULL;
    scsi->dev[dev_index]->tail_act = NULL;
    scsi->dev[dev_index]->head_pend = NULL;
    scsi->dev[dev_index]->tail_pend = NULL;
    scsi->dev[dev_index]->dev_event = EVENT_NULL;
    scsi->dev[dev_index]->async_func = NULL;

    /* initialize the watchdog timer for this device */
    scsi->dev[dev_index]->wdog.timer_id = SCSI_DEV_TMR;
    scsi->dev[dev_index]->wdog.dog.func = vsc_watchdog;
    scsi->dev[dev_index]->wdog.dog.next = NULL;
    scsi->dev[dev_index]->wdog.dog.prev = NULL;
    scsi->dev[dev_index]->wdog.scsi = scsi;
    scsi->dev[dev_index]->wdog.index = dev_index;
    /* the count, save_time and restart field values of the wdog.dog struct */
    /* are set to zero by the bzero call above */
#ifdef _POWER_MP
    while(w_init(&scsi->dev[dev_index]->wdog.dog));
#else
    w_init(&scsi->dev[dev_index]->wdog.dog);
#endif

    /* initialize the command element in the dev_info struct; note : the   */
    /* dev_info struct contains one command element to be used for cancel  */
    /* requests (abort, BDR, resume, reset) so that when it is required to */
    /* send one of these cancels, the device can be assured of getting a   */
    /* command element (the code does not have to consider the case for    */
    /* which there is no command element availible to send the cancel)     */

    vsc_init_cancel_cmd_elem(scsi, &scsi->dev[dev_index]->command_element);
    /* see if qstate must reflect a SCSI bus reset in progress */
    if (scsi->reset_cmd_elem.cmd_state != INACTIVE) {
        scsi->dev[dev_index]->qstate |= RESET_IN_PROG;
    }

    return(ret_code);

}  /* end vsc_init_dev */


/*
 *
 * NAME:	vsc_inquiry 
 *
 * FUNCTION: 
 *		This function issues a SCSI inquiry command to the specified
 *              SCSI id/lun.
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES :      This routine is called to issue an inquiry to a device.
 *              The calling process is responsible for NOT calling this rou- 
 *              tine if the SCIOSTART failed.  Such a failure would indicate
 *              that another process has this device open and interference
 *              could cause improper error reporting. 
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOINQ)
 *
 * EXTERNAL CALLS:
 *              copyin                bcopy
 *              xmfree                
 *              copyout
 *		
 * INPUT:
 *		scsi    - scsi_info structure ptr for this device
 *		devno	- major/minor number
 *		arg     - pointer to sc_inquiry structure
 *		devflag	- kernel or user process caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EFAULT    - copyin or copyout failed
 *              EINVAL    - Device not opened 
 *              ENOMEM    - Could not allocate an sc_buf 
 *              ENODEV    - Device could not be selected.
 *              ETIMEDOUT - The inquiry command timed out.
 *              ENOCONNECT- SCSI bus fault
 *              EIO       - Error returned from vsc_strategy or I/O error 
 *                          occurred
 *	
 */
int
vsc_inquiry(
	struct  scsi_info *scsi,
	int     arg,
	dev_t   devno,
	int     devflag )
{
    int  ret_code,                      /* return code for this function    */
         rc,                            /* locally used return code         */
         dev_index,                     /* index into dev_info structs      */
         inquiry_length;                /* number of bytes of inquiry data  */
    uchar lun;                          /* temp used to store SCSI lun      */
    struct vsc_buf *ptr;                /* pointer to vsc_buf containing    */
                                        /* inquiry command                  */
    struct sc_inquiry sci;              /* structure to contain passed in   */
                                        /* argurement to SCIOINQ ioctl      */
 

    ret_code = 0;  /* set default return code */

    if (!(devflag & DKERNEL)) { /* handle user process */
        rc = copyin((char *) arg, (char *) &sci, sizeof(struct sc_inquiry));
        if (rc != 0) {
            ret_code = EFAULT;
            return(ret_code);
        }
    }
    else {      /* handle kernel process */
        bcopy((char *) arg, (char *) &sci, sizeof(struct sc_inquiry));
    }

    dev_index = INDEX(sci.scsi_id, sci.lun_id);

    if (sci.lun_id <= 7)
        lun =  sci.lun_id;
    else 
        lun = 0;

    /* validate device is started (opened) */
    if (scsi->dev[dev_index] == NULL) {
        ret_code = EINVAL; /* device not started */
        return(ret_code);
    }
    ptr = vsc_bld_vsc_buf();
    if (ptr == NULL) {
        ret_code = ENOMEM;
        return(ret_code);
    }

    ptr->scsi = scsi;
    ptr->scbuf.scsi_command.scsi_id = sci.scsi_id;
    ptr->scbuf.scsi_command.scsi_length = 6;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_op_code = SCSI_INQUIRY;
    if (sci.get_extended) {
        ptr->scbuf.scsi_command.scsi_cmd.lun = (lun << 5) | 0x01;
        ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[0] = sci.code_page_num;
    }
    else {
        ptr->scbuf.scsi_command.scsi_cmd.lun = lun << 5;
        ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    }

    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[2] = 255;     /* max count */
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* insure that the command works by always setting no-disc flag */
    /* set ASYNC flag according to flag in input struct */
    ptr->scbuf.scsi_command.flags = (sci.flags & (SC_ASYNC | SC_NODISC));

    ptr->scbuf.bufstruct.b_bcount = 255;
    ptr->scbuf.bufstruct.b_flags |= B_READ;
    ptr->scbuf.bufstruct.b_dev = devno;

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the inquiry is only running single-threaded  */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->scbuf.flags = SC_RESUME | SC_DELAY_CMD;
    ptr->scbuf.lun = sci.lun_id;

    rc = vsc_strategy((struct scbuf *)ptr);
    if (rc) {    /* error returned from strategy */
        (void) xmfree((void *) ptr, pinned_heap);
        ret_code = EIO;
        return(ret_code);
    }

    vsc_ioctl_sleep((struct buf *) ptr, scsi); /* wait for completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (ptr->scbuf.bufstruct.b_flags & B_ERROR) {  /* an error occurred */
        if (ptr->scbuf.status_validity & SC_ADAPTER_ERROR) { 
           /* if adapter error */
            switch (ptr->scbuf.general_card_status) {
            case SC_CMD_TIMEOUT:
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 1, NULL, COMMAND_TIMEOUT);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            } /* end switch */
        }
        else {
            ret_code = EIO;
        }
    } /* end if error occurred */

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) &&
        (ptr->scbuf.bufstruct.b_resid == ptr->scbuf.bufstruct.b_bcount)) {
        ret_code = EIO;
    }

    /* if no errors detected */
    if (ret_code == 0) {
        inquiry_length = ptr->scbuf.bufstruct.b_bcount - 
                         ptr->scbuf.bufstruct.b_resid;
        if (inquiry_length > sci.inquiry_len)
            inquiry_length = sci.inquiry_len;
        /* Copy out the inquiry data. If the buffer resides */
        /* user space, use copyin, else use bcopy.          */
        if (!(devflag & DKERNEL)) {
            rc = copyout((char *) ptr->scbuf.bufstruct.b_un.b_addr,
			 (char *) sci.inquiry_ptr, inquiry_length);
            if (rc) {
                ret_code = EFAULT;
            }
        }
        else { /* buffer is in kernel space */
            bcopy((char *) ptr->scbuf.bufstruct.b_un.b_addr, 
                  (char *) sci.inquiry_ptr, inquiry_length);
        }
    }
    /* free the memory for the sc_buf */
    (void) xmfree((void *) ptr, pinned_heap);

    return(ret_code);

}  /* end vsc_inquiry */

/*
 *
 * NAME:	vsc_start_unit 
 *
 * FUNCTION: 
 *		This function issues a SCSI start unit command to the specified
 *              device (SCSI id/lun).
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:       The calling process is responsible for NOT calling this rou- 
 *              tine if the SCIOSTART failed.  Such a failure would indicate
 *              that another process has this device open and interference
 *              could cause improper error reporting. 
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOSTUNIT)
 *
 * EXTERNAL CALLS:
 *              copyin                bcopy
 *              xmfree                
 *		
 * INPUT:
 *		scsi    - scsi_info structure ptr for this device
 *		devno	- major/minor number
 *		arg     - pointer to sc_startunit structure
 *		devflag	- kernel or user process caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EFAULT    - copyin  failed
 *              EINVAL    - Device not opened 
 *              ENOMEM    - Could not allocate an sc_buf 
 *              ENODEV    - Device could not be selected.
 *              ETIMEDOUT - The start unit command timed out.
 *              ENOCONNECT- SCSI bus fault
 *              EIO       - Error returned from vsc_strategy or I/O error 
 *                          occurred
 *	
 */
int
vsc_start_unit(
	struct  scsi_info *scsi,
	int     arg,
	dev_t   devno,
	int     devflag )
{
    int  ret_code,                      /* return code for this function    */
         rc,                            /* locally used return code         */
         dev_index;                     /* index into dev_info structs      */
    uchar lun;                          /* temp used to store SCSI lun      */
    struct vsc_buf *ptr;                 /* pointer to vsc_buf containing   */
                                        /* start unit command               */
    struct sc_startunit sci;            /* structure to contain passed in   */
                                        /* argurement to SCIOSTUNIT ioctl   */


    ret_code = 0;  /* set default return code */

    if (!(devflag & DKERNEL)) { /* handle user process */
        rc = copyin((char *) arg, (char *) &sci, sizeof(struct sc_startunit));
        if (rc != 0) {
            ret_code = EFAULT;
            return(ret_code);
        }
    }
    else {      /* handle kernel process */
        bcopy((char *) arg, (char *) &sci, sizeof(struct sc_startunit));
    }

    dev_index = INDEX(sci.scsi_id, sci.lun_id);

    if (sci.lun_id <= 7)
        lun =  sci.lun_id;
    else 
        lun = 0;

    /* validate device is started (opened) */
    if (scsi->dev[dev_index] == NULL) {
        ret_code = EINVAL; /* device not started */
        return(ret_code);
    }
    ptr = vsc_bld_vsc_buf();
    if (ptr == NULL) {
        ret_code = ENOMEM;
        return(ret_code);
    }

    ptr->scsi = scsi;
    ptr->scbuf.scsi_command.scsi_id = sci.scsi_id;
    ptr->scbuf.scsi_command.scsi_length = 6;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;
    ptr->scbuf.scsi_command.scsi_cmd.lun =    /* set immed bit here */
        (lun << 5) | (sci.immed_flag ? 0x01 : 0);
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[2] =  /* set start option */
        (sci.start_flag ? 0x01 : 0);
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    ptr->scbuf.bufstruct.b_bcount = 0;
    ptr->scbuf.bufstruct.b_dev = devno;

    ptr->scbuf.timeout_value = sci.timeout_value;     /* set timeout value */

    /* do not set the no-disc flag for this command */
    /* set ASYNC flag according to flag in input struct */
    ptr->scbuf.scsi_command.flags = (sci.flags & SC_ASYNC);

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the command is only running single-threaded  */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->scbuf.flags = SC_RESUME | SC_DELAY_CMD;

    ptr->scbuf.lun = sci.lun_id;

    rc = vsc_strategy((struct sc_buf *)ptr);
    if (rc) {    /* error returned from strategy */
        (void) xmfree((void *) ptr, pinned_heap);
        ret_code = EIO;
        return(ret_code);
    }

    vsc_ioctl_sleep((struct buf *) ptr, scsi); /* wait for completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (ptr->scbuf.bufstruct.b_flags & B_ERROR) {  /* an error occurred */
        if (ptr->scbuf.status_validity & SC_ADAPTER_ERROR) {  
           /* if adapter error */
            switch (ptr->scbuf.general_card_status) {
            case SC_CMD_TIMEOUT:
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 2, NULL, COMMAND_TIMEOUT);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            } /* end switch */
        }
        else {
            ret_code = EIO;
        }
    } /* end if error occurred */

    /* free the memory for the sc_buf */
    (void) xmfree((void *) ptr, pinned_heap);

    return(ret_code);

}  /* end vsc_start_unit */

/*
 *
 * NAME:	vsc_test_unit_rdy
 *
 * FUNCTION: 
 *		This function issues a SCSI test unit ready command to a
 *              specified device (SCSI id/lun)
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:       The calling process is responsible for NOT calling this rou- 
 *              tine if the SCIOSTART failed.  Such a failure would indicate
 *              that another process has this device open and interference
 *              could cause improper error reporting.
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOTUR)
 *
 * EXTERNAL CALLS:
 *              copyin                bcopy
 *              xmfree                
 *		
 * INPUT:
 *		scsi    - scsi_info structure ptr for this device
 *		devno	- major/minor number
 *		arg     - pointer to sc_ready structure
 *		devflag	- kernel or user process caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EFAULT    - copyin  failed
 *              EINVAL    - Device not opened 
 *              ENOMEM    - Could not allocate an sc_buf 
 *              ENODEV    - Device could not be selected.
 *              ETIMEDOUT - The test unit ready command timed out.
 *              ENOCONNECT- SCSI bus fault
 *              EIO       - Error returned from vsc_strategy or I/O error 
 *                          occurred
 *	
 */
int
vsc_test_unit_rdy(
	struct  scsi_info *scsi,
	int     arg,
	dev_t   devno,
	int     devflag )

{
    int  ret_code,                      /* return code for this function    */
         rc,                            /* locally used return code         */
         dev_index;                     /* index into dev_info structs      */
    uchar lun;                          /* temp used to store SCSI lun      */
    struct vsc_buf *ptr;                /* pointer to vsc_buf containing    */
                                        /* test unit ready  command         */
    struct sc_ready sci;                /* structure to contain passed in   */
                                        /* argurement to SCIOTUR ioctl      */


    ret_code = 0;  /* set default return code */

    if (!(devflag & DKERNEL)) { /* handle user process */
        rc = copyin((char *) arg, (char *) &sci, sizeof(struct sc_ready));
        if (rc != 0) {
            ret_code = EFAULT;
            return(ret_code);
        }
    }
    else {      /* handle kernel process */
        bcopy((char *) arg, (char *) &sci, sizeof(struct sc_ready));
    }

    dev_index = INDEX(sci.scsi_id, sci.lun_id);

    if (sci.lun_id <= 7)
        lun =  sci.lun_id;
    else 
        lun = 0;

    /* validate device is started (opened) */
    /* validate device is started (opened) */
    if (scsi->dev[dev_index] == NULL) {
        ret_code = EINVAL; /* device not started */
        return(ret_code);
    }
    ptr = vsc_bld_vsc_buf();
    if (ptr == NULL) {
        ret_code = ENOMEM;
        return(ret_code);
    }

    sci.status_validity = 0;    /* set default status */
    sci.scsi_status = 0;        /* set default status */

    ptr->scsi = scsi;
    ptr->scbuf.scsi_command.scsi_id = sci.scsi_id;
    ptr->scbuf.scsi_command.scsi_length = 6;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_op_code = SCSI_TEST_UNIT_READY;
    ptr->scbuf.scsi_command.scsi_cmd.lun = lun << 5;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[2] = 0;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    ptr->scbuf.bufstruct.b_bcount = 0;
    ptr->scbuf.bufstruct.b_dev = devno;

    ptr->scbuf.timeout_value = 15;    /* set timeout value */

    /* do not set the no-disc flag for this command */
    /* set ASYNC flag according to flag in input struct */
    ptr->scbuf.scsi_command.flags = (sci.flags & SC_ASYNC);

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the command is only running single-threaded  */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->scbuf.flags = SC_RESUME | SC_DELAY_CMD;

    ptr->scbuf.lun = sci.lun_id;

    /* Initialize default status to zero. */
    sci.status_validity = 0;
    sci.scsi_status = 0;


    rc = vsc_strategy((struct sc_buf *)ptr);
    if (rc) {    /* error returned from strategy */
        (void) xmfree((void *) ptr, pinned_heap);
        ret_code = EIO;
        return(ret_code);
    }

    vsc_ioctl_sleep((struct buf *) ptr, scsi); /* wait for completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (ptr->scbuf.bufstruct.b_flags & B_ERROR) {  /* an error occurred */
        if (ptr->scbuf.status_validity & SC_ADAPTER_ERROR) {  
           /* if adapter error */
            switch (ptr->scbuf.general_card_status) {
            case SC_CMD_TIMEOUT:
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 3, NULL, COMMAND_TIMEOUT);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            } /* end switch */
        }
        else {
            if (ptr->scbuf.status_validity & SC_SCSI_ERROR) {   
                /* if a scsi status error */
                sci.status_validity = SC_SCSI_ERROR;
                sci.scsi_status = ptr->scbuf.scsi_status;
                ret_code = EIO;
            }
            else {   /* if general error (fall through case) */
                ret_code = EIO;
            }

        }
    } /* end if error occurred */

    /* Copy out the device status to the st_ready structure      */
    /* passed in by the calling application.                     */
    if (!(devflag & DKERNEL)) {
        rc = copyout((char *) &sci, (char *) arg, sizeof(struct sc_ready));
        if (rc) {
            ret_code = EFAULT;
        }
    }
    else {   /* buffer is in kernel space */
        bcopy((char *) &sci, (char *) arg, sizeof(struct sc_ready));
    }


    /* free the memory for the sc_buf */
    (void) xmfree((void *) ptr, pinned_heap);

    return(ret_code);

}  /* end vsc_test_unit_rdy */

/*
 *
 * NAME:	vsc_read_blk 
 *
 * FUNCTION: 
 *		This function issues a SCSI read command (6-byte) to a
 *              specified device (SCSI id/lun)
 *
 * EXECUTION ENVIRONMENT:
 *		process level only
 *
 * NOTES:       The calling process is responsible for NOT calling this rou- 
 *              tine if the SCIOSTART failed.  Such a failure would indicate
 *              that another process has this device open and interference
 *              could cause improper error reporting.
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOREAD)
 *
 * EXTERNAL CALLS:
 *              copyin                bcopy
 *              xmfree                
 *              copyout               xmalloc
 *		
 * INPUT:
 *		scsi    - scsi_info structure ptr for this device
 *		devno	- major/minor number
 *		arg     - pointer to sc_readblk structure
 *		devflag	- kernel or user process caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EFAULT    - copyin or copyout failed
 *              EINVAL    - Device not opened or transfer size is > 1 page
 *              ENOMEM    - Could not allocate an sc_buf or memory to hold read 
 *                          data for this command.
 *              ENODEV    - Device could not be selected.
 *              ETIMEDOUT - The read command timed out.
 *              ENOCONNECT- SCSI bus fault
 *              EIO       - Error returned from vsc_strategy or no data 
 *                          returned from read command.
 *
 *
 */
int
vsc_read_blk(
	struct  scsi_info *scsi,
	int     arg,
	dev_t   devno,
	int     devflag )
{
    int  ret_code,                      /* return code for this function    */
         rc,                            /* locally used return code         */
         dev_index;                     /* index into dev_info structs      */
    uchar lun;                          /* temp used to store SCSI lun      */
    struct vsc_buf *ptr;                /* pointer to vsc_buf containing    */
                                        /* read command                     */
    struct sc_readblk sci;              /* structure to contain passed in   */
                                        /* argurement to SCIOREAD ioctl     */


    ret_code = 0;  /* set default return code */

    if (!(devflag & DKERNEL)) { /* handle user process */
        rc = copyin((char *) arg, (char *) &sci, sizeof(struct sc_readblk));
        if (rc != 0) {
            ret_code = EFAULT;
            return(ret_code);
        }
    }
    else {      /* handle kernel process */
        bcopy((char *) arg, (char *) &sci, sizeof(struct sc_readblk));
    }

    dev_index = INDEX(sci.scsi_id, sci.lun_id);

    if (sci.lun_id <= 7)
        lun =  sci.lun_id;
    else 
        lun = 0;

    /* validate device is started (opened) */
    /* validate device is started (opened) */
    if (scsi->dev[dev_index] == NULL) {
        ret_code = EINVAL; /* device not started */
        return(ret_code);
    }

    /* validate that transfer size will fit within a page   */
    if ((int) sci.blklen > PAGESIZE) {
        ret_code = EINVAL;      /* xfer too long */
        return(ret_code);
    }

    ptr = vsc_bld_vsc_buf();
    if (ptr == NULL) {
        ret_code = ENOMEM;
        return(ret_code);
    }

    /* malloc area to be used for data transfer  */
    ptr->scbuf.bufstruct.b_un.b_addr =
        (char *) xmalloc((uint) PAGESIZE, (uint) PGSHIFT,
                         pinned_heap);
    if (ptr->scbuf.bufstruct.b_un.b_addr == NULL) {
        /* xmalloc failed--return error */
        (void) xmfree((void *) ptr, pinned_heap);
        ret_code = ENOMEM;
        return(ret_code);
    }

    ptr->scsi = scsi;
    ptr->scbuf.scsi_command.scsi_id = sci.scsi_id;
    ptr->scbuf.scsi_command.scsi_length = 6;
    ptr->scbuf.scsi_command.scsi_cmd.scsi_op_code = SCSI_READ;
    ptr->scbuf.scsi_command.scsi_cmd.lun =
        (lun << 5) | ((sci.blkno >> 16) & 0x1f);
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[0] =
        ((sci.blkno >> 8) & 0xff);
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[1] =
        (sci.blkno & 0xff);
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[2] = 1;       /* single blk */
    ptr->scbuf.scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* set ASYNC and NODISC flag according to flag in input struct */
    ptr->scbuf.scsi_command.flags = sci.flags;

    ptr->scbuf.bufstruct.b_bcount = (unsigned) sci.blklen;
    ptr->scbuf.bufstruct.b_flags |= B_READ;
    ptr->scbuf.bufstruct.b_dev = devno;

    ptr->scbuf.timeout_value = sci.timeout_value;     /* set timeout value */

    /* set resume flag in case caller is retrying this operation */
    /* this assumes the read is only running single-threaded     */
    /* to this device. set delay flag in case this is a device   */
    /* which requires delay after reset occurs.                  */
    ptr->scbuf.flags = SC_RESUME | SC_DELAY_CMD;

    ptr->scbuf.lun = sci.lun_id;

    rc = vsc_strategy((struct sc_buf *)ptr);
    if (rc) {    /* error returned from strategy */
        (void) xmfree((void *) ptr->scbuf.bufstruct.b_un.b_addr, pinned_heap);
        (void) xmfree((void *) ptr, pinned_heap);
        ret_code = EIO;
        return(ret_code);
    }

    vsc_ioctl_sleep((struct buf *) ptr, scsi); /* wait for completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (ptr->scbuf.bufstruct.b_flags & B_ERROR) {  /* an error occurred */
        if (ptr->scbuf.status_validity & SC_ADAPTER_ERROR) {  
           /* if adapter error */
            switch (ptr->scbuf.general_card_status) {
            case SC_CMD_TIMEOUT:
                vsc_log_err(scsi, ERRID_SCSI_ERR10, 4, NULL, COMMAND_TIMEOUT);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            } /* end switch */
        }
        else {
            ret_code = EIO;
        }
    } /* end if error occurred */

    /* if no other errors, and yet there is a resid count--fail */
    if ((ret_code == 0) && (ptr->scbuf.bufstruct.b_resid))
        ret_code = EIO;

    /* if no errors detected, return the caller's data. */
    if (ret_code == 0) {
        if (!(devflag & DKERNEL)) {     /* handle user process */
            rc = copyout((char *) ptr->scbuf.bufstruct.b_un.b_addr,
                         (char *) sci.data_ptr,
                         sci.blklen);
            if (rc != 0)
                ret_code = EFAULT;
        }
        else {  /* handle kernel process */
            bcopy((char *) ptr->scbuf.bufstruct.b_un.b_addr, 
                  (char *) sci.data_ptr, sci.blklen);
        }
    }

    (void) xmfree((void *) ptr->scbuf.bufstruct.b_un.b_addr, pinned_heap);
    (void) xmfree((void *) ptr, pinned_heap);

    return(ret_code);

}  /* end vsc_read_blk */

/*
 *
 * NAME:	vsc_register_async
 *
 * FUNCTION: 
 *		This function registers a device's asynchronous event 
 *              notification function.
 *
 * EXECUTION ENVIRONMENT:
 *		this routine can only be called from the 
 *		process level. only KERNEL processes can register
 *		for this function.
 * 
 * NOTES:   
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOEVENT)
 *
 * EXTERNAL CALLS:
 *              bcopy
 *		
 * INPUT:
 *		scsi    - scsi_info structure ptr for this device
 *		arg     - pointer to sc_event_struct structure
 *		devflag	- kernel or user process caller
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EPERM  - caller not a kernel process
 *              EINVAL - device not opened or already registered
 */
int
vsc_register_async(
	struct  scsi_info *scsi,
	int     arg,
	ulong   devflag )
{
    int  ret_code,                  /* return code for this function         */
        dev_index;                  /* index into array of dev_info or       */
                                    /* tm_info structs depending on sce.mode */
    struct dev_info *dev;           /* pointer to dev_info struct for im dev */
    struct tm_info  *tm;            /* pointer to tm_info struct for tm dev  */
    struct sc_event_struct sce;     /* structure to contain the passed in arg*/
                                    

    ret_code = 0;  /* set default return code */

    if (!(devflag & DKERNEL)) { /* if user process */
        ret_code = EPERM;
        return (ret_code);
    }

    /* handle kernel process     */
    bcopy((char *) arg, (char *) &sce, sizeof(struct sc_event_struct));
  
    if (sce.mode == SC_TM_MODE) { /* target mode device */
        dev_index = sce.id;
        tm = scsi->tm[dev_index];
        if ((tm == NULL) || (tm->async_func != NULL)) { 
        /* device not opened or device already registered */
            ret_code = EINVAL;
            return(ret_code);
        }
        tm->async_correlator = sce.async_correlator;
        tm->async_func = sce.async_func;
    } 
    else {   /* initiator mode device */
        dev_index = INDEX(sce.id, sce.lun);
        dev = scsi->dev[dev_index];
        if ((dev == NULL) || (dev->async_func != NULL)) { 
        /* device not opened or device already registered */
            ret_code = EINVAL;
            return(ret_code);
        }
        dev->async_correlator = sce.async_correlator;
        dev->async_func = sce.async_func;
    }

    return(ret_code);

}  /* end vsc_register_async */

/*
 *
 * NAME:        vsc_bld_vsc_buf
 *
 * FUNCTION:
 *              This function builds a vsc_buf structure for use when sending
 *              internal SCSI commands (through ioctls).
 *
 * EXECUTION ENVIRONMENT:
 *              process level only
 *
 * NOTES :     
 *
 * EXTERNAL CALLS:
 *              xmalloc                  bzero
 *     
 * INPUT:
 *              NONE 
 *
 * RETURNS:
 *              pointer to an sc_buf for good completion,  
 *              NULL if sc_buf could not be allocated
 */
struct vsc_buf *
vsc_bld_vsc_buf()

{
    struct vsc_buf *vscb;            /* vsc_buf pointer returned by function */

    vscb = (struct vsc_buf *) xmalloc((uint) PAGESIZE, (uint) PGSHIFT,
                                    pinned_heap);
    if (vscb == NULL) {
        /* xmalloc failed--return NULL pointer */
        return (vscb);
    }

    /* assure sc_buf is initialized */
    bzero((char *)vscb, sizeof(struct vsc_buf));

    /* dummy bufstruct initialization */
    vscb->scbuf.bufstruct.b_forw = NULL;
    vscb->scbuf.bufstruct.b_back = NULL;
    vscb->scbuf.bufstruct.av_forw = NULL;
    vscb->scbuf.bufstruct.av_back = NULL;
    /* point at us */
    vscb->scbuf.bufstruct.b_iodone = (void (*) ()) vsc_iodone;
    vscb->scbuf.bufstruct.b_vp = NULL;
    vscb->scbuf.bufstruct.b_work = 0;
    vscb->scbuf.bufstruct.b_options = 0;
    vscb->scbuf.bufstruct.b_event = EVENT_NULL;
    vscb->scbuf.bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
#ifdef _POWER_MP
    vscb->scbuf.bufstruct.b_flags |= B_MPSAFE;
#endif
    vscb->scbuf.bufstruct.b_un.b_addr = (char *) vscb + sizeof(struct vsc_buf);

    /* additional sc_buf initialization */
    vscb->scbuf.bp = NULL;     /* set for non-spanned cmd */

    vscb->scbuf.timeout_value = 30;    /* set default timeout value */

    return (vscb);

} /* end vsc_bld_vsc_buf */


/* NOTES ON TARGET MODE IMPLEMENTATION 
 * 
 * Buffer Pool Allocation :
 *
 * There will be a single pool of buffers, the number of which is an attribute
 * of the virtual SCSI bus, which will be used to receive data from all   
 * possible intitiators on the bus.  This buffer pool is owned by the adapter
 * driver and the protocol driver makes a request to establish it with the 
 * ns_add_filter call done at the first open to the adapter driver. After 
 * the ns_add_filter call is made, the buffers are malloced, mapped, and 
 * enabled by the adapter driver.  SCIOSTARTTGT calls to the protocol driver
 * is what the target mode head uses to register to receive data from a 
 * particular initiator on the bus.  Each instance of a target mode device
 * has a number of buffers attribute associated with it that it will give 
 * to the protocol driver as part of the SCIOSTARTTGT call.  In this 
 * implementation, this number of buffers will represent the maximum number
 * of buffers that the protocol driver will use from the already registered
 * buffer pool for this particular initiator to target device instance.
 * If there are multiple initiator to target instances on this virtual SCSI
 * bus, then each instance that is to become active must register with the
 * SCIOSTARTTGT ioctl and request a number of buffers to be used from the
 * global buffer pool for its particular intitiator to target instance.
 * 
 * It is the responsibility of the user to ensure that the number of 
 * buffers attribute for the protocol driver corresponds well to the 
 * number of buffers each target mode device instance will request.
 * For instance, if the number of buffers configured for the protocol layer
 * (virtual SCSI bus) is equal to 50, and there is only one target mode 
 * device instance for this virtual bus and it is configured to have 16
 * buffers, then the protocol driver will request 50 buffers to the adapter
 * driver but only 16 of these will be used; 50 will be malloced, mapped, and
 * enabled by the adapter, but the target device will request that a maximum
 * of 16 be used when the target mode device issues the SCIOSTARTTGT ioctl.
 *
 * Conversely, it is possible that the number of buffers attribute for the 
 * protocol driver is equal to 20, and there could be two target mode device
 * instances on this virtual SCSI bus, each configured to have 24 buffers. 
 * In this case, the total all device instances will request exceeds the number
 * of buffers available to the protocol driver, so each target device instance
 * will not be getting the maximum which they requested.
 *
 *
 * Registering for unsolicited data
 *
 * Whenever the first open to a virtual SCSI bus (protocol layer) for a 
 * particular physical SCSI adapter occurs, the protocol layer will issue the 
 * ns_alloc call to register for use of a network device driver(adapter driver).
 * Directly after this, the protocol driver will issue an ns_add_filter call
 * to register with the adapter driver to receive unsolicited data.  The 
 * parameters to the ns_add_filter call will include the size of buffers the
 * adapter should use, the number of buffers, and an address to which the
 * buffers should be sent.  The adapter driver will malloc the space for the
 * buffers, map the buffers, and enable them at the adapter.  The adapter
 * driver will return an address for the protocol driver to call when it 
 * returns a buffer back to the adapter driver.  The number of buffers the
 * protocol driver registers for will be represented in the ddi for the 
 * particular virtual SCSI device instance.  This will be a customized 
 * configurable attribute.  Once the ns_add_filter call completes, the
 * buffers will be enabled at the adatper but will not be availible for
 * use because no initiators on the bus have been enabled.
 *
 * A target mode device special file is created for each initiator to target 
 * instance on the SCSI bus during SCSI bus configuration.  Whenever an open
 * is done to the target special file, this will cause the target mode device
 * driver to issue an SCIOSTARTTGT ioctl to the SCSI protocol driver.  The
 * target mode device driver will request a number of buffers in this call and
 * the two drivers will exchange function addresses to receive and return  
 * buffers with.  The protocol driver will enable the initiator corresponding
 * to this initiator to target device and will begin to send buffers with data
 * from this initiator up to the target mode device driver as the data comes
 * in from the adapter driver.  The number of buffers the target mode device
 * driver requested in the SCIOSTARTTGT ioctl will represent the maximum 
 * number of buffers from the shared buffer pool which will ever be used for
 * this initiator to target instance.  Whenever the number of buffers queued
 * to the target mode head for this initiator to target device reaches that 
 * maximum, then the protocol driver will disable the initiator until at
 * least 25% of the buffers are returned by the head. 
 *
 * The target mode device driver and SCSI protocol device driver exchange
 * buffer by means of a tm_buf structure.  This structure contains a pointer
 * to the physical buffer containing the data and information about the
 * data such as if it is then end of a send command, if there was an error,
 * and the amount of valid data in the buffer.  The protocol driver will
 * maintain a pool of free tm_buf structures (known as a b_link to the 
 * protocol driver), one for each buffer it requested in the ns_add_filter
 * call.  Whenever a buffer is received by the protocol driver from the 
 * adapter driver, a free tm_buf struct is fetched from the free pool, filled
 * in with the proper values, and sent to the target mode device driver.  
 * When the target mode device driver returns the tm_buf to the protocol driver
 * the tm_buf is returned to the free pool.
 */

/*
 * NAME:	vsc_alloc_tgt 
 *
 * FUNCTION: 
 *              Allocates target mode buffers and enables the id that can
 *              can act as an initiator.
 *	
 *
 * EXECUTION ENVIRONMENT:
 *		This function is called from the process level.  Only KERNEL
 *              processes can call this function.
 *
 * NOTES:       The caller provides the following information in the 
 *              sc_strt_tgt structure passed in as a parameter to this 
 *              function.
 *              - the id of the initiator to receive data from 
 *              - the size of the buffers to use to receive data
 *              - the number of buffers used to receive data
 *              - a pointer to the function to be called when data is received
 *                into the buffers.
 *              This function returns to the caller, the address of the    
 *              function to be used by the caller to return buffers after data
 *              has be read from them. The number of buffers the caller 
 *              requests will indicate the maximum number of buffers from 
 *              the global buffer pool for all initiators that will be used
 *              for the specified initiator.
 *    
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOSTARTTGT)
 *
 * EXTERNAL CALLS:
 *		xmalloc                  bzero
 *              bcopy
 *              xmfree
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EACCESS - adapter already open in diagnostic mode
 *              EPERM   - caller not a kernel process
 *		ENOMEM	- error allocating resources
 *		EINVAL	- initiator to target path already open or a parameter
 *                        in the sc_strt_tgt structure is invalid
 *		EIO	- kernel service failure or invalid operation
 *
 */
int
vsc_alloc_tgt (struct scsi_info *scsi,    /* virtual scsi structure          */
	       int               arg,     /* sc_strt_tgt structure           */
	       ulong             devflag) /* indicates kernel or user caller */
{
    int             ret_code;	 /* return code for this function        */
    struct tm_info *user;        /* target mode initiator                */
    int             dev_index;   /* index to device list                 */
    struct sc_strt_tgt stgt;     /* start target structure               */
    ulong           punlunbus;   /* physical/logical unit number and bus */
    struct ns_user  scsi_nsuser; /* a struct passed to network services  */
                                 /* with the ns_add_filter call          */
    int             rc;          /* local return code                    */

    ret_code = 0;	/* set default return code */

    if (!(devflag & DKERNEL))
    {	/* if user process */
	ret_code = EPERM;
	return (ret_code);
    }

    bcopy ((char *) arg, (char *) &stgt, sizeof (struct sc_strt_tgt));

    dev_index = stgt.id;

    /*
     * Index into array of target devices to obtain structure for this
     * inititator and verify it is not already opened.
     */
    if (scsi->tm[dev_index] != NULL)
    {
	ret_code = EINVAL;
	return (ret_code);	/* already opened */
    }

    /* 
     * Make sure the passed scsi id is different from the card's.
     */
    if ((stgt.id == scsi->ddi.bus_scsi_id) || (stgt.lun != 0))
    {
	ret_code = EINVAL;
	return (ret_code);	/* bad SCSI ID */
    }

    /*
     * Check for validity of other parameters.
     * Eventually, check for: (stgt.buf_size & (TCWRANGE - 1))
     * right now, buf_size is fixed.
     */
    if ((stgt.recv_func == NULL) ||
	(stgt.buf_size != SC_TM_BUFSIZE) ||
	(stgt.num_bufs < SC_TM_MIN_NUMBUFS))
    {
	ret_code = EINVAL;
	return (ret_code);
    }

    /*
     * xmalloc memory for the tm_info structure.
     */
    scsi->tm[dev_index] = xmalloc (sizeof (struct tm_info), (uint) 2,
				   pinned_heap);

    if (scsi->tm[dev_index] == NULL)
    {	/* xmalloc failed */
	ret_code = ENOMEM;
	return (ret_code);
    }
    user = scsi->tm[dev_index];	/* tm_info struct ptr for this device   */

    /* use bzero to clear struct */
    bzero ((char *) user, sizeof (struct tm_info));

    user->buf_size = SC_TM_BUFSIZE;
    user->num_bufs = stgt.num_bufs;

    /* user->num_bufs_qued = 0; */
    /* user->previous_err = 0; */
    /* user->num_bufs_recvd = 0; *//* reset number of bufs received */
    /* user->num_bytes_recvd = 0; *//* reset number of bytes received */
    /* user->dev_abort             do not reset, as vsc_dealloc_tgt sets */
    user->async_func = NULL;	/* invalidate last registration */

    user->stopped = FALSE;	/* flag port enabled in sync with enable id */
/*  user->opened = TRUE;	   mark this device opened */

    /*
     * Copy out the arg structure to the caller with buf_free func addr. Note
     * that caller must have been kernel process, not user proc. 
     */
    stgt.free_func = (void (*) ()) vsc_buf_free;
    bcopy ((char *) &stgt, (char *) arg, sizeof (struct sc_strt_tgt));

    /*
     * The following sets the boundary for determining when to enable again.
     * 75% in use (25% freed) enables again 
     */
    user->num_to_resume = (user->num_bufs * 3) >> 2;
    user->tm_correlator = stgt.tm_correlator;
    user->recv_func = stgt.recv_func;

    /*
     * Check for any instance of target mode for this adapter. 
     */
    if (scsi->num_tm_devices == 0)
    {
	/* 
	 * Initialize the ns_user struct for the initiator ns_add_filter.
	 */
	scsi_nsuser.isr = (void (*) ()) vsc_target_receive;
	scsi_nsuser.isr_data = (caddr_t) scsi;
	scsi_nsuser.protoq = NULL;
	scsi_nsuser.netisr = 0;
	scsi_nsuser.pkt_format = NS_PROTO;
	scsi_nsuser.ifp = NULL;

#ifdef VSC_TRACE
	/* Trace this scsi ids add_filter call */
/*	vsc_internal_trace(scsi, dev_index, (uint *) &dev_index, 7, 1); */
#endif VSC_TRACE

	/*
	 * Allocate the buffers for the data transfers for this target mode
	 * instance.  This is done by ns_add_filter(). 
	 */
	rc = ns_add_filter (scsi->shared->ndd, &scsi->shared->tm_filter,
			    sizeof (struct scb_filter), &scsi_nsuser);
	if (rc != 0)
	{
	    ret_code = EIO;
	    (void) xmfree (scsi->tm[dev_index], pinned_heap);
	    scsi->tm[dev_index] = NULL;
	    return (ret_code);
	}
	/*
	 * Allocate the b_link for the data. 
	 */
	rc = vsc_alloc_b_link_pool (scsi);
        if (rc) {
	    ret_code = ENOMEM;
	    return (ret_code);
        }
    }
    /*
     * Tell the adapter that target mode is enabled by issuing an adapter
     * device driver ioctl. 
     */
    punlunbus = scsi->ddi.location << 14;
    punlunbus |= stgt.id << 5;
    punlunbus |= stgt.lun;

#ifdef VSC_TRACE
    /* Trace this scsi id */
    /*vsc_internal_trace(scsi, dev_index, (uint *) &dev_index, 5, 1);*/
#endif VSC_TRACE

    scsi->proc_sleep_id = stgt.id;
    rc = (scsi->shared->ndd->ndd_ctl) (scsi->shared->ndd,
				       NDD_TGT_ADD_DEV, punlunbus);
    if (rc == 0)
    {
	/*
	 * Wait for ndd_ctl() to finish. 
	 */
/*
WORK!
	e_sleep (&scsi->ioctl_event, EVENT_SHORT);
*/
    } else	/* device registration failed */
    {
	ret_code = EIO;
	if (scsi->num_tm_devices == 0)
	{
#ifdef VSC_TRACE
	    /* Trace this scsi ids del_filter call */
	    /*vsc_internal_trace(scsi, dev_index, (uint *) &dev_index, 8, 1);*/
#endif VSC_TRACE

	    (void) xmfree (scsi->tm[dev_index], pinned_heap);
	    scsi->tm[dev_index] = NULL;
	    (void) ns_del_filter (scsi->shared->ndd, &scsi->shared->tm_filter,
				  sizeof (struct scb_filter));
	}
	return (ret_code);
    }

    scsi->num_tm_devices++;

    return (ret_code);

}  /* end vsc_alloc_tgt */


/*
 * NAME:	vsc_dealloc_tgt 
 *
 * FUNCTION: 
 *              Function to stop target device.  It first disables the id of
 *              the initiator for this initiator to target path.  Buffers 
 *              allocated for this initiator to target path which are currently
 *              on the free list will be deallocated.  Buffers which are on
 *              the readbuf list (mapped or unmapped) will be freed.
 *	
 *
 * EXECUTION ENVIRONMENT:
 *		This function is called from the process level.  Only KERNEL
 *              processes can call this function.
 *
 * NOTES:
 *
 * CALLED FROM:
 *		vsc_ioctl(SCIOSTOPTGT)
 *
 * EXTERNAL CALLS:
 *              bcopy                  xmfree
 *
 * RETURNS:  
 *		0 for good completion,  ERRNO on error
 *              EACCESS - adapter already open in diagnostic mode
 *              EPERM   - caller not a kernel process
 *		EINVAL	- initiator to target path not open
 *
 */
int
vsc_dealloc_tgt (struct scsi_info *scsi,    /* virtual scsi structure */
		 int               arg,     /* sc_stop_tgt structure  */
		 ulong             devflag) /* kernel or user caller  */
{
    int             ret_code;	 /* return code for this function        */
    int             dev_index;   /* index to device list                 */
    struct sc_stop_tgt stop_tgt; /* stop target structure                */
    ulong           punlunbus;   /* physical/logical unit number and bus */
    int             rc;          /* local return code                    */

    ret_code = 0;	/* set default return code */

    if (!(devflag & DKERNEL))
    {	/* if user process */
	ret_code = EPERM;
	return (ret_code);
    }

    bcopy ((char *) arg, (char *) &stop_tgt, sizeof (struct sc_stop_tgt));

    /*
     * Index into array of target devices to obtain structure for this
     * inititator and verify it is not already opened.
     */
    dev_index = stop_tgt.id;
    if (scsi->tm[dev_index] == NULL)
    {
	ret_code = EINVAL;
	return (ret_code);
    }

    /*
     * First mark the device closed, so that its buffers won't
     * get reenabled while its being stopped.
     */
    scsi->num_tm_devices--;

    /*
     * Need to set flag to indicate device disabled while in temporary
     * disable state so that first buffer abort can be ignored (stopped is
     * TRUE if temp disabled) 
    user->dev_abort = user->stopped;
     */

    /*
     * Tell the adapter that this initiator id is disabled by issuing an
     * adapter device driver ioctl. 
     */
    punlunbus = scsi->ddi.location << 14;
    punlunbus |= stop_tgt.id << 5;
    punlunbus |= stop_tgt.lun;

#ifdef VSC_TRACE
    /* Trace this scsi id */
    /* vsc_internal_trace(scsi, dev_index, (uint *) &dev_index, 6, 1); */
#endif VSC_TRACE

    (void) (scsi->shared->ndd->ndd_ctl) (scsi->shared->ndd,
				       NDD_TGT_DEL_DEV, punlunbus);

    if (scsi->tm[dev_index]->previous_error) 
    {
        scsi->pending_err--;
    }
    /*
     * Disable and free buffers, vsc_free_rdbufs frees any
     * buffers queued for the device and not yet returned by the device.
     */
    vsc_free_rdbufs (scsi, dev_index);

    /* 
     * Free the memory for this tm_info structure.
     */
    (void) xmfree (scsi->tm[dev_index], pinned_heap);
    scsi->tm[dev_index] = NULL;

    if (scsi->num_tm_devices == 0)
    {
	ASSERT (scsi->head_free != NULL);
	ASSERT (scsi->b_pool != NULL);
	ASSERT (scsi->read_bufs == NULL);

	/*
	 * Free the b_link pool 
	 */
	(void) xmfree (scsi->b_pool, pinned_heap);
	scsi->b_pool = NULL;

	(void) ns_del_filter (scsi->shared->ndd, &scsi->shared->tm_filter,
			      sizeof (struct scb_filter));
    }
    return (ret_code);
}  /* end vsc_dealloc_tgt */



/*
 * NAME:        vsc_alloc_b_link_pool
 *
 * FUNCTION:
 *              Allocates a pool of b_link structures, one for each buffer in
 *              the buffer pool requeseted in the ns_add_filter call.  The 
 *              b_link structures are what are passed to a target mode head
 *              when the protocol driver receives target mode data from the
 *              SCSI adapter.
 *
 *
 * EXECUTION ENVIRONMENT:
 *              This function is called from the process level.  Only KERNEL
 *              processes can call this function.
 *
 * NOTES:       
 *
 *
 *
 * EXTERNAL CALLS:
 *              xmalloc              bzero
 *
 * RETURNS:
 *              0 for good completion
 *		ENOMEM	- error allocating resources
 */
int
vsc_alloc_b_link_pool (struct scsi_info * scsi)  /* virtual scsi structure */
{
    int            i;            /* loop counter    */
    struct b_link *b;            /* current b_link  */
    struct b_link *prevb;        /* previous b_link */
    uint           num_bytes;    /* number of bytes */
    int            ret_code = 0; /* return code     */

    /*
     * Allocate all the memory needed in one big chunk.
     */
    num_bytes = scsi->ddi.num_tm_bufs * (sizeof (struct b_link));
    scsi->b_pool = (struct b_link *) xmalloc (num_bytes,
					      (uint) 12, pinned_heap);
    if (scsi->b_pool == NULL)
    {
	ret_code = ENOMEM;
	return (ret_code);
    }
    bzero ((char *)scsi->b_pool, num_bytes);

    /*
     * Parse the b_link pool of memory into b_links.  Link each b_link to the
     * tail of the free list.
     */
    scsi->head_free = scsi->b_pool;
    prevb = b = scsi->head_free;
    b->sp = scsi;
    b->adap_devno = scsi->ddi.parent_unit_no;

    for (i = 0; i < (scsi->ddi.num_tm_bufs - 1); i++)
    {
	b++;
	prevb->next = b;
	b->sp = scsi;
	b->adap_devno = scsi->ddi.parent_unit_no;

	prevb++;
    }

    b->next = NULL;

    return (ret_code);

}  /* end vsc_alloc_b_link_pool */
