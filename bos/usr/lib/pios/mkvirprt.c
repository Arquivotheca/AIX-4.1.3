static char sccsid[] = "@(#)13  1.17.3.9  src/bos/usr/lib/pios/mkvirprt.c, cmdpios, bos41J, 9510A_all 3/4/95 17:49:05";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, interactive, non_interactive, attach,
 *            put_env_vars, replace_text, get_codeset, getmessage,
 *            retrieve_msg, mkv_update_cusfl
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** mkvirprt.c ***/

#include <locale.h>
#include "virprt.h"
#include <stdio.h>
#include <dirent.h>
#include "piobe_msg.h"
#include <memory.h>
#include <locale.h>
#include <langinfo.h>
#include <stdlib.h>
#include	<signal.h>
#include	<errno.h>
#include	<fcntl.h>	/* for O_RDWR   */
#include	<sys/types.h>
#include	<sys/wait.h>
#include <sys/stat.h>

#define BUFLEN 1000 

/**** 	Borrowed from "piostruct.h"	*****/
#define PIO_CFLEVEL_ATTR	"zL"	/* level of a colon file */
#   define PIO_CFLEVEL_NEW	2	/* new colon files with limits */
#   define PIO_CFLEVEL_CONV	1	/* old colon files converted to new */
#   define PIO_CFLEVEL_OLD	0	/* old colon files */


extern char  dsdesc[], rmq[], rmqd[], devstr[], configpath[];
extern char  txtstring[];
extern int configline;
extern char *screen[];

extern char *hdrstrp, *trlstrp;

struct pn_struct *p;
struct sn_struct *s;
char *prgname;		/* Program/Script name */
char *tty_attached;	/* Printer is attached to tty ot not */

static char hdrstr_a[] = "'header = always'";
static char hdrstr_g[] = "'header = group'";
static char trlstr_a[] = "'trailer = always'";
static char trlstr_g[] = "'trailer = group'";

int customize();
char *replace_text();
char *getmessage();
char *get_codeset();
char *retrieve_msg();
static void		mkv_update_cusfl(char const *Prognm);

#define ENVIRONMENT  "/etc/environment"
#define UNIQUE_STR  "###!@$%&*###"

/*===========================================================================*/
void main(argc,argv)
int argc;
char *argv[];
{
char *ptr;

   ptr =  setlocale(LC_ALL,NULL);

   putenv("IFS=' \t\n'");
   get_codeset(ptr);  /* get codeset based on LANG in /etc/environment */

   /* Convert New Flag Letters (if present) To Old Flag Letters (s->d, d->v) */
    { CNVTFLAGS }

    make_files();         /* create full path names from PIOBASEDIR */
 
    prt_head = make_list(prepath,'.');

    if ( argc == 1 )      /* no arguments */
        err_sub(ABORT,USAGE_MK);
    else if ( argc < 4 )  /* there are two arguments on the command line */
        interactive(argc,argv);
    else
        non_interactive(argc,argv);
}

