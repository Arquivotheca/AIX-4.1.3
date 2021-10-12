/* @(#)25	1.12.1.3  src/bos/usr/bin/pwdck/pwdck.h, cmdsadm, bos411, 9428A410j 10/12/93 10:00:08 */
/*
 *   COMPONENT_NAME: CMDSADM
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* size limits */

#define PWLINESIZE 4000	/* maximum size of line we can read from /etc/passwd */
#define TABLESIZE  200	/* initial size of password tables (entries) 	     */
#define MAXFLAGS   20	/* maximum number of flags we can deal with	     */

/* string constants */

#define PASSWORD 	"password"
#define FLAGS 		"flags"
#define LASTUPDATE 	"lastupdate"
#define	PASSFILE	"/etc/passwd"
#define SPASSFILE	"/etc/security/passwd"
#define	ACLPUT		"/usr/bin/aclput"
#define	PRIVPUT		"/usr/bin/privput"

/* audit events */

#define	PWDEVENT	"PASSWORD_Check"
#define	PWDERROR	"PASSWORD_Ckerr"

/* defines used 
 * for reporting */

#define FIX	1
#define NFIX	2

/* for buildsecpw() */

#define	NEW	0
#define	ADD	1

/* structure used to build 
 * the /etc/passwd table */

struct pwfile {
	char   *pwline;	/* actual line as it appears in /etc/passwd 	*/
	char   *user;	/* parsed user name field			*/
	char   *passwd;	/* parsed password field			*/
	short  check;	/* flag to indicate this line must be checked   */
	short  delete;	/* flag to indicate this line must be deleted 	*/
	};

/*
 * Type of lines in /etc/security/passwd file
 */

#define	SPW_NAME	1
#define	SPW_PASSWD	2
#define	SPW_LAST	3
#define	SPW_FLAGS	4
#define	SPW_COMMENT	5

/* structure used to build 
 * the /etc/security/passwd table */

struct	secpwline {
	int	type;
	char	*line;
	struct	secpwline *next;
	};

struct	pwdck	{
	struct	userpw	upw;	/* user's password structure	     	     */
				/* flags:				     */
	char  *raw_flags;	/* pointer to the exact flags attribute      */
	short password;	/* flag to indicate that the password field exists   */
	short flags;	/* flag to indicate that the flags field exists	     */
	short lastupdate;/*flag to indicate that the lastupdate field exists */
	short invalid_flags;	/* indicates an invalid value in 'flags ='   */
	short principle;/* flag to indicate the user has an authname entry   */
	short check;	/* flag to indicate this stanza must be checked      */
	short delete;	/* flag to indicate this entry should be deleted     */
	struct secpwline *lines; /* pointer to list of lines in each entry   */
	struct pwfile	*pwfile; /* pointer to entry from /etc/passwd        */
	};

/* strings corresponding to the message catalog: */

/* strings to indicate user database 
 * errors caught by pwdck */

#define DAUTH \
"The authname \"%s\" has no entry in /etc/security/passwd.\n"
#define DBADP \
"The user \"%s\" has an invalid password field in /etc/passwd.\n"
#define DDEL \
"Deleting the stanza for \"%s\" from /etc/security/passwd.\n"
#define DEPLINE \
"Bad line found in /etc/passwd:\n\"%s\"\n"
#define DEXIST \
"/etc/security/passwd does not exist.\n"
#define DFLG \
"The user \"%s\" has invalid flags specified.\n"
#define DIGN \
"\"%s\" ignored.\n"
#define DLINE \
"Bad line found in /etc/security/passwd:\n\"%s\"\n"
#define DLST \
"The user \"%s\" has an invalid lastupdate attribute.\n"
#define DMIS \
"The user \"%s\" is missing the lastupdate attribute.\n"
#define DMODIF \
"WARNING: The password files were modified by another process\nwhile pwdck was running. The other process' changes may be lost.\n"
#define DNAM \
"The user name \"%s\" in /etc/passwd is invalid.\n"
#define DNENT \
"The user \"%s\" was not found in /etc/passwd.\n"
#define DNEW \
"Adding \"%s\" stanza to /etc/security/passwd.\n"
#define DNFND \
"The stanza for \"%s\" was not found in /etc/security/passwd.\n"
#define DNPW \
"The user \"%s\" has an invalid password attribute.\n"
#define DOLDDBM \
"The DBM files for /etc/passwd are out of date.\nUse the mkpasswd command to update these files.\n"
#define DUAUTH \
"The user \"%s\"in /etc/security/passwd is not specified as an\nauthentication name in /etc/security/user.\n"
#define DORDER \
"Sorting /etc/security/passwd in the same order as /etc/passwd.\n"
#define DSORT \
"/etc/security/passwd is not in the same order as /etc/passwd.\n"

