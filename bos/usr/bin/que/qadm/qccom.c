static char sccsid[] = "@(#)20	1.21.1.3  src/bos/usr/bin/que/qadm/qccom.c, cmdque, bos411, 9438C411a 9/23/94 11:45:23";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <IN/standard.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common.h"
#include "qcadm.h"
#include <ctype.h>
#include <sys/tcb.h>


/*----Definitions for temporary qconfig working file */
/*---- and owner/group for /etc/qconfig */
#define	TMP_NAME "qadm.config"
#define TMP_PERM 0664

/*----State machine defs for parsing lines in a file (read_qconfig) */
#define STINIT	0               /* initial state, looking for first item */
#define STINQUE 1               /* in queue stanza, looking for "device = " */
#define STINDEV	2               /* verifying devices of a queue */

/*----State machine defs for parsing chars in a line (parseline) */
#define STINVAR	3		/* looking for end of queue ':' or variable name ' ' */
#define STINSPC 4		/* looking for delimiter symbol "=" */
#define STPSTEQ 5		/* looking for at least one space after '=' */


#include "qcadm_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_QCADM,num,str)

char	errstr[MAXLINE];	/* for error messages only */
struct quelist	*qdgarbage = NULL;
struct clause	*clgarbage = NULL;
extern char	*progname;

#if 0
char *                  /* debug */
dirname (path)
     char *path;
{
  char        *cp;
  static char  buf[PATH_MAX];

  if (!(cp = strrchr (path, '/')))
      return(cp);
  buf [cp-path] = '\0';
  return(strncpy (buf, path, cp - path));

}
#endif


/*====ADD A DESIRED QUEUE/DEVICE TO LINKED LIST */
addqd(que,dev,flagval,qdlist)
char		*que;			/* contents to put into new element */
char		*dev;
boolean		flagval;
struct quelist	**qdlist;		/* linked list to add to */
{
	struct quelist *newque;

	/*----Allocate the space for new item */
	if(qdgarbage == NULL)
		newque = (struct quelist *)Qalloc(sizeof(struct quelist));
	else
	{
		newque = qdgarbage;
		qdgarbage = qdgarbage->next;
	}

	/*----Set the value of the item */
	TRUNC_NAME( que, QNAME ) ;
	strcpy(newque->qname,que);
	TRUNC_NAME( dev, DNAME ) ;
	strcpy(newque->dname,dev);
	newque->flag = flagval;
	newque->clauses = NULL;

	/*----Link it in */
	newque->next = *qdlist;
	*qdlist = newque;
	return;
}

/*====ADD A STANZA CLAUSE TO DEVICE OR QUEUE (NO GARBAGE COLLECTION NEEDED) */
addcl(line,clist)
char		*line;		/* string containing clause (line) */
struct clause	**clist;	/* clause list to add to */
{
	struct clause	*newclause;	/* pointer to newly created clause */

        /*----Allocate the space for new item */
	if(clgarbage == NULL)
		newclause = (struct clause *)Qalloc(sizeof(struct clause));
	else
	{
		newclause = clgarbage;
		clgarbage = clgarbage->cnext;
	}

        /*----Set the value of the item */
	strcpy(newclause->ctext,line);
	newclause->cflag = FALSE;

	/*----Link it in */
	newclause->cnext = *clist;
	*clist = newclause;
	return;
}

/*====INVERT A CLAUSE LIST */
clinvert(clist)
struct clause 	**clist;       /* list to reverse */
{
        register struct clause *thiscl;        /* current element */
        register struct clause *newcl;         /* new, reversed list */

        for(newcl = NULL;
            *clist != NULL;)
        {
                thiscl = *clist;
                *clist = (*clist)->cnext;
                thiscl->cnext = newcl;
                newcl = thiscl;
        }
        *clist = newcl;
        return;
}

/*====PRINT OUT ALL CLAUSES IN A CLAUSE LIST */
spew_clauses(wfile,clauselist)
FILE            *wfile;
struct clause   **clauselist;
{
        struct clause   *thisclause;

