static char sccsid[] = "@(#)23	1.35.1.6  src/bos/usr/bin/que/digest.c, cmdque, bos411, 9428A410j 6/15/94 13:44:18";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: digest
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#define _ILS_MACROS
#include <stdio.h>
#include <IN/standard.h>
#include <sys/fullstat.h>
#include <errno.h>
#include <sys/types.h>
#include <locale.h>
#include <time.h>
#include <fcntl.h>
#include <IN/backend.h>
#include "common.h"
/* AIX security enhancement	*/
#include <security.h>
#include <sys/priv.h>
#include <sys/access.h>
#include <sys/id.h>
#include <ctype.h>
#include "digest.h"

#include "digest_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_DIGEST,num,str)


/*
 * qconfig digester -- reads config file, builds structures,
 * then writes them out to a binary file.  run invisibly
 * by print and qdaemon when they start up, if current config
 * not already digested.
 */

int lineno, linesv;             /* line number in config file */
time_t qmdate;			/* modification date of qconfig file */
struct q *getq(), *getrq();
struct d *getd();
char	*strchr();
char sbuf[MAXLINE];	/* Holding place for sprinf() error messages */
char *config_name;
char *progname = "digest";

/* AIX security enhancement	*/
void	privilege();

error(args)
char *args;
{
	systell(MSGSTR(MSGCNFG,"error in config file %s,line %d."),config_name,lineno);
	syserr((int)EXITFILE,args);
}

/*
 * read a line (or portion) from the config file.  return an
 * av struct describing that line.
 */
