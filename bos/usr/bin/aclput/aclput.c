static char sccsid[] = "@(#)62	1.14  src/bos/usr/bin/aclput/aclput.c, cmdsdac, bos411, 9428A410j 12/1/93 10:32:14";
/*
 * COMPONENT_NAME:  (CMDSDAC) security: access control
 *
 * FUNCTIONS:  aclput
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * aclput [-i infile] filename
 *
 * The aclput command sets the access control information of a file.
 * If no input file is specified (by the -i option), the access control
 * information is read from standard input.
 *
 * This information is expected in the form:
 *
 *	Attributes:  { SUID | SGID | SVTX }
 * 	Base Permissions
 *		Owner ('name'): rwx
 *		Group ('name'): r
 *		Others:
 * 	Extended Permissions
 *		{ Enabled | Disabled }
 *		Permit	rw	u:dhs
 *		Deny	w	u:nickc,g:gateway
 *		Specify	r	u:nickc,g:gateway
 *
 * Lines that begin with the character '*' are interpreted as comments
 *
 * If an error is detected in the input ACL the ACL is written (with
 * diagnostic comments) to the input file (if no input file was
 * specified with the -i flag, the ACL is written to stdout).
 *
 */


#include "stdio.h"
#include "locale.h"
#include "sys/acl.h"
#include "sys/access.h"
#include "sys/stat.h"
#include "sys/mode.h"
#include "sys/errno.h"
#include "pwd.h"
#include "grp.h"

#include "aclput_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ACLPUT,n,s) 

#define	DPRINTF(args)

extern	errno;

char	*inputfile;
FILE	*infile;
char	*errorfile = "/tmp/aclpXXXXXX";
char	*mktemp(char *);
FILE	*erf;
char	*filename;
struct acl	*aclbuf;

char	*token[128];
char	line[BUFSIZ];
int	aclbufsiz = BUFSIZ;
int	line_number = 0;
int	num_errors = 0;


main(argc, argv)
char	**argv;
{
	extern	int	optind;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_ACLPUT, NL_CAT_LOCALE);

	inputfile = NULL;
	infile = stdin;
	while (1)
	{
		int	c;
		extern char	*optarg;

		c = getopt(argc, argv, "i:");
		if (c == EOF)
			break;
		switch (c)
		{
			case 'i':
				inputfile = optarg;
				infile = fopen(inputfile, "r");
				if (infile == NULL)
				{
					perror(inputfile);
					exit(errno);
				}
				break;
			default:
			usage:
				fprintf(stderr, 
MSGSTR(USAGE, "Usage:  aclput [-i infile] filename\n"));
				exit(EINVAL);
		}
	}
	if (optind != (argc-1))
		goto usage;
	filename = argv[optind];

	if ((aclbuf = (struct acl *)malloc(aclbufsiz)) == NULL)
		abort("can't allocate buffer for ACL (size=%d)\n", aclbufsiz);

	aclbuf->acl_len = sizeof(struct acl) - sizeof(struct acl_entry);

	/* create error file */
	if ((mktemp(errorfile) == NULL)  ||
	    ((erf = fopen(errorfile, "w+")) == NULL))
	{
		fprintf(stderr,
MSGSTR(NOTMP, "Cannot create temp file\n"));
		exit(errno);
	}
	unlink(errorfile);
	
	readacl(infile);
	if (num_errors)
	{
		int	c;

		if (inputfile)
		{
			fclose(infile);
			if ((infile = fopen(inputfile, "w")) == NULL)
				abort(
MSGSTR(NOWRITE, "Cannot open %s for writing\n"), inputfile);
		}
		else
			infile = stdout;
		rewind(erf);
		while ((c = getc(erf)) != EOF)
			putc(c, infile);
		exit(-1);
	}

	if (chacl(filename, aclbuf, aclbuf->acl_len))
	{
		switch (errno) {
			case ENOSPC:
				fprintf (stderr,
MSGSTR (TOOBIG, "The size of the ACL exceeds the system limit.\n"));
				exit (ENOSPC);
			default:
				perror("aclput");
				exit(errno);
		}
	}

	exit(0);
}
		

