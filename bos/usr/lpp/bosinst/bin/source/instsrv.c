static char sccsid[] = "@(#) 75 1.2 src/bos/usr/lpp/bosinst/bin/source/instsrv.c, bosinst, bos411, 9428A410j 94/02/24 15:37:17";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: instsrv - network install service server daemon
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/param.h>

#define OPTIONLIST "r:"

/* number of items server adds to environment before exec'ing */
#define NUMBER_SERVER_ENV_ENTRIES 1

#define SHELLPATH "/bin/sh"     /* shell to interpret incoming command */
#define SHELLNAME "sh"          /* shell name */
#define SHELLFLAG "-c"          /* shell uses:    sh -c CMD */
#define TESTSTRING "`;&|^\n"    /* characters that can separate commands */

#define USAGEMSG "usage: %s [-r reportfile] safe_directory_name\n",progname

int netserv();                     /* Handles inetd inited servers     */
int getsize();
int getstuff();
char **setup_environ();
int report();
int start_shell();
char *mkstr();
void ealarm(void);

/*
 * netsconn - This structure is filled in by the routine netserv.  It
 *            contains everything known about the request passed by
 *            inetd.
 */

struct netsconn {
   void *context;                   /* Used for session management */
   unsigned short port;             /* Port to receive on */
   struct sockaddr_in sin;          /* The socket to be filled in */
};

struct netsconn data;               /* Where to put network related data */
static jmp_buf failjmpbuf;          /* Used for timeouts */
char *progname;                     /* argv[0] for functions */
char *safedir;                      /* argv[1] for functions */
FILE *reportfile = NULL;	/* where to print report message to */

main(argc,argv,envp)
   int argc;
   char *argv[];
   char *envp[];
{
   char *cmdmsg;		/* command passed from client */
   char *envmsg;		/* environment passed from client */
   char **newenvp;		/* environment to pass to cmd */
   int filetype;		/* is argv[1] a dirname? */
   int flag;			/* used with getopt processing */
   int msgsize;			/* how much are we to read in? */
   int usageflag;		/* invoked wrong? */
   struct stat statbuf;		/* for testing if argv[1] is a direct */
   FILE *crisisfp;		/* for printing invocation errors */

   progname = argv[0];		/* save for later err msgs */
   usageflag = 0;		/* assume no usage errors */

   opterr = 0;			/* turn off getopt error message */
   while((flag = getopt (argc, argv, OPTIONLIST)) != EOF) {
      switch(flag) {
      case 'r': if ((reportfile = fopen(optarg,"a")) == NULL) {
                   if ((reportfile = fopen("/dev/console","w")) != NULL)
                      fprintf(reportfile,
   "%s: can't open report file '%s' as specified in /etc/inetd.conf\n",
                     progname,optarg);
                }
                break;
      default:  break;
      }
   }

   if(optind != argc - 1)
      usageflag++;
   else {
      if(stat(argv[optind], &statbuf) < 0) {
         if ((crisisfp = fopen("/dev/console","w")) != NULL)
            fprintf(crisisfp,"%s: can't stat '%s'\n", progname,argv[optind]);
         exit(112);
      }
      else {
         /* extract file description bits */
         filetype = statbuf.st_mode & S_IFMT;


         if(filetype != S_IFDIR) {		/* is file a directory? */
            if ((crisisfp = fopen("/dev/console","w")) != NULL) {
               fprintf(crisisfp, "%s: '%s' is not a directory\n",
                  progname,argv[optind]);
            }
            exit(111);
         }
      }
   }

   if(usageflag) {
      if ((crisisfp = fopen("/dev/console","w")) != NULL)
         fprintf(crisisfp,USAGEMSG);
      exit(113);
   }

   /*
    * cd to P_tmpdir early in main() so if need be, su can
    * kill -11 us to get us to dump core, and we will be in
    * a writable directory.
    */
   if(chdir(P_tmpdir) < 0) {	/* P_tmpdir is defined in stdio.h */
      sprintf(cmdmsg,"%s: can't chdir to '%s'",progname,P_tmpdir);
      perror(cmdmsg);
   }

   safedir = argv[optind];

   bzero((char *)&data,sizeof(data));
   netserv(&data,0,0);

   msgsize = getsize();
   envmsg = (char *)malloc(msgsize + 1);
   if(envmsg == (char *)NULL)
      exit(12);

   /* loop around read because we're reading from socket... */
   if (getstuff(envmsg,msgsize) < 0)
      exit(12);
   envmsg[msgsize] = 0;

   newenvp = setup_environ(envp,envmsg);

   msgsize = getsize();
   cmdmsg = (char *)malloc(msgsize + 1);
   if(cmdmsg == (char *)NULL)
      exit(12);

   /* loop around read because we're reading from socket... */
   if (getstuff(cmdmsg,msgsize) < 0)
      exit(12);
   cmdmsg[msgsize] = 0;

   if(reportfile)
      report(inet_ntoa(data.sin.sin_addr),cmdmsg);
   start_shell(cmdmsg,newenvp);
   exit(1);        /* should never get here, start_shell should exec out */
}

