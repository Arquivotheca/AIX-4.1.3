static char sccsid[] = "@(#)48  1.2.1.7  src/bos/usr/bin/tcbck/tcbadd.c, cmdsadm, bos411, 9438C411e 9/25/94 13:36:37";
/*
 * COMPONENT_NAME: (CMDSADM) sysck - sysck configuration checker
 *
 * FUNCTIONS: make_tcbent, add_tcbent, add_tcbents
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

#include <sys/types.h> 
#include <sys/access.h>
#include <sys/stat.h>
#define _KERNEL
#include <sys/audit.h>
#undef _KERNEL
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
#include "tcbaudit.h"

#include <stdio.h>
#include <fcntl.h>
/*
#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL
*/
#include <jfs/filsys.h>
#include <jfs/ino.h>
/*#include <IN/AFdefs.h> */
#include <IN/FSdefs.h> 
#include <libfs/libfs.h>
#include <sys/vmount.h>

#ifdef DEBUG
#define Dprintf printf
#else
#define Dprintf
#endif

/*
 * Arbitrarily chosen table sizes; these may be altered at runtime by
 * setting environment variables ISIZE and HSIZE
 */
#define HSIZE           50000
int                     hsize = HSIZE;
#define ISIZE   512
int             isize;

#define vm_next(vm)     ((struct vmount *) (((char *) vm) + vm->vmt_length))

ino_t   ilist;          /* list of inodes specified on cmd line (-i)    */
int     nxfile;         /* index for ilist                              */

struct  htab            /* Hash table for directory inodes              */
{
        ino_t   h_ino;                  /* inode number                 */
        ino_t   h_pino;                 /* parent inode number of h_ino */
        char    h_name[MAXNAMLEN];      /* name of h_ino                */
};

struct htab     *htab;

int             nhent;                  /* number hashed                */
char            linknames[4096];        /* buf to build list of links   */
char            tempname[4096];
char            fsname[4096];           /* Name of fs file belongs to   */

int             nerror  = 0;            /* count of non-fatal errors    */
int             fd;                     /* file reading from            */

extern int      errno;

int             check (char*, char *);
char            *get_links(char *, char *);
void            pass1 (struct dinode*, ino_t);
void            pass2 (struct dinode*, ino_t, int);
void            pass3 (struct dinode*, ino_t, int, char*);
int             dotname (struct direct*);
void            pname (ino_t, int);
int             dirblk_read (struct dinode*, int, char*, ino_t);
struct htab     *lookup (ino_t, int);
struct dinode   *ginode (int, ino_t, struct superblock*);
void            read_super (int, struct superblock*, char*);
int             get_filesystem_name(char *);


/*
 * A macro defining the security relevant mode bits
 */

#define	ALL_MODES (S_ITCB|S_ITP|S_ISUID|S_ISGID|S_ISVTX|0777)

extern	char	*xstrdup ();
extern	char	*Aclget;
extern	char	*Aclput;
extern	char	*Pclget;
extern	char	*Checksum;
extern	char	*Attributes[];
extern	AFILE_t	afopen();
extern	char	*get_program();
extern	char	**comma_list();
extern	char	*comma2null();
struct	tcbent	*gettcbstanza();
struct	tcbent	*gettcbnam();
int		save_tcbents(char*, char*);
void		blankstrip(char *);

extern 	char 	temp_init[TEMP_SIZE]; 
extern 	FILE    *fdtemp;  

/*
 * Constant values representing each attribute.
 */

typedef	enum	{
	ATTR_CLASS,	ATTR_TYPE,	ATTR_OWNER,
	ATTR_GROUP,	ATTR_ACL,	ATTR_PCL,	ATTR_LINKS,
	ATTR_SYMLINKS,	ATTR_CHECKSUM,	ATTR_PROGRAM,	ATTR_SOURCE,
	ATTR_MODE,	ATTR_SIZE, 	ATTR_TARGET
} attr_t;

/*
 * Traits of each attribute.  The name, constant from above, and whether
 * the attribute is set on the command line or from the file appear here.
 */

struct	{
	char	*name;
	int	attr;
	int	arg;
} Attribute_Info[] = {
	{ "class",	ATTR_CLASS,	1},
	{ "type",	ATTR_TYPE,	0},
	{ "owner",	ATTR_OWNER,	0},
	{ "group",	ATTR_GROUP,	0},
	{ "acl",	ATTR_ACL,	0},
	{ "pcl",	ATTR_PCL,	0},
	{ "links",	ATTR_LINKS,	1},
	{ "symlinks",	ATTR_SYMLINKS,	1},
	{ "checksum",	ATTR_CHECKSUM,	0},
	{ "program",	ATTR_PROGRAM,	1},
	{ "source",	ATTR_SOURCE,	0},
	{ "mode",	ATTR_MODE,	0},
	{ "size",	ATTR_SIZE,	0},
	{ "target",	ATTR_TARGET,	0},
	{ 0, 0, 0}
};

#define	N_CLASS		0
#define	N_TYPE		1
#define	N_OWNER		2
#define	N_GROUP		3
#define	N_ACL		4
#define	N_PCL		5
#define	N_LINKS		6
#define	N_SYMLINKS	7
#define	N_CHECKSUM	8
#define	N_PROGRAM	9
#define	N_SOURCE	10
#define	N_MODE		11
#define	N_SIZE		12
#define N_TARGET	13
#define	N_ATTR	(sizeof Attribute_Info / sizeof Attribute_Info[0])

/*
 * NAME:	add_type
 *
 * FUNCTION:	Update the type value from the command line
 *
 * RETURN VALUE:
 *	Zero on success, non-zero otherwise
 */

int
add_type (
struct	tcbent	*tcbent,
char	*type,
struct	fullstat *sb)
{
	/*
	 * See if the file exists or a type was specified.  If
	 * neither of these is true report an error.
	 */

	if (type == 0 && sb == 0) {
		fmsg1 (Needs_A_Value, "type");
		return -1;
	}

	/*
	 * The type wasn't specified, so I get the type directly
	 * from the file itself.
	 */