int
readacl(inf)
FILE	*inf;
{
	/* to allow caching call setpwent() and setgrent() */
	setpwent();
	setgrent();

	/*
	 * order is not enforced
	 */
	while (readln(inf, line, sizeof(line)) == 0)
	{
		parseln(line, token);
		DTOKPRT(token);
		
/* attributes */
		if (tokcmp(token[0], "attributes") == 0)
		{
			DPRINTF(("found attribute line\n"));
			getattrs(token);
			continue;
		}

/* base permissions */
		if (tokcmp(token[0], "base") == 0)
		{
			DPRINTF(("found base permissions line\n"));
			continue;
		}

/* owner */
		if (tokcmp(token[0], "owner") == 0)
		{
			DPRINTF(("found owner line\n"));
			getowner(token);
			continue;
		}

/* group */
		if (tokcmp(token[0], "group") == 0)
		{
			DPRINTF(("found group line\n"));
			getgroup(token);
			continue;
		}

/* others */
		if (tokcmp(token[0], "others") == 0)
		{
			DPRINTF(("found others line\n"));
			getothers(token);
			continue;
		}

/* extended permissions */
		if (tokcmp(token[0], "extended") == 0)
		{
			DPRINTF(("found extended permissions line\n"));
			continue;
		}

/* enabled */
		if (tokcmp(token[0], "enabled") == 0)
		{
			DPRINTF(("found enabled line\n"));
			aclbuf->acl_mode |= S_IXACL;
			continue;
		}

/* disabled */
		if (tokcmp(token[0], "disabled") == 0)
		{
			DPRINTF(("found disabled line\n"));
			aclbuf->acl_mode &= ~S_IXACL;
			continue;
		}

/* permit */
		if (tokcmp(token[0], "permit") == 0)
		{
			DPRINTF(("found permit line\n"));
			getext(token, ACC_PERMIT);
			continue;
		}

/* deny */
		if (tokcmp(token[0], "deny") == 0)
		{
			DPRINTF(("found deny line\n"));
			getext(token, ACC_DENY);
			continue;
		}

/* specify */
		if (tokcmp(token[0], "specify") == 0)
		{
			DPRINTF(("found specify line\n"));
			getext(token, ACC_SPECIFY);
			continue;
		}

		err(
MSGSTR(UNKKEY, "unknown keyword: %s"), token[0]);
		DPRINTF(("found \"unknown keyword\"\n"));
	}

	DPRINTF(("num_errors = %d\n", num_errors));
}


getattrs(token)
char	**token;
{
	int	i;

	for (i=1; token[i]; i++)
	{
		DPRINTF(("getattrs(): i=%d\n", i));
		if (tokcmp(token[i], "SUID") == 0)
		{
			DPRINTF(("found SUID\n"));
			aclbuf->acl_mode |= S_ISUID;
			continue;
		}
		if (tokcmp(token[i], "SGID") == 0)
		{
			DPRINTF(("found SGID\n"));
			aclbuf->acl_mode |= S_ISGID;
			continue;
		}
		if (tokcmp(token[i], "SVTX") == 0)
		{
			DPRINTF(("found SVTX\n"));
			aclbuf->acl_mode |= S_ISVTX;
			continue;
		}
		DPRINTF(("unknown attribute\n"));
		err(
MSGSTR(UNKATTR, "unknown attribute: %s"), token[i]);
		break;
	}
}


/* get owner's permissions */
getowner(token)
char	**token;
{
	int	tmpmode;

	if (token[2] == NULL)
	{
		err(
MSGSTR(NOUPERM, "permissions for owner not found"), NULL);
		return(-1);
	}
	if ((tmpmode = atoperm(token[2])) < 0)
	{
		err(
MSGSTR(EXPPSET, "expected permission set"), NULL);
		return(-1);
	}
	aclbuf->u_access = tmpmode;
	return(0);
}


