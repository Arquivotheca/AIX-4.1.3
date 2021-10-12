/* @(#)22       1.9.2.7  src/bos/kernext/disp/gem/inc/gem_ddf.h, sysxdispgem, bos411, 9428A410j 4/26/94 18:31:36 */
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


#include <sys/watchdog.h>

  struct gem_ddf
  {
    long cmd;
    caddr_t sleep_addr;
    int (*callback)();
    caddr_t callbackarg;
    int     callbackindx;
    ulong *diaginfo;
    ulong sync_event_mask;
    ulong async_event_mask;
    ulong  bufcnt;
    char  *bufptr;
    ulong immpid_flags;
    ulong travpid_flags;
    pid_t imm_pid;
    pid_t trav_pid;
    eventReport report;
    struct watch_imm
     {
	struct watchdog wi;
	pid_t imm;
	struct gem_ddf *ddf;
     } watch_imm;

    struct watch_trav
     {
	struct watchdog wt;
	pid_t trav;
	struct gem_ddf *ddf;
     } watch_trav;

    struct watch_event
     {
	struct watchdog w_event;
	caddr_t sleep_addr;
	struct gem_ddf *ddf;
     } watch_event;

    ulong sleep_flags;
    ulong adapter_ready;
    int    num_of_process;

  };


