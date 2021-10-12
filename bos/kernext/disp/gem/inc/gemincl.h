/* @(#)26       1.11.1.10  src/bos/kernext/disp/gem/inc/gemincl.h, sysxdispgem, bos411, 9428A410j 4/15/94 18:15:15 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		
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


#define Bool       unsigned

#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/device.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <sys/display.h>
#include <sys/aixfont.h>
#include <sys/adspace.h>
#include <sys/errids.h>
#include <sys/malloc.h>
#include <sys/dma.h>
#include <unistd.h>
#include <vt.h>

#include "gem_def.h"
#include "gemras.h"
#include "gem_ddf.h"
#include "gem_cb.h"
#include "gmshr.h"
#include "gem_ldat.h"
#include "gem_mac.h"
#include "gem_ddmac.h"
#include "gem_wfifo.h"
#include "gem_client_clip.h"
