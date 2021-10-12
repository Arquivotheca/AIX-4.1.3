/* @(#)98       1.1  src/bos/usr/lbin/tty/crash/unpack.h, cmdtty, bos411, 9428A410j 3/3/94 10:08:46 */
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

ulong *unpack(int mem, int index, char *data_name, char *ubuf, ulong vad,
	      ulong len, int relative);
int name2cdt(int mem, char *name);
int read_memory(int mem, int rflag, void *buf, ulong vad, int len);
void pdbg_clist(int mem, int rflag, char *s, struct clist *cl);
void dbg_pstr(char *from, char *to, int bcnt);
int tty_read_mem(ulong addr, void *buf, int size);


/* these are used by the crash programs to indicate to the
 * tty_read_mem() function where to read the private data.
 */
extern int dump_mem;
extern int readflag;
