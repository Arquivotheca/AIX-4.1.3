static char sccsid[] = "@(#)54 1.11 src/bos/usr/lib/boot/ssh.c, bosboot, bos411, 9428A410j 93/10/13 13:17:37";
/*
 * COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
 *
 * FUNCTIONS: ssh.c (boot init)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fullstat.h>
#include <errno.h>
#include <sys/termio.h>
#include <ustat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/access.h>
#include <signal.h>
#include <assert.h>
#include <sys/shm.h>
#include <sys/sysconfig.h>
#include <sys/var.h>

FILE *fp0, *fp1, *fp2;
int fd0;
struct termio tt;
int trace;
int lineo;
char *dflt_env[] = { "HOME=/", "PATH=/usr/sbin:/etc:/usr/bin::", (char *) 0 };
char **env = dflt_env;
int Xrv;
int Xerrno;
int init;

/*
* When the system is booted, very little device support is available.
* Before the system console can be accessed, a variety of device drivers
* must be loaded and a bunch of configuration stuff must be performed.
*/

#define DBG_CONSOLE	"/dev/dbgcons"
#define	CONSOLE		"/dev/console"
#define	NO_CONSOLE	"/dev/null"

#define	RCBOOT		"/sbin/rc.boot"
#define	SYSINIT		"/sbin/rc.boot 1"
#define	BRC		"/sbin/rc.boot 2"
#define	SHELL		"/usr/bin/sh"

FILE *coninit();

#define	INTERACTIVE	1
#define	BATCH		2

/* return values from icmd */

#define	ICMD		0
#define	XCMD		1

#define	SUCCESS		0
#define	FAILURE		1

/* special return values from doit */

#define	EFATAL		(NSIG+1)

static get_ctl_term = 0;	/* get a controlling terminal */

main(argc,argv,envp)
char **argv, **envp;
{
	FILE *fp;
	int rc, done;

	if( envp )
		if( *envp )
			env = envp;

	init = getpid() == 1 ? 1 : 0;

	/*
	* Check to see if base file descriptors are already open
	* if not, then open /dev/console.
	*/

	if( init )
	{
		/*
		* Perform any boot time initializations before
		* attempting to access the console.
		*/
		fp = coninit(DBG_CONSOLE);

		if( access(RCBOOT,X_OK) == 0 )
		{
			fprintf(fp,"\nINIT: EXECUTING %s\n", SYSINIT);

			if( execute(SYSINIT) )
			{
				fprintf(fp,"\nINIT: %s failed\n", SYSINIT);
				exit(1);
			}
		}
		fclose(fp);

		fp = coninit(CONSOLE);

		errno = 0;

		/*
		* If brc returns then BRC exited with a non-zero status,
		* attempt to start a shell on the console.
		*/

		if( access(RCBOOT,X_OK) == 0 ) brc(BRC,fp);
		done = 0;
		/* set up stdin (fp0), stdout(fp1), stderr(fp2) for doit() */
		fp0 = fp;
		fp1 = fp;
		fp2 = fp;
		while( !done )
		{
			fprintf(fp,"\nINIT: SINGLE USER MODE\n");
			get_ctl_term = 1;
			if( (rc = doit(SHELL)) == EFATAL )
				done = 1;
		}

		fprintf(fp,"\nINIT: FATAL ERROR IN %s\n", SHELL);

		/*
		* If a shell cannot be started on the console,
		* then attempt to go into interactive mode.
		*/

	}
	else
	{
		fp0 = stdin;
		fp1 = stdout;
		fp2 = stderr;

		if( argc > 1 )	/* support ssh -c command ala the shell */
			if( *argv[1] == '-' && *(argv[1]+1) == 'c' )
				return(doit(argv[2]));
	}

	loop(fp0,INTERACTIVE);
}

