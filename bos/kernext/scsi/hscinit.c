static char sccsid[] = "@(#)65	1.4.5.6  src/bos/kernext/scsi/hscinit.c, sysxscsi, bos411, 9428A410j 5/26/94 15:21:11";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	hsc_config, hsc_alloc_adap, hsc_free_adap, hsc_config_adapter
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        hscinit.c                                               */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Source File                     */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

#include	"hscincl.h"

/************************************************************************/
/* Global device driver static data areas                               */
/************************************************************************/
/* static array containing pointer to adapter information               */
struct adapter_def *adapter_ptrs[MAXADAPTERS] = {NULL};

/* static variable to count number of inited adapters                   */
ulong   inited_adapters = 0;

/* static variable to count number of opened adapters                   */
ulong   opened_adapters = 0;

/* number of pages of memory allocated for adapter structures           */
ulong   num_adap_pages = 0;

/* total number of pages of memory allocated for structures             */
ulong   num_totl_pages = 0;

/* global static structure to hold the driver's EPOW handler struct     */
struct intr epow_struct;

/* global adapter device driver lock word                               */
lock_t  hsc_lock = LOCK_AVAIL;

/* global adapter device driver mp lock word                            */
Simple_lock  hsc_mp_lock;

/* MP lock word for ioctl's */

Simple_lock  hsc_ioctl_scbuf_lock;

/* MP lock word for epow */

Simple_lock hsc_epow_lock;

/* global driver debug print enable/disable                             */
int     hsc_debug = FALSE;

/* global driver debug error print enable/disable                       */
int     hsc_error = FALSE;

/* global driver component dump table pointer                           */
struct hsc_cdt_tab *hsc_cdt = NULL;

#ifdef HSC_TRACE
/* global driver trace enable/disable                                   */
int     hsc_trace = TRUE;


struct trace_element hsc_trace_tab[TRACE_ENTRIES];	/* the trace table */

struct trace_element *hsc_trace_ptr = NULL;	/* pointer to trace table */
#endif HSC_TRACE

/************************************************************************/
/* End of global device driver static data areas                        */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* NAME:        hsc_config                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Configuration Routine                    */
/*                                                                      */
/*      For the INIT option, this routine allocates and initializes     */
/*      data structures required for processing user requests to the    */
/*      adapter.  If the TERM option is specified, this routine will    */
/*      delete a previously defined device and free the structures      */
/*      associated with it.  If the QVPD option is specified, this      */
/*      routine will return the adapter vital product data.             */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*      adap_ddi - adapter dependent information structure              */
/*      uio     - user i/o area struct                                  */
/*      devsw   - kernel entry point struct                             */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      op      - operation code (INIT, TERM, or QVPD)                  */
/*      uiop    - pointer to uio structure for data for the             */
/*                specified operation code                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      EIO     - bad operation, or permanent I/O error                 */
/*      EBUSY   - on terminate, means device still opened               */
/*      ENOMEM  - memory space unavailable for required allocation      */
/*      EINVAL  - invalid config parameter passed                       */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      devswadd        devswdel                                        */
/*      w_init          w_clear                                         */
/*      bcopy           bzero                                           */
/*      xmalloc         xmfree                                          */
/*      uiomove                                                         */
/*                                                                      */
/************************************************************************/
int
hsc_config(
	   dev_t devno,
	   int op,
	   struct uio * uiop)
{
    struct adapter_def *ap;
    struct devsw hsc_dsw;
    struct adap_ddi local_ddi;
    int     ret_code, rc;
    int     i, i_hash, ioerr;
    uchar   vpd[VPD_SIZE];
    int     avail_tcws, tcws, data;
    extern int nodev();
    long    total_tcw_length;


    ret_code = 0;	/* default return code */

