/* @(#)67	1.5  src/bos/usr/bin/pax/func.h, cmdarch, bos412, 9446B 11/11/94 21:54:04 */
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: pax
 *
 * ORIGINS: 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 */

/* RCSfile Revision (OSF) Date */
/* $Source: /u/mark/src/pax/RCS/func.h,v $
 *
 * $Revision: 1.3 $
 *
 * func.h - function type and argument declarations
 *
 * DESCRIPTION
 *
 *	This file contains function delcarations in both ANSI style
 *	(function prototypes) and traditional style. 
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Mark H. Colburn and sponsored by The USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _PAX_FUNC_H
#define _PAX_FUNC_H

/* Function Prototypes */


extern Link    	       *linkfrom(char *, Stat *);
extern Link    	       *linkto(char *, Stat *);
extern char    	       *mem_get(uint);
extern char    	       *mem_str(char *);
extern int      	ar_read(void);
extern int      	buf_read(char *, uint);
extern int      	buf_skip(OFFSET);
extern int      	create_archive(void);
extern int      	dirneed(char *);
extern int      	read_archive(void);
extern int      	inentry(char *, Stat *);
extern int      	lineget(FILE *, char *);
extern int      	name_match(char *, int);
extern int      	name_next(char *, Stat *);
extern int      	nameopt(char *);
extern int      	open_archive(int);
extern int      	open_tty(void);
extern int      	openin(char *, Stat *);
extern int      	openout(char *, Stat *, Link *, int);
extern int      	pass(char *);
extern int      	read_header(char *, Stat *);
extern int      	wildmat(char *, char *);
extern void     	buf_allocate(OFFSET);
extern void     	close_archive(void);
extern void     	fatal(char *);
extern void     	name_gather(void);
extern void     	name_init(int, char **);
extern void     	names_notfound(void);
extern void     	next(int);
extern int      	nextask(char *, char *, int);
extern void     	outdata(int, char *, Stat *);
extern void     	outwrite(char *, uint);
extern void     	passdata(char *, int, char *, int);
extern void     	print_entry(char *, Stat *);
extern void     	warn();
extern void		warnarch(char *, OFFSET);
extern void     	write_eot(void);
extern struct group    *getgrgid(gid_t);
extern struct group    *getgrnam(char *);
extern struct passwd   *getpwuid(uid_t);
extern SIG_T	      (*signal(int, void (*)(int)))(int);
extern Link            *islink(char *, Stat *);
extern char            *finduname(int);
extern char            *findgname(int);
extern time_t 	       	hash_lookup(char *);
extern void 		hash_name(char *, Stat *);
extern int		charmap_convert(char *);
extern int		get_header(char *, Stat *);
extern int		dirmake(char *);
extern OFFSET		pax_optsize(char *);
extern void		parse_opts(char *);

#endif /* _PAX_FUNC_H */
