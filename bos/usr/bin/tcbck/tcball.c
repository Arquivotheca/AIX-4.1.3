static char sccsid[] = "@(#)53	1.1.1.2  src/bos/usr/bin/tcbck/tcball.c, cmdsadm, bos411, 9440F411a 10/12/94 12:57:51";

/*
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
 *
 * FUNCTIONS: add_tcb_table, list_member, check_tcb
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/stat.h>
#include <sys/audit.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tcbaudit.h"
#include "tcbdbio.h"
#include "tcbmsg.h"

#define	TCBINCR	256
#define TCBSIZE(n)	((n)*sizeof (struct tcbent))

extern	struct	tcbent	**tcb_table;
extern	int	tcb_cnt;
extern	int	tcb_max;
extern	int	all;
extern	int	tree;
extern	int	verbose;

extern	char	*xmalloc();
extern	char	*realloc();
extern	char	*xstrdup();
extern	struct	tcbent	*gettcbent();
extern	void	*hash_find();

char	**file_list;
struct	symlink_rec	*symlist=NULL;	/* linked list of symlinks in file */

/*
 * NAME: add_tcb_table
 *                                                                    
 * FUNCTION: Add an entry from the TCB file to the internal array.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.
 *                                                                   
 * DATA STRUCTURES: May change the size of the TCB entry table and the
 *	location of the table itself.
 *
 * RETURNS: NONE
 */  

int
add_tcb_table (tcbent)
struct	tcbent	*tcbent;
{
	int			i;
	struct	tcbent		*tp;
	struct	stat		statbuf;
	char			**links;
	char			*file;
	struct	symlink_rec	*symrec;

	/*
	 * Create some nice aliases for the members of the structure
	 */

	file = tcbent->tcb_name;
	links = tcbent->tcb_links;

	/*
	 * See if file is in the table as an entry or a link already.
	 * This is not permitted.
	 */

	if (tp = (struct tcbent *) hash_find (file)) {
		if (strcmp (file, tp->tcb_name) == 0) {
			fmsg2 (Duplicate_Stanza, file);
			mk_vaudit (SYSCK_Check, AUDIT_FAIL,
				SYSCK_Cfg, CantFix,
				Duplicate_Stanza, file, NULL);
			return ENOTRUST;
		} else {
			fmsg3 (Duplicate_Name, file, file, tp->tcb_name);
			mk_vaudit (SYSCK_Check, AUDIT_FAIL,
				SYSCK_Cfg, CantFix,
				Duplicate_Name, file, tp->tcb_name, NULL);
			return ENOTRUST;
		}
	}

	/*
	 * See if there is room in the current table is able to
	 * be added to.  If not, expand it by the increment amount.
	 */

	if (tcb_cnt >= tcb_max) {
		if (!tcb_table) { /* tcb_table  is NULL-must be first time */
			tcb_max = TCBINCR;
			tcb_table = (struct tcbent **)
				xmalloc (TCBINCR * sizeof (struct tcbent *));
		}
		else {		/* table exists, re-allocate it		  */
			tcb_max += TCBINCR;
			tcb_table = (struct tcbent **)
				xrealloc (tcb_table,
					tcb_max * sizeof (struct tcbent *));
		} 
		if (!tcb_table) /* If we have invalid ptr, error out	  */
			fatal (Out_Of_Memory, 0, ENOMEM);
	}
	tcb_table[tcb_cnt++] = tcbent;

	/*
	 * Add the file and its links to the hash table.  Each hard
	 * and soft link is added as a key with the table entry as
	 * the data.  The link name may not exist in the table already!
	 */

	hash_add (file, tcbent);

