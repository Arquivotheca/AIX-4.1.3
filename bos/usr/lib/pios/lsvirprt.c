static char sccsid[] = "@(#)12  1.16.1.6  src/bos/usr/lib/pios/lsvirprt.c, cmdpios, bos411, 9438C411a 9/24/94 10:39:04";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
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

/*** lsvirprt.c ***/

#include <sys/types.h>
#include <sys/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <sys/lockf.h>
#include <fcntl.h>
#include <sys/mode.h>
#include <signal.h>
#include <setjmp.h>
#include <regex.h>
#include <mbstr.h>
#include <wcstr.h>
#include "virprt.h"

#define COL2W 50
#define COL3W 18
#define NROWS 57
#define NCOLS (COL2W+1)
#define WDESCSZ 1000
#define SEP   "   "

#define QUIT      1
#define SHOW_ALL  2
#define CHANGE    3
#define EDIT      4
#define FILE_EDIT 5
#define REGANMLEN	2
#define FLAGCHR		'_'
#define WHSPSTR		"\x20"
#define MAXCLNFLDLEN	1000
#define SECTNMLEN	5
#define NULLSTR		""
#define MAXINTSTRLEN	11
#define DIGITCHRS	"0123456789."
#define WKBUFLEN	1023

#define NOELEMS(a)	(sizeof(a)/sizeof(*(a)))
#define OPTLIST		":q:d:f:a:s:inD"
#define DS_NOATTRS	(2)		/* 1 for "i." */
#define DS_DEFDSATTR	"_d"
#define DS_GENIPATTR	"i."
#define DS_DSPFMTSTR	"%7$c %6$s\n"
#define MALLOC(p,sz)		do \
				{ \
				   if (!((p) = malloc((size_t)(sz)))) \
				      err_sub(ABORT,MSG_MALLOC); \
				} while (0)
#define REALLOC(p,op,sz)	do \
				{ \
				   if (!((p) = realloc((void *)(op), \
						       (size_t)(sz)))) \
				      err_sub(ABORT,MSG_MALLOC); \
				} while (0)
#define FILLATTR(ap,cp,v,f)	do \
				{ \
				   if (*((cp)+1)) \
				      (ap)->attribute = (cp); \
				   else \
				   { \
				      MALLOC((ap)->attribute,REGANMLEN+1); \
				      *(ap)->attribute = FLAGCHR; \
				      *((ap)->attribute+1) = *(cp); \
				      *((ap)->attribute+2) = 0; \
				   } \
				   (ap)->value = (v); \
				   (ap)->flag = (f); \
				} while (0)


extern char defcatalog[];

typedef struct {
		    struct attr		*ap;
		    regex_t		are;
		    uchar_t		reflag;
		 } rattr_t;
typedef struct {
		    char		anm[REGANMLEN];
		    char		*dspmsg;
		    uchar_t		dspd;
	       } dspinfo_t;

void			interactive(void);
void			non_interactive(void);
static void		lv_catch_sigpipe(int);
static int		attr_found(rattr_t *,char const *);
static int		attr_not_found(const rattr_t *,int);
static int		splitws(wchar_t *,int,wchar_t **,int);
static char const	*fmtstr;
static int		sigpipe;
static sigjmp_buf	jenv;
static uchar_t		sectnmflg;
static uchar_t		dsflg;
static uchar_t		nnatflg;

