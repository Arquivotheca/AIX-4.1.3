static char sccsid[] = "@(#)64  1.4  src/bos/usr/ccs/lib/libc_r/getpwent.c, libcs, bos41J, 9515B_all 4/12/95 12:44:46";
/*
 *   COMPONENT_NAME: LIBCS
 *
 *   FUNCTIONS: PWSCAN, SETPWENT, __shadow_chk, __shadow_pass, __stanzmat,
 *		endpwent, endpwent_r, fgetpwent_r, getpwent_r, pwscan, pwskip,
 *		setpwent, setpwent_r, setpwfile
 *
 *   ORIGINS: 27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <pwd.h>
#include <ndbm.h>
#include <userpw.h>
#include <sys/types.h>
#include <sys/access.h>

#include "ts_supp.h"
#include "push_pop.h"

extern void __shadow_chk(struct passwd *);
extern char * __shadow_pass(char *u);
extern int __stanzmat(char *, char *);

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex	_passwd_rmutex;

#define PW_FP			(*pw_fp)

#define	SETPWENT()		setpwent_r(pw_fp)
#define	PWSCAN(pwent, line)	pwscan(pwent, line, len, PW_FP)

#else

#define	MAXLINELENGTH	1024

#define PW_FP			pw_fp

#define	SETPWENT()		setpwent()
#define	PWSCAN(pwent, line)	pwscan(pwent, line, MAXLINELENGTH, PW_FP)

static struct passwd		pw_passwd;
static FILE			*pw_fp;
static char			line[MAXLINELENGTH];

#endif /* _THREAD_SAFE */

#define PWD_FILENAME    "/etc/passwd"
#define SPWD_FILENAME   "/etc/security/passwd"

/*
 * The following are shared with getpwnamuid.c
 */
char	*_pw_file = PWD_FILENAME ;
DBM	*_pw_db;


static char *
pwskip(register char *p)
{
	while (*p && *p != ':' && *p != '\n')
		++p;
	if (*p)
		*p++ = 0;
	return (p);
}


static int
pwscan(struct passwd *pwent, char *line, int len, FILE *fp)
{
	register char	*p;
	register int	c;

	for (;;) {
		if (!(p = fgets(line, len, fp)))
			return (0);

		/* line without colon separators is no good, so ignore it */
		if(!strchr(p,':'))
		    continue;

		/* skip lines that are too big */
		if (!index(line, '\n')) {
			while ((c = getc(fp)) != '\n' && c != EOF)
				;
			continue;
			
		}
		break;
	}
	pwent->pw_name = p;
	p = pwskip(p);
	pwent->pw_passwd = p;
	p = pwskip(p);
	pwent->pw_uid = (uid_t)strtoul(p, NULL, 10);
	p = pwskip(p);
	pwent->pw_gid = (gid_t)strtoul(p, NULL, 10);
	p = pwskip(p);
	pwent->pw_gecos = p;
	p = pwskip(p);
	pwent->pw_dir = p;
	p = pwskip(p);
	pwent->pw_shell = p;
	while (*p && *p != '\n')
		p++;
	*p = '\0';
	return (1);
}


