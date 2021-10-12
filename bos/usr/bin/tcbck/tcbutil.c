static char sccsid[] = "@(#)57	1.1.1.2  src/bos/usr/bin/tcbck/tcbutil.c, cmdsadm, bos411, 9428A410j 3/6/94 16:12:21";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: sig_ignore, sig_reset, isnumeric, xperror, xmalloc, xrealloc,
 *		xstrdup, xchown, xlink, xmkdir, xmknod,
 *		xsymlink, xunlink, xremove, comma_list, null_list,
 *		get_program, put_program, validate_name,
 *		mk_checksum, fuzzy_compare, free_list
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
#include <sys/fullstat.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/sysmacros.h>
#include <sys/priv.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <varargs.h>
#include "tcbdbio.h"
#include "tcbmsg.h"

void	*signals[NSIG];
int	once;

extern	char	*malloc(unsigned);
extern	char	*realloc(void *, unsigned);

void	msg(char *);
void	msg1(char *, char *);
void	msg2(char *, char *, char *);
void	fatal(char *, char *, int);

/*
 * NAME: sig_ignore
 *                                                                    
 * FUNCTION: Set user creatable signals to SIG_IGN.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: NONE
 */  

void
sig_ignore (void)
{
	once++;

	signals[SIGHUP] = (void *) signal (SIGHUP, SIG_IGN);
	signals[SIGINT] = (void *) signal (SIGINT, SIG_IGN);
	signals[SIGQUIT] = (void *) signal (SIGQUIT, SIG_IGN);
	signals[SIGTERM] = (void *) signal (SIGTERM, SIG_IGN);
	signals[SIGTSTP] = (void *) signal (SIGTSTP, SIG_IGN);
}

/*
 * NAME: sig_reset
 *                                                                    
 * FUNCTION: Reset user createable signals to their previous values
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: NONE
 */  

void
sig_reset (void)
{
	if (! once)
		return;

	signal (SIGHUP, (void (*) (int)) signals[SIGHUP]);
	signal (SIGINT, (void (*) (int)) signals[SIGINT]);
	signal (SIGQUIT, (void (*) (int)) signals[SIGQUIT]);
	signal (SIGTERM, (void (*) (int)) signals[SIGTERM]);
	signal (SIGTSTP, (void (*) (int)) signals[SIGTSTP]);

	once = 0;
}

/*
 * NAME: isnumeric
 *                                                                    
 * FUNCTION: Determine if a string is all digits
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero if there are any non-numeric characters, non-zero otherwise
 */  

int
isnumeric (char *str)
{
	char	*cp;

	if (str == 0 || *str == '\0')
		return 0;

	for (cp = str;*cp;cp++)
		if (! isdigit (*cp))
			return 0;

	return 1;
}

/*
 * NAME: xperror
 *                                                                    
 * FUNCTION: variable argument list perror()
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: NONE
 */  

void
xperror (va_alist)
va_dcl
{
	va_list	ap;
	char	buf[BUFSIZ];
	char	*cp;

	va_start (ap);

	strcpy (buf, va_arg (ap, char *));
	strcat (buf, "(");

	if (cp = va_arg (ap, char *)) {
		strcat (buf, cp);
		while (cp = va_arg (ap, char *)) {
			strcat (buf, ", ");
			strcat (buf, cp);
		}
	}
	strcat (buf, ")");

	perror (buf);
}

/*
 * NAME: xmalloc
 *                                                                    
 * FUNCTION: malloc() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Pointer to region
 */  

char *
xmalloc (int len)
{
	char	*cp;

	if (len == 0)
		len = 4;

	if (cp = malloc ((unsigned) len)) {
		memset ((void *) cp, 0, len);
		return (cp);
	}
	fatal (Out_Of_Memory, 0, ENOMEM);
	/*NOTREACHED*/
}

/*
 * NAME: xrealloc
 *                                                                    
 * FUNCTION: realloc() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Pointer to new region
 */  

char *
xrealloc (char *buf, int len)
{
	char	*cp;

	if (cp = realloc (buf, len))
		return (cp);

	fatal (Out_Of_Memory, 0, ENOMEM);
	/*NOTREACHED*/
}

/*
 * NAME: xstrdup
 *                                                                    
 * FUNCTION: Duplicate a string into a new buffer with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Pointer to string
 */  

char *
xstrdup (char *s)
{
	char	*cp;

	cp = xmalloc (strlen (s) + 1);
	return (strcpy (cp, s), cp);
}