/* strings used for pwdck interactive mode. 
 * they all require yes/no answers. */

#define DFAUTH \
"add a stanza to /etc/security/passwd ?"
#define DFBADP \
"replace it with '!' and transfer original password ?"
#define DFEPLINE \
"remove it ?"
#define DFEXIST \
"create /etc/security/passwd ?"
#define DFFLG \
"remove the invalid flags ?"
#define DFLST1 \
"remove it ?"
#define DFLST2 \
"update it ?"
#define DFMIS \
"add it ?"
#define DFMODIF \
"overwrite the files anyway ? "
#define DFNAM \
"remove the entry ?"
#define DFNPW \
"make 'password = *' ?"
#define DFUAUTH \
"remove the stanza from /etc/security/passwd ?"
#define DFSORT \
"sort it ?"

/* strings to indicate 
 * system errors */

#define DACLGET \
"pwdck: could not get acl of \"%s\".\n"
#define DACLPUT \
"pwdck: could not put acl on \"%s\".\n"
#define DATTR \
"pwdck: could not get attribute for user \"%s\".\n"
#define DCHOWN \
"pwdck: could not chown %s.\n"
#define DCREAT \
"pwdck: could not create /etc/security/passwd.\n"
#define DEPENT \
"pwdck: no entries were found in /etc/passwd.\n"
#define DGETCONF \
"pwdck: could not read password restrictions.\n"
#define DLOCK \
"pwdck: could not lock on \"%s\". Please try again later.\n"
#define DMALLOC \
"pwdck: could not allocate memory.\n"
#define DMKTEMP \
"pwdck: could not make temp file name.\n"
#define DOPENDB \
"pwdck: could not open user data base.\n"
#define DOPENEP \
"pwdck: could not open /etc/passwd.\n"
#define DOPENESP \
"pwdck: could not open /etc/security/passwd.\n"
#define DOPENTEP \
"pwdck: could not open temporary /etc/passwd.\n"
#define DOPENTESP \
"pwdck: could not open temporary /etc/security/passwd.\n"
#define DPCLGET \
"pwdck: could not get pcl of \"%s\".\n"
#define DPCLPUT \
"pwdck: could not put pcl on \"%s\".\n"
#define DPOPEN \
"pwdck: could not popen to run \"%s\".\n"
#define DREALLOC \
"pwdck: could not reallocate memory.\n"
#define DCOPYTEMP \
"pwdck: could not copy temp files.\n"
#define DSTAT \
"pwdck: could not stat %s.\n"
#define DTCBATTR \
"pwdck: could not get tcb attributes for /etc/security/passwd.\n"
#define	DTIME \
"pwdck: could not get last modified time of \"%s\".\n"
#define DUNLNK \
"pwdck: could not unlink temporary file %s.\n"
#define DUSG \
"usage: pwdck [ -p | -y | -n | -t ] [ user1 user2 ... | ALL ]\n"
#define DWRITE \
"pwdck: could not write into \"%s\".\n"

/* audit information 
 * messages */

