/* @(#)20	1.1  src/bos/kernext/dlpi/include.h, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:27  */
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * include.h - just include the world, it's easier
 */

/* generic */
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/syspest.h>

/* cdli specific */
#include <sys/mbuf.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <sys/ndd.h>

/* mutex */
#include <net/spl.h>
#include <net/netisr.h>

/* streams */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>

/* streams driver config only */
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/strconf.h>

/* this driver */
#include <sys/dlpi.h>
#include <sys/dlpistats.h>
#include <sys/dlpi_aix.h>
#include "llc.h"
#include "driver.h"
