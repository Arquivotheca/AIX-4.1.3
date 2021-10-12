/* @(#)50  1.3.1.1  src/bos/kernext/ientdiag/i_cioddlo.h, diagddient, bos411, 9428A410j 11/10/93 14:09:28 */
#ifndef _H_CIODDLO
#define _H_CIODDLO

/*
 * COMPONENT_NAME: sysxcio -- Common Communications Code Device Driver Head
 *
 * FUNCTIONS: cioddlo.h
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
/* prototypes of functions provided in ciodd.c                               */
/*****************************************************************************/
/* old-style declaration due to variable number of arguments */
extern void cio_save_trace ();

/* unlink first element of list and return pointer to that element */
extern sll_elem_ptr_t sll_unlink_first (s_link_list_t *ll_ptr);

/* report connection complete to all waiting starters */
extern void cio_conn_done (dds_t *dds_ptr);

/* report status to user */
extern void cio_report_status (dds_t          *dds_ptr,
                               open_elem_t    *open_ptr,
                               cio_stat_blk_t *stat_blk_ptr);

/* final processing for a valid received packet */
extern void cio_proc_recv (dds_t       *dds_ptr,
                           open_elem_t *open_ptr,
                           rec_elem_t  *rec_ptr);

/* final processing after a transmit complete */
extern void cio_xmit_done (dds_t      *dds_ptr,
                           xmt_elem_t *xmt_ptr,
                           ulong       status,
                           ulong       stat2);

/* add an entry to the component dump table */
extern void cio_add_cdt (char *name,
                         char *ptr,
                         int   len);

/* delete an entry from the component dump table */
extern void cio_del_cdt (char *name,
                         char *ptr,
                         int   len);

#ifdef DEBUG
extern void cio_hex_dump (uchar *dump_ptr,
                          ulong  dump_offset,
                          ulong  bytes_to_dump,
                          ulong  label_value);
#endif /* DEBUG */

#endif /* ! _H_CIODDLO */
