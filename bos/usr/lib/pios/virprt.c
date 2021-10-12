static char sccsid[] = "@(#)15  1.35  src/bos/usr/lib/pios/virprt.c, cmdpios, bos411, 9428A410j 4/20/94 13:40:46";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
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

/*** virprt.c ***/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "virprt.h"
#include <nl_types.h>
#include <curses.h>

#ifdef PIOTEST
#include <NLchar.h>
#endif

#include <stdlib.h>

static char argx[32][MAXARGLEN];    /* Used to hold modified argv[] during */
                                    /* command line parsing in parse() */
char  *hdrstrp,                     /* pointer to "header = " string */
      *trlstrp,                     /* pointer to "trailer = " string */
      devstr[50],                   /* string for "device = " stanza */
      dsdesc[200],                  /* Datastream description */
      forced_qdname[NAMESIZE],      /* Queue device name specified by user */
      defcatalog[200],              /* name of default descriptor catalog */
      mkq[256],                     /* mkqueue command string */
      rmq[256],                     /* rmqueue command string */
      rmqd[256],                    /* rmqueuedev command string */
      configpath[200],              /* path name of *.config file */
      txtstring[200],               /* text to overlay "Description" on
                                       heading line */
      *screen[10],                  /* screen cntrl cmds from terminfo */
      wkbuf[100];                   /* work buffer */
int   key[10];                     /* keys to scan from terminfo */
WINDOW *win;

extern int odmerrno;				/* used in err_sub, defined in rmvirprt */

static const char		*vp_parsemsg(const char *);
static const char       	*vp_getmsg(const char *,int,int);
int multi_byte_check();
int valid_chars();

int   prt_type = NOTHOSTATTACH,     /* Printer interface type */
      configline = 0,
      ask_for_default = TRUE;       /* should this be the default queue */

int   interactive_mode = FALSE;	    /* interactive session set to FALSE */

/*===========================================================================*/
int parse(argc,argv,flags,reqd,addeq,attrs,flagcnt,flagvars)
/*
**  Parse the command line parameters. Check that all required flags
**  have been specified; add an '=' (for lsvirprt) if needed; find all
**  arguments for flags and store in attrs; prepend an underscore to
**  any single-character attribute name; etc.
*/
struct attr **attrs;                 /* List of attribute-value pairs */
char   **argv,
       **flagvars;
char   *flags,
       *reqd;
int    argc,
       addeq,
       flagcnt;
{ int flagno, parmflg, index, i, j, looping;
  struct attr *locattr;
  int argn;

char   t;

  if ( argc == 1 ) return -1;
  for ( i=0 ; i<argc ; i++ ) (void)strncpy(argx[i],argv[i],sizeof(argx[i])-1);
  argn = argc;

  for ( i=1 ; i<argn ; i++ )
      {
      if ( argx[i][0] == '-' && argx[i][2] != NULL )
      {
      for( j=argn ; j>i+1 ; j-- ) strcpy(argx[j],argx[j-1]);
      strcpy(argx[i+1],argx[i]+2);
      argx[i][2] = NULL;
      ++argn;
      }
      }

  if (addeq)
      for(i=1;i<argn;i++)
      {
      if ( argx[i][0] == '-' && argx[i][1] == 'a' )
          {
          for ( j=i+1 ; j<argn ; j++ )
          if (argx[j][0]!='-') strcpy(argx[j]+strlen(argx[j]),"=");
          break;
          }
      }

  /*  Check that all required flags are present. */
  for ( i=0 ; reqd[i] ; i++ )
      {
      looping = TRUE;
      for( j=1 ; looping ; j++ )
         {
         if ( j>argn ) break;
         if ( argx[j][0] != '-' ) continue;
         if ( argx[j][1] == reqd[i] ) looping=0;
         }
      if (looping) return -1;
      }

  for ( index=0 ; index<flagcnt ; index++ ) **(&flagvars+index) = NULL;
  index = 1;
  while (index < argn)
      {
      if ( strncmp(argx[index],"-a",2) == 0 )
      {
      int attrcnt, tmpindex;
      char *eqptr;

      index++;
      attrcnt = 0;
      tmpindex = index;
      while ( (tmpindex < argn) && (*argx[tmpindex] != '-') )
          {
          attrcnt++;
          tmpindex++;
          }

      if ( attrcnt == 0 ) return -1;

      locattr = (struct attr *)calloc(attrcnt+1,sizeof(struct attr));
      if (locattr == NULL) return -1;
      *attrs = locattr;

      while (index < tmpindex)
          {
          if ((eqptr = strchr(argx[index],'=')) == NULL) return -1;
          *eqptr = NULL;
          locattr->attribute = argx[index];
          locattr->value     = eqptr + 1;
              locattr->flag = NULL;
          if ( strlen(locattr->attribute) == 1 )
             {
             char *p;
             p = malloc(3);
             *p = '_';
             strcpy(p+1,locattr->attribute);
             locattr->attribute=p;
             }
          index++;
          locattr++;
          }
      }
      else
      {
      if ( findflag(argx[index]+1,flags,&flagno,&parmflg) == -1 )
         return -1;
      else
          {
          if (parmflg)
             {
             index++;
             if ( *argx[index] == '-' ) return -1;
             }

          if ( **(&flagvars+flagno) == NULL) **(&flagvars+flagno) = argx[index];
          else return -1;
          index++;
          }
      }
      }
  return 0;
}

/*===========================================================================*/
int start_cur()
/*
**  Initialize the curses subroutines.  Return the screen height.
*/
{
  extern char *tgetstr();
  extern int   tgetnum();
  int i;

  SAVE_TTY;                    /* save original term stat */
  interactive_mode = TRUE;     /* interactive session starts */

  ioctl(0,TCGETA,&alterd);          /* set up tty for ignore signals, */
  alterd.c_lflag &= ~(ISIG | ECHO | ICANON);/* no echo, and cononical */
  alterd.c_cc[4] = 3;                       /* processing */
  alterd.c_cc[5] = 1;

  win = initscr();
  noecho();
  screen[CLR_SCR]  = tgetstr("cl", NULL);
  screen[CLR_EOL]  = tgetstr("ce", NULL);
  screen[STANDOUT] = tgetstr("so", NULL);
  screen[NORMAL]   = tgetstr("se", NULL);
  screen[PUT_CUR]  = tgetstr("cm", NULL);
  screen[5]        = NULL;

  key[BREAK]       = 0x03;
  key[QUIT]        = 0x7f;
  key[ENTER]       = KEY_ENTER;
  key[KEYBS]       = KEY_BACKSPACE;
  key[KEYUP]       = KEY_UP;
  key[KEYUP_ALT]   = '-';
  key[KEYDN]       = KEY_DOWN;
  key[KEYDN_ALT]   = '+';
  key[8]           = NULL;

  return tgetnum("li") - 1;
}

