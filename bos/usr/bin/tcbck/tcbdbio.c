static char sccsid[] = "@(#)49	1.3.1.1  src/bos/usr/bin/tcbck/tcbdbio.c, cmdsadm, bos411, 9428A410j 3/6/94 16:05:59";
/*
 * COMPONENT_NAME: (CMDSADM) sysck - sysck configuration checker
 *
 * FUNCTIONS: __gettcbnam, gettcbnam, gettcbent,
 *		puttcbent, gettcbstanza
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

#include <sys/types.h>
#include <sys/access.h>
#include <sys/stat.h>
#include <sys/audit.h>
#include <sys/fullstat.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <ctype.h>
#include <usersec.h>
#include "tcbattr.h"
#include "tcbdbio.h"
#include "tcbmsg.h"

/*
 * A macro defining the security relevant mode bits
 */

#define	ALL_MODES (S_ITCB|S_ITP|S_ISUID|S_ISGID|S_ISVTX|0777)

#ifdef	_NO_PROTO
extern	char	*xstrdup ();
extern	char	**null_list ();
extern	char	*list_null ();
extern	char	*get_program ();
extern	char	*mk_sum ();
extern	struct	group	*getgrnam ();
extern	struct	group	*getgrgid ();
extern	struct	passwd	*getpwnam ();
extern	struct	passwd	*getpwuid ();
#else
extern	char	*xstrdup (char *);
extern	char	**null_list (char *);
extern	char	*list_null (char **, char *, int);
extern	char	*get_program (char *, char *);
extern	char	*mk_sum (struct tcbent *);
extern	struct	group	*getgrnam (char *);
extern	struct	group	*getgrgid (gid_t);
extern	struct	passwd	*getpwnam (char *);
extern	struct	passwd	*getpwuid (uid_t);
#endif
extern	char	*Aclget;
extern	char	*Pclget;
extern	char	*Checksum;

extern	int	dflg;

/*
 * NAME:	xputtcbattr
 *
 * FUNCTION:	Wrapper for puttcbattr
 *
 * RETURNS: return value from puttcbattr
 */

int
#ifdef	_NO_PROTO
xputtcbattr (stz, atr, val, typ)
char	*stz;
char	*atr;
void	*val;
int	typ;
#else
xputtcbattr (char *stz,
	char	*atr,
	void	*val,
	int	typ)
#endif
{
	char	*cp;
	int	i;

	switch (typ) {
		case SEC_INT:
			if (val == (void *) -1) {
				return puttcbattr (stz, atr, 0, SEC_DELETE);
			} else {
				return puttcbattr (stz, atr, val, SEC_INT);
			}
		case SEC_CHAR:
		case SEC_LIST:
			if (val == 0) {
				return puttcbattr (stz, atr, 0, SEC_DELETE);
			} else {
				return puttcbattr (stz, atr, val, typ);
			}
		case SEC_DELETE:
		case SEC_COMMIT:
			return puttcbattr (stz, atr, 0, typ);
	}
	return -1;
}

/*
 * NAME:	init_tcbent
 *
 * FUNCTION:	Initialize members of struct tcbent
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User Process
 *
 * RETURNS: NONE
 */

void
#ifdef	_NO_PROTO
init_tcbent (tcb)
struct	tcbent	*tcb;
#else
init_tcbent (struct tcbent *tcb)
#endif
{
	static	struct	tcbent	zero = {
		(char *) 0,	(char **) 0,	(tcb_type_t) -1, (uid_t) -1,
		(gid_t) -1,	(char *) 0,	(char *) 0,	(char *) 0,
		(char **) 0,	(char **) 0,	(char **) 0,	(char *) 0,
		(char *) 0,		-1,		-1,		0,
			0, 		0,		0,		0,
			0, 		0
	};

	*tcb = zero;
}

/*
 * NAME:	__gettcbnam
 *
 * FUNCTION:	Get or create a tcbent structure by name
 *
 * NOTES:
 *	The flag create_tcbent is used to indicate that a new
 *	tcbent is to be created if the named tcbent does not
 *	exist.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Called only by gettcbnam and gettcbent.
 *
 * RETURNS:	NULL on failure, and a tcbent structure on success
 */

