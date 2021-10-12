static char sccsid[] = "@(#)84	1.2  src/bos/usr/lib/pios/piovalfgs.c, cmdpios, bos411, 9428A410j 4/7/94 16:04:22";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: piovalflags, config_obe, pio_info, get_str, getdocpipe,
 *            chkprohib
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/lpio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <memory.h>
#include <time.h>
#include <locale.h>
#include "pioformat.h"

#define MAXFILES  100           /* maximum number of files per invocation */
#define HDRPIPE   1
#define TRLPIPE   2

#define SET_FLAG(flagattr, strptr, strlen)   \
  { attr_tab_p2 = get_attrtab_ent(flagattr); \
    attr_tab_p2->ptypes.sinfo->ptr = strptr; \
    attr_tab_p2->ptypes.sinfo->len = strlen; \
    attr_tab_p2->flags |= OVERRIDE;  }

#define RESET_FLAG(flagattr)                 \
  { attr_tab_p2 = get_attrtab_ent(flagattr); \
    attr_tab_p2->ptypes.sinfo->len = 0;      \
    attr_tab_p2->flags |= OVERRIDE;          \
    attr_tab_p2->flags ^= OVERRIDE;  }


/*
** Option list passed to piogetopt
*/
static char optlist[] = "a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:\
A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z:\
0:1:2:3:4:5:6:7:8:9:";


/* The following are referenced by subroutines and so must exist */
int     statusfile = 0;         /* we're not running under the spooler */
int     objtot = 0;             /* Number of objects in ODM tables */
int     piopgskip = 0;          /* needed for pioputc and pioputchar */
int     piodatasent = 0;        /* needed for pioputc and pioputchar */

int     piomode = PIO_CURR_MODE;/* ODM values overridden by command line */

static char    fragname[100];          /* ".queuename:queuedevicename" */
static struct str_info *get_str(char *attrname,int msgnum);
static struct str_info *strinfop;
static char  basedir[PATH_MAX+1];
static char  basevardir[PATH_MAX+1];
static struct attr_info *attr_tab_p;

static const char *qname;
static const char *qdname;

static char odm_fullpath[PATH_MAX+1];	/* full path name of odm file */
char	*mem_start;		/* start address of odm data memory */
char	*hash_table;		/* address of primary hash table */
char	*odm_table;		/* address of odm data memory */
static char *usedbypiobe[] =
			 {PREVIEWLEVEL, DIAGLEVEL, IN_DATASTREAM, FILTERNAME,
			  COPIES, PRINTQUEUE, BURSTPAGE, MAILONLY,
                          "_H", "_X"};
static int *dptr;               /* dummy pointer for piogetvals to store into*/

extern char piopipeline;            /* set by pioparm() when it finds %ix */
extern int piofilterloc;            /* set by pioparm() when it finds %p */
extern int piodevdriloc;            /* set by pioparm() when it finds %z */
extern short usedbysomeone;

extern char *progname;

extern char 		*getmsg(char *,int,int);
extern int		piogetstr(char *, char *, int, error_info_t *);
extern char		*piovalav(const char *attrnm);
extern int              attrtab_init(char *);
extern int		piogetvals(struct attrparms *attr_parm,
				    int return_on_err);
extern int		piogetopt(int ac, char **av, char *optstring,
				   int return_on_err);
extern void		piomsgout(char *msgstr);
extern int		pioexit(int exitcode);

static char *getdocpipe(char,int);
static int chkprohib(char *);

/* attrparm parameter to pass to piogetvals() */
static struct attrparms attrval[] = {
{ INTERFACETYPE, VAR_INT, NULL, (union dtypes *) &dptr },
{ NULL,                0, NULL, NULL }
};

/*
*******************************************************************************
*******************************************************************************
** NAME:	piovalflags()
**
** DESCRIPTION:	piovalflags() validates the command line flags for a specified
**		queue and queue device pair.  This function performs the same
**		validation as that is done in 'piobe', and is invoked by
**		'qprt' at the time of print job submission.  The validation
**		is done here so that any errors that my be found during the
**		actual processing of the print file (by 'piobe') would be
**		found right at job submission time.  Validation involves
**		checks for invalid flags, verification to see if specified
**		flags are appropriate for the selected data stream (and
**		prefix processing, if any), and finally, type and range
**		validation of values for flags.
**
** PARAMETERS:	qnm	- queue name
**		qdnm	- queue device name
**		ac	- number of arguments passed, including the name
**		av	- pointer to the argument vector
**
*******************************************************************************
*******************************************************************************
*/