    rc = lockl(&(hsc_lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC) {
	ret_code = EIO;	/* error--kernel service call failed */
	goto end;
    }

    /* search adapter list for this devno */
    /* build the hash index */
    i_hash = minor(devno) & ADAP_HASH;
    ap = adapter_ptrs[i_hash];
    while (ap != NULL) {
	if (ap->devno == devno)
	    break;
	ap = ap->next;
    }	/* endwhile */


    switch (op) {
/************************************************************************/
/*	handle request to add an adapter definition here                */
/************************************************************************/
      case CFG_INIT:	/* this is an add request */
	if (ap != NULL) {	/* if already in pointer table */
	    ret_code = EIO;	/* error--already initialized */
	    goto exit;
	}	/* not already in pointer table */

	if (inited_adapters == 0) {

	    /* initialize the devsw struct */
	    hsc_dsw.d_open = (int (*) ()) hsc_open;
	    hsc_dsw.d_close = (int (*) ()) hsc_close;
	    hsc_dsw.d_read = nodev;
	    hsc_dsw.d_write = nodev;
	    hsc_dsw.d_ioctl = (int (*) ()) hsc_ioctl;
	    hsc_dsw.d_strategy = (int (*) ()) hsc_strategy;
	    hsc_dsw.d_ttys = 0;
	    hsc_dsw.d_select = nodev;
	    hsc_dsw.d_config = (int (*) ()) hsc_config;
	    hsc_dsw.d_print = nodev;
	    hsc_dsw.d_dump = (int (*) ()) hsc_dump;
	    hsc_dsw.d_mpx = nodev;
	    hsc_dsw.d_revoke = nodev;
	    hsc_dsw.d_dsdptr = 0;
	    hsc_dsw.d_selptr = 0;
#ifdef _POWER_MP
	    hsc_dsw.d_opts = DEV_MPSAFE;
#else
	    hsc_dsw.d_opts = 0;
#endif

	    rc = devswadd(devno, &hsc_dsw);
	    if (rc != 0) {
		ret_code = EIO;	/* devswadd failed */
		goto exit;
	    }

	    opened_adapters = 0;	/* init open adapter counter */

#ifdef HSC_TRACE
	    /* init internal trace table pointer */
	    hsc_trace_ptr = &hsc_trace_tab[0];
#endif HSC_TRACE

	    /* set up for the component dump table entry */
	    /* allocate storage */
	    hsc_cdt = (struct hsc_cdt_tab *) xmalloc((uint)
						 sizeof(struct hsc_cdt_tab),
						     (uint) 2,
						     pinned_heap);
	    if (hsc_cdt == NULL) {
		/* xmalloc failed */
		(void) devswdel(devno);	/* clean up */
		ret_code = EIO;
		goto exit;
	    }

	    /* initialize the storage for the dump table */
	    bzero((char *) hsc_cdt, sizeof(struct hsc_cdt_tab));

	}

	/* move adapter configuration data into local area */
	rc = uiomove((caddr_t) (&local_ddi),
		     (int) sizeof(struct adap_ddi),
		     UIO_WRITE,	/* from uio space to kernel space */
		     (struct uio *) uiop);
	if (rc != 0) {	/* if unsuccessful copy */
	    if (inited_adapters == 0) {
		(void) xmfree((void *) hsc_cdt, pinned_heap);
		(void) devswdel(devno);	/* clean up */
	    }
	    ret_code = EIO;	/* unsuccessful uiomove */
	    goto exit;
	}

	/* do any data validation here */
	/* tcw area must start on a tcw boundary, and be twice MAXREQUEST */
	/* in length, intrpt priority must be CLASS2, bus type must be    */
	/* Micro Channel, and base address must be on a 4KB boundary.     */
	if (((local_ddi.tm_enabled) &&
	     (local_ddi.tcw_length < (SC_IM_MIN_TCWLEN + SC_TM_MIN_TCWLEN))) ||
	    (!(local_ddi.tm_enabled) &&
	     (local_ddi.tcw_length < (SC_IM_MIN_TCWLEN))) ||
	    (local_ddi.tm_tcw_percent < 0) ||
	    (local_ddi.tm_tcw_percent > 100) ||
	    (local_ddi.tcw_start_addr & (TCWRANGE - 1)) ||
	    (local_ddi.int_prior != INTCLASS2) ||
	    (local_ddi.bus_type != BUS_MICRO_CHANNEL) ||
	    (local_ddi.base_addr & 0xfff)) {

	    /* if a problem found with ddi data, clean up and exit */
	    if (inited_adapters == 0) {
		(void) xmfree((void *) hsc_cdt, pinned_heap);
		(void) devswdel(devno);	/* clean up */
	    }
	    ret_code = EINVAL;	/* unable to allocate adapter space */
	    goto exit;
	}
        /* if tm_enabled attribute in ddi is false then no tcws are needed */
        /* for target mode */
        if (local_ddi.tm_enabled == 0) local_ddi.tm_tcw_percent = 0;
	/* make sure the minimum initiator tcw area is left so machine is
	   guaranteed to boot/run despite target mode requirements */
	local_ddi.tm_tcw_length = (((local_ddi.tcw_length - SC_IM_MIN_TCWLEN)
				    * local_ddi.tm_tcw_percent) / 100);
        /* ensure that tm tcw length is on a 4K boundary */
        if (local_ddi.tm_tcw_length & 0xfff) {
           local_ddi.tm_tcw_length &= ~(0xfff);
           local_ddi.tm_tcw_length += 0x1000;
        }
        /* ensure that minimum tm tcw length is satisfied if target mode is
	   to be used */
	if ((local_ddi.tm_tcw_length > 0) &&
	    (local_ddi.tm_tcw_length < SC_TM_MIN_TCWLEN)) {
	    local_ddi.tm_tcw_length = SC_TM_MIN_TCWLEN;
	}
	total_tcw_length = local_ddi.tcw_length;
	/* tcw_length is now made the initiator-only tcw length */
	local_ddi.tcw_length -= local_ddi.tm_tcw_length;
	local_ddi.tm_tcw_start_addr = local_ddi.tcw_start_addr +
	    local_ddi.tcw_length;


	/* calculate size of tcw table to be malloc'ed */
	/* (this is number of complete tcws available) */
	tcws = total_tcw_length / TCWRANGE;

	/* allocate adapter struct memory, and put in table */
	ap = hsc_alloc_adap(devno, (uint) tcws);
	if (ap == NULL) {	/* NULL means unsuccessful allocation */
	    if (inited_adapters == 0) {
		(void) xmfree((void *) hsc_cdt, pinned_heap);
		(void) devswdel(devno);	/* clean up */
	    }
	    ret_code = ENOMEM;	/* unable to allocate adapter space */
	    goto exit;
	}

	/* copy local ddi to adapter structure */
	bcopy(&local_ddi, &ap->ddi, sizeof(struct adap_ddi));

	/* if initiator tcw area greater than 1MB+STA area */
	/* then scale-up maximum transfer length */
	if (ap->ddi.tcw_length > (0x100000 + STA_ALLOC_SIZE))
	    ap->maxxfer = ap->ddi.tcw_length - (0x100000 + STA_ALLOC_SIZE) +
		MAXREQUEST;
	else
	    ap->maxxfer = MAXREQUEST;

	/* if maximum transfer greater than the adapter's hardware limit
	   of 16MB-1, then set it back to 16MB-1. */
	if (ap->maxxfer > 0x00ffffff) {
	    ap->maxxfer = 0x00ffffff;
	}

	/* do any required processing on the configuration data */
	ap->ddi.bus_id &= 0x0ff00000;	/* mask off all but BUID */
	ap->ddi.bus_id |= 0x800c0020;	/* enable I/O mode, addr check, addr
					   increment, TCW bypass */
	ap->ddi.card_scsi_id &= 0x07;	/* mask for scsi id */

        /* the default for all adapters is to not support cmd_tag_queuing.*/
        /* Queuing can be enabled via the SC_ENABLE_CMD_Q option of the   */
        /* SCIODNLD ioctl.  This method allows queuing to be enabled      */
        /* by the configuration method when it determines that the adapter*/
        /* is of the proper level and has the proper level of microcode   */
        /* to support cmd_tag_queing.                                     */
        ap->enable_queuing = FALSE;

	/* total number of tcws available for initiator mode */
	ap->num_tcws = ap->ddi.tcw_length / TCWRANGE;
	/* get net available tcws */
	avail_tcws = ap->num_tcws - NUM_STA_TCWS;
	/* calc starting page tcw */
	ap->page_req_begin = 0;
	/* calculate ending page tcw */
	ap->page_req_end = ((avail_tcws) >> 1) - 1;
	/* calc starting large tcw */
	ap->large_req_begin = (avail_tcws) >> 1;
	/* calc ending large tcw */
	ap->large_req_end = avail_tcws - 1;
	/* calc starting STA tcw */
	ap->sta_tcw_start = ap->num_tcws - NUM_STA_TCWS;

	/* if enough tcws available for maximum number of page-sized */
	/* requests, then make sure the remaining ones are given to  */
	/* the large request area.                                   */
	if (ap->page_req_end > (NUM_MBOXES - 1)) {
	    /* new maximum page req limit  */
	    ap->page_req_end = (NUM_MBOXES - 1);
	    /* new minimum large req limit */
	    ap->large_req_begin = (ap->page_req_end + 1);
	}
	strcpy(ap->tm_start, "tmscsi");	/* just to locate tmscsi data */
	ap->tgt_req_begin = ap->num_tcws;
	ap->tgt_req_end = tcws - 1;
	ap->tgt_next_req = ap->tgt_req_begin;
	ap->num_tgt_tcws = tcws - ap->num_tcws;
	ap->num_tgt_tcws_used = 0;
	/* init adapter watchdog timer struct */
	ap->wdog.dog.next = NULL;
	ap->wdog.dog.prev = NULL;
	ap->wdog.dog.func = hsc_watchdog;
	ap->wdog.dog.count = 0;
	ap->wdog.dog.restart = 0;	/* set this before each use */
	ap->wdog.timer_id = SC_ADAP_TMR;
	ap->wdog.adp = ap;
#ifdef _POWER_MP

	while (w_init(&ap->wdog.dog));
#else
	w_init(&ap->wdog.dog);
#endif

	/* init adapter watchdog timer struct */
	ap->wdog2.dog.next = NULL;
	ap->wdog2.dog.prev = NULL;
	ap->wdog2.dog.func = hsc_watchdog;
	ap->wdog2.dog.count = 0;
	ap->wdog2.dog.restart = 0;	/* set this before each use */
	ap->wdog2.timer_id = SC_ADAP_TMR_2;
	ap->wdog2.adp = ap;
#ifdef _POWER_MP
	while(w_init(&ap->wdog2.dog));
#else
	w_init(&ap->wdog2.dog);
#endif

	/* init wait queue watchdog timer struct */
	ap->wdog3.dog.next = NULL;
	ap->wdog3.dog.prev = NULL;
	ap->wdog3.dog.func = hsc_watchdog;
	ap->wdog3.dog.count = 0;
	ap->wdog3.dog.restart = WAITQ_CMD_T_O;	/* set this just once */
	ap->wdog3.timer_id = SC_ADAP_TMR_3;
	ap->wdog3.adp = ap;
#ifdef _POWER_MP
	while(w_init(&ap->wdog3.dog));
#else
	w_init(&ap->wdog3.dog);
#endif


	/* make adapter ready for first open, config POS regs */
	rc = hsc_config_adapter(ap);
	if (rc != 0) {	/* unsuccessful adapter config */
#ifdef _POWER_MP
	    while(w_clear(&ap->wdog.dog));
	    while(w_clear(&ap->wdog2.dog));
	    while(w_clear(&ap->wdog3.dog));
#else
	    w_clear(&ap->wdog.dog);
	    w_clear(&ap->wdog2.dog);
	    w_clear(&ap->wdog3.dog);
#endif
	    hsc_free_adap(ap);	/* free memory for the adapter */
	    if (inited_adapters == 0) {
		(void) xmfree((void *) hsc_cdt, pinned_heap);
		(void) devswdel(devno);	/* clean up */
	    }
	    ret_code = EIO;	/* indicate I/O error */
	    goto exit;
	}
        if(inited_adapters == 0) {

	  	/* Initialize MP locks on first adapter to be configured*/

                lock_alloc(&(hsc_mp_lock),LOCK_ALLOC_PIN,HSC_ADAP_LOCK_CLASS,
                           -1);
                lock_alloc(&(hsc_ioctl_scbuf_lock),LOCK_ALLOC_PIN,
                           HSC_IOCTL_LOCK_CLASS,-1);
                lock_alloc(&(hsc_epow_lock),LOCK_ALLOC_PIN,
                           HSC_EPOW_LOCK_CLASS,-1);

	  	simple_lock_init(&(hsc_mp_lock));
        	simple_lock_init(&(hsc_ioctl_scbuf_lock));
	  	simple_lock_init(&(hsc_epow_lock));


	}

	ap->inited = TRUE;	/* mark adapter inited */
	ap->opened = FALSE;	/* mark adapter not opened */
	inited_adapters++;	/* increment global counter */

	break;


/************************************************************************/
/*	handle request to terminate an adapter here                     */
/************************************************************************/
      case CFG_TERM:	/* this is a delete request */
	if (ap == NULL) {
	    ret_code = 0;	/* already terminated */
	    goto exit;
	}

	if (ap->opened) {
	    ret_code = EBUSY;	/* error--not closed */
	    goto exit;
	}

#ifdef _POWER_MP
	while(w_clear(&ap->wdog.dog));	/* clear the adapter cmd timer */
	while(w_clear(&ap->wdog2.dog));	/* clear the adapter cmd timer */
	while(w_clear(&ap->wdog3.dog));	/* clear the adapter cmd timer */
#else
	w_clear(&ap->wdog.dog);	/* clear the adapter cmd timer */
	w_clear(&ap->wdog2.dog);	/* clear the adapter cmd timer */
	w_clear(&ap->wdog3.dog);	/* clear the adapter cmd timer */
#endif
	ap->inited = FALSE;	/* mark adapter not inited */
	hsc_free_adap(ap);	/* free memory for the adapter */
	inited_adapters--;	/* dec the global counter */
	if (inited_adapters == 0) {
	    (void) xmfree((void *) hsc_cdt, pinned_heap);
	    (void) devswdel(devno);	/* clean up */
	}
	break;


/************************************************************************/
/*	handle query for adapter VPD here                               */
/************************************************************************/
      case CFG_QVPD:	/* this is a query request */
	if ((ap == NULL) || (!ap->inited)) {
	    ret_code = EIO;	/* error--not inited       */
	    goto exit;
	}

	/* get adapter's vital product data */
	ioerr = FALSE;	/* init to no perm i/o errors */
	for (i = 0; i < VPD_SIZE; i++) {
	    if (hsc_write_POS(ap, (uint) POS7, (uchar) 0x00) != 0) {
		ioerr = TRUE;
		break;
	    }
	    if (hsc_write_POS(ap, (uint) POS6, (uchar) (i + 1)) != 0) {
		ioerr = TRUE;
		break;
	    }
	    data = hsc_read_POS(ap, (uint) POS3);
	    if (data == -1) {
		ioerr = TRUE;
		break;
	    }

	    vpd[i] = (uchar) data;	/* load the vpd byte */

	}	/* endfor */

	/* leave POS6 in known state */
	(void) hsc_write_POS(ap, (uint) POS6, (uchar) 0x01);

	if (ioerr == TRUE) {
	    ret_code = EIO;	/* permanent I/O error */
	    goto exit;	/* leave               */
	}

	/* move the VPD to the caller's uio structure */
	rc = uiomove((caddr_t) vpd,
		     (int) VPD_SIZE,
		     UIO_READ,	/* move from kernel to uio space */
		     uiop);
	if (rc != 0) {	/* if unsuccessful copy */
	    ret_code = EIO;	/* bad data move */
	    goto exit;
	}
	break;


/************************************************************************/
/*	handle invalid config parameter here                            */
/************************************************************************/
      default:	/* this is an invalid request */
	ret_code = EINVAL;	/* error--invalid request */
	goto exit;

    }	/* endswitch */

exit:
    unlockl(&(hsc_lock));	/* release the lock */
end:
    return (ret_code);
}  /* end hsc_config */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_alloc_adap                                          */
/*                                                                      */
/* FUNCTION:    Allocate Adapter Information Structure                  */
/*                                                                      */
/*      This internal routine performs actions required to obtain       */
/*      the required memory to hold needed adapter structures.          */
/*      Then, if successful, it saves the pointer to the adapter        */
/*      structure in the global pointer table, and returns the          */
/*      pointer to the caller as the return value.                      */
/*                                                                      */
/*      The needed structure includes the adapter dependent structure,  */
/*      the device information vector table, the device information     */
/*      structures, and the mailboxes.                                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called on the process level.           */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - adapter major/minor number                            */
/*      tcw_size - size, in bytes, needed for tcw table area            */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = NULL if unsuccessful completion (no resources     */
/*                    will have been allocated)                         */
/*                  = non-NULL indicates successful completion and      */
/*                    return value is pointer to adapter information    */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      xmalloc    xmfree                                               */
/*      bzero                                                           */
/*                                                                      */
/************************************************************************/
struct adapter_def *
hsc_alloc_adap(
	       dev_t devno,
	       uint tcw_size)
{
    struct adapter_def *ap, *temp_ptr;
    int     i, j, i_hash, page_count;


    /* malloc adapter struct space word-alligned from pinned_heap */
    ap = (struct adapter_def *) xmalloc((uint) (tcw_size +
						sizeof(struct adapter_def)),
					4, pinned_heap);
    if (ap == NULL) {
	/* xmalloc failed -- return NULL pointer */
	goto end;
    }	/* successfully malloc'ed and pinned adapter struct */

    /* initially, clear the adapter structure */
    bzero(ap, tcw_size + sizeof(struct adapter_def));

    page_count = (tcw_size + sizeof(struct adapter_def)) / PAGESIZE;
    if ((tcw_size + sizeof(struct adapter_def)) % PAGESIZE)
	page_count++;
    ap->page_count = page_count;	/* store the size in the adapter
					   struct */
    num_totl_pages += page_count;	/* increment total allocated page
					   count */
    num_adap_pages += page_count;	/* increment adapter allocated page
					   count */

    /* add this adapter pointer to hash table */
    i_hash = minor(devno) & ADAP_HASH;	/* get hash pointer */
    if (adapter_ptrs[i_hash] == NULL) {	/* if no chain here */
	adapter_ptrs[i_hash] = ap;	/* store in ptr array */
    }
    else {	/* points to a chain */
	temp_ptr = adapter_ptrs[i_hash];	/* point at first one */
	while (temp_ptr->next != NULL)	/* loop until end found */
	    temp_ptr = temp_ptr->next;	/* look at next struct */
	temp_ptr->next = ap;	/* add at end of chain */
    }
    ap->next = NULL;	/* mark new end of chain */
    ap->devno = devno;	/* store the adap devno  */

    /* init the device table entries */
    for (i = 0; i < DEVPOINTERS; i++) {
	ap->dev[i].opened = FALSE;
	ap->dev[i].scsi_id = SID(i);
	ap->dev[i].lun_id = LUN(i);
	ap->dev[i].waiting = FALSE;
	ap->dev[i].num_act_cmds = 0;
	ap->dev[i].qstate = 0;
	ap->dev[i].state = 0;
	ap->dev[i].init_cmd = 0;
	ap->dev[i].pqstate = 0;
	ap->dev[i].stop_event = EVENT_NULL;
	ap->dev[i].head_pend = NULL;
	ap->dev[i].tail_pend = NULL;
	ap->dev[i].head_act = NULL;
	ap->dev[i].tail_act = NULL;
	ap->dev[i].trace_enable = TRUE;
	ap->dev[i].end_flag = 0xffff;
    }
    for (i = IMDEVICES; i < DEVPOINTERS; i++) {
	ap->dev[i].scsi_id = i - IMDEVICES;
	ap->dev[i].lun_id = 0;
    }

    /* for ALL mailboxes, init the common mailbox structure fields */
    for (i = 0; i < NUM_MBOXES; i++) {
	ap->MB[i].id0 = 'M';
	ap->MB[i].id1 = 'B';
	ap->MB[i].id2 = ':';
	ap->MB[i].MB_num = i;
	ap->MB[i].next = NULL;
	ap->MB[i].prev = NULL;
	ap->MB[i].sc_buf_ptr = NULL;
	ap->MB[i].cmd_state = INACTIVE;
	ap->MB[i].tcws_allocated = 0;
	ap->MB[i].tcws_start = 0;
	ap->MB[i].sta_index = 0;
	ap->MB[i].mb.m_sequ_num = 0;
	ap->MB[i].mb.m_cmd_len = 0;
	ap->MB[i].mb.m_xfer_id = 0;
	ap->MB[i].mb.m_op_code = 0;
	ap->MB[i].mb.m_dma_addr = 0;
	ap->MB[i].mb.m_dma_len = 0;
	ap->MB[i].mb.m_scsi_cmd.scsi_op_code = 0;
	ap->MB[i].mb.m_scsi_cmd.lun = 0;
	for (j = 0; j < 10; j++)
	    ap->MB[i].mb.m_scsi_cmd.scsi_bytes[j] = 0;
	ap->MB[i].mb.m_resid = 0;
	ap->MB[i].mb.m_resvd = 0;
	ap->MB[i].mb.m_scsi_stat = 0;
	ap->MB[i].mb.m_extra_stat = 0;
	ap->MB[i].mb.m_adapter_rc = 0;
	ap->MB[i].d_cmpl_done = FALSE;
	ap->MB[i].end_flag = 0xffff;
    }

    /* init the mailbox 0 - 29 structure data fields */
    for (i = 0; i < NUM_MBOXES - 2; i++) {
	/* the following builds the mailbox free list */
	if (i == 0)	/* if first mailbox */
	    ap->MB[i].prev = NULL;
	else
	    ap->MB[i].prev = (struct mbstruct *) (&ap->MB[i - 1].id0);

	if (i == (NUM_MBOXES - 3))	/* if last mailbox */
	    ap->MB[i].next = NULL;
	else
	    ap->MB[i].next = (struct mbstruct *) (&ap->MB[i + 1].id0);

	ap->MB[i].mb.m_sequ_num = i + 1;	/* init to point to next mb */
	ap->MB[i].mb.m_op_code = SCSI_COMMAND;
    }

    /* init the needed adapter information */
    ap->errlog_enable = TRUE;	/* enable logging for init      */
    ap->trace_enable = TRUE;	/* enable adap internal tracing */
    ap->event = EVENT_NULL;	/* needed for sleep/wakeups     */
    ap->IPL_tmr_cnt = 0;	/* needed for hsc_config_adap() */
    ap->MB30_retries = 0;	/* retry count for adap cmds    */
    ap->epow_state = 0;	/* set initial epow state       */
    ap->devices_in_use = 0;	/* clear count for epow hdlr    */
    /* calculate the pointer to the reserved TCW mngmt table */
    ap->TCW_tab = (char *) ((uint) ap + (uint) sizeof(struct adapter_def));
    /* initial head of MB free list */
    ap->head_MB_free = (struct mbstruct *) (&ap->MB[0].id0);
    /* set tail of MB free list */
    ap->tail_MB_free = (struct mbstruct *) (&ap->MB[NUM_MBOXES - 3].id0);
    /* initialize mailbox active and waiting list pointers */
    ap->head_MB_wait = NULL;
    ap->tail_MB_wait = NULL;
    ap->head_MB_act = NULL;
    ap->tail_MB_act = NULL;
    /* set pointer to MB30 mbstruct */
    ap->MB30p = (struct mbstruct *) (&ap->MB[30].id0);
    /* set pointer to MB31 mbstruct */
    ap->MB31p = (struct mbstruct *) (&ap->MB[31].id0);
    ap->end_flag = 0xffff;	/* flag for end of structure    */

end:
    return (ap);

}  /* end hsc_alloc_adap */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_free_adap                                           */
/*                                                                      */
/* FUNCTION:    Free Adapter Information Structures                     */
/*                                                                      */
/*      This internal routine frees memory which was previously         */
/*      allocated by the hsc_alloc_adap() routine for needed            */
/*      adapter structures.                                             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called from any other routine.              */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      xmfree                                                          */
/*                                                                      */
/************************************************************************/
void
hsc_free_adap(
	      struct adapter_def * ap)
{
    struct adapter_def *tptr;
    int     i_hash;

    /* it is already known that ap is contained in the hash chain */
    /* find ap and remove it from the adapter hash table          */
    i_hash = minor(ap->devno) & ADAP_HASH;	/* get hash pointer */
    if (adapter_ptrs[i_hash] == ap)
	adapter_ptrs[i_hash] = ap->next;	/* remove ap from chain */
    else {
	tptr = adapter_ptrs[i_hash];	/* starting pointer */
	while (tptr->next != NULL) {	/* follow chain     */
	    if (tptr->next == ap) {
		tptr->next = ap->next;	/* remove ap from chain */
		break;
	    }
	    tptr = tptr->next;	/* look at next element */
	}	/* endwhile */
    }

    /* decrement adapter page count */
    num_adap_pages -= ap->page_count;
    /* decrement total page count */
    num_totl_pages -= ap->page_count;

    /* unpin and deallocate the adapter structure */
    (void) xmfree((void *) ap, pinned_heap);

}  /* end hsc_free_adap */


/************************************************************************/
/*                                                                      */
/* NAME:        hsc_config_adapter                                      */
/*                                                                      */
/* FUNCTION:    Adapter Configuration Routine                           */
/*                                                                      */
/*      This internal routine performs actions required to make         */
/*      the driver ready for the first open to the adapter.             */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine must be called from the process level only.        */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      ap      - pointer to adapter structure                          */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      return code = 0 for successful return                           */
/*                  = EIO for unsuccessful operation                    */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      delay                                                           */
/*                                                                      */
/************************************************************************/
int
hsc_config_adapter(
		   struct adapter_def * ap)
{
    int     ret_code;
    int     old_pri;
    int     val0, val1;
    uchar   ilvl, data;

    ret_code = 0;	/* default to no error */

/************************************************************************/
/*	check here to make sure card is ready for POS reg set-up        */
/************************************************************************/
    ap->IPL_tmr_cnt = 0;	/* set-up initial value */
    /* loop waiting for either card ready or timeout */
    while (TRUE) {	/* loop as long as it takes */
	val0 = hsc_read_POS(ap, POS0);
	val1 = hsc_read_POS(ap, POS1);
	/* was there any permanent I/O error? */
	if ((val0 == -1) || (val1 == -1)) {
	    ret_code = EIO;
	    goto end;
	}
	/* have both POS0 and POS1 been set to proper values? */
	if (((uchar) val0 == POS0_VAL) && ((uchar) val1 == POS1_VAL)) {
	    break;	/* card is ready for POS reg loading */
	}
	else {	/* card IPL is still going ! */
	    if (ap->IPL_tmr_cnt >= IPL_MAX_SECS) {
		/* here, timed-out waiting for card diag to complete */
		hsc_logerr(ap, ERRID_SCSI_ERR1, NULL, UNKNOWN_CARD_ERR, 45, 0);
		ret_code = EIO;
		goto end;
	    }
	    /* delay, then come back and check POS0 and POS1 again.      */
	    delay(1 * HZ);	/* one second delay */
	    ap->IPL_tmr_cnt++;	/* inc the timer counter */
	}
    }	/* endwhile */


/************************************************************************/
/*	set the card address extension regs to the card base address    */
/************************************************************************/
    /* point POS6 and POS7 at location 256 decimal                      */
    if (hsc_write_POS(ap, POS7, 0x01) != 0) {
	ret_code = EIO;
	goto end;
    }
    if (hsc_write_POS(ap, POS6, 0x00) != 0) {
	ret_code = EIO;
	goto end;
    }
    /* write the most significant byte of the base address to loc 256   */
    data = (ap->ddi.base_addr & 0xff000000) >> 24;
    if (hsc_write_POS(ap, POS3, data) != 0) {
	ret_code = EIO;
	goto end;
    }
    /* point POS6 and POS7 at location 257 decimal                      */
    if (hsc_write_POS(ap, POS6, 0x01) != 0) {
	ret_code = EIO;
	goto end;
    }
    /* write the next most significant byte of the base address to loc 257 */
    data = (ap->ddi.base_addr & 0x00ff0000) >> 16;
    if (hsc_write_POS(ap, POS3, data) != 0) {
	ret_code = EIO;
	goto end;
    }
    /* point POS6 and POS7 at location 258 decimal                        */
    if (hsc_write_POS(ap, POS6, 0x02) != 0) {
	ret_code = EIO;
	goto end;
    }
    /* write the next least sign. byte of the base address to loc 258      */
    /* note that only the top nibble of this byte is used in the base addr */
    data = (ap->ddi.base_addr & 0x0000f000) >> 8;
    if (hsc_write_POS(ap, POS3, data) != 0) {
	ret_code = EIO;
	goto end;
    }
    /* restore POS6 and POS7 registers                                    */
    if (hsc_write_POS(ap, POS7, 0x00) != 0) {
	ret_code = EIO;
	goto end;
    }
    if (hsc_write_POS(ap, POS6, 0x01) != 0) {
	ret_code = EIO;
	goto end;
    }


/************************************************************************/
/*	set control 3 reg with interrupt level and nibble enable bits   */
/************************************************************************/
    switch (ap->ddi.int_lvl) {
      case 3:
	ilvl = 0;
	break;	/* interrupt level  3 */
      case 4:
	ilvl = 1;
	break;	/* interrupt level  4 */
      case 5:
	ilvl = 2;
	break;	/* interrupt level  5 */
      case 7:
	ilvl = 3;
	break;	/* interrupt level  7 */
      case 10:
	ilvl = 4;
	break;	/* interrupt level 10 */
      case 11:
	ilvl = 5;
	break;	/* interrupt level 11 */
      case 12:
	ilvl = 6;
	break;	/* interrupt level 12 */
      case 14:
	ilvl = 7;
	break;	/* interrupt level 14 */
      default:
	ret_code = EIO;
	goto end;	/* catch bad parm   */
    }	/* endswitch */

    data = ilvl;	/* set int level, enable streaming mode */

    /* if dds attribute bb_stream is false then disable nibble mode for */
    /* this adapter by setting bits 3 and 4 in POS4 to 1 */
    if(!(ap->ddi.bb_stream))
        data |= 0x18;

    if (hsc_write_POS(ap, POS4, data) != 0) {
	ret_code = EIO;
	goto end;
    }


/************************************************************************/
/*	set control 1 reg with dma arb level, and misc enable bits      */
/************************************************************************/
    data = ((ap->ddi.dma_lvl) << 1) | 0x61;	/* set arb, enable all  */
    if (hsc_write_POS(ap, POS2, data) != 0) {
	ret_code = EIO;
	goto end;
    }

end:
    ap->IPL_tmr_cnt = 0;	/* reset IPL timer flag */

    return (ret_code);

}  /* end hsc_config_adapter */