loop(fp,mode)
FILE *fp;
int mode;
{
	char buf[256], *p;

	fprintf(fp1,"XIX s-shell");
	for( lineo = 0; ; mode == INTERACTIVE ? lineo : lineo++ )
	{
		fprintf(fp1,"\n# ");
		fflush(fp1);

		for( p = buf; p < &buf[256]; p++ )
			*p = '\0';

		if( fgets(buf,256,fp) == NULL )
			return SUCCESS;

		/*
		if( mode != INTERACTIVE )
			fprintf(fp1,"%s",buf);
		*/

		for( p = buf; *p; p++ )
			if( *p == '\n' || *p == '\r' )
			{
				*p = '\0';
				break;
			}

		if( *buf != '\0'  && *buf != '#' )
			if( doit(buf) != 0 && mode != INTERACTIVE )
				return FAILURE;

	}
}

char *args[80];
int nargs;
char fname[80];

doit(ptr)
char *ptr;
{
	int pid, epid, waitloc, r, ic, weird;
	char **cp;

	for( pid = 0; pid < 80; pid++ )
		args[pid] = (char *) 0;

	for( nargs = 0, cp = args; *ptr; ptr++ )
	{
		if( !*cp )
			*cp = ptr;

		if( *ptr == ' ' )
		{
			*ptr = '\0';
			cp++;
			nargs++;
		}
	}
	cp++;
	*cp = (char *) 0;

	r = icmd(nargs,args,&ic);

	if( ic == ICMD )
		return r;

	switch( pid = fork() )
	{
	case 0:
	    	if (init) {
		    /* terminal driver sends signals generated from
		       the keyboard to the foreground process group.
		       A child process that opens its terminal should
		       ensure the terminal driver sends no signals
		       to the init process.  setsid() puts the child
		       in its own process group. */
		    setsid();

		    if (get_ctl_term) {
			FILE *fp;

			get_ctl_term = 0;

			/* setup the console as the controlling terminal */
			fp = coninit(CONSOLE);

			/* ignore errors */
			errno = 0;
		    }
		}

		if( *args[0] != '/' && *args[0] != '.' )
		{
			/* try paths */

			strcpy(fname,"/usr/sbin/");
			strcat(fname,args[0]);
			execve(fname,args,env);
			if( errno != ENOENT )
				goto badexec;

			strcpy(fname,"/etc/");
			strcat(fname,args[0]);
			execve(fname,args,env);
			if( errno != ENOENT )
				goto badexec;

			strcpy(fname,"/usr/bin/");
			strcat(fname,args[0]);
			execve(fname,args,env);
			if( errno != ENOENT )
				goto badexec;
		}
		else
		{
			strcpy(fname,args[0]);
			execve(fname,args,env);
			if( errno != ENOENT )
				goto badexec;
		}

		fprintf(fp1,"%s not found\n", args[0]);
		_exit(EFATAL);

		badexec:
		fprintf(fp1,"execve %s failed, errno = %d\n", fname, errno);
		_exit(EFATAL);

	case -1:
		fprintf(fp1,"fork failed, errno = %d\n", errno);
		r = EFATAL;
		break;
	default:
		epid = -1;
		while (epid != pid)
		{
			waitloc = 0;
			epid = wait(&waitloc);
		}

		Xrv = waitloc;

		if( waitloc )
		{
			int v;
			char *msg;

			if( (waitloc & 0x7f) == 0x7f )
			{
				msg = "stopped by signal";
				v = waitloc & 0x7f;
				weird = 1;
				r = EFATAL;
			}
			else if( waitloc & 0xff )
			{
				msg = "killed by signal";
				v = waitloc & 0x7f;
				weird = 1;
				r = EFATAL;
			}
			else
			{
				msg = "exited with status";
				v = waitloc >> 8;
				weird = 0;
				r = v;
			}
			if( trace || weird )
				fprintf(fp1,"%s %s %d\n", args[0], msg, v);
		}
		else
			r = 0;
		break;
	}

	return r;
}

xcmp(a,b)
char *a, *b;
{
	while( *a++ == *b++ )
		if( *(a-1) == '\0' )
			return 0;

	return 1;
}

atoo(p)
char *p;
{
	int base, v = 0;

	do
	{
		v <<= 3;
		switch( *p )
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8':
			base = (int) '0';
			break;
			break;
		default:
			return -1;
		}
		v += (int) *p - base;
	}
	while( *++p );

	return v;
}

