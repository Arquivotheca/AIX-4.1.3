/* @(#)14  1.3  src/bos/kernext/dlc/qlc/qlcoutputf.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:29:43 */
#ifndef _H_QLCOUTPUTF
#define _H_QLCOUTPUTf
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

extern void outputf(char *string,...);

extern void mustoutputf(char *string,...);

extern void print_stat_block (
  char *status_block_ptr);

extern void  print_x25_buffer (
  char *buf_pt);

extern void output_two_hex_digits(int count);

extern void  hex_data_list(
  char *data,
  int count);

extern void print_start_data (
  char *start_data_pt);

extern void print_halt_data (
  char *halt_data_pt);

extern void print_write_ext (
  char *write_ext_pt );

extern void qllc_state(void);

extern void print_cb_fac(char *fac_ptr);

extern void print_sna_fac(char *fac_ptr);

#endif