	if (links) {
		for (i = 0;links[i];i++) { 
			if (tp = (struct tcbent *) hash_find (links[i])) {
				fmsg3 (Duplicate_Name, links[i],
					file, tp->tcb_name);
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					SYSCK_Cfg, CantFix,
					Duplicate_Name, links[i], file, NULL);
				tcbent->tcb_valid = 0;
			} else
				hash_add (links[i], tcbent);
		}
	}

	/*
	** If this is a symlink, add it to
	** the symlist linked list so we
	** can efficiently check a given
	** tcbent for symlinks to it.
	*/
	if (tcbent->tcb_type==TCB_SYMLINK) {
		symrec=(struct symlink_rec *)
			xmalloc(sizeof(struct symlink_rec));
		if (!symrec)
			fatal(Out_Of_Memory,0,ENOMEM);
		/*
		** Initialize the entry
		*/
		symrec->tcbent=tcbent;

		/*
		** Got it filled out so add this one to the head of the
		** list.  Note that this works if symlinks is NULL 
		** or not (i.e. if there is nothing already on the 
		** list or not).
		*/
		symrec->next=symlist;
		symlist=symrec;
	}

	return tcbent->tcb_valid ? 0:ENOTRUST;
}

/*
 * NAME: list_member
 *
 * FUNCTION: Check a string for membership in a list of strings
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUES:
 *	Non-zero if the string is a member of the supplied list, else zero
 */

int
#ifdef	_NO_PROTO
list_member (string, list)
char	*string;
char	**list;
#else
list_member (
char	*string,
char	**list)
#endif
{
	/*
	 * If list is completely empty, return FALSE
	 */

	if (list == 0)
		return 0;

	/*
	 * Otherwise, scan list and return TRUE as soon as
	 * a match is found.
	 */

	while (*list)
		if (strcmp (string, *list++) == 0)
			return 1;

	/*
	 * No match found, return FALSE
	 */

	return 0;
}