/*===========================================================================*/
int
main(int argc,char **argv)
{
    register int		i;
    int				qflag 	= 0;
    int				dflag 	= 0;
    int				fflag 	= 0;
    int				aflag 	= 0;
    int				sflag 	= 0;
    int				iflag 	= 0;
    int				nflag 	= 0;
    int				Dflag 	= 0;

    (void) setlocale(LC_ALL, "");

    /* Set I/O streams to unbuffered mode. */
    (void)setvbuf(stdout,NULL,_IONBF,0);
    (void)setvbuf(stderr,NULL,_IONBF,0);

    make_files();                 /* create full path names from PIOBASEDIR */

    /* Parse the arguments.  If no arguments or if -i flag was specified,
       invoke interactive().  Else, invoke non_interactive(). */
    if (argc == 1)
       interactive();
    else
    {
       for (opterr = 0, optind = 1;
	    (i = getopt((int)argc,argv,OPTLIST)) != EOF; )
          switch (i)
          {
	     case 'q':
		pqname = optarg;
		qflag++;
		break;
	     case 'd':
		vpname = optarg;
		dflag++;
		break;
	     case 'f':
		fmtstr = optarg;
		fflag++;
		break;
	     case 'a':
		aflag++;
		break;
	     case 's':
		sflag++;
		break;
	     case 'i':
		iflag++;
		break;
	     case 'n':
		nflag++;
		break;
	     case 'D':
		Dflag++;
		break;
	     case ':':
	     case '?':
		err_sub(ABORT,USAGE_LS);
	  }
       if (!qflag  ||  !dflag  ||  aflag && sflag  ||  iflag && Dflag  ||
	   (iflag || Dflag) && (fflag || nflag || aflag || sflag))
	  err_sub(ABORT,USAGE_LS);
       if (aflag  ||  sflag)
       {
	  register struct attr		*ap;

	  sectnmflg = sflag;
	  MALLOC(att,((aflag|sflag)+1)*sizeof(*att));
	  (ap = att+(aflag|sflag))->attribute = NULL;
	  ap->value = NULL;
	  ap->flag = 0;
          for (ap = att, opterr = 0, optind = 1;
	       (i = getopt((int)argc,argv,OPTLIST)) != EOF; )
             switch (i)
             {
	        case 'a':
	        case 's':
		   FILLATTR(ap,optarg,NULL,0);
		   ap++;
		   break;
	     }
       }
       if (Dflag)
       {
	  register struct attr		*ap;

	  MALLOC(att,DS_NOATTRS*sizeof(*att));
	  (ap = att+DS_NOATTRS-1)->attribute = NULL;
	  ap->value = NULL;
	  ap->flag = 0;
	  ap = att;
	  FILLATTR(ap,DS_GENIPATTR,NULL,0);
	  fmtstr = DS_DSPFMTSTR;
       }
       nnatflg = nflag; dsflg = Dflag;
       (iflag?interactive:non_interactive)();
    }
    goodbye(GOOD);

    /* Not reached */
    return EXIT_SUCCESS;
}