        clinvert(clauselist);
        for(thisclause = *clauselist;
            thisclause != NULL;
            thisclause = thisclause->cnext)
                writln(wfile,thisclause->ctext);
        dumpcl(clauselist);
        return;
}

/*====THROW A QUEUE/DEVICE LIST AWAY */
dumpqd(qdlist)
struct quelist	**qdlist;	/* pointer to doomed list */
{
	struct quelist	*thisqd;	/* current element */

	while(*qdlist != NULL)
	{
		thisqd = *qdlist;
		*qdlist = (*qdlist)->next;
		thisqd->next = qdgarbage;
		qdgarbage = thisqd;
	}
	return;
}

/*====THROW A QUEUE/DEVICE LIST AWAY */
dumpcl(cllist)
struct clause  **cllist;	/* pointer to doomed list */
{
        struct clause  *thiscl;        /* current element */

        while(*cllist != NULL)
        {
                thiscl = *cllist;
                *cllist = (*cllist)->cnext;
                thiscl->cnext = clgarbage;
                clgarbage = thiscl;
        }
	return;
}

/*====WRITE QUEUE STANZA TO OUTPUT FILE */
write_stanza(wfile,pqdlist,itisaque)
FILE            *wfile;
struct quelist  *pqdlist;
boolean		itisaque;
{
        char    	thisline[MAXLINE];      /* for constructing line images */
	struct clause	*thisclause;		/* current line clause of queue */

        /*----Append a blank line for visual beauty */
        /* writln(wfile,"");  naw, never mind */

        /*----Append the queue (or device) name line */
	if(itisaque == TRUE)
        	sprintf(thisline,"%s:",pqdlist->qname);
	else
        	sprintf(thisline,"%s:",pqdlist->dname);
        writln(wfile,thisline);         /* no need to scan list, only one queue allowed */

        /*----Append the clauses */
        for(thisclause = pqdlist->clauses;
            thisclause != NULL;
            thisclause = thisclause->cnext)
        {
                sprintf(thisline,"\t%s",thisclause->ctext);
                writln(wfile,thisline);
        }
        return;
}

/*====SET FLAG TO VALUE FOR THE ENTIRE LIST */
set_listflags(qdlist,value)
struct quelist	*qdlist;	/* list to change */
boolean		value;		/* value to change flag to */
{
	for(;
	    qdlist != NULL;
	    qdlist = qdlist->next)
		qdlist->flag = value;
	return;
}

/*====SET FLAGS IN LIST OF CLAUSES TO A VALUE */
set_clauseflags(cllist,value)
struct clause	*cllist;
boolean		value;
{
	for(;
	    cllist != NULL;
	    cllist = cllist->cnext)
		cllist->cflag = value;
	return;
}

/*====SEARCH QUEUE/DEVICE LIST FOR A MATCHED QUEUE DEVICE */
struct quelist *searchqd(que,dev,qdlist)
char		*que;		/* queue to match */
char		*dev;		/* device to match */
struct quelist	*qdlist;	/* list to search */
{
	register struct quelist	*thisqd;
	for(thisqd = qdlist;
	    thisqd != NULL;
	    thisqd = thisqd->next)
		if( ! strncmp( que, thisqd->qname, QNAME ) &&
		    ! strncmp( dev, thisqd->dname, DNAME ))
			break;
	return(thisqd);
}