struct av *lread(fname)
char *fname;
{       static struct av av;    /* we return this each time */
	static char fileopen = FALSE;
	static FILE *confile;
	static char confline[CONFLINE];
	static char inadev = FALSE;
	static char inaque = FALSE;
	static char devsexpected = 0;
	static char *devnames[256];
	static char *devbuffer = NULL;
	static char dev_names_saved = 0;
	static char dev_list_count = 0;
	static boolean hostlinefound = FALSE;
	static boolean rqlinefound = FALSE;
	static boolean firststanza = TRUE;
	static boolean devlinefound = FALSE;
	static char tsaved;
	char stanzabegin;
	register char *begin, *end;
	register struct legalvals *lvp;
	register int offset, *vp;
	struct stat statbuf;
	char c;

	/*----Obtain date of file and get status (last mod date) if not already done */
	if (fileopen == FALSE) {
		if (stat(fname,&statbuf) == -1) {
			sprintf(sbuf,MSGSTR(MSGSTAT,"cannot obtain file status for %s."),fname);
			error(sbuf);
		}
		qmdate = statbuf.st_mtime;
		if ((confile = fopen(fname,"r")) == NULL) {
			sprintf(sbuf,MSGSTR(MSGRD,"cannot open %s for reading."), fname);
			error(sbuf);
		}
		fileopen = TRUE;
	}


	/* ignore comments and blank lines */
nextln: do          /* return NULL on end of file */
	{   if (!getqline(confline,CONFLINE,confile)) {
		if (devsexpected) {
			sprintf(sbuf,MSGSTR(MSGMST,"missing device stanza."));
			error(sbuf);
		}
		if (!inadev && !devlinefound && inaque)
		{
			sprintf(sbuf,MSGSTR(MSGNODEV,"No device line in queue stanza."));
			error(sbuf);
		}
	
		fclose(confile);
		return(NULL);           /* all done */
	    }
	    lineno++;
	    c = confline[0];
	} while (c == '*' || c == '\0');

	/* if we see a left-justified line, we */
	/* know that this begins a new stanza. */
	if (c != ' ' && c != '\t') {
	   stanzabegin = TRUE;
	/* if the previous stanza had no device = 'xxx' line, then it's wrong
	 unless its a device stanza */
	    if (!firststanza)		/* 1st stanza has no previous stanza */
		if (!devlinefound && !inadev) {
			sprintf(sbuf,MSGSTR(MSGNODEV,"No device line in queue stanza."));
			error(sbuf);
		}
	    if (rqlinefound)
		if (!hostlinefound) {
			sprintf(sbuf,MSGSTR(MSGRQ,"Cannot have rq line without hostid line."));
			error(sbuf);
		}
	    if (hostlinefound)
		    if (!rqlinefound) {
			sprintf(sbuf,MSGSTR(MSGHOST,"Cannot have hostid line without rq line."));
			error(sbuf);
		    }
	    inaque = TRUE;
	    inadev = FALSE;
	    hostlinefound = FALSE;
	    firststanza = FALSE;
	    devlinefound = FALSE;
	    rqlinefound = FALSE;
	}
	else
	    stanzabegin = FALSE;

	/* set av.atype to the type of line, and begin to */
	/* the beginning of the value. */
	if (stanzabegin)        /* stanza header line */
	{   begin = confline;
	    end = strchr(begin,':');
	    if (NULL == end) {
		sprintf(sbuf,MSGSTR(MSGCOLON,"missing `:' after stanza name."));
		error(sbuf);
	    }
	    if (devsexpected)
	    {   /* now processing a device stanza */
		int i;
		av.atype = FDNAME;
		--devsexpected;
		inadev = TRUE;
		inaque = FALSE;
		*end = '\0';
		/* now confirm the name of this stanza matches a name
		   on the devices line for the queue */
		for( i = 0;  i < dev_list_count;  i++ )
		    if (!strcmp(begin, devnames[i]))
			break;
		if( i == dev_list_count )
		{
		    sprintf( sbuf, 
	MSGSTR(MSGBADDEV, "device `%s' wasn't specified on devices line for queue."), begin );
		    error(sbuf);
		}
	    }
	    else
	    {
		/* now processing a queue stanza */
		av.atype = FQNAME;     /* its a real queue stanza   */
		inaque = TRUE;
		inadev = FALSE;
		dev_list_count = 0;
		/* this ftell()/fseek() *SHOULD* be a no-op, but
		   removing them causes lread() to always return
		   a bad value.  why?  */
		offset =  ftell(confile);  /* save position */
		linesv = lineno;           /* save line number */
		fseek(confile,(long)offset,0);  /* reset file pointer */
		lineno = linesv;          /* reset line number  */
		if( devbuffer != NULL )
		{
		    free( devbuffer );
		    devbuffer = NULL;
		}
	    }
	    *end = '\0';
	}
	else            /* normal stanza line */
	{   for (begin = confline; (*begin == '\t') || (*begin == ' '); begin++);
	    end = strchr(begin,' ');
	    if (end == NULL) {
		sprintf(sbuf,MSGSTR(MSGBLANK,"missing blank after field name."));
		error(sbuf);
	    }
	    *end = '\0';  /* begin now points to null terminated field name */
	    if ((av.atype = lookup(begin,fnames)) == -1)
	    {
		sprintf(sbuf,MSGSTR(MSGBADNAME,"unrecognized field name %s."), begin );
		error(sbuf);
		goto nextln;    /* ignore unrecognized field name */
	    }
	    begin = strchr(++end,'=');
	    if (begin == NULL) {
		sprintf(sbuf,MSGSTR(MSGMEQ, "missing `='."));
		error(sbuf);
	    }
	    for (++begin; (*begin == '\t') || (*begin == ' '); begin++);
	}

	if (av.atype == FHOSTID )
		hostlinefound = TRUE;

	if (av.atype == FRQ )
		rqlinefound = TRUE;

	/* if this is a device line, we may have multiple values */
	if (av.atype == FDEVICE ) {
	    char *t;
	    /* my goals here are two-fold: to count the number of
	       devices named on the devices line, so that we know
	       how many devices stanzas follow; and to collect the
	       names of those devices so that can do error checking */
	    devlinefound = TRUE;
	    devsexpected=1;
	    /* the buffer containing the device names is going to get
	       clobbered on the next read, so we need to save its contents */
	    t = devbuffer = Qalloc( (size_t)strlen(begin) + 1 );
	    strcpy( devbuffer, begin );
	    devnames[dev_list_count++] = t;
	    for (; *t; t++) {
		if (*t == ',') {
		    ++devsexpected;
		    *t = '\0';
		    for (++t; (*t == '\t') || (*t == ' '); t++);
		    if (!*t) {
			sprintf(sbuf,MSGSTR(MSGTC, 
				"devices line contains trailing comma."));
			error(sbuf);
		    }
		    devnames[dev_list_count++] = t;
		    tsaved = av.atype;
		}
	    }
	    end = t;
	    dev_names_saved = dev_list_count;
	}

	/* if this is a backend line, throw away quotes */
	if (av.atype == FBACKEND)
	{   end = strchr(begin,'\0') - 1;
	    if (*begin == '"' && *end == '"')
	    {   *end = '\0';
		begin++;
	    }
	}

	/* atype (attribute type) is correct. */
	/* look at vtype (value type) now. */
	if ((av.vtype = lookup(begin,vnames)) == -1)    /* not a literal */
	{
	switch(av.atype)
		{
		case FFEED:
		     if (isnumeric(begin))
		     {   av.vtype = VNUMBER;
			 av.vval.vnumber = atoi(begin);
		     }
		     else
		     {   av.vtype = VTEXT;
			 av.vval.vtext = begin;
		     }
		     break; 
		default:
		     av.vtype = VTEXT;
		     av.vval.vtext = begin;	
		     break;
		}
	}
			 	
	/* vtype is correct, but may not be legal */
	/* find its entry in the legalval table */
	for (lvp = lv; lvp->field; lvp++)
	{   if (lvp->field == av.atype)
		break;
	}

	/* is the field in the right kind of stanza? */
	if (inadev)   /* if its a device stanza */
	{
		if (!lvp->devstanza)  /* if its not a legal field */
		{
		   sprintf(sbuf,MSGSTR(MSGDFLD,"illegal field name for device stanza."));
		   error(sbuf);
		}
	}
     /* if its a real queue, then it can have any field that device can't  */
	else
	{
		if (lvp->devstanza)  /* if its not a legal field */
		{
		   sprintf(sbuf,MSGSTR(MSGQFLD,"illegal field name for queue stanza."));
		   error(sbuf);
		}
	}



	/* is the value legal for this field name? */
	for (vp = lvp->value; *vp != END; vp++)
	{   if (*vp == av.vtype)
	    {   switch(*vp)
		{   case VTEXT:
			++vp;
			if (*vp && strlen(av.vval.vtext) >= *vp) {
				sprintf(sbuf,MSGSTR(MSGFLNG,
					"field value too long; limit is %d chars."),(*vp -1) );
				error(sbuf);
			    }
			break;
		    case VNUMBER:
			++vp;
			if (*vp && av.vval.vnumber > *vp)
			    {
				sprintf(sbuf,MSGSTR(MSGFLRG,
					"field value too large; maximum is %d."),*vp);
				error(sbuf);
			    }
			    break;
		    default:
			break;
		}
		/* yes, everything is correct */
		return(&av);
	    }
	}
	/* no, the value was not in the table */
	{ 
	    sprintf(sbuf,MSGSTR(MSGIVAL,"illegal value `%s'."),begin);
	    error(sbuf);
	}
}

