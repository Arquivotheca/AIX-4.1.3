static char sccsid[] = "@(#)61	1.1  src/bos/usr/bin/tcbck/tcbdel.c, cmdsadm, bos411, 9428A410j 3/12/91 18:31:29";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: del_tcbent, del_tcbents
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
#include <sys/audit.h>
#include <sys/fullstat.h>
#include <sys/stat.h>
#include <sys/mode.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>


#include <usersec.h>
#include "tcbdbio.h"
#include "tcbmsg.h"
#include "tcbattr.h"
#include "tcbaudit.h"

extern	struct	tcbent	*gettcbent();
extern	struct	tcbent	*gettcbstanza();
extern	void	*hash_find();
extern	char	*xstrdup();

extern	int	verbose;

extern	struct	tcbent	**tcb_table;
extern	int	tcb_cnt;


extern	int	dflg;

#define	ADD_MODE	(S_ITCB|S_ITP|S_ISUID|S_ISGID|S_ISVTX)

/*
 * NAME: deltcbent
 *
 * FUNCTION: Remove an entry from the system configuration database.
 *
 * NOTES:
 *	Removes a single named stanza from an attribute file.  If
 *	the argument `backup' is non-zero, the contents of the old
 *	file shall be saved away.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */

int
deltcbent (name, backup)
char	*name;		/* Name of stanza to remove */
int	backup;		/* Backup file creation flag */
{
	char	*remainder;	/* pointer after the deleted record */
	char	*firstpart;	/* pointer before the deleted record */
	char	*mbuf = 0;	/* memory location for file */
	int	fdorg;		/* current file */
	int	fdold;		/* backup file */
	unsigned int 	orgsize; /* size of orginal file */
	struct	stat 	sbuf;	/* stat buffer */
	int	created = 0;	/* backup file just created */

	/*
	 * Open the original file and get its size.  The entire
	 * file will be read in one read then written out to the
	 * backup file, if requested.
	 */

	if ((fdorg = open (SYSCK_NAME, O_RDWR)) < 0) {
		mk_audit_rec (SYSCK_Update, AUDIT_FAIL,
			name, "ALL", NULL);
		return (errno);
	}
        if (fstat (fdorg, &sbuf) < 0) {
		mk_audit_rec (SYSCK_Update, AUDIT_FAIL,
			name, "ALL", NULL);
                return(errno);
	}
        orgsize = sbuf.st_size;

	if (backup) {
		if ((fdold = open (OSYSCK_NAME, O_WRONLY)) < 0) {
			if ((fdold = open (OSYSCK_NAME,
					O_CREAT|O_RDWR, sbuf.st_mode)) < 0) {
				close (fdorg);
				mk_audit_rec (SYSCK_Update, AUDIT_FAIL,
					name, "ALL", NULL);
				return (errno);
			}
			created++;
		}
	}

	/*
	 * Read the file:
	 *	1) Allocate memory for the entire file
	 *	2) Read in the file and NUL terminate it
	 *	3) Make a copy by writing it back out to another file [opt]
	 */

        if ((mbuf = (char *) malloc (orgsize + 1)) == NULL)
		goto error;

        if (read (fdorg,mbuf,orgsize) != orgsize)
		goto error;

	mbuf[orgsize] = '\0';

	if (backup) {
		if (ftruncate (fdold, 0) < 0 ||
				write (fdold, mbuf, orgsize) != orgsize)
			goto error;
	}

	/*
	 * Edit the file:
	 *	1) Find the beginning and the ending.
	 *	2) Point to the end of the beginning
	 *	3) Point to the beginning of the ending
	 */

	firstpart = 0;			/* initialize */
	remainder = mbuf + orgsize;	/* initialize */

	if (get_record (name, mbuf, &firstpart, &remainder)) {
		msg1 (No_File_Matched, name);
		errno = ENOENT;
		goto error;
	}

	/*
	 * Write out the new file:
	 *	1) Truncate the original file
	 *	2) Write the beginning
	 *	3) Write the ending
	 */

	ftruncate (fdorg, 0);
	lseek (fdorg, 0, 0);

	if (firstpart)
		write (fdorg, mbuf, firstpart - mbuf);

	if (remainder)
		write (fdorg, remainder, orgsize - (remainder - mbuf));

	/*
	 * Free the buffer used to hold the file and close the file
	 * descriptors.
	 */

	free (mbuf);

	close (fdorg);

	if (backup)
		close (fdold);

	mk_audit_rec (SYSCK_Update, AUDIT_OK, name, "ALL", NULL);

	return 0;

	/*
	 * Common error point.  Both file have been opened and the
	 * file possibly read in.  Some fatal error has occured and
	 * we need to bail out.  Free up the memory buffer and close
	 * the open file.  Return errno to the caller.
	 */

error:
	if (mbuf)
		free (mbuf);

	close (fdorg);

	if (backup) {
		close (fdold);

		if (created)
			unlink (OSYSCK_NAME);
	}
	mk_audit_rec (SYSCK_Update, AUDIT_FAIL, name, "ALL", NULL);
	return errno;
}

