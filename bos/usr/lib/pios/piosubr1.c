static char sccsid[] = "@(#)43  1.18  src/bos/usr/lib/pios/piosubr1.c, cmdpios, bos411, 9428A410j 12/8/93 18:32:49";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  get_odmtab_ent, piogenmsg, piomsgout, pioexit
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <IN/backend.h>
#include <time.h>
#include "pioformat.h"

/* include for getmsg routine */
#include <nl_types.h>

extern int	lockf (int fd, int request, off_t size);
						/* should be in sys/lockf.h */
extern int	get_job_number (void);		/* available in libqb.a */
extern int	sysnot (char *user, char *host, char *msg, unsigned int pref);
						/* available in libqb.a */

/* Declarations for message ipc (for Palladium) */
extern uchar_t		piomsgipc;	/* flag to indicate if msgs need be
					   sent via ipc */
extern int	pmi_initmsgipc(void);
extern int	pmi_bldhdrfrm(uchar_t,ushort_t,ushort_t,char const *);
extern int	pmi_bldintfrm(int);
extern int	pmi_bldstrfrm(const char *);
extern int	pmi_dsptchmsg(const char *);

void 		piomsgout (char *msgstr);
char 		*getmsg (char *CatName, int set, int num);
int 		pioexit (int exitcode);       /* exit code (good or bad) */

char defbasedir[] = DEFVARDIR;

/* external variables used by various functions (externs in pioformat.h) */
char msgbuf[MSGBUFSIZE];  /* buffer in which to build message text */

struct err_info errinfo = {0,"_______________","________","_________",
"______","______","______________","______________",0};


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           get_odmtab_ent                                               *
*                                                                              *
* DESCRIPTION:    Determine the offset (in the odm table) of the attribute     *
*                 associated with the attribute name (passed as a parameter).  *
*                 The name is really a key to a pseudo hash function.  The     *
*                 first character is an index into the primary hash table and  *
*                 the second character is an index into a secondary hash       *
*                 table based on the first character.                          *
*                                                                              *
* PARAMETERS:     name - name of attribute in odm table                        *
*                                                                              *
* RETURN VALUES:  pointer to an odm table entry.                               *
*                 NULL    - no odm attribute asociated with name argument.     *
*                                                                              *
*******************************************************************************/

struct odm *
get_odmtab_ent (char *name)                 /* name of odm attribute */
{
    extern char *hash_table;    /* Ptr to start of primary hash table */
    extern int *mem_start;      /* Ptr to start of malloced memory */
    extern int  objtot;         /* number of object in odm table */

    register char *cp = name;   /* character pointer */
    register int index_ht;      /* index into hash table */
    mem_shval_t *hash_tab_p;    /* pointer to hash tab entry */
    struct odm *odmptr;         /* pointer to odm table entry */
    void *tmp_vp = NULL;        /* temp. generic pointer */

    if (*cp < '0' || *cp > 'z')
	return((struct odm *) 0);

    index_ht = GET_INDEX(*cp++);                /* calc. primary hash index */
    tmp_vp = hash_table + index_ht;
    hash_tab_p = *(mem_shval_t **) &tmp_vp;
						/* position hash tab pointer */

    /* odm table offset is in primary hash table */
    if (*hash_tab_p < 0)
    {
	tmp_vp = (char *) mem_start - *hash_tab_p;
	odmptr = *(struct odm **) &tmp_vp;
	if (odmptr->name[1] == *cp)
	    return(odmptr);                /* attr name is valid */
	else
	    return((struct odm *) 0);   /* attr name is not valid */
    }

    /* odm table offset might be in sec. hash table */
    else if (*hash_tab_p > 0)
    {
	if (*cp < '0' || *cp > 'z')
	    return((struct odm *) 0);

	index_ht = GET_INDEX(*cp);             /* calc. secondary hash index */
	hash_tab_p = (mem_shval_t *) GET_OFFSET(*hash_tab_p + index_ht);

	/* uh oh, bad name, go to bed without your supper */
	if (*hash_tab_p == 0)
	    return((struct odm *) 0);

	/* Good name !! return the adjusted offset */
	return((struct odm *) GET_OFFSET(*hash_tab_p));
    }
    /* bad name, boo hoo */
    else
	return((struct odm *) 0);
}


/*******************************************************************************
*                                                                              *
* NAME:           piogenmsg                                                    *
*                                                                              *
* DESCRIPTION:    Generate message text for the specified message.             *
*                                                                              *
* PARAMETERS:     msgnum - message number in message catalog                   *
*		  sendmsg_flag - send message text to the user - TRUE/FALSE   *
*                                                                              *
* GLOBALS:                                                                     *
*     MODIFIED:   msgbbuf     - message buffer                                 *
*                                                                              *
* RETURN VALUES:   0      all returns                                          *
*                                                                              *
*******************************************************************************/

#define NUMARGS  8              /* number of args to sprintf */