/*===========================================================================*/
void interactive(int argc, char *argv[])
/*
**  Interactively prompt the user for the parameters needed for this
**  command, testing each one separately and performing such additional
**  tasks as creating the queue and queue device if necessary. This really
**  is the equivalent of more than one non-interactive "mkvirprt" command,
**  since the function loops based on the data stream type.
*/
{
    int multi_ds = FALSE;      /* does printer support multiple data streams */
    int cnt;
    char misc[10];
	int c;
	char attach_attr[1024];
	char *tmp_ptr;

    ISATTY;                     /* quit if not on a tty port */

    INIT(ptype);                /* initialze printer type */
    INIT(dstype);               /* initialize data stream type */
    INIT(dname);                /* initialize device name */
    INIT(pqname);               /* initialize print queue name */
    INIT(vpname);               /* initialize virtual printer name */
    INIT(fname);
    INIT(prgname);				/* initialize program/script name */

	/* set prgname if PIOprgname is in the environment */
	if ((tmp_ptr = getenv("PIOprgname")) != NULL)
		(void)strncpy(prgname,tmp_ptr,NAMESIZE);

	/* get the attachment type from the command line */
	opterr = 0;  /* suppress error message */
	while ((c = getopt(argc,argv,":A:")) != EOF)
	{
		switch(c)
		{
		case 'A':
				attach_t = optarg;
				break;
		case ':':   /* error because '-A' didn't have an argument */
    			err_sub(ABORT,USAGE_MK);
				break;
		case '?':
    			err_sub(ABORT,USAGE_MK);
				break;
		}
	}

    /* error if they didn't specify -A or if there were additional
       arguments passed */
    if ((attach_t == NULL) || (optind < argc))
        err_sub(ABORT,USAGE_MK);

    scrn_ht = start_cur();      /* get screen height from starting curses */

    attach();   /* process the .config file */

    if ( !(*ptype) )
        {
        *misc = NULL;
        make_plist(prt_head);
        cnt = resp_menu(GET_PR_NUM,PREDEF_HDG,misc,GETNUM) - 1;
        get_pr( printers[ cnt ] , 1 );
        }
    hilite(scr_line[cnt],TRUE);

    get_burst(ENTER_HS, &hdrstrp, hdrstr_g, hdrstr_a);      /* hdr pg option */
    get_burst(ENTER_TS, &trlstrp, trlstr_g, trlstr_a);      /* trl pg option */

    p = found_pn(prt_head,ptype);
    if (p->sn_head->sn_next) multi_ds = TRUE;   /* does this printer support */
                                                    /* multiple data streams */
    if (multi_ds) err_sub(CONT,MULTI_DS_GREET);
    for ( s=p->sn_head ; s ; s=s->sn_next )
        {
        *pqname = NULL;
        *vpname = NULL;

        do { strcpy(dstype,s->sn);

           sprintf(prefile,"%s%s.%s",prepath,ptype,dstype);
           if (multi_ds)
             {
             getval("mA",dsdesc,prefile);  /* get the datastream description */
             if (!strlen(dsdesc)) strcpy(dsdesc,dstype);/*if none use dstype */
             err_sub(CONT,MULTI_DS_HEAD);           /* display the heading */
             }

          errflag = 0;

          get_queue(pqname,p->sn_head->sn_next,1 + multi_ds);
          if ( *pqname == '!' ) break;
          get_quedev(vpname,1);

          sprintf(cusfile,"%s%s:%s",cuspath,pqname,vpname);
          } while (errflag);

        if ( *pqname == '!' ) continue;

        umask(75);

		/* run piocnvt to copy the predef to custom */
        (void)sprintf(cmd,"%s -s+ -i%s -o%s",PIOCNVT,prefile,cusfile);
		if (system(cmd))
		{
			(void)unlink(cusfile);
			goodbye(TERMINATE);
		}

		/* run script to convert old colon files */
		(void)strcpy(attach_attr,UNIQUE_STR);
		(void)getval(PIO_CFLEVEL_ATTR,attach_attr,cusfile);	
		if (strcmp(attach_attr,UNIQUE_STR) == 0  /* if zL was not found */
		    || strtol(attach_attr,NULL,10) < PIO_CFLEVEL_CONV)
		{
			(void)sprintf(cmd,"%s -i %s",PIOUPDATE,cusfile);
			if (system(cmd) != 0)
			{
				(void)unlink(cusfile);
				goodbye(TERMINATE);
			}
		}

   	        get_codeset(NULL);

		/* run chvirprt to update the custom file */
		(void)sprintf(cmd,"%s -q %s -d %s -a md=%s mn=%s mq=%s mt=%s mv=%s _X=%s zA=%s",
			CHVIRPRT,pqname,vpname,dstype,dname,pqname,ptype,vpname,codeset,attach_t);
		if (system(cmd) != 0)
		{
			(void)unlink(cusfile);  /* clean up the cusfile before exiting */
			goodbye(TERMINATE);
		}

	/* If "prgname" variable was set to a program or script name,
	   execute the specified program/script and update the custom
	   colon file. */
	mkv_update_cusfl(prgname);

/* If this string exists, then the queue device stanza already existed in */
/* /etc/qconfig but the 'file =' line was 'file = false' and must be changed */
        if ( *devstr )
            {
            sprintf(cmd,"%s -q%s -d%s -a %s",CHQUEDEV,pqname,vpname,devstr);
            if (system(cmd))
	    {
		(void)unlink(cusfile);
		goodbye(TERMINATE);
	    }
            }
/* If rmqd string is null, then the queue device stanza already existed in */
/* /etc/qconfig therefore the header and trailer lines will be changed to */
/* reflect the responses given in this invokation of mkvirprt */
        if ( !(*rmqd) )
            {
            sprintf(cmd,"%s -q%s -d%s -a %s -a %s",CHQUEDEV,pqname,vpname,
            hdrstrp?hdrstrp:"'header = never'",
            trlstrp?trlstrp:"'trailer = never'");
            if (system(cmd))
	    {
		(void)unlink(cusfile);
		goodbye(TERMINATE);
	    }
            }


        strcpy(cmd,pqname);         /* build confirmation message */
                            /* kill this when grep method works */

        if (multi_ds) err_sub(CONT,MKVIR_SUMMARY_DS);
        else err_sub(CONT,MKVIR_SUMMARY);

        }  /* for */

    if ( getenv("PIOSMIT") ) proceed(CONFIRM);
    goodbye(GOOD);
}

