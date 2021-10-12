static char sccsid[] = "@(#)01	1.15  src/bos/usr/ccs/lib/libc/getttyent.c, libcio, bos411, 9428A410j 4/20/94 17:48:44";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: endttyent, setttyent, getttyent 
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
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <string.h>
#include <ttyent.h>
#include <stdio.h>
#include <odmi.h>
#include <sys/dir.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/errno.h>
#include "ts_supp.h"
#include "push_pop.h"

/*  The following library functions are include in this file:
 *	endttyent - closes the file.
 *	setttyent - opens and rewinds the file.
 *      getttyent - gets the next tty description.
 */

/* must link this programs which use getttyent with libcfg.a and libodm.a */

static getpstat(char *);

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _odm_rmutex;

#define SETTTYENT(f)	setttyent_r(f)
#define ENDTTYENT()	endttyent_r(ttyinfo)
int getget_r();
int get_tty_info_r();

#else
#define SETTTYENT(f)	setttyent()
#define ENDTTYENT()	endttyent()
static struct ttyent *curinfo = NULL;
static struct ttyent *ttyinfo = NULL;
static char whatflg();
static stanname();
static getact();
static getcmd();
static isgetty();
static getget();
static struct ttyent *get_tty_info();

#endif

/*
 * NAME: endttyent
 *
 * FUNCTION: closes the file
 *
 */

#ifdef _THREAD_SAFE
void
endttyent_r(struct ttyent **ttyinfo)
{
    /* free ttyinfo and ty_getty field */
    struct ttyent *curinfo;

    if (ttyinfo == NULL) {
	errno = EINVAL;
	return;
    }

    for (curinfo = *ttyinfo; curinfo->ty_name; curinfo++)
	free (curinfo->ty_name);
	free (curinfo->ty_getty);
	free (curinfo->ty_type);
    free (*ttyinfo);
    *ttyinfo = NULL;
}
#else
void
endttyent()
{
    /* free ttyinfo and ty_getty field */
    struct ttyent *curinfo;

    for (curinfo = ttyinfo; curinfo->ty_name; curinfo++) {
	free (curinfo->ty_name);
	free (curinfo->ty_getty);
	free (curinfo->ty_type);
    }
    free (ttyinfo);
    ttyinfo = NULL;
}
#endif


/*
 * NAME: setttyent
 *
 * FUNCTION: rewinds the file
 *
 */
#ifdef _THREAD_SAFE
void
setttyent_r(struct ttyent **curinfo)
#else
void
setttyent()
#endif
{
#ifdef _THREAD_SAFE
    if (curinfo != NULL){
	endttyent_r(curinfo);
    }
#endif /* _THREAD_SAFE */
    curinfo = NULL;
}

/*
 * NAME: getttyent
 *
 * FUNCTION: gets next tty record from the file
 *
 * RETURN VALUE DESCRIPTION: 
 *	returns a pointer to an object with ttyent structure or a null pointer
 *  	if EOF or error.
 */

#ifdef _THREAD_SAFE
int getttyent_r(char *linebuf, struct ttyent **curinfo, 
		struct ttyent *ttyinfo)
{ 
    int rc;

    /* ttyinfo can not be NULL */
    if (ttyinfo == NULL) {
	errno = EINVAL;
	return (-1);
    }

    if (curinfo[0] == NULL) {
	rc = get_tty_info_r(curinfo,linebuf);
	if (rc == -1) {
	    curinfo[0] = NULL;
	    return(-1);
	}
    }
    else {
        curinfo[0]++;
    }
    *ttyinfo = *curinfo[0];
    if (NULL == curinfo[0]->ty_type)
	return(-1);
    return(0);

}
#else
struct ttyent *
getttyent()
{
	int rc;
	
	if (NULL == ttyinfo)
	    curinfo = ttyinfo = get_tty_info();
	else if (NULL == curinfo)
	    curinfo = ttyinfo;
	else 
	    curinfo++;

	if (NULL == curinfo->ty_type)
	   return NULL;
	return(curinfo);
}
#endif

#define	MAXLLEN 2048		/* max inittab cmd line length (see init.h) */
#define	MAXACT 	20		/* max action length in inittab (see init.h) */
#define	MAXCMDL	1024		/* max command length in inittab (see init.h) */
#define BAD 255

#ifdef _THREAD_SAFE
#define TTYINFO		(*ttyinfo)
#else /* _THREAD_SAFE */
#define TTYINFO		ttyinfo
static FILE *fpinit;		/* FILE pointer for /etc/inittab */
static char linebuf[MAXLLEN];	/* inittab line bufffer */
#endif

