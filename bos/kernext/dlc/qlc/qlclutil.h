/* @(#)66  1.7  src/bos/kernext/dlc/qlc/qlclutil.h, sysxdlcq, bos411, 9437B411a 9/14/94 11:29:03 */
#ifndef _H_QLCLUTIL
#define _H_QLCLUTIL
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

/* Start of declarations for qlclutil.c                                      */
#ifdef _NO_PROTO
unsigned short        qlm_select_netid();
struct station_type  *qlm_find_ls_anywhere();
struct station_type  *qlm_find_ls_in_channel();
struct station_type  *qlm_find_ls_in_sap();
struct station_type  *qlm_find_ls_given_netid();
struct station_type  *qlm_find_ls_in_sap_by_corr();
struct station_type  *qlm_find_ls_in_channel_by_corr();
struct station_type  *qlm_find_ls_given_correlator();
struct station_type  *qlm_find_contending_ls();
struct station_type  *qlm_find_listening_ls();
struct station_type  *qlm_remote_stn_id_is_valid();
lock_t                qlm_lock_list_and_ls();
void                  qlm_delete_station();
#else
extern unsigned short qlm_select_netid(void);

extern station_type *qlm_find_ls_anywhere(
  unsigned short netid,
  bool *unlock);

extern station_type *qlm_find_ls_in_channel(
  channel_type *channel,
  unsigned short netid,
  bool *unlock);

extern station_type *qlm_find_ls_in_sap(
  sap_type *sap,
  unsigned short netid,
  bool *unlock);

extern station_type *qlm_find_ls_given_netid(
  unsigned short  netid,
  bool *unlock);

extern  station_type *qlm_find_ls_in_channel_by_corr(
  channel_type *channel,
  correlator_type  correlator,
  bool *unlock);

extern  station_type *qlm_find_ls_in_sap_by_corr(
  sap_type *sap,
  correlator_type  correlator,
  bool *unlock);

extern station_type *qlm_find_ls_given_correlator(
  correlator_type  correlator,
  bool *unlock);

extern station_type *qlm_find_cont_ls_anywhere(
  char           *addr,
  char           *local,    /* defect 156503 */
  unsigned short  netid,
  bool *unlock);

extern station_type *qlm_find_cont_ls_in_channel(
  channel_type   *channel,
  char           *addr,
  char           *local,    /* defect 156503 */
  unsigned short  netid,
  bool *unlock);

extern station_type *qlm_find_cont_ls_in_sap(
  sap_type *sap,
  char     *addr,
  char     *local,          /* defect 156503 */
  unsigned short netid,
  bool *unlock);

extern station_type *qlm_find_contending_ls(
  sap_type        *sap_ptr,      /* defect 156503 */
  station_type    *station_ptr,
  char            *addr,
  bool *unlock);

extern station_type *qlm_find_listening_ls(
  port_type *port_id,
  bool *unlock);

extern bool qlm_remote_stn_id_is_valid(
  char *address,
  unsigned long length);

extern lock_t qlm_lock_list_and_ls(
  station_type *station_id);
  
extern void qlm_delete_station(
  station_type *station_id,
  correlator_type other_info,
  gen_buffer_type *buffer_ptr);

extern station_type *qlm_find_repoll_ls_in_channel(
  channel_type   *channel,
  bool *unlock);

extern station_type *qlm_find_repoll_ls_in_sap(
  sap_type *sap,
  bool *unlock);

extern station_type *qlm_find_repoll_ls_on_port(
  port_type *port_id,
  bool *unlock);

extern station_type *qlm_find_forced_ls_in_channel(
  channel_type   *channel,
  bool *unlock);

extern station_type *qlm_find_forced_ls_in_sap(
  sap_type *sap,
  bool *unlock);

extern station_type *qlm_find_forced_ls_on_port(
  port_type *port_id,
  bool *unlock);

extern station_type *qlm_find_inact_ls_in_channel(
  channel_type   *channel,
  bool *unlock);

extern station_type *qlm_find_inact_ls_in_sap(
  sap_type *sap,
  bool *unlock);

extern station_type *qlm_find_inact_ls_on_port(
  port_type *port_id,
  bool *unlock);

extern station_type *qlm_find_retry_ls_in_channel(
  channel_type   *channel,
  bool *unlock);

extern station_type *qlm_find_retry_ls_in_sap(
  sap_type *sap,
  bool *unlock);

extern station_type *qlm_find_retry_ls_on_port(
  port_type *port_id,
  bool *unlock);

#endif /* _NO_PROTO */
/* End of declarations for qlclutil.c                                        */

#endif