atoh(p)
char *p;
{
	int base, v = 0;

	do
	{
		v <<= 4;
		switch( *p )
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			base = (int) '0';
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			base = (int) 'a';
			break;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			base = (int) 'A';
			break;
		default:
			return -1;
		}
		v += (int) *p - base;
	}
	while( *++p );

	return v;
}

/*
* System V basic system calls
*/

int Xaccess();
int Xclose();
int Xchmod();
int Xdup();
int Xexec();
int Xexit();
int Xfork();
int Xgetpid();
int Xioctl();
int Xkill();
int Xopen();
int Xwait();
int Xunlink();
int Xtelinit();

/*
* psuedo functions
*/
int Xprintf();

struct ix
{
	char *		cmdname;
	int (*		func)();
	int		argcount;
	int		return_rv;
};

struct ix ix[] =
{
	{"access",	Xaccess,2,				},
	{"close",	Xclose,1,				},
	{"chmod",	Xchmod,2,				},
	{"dup",		Xdup,1,					},
	{"exec",	Xexec,-1,				},
	{"exit",	Xexit,1,				},
	{"fork",	Xfork,0,				},
	{"getpid",	Xgetpid,1,				},
	{"ioctl",	Xioctl,3,				},
	{"kill",	Xkill,2,				},
	{"open",	Xopen,2,				},
	{"wait",	Xwait,1,				},
	{"unlink",	Xunlink,1,				},

	{"printf",	Xprintf,-1,				},
	{"telinit",	Xtelinit,1,				},
	{ (char *) 0,		 				},
};

icmd(argc,argv,flagp)
char **argv;
int *flagp;
{
	struct ix *ixp;
	int r;

	for( ixp = ix; ixp->cmdname; ixp++ )
		if( xcmp(ixp->cmdname,argv[0]) == 0 )
		{
			*flagp = ICMD;
			r = (*ixp->func)(argc,argv);
			if( ixp->return_rv )
				return r;
			else
				return SUCCESS;
		}
	*flagp = XCMD;
	return FAILURE;
}


#define	MODULE(X)	X(argc,argv)\
			int argc;\
			char **argv;

#define	EMOD(X)	X(){fprintf(fp1,"Not implemented\n"); fflush(fp1);}


MODULE(Xprintf)
{
	int arg2;

	arg2 = atoh(argv[2]);
	/* sscanf(argv[2],"%x",&arg2); */
	fprintf(fp1,argv[1],arg2);
}


#define	FMT_STRING "%.8x: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %s\n"


MODULE(Xaccess)
{
	int amode;

	amode = atoi(argv[2]);
	Xrv = access(argv[1],amode);
	logx("access(%s,%d) = %d", argv[1], amode, Xrv);
}

MODULE(Xchmod)
{
	int mode;

	mode = atoo(argv[2]);
	Xrv = chmod(argv[1],mode);
	logx("chmod(%s,0%o) = %d", argv[1], mode, Xrv);
}

MODULE(Xclose)
{
	int fd;

	fd = atoi(argv[1]);

	if( (fd == 0 || fd == 1 || fd == 2) && lineo == 0 )
	{
		fprintf(fp2,"Can't close that\n");
		return 1;
	}

	Xrv = close(fd);
	logx("close(%d) = %d", fd, Xrv);
}

MODULE(Xdup)
{
	int fd;

	fd = atoi(argv[1]);
	Xrv = dup(fd);
	logx("dup(%d) = %d", fd, Xrv);
}

MODULE(Xexec)
{
	Xrv = execve(argv[1],argv+1,env);
	logx("execve(%s,...) = %d\n", argv[1], Xrv);
}

MODULE(Xexit)
{
	int ev;

	ev = atoi(argv[1]);
	exit(ev);
}


EMOD(Xfork)

MODULE(Xgetpid)
{
	Xrv = getpid();
	logx("getpid() = %d", Xrv);
}


EMOD(Xioctl)

MODULE(Xkill)
{
	int pid, signo;

	pid = atoi(argv[1]);
	signo = atoi(argv[2]);
	Xrv = kill(pid,signo);
	logx("kill(%d,%d) = %d", pid, signo, Xrv);
}