#ifdef _THREAD_SAFE
int get_tty_info_r(struct ttyent **ttyinfo, char *linebuf)
#else
static struct ttyent *
get_tty_info()
#endif
{
    struct CuDv *cudv;
    struct CuDv *cudv_save;
    struct CuAt *cuat;
    struct listinfo cudv_info;
    char criteria[MAX_CRITELEM_LEN];
    int howmany, i;
    struct ttyent *ttyinfop;
    char devname[1024];
    char *itfn = "/etc/inittab";  	/* inittab file name */
#ifdef _THREAD_SAFE
    FILE *fpinit;
#endif
    int leave = 0;

    TS_LOCK(&_odm_rmutex);
    TS_PUSH_CLNUP(&_odm_rmutex);

    /* initialize the odm */
    odm_initialize(); 

    /* initialize the criteria */
    sprintf( criteria, "name like tty* and status = %d", AVAILABLE );

    /* get a list of tty objects */
    if ((cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,1,1))==
      (struct CuDv *) -1){
	leave = 1;
	goto pop_n_leave;
    }

    cudv_save = cudv;
    /* check the number returned */
    if (cudv_info.num == 0) {
	odm_free_list(cudv_save, &cudv_info);
#ifdef _THREAD_SAFE
	ttyinfo = NULL;
#endif /* _THREAD_SAFE */
	leave = 1;
	goto pop_n_leave;
    }

    if ((TTYINFO = (struct ttyent *)calloc((cudv_info.num+1), sizeof(struct ttyent))) == (struct ttyent *)NULL) {
	odm_free_list(cudv_save, &cudv_info);
#ifdef _THREAD_SAFE
	ttyinfo = NULL;
#endif /* _THREAD_SAFE */
	leave = 1;
	goto pop_n_leave;
    }

    /* loop for each tty */
    for(i = 0, ttyinfop = TTYINFO; i < cudv_info.num; i++, cudv++)
    {
	extern void *getattr(char *, char *, int, int *);
	extern char *strdup (char *);

	ttyinfop[i].ty_name = strdup(cudv->name);
	if ((cuat = getattr(cudv->name,"term",0,&howmany)) == NULL) {
	    break;
 	}
	if (howmany)
	    ttyinfop[i].ty_type = strdup (cuat->value);
    }
    /* terminate the ODM */
    odm_free_list(cudv_save, &cudv_info);
    odm_terminate();

pop_n_leave:
    TS_POP_CLNUP(0);
    TS_UNLOCK(&_odm_rmutex);
    if (leave)
	return(TS_FAILURE);		/* if leaving, then we failed */

    /* get ty_gtty from inittab */
    if ((fpinit = fopen(itfn, "r")) == NULL)  { /* open inittab */
	    ENDTTYENT();
	    return(TS_FAILURE);
    }
#ifdef _THREAD_SAFE
    while (getget_r(linebuf,fpinit) == TRUE)
#else
    while (getget() == TRUE)
#endif
    {
	(void)stanname(linebuf, devname);
	for(i = 0; i < cudv_info.num; i++)
	    if(strcmp(devname,TTYINFO[i].ty_name) == 0)
	    {
		char *cmd = malloc(1024 * sizeof(char));

		if (cmd == (char *)NULL) {
			errno = ENOMEM;
			return(TS_FAILURE);
		}

		TTYINFO[i].ty_status = getpstat(linebuf);
		if (getcmd(linebuf, cmd))
		    TTYINFO[i].ty_getty = cmd;
		break;
	    }
    }
    fclose(fpinit);
    return(TS_FOUND(ttyinfo));
}

/* the remaining code was borrowed from penable */

#ifdef _THREAD_SAFE
int getget_r(char *linebuf, FILE * fpinit)
#else
static
getget() /* routine to get a line with "*getty *".  It is assumed to be a port */
#endif
{
	char	*s;

	while (fgets(linebuf, MAXLLEN, fpinit)) {
		/* skip leading white space */
		for (s = linebuf; *s == ' ' || *s == '\t'; s++);
		if(isgetty(s) != BAD)
			return(TRUE); /* getty command line found */
	}
	return(BAD); /* EOF and no = "*getty*" */
}

