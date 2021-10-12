static char sccsid[] = "@(#)51  1.2.1.4  src/bos/usr/bin/tcbck/tcbtree.c, cmdsadm, bos411, 9439B411a 9/27/94 14:31:21";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: ftwx, has_uid, has_gid, treeck_one, treeck
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

#include <sys/types.h>
#include <sys/mode.h>
#include <sys/audit.h>
#include <sys/stat.h>
#include <sys/fullstat.h>
#include <sys/vmount.h>
#include <sys/priv.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <ftw.h>
#include "tcbdbio.h"
#include "tcbmsg.h"
#include "tcbaudit.h"

#ifndef	S_ISLNK
#define	S_ISLNK(n)	(((n) & S_IFMT) == S_IFLNK)
#endif

#define	ADMIN_ID (200)
#define	ALL_MODES (S_IXACL|S_ITP|S_ITCB|S_ISUID|S_ISGID|S_ISVTX|0777)

extern	char	**novfs;
extern	char	**nodir;
extern	uid_t	*setuids;
extern	gid_t	*setgids;
extern	void	*hash_find();
extern	struct	pcl	*priv_get();

static	struct	fullstat root;	/* stat structure for root directory */
static	int	errors;		/* Count of errors from tree check */

extern	char	*xmalloc();
extern	char	*xstrdup();
extern	int	errno;

/*
 * NAME: ftwx
 *                                                                    
 * FUNCTION: Enhanced file tree walk
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	May be called by a user process.  Recursively calls itself.
 *                                                                   
 * NOTES:
 *	modified ftw() which:
 *		1) uses opendir/readdir
 *		2) calls the function with a "fullstat" structure,
 *		   instead of "stat"
 *		3) does not traverse symbolic links
 *		4) uses the return from the function to prune the tree,
 *	   	   rather than terminate.  this is applicable only to
 *		   directories.
 *	(1)-(3) should have been done in the real ftw();
 *	(4) is strictly for the benefit of <sysck>
 *
 *	the return value is non-zero only for a "hard" error; e.g.,
 *	xmalloc() failed.
 *
 * RETURNS:  Zero on success, non-zero on non-recoverable errors.
 */  

int
ftwx (	char *path,
	int (*fn)(char *, struct fullstat *, int),
	int depth)
{
	int	n;		/* index to end of filename                 */
	off_t	here;		/* offset into current directory            */
	char	*subpath,	/* pointer to new directory or pathname     */
		*component;	/* pointer to where last component goes     */
	struct	fullstat sb;	/* info about current file or directory     */
	DIR	*dirp;		/* handle of current directory              */
	struct	dirent	*dp;	/* pointer to a directory entry             */
	int	i;		

	/*
	 * Try to get file status.  If unsuccessful, errno will say why.
	 */

	if(fullstat(path, FL_NOFOLLOW | FL_STAT, &sb) < 0) {
		(*fn) (path, &sb, FTW_NS);
		return (0);
	}

	/*
	 *	The stat succeeded, so we know the object exists.
	 *	If not a directory, call the user function and return.
	 */

	if (! S_ISDIR (sb.st_mode)) {
		(*fn) (path, &sb, FTW_F);
		return (0);
	}

	/*
	 * Make sure directory is not in the list of directory points
	 * to ignore
	 */

	if (nodir)
		for (i = 0;nodir[i];i++)
			if (strcmp (path, nodir[i]) == 0)
				return 0;

	/*
	 *	The object was a directory.
	 *
	 *	Open a file to read the directory
	 */

	dirp = opendir (path);

	/*
	 *	Call the user function, telling it whether
	 *	the directory can be read.  If it can't be read
	 *	call the user function or indicate an error,
	 *	depending on the reason it couldn't be read.
	 */

	if(dirp == NULL) {
		(*fn) (path, &sb, FTW_DNR);
		return (0);
	}

	/*
	 * We could read the directory.  Call user function.  Return
	 * immediately if function returns non-zero.
	 */

	if ((*fn) (path, &sb, FTW_D))
		return 0;

	/*
	 * Allocate a buffer to hold generated pathnames.
	 */

	n = strlen (path);
	subpath = xmalloc ((unsigned) (PATH_MAX));
	if (subpath == NULL) {
		(void) close (dirp);
		errno = ENOMEM;
		return errno;
	}

	/*
	 * Create a prefix to which we will append component names
	 */

	(void) strcpy (subpath, path);
	if(subpath[0] != '\0' && subpath[n-1] != '/')
		subpath[n++] = '/';
	component = &subpath[n];

	/*
	 *	Read the directory one component at a time.
	 *	We must ignore "." and "..", but other than that,
	 *	just create a path name and call self to check it out.
	 */

	while (dp = readdir (dirp)) {

		/*
		 * Find the final component of the new
		 * file.  Ignore '.' and '..' directories.
		 */

		if (dp->d_name[0] == '\0')
			continue;

		if (dp -> d_name[0] == '.') {
			if (dp -> d_name[1] == '\0')
				continue;
			if (dp -> d_name[1] == '.') {
				if (dp -> d_name[2] == '\0')
					continue;
			}
		}

		/*
		 * Build the full pathname
		 */

		strcpy (component, dp -> d_name);

		/*
		 *	If we are about to exceed our depth,
		 *	remember where we are and close a file.
		 */

		if(depth <= 1) {
			here = telldir(dirp);
			closedir (dirp);
			dirp = NULL;
		}

		/*
		 *	Do a recursive call to process the file.
		 */

		ftwx (subpath, fn, depth-1);

		/*
		 *	If we closed the file, try to reopen it.
		 */

		if (dirp == NULL) {
			dirp = opendir (path);
			if (dirp == NULL) {
				free (subpath);
				return errno;
			}
			seekdir (dirp, here);
		}
	}

	/*
	 *	We got out of the subdirectory loop.  Free the
	 *	pathname buffer and close the current directory.
	 */

	free (subpath);
	closedir (dirp);
	return 0;
}

