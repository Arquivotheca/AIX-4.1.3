/* @(#)56	1.4  src/bos/kernext/dlc/qlc/qlch.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:03:02 */
#ifndef _H_QLCH
#define _H_QLCH
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
/* The following pair of #defines are for coverage testing.                  */
/*****************************************************************************/
#define QLC_COVQUERY (100)
#define QLC_COVDUMP  (101)
#define QLC_DEBUG    (102)
#define QLC_GETTRACE (103)
/*****************************************************************************/
/* Define types needed for Device Head                                       */
/*****************************************************************************/
union ioctl_ext_type
{
  struct dlc_esap_arg enable_sap;
  struct dlc_corr_arg disable_sap;
  struct qlc_sls_arg start_ls;                    /* note qllc specific type */
  struct dlc_corr_arg halt_ls;
  struct qlc_qsap_arg query_sap;                  /* note qllc specific type */
  struct dlc_qls_arg query_ls;
  struct dlc_alter_arg alter;
  struct dlc_trace_arg trace;
  struct dlc_corr_arg contact;
  struct dlc_corr_arg test;
  struct dlc_corr_arg enter_local_busy;
  struct dlc_corr_arg exit_local_busy;
  struct devinfo device_info;
};
typedef union ioctl_ext_type ioctl_ext_type;

/*****************************************************************************/
/* The qlch.c module provides the follwoing functions.                       */
/*****************************************************************************/

/* Start of declarations for qlch.c                                          */
#ifdef _NO_PROTO
int qlcconfig();
int qlcmpx();
int qlcopen();
int qlcclose();
int qlcread();
int qlcwrite();
int qlcioctl();
int qlcselect();
void qllc_state();

#else

extern int qlcconfig(
  dev_t  devno,
  int    op,
  struct uio *uio_ptr);

extern int  qlcmpx(
  dev_t  dev,
  int *channel_id,
  char *port_name);

extern int qlcopen(
  dev_t devno,
  unsigned long devflag,
  int   chan,
  int   ext);

extern int qlcclose(
  dev_t devno,
  int chan,
  int ext);

extern int  qlcread(
  dev_t devno,
  struct uio *uiop,
  int chan,
  int ext);

extern int  qlcwrite (
  dev_t devno,
  struct uio *uiop,
  int chan,
  int ext);

extern int  qlcioctl(
  dev_t devno,
  int  op,
  int arg,
  unsigned long devflag,
  int chan,
  int ext);

extern int  qlcselect(
  dev_t  devno,
  unsigned short  events,
  unsigned short  *reventp,
  int chan);

extern void qllc_state(void);

#endif /* _NO_PROTO */
/* End of declarations for qlch.c                                            */


#endif

