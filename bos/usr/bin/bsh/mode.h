/* @(#)07	1.17  src/bos/usr/bin/bsh/mode.h, cmdbsh, bos411, 9428A410j 9/1/93 17:33:37 */
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

typedef short BOOL;

#define BYTESPERWORD	(sizeof (char *))
#define	NIL	((char*)0)


/* the following nonsense is required
 * because casts turn an Lvalue
 * into an Rvalue so two cheats
 * are necessary, one for each context.
 */
		/*	PTM 23051
		*	remove Lcheat because lint does not like it
		*	it is not used in the code
union { int _cheat;};
#define Lcheat(a)	((a)._cheat)
		*/
#define Rcheat(a)	((long)(a))


/* address puns for storage allocation */
typedef union
{
	struct forknod	*_forkptr;
	struct comnod	*_comptr;
	struct fndnod	*_fndptr;
	struct parnod	*_parptr;
	struct ifnod	*_ifptr;
	struct whnod	*_whptr;
	struct fornod	*_forptr;
	struct lstnod	*_lstptr;
	struct blk	*_blkptr;
	struct namnod	*_namptr;
	uchar_t	*_bytptr;
} address;


/* heap storage */
struct blk
{
	struct blk	*word;
};

#define	SH_BUFSIZ	128
struct fileblk
{
	int	fdes;
	unsigned flin;
	BOOL	sh_feof;
	BOOL    fraw;
	uchar_t	fsiz;
	uchar_t	*fnxt;
	uchar_t	*fend;
	uchar_t	**feval;
	struct fileblk	*fstak;
	uchar_t	fbuf[SH_BUFSIZ];
};

struct tempblk
{
	int fdes;
	struct tempblk *fstak;
};


/* for files not used with file descriptors */
struct fileheader
{
	int	fdes;
	unsigned	flin;
	BOOL	sh_feof;
	uchar_t	fsiz;
	uchar_t	*fnxt;
	uchar_t	*fend;
	uchar_t	**feval;
	struct fileblk	*fstak;
	uchar_t	_fbuf[1];
};

struct sysnod
{
	char	*sysnam;
	int	sysval;
};

/* this node is a proforma for those that follow */
struct trenod
{
	int	tretyp;
	struct ionod	*treio;
};

/* dummy for access only */
struct argnod
{
	struct argnod	*argnxt;
	uchar_t	argval[1];
};

struct dolnod
{
	struct dolnod	*dolnxt;
	int	doluse;
	uchar_t	*dolarg[1];
};

struct forknod
{
	int	forktyp;
	struct ionod	*forkio;
	struct trenod	*forktre;
};

struct comnod
{
	int	comtyp;
	struct ionod	*comio;
	struct argnod	*comarg;
	struct argnod	*comset;
};

struct fndnod
{
	int 	fndtyp;
	uchar_t	*fndnam;
	struct trenod	*fndval;
};

struct ifnod
{
	int	iftyp;
	struct trenod	*iftre;
	struct trenod	*thtre;
	struct trenod	*eltre;
};

struct whnod
{
	int	whtyp;
	struct trenod	*whtre;
	struct trenod	*dotre;
};

struct fornod
{
	int	fortyp;
	struct trenod	*fortre;
	uchar_t	*fornam;
	struct comnod	*forlst;
};

struct swnod
{
	int	swtyp;
	uchar_t *swarg;
	struct regnod	*swlst;
};

struct regnod
{
	struct argnod	*regptr;
	struct trenod	*regcom;
	struct regnod	*regnxt;
};

struct parnod
{
	int	partyp;
	struct trenod	*partre;
};

struct lstnod
{
	int	lsttyp;
	struct trenod	*lstlef;
	struct trenod	*lstrit;
};

struct ionod
{
	int	iofile;
	uchar_t	*ioname;
	uchar_t	*iolink;
	struct ionod	*ionxt;
	struct ionod	*iolst;
};

struct fdsave
{
	int org_fd;
	int dup_fd;
};


	/*	ths following structure is used to save a
	 *	copy of the current positional parameters.
	*/
struct dolsave
{
	struct dolnod	*s_dolh;
	uchar_t		**s_dolv;
	int		s_dolc;
};


#define		fndptr(x)	((struct fndnod *)x)
#define		comptr(x)	((struct comnod *)x)
#define		forkptr(x)	((struct forknod *)x)
#define		parptr(x)	((struct parnod *)x)
#define		lstptr(x)	((struct lstnod *)x)
#define		forptr(x)	((struct fornod *)x)
#define		whptr(x)	((struct whnod *)x)
#define		ifptr(x)	((struct ifnod *)x)
#define		swptr(x)	((struct swnod *)x)