void
piovalflags(const char *qnm,const char *qdnm,uint_t ac,char **av)
{
    register char	*cp;
    register char	*cp1;
    char        datastream_db;
    char        attrname[3];
    int         fraglen;
    int		namelen;		/* length of file name found */
    int         cnt, val;
    DIR		*dirp;
    struct dirent *dp;		/* points to particular directory entry */

    int         flagchar;
    int		szused;
    int		i;
    uint_t	valerrs = 0;


    /* Set Up the Base Directory */
    /* Set Up the Base Variable Directory (the base variable directory */
    /* defaults to /var/spool/lpd/pio/@local )   */
    (void)strncpy(basedir,(cp = getenv("PIOBASEDIR"))?cp:DEFBASEDIR,
		  sizeof(basedir)-1),
    *(basedir+sizeof(basedir)-1) = 0;
    (void)strncpy(basevardir,(cp1 = getenv("PIOVARDIR"))?cp1:cp?cp:DEFVARDIR,
		  sizeof(basevardir)-1),
    *(basevardir+sizeof(basevardir)-1) = 0;

    /* set up info. in case we have to generate an error message */
    (void) strncpy(errinfo.progname, progname, sizeof(errinfo.progname) - 1);

    /*
    ** First must try to find the ddi data file for this queue and device.
    ** The name is of the form:
    **			*.*.*.quename:devname
    ** The que and device names can be obtained with qdaemon calls.  Once
    ** accomplished a partial name can be constructed.
    */

    (void) strncpy(errinfo.qname ,  qname = qnm, sizeof(errinfo.qname)  - 1);
    (void) strncpy(errinfo.qdname, qdname = qdnm, sizeof(errinfo.qdname) - 1);

    (void) strcpy(fragname,".");
    (void) strcat(fragname,qname);
    (void) strcat(fragname,":");
    (void) strcat(fragname, qdname);

    fraglen = strlen(fragname);

    (void) strcpy(odm_fullpath,basevardir);
    (void) strcat(odm_fullpath, "/ddi/");

    if((dirp = opendir(odm_fullpath)) == NULL)
	ERREXIT(0, MSG_DIRFAIL, odm_fullpath, NULL, 0);

    /*
    ** Know that the full name containing the partial name created is unique.
    ** Therefore, only the end portion of the full path name must match the
    ** partial name we have created.  So when doing a string compare, can
    ** increment the pointer past the first part of the name and compare the
    ** end portion.
    */

    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	namelen = strlen(dp->d_name);
	if (!strcmp(dp->d_name + (namelen - fraglen), fragname))
	{
	    break;                     /* we found it */
	}
    }

    /* See if we found a data base file */
    /* If our query is not successful, just skip all the validation.
       This is a generic queue. */
    if (dp == NULL)
	return;

    (void) strcat(odm_fullpath, dp->d_name);
    (void)closedir(dirp);

    /*
    ** Open and read the data base file
    */

    (void) attrtab_init(odm_fullpath);

    /* Set Up @4 Attribute Value (path for base (i.e., "pio") directory) */
    attr_tab_p = get_attrtab_ent(BASE_DIR);
    attr_tab_p->ptypes.sinfo->ptr = basedir;
    attr_tab_p->ptypes.sinfo->len = strlen(basedir);

    /* Set Up @5 Attribute Value (path for var (i.e., "pio") directory) */
    attr_tab_p = get_attrtab_ent(BASE_VARDIR);
    attr_tab_p->ptypes.sinfo->ptr = basevardir;
    attr_tab_p->ptypes.sinfo->len = strlen(basevardir);

    /* Fix it so piogetopt won't turn on USEDBYSOMEONE flag */
    usedbysomeone = 0;

    /*
    ** Process command line flags
    */

    (void) piogetopt(ac, av, optlist, 0);

    /* piogetopt is done, so put things back like they were */
    usedbysomeone = (short) USEDBYSOMEONE;

    /* determine the type of device interface and save it in     */
    /* attribute @3, which can be referenced by other attributes */
    val = 0;				/* initial assumption: unknown type */
    attr_tab_p = get_attrtab_ent(INTERFACETYPE);
    if (attr_tab_p != NULL)		/* safety valve */
    {
      (void) piogetvals(attrval, 0);     /* set up variable integer attr. */
      for (cnt = 0; cnt < PIO_NUM_MODES; cnt++) /* initialize integer values */
        (attr_tab_p->ptypes.ivals)->value[cnt] = val;
    }

    /* Set Up Flag For Print Queue and Queue Device Names */
    /* So It Looks Like We Got It From the Command Line   */
    attr_tab_p = get_attrtab_ent(PRINTQUEUE);
    attr_tab_p->ptypes.sinfo->ptr = fragname + 1; /* skip over '.' */
    attr_tab_p->ptypes.sinfo->len = strlen(fragname + 1);

    /* Check For Command Line Flags Prohibited For All Data Streams */
    (void) chkprohib(PROHIBITEDFLGS);

    /* Check For Command Line Flags Prohibited For The Input Data Stream */
    strinfop = get_str(IN_DATASTREAM, MSG_ATTRFAIL);
    datastream_db = strinfop->ptr[0];
    (void) strcpy(attrname, PROHIBITCHAR);
    attrname[1] = datastream_db;
    (void) chkprohib(attrname);

    /* Check for a few attributes. */
    attr_tab_p = get_attrtab_ent(DB_FORMFEED);
    if (attr_tab_p != NULL)  /* if its in the data base */
	(void)get_str(DB_FORMFEED, MSG_ATTRFAIL);  /* data base value */
    (void)get_str(DB_CANCELSTRING, MSG_ATTRFAIL);  /* data base value */
    (void)get_str(DB_NUM_CAN_STRS, MSG_ATTRFAIL);  /* data base value */
    (void)get_str(DB_READROUTINE, MSG_ATTRFAIL);  /* data base value */
    (void)get_str(DB_INT_REQ_USER, MSG_ATTRFAIL);  /* data base value */
    attr_tab_p = get_attrtab_ent(PS_STRING);
    if (attr_tab_p != NULL)    /* if it's in the data base */
    {
	(void)get_str(PS_STRING, MSG_ATTRFAIL); /* PostScript string*/
	(void)get_str(PS_DS_NAME, MSG_ATTRFAIL); /* pipeline ID */
    }
    (void)get_str(DEVICENAME, MSG_ATTRFAIL);  /* data base value */
    (void)get_str(PRINTERTYPE, MSG_ATTRFAIL);    /* data base value */
    strinfop = get_str(PREVIEWLEVEL, MSG_ATTRFAIL);
    if (!atoi(strinfop->ptr))
	(void)get_str(DIAGLEVEL, MSG_ATTRFAIL);
    (void)get_str(TEMPDIR, MSG_ATTRFAIL);
    (void)get_str(ATTR_WIDTH, MSG_ATTRFAIL);
    attr_tab_p = get_attrtab_ent(PRINTERDESC);
    if (attr_tab_p != NULL)  /* if it's in the data base */
	(void)get_str(PRINTERDESC, MSG_ATTRFAIL);
    else
	(void)get_str(PRINTERTYPE, MSG_ATTRFAIL);
    attr_tab_p = get_attrtab_ent(DATASTRDESC);
    if (attr_tab_p != NULL)  /* if it's in the data base */
	(void)get_str(DATASTRDESC, MSG_ATTRFAIL);

    /* Construct document pipeline so that any references */
    /* to command line flags will be noted                 */
    (void) getdocpipe(datastream_db, 1);

    /*
    ** Update attr_info struct to indicate those arguments that are used by
    ** piobe.  usedbypiobe is defined by:
    **          char *usedbypiobe[] = {"_a","_A"......etc}
    */

    szused = (sizeof(usedbypiobe)/sizeof(char *));
    for (i=0; i < szused; i++)
    {
	if ((attr_tab_p = get_attrtab_ent(usedbypiobe[i])) != NULL)
	    attr_tab_p->flags |= USEDBYPIOBE;
    }

    /*
    ** Check to make sure that there are no unexpected flags
    ** in the command line.  If there are, something is wrong somewhere so
    ** issue error messsage and return.
    */

    (void) strcpy(attrname, FLAGCHAR);
    for (flagchar = 'a'; flagchar != ('9' + 1); flagchar++)
    {
	/* Skip over undefined flag letters */
	if (flagchar == ('z' + 1))
	    flagchar = 'A';
	else if (flagchar == ('Z' + 1))
	    flagchar = '0';

	/* Build attribute name for flag and get pointer to its info. */
	*(attrname + 1) = (char) flagchar;
	if ((attr_tab_p = get_attrtab_ent(attrname)) == NULL)
	    continue;  /* flag not defined in data base, so ignore it */
		       /* if flag is specified on cmd line, piogetopt() */
		       /* will detect it. */

	/* Determine if flag on command line is expected by filter or piobe */
	if (!(attr_tab_p->flags & (USEDBYSOMEONE + USEDBYPIOBE)))
	{
	    if (attr_tab_p->flags & ONCMDLINE)
		ERREXIT(0, MSG_BADFLAG, attrname + 1, NULL, 0)
	    continue;  /* not on command line & nobody is expecting it */
	}

	/* Validate the flag if specified on the command line. */
	if (attr_tab_p->flags & ONCMDLINE)
	   if (cp = piovalav((const char *)attrname))
	   {
	      ERRDISP(0,MSG_DUMMY,cp,(char *)NULL,0);
	      valerrs++;
	   }
    } /* end for flagchar loop */

    /* If there were any validation errors, exit (error messages'd have been
       displayed already). */
    if (valerrs)
       exit(EXIT_FAILURE);

    return;
}