/* is str all-numeric? */
isnumeric(str)
register char *str;
{
	while (isdigit((int)*str))
		str++;
	return(*str == '\0');
}

/*
 * lookup string in a namtab.
 * return type, or -1 if wasn't there.
 */
lookup(str,ntp)
register char *str;
register struct namtab *ntp;
{
	while (ntp->name)
	{   if (!strcmp(str,ntp->name))
		return(ntp->val);
	    ntp++;
	}
	return(-1);
}


/*
 * turn the config file into a list of queues and devices.  repeatedly
 * calls lread, building the proper structures with the info returned.
 */
struct q *digest(fname)
char *fname;
{       register struct av *avp;
	register struct q *lastq = NULL;
	register struct d *lastd = NULL;
	register struct q *thisq;
	register struct d *thisd;
	register struct q *qlist = NULL;       /* list of all queues */

	while (1)
	{   avp = lread(fname);
	    if (avp == NULL)        /* end of file */
	    {   if (lastd != NULL)
			checkd(lastd);
		return(qlist);
	    }
	    switch(avp->atype)
	    {
		case FQNAME:
		    /* be sure the last dev stanza was ok */
		    if (lastd != NULL)
			checkd(lastd);
		    lastd = NULL;
		    /* insert into list */
		    thisq = getq(avp->vval.vtext);
		    if (lastq == NULL)
			qlist = lastq = thisq;
		    else
		    {   
			checkq(qlist,thisq->q_name);		
			lastq->q_next = thisq;
			lastq = lastq->q_next;
		    }
		    break;
		case FDNAME:
		    /* check last item we built */
		    if (lastq->q_devcount != 0)
			checkd(lastd);
		    /* and insert into list */
		    thisd = getd(avp->vval.vtext);
		    lastq->q_devcount++;
		    if (lastd == NULL)
			lastq->q_devlist = lastd = thisd;
		    else
		    {   lastd->d_next = thisd;
			lastd = lastd->d_next;
		    }
		    lastd->d_q = lastq;
		    break;
		case FDEVICE:
		    /* ignore these; lread ensures correct order */
		    break;
		case FHOSTID:
		    strcpy(lastq->q_hostname,avp->vval.vtext);
		    break;
		case FRQ:
		    TRUNC_NAME( avp->vval.vtext, QNAME ) ;
		    strcpy(lastq->q_rname,avp->vval.vtext);
		    break;
		case FSROS:
		    if (strcmp(avp->vval.vtext,VNONE)==0)
			    lastq->q_sros[0] = '\0';
	    	    else
			    strcpy(lastq->q_sros,avp->vval.vtext);
		    break;
		case FLROS:
		    if (strcmp(avp->vval.vtext,VNONE)==0)
			    lastq->q_lros[0] = '\0';
	    	    else
			    strcpy(lastq->q_lros,avp->vval.vtext);
		    break;
		case FALIGN:
		    if (avp->vtype == VTRUE)
			lastd->d_align = TRUE;
		    else
			lastd->d_align = FALSE;
		    break;
		case FDISC:
		    if (avp->vtype == VFCFS)
			lastq->q_disc = FCFS;
		    else
			lastq->q_disc = SJN;
		    break;
		case FUP:
		    if (avp->vtype == VTRUE)
			lastq->q_up = TRUE;
		    else
			lastq->q_up = FALSE;
		    break;
		case FACCTF:
		    if (avp->vtype == VFALSE)
			lastq->q_acctf[0] = '\0';
		    else
			strcpy(lastq->q_acctf,avp->vval.vtext);
		    break;
		case FFILE:
		    if (avp->vtype == VFALSE)
			lastd->d_file[0] = '\0';
		    else
			strcpy(lastd->d_file,avp->vval.vtext);
		    break;
		case FACCESS:
		    if (avp->vtype == VWRITE)
			lastd->d_access = WRITE;
		    else
			lastd->d_access = BOTH;
		    break;
		case FFEED:
		    if (avp->vtype == VNEVER)
			lastd->d_feed = NOFEED;
		    else
			lastd->d_feed = avp->vval.vnumber;
		    break;

		case FHEAD:
		    if (avp->vtype == VNEVER)
			lastd->d_head = NEVER;
		    else
		    {   if (avp->vtype == VGROUP)
			    lastd->d_head = GROUP;
			else
			    lastd->d_head = ALWAYS;
		    }
		    break;
		case FTRAIL:
		    if (avp->vtype == VNEVER)
			lastd->d_trail = NEVER;
		    else
		    {   if (avp->vtype == VGROUP)
			    lastd->d_trail = GROUP;
			else
			    lastd->d_trail = ALWAYS;
		    }
		    break;
		case FBACKEND:
		    lastd->d_backend = (char *)Qalloc((size_t)BENAME);
		    strcpy(lastd->d_backend,avp->vval.vtext);
		    break;
		default:
		    sprintf(sbuf,MSGSTR(MSGLREAD,"bug in digest, lread returned trash."));
		    error(sbuf);
		    break;
	    }
	}
}




