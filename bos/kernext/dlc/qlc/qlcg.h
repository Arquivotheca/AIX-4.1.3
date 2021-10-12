/* @(#)53	1.8  src/bos/kernext/dlc/qlc/qlcg.h, sysxdlcq, bos411, 9428A410j 11/12/93 16:46:30 */
#ifndef _H_QLCG
#define _H_QLCG
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sysmacros.h> 
#include <sys/errno.h>     
#include <sys/types.h>
#include <sys/proc.h>       
#include <sys/lockl.h>   
#include <sys/devinfo.h>
#include <sys/device.h> 
#include <sys/user.h>    
#include <sys/comio.h>   
#include <sys/sleep.h>  
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <sys/fp_io.h>
#include <sys/fullstat.h>
#include <sys/pin.h>
#include <sys/gdlextcb.h>
#include <sys/watchdog.h>
#include <sys/err_rec.h>
#include <sys/trchkid.h>

#include <x25/jsmalloc.h>  
#include <x25/jsdefs.h>      
#include <x25/jsfac.h>       

#include "qlcoutputf.h"
#include <sys/qlcextcb.h>
#include <sys/qllcerr.h>
#include "qlcerrlog.h"

/*****************************************************************************/
/* Define Constants used in QLLC                                             */
/*****************************************************************************/
#define DIAG_TAG_LENGTH                16
#define X25_ADDRESS_LENGTH             20
#define TEST_STRING_LENGTH            255      
#define MAX_ASCII_X25_ADDRESS_LENGTH   20
#define TX_DATA_OFFSET                  4
#define WORD                            4
#define DTE_GENERATED                   0
#define QLLC_CUD_80                  0xC3
#define QLLC_CUD_84                  0xCB
#define QLLC_CUD_LENGTH                 1

/*****************************************************************************/
/* Increment RAS counter unless it would wrap round                          */
/*****************************************************************************/
#define INC_RAS_COUNTER(ctr)    if ((ctr+1)>(ctr)) (ctr)++

/*****************************************************************************/
/* QLLC typedefs.                                                            */
/*****************************************************************************/
typedef byte             diag_code_type;
typedef unsigned int     channel_reference_type;
typedef unsigned char    boolean;
typedef char             diag_tag_type[DLC_MAX_DIAG];
typedef unsigned long    correlator_type;
typedef unsigned int     ras_counter_type;
typedef unsigned int     trace_channel_type;
typedef short int        lcn_type;
typedef int           (* function_address_type)();
typedef unsigned int     address_type;


#endif