static	struct tcbent *
#ifdef	_NO_PROTO
__gettcbnam (name, create_tcbent)
char	*name;		/* name of stanza to find or create                 */
int	create_tcbent;	/* return a structure with no initialized values    */
#else
__gettcbnam (char *name,
	int	create_tcbent)
#endif
{
	char	*cp;
	char	*cp2;
	int	i;
	int	nattr = 0; /* count of attributes which were found          */
	int	errors = 0;
	struct	tcbent	tcbent;
	struct	tcbent	*ntcbent;
	struct	passwd	*pw;
	struct	group	*gr;

	/*
	 * Initialize a scratch TCB entry and copy in the name of the
	 * file.  The name may not contain any special characters.
	 */

	init_tcbent (&tcbent);

	if (validate_name (name)) {
		fmsg1 (Illegal_Entry, name);
		errors++;
	}
	if (! (tcbent.tcb_name = xstrdup (name)))
		return 0;

	/*
	 * Each attribute for the stanza is fetched from the
	 * configuration file and copied into the user structure.
	 * Missing entries will be initialized to some illegal
	 * value.  For character pointers it is NULL, for numeric
	 * values it is -1.
	 */

	/* Get the "type" attribute */

	if (gettcbattr (tcbent.tcb_name, "type", &cp, 0) == 0) {
		if (! strcmp (cp, "FILE"))
			i = TCB_FILE;
		else if (! strcmp (cp, "DIRECTORY"))
			i = TCB_DIR;
		else if (! strcmp (cp, "BLK_DEV"))
			i = TCB_BLK;
		else if (! strcmp (cp, "CHAR_DEV"))
			i = TCB_CHAR;
		else if (! strcmp (cp, "MPX_DEV"))
			i = TCB_MPX;
		else if (! strcmp (cp, "FIFO"))
			i = TCB_FIFO;
                else if (! strcmp (cp, "SYMLINK"))
                        i = TCB_SYMLINK;
                else if (! strcmp (cp, "LINK"))
                        i = TCB_LINK;
		else {
			i = -1;
			fmsg2 (Illegal_Type, tcbent.tcb_name, cp);
			errors++;
		}
		tcbent.tcb_type = i;
		nattr++;
	}

	/* Get the "class" attribute */

	if (gettcbattr (tcbent.tcb_name, "class", &cp, 0) == 0) {
		tcbent.tcb_class = null_list (cp);
		nattr++;
	}

	/* Get the "owner" attribute */

	if (gettcbattr (tcbent.tcb_name, "owner", &cp, 0) == 0) {
		if (! isnumeric (cp)) {
			if (pw = getpwnam (cp)) {
				tcbent.tcb_owner = pw->pw_uid;
				nattr++;
			} else {
				fmsg1 (Unknown_User, cp);
				errors++;
			}
		} else {
		 	tcbent.tcb_owner = atoi (cp);
			nattr++;
		}
	}

	/* Get the file group value */

	if (gettcbattr (tcbent.tcb_name, "group", &cp, 0) == 0) {
		if (! isnumeric (cp)) {
			if (gr = getgrnam (cp)) {
				tcbent.tcb_group = gr->gr_gid;
				nattr++;
			} else {
				fmsg1 (Unknown_Group, cp);
				errors++;
			}
		} else {
			tcbent.tcb_group = atoi (cp);
			nattr++;
		}
	}

	/* Get the file ACL value */

	if (gettcbattr (tcbent.tcb_name, "acl", &cp, 0) == 0) {
		tcbent.tcb_acl = xstrdup (cp);
		nattr++;
	}

	/* Get the "pcl" attribute */

	if (gettcbattr (tcbent.tcb_name, "pcl", &cp, 0) == 0) {
		tcbent.tcb_pcl = xstrdup (cp);
		nattr++;
	}

	/*
	 * Get the "mode" attribute
	 *
	 * The mode is a 9 character string which must be converted
	 * to the 12 bit equivalent.
	 */

	if (gettcbattr (tcbent.tcb_name, "mode", &cp, SEC_LIST) == 0) {
		if (string2mode (cp, &tcbent.tcb_mode, tcbent.tcb_name))
			errors++;
	}

	/*
	 * Get the "links" attribute
	 *
	 * The links attribute is a double-null terminated list of
	 * hard links names
	 */

	if (gettcbattr (tcbent.tcb_name, "links", &cp, 0) == 0) {
		tcbent.tcb_links = null_list (cp);
		nattr++;
	}

	/*
	 * Get the "symlinks" attribute
	 *
	 * The "symlinks" attribute is a double-null terminated list
	 * of symbolic links
	 */

	if (gettcbattr (tcbent.tcb_name, "symlinks", &cp, 0) == 0) {
		tcbent.tcb_symlinks = null_list (cp);
		nattr++;
	}

	/* Get the "checksum" attribute */
	 
	if (gettcbattr (tcbent.tcb_name, "checksum", &cp, 0) == 0) {
		tcbent.tcb_checksum = xstrdup (cp);
		nattr++;
	}
	if (! strcmp (tcbent.tcb_checksum, "VOLATILE"))
		tcbent.tcb_vchecksum = 1;
	
	/* Get the "program" attribute */

	if (gettcbattr (tcbent.tcb_name, "program", &cp, 0) == 0) {
		tcbent.tcb_program = null_list (cp);
		nattr++;
	}
	
	/* Get the "source" attribute */

	if (gettcbattr (tcbent.tcb_name, "source", &cp, 0) == 0) {
		tcbent.tcb_source = xstrdup (cp);
		nattr++;
	}

	/* Get the "size" attribute */

	errno = 0;

	if (gettcbattr (tcbent.tcb_name, "size", &tcbent.tcb_size, SEC_INT)) {
		tcbent.tcb_size = -1;

		if (errno == ENOTRUST) {
			fmsg2 (Invalid_Attribute, "size", tcbent.tcb_name);
			errors++;
		}
	} else
		nattr++;

        /*
         * Get the "target" attribute.  "target" is a fullpath name
         * for a symbolic link or hard link target.  This attribute
         * requires a value.
         * ex. /tmp/foo -> /tmp/bar  (bar is the target)
         */

	if (gettcbattr (tcbent.tcb_name, "target", &cp, 0) == 0) {
		tcbent.tcb_target = xstrdup (cp);
		nattr++;
	}

	/*
	 * If the create_tcbent flag is false and no attributes were
	 * added, return NULL.
	 */

	if (! create_tcbent && ! nattr)
		return 0;

	/*
	 * Multiplexed devices have the SVTX bit set.  The mode needs
	 * to specify this.  It is stripped out on writing, and added
	 * on reading.
	 */

	if (tcbent.tcb_type == TCB_MPX && tcbent.tcb_mode != -1)
		tcbent.tcb_mode |= S_ISVTX;

	/*
	 * Mark the entry as valid and return to the caller in a
	 * malloc()'d buffer
	 */

	tcbent.tcb_valid = errors ? 0:1;
	tcbent.tcb_changed = 0;

	ntcbent = (struct tcbent *) malloc (sizeof (struct tcbent));
	*ntcbent = tcbent;

	if (! tcbent.tcb_valid)
		fmsg1 (Illegal_Entry, tcbent.tcb_name);

	return ntcbent;
}