/*============================================================================*/
void non_interactive(argc,argv)
/*
**  Perform a "mkvirprt" operation for a single virtual printer. This
**  is the same as the function "interactive()" except that it does NOT
**  interact with the user and can only make one printer at a time. All
**  the flags and their parameters must be specified, except that the
**  data stream type may be omitted unless there is a unique data stream
**  for the specified printer type.
*/
int argc;
char *argv[];
{
  char attach_attr[1024];
  char *tmp_ptr;
  struct stat sbuf;

  INIT(prgname);				/* initialize program/script name */

  /* set prgname if PIOprgname is in the environment */
  if ((tmp_ptr = getenv("PIOprgname")) != NULL)
     (void)strncpy(prgname,tmp_ptr,NAMESIZE);

  if (parse(argc,argv,"d:n:q:t:v:T:A:","nqtv",0,&att,6,
        &dstype,&dname,&pqname,&ptype,&vpname,&tty_attached,&attach_t)) err_sub(ABORT,USAGE_MK);

  /* if no attachment type was specified make it 'file' */
  if (attach_t == NULL)
  {
       INIT(attach_t);               /* initialize attachment type */
       (void)strcpy(attach_t,"file");
  }

  if (strstr(dname,"/dev/")) strcpy(dname,dname+5); /* user enterd /dev/xxx */

  if (tty_attached )
      {
      INIT(prgname);          /* initialize program/script name */
      put_env_vars( "prgname", "piocustp" ); /* set prgname and PIOprgname */
      }

  if (!(p = found_pn(prt_head,ptype))) err_sub(ABORT,PTYPE);

  if ( !dstype )
      {
      INIT(dstype);               /* initialize data stream type */
      if (p->sn_head->sn_next) err_sub(ABORT,DSTYPE2);
      else strcpy(dstype,p->sn_head->sn);
      }
  else if ( !found_sn(&prt_head,ptype,dstype) ) err_sub(ABORT,DSTYPE);

                                                          /* abort if the */
  if ( !valid_queue(pqname) ) err_sub(ABORT,QUEUE);     /* queue and queue */
  if ( !valid_quedev(vpname) ) err_sub(ABORT,QUEDEV);   /* device aren't */
                                                          /* valid */
  sprintf(prefile,"%s%s.%s",prepath,ptype,dstype);
  sprintf(cusfile,"%s%s:%s",cuspath,pqname,vpname);

  /* If a virtual printer already exists, abort. */
  if (!stat(cusfile,&sbuf))
     err_sub(ABORT,VPEXISTS);

  umask(75);

  /* run piocnvt to copy the predef to custom */
  (void)sprintf(cmd,"%s -s+ -i%s -o%s",PIOCNVT,prefile,cusfile);
  if (system(cmd))
  {
	(void)unlink(cusfile);
	goodbye(TERMINATE);
  }
  
  /* run script to convert old colon files */
  (void)strcpy(attach_attr,UNIQUE_STR);
  (void)getval(PIO_CFLEVEL_ATTR,attach_attr,cusfile);	
  if (strcmp(attach_attr,UNIQUE_STR) == 0  /* if zL was not found */
      || strtol(attach_attr,NULL,10) < PIO_CFLEVEL_CONV)
  {
  	(void)sprintf(cmd,"%s -i %s",PIOUPDATE,cusfile);
  	if (system(cmd) != 0)
	{
		(void)unlink(cusfile);
		goodbye(TERMINATE);
	}
  }
  
  get_codeset(NULL);
  
  /* run chvirprt to update the custom file */
	(void)sprintf(cmd,"%s -q %s -d %s -a md=%s mn=%s mq=%s mt=%s mv=%s _X=%s zA=%s",
  		CHVIRPRT,pqname,vpname,dstype,dname,pqname,ptype,vpname,codeset,attach_t);
	if (system(cmd) != 0)
	{
		(void)unlink(cusfile);  /* clean up the cusfile before exiting */
		goodbye(TERMINATE);
	}

  /* If "prgname" variable was set to a program or script name,
     execute the specified program/script and update the custom
     colon file. */
  mkv_update_cusfl(prgname);

  exit(0);
}

/*=========================================================================*/