/*====OPEN A NAMED FILE FOR WRITING, READING, OR APPENDING */
FILE *open_stream(fname,mode)
char		*fname;
char		*mode;
{
	FILE   *file;

        if((file = fopen(fname,mode)) == NULL)
	{
		switch ( errno ) {
		  case EACCES:
		     if (0 == strcmp(mode,"r") || 1 == strcmp(mode, "r+"))
			     syserr((int)EXITFATAL,MSGSTR(MSGORD,"denied permission to open %s for reading"),fname);
		     else if (0 == strcmp(mode,"a") || 1 == strcmp(mode, "a+"))
			     syserr((int)EXITFATAL,MSGSTR(MSGOAPP,"denied permission to open %s for appending."),fname);
		     else if (0 == strcmp(mode,"w") || 1 == strcmp(mode, "w+"))
			     syserr((int)EXITFATAL,MSGSTR(MSGOWR,"denied permission to open %s for writing."),fname);
		     else if (0 == strcmp(mode,"rw") )
			     syserr((int)EXITFATAL,MSGSTR(MSGORDWR,"denied permission to open %s for reading or writing."),fname);
		     else
			     syserr((int)EXITFATAL,MSGSTR(MSGOPERM,"denied permission to open %s."),fname);
		   default:
                     syserr((int)EXITFATAL,MSGSTR(MSGWOPN,"Unable to fopen(\"%s\", \"%s\") errno = %d."),fname,mode, errno);
	        }
	}
	return(file);
} 

/*====READ A LINE FROM A STREAM */
/* ?doesn't check for overflow? */
/* Any changes to this function should also be put in readln2 */
readln(file,str)	/* returns -1 if EOF reached */
FILE	*file;		/* file pointer */
char	*str;		/* string read in */
{
	int	chr;	/* character read in */
	char	*s, *t = str;
	for(;;)
	{
		*str = '\0';
		chr = fgetc(file);
		if(chr == EOF)
			return(NOTOK);
		if(chr == '\n')
		{
			s = t;
			/* if DUMMY appears on line, ignore it, 
				and read the next one */
			while( (s = strchr(s,*DUMMY)) != NULL )
			{
			    if( strncmp(s, DUMMY, strlen(DUMMY)) == 0 )
				return( readln(file, t) );
			    s++;
			}
			/* if not DUMMY, return this line */
			return(OK);
		}
		*str++ = chr;
	}
}

/* The reason for defining this new function, instead of changing the
 * readln() is this that readln() is already being used by many
 * programs, and yet I need a readln() which won't ignore dummy lines,
 * for D30039.
 */
readln2(file,str)       /* returns -1 if EOF reached */
FILE    *file;          /* file pointer */
char    *str;           /* string read in */
{
        int     chr;    /* character read in */
        char    *s, *t = str;
        for(;;)
        {
                *str = '\0';
                chr = fgetc(file);
                if(chr == EOF)
                        return(NOTOK);
                if(chr == '\n')
                {
                        s = t;
                        return(OK);
                }
                *str++ = chr;
        }
}

/*====WRITE A LINE TO STREAM */
writln(file,str)
FILE		*file;		/* file pointer */
register char	*str;		/* string to write */
{
	if(fprintf(file,"%s\n",str) < 0)
		syserr((int)EXITFATAL,MSGSTR(MSGEWRT,"Failure writing to file."));
	return;
}

/*====CREATE A TEMPORARY FILE FOR WRITING TO */
int open_temp(dir,fname)
register char	*dir,		/* directory to create in, NULL = current dir */
		*fname;		/* name of file created */
{
	int 		retry,
			fdesc;

	/*----Use a fixed temp file name, so we can always look at
		the results if we bomb */
	if( dir != NULL )
		sprintf(fname,"%s/%s",dir,TMP_NAME);
	else
		sprintf(fname,"%s",TMP_NAME);
	/*----Create exclusive access, fails if already exists */
	if((fdesc = open(fname,O_RDWR|O_CREAT|O_EXCL,TMP_PERM)) >= 0 )
	{
		close(fdesc);
		return(OK);
	} else
		return(NOTOK);
}