#define	AEXIST		"/etc/security/passwd does not exist"
#define	AMIS		"Missing the lastupdate attribute"
#define	AMODIF		"Database modified by another process during pwdck"
#define	ASTAT		"Could not stat"
#define	ATIME		"Could not get last modified time"
#define AACLGET		"Could not get acl"
#define AACLPUT		"Could not put acl"
#define AATTR		"Could not get user attribute"
#define AAUTH		"No stanza in /etc/security/passwd"
#define ABADP 		"Invalid password field in /etc/passwd"
#define ACHOWN		"Could not chown"
#define ACREAT		"Could not create /etc/security/passwd"
#define ADEL		"Removing stanza from /etc/security/passwd"
#define AEPENT		"No entries found in /etc/passwd"
#define AEPLINE		"Bad line found in /etc/passwd"
#define AFLG		"Invalid flags in /etc/security/passwd"
#define AGETCONF	"Could not read password restrictions"
#define AIGN		"Checks ignored"
#define ALINE		"Bad line in /etc/security/passwd"
#define ALOCK		"Error setting file lock"
#define ALST 		"Invalid lastupdate attribute"
#define AMALLOC		"Could not allocate memory"
#define AMKTEMP		"Could not make temp file name"
#define ANAM 		"Invalid user name in /etc/passwd"
#define ANENT		"User not found in /etc/passwd"
#define ANEW		"Adding stanza to /etc/security/passwd"
#define ANFND		"Stanza not found in /etc/security/passwd"
#define ANPW 		"Invalid password attribute in /etc/security/password"
#define	AOLDDBM		"DBM files are out of date"
#define AOPENDB		"Could not open user data base"
#define AOPENEP		"Could not open /etc/passwd"
#define AOPENESP	"Could not open /etc/security/passwd"
#define AOPENTEP	"Could not open temporary /etc/passwd"
#define AOPENTESP	"Could not open temporary /etc/security/passwd"
#define APCLGET		"Could not get pcl" 
#define APCLPUT		"Could not put pcl"
#define APOPEN		"Could not popen"
#define AREALLOC	"Could not reallocate memory"
#define ACOPYTEMP	"Could not copy temp files"
#define ATCBATTR	"Could not gettcbattr for /etc/security/passwd"
#define AUAUTH		"Not an authentication name in /etc/security/user."
#define AORDER          "/etc/security/passwd is not in the same order as /etc/passwd."
#define ASORT		"Sorting /etc/security/passwd in the same order as /etc/passwd."
#define AUNLNK		"Could not unlink temporary file"
#define AUSG 		"Bad usage"
#define AWRITE		"Could not write file"

/* C function prototypes */

int	buildpw(char **users, int fd);
int	buildsecpw(int op, char	*user, char *passwd, int fd);
int	ckauth(struct pwfile *pwtp);
int	ckauthnames();
int	ckusername(struct pwfile *ptabp);
int	ckpwfield(struct pwfile *tabptr);
int 	ckuserpw(struct pwfile *ptabp);
int	convflags(ulong *flags, char *valptr);
char **	findsystem(char *authstr,  char *user);
void 	usage();
int 	pwrequired(char *uname);
int 	userfound(char *user, char **list);
void 	unlinktmp(char *tmpep, char *tmpesp);
char * 	pwmalloc(unsigned int size);
char * 	pwrealloc(char *ptr, unsigned int size);
int 	report(char *mp, char *ap, char *fxp, char *user, int cmd);
int 	pwexit(char *mstr, char *astr, char *usr);
int 	pwopen(char *file);
int 	pwfree(void *mem);
time_t 	gettime(char *filename);
struct pcl * priv_get(char *file);
int	priv_put(char *file, struct pcl *pcl, int flag);
int 	put_program(char *prog, char *arg, char *input);
char * 	pwdup(char *s);
int	writepw(int epfd, int espfd);
int	pwcopy(int sourcefd, int targetfd);