/*===========================================================================*/
void
interactive(void)
{
    struct pn_struct *p;
    struct sn_struct *s;
    struct sn_struct *not_unique;
    int cnt, action, cntnue = TRUE;
    int chng_perm;  /* is changing the custom file permitted by this user */
    int edit_code;  /* for pioattred */
    int fd;         /* file descriptor used to lock custom colon file */
    int first_char; 
    char *file_name;
    char tmp[5];
    char tmpbuf[1024];
    char buf[1024],
    long_cmd[1536],
    misc[10] = "",
    *ptr, *ptr1, *ptr2, *env_ptr;
    char *buffer = (char *)buf;

    ISATTY;                  /* quit if not on a tty port */

    /* Initialize qname and dname only if they were not specified in the cmd
       line args. */
    if (!pqname)
    {
       INIT(pqname);                 /* initialize print queue name */
       INIT(vpname);                 /* initialize virtual printer name */
    }
    INIT(custom_name);            /* initialize custom colon file name */

    if ( geteuid() ) chng_perm = FALSE;   /* check if user is superuser  */
    else chng_perm = TRUE;

    /* If qname and dname were not already specified in the command line
       arguments, display a list of virtual printers, and accept user's
       response. */
    scrn_ht = start_cur();
    if (!*pqname)
    {
       prt_head = make_list(cuspath,':');
       make_qlist(prt_head);
       cnt = resp_menu(GET_PR_NUM,VIRPRT_HDG,misc,GETNUM) - 1;
       get_pr( printers[ cnt ] , 2 );
       hilite(scr_line[cnt],TRUE);
    }

    do { TEXT(str,chng_perm ? LS_CH_ATTRS2 : LS_ATTRS2);
        strcat(str,"  ");
        WRITE(1,str);
        action = FALSE;
        readkbd(buffer,FALSE,0,0);
        NULN;

        if ( !(*buffer) ) action = QUIT;     /* the buffer is empty - quit */
           else   
             for(; *buffer == ' '; buffer++);  /* strip off leading blanks */

        if (( *buffer == '*' ) && (strlen(buffer) == 1)) action = SHOW_ALL;

        /* look thru the entered string to see if */
        /* characters specially interpreted by the shell were entered */
        ptr1 = cmd;
        *ptr1 = NULL;
        ptr = buffer;
        while ( *ptr )
            {
            *misc = NULL;
            if ( *ptr == '\'') sprintf(misc,"\\0%o",'\'');
            if ( *ptr == '\"') sprintf(misc,"\\0%o",'\"');
            if ( *ptr == ':')  sprintf(misc,"\\0%o",':');
            if ( *ptr == '`')  sprintf(misc,"\\0%o",'`');
            if ( *misc )
                {
                strcat(ptr1,misc);
                while ( *ptr1++ );
                ptr1--;
                ptr++;
                }
            else
                {
                *ptr1++ = *ptr++;
                *ptr1 = NULL;
                }
            }
        strcpy(buffer,cmd);

        ptr = strchr(buffer,'=');
        if ( ptr && ((int)(ptr-buffer) == 1 || (int)(ptr-buffer) == 2) )
            action = CHANGE;

        if (strcmp(buffer,"~v") == 0)  /* vi the whole colon file */
            action = FILE_EDIT;

        ptr2 = strchr(buffer,'~');  /* ~V means edit with pioattred  */
        if ((*(ptr2 + 1) == 'v')  &&  ((int)(ptr2-buffer) == 1 ||
                    (int)(ptr2-buffer) == 2) )
        { 
        *ptr2 = NULL;
        if ((int)(ptr2-buffer) == 1 )   /* prepend '_' to 1 char attr names  */
           {
           sprintf(tmp,"_%s",buffer); 
           strcpy(buffer,tmp);
           }
        if (strcmp(buffer,"__"))   /* don't accept "__" as an attr  */
            action = EDIT;
        }

        switch( action )
            {
            case QUIT:
                cntnue = FALSE;
                continue;

            case SHOW_ALL:
	    {
		FILE 			*pp = popen(PG, "w");
		int			savfd;
		struct attr		*savap;

		if (!pp)
		{
		   (void) strncpy(cmd,PG,sizeof(cmd)-1),
		   *(cmd+sizeof(cmd)-1) = 0;
		   err_sub(ABORT,MSG_POPENERR);
		}
		(void) fflush(stdout);
		savfd = dup(fileno(stdout));
		(void) fflush(pp);
		(void) dup2(fileno(pp),fileno(stdout));
		savap = att, att = NULL;
		sigpipe = 1;
		non_interactive();
		sigpipe = 0;
		att = savap;
		(void) fflush(stdout);
		(void) dup2(savfd,fileno(stdout));
		(void) pclose(pp);
		(void) close(savfd);
                break;
	    }

            case CHANGE:
	    {
		struct attr		aa[] = { { buffer, NULL, 0 },
						 { NULL, NULL, 0 } };
		struct attr		*savap;

                if ( !chng_perm )
                    {
                    TEXT(str,CANT_CHNG);
                    WRITE(2,str);
                    continue;
                    }

                sprintf(long_cmd, "%s -q %s -d %s -a '%s'",
                    CHVIRPRT,pqname, vpname, buffer);
                system(long_cmd);
                *ptr = NULL;

		if (!*(buffer+1))
		{
		   MALLOC(aa[0].attribute,REGANMLEN+1);
		   *aa[0].attribute = FLAGCHR;
		   *(aa[0].attribute+1) = *buffer;
		   *(aa[0].attribute+2) = 0;
		}
		savap = att; att = aa;
		non_interactive();
		att = savap;
                break;
	    }

            case EDIT:
                if ( !chng_perm )   /* check if user is root  */
                { 
                  TEXT(str,CANT_CHNG);
                  WRITE(2,str);
                  continue;
                }

                sprintf(cusfile,"%s%s:%s",cuspath,pqname,vpname);
                switch( file_stat(cusfile) )
                   {
                   case DZNTXST:  /* doesn't exist */
                                err_sub(ABORT,CUS_NOXST);
                   case PERM_OK:  /* exists */
                                break;
                   case PERM_BAD: /* can't open */
                                err_sub(ABORT,CUS_NOPEN);
                   }
                fd = open(cusfile,O_RDONLY);
                if (lockf(fd,F_TEST,0) == -1)
                {   /* file is locked by somebody else */
                   sprintf(custom_name,"%s:%s",pqname,vpname);
                   err_sub(ABORT,MSG_FILELOCK); 
                }
                else
                  lockf(fd,F_LOCK,0);

                putenv("PIOLSVIRPRT=true");
                edit_code = 1;
                sprintf(long_cmd, "%s -q %s -d %s -o %d -a %s",
                         PIOATTRED,pqname,vpname,edit_code,buffer);
                while (system(long_cmd) >> 8)  /* repeat if they messed up */
                 {
                 lockf(fd,F_ULOCK,0);
                 if (yes_no(MSG_ATTRED,TRUE) == TRUE)
                    {
                    edit_code = 2;
                    lockf(fd,F_LOCK,0);
                    sprintf(long_cmd, "%s -q %s -d %s -o %d -a %s",
                           PIOATTRED,pqname,vpname,edit_code,buffer);
                    }
                 else
                    {
                    edit_code = 4;
                    sprintf(long_cmd, "%s -q %s -d %s -o %d -a %s",
                            PIOATTRED,pqname,vpname,edit_code,buffer);
                    system(long_cmd);
                    break;
                    }
                 } 
                lockf(fd,F_ULOCK,0);
                close(fd);
                break;

            case FILE_EDIT :
                 if ( !chng_perm )   /* check if user is root  */
                 {
                    TEXT(str,CANT_CHNG);
                    WRITE(2,str);
                    continue;
                 }

                 sprintf(cusfile,"%s%s:%s",cuspath,pqname,vpname);
                 switch( file_stat(cusfile) )
                   {
                   case DZNTXST: err_sub(ABORT,CUS_NOXST);  /* doesn't exist */
                   case PERM_OK: break;                     /* exists */
                   case PERM_BAD: err_sub(ABORT,CUS_NOPEN); /* can't open */
                   }
                 fd = open(cusfile,O_RDONLY);
                 if (lockf(fd,F_TEST,0) == -1)
                    {   /* file is locked by somebody else */
                    sprintf(custom_name,"%s:%s",pqname,vpname);
                    err_sub(ABORT,MSG_FILELOCK);
                    }
                    else
                      lockf(fd,F_LOCK,0);

                 if (!(env_ptr = (char *)getenv("VISUAL")))
		    env_ptr = getenv("EDITOR");
                 sprintf(long_cmd,"%s %s",env_ptr ? env_ptr : VI ,cusfile);
                 rc = system(long_cmd);
                 lockf(fd,F_ULOCK,0);  /* close custom file */
                 close(fd);

                 sprintf(long_cmd,"%s/piodigest -q%s -d%s %s",
                           etcpath, pqname, vpname, cusfile);
                 rc = system(long_cmd);
                 break;

            default:
                 if (*buffer)  /* if buffer is not null */
                 {
		    register char		*cp;

		    if (cp = strtok(buffer,WHSPSTR))
		    {
		       struct attr		*savap;
		       struct attr		*ap = NULL;
		       register struct attr	*tap;
		       register size_t		acnt = 0;

		       MALLOC(ap,sizeof(*ap));
		       FILLATTR(ap,cp,NULL,0);
		       acnt++;
		       for (; cp = strtok((char *)NULL,WHSPSTR); )
		       {
		          REALLOC(ap,ap,++acnt*sizeof(*ap));
		          tap = ap+acnt-1;
		          FILLATTR(tap,cp,NULL,0);
		       }
		       REALLOC(ap,ap,(acnt+1)*sizeof(*ap));
		       (tap = ap+acnt)->attribute = NULL;
		       tap->value = NULL;
		       tap->flag = 0;
		       savap = att; att = ap;
		       non_interactive();
		       att = savap;
		       free((void *)ap);
		    }
                }
                break;
            }

        NULN;
        } while ( cntnue );

   return;
}