#ifdef _THREAD_SAFE
int
fgetpwent_r(FILE *fp, struct passwd *pwent, char *line, int len)
{
	FILE	**pw_fp = &fp;
#else
struct passwd *
fgetpwent(FILE *pw_fp)
{
	register struct passwd	*pwent = &pw_passwd;
#endif	/* _THREAD_SAFE */

	TS_EINVAL((pwent == 0 || line == 0 || len <= 0));

	if (!PWSCAN(pwent, line))
		return (TS_NOTFOUND);
	__shadow_chk(pwent);
	return (TS_FOUND(pwent));
}


#ifdef _THREAD_SAFE
int
getpwent_r(struct passwd *pwent, char *line, int len, FILE **pw_fp)
{
	register int pwrc;
	pwrc = _getpwent_r(pwent, line, len, pw_fp);
#else
struct passwd *
getpwent(void)
{
	register struct passwd	*pwent = &pw_passwd;
	register struct passwd	*pwrc;
	pwrc = _getpwent(void);
#endif	/* _THREAD_SAFE */
	if (pwrc != TS_FAILURE)
		__shadow_chk(pwent);
	return (pwrc);
}

#ifdef _THREAD_SAFE
int
_getpwent_r(struct passwd *pwent, char *line, int len, FILE **pw_fp)
{
#else
struct passwd *
_getpwent(void)
{
	register struct passwd	*pwent = &pw_passwd;
#endif	/* _THREAD_SAFE */

	TS_EINVAL((pw_fp == 0 || pwent == 0 || line == 0 || len <= 0));

	if (!PW_FP && SETPWENT() != TS_SUCCESS || !PWSCAN(pwent, line))
		return (TS_NOTFOUND);
	return (TS_FOUND(pwent));
}


void
setpwfile(const char *file)
{
	TS_LOCK(&_passwd_rmutex);

	_pw_file = (char *) file;

	TS_UNLOCK(&_passwd_rmutex);
}


#ifdef _THREAD_SAFE
int
setpwent_r(FILE **pw_fp)
#else
int
setpwent(void)
#endif /* _THREAD_SAFE */
{
	int	flags;
	int	fail = 1;

	TS_EINVAL((pw_fp == NULL));

	if (PW_FP) {
		rewind(PW_FP);
		return (TS_SUCCESS);
	}

	TS_LOCK(&_passwd_rmutex);
	TS_PUSH_CLNUP(&_passwd_rmutex);

	if ((PW_FP = fopen(_pw_file, "r")) != NULL) {
		flags = fcntl(fileno(PW_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(PW_FP), F_SETFD, flags) == 0) {
		    fail = 0;
		} else 
			(void)fclose(PW_FP);
	}

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_passwd_rmutex);

      if (!fail)
	  return (TS_SUCCESS);

	return (TS_FAILURE);
}


#ifdef _THREAD_SAFE
void
endpwent_r(FILE **pw_fp)
#else
void
endpwent(void)
#endif /* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (pw_fp != NULL)
#endif	/* _THREAD_SAFE */

	if (PW_FP != NULL) {
		(void)fclose(PW_FP);
		PW_FP = NULL;
#ifdef _THREAD_SAFE
		return;
#endif	/* _THREAD_SAFE */
	}

	TS_LOCK(&_passwd_rmutex);
	TS_PUSH_CLNUP(&_passwd_rmutex);

	if (_pw_db != (DBM *)0) {
		dbm_close(_pw_db);
		_pw_db = (DBM *)0;
	}

	TS_POP_CLNUP(0);
	TS_UNLOCK(&_passwd_rmutex);
}

static char *
__shadow_pass(char *u)
{
#	define iswhite(c) ((c) == '\t' || (c) == ' ' || (c) == '\n')

	char line[BUFSIZ+1];
	long start;
	FILE *spwf;

	/**********
	  open the /etc/securty/passwd
	**********/
	if ((spwf = fopen(SPWD_FILENAME, "r")) == NULL)
		return(NULL);

	while (fgets(line, BUFSIZ, spwf) != NULL) {
		if (!iswhite(*line) && __stanzmat(line, u) == 0)
		{
			/* get the password from the stanza */
			for (;;)
			{
				static char pass[MAX_PASS+1];
				char *l, *p;

				l = line;
				p = pass;
				if (fgets(line, BUFSIZ, spwf) == NULL)
					break;
				if (*l == '*' || *l == '#')
					continue;
				if (!isspace(*l))
					break;

				while (isspace(*l))
					l++;
				if (strncmp(l, "password", 8) == 0)
				{
					l += 8;
					while (isspace(*l))
						l++;
					if (*l++ != '=')
						continue;
					while (isspace(*l))
						l++;
					while (*l && !isspace(*l))
						*p++ = *l++;
					*p = '\0';
					fclose(spwf);
					return pass;
				}
			}
			fclose(spwf);
			return NULL;
		}
	}
	fclose(spwf);
	return (NULL);
}

/*
 * shadow_chk -	check for shadowed password entries
 *
 * Input:
 *	p	-	^ to password entry structure
 *
 * Returns:
 *	nothing
 *
 * Dexcription:
 *	if shadowing not enabled for this entry, the entry itself
 *	otherwise, the shadow, if it exists, otherwise "*", which cannot match
 *	the encrypted text of any password string.
 */
void
__shadow_chk(register struct passwd *p) 
{ 	char 		*pass;
	static	char	buf[MAX_PASS+1];
	char 		*name2use;

	/* 
	 * The "!" indicates a shadow password file is being used.
	 */
	if (p) {
		if (strcmp(p->pw_passwd, "!") == 0)	/* shadowed entry?	*/
		{
			name2use=p->pw_name;
	
			if (accessx(SPWD_FILENAME,R_ACC,ACC_SELF) ||
			    (p->pw_passwd = __shadow_pass(name2use)) == NULL)
				p->pw_passwd = "*";		/* no, force NOLOGIN*/
		} 	/* (strcmp)  else no change		*/
	}
}
static int
__stanzmat(char *sz, char *s)
{
	/* matches a stanza colon line against a username (no colon)
	 * returns 0 if	they match, -1 if not
	 */

	while (*sz == *s && *sz	&& *s && *sz !=	':') {
		sz++;
		s++;
	}
	if (*sz	== ':' && *s ==	'\0')
		 return	0;
	return -1;
}

/*
 *	This routine in libc_r is just a stub.  The real 
 *	routine in libc resets a static variable named authstate
 *	in the _pwjunk structure used for NIS and DCE lookups.  
 *	_pwjunk is not defined in libc_r because NIS and DCE lookups 
 *	are not thread-safe.  But any threaded program binding to libs 
 *	still needs to find this routine.
 */
void
reset_pwjunk_authstate()
{
	return;
}
