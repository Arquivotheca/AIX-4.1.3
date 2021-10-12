static char sccsid[] = "@(#)63	1.3  src/bos/usr/bin/tcbck/systcbck.c, cmdsadm, bos411, 9428A410j 4/24/91 16:25:00";
/*
 * COMPONENT_NAME: (CMDSADM) sysck - sysck configuration checker
 *
 * FUNCTIONS: update_sysck
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/acl.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <errno.h>
#include <stdio.h>
#include <usersec.h>
#include "tcbattr.h"
#include "tcbmsg.h"
#include "tcbaudit.h"

extern	char	*comma2null();

struct	sys_attr {
	char	*sys_name;
	enum { SYS_VOID, SYS_CHAR, SYS_INT, SYS_LIST } sys_type;
} Sysck[] = {
	{ "checksum",		SYS_CHAR },
	{ "treeck_novfs",	SYS_LIST },
 	{ "treeck_nodir",       SYS_LIST },
	{ "setuids",		SYS_LIST },
	{ "setgids",		SYS_LIST },
	{ 0,			SYS_VOID }
};

/*
 * This is a dummy stanza structure used to build a SYSCK stanza
 * in.  It must be large enough to hold one value for each attribute,
 * plus one for the name and one more for the trailing NULL.
 */

#define	NVALUES	(8)
static	struct	ATTR	dummy[NVALUES];

/*
 * NAME:	update_sysck
 *
 * FUNCTION:	Modifies the SYSCK stanza in the configuration database
 *
 * SPECIAL NOTES:
 *	This function calls exit.  
 *
 * RETURN VALUES:
 *	NONE
 */