/*====PARSE A LINE FROM QCONFIG AND EXTRACT NAME */
int parseline(qline,name)
char    *qline;		/* line to parse */
char	*name;		/* name extracted from line */
{
        int	i,j,
       		type,   	/* type of qconfig line detected: TYPNAME TYPASSG TYPCOMM TYPBAD */
		state;		/* state machine indicator */
	boolean	done;		/* indicates that type is done */

        for(i = 0,j = 0,done = FALSE,state = STINIT,type = TYPCOMM;
	    done == FALSE;
	    i++)
        {
		switch(qline[i])
		{
		case '\0':
			if(state != STINIT)
				type = TYPBAD;
			done = TRUE;
			break;
		default:
			switch(state)
			{
			case STINIT:		/* moving past spaces before anything */
                		switch(qline[i])
                		{
				case '#':
				case '*':
					if(i == 0)
					{
						done = TRUE;
						break;
					}
				case ':':
				case '=':
					type = TYPBAD;
					done = TRUE;
					break;
				case ' ':
				case '\t':
					break;
				default:
					name[j++] = qline[i];
					state = STINVAR;
				}
				break;

			case STINVAR:		/* reading in characters of device or assignment
						   portion of line */
				switch(qline[i])
				{
				case ':':
					type = TYPNAME;
					done = TRUE;
					break;
				case '=':
					type = TYPBAD;
					done = TRUE;
					break;
				case ' ':
				case '\t':
					state = STINSPC;
					break;
				default:
					name[j++] = qline[i];
				}
				break;

			case STINSPC:		/* passing spaces, looking for '=' */
				switch(qline[i])
				{
				case ':':
					type = TYPBAD;
					done = TRUE;
					break;
				case '=':
					state = STPSTEQ;
					break;
				case ' ':
				case '\t':
					break;
				default:
					type = TYPBAD;
					done = TRUE;
				}
				break;

			case STPSTEQ:
				switch(qline[i])
				{
				case ' ':
				case '\t':
					type = TYPASSG;
					done = TRUE;
					break;
				default:
					type = TYPBAD;
					done = TRUE;
				} /* case */
			} /* case */
		} /* case */
	} /* for */
	name[j] = '\0';
	return(type);
}

/*====GET CONTENTS OF DEVICES VARIABLE (assumes valid assignment line) */
void getdevs(qline,currque,qdlist,value)
char		*qline;
char		*currque;
struct quelist	**qdlist;
boolean		value;
{
	int		i,j;
	boolean		done;
	boolean		endofline;
	char		tmpdev[DNAME +1];

	/*----Skip past the '=' */
	i = 0;
	done = FALSE;
	while(done != TRUE)
	{
		switch(qline[i])
		{
		case '\0':		/* just to be safe */
		case '=':
			done = TRUE;
			break;
		}
		i++;
	}

	/*----Get each device */
	endofline = FALSE;
	while(endofline != TRUE)
	{
		/*----Skip past any blank space */	
		done = FALSE;
		while(done != TRUE)
		{
			switch(qline[i])
			{
			case ',':
			case ' ':
			case '\t':
				i++;
				break;
			default:
				done = TRUE;
			}
		}

		/*----Get variable */
		j = 0;
		while( 1 )
		{
			if ( j < DNAME )
				switch(qline[i])
				{
				case '\0':
					endofline = TRUE;
				case ',':
					break;
				default:
					tmpdev[j++] = qline[i++];
					continue;
				}
			else
			{
				done = FALSE;
				while (TRUE != done)
				{
					switch (qline[i])
					{
					case '\0':
						endofline = TRUE;
						done = TRUE;
						break;
					case ',':
						done = TRUE;
						break;
					default:
						i++;
						continue;
					}
				}
			}
			break;
		}
		tmpdev[j] = '\0';
		/*---- dump all trailing blanks from device and add to queue list */
		strip(tmpdev);
		addqd(currque,tmpdev,value,qdlist);
	}
	/* error if no devices extracted */
	if(*qdlist == NULL)
	{
		sprintf(errstr,MSGSTR(MSGNDEV,
			"Devices clause for queue: %s contains no devices."),
			currque);
		mangled(0,qline,errstr);
	}
	return;
}

/*====STRIP OFF ALL TRAILING BLANKS FROM A STRING */
strip(str)
char	*str;			/* string to strip */
{
	int	len;		/* starting position of backwards scan */

	if (str != NULL && str[0] != '\0')
		for (len = strlen(str) - 1; (str[len] == ' ') || (str[len] == '\t'); len--)
			str[len] = '\0';
	return;
}