	 if (! type) {
		if (S_ISFIFO (sb->st_mode))
			tcbent->tcb_type = TCB_FIFO;
		else if (S_ISDIR (sb->st_mode))
			tcbent->tcb_type = TCB_DIR;
		else if (S_ISCHR (sb->st_mode)) {

			/* see if character device is multiplexed */

			if (S_ISMPX (sb->st_mode))
				tcbent->tcb_type = TCB_MPX;
			else
				tcbent->tcb_type = TCB_CHAR;
		} else if (S_ISBLK (sb->st_mode))
			tcbent->tcb_type = TCB_BLK;
		else if (S_ISREG (sb->st_mode))
			tcbent->tcb_type = TCB_FILE;
		else if (S_ISLNK(sb->st_mode))
			tcbent->tcb_type = TCB_SYMLINK;
		else {
			fmsg1 (Unknown_Type, tcbent->tcb_name);
			tcbent->tcb_type = -1;
			return -1;
		}
	} else {

	/*
	 * There is a type string.  It overrides the file value.
	 */

		if (! strcmp (type, "FILE"))
			tcbent->tcb_type = TCB_FILE;
		else if (! strcmp (type, "DIRECTORY"))
			tcbent->tcb_type = TCB_DIR;
		else if (! strcmp (type, "BLK_DEV"))
			tcbent->tcb_type = TCB_BLK;
		else if (! strcmp (type, "CHAR_DEV"))
			tcbent->tcb_type = TCB_CHAR;
		else if (! strcmp (type, "MPX_DEV"))
			tcbent->tcb_type = TCB_MPX;
		else if (! strcmp (type, "FIFO"))
			tcbent->tcb_type = TCB_FIFO;
		else if (! strcmp (type, "SYMLINK"))
			tcbent->tcb_type = TCB_SYMLINK;
		else if (! strcmp (type, "LINK"))
			tcbent->tcb_type = TCB_LINK;
		else {
			tcbent->tcb_type = -1;
			fmsg2 (Illegal_Type, tcbent->tcb_name, type);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME:	add_owner
 *
 * FUNCTION:	Update TCB entry from command line argument for owner
 *
 * RETURN VALUE:
 *	-1 on error, 0 otherwise
 */

uid_t
add_owner (
struct	tcbent	*tcbent,
char	*owner,
struct	fullstat *sb)
{
	long	l;
	char	*cp;
	struct	passwd	*pw;

	/*
	 * No value means to compute it from the file
	 */

	if (owner == 0) {
		if (sb) {
			tcbent->tcb_owner = sb->st_uid;
			return 0;
		} else {
			fmsg1 (Needs_A_Value, "owner");
			return -1;
		}
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (*owner == '\0') {
		tcbent->tcb_owner = -1;
		return 0;
	}

	/*
	 * Try to convert the value.  Numbers are converted directly
	 * to user IDs, non-numbers are looked up in the database.
	 */

	if (! isnumeric (owner)) {
		if (pw = getpwnam (owner)) {
			tcbent->tcb_owner = pw->pw_uid;
		} else {
			fmsg1 (Unknown_User, owner);
			return -1;
		}
	} else {
		tcbent->tcb_owner = strtol (owner, &cp, 10);
		if (cp == (char *) 0 || *cp != '\0') {
			fmsg2 (Invalid_Value, "owner", owner);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME:	add_group
 *
 * FUNCTION:	Update TCB entry from command line argument for group
 *
 * RETURN VALUE:
 *	-1 on error, 0 otherwise
 */

uid_t
add_group (struct tcbent *tcbent, char *group, struct fullstat *sb)
{
	long	l;
	char	*cp;
	struct	group	*gr;

	/*
	 * No value means to compute it from the file
	 */

	if (group == 0) {
		if (sb) {
			tcbent->tcb_group = sb->st_gid;
			return 0;
		} else {
			fmsg1 (Needs_A_Value, "group");
			return -1;
		}
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (*group == '\0') {
		tcbent->tcb_group = -1;
		return 0;
	}

	/*
	 * Try to conver the value.  Numbers are converted directly
	 * to user IDs, non-numbers are looked up in the database.
	 */

	if (! isnumeric (group)) {
		if (gr = getgrnam (group)) {
			tcbent->tcb_group = gr->gr_gid;
		} else {
			fmsg1 (Unknown_Group, group);
			return -1;
		}
	} else {
		tcbent->tcb_group = strtol (group, &cp, 10);
		if (cp == (char *) 0 || *cp != '\0') {
			fmsg2 (Invalid_Value, "group", group);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME:	add_checksum
 *
 * FUNCTION:	Update the checksum value from the command line argument
 *
 * RETURN VALUE:
 *	0 for success, non-zero otherwise
 */

static int
add_checksum (
struct	tcbent	*tcbent,
char	*sum,
struct	fullstat *sb)
{
	/*
	 * No value means to compute it from the file.  Give up if
	 * the file doesn't exist and there is no value.
	 */

	if (sum == 0 && sb == 0) {
		fmsg1 (Needs_A_Value, "checksum");
		return -1;
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (sum && *sum == '\0') {
		tcbent->tcb_checksum = 0;
		return 0;
	}

	/*
	 * Checksums are only valid on regular files.
	 */

	if (sb && ! S_ISREG (sb->st_mode)) {
		fmsg2 (Illegal_Attribute, "checksum", tcbent->tcb_name);
		return -1;
	}

	/*
	 * Either a value was specified and we use it, or no value
	 * was specified and we compute it directly
	 */

	if (sum)
		tcbent->tcb_checksum = sum;
	else
		tcbent->tcb_checksum =
			get_program (Checksum, tcbent->tcb_name);

	return tcbent->tcb_checksum == 0;
}

/*
 * NAME:	add_program
 *
 * FUNCTION:	Update the program value from the command line
 *
 * RETURN VALUE:
 *	0 for success, non-zero otherwise
 */

static int
add_program (
struct	tcbent	*tcbent,
char	*program)
{
	/*
	 * Blank value means to remove the attribute
	 */

	if (*program == '\0') {
		tcbent->tcb_program = 0;
		return 0;
	}
	tcbent->tcb_program = comma_list (program);
	return 0;
}

/*
 * NAME:	add_mode
 *
 * FUNCTION:	Update the mode value from the command line argument
 *
 * RETURN VALUE:
 *	0 for success, non-zero otherwise
 */

int
add_mode (
struct	tcbent	*tcbent,
char	*mode,
struct	fullstat *sb)
{
	char	*cp;

	/*
	 * No value means to compute it from the file.  Give up if
	 * the file doesn't exist and there is no value.
	 */

	if (mode == 0 && sb == 0) {
		fmsg1 (Needs_A_Value, "mode");
		return -1;
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (mode && *mode == '\0') {
		tcbent->tcb_mode = -1;
		return 0;
	}

	/*
	 * Either a value was specified and we use it, or no value
	 * was specified and we compute it directly
	 */

	if (mode) {
		cp = comma2null (mode);
		if (string2mode (cp, &tcbent->tcb_mode, tcbent->tcb_name))
			return -1;
		else
			return 0;
	} else {
		tcbent->tcb_mode = sb->st_mode & ALL_MODES;
		return 0;
	}
}

/*
 * NAME:	add_size
 *
 * FUNCTION:	Update the size value from the command line argument
 *
 * RETURN VALUE:
 *	0 for success, non-zero otherwise
 */

static int
add_size (
struct	tcbent	*tcbent,
char	*size,
struct	fullstat *sb)
{
	char	*cp;

	/*
	 * No value means to compute it from the file.  Give up if
	 * the file doesn't exist and there is no value.
	 */

	if (size == 0 && sb == 0) {
		fmsg1 (Needs_A_Value, "size");
		return -1;
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (size && *size == '\0') {
		tcbent->tcb_size = -1;
		return 0;
	}

	/*
	 * File size is only valid on regular files.
	 */

	if (sb && ! S_ISREG (sb->st_mode)) {
		fmsg2 (Illegal_Attribute, "size", tcbent->tcb_name);
		return -1;
	}

	/*
	 * Either a value was specified and we use it, or no value
	 * was specified and we compute it directly
	 */

	if (size) {
		tcbent->tcb_size = strtol (size, &cp, 10);
		if (cp == (char *) 0 || *cp != '\0') {
			fmsg2 (Invalid_Value, "size", size);
			return -1;
		}
	} else
		tcbent->tcb_size = sb->st_size;

	return 0;
}

/*
 * NAME:	add_acl
 *
 * FUNCTION:	Update the ACL value from the command line argument
 *
 * RETURN VALUE:
 *	0 for success, non-zero otherwise
 */

int
add_acl (
struct	tcbent	*tcbent,
char	*acl,
struct	fullstat *sb)
{
	/*
	 * No value means to compute it from the file.  Give up if
	 * the file doesn't exist and there is no value.
	 */

	if (acl == 0 && sb == 0) {
		fmsg1 (Needs_A_Value, "acl");
		return -1;
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (acl && *acl == '\0') {
		tcbent->tcb_acl = 0;
		return 0;
	}

	/*
	 * Either a value was specified and we use it, or no value
	 * was specified and we compute it directly
	 */

	if (acl)
		tcbent->tcb_acl = acl;
	else
		tcbent->tcb_acl = get_program (Aclget, tcbent->tcb_name);

	return tcbent->tcb_acl == 0;
}

/*
 * NAME:	add_pcl
 *
 * FUNCTION:	Update the PCL value from the command line argument
 *
 * RETURN VALUE:
 *	0 for success, non-zero otherwise
 */

int
add_pcl (
struct	tcbent	*tcbent,
char	*pcl,
struct	fullstat *sb)
{
	/*
	 * No value means to compute it from the file.  Give up if
	 * the file doesn't exist and there is no value.
	 */

	if (pcl == 0 && sb == 0) {
		fmsg1 (Needs_A_Value, "pcl");
		return -1;
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (pcl && *pcl == '\0') {
		tcbent->tcb_pcl = 0;
		return 0;
	}

	/*
	 * Either a value was specified and we use it, or no value
	 * was specified and we compute it directly
	 */

	if (pcl)
		tcbent->tcb_pcl = pcl;
	else
		tcbent->tcb_pcl = get_program (Pclget, tcbent->tcb_name);

	return tcbent->tcb_pcl == 0;
}

/*
 * NAME: add_audit_info
 *
 * FUNCTION: create audit records for each field in a tcbent struct
 *
 * ENVIRONMENT:
 *
 *	User Process
 *
 * NOTES:
 *
 *	This routine creates a "SYSCK_Update" audit record for
 *	each field in a tcbent structure.  It is assumed that the
 *	passed in structure is going to be added to the database
 *	and so this information will be changed.
 *
 * RETURNS: NONE
 */

void
#ifdef	_NO_PROTO
add_audit_info (tcbent, status)
struct	tcbent	*tcbent;
int	status;
#else
add_audit_info (struct tcbent *tcbent, int status)
#endif
{
	char	*file = tcbent->tcb_name;

	if (tcbent->tcb_class)
		mk_audit_rec (SYSCK_Update, status, file, "class", NULL);

	if (tcbent->tcb_type != -1)
		mk_audit_rec (SYSCK_Update, status, file, "type", NULL);

	if (tcbent->tcb_owner != (uid_t) -1)
		mk_audit_rec (SYSCK_Update, status, file, "owner", NULL);

	if (tcbent->tcb_group != (gid_t) -1)
		mk_audit_rec (SYSCK_Update, status, file, "group", NULL);

	if (tcbent->tcb_mode != -1)
		mk_audit_rec (SYSCK_Update, status, file, "mode", NULL);

	if (tcbent->tcb_acl)
		mk_audit_rec (SYSCK_Update, status, file, "acl", NULL);

	if (tcbent->tcb_pcl)
		mk_audit_rec (SYSCK_Update, status, file, "pcl", NULL);

	if (tcbent->tcb_links)
		mk_audit_rec (SYSCK_Update, status, file, "links", NULL);

	if (tcbent->tcb_symlinks)
		mk_audit_rec (SYSCK_Update, status, file, "symlinks", NULL);

	if (tcbent->tcb_checksum)
		mk_audit_rec (SYSCK_Update, status, file, "checksum", NULL);

	if (tcbent->tcb_program)
		mk_audit_rec (SYSCK_Update, status, file, "program", NULL);

	if (tcbent->tcb_source)
		mk_audit_rec (SYSCK_Update, status, file, "source", NULL);

	if (tcbent->tcb_size != -1)
		mk_audit_rec (SYSCK_Update, status, file, "size", NULL);

	if (tcbent->tcb_target)
		mk_audit_rec (SYSCK_Update, status, file, "target", NULL);

}

/*
 * NAME:	make_tcbent
 *
 * FUNCTION:	Convert command line arguments into a tcb structure
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to new tcbent structure or NULL on error
 */

#ifdef	_NO_PROTO
int
make_tcbent (argv)
char	**argv;
#else
int
make_tcbent (char **argv)
#endif
{
	char	*arg;
	char	*cp;
	char	*file;
	char	buf[MAXTCBSIZ];
	int	i;
	int	errors = 0;
	int	exists = 0;		/* File being added exists */
	attr_t	attr;
	FILE	*fp;
	struct	tcbent	tcbent;
	struct	tcbent	*ntcbent;
	struct	fullstat statbuf;

	/*
	 * A file name must be specified and the named file must be an
	 * absolute pathname and exist.
	 */

	if (*argv == 0)
		usage ();

	if (strcmp (argv[0], "sysck") == 0)
		return update_sysck (argv);

	if (*argv[0] != '/') {
		fmsg1 (Absolute_File, argv[0]);
		return EINVAL;
	}

	if (ntcbent = gettcbnam (argv[0])) {
		tcbent = *ntcbent;
		file = tcbent.tcb_name;
		argv++;
	} else {
		init_tcbent (&tcbent);
		tcbent.tcb_name = file = *argv++;
	}
	if (validate_name (file)) {
		fmsg1 (Illegal_Entry, file);
		return EINVAL;
	}

	/*
	 * Get the stat structure for the file, ignoring symbolic
	 * links and such.
	 */

	if (fullstat (file, FL_STAT|FL_NOFOLLOW, &statbuf)) {
		memset (&statbuf, 0, sizeof statbuf);
		exists = 0;
	} else
		exists = 1;

	/*
	 * Set the file type from the file itself.  This entry must always
	 * exist.  Also set the owner and group, which are added by
	 * default.
	 */

	if (exists) {
		add_type (&tcbent, (char *) 0, &statbuf);
		tcbent.tcb_owner = statbuf.st_uid;
		tcbent.tcb_group = statbuf.st_gid;

	} else
		tcbent.tcb_type = -1;

	/*
	 * For each argument, see if it is a valid attribute and add
	 * the value associated with it to the structure.  Some
	 * attributes are set from the file itself, others on the
	 * command line.  The Attribute_Info array has a list of which
	 * attributes work how.
	 */

	for (;*argv;argv++) {

		/*
		 * Scan the attribute array for the named attribute
		 */

		for (i = 0;Attribute_Info[i].name != 0;i++) {
			if (strncmp (*argv, Attribute_Info[i].name,
					strlen (Attribute_Info[i].name)) == 0) {
				attr = Attribute_Info[i].attr;
				break;
			}
		}

		/*
		 * If the attribute does not exist in the table, produce
		 * the attribute usage message and exit.
		 */

		if (Attribute_Info[i].name == 0)
			usage (Attributes);

		/*
		 * See if the attribute needs a value and one was supplied
		 * and vice versa.
		 */

		if (Attribute_Info[i].arg) {
			if (strchr (*argv, '=') == 0) {
				fmsg1 (Needs_A_Value, Attribute_Info[i].name);
				errors++;
				continue;
			}
		}
		if (arg = strchr (*argv, '='))
			arg++;

		/*
		 * Switch on the attribute, updating the appropriate
		 * field from either the file or the command line argument.
		 */

		switch (attr) {
			case ATTR_TYPE:
				if (arg && *arg == 0) {
					tcbent.tcb_type = -1;
				} else if (add_type (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_CLASS:
				if (arg && *arg == '\0') {
					tcbent.tcb_class = 0;
				} else
					tcbent.tcb_class = comma_list (arg);
				break;
			case ATTR_OWNER:
				if (arg && *arg == '\0') {
					tcbent.tcb_owner = -1;
				} else if (add_owner (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_GROUP:
				if (arg && *arg == '\0') {
					tcbent.tcb_group = -1;
				} else if (add_group (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_ACL:
				if (arg && *arg == '\0') {
					tcbent.tcb_acl = 0;
				} else if (add_acl (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_PCL:
				if (arg && *arg == '\0') {
					tcbent.tcb_pcl = 0;
				} else if (add_pcl (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_LINKS:
				if (arg && *arg == '\0') {
					tcbent.tcb_links = 0;
				} else
					tcbent.tcb_links = comma_list (arg);
				break;
			case ATTR_SYMLINKS:
				if (arg && *arg == '\0') {
					tcbent.tcb_symlinks = 0;
				} else
					tcbent.tcb_symlinks = comma_list (arg);
				break;
			case ATTR_CHECKSUM:
				if (arg && *arg == '\0') {
					tcbent.tcb_checksum = 0;
				} else if (add_checksum (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_PROGRAM:
				if (arg && *arg == '\0') {
					tcbent.tcb_program = 0;
				} else if (add_program (&tcbent, arg))
					errors++;
				break;
			case ATTR_SOURCE:
				if (arg && *arg != '\0')
					tcbent.tcb_source = arg;
				else if (arg && *arg == '\0')
					tcbent.tcb_source = 0;
				else
					tcbent.tcb_source = "";

				break;
			case ATTR_MODE:
				if (arg && *arg == '\0') {
					tcbent.tcb_mode = -1;
				} else if (add_mode (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_SIZE:
				if (arg && *arg == '\0') {
					tcbent.tcb_size = -1;
				} else if (add_size (&tcbent, arg,
						exists ? &statbuf:NULL))
					errors++;
				break;
			case ATTR_TARGET:
				if (arg && *arg != '\0') 
					tcbent.tcb_target = arg;
				else if (arg && *arg == '\0')
					tcbent.tcb_target = 0;
				else
					tcbent.tcb_target = "";
				break;
		}
	}

	/*
	 * One last chance to see if the type was ever specified.
	 */

	if (tcbent.tcb_type == -1) {
		fmsg1 (Donot_Know_What, file);
		errors++;
	}

	/*
	 * Mark the entry as new and allocate space for it.  Return the
	 * new pointer to the newly allocated memory.
	 */

	add_audit_info (&tcbent, errors ? AUDIT_FAIL:AUDIT_OK);

	if (errors)
		return exists ? EINVAL:ENOENT;

	tcbent.tcb_valid = 1;
	tcbent.tcb_changed = 1;

	if (puttcbent (&tcbent)) {
		perror (file);
		return -1;
	}
	puttcbattr ((char *) 0,(char *) 0,(void *) 0, SEC_COMMIT);
	enduserdb ();

	return 0;
}

/*
 * NAME: add_tcbent
 *                                                                    
 * FUNCTION: Add a TCB file entry or update an existing entry.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Updates a TCB entry by getting the present value of the entry
 *	and updating the fields in the old entry with the values in the
 *	new one.  The entry is flagged as changed.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
add_tcbent (tcbent)
struct	tcbent	*tcbent;
{
	struct	tcbent	*tp;
	char	*file;
	int	i;
	int	rc = 0;
	int	exists = 0;

	/*
	 * Search for the entry in the hash table or in the TCB file
	 */

	if (! (tp = (struct tcbent *)
			hash_find (tcbent->tcb_name))) {
		if (tp = gettcbnam (tcbent->tcb_name)) {
			if (add_tcb_table (tp))
				return ENOTRUST;

			exists = 1;
		}
	} else
		exists = 1;

	/*
	 * If the record wasn't in the database, and it wasn't already
	 * added, allocate memory for it and copy stanza name and other
	 * non-attribute fields.
	 */

	if (! exists) {
 		tp = (struct tcbent *) xmalloc (sizeof *tp);
		init_tcbent (tp);
		tp->tcb_name = tcbent->tcb_name;
		tp->tcb_vsize = tcbent->tcb_vsize;
		tp->tcb_vchecksum = tcbent->tcb_vchecksum;
	}

	file = tp->tcb_name;

	/*
	 * One by one, copy the values into the new structure and flag
	 * the field as having been updated.
	 */

	if (tcbent->tcb_class) {
		if (tp->tcb_class)
			free_list (tp->tcb_class);

		tp->tcb_class = tcbent->tcb_class;
	}
	if (tcbent->tcb_type != -1) {
		tp->tcb_type = tcbent->tcb_type;
	}
	if (tcbent->tcb_owner != (uid_t) -1) {
		tp->tcb_owner = tcbent->tcb_owner;
	}
	if (tcbent->tcb_group != (gid_t) -1) {
		tp->tcb_group = tcbent->tcb_group;
	}
	if (tcbent->tcb_mode != -1) {
		tp->tcb_mode = tcbent->tcb_mode;
	}
	if (tcbent->tcb_acl) {
		if (tp->tcb_acl)
			free (tp->tcb_acl);

		tp->tcb_acl = tcbent->tcb_acl;
	}
	if (tcbent->tcb_pcl) {
		if (tp->tcb_pcl)
			free (tp->tcb_pcl);

		tp->tcb_pcl = tcbent->tcb_pcl;
	}
	if (tcbent->tcb_links) {
		if (tp->tcb_links) {
			for (i = 0; tp->tcb_links[i]; i++)
				hash_del (tp->tcb_links[i]);

			free_list (tp->tcb_links);
		}
		tp->tcb_links = tcbent->tcb_links;

		if (exists) 
		{
			for (i = 0; tp->tcb_links[i]; i++)
				hash_add (tp->tcb_links[i], tp);
		}
	}

	if (tcbent->tcb_symlinks) {
		if (tp->tcb_symlinks) {
			for (i = 0; tp->tcb_symlinks[i]; i++)
				hash_del (tp->tcb_symlinks[i]);

			free_list (tp->tcb_symlinks);
		}
		tp->tcb_symlinks = tcbent->tcb_symlinks;

		if (exists) 
		{
			for (i = 0; tp->tcb_symlinks[i]; i++)
				hash_add (tp->tcb_symlinks[i], tp);
		}
	}

	if (tcbent->tcb_checksum) {
		if (tp->tcb_checksum)
			free (tp->tcb_checksum);

		tp->tcb_checksum = tcbent->tcb_checksum;
	}
	if (tcbent->tcb_program) {
		if (tp->tcb_program)
			free (tp->tcb_program);

		tp->tcb_program = tcbent->tcb_program;
	}
	if (tcbent->tcb_source) {
		if (tp->tcb_source)
			free (tp->tcb_source);

		tp->tcb_source = tcbent->tcb_source;
	}
	if (tcbent->tcb_target) {
		if (tp->tcb_target)
			free (tp->tcb_target);

		tp->tcb_target = tcbent->tcb_target;
	}
	if (tcbent->tcb_size != -1) {
		tp->tcb_size = tcbent->tcb_size;
	}
	tp->tcb_changed = 1;
	tp->tcb_valid = 1;

	/*
	 * If the entry did not previously exist it needs to be added
	 * to the table.
	 */

	if (! exists) {
		if (add_tcb_table (tp))
			rc = ENOTRUST;
	}

	/*
	 * Now cut the audit records and free up the new fields
	 */

	add_audit_info (tcbent, rc ? AUDIT_FAIL:AUDIT_OK);
	free (tcbent);

	return rc;
}

/*
 * NAME: add_tcbents
 *                                                                    
 * FUNCTION: Add a file of entries, or update existing entries from a file
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Reads a file full of stanzas and adds entries which do not
 *	exist already, or updates entries which already existed.
 *
 * RETURNS: Zero on success, non-zero otherwise
 */  

int
add_tcbents (file)
char	*file;		/* Name of the file to be read for new entries */
{
	int	errors = 0;
	AFILE_t	afp;
	struct	tcbent	*tp;

	/*
	 * Open the stanza file containing the new entries
	 */

	if (! (afp = afopen (file))) {
		msg1 (No_Such_File, file);
		return ENOENT;
	}

	/*
	 * Read the entire file, updating stanzas for each
	 * as you go.
	 */

	while (1) {
		if (! (tp = gettcbstanza (afp))) {
			if (errno != 0)
				errors++;
			else
				break;
		} else if (add_tcbent (tp))
			errors++;
	}
	afclose (afp);

	return errors ? EINVAL:0;
}

/*****************************************************************************
 * NAME: save_tcbents
 *
 * FUNCTION: Save existing sysck.cfg stanzas that match the stanzas listed in
 *           list_file into save_file.
 *
 * EXECUTION ENVIRONMENT:
 *      User process
 *
 * NOTES:
 *      Reads list_file, finds each corresponding stanza in sysck.cfg,
 *      and appends the sysck.cfg version to save_file
 *
 * RETURNS: Zero on success, non-zero otherwise
 *
 *****************************************************************************/

int
save_tcbents (list_file, save_file)
char    *list_file;             /* Filename containing stanzas to save       */
char    *save_file;             /* Filename indicating where to save stanzas */
{
        int     errors=0;       /* Any errors found                    */
        int     rc=0;           /* Return code temporary variable      */
        FILE    *savef;         /* Where to save the current sysck.cfg */
                                /* stanzas                             */

        FILE    *listf;         /* Stanza file (.tcb format file) con- */
                                /* taining stanzas that we want to     */
                                /* save from /etc/security/sysck.cfg   */

        FILE    *sysckf;        /* /etc/security/sysck.cfg file        */

        char    list_stz[LINE_MAX];     /* Current stanza in list file */
        char    line[LINE_MAX];         /* Temp place to read a line   */
        char    save_stz[MAXTCBSIZ];    /* Stanza from sysck.cfg       */

        char    *p;                     /* Temp string pointer         */


        /*
         * First, lock out other procs from doing put*attr's
         */

        if (setuserdb (S_WRITE))
                return (-1);

        /*
         * Open /etc/security/sysck.cfg.
         */
        if ((sysckf=fopen(SYSCK_NAME,"r")) == NULL) {
                msg1 (No_Such_File, SYSCK_NAME);
                return ENOENT;
        }

        /*
         * Open the stanza list file containing list of affected stanzas.
         */
        if ((listf=fopen(list_file,"r")) == NULL) {
                msg1 (No_Such_File, list_file);
                return ENOENT;
        }

        /*
         * Open the stanza save file to save copies of sysck.cfg stanzas.
         */
        if ((savef=fopen(save_file,"w")) == NULL) {
                msg1 (Create_Failed, save_file);
                return ENOENT;
        }

        /*
         * Read the entire list file, saving stanzas as we go.
         */
        while (fgets(list_stz,LINE_MAX-1,listf)) {
                blankstrip(list_stz);
                /*
                 * A '/' means we have the start of a stanza. If
                 * we don't have one, just keep going to the next
                 * line.
                 */
                if (*list_stz!='/')
                        continue;
                /*
                 * Must be at the start of a stanza.
                 * Go back to the beginning of the sysck file
                 * to start search
                 */
                fseek(sysckf,0L,SEEK_SET);

                /*
                 * Scan sysck file for stanza name matching the
                 * current one we found in the list file (listf)
                 */
                *save_stz='\0';
                while(fgets(line,LINE_MAX,sysckf)) {
                        blankstrip(line);
                        if (!strcmp(line,list_stz)) {
                                /*
                                 * We have a match!
                                 */
                                strcpy(save_stz,line);
                                while(fgets(line,LINE_MAX,sysckf)) {
                                        /*
                                         * Collect whole stanza
                                         */
                                        strcat(save_stz,line);

                                        /*
                                         * Check for end of stanza (a
                                         * blank line)
                                         */
                                        p=line;
                                        while (isspace(*p))
                                                p++;
                                        if (!*p) {
                                                fwrite(save_stz,
                                                       strlen(save_stz),
                                                       1,
                                                       savef);
						*save_stz='\0';
                                                break;
                                        }
                                }
				/*
				 * If we drop out of the inner while loop -
				 * where we are collecting a matched stanza -
				 * and we have a string in save_stz, then we
				 * hit EOF before we found a blank line so
				 * so we need to go ahead and write it out 
				 * to the save file.
				 */
				if (*save_stz) 
				{
					fprintf("\n", 1, 1, savef);
                               		fwrite(save_stz,
                                               strlen(save_stz),
                                               1,
                                               savef);
				}
					
                        }
                        /*
                         * If we found a matching stanza, quit
                         * looking (and, thus, looping).
                         */
                        if (*save_stz)
                                break;
                }
        }


        /*
         * Close up and return
         * NOTE: we never check the return codes from closes.
         */
        fclose(sysckf);
        fclose(listf);
        fclose(savef);

        enduserdb();

        return (errors ? EINVAL : 0);
}



/*****************************************************************************
 * NAME: blankstrip
 *
 * FUNCTION: strip all whitespace out of a string
 *
 * EXECUTION ENVIRONMENT:
 *      User process
 *
 * RETURNS: Nothing
 *
 *****************************************************************************/

void
blankstrip(char *s)
{
        char    *curp=s;        /* Pointer to where to put next non-blank */

        if (!s || !*s)          /* Null pointer or string, return.        */
                return;

        while (*s) {
                if (!(*s==' '||*s=='\t'))    /* If it isn't whitespace         */
                        *curp++=*s;          /* Move it to p and increment p   */
                s++;                         /* In any case, increment s       */
        }
	*curp='\0';
}









/*
 * NAME: error_write_temp
 *
 * FUNCTION: Error writing stanza to temp file.
 *
 *
 * RETURNS: Zero on success, non-zero otherwise.
 *
 */

int
error_write_temp (void)
{
	fmsg (Update_Temp_File_Error );
	unlink(temp_init);
	fclose(fdtemp);
	exit (errno);
}


/*
 * NAME: build_temp_file
 *
 * FUNCTION: Build a stanza for each /dev entry listed and write it
 *           to a temporary file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */

int
build_temp_file (argv)
char    **argv;
{
        struct  fullstat statbuf, *st=&statbuf;     
        char buf[BUFSIZ];
        mode_t  omask, type;
        struct passwd *pw;
        struct group *gr;
        int i, rc, len, cc;
        char linkname[BUFSIZ];
	char linkbuf[4096];


        /*  Create a /tmp file to contain the information from the
            command line arguments.                                      */

        strcpy(temp_init, SYSCK_FILE);
        strcpy ( &temp_init [TEMP_SIZE - 7] , XXXXXX );
        if ( mktemp (temp_init) == 0 )
        {
                msg1 (Create_Failed, temp_init);
                exit (errno);
        }

        /*  Open the /tmp file. */
        omask = umask((mode_t)077);
        if ((fdtemp = fopen(temp_init, "w")) == NULL)
        {
		fmsg (Open_Temp_File_Error);
                unlink(temp_init);
                fclose(fdtemp);
                exit (errno);
        }
        (void) umask(omask);

        /*  For each command line argument, get stanza information and
            write to a temporary file.  */
        for (i = 0;argv[i];i++)
        {
	   
         /* See if this is a valid /dev entry; if not exit */

                memset (st, 0, sizeof st);
                rc = (fullstat (argv[i], FL_STAT|FL_NOFOLLOW, st));
                if (rc!=0)
			{
                        msg1 (No_Such_File, argv[i]);
                        unlink(temp_init);
                        fclose(fdtemp);
                        exit (errno);
			}

	/* We do not create a entry in the sysck.cfg file for Sockets.
           Just continue to next argument  */

        	if (!S_ISSOCK(st->st_mode))
	      	{
                /* Get header stanza line */

                strcpy (buf, argv[i]);
                strcat (buf, ":\n");
                len = strlen(buf);
                if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
			error_write_temp();

                /* Get Owner stanza line */

                type = st->st_mode & S_IFMT;
                strcpy (buf, "\towner = ");
                pw = getpwuid(st->st_uid);
                strcat (buf, pw->pw_name);
                strcat (buf, "\n");
                len = strlen(buf);
                if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
			error_write_temp();

                /* Get Group stanza line */

                strcpy (buf, "\tgroup = ");
                gr = getgrgid(st->st_gid);
                strcat (buf, gr->gr_name);
                strcat (buf, "\n");
                len = strlen(buf);
                if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
			error_write_temp();

                /* Get Type stanza line */

                strcpy (buf, "\ttype = ");
                if (S_ISMPX(st->st_mode))
                        strcat (buf, "MPX_DEV\n");
                else
                {
                switch (type)
                {
                        case S_IFDIR:
                        strcat (buf, "DIRECTORY\n");
                        break;

                        case S_IFBLK:
                        strcat (buf, "BLK_DEV\n");
                        break;

                        case S_IFCHR:
                        strcat (buf, "CHAR_DEV\n");
                        break;

                        case S_IFREG:
                        strcat (buf, "FILE\n");
                        break;

                        case S_IFIFO:
                        strcat (buf, "FIFO\n");
                        break;

                        case S_IFLNK:
                        strcat (buf, "SYMLINK\n");
                	len = strlen(buf);
                	if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
				error_write_temp();

                        if ((cc = readlink(argv[i], linkname, BUFSIZ)) < 0)
                        	{
				msg2 (No_Such_Symlink, argv[i], linkname);
                        	unlink(temp_init);
                        	fclose(fdtemp);
                        	exit (errno);
                        	}
                	strcpy (buf, "target = ");
			strcat (buf, linkname);
                	strcat (buf, "\n");
                        break;

                        default:
                        break;
                }
                }
		
                len = strlen(buf);
                if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
			error_write_temp();

		/* See if there are any Hard Links 
		 * associated with this entry, if so 
		 * get a list of the hard links
		 */

		if ((st->st_nlink > 1) && (type != S_IFDIR) && (type != S_IFLNK))
		{
			*linkbuf = '\0';
			if (get_links(argv[i], linkbuf) != NULL);
                        {
                	strcpy (buf, "\tlinks = ");
			strcat (buf, linkbuf); 
                	strcat (buf, "\n");
                	len = strlen(buf);
                if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
				error_write_temp();
			}
		}


                /* Get mode stanza line;              *
                 * (owner, group, other permissions)  *
                 * (SUID, SGID, SVTX and TCB bits)    */

                /* Get owner permissions */

                strcpy (buf, "\tmode = ");
                if (st->st_mode & S_IRUSR)
                        strcat (buf, "r");
                else
                        strcat (buf, "-");
                if (st->st_mode & S_IWUSR)
                        strcat (buf, "w");
                else
                        strcat (buf, "-");
                if (st->st_mode & S_IXUSR)
                        strcat (buf, "x");
                else
                        strcat (buf, "-");

             	/*     Get group permissions */

                if (st->st_mode & S_IRGRP)
                        strcat (buf, "r");
                else
                        strcat (buf, "-");
                if (st->st_mode & S_IWGRP)
                        strcat (buf, "w");
                else
                        strcat (buf, "-");
                if (st->st_mode & S_IXGRP)
                        strcat (buf, "x");
                else
                        strcat (buf, "-");

                /* Get other permissions */

                if (st->st_mode & S_IROTH)
                        strcat (buf, "r");
                else
                        strcat (buf, "-");
                if (st->st_mode & S_IWOTH)
                        strcat (buf, "w");
                else
                        strcat (buf, "-");
                if (st->st_mode & S_IXOTH)
                        strcat (buf, "x");
                else
                        strcat (buf, "-");

                /* Get SUID, SGID, SVTX, TCB*/

                if (st->st_mode & S_ISUID)
                        strcat (buf, ",SUID");
                if (st->st_mode & S_ISGID)
                        strcat (buf, ",SGID");
                if (st->st_mode & S_ISVTX)
                        strcat (buf, ",SVTX");
                if (st->st_mode & S_ITCB)
                        strcat (buf, ",TCB");

                strcat (buf, "\n");
                len = strlen(buf);
                if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
			error_write_temp();

                /* Add a extra line to the file and between stanzas) */

                strcpy (buf, "\n");
                len = strlen(buf);
                if (fwrite ((void *)buf,(size_t)1,(size_t)len,fdtemp) != len)
			error_write_temp();
	}
     }
        fclose(fdtemp);
        return (0);
}

/*
 * NAME:	get_links
 * PURPOSE:	Retrieve all hard links to the filename passed in
 * 
 * RETURNS:	Pointer passed in linkbuf if OK
 *		NULL, otherwise.
 */

char *
get_links (char *filename, char *linkbuf)
{
#include <IN/AFdefs.h> 
	int	i;
	int	lastcomma;
	long 	n;
	char	*end;
	char	*ptr;

	*linkbuf='\0';
	*linknames='\0';
	if (get_filesystem_name(filename)) {
		return(NULL);
	}
	/*
	 * Get the size of the required table and malloc it.
	 */
	
	if ((htab =
	     (struct htab *) malloc (HSIZE * sizeof(struct htab))) == NULL)
		return(NULL);

	check (fsname, filename);
	strcpy(linkbuf, linknames);

	/*
	 * One more thing - strip off the last comma...
	 */
	lastcomma=strlen(linkbuf)-1;
	if (lastcomma >= 0)
		linkbuf[lastcomma]='\0';
	return(linkbuf);
}

/*
 *
 *	NAME:	check
 *
 *	FUNCTION:
 *		ncheck one filesystem for a single file
 *
 */

int
check (char *filesystem, char *file)
{
	ino_t 			imax, ino;
	fdaddr_t		fmax;
	struct dinode 		dp;
	struct superblock 	super;

	
	if ((fd = fsopen (filesystem, O_RDONLY)) < 0)
		return (-1);
	
	if (get_super (fd, &super) != LIBFS_SUCCESS)
		return(-1);

	/*
	 * Get the max number of inodes and blocks
	 */	
	fsmax (&imax, &fmax);
	if ((imax == 0) || (fmax.f.addr <= 0))
		return(-1);

	nhent = 0;
	memset (htab, 0 , sizeof(struct htab) * hsize);

	/*
	 *	pass1
	 *		Store all directory inodes in the
	 *		hash table
	 *
	 */
	for (ino = 0; ino < imax; ino++)
	{
		if (ino == INODES_I)
			continue;
		if ((get_inode (fd,  &dp, ino )) != LIBFS_SUCCESS)
			continue;
		if (dp.di_nlink > 0) {
		        if ((dp.di_mode & IFMT) == IFDIR) {
				/* 
				 * If this is a dir, hash it (to save it)
			 	*/
				lookup (ino, 1);
			}
		}
	}

	/*
	 * Pass2:
	 *	Save the name and parent inode number
	 *	of all hashed directory inodes found in pass1.
	 */
	for (ino = 0; ino < imax; ino++)
	{
		if (ino == INODES_I)
			continue;
		if ((get_inode (fd, &dp, ino )) != LIBFS_SUCCESS)
			continue;
		if (dp.di_nlink > 0)
			pass2(&dp, ino, super.s_bsize);
	}
/*
	for (hp = &htab[i % hsize]; hp->h_ino;)
	{
		if (hp->h_ino == i)
			return (hp);
		if (++hp >= &htab[hsize])
			hp = htab;
	}
*/
	/*
	 * Pass3:
	 *	Print the path to each allocated inode.
	 */
	for (ino = 0; ino < imax; ino++)
	{
		if (ino == INODES_I)
			continue;
		if ((get_inode (fd, &dp, ino)) != LIBFS_SUCCESS)
			continue;
		if (dp.di_nlink > 0)
			pass3(&dp, ino, super.s_bsize,file);
	}
}



/*
 *
 *	NAME:	pass2
 *
 *	FUNCTION:
 *		Process all directory inodes and their contents.
 *
 *		Store dir name and inode number
 *		in htab for all directories hashed in pass1.
 *
 */

void
pass2 (struct dinode 	*ip,
       ino_t		inode_num,
       int  		sb_bsize)
{

	char 		dbuf[BLKSIZE];
	struct direct 	*dp;
	register 	i, j, l;
	int 		numb, frg_len;
	struct htab 	*hp;
	long  		dirsize;

	if ((ip->di_mode & IFMT) != IFDIR)
		return;
	/*
	 * Get the directory size and the number of blocks.
	 */
	dirsize = ip->di_size;
	numb = BYTE2BLK (ip->di_size + sb_bsize - 1);

	/*
	 * go through all the blocks of this inode.
	 */
	for (i = 0; i < numb; i++)
	{
		/*
		 * Read a directory frag and go through all its
		 * directory chunks.
		 */
		frg_len = dirblk_read (ip, i, &dbuf, inode_num);
		
		for (j = 0; j < frg_len / DIRBLKSIZ; j++)
		{
			/*
			 * pick the next  directory chunk
			 */
			dp = (struct direct *) &dbuf[j * DIRBLKSIZ];

			/*
			 * For each entry in the chunk save the
			 * parent inode number and name of all allocated,
			 * directory, non-dot, inodes in this chunk (dp).
			 */
			for (l = 0; l < DIRBLKSIZ; l += dp->d_reclen,
			     dp = (struct direct *) ((char *) dp +
						     dp->d_reclen))
			{
				if (dp->d_ino == 0)
					continue;
				/*
				 * look for the inode - only directory
				 * inodes are hashed.
				 */
				if ((hp = lookup(dp->d_ino, 0)) ==
				    (struct htab*) NULL)
				{
					continue;
				}
				/*
				 * forget . and ..
				 */
				if (dotname(dp))
					continue;

				/*
				 * Save the name and parent ino_t
				 */
				hp->h_pino = inode_num;
				strcpy (hp->h_name, dp->d_name);
			}
			if (dirsize <= ((j+1) * DIRBLKSIZ))
				break;
		}
		dirsize -= frg_len;		
	}
}

/*
 *
 *	NAME:	pass3
 *
 *	FUNCTION:
 *		Print the path to each inode
 *
 */

void
pass3 (struct dinode	*ip,
       ino_t		inode_num,
       int		sb_bsize,
       char		*filename)
{
	char 		dbuf [BLKSIZE];
	struct direct 	*dp;
	register 	i,j, k, l;
	int 		numb, frg_len;
	ino_t 		kno;
	long  		dirsize;

	
	if ((ip->di_mode & IFMT) != IFDIR)
		return;

	/*
	 * Get the directory size and its number of blocks
	 */
	dirsize = ip->di_size;
	numb = BYTE2BLK(ip->di_size + sb_bsize - 1);

	/*
	 * go through all the blocks
	 */
	for (i = 0; i < numb; i++)
	{
		/*
		 * Read a directory frag and go through all its
		 * directory chunks.
		 */
		frg_len = dirblk_read(ip, i, dbuf, inode_num);

		for (j = 0; j < frg_len / DIRBLKSIZ; j++)
		{
			/*
			 * pick the next  directory chunk
			 */
			dp = (struct direct *) &dbuf[j * DIRBLKSIZ];

			/*
			 * For each entry in the chunk print the
			 * full path to each allocated inode.
			 */			
			for (l = 0; l < DIRBLKSIZ; l += dp->d_reclen,
			     dp = (struct direct *) ((char *) dp +
						     dp->d_reclen))
			{
				if (dp->d_ino == 0)
					continue;
				kno = dp->d_ino;

                                if (dotname(dp))
                                        continue;

				if (kno==ilist)
					goto pr;
				continue;
			
			pr:
				*tempname='\0';
				if (strcmp(fsname,"/"))
					strcat(tempname, fsname);
				/*
				 * print the path to kno
				 */
				pname (inode_num, 0);
				strcat(tempname,"/");
				strcat(tempname,dp->d_name);
				if (strcmp(filename,tempname)) {
					strcat (linknames, tempname);
					strcat(linknames, ",");
				}
			}
			if (dirsize <= ((j+1) * DIRBLKSIZ))
				break;	
		}
		dirsize -= frg_len;
	}
}		

/*
 *
 *	NAME:	dirblk_read
 *
 *	FUNCTION:
 *		Read directory block corresponding to the logical block
 *		(lbno) for inode *d_inode.
 *
 *	RETURN:
 *		return the size of the frag in bytes if successful
 *		return 0 if failure.
 */

int
dirblk_read (struct dinode	*d_inode,
	     int  		lbno,
	     char  		*bp,
	     ino_t		inode_num)		
{

	fdaddr_t  frg;

	if (ltop (fd, &frg.f, d_inode, lbno) != LIBFS_SUCCESS)
	{
		return (0);
	}
	if (frg.f.addr == 0)
		return (0);

	if (bread (fd, bp, frg.f) < 0)
	{
		return (0);
	}
	return (FRAGLEN(frg.f));
}

/*
 *
 *	NAME:	dotname
 *
 *	FUNCTION:
 *		check for names '.' or '..'
 *
 *	RETURN:
 *		return 1 if d_name == '.' or '..'
 */

int
dotname (register struct direct *dp)
{

	if (dp->d_name[0]=='.')
	{
		if (dp->d_name[1] == 0 || (dp->d_name[1] == '.' &&
					   dp->d_name[2] == 0))
		{
			return (1);
		}
	}
	return(0);
}

/*
 *
 *	NAME:	pname
 *
 *	FUNCTION:
 *		print file names
 *
 */

void
pname (ino_t i,
      int lev)
{
	register struct htab *hp;

	if (i == ROOTINO)
		return;
	if ((hp = lookup(i, 0)) == 0)
	{
		strcat(linknames, "???");
		return;
	}
	if (lev > 10)
	{
		strcat(linknames, "...");
		return;
	}
	pname(hp->h_pino, ++lev);
	strcat(tempname, "/");
	strcat(tempname, hp->h_name);
}

/*
 *
 *	NAME:	lookup
 *
 *	FUNCTION:
 *		Search htab for the given inode number (i).
 *		Save the inode number if not found and ef != 0.
 *		Handle collisions by moving to next slot
 *
 *	RETURN:
 *		Return a pointer into htab if the inode number was found
 *		or it was not found but was addedd to htab (ie ef != 0).
 *		Return Null if inode number was not found and ef == 0.
 *		Exit with error if HSIZE exceeded (bogus).
 */

struct htab
*lookup (ino_t	i,	/* inode number to to locate	*/
	int 	ef)	/* Side effect			*/
{
	register struct htab *hp;

	/*
	 * search for ino_t i
	 */
	for (hp = &htab[i % hsize]; hp->h_ino;)
	{
		if (hp->h_ino == i)
			return (hp);
		if (++hp >= &htab[hsize])
			hp = htab;
	}
	Dprintf ("i = %d nhent = %d \n", i, nhent);
	/*
	 * Inode not found - process the side effect
	 * (that is: save the inode number and return its
	 * corresponding struct htab)
	 */
	if (ef == 0)
		return (struct htab *) NULL;

	/*
	 * Only print the error message once.
	 */
	if (nhent == hsize - 1)
	{
		nerror++;
		nhent++;		
		return (struct htab *) NULL;
	}
	else if (nhent >= hsize)
		return (struct htab *) NULL;

	nhent++;
	hp->h_ino = i;
	return hp;
}


int
get_filesystem_name(char *filename)
{
        struct  vmount  *vmtab; /* Mount entries returned by kernel         */
        struct  vmount  *vm;    /* Cursor into mount table                  */
        int     i;              /* Loop index                               */
        int     size;           /* Size of mount table                      */
        int     nmount;         /* Number of entries in mount table         */
	struct	stat	statbuf;/* stat info buffer for vfs number          */
	ulong_t		vfsnum;	/* the vfs number of the filename	    */
	char		buf[4096];



	if (lstat(filename, &statbuf) < 0)
		return(-1);

	vfsnum=statbuf.st_vfs;
	ilist=statbuf.st_ino;

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
                if (!(vmtab = (struct vmount *) malloc (size)))
			return(-1);
                nmount = mntctl (MCTL_QUERY, size, vmtab);

                if (nmount > 0 && (vmtab->vmt_revision != VMT_REVISION))
                        return(-1);

                if (nmount < 0)
			return(-1);

                if (nmount == 0) {
                        size = vmtab->vmt_revision;
                        free (vmtab);
                        continue;
                }
                break;
        }

        /*
         * Now scan the mount table for the entry with the 
         * same vfs number as the one we found for the file.
         */
        for (vm = vmtab, i = 0;i < nmount;i++, vm = vm_next (vm)) {
		if (vfsnum == vm->vmt_vfsnumber) {
                	size = vmt2datasize (vm, VMT_STUB) + 1;
                	strncpy (fsname, vmt2dataptr (vm, VMT_STUB), size - 1);
			free(vmtab);
			return(0);
		}
        }
        free (vmtab);
	return(-1);
}