/*
 * NAME: del_tcbent
 *                                                                    
 * FUNCTION: Delete a single entry or class of entire entries
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Scans the entire TCB file for entries matching the names file
 *	or class.  All matching entries are removed.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
del_tcbent (argv)
char	**argv;
{
	char	last[BUFSIZ];
	struct	tcbent	*tp;
	int	committed = 0;		/* Files needs to be committed */
	int	i;
	int	j;
	int	*found;
	int	once = 0;
	int	errors = 0;

	/*
	 * Turn on error messages while running
	 */

	verbose = 1;

	/*
	 * Create a set of flags to indicate if a matching entry was
	 * found for an argument.
	 */

	for (i = 0;argv[i];i++)
		;

	found = (int *) xmalloc (i * sizeof *found);
	memset (found, 0, i * sizeof *found);

	/*
	 * Scan the entire TCB file, comparing the file name and class
	 * against the list of filenames and classes to be deleted.
	 *
	 * File names all begin with "/", classes all don't.
	 */

	while (1) {
		if (! (tp = gettcbent ())) {
			if (errno == 0)
				break;

			fmsg (Database_Error);
			if (last[0])
				fmsg1 (Last_Stanza, last);
			else
				fmsg (No_Last_Stanza);
			continue;
		}
		strncpy (last, tp->tcb_name, sizeof last);
		last[sizeof last - 1] = '\0';

		for (i = 0;argv[i];i++) {

			/*
			 * See if this is a request by name and see if the
			 * name matches the argument.  If it does not, skip
			 * this argument and go to the next.
			 */

			if (argv[i][0] == '/') {
				if (strcmp (tp->tcb_name, argv[i]) != 0)
					continue;

				/*
				 * Add the entry to the table so it can be
				 * deleted after all the classes are updated.
				 */

				found[i] = 1;

				add_tcb_table (tp);
				break;
			}

			/*
			 * See if this is a request by class and see if the
			 * class is in the list of classes for this file.  If
			 * it is break out.  Later, check for not having run
			 * through the entire class list.
			 */

			if (argv[i][0] != '/') {
				if (! tp->tcb_class || ! tp->tcb_class[0])
					continue;

				/* Search for the class in the class-list */

				for (j = 0;tp->tcb_class[j];j++)
					if (! strcmp (tp->tcb_class[j],
							argv[i]))
						break;

				/*
				 * If the class was not found, try the next
				 * argument.
				 */

				if (tp->tcb_class[j] == 0)
					continue;

				/*
				 * See if this is not the last class in the
				 * list.  If it isn't, copy the rest of the
				 * list down and write out the new entry.
				 */

				if (j == 0 && tp->tcb_class[1] == 0) {
					found[i] = 1;
					add_tcb_table (tp);
					break;
				}
				for (;tp->tcb_class[j];j++)
					tp->tcb_class[j] =
						tp->tcb_class[j + 1];

				tp->tcb_changed = 1;

				/*
				 * See if there are any more class or
				 * file arguments to match.  Process the
				 * next one if there are any.
				 */

				if (argv[i + 1])
					continue;

				/*
				 * I've not deleted this entry just yet,
				 * so I must have not removed all the
				 * classes.
				 */

				puttcbent (tp);
				committed = 1;
				found[i] = 1;
			}
		}
	}

	/*
	 * Commit any changes made via the database access routines.
	 * Then make any changes which couldn't be made that way ...
	 */

	if (committed) {
		puttcbattr ((char *) 0,(char *) 0, (void *) 0, SEC_COMMIT);
		committed = 1;
		once = 1;
	}
	for (i = 0;i < tcb_cnt;i++) {
		if (deltcbent (tcb_table[i]->tcb_name, ! once)) {
			msg1 (Del_Failed, tcb_table[i]->tcb_name);
			errors++;
		}
		if (! once)
			once++; /* only backup the file once */
	}

	/*
	 * The list of arguments is scanned to see if they were all
	 * matched.  Any arguments which did not have a matching
	 * file entry are reported as errors.
	 */

	for (i = 0;argv[i];i++) {
		if (found[i] == 0) {
			if (argv[i][0] == '/')
				msg1 (No_File_Matched, argv[i]);
			else
				msg1 (No_Class_Matched, argv[i]);

			errors++;
		}	
	}
	return errors == 0 ? 0:ENOENT;
}

/*
 * NAME: del_tcbents
 *                                                                    
 * FUNCTION: Delete TCB entries named in a file.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Scans the a stanza file removing entries matched in the TCB
 *	file.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
del_tcbents (file)
char	*file;
{
	AFILE_t	fp;
	struct	tcbent	*tp;
	struct	fullstat sb;
	int	i;
	int	rc;
	int	once = 0;
	int	errors = 0;
	int	lastrc;

	/*
	 * It is expected that there may be some errors from the
	 * input file.  It would be best to specify exactly which
	 * error messages we want output.  For DELETE mode we
	 * assume the invoker knows what they are doing and wants
	 * to see all error messages.  For UNINSTALL mode we
	 * assume the invoker is INSTALLP or some such and is only
	 * calling SYSCK to cleanup a database which may or may
	 * not be complete.
	 */

	if (dflg)
		verbose = 1;
	else
		verbose = 0;

	/*
	 * Open the named stanza file and read in the stanzas.  The
	 * file name in each stanza is then used to delete the named
	 * stanza from the TCB file.
	 */

	if (! (fp = afopen (file))) {
		fmsg1 (No_Such_File, file);
		return ENOENT;
	}
	while (1) {

		/*
		 * Read all of the stanzas to be removed from the
		 * SYSCK database.
		 */

		if (! (tp = gettcbstanza (fp))) {
			if (errno != 0) {
				errors++;
				continue;
			} else
				break;
		}


		/*
		 * The stanza from the SYSCK database is to be removed.
		 */

		if (rc = deltcbent (tp->tcb_name, ! once)) {
				fmsg1 (Del_Failed, tp->tcb_name);
				errors++;
		}
		if (! once)
			once++; /* only backup the file once */
	}

	return errors ? ENOTRUST:0;
}