/*
 * check for a legal device stanza.  backend must not be omitted.
 */
checkd(d)
register struct d *d;
{
	if (!d->d_backend)
	{
		sprintf(sbuf,MSGSTR(MSGMISS,"queue %s, device %s: missing backend field."),
		   d->d_q->q_name, d->d_name);
		error(sbuf);
	}
}


/*
 * get a new queue structure, filling in defaults (except argname).
 * the arg is the queue name.
 */
struct q *getq(name)
char *name;
{       register struct q *q;

	q = (struct q *)Qalloc((size_t)sizeof(struct q));
	q->q_next = NULL;
	q->q_entlist = NULL;
	q->q_devlist = NULL;
	TRUNC_NAME( name, QNAME ) ;
	strcpy(q->q_name,name);
	q->q_acctf[0] =  '\0';
	q->q_rname[0] =  '\0';
	strcpy(q->q_sros,S_DEFAULT_STATFILTER );
	strcpy(q->q_lros,L_DEFAULT_STATFILTER );
	q->q_hostname[0] = '\0';
	q->q_disc = FCFS;
	q->q_up = TRUE;
	q->q_devcount = 0;
	return(q);
}



/*
 * get a new d structure; the arg is the device name.
 */
struct d *getd(name)
{       register struct d *d;