/* Format of Info. From Each Line of *.config Files in /usr/lib/lpd/pio/etc */
/* Fields are separated by colons */
struct listinfo {
                    /* M m l V v T # (x: used;  -: not used)
                       - - - - - - -                     */
   char type;       /* x x x x x x x  type of info.:
                                          'M': Header for list of menu choices
                                          'm': Menu choice
                                          'l': Pipeline to get menu choices
                                          'V': Prompt user for value
                                          'v': Validation pipeline & error msg.
                                          'T': Text 
                                          '#': Comment */
   char *label;     /* x - - x - x -  label identifying this statement */
   char *gotolabel; /* - x - x - x -  label of statement to go to
                                          (fall through if null string) */
   char *reserved;  /* - - - - - - -  (reserved for future use) */
   char *text;      /* x x - x - x -  text for "backend =" */
   char *variable;  /* - - - 1 - - -  (see footnote) */
   char *defval;    /* x - - x - - -  default input string */
   char *limits;    /* - - 3 - 2 - -  limits for user input */
   char *msgid;     /* - x - x x - -  CatalogName,SetNumber,MessageNumber */
   char *defmsg;    /* - x - x x - -  default message text */
   struct listinfo  /*                ptr to next listinfo element in chain */
     *next;
};
                 /* 1: Indicates type of value (optional):
                      "dname" - device name
                      "fname" - file= value
		      "prgname" - program/script name
                    2: Pipeline to validate value entered
                    3: Pipeline to get menu values
                 */

char fragname[] = ".config";
int fraglen = sizeof(fragname) - 1;

int catset;
char catname[100];