/*====READ THE QCONFIG FILE AND SET UP QUEUE/DEVICE LIST */
void read_qconfig(qdlist)
struct quelist		**qdlist;		/* linked list pointer to add to */
{
        FILE	 	*rfile;			/* file descriptor */
        char            currque[QNAME +1];		/* name of current queue stanza */
        char            tmpstr[MAXLINE];	/* temp string */
        char            thisline[MAXLINE];	/* storage for current line */
	char		*x;			/* pointer */
        int             state;			/* state machine indicator */
	int		type;			/* type of qconfig line detected */
        int             err;			/* error status variable */
	int		line;			/* current line in qconfig file */
	FILE		*open_stream();		/* function declarations */
	int		parseline();
	int		ck_integ();
	int		integ_done();
	int		readln();

	/*----Open the qconfig file */
	rfile = open_stream(QCONFIG,"r");

	/*----Preliminary setup */
	*qdlist = NULL;
	currque[0] = '\0';
	tmpstr[0] = '\0';
	thisline[0] = '\0';
	line = 0;
	state = STINIT;
	err = OK;

	/*----For each line in file act appropriately */
	while(err == OK)
	{
		/*----Read in the line, and and act on it according to state machine */
		err = readln(rfile,thisline);
		line++;
		type = parseline(thisline,tmpstr);
		if(type == TYPBAD)
		{
			mangled(line,thisline,MSGSTR(MSGTBAD,"Syntax error in qconfig file."));
		}
		switch(state)
		{
		case STINIT:    /* State: looking for queue name in first line */
			/*----Check for name-type field */
			switch(type)
			{
			case TYPNAME:
				TRUNC_NAME( tmpstr, QNAME ) ;
				strcpy(currque,tmpstr);
				addqd(currque,"",FALSE,qdlist);	
				state = STINQUE;
				break;

			case TYPASSG:
				mangled(line,thisline,MSGSTR(MSGASSG,
				        "Assignment statement found before queue name."));
			}
			break;

			case STINQUE:
				/* State: looking for "device = " or another queue */
				/*----Check for another queue name first */
				switch(type)
				{
					case TYPNAME:
						/*----Set new queue name */
						TRUNC_NAME( tmpstr, QNAME ) ;
						strcpy(currque,tmpstr);
						addqd(currque,"",FALSE,qdlist);	
						break;

					/*----Check for assignment line w or w/o "device = " */
					case TYPASSG:
						/*----If device variable, extract devices and put in list */
						if(!strcmp(tmpstr,DEVICES))
						{
							getdevs(thisline,currque,qdlist,TRUE);
							state = STINDEV;
						}
						break;
					}
					break;

			case STINDEV:   /* State: checking named devices for integrity */
				/*----Check for a device name line first */
				switch(type)
				{
				case TYPNAME:
					/*----Check if this device was listed in "device = " clause */
					if(ck_integ(currque,tmpstr,*qdlist) == NOTOK)
					{
						/*----Not listed, if integrity test complete, then
						      it must be a name of a queue */
						if(integ_done(*qdlist) == OK)
						{
							TRUNC_NAME( tmpstr, QNAME ) ;
							strcpy(currque,tmpstr);
							addqd(currque,"",FALSE,qdlist);
							state = STINQUE;
							break;
						}
						/*----Not listed, integrity test not complete, bad */
						mangled(0,strcat(currque,":"),MSGSTR(MSGDEVC,
						        "Device stanzas do not match \"device =\" clause."));
					}
					/*----Listed, must be ok, look for next device */
					break;

					/*----Check for assignment lines next */
					case TYPASSG:
						/*----If device variable, error, not allowed in device stanza */
						if(!strcmp(tmpstr,DEVICES))
							mangled(line,thisline,MSGSTR(MSGDEVD,
							        "Illegal \"device =\" clause in device stanza."));
						break;
				}
		} /* case */
	} /* for */