/*
*******************************************************************************
** NAME:	get_str()
**
** DESCRIPTION:	retrieves a string from the attribute parameter table with all
**		it variables resolved.  All error handling is handled in this
**		routine.  The calling process includes the error message
**		number in the parameters passed.  If anything at all goes
**		wrong then the asked for message is output and the process is
**		exited.  This relieves the calling process of having any direct
**              knowledge of the structure of the attribute table.
**
** CALLING
**  SEQUENCE:	struct str_info *
**		  get_str(attribute_name, message_number);
**		  char	*attribute_name
**		  int	message_number
**
*******************************************************************************
*/

struct str_info *
get_str(char *attrname,int msgnum)
{
    char	*buf;
    char        *wkptr;
    struct str_info *strinfo;

    attr_tab_p = get_attrtab_ent(attrname);
    if(attr_tab_p == NULL)
	ERREXIT(0, msgnum, attrname, NULL, 0);

    /*
    ** If the datatype is a VAR_STR then the variables are already
    ** resolved and can pass the pointer to them back directly.  If not
    ** a VAR_STR then they must be resolved manually.
    */

    if(attr_tab_p->datatype == VAR_STR)
    {
	size_t		avlen;

	wkptr = (attr_tab_p->ptypes.sinfo + piomode)->ptr;
	avlen = (attr_tab_p->ptypes.sinfo + piomode)->len;
	if (avlen)
	{
	if (!strncmp(wkptr, YES_STRING, avlen))
	    *wkptr = '1';
	else if (!strncmp(wkptr, NO_STRING, avlen))
	    *wkptr = '0';
	}
	return(attr_tab_p->ptypes.sinfo + piomode);
    }

    MALLOC(wkptr, sizeof(struct str_info));
    strinfo = (struct str_info *) wkptr;
    MALLOC(buf, 2000);
    strinfo->len = piogetstr(attrname, buf, 2000, NULL);
    if (!strcmp(buf, YES_STRING))
	(void) strcpy(buf, "1");
    else if (!strcmp(buf, NO_STRING))
	(void) strcpy(buf, "0");

    strinfo->ptr = buf;		/* buf contains data, update struct */
    return(strinfo);
}