/*
 * NAME: check_tcb
 *                                                                    
 * FUNCTION: Check the entire TCB or selected files only
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	check_tcb() scans each entry in the TCB table to see if it is
 *	to be tested.  If there are no arguments, all of the entries
 *	are tested.  Otherwise each argument is checked against to
 *	see if the table entries match on of the arguments.  An
 *	argument may be either a class [ no leading / ] or a file
 *	name [ leading / ].
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
check_tcb (argv)
char	**argv;
{
	char	**argp;
	char	key[16];
	char	last[BUFSIZ];
	char	*cp;
	int	i, j;
	int	errors = 0;
	struct	tcbent	*tcbent;
	struct	stat	statbuf;
	int	*found;

	/*
	 * Count the arguments from the user.  Set up an array
	 * of flags so we can see which arguments weren't
	 * satisfied with entries from the TCB.
	 */

	for (i = 0, argp = argv;*argp;argp++)
		i++;

	found = (int *) xmalloc (i * sizeof (int));
	last[0] = '\0';

	/*
	 * Read in the entire TCB file.  Entries are tested for
	 * validity before being added.  Every link, source
	 * file and program name must be an absolute pathname.
	 *
	 * The arguments are tested, unless 'all' is non-zero,
	 * to see if they match the TCB entry in either the
	 * class or filename.
	 */

	while (1) {

		/*
		 * Get the next entry out of the database.  If
		 * NULL is returned we must see if errno was set
		 * or not.  On EOF errno is set to zero, otherwise
		 * it is an error.
		 */

		if (! (tcbent = gettcbent ())) {
			if (errno == 0)
				break;

			msg (Database_Error);
			mk_vaudit (SYSCK_Check, AUDIT_FAIL,
				SYSCK_Cfg, CantFix, Database_Error, NULL);

			if (last[0]) {
				msg1 (Last_Stanza, last);
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					SYSCK_Cfg, CantFix,
					Last_Stanza, last, NULL);
			} else {
				msg (No_Last_Stanza);
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					SYSCK_Cfg, CantFix,
					No_Last_Stanza, NULL);
			}
			continue;
		}
		strncpy (last, tcbent->tcb_name, sizeof last);
		last[sizeof last - 1] = '\0';

		if (tcbent->tcb_valid == 0) {
			errors++;
			continue;
		}

		/* Absolute Pathname */

		if (tcbent->tcb_name[0] != '/') {
			if (all || tree) {
				fmsg1 (Illegal_Entry, tcbent->tcb_name);
				mk_vaudit (SYSCK_Check, AUDIT_FAIL,
					SYSCK_Cfg, CantFix,
					Illegal_Entry, tcbent->tcb_name, NULL);
			}
			free (tcbent);
			errors++;
			continue;
		}

		/*
		 * Short circuit test for all files being tested.
		 */

		if (all || tree) {
			if (add_tcb_table (tcbent))
				errors++;
			else
				tcbent->tcb_test = 1;

			continue;
		}

		/*
		 * Set the test flag to FALSE and walk the entire
		 * list of arguments.
		 */

		tcbent->tcb_test = 0;

		for (argp = argv;*argp;argp++) {

		/*
		 * See if entry is to be added to the set of entries
		 * to be tested.
		 */

			if (*argp[0] != '/') {
				if (list_member (*argp, tcbent->tcb_class)) {
					tcbent->tcb_test = 1;
					found[argp - argv] = 1;
					break;
				}
			} else {
				if (strcmp (*argp, tcbent->tcb_name) == 0) {
					tcbent->tcb_test = 1;
					found[argp - argv] = 1;
					break;
				}
			}
		}
		if (add_tcb_table (tcbent))
			errors++;
	}

	/*
	 * Now go back and see which arguments didn't get satisfied
	 * from the database.
	 */

	for (i = j = 0;argv[i] != (char *) 0;i++) {
		if (found[i] == 0) {
			msg1 (argv[i][0] == '/' ?
				 No_File_Matched:No_Class_Matched, argv[i]);
			j++;
		}
	}
	if (j)
		return ENOENT;

	/*
	 * ck_tcbent() is now called on each entry in the TCB file
	 * which was selected for testing.
	 */

	for (i = 0;i < tcb_cnt;i++) {
		/*
		** if we don't need to test it or
		** it isn't valid, skip it
		*/
		if (! tcb_table[i]->tcb_test || ! tcb_table[i]->tcb_valid)
			continue;

		/*
		** ck_tcbent checks one entry in the tcb_table
		** for many different possible problems.
		*/
		if (ck_tcbent (tcb_table[i]))
			errors++;

		/*
		** Save the device and i-number pair in case the user
		** has some files with weird links that need resolving.
		** This information is very useful in tree checking
		** to reverse map <device,inumber> to a file name.
		** The <device,inumber> must be unique, unless the entry
		** refers to a multiplexed device.
		*/
		if (stat (tcb_table[i]->tcb_name, &statbuf) == 0) {
			sprintf (key,"%d,%d",statbuf.st_dev,statbuf.st_ino);
			cp = xstrdup (key);

			/*
			** Try to add the entry to the hash table
			*/
			if ((tcbent=(struct tcbent *)hash_find(cp))==0)
				/*
				** Nothing there, go ahead and add it.
				*/
				hash_add (cp, tcb_table[i]);

			/*
			** Else, found something there, already.  If what we
			** found in the hash table is an MPX device or the 
			** entry we are processing is a hard or soft link,
			** don't worry about it - otherwise, it's an error.
			*/
			else if (tcbent->tcb_type != TCB_MPX &&
				 tcb_table[i]->tcb_type != TCB_SYMLINK &&
				 tcbent->tcb_type != TCB_SYMLINK) {

 				/*
 				** Since we can't fix this error, we need
 				** to return an error code.
				*/
 				errors++;

 				/*
 				** If user specified -p, don't issue a
				** message; otherwise, do so.
 				*/
 				msg2(Duplicate_Object,
					tcbent->tcb_name,
 					tcb_table[i]->tcb_name);
			}
		}
	}

	/*
	 * Return ENOTRUST for all errors not previously taken care of
	 */

	return errors ? ENOTRUST:0;
}