/*===========================================================================*/
int file_stat(file_name)
/*
**  Return the status of the named colon file.
**      rc = 1 (DZNTXST)  -> File does not exist.
**      rc = 2 (PERM_OK)  -> File exists with correct permissions.
**      rc = 3 (PERM_BAD) -> File exists, but permissions are wrong.
**
**  The correct permissions are 664
*/
char *file_name;
{
  struct stat buffer;
  ushort min_perm = ( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
  ushort file_perm;

  if (!stat(file_name,&buffer))
     {
     file_perm = buffer.st_mode;
     if ( (file_perm & min_perm) == min_perm ) rc = PERM_OK;
     else rc = PERM_BAD;
     }
  else rc = DZNTXST;

  return rc;
}

/*===========================================================================*/
int findflag(c,flags,flgno,parmflg)
/*
**  Used in parse().
*/
register char *c, *flags;
int *flgno, *parmflg;
{
  *flgno = 0;
  *parmflg = FALSE;
  while (*flags!=*c && *flags!=NULL)
      {
      if (*flags!=':') *flgno += 1;
      flags++;
      }
  if ( *flags != *c ) return -1;
  if ( *(flags+1) == ':' ) *parmflg = TRUE;
  return 0;
}

/*===========================================================================*/
int fgetln(file,line)
/*
**  Get a single line from the specified file.
*/
FILE *file;
char *line;
{
  int i;
  char ch;

  i = 0;
  while ( (ch = fgetc(file) ) != '\n' && !feof(file) && i<LINE_MAX) line[i++] = ch;
  line[i] = NULL;
  return feof(file) ? EOF : 0 ;
}

/*===========================================================================*/
char *getf(cs,num)
/*
**  Return the numth field in a colon string.
*/
char *cs;
int num;
{
  static char fld[1000];
  int i, j, length, cc;

  length = strlen(cs);
  cc = i = 0;
  while ( cc<num )
      {
      if ( cs[i] == ':' ) cc++;
      if ( ++i > length ) break;
      }
  for ( j=0 ; i<length && cs[i] && cs[i]!=':' ; ) fld[j++]=cs[i++];
  fld[j] = NULL;
  return fld;
}

/*===========================================================================*/
putval(fld,val)
/*
**  Put a value into a colon-string field.
*/
char *fld,*val;
{
  int rc;
  char *cp1, *cp2;
  char valstr[1500];

  switch( file_stat(cusfile) )
      {
      case DZNTXST: err_sub(ABORT,CUS_NOXST);            /* doesn't exist */
      case PERM_OK: break;                      /* exists */
      case PERM_BAD: err_sub(ABORT,CUS_NOPEN);          /* can't open */
      }

  for (cp1 = val, cp2 = valstr; *cp1 != NULL; cp1++, cp2++)
      {
      if (*cp1 == '&' || *cp1 == '%' || *cp1 == '\\') *cp2++ = '\\';
      *cp2 = *cp1;
      }
  *cp2 = NULL;

  umask(75);
  sprintf(cmd, "%s -F: '{ if ( $3 == \"%s\" ) \
            { $5 = \"%s\" ; print $1\":\"$2\":\"$3\":\"$4\":\"$5 } \
                      else print $0 }' %s >%s ; %s %s %s", \
                  AWK, fld,valstr,cusfile,tempfile, MV, tempfile,cusfile);
  system(cmd);

}

/*===========================================================================*/
struct attr *param(attrs,attrname)
/*
**  Used in parse().
*/
struct attr *attrs;
char *attrname;
{
  register int namelen;
  register int i = 0;

  namelen = strlen(attrname);
  while ((attrs[i].attribute != NULL)
             && (strncmp(attrs[i].attribute,attrname,namelen))) i++;
  return !attrs[i].attribute?NULL:&attrs[i];
}

/*===========================================================================*/
char *getval(fld,val,filename)
/*
**  Get a value from a colon-string field.
*/
char *fld, *val, *filename;
{
    FILE *file;
    char line[LINE_MAX];
    int done = FALSE;

  switch( file_stat(filename) )
      {
      case DZNTXST: err_sub(ABORT,CUS_NOXST);   /* doesn't exist */
      case PERM_OK: break;                      /* exists */
      }

    file = fopen(filename,"r");
    while ( !fgetln(file,line) && !done ) 
            {
                if (line[0] == '#')
                   continue;
        if ( !strcmp(fld,getf(line,2)) )
            {
            strcpy(val,getf(line,4));
            done = TRUE;
            }
            }
    fclose(file);

    return val;
}

/*===========================================================================*/
char *mkdigestname()
/*
**  Generate the digest file name.
*/
{
  char mt[100], md[100], mn[100], fld[100], line[LINE_MAX];
  static char name[500];
  FILE *file;

  file = fopen(cusfile,"r");
  *mt = NULL; *md = NULL; *mn = NULL;

  while ( !fgetln(file,line) )
      {
      strcpy(fld,getf(line,2));
      if ( !strcmp(fld,"mt") ) strcpy(mt,getf(line,4));      /* ptr type */
      if ( !strcmp(fld,"md") ) strcpy(md,getf(line,4));      /* ds name */
      if ( !strcmp(fld,"mn") ) strcpy(mn,getf(line,4));      /* device file */
      }
  fclose(file);

  sprintf(name,"%s%s.%s.%s.%s:%s",digpath,mt,md,mn,pqname,vpname);
  return name;
}

/*===========================================================================*/
int call_piodigest()
/*
**  Invoke the piodigest command.
*/
{
	int  rc;

    sprintf(cmd,"%s -q %s -v %s %s%s:%s",
            digestcmd,pqname,vpname,cuspath,pqname,vpname);
    rc = system(cmd);

	return(rc);
}

/*===========================================================================*/
err_sub(action,err_num)
/*
**  Generalized error handler.
*/
int action, err_num;
{
  char local_str[1024];
  errflag = TRUE;
  bzero(local_str, sizeof(local_str));
  TEXT(str,err_num);
  switch(err_num)
      {
      case QUEUE_XST:                  /* exist to existing queue */
      case BAD_NAME:               /* bad queue and queue device name */
      case BAD_BACKEND:        /* backend found in existing quedev is bad */
      case CUS_XST:             /* customized file already exists */
      case CUS_NOXST:               /* customized file does not exist */
      case CUS_XST_QQD:             /* queue and quedev already exist */
      case USAGE_RM:                    /* rmvirprt usage command */
      case USAGE_LS:                    /* lsvirprt usage command */
      case USAGE_CH:                    /* chvirprt usage command */
      case USAGE_MK:                    /* mkvirprt usage command */
      case USAGE_PP:                   /* piopredef usage command */
      case DSTYPE:                 /* not a valid data stream */
      case PTYPE:                     /* no such printer type */
      sprintf(local_str,str,pqname,vpname,dname,ptype,dstype);
      break;

      case WRONG_DEV:                    /* incorrect device name */
      sprintf(local_str,str,vpname,dname);
      break;

      case TTY:                /* quit if not connected to a tty port */
      case INV_DNAME:               /* device name does not exist */
      case QUEUE_NAMES:             /* bad queue or queue device name */
      sprintf(local_str,str);
      break;

      case TOO_MANY:            /* D45330 - too many virtual printers defined */
          NLsprintf(local_str,str,MAXPRINTERS);
          break;


      case QUEUE:           /* quit if invalid queue */
      sprintf(local_str,str,pqname);
      break;

      case QUEDEV:          /* quit if invalid queue device */
      sprintf(local_str,str,pqname,vpname);
      break;

      case PRE_XST:             /* predefined file does not exist */
      case PRE_NOXST:               /* predefined file does not exist */
      case MULTI_DS_GREET:         /* preamble for multi data stream loop */
      case DSTYPE2:                 /* data stream not spec'd */
      sprintf(local_str,str,ptype,dstype);
      break;

      case MULTI_DS_HEAD:    /* heading for multiple data stream printers */
      sprintf(local_str,str,dsdesc);
      break;

      case MKVIR_SUMMARY:              /* summary of mkvirprt command */
      sprintf(local_str,str,ptype,cmd);
      break;

      case MKVIR_SUMMARY_DS:     /* summary of mkvirprt command (multi ds) */
      sprintf(local_str,str,ptype,dsdesc,cmd);
      break;

      case PRE_NOPEN:              /* predefined file can't be opened */
      sprintf(local_str,str,ptype,dstype,prefile,errno);
      break;

      case CUS_NOPEN:              /* customized file can't be opened */
      sprintf(local_str,str,cusfile,errno);
      break;

      case ATTR_NOXST:          /* specified attribute does not exist */
      sprintf(local_str,str,cmd);
      break;

      case MSG_PIFSHORT:
          sprintf(local_str,str,configline,configpath,NUMFIELDS);
          break;
      case MSG_OPENDIR2:
          sprintf(local_str,str,ETCPATH,errno);
          break;
      case MSG_OPENFILE:
          sprintf(local_str,str,configpath,errno);
          break;
      case MSG_FILELOCK:
          sprintf(local_str,str,custom_name);
          break;
      case MSG_ATTR_NONMODFBLE:  /* specified attribute can not be modified */
         (void) sprintf (local_str, str, cmd);
         break;
      case MSG_MALLOC:             /* malloc error */
         (void) sprintf(local_str,str);
         break;
      case MSG_POPENERR:
	 (void) sprintf(local_str,str,strerror(errno),cmd);
	 break;

      case MSG_RM_ERRODMINIT:
          sprintf(local_str,str,odmerrno);
          break; 

      case MSG_RM_ERRODMSP:
          sprintf(local_str,str,odmpath,odmerrno);
          break;

      case MSG_RM_ERRODMLOCK:
          sprintf(local_str,str,odmpath,odmerrno);
          break;

      case MSG_RM_ERRODMOPEN:
          sprintf(local_str,str,cmd,odmerrno);
          break;  
          
      case VPEXISTS:          /* quit if virtual printer already exists */
      	  sprintf(local_str,str,pqname,vpname);
      	  break;

      }
  WRITE(2,local_str);
  if ( action == ABORT ) goodbye(TERMINATE);
}

/*===========================================================================*/
valid_queue(pqname)
/*
**  Return 1 if the specified queue name is valid, 0 otherwise.
*/
char *pqname;
{
  sprintf(cmd,"%s -q%s %s", LSQUECMD, pqname, WASTE);
  return !system(cmd);
}

/*===========================================================================*/
valid_quedev(vpname)
/*
**  Return 1 if the specified queue device name is valid, 0 otherwise.
*/
{
  sprintf(cmd,"%s -q%s -d%s %s", LSQUEDEVCMD, pqname, vpname, WASTE);
  return !system(cmd);
}

/*===========================================================================*/
struct pn_struct *make_list(path,delim)
/*
**  Given a path and a delimiter character, read the two-part file
**  names into a linked list of linked lists and return a pointer to
**  the head of the linked list (the first of the primary nodes).
*/
char *path;
char delim;
{
  FILE *file;
  char local_str[LINE_MAX], *cp;
  struct pn_struct *head;

  sprintf(cmd,"%s %s | %s >%s", LS, path, SORT, tempfile);
  system(cmd);
  OPENTEMP;
  head = NULL;
  while ( !fgetln(file,local_str) )
      {
      if ( !(cp = strchr(local_str,delim)) ) continue;
      *cp++ = NULL;
      add_nodes(&head,local_str,cp);
      }
  CLOSETEMP;
  KILLTEMP;
  if ( !head )
      {
      TEXT(local_str,NO_VIRPTS);
      WRITE(2,local_str);
      if ( getenv("PIOSMIT") ) proceed(CONFIRM);
      goodbye(TERMINATE);
      }
  return head;
}

/*===========================================================================*/
make_plist(phead)
/*
**  Build the list of predefined printers.
*/
struct pn_struct *phead;
{
  char mL[80], mt[80], fld[20], line[LINE_MAX], local_str[200];
  int cnt = 0;
  struct pn_struct *prm;
  struct sn_struct *sec;
  FILE *file;
  char const *cp;
  char const *cp1;

  for ( prm=phead ; prm ; prm=prm->pn_next )
      {
      if ( cnt >= MAXPRINTERS )    /* sanity check  D45330 */
           err_sub( ABORT, TOO_MANY );
      printers[cnt] = (char *)malloc(strlen(prm->pn)+1);
      strcpy(printers[cnt],prm->pn);

      sec = prm->sn_head;
      sprintf(local_str, "%s%s.%s", prepath, prm->pn, sec->sn);
      sprintf(prefile, "%s%s.%s", prepath, prm->pn, sec->sn);

      file = fopen(local_str,"r");
      if (file == NULL)
      	err_sub(ABORT,PRE_NOPEN); 			/* can't open */
      *mL = NULL;  *mt = NULL;
      while ( !fgetln(file,line)) 
      {
          if (line[0] == '#')
             continue;
          strcpy(fld,getf(line,2));
      if ( !strcmp(fld,"mt") ) strcpy(mt,getf(line,4));   /* ptr type */
      if ( !strcmp(fld,"mL") )				  /* ptr desc */
	  {
	     (void)strncpy(mL,(cp = vp_parsemsg(cp1 = getf(line,4)))?cp:cp1,
			   sizeof(mL)-1),
	     *(mL+sizeof(mL)-1) = 0;
	  }
      }
      fclose(file);

      if ( !(*mL) ) strcpy(mL,mt);          /* if no desc use pr type */
      if ( !(*mL) ) strcpy(mL,prm->pn);         /* if no desc use pr type */

      sprintf(local_str,"     %-3d    %s  ",cnt+1,mL);
      if ( getenv("LOGVIRPRT") )
       sprintf(local_str,"     %-3d  %-16s    %s  ",cnt+1,printers[cnt],mL);
      scr_line[cnt] = (char *)malloc(strlen(local_str)+1);
      strcpy(scr_line[cnt++],local_str);
      scr_line[cnt] = NULL;
      }
}

/*===========================================================================*/
make_qlist(phead)
/*
**  Build the list of virtual printers.
*/
struct pn_struct *phead;
{
  char mt[70], md[70], mn[70], mA[70], mL[70];
  char fld[20], line[LINE_MAX], local_str[200], tmp_str[100];
  int multi_ds, cnt = 0;
  struct pn_struct *prm;
  struct sn_struct *sec;
  FILE *file;
  char const *cp;
  char const *cp1;

  for ( prm=phead ; prm ; prm=prm->pn_next )
      {
      if ( prm->sn_head->sn_next ) multi_ds = TRUE;
      else multi_ds = FALSE;
      for ( sec=prm->sn_head ; sec ; sec=sec->sn_next )
      {
      if ( cnt >= MAXPRINTERS )  /* sanity check D45330 */
           err_sub( ABORT, TOO_MANY );
      printers[cnt] = (char *)malloc(strlen(prm->pn)+strlen(sec->sn)+2);
      sprintf(printers[cnt], "%s:%s", prm->pn, sec->sn);

      sprintf(local_str, "%s%s:%s", cuspath, prm->pn, sec->sn);
      sprintf(cusfile,"%s%s:%s",cuspath,prm->pn,sec->sn);
      file = fopen(local_str,"r");
      if (file == NULL)
     	  err_sub(ABORT,CUS_NOPEN); 			/* can't open */
      *mt = NULL; *md = NULL; *mn = NULL; *mA = NULL; *mL = NULL;

      while ( !fgetln(file,line) )
          {
          strcpy(fld,getf(line,2));
          if ( !strcmp(fld,"mt") ) strcpy(mt,getf(line,4));   /* ptr type */
          if ( !strcmp(fld,"md") ) strcpy(md,getf(line,4));    /* ds name */
          if ( !strcmp(fld,"mn") ) strcpy(mn,getf(line,4));/* device file */
          if ( !strcmp(fld,"mA") ) strcpy(mA,getf(line,4));    /* ds desc */
          if ( !strcmp(fld,"mL") )			      /* ptr desc */
	  {
	     (void)strncpy(mL,(cp = vp_parsemsg(cp1 = getf(line,4)))?cp:cp1,
			   sizeof(mL)-1),
	     *(mL+sizeof(mL)-1) = 0;
	  }
          }
      fclose(file);

      if ( !(*mA) && *mL ) strcpy(mt,mL);

      if ( multi_ds ) sprintf(local_str," %-3d %-16s  %-8s %s  ",
                             cnt+1,printers[cnt],mn,mt);
      else sprintf(local_str," %-2d  %-16s  %-8s %s  ",cnt+1,prm->pn,mn,mt);
      if ( *mA )
          {
          sprintf(tmp_str,"(%s)  ",mA);
          strcat(local_str,tmp_str);
          }

      scr_line[cnt] = (char *)malloc(strlen(local_str)+1);
      strcpy(scr_line[cnt++],local_str);
      scr_line[cnt] = NULL;
      }
      }
  return cnt;                   /* number of virtual printers */
}

/*===========================================================================*/
make_dlist()
/*
**  Build the list of printer devices.
*/
{
  char local_str[LINE_MAX];
  int cnt = 0;
  FILE *file;

  sprintf(cmd,"%s -C -c printer -S a -F \"%s\" >%s 2>%s", LSDEVCMD,
                 "      name          description", tempfile, DVNL);
  system(cmd);

  OPENTEMP;
  while ( !fgetln(file,local_str) )
      {
      if ( cnt >= MAXPRINTERS )    /* sanity check  D45330 */
           err_sub( ABORT, TOO_MANY );
      scr_line[cnt] = (char *)malloc(strlen(local_str)+1);
      strcpy(scr_line[cnt++],local_str);
      }
  scr_line[cnt] = NULL;
  CLOSETEMP;

  cnt = 0;
  sprintf(cmd,"%s -C -c printer -S a -F \"%s\" >%s 2>%s", LSDEVCMD,
                           "name:type", tempfile, DVNL);
  system(cmd);
  OPENTEMP;

  while ( !fgetln(file,local_str) )
      {
      if ( cnt >= MAXPRINTERS )    /* sanity check D45330 */
           err_sub( ABORT, TOO_MANY );
      printers[cnt] = (char *)malloc(strlen(local_str)+2);
      strcpy(printers[cnt++],local_str);
      }
  printers[cnt] = NULL;
  CLOSETEMP;
  KILLTEMP;
}

/*===========================================================================*/
get_pr(line,num)
/*
**  Parses printers[] to get the queue name and queue device name.
**  If num==1 then parsing for mkvirprt.  Copy only field into
**  ptype.  If num==2 then parsing for ls/ch/rm.  Copy fields
**  into pqname and vpname respectively.
*/
char *line;
int num;
{
  char *wp1 = line;

  switch ( num )
      {
      case 1:
      strcpy(ptype,wp1);
      break;

      case 2:
      wp1 = strchr(line,':');
      *wp1++ = NULL;
      strcpy(pqname,line);
      strcpy(vpname,wp1--);
      *wp1 = ':';
      break;

      case 3:
      wp1 = strchr(line,':');
      *wp1++ = NULL;
      strcpy(dname,line);
      strcpy(ptype,wp1--);
      *wp1 = ':';
      break;
      }
}

/*===========================================================================*/
add_nodes(head,pn,sn)
/*
**  Add the given secondary node to its proper place in the list,
**  creating a primary node if the corresponding one does not already
**  exist.
*/
struct pn_struct **head;
char *pn, *sn;
{
  struct pn_struct *p;

  if (!*head) p = *head = new_pn(pn,sn);
  else
      {
      if (!(search_pn(&p,*head,pn))) p = p->pn_next = new_pn(pn,sn);
      else p->sn_head = new_sn(sn,p->sn_head);
      }
}

/*===========================================================================*/
struct pn_struct *new_pn(pn,sn)
/*
**  Create a new primary node and return a pointer to it.
*/
char *pn, *sn;
{
  struct pn_struct *sp;

  sp = (struct pn_struct *) malloc(sizeof (struct pn_struct));
  sp->pn = malloc(strlen(pn)+1);
  strcpy(sp->pn,pn);
  sp->sn_head = new_sn(sn,NULL);
  sp->pn_next = NULL;
  return sp;
}

/*===========================================================================*/
struct sn_struct *new_sn(sn,next)
/*
**  Create a new secondary node and return a pointer to it.
*/
char *sn;
struct sn_struct *next;
{
  struct sn_struct *s;

  s = (struct sn_struct *) malloc(sizeof (struct sn_struct));
  s->sn = malloc(strlen(sn)+1);
  strcpy(s->sn,sn);
  s->sn_next = next;
  return s;
}

/*===========================================================================*/
struct pn_struct *found_pn(head,pn)
/*
**  Return 0 or pointer based on whether the given primary node was
**  found in the linked list.
*/
struct pn_struct *head;
char *pn;
{
  struct pn_struct *p;

  for (p=head ; p && strcmp(p->pn,pn) ; p=p->pn_next);
  return p;
}

/*===========================================================================*/
search_pn(ptr,head,pn)
/*
**  Search for the given primary node; if found, set ptr equal to
**  it; if not found, let ptr point to the last node in the list.
**  Return 0 or 1 based on whether the node was actually found.
*/
struct pn_struct **ptr;
struct pn_struct *head;
char *pn;
{
  int found, quit;
  struct pn_struct *p;

  found = quit = 0;
  for (p=head ; !found && !quit ; )
      {
      found=!strcmp(p->pn,pn);
      quit = !(p->pn_next);
      if ( !found && !quit ) p=p->pn_next;
      }
  *ptr = p;
  return found;
}

/*===========================================================================*/
struct sn_struct *found_sn(head,pn,sn)
/*
**  Search for the given secondary node in the list attached to the
**  given primary node; return 1 if found, 0 otherwise. If the second
**  string is null, found_sn() is basically equivalent to found_pn().
*/
struct pn_struct **head;
char *pn, *sn;
{
  struct pn_struct *p;
  struct sn_struct *s;

  s = NULL;
  p = found_pn(*head,pn);
  if (p && *sn)
      {
      for( s=p->sn_head ; s && strcmp(s->sn,sn) ; s=s->sn_next);
      }
  return ( *sn ? s : (struct sn_struct *)p );
}

/*===========================================================================*/
get_dev(dn)
/*
**  Get a valid device name from the user.
*/
char *dn;
{
  char *wp, local_str[40];
  int bad;
  int cnt = 0;
  int found = TRUE;
  int looping = TRUE;
  int pr_list = FALSE;

/*  Can there be a default device name?  The default name should be
 *  the first unassigned device file.  This is determined by splitting
 *  each line in printers[] into device name and printer type.  The
 *  colon files in custom are then examined to see if that device has
 *  been previously assigned.  If no, then there can be no default
 *  device file name.
 */

  prt_type = HOSTATTACH;
  while( found && printers[cnt] )
      {
      if ( cnt >= MAXPRINTERS )    /* sanity check D45330 */
           err_sub( ABORT, TOO_MANY );
      get_pr( printers[cnt++],3 );
      sprintf(cmd,"%s -l ':mn:\\(.*\\):%s' %s* %s", GREP, dn, cuspath, WASTE);
      found = !system(cmd);
      }

  if ( !found ) strcpy(dn,dname);        /* virtual printers are assigned */
  else *dn = NULL;                /* to all available devices */

  while(looping)
      {
      bad = FALSE;
      pr_list = FALSE;
      resp_menu(GET_DEVICE,DEVICE_HDG,dn,GETTEXT);

      if ( *dn == '*' )               /* what if the user enters *dev */
      {
      wp = dn;
      strcpy(wp,dn+1);
      pr_list = TRUE;
      }

      if ( strstr(dn,"/dev/") )       /* what if the user enters /dev/xxx */
      {
      wp = dn;
      strcpy(wp,dn+5);
      }

/*  Find out if the device name entered is one displayed on the screen.
 *  The variable found will become 1 when (if) the search is successful.
 *  Otherwise found will remain zero and the search will terminate when
 *  no more printers[] exist.  If the line is located, it is displayed
 *  in the terminals hilited mode.
 */
      found = FALSE;
      if ( !pr_list )
      {
      strcpy(local_str,dn);           /* save response in tmp var */
      cnt = 0;
      while( !found && printers[cnt] )
          {
          if ( cnt >= MAXPRINTERS )    /* sanity check D45330 */
               err_sub( ABORT, TOO_MANY );
          get_pr( printers[cnt++],3 );
          found = !strcmp(local_str,dname);
          }
      strcpy(dn,local_str);              /* put the response back */
      }

/*  Is there a colon file in the predef directory matching the printer
 *  type selected with the device.  If not, then wipe out ptype so the
 *  user will get the list of predefined files.
 */
      if ( pr_list || !found ) *ptype = NULL;
      else
      {
      sprintf(cmd,"%s -1 -Of %s*| %s -l '.*/%s\\..*' > %s", 
				LI, prepath, GREP, ptype, DVNL);
      if ( system(cmd) ) *ptype = NULL;

/*  Has the entered device been specified as the device file in any
 *  existing colon file in the custom directory?  If yes, then the
 *  printer type can no longer be assumed and is therefore blanked out.
 */
      sprintf(cmd,"%s -l ':mn:\\(.*\\):%s' %s* %s", GREP, dn, cuspath, WASTE);
      if ( *ptype && !system(cmd) ) bad = DEV_USED;

      if ( found && *ptype ) hilite(scr_line[cnt-1],FALSE);
      }

       if ( !bad )        /* is *dn a legal existing device file name */
       {
       sprintf(cmd,"%s -f /dev/%s || %s -b /dev/%s || %s -c /dev/%s", 
					  TEST, dn, TEST, dn, TEST, dn);
       if ( system(cmd) ) bad = INV_DNAME;
       }

       switch( bad )
       {
       case INV_DNAME:             /* if the specified device file is not */
           strcpy(dname,dn);     /* a regular file, a block special file, */
           *dn = NULL;       /* or a character special file, then */
           proceed(INV_DNAME);             /* reject it and try again */
           break;

       case DEV_USED:
           looping = !yes_no(DEV_USED,FALSE);
           break;

       case FALSE:
           looping = FALSE;
           break;
       }
       }
  return(cnt-1);
}

/*===========================================================================*/
get_queue(pq,not_unique,makeflag)
/*
**  Get a valid queue name from the user; make the queue if necessary.
**  If the queue exists (is validated by valid_queue) then the string
**  'rmq' will remain null.  If the queue can be successfully made,
**  the 'rmq' string will contain the command necessary to perform
**  the un-make.
*/
char *pq;
struct sn_struct *not_unique;
int makeflag;
{
  int looping = TRUE;
  int len, cnt;
  char *wp;
  char *work_str;
  char defval[NAMESIZE];
  char ans[5];

  while(looping)
      {
      if (makeflag)
      {
      strcpy( defval , not_unique ? dstype : dname );
          if ((multi_byte_check(defval) == 1) || (valid_chars(defval) == 1))
         *defval = NULL;
          else if (!not_unique && strlen(defval) > 20)
                   *(defval+20) = '\0';
      }
      else *defval = NULL;

      response(makeflag?(makeflag==2?ENTER_PQ_SKIP:ENTER_PQ):PICK_PQ,pq,defval);
                      /* permits user to skip creation of */
      if ( (*pq == '!') && (makeflag == 2) ) return;  /* virtual printers for */
                 /* printers supporting multiple data streams */

      work_str = (char *)malloc(strlen(pq) + 1);
      strcpy(work_str,pq);
      wp = strchr(work_str, ':');
      if (wp == NULL) 
         if ((multi_byte_check(pq) == 1) || (valid_chars(pq) == 1))
            { 
            proceed(BAD_NAME);
            *defval = NULL;
            continue;
            }
         else;
      else
         {
         *wp++ = '\0';  /* now work_str points to queue and wp to device */
         if ( (*wp == NULL) || (multi_byte_check(work_str) == 1) ||
             (valid_chars(work_str) == 1) || (multi_byte_check(wp) == 1) ||
             (valid_chars(wp) == 1) )
            {
            proceed(BAD_NAME);
            *defval = NULL;
            continue;
            }
         }

      if ( wp = strchr(pq, ':') )
      {
      *wp = NULL;
      strcpy(forced_qdname, wp+1);
      }
      else *forced_qdname = NULL;

      if ( strlen(pq) > 20 )
      {
      proceed(NAME_TOO_LONG);
      *defval = NULL;
      continue;
      }

      *rmq = NULL;         /* init rmq str to null - will be tested later */
      if (valid_queue(pq))
      {
      if ( makeflag ) looping = !yes_no(QUEUE_XST,FALSE);
      else looping = FALSE;
      }
      else if (makeflag && mkque(pq)) looping = FALSE;      /* queue was made */
       else proceed(QUEUE_NAMES);
      }
}

/*===========================================================================*/
get_quedev(vp,makeflag)
/*
**  Get a valid queue device name from the user.
**  If the queuedev exists (is validated by valid_quedev) then the string
**  'rmqd' will remain null.  If the queuedev can be successfully made,
**  the 'rmqd' string will contain the command necessary to perform
**  the un-make.
*/
char *vp;
int makeflag;
{
  int looping = TRUE;
  int ask = !makeflag;         /* tells the routine to make an assumption */
                  /* and check it once before asking the question */
  int len, cnt;
  char device[NAMESIZE];
  char bknd[NAMESIZE];
  char defval[NAMESIZE];
  char dname_tmp[20];

  strcpy( defval , *forced_qdname ? forced_qdname : dname );
  if (makeflag)
      if ((multi_byte_check(defval) == 1) || (valid_chars(defval) == 1))
         {
         *defval = NULL;
         ask = TRUE;
         }
  if (strlen(defval) > 20)
      *(defval+20) = '\0';
  while (looping)
      {
      if ( ask ) response(makeflag ? ENTER_VP : PICK_VP,vp,defval);
      else strcpy(vp,defval);

      if ((multi_byte_check(vp) == 1) || (valid_chars(vp) == 1)) 
      {
      proceed(BAD_NAME);
      continue;
      }

      if ( strlen(vp) > 20 )
      {
      proceed(NAME_TOO_LONG);
      ask = TRUE;
      continue;
      }

      *rmqd = NULL;
      *devstr = NULL;

      sprintf(cusfile,"%s%s:%s",cuspath,pqname,vp);/* is there a custom file */
      switch( file_stat(cusfile) )                 /* by that name already */
      {
      case DZNTXST:                      /* doesn't exist */
         errflag = FALSE;               /* assume all is well */
         if ( valid_quedev(vp) )
         {
         sprintf(cmd,"%s -q%s -d%s",LSQUEDEVCMD,pqname,vp);
         search_output(cmd,"file = ",device);
         if ( !strcmp(device,"FALSE") || !strcmp(device,"false") )
             {
             /* This situation is OK, but must use CHVIRPRT to
            correct the 'file = ' stanza */
             sprintf(devstr,"'file = /dev/%s'",dname);
             errflag = FALSE;
             }
         else if ( strcmp(device+5,dname) )
             {
             strcpy(dname_tmp,dname);
             strcpy(dname,device+5);
             strcpy(vpname,vp);
             err_sub(CONT,WRONG_DEV);
             strcpy(dname,dname_tmp);
             }

         /* Build a string looking like the 'backend = ' stanza ought */
         /* to look.  Put it into bknd' */
         strcpy(bknd, backend);                       

         search_output(cmd,"backend = ",str);         /* get existing */
                                /* 'backend' spec */
         if ( (len = strlen(str)) > strlen(bknd) ) len = strlen(bknd);
                           /* cmp the 'n' chars where 'n' */
         if ( strncmp(bknd,str,len) )    /* is the len of the shorter */
             {                          /* string */
             strcpy(vpname,vp);
             err_sub(CONT,BAD_BACKEND);  /* sets errflag = TRUE */
             }

         }
         else if ( makeflag ) errflag = !mkquedev(pqname,vp);
         break;

     case PERM_OK:                         /* exists */
     case PERM_BAD:
         if ( makeflag )
         {
         err_sub(CONT,CUS_XST_QQD);      /* make the default name */
         *defval = NULL;            /* null since its bad */
         }
         break;
     }
     ask = TRUE;
     looping = errflag;              /* looping = 0 indicates success */
     }
}

/*==========================================================================*/
int valid_chars(name)  /* the only valid characters are [0-9], [a-z],
                          [A-Z], '_', '-', '+' and '@' */
char *name;
{
   char *ptr;

   for(ptr = name; *ptr != NULL; ptr++)
      if ((*ptr<'+') || (*ptr>'+' && *ptr<'-') || (*ptr>'-' && *ptr<'0') ||
          (*ptr>'9' && *ptr<'@') || (*ptr>'Z' && *ptr<'_') ||
          (*ptr>'_' && *ptr<'a') || (*ptr>'z'))
             return(1);   /* bad character found */
   return(0);
}

/*===========================================================================*/
int multi_byte_check(name)
char *name;
{
int length;

      length = 1;
      if (MB_CUR_MAX > 1)    /* then possibility of multi-byte characters  */
         {
         while (*name != NULL)
            {
            length = mblen(name,MB_CUR_MAX);  
            if (length > 1)
               return(1);
            name++;
            } 
         }

      return(0);   /* no multi-byte chars found */
}

/*===========================================================================*/
get_burst(msgid, strptr, str_g, str_a)
/*
**  Process burst (header or trailer) page option
*/
int msgid;
char **strptr;
char *str_g, *str_a;
{
  char ans[30];
  char local_str[100];
  char prompt_str[100];
  int cntnue = TRUE;

  TEXT(local_str,msgid);
  *ans = NULL;
  *strptr = NULL;
  sprintf(prompt_str,"\r%s%s  -> (n)   ",screen[CLR_EOL],local_str);

  while ( cntnue )
      {
      WRITE(1,prompt_str);
      readkbd(ans,FALSE,0,0);
      if ( !strcmp(ans,"n") || !*ans ) cntnue = FALSE;
      if ( !strcmp(ans,"g") ) { *strptr = str_g; cntnue = FALSE; }
      if ( !strcmp(ans,"a") ) { *strptr = str_a; cntnue = FALSE; }
      }
  NULN;
}

/*===========================================================================*/
yes_no(msgid,dflt)
/*
**  Accept yes or no response
*/
int msgid, dflt;
{
  char ans[5], yes_str[5], no_str[5], dflt_str[5];
  char local_str[1000], prompt_str[1000];
  int cntnue = TRUE;
  int value;

  TEXT(local_str,msgid);
  TEXT(yes_str,YES_LETTER);
  TEXT(no_str,NO_LETTER);
  if ( dflt ) strcpy(dflt_str,yes_str);
  else strcpy(dflt_str,no_str);
  *ans = NULL;
  sprintf(prompt_str,"\r%s%s -> (%s)  ",screen[CLR_EOL],local_str,dflt_str);

  while ( cntnue )
      {
      WRITE(1,prompt_str);
      readkbd(ans,FALSE,0,0);
      if ( !*ans ) cntnue = FALSE;
      if ( !strcmp(ans,yes_str) ) { value = TRUE;  cntnue = FALSE; }
      if ( !strcmp(ans,no_str) )  { value = FALSE; cntnue = FALSE; }
      }
  NULN;
  return  *ans ? value : dflt ;
}

/*===========================================================================*/
void proceed(msgid)
/*
**  Display a message then wait until enter (or break) is pressed
*/
int msgid;
{
  char local_str[500];

  TEXT(local_str,msgid);
  NULN;
  WRITE(1,local_str);
  readkbd(local_str,FALSE,0,0);
  NULN;
}

/*===========================================================================*/
search_output(cmd,key,obj)
/*
**  Search for a given target string (key) in the output from a specified
**  command (cmd); then scan for the string immediately following the target
**  string and return this second string (obj) to the caller.
*/
char *cmd, *key, *obj;
{
  FILE *file;
  char local_str[LINE_MAX];
  char *p;

  sprintf(local_str,"%s | %s \"%s\" >%s 2>%s", cmd, FGREP, key, tempfile, DVNL);
  system(local_str);

  OPENTEMP;
  p = NULL;
  while ( !fgetln(file,local_str) && !p ) p = strstr(local_str,key);
  CLOSETEMP;
  KILLTEMP;
  p += strlen(key);
  *obj = NULL;
  sscanf(p,"%s",obj);
}

/*===========================================================================*/
mkque(pq)
/*
**  This routine is intended to use the MakeQueue command to verify
**  that the queue is 'makeable'.  If the queue can be made, rc will
**  be initially set to 1.  The queue will then immediately be unmade.
**  This permits the user (or the program) to terminate at any point
**  along the way with no permanent, but unfinished, work being done.
*/
char *pq;
{
  int yes = FALSE;

  if ( ask_for_default )
      {
      yes = yes_no(ENTER_DEFQ,TRUE);
      ask_for_default = !yes;
      }

  sprintf(mkq,"%s %s -q %s", MKQUECMD, yes ? "-D" : " ", pq);
  sprintf(rmq,"%s -q %s %s", RMQUE, pq, WASTE);

  if ( rc = !system(mkq) )    /* if the mkq command is successful */
      system(rmq);             /* immediatly unmake the queue */
  return rc;
}

/*===========================================================================*/
mkquedev(pq,vp)
/*
**  For us to have gotten this far, the queue must be makeable (or exist).
**  If the queue must be made, then the mkq string will be executed.  Then the
**  QueueDev will be made and immediately unmade.  Finally, if the queue
**  was made here, it will be unmade.  If the queue existed, it will be
**  left intact.
*/
char *pq, *vp;
{
  char filestr[200];

  switch ( prt_type )
      {
      case HOSTATTACH:
      sprintf(filestr,"file = /dev/%s", dname);
          strcpy(wkbuf, "-a 'access = both'");
      break;

      case NOTHOSTATTACH:
          if (*fname != NULL)
              sprintf(filestr, "file = %s",fname);
          else
              strcpy(filestr, "file = FALSE");
          *wkbuf = '\0';
      break;
      }

  if (*mkq) system(mkq); /* if mkq string is null then queue exists */

  sprintf(cmd,"%s -q %s -d %s -a '%s' -a %s -a %s %s \
   -a 'backend = %s'", MKQUEDEVCMD, pq, vp, filestr,
  hdrstrp ? hdrstrp:"'header = never'", trlstrp ? trlstrp:"'trailer = never'",
  wkbuf, backend);

  sprintf(rmqd,"%s -q %s -d %s %s", RMQUEDEV, pq, vp, WASTE);
  return !system(cmd);
}

/*===========================================================================*/
response(prompt_id,str,def)
/*
**  Get a response from the user (a character string from stdin); if the
**  user merely presses Enter (in order to accept the default value),
**  the default value "def" is used instead. However, if "str" already has
**  a value on entry to the function, this value will override even the
**  default value "def" to become the new default.
*/
int prompt_id;
char *str, *def;
{
  char prompt[100];
  char prompt_str[100];
  char string[100];

  if (prompt_id < 0)
      strcpy(prompt, txtstring);
  else
     TEXT(prompt,prompt_id);
  strcpy(str,(*str ? str : def));          /* copy the default into 'str' */

  if ( *str ) sprintf(prompt_str,"%s   -> (%s)   ",prompt,str);
  else sprintf(prompt_str,"%s   ->   ",prompt);
  WRITE(1,prompt_str);
  readkbd(string,FALSE,0,0);
  NULN;

  if ( *string ) strcpy(str,string);
}

/*===========================================================================*/
resp_menu(prompt_id,heading_id,rtn_str,action)
/*
**  Get a response from the user (a character string from stdin).
*/
int prompt_id, heading_id, action;
char *rtn_str;
{
  char prompt[400], string[100], heading[320], local_str[320];
  int pr_num, prompt_line;
  int i, rc = 1;
  int num_lines = 0;
  int cnt = 0;
  int insert = FALSE;

  if (heading_id < 0)
  {
      TEXT(heading, PREDEF_HDG);
      strcpy(heading+12, txtstring);
  }
  else
     TEXT(heading,heading_id);
  strcpy(prompt,screen[CLR_EOL]);
  TEXT(local_str,prompt_id);
  strcat(prompt,local_str);
  if ( *rtn_str )
      {
      sprintf(local_str,"  (%s)",rtn_str);
      strcat(prompt,local_str);
      }
  strcat(prompt,"  ->  ");

  hilite(heading,FALSE);
  while ( *scr_line[num_lines+1] ) num_lines++;  /* count displayable lines */
  prompt_line = scroll(0,num_lines);

  do { *string = NULL;
       PUT(prompt_line,0);
       WRITE(1,prompt);
       readkbd(string,TRUE,prompt_line,strlen(prompt));

       switch ( rc = validate(action,string,num_lines+1,rtn_str) )
       {
       case -1:
           goodbye(BAD);
           break;

       case  0:
           if ( action == GETNUM ) i = atoi(string);
           else i = 0;
           break;

       case  1:
           break;
       }
      } while ( rc );
  return ( i );
}

/*===========================================================================*/
align_scrn(top, delta)
int top, delta;
{
  int last = 0;
  int top_tmp = top;

  while ( *scr_line[last+1] ) last++;          /* count displayable lines */

  if ( delta > 0 ) for ( ; delta && scrn_ht<=last-top+3 ; delta--, top++);
  else             for ( ; delta && top                 ; delta++, top--);

  if ( top - top_tmp )
      {
      PUT(1,0);                        /* home the cursor */
      scroll(top,last);                  /* scroll the screen */
      }
  return top;           /* rtn the number of the first line displayed */
}

/*===========================================================================*/
scroll(line_no,last)
int line_no, last;
{
  char local_str[200];
  int rc, line = 1;
  int first = line_no;

  while ( (*scr_line[line_no]) && (line < scrn_ht-2) )       /* show screen */
      {
      line++;                           /* HERE */
      sprintf(local_str,"%s%s\n",screen[CLR_EOL],scr_line[line_no++]);
      WRITE(1,local_str);
      }

  if ( first )                      /* put [ x more ] at top */
      {                         /* of screen if necessary */
      sprintf(local_str,"[ %d more ]", first);
      PUT(1,68);
      WRITE(1,local_str);
      }

  if ( *scr_line[line_no] )           /* put [ x more ] at bottom */
      {                         /* of screen if necessary */
      sprintf(local_str,"[ %d more ]", last-line_no+1);
      PUT(line-1,68);
      WRITE(1,local_str);
      }

  return line + 1;
}

/*===========================================================================*/
validate(action,str,max,rts)
/*
**  Validate the users response to resp_meu().
**      action    - GETNUM;  expect a numeric response
**                  GETTEXT; any text is legal
**      str       - a pointer to the string containing the response
**      max       - the largest permissible line number
**      rts       - a pointer to the return string
**
**      Internal variables:
**      ln_num    - the line number entered by the user
**      rc        - the return code
**              -1; terminate and exit the program
**               0; good response given
**               1; bad response given
*/
int action,max;
char *str, *rts;
{
  int ln_num, rc;

  rc = -1;                        /* assume the exit code */
  switch ( action )
      {
      case GETNUM:
      if ( *str )
          {
          ln_num = atoi(str);
          strcpy(rts,str);
          rc = (int)((ln_num < 1) | (ln_num > max));
          }
      break;

      case GETTEXT:
      if ( !(*str) && !(*rts) ) rc = 1;   /* both are null - exit bad */
      else
          {
          rc = 0;                          /* assume good */
          if ( *str )               /* something was typed in */
          {
          if ( *str == '!' ) rc = -1;    /* if '!' then exit and quit */
          else strcpy(rts,str);         /* else copy response */
          }
          }
      break;
      }
  return rc;
}

/*===========================================================================*/
hilite(str,clear_field)
/*
**  Use the shell and terminfo entries to display the selected
**  line in standout mode at the top of the screen
*/
char *str;
int clear_field;
{
  char *wp = str;
  char buf[BUFSIZ];

  if ( clear_field )
      {
      while ( *wp == ' ' ) wp++;
      while ( *wp != ' ' ) *wp++ = ' ';
      }
  sprintf(buf,"%s%s%-79s%s\n",screen[CLR_SCR],screen[STANDOUT],str,screen[NORMAL]);
  WRITE(1,buf);
}

/*===========================================================================*/
void readkbd(str,menu,row,col)
/*
**  Wrests control of the keyboard from the kernel and processes
**  keystokes as they come in.  Only allowable keystrokes are echoed
**  back to the tube.
**
**  If menu = TRUE then the scrolling functions are legal hence the
**  up and down arrow keys should be honored.
*/
char *str;
int menu,row,col;
{
  char string[100];
  char buffer[20];
  char *ptr = buffer + 1;
  int cntnue = TRUE;
  int i, top_line = 0;

/*------------------------------------------------------------------------------

    This routine deserves some explanation.  We tried to use the curses
    library to implement our user interface, but found our code too
    dependent on the unreliable curses routines.  In order to get away
    from curses, but still have good user friendliness characteristics,
    two very significant things had to work.

        1) had to be able to address specific screen coords
        2) had to get unbuffered keystrokes

    start_cur() interogates terminfo to find out how to address
    the screen.  The command set chosen is the minimum that might be
    found on all but the dumbest of tubes.  terminfo is also asked
    for the key sequences which will be sent when certain keys are pressed.
    These sequences are saved in strings for later comparison.

    Finally, start_cur() saves the current status of the tty.  This
    status is modified as required, then restored.

    Normally, the tty will buffer keystokes so that they will only be
    passed to a read() when enter is pressed.  BUT the tty echoes them
    back to the tube anyway, so they show up on the screen before
    read() ever sees them.  That's no good for us.  We have to be
    able to respond to scroll commands, breaks, etc. ass soon as the key
    is pressed.  Therefore, the tty status is modified to not echo
    keystrokes, don't pass signal keys to the kernel, and don't wait
    til enter is pressed to send keystrokes.

    When the keyboard is polled, the read is told to look for 10 bytes.
    The tty is told to satisfy the read request with a minimum of 3 bytes
    or after one tenth of a second has elapsed since receipt of the
    last character.  When the read terminates, I should have a single
    keystroke.  That keysroke may be defined by more that one character
    (as in the case on vt220 arrows; esc[A = up).  The return code from
    the read will tell me where to put a terminating null.  I'll then
    compare the keystroke with all the legal choices and act accordingly.

    Before returning, the tty is restored.

-----------------------------------------------------------------------------*/

  CHANGE_TTY;
  *string = NULL;
  while ( cntnue )
      {
      int ch;

      nonl();
      cbreak();
      keypad(win,TRUE);
      ch = getch();

      i = 0;
      rc = TRUE;
      while ( key[i] && rc )
	   if ( key[i++] == ch )
	       rc = 0;
      i -= 1;
      keypad(win,FALSE);
      nocbreak();
      nl();
      if ((!rc              /* rc = 0 therefore a match was found */
           && (menu || (i != KEYUP_ALT && i != KEYDN_ALT))) ||
	  ( ch == '\n' ))   /* when KEY_DOWN generates '\n' we also */
			    /* have to go here */
      {
      switch ( i )
          {
          case BREAK:                       /* ^C */
          case QUIT:
          goodbye(TERMINATE);
          break;

          case ENTER:                    /* enter key */
          cntnue = FALSE;
          break;

          case KEYBS:                   /* backspace */
          if ( strlen(string) )         /* only if there is stuff */
              {                    /* to back up over */
              write(1,"\b \b",3);
              string[strlen(string)-1] = '\0';
              col--;
              }
          break;

          case KEYUP:               /* terminfo 'up' key */
          case KEYUP_ALT:              /* alternate 'up' key '-' */
          if ( menu )
              {
              top_line = align_scrn(top_line,-1);
              PUT(row,col);
              }
          *buffer = NULL;
          break;

          case KEYDN:		/* terminfo 'down' key */
          case 0xa :		/* curses is not recognizing the KEY_DOWN */
				/* when it generates a '\n' char.         */
          case KEYDN_ALT:	/* alternate 'down' key '+' */
          if ( menu )
              {
              top_line = align_scrn(top_line,1);
              PUT(row,col);
              }
          *buffer = NULL;
          break;
          }
      }
      else {
	  buffer[0] = ch;
	  buffer[1] = NULL;
	  if ( ch >= ' ' && ch < 0xfe )
	      {
	      WRITE(1,buffer);
	      strcat(string,buffer);
	      col++;
	      }
      }
      }
  RESTORE_TTY;
  strcpy(str,string);
}

/*===========================================================================*/
void goodbye(exit_code)
/*
**  Exits.  Shuts down curses.  Exit_code is dependent on PIOSMIT.
**
**  exit_code:
**      GOOD            0   success
**      BAD             1   failure
**      TERMINATE       2   terminated
*/
int exit_code;
{
    if (interactive_mode)/* TTY is restored only after an interactive session */
    {
    	RESTORE_TTY;
        NULN;
    }
    exit( getenv("PIOSMIT") ? 0 : exit_code);
}

/*===========================================================================*/
void make_files()
/*
**  Makes the paths to files and commands based on the PIOBASEDIR environment
**  variable (if present) or /usr/lib/lpd/pio (if not present).
**                        or /var/spool/lpd/pio/@local 
*/
{
    char base_dir[100], *ptr, *ptr1;
        char var_dir[100], *var_ptr;

    ptr = (char *)getenv("PIOBASEDIR");
        if (ptr != NULL)  /* use it for PIOBASEDIR and PIOVARDIR */
           {     
       strcpy(base_dir,ptr);
           }
        else  /* use default directories for both basedir and vardir */
           {
           strcpy(base_dir, PIOBASEDIR);
           }
    *(var_dir+sizeof(var_dir)-1) = 0;
    if (ptr1 = getenv("PIOVARDIR"))
       (void)strncpy(var_dir,ptr1,sizeof(var_dir)-1);
    else if (ptr)
       (void)strncpy(var_dir,base_dir,sizeof(var_dir)-1);
    else
       (void)strncpy(var_dir,PIOVARDIR,sizeof(var_dir)-1);

        sprintf(etcpath,"%s/etc/",base_dir);
    sprintf(prepath,"%s/predef/",base_dir);
    sprintf(cuspath,"%s/custom/",var_dir);
    sprintf(digpath,"%s/ddi/",var_dir);
    sprintf(digestcmd,"%s/etc/piodigest/",base_dir);
	sprintf(odmpath,"%s/smit",var_dir);

    ptr = base_dir;
    while (*ptr) ptr++;             /* move ptr to end of string */
    while (*ptr != '/') ptr--;              /* back up to last slash */
    *ptr = NULL;          /* terminate the base_dir string at the last slash */
    sprintf(backend,"%s/piobe",base_dir);

    /* create a unique temporary file name */
    sprintf(tempfile,"/tmp/tempfile.%d",getpid());
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
static char *msgbuf = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

char *getmsg(CatName, set, num)
char *CatName;  
int set;
int num;
{
	char *ptr;
	char *nlspath;
	char *defpath = malloc(200);
	char default_msg[100];

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
	
	if (def_catd == 0)
		{
		strcpy(defpath, ETCPATH);
		strcat(defpath, CatName);
		def_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	sprintf(default_msg,"Cannot access message catalog %s.\n",CatName);
	ptr = catgets(def_catd,set,num, default_msg);
	if (!msgbuf)
		msgbuf = malloc(4001);
	if (msgbuf)
		strncpy(msgbuf, ptr, 4000);

	free(defpath);
	return(msgbuf);
}
/*
*******************************************************************************
*******************************************************************************
** NAME:        get_attr_msg()
**
** DESCRIPTION: The same as the getmsg routine.  The only difference is
** that they use different catalog descriptor variables.  This routine
** is used to access attibute description catalogs, while the getmsg routine is
** always used to access piobe.cat.  The reason for get_attr_msg() is so
** piobe.cat is not being constantly opened and closed.  Eventually these
** routines should be rewritten to handle multiple catalogs opened at once.
**
*******************************************************************************
*******************************************************************************
*/
static char *attr_msgbuf = NULL;
static nl_catd attr_catd;
static nl_catd def_attr_catd;
static char save_attr_name[100];

char *get_attr_msg(CatName, set, num)
char *CatName;  
int set;
int num;
{
	char *ptr;
	char *nlspath;
	char *defpath = malloc(200);
	char default_msg[100];

	if (strcmp(CatName,save_attr_name) != 0)  /* is it a different catalog */
	{
	catclose(attr_catd);  /* close /usr/lpp message catalog */
	catclose(def_attr_catd);  /* close default message catalog */
	attr_catd = def_attr_catd = (nl_catd)NULL;  /* set catalog descriptors to NULL */
	strcpy(save_attr_name,CatName);
	}

	if (attr_catd != -1)  
		if (attr_catd == 0) /* if it hasn't been open before */
			attr_catd = catopen(CatName,NL_CAT_LOCALE);
	
	if (attr_catd != -1)
		{
		ptr = catgets(attr_catd,set,num,"dummy");
		if (!attr_msgbuf)
			attr_msgbuf = malloc(4001);
		if (attr_msgbuf)
			strncpy(attr_msgbuf, ptr, 4000);
		if (strcmp(ptr,"dummy") != 0)  /* did catgets fail? */
			return(attr_msgbuf);
		}
	
	if (def_attr_catd == 0)
		{
		strcpy(defpath, ETCPATH);
		strcat(defpath, CatName);
		def_attr_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	sprintf(default_msg,"Cannot access message catalog %s.\n",CatName);
	ptr = catgets(def_attr_catd,set,num, default_msg);
	if (!attr_msgbuf)
		attr_msgbuf = malloc(4001);
	if (attr_msgbuf)
		strncpy(attr_msgbuf, ptr, 4000);

    free(defpath);
	return(attr_msgbuf);
}
		



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vp_parsemsg                                                  *
*                                                                              *
* DESCRIPTION:    Parse a given message and fetch it, if necessary.            *
*                                                                              *
* PARAMETERS:     mp		msg                                            *
*                                                                              *
* RETURN VALUES:  fmp 		fetched msg                                    *
*                                                                              *
*******************************************************************************/
static char const *
vp_parsemsg(const char *mp)
{
   register const char		*cp;
   register const char		*sp;
   const char			*tp;
   char				*fmp		= NULL;
   char				tmp[LINE_MAX+1];
   char 			mcnm[PATH_MAX+1];
   int				setno		= MF_PIOATTR1SETNO;
   int				msgno;
   int				parseerr	= FALSE;
   size_t			maxlen;

   if (!mp || !*mp)
      return (char *)NULL;

   if (*mp != SETBEGINCHR || *(cp = mp+strlen(mp)-1) != SETENDCHR)
   {
      MALLOC(fmp,strlen(mp)+1);
      (void)strcpy(fmp,mp);
      return fmp;
   }

   (void)memcpy(tmp,mp+1,maxlen = MIN((char *)cp-(mp+1),sizeof(tmp)-1)),
   *(tmp+maxlen) = 0;
   if (!(cp = strchr(tmp,MSGSEPCHR)))
   {
      MALLOC(fmp,strlen(tmp)+1);
      (void)strcpy(fmp,tmp);
      return fmp;
   }

   if (cp == tmp)
   {
      cp++;
      MALLOC(fmp,strlen(cp)+1);
      (void)strcpy(fmp,cp);
      return fmp;
   }

   *mcnm = 0;
   do
   {
      /* Get msg no. */
      for (sp = cp-1; *sp != FLDSEPCHR; )
	 if (sp == tmp)
	    break;
	 else
	    sp--;
      if (msgno = (int)strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,
			      (char **)&tp,10), *tp != MSGSEPCHR)
      {
	 parseerr++;
	 break;
      }
      if (*sp != FLDSEPCHR  ||  sp == tmp)
	 break;

      /* Get set no. */
      for (--sp; *sp != FLDSEPCHR; )
	 if (sp == tmp)
	    break;
	 else
	    sp--;
      if (setno = (int)strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,
			      (char **)&tp,10), *tp != FLDSEPCHR)
      {
	 parseerr++;
	 break;
      }
      if (*sp != FLDSEPCHR || sp == tmp)
	 break;

      /* Get message catalog name. */
      (void)memcpy((void *)mcnm,(void *)tmp,
		    /***  COMPILER ERROR! ***/
		    /*
		    maxlen = MIN(sp-tmp,sizeof(mcnm)-1)),
		     */
		    maxlen = MIN((char *)sp-tmp,sizeof(mcnm)-1)),
      *(mcnm+maxlen) = 0;
   } while (0);

   if (parseerr || !*mcnm ||
       !(sp = vp_getmsg(mcnm,setno?setno:MF_PIOATTR1SETNO,msgno)))
   {
      cp++;
      MALLOC(fmp,strlen(cp)+1);
      (void)strcpy(fmp,cp);
   }
   else
   {
      MALLOC(fmp,strlen(sp)+1);
      (void)strcpy(fmp,sp);
   }

   return fmp;
}	/* end - vp_parsemsg() */




/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vp_getmsg                                                    *
*                                                                              *
* DESCRIPTION:    Fetch an error message and put it in a msg buffer.           *
*                                                                              *
* PARAMETERS:     catnm		catalog name                                   *
*                 setno		set no.                                        *
*                 msgno		message no.                                    *
*                                                                              *
* RETURN VALUES:  message	success                                        *
*                 NULL		failure                                        *
*                                                                              *
*******************************************************************************/
static char const *
vp_getmsg(const char *catnm,int setno,int msgno)
{
   static char			prevcatnm[FILENAME_MAX+1];
   static nl_catd		md = (nl_catd)-1;
   char				defmcpath[PATH_MAX+1];
   const char			*const dmsg = "d u m m y";
   char				*mp;

   /* Fetch the specified message. */
   if (strcmp(catnm,prevcatnm))
   {
      (void)strncpy(prevcatnm,catnm,sizeof(prevcatnm)-1),
      *(prevcatnm+sizeof(prevcatnm)-1) = 0;
      if (md != (nl_catd)-1)
         md = ((void)catclose(md), (nl_catd)-1);
   }
   if (md == (nl_catd)-1  &&
       (md = catopen((char *)catnm,NL_CAT_LOCALE)) == (nl_catd)-1)
   {
      (void)strncpy(defmcpath,DEFMC_PREFIXPATH,sizeof(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      (void)strncat(defmcpath,catnm,sizeof(defmcpath)-strlen(defmcpath)-1);
      if ((md = catopen(defmcpath,NL_CAT_LOCALE)) == (nl_catd)-1)
	 return NULL;
   }

   return strcmp(mp = catgets(md,setno,msgno,(char *)dmsg),
		 (char const *)dmsg) ? mp : NULL;
}	/* end - vp_getmsg() */