/*
*******************************************************************************
** NAME:        getdocpipe()
**
** DESCRIPTION: return the pipeline for the specified print file number.  If
**              the pipeline had been previously generated for that print
**              file, a pointer to the pipeline is returned.  Otherwise,
**              the pipeline is generated and a pointer returned to the caller.
**
** CALLING
**  SEQUENCE:   char *getdocpipe(ds,opt);
**                char ds;
**                int opt;
*******************************************************************************
*/
char *
getdocpipe(char ds,int opt)
       /* 1: without pioout;  2: pioout also;  3: pioout only */
{
    static char nullstr[] = "";
    char attrname[3];
    char *wkptr;
    char *docpipe1, *docpipe2;
    struct str_info *strinfop_filter;

    if (opt == 3)
	docpipe1 = nullstr;
    else
    {
	/* Get (optional) Filter String */
	strinfop = get_str(FILTERNAME, MSG_ATTRFAIL);  /* get _f attr value */
	strinfop_filter = NULL;
	piopipeline = '\0';   /* so can tell if %ix is in filter pipeline */
	if (strinfop->len >= 1)
	{
	    (void) strcpy(attrname, FILTERCHAR);
	    attrname[1] = strinfop->ptr[0];
	    strinfop_filter = get_str(attrname, MSG_FILTER);
	}

	/* Get Main Pipeline */
	piofilterloc = 0;  /* filter string goes at beginning (offset 0)
			      of the main pipeline, unless pioparm() finds
			      %p in the main pipeline */
        piodevdriloc = -1; /* pioout (device driver interface routine) string
                              goes at end of the main pipeline, unless
                              pioparm() finds %z in the main pipeline */
	(void) strcpy(attrname, PIPELINECHAR);    /* "i" */
	if (piopipeline == '\0')   /* if pioparm() didn't find a %ix */
	{
	    attrname[1] = ds; /* datastream character */
	    strinfop = get_str(attrname, MSG_DATASTREAM);
            if (piodevdriloc == -1)  /* if pioparm() didn't find %z */
                piodevdriloc = strinfop->len; /* pioout goes at end of string*/
	    docpipe1 = strinfop->ptr;
	}
	else
	{  /* pioparm() found a %ix */
	    if (piopipeline == '!')
            {
		docpipe1 = nullstr;   /* make main pipeline a null string */
                piodevdriloc = 0;
            }
	    else
	    { /* use main pipeline as overridden by the filter string (%ix) */
		attrname[1] = piopipeline;
		strinfop = get_str(attrname, MSG_DATASTREAM);
                if (piodevdriloc == -1)  /* if pioparm() didn't find %z */
                    piodevdriloc = strinfop->len; /* pioout at string end */
		docpipe1 = strinfop->ptr;
	    }
	}

	/* Prepend (optional) Filter Pipeline to Main Pipeline */
	/* (leave room for " | " and null terminator)          */
	if (strinfop_filter != NULL && strinfop_filter->len > 0)
	{
	    if (*docpipe1 != '\0')       /* is there a document pipeline? */
	    {
		wkptr = docpipe1;
		MALLOC(docpipe1, strlen(docpipe1) + strinfop_filter->len + 5);
		if (piofilterloc > 0)  /* if filter pipeline not at beginning
		 			  of main pipeline */
		{
		    strncpy(docpipe1, wkptr, piofilterloc);
		    strcat(docpipe1, " ");
                    piodevdriloc += 1;           /* add length of " " */
		    wkptr += piofilterloc;
		}
		else
		    *docpipe1 = '\0';
		(void) strcat(docpipe1, strinfop_filter->ptr);
		(void) strcat(docpipe1, " | ");
                piodevdriloc += 3;               /* add length of " | " */
		(void) strcat(docpipe1, wkptr);
	    }
	    else
		docpipe1 = strinfop_filter->ptr;
            piodevdriloc += strinfop_filter->len;  /* add len of filter str */
	}
    }
    if (opt == 1)
	return(docpipe1);

    /*
    ** Add Device Driver Interface to Document Pipeline
    */

    strinfop = get_str(DDIF_CMDSTRING, MSG_ATTRFAIL);
    if (strinfop->len > 0)
    {
	if (*docpipe1 != '\0')
	{
	    MALLOC(docpipe2, strlen(docpipe1) + strinfop->len + 4);
	    (void) strncpy(docpipe2, docpipe1, piodevdriloc);
            *(docpipe2 + piodevdriloc) = '\0';
	    (void) strcat(docpipe2," | ");
	    (void) strcat(docpipe2, strinfop->ptr);
            (void) strcat(docpipe2, docpipe1 + piodevdriloc);
	}
	else
	    docpipe2 = strinfop->ptr;
    }
    else
	docpipe2 = docpipe1;
    return(docpipe2);
}


/*
*******************************************************************************
** NAME:        chkprohib
**
** DESCRIPTION: Scans the list of flag letters in the attribute string for
**              the specified attribute name to see if any of the flags are
**              specified on the command line.  If so, error message & quit.
**
** CALLING
**  SEQUENCE:   chkprohib(attrname)
**                char *attrname;
**
*******************************************************************************
*/
static int
chkprohib(char *attrname)
{
int cnt;
char *cp;
char flagattr[2];

(void) strncpy(flagattr, FLAGCHAR, 1);
attr_tab_p = get_attrtab_ent(attrname);
if (attr_tab_p != NULL)
{
	strinfop = get_str(attrname, MSG_ATTRFAIL);
	cp = strinfop->ptr;
	for (cnt = 0; cnt < strinfop->len; cnt++)
	{
		*(flagattr + 1) = *(cp + cnt);
		attr_tab_p = get_attrtab_ent(flagattr);
		if (attr_tab_p != NULL)
			if (attr_tab_p->flags & ONCMDLINE)
				ERREXIT(0, MSG_PROHIBFLAG, flagattr+1, NULL, 0);
	}
}

return(0);

}

