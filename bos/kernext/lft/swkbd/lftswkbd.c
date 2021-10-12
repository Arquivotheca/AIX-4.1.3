static char sccsid[] = "@(#)83  1.3  src/bos/kernext/lft/swkbd/lftswkbd.c, lftdd, bos411, 9428A410j 1/3/94 15:34:29";
/* COMPONENT_NAME: (LFTDD) Low Function Terminal lftswkbd.c
 *
 * FUNCTIONS:
 *
 * ORIGINS:  27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ---------------------------------------------------------------------- *
 * Includes                                                               *
 * ---------------------------------------------------------------------- */

#include <lft.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/fullstat.h>
#include <sys/sysmacros.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <graphics/gs_trace.h>

#include <lft_debug.h>

extern lft_ptr_t	lft_ptr;
BUGVDEF(db_lftswkbd, 99);

GS_MODULE(lftswkbd);

/* -------------------------------------------------------------------	*
 *  Name:               lft_swkbd_init					*
 *  Description:        Initialize the swkbd in LFT			*
 *  Parameters:								*
 *      input   		none		       			*
 *									*
 *  Process:								*
 *	Open and read the swkbd file					*
 *  Returns:								*
 *      0 = Success							*
 *      Otherwise an error code is returned.				*
 * --------------------------------------------------------------------	*/
int
lft_swkbd_init(swkbd_name)
char	*swkbd_name;
{
    lft_dds_t		*dds_ptr;
    int			rc;
    struct file		*fp;
    struct fullstat	f_stat;
    int			f_len;
    int			countp;
    extern struct diacritic diactbl850,	      	/* These are the diacritic */
                            diactbl88591,	/*   tables */
                            diactbl88593,
                            diactbl88597,
                            diactbl88592,
                            diactbl88594,
                            diactbl88599;
    struct diacritic	*lftdiaclst;

    GS_ENTER_TRC(HKWD_GS_LFT,lftswkbd,1,lftswkbd,lft_ptr,
                 lft_ptr->dds_ptr,0,0,0);

    dds_ptr = lft_ptr->dds_ptr;

    rc = fp_open(swkbd_name, O_RDONLY, 0, 0, SYS_ADSPACE, &fp);
    if( rc )
    {
	lfterr(NULL,"LFTDD", "lftswkbd", "fp_open", rc, LFT_FP_OPEN, UNIQUE_1);
	BUGLPR(db_lftswkbd, BUGNFO, ("lft_swkbd_init: Could not open %s\n",dds_ptr->swkbd_file));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftswkbd, 1, lftswkbd, rc);
	return(rc);
    }
    /*
      Get some info about the file
      */

    rc = fp_fstat(fp, &f_stat, sizeof(struct fullstat));
    if( rc )
    {
	lfterr(NULL,"LFTDD", "lftswkbd", "fp_fstat", rc, LFT_FP_FSTAT, UNIQUE_2);
	BUGLPR(db_lftswkbd, BUGNFO, ("lft_swkbd_init: Could not stat file\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftswkbd, 1, lftswkbd, rc);
	fp_close(fp); /* Close swkbd map file */
	return (rc);
    }
    f_len = f_stat.st_size;		/* Length of file in bytes	*/
    /*
      Seek to start of file
      */
    rc = fp_lseek(fp, 0, SEEK_SET);
    if(rc)
    {
	lfterr(NULL,"LFTDD", "lftswkbd", "fp_lseek", rc, LFT_FP_LSEEK, UNIQUE_3);
	BUGLPR(db_lftswkbd, BUGNFO, ("lft_swkbd_init: swkbd file seek failed\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftswkbd, 1, lftswkbd, rc);
	fp_close(fp);  /* Close swkbd map file */
	return(rc);
    }
    /*
      Allocate space for the swkbd file
      */
    lft_ptr->swkbd = xmalloc(f_len, 0, pinned_heap);
    if(lft_ptr->swkbd == NULL)
    {
	lfterr(NULL,"LFTDD", "lftswkbd", NULL, 0, LFT_ALLOC_FAIL, UNIQUE_4);
	BUGLPR(db_lftswkbd, BUGNFO, ("lft_swkbd_init: Could not allocate space\n"));
	GS_EXIT_TRC1(HKWD_GS_LFT, lftswkbd, 1, lftswkbd, ENOMEM);
	fp_close(fp);  /* Close swkbd map file */
	return(ENOMEM);
    }
    /*
      Read the swkbd map file
      */
    rc = fp_read(fp, lft_ptr->swkbd, f_len, 0, UIO_SYSSPACE, &countp);
    if(rc || countp < f_len)	/* If error or # of bytes read is incorrect */
    {
	lfterr(NULL,"LFTDD", "lftswkbd", "fp_read", rc, LFT_FP_READ, UNIQUE_5);
	BUGLPR(db_lftswkbd, BUGNFO, ("lft_swkbd_init: fp_read error\n"));
	xmfree(lft_ptr->swkbd, pinned_heap);
	GS_EXIT_TRC1(HKWD_GS_LFT, lftswkbd, 1, lftswkbd, EBADF);
	fp_close(fp);  /* Close swkbd map file */
	return(EBADF);
    }
    fp_close(fp); /* Done with swkbd map file so close it */

    /*
      Now set up the diacritic tables and find the one we need to use
      for this software keyboard map
      */
    lftdiaclst  = &diactbl850;
    diactbl850.next   = &diactbl88591;
    diactbl88591.next = &diactbl88592;
    diactbl88592.next = &diactbl88593;
    diactbl88593.next = &diactbl88594;
    diactbl88594.next = &diactbl88597;
    diactbl88597.next = &diactbl88599;
    diactbl88599.next = NULL;
    
    /*
        This while loop implies that not all swkb map files will have a
 	diacritic table.   
      */
    while (lftdiaclst &&
           strncmp( lftdiaclst->code_set, lft_ptr->swkbd->disp_set, 8) != 0)
        lftdiaclst = lftdiaclst->next;
    lft_ptr->dds_ptr->kbd.diac = lftdiaclst;

    GS_EXIT_TRC0(HKWD_GS_LFT, lftswkbd, 1, lftswkbd);
    return(SUCCESS);
}
/* -------------------------------------------------------------------	*
 *  Name:               lft_swkbd_term					*
 *  Description:        Undo the swkbd allocation			*
 *  Parameters:								*
 *      input   none				       			*
 *									*
 *  Process:								*
 *	Free up space allocated for the swkbd map			*
 *  Returns:								*
 *      0 = Success							*
 *      Otherwise an error code is returned.				*
 * --------------------------------------------------------------------	*/

int
lft_swkbd_term()
{
    xmfree(lft_ptr->swkbd, pinned_heap);
    return(SUCCESS);
}