/* get group's permissions */
getgroup(token)
char	**token;
{
	int	tmpmode;

	if (token[2] == NULL)
	{
		err(
MSGSTR(NOGPERM, "permissions for group not found"), NULL);
		return(-1);
	}
	if ((tmpmode = atoperm(token[2])) < 0)
	{
		err(
MSGSTR(EXPPSET, "expected permission set"), NULL);
		return(-1);
	}
	aclbuf->g_access = tmpmode;
	return(0);
}


/* get other's permissions */
getothers(token)
char	**token;
{
	int	tmpmode;

	if (token[1] == NULL)
	{
		err(
MSGSTR(NOOPERM, "permissions for others not found"), NULL);
		return(-1);
	}
	if ((tmpmode = atoperm(token[1])) < 0)
	{
		err(
MSGSTR(EXPPSET, "expected permission set"), NULL);
		return(-1);
	}
	aclbuf->o_access = tmpmode;
	return(0);
}


/* get an extended entry */
getext(token, type)
char	**token;
int	type;
{
	struct acl_entry	*acep;
	struct ace_id		*idp;
	struct ace_id		*idend;
	int	numids;
	int	siz;
	int	mode;
	char	**currtok;

	if ((mode = atoperm(token[1])) < 0)
	{
		err(
MSGSTR(BADACC, "bad access mode: %s"), token[1]);
		return(-1);
	}

	if ((numids = countids(&token[2])) < 0)
	{
		err(
MSGSTR(ILLFORM, "ill formed type/ID pair"), NULL);
		return(-1);
	}
	if (numids==0)
	{
		err(
MSGSTR(NOIDENT, "no identifiers"), NULL);
		return(-1);
	}

	numids--;	/* acl_entry already has room for one ace_id */
	siz = (sizeof(struct ace_id)*numids + sizeof(struct acl_entry));

	/*
	 * if the size of the existing ACL + the size of the new entry
	 * exceed the buffer size, grow the buffer
	 */
	if ((aclbuf->acl_len + siz) > aclbufsiz)
		growacl(siz);

	acep = (struct acl_entry *)((char *)aclbuf + aclbuf->acl_len);
	aclbuf->acl_len += siz;
	acep->ace_len = siz;
	DPRINTF(("ace_len = %d\n", siz));
	acep->ace_type = type;
	acep->ace_access = mode;
	
	idend = id_last(acep);
	currtok = &token[2];

	for (idp=acep->ace_id; idp<idend; idp=id_nxt(idp))
	{
		if (tokcmp(*currtok, "u") == 0)
		{
			idp->id_type = ACEID_USER;
			idp->id_len = sizeof(struct ace_id);
			currtok++;
			if (getusr(*currtok, idp->id_data) < 0)
			{
				err(
MSGSTR(UNKUSER, "unknown user: %s"), *currtok);
				aclbuf->acl_len -= siz;
				return(-1);
			}
			currtok++;
		}
		else
			if (tokcmp(*currtok, "g") == 0)
			{
				currtok++;
				idp->id_type = ACEID_GROUP;
				idp->id_len = sizeof(struct ace_id);
				if (getgrp(*currtok, idp->id_data) < 0)
				{
					err(
MSGSTR(UNKGRP, "unknown group: %s"), *currtok);
					aclbuf->acl_len -= siz;
					return(-1);
				}
				currtok++;
			}
			else
			{
				aclbuf->acl_len -= siz;
				err(
MSGSTR(UNKID, "unknown ID type: %s"), *currtok);
				return(-1);
			}
	}
	return(0);
}


parseln(str, token)
char	*str;
char	**token;
{
	char	*p;
	int	i = 0;

	p = str;
	while (1)
	{
		for ( ; *p; p++)
			if ((*p != ' ')  &&
			    (*p != '\t') &&
			    (*p != ':')  &&
			    (*p != ',')  &&
			    (*p != '(')  &&
			    (*p != ')'))
				break;
		if ((*p == (char)NULL) || (*p == '\n'))
			break;

		token[i++] = p++;

		for ( ; *p; p++)
			if ((*p == ' ')  ||
			    (*p == '\t') ||
			    (*p == ':')  ||
			    (*p == ',')  ||
			    (*p == '(')  ||
			    (*p == ')'))
				break;
		if (*p == (char)NULL)
			break;
		*p++ = (char)NULL;
	}

	*p = (char)NULL;	/* last character most likely a newline */
	token[i] = NULL;
}
	
		
atoperm(str)
char	*str;
{
	int	perm = 0;

	if (str == NULL)
		return(-1);
	if (str[0] == 'r')
		perm |= 4;
	else
		if (str[0] != '-')
			return(-1);
	if (str[1] == 'w')
		perm |= 2;
	else
		if (str[1] != '-')
			return(-1);
	if (str[2] == 'x')
		perm |= 1;
	else
		if (str[2] != '-')
			return(-1);
	if (str[3] != 0)
		return(-1);
	return(perm);
}