attach()
{ /* Determines and Processes Various Types of Printer Attachment */
  /* If not host attach, generates special "backend =" for qconfig file */
  /* return codes: 0 - host attach     */
  /*               1 - not host attach */

struct listinfo *listptr;	/* pointer to the head of the list */
int ch, fieldcnt, fieldlen;
int cnt, indx;
FILE *infile;
char wkbuf[300], local_str[1000], misc[20], *cp;
char *variables;    /* pointer to variables to put in environment */
char configpath[300];
char *text_str;
char *tmp_str; 
char *saved_text_str;
int  put_backend;
struct listinfo *listp, *listp2, *prev_listp;


*catname = '\0';   /* if msg catalog not specified, force default msg */
catset = -1;       /* if set number not specified, force default msg */

/* For Each *.config File, One Structure For Each Line in the File.  The     */
/* Chains of listinfo Structures Are Anchored by the listptr pointer  */

	/* build full path to .config file */
    strcpy(configpath, etcpath);
    sprintf(configpath,"%s%s.config",etcpath,attach_t);

	/* open .config file */
    if ((infile = fopen(configpath, "r")) == NULL)
        err_sub(ABORT, MSG_OPENFILE);

	prev_listp = NULL; 
    configline = 0;
    /* For Each Line In This .config File */
    for (;;)
    {
        if ((ch = getc(infile)) == EOF)
        goto ATEOF;
        else
        (void) ungetc(ch, infile);

        if ((ch = getc(infile)) == '#')  /* this line is a comment */
           {
           while (ch != '\n')
             ch = getc(infile);
           configline++;
           goto NEXTLINE;
           }
        else
           (void) ungetc(ch, infile);
            
        configline++;
        listp = (struct listinfo *) calloc(1, sizeof(struct listinfo));
        listp->type = '?';
		if (prev_listp == NULL)
			listptr = listp; 	/* this is the beginning of the linked list */
		else
			prev_listp->next = listp;	/* add new member to the list */

		prev_listp = listp;
		
		/* loop once for each field on the line */
        for (fieldcnt = 1, ch = 0;; fieldcnt++)
        {
        if (ch == '\n')
            break;
        cp = wkbuf;
        for (;;)
        {
            ch = getc(infile);
            if (ch == ':' && cp > wkbuf && *(cp-1) == '\\')
            {
            *(cp-1) = ':';
            continue;
            }
            if (ch == ':' && fieldcnt < NUMFIELDS)
            break;
            if (ch == '\n' || ch == EOF)
            if (fieldcnt == NUMFIELDS)
                break;
            else
                err_sub(ABORT, MSG_PIFSHORT);
            *cp++ = (char) ch;
        }
        fieldlen = cp - wkbuf;
        switch(fieldcnt)
        {
        case 1: /* Statement Type */
            if (fieldlen > 0)
            listp->type = *(cp - 1);
            break;
        case 2: /* Statement Label */
            listp->label = malloc(fieldlen + 1);
            strncpy(listp->label, wkbuf, fieldlen);
            break;
        case 3: /* Label to Go To When Statement is Finished */
            listp->gotolabel = malloc(fieldlen + 1);
            strncpy(listp->gotolabel, wkbuf, fieldlen);
            break;
        case 4: /* Reserved */
            listp->reserved = malloc(fieldlen + 1);
            strncpy(listp->reserved, wkbuf, fieldlen);
            break;
        case 5: /* Text */
            listp->text = malloc(fieldlen + 1);
            strncpy(listp->text, wkbuf, fieldlen);
            *(listp->text+fieldlen) = '\0';
            break;
        case 6: /* Miscellaneous Value */
            listp->variable = malloc(fieldlen + 1);
            strncpy(listp->variable, wkbuf, fieldlen);
            break;
        case 7: /* Default Value */
            listp->defval = malloc(fieldlen + 1);
            strncpy(listp->defval, wkbuf, fieldlen);
            break;
        case 8: /* limits */
            listp->limits = malloc(fieldlen + 1);
            strncpy(listp->limits, wkbuf, fieldlen);
            break;
        case 9: /* CatalogName,SetNumber,MessageNumber */
            listp->msgid = malloc(fieldlen + 1);
            strncpy(listp->msgid, wkbuf, fieldlen);
            break;
        case 10: /* Default Message Text */
            listp->defmsg = malloc(fieldlen + 1);
            strncpy(listp->defmsg, wkbuf, fieldlen);
            break;
        }
        } /*for*/
NEXTLINE: ;
    } /*for*/
ATEOF: ;
(void) fclose(infile);

listp = listptr;
strcpy(backend, listp->text);
strcpy(txtstring, getmessage(listp));
hilite(txtstring,FALSE);

/* Process Dialog Statments for the Selected Printer Interface */
*catname = '\0';   /* if msg catalog not specified, force default msg */
catset = -1;       /* if set number not specified, force default msg */
getmessage(listp);  /* set the default catname and catset */
listptr = listp->next;
for (listp = listptr; listp != NULL;)
{
    switch(listp->type)
    {
    case 'M':
        text_str = replace_text(listp->text,TRUE);
        strncat(backend, text_str, sizeof(backend) - strlen(backend) - 1); 
        if (text_str != (char *)NULL)
           free(text_str);
        variables = listp->variable;
        strcpy(txtstring, getmessage(listp));         /* for resp_menu() */
        cnt = 0;
        listp2 = listp;
        listp = listp->next;
        if (listp->type == 'l')
        {
           tmp_str = (char *)malloc(strlen(listp->limits) + 3);
           sprintf(tmp_str,"`%s`",listp->limits); 
                      /* get string of menu choices */
           text_str = replace_text(tmp_str,FALSE);
           tmp_str = text_str; 
           saved_text_str = text_str;
           while (text_str != NULL)
           { 
             text_str = strchr(tmp_str,'\n'); /* \n is menu choice delimiter */
             if (text_str != NULL)
                *text_str = '\0';
             if (*tmp_str != NULL)
              {   
                 (void) sprintf(local_str,
                   "     %-3d    %s", cnt+1, tmp_str);
                 scr_line[cnt] = (char *) malloc(strlen(local_str) + 1);
                 strcpy(scr_line[cnt++], local_str);
                 scr_line[cnt] = NULL;
              }
              if (text_str != NULL)
                 tmp_str = ++text_str;
           }
           *misc = '\0';
           cnt = resp_menu(GET_PR_NUM, -1, misc, GETNUM);
             /* put variables in environment  */
           put_backend = put_env_vars(variables,(char*)(scr_line[cnt-1] + 12));
           if (put_backend)   /* flag was set to TRUE */
               strcat(backend, (char *)(scr_line[cnt-1] + 12));

           listp = listp->next;   /* advance to next pointer */
           if (saved_text_str != (char *) NULL)
             free (saved_text_str);
        }
        else
        {
             for (; listp != NULL && listp->type == 'm';
                    listp = listp->next)
             {
             (void) sprintf(local_str,
               "     %-3d    %s  ", cnt+1, getmessage(listp));
             scr_line[cnt] = (char *) malloc(strlen(local_str) + 1);
             strcpy(scr_line[cnt++], local_str);
             scr_line[cnt] = NULL;
         }
         *misc = '\0';
         cnt = resp_menu(GET_PR_NUM, -1, misc, GETNUM);
         if (*(listp2->gotolabel)) /*  if there is a gotolabel, find target */
             for (listp = listptr;
               listp != NULL && strcmp(listp->label, listp2->gotolabel);
               listp = listp->next);
         for (ch = 0; ch < cnt; ch++)
             listp2 = listp2->next;

             /* put variables in environment  */
         text_str = replace_text(listp2->text,TRUE);
         put_backend = put_env_vars(variables,text_str);
         if (put_backend)   /* flag was set to TRUE */
            strncat(backend, text_str, sizeof(backend) - strlen(backend) - 1);

         if (text_str != (char *)NULL)
            free (text_str);

         if (*(listp2->gotolabel))  /* if there is a gotolabel find target */
             for (listp = listptr;
               listp != NULL && strcmp(listp->label, listp2->gotolabel);
               listp = listp->next);
        }
    break;

    case 'V':
        text_str = replace_text(listp->text,TRUE);
        strncat(backend, text_str, sizeof(backend) - strlen(backend) - 1);

        if (text_str != (char *)NULL)
           free (text_str);
        variables = listp->variable;
        (void) strcpy(txtstring, getmessage(listp));
        listp2 = listp;
    MSGLOOP:
        NULN;
        NULN;
        NULN;
        *local_str = '\0';
        (void) response(-1, local_str, "");

        put_backend = put_env_vars(variables,local_str);

      /* execute the pipeline for every 'v' statement  */
        for (listp = listp2->next; listp != NULL && listp->type == 'v';
           listp = listp->next)
        if ((rc = system(listp->limits)))
           {
           WRITE(1, getmessage(listp));
           goto MSGLOOP;
           }

        if (put_backend)   /* flag was set to TRUE */
           (void) strcat(backend, local_str);

      /* if a goto label exists, find statement to branch to */
        if (*(listp2->gotolabel))
          for (listp = listptr;
            listp != NULL && strcmp(listp->label, listp2->gotolabel);
            listp = listp->next);
        break;


    case 'T':
        text_str = replace_text(listp->text,TRUE);
        put_backend = put_env_vars(listp->variable,text_str);
        if (put_backend)   /* flag was set to TRUE */
           strncat(backend, text_str, sizeof(backend) - strlen(backend) - 1);

        if (text_str != (char *)NULL)
           free (text_str);
        listp2 = listp;
        if (*(listp2->gotolabel))
        for (listp = listptr;
             listp != NULL && strcmp(listp->label, listp2->gotolabel);
             listp = listp->next);
        else
          listp = listp->next;
        break;
   }
}

return(NOTHOSTATTACH);
}

