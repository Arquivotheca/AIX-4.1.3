static char sccsid[] = "@(#)61  1.16  src/bos/usr/bin/aclget/aclget.c, cmdsdac, bos411, 9428A410j 2/2/94 13:22:03";
/*
 * COMPONENT_NAME:  (CMDSDAC) security: access control
 *
 * FUNCTIONS:  aclget
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * aclget [-o outfile] filename
 *
 * The aclget command retrieves the access control information of a file.
 * If no output file is specified (by the -o option), the access control
 * information is written to standard output.
 *
 * This information is written in the form:
 *
 *	attributes:  { SUID | SGID | SVTX }
 * 	base Permissions
 *		owner ('name'): rwx
 *		group ('name'): r
 *		others:
 * 	extended permissions
 *		{ enabled | disabled }
 *		permit	rw	u:dhs
 *		deny	w	u:nickc,g:gateway
 *		specify	r	u:nickc,g:gateway
 */


#include <stdio.h>
#include <locale.h>
#include "sys/acl.h"
#include "sys/access.h"
#include "sys/stat.h"
#include "sys/mode.h"
#include <sys/errno.h>
#include <pwd.h>
#include <grp.h>

#include "aclget_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ACLGET,n,s) 

#define DPRINTF(args)

#define	ACL_SIZE	128

extern	errno;
char	*prtmode(int mode);

FILE	*outfile;
char	*filename;
struct acl	*aclbuf;

void prtusr(FILE *fp, char *data);
void prtgrp(FILE *fp, char *data);
void prtwhat(FILE *fp, char *data, int len);


int
main(int	argc, 
     char	**argv)
{
	struct acl	*aclbuf;
	struct acl_entry	*acep;
	struct acl_entry	*aclend;
	struct stat	sbuf;
	struct passwd	*pwdp;
	struct group	*grpp;
	extern int	optind;
	int	idcomma;
	int	aclsize;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_ACLGET, NL_CAT_LOCALE);

	outfile = stdout;
	while (1)
	{
		int	c;
		extern	char	*optarg;

		c = getopt(argc, argv, "o:");
		if (c == EOF)
			break;
		switch (c)
		{
			case 'o':
				outfile = fopen(optarg, "w");
				if (outfile == NULL)
				{
					perror(optarg);
					exit(errno);
				}
				break;
			default:
			usage:
				fprintf(stderr,
MSGSTR(USAGE, "Usage: aclget [-o outfile] filename\n"));
				exit(EINVAL);
		}
	}
	if (optind != (argc-1))
		goto usage;

	filename = argv[optind];

	if (stat(filename, &sbuf) < 0)
	{
		fprintf(stderr,
MSGSTR(NOFIND, "Cannot find %s\n"), filename);
		exit(errno);
	}

	aclsize = BUFSIZ;
	while (1)
	{
		if ((aclbuf = (struct acl *)malloc(aclsize)) == NULL)
		{
			fprintf(stderr,
MSGSTR(NOBUF, "Cannot allocate buffer for ACL\n"));
			exit(errno);
		}
		if (statacl(filename, 0, aclbuf, aclsize) != -1)
			break;
		if (errno != ENOSPC)
		{
			perror("aclget:");
			exit(errno);
		}
		aclsize = aclbuf->acl_len;
		DPRINTF(("---- acl buffer len = %d ----\n", aclsize));
		free(aclbuf);
	}

	fprintf(outfile, "attributes: ");
	if (aclbuf->acl_mode & S_ISUID)
		fprintf(outfile, "SUID ");
	if (aclbuf->acl_mode & S_ISGID)
		fprintf(outfile, "SGID ");
	if (aclbuf->acl_mode & S_ISVTX)
		fprintf(outfile, "SVTX ");
	fprintf(outfile, "\n");

	fprintf(outfile, "base permissions\n");

	/*
 	 * to allow cached accesses we do a setpwent() and setgrent() here
 	 */
	setpwent();
	setgrent();

	fprintf(outfile, "    owner");
	if ((pwdp = getpwuid(sbuf.st_uid)) == NULL)
		fprintf(outfile, "(%d):", sbuf.st_uid);
	else
		fprintf(outfile, "(%s):", pwdp->pw_name);

	fprintf(outfile, "  %s\n", prtmode(aclbuf->u_access));

	fprintf(outfile, "    group");
	if ((grpp = getgrgid(sbuf.st_gid)) == NULL)
		fprintf(outfile, "(%d):", sbuf.st_gid);
	else
		fprintf(outfile, "(%s):", grpp->gr_name);
	fprintf(outfile, "  %s\n", prtmode(aclbuf->g_access));

	fprintf(outfile, "    others:");
	fprintf(outfile, "  %s\n", prtmode(aclbuf->o_access));

	fprintf(outfile, "extended permissions\n");
	fprintf(outfile, "    %s\n", (aclbuf->acl_mode & S_IXACL) ? 
						"enabled" : "disabled");

	acep = aclbuf->acl_ext;
	aclend = acl_last(aclbuf);
	DPRINTF(("acep=0x%x aclend=0x%x\n", acep, aclend));