void
update_sysck (char **argv)
{
	char	*cp;
	int	c;
	int	i;
	int	j;
	int	len;
	int	created = 0;
	FILE	*fp;
	AFILE_t	afp;
	FILE	*bkfp;
	ATTR_t	sysck;
	ATTR_t	new;
	struct	sys_attr *attr;
	struct	acl	*acl;
	off_t	begin = 0;
	off_t	end = 0;
	off_t	pos;

	/*
	 * Must be root to update the sysck stanza, ACLs don't cut it ...
	 */

	if (getuidx (ID_REAL) != 0) {
		fmsg (No_Permission);
		exit (EPERM);
	}

	/*
	 * Open the SYSCK database
	 */

	if (! (afp = afopen (SYSCK_NAME))) {
		switch (errno) {
			case EPERM:
				fmsg (No_Permission);
				break;
			case EACCES:
			case ENOENT:
				fmsg1 (No_Such_File, SYSCK_NAME);
				break;
			default:
				xperror ("afopen", SYSCK_NAME, 0);
				break;
		}
		exit (errno);
	}
	fp = afp->AF_iop;

	/*
	 * Locate the start of the SYSCK stanza.  It is best
	 * to skip over any comments leading up to the stanza
	 * so they won't be removed accidentally ...
	 */

	while (1) {
		/*
		 * Skip any leading comment lines, remember to
		 * push back the first non-comment character.
		 */

		while ((c = getc (fp)) == '*')
			while ((c = getc (fp)) != '\n' && c != EOF)
				;

		if (c != '\n')
			ungetc (c, fp);

		/*
		 * Save my current position and read in the
		 * next stanza.
		 */

		pos = ftell (fp);
		if (! afread (afp)) {
			sysck = (ATTR_t) 0;
			break;
		}
		sysck = afp->AF_catr;

		/*
		 * If this is the stanza I am searching for then
		 * I want to locate it's end and break out.
		 *
		 * Otherwise, execution will continue until all
		 * of the stanzas have been read.
		 */

		if (strcmp (sysck->AT_value, "sysck") == 0) {
			begin = pos;
			end = ftell (fp);
			break;
		}
	}

	/*
	 * It is possible that there is no SYSCK stanza, in which
	 * case I have to make up one of my own.
	 */

	if (! sysck) {
		created = 1;

		dummy[0].AT_name = 0;
		dummy[0].AT_value = "sysck";

		dummy[1].AT_name = 0;
		dummy[1].AT_value = 0;

		sysck = dummy;
	}

	/*
	 * Now I have to wander through the list of arguments
	 * looking for attributes to update.
	 */

	for (j = 1;argv[j];j++) {

		/*
		 * Find out the length of this attribute name.  I'll
		 * compare the first "len" characters against the
		 * list of known attributes.
		 */

		if ((cp = (char *)strchr (argv[j], '=')) == 0)
			continue;

		len = cp - argv[j];

		/*
		 * See if the first "len" characters are the name of
		 * a valid attribute.
		 */

		for (i = 0;Sysck[i].sys_name;i++) {
			if (strncmp (argv[j], Sysck[i].sys_name, len) != 0)
				continue;

			if (argv[j][len] == '=')
				break;
		}

		/*
		 * If I reach the end of the list I report an error
		 * for the sysck.cfg file.
		 */

		if (! Sysck[i].sys_name) {
			argv[j][len] = '\0';
			fmsg2 (Illegal_Attribute, argv[j], SYSCK_Cfg);
			mk_audit_rec (SYSCK_Update, AUDIT_FAIL,
				SYSCK_Cfg, argv[j], NULL);

			continue;
		}

		/*
		 * I have the attribute information, now I have to see
		 * if this attribute already exists in the database.
		 */

		attr = &Sysck[i];

		for (i = 1;sysck[i].AT_name;i++) {
			if (strncmp (argv[j], sysck[i].AT_name, len) != 0)
				continue;

			if (argv[j][len] == '=')
				break;
		}
		if (sysck[i].AT_name) {

			/*
			 * The attribute was found, just update the existing
			 * information with the new value
			 */

			sysck[i].AT_value = argv[j] + len + 1;
		} else {

			/*
			 * This is a new attribute, create an entire new
			 * entry and initialize it with the given values.
			 */

			sysck[i + 1].AT_name = 0;
			sysck[i].AT_name = argv[j];
			argv[j][len] = '\0';
			sysck[i].AT_value = argv[j] + len + 1;
		}
		mk_audit_rec (SYSCK_Update, AUDIT_OK,
			SYSCK_Cfg, sysck[i].AT_name, NULL);

		/*
		 * See if this attribute is a double-NUL terminated
		 * string and convert to that format if it is one.
		 */

		if (attr->sys_type == SYS_LIST)
			sysck[i].AT_value = comma2null (sysck[i].AT_value);
	}

	/*
	 * Backup the old SYSCK file
	 */

	if ((bkfp = fopen (OSYSCK_NAME, "w+")) == 0)
		exit (errno);

	if (! (acl = (struct acl *)acl_fget (fileno (fp))))
		perror ("acl_fget");

	if (acl_fput (fileno (bkfp), acl, 0))
		perror ("acl_fput");

	ftruncate (fileno (bkfp), 0);
	rewind (fp);

	while ((c = getc (fp)) != EOF)
		putc (c, bkfp);

	fflush (bkfp);

	/*
	 * Now truncate the original SYSCK file and copy the
	 * contents up to the SYSCK stanza from the backup file.
	 */

	if (fp = fopen (SYSCK_NAME, "w")) {
		acl_fput (fileno (fp), acl, 0);
	} else {
		xperror ("fopen", SYSCK_NAME, 0);
		exit (ENOTRUST);
	}
	rewind (bkfp);

	for (pos = 0;pos < begin;pos++) {
		if ((c = getc (bkfp)) == EOF)
			break;

		putc (c, fp);
	}

	/*
	 * Output the the new SYSCK stanza to the new SYSCK file
	 */

	fprintf (fp, "%s:\n", sysck[0].AT_value);

	for (i = 1;sysck[i].AT_name;i++) {

		/*
		 * Locate the attribute in the table of SYSCK stanza attributes
		 * to determine what the type of the attribute is.  Attributes
		 * which don't exist are ignored.
		 */

		for (j = 0;Sysck[j].sys_name;j++)
			if (strcmp (Sysck[j].sys_name, sysck[i].AT_name) == 0)
				break;

		if (Sysck[j].sys_name == 0)
			continue;

		attr = &Sysck[j];

		if (sysck[i].AT_value == 0 || *sysck[i].AT_value == '\0')
			continue;

		/*
		 * Output the name of the attribute then figure out what to do
		 * with the value based on the database of the attribute.
		 */

		fprintf (fp, "\t%s = ", sysck[i].AT_name);

		if (attr->sys_type == SYS_LIST) {

			/*
			 * Output each string, separated one from the other
			 * with a comma.
			 */

			cp = sysck[i].AT_value;
			while (*cp) {
				fprintf (fp, "\"%s\"", cp);

				while (*cp++)
					;

				if (*cp)
					putc (',', fp);
			}
		} else if (attr->sys_type == SYS_CHAR) {

			/*
			 * Print out the value, surrounded by double
			 * quotes.
			 */

			fprintf (fp, "\"%s\"", sysck[i].AT_value);
		} else if (attr->sys_type == SYS_INT) {

			/*
			 * Just output the string
			 */

			fprintf (fp, "%s", sysck[i].AT_value);
		}
		putc ('\n', fp);
	}
	putc ('\n', fp);

	/*
	 * Finish up by copying the rest of the stanzas.  This is done
	 * by seeking to the end of the SYSCK stanza in the backup
	 * SYSCK file and copying from there to EOF into the new SYSCK
	 * file.
	 */

	fseek (bkfp, end, 0);

	while ((c = getc (bkfp)) != EOF)
		putc (c, fp);

	fflush (fp);

	fclose (bkfp);

	exit (0);
}