/*
 * NAME: xchown
 *                                                                    
 * FUNCTION: xchown() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xchown (char *file, uid_t owner, gid_t group)
{
	char	sown[16];
	char	sgrp[16];

	if (chown (file, owner, group)) {
		sprintf (sown, "%d", owner);
		sprintf (sgrp, "%d", group);

		xperror ("chown", file, sown, sgrp, 0);
		return -1;
	}
	return 0;
}


/*
 * NAME: xlchown
 *                                                                    
 * FUNCTION: lchown() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xlchown (char *file, uid_t owner, gid_t group)
{
	char	sown[16];
	char	sgrp[16];

	if (lchown (file, owner, group)) {
		sprintf (sown, "%d", owner);
		sprintf (sgrp, "%d", group);

		xperror ("lchown", file, sown, sgrp, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xlink
 *                                                                    
 * FUNCTION: xlink() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xlink (char *from, char *to)
{
	if (link (from, to)) {
		xperror ("link", from, to, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xmkdir
 *                                                                    
 * FUNCTION: mkdir() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xmkdir (char *dir, int mode)
{
	char	buf[BUFSIZ];

	if (mkdir (dir, mode)) {
		sprintf (buf, "0%o", mode);
		xperror ("mkdir", dir, buf, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xmknod
 *                                                                    
 * FUNCTION: xmknod() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xmknod (char *file, int mode, dev_t dev)
{
	char	smode[16];
	char	sdev[16];

	if (mknod (file, mode, dev)) {
		sprintf (smode, "0%o", mode);
		sprintf (sdev, "<%d,%d>", major (dev), minor (dev));
		xperror ("mknod", file, smode,
			(S_ISBLK (mode) || S_ISCHR (mode)) ? sdev:0, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xsymlink
 *                                                                    
 * FUNCTION: symlink() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xsymlink (char *file, char *link)
{
	if (symlink (file, link)) {
		xperror ("symlink", file, link, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xunlink
 *                                                                    
 * FUNCTION: unlink() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xunlink (char *file)
{
	if (unlink (file)) {
		xperror ("unlink", file, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xremove
 *                                                                    
 * FUNCTION: Remove various types of files, devices and directories
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xremove (char *name, struct fullstat *stat)
{
	if (S_ISDIR (stat->st_mode)) {
		if (rmdir (name)) {
			xperror ("rmdir", name, 0);
			return -1;
		}
	} else {
		if (unlink (name)) {
			xperror ("unlink", name, 0);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME:	comma2null
 *
 * FUNCTION:	Convert comma separated list into null separated list
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to double-null terminated list
 */

char *
comma2null (char *arg)
{
	char	*cp, *cp2;

	/*
	 * Duplicate the string so it can be modified in place.  The
	 * commas are replaced with NUL characters.
	 */

	cp = xmalloc (strlen (arg) + 2);
	memset (cp, 0, strlen (arg) + 2);
	strcpy (cp, arg);

	for (cp2 = cp;*cp2;cp2++)
		if (*cp2 == ',')
			*cp2 = '\0';

	return cp;
}

/*
 * NAME:	comma_list
 *
 * FUNCTION:	Convert comma separated list into an array of strings
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to array of allocated characters strings
 */

char **
comma_list (char *arg)
{
	char	*cp, *cp2;
	int	i;
	char	**list;

	/*
	 * Duplicate the string so it can be modified in place.  The
	 * commas are replaced with NUL characters.  Allocate a list
	 * of pointers for each [ plus the terminator ] pointer.
	 */

	cp = xstrdup (arg);

	for (cp2 = cp, i = 1;*cp2;cp2++)
		if (*cp2 == ',')
			i++;

	list = (char **) calloc (i + 1, sizeof (char *));

	/*
	 * Find each element in the string, point to it in the table
	 * and then NUL terminate.  The list itself must then be
	 * terminated with a (char *) 0.
	 */

	for (cp2 = cp, i = 0;*cp2;i++) {
		list[i] = cp2;

		while (*cp2 && *cp2 != ',')
			cp2++;

		if (*cp2)
			*cp2++ = '\0';
	}
	list[i] = 0;

	return list;
}

/*
 * NAME:	null_list
 *
 * FUNCTION:	Convert NUL separated list into an array of strings
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to array of allocated characters strings
 */

char **
null_list (char	*arg)
{
	char	*cp, *cp2;
	int	i;
	char	**list;

	/*
	 * Duplicate the string so it can be modified in place.  The
	 * commas are replaced with NUL characters.  Allocate a list
	 * of pointers for each [ plus the terminator ] pointer.
	 */

	for (i = 0, cp = arg;cp[0] || cp[1];cp++, i++)
		;

	cp = xmalloc (i + 2);
	memset (cp, 0, i + 2);
	memcpy (cp, arg, i);

	/*
	 * Count the number of strings in the list so an array of
	 * pointers can be allocated for it.
	 */

	for (cp2 = cp, i = 0;*cp2;) {
		i++;

		while (*cp2++)
			;
	}

	list = (char **) calloc (i + 1, sizeof (char *));

	/*
	 * Find each element in the string and point to the beginning
	 * of it.  NULL terminate the entire list when done.
	 */

	for (cp2 = cp, i = 0;*cp2;i++) {
		list[i] = cp2;

		while (*cp2++)
			;
	}
	list[i] = 0;

	return list;
}