/*
 * NAME: has_uid
 *                                                                    
 * FUNCTION: Search a list of UIDs for a match
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *                                                                   
 * RETURNS: Zero if no match, non-zero otherwise.
 */  

static int
has_uid (uid_t uid, uid_t *uids)
{
	if (! uids)
		return 0;

	while (*uids != (uid_t) -1)
		if (*uids++ == uid)
			return 1;

	return 0;
}

/*
 * NAME: has_gid
 *
 * FUNCTION: Search a list of GIDs for a match
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Zero if GID not found, non-zero otherwise
 */  

static int
has_gid (gid_t gid, gid_t *gids)
{
	if (! gids)
		return 0;

	while (*gids != (gid_t) -1)
		if (*gids++ == gid)
			return 1;

	return 0;
}

/*
 * NAME: new_entry
 */

struct tcbent *
new_entry (struct fullstat *sb, char *name)
{
	struct	tcbent	*tcbent;
	char	device[32];

	/*
	 * Allocate space for a new entry and initialize it
	 */

	tcbent = (struct tcbent *) xmalloc (sizeof *tcbent);
	init_tcbent (tcbent);

	/*
	 * Set up the default entries - every stanza needs to
	 * have name, owner, group, mode, and type.  The modes
	 * can not be more than just 0777.
	 */

	tcbent->tcb_name = xstrdup (name);
	tcbent->tcb_owner = sb->st_uid;
	tcbent->tcb_group = sb->st_gid;

	if (sb->st_mode & (ALL_MODES & (~0777))) {
		add_acl (tcbent, (char *) 0, sb);
		tcbent->tcb_mode = sb->st_mode & ALL_MODES;
	} else
		tcbent->tcb_mode = sb->st_mode & 0777;

	/*
	 * Figure out the type the hard way ...
	 */

	if (S_ISDIR (sb->st_mode))
		tcbent->tcb_type = TCB_DIR;
	else if (S_ISFIFO (sb->st_mode))
		tcbent->tcb_type = TCB_FIFO;
	else if (S_ISMPX (sb->st_mode))
		tcbent->tcb_type = TCB_MPX;
	else if (S_ISCHR (sb->st_mode))
		tcbent->tcb_type = TCB_CHAR;
	else if (S_ISBLK (sb->st_mode))
		tcbent->tcb_type = TCB_BLK;
	else if (S_ISREG (sb->st_mode))
		tcbent->tcb_type = TCB_FILE;

	if (tcbent->tcb_type == TCB_MPX && tcbent->tcb_mode != -1)
		tcbent->tcb_mode &= ~S_ISVTX;
	
	/*
	 * Flag this entry as changed and valid, and add it
	 * to the hash table.  Right now it only has one known
	 * link - worry about the rest later.
	 */

	tcbent->tcb_changed = 1;
	tcbent->tcb_valid = 1;

	add_tcb_table (tcbent);
	sprintf (device, "%d,%d", sb->st_dev, sb->st_ino);
	(void) hash_add (xstrdup (device), (char *) tcbent);
	return tcbent;
}

