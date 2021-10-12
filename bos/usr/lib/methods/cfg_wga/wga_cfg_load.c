static char sccsid[] = "@(#)76  1.4  src/bos/usr/lib/methods/cfg_wga/wga_cfg_load.c, wgadd, bos411, 9428A410j 3/1/94 12:18:52";
/*
 * COMPONENT_NAME: (WGADD) Whiteoak Graphics Adapter Device Driver
 *
 * FUNCTIONS:	dev_cfg_entry()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*==========================================================================
|
|  Define some useful constants
|
|==========================================================================*/


#define PASS 0
#define FAIL -1

#ifdef CFGDEBUG
#	 define STATIC
#else
#	define STATIC static
#endif

/*==========================================================================
|
|  System include files required by all cfg method main programs
|
|==========================================================================*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <errno.h>


#include <cf.h>

/*==========================================================================
|
|  Local debug routines are defined here       
|
|==========================================================================*/

#include "cfgdebug.h"






/*==========================================================================
|
|  Include files which describe the common character mode interface and
|  the video ROM scan interface
|
|==========================================================================*/

#define Boolean unsigned int		/* need by aixfont.h		*/
#define Bool    unsigned int		/* need by aixfont.h		*/

#include <sys/aixfont.h>
#include <sys/display.h>
#include <sys/dir.h>                    /* needed by ccm_dds.h          */
#include <sys/mdio.h>                   /* needed by cdd_macros.h       */
#include <sys/file.h>                   /* needed by ccm.h              */
#include <sys/intr.h>                   /* needed by vt.h               */

#include <sys/ioacc.h>		   /* needed by ccm_dds.h	  */
#include <sys/adspace.h>	   /* needed by ccm_dds.h	  */

#include "vt.h"

#include  "cdd.h"
#include  "cdd_macros.h"
#include  "cdd_intr.h"
#include  "cdd_intr_macros.h"
#include  "ccm.h"
#include  "ccm_macros.h"


/*==========================================================================
|
| include a file which defines the cfg macros used by our cfg methods
|
|==========================================================================*/

#include "cfg_graphics.h"
#include "cfg_graphics_macros.h"


/*---- from cfg_wga.o, the actual device specific code ----*/

extern	int		generate_minor( );
extern	int		build_dds( );
extern	int		query_vpd( );
extern	int		download_microcode( );


int
dev_cfg_entry(cfg_graphics_funcs_t * p_function)
{


/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	/*----------------------------------------------------
	| This routine references the functions in cfg_wga.o
	| and is responsible for putting the addresses of functions
	| uses in cfg_graphics.o into a structure passed in.   
	|-----------------------------------------------------*/


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*-------------------------------------------------
        |
        |  First test that the supplied pointer is valid
        |
        |----------------------------------------------------*/

        if  ( p_function == NULL )
        {
                /*-------------------------------------------
                | the p_function pointer is invalid
                |------------------------------------------*/

                return( E_ARGS );
        }

	/*-------------------------------------------
	| set up the function addresses in the structure
	|--------------------------------------------*/

	/*---- init the cfg_graphics_funcs_t function pointers ----*/

        CFG_gen_minor_dev_num(p_function) 	= generate_minor;
        CFG_build_dds(p_function)		= build_dds;	
        CFG_query_vpd(p_function)		= query_vpd;	
        CFG_download_ucode(p_function)		= download_microcode;

	return PASS;
}