/*
 * NAME:	gettcbnam
 *
 * FUNCTION:	Get a tcbent structure by name
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS:	Pointer to named structure, or NULL if entry does not exist
 */

struct tcbent *
#ifdef	_NO_PROTO
gettcbnam (name)
char	*name;
#else
gettcbnam (char *name)
#endif
{
	return __gettcbnam (name, 0);
}

/*
 * NAME:	gettcbent
 *
 * FUNCTION:	Get the next tcbent member in the configuration file
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS:	Pointer to next entry, or NULL if no entries remaining.
 */

struct tcbent *
gettcbent ()
{
	struct	tcbent	*tcbent;
	char	*cp;

	/*
	 * Reserved stanzas do not have leading '/' characters and
	 * must be skipped over.  Once the name of a new stanza is
	 * found we can fetch it by name!  Some stanza may contain
	 * no attributes so we ask __gettcbnam to create an empty
	 * entry if needed.
	 */

	while (1) {
		if (gettcbattr (0, 0, &cp, 0))
			return 0;

		if (*cp == '/')
			break;
	}
	return __gettcbnam (cp, 1);
}

/*
 * NAME:	puttcbent
 *
 * FUNCTION:	Output a tcbent structure updating the configuration file
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * RETURNS:
 *	Zero on success, non-zero on failure
 */