/*
 * NAME:	get_program
 *
 * FUNCTION:	Execute a program and return its output
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to output string or NULL on error.
 */

char *
get_program (char *prog, char *arg)
{
	char	buf[MAXTCBSIZ];
	FILE	*fp;
	int	i;
	int	rc;

	/*
	 * Create the command line by appending the argument to
	 * the pathname which was provided in prog.
	 */

	if (! arg)
		strcpy (buf, prog);
	else
		sprintf (buf, "%s %s", prog, arg);

	/*
	 * Execute the program, and pipe the results back in to
	 * the buffer.  Report read or execution errors.
	 */

	if (! (fp = popen (buf, "r"))) {
		msg1 (Program_Error, prog);
		return 0;
	}
	i = fread (buf, 1, sizeof buf - 1, fp);
	if (i < 0 || ferror (fp)) {
		msg1 (Program_Error, prog);
		return 0;
	}

	/*
	 * Close the pipeline and NUL terminate the string.  Pass
	 * back a malloc'd version to the caller.
	 */

	if (pclose (fp))
		msg1 (Program_Error, prog);

	if (buf[i - 1] == '\n')
		buf[i - 1] = '\0';
	else
		buf[i] = '\0';

	return xstrdup (buf);
}

/*
 * NAME:	put_program
 *
 * FUNCTION:	Execute a program piping it's input from a buffer
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Exit status from executed command.
 */

int
put_program (char *prog, char *arg, char *input)
{
	FILE	*fp;
	int	i;
	int	rc;
	char	buf[BUFSIZ];

	/*
	 * Create the command line by appending the argument to
	 * the pathname which was provided in prog.
	 */

	if (! arg)
		strcpy (buf, prog);
	else
		sprintf (buf, "%s %s", prog, arg);

	/*
	 * Execute the program, and pipe the input from the user's
	 * buffer.  Report write or execution errors.
	 */

	if (! (fp = popen (buf, "w"))) {
		msg1 (Program_Error, prog);
		return -1;
	}
	while (*input && ! ferror (fp))
		putc (*input++, fp);

	if (! ferror (fp)) {
		putc ('\n', fp);
		fflush (fp);
		if (rc = pclose (fp))
			msg1 (Program_Error, prog);

		return rc;
	} else {
		msg1 (Program_Error, prog);
		(void) pclose (fp);
		return -1;
	}
}

/*
 * NAME: priv_get
 *                                                                    
 * FUNCTION: Return a pointer to a PCL for a file.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Performs similiar function to acl_get, except priv_get isn't
 *	in the C library.
 *
 * RETURNS: Pointer to malloc()'d PCL or NULL on error.
 */  

struct pcl *
priv_get (char *file)
{
	int	length;
	struct	pcl	*pcl;

	for (length = sizeof *pcl;;) {
		if (! (pcl = (struct pcl *) xmalloc (length)))
			return 0;

		if (statpriv (file, 0, pcl, length)) {
			if (errno != ENOSPC) {
				free (pcl);
				return 0;
			}
			length = pcl->pcl_len;
			free (pcl);
			continue;
		} else
			break;
	}
	return pcl;
}

/*
 * NAME: priv_put
 *                                                                    
 * FUNCTION: Apply a PCL to a named file.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Performs similiar function to acl_put, except priv_put isn't
 *	in the C library.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
priv_put (char *file, struct pcl *pcl, int flag)
{
	int	rc;

	rc = chpriv (file, pcl, pcl->pcl_len);

	if (flag)
		free (pcl);

	return rc;
}

/*
 * NAME: validate_name
 *                                                                    
 * FUNCTION: Test a name for containing special character
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
validate_name (char *name)
{
	return strpbrk (name, "~`'\"$^&()\\|{}[]<>?") != 0;
}

/*
 * NAME: mk_checksum
 *                                                                    
 * FUNCTION: Sum the bytes in a file
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Reads a file byte-by-byte summing each byte as it goes.
 *
 * RETURNS: Zero on success, non-zero otherwise
 */  

