/* @(#)64  1.4  src/bos/kernext/dlc/qlc/qlcltime.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:06:33 */
#ifndef _H_QLCLTIME
#define _H_QLCLTIME
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
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* Define the types required by the Timer Procedures                         */
/*****************************************************************************/
enum timer_name_type
{
  halt_timer,
  inact_timer,
  repoll_timer,
  retry_timer
};
typedef enum timer_name_type timer_name_type;

/* Start of declarations for qlcltime.c                                      */
#ifdef _NO_PROTO
void qlm_qllc_repoll_timeout();
void qlm_ls_contacted();
void qlm_halt_timeout();
void qlm_inactivity_timeout();

#else

extern void qlm_repoll_timeout(
  struct watchdog *w);

extern void qlm_service_repolls(
  port_type *port_id);

extern void qlm_ls_contacted(
  correlator_type correlator);

extern void qlm_halt_timeout(
  struct watchdog *w);

extern void qlm_service_halt_timeouts(
  port_type *port_id);

extern void qlm_inactivity_timeout(
  struct watchdog *w);

extern void qlm_service_inactivity_timeouts(
  port_type *port_id);

extern void qlm_retry_timeout(
  struct watchdog *w);

extern void qlm_init_watchdog(
  struct watchdog  *w,
  void            (*func)(),
  unsigned int      dur);


#endif /* _NO_PROTO */
/* End of declarations for qlcltime.c                                        */

#endif