int
piogenmsg(int msgnum, int sendmsg_flag)
{
    int value;
    char wkbuf[MSGBUFSIZE];
    char *cp, *msgp, *wp;
    char *pp[20];
    int parmnum = 0;
    char intstring[20];

    /* get the message text from the message catalog */
    msgp = getmsg(MF_PIOBE, 1, msgnum);

    /* copy message text to work buffer while checking for %x$ (x = 1,...) */
    for (wp = wkbuf; *msgp != '\0'; msgp++) {
	*wp++ = *msgp;
	if (*msgp == '%' && *(msgp + 1) != '\0' && *(msgp + 2) == '$') {
	    value = *(msgp + 1) - '0';
	    if (value < 1 || value > NUMARGS)
		continue;
	    if (*(msgp+3) == 'd') {
		sprintf(intstring, "%d", errinfo.integer);
		cp = intstring;
		*(msgp+3) = 's';
	    } else {
		switch (value)
		{
		case 1: cp = errinfo.title;    break;
		case 2: cp = errinfo.progname; break;
		case 3: cp = errinfo.subrname; break;
		case 4: cp = errinfo.qname;    break;
		case 5: cp = errinfo.qdname;   break;
		case 6: cp = errinfo.string1;  break;
		case 7: cp = errinfo.string2;  break;
		case 8: cp = intstring;        break;
		}
	    }
	    pp[parmnum++] = cp;
	    msgp += 2;  /* skip over index and '$' */
	}
    }

    *wp = '\0';
    sprintf(msgbuf,wkbuf,pp[0],pp[1],pp[2],pp[3],pp[4],pp[5],pp[6],pp[7]);

    /* send the message, only if told to */
    if (sendmsg_flag) {
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		msgnum,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(errinfo.title);
	    (void)pmi_bldstrfrm(errinfo.progname);
	    (void)pmi_bldstrfrm(errinfo.subrname);
	    (void)pmi_bldstrfrm(errinfo.qname);
	    (void)pmi_bldstrfrm(errinfo.qdname);
	    (void)pmi_bldstrfrm(errinfo.string1);
	    (void)pmi_bldstrfrm(errinfo.string2);
	    (void)pmi_bldintfrm(errinfo.integer);
	}
        piomsgout(msgbuf);
    }

    return(0);
}



/*******************************************************************************
*                                                                              *
* NAME:           piomsgout                                                    *
*                                                                              *
* DESCRIPTION:    If called by Qdaemon piomsgout locks the statusfile and      *
*                 calls sysnot() and then releases the lock.  Otherwise        *
*                 the message goes to standard error.                          *
*                                                                              *
* PARAMETERS:     msgstr - message text                                        *
*                                                                              *
* GLOBALS:                                                                     *
*     REFERENCED: statusfile - indicates whether a statusfile exists.          *
*                                                                              *
* RETURN VALUES:  void                                                         *
*                                                                              *
*******************************************************************************/

