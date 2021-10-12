/* @(#)61       1.3  src/bos/diag/tu/siosl/tabletdev/tab_str.h, tu_siosl, bos411, 9428A410j 3/9/94 12:17:21 */
/*
 * COMPONENT_NAME: TU_SIOSL 
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TAB_STR
#define _H_TAB_STR


struct  kbd_data {
	long loopmode;    /* TRUE= in loop mode */
	long num_loops;
	long num_errs;
	long ex_mode;     /* TRUE = advanced mode */
	long console;};   /* TRUE = console is not exist */

struct  kbd_tu_data {
	struct tucb_t  header;    /* TU header data block */
	struct kbd_data addata;}; /* keyboard TU data block */

struct  tab_tu_data {
	struct tucb_t header;    /* TU header data block */
	struct kbd_data addata;}; /* tablet TU data block */

struct  read_write {      /* store I/O call results */
	uchar flag;      /* 1 if I/O call fails, 0 if I/O call success */
	uchar result; }; /* data from adapter or error code */

struct  poll {            /* store poll call results */
	uchar flag;      /* 1 if poll call fails, 0 if poll call success */
	uchar interrupt; /* interrupt level */
	uchar data; };   /* data from adapter */

#endif /* _H_TAB_STR */