int netserv(datap,inet,disc)
struct netsconn *datap;		/* Pointer to data are to fill */
short inet;			/* 0=called from inet, 1=cmnd line */
short disc;			/* 0=don't disconnect, 1=disconnect */
{
   int size;			/* Holds common structure sizes */
   struct sockaddr_in ssin;	/* Temporary socket structure */

   /* Do some basic checking.  If inet, there should be no disc */
   if (inet & disc)		/* Did they goof? */
      return(-1);		/* YES - we can't run then */

   if (!datap)			/* Is there a data structure? */
      return(-1);

   /* If they want to disconnect from the terminal do it now */
   /* Should we lose the terminal? */
   if (disc) {			/* YES - Ok, end stdin,out,err */
      int i,term;		/* We need some help */
      for (i=0;i<3;i++)		/* Close first 3 handles */
         close(i);		/* stdin, stdout, & stderr */
      open("/",O_RDONLY);	/* Setup funny file handles */
      dup2(0,1);		/* for all 3 stdxxx's */
      dup2(0,2);
      term = open("/dev/tty",O_RDWR);
      /* Is there a terminal associated? */
      if (term>0) {		/* YES - Ok, get rid of it then */

         /* Disassociate tty from process */
         ioctl(term,TIOCNOTTY,NULL);
         close(term);
      }
   }

   /* Find out where the peer is connecting to us from */
   size = sizeof(struct sockaddr_in);
   if (getpeername(0,&datap->sin,&size) < 0)
      return(-1);

   /* Get the socket name on the other side too */
   size = sizeof(struct sockaddr_in);
   if (getsockname(0,&ssin,&size) < 0)
      return(-1);
   datap->port = ntohs(ssin.sin_port);

   /* Ok, everything worked.  Return with a big nought grin */
   return(0);
}

getsize()
{
   int acc = 0;
   char c;
   /* read to first blank */
   while(read((int)0,(char *)&c,(unsigned)1) && !isspace((int)c))
      acc = acc * 10 + c - '0';   /* accumulate integer */

   return(acc);
}

/* getstuff - Read a buffer of length passed, or die trying */
int getstuff(where,len)
char *where;			/* Where to read from stdin too */
int len;			/* How much to get */
{
   void (*esig)(int);		/* In case there is an existing signal */
/*   int (*esig)(void);		This is the original declaration */
   register int i,ret;
   char *p;			/* Used to index into buffer */

   /* Establish a time out handler */
   esig=signal(SIGALRM,(void (*)(int))ealarm);
/* esig=(int (*)(void))signal(SIGALRM, (void (*)(int))ealarm);original call */
   if(setjmp(failjmpbuf))
      return(-1);

   /* Go into a loop waiting for 'len' of data */
   for (p = where, i = 0; i < len; i += ret) {
      alarm((unsigned)60);
      if ((ret = read(0, p, (unsigned) len - i)) < 0) {
         alarm((unsigned)0);
         signal(SIGALRM, (void (*)(int))esig);
         return(-1);
      }
      p += ret;
      if (ret == 0)
         break;
   }
   alarm((unsigned)0);
   *p = 0;
   signal(SIGALRM, (void (*)(int))esig);
   return(i);
}

