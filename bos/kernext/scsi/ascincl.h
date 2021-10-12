/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Device Driver
 *
 * FUNCTIONS:	NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define	_KERNSYS		/* Turn on defines to get platform info */
#define	_RS6K_SMP_MCA
#include <sys/systemcfg.h>
#undef	_RS6K_SMP_MCA
#undef	_KERNSYS 
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/priv.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/time.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/m_except.h>
#include <sys/m_param.h>
#include <sys/scsi.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ascsidd.h>
#include <sys/scsi_scb.h>
