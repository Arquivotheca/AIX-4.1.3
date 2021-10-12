/* @(#)63	1.5  src/bos/kernext/scsi/hscxdec.h, sysxscsi, bos411, 9428A410j 1/24/94 11:22:16 */
#ifndef _H_HSCXDEC
#define _H_HSCXDEC
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        hscxdec.h                                               */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Source File                     */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

/*	external declarations	*/

extern struct adapter_def *adapter_ptrs[MAXADAPTERS];
extern ulong inited_adapters;
extern ulong opened_adapters;
extern ulong num_adap_pages;
extern ulong num_totl_pages;
extern struct intr epow_struct;
extern lock_t hsc_lock;
extern Simple_lock hsc_mp_lock;
extern Simple_lock hsc_ioctl_scbuf_lock;
extern Simple_lock hsc_epow_lock;
extern int hsc_debug;
extern int hsc_error;
extern struct hsc_cdt_tab *hsc_cdt;
#ifdef HSC_TRACE
extern int hsc_trace;
extern struct trace_element hsc_trace_tab[TRACE_ENTRIES];
extern struct trace_element *hsc_trace_ptr;
#endif HSC_TRACE

#endif /* _H_HSCXDEC */
