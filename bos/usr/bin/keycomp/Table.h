/* @(#)46	1.1  src/bos/usr/bin/keycomp/Table.h, cmdimkc, bos411, 9428A410j 7/8/93 19:59:43 */
/*
 * COMPONENT_NAME : (cmdimkc) AIX Input Method Keymap Compiler
 *
 * FUNCTIONS : keycomp
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_Table_h
#define	_Table_h

typedef struct _ParseTable {
	unsigned int	token;
	unsigned int	val;
	unsigned char	str[64];
} ParseTable;

#endif	/* _Table_h */