char **setup_environ(envp,buf)
char **envp;
char *buf;
{
   int i;
   int j;
   int entries;
   char *cp;
   char **envpout;
   char **servnewenv;
   char workbuf[128];

   servnewenv = (char **)calloc(NUMBER_SERVER_ENV_ENTRIES + 1,
       sizeof(char *));
   
   if(servnewenv == NULL)
      exit(1);

   sprintf(workbuf,"Servers_Client_IP=%s",inet_ntoa(data.sin.sin_addr));
   servnewenv[0] = mkstr(workbuf);
   servnewenv[1] = 0;

   /* find number of env entries in env inherited from inetd */
   for(entries = 0; envp[entries]; entries++)
      ;

   /* add number of entries added to env by server */
   entries += NUMBER_SERVER_ENV_ENTRIES;

   /* add number of env entries passed by client */
   for(cp = buf; *cp ; cp++)
      if(*cp == '\n')
         entries++;

   envpout = (char **)calloc(entries + 1,sizeof(char *));

   if(envpout == NULL)
      exit(1);

   envpout[0] = buf;
   for(i = 1, cp = buf; *cp; cp++)
      if(*cp == '\n') {
         *cp = 0;
         envpout[i] = cp + 1;
         i++;
      }
   i--;
   for(j = 0; servnewenv[j]; i++, j++)
      envpout[i] = servnewenv[j];

   for(j = 0; envp[j]; i++, j++)
      envpout[i] = envp[j];
   envpout[i] = 0;

   return(envpout);
}

report(clientip,cmd)
   char *clientip;
   char *cmd;
{
   long curtime;
   struct tm *ts;

   time(&curtime);
   ts = localtime(&curtime);

   fprintf(reportfile,"%s: %02d/%02d/%02d %02d:%02d:%02d - %s: %s\n",
         progname,
         ts -> tm_year,
         ts -> tm_mon + 1,
         ts -> tm_mday,
         ts -> tm_hour,
         ts -> tm_min,
         ts -> tm_sec,
         clientip,
         cmd);
   fclose(reportfile);
}

start_shell(readbuf,environment)
char *readbuf;
char **environment;
{
   char *buf;
   char *cp;
   char normalpath[MAXPATHLEN];

      /* newargv[0] = name of real shell */
      /* newargv[1] = "-c" flag to shell */
      /* newargv[2] = shell command line */
      /* newargv[3] = trailing null pointer */
   char *newargv[4];

   buf = (char *)malloc(strlen(readbuf) + strlen(safedir) + 3);

   if(buf == (char *)NULL)
      exit(1);


   for(cp = TESTSTRING; *cp; cp++)
      if(strchr(readbuf,(int)*cp)) {
         fprintf(stderr,
             "%s: illegal character '%c' in command line '%s'\n",
             progname, *cp, readbuf);
         exit(1);
      }

   if((strncmp(readbuf,"../",(size_t)3) == 0) || strstr(readbuf,"/../")) {
      fprintf(stderr,
   "%s: illegal backwards reference in command line '%s'\n",
         progname, readbuf);
      exit(1);
   }

   sprintf(normalpath,"%s/%s",safedir,readbuf);
   for(cp = normalpath; *cp && !isspace(*cp); cp++)
      ;
   *cp = 0;

   if(access(normalpath,F_OK | X_OK)) {    /* can we execute the file? */
      fprintf(stderr,
          "%s: can't access file '%s'\n",progname,normalpath);
      exit(1);
   }

   newargv[0] = SHELLNAME;
   newargv[1] = SHELLFLAG;

   sprintf(buf,"%s/%s",safedir,readbuf);

   newargv[2] = buf;       /* now arg's == "/bin/sh -c cmd" */
   newargv[3] = 0;

   execve(SHELLPATH,newargv,environment);
   perror("execv");
   exit(1);
}

char *mkstr(str)
char *str;
{
   char *cp = (char *)malloc(strlen(str) + 1);
   if(cp == (char *)NULL)
      exit(1);
   strcpy(cp,str);
   return(cp);
}

/*
 * ealarm - Handles the alarm signal by doing a longjump to
 *          the global failjmpbuf.
 */
void ealarm(void)
{
   longjmp(failjmpbuf,1);
}
