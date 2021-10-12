static char sccsid[] = "@(#)93  1.2  src/bos/kernext/lft/config/lftconfig.c, lftdd, bos411, 9430C411a 7/22/94 10:18:46";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: lftconfig
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

/* ---------------------------------------------------------------------- * 
 * Includes								  * 
 * ---------------------------------------------------------------------- */

#include <lft.h>
#include <sys/signal.h>       
#include <sys/sysmacros.h>   
#include <sys/lockl.h>   
#include <sys/syspest.h>    
#include <sys/malloc.h>    
#include <sys/device.h> 
#include <sys/uio.h>   
#include <graphics/gs_trace.h>
#include <lft_debug.h>

BUGVDEF(db_lftconfig, 99);

GS_MODULE(lftconfig);			/* TRACE registration		*/
extern int devswchg();

/* -------------------------------------------------------------------- * 
 * Functions:								* 
 *		lft_config()						* 
 *									* 
 * -------------------------------------------------------------------- */ 

extern int      lft_init();
extern int      lft_term();
extern int      lft_fonts_init(char *ff_names);
extern int      lft_fonts_term();
extern int      lft_swkbd_init(char *swkbd_name);
extern int      lft_swkbd_term();

lft_ptr_t lft_ptr = NULL;	/* lft anchor structure pointer */


/* -------------------------------------------------------------------- * 
 * Name:	lftconfig						* 
 * Description:	Configures the lft					* 
 * Parameters:								* 
 *	input	dev_t	devno;		  Device number of lft		* 
 *	input	int	command;	  Command to perform		* 
 *	input	struct	uio	*uio_ptr; UIO pointer with DDS 		* 
 *									* 
 * Process:								* 
 *	If the command is CFG_INIT, initialize the lft by invoking the  * 
 *	lft_init function.						* 
 *	If the command is CFG_TERM, terminate the lft by invoking the   * 
 *	lft_term function.						* 
 *									* 
 * Return:								* 
 *	SUCCESS if successful						* 
 *	Otherwise an error occurred					* 
 *									* 
 * -------------------------------------------------------------------- */
int lftconfig( devno, command, uio_ptr )
dev_t		devno;			/* Major/minor number		*/
int		command;		/* Command			*/
struct	uio	*uio_ptr;		/* UIO pointer with DDS		*/

{
    
    int rc;			/* Return code			*/
    int	dds_sz;			/* Computed size of dds struc	*/
    lft_dds_t *dds_ptr;
    struct iovec *iov;
    int name_len;
    char *ff_names;		/* Pointer to font file names */
    char *swkbd_name;		/* Pointer to the swkbd file name */
    int *savefunc;		/* temp pointer for devswchg */
    
