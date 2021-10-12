static char sccsid[] = "@(#)39  1.1  src/bos/kernext/lft/streams/lftsicfg.c, lftdd, bos411, 9428A410j 10/29/93 14:51:54";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: lft_streams_init
 *		lft_streams_term
 *
 *   ORIGINS: 27, 83
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

/*------------

  This main line file contains the driver streams configuration routines:
     lft_streams_init   - initialize streams module
     lft_streams_term   - terminate streams module
  ------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/errno.h>          /* System error numbers */
#include <sys/termio.h>         /* Terminal input/output */
#include <sys/syspest.h>

/* Private includes
   ================
*/
#include <lft.h>
#include <sys/lft_ioctl.h>
#include <sys/inputdd.h>
#include <sys/display.h>
#include <graphics/gs_trace.h>
#include <lft_debug.h>

/* Streams and tty includes
   ========================
*/
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strconf.h>
#include <unistd.h>
#include <sys/ttydefaults.h>
#include <sys/str_tty.h>
#define INITLFT 1		/* to get the streams struct initialized */
#include <lftsi.h>              /* LFT streams information */

GS_MODULE(lftsicfg);
BUGVDEF(db_lftsicfg, 99);        /* define initial debug level */



/**********************************************************************
Function : lft_streams_init
Input parameters :
	    None
**********************************************************************/
int lft_streams_init ()
{
static strconf_t lft_conf;
int rc;
/* TRACE "Entering lft_streams_init" */
	GS_ENTER_TRC1(HKWD_GS_LFT,lftsicfg,1,lft_streams_init,lft_ptr);

	/* Initialize lft_conf as follow */
	lft_conf.sc_name=lft_ptr->dds_ptr->lft.devname;
	lft_conf.sc_str = &lftinfo;
	lft_conf.sc_open_style = STR_NEW_OPEN;
	lft_conf.sc_major = major(lft_ptr->dds_ptr->lft.devno);
	lft_conf.sc_sqlevel = 0;
 
	rc = str_install(STR_LOAD_DEV, &lft_conf);
	if(rc)
	    lfterr(NULL,"LFTDD", "lft_streams_init", "str_install", rc, LFT_STR_INSTALL, UNIQUE_1);

/* LFTERR TRACE "Leaving lft_streams_init" */
	GS_EXIT_TRC1(HKWD_GS_LFT,lftsicfg,1,lft_streams_init,rc);
	return (rc);
}

/**********************************************************************
Function : lft_streams_term
Input parameters :
	None
**********************************************************************/
int lft_streams_term ()
{
static strconf_t lft_conf;
int rc;
/* TRACE "Entering lft_streams_term" */
	GS_ENTER_TRC1(HKWD_GS_LFT,lftsicfg,1,lft_streams_term,lft_ptr);

	/* Initialize lft_conf as follow */
	
	lft_conf.sc_name=lft_ptr->dds_ptr->lft.devname;
	lft_conf.sc_str = &lftinfo;
	lft_conf.sc_open_style = STR_NEW_OPEN;
	lft_conf.sc_major = major(lft_ptr->dds_ptr->lft.devno);
	lft_conf.sc_sqlevel = 0;
 
	rc = str_install(STR_UNLOAD_DEV, &lft_conf);
	if(rc)
	    lfterr(NULL,"LFTDD", "lft_streams_term", "str_install", rc, LFT_STR_INSTALL, UNIQUE_2);

/* LFTERR TRACE "Leaving lft_streams_term" */
	GS_EXIT_TRC1(HKWD_GS_LFT,lftsicfg,1,lft_streams_term,rc);
	return (rc);
}
