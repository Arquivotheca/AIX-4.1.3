/* @(#)12  1.11  src/bos/kernext/psla/gswincl.h, sysxpsla, bos41J, bai15 4/12/95 12:42:47 */
/* @(#)12	1.9  10/12/93 10:26:03 */
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/* PURPOSE:     Device driver include files                             */
/*                                                                      */
/*bb 04/09/90   Add except.h                                            */
/*fp 03/08/95   JCC marks 3.2 to 4.1 port                               */
/************************************************************************/

/* INCLUDE FILES */

#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#define Bool unsigned

#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/signal.h>
#include <sys/intr.h>
#include <sys/pri.h>
#include <sys/ioctl.h>
#include <sys/dma.h>
#include <sys/xmem.h>

#include "gswcb.h"
#include <sys/types.h>
#include <sys/except.h>

/* Added - JCC */
#include <sys/lock_alloc.h>
#include <sys/lock_def.h>