static
isgetty(s) /* routine is see if the word getty is in this line */
char *s;
{
	while(1) {
		while(*s != 'g') { /* find the g in getty */
			if(*s == '\n')return(BAD); /* no getty here */
			if(*s == '\0')return(BAD); /* no getty here */
			s++;
		}
		if(strncmp("getty ", s, (size_t)6) == 0)return(TRUE); /* we have getty */
		s++; /* skip this g */
	}
}

static
getpstat(char *linebuf) /* routine to get port status of current line */
{
	char devname[MAXNAMLEN]; /* hold the device name */
	char act[MAXACT];
	char cmd[MAXCMDL];
	char whatflg();

	(void)stanname(linebuf, devname); /* store the device name */
	if (!getact(linebuf, act))
		return(-2); /* can't know status without action */
	if(strncmp("off", act, (size_t)3) == 0) {
		return(FALSE);  /* no process then must be disabled */
	}
	/* only respawn or ondemand means it is enabled */
	if ((strncmp("respawn", act, (size_t)7) == 0) || 
			(strncmp("ondemand", act, (size_t)8) == 0)) {
		if (!getcmd(linebuf, cmd))  /* need command line to know if */
			return(-2);         /* port shared, delayed, enabled */

		/* if -u is used then it is shared */
		/* if -r is used then it is delayed */
		/* else enabled */
		return((int)whatflg(cmd));
	}
	else /* anything else is not correct */
		return(-2);
}

/* Get command specified in from */
static
getcmd(from, to)
char	*from;
char	*to;
{
	char	*s = from;
	int	i;
	
	/* skip over name */
	for (; *s != ':' && *s != '\n' && *s != '\0'; s++);
	if (*s == '\n' || *s == '\0')
		return(0);

	/* skip over runlevel */
	for (s++; *s != ':' && *s != '\n' && *s != '\0'; s++);
	if (*s == '\n' || *s == '\0')
		return(0);

	/* skip over action */
	for (s++; *s != ':' && *s != '\n' && *s != '\0'; s++);
	if (*s == '\n' || *s == '\0')
		return(0);

	/* skip leading white space */
	for (s++; *s == ' ' || *s == '\t'; s++);
	if (*s == '\n' || *s == '\0')
		return(0);

	/* copy command to "to" */
	for (i = 0; *s != '\n' && *s != '\0'; s++, i++)
		to[i] = *s;
	to[i] = '\0';
	return(i);
}

/* Get action (respawn, off, etc.) specified in from */
static
getact(from, to)
char	*from;
char	*to;
{
	char	*s = from;
	int	i;
	
	/* skip over name */
	for (; *s != ':' && *s != '\n' && *s != '\0'; s++);
	if (*s == '\n' || *s == '\0')
		return(0);

	/* skip over runlevel */
	for (s++; *s != ':' && *s != '\n' && *s != '\0'; s++);
	if (*s == '\n' || *s == '\0')
		return(0);

	/* skip leading white space */
	for (s++; *s == ' ' || *s == '\t'; s++);
	if (*s == '\n' || *s == '\0')
		return(0);

	/* copy action to "to" */
	for (i = 0; *s != ':' && *s != ' ' && *s != '\t' && *s != '\n' && 
			*s != '\0'; s++, i++)
		to[i] = *s;
	to[i] = '\0';
	return(i);
}

static
stanname(sb,s) /* routine to get device name and store */
char *sb; /* pointer to the buffer */
char *s;  /* destination */
{
	int 	i = 0;
	char	*s2;

	while(sb[i] != ':') {
		if(sb[i] == '\n')return(BAD); /* we have problems */
		if(sb[i] == '\0')return(BAD); /* we have problems */
		i++;
	}
	while(--i >= 0) { /* only the name if possible */
		if(sb[i] == '/')break; /* last name if path is all we need */
		if(sb[i] == ' ')break; /* don't want white space */
		if(sb[i] == '\t')break; /* don't want white space */
	}
	i++; /* start of string */
	s2 = s;
	while(( *s++ = sb[i++]) != ':'); /* move in the name */
	*--s = '\0'; /* null terminate */
	if (!strncmp("cons", s2, (size_t)4))
		strcpy(s2, "console");
	return(TRUE);
}

static char 
whatflg(s) /* get the current flag in the getty command */
char *s;
{
	while(*s != '\n' && *s != '\0') { /* go to the end of the line */
		if(*s++ == '-') { /* we have a flag */
			if (*s == 'u')
				return(ENABLE_SHARE); /* found a shared flag */
			if (*s == 'r')
				return(ENABLE_DELAY); /* found a delay flag */
		}
	}
	return(ENABLE_TRUE); /* no shared or delay so must be enable */
}