/*
 * NAME: new_link
 */

int
new_link (struct tcbent *tcb, char *link)
{
	char	**links;
	int	nlinks = 0;

	/*
	 * Count the number of hard links.  One more is actually
	 * needed because of the trailing NULL, plus another one for
	 * the new member.
	 */

	if (tcb->tcb_links)
		for (nlinks = 0;tcb->tcb_links[nlinks];nlinks++)
			;

	links = (char **) malloc ((nlinks + 2) * sizeof *links);
	memcpy ((void *) links, (void *) tcb->tcb_links,
			(nlinks + 1) * sizeof *links);

	/*
	 * Copy for the file name into the old last entry, and
	 * add to the hash table.  Add a new NULL to terminate
	 * the list.
	 */

	links[nlinks] = xstrdup (link);
	(int) hash_add (links[nlinks - 1], (char *) tcb);
	links[nlinks + 1] = (char *) 0;

	/*
	 * Now free the old list of links and make put the
	 * new list into the tcbent structure.  Mark the entry
	 * as having been changed.
	 */

	if (tcb->tcb_links)
		free ((void *) tcb->tcb_links);

	tcb->tcb_links = links;
	tcb->tcb_changed = 1;

	return 0;
}

/* 
 * NAME: new_symlink
 */

int
new_symlink (struct tcbent *tcb, char *link)
{
	char	**links;
	int	nlinks = 0;

	/*
	 * Count the number of symbolic links.  One more is actually
	 * needed because of the trailing NULL, plus another one for
	 * the new member.
	 */

	if (tcb->tcb_symlinks)
		for (nlinks = 0;tcb->tcb_symlinks[nlinks];nlinks++)
			;

	links = (char **) malloc ((nlinks + 2) * sizeof *links);
	memcpy ((void *) links, (void *) tcb->tcb_symlinks,
			(nlinks + 1) * sizeof *links);

	/*
	 * Copy for the file name into the old last entry, and
	 * add to the hash table.  Add a new NULL to terminate
	 * the list.
	 */

	links[nlinks] = xstrdup (link);
	(int) hash_add (links[nlinks - 1], (char *) tcb);
	links[nlinks + 1] = (char *) 0;

	/*
	 * Now free the old list of links and make put the
	 * new list into the tcbent structure.  Mark the entry
	 * as having been changed.
	 */

	if (tcb->tcb_symlinks)
		free ((void *) tcb->tcb_symlinks);

	tcb->tcb_symlinks = links;
	tcb->tcb_changed = 1;

	return 0;
}

/*
 * NAME: treeck_one
 *                                                                    
 * FUNCTION: Test a single non-TCB file for violations of the
 *	Security Policy.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Called by ftwx() to process each file in the directory structure.
 *
 * RETURNS: Zero on success, non-zero on error.
 */