	/*----Close the stream */
	fclose(rfile);

	/*----Check final integrity of file */
	if(integ_done(*qdlist) == NOTOK)
		mangled(0,currque,MSGSTR(MSGDEVC,
		        "Device stanzas do not match \"device =\" clause."));

	/*----Reverse the order of the list */
	qdinvert(qdlist);
	return;
}

/*====SEE IF PRESENT QUE:DEV IS LISTED IN MASTER LIST (QDLIST) */
int ck_integ(que,dev,qdlist)
char            *que;		/* queue to check for */
char            *dev;		/* device to check for */
struct quelist  *qdlist;	/* list to check */
{
        register struct quelist *thisqd;
	struct quelist		*searchqd();

        /*----Check for not found at all */
        thisqd = searchqd(que,dev,qdlist);
        if(thisqd == NULL)
                return(NOTOK);

        /*----Check for originality */
        if(thisqd->flag == TRUE)
	{
                thisqd->flag = FALSE;
                return(OK);
        }
        /*----Must be re-defined */
	return(NOTOK);
}

/*====CHECK IF INTEGRITY TEST IS COMPLETELY DONE FOR QUE:DEV LIST */
int integ_done(qdlist)
struct quelist  *qdlist;	/* list to check */
{
        register struct quelist *thisqd;

        for(thisqd = qdlist;
            thisqd != NULL;
            thisqd = thisqd->next)
                if(thisqd->flag == TRUE)
                        return(NOTOK);
        return(OK);
}

/*====REVERSE THE ORDER OF A QUEDEV LIST */
qdinvert(qdlist)
struct quelist	**qdlist;	/* list to reverse */
{
	register struct quelist	*thisqd;	/* current element */
	register struct quelist *newqd;		/* new, reversed list */

	for(newqd = NULL;
	    *qdlist != NULL;)
	{
		thisqd = *qdlist;
		*qdlist = (*qdlist)->next;
		thisqd->next = newqd;
		newqd = thisqd;
	}
	*qdlist = newqd;
	return;
}

/*====INDICATE BAD QCONFIG FILE */
mangled(line,linestr,message)
int	line;		/* line number of qconfig file, 0 = not determined */
char	*linestr;	/* faulty line from qconfig */
char	*message;	/* error message to print */
{
	if(line != 0)
		systell(MSGSTR(MSGLINE,"Problem with line %d in %s:"),line,QCONFIG);
	else
		systell(MSGSTR(MSGQCON,"Problem with %s:"),QCONFIG);
	sysraw("%s\n",linestr);
	syserr((int)EXITFATAL,message);
}

/* === lock /etc/qconfig */
lock_qconfig(mode)
int mode;		/* READ=0 OR WRITE=1 */
{
	register int fd;
	int fmodes = O_RDONLY;
	extern int errno;
	/* AIX security enhancement	*/
	gid_t	effgid,getgidx();
	struct flock flock_struct;

	if (mode) fmodes = O_WRONLY | O_CREAT;
	/* AIX security enhancement				*/
	/* create with no permissions. They're added below	*/
	if ((fd = open(QCONFIG, fmodes)) == -1)
	        syserr((int)EXITBAD,
		    mode 
			? MSGSTR(MSGNOPNR,"Cannot open %s for writing") 
			: MSGSTR(MSGNOPNW,"Cannot open %s for reading"),
		    QCONFIG);

	flock_struct.l_type = mode ? F_WRLCK : F_RDLCK;
	flock_struct.l_whence = 0;
	flock_struct.l_start = 0;
	flock_struct.l_len = 0;
	if ( -1 == fcntl(fd, F_SETLKW, &flock_struct ) )
	    syserr((int)EXITBAD,
		mode
		    ? MSGSTR(MSGNLCKW,"Cannot lock %s for writing")
	    	    : MSGSTR(MSGNLCKR,"Cannot lock %s for reading"),
		QCONFIG);
	return(0);
}

