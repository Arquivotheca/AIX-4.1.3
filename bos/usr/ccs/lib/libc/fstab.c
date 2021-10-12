static char sccsid[] = "@(#)57	1.4  src/bos/usr/ccs/lib/libc/fstab.c, libcenv, bos411, 9428A410j 4/20/94 17:46:48";
/*
 * COMPONENT_NAME: (LIBCENV) 
 *
 * FUNCTIONS: endfsent, getfsent, setfsent, getfsfile, getfsspec, getfstype
 *	fstabscan, endfsent_r, getfsent_r, setfsent_r, getfsfile_r, getfsspec_r,
 *	getfstype_r
 *
 * ORIGINS: 26,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <fstab.h>
#include <ctype.h>
#include <string.h>
#include "ts_supp.h"
#include "push_pop.h"
#include <sys/errno.h>

#define	FSFIELDS 6	/* number of fields returned by fstabscan() */
/*
 * FUNCTION: return static file system information
 *
 * EXTERNAL PROCEDURES CALLED:
 */

extern char * AFgetatr();  /* gets attribute */

/* The following library functions are included in this file:
	endfsent - closes the file.
	getfsent - gets the next file system description.
	getfsfile - searchs file system file for name.
	getfsspec - searchs file system file for special file name.	
	getfstype - searchs file system file for type.
	setfsent - opens and rewinds the file.
 */

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _AF_rmutex;
#define FS_FILE (*fs_file)
#define PASS_NO (*pass_no)
#else

static	struct fstab fs; /* fstab structure returned to the calling procedure */
static	AFILE_t fs_file = 0;   /* attribute file information */
static  int pass_no;           /* the pass number for the current fs */
#define PASS_NO (pass_no)
#define FS_FILE (fs_file)

#endif

/*
 * NAME: fstabscan
 *
 */
#ifdef _THREAD_SAFE
static
fstabscan(struct fstab *fs, AFILE_t *fs_file, int *pass_no)    
#else
static
fstabscan(struct fstab *fs)
#endif
{
	register ATTR_t cp;   /* attribute pair structure */
        register char *m;     /* temp string holder */
	int readonly = 0;     /* readonly flag */
	int quotas = 0;       /* quotas flag */

	cp = AFnxtrec(FS_FILE);
	if (cp == NULL)
		return (EOF);
	fs->fs_file = cp->AT_value;	/* stanza name */
	fs->fs_spec = AFgetatr(cp,"dev");

	m =  AFgetatr(cp,"mount"); 
	while (*m)
	{
		if (strcmp(m,"readonly")==0)
			readonly++;
		m += strlen(m) +1;
	}

	m =  AFgetatr(cp,"options"); 
	while (*m)
	{
		if (strcmp(m,FSTAB_RO)==0)
			readonly++;
		m += strlen(m) +1;
	}

	if (AFgetatr(cp,"quota"))
		quotas++;

	if (readonly > 0)
		fs->fs_type = FSTAB_RO;
	else if (quotas > 0)
		fs->fs_type = FSTAB_RQ;
	else
		fs->fs_type = FSTAB_RW;

	m =  AFgetatr(cp, "check"); 
	if (m == NULL || !strcmp(m, "false"))
		fs->fs_check = -1;
	else if (!strcmp(m, "true"))
		fs->fs_check = 0;
	else
		fs->fs_check = atoi(m);

	fs->fs_freq = 0;
	fs->fs_passno = ++PASS_NO;
	readonly = 0;
	return (FSFIELDS);
}
	
/*
 * NAME: setfsent
 *
 * FUNCTION: opens and rewinds the file system file.
 *
 * RETURN VALUE DESCRIPTION: 
 *     returns 0 if error otherwise 1.
 */
#ifdef _THREAD_SAFE
int 
setfsent_r(AFILE_t *fs_file, int *pass_no)
{
	int rc = 0;

	TS_LOCK(&_AF_rmutex);
	TS_PUSH_CLNUP(&_AF_rmutex);
        PASS_NO = 0;
	if (FS_FILE)
		endfsent_r(fs_file);
	if (FS_FILE == NULL) {
	    if ((FS_FILE = AFopen(FSYSname,MAXREC,MAXATR)) == NULL) {
		FS_FILE = 0;
		rc = -1;
	    }
	}
	TS_POP_CLNUP(0);
	TS_UNLOCK(&_AF_rmutex);
	return(rc);
}

#else
int
setfsent()          
{

        PASS_NO = 0;
	if (FS_FILE)
		endfsent();
	    if ((FS_FILE = AFopen(FSYSname,MAXREC,MAXATR)) == NULL) {
		FS_FILE = 0;
		return (0);
	    }
	return (1);
}
#endif

/*
 * NAME: endfsent
 *
 * FUNCTION: closes the file system file.
 *
 * RETURN VALUE DESCRIPTION: 
 *	returns 0 if error otherwise 1
 */
#ifdef _THREAD_SAFE
int
endfsent_r(AFILE_t *fs_file)
{
	TS_LOCK(&_AF_rmutex);
	TS_PUSH_CLNUP(&_AF_rmutex);
	if (FS_FILE) {
		AFclose(FS_FILE);
		FS_FILE = 0;
	}
	TS_POP_CLNUP(0);
	TS_UNLOCK(&_AF_rmutex);
	return(0);
}
#else