/*=========================================================================*/
int put_env_vars( vars_ptr, text )
char *vars_ptr;
char *text;
{
char env_var[20];    /* contains name of environment variable */
char *environ_var;
char *variable_str;
char *ptr;
int flag = FALSE;
size_t	cplen;

        variable_str = (char *)malloc(300);
        ptr = variable_str;
        strcpy(variable_str,vars_ptr);
        while (ptr != NULL) {
              ptr = strchr(variable_str,',');   /* find next comma */
              if (ptr != NULL)
                 *ptr = '\0';
              if (*variable_str != NULL) {
                 sprintf(env_var,"PIO%s",variable_str);
                 environ_var = (char *)malloc(strlen(env_var)+strlen(text)+2); 
                 sprintf(environ_var,"%s=%s",env_var,text);
                 (void) putenv(environ_var);
             if (!strcmp(variable_str, "dname"))
                 (void) strcpy(dname, text);
             if (!strcmp(variable_str, "fname"))
                 (void) strcpy(fname, text);
             if (!strcmp(variable_str, "prgname"))
                 (void) strncpy(prgname, text, cplen = strlen(text) >
				NAMESIZE - 1 ? NAMESIZE - 1 : strlen(text)),
		 *(prgname + cplen) = 0;
              }
              else 
            flag = TRUE;  /* put local_str on backend */
  
              if (ptr != NULL)
                 variable_str = ++ptr;
        }
   return(flag);
}