static int
treeck_one (char *file, struct fullstat *sb, int flag)
{
	int	i;
	char	key[16];
	char	symlink[BUFSIZ];
	struct	tcbent	*tcbent;
	struct	tcbent	*newent = 0;
	struct	pcl	*pcl;
	struct	fullstat statbuf, *stat2=&statbuf;
	int checkfile = 0;

	/*
	 * Do not cross any mount points.  Return TRUE, which causes
	 * ftwx() to abandon work on that branch of the directory
	 */

	if ((sb->fst_flag & FS_VMP) &&
			(root.st_dev != sb->st_dev ||
			 root.st_ino != sb->st_ino))
		return 1;

	/*
	 * Search for the file in the TCB entries which already have
	 * been checked.  Return if this file is in the TCB
	 */

	if (hash_find (file))
		return 0;

	/*
	 * Get the inode for this file, or if a symbolic link the file
	 * the link points to.  This file may not be linked to a file
	 * in the TCB.  Any illegal links are removed.
	 * As of 4.1.1 you may have SUID or SGID programs in the sysck.cfg
         * file that are not part of the TCB.  These symlinks don't have
	 * to be in the sysck.cfg file if the target is only SUID or SGID
         * and not part of the TCB.
	 */

	if (S_ISLNK (sb->st_mode)) {
		if (fullstat (file, FL_STAT, stat2))
			return 0;

		sprintf (key, "%d,%d", stat2->st_dev, stat2->st_ino);
		if (stat2->st_mode & S_ITCB)
			checkfile++;
		
	} else
		{
		sprintf (key, "%d,%d", sb->st_dev, sb->st_ino);
		if (sb->st_mode & S_ITCB)
			checkfile++;
		}

	if (checkfile)
	{
	   if (tcbent = (struct tcbent *) hash_find (key))
		{
		errors++;
		msg2 (S_ISLNK (sb->st_mode) ? Illegal_Symlink:Illegal_Link,
			file, tcbent->tcb_name);

		if (ck_query (Remove_File, file)) {
			if (xremove (file, sb))
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					file, CantFix,
					"link to %s", tcbent->tcb_name, NULL);
			else
				mk_vaudit (SYSCK_Check, AUDIT_OK,
					file, Fixed,
					"link to %s", tcbent->tcb_name, NULL);
		} else if (ck_query (S_ISLNK (sb->st_mode) ?
				New_Symlink:New_Link, file, tcbent->tcb_name)) {
			if (S_ISLNK (sb->st_mode))
				new_symlink (tcbent, file);
			else
				new_link (tcbent, file);
		} else
			mk_vaudit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed,
				"link to %s", tcbent->tcb_name, NULL);

		return 0;
		}
	}

	/*
	 * If this is a SUID program with an administrative ID, or one
	 * of the additional IDs to be tested, turn off the SUID bit in
	 * the mode.
	 */

	if (S_ISREG (sb->st_mode) && (sb->st_mode & S_ISUID) &&
			(sb->st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))) {
		if (sb->st_uid < ADMIN_ID || has_uid (sb->st_uid, setuids)) {
			errors++;
			msg1 (Unknown_SUID_File, file);

			if (ck_query (Disable_Mode, file)) {
				sb->st_mode &= ~S_ISUID;
				if (ch_mode (file, sb->st_mode)) {
					msg1 (Chmod_Failed, file);
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "setuid");
				} else
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "setuid");
			} else if (ck_query (newent ? New_Modes:New_Entry,
					file)) {
				if (! newent)
					newent = new_entry (sb, file);

				newent->tcb_mode |= S_ISUID;
			} else
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, NotFixed, "setuid");
		}
	}

	/*
	 * If this is a SGID program with an administrative ID, or one
	 * of the additional IDs to be tested, turn off the SGID bit in
	 * the mode.
	 */

	if (S_ISREG (sb->st_mode) && (sb->st_mode & S_ISGID) &&
			(sb->st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))) {
		if (sb->st_gid < ADMIN_ID || has_gid (sb->st_gid, setgids)) {
			errors++;
			msg1 (Unknown_SGID_File, file);

			if (ck_query (Disable_Mode, file)) {
				sb->st_mode &= ~S_ISGID;
				if (ch_mode (file, sb->st_mode)) {
					msg1 (Chmod_Failed, file);
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "setgid");
				} else
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "setgid");
			} else if (ck_query (newent ? New_Modes:New_Entry,
					file)) {
				if (! newent)
					newent = new_entry (sb, file);

				newent->tcb_mode |= S_ISGID;
			} else
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, NotFixed, "setgid");
		}
	}

	if (S_ISCHR (sb->st_mode) || S_ISBLK (sb->st_mode)) {
		errors++;
		msg1 (Unknown_Device, file);
		msg (Register_Trusted);

	/*  Tcbck will not remove the /dev entries if they are not
    	 *  registered in the TCB. Tcbck will will issue a warning
         *  to that effect and suggest to the administrator to add the 
         *  device with the -l option of tcbck.

         *  This will depend upon the administrator trusting the current
         *  status of the device in /dev.
         */

		if (ck_query (New_Entry, file))
			newent = new_entry (sb, file);
		else
			mk_audit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed, "device");
	}

	/*
	 * Query the file TCB attribute status.  All files with the
	 * TCB attribute turned on must be listed in the TCB file.
	 */

	if (sb->st_mode & S_ITCB) {
		errors++;
		msg1 (Unknown_TCB_File, file);

		if (ck_query (Disable_Mode, file)) {
			sb->st_mode &= ~S_ITCB;
			if (ch_mode (file, sb->st_mode)) {
				msg1 (Chmod_Failed, file);
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, CantFix, "tcb bit");
			} else
				mk_audit (SYSCK_Check, AUDIT_OK,
					file, Fixed, "tcb bit");
		} else if (ck_query (newent ? New_Modes:New_Entry, file)) {
			if (! newent)
				newent = new_entry (sb, file);

			newent->tcb_mode |= S_ITCB;
		} else
			mk_audit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed, "tcb bit");
	}

	/*
	 * Query the file TP attribute status.  All files with the
	 * TP attribute turned on must be listed in the TCB file.
	 */

	if (sb->st_mode & S_ITP) {
		errors++;
		msg1 (Unknown_TP_File, file);

		if (ck_query (Disable_Mode, file)) {
			sb->st_mode &= ~S_ITP;
			if (ch_mode (file, sb->st_mode)) {
				msg1 (Chmod_Failed, file);
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, CantFix, "tp bit");
			} else
				mk_audit (SYSCK_Check, AUDIT_OK,
					file, Fixed, "tp bit");
		} else if (ck_query (newent ? New_Modes:New_Entry, file)) {
			if (! newent)
				newent = new_entry (sb, file);

			newent->tcb_mode |= S_ISGID;
		} else
			mk_audit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed, "tp bit");
	}

	/*
	 * All files with privilege must be listed in the TCB
	 * file.  Non-empty privilege lists are removed.
	 */

	if ((pcl = priv_get (file)) &&
		       (pcl->pcl_len > PCL_SIZ ||
			pcl->pcl_default.pv_priv[0] ||
			pcl->pcl_default.pv_priv[1])) {
		errors++;

		msg1 (Unknown_Priv_File, file);

		if (ck_query (Disable_PCL, file)) {
			pcl->pcl_len = PCL_SIZ;
			pcl->pcl_default.pv_priv[0] = 0;
			pcl->pcl_default.pv_priv[1] = 0;
			if (priv_put (file, pcl, 0))
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, CantFix, "illegal pcl");
			else
				mk_audit (SYSCK_Check, AUDIT_OK,
					file, Fixed, "illegal pcl");
		} else if (ck_query (newent ? New_Modes:New_Entry, file)) {
			if (! newent)
				newent = new_entry (sb, file);

			add_pcl (newent, (char *) 0, sb);
		} else
			mk_audit (SYSCK_Check, AUDIT_FAIL,
				file, NotFixed, "illegal pcl");
	}
	if (pcl)
		free (pcl);

	return 0;
}

/*
 * NAME: treeck
 *                                                                    
 * FUNCTION: Test an entire directory structure for violations of the
 *	Security Policy
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *
 * NOTES:
 *	treeck is called for each mount point on the system.  It selects
 *	which branchs will be tested by checking a list of forbidden
 *	sub-trees.
 *
 * RETURNS: Zero if no illegal files were found, non-zero otherwise.
 */  

int
treeck (char *dir)
{
	int	i;

	/*
	 * Initialize count of errors while checking this tree
	 */

	errors = 0;

	/*
	 * Make sure directory is not in the list of mount points
	 * to ignore
	 */

	if (novfs)
		for (i = 0;novfs[i];i++)
			if (strcmp (dir, novfs[i]) == 0)
				return 0;

	/*
	 * stat the root directory to test crossing a mount
	 * point.  Then check this entire directory, abandoning
	 * any directories on different devices from the current
	 * root.
	 */

	fullstat (dir, FL_STAT|FL_NOFOLLOW, &root);
	ftwx (dir, treeck_one, 20);

	return errors ? ENOTRUST:0;
}