/*===========================================================================*/
void
non_interactive(void)
{
    char attname[6];
    FILE *file;
    char line[LINE_MAX], msg[100], catID[100], catalog[100],
         desc[1000], value[1000], limit[MAXCLNFLDLEN+1];
    int  msgnum, lines, i, c2, c3, hdng = FALSE;
    char col2[NROWS][NCOLS], col3[NROWS][NCOLS], format[256];
    struct sigaction		newspipe;
    struct sigaction		oldspipe;
    rattr_t			*rap;
    register struct attr	*tattp = att;
    register int		acnt = 0;
    register rattr_t		*tap;
    dspinfo_t			*dip	= NULL;
    register dspinfo_t		*tdip;
    uint_t			dicnt	= 0;
    char			defds[LINE_MAX+1];
    int				mb_cur_max	= MB_CUR_MAX;
    wchar_t			*wdesc		= NULL;
    wchar_t			**wcol2		= NULL;

    if (sigpipe)
    {
       if (sigsetjmp(jenv,1))
	  goto ENDFUNC;		/* unaesthetic but appropriate here */
       newspipe.sa_handler = lv_catch_sigpipe;
       (void) sigemptyset(&newspipe.sa_mask);
       newspipe.sa_flags = 0;
       (void) sigaction(SIGPIPE,&newspipe,&oldspipe);
    }

    if (mb_cur_max > 1)
    {
       MALLOC(wcol2,NROWS*sizeof(*wcol2));
       MALLOC(*wcol2,NROWS*NCOLS*sizeof(**wcol2));
       for (i = 1; i < NROWS; i++)
	  *(wcol2+i) = *wcol2+i*NCOLS;
       MALLOC(wdesc,WDESCSZ*sizeof(*wdesc));
    }

    sprintf(cusfile,"%s%s:%s",cuspath,pqname,vpname);
    switch( file_stat(cusfile) )
        {
        case DZNTXST: err_sub(ABORT,CUS_NOXST);         /* doesn't exist */
        case PERM_OK: break;                    /* exists */
        case PERM_BAD: err_sub(ABORT,CUS_NOPEN);        /* can't open */
        }

    umask(75);
    sprintf(cmd,"%s -s+ -i%s -o%s",PIOCNVT,cusfile,tempfile);
    system(cmd);

    getval("mD", defcatalog, tempfile);        /* default message catalog */
    if (dsflg)
       getval(DS_DEFDSATTR, defds, tempfile);     /* default datastream */

    OPENTEMP;

    /* Compile attribute names (assuming they are regular expressions)
       and store them in an allocated array. */
    do
    {
       acnt++;
    } while (tattp++->attribute);
    MALLOC(rap,acnt*sizeof(*rap));
    tattp = att, tap = rap;
    do
    {
       tap->ap = tattp;
       tap->reflag = !sectnmflg  &&  (tap->ap->attribute ?
		     		      !regcomp(&tap->are,tap->ap->attribute,
			      		       REG_EXTENDED|REG_NOSUB) : FALSE);
    } while (tap++, tattp++->attribute);

    *(limit+sizeof(limit)-1) = 0;
    while( !fgetln(file,line) )
        {
        strcpy(attname,getf(line,2));
        if ( !(rap->ap->attribute) || attr_found(rap,attname) )
            {
	    /* print the heading if it hasn't already been done */
            if ( !hdng && !fmtstr ) hdng = heading();

            strcpy(catID,getf(line,0));
            strcpy(msg,getf(line,1));
            sscanf(msg,"%d",&msgnum);

            if (*catID)
                {
                if (*(catID+1)) strcpy(catalog, catID);
                else sprintf(catalog, "pioattr%s.cat", catID);
                }
            else strcpy(catalog, defcatalog);

            strcpy(value,getf(line,4));
	    if (nnatflg && !*value)
	       continue;
            strcpy(desc,get_attr_msg(catalog,ATTSETNUM,msgnum));

	    (void) strncpy(limit,getf(line,3),sizeof(limit)-1);

	    if (fmtstr)
	    {
	       char			*acp[] =
						{ NULLSTR, NULLSTR, NULLSTR,
						  NULLSTR, NULLSTR, NULLSTR,
						  NULLSTR };
	       char			wkbuf[LINE_MAX+1];
	       char			is[MAXINTSTRLEN+1];
	       char			ac[REGANMLEN+1];
	       char			fs[WKBUFLEN+1];
	       const char		*ccp;
	       register char		*cp;
	       register char		*cp1;
	       register char		*wp;
	       register char		**cpp = acp;
	       int			val;
	       register int		bksh = FALSE;

	       (void) strncpy(fs, fmtstr, sizeof(fs)-1),
	       *(fs+sizeof(fs)-1) = 0;
	       for (cp = fs, wp = wkbuf;
		    *cp  &&  wp-wkbuf < sizeof(wkbuf); cp++, wp++)
		  if ((*wp = *cp) == '%'  &&  *(cp+1)  &&  *(cp+2) == '$')
		  {
		     bksh = FALSE;
		     if ((val = *(cp+1)-'0') < 1  ||  val > NOELEMS(acp))
			continue;
		     cp1 = cp+3+strspn(cp+3,DIGITCHRS);
		     switch (*cp1)
		     {
			case 'd':
			   (void) sprintf(is,"%d",msgnum);
			   ccp = is;
			   *cp1 = 's';
			   break;

			case 'c':
			   *ac = *(attname+1),
			   *(ac+1) = 0;
			   *cp1 = 's';
			   ccp = ac;
			   break;

			case 's':
			   switch (val)
			   {
			      case 1:
				 ccp = catalog;
				 break;
			      case 2:
				 ccp = is;
				 break;
			      case 3:
				 ccp = attname;
				 break;
			      case 4:
				 ccp = limit;
				 break;
			      case 5:
				 ccp = value;
				 break;
			      case 6:
				 ccp = desc;
				 break;
			      case 7:
				 ccp = ac;
				 break;
			   }
			   break;
		     }
		     *cpp++ = (char *)ccp;
		     cp += 2;
		  }
		  else if (bksh)
		  {
		     bksh = FALSE;
		     switch (*wp)
		     {
			case '\\':
			   --wp;
			   break;
			case 'a':
			   *--wp = '\a';
			   break;
			case 'b':
			   *--wp = '\b';
			   break;
			case 'f':
			   *--wp = '\f';
			   break;
			case 'n':
			   *--wp = '\n';
			   break;
			case 'r':
			   *--wp = '\r';
			   break;
			case 't':
			   *--wp = '\t';
			   break;
			case 'v':
			   *--wp = '\v';
			   break;
			case '\?':
			   *--wp = '\n';
			   break;
			case '\'':
			   *--wp = '\'';
			   break;
			case '\"':
			   *--wp = '\"';
			   break;
		     }
		  }
		  else
		     bksh = *wp == '\\' ? TRUE : FALSE;
	       *wp = 0;
	       if (dsflg)
	       {
		  char		tbuf[LINE_MAX+1];

		  (void)sprintf(tbuf,wkbuf,acp[0],acp[1],acp[2],acp[3],acp[4],
				acp[5],acp[6]);
		  REALLOC(dip,dip,++dicnt*sizeof(*dip));
		  tdip = dip+dicnt-1;
		  (void)memcpy(tdip->anm,attname,sizeof(tdip->anm));
		  MALLOC(tdip->dspmsg,strlen(tbuf)+1);
		  (void)strcpy(tdip->dspmsg,tbuf);
		  tdip->dspd = FALSE;
	       }
	       else
	          (void) printf(wkbuf,acp[0],acp[1],acp[2],acp[3],acp[4],acp[5],
			     acp[6]);
	    }
	    else
	    {
	       uchar_t		mbflg;

	       if (mb_cur_max > 1 &&
		   mbstowcs(wdesc,desc,WDESCSZ-1) != -1)
	       {
		  *(wdesc+WDESCSZ-1) = 0;
                  c2 = splitws(wdesc,COL2W,wcol2,1);
                  (void)sprintf(format,"%%-5s%%s%%-%dS%%s%%-%ds\n",COL2W,COL3W);
		  mbflg = 1;
	       }
	       else
	       {
                  c2 = split(desc,COL2W,col2,1);
                  (void)sprintf(format,"%%-5s%%s%%-%ds%%s%%-%ds\n",COL2W,COL3W);
		  mbflg = 0;
	       }
               c3 = split(value,COL3W,col3,0);
               lines = c2>c3 ? c2 : c3;
	       if (mbflg)
                  for ( i=0 ; i<lines ; i++ )
                  printf(format,!i ? attname : "  ",SEP,*(wcol2+i),SEP,col3[i]);
	       else
                  for ( i=0 ; i<lines ; i++ )
                     printf(format,!i ? attname : "  ",SEP,col2[i],SEP,col3[i]);
	    }
            }
        }

    if (dsflg)
    {
       for (tdip = dip; tdip < dip+dicnt; tdip++)
	  if (*(tdip->anm+1) == *defds)
	  {
	     (void)printf("%s",tdip->dspmsg);
	     tdip->dspd++;
	     break;
	  }
       for (tdip = dip; tdip < dip+dicnt; tdip++)
	  if (!tdip->dspd)
	     (void)printf("%s",tdip->dspmsg),
	     tdip->dspd++;
       (void)fflush(stdout);
       for (tdip = dip; tdip < dip+dicnt; tdip++)
	  free((void *)tdip->dspmsg);
       free((void *)dip);
    }

    ENDFUNC:
    if (sigpipe)
       (void) sigaction(SIGPIPE,&oldspipe,(struct sigaction *)NULL);

    CLOSETEMP;
    KILLTEMP;

    attr_not_found(rap,hdng);         /* display any attributes not found */

    /* Free the temporary memory allocated for regular expressions. */
    for (tap = rap; tap->ap->attribute; tap++)
       if (tap->reflag)
	  regfree(&tap->are);
    free((void *)rap);

    if (mb_cur_max > 1)
    {
       if (wdesc)
	  free((void *)wdesc);
       if (wcol2)
       {
	  if (*wcol2)
	     free((void *)*wcol2);
	  free((void *)wcol2);
       }
    }

    return;
}