/* redundant check (with for() loop) */
	if (acep >= aclend)
		exit(0);
	for ( ; acep < aclend; acep=acl_nxt(acep))
	{
		struct ace_id	*idp;
		struct ace_id	*idend;

		DPRINTF(("iterate\n"));

		switch (acep->ace_type)
		{
		    case ACC_PERMIT:
			fprintf(outfile, "    permit   ");
			break;
		    case ACC_DENY:
			fprintf(outfile, "    deny     ");
			break;
		    case ACC_SPECIFY:
			fprintf(outfile, "    specify  ");
			break;
		    default:
			fprintf(outfile, "    ???     ");
		}

		fprintf(outfile, "%s     ", prtmode(acep->ace_access));
		idend = id_last(acep);
		idcomma=0;
		for (idp=acep->ace_id; idp<idend; idp=id_nxt(idp))
		{
			if (idcomma)
				fprintf(outfile, ",");
			else
				idcomma = 1;

	 		switch (idp->id_type)
			{
			    case ACEID_USER:
				fprintf(outfile, "u:");
				prtusr(outfile,(char *) idp->id_data);
				break;
			    case ACEID_GROUP:
				fprintf(outfile, "g:");
				prtgrp(outfile,(char *) idp->id_data);
				break;
			    default:
				fprintf(outfile, "?:");
				prtwhat(outfile, (char *) idp->id_data, idp->id_len);
				}
		}
		fprintf(outfile, "\n");
	}
	
	exit(0);
}


/* print out information for user */
void
prtusr(FILE	*fp, 
       char	*data)
{
	uid_t	tmpuser;
	struct passwd	*pwdp;

	bcopy(data, &tmpuser, sizeof(uid_t));
	if ((pwdp = getpwuid(tmpuser)) == NULL)
		fprintf(fp, "%d", tmpuser);
	else
		fprintf(fp, "%s", pwdp->pw_name);
}


/* print out information for group */
void
prtgrp(FILE	*fp, 
       char	*data)
{
	gid_t	tmpgroup;
	struct group	*grpp;

	bcopy(data, &tmpgroup, sizeof(gid_t));
	if ((grpp = getgrgid(tmpgroup)) == NULL)
		fprintf(fp, "%d", tmpgroup);
	else
		fprintf(fp, "%s", grpp->gr_name);
}


/*
 * take length as argument,
 * and print bytes in hex
 */
void
prtwhat(FILE	*fp, 
        char	*what, 
        int	len)
{
	int	tmp;

	switch (len)
	{
	    case sizeof(short):
		{
			short	s;

			bcopy(what, &s, sizeof(s));
			tmp = s;
		}
	    case sizeof(long):
		{
			long	l;

			bcopy(what, &l, sizeof(l));
			tmp = l;
		}
	    default:
		tmp = 0;
	}

	fprintf(fp, "0x%x", tmp);
}


static char	modebuf[4];
char *
prtmode(int	mode)
{
	sprintf(modebuf, "%c%c%c", (mode & 4) ? 'r' : '-',
				   (mode & 2) ? 'w' : '-',
				   (mode & 1) ? 'x' : '-');
	return(modebuf);
}