	d = (struct d *)Qalloc((size_t)sizeof(struct d));
	d->d_backend = NULL;
	d->d_q = NULL;
	d->d_e = NULL;
	d->d_next = NULL;
	d->d_pid = 0;
	d->d_file[0] = d->d_user[0] = '\0';
	d->d_access = WRITE;
	d->d_up = TRUE;
	d->d_head = d->d_trail = NEVER;
	d->d_feed = NOFEED;
	TRUNC_NAME( name, DNAME ) ;
	strcpy(d->d_name,name);
	d->d_align = FALSE;
	return(d);
}


/*
 * digest the file, then write out the binary version.
 */
main(argc,argv)
int  argc;
char **argv;
{       struct q *qlist;

	/* AIX security enhancement	*/
	privilege(PRIV_LAPSE); 	/* temporarily drop any privileges we have,	*/
				/* we'll get them only when we need them.	*/

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
        catd = catopen(MF_DIGEST, NL_CAT_LOCALE);

#ifdef DEBUG
	if(getenv("DIGEST"))
		sysraw(stdout,"DIGEST arg1 %s arg2 %s\n",argv[1],argv[2]);
#endif

	if (argc < 3) {
	    sprintf(sbuf,MSGSTR(MSGNARGS, "Not enough args to digest."));
	    error(sbuf);
	}
	config_name = argv[1];
	qlist = digest(argv[1]);
	dump(qlist,argv[2]);
	qexit((int)EXITOK);           /* show caller we succeeded */
}


/*
 * given a qlist, dump the image to the file named fname.
 * the image is the queue structures, each of which is followed by
 * its device structures.  each device structure is followed by an
 * int giving the length of the backend field, then the backend field
 * itself.
 */