/*===========================================================================*/
int
heading(void)
{
    char local_str[200];

    sprintf(local_str,"%s\n",getmsg(MF_PIOBE,ERRSETNUM,LS_HEADER));
    WRITE(1,local_str);
    return TRUE;
}

/*===========================================================================*/
int
attr_found(rattr_t *list_ptr,const char *name)
{
    register rattr_t *a;
    int rc = FALSE;
    static char		lastsect[SECTNMLEN+1];

    if (sectnmflg)
    {
       if (*name == FLAGCHR  &&  *(name+1) == FLAGCHR)
          (void) strncpy(lastsect,name,sizeof(lastsect)-1);
       else
	  for (a=list_ptr; a->ap->attribute; a++)
	     if (!strcmp(a->ap->attribute,lastsect))
		a->ap->flag = 'F',
		rc = TRUE;
    }
    else
    {
    for ( a=list_ptr ; a->ap->attribute ; a++)
	if (a->reflag)
	{
	   if (!regexec(&a->are,name,(size_t)0,(regmatch_t *)NULL,0))
	      a->ap->flag = 'F',
	      rc = TRUE;
	}
	else if ( !(strcmp(a->ap->attribute,name)))
            {                       /* if the attribute is found */
            a->ap->flag = 'F';                      /* then mark it found by */
            rc = TRUE;                  /* setting the flag to 'F'. */
            }
    }

    return rc;
}