/*=========================================================================*/
char *replace_text(ptr, flag)
char *ptr;
int flag;
{
  char *out_string = (char *)calloc(BUFLEN, sizeof(char));
  char *cmd_str = (char *)calloc(strlen(ptr)+1,sizeof(char));
  char tmp_file_nam[50];
  char *ptr2 = out_string;
  char *ptr3; 
  char c;
  int rc;
  FILE *fp;  /* file pointer */
  pid_t pid;
  int status;
  int char_cnt = 0; 
  int outbuf_size = BUFLEN;
  struct stat stbuf;

  ptr3 = cmd_str;
 
  while (*ptr != NULL)
     switch (*ptr) {
        case '`': /* start of shell command */
           ptr++;  
           while ((*ptr != '`') && (*ptr != NULL)) 
           {
           if (*ptr == '\\')  /* backslash is protector character */
              ptr++;
           *ptr3++ = *ptr++;   
           }
           if (*ptr++ == '`')
           {
              *ptr3 = '\0';  /* terminate the shell command string */

              tmpnam(tmp_file_nam);  /* get temp file name for stdout */
            
              fflush(stdout);   /* flush stdout before forking  */

              pid = fork();
              if (pid == 0)   /* this path for the child only */
              {  /* redirect stdout to temp file */
                 fp = freopen(tmp_file_nam,"w+",stdout);
                 rc = system(cmd_str);
                 fclose(fp);
                 exit(0);
              }
              else if (pid == -1)
                      return("");
              else
                  pid = wait(&status);

              if ((fp = fopen(tmp_file_nam,"r")))  
              {
                 stat(tmp_file_nam, &stbuf);
                 if ((char_cnt + stbuf.st_size+1) >= outbuf_size)
                 {
                   out_string = (char *) realloc(out_string, char_cnt+stbuf.st_size+1);
                   outbuf_size = char_cnt+stbuf.st_size+1;	
                   ptr2 = out_string + char_cnt;
                 }

                 while((c = getc(fp)) != (char)EOF)  /* read the entire file */
                 {
                    if ((c == '\n') && (flag == TRUE))
                       c = ' ';
                    *ptr2++ = c;
                 }
                 char_cnt += stbuf.st_size;
                 
                 unlink(tmp_file_nam);
              }
           }
              /* reset ptr3 */
           ptr3 = cmd_str;
           break;
        
        default:
           if ((char_cnt+1) >= outbuf_size)
           {
               out_string = (char *) realloc(out_string, outbuf_size+BUFLEN);
               outbuf_size += BUFLEN;
               ptr2 = out_string + char_cnt; 
           }
           *ptr2++ = *ptr++;
           char_cnt++;
           break;
     }
     *ptr2 = '\0';
     free(cmd_str);
     return(out_string);
}

/*=========================================================================*/
#define MBCS_TBL     "/usr/lib/lpd/pio/etc/mbcs.tbl"

char *get_codeset(ptr)
char *ptr;     /* pointer to the current locale setting */
/* sets the global variable codeset to the system codeset value */
{
   static char *save_locale;
   static char *env_lang = (char *)NULL;
   FILE *file;
   char line[LINE_MAX];  
   char *ptr2 = (char *)NULL;

#ifndef PIOTEST
   FILE *filembcs;
   char linembcs[LINE_MAX];  
   int i;

   if (ptr){
   save_locale = (char *)malloc(strlen(ptr) + 1);
   strcpy(save_locale,ptr);  /* save current locale setting */

   file = fopen(ENVIRONMENT,"r");  /* open /etc/environment */
 
   if (file == NULL) {
      strcpy(configpath,ENVIRONMENT);  /* kludge for err_sub routine */
      err_sub(ABORT,MSG_OPENFILE);   /* can open /etc/environment so quit */
   }

   while(!fgetln(file,line) ) {
      register char *cp = line;

      while(isspace(*cp))               /* ignore commented lines */
         cp++;
      if (*cp == '#')
         continue;

       ptr2 = strstr(line,"LANG");
       if (ptr2 != NULL) 
          break;
   }

   if (ptr2 != NULL) {
      ptr2 = strchr(line,'=');
      ptr2++;    /* ptr2 now points to the value of LANG */
         env_lang = (char *)malloc(strlen(ptr2) + 1);
         strcpy(env_lang,ptr2);  /* save LANG */
      }
   }

   if (env_lang && dstype){
       char mbenv1[LINE_MAX], mbenv2[LINE_MAX], mbstrm[LINE_MAX];

       ptr2 = env_lang;
       filembcs = fopen( MBCS_TBL, "r" );
      if (filembcs == NULL) {
         /* works without MBCS_TBL file */
         /* strcpy(configpath,MBCS_TBL); */ /* kludge for err_sub routine */
         /* err_sub(ABORT,MSG_OPENFILE); */ /* cant open mbcs.tbl so quit */
      }else{
         while( !fgetln(filembcs,linembcs) ){
            if( linembcs[0] == '#' ) continue;
             sscanf(linembcs, "%s%s%[^\n]", mbenv1, mbenv2, mbstrm);
	     if(!strcmp(mbenv1, ptr2) && mbenv2[0]){
	        if (strstr(mbstrm, dstype)){
		   ptr2 = mbenv2;
                  break;
               }
            }
         }
         fclose( filembcs );
      }

      (void)setlocale(LC_CTYPE,ptr2); 
      codeset = nl_langinfo(CODESET);
   }

   (void)setlocale(LC_ALL,save_locale);   /* restore locale settting */
#else
   codeset = (char *)malloc(10);
   strcpy(codeset,"IBM-850");
#endif

   return;
}
/*=========================================================================*/
char *getmessage(listptr)
struct listinfo *listptr;
{
int catmsg;
char *cp1 = (char *)malloc(300);
char *cp2;

catmsg = -1;       /* if msg number not specified, force default msg */
strcpy(cp1 , listptr->msgid);
if (*cp1)
{
    if (cp2 = strchr(cp1, ','))
    {
    if (cp2 > cp1)
    {
        *cp2 = '\0';
        (void) strcpy(catname, cp1);
    }
    cp1 = cp2 + 1;
    if (cp2 = strchr(cp1, ','))
    {
        if (cp2 > cp1)
        {
        *cp2 = '\0';
        catset = atoi(cp1);
        }
        cp1 = cp2 + 1;
        if (*cp1)
        catmsg = atoi(cp1);
    }
    }
}
cp1 = retrieve_msg(catname, catset, catmsg, listptr->defmsg);
return(cp1);
}