MODULE(Xopen)
{
	int mode;

	mode = atoi(argv[2]);
	Xrv = open(argv[1],mode);
	logx("open(%s,%d) = %d", argv[1], mode, Xrv);
}

MODULE(Xunlink)
{
	int fd;
	char *buff;
	struct stat sbuf;

	if (stat(argv[1], &sbuf))
	{
		return 2;
	}

	fd = open(argv[1], O_RDWR|O_EXCL, 0);
	if (fd == -1)
	{
		return 3;
	}

	buff = shmat(fd, 0, SHM_MAP);
	assert((int)buff != -1);

	bzero(buff, sbuf.st_size);
	fsync(fd);
	close(fd);

	Xrv = unlink(argv[1]);
	logx("unlink(%s) = %d", argv[1], Xrv);
}


MODULE(Xtelinit)
{
        int Xrv;
	struct var newv;
	char  c;

        Xrv = sysconfig(SYS_GETPARMS, &newv, sizeof(struct var));
	c = argv[1][0];
	if( c == 's' || c == 'S' || c == 'm' || c == 'M' ) {
		strncpy(&newv.v_initlvl,argv[1], 4);	
	}
	else {
		strcpy(&newv.v_initlvl,"");	
	}
	Xrv = sysconfig(SYS_SETPARMS, &newv, sizeof(struct var));
    	logx("telinit(%s) = %d", argv[1],Xrv);       
}









EMOD(Xwait)

/* AIX 2.2.1 or 3.x specific */


logx(fmt,arg1,arg2,arg3,arg4,arg5)
{
	Xerrno = errno;

	fprintf(fp1,fmt,arg1,arg2,arg3,arg4,arg5);

	if( Xerrno )
		fprintf(fp1,", errno = %d\n", Xerrno);
	else
		fprintf(fp1,"\n");

	errno = 0;
}

/*
* Execute the BRC if it exists.  If it executes
* successfully, then exit.  This will hopefully
* cause the system to be restarted.
*/

brc(pgm,fp)
char *pgm;
FILE *fp;
{
	int rc;

	fprintf(fp,"\nINIT: EXECUTING %s\n", pgm);

	if( execute(pgm) == 0 )
	{
		fprintf(fp, "\nINIT: EXITING\n");

		/* terminate all process */
		kill(-1,9);

		/* pick up those zombies */
		while( wait(&rc) != -1 && errno != ECHILD )
			;
		exit(0);
	}
}

execute(pgm)
char *pgm;
{
	int pid, wstat;
	FILE *fp;

	switch( pid = fork() )
	{
	case -1:
		wstat = errno;
		break;
	case 0:
		if (init) {
		/* terminal driver sends signals generated from
		   the keyboard to the foreground process group.
		   A child process that opens its terminal should
		   ensure the terminal driver sends no signals
		   to the init process.  setsid() puts the child
		   in its own process group. */
		    setsid();	/* become session and process group leader */
		fp = coninit(CONSOLE);
		}
		execl(SHELL,"sh","-c",pgm,0);
		for(;;)
			_exit(errno);	/* EAGAIN? */
	default:
		while(wait(&wstat) != pid)
			;
		break;
	}
	return wstat;
}

/*
* This zero filled FILE struct will cause fprintf to fail
* w/o error.  This is used when coninit can't open the
* requested device, usually due to a device error.
*/

FILE nofile;

FILE *
coninit(dev)
char *dev;
{
	int rc;
	FILE *fp;

	close(0); close(1); close(2); errno = 0;

	if( open(dev,O_RDWR) != 0 )	/* stdin */
		return &nofile;

	if( dup(0) != 1 )		/* stdout */
	{
		close(0);
		return &nofile;
	}

	if( dup(0) != 2 )		/* stderr */
	{
		close(0);
		close(1);
		return &nofile;
	}

	if( (fp = fdopen(1,"r+")) == NULL )
	{
		close(0);
		close(1);
		close(2);
		return &nofile;
	}
	return fp;
}
