/* @(#)31	1.1  src/bos/usr/include/unpack.h, cmdcrash, bos411, 9428A410j 4/26/91 00:29:17 */

/*
 * COMPONENT_NAME: (inc) Include files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

ulong *unpack(int mem, int index, char *data_name, char *ubuf, ulong vad,
	      ulong len, int relative);
int name2cdt(int mem, char *name);
int read_memory(int mem, int rflag, void *buf, ulong vad, int len);
void pdbg_clist(int mem, int rflag, char *s, struct clist *cl);
void dbg_pstr(char *from, char *to, int bcnt);