/*===========================================================================*/
int
attr_not_found(const rattr_t *list_ptr,int hdng_done)
{
  register const rattr_t *a;
  char local_str[100];
  char temp_str[200];

  for ( a=list_ptr; a->ap->attribute; a++) /* any time an attribute is found  */
      if ( a->ap->flag != 'F' )         /* the flag field is not set to 'F'  */
      {                             /* then it was not found in the */
      if ( !hdng_done )                     /* custom file */
          {
          printf("\n");
          hdng_done = TRUE;
          }
      TEXT(local_str,ATTR_NOXST);           /* custom file */
      sprintf(temp_str,local_str,a->ap->attribute);
      WRITE(2,temp_str);
      }

  return TRUE;
}

/*===========================================================================*/
int
split(char *str,int width,char array[][NCOLS],int wrap)
{
    int len, i, j, r, c;

    for( i=0 ; i<NROWS ; i++ )
        for( j=0 ; j<NCOLS ; j++)
            array[i][j] = '\0';

    len = strlen(str);

    r = c = 0;
    for (i=0; i<len; i++)
        {
        if (c>width-1)
            {
            array[r][c]='\0';
            ++r;
            c = 0;
            if ( wrap )
                {
                array[r][0]=array[r][1]=' ';
                c = 2;
                }
            }
        if (wrap && str[i]!=' ')
            {
            for (j=i; j<len && str[j] && str[j]!=' '; ) j++;
            if ( (j-i)>(width-c) )
                {
                array[r][c++]='\0';
                ++r;
                array[r][0]=array[r][1]=' ';
                c=2;
                }
            array[r][c++]=str[i];
            }
        else array[r][c++]=str[i];
        }

    return r+1;  /* number of lines */
}