unsigned long
mk_checksum (char *file)
{
	int	c;
	unsigned tmp = 0;
	FILE	*fp;

	if (! (fp = fopen (file, "r")))
		return (unsigned) -1;

	while ((c = getc (fp)) != EOF) {
		if (tmp & 01)
			tmp = ((tmp >> 1) + 0x8000);
		else
			tmp >>= 1;

		tmp += c;
		tmp &= 0xffff;
	}
	fclose (fp);
	return tmp;
}

/*
 * NAME: mk_sum
 *
 * FUNCTION: perform identical processing to "/bin/sum -r" command
 *
 * NOTE:
 *
 *	mk_sum() is called if BuiltInSum is TRUE and the user needs
 *	to checksum a file.  It will invoke mk_checksum, and save
 *	away the checksum value.
 *
 * RETURNS: pointer to static character array with string, or NULL
 *	on error.
 */

char *
mk_sum (struct tcbent *tcb)
{
	static	char	checksum[20];
	struct	stat	sb;

	/*
	 * Compute the checksum, save it away, and set the flag
	 * that say's we've been here before.
	 */

	tcb->tcb_sum = mk_checksum (tcb->tcb_name);
	if (tcb->tcb_sum == (unsigned) -1) {
		msg1 (Program_Error, "sum");
		return 0;
	}
	tcb->tcb_vsum = 1;

	/*
	 * Stat the file to get the size.  Take the sum and size
	 * and plaster them together the same as what /bin/sum -r
	 * does.
	 */

	if (stat (tcb->tcb_name, &sb))
		return 0;

	sprintf (checksum, "%05u%6ld ",
		tcb->tcb_sum, (sb.st_size + 1023) / 1024); 

	return checksum;
}

/*
 * NAME: fuzzy_compare
 *                                                                    
 * FUNCTION: Compare two strings, ignore differences in whitespace
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Similiar to strcmp, except differences in the amount and
 *	type of whitespace are ignored.  Strings must match
 *	byte-for-byte otherwise.
 *
 * RETURNS: Zero on match, non-zero otherwise
 */  

int
fuzzy_compare (char *a, char *b)
{
	int	newline = 1;

	/*
	 * Scan both strings, looking for a difference, or the
	 * end of both strings ...
	 */

	while (*a && *b) {

		/*
		 * At the beginning of a line, eat whitespace in both
		 * strings.
		 */

		if (newline) {
			while (*a == ' ' || *a == '\t')
				a++;

			while (*b == ' ' || *b == '\t')
				b++;

			newline = 0;
		}

		/*
		 * Whitespace matches if both strings have whitespace
		 * at the same general location in the string.  Newlines
		 * are considered to be whitespace, to some extent -
		 * but you can't have more consecutive newlines in one
		 * string than another.  The intent is simply to gobble
		 * up trailing blanks.
		 */

		if ((*a == ' ' || *a == '\t' || *a == '\n')
				&& (*b == ' ' || *b == '\t' || *b == '\n')) {
			while (*a == ' ' || *a == '\t')
				a++;

			while (*b == ' ' || *b == '\t')
				b++;

			if (*a != '\n' && *b != '\n')
				continue;

			newline = 1;
			a++, b++;
			continue;
		}

		/*
		 * Now each byte must match.  This is just a regular
		 * strings compare at this time
		 */

		if (*a++ != *b++)
			return -1;
	}

	/*
	 * One or both of the strings have terminated.  If "a" is done,
	 * strip any whitespace in this position off of "b".  Do the
	 * same for "b".  This makes trailing whitespace insignificant.
	 *
	 * Once that's over and done with, return 0 if both strings have
	 * been scanned completely.
	 */


	if (*a == '\0' && (*b == ' ' || *b == '\t')) {
		while (*b == ' ' || *b == '\t')
			b++;
	}
	if (*b == '\0' && (*a == ' ' || *a == '\t')) {
		while (*a == ' ' || *a == '\t')
			a++;
	}
	return *a != '\0' || *b != '\0';
}

/*
 * NAME: free_list
 *
 * FUNCTION: free a NULL-terminated list of strings
 *
 * RETURNS: NONE
 */

void
free_list (char **list)
{
	char	**org = list;

	while (*list){
		free (*list);
		list ++;
	}

	free (org);
}

/*
 * NAME: list_null
 *
 * FUNCTION: convert a list of strings to NUL-separate format
 *
 * RETURNS: Address of buffer, or NULL on error.
 */

char *
list_null (char **list, char *string, int i)
{
	char	*org = string;
	char	*cp = string;
	int	len;

	while (*list) {
		if ((len = strlen (*list)) > i + 2)
			return 0;

		strcpy (cp, *list++);
		cp += len;
		*cp++ = '\0';
		i -= (len + 1);
	}
	*cp = '\0';

	return org;
}