/*====move new qconfig in place and digest */
dogoose(file)
char *file;
{
	char *cmdbuf;
	int tcbflag;

	/* THE 4 is for " -d" and a NULL on the end */
	cmdbuf = Qalloc((size_t)(strlen(ENQ)+4));

	/* Get the current TCB status of /etc/qconfig */
	tcbflag = tcb(QCONFIG, TCB_QUERY);

	/*----put new qconfig file into place */
	/*	remove the old one */
	if(unlink(QCONFIG))
	    syswarn(MSGSTR(MSGULNK,"Cannot unlink old %s file"), QCONFIG);

	/*	this assumes that we're running setuid root, and setgid
		printq, and that these are the correct owners of QCONFIG */
	if(chown(file,geteuid(),getegid()) || 
			chmod(file,TMP_PERM) || rename(file,QCONFIG))
	    syswarn(MSGSTR(MSGINST,"Cannot install new %s file"), QCONFIG);

	/* Reset the original TCB status of /etc/qconfig to the new one */
	if((tcbflag == -1) || (tcb(QCONFIG, tcbflag)))
	    syswarn(MSGSTR(MSGINST,"Cannot install new %s file"), QCONFIG);

	strcpy(cmdbuf,ENQ);
	strcat(cmdbuf," -d");
	system(cmdbuf);
	free(cmdbuf);
}

/*====see if the temp file is digestable before installing it. */
trydigest(infile)
char *infile;
{
	char bconfig[MAXPATH], aconfig[MAXPATH];
	char error_msg[MAXLINE];
	int pid, status=0, got, place=0;
	int p[2];

	memset((void *)error_msg, (int)0, (size_t)MAXLINE);

	strncpy((char *)aconfig,infile,(size_t)MAXPATH);	/* temp. qconfig */

	/*----Generate template for temp. qconfig.bin and get new filename */
	sprintf(bconfig,"%s/%cXXXXXX",TMPDIR,progname[0]);
	mktemp(bconfig);

	if (-1==pipe(p))
		syserr((int)EXITFATAL,MSGSTR(MSGPIPE,"Digest pipe error."));

	switch( (pid = fork()) )
	{
	case -1:
		syserr((int)EXITFATAL,MSGSTR(MSGFORK,"Cannot fork for %s."),DIGEST);

	case 0:                          /* child */
		close(2);                /* close stderr */
		dup(p[1]);
		close(p[0]);
		close(p[1]);

		execl(DIGEST,"digest",aconfig,bconfig,0);
		syserr((int)EXITFATAL,MSGSTR(MSGEXEC,"Cannot exec digester %s."),DIGEST);

	default:                        /* parent */
		close(p[1]);
		while ((got = read(p[0], error_msg + place, MAXLINE-1)) > 0)
			place += got;
		error_msg[place] = '\0';
		close(p[0]);

		errno = 0;

		while( (got = waitpid(pid,&status,0)) != pid )
		{
			if( got == -1 && errno == EINTR )
			{
				/*
				* Wait was interrupted by a signal, probably
				* a new incoming request.  Ignore it.
				*/
				errno = 0;
				continue;
			}
			syserr((int)EXITFATAL,MSGSTR(MSGWPID,"Redigest wait: wrong pid."));
		}

	}

	unlink(bconfig);
	if( status )
	{
		sysraw(error_msg);
		syserr((int)EXITFATAL,MSGSTR(MSGDIGE,
			"Error from digester %s, status = %d, rv = %d."),
		DIGEST,status,got);
	}
	return(0);
}

/* === Check if the attribute clause has no value to right of equal sign */
chk_empty(char *attr_str)
{
	int	i = 1;			/* loop counter */
	char	*idx;			/* index of equal sign in clause */

	idx = strchr(attr_str,'=');
	while(idx[i] == ' ' || idx[i] == '\t')
		i++;
	if(idx == NULL || idx[i] == '\0')
		syserr((int)EXITFATAL,MSGSTR(MSGATTR,
			"Attribute value missing: \'%s\'."),attr_str);
}
