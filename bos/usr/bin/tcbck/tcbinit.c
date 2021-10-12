static char sccsid[] = "@(#)46	1.2  src/bos/usr/bin/tcbck/tcbinit.c, cmdsadm, bos411, 9428A410j 4/5/91 14:23:28";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: init_sysck, init_vfs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/vfs.h>
#include <sys/vmount.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <usersec.h>
#include "tcbmsg.h"
#include "tcbattr.h"

/*
 * Global variables and external functions
 */

extern	char	*Checksum;
extern	int	BuiltInSum;
extern	char	**novfs;
extern	char	**nodir;
extern	char	**vfs;
extern	gid_t	*setgids;
extern	uid_t	*setuids;

extern	char	*xstrdup(char *);
extern	char	*xmalloc(unsigned);
extern	char	**null_list(char *);

#define	SYSCK_NAME	"/etc/security/sysck.cfg"

#define	vm_next(vm)	((struct vmount *) (((char *) vm) + vm->vmt_length))

/*
 * NAME: init_sysck
 *
 * FUNCTION: Initialize tables required by sysck
 *
 * NOTES:
 *	Various tables, such as administrative UIDs and
 *	GIDs are built from a stanza in the sysck configuration
 *	file.  Alternate checksum programs and file systems to
 *	ignore are also listed.
 *
 *	TBD: This code assumes the sysck.cfg file is a stanza
 *	file.  This assumption will be incorrect in a future
 *	release.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: NONE
 */

void
init_sysck (void)
{
	char	*cp, *cp2;	/* String manipulation pointers             */
	int	i, j;		/* Loop counters, etc.                      */
	struct	passwd	*pw,	/* Password file entry for UID list         */
			*getpwnam(char *);
	struct	group	*gr,	/* Group file entry for GID list            */
			*getgrnam(char *);
	AFILE_t	fp;		/* Stanza file pointer                      */
	ATTR_t	attr;		/* Pointer to stanza from config file       */
	char	**ids;		/* Pointer to list of user and group IDs    */

	/*
	 * Open the sysck stanza file and read until the 'sysck' stanza
	 * is read.
	 */

	if (! (fp = afopen (SYSCK_NAME)))
		return;

	while (1) {
		if (! afread (fp)) {
			if (errno == 0) {
				afclose (fp);
				return;
			} else {
				fmsg (Database_Error);
				continue;
			}
		}
		attr = fp->AF_catr;
		if (strcmp (attr->AT_value, "sysck") == 0)
			break;
	}

	/* 
	 * Look for a replacement "checksum" program, override
	 * the default value if so.
	 */

	for (i = 0;attr[i].AT_name;i++) {
		if (strcmp (attr[i].AT_name, "checksum") == 0) {
			Checksum = xstrdup (attr[i].AT_value);
			BuiltInSum = 0;
			break;
		}
	}

	/*
	 * Look for a list of file systems to not test.  The
	 * treeck_novfs attribute is a list of mount points to not
	 * examine.
	 */

	for (cp = (char *) 0, i = 0;attr[i].AT_name;i++) {

		/*
		 * Locate the treeck_novfs attribute in the
		 * "sysck" stanza.  It is an optional item.
		 */

		if (strcmp (attr[i].AT_name, "treeck_novfs") == 0) {
			cp = attr[i].AT_value;
			break;
		}
	}
	if (cp)
		novfs = null_list (cp);

	/*
	 * Look for a list of directories not to test.  The
	 * treeck_nodir attribute is a list of directory points not to 
	 * examine.
	 */

	for (cp = (char *) 0, i = 0;attr[i].AT_name;i++) {

		/*
		 * Locate the treeck_novfs attribute in the
		 * "sysck" stanza.  It is an optional item.
		 */

		if (strcmp (attr[i].AT_name, "treeck_nodir") == 0) {
			cp = attr[i].AT_value;
			break;
		}
	}
	if (cp)
		nodir = null_list (cp);

	/*
	 * Look for an array of additional UIDs to test for.  This
	 * optional attribute lists administrative UIDs which must
	 * also be tested for.  Each ID must be a valid username.
	 */

	for (cp = (char *) 0, i = 0;attr[i].AT_name;i++) {
		if (strcmp (attr[i].AT_name, "setuids") == 0) {
			cp = attr[i].AT_value;
			break;
		}
	}
	if (cp) {
		ids = null_list (cp);

		/*
		 * Count the number of IDs in the list so a table
		 * can be allocated to hold them.
		 */

		for (i = 0;ids[i];i++)
			;

		i++;		/* add 1 last item */
		setuids = (uid_t *) xmalloc (i * sizeof (uid_t));

		/*
		 * Scan the list, adding UIDs to the table as we
		 * go.  Each ID is looked up for it's numerical UID.
		 * The entry must exist or an error is reported.  THe
		 * list itself is terminated with -1, which is an
		 * invalid ID.
		 */

		for (i = j = 0;ids[j];j++) {
			if (pw = getpwnam (ids[j]))
				setuids[i++] = pw->pw_uid;
			else
				msg1 (Unknown_User, ids[j]);
		}
		setuids[i] = -1;

		for (j = 0;ids[j];j++)
			free (ids[j]);

		free (ids);
	}

	/*
	 * Look for an additional array of GIDs to test for.  This
	 * optional attribute lists administrative UIDs which must
	 * also be tested for.  Each ID must be a valid username.
	 */

	for (cp = (char *) 0, i = 0;attr[i].AT_name;i++) {
		if (strcmp (attr[i].AT_name, "setgids") == 0) {
			cp = attr[i].AT_value;
			break;
		}
	}
	if (cp) {
		ids = null_list (cp);

		/*
		 * Count the number of IDs in the list so a table
		 * can be allocated to hold them.
		 */

		for (i = 0;ids[i];i++)
			;

		i++;		/* add 1 last item */
		setgids = (gid_t *) xmalloc (i * sizeof (gid_t));

		/*
		 * Scan the list, adding GIDs to the table as we
		 * go.  Each ID is looked up for it's numerical GID.
		 * The entry must exist or an error is reported.  THe
		 * list itself is terminated with -1, which is an
		 * invalid ID.
		 */

		for (i = j = 0;ids[j];j++) {
			if (gr = getgrnam (ids[j]))
				setgids[i++] = gr->gr_gid;
			else
				msg1 (Unknown_Group, ids[j]);
		}
		setgids[i] = -1;

		for (j = 0;ids[j];j++)
			free (ids[j]);

		free (ids);
	}
	afclose (fp);
}