/*
*******************************************************************************
** NAME:        retrieve_msg()
**
** DESCRIPTION: The same routine as those used through the rest of cmdpios
**              except this allows for default messages being passed by the 
**              calling routine. 
**
*******************************************************************************
*/
static char *msgbuf = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

char *retrieve_msg(CatName, set, num, defmsg)
char *CatName;  
int set;
int num;
char *defmsg;
{
	char *ptr;
	char *nlspath;
	char *defpath = malloc(200);

	if (strcmp(CatName,save_name) != 0)  /* is it a different catalog */
	{
	catclose(catd);  /* close /usr/lpp message catalog */
	catclose(def_catd);  /* close default message catalog */
	catd = def_catd = (nl_catd)NULL;  /* set catalog descriptors to NULL */
	strcpy(save_name,CatName);
	}

	if (catd != -1)  
		if (catd == 0) /* if it hasn't been open before */
			catd = catopen(CatName,NL_CAT_LOCALE);
	
	if (catd != -1)
		{
		ptr = catgets(catd,set,num,"dummy");
		if (!msgbuf)
			msgbuf = malloc(4001);
		if (msgbuf)
			strncpy(msgbuf, ptr, 4000);
		if (strcmp(ptr,"dummy") != 0)  /* did catgets fail? */
			return(msgbuf);
		}
	
	if (def_catd == 0)  /* if default catalog hasn't been opened yet */
		{
		strcpy(defpath, ETCPATH);
		strcat(defpath, CatName);
		def_catd = catopen(defpath,NL_CAT_LOCALE);
		}

    ptr = catgets(def_catd,set,num, defmsg);

	if (!msgbuf)
		msgbuf = malloc(4001);
	if (msgbuf)
		strncpy(msgbuf, ptr, 4000);

	free(defpath);
	return(msgbuf);
}

/*
*******************************************************************************
** NAME:        mkv_update_cusfl
**
** DESCRIPTION: The function mkv_update_cusfl updates the custom file with 
**              the program/script specified by "PIOprgname" environment
**              variable.  Additionally, after updating, it expands the custom
**		colon file, digests the file, and finally
**		restores the file to its default state using "piocnvt".
**
** INPUT:	const char *prognm
** OUTPUT:	(none)
** RETURNS:	(none)
*******************************************************************************
*/
static void
mkv_update_cusfl(const char *prognm)
{
   char		cmdstr[1000];
   int		cmdstatus;

   if (!prognm || !*prognm)		/* If NULL or a null string, return */
      return;

   /* Run the program/script specified by "prgname" variable in device
      configuration file.  If failure, display the error message and skip
      the rest of the commands.  But need not exit. */
   (void) sprintf(cmdstr, "%s%s -i %s", *prognm != '/' ? etcpath : "",
		  prognm, cusfile);
   if ((cmdstatus = system(cmdstr)) & 0xff  ||
       cmdstatus >> 8 & 0xff)			/* if command failed */
   {
      /* The error message is supposedly displayed by the program/script that
	 was run.  Hence just cleanup and exit.
       */
      (void)unlink(cusfile);
      goodbye(TERMINATE);
   }

   /* Expand, digest and restore the colon file. */
   (void) sprintf(cmdstr, "%s -s+ -i%s;" "%s -q %s -v %s %s;" "%s -i%s",
		  PIOCNVT, cusfile,
		  digestcmd, pqname, vpname, cusfile,
		  PIOCNVT, cusfile);
   (void) system(cmdstr);

   return;
}

