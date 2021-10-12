static char sccsid[] = "@(#)02	1.22  src/bos/usr/bin/bsh/hashserv.c, cmdbsh, bos411, 9431A411a 7/28/94 19:38:18";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: pathlook zapentry  zaphash zapcd hashout hashpr set_dotpath
 *	      hash_func func_unhash hash_cmd what_is_path findpath chk_access
 *	      pr_path rtn_path argpath
 *
 * ORIGINS: 3, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.15  com/cmd/sh/sh/hashserv.c, bos320, 9142320i 10/16/91 09:54:00
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */


#include	"defs.h"
#include	"hash.h"
#include	<sys/access.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>

#define		EXECUTE		01

static uchar_t	cost;
static int	dotpath;
static int	multrel;
static struct entry	*relcmd = 0;
static uchar_t msgbuf[SH_BUFSIZ];

	uchar_t	*rtn_path();
static	int	argpath();
void	pr_path();

short
pathlook(com, flg, arg)
	uchar_t	*com;
	int		flg;
	register struct argnod	*arg;
{
	register uchar_t	*name = com;
	register ENTRY	*h;

	ENTRY		hentry;
	int		count = 0;
	int		i;
	int		pathset = 0;
	int		oldpath = 0;


	hentry.data = 0;

#ifdef NLSDEBUG
	if (!NLSisencoded(name)) debug("pathlook - not encoded",name);
	else debug("pathlook",name);
#endif

	if (any('/', name))
		return(COMMAND);

	h = hfind(name);

	if (h)
	{
		if (h->data & (BUILTIN | FUNCTION))
		{
			if (flg)
				h->hits++;
			return(h->data);
		}

		if (arg && (pathset = argpath(arg)))
			return(PATH_COMMAND);

		if ((h->data & DOT_COMMAND) == DOT_COMMAND)
		{
			if (multrel == 0 && hashdata(h->data) > dotpath)
				oldpath = hashdata(h->data);
			else
				oldpath = dotpath;

			h->data = 0;
			goto pathsrch;
		}

		if (h->data & (COMMAND | REL_COMMAND))
		{
			if (flg)
				h->hits++;
			return(h->data);
		}

		h->data = 0;
		h->cost = 0;
	}

	if (i = syslook(name, commands, no_commands))
	{
		hentry.data = (BUILTIN | i);
		count = 1;
	}
	else
	{
		if (arg && (pathset = argpath(arg)))
			return(PATH_COMMAND);
pathsrch:
			/* At this point name is encoded and the path is
			   not.  Decoding com here seems to be the best
			   short term fix, otherwise the code fails to work
			   when 1 byte NLS uchar_ts are in the PATH and cmd name.
			   Set it back right away. Note that after decoding,
			   name points to static buffer in NLSndecode().
			 */

			name = NLSndecode( com );
			count = findpath(name, oldpath);
			name = com;
			
	}

	if (count > 0)
	{
		if (h == 0)
		{
			hentry.cost = 0;
			hentry.key = make(name);
			h = henter(hentry);
		}

		if (h->data == 0)
		{
			if (count < dotpath)
				h->data = COMMAND | count;
			else
			{
				h->data = REL_COMMAND | count;
				h->next = relcmd;
				relcmd = h;
			}
		}


		h->hits = flg;
		h->cost += cost;
		return(h->data);
	}
	else 
	{
		return(-count);
	}
}
			

static void
zapentry(h)
	ENTRY *h;
{
	h->data &= HASHZAP;
}

void
zaphash()
{
	hscan(zapentry);
	relcmd = 0;
}

void 
zapcd()
{
	while (relcmd)
	{
		relcmd->data |= CDMARK;
		relcmd = relcmd->next;
	}
}


static void
hashout(h)
	ENTRY *h;
{
	sigchk();

	if (hashtype(h->data) == NOTFOUND)
		return;

	if (h->data & (BUILTIN | FUNCTION))
		return;

	prn_buff(h->hits);

	if (h->data & REL_COMMAND)
		prc_buff('*');


	prc_buff(TAB);
	prn_buff(h->cost);
	prc_buff(TAB);

	pr_path(h->key, hashdata(h->data));
	prc_buff(NL);
}

void
hashpr()
{
	prs_buff(MSGSTR(M_HPR, "hits\tcost\tcommand\n"));
	hscan(hashout);
}


void
set_dotpath()
{
	register uchar_t	*path;
	register int	cnt = 1;

	dotpath = 10000;
	path = getpath("");

	while (path && *path)
	{
		if (*path == '/')
			cnt++;
		else
		{
			if (dotpath == 10000)
				dotpath = cnt;
			else
			{
				multrel = 1;
				return;
			}
		}
	
		path = nextpath(path);
	}

	multrel = 0;
}

void
hash_func(name)
	uchar_t *name;
{
	ENTRY	*h;
	ENTRY	hentry;

	uchar_t nm[1000];
	NLSencode(name,nm,sizeof(nm));
	name = nm;

	h = hfind(name);

	if (h)
	{

		if (h->data & (BUILTIN | FUNCTION))
			return;
		else
			h->data = FUNCTION;
	}
	else
	{
		int i;

		if (i = syslook(name, commands, no_commands))
			hentry.data = (BUILTIN | i);
		else
			hentry.data = FUNCTION;

		hentry.key = make(name);
		hentry.cost = 0;
		hentry.hits = 0;
	
		henter(hentry);
	}
}