/*
 * NAME:	init_vfs
 *
 * FUNCTION:	Get the names of all mounted filesystems
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	Reads the kernel mount table to get the names of the
 *	mounted filesystems.  The /etc/mnttab file is not
 *	reliable for this operation.
 *
 * RETURNS: NONE
 */

void
init_vfs (void)
{
	struct	vmount	*vmtab;	/* Mount entries returned by kernel         */
	struct	vmount	*vm;	/* Cursor into mount table                  */
	int	i;		/* Loop index                               */
	int	size;		/* Size of mount table                      */
	int	nmount;		/* Number of entries in mount table         */

	/*
	 * Get the system list of mounted filesystems and their
	 * mount points, types, etc.
	 *
	 * It is maintained as a variable length array of variable
	 * length elements.  Begin by attempting to read an element.
	 * If the size is too small, the call will fail but the
	 * correct size will be returned in vmt_revision.  Use that
	 * size to try again.  Do this until it is done correctly.
	 */

	size = sizeof *vmtab;
	while (1) {
		vmtab = (struct vmount *) xmalloc (size);
		nmount = mntctl (MCTL_QUERY, size, vmtab);

		if (nmount > 0 && (vmtab->vmt_revision != VMT_REVISION))
			return;
		
		if (nmount < 0)
			xperror ("mntctl", 0);

		if (nmount == 0) {
			size = vmtab->vmt_revision;
			free (vmtab);
			continue;
		}
		break;
	}
	vfs = (char **) xmalloc ((nmount + 1) * sizeof (char *));

	/*
	 * Now scan the mount table for the names of mount points.
	 * Save these mount points away for later use.
	 */

	for (vm = vmtab, i = 0;i < nmount;i++, vm = vm_next (vm)) {
		size = vmt2datasize (vm, VMT_STUB) + 1;
		vfs[i] = xmalloc (size);
		strncpy (vfs[i], vmt2dataptr (vm, VMT_STUB), size - 1);
		vfs[i][size - 1] = '\0';
	}
	vfs[nmount] = 0;

	free (vmtab);
}