int
endfsent()           
{

	if (FS_FILE) {
		AFclose(FS_FILE);
		FS_FILE = 0;
	}
	return (1);
}
#endif

/*
 * NAME: getfsent
 *
 * FUNCTION: reads the next record of the file opening if necessary.
 *
 * RETURN VALUE DESCRIPTION: 
 *    returns a pointer to an object with fstab structure
 *    a null pointer is return if EOF or error
 */
#ifdef _THREAD_SAFE
int
getfsent_r(struct fstab *fs, AFILE_t *fs_file, int *pass_no)
{

	int nfields;    /* number of fields filled by fstabscan */

	if ((FS_FILE == 0) && (setfsent_r(fs_file, pass_no) == -1)){
		fs = ((struct fstab *)0);
		return (-1);
	}
	TS_LOCK(&_AF_rmutex);
	TS_PUSH_CLNUP(&_AF_rmutex);
	nfields = fstabscan(fs, fs_file, pass_no);
	TS_POP_CLNUP(0);
	TS_UNLOCK(&_AF_rmutex);
	if (nfields == EOF || nfields != FSFIELDS) {
		fs = (struct fstab *)0;
		return (-1);
	}
	return (0);
}

#else

struct fstab *
getfsent()   
{
	int nfields;    /* number of fields filled by fstabscan */

	if ((FS_FILE == 0) && (setfsent() == 0))
		return ((struct fstab *)0);
	nfields = fstabscan(&fs);
	if (nfields == EOF || nfields != FSFIELDS) {
		return ((struct fstab *)0);
	}
	return (&fs);
}
#endif

/*
 * NAME: getfsspec
 *
 * FUNCTION: searches for name in the file system spec field. 
 *
 * RETURN VALUE DESCRIPTION: 
 * 	returns a pointer to an object with fstab structure or a null
 *      pointer if name is not found or error.
 */
#ifdef _THREAD_SAFE
int
getfsspec_r(const char *name,struct fstab *fsp, AFILE_t *fs_file, int *pass_no)
{
	if (setfsent_r(fs_file, pass_no) == -1) {
		fsp = (struct fstab *)0;
		errno = ENOENT;
		return (-1);
	}
	while (getfsent_r(fsp, fs_file, pass_no) != -1)
		if (strcmp(fsp->fs_spec, name) == 0) {
			return (0);
	}
	fsp = (struct fstab *)0;
	errno = ENOENT;
	return(-1) ;
}
#else
struct fstab *
getfsspec(name)   
	char *name;     /* special file name that is being searched for */
{
	register struct fstab *fsp;  /* current file system description */

	if (setfsent() == 0)	/* start from the beginning */
		return ((struct fstab *)0);
	while((fsp = getfsent()) != 0)
		if (strcmp(fsp->fs_spec, name) == 0) {
			return (fsp);
	}
	return ((struct fstab *)0);
}

#endif
/*
 * NAME: getfsfile
 *
 * FUNCTION: searches for name in the file system file field. 
 *
 * RETURN VALUE DESCRIPTION: 
 *  	returns a pointer to an object with fstab structure or return a
 *	null pointer if name is not found or error.
 */
#ifdef _THREAD_SAFE
int
getfsfile_r(char *name,struct fstab *fsp, AFILE_t *fs_file, int *pass_no)
{
	if (setfsent_r(fs_file,pass_no) == -1) {
		fsp = (struct fstab *)0;
		errno = ENOENT;
		return (-1);
	}
	while (getfsent_r(fsp, fs_file, pass_no) != -1) {
		if (strcmp(fsp->fs_file, name) == 0) {
			return (0);
		}
	}
	fsp = (struct fstab *)0;
	errno = ENOENT;
	return (-1);
}
#else
struct fstab *
getfsfile(name) 
	char *name;  /* file system name being searched for */
{
	register struct fstab *fsp;   /* current file system description */

	if (setfsent() == 0)	/* start from the beginning */
		return ((struct fstab *)0);
	while ((fsp = getfsent()) != 0)
		if (strcmp(fsp->fs_file, name) == 0) {
			return (fsp);
	}
	return ((struct fstab *)0);
}

#endif
/*
 * NAME: getfstype
 *
 * FUNCTION: searches for type in the file system type field. 
 *
 * RETURN VALUE DESCRIPTION: 
 *  	returns a pointer to an object with fstab structure or return a
 *	null pointer if type is not found or error.
 */
#ifdef _THREAD_SAFE
int
getfstype_r(const char *type, struct fstab *fs, AFILE_t *fs_file, int *pass_no)
{

	while (getfsent_r(fs, fs_file,pass_no) != -1)
		if (strcmp(fs->fs_type, type) == 0) {
			return (0);
	}
	fs = (struct fstab *)0;
	return (-1);
}
#else
struct fstab *
getfstype(type) 
	char *type;      /* file system type being searched for */
{
	register struct fstab *fs;  /* current file system description */

	while ((fs = getfsent()) != 0)
		if (strcmp(fs->fs_type, type) == 0) {
			return (fs);
	}
	return ((struct fstab *)0);
}
#endif