int
#ifdef	_NO_PROTO
puttcbent (tcbent)
struct	tcbent	*tcbent;
#else
puttcbent (struct tcbent *tcbent)
#endif
{
	char	buf[BUFSIZ];
	char	*name;
	char	*cp;
	char	*cp2;
	int	i;
	struct	passwd	*pw;
	struct	group	*gr;

	/*
	 * Because the tcb_mode may contain SVTX for a multiplexed
	 * device, and we don't want people screwing with it, we
	 * are going to TURN OFF that bit if the mode is defined.
	 */

	if (tcbent->tcb_type == TCB_MPX && tcbent->tcb_mode != -1)
		tcbent->tcb_mode &= ~S_ISVTX;

	/*
	 * Pointer must be non-NULL and the entry must be flagged as
	 * valid and the "type" attribute must be present
	 */

	if (! tcbent || tcbent->tcb_valid != 1 || tcbent->tcb_type == -1)
		return ENOTRUST;

	if (! tcbent->tcb_changed)
		return 0;

	name = tcbent->tcb_name;
	cp = 0;

	/*
	 * Determine the correct "type" value and commit it to the
	 * TCB file.
	 */

	switch (tcbent->tcb_type) {
		case TCB_DIR:		cp = "DIRECTORY"; break;
		case TCB_BLK:		cp = "BLK_DEV"; break;
		case TCB_CHAR:		cp = "CHAR_DEV"; break;
		case TCB_MPX:		cp = "MPX_DEV"; break;
		case TCB_FIFO:		cp = "FIFO"; break;
		case TCB_FILE:		cp = "FILE"; break;
		case TCB_SYMLINK:	cp = "SYMLINK"; break;
		case TCB_LINK:		cp = "LINK"; break;
	}
	if (cp && xputtcbattr (name, "type", cp, SEC_CHAR)) {
		fmsg1 (Update_Failed, "type");
		return ENOTRUST;
	}

	/*
	 * Commit the "class" attribute.  It is a null-separated
	 * double-null terminated list of classes the file is a
	 * member of.
	 */

	if (tcbent->tcb_class)
		if (! list_null (tcbent->tcb_class, buf, BUFSIZ))
			return ENOTRUST;

	if (xputtcbattr (name, "class",
			tcbent->tcb_class ? buf:(char *) 0, SEC_LIST)) {
		fmsg1 (Update_Failed, "class");
		return ENOTRUST;
	}

	/*
	 * Get the user name for the file owner and commit the "owner"
	 * attribute with that value
	 */

	if (tcbent->tcb_owner != (uid_t) -1) {
		if (pw = getpwuid (tcbent->tcb_owner))
			cp = pw->pw_name;
		else {
			sprintf (buf, "%d", tcbent->tcb_owner);
			cp = buf;
		}
	}
	if (xputtcbattr (name, "owner", tcbent->tcb_owner !=
			(uid_t) -1 ? cp:(char *) 0, SEC_CHAR)) {
		fmsg1 (Update_Failed, "owner");
		return ENOTRUST;
	}

	/*
	 * Get the group name for the file group and commit the "group"
	 * attribute with that value
	 */

	if (tcbent->tcb_group != (gid_t) -1) {
		if (gr = getgrgid (tcbent->tcb_group))
			cp = gr->gr_name;
		else {
			sprintf (buf, "%d", tcbent->tcb_group);
			cp = buf;
		}
	}
	if (xputtcbattr (name, "group", tcbent->tcb_group !=
			(gid_t) -1 ? cp:(char *) 0, SEC_CHAR)) {
		fmsg1 (Update_Failed, "group");
		return ENOTRUST;
	}

	/*
	 * Get the "acl" attibute value and perform any special character
	 * escaping which must be done.  Commit the escaped "acl" value
	 */

	if (tcbent->tcb_acl) {
		if (strlen (tcbent->tcb_acl) > (BUFSIZ-3))
			return ENOTRUST;

		for (cp2 = buf, i = 0;tcbent->tcb_acl[i];i++) {
			if (tcbent->tcb_acl[i] == '\n') {
				*cp2++ = '\\';
				*cp2++ = 'n';
			} else
				*cp2++ = tcbent->tcb_acl[i];
		}
		*cp2++ = 0;
	}
	if (xputtcbattr (name, "acl",
			tcbent->tcb_acl ? buf:(char *) 0, SEC_CHAR)) {
		fmsg1 (Update_Failed, "acl");
		return ENOTRUST;
	}

	/*
	 * Get the "pcl" attibute value and perform any special character
	 * escaping which must be done.  Commit the escaped "pcl" value
	 */

	if (tcbent->tcb_pcl) {
		if (strlen (tcbent->tcb_pcl) > (BUFSIZ-3))
			return -1;

		for (cp2 = buf, i = 0;tcbent->tcb_pcl[i];i++) {
			if (tcbent->tcb_pcl[i] == '\n') {
				*cp2++ = '\\';
				*cp2++ = 'n';
			} else
				*cp2++ = tcbent->tcb_pcl[i];
		}
		*cp2++ = 0;
	}
	if (xputtcbattr (name, "pcl",
			tcbent->tcb_pcl ? buf:(char *) 0, SEC_CHAR)) {
		fmsg1 (Update_Failed, "pcl");
		return ENOTRUST;
	}

	/*
	 * The mode bits can fill in where either the PCL or the
	 * ACL left off, so it is important to have the mode =and=
	 * the ACL or PCL.
	 */

	if (tcbent->tcb_mode != -1)
		mode2string (tcbent->tcb_mode, buf);

	if (xputtcbattr (name, "mode", tcbent->tcb_mode != -1 ?
			buf:(char *) 0, SEC_LIST)) {
		fmsg1 (Update_Failed, "mode");
		return ENOTRUST;
	}

	/*
	 * Convert the array of links into a double-null terminated
	 * string and commit that string as the "links" attribute
	 */

	if (tcbent->tcb_links)
		if (! list_null (tcbent->tcb_links, buf, BUFSIZ))
			return ENOTRUST;

	if (xputtcbattr (name, "links",
			tcbent->tcb_links ? buf:(char *) 0, SEC_LIST)) {
		fmsg1 (Update_Failed, "links");
		return ENOTRUST;
	}

	/*
	 * Convert the array of symlinks into a double-null terminated
	 * string and commit that string as the "symlinks" attribute
	 */

	if (tcbent->tcb_symlinks)
		if (! list_null (tcbent->tcb_symlinks, buf, BUFSIZ))
			return ENOTRUST;

	if (xputtcbattr (name, "symlinks",
			tcbent->tcb_symlinks ? buf:(char *) 0, SEC_LIST)) {
		fmsg1 (Update_Failed, "symlinks");
		return ENOTRUST;
	}

	/*
	 * Get the "checksum" attibute value and perform any special
	 * character escaping which must be done.  Commit the escaped
	 * "checksum" value.
	 */

	if (tcbent->tcb_vchecksum)
		tcbent->tcb_checksum = (char *) 0;

	if (tcbent->tcb_checksum) {
		if (strlen (tcbent->tcb_checksum) > (BUFSIZ-3))
			return ENOTRUST;

		for (cp2 = buf, i = 0;tcbent->tcb_checksum[i];i++) {
			if (tcbent->tcb_checksum[i] == '\n') {
				*cp2++ = '\\';
				*cp2++ = 'n';
			} else
				*cp2++ = tcbent->tcb_checksum[i];
		}
		*cp2 = '\0';
	}
	if (xputtcbattr (name, "checksum",
			tcbent->tcb_checksum ? buf:(char *) 0, SEC_CHAR)) {
		fmsg1 (Update_Failed, "checksum");
		return ENOTRUST;
	}

	/*
	 * Get the "program" attibute value and perform any special
	 * character escaping which must be done.  Commit the escaped
	 * "program" value.
	 */

	if (tcbent->tcb_program)
		if (! list_null (tcbent->tcb_program, buf, BUFSIZ))
			return ENOTRUST;

	if (xputtcbattr (name, "program",
			tcbent->tcb_program ? buf:(char *) 0, SEC_LIST)) {
		fmsg1 (Update_Failed, "program");
		return ENOTRUST;
	}

	/* commit the "source" attribute */

	if (xputtcbattr (name, "source", tcbent->tcb_source, SEC_CHAR)) {
		fmsg1 (Update_Failed, "source");
		return ENOTRUST;
	}

	/* commit the "target" attribute */

	if (xputtcbattr (name, "target", tcbent->tcb_target, SEC_CHAR)) {
		fmsg1 (Update_Failed, "target");
		return ENOTRUST;
	}

	/* commit the "size" attribute */

	if (tcbent->tcb_vsize)
		tcbent->tcb_size = -1;

	if (xputtcbattr (name, "size", (void *) tcbent->tcb_size, SEC_INT)) {
		fmsg1 (Update_Failed, "size");
		return ENOTRUST;
	}
	return 0;
}