void
piomsgout (char *msgstr)           /* error message text */
{
    int statfd = 3;     /* file descriptor of statusfile */
    char *msgptr, *catmsg, *sub_node, *sub_user, *cp1;
    size_t msglen, hdrlen;
    FILE *fd;
    char wkbuf[1000];
    char titlebuf[1000];

    char *basedir;
    char *title;
    char *qname;
    char *qdname;
    char *from;
    int   mailonly;
    int   cnt;
    char  fname[200];
    struct flock lockdat; /* lockdata for fcntl() D61224 */

    if (statusfile)
    {
	freopen("/dev/null", "a", stderr);
	title    = getenv("PIOTITLE");
	qname    = getenv("PIOQNAME");
	qdname   = getenv("PIOQDNAME");
	from     = getenv("PIOFROM");
	mailonly = atoi(getenv("PIOMAILONLY"));
	msglen = strlen(msgstr);
	catmsg = getmsg(MF_PIOBE, 1, MSG_SPOOLHDR);
	(void) sprintf(titlebuf, "%d (%s)", get_job_number(), title);
	sprintf(wkbuf, catmsg, titlebuf);
	hdrlen = strlen(wkbuf);
	if ((msgptr = malloc(msglen + hdrlen + 1)) == NULL) {
	    (void) fprintf(stderr, (getmsg(MF_PIOBE,1,MSG_MALLOC)));
	    pioexit(PIOEXITBAD);
	}
	(void) memcpy(msgptr, wkbuf, hdrlen);
	(void) memcpy(msgptr + hdrlen, msgstr, msglen);
	*(msgptr + hdrlen + msglen) = '\0';

	/* Get User Name & Node Name of Print Job Submitter */
	sub_node = NULL;
	for (sub_user = cp1 = from; *cp1 != '\0'; cp1++)
	    if (*cp1 == '@') {
		*cp1 = '\0';
		sub_node = ++cp1;
		break;
	    }

        /* Set up the lockdat for fcntl() replaces lockf() D61224 */
        lockdat.l_whence = 0;  /* measure from the beginning of file */
        lockdat.l_start = 0;   /* start lock from the beginning */
        lockdat.l_len  = 0;    /* Go till end of file */
        lockdat.l_type = F_WRLCK;
        (void) fcntl(3, F_SETLKW, &lockdat);
	if (piomsgipc) {		/* if message is to be sent via ipc */
	   (void)pmi_dsptchmsg(msgptr);
	}
	else
	   (void)sysnot(sub_user, sub_node, msgptr, mailonly ? DOMAIL:DOWRITE);
	{ /* begin block */
	long clocktm;
	struct tm *tdate;
	char pdate[50];
	clocktm = time ((long *) 0);  /* get the time */
	tdate = localtime(&clocktm);
	(void) strftime(pdate,350,"%a %h %d %T %Y",tdate);

        if ((basedir = getenv("PIOVARDIR")) == NULL)
            basedir = defbasedir;                 /* default base directory */
        (void) sprintf(fname, "%s/msg1.%s:%s", basedir, qname, qdname);

	(void) unlink(fname);
	(void) umask (0777 ^ (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
	cnt = open(fname, O_CREAT + O_WRONLY + O_TRUNC, 0664);
	if (cnt > 0) {
	    fd = fdopen(cnt, "w");
	    fprintf(fd,
	    "-----------------------------------------------------------\n");
	    fprintf(fd,"%s", pdate);
	    fprintf(fd, " (%s", sub_user);
	    if (*sub_node)
		fprintf(fd, " @ %s", sub_node);
	    fprintf(fd, ")");
	    fprintf(fd, "\n");
	    fprintf(fd,
	    "-----------------------------------------------------------\n\n");
	    cnt=fprintf(fd,"%s",msgptr);
	    fflush(fd);
	}
	} /* end block */
        lockdat.l_type = F_UNLCK;
        (void) fcntl(3, F_SETLKW, &lockdat);
    }
    else
    {
	if (piomsgipc) {		/* if message is to be sent via ipc */
	   (void)pmi_dsptchmsg(msgstr);
	}
	else {
	   (void) fprintf(stderr, "%s", msgstr);
	   (void) fflush(stderr);
	}
    }
}


/*******************************************************************************
*                                                                              *
* NAME:           pioexit                                                      *
*                                                                              *
* DESCRIPTION:    exit routine, cleans up things and then exits.               *
*                                                                              *
* PARAMETERS:     exitcode - indicates normal or abnormal exit.                *
*                                                                              *
* GLOBALS:                                                                     *
*     MODIFIED:                                                                *
*                                                                              *
*     REFERENCED:                                                              *
*                                                                              *
* EXIT VALUES:    EXITGOOD  everything worked!                                 *
*                 EXITBAD   uh oh, something has messed up                     *
*                                                                              *
*******************************************************************************/

int
pioexit (int exitcode)       /* exit code (good or bad) */
{
	fflush(stdout);     /* just to make sure */
	fflush(stderr);     /* just to make sure */

	if (statusfile)     /* if running under spooler */
	    exit(0);            /* don't upset the qdaemon */
	else                /* otherwise, running standalone, so */
	    exit(exitcode);     /* tell it like it is */
}

/*
*******************************************************************************
*******************************************************************************
** NAME:        getmsg()
**
** DESCRIPTION: Replaces the NLgetamsg routine used in 3.1 code  If the catalog
**              is not found in the NLSPATH, it will look for a default in
**              /usr/lib/lpd/pio/etc.
**
** ROUTINES
**   CALLED:    catopen() - gets catalog descriptor
**
**              catgets() - gets message
**              catclose  - closes catalog
**
** PARAMETERS:  catalog name, set number, message number
**
**
*******************************************************************************
*******************************************************************************
*/
static char *msgbuffer = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

char *
getmsg(char *CatName, int set, int num)
{
	char *ptr;
	char *defpath = malloc(200);
	char default_msg[100];

	if (strcmp(CatName,save_name) != 0)  /* is it a different catalog */
	{
	catclose(catd);  /* close /usr/lpp message catalog */
	catclose(def_catd);  /* close default message catalog */
	catd = def_catd = NULL;  /* set catalog descriptors to NULL */
	strcpy(save_name,CatName);
	}

	if (catd != (nl_catd)-1)  
		if (catd == 0) /* if it hasn't been open before */
			catd = catopen(CatName,NL_CAT_LOCALE);
	
	if (catd != (nl_catd)-1)
		{
		ptr = catgets(catd,set,num,"dummy");
		if (!msgbuffer)
			msgbuffer = malloc(4001);
		if (msgbuffer)
			strncpy(msgbuffer, ptr, 4000);
		if (strcmp(ptr,"dummy") != 0)  /* did catgets fail? */
			return(msgbuffer);
		}
	
	if (def_catd == 0)
		{
		sprintf(defpath,"/usr/lib/lpd/pio/etc/%s",CatName);
		def_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	sprintf(default_msg,"Cannot access message catalog %s.\n",CatName);
	ptr = catgets(def_catd,set,num, default_msg);
	if (!msgbuffer)
		msgbuffer = malloc(4001);
	if (msgbuffer)
		strncpy(msgbuffer, ptr, 4000);

	free(defpath);
	return(msgbuffer);
}
