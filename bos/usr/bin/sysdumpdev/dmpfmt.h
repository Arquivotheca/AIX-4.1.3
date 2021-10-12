/* @(#)67    1.1  src/bos/usr/bin/sysdumpdev/dmpfmt.h, cmddump, bos411, 9428A410j  3/19/93  17:42:05 */

/*
 * COMPONENT_NAME: CMDDUMP   system dump control and formatting
 *
 * FUNCTIONS: header file for sysdumpdev and dmpfmt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern char *Filename;
extern int binaryflg;
extern int batchflg;
extern int listflg;
extern int allflg;
extern int da_idx;
extern int da_num[1000];
extern int Fd;

#define DA_NAMESIZE 8
#define C_NAMESIZE  16

struct da_table {
	char da_name[DA_NAMESIZE+1];
	int  da_ptr;
	int  da_len;
	int  da_bmoffset;
	int  da_offset;
};
extern struct da_table *da_tableinit();

struct c_table {
	char c_name[C_NAMESIZE+1];
	int  c_offset;
};
extern struct c_table *c_tableinit();

struct dmp_displayopts {
	unsigned opt_fieldsize; /* number of bytes in a field (1 - 16)   dflt=4  */
	unsigned opt_nfields;   /* number of fields in a line            dflt=4  */
	unsigned opt_align;     /* alignment of the start of a line      dflt=16 */
	unsigned opt_wrapwidth; /* width of screen.          dflt=0 (don't care) */
	unsigned opt_asciiflg;  /* display ascii equivalent on right     dflt=1  */
	unsigned opt_compactflg;/* don't display duplicate line like od  dflt=0  */
};

#define MCS_CATALOG "cmddump.cat"
#include "cmddump_msg.h"

#include <ras.h>