/*
 * NAME:	gettcbstanza
 *
 * FUNCTION:	Get a tcbent structure from an ASCII stanza file
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	This routine differs from gettcbent() in that it knows it
 *	is reading from an attribute file.  gettcbent() only knows
 *	about the interface, this routine is required to know
 *	about the actual underlying file structure.  Attributes
 *	which have a NULL value are computed from the file itself.
 *
 * RETURNS:
 *	NULL on EOF or error, a pointer to the next structure in the
 *	file if there is one.
 */

struct tcbent *
#ifdef	_NO_PROTO
gettcbstanza (fp)
AFILE_t	fp;
#else
gettcbstanza (AFILE_t fp)
#endif
{
	char	*cp;
	char	*cp2;
	char	*file;
	char	*xacl = 0;
	int	i;
	int	errcnt = 0;
	int	exists = 0;
	int	needed = 0;
	ATTR_t	attr;
	struct	tcbent	tcbent;
	struct	tcbent	*ntcbent;
	struct	passwd	*pw;
	struct	group	*gr;
	struct	fullstat statbuf;
	static	char	last[BUFSIZ];

	memset ((void *) &tcbent, 0, (size_t) sizeof tcbent);

	/*
	 * Read the next stanza and setup up the default values.
	 * The filename in the stanza is checked for existance and
	 * the attributes fetched.  These attributes will be used
	 * as defaults if the user specifies an attribute with no
	 * value.
	 */

	while (1) {
		if (! afread (fp)) {
			if (errno == 0)
				return 0;

			fmsg (Input_File_Error);
			if (last[0])
				fmsg1 (Last_Stanza, last);
			else
				fmsg (No_Last_Stanza);

			return 0;
		}
		attr = fp->AF_catr;

		strcpy (last, attr->AT_value);

		/*
		 * File name must begin with a slash
		 */

		if (attr->AT_value[0] != '/') {
			msg1 (Absolute_File, attr->AT_value);
			errcnt++;
			break;
		}

		/*
		 * See if the name is valid
		 */

		if (validate_name (attr->AT_value)) {
			msg2 (Illegal_Attribute, "name", attr->AT_value);
			errcnt++;
			break;
		}

		/* 
		 * See if the file exists.
		 */

		if (! fullstat (attr->AT_value, FL_STAT|FL_NOFOLLOW, &statbuf))
			exists = 1;

		break;
	}

	/*
	 * Initialize the structure to just a filename and all
	 * of the attributes set to values indicating no value
	 * for that attribute.
	 */

	init_tcbent (&tcbent);

	tcbent.tcb_name = file = xstrdup (attr->AT_value);
	if (validate_name (tcbent.tcb_name)) {
		msg1 (Illegal_Entry, tcbent.tcb_name);
		errcnt++;
	}
	if (! exists)
		memset ((void *) &statbuf, 0, (size_t) sizeof statbuf);

	/*
	 * Initialize the fields in the structure from the attributes
	 * in the stanza.
	 */

	for (attr++;attr->AT_name;attr++) {
		cp = attr->AT_value;

		/*
		 * Get the "class" attribute.  It is an error if this
		 * attribute is specified without a value.
		 */
		 
		if (! strcmp (attr->AT_name, "class")) {
			if (cp && *cp) {
				tcbent.tcb_class = null_list (cp);
			} else {
				msg1 (Needs_A_Value, attr->AT_name);
				errcnt++;
			}
			continue;
		}

		/*
		 * Get the "type" attribute.  This attribute must be
		 * specified for all entries.
		 */

		if (! strcmp (attr->AT_name, "type")) {
			if (! *cp) {
				msg1 (Needs_A_Value, attr->AT_name);
				errcnt++;
				continue;
			}
			if (add_type (&tcbent, cp, exists ? &statbuf:0))
				errcnt++;

			continue;
		}

		/*
		 * Get the "owner" attribute.  Not specified by default.
		 * Value will be computed from the file if the attribute
		 * value is missing.
		 */

		if (! strcmp (attr->AT_name, "owner")) {
			if (! *cp && ! exists) {
				needed = 1;
				continue;
			} else if (add_owner (&tcbent,
					*cp ? cp:0, exists ? &statbuf:0))
				errcnt++;

			continue;
		}

		/*
		 * Get the "group" attribute.  No default value.
		 * The value of the group attribute will be computed
		 * from the file if the attribute value is blank.
		 */

		if (! strcmp (attr->AT_name, "group")) {
			if (! *cp && ! exists) {
				needed = 1;
				continue;
			} else if (add_group (&tcbent,
					*cp ? cp:0, exists ? &statbuf:0))
				errcnt++;

			continue;
		}

		/*
		 * Get the "acl" attribute.  No default value.
		 * The value of the acl attribute will be computed
		 * from the file if the attribute value is blank.
		 *
		 * Here we have to invoke the aclget program to
		 * fetch the acl.
		 */

		if (! strcmp (attr->AT_name, "acl")) {
			if (! *cp && ! exists) {
				needed = 1;
				continue;
			}
			cp = xstrdup (cp);
			if (add_acl (&tcbent,
					*cp ? cp:0, exists ? &statbuf:0))
				errcnt++;

			continue;
		}

		/*
		 * Get the "pcl" attribute.  No default value.
		 * The value of the pcl attribute will be computed
		 * from the file if the attribute value is blank.
		 *
		 * Here we have to invoke the pclget program to
		 * fetch the pcl.
		 */

		if (! strcmp (attr->AT_name, "pcl")) {
			if (! *cp && !exists) {
				needed = 1;
				continue;
			}
			cp = xstrdup (cp);
			if (add_pcl (&tcbent,
					*cp ? cp:0, exists ? &statbuf:0))
				errcnt++;

			continue;
		}

		/*
		 * Get the "mode" attribute.  Convert character
		 * strings to a full file mode word.  If the attribute
		 * value is blank the mode on the file will be used.
		 */

		if (! strcmp (attr->AT_name, "mode")) {
			if (*cp == '\0') {
				if (exists) {
					tcbent.tcb_mode =
						statbuf.st_mode & ALL_MODES;
				} else
					needed = 1;

				continue;
			}
			if (string2mode (cp, &tcbent.tcb_mode, tcbent.tcb_name))
				errcnt++;

			continue;
		}

		/*
		 * Get the "links" attribute.  "links" is a comma
		 * separated list of hard links.  This attribute
		 * requires a value.
		 */

		if (! strcmp (attr->AT_name, "links")) {
			if (! *cp) {
				msg1 (Needs_A_Value, attr->AT_name);
				errcnt++;
				continue;
			}
			tcbent.tcb_links = null_list (cp);
			continue;
		}

		/*
		 * Get the "symlinks" attribute.  "symlinks" is a comma
		 * separated list of symbolic links.  This attribute requires
		 * a value.
		 */

		if (! strcmp (attr->AT_name, "symlinks")) {
			if (! *cp) {
				msg1 (Needs_A_Value, attr->AT_name);
				errcnt++;
				continue;
			}
			tcbent.tcb_symlinks = null_list (cp);
			continue;
		}

		/*
		 * Get the "checksum" attribute.  No default value.
		 * The checksum attribute will be computed from the
		 * file if the value is blank.
		 *
		 * The system checksum utility is invoked to compute
		 * the file's checksum.
		 *
		 * After installation it may be required to have the
		 * checksum ignored, for example, for files which
		 * change after install but need to be validated at
		 * install.  These files have ",VOLATILE" after the
		 * checksum to indicate this.
		 */

		if (! strcmp (attr->AT_name, "checksum")) {

			/*
			 * Checksum is only defined for regular files -
			 * report an error if someone tries to specify
			 * the checksum of a non-regular file file.
			 */

			if (exists && ! S_ISREG (statbuf.st_mode)) {
				msg2 (Illegal_Attribute, "checksum", file);
				errcnt++;
				continue;
			}

			/*
			 * If the checksum attribute has no value, compute
			 * the checksum from the file.  It is an error if
			 * the file does not exist.
			 */

			if (! *cp) {
				if (exists)
					tcbent.tcb_checksum =
						xstrdup (mk_sum (&tcbent));
				else
					needed = 1;

				continue;
			}

			/*
			 * The checksum attribute is a list of attributes,
			 * including "NOSIZE", "VOLATILE", and a string
			 * valued checksum.  The entire list must be
			 * walked through to find the parts since there
			 * is no ordering of the members.
			 */

			while (*cp) {
				if (! strcmp (cp, "NOSIZE"))
					tcbent.tcb_checksum = (char *) 0;
				else if (! strcmp (cp, "VOLATILE"))
					tcbent.tcb_vchecksum = 1;
				else
					tcbent.tcb_checksum = xstrdup (cp);

				cp += strlen (cp) + 1;
			}
			continue;
		}

		/*
		 * Get the "program" attribute.  The value for this
		 * attribute must be specified.
		 */

		if (! strcmp (attr->AT_name, "program")) {
			if (! *cp) {
				msg1 (Needs_A_Value, attr->AT_name);
				errcnt++;
				continue;
			}
			tcbent.tcb_program = null_list (cp);
			continue;
		}

		/*
		 * Get the "source" attribute.  No default value.
		 * This attribute is unique in that if the value is
		 * NULL it means the file is created from scratch.
		 */

		if (! strcmp (attr->AT_name, "source")) {
			if (cp)
				tcbent.tcb_source = xstrdup (cp);

			continue;
		}

		/*
		 * Get the "target" attribute.  No default value.
		 */

		if (! strcmp (attr->AT_name, "target")) {
			if (cp) 
				tcbent.tcb_target = xstrdup (cp);
			continue;
		}

		/*
		 * Get the "size" attribute.  The attribute value
		 * is computed directly from the file if one is not
		 * given.  This attribute is valid for regular files
		 * only.
		 *
		 * After installation it may be required to have the
		 * size ignored, for example, for files which
		 * change after install but need to be validated at
		 * install.  These files have ",VOLATILE" after the
		 * size to indicate this.
		 */

		if (! strcmp (attr->AT_name, "size")) {

			/*
			 * The size is only defined for regular files -
			 * report an error if someone tries to specify
			 * the size of a non-regular file file.
			 */

			if (exists && ! S_ISREG (statbuf.st_mode)) {
				msg2 (Illegal_Attribute, "size", file);
				errcnt++;
				continue;
			}

			/*
			 * If the size attribute has no value, compute
			 * the size from the file.  It is an error if
			 * the file does not exist.
			 */

			if (! *cp) {
				if (exists)
					tcbent.tcb_size = statbuf.st_size;
				else
					needed = 1;

				continue;
			}

			/*
			 * The size attribute is a list of attributes,
			 * including "NOSIZE", "VOLATILE", and a decimal
			 * value in bytes.  The entire list must be
			 * walked through to find the parts since there
			 * is no ordering of the members.
			 */

			while (*cp) {
				if (! strcmp (cp, "NOSIZE"))
					tcbent.tcb_size = -1;
				else if (! strcmp (cp, "VOLATILE"))
					tcbent.tcb_vsize = 1;
				else {
					tcbent.tcb_size = strtol (cp, &cp2, 10);
					if (cp2 == cp || *cp2 != '\0') {
						msg2 (Invalid_Value,
							"size", cp);
						errcnt++;
					}
				}
				cp += strlen (cp) + 1;
			}
			continue;
		}

	}


	/*
	 * If this is DELETE mode the file does not have
	 * to exist - in fact, it probably won't under many circumstances.
	 * So there is no point in flagging the entry as invalid if we
	 * are in either of those modes ...
	 */

	if (dflg)
		needed = 0;

	/*
	 * Flag the entry and return it in a newly malloc()d buffer
	 * Any errors during initialization will cause the entry to
	 * not be marked valid.  A valid tag is required for the
	 * entry to be added to the configuration file.
	 */

	if (! errcnt && ! needed) {
		tcbent.tcb_valid = 1;
		tcbent.tcb_changed = 1;
	}
	if (needed && !dflg) 
		fmsg1 (No_Such_File, file);

	if ((needed || errcnt) && !dflg)
		fmsg1 (Illegal_Entry, file);

	ntcbent = (struct tcbent *) malloc (sizeof (struct tcbent));
	*ntcbent = tcbent;

	return ntcbent;
}