/*===========================================================================*/
static int
splitws(wchar_t *str,int width,wchar_t **array,int wrap)
{
    int len, i, j, r, c;
    int dw, cw, dw1;

    for( i=0 ; i<NROWS ; i++ )
        for( j=0 ; j<NCOLS ; j++)
            array[i][j] = '\0';

    len = wcslen(str);

    dw = r = c = 0;
    for (i=0; i<len && r<NROWS-1; i++)
        {
        if (dw>width-1)
            {
            array[r][c]='\0';
            ++r;
            dw = c = 0;
            if ( wrap )
                {
                array[r][0]=array[r][1]=' ';
                c = 2;
		dw = 2 * ((cw = wcwidth(*array[r])) < 0 ? 1 : cw);
                }
            }
        if (wrap && str[i]!=' ')
            {
            for (dw1 = 0, j=i; j<len && str[j] && str[j]!=' '; j++)
		dw1 += (cw = wcwidth(str[j])) < 0 ? 1 : cw;
            if ( dw1>(width-dw) && dw1 < width/2)
                {
                array[r][c++]='\0';
                ++r;
                array[r][0]=array[r][1]=' ';
                c=2;
		dw = 2 * ((cw = wcwidth(*array[r])) < 0 ? 1 : cw);
                }
            array[r][c++]=str[i];
	    dw += (cw = wcwidth(str[i])) < 0 ? 1 : cw;
            }
        else array[r][c++]=str[i],
	     dw += (cw = wcwidth(str[i])) < 0 ? 1 : cw;
        }

    return r+1;  /* number of lines */
}

/*===========================================================================*/
static void
lv_catch_sigpipe(int signo)
{
   siglongjmp(jenv,1);
}