readln(fp, buf, len)
FILE	*fp;
char	*buf;
int	len;
{
	char	*p;

	while (1)
	{
		if (fgets(buf, len, fp) == NULL)
			return(-1);
   		/* go past white space */
		for (p=buf; (*p==' ') || (*p=='\t'); p++);
		if ((*p == '*') || (*p == '\n') || (*p == '\0'))
			continue;
		line_number++;
		fputs(buf, erf);
		break;
	}
	p = buf+strlen(buf)-1;
	if (*p == '\n')
		*p = (char)NULL;
	return(0);
}


abort(str, arg)
char	*str;
int	arg;
{
	fprintf(stderr, str, arg);
	exit(errno);
}


tokcmp(s1, s2)
char	*s1;
char	*s2;
{
	if ((s1 == NULL) || (s2 == NULL))
		return(-1);

	while (*s1 && *s2)
		if (*s1++ != *s2++)
			return(-1);
	return((*s1 || *s2) ? -1 : 0);
}


err(str, arg)
char	*str;
int	arg;
{
	char	buf[128];

	num_errors++;
	sprintf(buf, str, arg);
	fprintf(erf,
MSGSTR(LINENO, "* line number %d: %s\n"), line_number++, buf);
}


countids(token)
char	**token;
{
	int	i = 0;
	int	rc;

	while (token[i])
		i++;

	if (i&1)	/* odd number of tokens */
		rc = -1;
	else
		rc = i>>1;

	DPRINTF(("countids(): %d\n", rc));
	return(rc);
}


growacl(siz)
int	siz;
{
	char	*newaclbuf;

	aclbufsiz = (aclbufsiz+siz)*2;

	if ((newaclbuf = (char *)malloc(aclbufsiz)) == 0)
		abort(
MSGSTR(CANTMALL, "Cannot malloc new ACL buffer\n"));

	bcopy(aclbuf, newaclbuf, aclbuf->acl_len);
	free(aclbuf);
	aclbuf = (struct acl *)newaclbuf;
}


getusr(name, data)
char	*name;
char	*data;
{
	struct passwd	*pwdp;
	uid_t	tmp;

	if (isnumber(name))
	{
		tmp = atoi(name);
		bcopy(&tmp, data, sizeof(tmp));
		return(0);
	}
	if ((pwdp = getpwnam(name)) == NULL)
		return(-1);
	tmp = pwdp->pw_uid;
	bcopy(&tmp, data, sizeof(tmp));
	return(0);
}


getgrp(name, data)
char	*name;
char	*data;
{
	struct group	*grpp;
	gid_t	tmp;

	if (isnumber(name))
	{
		tmp = atoi(name);
		bcopy(&tmp, data, sizeof(tmp));
		return(0);
	}
	if ((grpp = getgrnam(name)) == NULL)
		return(-1);
	tmp = grpp->gr_gid;
	bcopy(&tmp, data, sizeof(tmp));
	return(0);
}


DTOKPRT(token)
char	**token;
{
	int	i;

	for (i=0; token[i]; i++)
	{
		DPRINTF(("token[%d] = \"%s\"  ", i, token[i]));
	}
	DPRINTF(("\n"));
}


isnumber(s)
char	*s;
{
	char	*p;

	for (p=s; *p; p++)
		if ((*p < '0') || (*p > '9'))
			return(0);
	if (p != s)
		return(1);
	return(0);
}