    GS_ENTER_TRC(HKWD_GS_LFT,lftconfig,1,lftconfig,devno,
		 command, uio_ptr, 0, 0);
    switch(command)
    {
    case CFG_INIT :
	if(lft_ptr != NULL && lft_ptr->initialized)
	{
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: lft is already initialized\n"));
            lfterr(NULL,"LFTDD", "lftconfig", NULL, 0, LFT_ALREADY_INIT, UNIQUE_1);
	    return(EALREADY);
	}
	if(lft_ptr)		/* Implies that the initialized  */
	{			/*   flag is not ON		*/
	    lfterr(NULL,"LFTDD", "lftconfig", NULL, 0, LFT_INV_STATE, UNIQUE_2);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftcongig: lft is in an unknown state - reboot\n"));
	    return(EAGAIN);
	}
	/*
	  Allocate and initialize (to zero) the lft structure
	  */
	lft_ptr = xmalloc(sizeof(lft_t), 0, pinned_heap);
	if (lft_ptr == NULL)		/* No memory */
	{
	    lfterr(NULL,"LFTDD", "lftconfig", "xmalloc", 0, LFT_ALLOC_FAIL, UNIQUE_3);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: Cannot allocate the lft structure\n"));	
	    return(ENOMEM);
	}
	
	bzero(lft_ptr, sizeof(lft_t));
	
	/*
	  Just to be safe - set lft to NOT initialized
	  */

	lft_ptr->initialized = FALSE;

	/*
	  Allocate and initialize (to zero)  the lft_dds struct.
	  We would normally use uiomove, but in this case the size of
	  the dds is dependent upon the number of displays. This is set
	  up in the lft config method (cfglft).  So instead of the uiomove
	  we do a copyin.  The base address is contained in the uio
	  struct and so is the length.  This should not be a problem as
	  uiomove ends up doing a vmcopyin which is the same function that
	  copyin calls.
	  */
	
	iov = uio_ptr->uio_iov;		/* Get the iov structure	*/
	dds_sz = iov->iov_len;		/* Length of the dds structure	*/
	
	if(dds_sz <= 0)			/* Sanity check			*/
	{
	    lfterr(NULL,"LFTDD", "lftconfig", NULL, 0, LFT_USPACE, UNIQUE_4);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: User space data - invalid count\n"));
	    return(EINVAL);
	}
	/*
	  Allocate some space for the dds struct
	  */
	dds_ptr = xmalloc(dds_sz, 0, pinned_heap);
	
	if (dds_ptr == NULL)
	{	
            lfterr(NULL,"LFTDD", "lftconfig", "xmalloc", 0, LFT_ALLOC_FAIL, UNIQUE_5);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: Cannot allocate the lft dds structure\n"));
	    return(ENOMEM);
	}
	bzero(dds_ptr, dds_sz);
	rc = copyin(iov->iov_base, dds_ptr, dds_sz);
	if(rc)			/* Error	       */
	{
	    lfterr(NULL,"LFTDD", "lftconfig", "copyin", rc, LFT_COPYIN, UNIQUE_6);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: could not copy the dds\n"));
	    xmfree(dds_ptr, pinned_heap);
	    xmfree(lft_ptr, pinned_heap);
	    return(rc);
	}

	lft_ptr->dds_ptr = dds_ptr;  	  /* Init. dds ptr in lft struct*/
	
	/*
	  If no font file names or number of fonts is invalid
	  */
	if( (dds_ptr->font_file_name == NULL)	
	   || (dds_ptr->number_of_fonts <= 0 ))
	{
	    xmfree(dds_ptr, pinned_heap);
	    xmfree(lft_ptr, pinned_heap);
	    lfterr(NULL,"LFTDD", "lftconfig", NULL, 0, LFT_NOFONT_FILE, UNIQUE_7);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: No font file names in DDS\n"));
	    return(ENOENT);		/* No such file/dir */
	}
	/*
	  Compute total length of file names and allocate space
 	  */
	name_len = dds_ptr->number_of_fonts*FILE_NAME_LEN;
	ff_names = (char *) xmalloc(name_len, 3, pinned_heap);
	if(ff_names == NULL)
	{
	    lfterr(NULL,"LFTDD", "lftconfig", "xmalloc", 0, LFT_ALLOC_FAIL, UNIQUE_8);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: Cannot allocate space for font file names\n"));
	    xmfree(dds_ptr, pinned_heap);
            xmfree(lft_ptr, pinned_heap);
	    return(ENOMEM);
	}

	bzero(ff_names, name_len);
	/*
	  Copy in the font file names
	  */
	rc = copyin(dds_ptr->font_file_name, ff_names, name_len);
	if(rc)
	{
	    lfterr(NULL,"LFTDD", "lftconfig", "copyin", rc, LFT_COPYIN, UNIQUE_9);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: could not copy the font file names\n"));
            xmfree(dds_ptr, pinned_heap);
            xmfree(lft_ptr, pinned_heap);
            xmfree(ff_names, pinned_heap);
	    return(rc);
	}	
	
	dds_ptr->font_file_name=ff_names; /* init fonts names in kernel dds */
	
	/*
	  Font initialization
	  */
	rc = lft_fonts_init(ff_names);
	if (rc)
	{
	    xmfree(dds_ptr, pinned_heap);
	    xmfree(lft_ptr, pinned_heap);
	    xmfree(ff_names, pinned_heap);
	    lfterr(NULL,"LFTDD", "lftconfig", "lft_fonts_init", rc, LFT_FONT_INIT, UNIQUE_10);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: Font initialization failed\n"));
	    return(rc);
	}
	
	/*	
	  Software Keyboard Initialization
	  */
	if(dds_ptr->swkbd_file == NULL)	/* No swkbd map file !! */
	{
	    lfterr(NULL,"LFTDD", "lftconfig", NULL, 0, LFT_NOSWKB_FILE, UNIQUE_11);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: No software keyboard map file pointer in DDS\n"));
	    lft_fonts_term();
	    xmfree(dds_ptr, pinned_heap);
	    xmfree(lft_ptr, pinned_heap);
	    return(ENOENT);
	}
	/*
	  Allocate space for and copyin the swkbd file name
	  */
	swkbd_name = xmalloc(FILE_NAME_LEN, 0, pinned_heap);
	if(swkbd_name == NULL)
	{
	    lfterr(NULL,"LFTDD", "lftconfig", "xmalloc", 0, LFT_ALLOC_FAIL, UNIQUE_12);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: Could not allocate space for swkbd map file name\n"));
	    lft_fonts_term();
	    xmfree(dds_ptr, pinned_heap);
	    xmfree(lft_ptr, pinned_heap);
	    return(ENOMEM);
	}
	rc = copyin(dds_ptr->swkbd_file, swkbd_name, FILE_NAME_LEN);
	if(rc)
	{
	    lfterr(NULL,"LFTDD", "lftconfig", "copyin", rc, LFT_COPYIN, UNIQUE_13);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: Error copying in the swkbd file name\n"));
	    lft_fonts_term();
	    xmfree(dds_ptr, pinned_heap);
	    xmfree(lft_ptr, pinned_heap);
	    xmfree(swkbd_name, pinned_heap);
	    return(rc);
	}
	dds_ptr->swkbd_file=swkbd_name; /* Save swkb file name in kernel dds */

	/*
	  Initialize the swkbd 
	  */
	rc = lft_swkbd_init(swkbd_name);
	if (rc)
	{
	    lft_fonts_term();			/* Clean up fonts*/
	    xmfree(dds_ptr, pinned_heap);
	    xmfree(lft_ptr, pinned_heap);
	    lfterr(NULL,"LFTDD", "lftconfig", "lft_swkbd_init", rc, 
						LFT_SWKB_INIT, UNIQUE_14);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: software keyboard initialization failed\n"));
	    return(rc);
	}
	/*	
	  Initialize the lft
	  */	
	rc = lft_init();
	if (rc) 
	{
	    lft_fonts_term();			/* Clean up fonts */
	    lft_swkbd_term();			/* Clean up swkbd */
	    xmfree(dds_ptr, pinned_heap);	/* Free dds mem   */
	    xmfree(lft_ptr, pinned_heap);	/* Free lft struct*/
	    lfterr(NULL,"LFTDD", "lftconfig", "lft_init", rc, LFT_INIT_FAIL, UNIQUE_15);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: lft initialization failed\n"));
	    return(rc);
	}
	/*
	  Invoke the streams specific initialization function
	  */	
	rc = lft_streams_init();
	if (rc)
	{
	    lft_term();				/* Terminate lft  */
	    lft_swkbd_term();			/* Clean up swkbd */
	    lft_fonts_term();			/* Clean up fonts */
	    xmfree(dds_ptr, pinned_heap);	/* Free dds struc */
	    xmfree(lft_ptr, pinned_heap);	/* Free lft struc */
	    lfterr(NULL,"LFTDD", "lftconfig", "lft_streams_init", rc,
                    LFT_STREAMS_INIT, UNIQUE_16);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: lft streams initialization failed\n"));
	    return(rc);
	}
	/*
	    Now add the config pointer into the devsw table
	*/
	rc = devswchg(devno, DSW_CONFIG, lftconfig, &savefunc);
	if(rc)
	{
	    lfterr(NULL,"LFTDD", "lftconfig", "devswchg", rc, 0, 0);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: devswchg failed: rc=%d\n",rc));
	    return(rc);
	}
	/*	
	  Pin required modules
	  */	
	rc = pincode((void (*)())lft_term );
	if (rc) 
	{
	    lft_streams_term();			/* Streams cleanup */
	    lft_term();				/* Terminate lft   */
	    lft_swkbd_term();			/* Clean up swkbd  */
	    lft_fonts_term();			/* Clean up fonts  */
	    xmfree(dds_ptr, pinned_heap);	/* Free dds struc  */
	    xmfree(lft_ptr, pinned_heap);	/* Free lft struc  */
	    lfterr(NULL,"LFTDD", "lftconfig", "pincode", rc, LFT_PIN_FAIL, UNIQUE_17);
	    BUGLPR( db_lftconfig, BUGNFO, ("lftconfig: could not pin code\n"));
	    return(rc);
	}
	
	/*
	  Set the initialized flag in lft
	  */
	lft_ptr->initialized = TRUE;
	return(SUCCESS);
	break;			/* CFG_INIT completed */
	
    case CFG_TERM:
	/*
	  If lft open count is > 0, lft cannot be terminated
	  */
	if (lft_ptr->open_count > 0)
	{
	    lfterr(NULL,"LFTDD", "lftconfig", NULL, 0, LFT_TERM_FAIL, UNIQUE_18);
	    BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: Cannot terminate lft, being used\n"));
	    return(EBUSY);
	}
	/*
	  Perform cleanup of streams, lft, swkbd and fonts
	  Return codes are not checked - we want to continue
	  cleanup even if one of the TERM function fails.
	  */
	lft_streams_term();
	lft_term();
	lft_swkbd_term();
	lft_fonts_term();

	/*
	  Unpin code pinned in CFG_INIT
	  */
	unpincode(lft_term);

	/*
	   Release the space allocated for file names
	*/
	xmfree(lft_ptr->dds_ptr->swkbd_file, pinned_heap);
        xmfree(lft_ptr->dds_ptr->font_file_name, pinned_heap);

	xmfree(lft_ptr->dds_ptr, pinned_heap);
	xmfree(lft_ptr, pinned_heap);
	break;
	/*	
	  The unknown soldier
	  */	
    default:
	lfterr(NULL,"LFTDD", "lftconfig", NULL, 0, LFT_BAD_CMD, UNIQUE_19);
	BUGLPR(db_lftconfig, BUGNFO, ("lftconfig: Unknown command\n"));
	return(EINVAL);
    }	/* End of switch */
}