dump(ql,fname)
register struct q *ql;
register char *fname;
{       register struct d *dl;
	register int fd;
	int belength;
	/* AIX security enhancement	*/
	gid_t	effgid,getgidx(int);
	struct flock flock_struct;

	/* AIX security enhancement				*/
	/* create with no permissions. They're added below	*/
	if ((fd = open(fname, O_WRONLY | O_CREAT, 0 )) == -1)
	    syserr((int)EXITFILE,MSGSTR(MSGNOPN,"cannot open %s for writing."),fname);

	flock_struct.l_type = F_WRLCK;
	flock_struct.l_whence = 0;
	flock_struct.l_start = 0;
	flock_struct.l_len = 0;
	if ( -1 == fcntl(fd, F_SETLKW, &flock_struct ) )
	    syserr((int)EXITFILE,MSGSTR(MSGNLCK,"cannot lock %s for writing."),fname);

	if ( -1 == ftruncate(fd, 0) )
	    syserr((int)EXITFILE,MSGSTR(MSGNTRN,"cannot truncate %s."),fname);

/* AIX security enhancement	*/
	/* acquire the privilege to make the chown() call	*/
	privilege(PRIV_ACQUIRE);

	/* get the effective gid to chown() the qconfig.bin file	*/
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITFILE,MSGSTR(MSGGDGS,"Cannot get effective gid for digester."));

	/* change the owner on digester file from user to a system user (ie. sys)	*/
	/* this will protect the file from tampering by a user.		*/
	if (chown(fname, OWNER_ADMDATA, effgid) == -1)
		syserr((int)EXITFILE,MSGSTR(MSGCHWN,"Cannot chown %s. Effective gid = %u."), fname, effgid);

	/* set the permissions on the qconfig.bin file	*/
	if (acl_set(fname, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
		syserr((int)EXITFILE,MSGSTR(MSGPERM,"Cannot set permissions on %s."), fname);

	/* redrop our privileges until we need them again	*/
	privilege(PRIV_LAPSE);
/* AIX security enhancement	*/

	/*----Write the modification date of qconfig */
	dowrite(fd,&qmdate,sizeof(qmdate),fname);

	/*----Write the queue-device structure */
	for ( ; ql; ql = ql->q_next)
	{   dowrite(fd,ql,sizeof(struct q),fname);
	    for (dl = ql->q_devlist; dl; dl = dl->d_next)
	    {   dowrite(fd,dl,sizeof(struct d),fname);
		belength = strlen(dl->d_backend)+1;
		dowrite(fd,&belength,sizeof(belength),fname);
		dowrite(fd,dl->d_backend,belength,fname);
	    }
	}
	return(0);
}


/* check a write result */
dowrite(fd,buf,len,name)
{       int wrote;

	wrote = write(fd,buf,len);
	if (wrote != len)
	    syserr((int)EXITFILE,MSGSTR(MSGEWRT,"write error on file %s."),name);
}


/* get first len-1 bytes of next line, null-terminate */
getqline(linep, len, filep)
register char  *linep;
register FILE  *filep;
{       register int  c;
	register char *eline;

	eline = linep + len - 1;


	while ((c = getc(filep)) != EOF)
	{   if (c == '\n')
	    { 
		linep--;
		while (iswspace(*linep))
			linep--;
		*(++linep) = 0;
		return(TRUE);
	    }
	    if (linep < eline)
		*linep++ = c;
	}
	return(FALSE);
}

/* make sure queue name is not used more then once. */ 

checkq(qlist,quename)
register struct q *qlist;
char quename[QNAME + 1];
{
	for(;qlist != NULL;qlist = qlist->q_next)
	{
	  	if (!strcmp(qlist->q_name,quename))
		{
		   sprintf(sbuf,MSGSTR(MSGCHECK,"queue name %s cannot be used more then once."),quename);
		   error(sbuf);
		}
	}
}