void
func_unhash(name)
	uchar_t *name;
{
	ENTRY 	*h;

	if( !NLSisencoded( name) ) {
	  uchar_t nm[1000];
	  NLSencode(name,nm,sizeof(nm));
	  h = hfind(nm);
	} else
	h = hfind(name);

	if (h && (h->data & FUNCTION))
		h->data = NOTFOUND;
}


short
hash_cmd(name)
	uchar_t *name;
{
	ENTRY	*h;

	if (any('/', name))
		return(COMMAND);

	h = hfind(name);

	if (h)
	{
		if (h->data & (BUILTIN | FUNCTION))
			return(h->data);
		else
			zapentry(h);
	}

	return(pathlook(name, 0, 0));
}

void
what_is_path(name)
	register uchar_t *name;
{
	register ENTRY	*h;
	int		cnt;
	short	hashval;

	h = hfind(name);

	if (h)
	{
		hashval = hashdata(h->data);

		switch (hashtype(h->data))
		{
			case BUILTIN: {
				sprintf((char *)msgbuf, MSGSTR(M_BUILT,
					"%s is a shell builtin\n"), 
					(char *)NLSndecode(name));
				prs_buff(msgbuf);
				return;
			}
			case FUNCTION:
			{
				struct namnod *n = lookup(NLSndecode(name));
				sprintf((char *)msgbuf, MSGSTR(M_FUNC, 
					"%s is a function\n"), 
					(char *)NLSndecode(name));
				prs_buff(msgbuf);
				prs_buff(NLSndecode(name));
				prs_buff("(){\n");
				prf(n->namenv);
				prs_buff("\n}\n");
				return;
			}
	
			case REL_COMMAND:
			{
				short hash;

				if ((h->data & DOT_COMMAND) == DOT_COMMAND)
				{
					hash = pathlook(name, 0, 0);
					if (hashtype(hash) == NOTFOUND)
					{
						exitval |= 1;
						sprintf((char *)msgbuf, MSGSTR(M_NOTF,
							"%s not found\n"), (char *)NLSndecode(name));
						prs_buff(msgbuf);
						return;
					}
					else
						hashval = hashdata(hash);
				}
			}

			case COMMAND: {
				char *temp;
				/* rtn_path() uses stack; msg retrieval could
				 * cause a malloc; problem if do both in
				 * same statement */
				temp = MSGSTR(M_ISH, "%s is hashed (%s)\n");
				sprintf ((char *)msgbuf, temp,
					(char *)NLSndecode(name),
					(char *)rtn_path(NLSndecode(name),
					hashval));
				prs_buff(msgbuf);
				return;
			}
		}
	}

	if (syslook(name, commands, no_commands))
	{
		sprintf((char *)msgbuf, (char *)MSGSTR(M_BUILT,
				"%s is a shell builtin\n"), (char *)NLSndecode(name));
		prs_buff(msgbuf);
		return;
	}

	NLSdecode(name);
	if ((cnt = findpath(name, 0)) > 0)
	{
		char *temp;
		/* rtn_path() uses stack; msg retrieval could
		 * cause a malloc; problem if do both in
		 * same statement */
		temp = MSGSTR(M_IS, "%s is %s\n");
		sprintf ((char *)msgbuf, temp, 
				(char *)name, (char *)rtn_path(NLSndecode(name), cnt));
		prs_buff(msgbuf);
	}
	else
	{
		exitval |= 1;
		sprintf((char *)msgbuf, MSGSTR(M_NOTF, 
				"%s not found \n"), (char *)NLSndecode(name));
		prs_buff(msgbuf);
	}
}

int
findpath(name, oldpath)
	register uchar_t *name;
	int oldpath;
{
	register uchar_t 	*path;
	register int	count = 1;

	uchar_t	*p;
	int	ok = 1;
	int 	e_code = 1;
	
	cost = 0;
	path = getpath(name);

	if (oldpath)
	{
		count = dotpath;
		while (--count)
			path = nextpath(path);

		if (oldpath > dotpath)
		{
			catpath(path, name);
			p = curstak();
			cost = 1;

			if ((ok = chk_access(p)) == 0)
				return(dotpath);
			else
				return(oldpath);
		}
		else 
			count = dotpath;
	}

	while (path)
	{
		path = catpath(path, name);
		cost++;
		p = curstak();

		if ((ok = chk_access(p)) == 0)
			break;
		else
			e_code = MAX(e_code, ok);

		count++;
	}

	return(ok ? -e_code : count);
}

int
chk_access(name)
	register uchar_t	*name;
{
	if (access(name, X_OK) == 0)
		return(0);

	return(errno == EACCES ? 3 : 1);
}

void
pr_path(name, count)
	uchar_t	*name;
	int count;
{
	prs_buff(rtn_path(name, count));
}


uchar_t *
rtn_path(name, count)
	register uchar_t   *name;
	int count;
{
	register uchar_t	*path;

	path = getpath(name);

	while (--count && path)
		path = nextpath(path, name);

	catpath(path, name);
	return(curstak());
}


static int
argpath(arg)
	register struct argnod	*arg;
{
	register uchar_t 	*s;
	register uchar_t	*start;

	while (arg)
	{
		s = arg->argval;
		start = s;
		if (s = scanset(s)) {  {
				*s = 0;

				if (eq(start, pathname))
				{
					*s = '=';
					return(1);
				}
				else
					*s = '=';
			}
		}
		arg = arg->argnxt;
	}

	return(0);
}

