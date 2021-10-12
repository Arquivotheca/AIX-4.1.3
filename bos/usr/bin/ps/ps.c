static char sccsid[] = "@(#)75	1.79.1.34  src/bos/usr/bin/ps/ps.c, cmdstat, bos41J, 9508A 2/20/95 13:29:26";
/*
 * COMPONENT_NAME: (CMDSTAT) Process Status
 *
 * FUNCTIONS: ps, proc_sort, psort
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#define _ILS_MACROS
#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/cred.h>
#ifdef _THREADS
#include <sys/pri.h>
#endif /*_THREADS */
#include <procinfo.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/proc.h>
#ifdef _THREADS
#include <sys/thread.h>
#include <sys/uthread.h>
#endif /*_THREADS */
#include <errno.h>
#include <locale.h>
#include <dirent.h>
#include <sys/m_param.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <nl_types.h>
#include <langinfo.h>
#include <string.h>
#include <mbstr.h>
#include <sys/priv.h>
#include <sys/id.h>
#include <sys/vnode.h>  /* For checking a multiplexed character device */
#ifdef _THREADS
#include <sys/systemcfg.h>
#endif /*_THREADS */


/*
         using KSTACK constant now...
#include <sys/pseg.h>
*/


#include "ps_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_PS,Num,Str)
static nl_catd catd;

#ifndef _THREADS
#define STRUCT_PROCINFO struct procinfo
#define USERSIZE        sizeof (struct userinfo)
#define U_SIZE          sizeof (struct user)
#define UICREDSIZE      sizeof (struct uicredinfo)
#else	/*_THREADS */
#define STRUCT_PROCINFO struct procsinfo
#define THRDSIZE	sizeof (struct thrdsinfo)
#define NOTBOUND	4294967295	/* = 0xffffffff */
#endif /*_THREADS */	

#define PROCSIZE        sizeof (STRUCT_PROCINFO)
#define UBSIZE          512
#define KBLOCKS         1024
#define PagetoKBLOCKS(i)        ((((i) * bytesperpage) / (KBLOCKS)) + \
                                    (((i) * bytesperpage) % KBLOCKS ? 1 : 0))
/* Convert a digit character to the corresponding number */

#ifdef _THREADS
#define THREAD		"user pid ppid tid st cp pri scount nwchan flag tty bnd args"
#define PI_CPU		processft->pift_cpu
#define PI_PRI		processft->pift_pri
#define PI_FLAG		process->pi_flags
#define PI_STAT		"O???IZTAW?"[(int)process->pi_state]
#define PI_STATUS	pi_state
#define PI_WCHAN	processft->pift_wchan
#define PI_WTYPE	processft->pift_wtype
#define PI_TRSS		(process->pi_trss == -1 ? 0 : process->pi_trss)
#define PI_DRSS		(process->pi_drss == -1 ? 0 : process->pi_drss)
#define PI_RSS          (PI_DRSS + PI_TRSS)    
#define PI_START	process->pi_start
#define PI_TIME		(process->pi_ru.ru_utime.tv_sec + process->pi_ru.ru_stime.tv_sec)
#define PI_RLIMIT_C	process->pi_rlimit[RLIMIT_RSS].rlim_cur
#define PI_RLIMIT_M	process->pi_rlimit[RLIMIT_RSS].rlim_max
#define PI_DVM		process->pi_dvm
#define PI_TSIZE	process->pi_tsize
#define PI_PRM		process->pi_prm
#define PI_COMM		process->pi_comm
#define PI_TTYP		process->pi_ttyp
#define PI_TTYD		process->pi_ttyd
#define PI_TTYMPX	process->pi_ttympx
#define PI_SCOUNT	processft->pift_scount
#else	/*_THREADS */
#define	PI_CPU		process->pi_cpu
#define PI_PRI		process->pi_pri
#define PI_FLAG		process->pi_flag
#define PI_STAT		"OS?RIZTT???"[(int)process->pi_stat]
#define PI_STATUS	pi_stat
#define PI_WCHAN	process->pi_wchan
#define PI_WTYPE	process->pi_wtype
#define PI_TRSS		(uinfo->ui_trss == -1 ? 0 : uinfo->ui_trss)
#define PI_DRSS		(uinfo->ui_drss == -1 ? 0 : uinfo->ui_drss)
#define PI_RSS          (PI_DRSS + PI_TRSS)    
#define PI_START	uinfo->ui_start
#define PI_TIME		(uinfo->ui_ru.ru_utime.tv_sec + uinfo->ui_ru.ru_stime.tv_sec)
#define PI_RLIMIT_C	uinfo->ui_rlimit[RLIMIT_RSS].rlim_cur
#define PI_RLIMIT_M	uinfo->ui_rlimit[RLIMIT_RSS].rlim_max
#define PI_DVM		uinfo->ui_dvm
#define PI_TSIZE	uinfo->ui_tsize
#define PI_PRM		uinfo->ui_prm
#define PI_COMM		uinfo->ui_comm
#define PI_TTYP		uinfo->ui_ttyp
#define PI_TTYD		uinfo->ui_ttyd
#define PI_TTYMPX	uinfo->ui_ttympx
#endif /*_THREADS */

#define         B_AFLAG         (1<<0)
#define         B_CFLAG         (1<<1)
#define         B_EFLAG         (1<<2)
#define         B_GFLAG         (1<<3)
#define         B_LFLAG         (1<<4)
#define         B_NFLAG         (1<<5)
#define         B_SFLAG         (1<<6)
#define         B_TFLAG         (1<<7)
#define         B_UFLAG         (1<<8)
#define         B_VFLAG         (1<<9)
#define         B_WFLAG         (1<<10)
#define         B_XFLAG         (1<<11)
#define         B_ONEPROC       (1<<12)

#define DBIT(exp)       if (jul) exp

#define         KSTACK  0		/* Processes no longer have
					   got any stacks */
static int berk_flags=0;

static char *terminal;
static int wide_out=0;
static int berk=0,jul=0;
static pid_t number;
static int     screen_size=80;
static int	screen_cols;

static STRUCT_PROCINFO *getprocdata();
static void    parseberkeley(), getarg(), getfieldlen(), print_line(); 
static void    des_build(), getgids(), przom(), prstime();
static char    *getwaittype(), *tty_hardway(), *gettty(), *mktimestr(), *strpad();
static size_t  dispwidth(), mkwcstr();

static struct	proc_cpu *proc_sort();
static void	p_sort();
static void 	prblank();

#ifdef _THREADS
static void	prthread(), prfmt_thread();
static struct thrdsinfo *getthreaddata();
static struct proc_from_thread *getprocftdata();
static char thread_state();
#endif /*_THREADS */

#define MAX_LST 128       /* Maximum entries allowed in argument lists */
#define ARGSIZ  LINE_MAX  /* Maximum size of command arguments */
#define WTSIZ   80
#define TIMESIZ 20
#define AMAX	LINE_MAX

#ifndef MAXLOGIN
#define MAXLOGIN        8
#endif /* MAXLOGIN */

/* Structure for storing user info */
struct udata {
        uid_t uid;              /* numeric user id */
        char name[MAXLOGIN];    /* char user id, not always null terminated */
};

static char udnamebuf[MAXLOGIN + 4];	/* used when name length == MAXLOGIN */

/* Structure for storing group info */
struct gdata {
        gid_t gid;              /* numeric group id */
        char name[MAXLOGIN+1];  /* char group id, not always null terminated */
};

/* Structure for format argument */
/* The position of field are;                                   */
/*    fmtdata[0] fmtdata[1] fmtdata[2] ...... fmtdata[MAX_LST - 1]  */
static int     ftitle;
struct fmtdata {
        int  fieldid;           /* field id                     */
        int  colsize;           /* column size field            */
        char *titlename;        /* title name for printing      */
};

struct titles {
        int     fieldid;        /* field id                     */
        char    *var_name;      /* Variable Name                */
        int     msg_id;         /* MSG ID for header string     */
        char    *def_name;      /* Default Header               */
};

struct proc_cpu {
	STRUCT_PROCINFO *process;
	double pcpu;
};

#ifdef _THREADS
struct proc_from_thread {
	unsigned long	pift_pri;
	unsigned long	pift_scount;
	unsigned long	pift_wchan;
	unsigned long	pift_wtype;
	unsigned long	pift_cpu;	
};
#endif /*_THREADS */

#define H_NOMSG 9999
#define SPACE   " "
#define ARGS    0x00
#define COMM    0x01
#define F_ETIME 0x02
#define GROUP   0x03
#define NICE    0x04
#define PCPU    0x05
#define PGID    0x06
#define PID     0x07
#define PPID    0x08
#define RGROUP  0x09
#define RUSER   0x0a
#define TIME    0x0b
#define TTY     0x0c
#define USER    0x0d
#define VSZ     0x0e
#define START   0x12
#define GID     0x13
#define PRI     0x14
#define PMEM    0x15
#define RGID    0x19
#define RUID    0x1a
#define UID     0x1d
#define RSS     0x1e
#define LOGNAME 0x2a
#define FLAG    0x31
#define STATUS  0x32
#define CP      0x33
#define PAGEIN  0x34
#define WCHAN   0x35
#define NWCHAN  0x36
     
#ifdef _THREADS
#define TID     0x37
#define SCOUNT  0x38
#define BIND    0x39
#define SCHED   0x3a
#define THCOUNT 0x3b
#define ST      0x3c
#endif /*_THREADS */
    
#define PAD_ID  0xff
#define ENDID   0xffffffff
static struct titles tnames[] = {
        { ARGS,  "args",  H_COMMAND, "COMMAND" },       /* */
        { COMM,  "comm",  H_COMMAND, "COMMAND" },       /* */
        { COMM,  "command",H_COMMAND,"COMMAND" },       /* */
        { COMM,  "ucomm", H_COMMAND,"COMMAND" },        /* */
        { F_ETIME, "etime", H_ELAPSED, "ELAPSED" },     /* */
        { GROUP, "group", H_GROUP,   "GROUP"   },       /* */
        { GROUP, "gname", H_GROUP,   "GROUP"   },       /* */
        { GID,   "gid",   H_NOMSG,   "GID"     },       /* */
        { NICE,  "nice",  H_NI,      "NI"      },       /* */
        { PRI,   "pri",   H_PRI,     "PRI"     },       /* */
        { NICE,  "ni",    H_NI,      "NI"      },       /* */
        { PCPU,  "pcpu",  H_PCPU,    "%CPU"    },       /* */
        { PMEM,  "pmem",  H_PMEM,    "%MEM"    },       /* */
        { PGID,  "pgid",  H_PGID,    "PGID"    },       /* */
        { PID,   "pid",   H_PID,     "PID"     },       /* */
        { PPID,  "ppid",  H_PPID,    "PPID"    },       /* */
        { RGROUP,"rgroup",H_RGROUP,  "RGROUP"  },       /* */
        { RGROUP,"rgname",H_RGROUP,  "RGROUP"  },       /* */
        { RGID,  "rgid",  H_NOMSG,   "RGID"    },       /* */
        { RUSER, "ruser", H_RUSER,   "RUSER"   },       /* */
        { RUSER, "runame",H_RUSER,   "RUSER"   },       /* */
        { RUID,  "ruid",  H_NOMSG,   "RUID"    },       /* */
        { TIME,  "time",  H_TIME,    "TIME"    },       /* */
        { TIME,  "cputime",H_TIME,   "TIME"    },       /* */
        { TTY,   "tty",   H_TT,      "TT"      },       /* */
        { TTY,   "tt",    H_TT,      "TT"      },       /* */
        { TTY,   "tname", H_TT,      "TT"      },       /* */
        { TTY,   "longtname",H_TT,   "TT"      },       /* */
        { USER,  "user",  H_USER,    "USER"    },       /* */
        { USER,  "uname", H_USER,    "USER"    },       /* */
        { UID,   "uid",   H_NOMSG,   "UID"     },       /* */
        { LOGNAME,"logname",H_NOMSG, "LOGNAME" },       /* */
        { START, "start", H_NOMSG,   "STARTED" },       /* */
        { VSZ,   "vsz",   H_VSZ,     "VSZ"     },       /* */
        { VSZ,   "vsize", H_VSZ,     "VSZ"     },       /* */
        { RSS,   "rssize",H_RSS,     "RSS"     },       /* */
        { FLAG,  "flag",  H_F,       "F"       },       /* */
        { STATUS,"status",H_NOMSG,   "STATUS"  },       /* */
        { CP,    "cp",    H_CP,      "CP"      },       /* */
        { PAGEIN,"pagein",H_PAGEIN,  "PAGEIN"  },       /* */
        { WCHAN, "wchan", H_WCHAN,   "WCHAN"   },       /* */
	{ NWCHAN,"nwchan",H_WCHAN,   "WCHAN"   },       /* */
#ifdef _THREADS
        { ST,    "st",    H_ST,      "S"       },
	{ TID,   "tid",   H_TID,     "TID"     },
	{ SCOUNT,"scount",H_SCOUNT,  "SC"      },
        { BIND,  "bnd"   ,H_BIND  ,  "BND"     },
        { SCHED, "sched" ,H_SCHED,   "SCH"     },
	{ THCOUNT, "thcount",H_THCOUNT, "THCNT"},
#endif /*_THREADS */
        { ENDID, "?????", ENDID,     "?????"   }        /* End of Table */
};

/* for print_head() */
#define P_LEFT  0
#define P_RIGHT 1
#define P_PAD	2

/* initial udata structure size (UDI) and exponential growth factor (UDE) */
#define UDI     64
#define UDE     2
/* initial gdata structure size (GDI) and exponential growth factor (GDE) */
#define GDI     64
#define GDE     2

/* Pointer to user data */
static struct udata *ud = NULL;
static int     nud = 0;                /* number of valid ud structures */
static int     maxud = 0;              /* number of ud's allocated */

struct udata uid_tbl[MAX_LST];  /* table to store selected uid's */
static int     nut = 0;                /* counter for uid_tbl */
static int     PidLen;
static int     UserLen;
static int     UidLen;
static int     GidLen;
static int     GroupLen;
static int     DateLen;

#ifdef _THREADS
static int	TidLen;
#endif /*_THREADS */	

static struct gdata *gd;
static int     ngd = 0;                /* number of valid gd structures */
static int     maxgd = 0;              /* number of ud's allocated     */
static char    *header_fmt;
static struct fmtdata fmt_tbl[MAX_LST];    /* table to store fields        */
static int     nfmt = 0;               /* counter for fmt_tbl          */
static struct  winsize win;

static STRUCT_PROCINFO *proc_table;
static struct ucred    *ucreddb;
static struct proc_cpu *proc_st;
#ifndef _THREADS
static struct userinfo   *uinfo;
static struct userinfo   u_info;
static struct ucred      ucred_info;
static struct user       userdb;
static struct uicredinfo ui;
static int    uicredflg = 0;          /* if true, ps could get uicredinfo.    */
#else	/*_THREADS */
static struct thrdsinfo  *thrd;
static struct proc_from_thread *processft; 
static long nthread;
static int run_cpus;
#endif /*_THREADS */

static int     lflg = 0;               /* long format      */
static int     eflg = 0;               /* print everything */
static int     cflg = 0;
static int     uflg = 0;               /* match user name  */
static int     aflg = 0;
static int     dflg = 0;
static int     pflg = 0;               /* match processes list          */
static int     fflg = 0;               /* print user name instead of id */
static int     gflg = 0;               /* match process groups          */
static int     Gflg = 0;               /* match real group id           */
static int     kflg = 0;               /* show kernel processes         */
static int     tflg = 0;               /* match terminal name           */
static int     oflg = 0;               /* format list according to posix*/
static int     f2flg = 0;              /* format list according to posix*/
static int	dflt_flg = 0;		/* no adegkptu flags specified */

#ifdef _THREADS
static int     mflg = 0;
static int     thrflg = 0;
static char    *list_ptr2;                              /* AJOUT */
#endif /*_THREADS */

static int     desflg = 0;
static int     needcred = 0;           /* */
static int     errflg=0;
static char    argbuf[ARGSIZ];
static char    *parg;
static char    *list_ptr;	/* pointer within option argument lists */
static char stdbuf[BUFSIZ];

/* initial devl structure size (NDEV) and exponential growth factor (EDEV) */
#define NDEV    512
#define EDEV    2
#define DEVMPX  01
static int     ndev = 0;               /* number of valid devl structures */
static int     maxdevl = 0;            /* number of devl's allocated   */

struct _devl {
        char    *dname;
        dev_t   dev;
        char    dflags;
};
static struct _devl *devl;

#define	LEFT	0
#define	RIGHT	1

static char    *tty[MAX_LST];    /* for t option */
static int     ntty = 0;
static pid_t   pid[MAX_LST];       /* for p option */
static int     npid = 0;
static pid_t   pgrpid[MAX_LST];     /* for g option */
static int     npgrpid = 0;
static gid_t   grpid[MAX_LST];     /* for G option */
static int     ngrpid = 0;
static uid_t	curusr_euid;

static unsigned long   bytesperpage;
static int cmdpresent;
static long cmdbase;
static char cmdbuf[512];       /* Must be power of 2 */

static int adrpresent;
static long adrbase;
static char adrbuf[512];       /* Must be power of 2 */

static char	line_out[LINE_MAX]; /* Holds output line */
static int	chindx = 0;	/* Index into output line */
static int	chcnt;		/* Count of characters printed */

#ifdef _THREADS
    #define	THREADOPT	"m"
#else
    #define	THREADOPT	""
#endif /*_THREADS */

#ifdef DEBUG
    #define	DEBUGOPT	"j"
#else	/* DEBUG */
    #define	DEBUGOPT	""
#endif /* DEBUG */

/*
 * According to the ANSI C standard, a string of the form:  "aaa" "bbb" "ccc"
 * will get concatenated into the string "aaabbbccc".
 */

#define	OPTSTR	THREADOPT DEBUGOPT "lkfeadn:t:p:g:u:AG:o:U:F:"


main(argc, argv)
int     argc;
char    **argv;
{
        int c;
        long nproc;
        register char **ttyp = tty;
        char *name;
        char *p;
        pid_t puid, ppid, psid;
        gid_t pgid;
        int i, j, found;
        int exitcode=1;
        extern char *optarg;
        extern int optind;
        char    *fmtstr, *bp, *t_name;
        int     fcode, width;
        int     tsize, argcount, argsize, rec;
        int     foundid;
        size_t  len;
        size_t size;
	int	empty_header;
	char	*iptr;
        /* MSG
         * This is the default format string for a language-specific message on
         * usage; note that the invariant parts of it (i.e., the command name
         and flags) are passed as arguments to the fprintf statement
         * MSG
         */
#ifdef _THREADS
        char *usage = "Usage: ps [-Aaedfklm] [-n namelist] [-F Format] \
[-o specifier[=header],...]\n\t\t[-p proclist][-G|-g grouplist] \
[-t termlist] [-U|-u userlist] \n";
#else	/*_THREADS */
        char *usage = "Usage: ps [-Aaedfkl] [-n namelist] \
[-o specifier[=header],...] [-p proclist]\n\
\t\t[-G|-g grouplist] [-t termlist] [-U|-u userlist] \n";
#endif /*_THREADS */
        char *usage_berk = "Usage: ps [aceglnsuvwxU] [t tty] \
[ProcessNumber]\n";

        (void) setlocale (LC_ALL,"");
        catd = catopen(MF_PS, NL_CAT_LOCALE);

        setbuf(stdout, stdbuf);

        /* If -F is used, and all fields are empty (i.e. "user=")
         * then header will be empty.  Don't print blank line if
         * header is empty.  Default is empty header.
         */
        empty_header = 1;

        if (argc > 1 && argv[1][0] == '-') {
           while ((c = getopt(argc,argv,OPTSTR)) != EOF) {
                switch(c) {
                case 'l':               /* long listing */
                        lflg++;
                        break;

                case 'f':               /* full listing */
                        fflg++;
                        break;

#ifdef _THREADS
                case 'm':
			mflg++;         /* threads */
			break;
#endif /*_THREADS */
                case 'e':               /* list for every "real" process */
                        eflg++;
                        break;

                case 'k':               /* kernel procs */
                        kflg++;
                        break;

                case 'a':               /* like e but no proc grp leaders */
                        aflg++;         /* and no non-terminal processes  */
                        break;

                case 'd':               /* like e but no proc grp leaders */
                        dflg++;
                        break;

                case 'n':               /* alternate namelist */
                        break;

                case 't':               /* terminals */
                        tflg++;
                        list_ptr = optarg;
                        do {
                                parg = argbuf;
                                if (ntty >= MAX_LST)
                                        break;
                                getarg();
                                if (strncmp(parg,"cons",(size_t)4) == 0)
                                        parg = "console";
                                if (strncmp(parg,"tty",(size_t)3) == 0)
                                        parg += 3;
                                size = strlen(parg);
                                if ((p = (char *)malloc((size_t)++size))
                                                        == NULL) {
                                        fprintf(stderr,MSGSTR(NO_MEM,
                                                "ps: no memory\n"));
                                        done(1);
                                }
                                strcpy(p,parg);
                                *ttyp++ = p;
                                ntty++;
                        }
                        while (*list_ptr);
                        break;

                case 'p':               /* proc ids */
                        pflg++;
                        list_ptr = optarg;
                        parg = argbuf;
                        do {
				pid_t newpid;
				char *endptr;

                                if (npid >= MAX_LST)
                                        break;
                                getarg();
				newpid = (pid_t) strtoul(parg, &endptr, 10);
				if ((endptr == parg) || (*endptr != '\0')) {
					errflg++;
					fprintf(stderr,MSGSTR(LISTERR,
					    "ps: invalid list with %s.\n"), "-p");
				} else
					pid[npid++] = newpid;
                        }
                        while (*list_ptr);
                        break;

                case 'g':               /* proc group */
                        gflg++;
                        list_ptr = optarg;
                        parg = argbuf;
                        do {
				pid_t newpgid;
				char *endptr;

                                if (npgrpid >= MAX_LST)
                                        break;
                                getarg();
				newpgid = (gid_t) strtoul(parg, &endptr, 10);
				if ((endptr == parg) || (*endptr != '\0')) {
					errflg++;
					fprintf(stderr,MSGSTR(LISTERR,
					    "ps: invalid list with %s.\n"), "-g");
				} else
					pgrpid[npgrpid++] = newpgid;
                        }
                        while (*list_ptr);
                        break;
                case 'G':               /* group ID numbers */
                        Gflg++;
                        list_ptr = optarg;
                        parg = argbuf;
                        do {
				gid_t newgid;
				char *endptr;

                                if (ngrpid >= MAX_LST)
                                        break;
                                getarg();
				newgid = (gid_t) strtoul(parg, &endptr, 10);
				if ((endptr == parg) || (*endptr != '\0')) {
					errflg++;
					fprintf(stderr,MSGSTR(LISTERR,
					    "ps: invalid list with %s.\n"),
					    (c == 'g' ? "-g" : "-G"));
				} else
					grpid[ngrpid++] = newgid;
                        }
                        while (*list_ptr);
                        break;

                case 'U':               /* user ID numbers */
                case 'u':               /* user name or number */
                        uflg++;
                        list_ptr = optarg;
                        parg = argbuf;
                        do {
                            getarg();
                            if(nut < MAX_LST)
                                strncpy(uid_tbl[nut++].name,parg,
                                        (size_t)MAXLOGIN);
                        }
                        while (*list_ptr);
                        break;

                case 'A':               /* all accessible processes */
                        eflg++;
                        kflg++;
                        break;

                case 'F':               /* POSIX 1003.2a D5     */
                        f2flg++;
                case 'o':               /* POSIX 1003.2a D6     */
                                        /* given in format */
                        oflg++;
                        list_ptr = optarg;
                        parg = argbuf;
                        if ( des_judge(list_ptr) )  {
                                /*
                                 * MakeUp Descriptors Format to
                                 * the structure fmtdata.
                                 */
                                if (empty_header)
                                        empty_header=0; /* not empty anymore */
                                des_build(list_ptr);
                                desflg++;
                        }
                        else {
                        do {
                          getarg();
#ifdef _THREADS
                          if (strcmp(parg,"THREAD") == 0) {    
				list_ptr2 = (char *)malloc((size_t)(ARGSIZ+1+strlen(THREAD)+1));
				if (list_ptr2 == NULL) {
                                	fprintf(stderr, MSGSTR(NO_MEM,
                                         	"ps: no memory\n"));
                                        done(1);
                                }
                          	strcpy(list_ptr2, THREAD);
				strcat(list_ptr2," ");
                                strcat(list_ptr2,list_ptr);         
                                list_ptr = list_ptr2;
				thrflg ++;
				continue;
			  }
#endif /*_THREADS */
                          if(nfmt < MAX_LST) {
                            if ((fmtstr = (char *)malloc(
                                        (size_t)strlen(parg) + 1)) == NULL) {
                                fprintf(stderr, MSGSTR(NO_MEM,
                                        "ps: no memory\n"));
                                done(1);
                            }
                            strcpy(fmtstr,parg);
                            if ((fcode = *fmtstr) == '"') {
                                size = strlen(fmtstr);
                                if ((fcode = fmtstr[size - 1]) != '"') {
                                        errflg++;
                                        break;
                                }
                                else {
                                        fmtstr++;
                                        fmtstr[size - 1] = '\0';
                                }
                            }
                            foundid = 0;
                            if ((bp = strchr(fmtstr, '=')) == NULL) {
                                /*
                                 * missed '='.
                                 */
                                if (empty_header)
                                        empty_header=0; /* not empty anymore */
                                for (i=0; tnames[i].fieldid!= ENDID; i++) {
                                    if (!strncmp(tnames[i].var_name, fmtstr,
                                        strlen(tnames[i].var_name))) {
                                        /*
                                         * found variable name.
                                         */
                                        foundid++;
                                        fmt_tbl[nfmt].fieldid =
                                                        tnames[i].fieldid;
                                        if ((t_name =
                                                (char *)malloc((size_t)64))
                                                == NULL) {
                                                fprintf(stderr,MSGSTR(NO_MEM,
                                                        "ps: no memory\n"));
                                                done(1);
                                        }
                                        strcpy(t_name, MSGSTR(tnames[i].msg_id,
                                                tnames[i].def_name));
                                        fmt_tbl[nfmt].titlename = t_name;
                                        fmt_tbl[nfmt].colsize =
                                            dispwidth(fmt_tbl[nfmt].titlename);
                                        fmt_tbl[++nfmt].fieldid = PAD_ID;
                                        fmt_tbl[nfmt].titlename = SPACE;
                                        free(fmtstr);
                                        break;
                                    }
                                }
                                if (!foundid) {
                                    errflg++;
                                    fprintf(stderr,MSGSTR(LISTERR,
                                        "ps: invalid list with %s.\n"),
                                        (f2flg ? "-F" : "-o") );
                                    break;
                                }
                            }
                            else {
                                /*
                                 * found '='.
                                 */
                                for (i=0; tnames[i].fieldid != ENDID; i++) {
                                    if (!strncmp(tnames[i].var_name,
                                        fmtstr, (size_t)(bp - fmtstr))) {
                                        foundid++;
                                        fmt_tbl[nfmt].fieldid =
                                                        tnames[i].fieldid;
                                        if ((fmt_tbl[nfmt].titlename =
                                                (char *)malloc(
                                                (size_t)strlen(++bp) + 1))
                                                == NULL) {
                                                fprintf(stderr, MSGSTR(NO_MEM,
                                                        "ps: no memory\n"));
                                                done(1);
                                        }
                                        strcpy(fmt_tbl[nfmt].titlename, bp);
                                        if (*bp && empty_header)
                                                empty_header=0;
                                        fmt_tbl[nfmt].colsize =
                                          dispwidth(fmt_tbl[nfmt].titlename);
                                        fmt_tbl[++nfmt].fieldid = PAD_ID;
                                        fmt_tbl[nfmt].titlename = SPACE;
                                        break;
                                    }
                                }
                                if (!foundid) {
                                    errflg++;
                                    fprintf(stderr,MSGSTR(LISTERR,
                                        "ps: invalid list with %s.\n"),
                                        (f2flg ? "-F" : "-o") );
                                    break;
                                }
                            }
                          }
                          nfmt++;
                        }
                        while (*list_ptr);
                        }
                        fmt_tbl[nfmt].fieldid = ENDID;
                        break;

#ifdef DEBUG
                case 'j':
                        jul++;          /* Turn on debugging */
                        break;
#endif /* DEBUG */

                case '?':               /* error */
                        errflg++;
                        break;
                }
	    }
	    if (optind < argc)
		errflg++;
        } else {
                berk++;
                parseberkeley(argc,argv);
        }
        if ( errflg ) {
                fprintf(stderr,MSGSTR(SYSV_USAGE,usage));
                fprintf(stderr,MSGSTR(BSD_USAGE,usage_berk));
                done(1);
        }
        if ((bytesperpage = getpagesize()) <= 0) {
                perror("invalid pagesize");
                exit(-1);
        }
        if (tflg)
                *ttyp = 0;
        /* if specifying options not used, current terminal is default */
        if ( !(aflg || eflg || dflg || uflg || tflg || pflg || gflg || Gflg || kflg )) {
                if ((name = ttyname(2))
                        || (name = ttyname(1))
                        || (name = ttyname(0))
                        || (name = tty_hardway()))
                {
                        if (strncmp(name+5,"tty",(size_t)3)==0)
                                *ttyp++ = name+8;
                        else
                                *ttyp++ = name+5;
                }
                else
                        fprintf(stderr,MSGSTR(NOTTY,
                                "ps: Cannot identify your terminal\n"));
                *ttyp = 0;
                ntty++;
                tflg++;
		dflt_flg = 1;
		/*
		 * Since this calls geteuid() get effective UID of the user
		 * it depends on the setuid bits set on /bin/ps file.
		 * Right now it's -r-xr-sr-x  bin system ( setgid set ),
		 * so it works fine.
		 */
		curusr_euid = geteuid();
        }
        if (eflg)
                tflg = uflg = pflg = gflg = Gflg = aflg = dflg = 0;
        if (aflg || dflg)
                tflg = 0;
        if (oflg)
                lflg = fflg = 0;
        if (berk_flags & B_VFLAG) {
                fflg = lflg = 0;
                berk_flags &= ~B_SFLAG;     /* s and v flags are exclusive */
                berk_flags &= ~B_NFLAG;     /* n and v flags are exclusive */
        }

        DBIT(printf ("getting data... \n");)

        /* Read in device information */
	getdev("/dev");

        if ( !(berk_flags & B_TFLAG) && berk_flags ) {
                if ((uid_tbl[0].uid = getuidx(ID_REAL)) != -1) {
                        /*
                         * Display the all processes owned by the
                         * the current user.
                         */
                        nut++; /* used ufind() */
                }
        } else {
                /*
                 * If berk_flags, uid would be set already.
                 */
                uconv();
        }

        DBIT(printf ("Getting proc... \n");)

        proc_table = getprocdata (&nproc);
        ucreddb = (struct ucred *)malloc(sizeof(struct ucred));

        getfieldlen(proc_table, nproc);

        /*
	 * Determine screen_size which is the width of the output.
	 *
	 * If the berkley command is specified then the screen width
	 * is 80, 132, or AMAX depending on the wide_out value.
	 *
	 * If the environment variable COLUMNS is valid use it.
	 * If stdout is a tty then get the width from the TIOCGWINSZ ioctl.
	 * If stdout is not a tty (redirection or pipe) then use 80.
	 * This is XPG4 compliant.
	 */
	if (berk > 0) {
		switch (wide_out) {
		case 0:
			screen_size = 80;
			break;
		case 1:
			screen_size = 132;
			break;
		default:
			screen_size = AMAX;
			break;
		}
	} else {
		char *  env_ptr;

		env_ptr = getenv("COLUMNS");
		if ((env_ptr == NULL) || (*env_ptr == 0)) {
			if ((isatty(fileno(stdout)) == 1) &&
			    (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1)) {
				screen_size = win.ws_col;
			} else {
				screen_size = 80;
			}
		} else {
			screen_size = strtol(env_ptr, NULL, 0);
		}
		if (screen_size <= 0) {
			screen_size = 80;
		}
	}

	screen_cols = screen_size;

        DBIT(printf ("done proc..%d. \n",proc_table);)

#ifdef _THREADS
	/* get number of running cpus (for %CPU field) */
	run_cpus = _system_configuration.ncpus;
#endif
        /*
         * Print header
         */
        if (berk_flags & B_VFLAG) {    /* Berkeley V flag takes precedents */
                chcnt = sprintf(&line_out[chindx], " %*s %*s %s %*s %*s %*s %*s %*s %*s %*s %*s %*s %s\n",
                        PidLen,MSGSTR(H_PID,"PID"),     /* PidLen + 1   */
                        6,MSGSTR(H_TTY,"TTY"),          /* 7            */
                        MSGSTR(H_STAT,"STAT"),          /* 5            */
                        5,MSGSTR(H_TIME,"TIME"),        /* 6            */
                        4,MSGSTR(H_PGIN,"PGIN"),        /* 5            */
                        5,MSGSTR(H_SIZE,"SIZE"),        /* 6            */
                        5,MSGSTR(H_RSS,"RSS"),          /* 6            */
                        5,MSGSTR(H_LIM,"LIM"),          /* 6            */
                        5,MSGSTR(H_TSIZ,"TSIZ"),        /* 6            */
                        5,MSGSTR(H_TRS,"TRS"),          /* 6            */
                        4,MSGSTR(H_PCPU,"%CPU"),        /* 5            */
                        4,MSGSTR(H_PMEM,"%MEM"),        /* 5            */
                        MSGSTR(H_COMMAND,"COMMAND"));   /* 2            */
		chindx += chcnt;
                screen_size = screen_size - 66 - PidLen;
        } else {
                if (berk_flags & B_SFLAG) {
                        chcnt = sprintf(&line_out[chindx], MSGSTR(HEAD_STACK," SSIZ "));
			chindx += chcnt;
        	        screen_size -= 6;
        }

        if (oflg) {
                for (i=0;i<nfmt;i++) {
                        switch (fmt_tbl[i].fieldid) {
                        case RGROUP:
                        case RGID:
                        case GROUP:
                        case GID:
                        case USER:
                        case UID:
                        case LOGNAME:
                            /* get group ID and the name        */
                            getgids();
                            needcred++;
                        default:
                                break;
                        }
                        if (needcred)
                                break;
                }
                argcount = 0;
                for (i=0;i<nfmt;i++) {
                        switch (fmt_tbl[i].fieldid) {
                        case ARGS:
                                argcount++;
                                fmt_tbl[i].colsize = 0;
                                break;
                        case COMM:
                                if (fmt_tbl[i].colsize < 8)
                                        fmt_tbl[i].colsize = 8;
                                break;
                        case F_ETIME:
                        case TIME:
                                if (fmt_tbl[i].colsize < 11)
                                        fmt_tbl[i].colsize = 11;
                                break;
                        case NICE:
                                if (fmt_tbl[i].colsize < 2)
                                        fmt_tbl[i].colsize = 2;
                                break;
                        case PRI:
                                if (fmt_tbl[i].colsize < 3)
                                        fmt_tbl[i].colsize = 3;
                                break;
                        case PCPU:
                        case PMEM:
                                if (fmt_tbl[i].colsize < 5)
                                        fmt_tbl[i].colsize = 5;
                                break;
                        case PGID:
                                if (fmt_tbl[i].colsize < PidLen)
                                        fmt_tbl[i].colsize = PidLen;
                                break;
                        case PID:
                        case PPID:
                                if (fmt_tbl[i].colsize < PidLen)
                                        fmt_tbl[i].colsize = PidLen;
                                break;
                        case GROUP:
                        case RGROUP:
                                if (fmt_tbl[i].colsize < GroupLen)
                                        fmt_tbl[i].colsize = GroupLen;
                                break;
                        case GID:
                        case RGID:
                                if (fmt_tbl[i].colsize < GidLen)
                                        fmt_tbl[i].colsize = GidLen;
                                break;
                        case TTY:
                                if (fmt_tbl[i].colsize < 6)
                                        fmt_tbl[i].colsize = 6;
                                break;
                        case RUSER:
                        case USER:
                        case LOGNAME:
                                if (fmt_tbl[i].colsize < UserLen)
                                        fmt_tbl[i].colsize = UserLen;
                                break;
                        case RUID:
                        case UID:
                                if (fmt_tbl[i].colsize < UidLen)
                                        fmt_tbl[i].colsize = UidLen;
                                break;
                        case VSZ:
                                if (fmt_tbl[i].colsize < 5)
                                        fmt_tbl[i].colsize = 5;
                                break;
                        case RSS:
                                if (fmt_tbl[i].colsize < 5)
                                        fmt_tbl[i].colsize = 5;
                                break;
                        case FLAG:
                                if (fmt_tbl[i].colsize < 8)
                                        fmt_tbl[i].colsize = 8;
                                break;
                        case STATUS:
                                if (fmt_tbl[i].colsize < 2)
                                        fmt_tbl[i].colsize = 2;
                                break;
                        case CP:
                                if (fmt_tbl[i].colsize < 3)
                                        fmt_tbl[i].colsize = 3;
                                break;
                        case PAGEIN:
                                if (fmt_tbl[i].colsize < 4)
                                        fmt_tbl[i].colsize = 4;
                                break;
                        case WCHAN:
                        case NWCHAN:
                                if (fmt_tbl[i].colsize < 8)
                                        fmt_tbl[i].colsize = 8;
                                break;
                        case START:
                                if (fmt_tbl[i].colsize < DateLen)
                                        fmt_tbl[i].colsize = DateLen;
                                break;
                        case PAD_ID:
                                fmt_tbl[i].colsize =
                                          dispwidth(fmt_tbl[i].titlename);
                                break;
#ifdef _THREADS
			case TID:
                                if (fmt_tbl[i].colsize < TidLen)     
                                        fmt_tbl[i].colsize = TidLen;
                                break;
			case SCOUNT:
				if (fmt_tbl[i].colsize < 2)
					fmt_tbl[i].colsize = 2;
				break;					
			case BIND:
                                if (fmt_tbl[i].colsize < 3)
                                        fmt_tbl[i].colsize = 3;
			case SCHED:
				if (fmt_tbl[i].colsize < 3)
					fmt_tbl[i].colsize = 3;
				break;
			case THCOUNT:
				if (fmt_tbl[i].colsize <5)
					fmt_tbl[i].colsize = 5;
				break;
                        case ST:
                                if (fmt_tbl[i].colsize < 1)
                                        fmt_tbl[i].colsize = 1;
                                break;
#endif /*_THREADS */
                        case ENDID:
                                fmt_tbl[i].colsize = 0;
                                break;
                        default:
                                break;
                        }
                        screen_size -= fmt_tbl[i].colsize;
                }
                if (argcount) {
			/* 
			 * Check to see if there is room for more than
			 * 15 characters left on the screen. The number
			 * 15 is used to make sure at least enough of
			 * the command arguments are displayed to be
			 * meaningful. Less than 15 characters can be
			 * confusing with only partial arguments showing.
			 */
                        if (screen_size > 15) {
                                argsize = (screen_size - 2) / argcount;
                                screen_size = 0;
                        }
                        else {
                                argsize = 15;
			}
		}
                for (i=0;i<nfmt;i++) {
                        switch (fmt_tbl[i].fieldid) {
                        case ARGS:
                                fmt_tbl[i].colsize = argsize;
                        case COMM:
#ifdef _THREADS
                        case ST:
#endif /*_THREADS */
                        case STATUS:
                                if (!empty_header)
                                   (void)print_head(fmt_tbl[i].colsize,
                                        fmt_tbl[i].titlename, P_LEFT );
                                break;
                        case PAD_ID:
                                if (!empty_header)
                                   (void)print_head(fmt_tbl[i].colsize,
                                        fmt_tbl[i].titlename, P_PAD );
                                break;
                        case ENDID:
                                break;
                        default:
                                if (!empty_header)
                                   (void)print_head(fmt_tbl[i].colsize,
                                        fmt_tbl[i].titlename, P_RIGHT );
                                break;
                        }
                }
		if (!empty_header) {
                	chcnt = sprintf(&line_out[chindx], "\n");
			chindx += chcnt;
		}
        } else if (fflg && lflg) {
                chcnt = sprintf(&line_out[chindx], "%*s %*s %*s %*s ",
                        8, MSGSTR(H_F,"F"),             /* 8            */
                        1, MSGSTR(H_S,"S"),             /* 2            */
                        8, MSGSTR(H_UID,"UID"),         /* 9            */
                        PidLen, MSGSTR(H_PID,"PID"));   /* PidLen + 2   */
		chindx += chcnt;
                chcnt = sprintf(&line_out[chindx], MSGSTR(HEADR2,"%*s   C PRI NI ADDR    SZ    WCHAN %*s    TTY  TIME CMD\n"),PidLen,"PPID",DateLen,"STIME");
		chindx += chcnt;
        } else if (fflg) {
                chcnt = sprintf(&line_out[chindx], "%*s %*s %*s %*s %*s %*s %*s %s\n",
                        8, MSGSTR(H_UID,"UID"),         /* 8            */
                        PidLen, MSGSTR(H_PID,"PID"),    /* PidLen + 1   */
                        PidLen, MSGSTR(H_PPID,"PPID"),  /* PidLen + 1   */
                        3, MSGSTR(H_C,"C"),             /* 4            */
                        DateLen, MSGSTR(H_STIME,"STIME"),/* DateLen + 1 */
                        6, MSGSTR(H_TTY,"TTY"),         /* 7            */
                        5, MSGSTR(H_TIME,"TIME"),       /* 6            */
                        MSGSTR(H_CMD,"CMD"));           /* 2            */
		chindx += chcnt;
                screen_size = screen_size - 31 - PidLen - PidLen - DateLen;
        }
        else if (lflg) {
                chcnt = sprintf(&line_out[chindx], MSGSTR(HEADR4,"       F S %*s %*s %*s   C PRI NI ADDR    SZ    WCHAN    TTY  TIME CMD\n"),UidLen,"UID",PidLen,"PID",PidLen,"PPID");
		chindx += chcnt;
                screen_size -= 59;
                screen_size -= (PidLen << 1);  /* PID and PPID */
                screen_size -= UidLen;         /* UID          */
        }
        else if (berk_flags & B_UFLAG) {
                if (berk_flags & B_NFLAG) {
                    chcnt = sprintf(&line_out[chindx], "%*s",UidLen,MSGSTR(H_UID,"UID"));/* UidLen  */
		    chindx += chcnt;
                    screen_size -= UidLen;
                } else {
                        chcnt = sprintf(&line_out[chindx], "%-*s", UserLen, MSGSTR(H_USER,"USER")); /* UserLen      */
			chindx += chcnt;
                        screen_size -= UserLen;
                }
                chcnt = sprintf(&line_out[chindx], " %*s %*s %*s %*s %*s %*s %s %*s %*s %s\n",
                        PidLen,MSGSTR(H_PID,"PID"),     /* PidLen + 1   */
                        4,MSGSTR(H_PCPU,"%CPU"),        /* 5            */
                        4,MSGSTR(H_PMEM,"%MEM"),        /* 5            */
                        4,MSGSTR(H_SZ,"SZ"),            /* 5            */
                        4,MSGSTR(H_RSS,"RSS"),          /* 5            */
                        6,MSGSTR(H_TTY,"TTY"),          /* 7            */
                        MSGSTR(H_STAT,"STAT"),          /* 5            */
                        DateLen,MSGSTR(H_STIME,"STIME"),/* DateLen + 1  */
                        5,MSGSTR(H_TIME,"TIME"),        /* 6            */
                        MSGSTR(H_COMMAND,"COMMAND") );  /* 2            */
		chindx += chcnt;
                screen_size = screen_size - 42 - PidLen - DateLen;
        }
        else if (berk_flags & B_LFLAG) {
                chcnt = sprintf(&line_out[chindx],
                "%*s %s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %s\n",
                        6,MSGSTR(H_F,"F"),              /* 6            */
                        MSGSTR(H_S,"S"),                /* 2            */
                        UidLen,MSGSTR(H_UID,"UID"),     /* UidLen + 1   */
                        PidLen,MSGSTR(H_PID,"PID"),     /* PidLen + 1   */
                        PidLen,MSGSTR(H_PPID,"PPID"),   /* PidLen + 1   */
                        3,MSGSTR(H_C,"C"),              /* 4            */
                        3,MSGSTR(H_PRI,"PRI"),          /* 4            */
                        2,MSGSTR(H_NI,"NI"),            /* 3            */
                        4,MSGSTR(H_ADDR,"ADDR"),        /* 5            */
                        3,MSGSTR(H_SZ,"SZ"),            /* 4            */
                        4,MSGSTR(H_RSS,"RSS"),          /* 5            */
                        7,MSGSTR(H_WCHAN,"WCHAN"),      /* 8            */
                        6,MSGSTR(H_TTY,"TTY"),          /* 7            */
                        5,MSGSTR(H_TIME,"TIME"),        /* 6            */
                        MSGSTR(H_CMD,"CMD"));           /* 2            */
		chindx += chcnt;
                screen_size = screen_size - 59 - UidLen - PidLen - PidLen;
        } else if (uflg) {
                chcnt = sprintf(&line_out[chindx], MSGSTR(HEADR7," %*s %*s    TTY  TIME CMD\n"),
                                UidLen,"UID",PidLen,"PID");
		chindx += chcnt;
                screen_size = screen_size - 21 - PidLen - UidLen;
        } else if (berk_flags) {
                chcnt = sprintf(&line_out[chindx], " %*s %*s %s %*s %s\n",
                        PidLen, MSGSTR(H_PID,"PID"),    /* PidLen + 1   */
                        6, MSGSTR(H_TTY,"TTY"),         /* 7            */
                        MSGSTR(H_STAT,"STAT"),          /* 5            */
                        5, MSGSTR(H_TIME,"TIME"),       /* 6            */
                        MSGSTR(H_COMMAND,"COMMAND"));   /* 2            */
		chindx += chcnt;
                screen_size = screen_size - 21 - PidLen;
        } else {
                chcnt = sprintf(&line_out[chindx], MSGSTR(HEADR5," %*s    TTY  TIME CMD\n"),PidLen,"PID");
		chindx += chcnt;
                screen_size = screen_size - 17 - PidLen;
        }
        }

	/*
	 * Print the complete header to the screen
	 */
	print_line();

        /*
         * Determine which processes to print info about.
         */
        proc_st = NULL;
        if ( berk_flags & B_UFLAG )
                /*
                 * If the u flag of BSD flag is specified, the ps command
                 * sorts according to the %CPU used by each process.
                 * The function proc_sort() remakes the process table by
                 * sorting %CPU.
                 */
                proc_st = proc_sort( proc_table, nproc );
        if ( proc_st == NULL ) {
                proc_st = (struct proc_cpu *)
                        malloc(sizeof(struct proc_cpu) * nproc);
                if (proc_st == NULL) {
                        fprintf(stderr,MSGSTR(NO_MEM, "ps: no memory\n"));
                        done(1);
                }
                for (i=0; i<nproc; i++)
                        proc_st[i].process = &proc_table[i];
        }

        for (i=0; i<nproc; i++) {
		chindx = 0;
                found = 0;
                if (proc_st[i].process->PI_STATUS == SNONE)
                        continue;
                puid = (int) proc_st[i].process->pi_uid;
                ppid = (int) proc_st[i].process->pi_pid;
                psid = (int) proc_st[i].process->pi_sid;
                pgid = (int) proc_st[i].process->pi_cred.cr_rgid;

                DBIT(fprintf(stderr,"pid: %d\n",ppid);)

                if (berk_flags & B_ONEPROC) {
                        if (ppid == number)
                                found++;
                        else {
                                DBIT(printf("skipping %d\n",ppid);)
                                continue;
                        }
                }
		if ((ppid == psid) && (dflg || aflg) &&
                        !(berk_flags & B_XFLAG))
                        if (!(berk_flags & B_AFLAG))
                                continue;
                if ( proc_st[i].PI_FLAG & SKPROC ){
                        if (kflg)
                                found++;
                        else if (!berk && eflg)
                                continue;
                }
                if (eflg || dflg)
                        found++;
                else if (pflg && search(pid, npid, ppid))
                        found++;
                else if ( ( uflg || berk_flags ) && ufind(puid) )
                        found++;
                else if (gflg && search(pgrpid, npgrpid, psid))
                        found++;
                else if (Gflg && search(grpid, ngrpid, pgid))
                        found++;

                if ( !found && !tflg && !aflg && !(berk_flags & B_XFLAG))
                        continue;

                if (prcom(puid, proc_st[i].process, found)) {
                        exitcode=0;
                }

                DBIT(printf("return from prcom\n");)
        }

	if (fclose(stdout) == EOF) {
		perror("ps");
		exitcode=1;
	}

        done(exitcode);
}

/*
 * NAME:  getdev
 *
 * FUNCTION: use the directory(3) library support
 *
 * NOTES:
 *
 * RETURNS: void
 */
static getdev(char *dir)
{
        DIR *dirp;
        struct dirent *dp;
        struct stat sbuf1;
	char devname[MAXPATHLEN];
	int name_size;

	char perror_msg[PATH_MAX]; /* buffer to generate perror msg*/
	
        if ((dirp = opendir(dir)) == NULL) {
		sprintf(perror_msg, MSGSTR(OPEN_ERR2, 
		    "ps: Cannot open the directory %s.\n"), dir);
		perror(perror_msg);
		done(1);
        }

        if (chdir(dir) == -1) {
		sprintf(perror_msg, MSGSTR(CH_ERR, 
		    "ps: Cannot change the current directory to %s.\n"), dir);
		perror(perror_msg);
		done(1);
        }

        while ((dp = readdir(dirp)) != NULL) {
		while(ndev >= maxdevl) {
                        if (maxdevl == 0) {
                            maxdevl = NDEV;
                            devl = (struct _devl *) malloc(
                                (size_t)(sizeof(struct _devl) * maxdevl));
                        }
                        else {
                            maxdevl *= EDEV;
                            devl = (struct _devl *) realloc((void *)devl,
                                (size_t)(sizeof(struct _devl) * maxdevl));
                        }
                        if(devl == NULL) {
                                fprintf(stderr, MSGSTR(NO_MEM2,
                                        "ps: not enough memory\n"));
                                exit(1);
                        }
                }
                if(dp->d_ino == 0)
                        continue;
                if(lstat(dp->d_name, &sbuf1) < 0)
                        continue;

		sprintf(devname, "%s/%s", dir, dp->d_name);

		/* search subdirectories */
		if ((sbuf1.st_mode&S_IFMT) == S_IFDIR) {
			/* If not "." or ".." */
			if (strcmp(dp->d_name, ".") &&
			    strcmp(dp->d_name, "..") &&
			    !access(devname,X_OK)) {
				getdev(devname);
				/* Return to correct dir. */
				chdir(dir);
			}
			continue;
		}
                if (((sbuf1.st_mode&S_IFMT) != S_IFCHR) || (sbuf1.st_type == VLNK))
                        continue;
		/* if this is a subdirectory, prepend the subdir name
		 * onto the device name, e.g. pts/0
		 */
		/* Device Name length - "/dev/" + NULL char. */
		name_size = strlen(devname) - 4;
                if ((devl[ndev].dname = malloc((size_t)(name_size))) == NULL) {
                        fprintf(stderr,MSGSTR(NO_MEM, "ps: no memory\n"));
                        done(1);
                }
		strcpy(devl[ndev].dname, devname+5); 
                devl[ndev].dev = sbuf1.st_rdev;
                devl[ndev].dflags = (sbuf1.st_type == VMPC) ? DEVMPX : 0;
                ndev++;
        }
        closedir(dirp);
}


/*
 *  NAME:  getgids
 *
 *  FUNCTION:  Get the group file data into the gd structure.
 *
 *  RETURN VALUE:  none
 */
static void
getgids()
{
        struct group *gp, *getgrent();

        gd = NULL;
        ngd = 0;
        maxgd = 0;

        setgrent();
        while((gp=getgrent()) != NULL) {
                while(ngd >= maxgd) {
                        if (maxgd == 0) {
                                maxgd = GDI;
                                gd = (struct gdata *) malloc(
                                        (size_t)(sizeof(struct gdata) * maxgd));
                        }
                        else {
                                maxgd *= GDE;
                                gd = (struct gdata *) realloc((void *)gd,
                                        (size_t)(sizeof(struct gdata) * maxgd));
                        }
                        if(gd == NULL) {
                                fprintf(stderr,
                                    MSGSTR(NO_MEM2,"ps: not enough memory\n"));
                                exit(1);
                        }
                }
                /* copy fields from pw file structure to udata */
                gd[ngd].gid = gp->gr_gid;
                strncpy(gd[ngd].name, gp->gr_name, (size_t)MAXLOGIN);
                gd[ngd].name[MAXLOGIN] = '\0';
                ngd++;
        }
        endgrent();
}

/*
 *  NAME:  getarg
 *
 *  FUNCTION:
 *      getarg is used to parse a command line option's argument list.
 *	For example, the -u command can take a comma or blank separated
 *	list of users. This routine will scan through the list until
 *	it finds the next comma or blank, putting the argument value
 *	into argbuf. The list_ptr variable will initially point to the
 *	optarg string associated with the command line option. It will
 *	then be bumped to the next character that is not a comma or
 *	a blank. When list_ptr is equal to NULL this indicates the
 *	end of the argument list.
 *
 *  GLOBALS:
 *      argbuf, list_ptr
 *
 *  RETURNS:  none
 */
static void
getarg()
{
        char *buf_ptr;		/* Pointer to argbuf used for incrementing */
        buf_ptr = argbuf;

	/* Scan through the string until first comma or blank character */
        while(*list_ptr && *list_ptr != ',' && !isspace(*list_ptr)) {
		/* Copy each character into argbuf and increment pointers */
                *buf_ptr++ = *list_ptr++;
		/*
		 * Handle case where a format specifier header is to be
		 * changed. If the equal is encountered, copy the remainder
		 * of the line.
		 */
		if (*list_ptr == '=')
        		while(*list_ptr && *list_ptr != ',' && !isspace(*list_ptr))
				*buf_ptr++ = *list_ptr++;
	}
	/* Put NULL at end of argbuf to indicate end of argument string */
        *buf_ptr = '\0';

	/* Increment list_ptr until it points to the next argument or the
	 * end of the argument list */
        while( *list_ptr && ( *list_ptr == ',' || isspace(*list_ptr)) )
                list_ptr++;
}

/*
 * NAME:  gettty
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: the user's tty number,
 *              -       if none,
 *              ?       if can't figure it
 */
static char *
#ifndef _THREADS
gettty()
#else	/*_THREADS */
gettty(STRUCT_PROCINFO *process)
#endif /*_THREADS */
{
        register i, c;
        register char *p, *q;
        static char buf[32];

        if (PI_TTYP==0)
                return("-");
        for (i=0; i<ndev; i++) {
                if (devl[i].dev == PI_TTYD) {
                        strcpy(buf, devl[i].dname);
                        p = buf;
                        c = 0;
                        if (strncmp(p,"tty",(size_t)3) == 0)
                                p += 3;
                        for (q = p; c < strlen(p) && *q; ++c, ++q)
                                ;

                        /* Check for a multiplexed device, and add a channel
                         * number if we've got one.
                         */
                        if (devl[i].dflags & DEVMPX) {
                                register int d, flg, num;

                                *q++ = '/';

                                /* The correspondence between u_ttympx and
                                 * the actual channel number depends on how
                                 * setmpx() is called in the kernel -- see,
                                 * e.g., hft.c.
                                 */
                                if ((num = PI_TTYMPX) < 0 ||
                                        10000 <= num)
                                        *q++ = '?';
                                else
                                   if (num == 0)
                                        *q++ = '0';
                                   else
                                        for (flg = 0, d = 10000; d;
                                                num -= c * d, d /= 10)
                                                if ((c = num / d) || flg) {
                                                        *q++ = c + '0';
                                                        flg = 1;
                                                }
                        }
                        *q++ = '\0';
                        return(p);
                }
        }
        return("?");
}

/*
 * NAME:  prcom
 *
 * FUNCTION:   print information about the process.
 *
 * NOTES:
 *
 * RETURNS:     0 failed
 *              1 succeeded
 */
static prcom(puid,process,found)
uid_t puid;
int found;
STRUCT_PROCINFO *process;
{
        register char *tp;
        char args[AMAX];
        int argcount;
        long addr;
        char evars[AMAX];
        int *limit;
        time_t *clock, *tloc;
        time_t tim;
        time_t curtime, sttime,elapse;
        struct tm *sttm;
        char timbuf[26];
        char *curtim = timbuf;
        char *sttim;
        long    tm;
        int     match, i, rec;
        int     uzero = 0;
        register char **ttyp, *str;
        int screen;

        DBIT(printf("prcom: %d found:%d\n",puid,found);)
        screen = screen_size;

#ifdef _THREADS
	thrd = getthreaddata(process->pi_pid, &nthread);
	DBIT(printf("getthreaddata: returned :%d\n",nthread);)
	processft = getprocftdata(thrd, nthread);
	DBIT(printf("getprocftdata: returned.\n");)
	ucreddb = &(process->pi_cred);
#endif /*_THREADS */

        /* if process is zombie, call print routine and return */
        if ( process->PI_STATUS == SZOMB ) {
                if ( tflg && !found)
                        return(0);
                else {
                        przom(puid,process,"<defunct>");
                        chcnt = sprintf(&line_out[chindx], "\n");
			print_line();
                        return(1);
                }
        }

#ifndef _THREADS
        DBIT(printf("getuser: \n");)

        /*
         *   Don't print anything if the u-block is not zero.
         */
        uzero = getuser (process, PROCSIZE,  &ui, UICREDSIZE );
        if (uzero) {
                uzero = getuser (process, PROCSIZE,  &u_info, USERSIZE );
                if (uzero) {
			przom(puid,process,"<exiting>");
                        chcnt = sprintf(&line_out[chindx], "\n");
			print_line();
                        return(1);
		}
                uinfo = &u_info;
                ucreddb = NULL;
        } else {
                uinfo = &(ui.uici_ui);
                ucreddb = &(ui.uici_cred);
        }

        DBIT(printf("getuser: returned :%d\n",uzero);)

	tp = gettty();
#else	/*_THREADS */
	tp = gettty(process);
#endif /*_THREADS */
 
        /* get current terminal - if none (?) and aflg is set    */
        /* then don't print info - if tflg is set, check if term */
        /* is in list of desired terminals and print if so       */
        /* If the xflag is set, then we even want the ? and -'s  */

        DBIT(printf("getty: returned :%s\n",tp);)

        if (found) {
                if (berk_flags & B_TFLAG) {     /* ps t */
                        /*
                         * BSD t flag takes priority over a and x flags.
                         * If a and x flag with t flag are specified, they are
                         * ignored.
                         */
                        DBIT(printf ("comparing: %s and %s\n",tp,terminal);)
                        if (strcmp (tp,terminal) != 0)
                                return(0);
                }
                else if ( berk_flags && !(berk_flags & B_ONEPROC) &&
                        (!(berk_flags & B_XFLAG) && !dflg) &&
                        ((*tp == '?' || *tp == '-') && tp[1] == '\0') )
                        return(0);
        }
        else if (berk_flags & B_TFLAG) {                /* ps t */
                /*
                 * BSD t flag takes priority over a and x flags.
                 * If a and x flag with t flag are specified, they are
                 * ignored.
                 */
                DBIT(printf ("comparing: %s and %s\n",tp,terminal);)
                if (strcmp (tp,terminal) != 0)
                        return(0);
        }
        else if (aflg &&((*tp != '?' && *tp != '-') || tp[1] != '\0'))
                                                        /* ps a */
                ;
        else if ((berk_flags & B_XFLAG) && (*tp == '?' || *tp == '-')
                                && tp[1] == '\0') {     /* ps x */
                if ( !(berk_flags & B_AFLAG) )
                        return(0);
        }
        else if (tflg) {        /* the t option */            /*ps -t */
                for (ttyp=tty, match=0; (str = *ttyp) !=0 && !match; ttyp++)
                        if (strcmp(tp,str) == 0)
                                match++;
		/*
		 * For XPG/4 standard : default process selection is to
		 * match effective UIDs of process and user, in addition
		 * to controlling terminal.
		 */
                if(!match || (dflt_flg && (curusr_euid != process->pi_suid)))
                        return(0);
        }
        else
                return(0);

        DBIT(printf("getty: returned :%s\n",tp);)

        if (oflg) {
                /* If -o flag is specified */
                /* If -F flag is specified */
                prfmt(puid,process);
		print_line();
#ifdef _THREADS
		if (mflg) {
			prfmt_thread(thrd,nthread);
			DBIT(printf("prfmt_thread returned :\n");)
		}
#endif /*_THREADS */
               	return(1);
        }

        if ((berk_flags & B_SFLAG) && !(berk_flags & B_VFLAG)) {
        	chcnt = sprintf(&line_out[chindx], " %4x ", KSTACK);  /* pseg.h */
		chindx += chcnt;
	}
        if (lflg) {
                chcnt = sprintf(&line_out[chindx], "%8x", PI_FLAG);  /* F */
		chindx += chcnt;
	}
        if (berk_flags & B_LFLAG) {
                chcnt = sprintf(&line_out[chindx], "%6x", PI_FLAG);  /* F */
		chindx += chcnt;
	}
        if (lflg || (berk_flags & B_LFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %c", PI_STAT); /* S */
		chindx += chcnt;
	}
        if (fflg) {
                if (lflg) {
                        /* If "ps -lf", a space needs in front of USER field. */
                        chcnt = sprintf(&line_out[chindx], " ");
			chindx += chcnt;
		}
                i = getunam(puid);
                if ((i >= 0) && !(berk_flags & B_NFLAG)) {
                        chcnt = sprintf(&line_out[chindx], "%s", strpad(ud[i].name, 8, RIGHT));
			chindx += chcnt;
		}
                else {
                        chcnt = sprintf(&line_out[chindx], "%8lu", puid);
			chindx += chcnt;
		}
        }
        else if (berk_flags & B_UFLAG) {
                if (berk_flags & B_NFLAG) {
                        chcnt = sprintf(&line_out[chindx], "%*lu", UidLen, puid);
			chindx += chcnt;
                } else if ((i = getunam(puid)) >= 0) {
                        chcnt = sprintf(&line_out[chindx], "%s", strpad(ud[i].name, UserLen, LEFT));
			chindx += chcnt;
                } else {
			chcnt = sprintf(&line_out[chindx], "%-*lu", UserLen, puid);
			chindx += chcnt;
                }
        }
        else if (uflg || lflg || (berk_flags & B_LFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %*lu", UidLen,puid);
		chindx += chcnt;
        }
        chcnt = sprintf(&line_out[chindx], " %*lu",PidLen,process->pi_pid);  /* PID */
	chindx += chcnt;
        if (lflg || fflg || (berk_flags & B_LFLAG))
        {
                chcnt = sprintf(&line_out[chindx], " %*lu %3ld", PidLen, process->pi_ppid,  /* PPID */
                        PI_CPU);	                        /* CPU */
		chindx += chcnt;
        }
        if (lflg || (berk_flags & B_LFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %3ld",PI_PRI);  /* PRI */
		chindx += chcnt;
                (process->pi_nice <= P_NICE_MAX) ?
                                 (chcnt = sprintf(&line_out[chindx], " %2ld",process->pi_nice)) :
                                 (chcnt = sprintf(&line_out[chindx], " --"));   /* NICE */
		chindx += chcnt;
        }

        if (berk_flags & B_LFLAG) {
                chcnt = sprintf(&line_out[chindx], " %4lx %3ld %4ld",
                        process->pi_adspace,
                        PagetoKBLOCKS (process->pi_size),       /* ADDR SZ */
                        PagetoKBLOCKS (PI_RSS));             /* RSS  */
		chindx += chcnt;
                if (PI_WCHAN) { 	                       /* WCHAN */
                        if (!berk || (berk_flags & B_NFLAG)){
#ifdef _THREADS
				if (PI_WCHAN != 1)
                                	chcnt = sprintf(&line_out[chindx], " %7lx",PI_WCHAN);
				else {
					prblank(7);
                                	chcnt = sprintf(&line_out[chindx], "*");
				}
#else	/*_THREADS */
                               	chcnt = sprintf(&line_out[chindx], " %7lx",PI_WCHAN);
#endif /*_THREADS */
			}
                        else {
#ifdef _THREADS
				if (PI_WTYPE != -1)
                                	chcnt = sprintf(&line_out[chindx], " %7s", getwaittype(PI_WTYPE));
				else {
					prblank(7);
                                	chcnt = sprintf(&line_out[chindx], "*");
				}
#else	/*_THREADS */
                                chcnt = sprintf(&line_out[chindx], " %7s", getwaittype(PI_WTYPE));
#endif /*_THREADS */
			}
                }
                else {
                        chcnt = sprintf(&line_out[chindx],"        ");
		}
		chindx += chcnt;
        }
        if (lflg) {
                chcnt = sprintf(&line_out[chindx], " %4lx %5ld", process->pi_adspace,
                            PagetoKBLOCKS (process->pi_size));  /* ADDR SZ */
		chindx += chcnt;
                DBIT(printf("chan: returned :%d\n",tp);)
                if (PI_WCHAN) {                /* WCHAN */
                        if (!berk || (berk_flags & B_NFLAG)) {
#ifdef _THREADS
				if (PI_WCHAN != 1)
                                	chcnt = sprintf(&line_out[chindx], " %8lx",PI_WCHAN);
				else {
					prblank(8);
					chcnt = sprintf(&line_out[chindx], "*");
				}
#else	/*_THREADS */
                               	chcnt = sprintf(&line_out[chindx], " %8lx",PI_WCHAN);
#endif /*_THREADS */
			}
                        else {
#ifdef _THREADS
				if (PI_WTYPE != -1)
                                	chcnt = sprintf(&line_out[chindx], " %8s", getwaittype(PI_WTYPE));
				else {
					prblank(8);
					chcnt = sprintf(&line_out[chindx],"*");
				}
#else	/*_THREADS */
                                chcnt = sprintf(&line_out[chindx], " %8s", getwaittype(PI_WTYPE));
#endif /*_THREADS */
			}
                }
                else {
                        chcnt = sprintf(&line_out[chindx], "         ");
		}
		chindx += chcnt;
        }

        DBIT(printf("fflg: returned :%d\n",tp);)

        if (fflg) {                                             /* STIME */
                sttime = PI_START;
                curtime = time((time_t *) 0);
                sttm = localtime(&sttime);
                chcnt = sprintf(&line_out[chindx], " ");
		chindx += chcnt;
                prstime(sttm, curtime, sttime, DateLen);
        }
        if (strcmp ("console",tp) == 0)
                tp = "cons";
        if (!(berk_flags & B_UFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %6s", tp);  /* TTY  */
		chindx += chcnt;
	}
        if (berk_flags && !lflg                                 /* STAT */
                  && ! (berk_flags & B_VFLAG)
                  && ! (berk_flags & B_UFLAG)
                  && ! (berk_flags & B_LFLAG)) {
		chcnt = sprintf(&line_out[chindx], " %c   ",PI_STAT);
		chindx += chcnt;
	}

        if (berk_flags & B_VFLAG) {
                chcnt = sprintf(&line_out[chindx], " %c   ",PI_STAT); /* STAT */
		chindx += chcnt;
                tm = PI_TIME;				         /* TIME */
                chcnt = sprintf(&line_out[chindx], " %2ld:%.2ld", tm/60, tm%60);
		chindx += chcnt;
                DBIT(printf("Rlimit data: %x %x\n",
                             PI_RLIMIT_C, PI_RLIMIT_M);)
                chcnt = sprintf(&line_out[chindx], " %4d %5d %5d",
                                process->pi_majflt,             /* PAGIN */
                                PagetoKBLOCKS (PI_DVM) ,	 /* SIZE */
                                PagetoKBLOCKS (PI_RSS));         /* RSS */
		chindx += chcnt;
                if (PI_RLIMIT_C == PI_RLIMIT_M)
                        chcnt = sprintf(&line_out[chindx], "    xx"); /* LIM */
                else            /* Check to see if it the system limit */

                if (PI_RLIMIT_C == RLIM_INFINITY)
                        chcnt = sprintf(&line_out[chindx], " unlim"); /* LIM */
                else
                        chcnt = sprintf(&line_out[chindx], " %5d", PI_RLIMIT_C/ KBLOCKS);
		chindx += chcnt;
                chcnt = sprintf(&line_out[chindx], " %5d %5d",
                        PI_TSIZE/KBLOCKS + (PI_TSIZE % KBLOCKS ? 1 : 0),  /* TSIZ */
                        PagetoKBLOCKS (PI_TRSS));              	/* TRS  */
		chindx += chcnt;
                sttime = PI_START;
                curtime = time((time_t *) 0);
                elapse = curtime - sttime;
                if (elapse)
#ifndef _THREADS
                chcnt = sprintf(&line_out[chindx], " %4.1f", ((double) PI_TIME / (double) elapse)
                                 * (double) 100.);               /* %CPU */
#else
                chcnt = sprintf(&line_out[chindx], " %4.1f", ((double) PI_TIME / (double) elapse)
                                 * (double) 100. / (double)run_cpus);               /* %CPU */
#endif /*_THREADS */
                else
                        chcnt = sprintf(&line_out[chindx], " %4.1f", (double) 0);
		chindx += chcnt;
		chcnt = sprintf(&line_out[chindx], " %4.1f ", (double)PI_PRM);     /* %MEM */
		chindx += chcnt;
        } else if (berk_flags & B_UFLAG) {
                sttime = PI_START;
                curtime = time((time_t *) 0);
                elapse = curtime - sttime;
                if (elapse)
#ifndef _THREADS
                        chcnt = sprintf(&line_out[chindx], " %4.1f",
                                ((double) PI_TIME / (double) elapse)
                                * (double) 100.);               /* %CPU */
#else /*_THREADS */
                        chcnt = sprintf(&line_out[chindx], " %4.1f",
                                ((double) PI_TIME / (double) elapse)
                                * (double) 100. / (double)run_cpus);               /* %CPU */
#endif /*_THREADS */
                else
                        chcnt = sprintf(&line_out[chindx], " %4.1f", (double) 0);
		chindx += chcnt;
                chcnt = sprintf(&line_out[chindx], " %4.1f %4ld %4ld %6s %c    ",   /* %MEM */
                        (double)PI_PRM, PagetoKBLOCKS (process->pi_size), /* SZ RSS */
                        PagetoKBLOCKS (PI_RSS), tp, PI_STAT);       /* TTY and STAT*/
		chindx += chcnt;
                sttime = PI_START;
                curtime = time((time_t *) 0);
                sttm = localtime(&sttime);
                prstime(sttm, curtime, sttime, DateLen);        /* STIME */
                tm = PI_TIME; 			        /* TIME */
                chcnt = sprintf(&line_out[chindx], " %2ld:%.2ld ", tm/60, tm%60);
		chindx += chcnt;
        } else {
                tm = PI_TIME;  			       /* TIME */
                chcnt = sprintf(&line_out[chindx], " %2ld:%.2ld ", tm/60, tm%60);
		chindx += chcnt;
        }

        if (process->pi_pid==0) {
                chcnt = sprintf(&line_out[chindx], "swapper\n");
		print_line();
#ifdef _THREADS
		if (mflg) {
			prthread(thrd,nthread);
			DBIT(printf("prthread returned :\n");)
		}
#endif /*_THREADS */
                return(1);
        }
        if (PI_FLAG & SKPROC ) {
                chcnt = sprintf(&line_out[chindx], "kproc\n");
		print_line();
#ifdef _THREADS
		if (mflg) {
			prthread(thrd,nthread);
			DBIT(printf("prthread returned :\n");)
		}
#endif /*_THREADS */
                return(1);
        }
        /* if fflg not set, or if cflg set, print command from u_block */
        if (cflg || (!berk && !fflg ))          /* CMD */
                /* account for the space */
                screen = screen - print_comm(screen, PI_COMM) - 1;
        else if (fflg || berk)
        {
                DBIT(printf("getargs: returned :\n");)

                if (getargs (process, PROCSIZE, args, sizeof(args)) != 0) {
                        (void)print_comm_special(10, PI_COMM);
                        chcnt = sprintf(&line_out[chindx], "\n");
			print_line();
#ifdef _THREADS
			if (mflg) {
				prthread(thrd,nthread);
				DBIT(printf("prthread returned :\n");)
			}
#endif /*_THREADS */
                        return (1);
                }
                if (*args == '\0')
                      	screen -= print_comm_special(10, PI_COMM);
                else
                 	if (berk) {
			   if (screen > 0)
				screen -= print_args (screen,args);
                        }
                        else
				screen -= print_args (screen,args);
        }
        if (berk_flags & B_EFLAG)
        {
                if (getevars (process, PROCSIZE, evars, sizeof(evars)) != 0)
                {
                        perror(MSGSTR(NOENV,"ps: Cannot get environment\n"));
#ifdef _THREADS
			if (mflg) {
				prthread(thrd,nthread);
				DBIT(printf("prthread returned :\n");)
			}
#endif /*_THREADS */
                        return (1);
                }
	       if (screen > 0)
		    screen -= print_args (screen, evars);
        }
        chcnt = sprintf(&line_out[chindx], "\n");
	print_line();

#ifdef _THREADS
	if (mflg) {
		prthread(thrd,nthread);
		DBIT(printf("prthread returned :\n");)
	}
#endif /*_THREADS */
        DBIT(printf("prcom returned :\n");)
        return (1);
}



#ifdef _THREADS
/*
 * NAME:  prthread
 *
 * FUNCTION:  print information about threads
 *
 * NOTES:
 *
 * RETURNS:
 */
void prthread(struct thrdsinfo *thread, long nthread)

{
	long cntr;
	struct thrdsinfo *cur_thread;

	for (cntr=0; cntr<nthread; cntr++) {
		chindx = 0;
		cur_thread = &thread[cntr];
	        if ((berk_flags & B_SFLAG) && !(berk_flags & B_VFLAG))
	                prblank(5);
	        if (lflg) {
	                chcnt = sprintf(&line_out[chindx], "%8x", cur_thread->ti_flag);  /* F */
			chindx += chcnt;
		}
	        if (berk_flags & B_LFLAG) {
	                chcnt = sprintf(&line_out[chindx], "%6x", cur_thread->ti_flag);  /* F */
			chindx += chcnt;
		}
       		if (lflg || (berk_flags & B_LFLAG)) {
	                chcnt = sprintf(&line_out[chindx], " %c", thread_state(cur_thread)); /* S */
			chindx += chcnt;
		}
	        if (fflg) {
	                if (lflg) {
	                        /* If "ps -lf", a space needs in front of USER field. */
	                        chcnt = sprintf(&line_out[chindx]," ");
				chindx += chcnt;
			}
			prblank(8);
	        }
	        else if (berk_flags & B_UFLAG) {
	                if (berk_flags & B_NFLAG) 
			        prblank(UidLen);
			else
				prblank(UserLen);
	        }
	        else if (uflg || lflg || (berk_flags & B_LFLAG)) {
	                prblank(UidLen+1);
	        }

                prblank(PidLen);
		chcnt = sprintf(&line_out[chindx], "-");
                chindx += chcnt;

	        if (lflg || fflg || (berk_flags & B_LFLAG)) {
			prblank(PidLen+1);
	                chcnt = sprintf(&line_out[chindx], " %3ld", cur_thread->ti_cpu);   /* CPU */
			chindx += chcnt;
	        }
	        if (lflg || (berk_flags & B_LFLAG)) {
	                chcnt = sprintf(&line_out[chindx], " %3ld",cur_thread->ti_pri);                /* PRI */
			chindx += chcnt;
			prblank(3);					/* NICE */
	        }
	
	        if (berk_flags & B_LFLAG) {
			prblank(14);
	                if (cur_thread->ti_wchan) {                        /* WCHAN */
	                        if (!berk || (berk_flags & B_NFLAG))
	                                chcnt = sprintf(&line_out[chindx], " %7lx",cur_thread->ti_wchan);
	                        else
	                                chcnt = sprintf(&line_out[chindx], " %7s", getwaittype(cur_thread->ti_wtype));
	                }
	                else
	                        chcnt = sprintf(&line_out[chindx], "        ");
			chindx += chcnt;
	        }
	        if (lflg) {
			prblank(11);
	                if (cur_thread->ti_wchan) {                /* WCHAN */
	                        if (!berk || (berk_flags & B_NFLAG))
	                                chcnt = sprintf(&line_out[chindx], " %8lx",cur_thread->ti_wchan);
	                        else
	                                chcnt = sprintf(&line_out[chindx], " %8s", getwaittype(cur_thread->ti_wtype));
	                }
	                else
	                        chcnt = sprintf(&line_out[chindx], "         ");
			chindx += chcnt;
	        }
	
	        if (fflg) {                                             /* STIME */
	                chcnt = sprintf(&line_out[chindx], " ");
			chindx += chcnt;
			prblank(DateLen);
	        }
	        if (!(berk_flags & B_UFLAG))
			prblank(7);
	        if (berk_flags && !lflg                                 /* STAT */
	                  && ! (berk_flags & B_VFLAG)
	                  && ! (berk_flags & B_UFLAG)
	                  && ! (berk_flags & B_LFLAG)) {
                	chcnt = sprintf(&line_out[chindx], " %c   ", thread_state(cur_thread));
			chindx += chcnt;
		}
	        if (berk_flags & B_VFLAG) {
	                chcnt = sprintf(&line_out[chindx], " %c   ", thread_state(cur_thread));		 /* STAT */
			chindx += chcnt;
	        } else if (berk_flags & B_UFLAG) {
			prblank(5);
			prblank(22);
	                chcnt = sprintf(&line_out[chindx], " %c    ", thread_state(cur_thread));       /* STAT*/
			chindx += chcnt;
		}	
	        chcnt = sprintf(&line_out[chindx], "\n");
		print_line();
	}
}
#endif /*_THREADS */

/*
 * NAME:  done
 *
 * FUNCTION:  call exit()
 *
 * NOTES:
 *
 * RETURNS:
 */
static done(exitno)
int     exitno;
{
        exit(exitno);
}

/*
 * NAME:  search
 *
 * FUNCTION:  after the u option
 *
 * NOTES:
 *
 * RETURNS: search returns 1 if arg is found in array arr
 *          which has length num, and returns 0 if not found.
 */
static search(arr, num, arg)
int arr[];
int num;
int arg;
{
        int i;
        for (i = 0; i < num; i++)
                if (arg == arr[i])
                        return(1);
        return(0);
}

/*
 * NAME: uconv
 *
 * FUNCTION:  after the u option
 *
 * NOTES:
 *
 * RETURNS:
 */
static uconv()
{
        int found;
        int pwuid;
        int i, j;

        /* search thru name array for oarg */
        for (i=0; i<nut; i++)
        {
		getuuid(uid_tbl[i].name); 	/*fill in ud array*/

                found = -1;
                for(j=0; j<nud; j++)
                {
                        if (strncmp(uid_tbl[i].name, ud[j].name,
                                (size_t)MAXLOGIN)==0)
                        {
                                found = j;
                                break;
                        }
                }

                /* if not found and oarg is numeric */
                /* then search through number array */

                if (found < 0 && isdigit((int)uid_tbl[i].name[0])) {
                        pwuid = atoi(uid_tbl[i].name);
			getunam(pwuid);		/*fill in ud array*/
                        for (j=0; j<nud; j++) {
                                if (pwuid == ud[j].uid) {
                                        found = j;
                                        break;
                                }
                        }
                }

                /* if found then enter found index into tbl array */
                if ( found != -1 )
                {
                        uid_tbl[i].uid = ud[found].uid;
                        strncpy(uid_tbl[i].name, ud[found].name,
                                (size_t)MAXLOGIN);
                } else
                {
                        for(j=i+1; j<nut; j++)
                        {
                                strncpy(uid_tbl[j-1].name,uid_tbl[j].name,
                                        (size_t)MAXLOGIN);
                        }
                        nut--;
                                i--;
                }
        }
        return;
}

static getuuid(puname)
char *puname;
{
	struct passwd *pw;
	int puuid=0;

	if ((pw=getpwnam(puname)) != NULL) {
		getunam(pw->pw_uid);
	}
	
}

/*
 * NAME: getunam
 *
 * FUNCTION:
 *
 * NOTES: for full command (-f flag) print user name instead of number
 *        search thru existing table of userid numbers and if puid is found,
 *        return corresponding name.  Else search thru /etc/passwd
 *
 * RETURNS: table number for uid
 *          if gid is not in table, return -1.
 */
static getunam(puid)
int puid;
{
        int i;
        struct passwd *pw, *getpwuid();

        for(i=0; i<nud; i++)
                if(ud[i].uid == puid)
                        return(i);

        if ((pw=getpwuid(puid)) != NULL) {
                while(nud >= maxud) {
                        if (maxud == 0) {
                                maxud = UDI;
                                ud = (struct udata *) malloc(
                                        (size_t)(sizeof(struct udata) * maxud));
                        }
                        else {
                                maxud *= UDE;
                                ud = (struct udata *) realloc((void *)ud,
                                        (size_t)(sizeof(struct udata) * maxud));
                        }
                        if(ud == NULL) {
                                fprintf(stderr,MSGSTR(NO_MEM3,
                                "ps: not enough memory for %d users\n"), maxud);
                                exit(1);
                        }
                }
                /* copy fields from pw file structure to udata */
                ud[nud].uid = pw->pw_uid;
                strncpy(ud[nud].name, pw->pw_name, (size_t)MAXLOGIN);
                nud++;
		return(nud-1);
        }
	else
		return(-1);
}

/*
 * NAME: getgnam
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: table number for gid
 *          if gid is not in table, return -1.
 */
static getgnam(gid)
int gid;
{
        int i;

        for(i=0; i<ngd; i++)
                if(gd[i].gid == gid)
                        return(i);
        return(-1);
}

/*
 * NAME: ufind
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: 0 ... puid is not in table
 *          1 ... puid is in table
 */
static ufind(puid)
uid_t puid;
{
        int i;

        for(i=0; i<nut; i++)
                if(uid_tbl[i].uid == puid)
                        return(1);
        return(0);
}

/*
 * NAME: prstime
 *
 * FUNCTION: Display the start time of a process
 *
 * NOTES: print starting time of process unless process started more than 24
 *       hours ago in which case print the start date
 *       compare curtime and sttime for 24 hr check...both are time_t
 *
 * RETURNS: void
 */
void
prstime(sttm, curtime, sttime, len)
struct  tm *sttm;
time_t  curtime, sttime;
int     len;
{
        char  time_str[TIMESIZ + 1];

        if (curtime - sttime < 24L*60L*60L)
                /* less than 24hrs ... print time */
                (void)strftime(time_str, TIMESIZ, nl_langinfo(T_FMT), sttm);
        else
                (void)strftime(time_str, TIMESIZ, "%sD", sttm);

        chcnt = sprintf(&line_out[chindx], "%*s", len, time_str );
	chindx += chcnt;
}

/*
 * NAME: przom
 *
 * FUNCTION: Display the status of zombie and exiting processes
 *
 * NOTES: print zombie process - zproc overlays mproc
 *
 * RETURNS: void
 */
void
przom(puid,process,descr)
uid_t puid;
STRUCT_PROCINFO *process;
char *descr;
{
        int     i, j, rec, screen;
        long    tm;
        char    *etime;
	
        if ( oflg ) {
            for (i=0;i<nfmt;i++) {
                switch (fmt_tbl[i].fieldid) {
                case ARGS:
                case COMM:
                        screen = sprintf(&line_out[chindx], descr);
			chindx += screen;
                        if ( (fmt_tbl[i+2].fieldid != ENDID) || desflg )
                            /* fmt_tbl[i+1].fieldid is PAD_ID. */
                            for (j = fmt_tbl[i].colsize-screen; j > 0; j--) {
                                chcnt = sprintf(&line_out[chindx], " ");
				chindx += chcnt;
			    }
                        break;
                case TIME:
                        tm = (process->pi_utime + process->pi_stime + HZ/2)/HZ;
                        etime = mktimestr(tm, fmt_tbl[i].fieldid);
                        chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, etime);
			chindx += chcnt;
                        break;
                case NICE:
                        (process->pi_nice <= P_NICE_MAX) ?
                                (chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize,
                                        process->pi_nice)) :
                                (chcnt = sprintf(&line_out[chindx], "--"));   /* NICE */
			chindx += chcnt;
                        break;
                case PRI:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, PI_PRI);
			chindx += chcnt;
                        break;
                case PGID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, process->pi_pgrp);
			chindx += chcnt;
                        break;
                case PID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, process->pi_pid);
			chindx += chcnt;
                        break;
                case PPID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, process->pi_ppid);
			chindx += chcnt;
                        break;
                case RUSER:
                        if ((j = getunam(puid)) >= 0)
                                chcnt = sprintf(&line_out[chindx], "%s", strpad(ud[j].name,fmt_tbl[i].colsize, RIGHT));
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, puid);
			chindx += chcnt;
                        break;
                case RUID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, puid);
			chindx += chcnt;
                        break;
                case FLAG:
                        chcnt = sprintf(&line_out[chindx], "%*x", fmt_tbl[i].colsize,
                                PI_FLAG);
			chindx += chcnt;
                        break;
                case CP:
                        chcnt = sprintf(&line_out[chindx], "%*ld", fmt_tbl[i].colsize,
                                PI_CPU);
			chindx += chcnt;
                        break;
                case STATUS:
                        chcnt = sprintf(&line_out[chindx], "%-*c", fmt_tbl[i].colsize, PI_STAT);
			chindx += chcnt;
                        break;
                case PAD_ID:
                        if ( !((fmt_tbl[i+1].fieldid == ENDID) && !desflg) ) {
                                chcnt = sprintf(&line_out[chindx], "%s", fmt_tbl[i].titlename);
				chindx += chcnt;
			}
                        break;
#ifdef _THREADS
                case ST:
                        chcnt = sprintf(&line_out[chindx], "%-*c", fmt_tbl[i].colsize, PI_STAT);
			chindx += chcnt;
                        break;
#endif /*_THREADS */
                case ENDID:
                        break;
                default:
                        chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, " ");
			chindx += chcnt;
                        break;
                }
            }
            return;
        }

        if ((berk_flags & B_SFLAG) && !(berk_flags & B_VFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %4x ", KSTACK); /* pseg.h */
		chindx += chcnt;
	}
        if (lflg) {
                chcnt = sprintf(&line_out[chindx], "%8x", PI_FLAG); /* F */
		chindx += chcnt;
	}
        if (berk_flags & B_LFLAG) {
                chcnt = sprintf(&line_out[chindx], "%6x", PI_FLAG); /* F */
		chindx += chcnt;
	}
        if (lflg || (berk_flags & B_LFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %c", PI_STAT);  /* S */
		chindx += chcnt;
	}

        if (fflg) {
                if (lflg) {
                        /* If "ps -lf", a space needs in front of USER field. */
                        chcnt = sprintf(&line_out[chindx], " ");
			chindx += chcnt;
		}
                i = getunam(puid);
                if ((i >= 0) && !(berk_flags & B_NFLAG)) {
			if (ud[i].name[MAXLOGIN-1] == '\0')
				chcnt = sprintf(&line_out[chindx], "%s", strpad(ud[i].name, 8, RIGHT));
			else {
				strncpy(udnamebuf, ud[i].name, MAXLOGIN);
				udnamebuf[MAXLOGIN] = '\0';
				chcnt = sprintf(&line_out[chindx], "%s", strpad(udnamebuf, 8, RIGHT));
			}
		}
                else
                        chcnt = sprintf(&line_out[chindx], "%8lu", puid);
		chindx += chcnt;
        }
        else if (berk_flags & B_UFLAG) {
                i = getunam(puid);
                if ((i >= 0) && !(berk_flags & B_NFLAG)) {
			if (ud[i].name[MAXLOGIN-1] == '\0') 
                        	rec = sprintf(&line_out[chindx], "%s", ud[i].name);
			else {
				strncpy(udnamebuf, ud[i].name, MAXLOGIN);
				udnamebuf[MAXLOGIN] = '\0';
                        	rec = sprintf(&line_out[chindx], "%s", udnamebuf);
			}
			chindx += rec;
                        for (i=0;i<UserLen-rec;i++) {
                                chcnt = sprintf(&line_out[chindx], " ");
				chindx += chcnt;
			}
                }
                else {
                        chcnt = sprintf(&line_out[chindx], "%*lu", UidLen, puid);
			chindx += chcnt;
                }
        }
        else if (uflg || lflg || (berk_flags & B_LFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %*lu", UidLen,puid);
		chindx += chcnt;
        }
        chcnt = sprintf(&line_out[chindx], " %*lu",PidLen,process->pi_pid); /* PID */
	chindx += chcnt;
        if (lflg || fflg || (berk_flags & B_LFLAG))
        {
                chcnt = sprintf(&line_out[chindx], " %*lu",PidLen, process->pi_ppid);  /* PPID */
		chindx += chcnt;
                chcnt = sprintf(&line_out[chindx], " %3ld", PI_CPU); /* CPU */
		chindx += chcnt;
        }
        if (lflg || (berk_flags & B_LFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %3ld", PI_PRI); /* PRI */
		chindx += chcnt;
                (process->pi_nice <= P_NICE_MAX) ?
                                 (chcnt = sprintf(&line_out[chindx], " %2ld",process->pi_nice)) :
                                 (chcnt = sprintf(&line_out[chindx], " rt")); /* NICE */
		chindx += chcnt;
        }
        if (berk_flags & B_LFLAG) {
                chcnt = sprintf(&line_out[chindx], "                      "); /* ADDR SZ RSS WCHAN */
		chindx += chcnt;
	}
        if (lflg) {
                chcnt = sprintf(&line_out[chindx], "                    "); /* ADDR SZ WCHAN */
		chindx += chcnt;
	}
        if (fflg) {
		chcnt = sprintf(&line_out[chindx], " %*s",DateLen," "); /* STIME */
		chindx += chcnt;
	}
        if (!(berk_flags & B_UFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %6s", " "); /* TTY  */
		chindx += chcnt;
	}
        if (berk_flags && !lflg                         /* STAT */
                  && ! (berk_flags & B_VFLAG)
                  && ! (berk_flags & B_UFLAG)
                  && ! (berk_flags & B_LFLAG)) {
                chcnt = sprintf(&line_out[chindx], " %c   ", PI_STAT);
		chindx += chcnt;
	}
        if (berk_flags & B_VFLAG) {
                chcnt = sprintf(&line_out[chindx], " %c   ", PI_STAT); /* STAT */
		chindx += chcnt;
                tm = (process->pi_utime + process->pi_stime + HZ/2)/HZ;
                chcnt = sprintf(&line_out[chindx], " %2ld:%.2ld ", tm/60, tm%60);   /* TIME */
		chindx += chcnt;
                /* PAGIN SIZE RSS LIM TSIZ TRS %CPU %MEM        */
                chcnt = sprintf(&line_out[chindx], "                                             ");
		chindx += chcnt;
        } else if (berk_flags & B_UFLAG) {
                /* %CPU %MEM SZ RSS TTY STAT                            */
                chcnt = sprintf(&line_out[chindx], "                            %c   ", PI_STAT);
		chindx += chcnt;
                tm = (process->pi_utime + process->pi_stime + HZ/2)/HZ;
                chcnt = sprintf(&line_out[chindx], " %*s %2ld:%.2ld ", DateLen, " ", /* STIME       */
                        tm/60, tm%60);                  /* TIME         */
		chindx += chcnt;
        } else {
                tm = (process->pi_utime + process->pi_stime + HZ/2)/HZ;
                chcnt = sprintf(&line_out[chindx], " %2ld:%.2ld ", tm/60, tm%60);
		chindx += chcnt;
        }
        chcnt = sprintf(&line_out[chindx], descr);
	chindx += chcnt;
}

/*
 * NAME: prfmt
 *
 * FUNCTION: Display information according to -F/-o flag
 *
 * NOTES: print -o format process
 *        print -F format process
 *
 * RETURNS: void
 */
static prfmt(puid, process)
uid_t puid;
STRUCT_PROCINFO *process;
{
        int     i, j, len;
        long    tm;
        int     screen;
        int     uzero = 0;
        time_t  sttime, curtime, elapse;
        char    args[AMAX];
        char    *tp, *etime, *specialname;
        struct  tm *sttm;

        for (i=0;i<nfmt;i++) {
                switch (fmt_tbl[i].fieldid) {
                case ARGS:
                        if (process->pi_pid==0) {
                                screen = sprintf(&line_out[chindx], "%s", "swapper");
				chindx += screen;
                        } else if (PI_FLAG & SKPROC ) {
                                screen = sprintf(&line_out[chindx], "%s", "kproc");
				chindx += screen;
                        } else if (getargs(process, PROCSIZE, args,
                                sizeof(args)) != 0) {
                /*  No need, brackets define what happened....
                        perror(MSGSTR(READ_ERR, "ps: Cannot read U area"));
                */
                                screen = print_comm_special(10, PI_COMM);
                        } else if (*args == '\0') {
                                screen = print_comm_special(10, PI_COMM);
                        } else {
                                screen = print_args(fmt_tbl[i].colsize,args);
                        }
                        if ( ((fmt_tbl[i+2].fieldid != ENDID) || desflg) &&
                                (fmt_tbl[i].colsize - screen > 0) ) {
                            /* fmt_tbl[i+1].fieldid is PAD_ID. */
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize-screen, " ");
				chindx += chcnt;
			}
                        break;
                case COMM:
                        if (process->pi_pid==0) {
                                screen = sprintf(&line_out[chindx], "%s", "swapper");
				chindx += screen;
                        } else if (PI_FLAG & SKPROC ) {
                                screen = sprintf(&line_out[chindx], "%s", "kproc");
				chindx += screen;
                        } else {
                                /* The command name is never truncated. */
                                screen = print_comm(AMAX, PI_COMM);
                        }
                        if ( ((fmt_tbl[i+2].fieldid != ENDID) || desflg) &&
                                (fmt_tbl[i].colsize - screen > 0) ) {
                            /* fmt_tbl[i+1].fieldid is PAD_ID. */
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize-screen, " ");
				chindx += chcnt;
			}
                        break;
                case F_ETIME:
                        sttime = (time_t)PI_START;
                        curtime = time((time_t *) 0);
                        elapse = curtime - sttime;
                        etime = mktimestr(elapse, fmt_tbl[i].fieldid);
                        chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, etime);
			chindx += chcnt;
                        break;
                case TIME:
                        tm = PI_TIME;
                        etime = mktimestr(tm, fmt_tbl[i].fieldid);
                        chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, etime);
			chindx += chcnt;
                        break;
                case GROUP:
                        if (ucreddb == NULL) {
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, "--");
                        }
                        else if ((j = getgnam(ucreddb->cr_gid)) >= 0)
                                chcnt = sprintf(&line_out[chindx], "%s", strpad(gd[j].name, fmt_tbl[i].colsize, RIGHT));
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu",fmt_tbl[i].colsize,
                                                        ucreddb->cr_gid);
			chindx += chcnt;
                        break;
                case GID:
                        if (ucreddb == NULL) {
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, "--");
                        }
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize,
                                                        ucreddb->cr_gid);
			chindx += chcnt;
                        break;
                case NICE:
                        (process->pi_nice <= P_NICE_MAX) ?
                                (chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize,
                                        process->pi_nice)) :
                                (chcnt = sprintf(&line_out[chindx], "--"));   /* NICE */
			chindx += chcnt;
                        break;
                case PRI:
                        chcnt = sprintf(&line_out[chindx], "%*ld", fmt_tbl[i].colsize, PI_PRI);
			chindx += chcnt;
                        break;
                case PCPU:
                        sttime = PI_START;
                        curtime = time((time_t *) 0);
                        elapse = curtime - sttime;
                        if (elapse)
#ifndef _THREADS
                                chcnt = sprintf(&line_out[chindx], "%*.1f",
                                    fmt_tbl[i].colsize, ((double) PI_TIME /
                                    (double) elapse) * (double) 100.);
#else /*_THREADS */
                                chcnt = sprintf(&line_out[chindx], "%*.1f",
                                    fmt_tbl[i].colsize, ((double) PI_TIME /
                                    (double) elapse) * (double) 100. / (double)run_cpus);
#endif /*_THREADS */
                        else
                                chcnt = sprintf(&line_out[chindx], "%*.1f",
                                        fmt_tbl[i].colsize, (double) 0);
			chindx += chcnt;
                        break;
                case PMEM:
                        chcnt = sprintf(&line_out[chindx], "%*.1f",
                                fmt_tbl[i].colsize, (double)PI_PRM);
			chindx += chcnt;
                        break;
                case PGID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, process->pi_pgrp);
			chindx += chcnt;
                        break;
                case PID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, process->pi_pid);
			chindx += chcnt;
                        break;
                case PPID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, process->pi_ppid);
			chindx += chcnt;
                        break;
                case RGROUP:
                        if (ucreddb == NULL) {
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, "--");
                        }
                        else if ((j = getgnam(ucreddb->cr_rgid)) >= 0)
                                chcnt = sprintf(&line_out[chindx], "%s", strpad(gd[j].name, fmt_tbl[i].colsize, RIGHT));
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize,
                                                        ucreddb->cr_rgid);
			chindx += chcnt;
                        break;
                case RGID:
                        if (ucreddb == NULL) {
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, "--");
                        }
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize,
                                                        ucreddb->cr_rgid);
			chindx += chcnt;
                        break;
                case RUSER:
                        if ((j = getunam(puid)) >= 0)
                                chcnt = sprintf(&line_out[chindx], "%s",
                                        strpad(ud[j].name, fmt_tbl[i].colsize, RIGHT));
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, puid);
			chindx += chcnt;
                        break;
                case RUID:
                        chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, puid);
			chindx += chcnt;
                        break;
                case TTY:
#ifndef _THREADS
                        tp = gettty();
#else	/*_THREADS */
                        tp = gettty(process);
#endif /*_THREADS */
                        chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, tp);
			chindx += chcnt;
                        break;
                case USER:
                        if (ucreddb == NULL) {
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, "--");
                        }
                        else if ((j = getunam(ucreddb->cr_uid)) >= 0)
                                chcnt = sprintf(&line_out[chindx], "%s",
                                        strpad(ud[j].name,fmt_tbl[i].colsize, RIGHT));
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize,
                                        ucreddb->cr_uid);
			chindx += chcnt;
                        break;
                case UID:
                        if (ucreddb == NULL) {
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, "--");
                        }
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu",fmt_tbl[i].colsize,
                                                        ucreddb->cr_uid);
			chindx += chcnt;
                        break;
                case LOGNAME:
                        if (ucreddb == NULL) {
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, "--");
                        }
                        else if ((j = getunam(ucreddb->cr_luid)) >= 0)
                                chcnt = sprintf(&line_out[chindx], "%s",
					strpad(ud[j].name, fmt_tbl[i].colsize, RIGHT));
                        else
                                chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize,
                                        ucreddb->cr_luid);
			chindx += chcnt;
                        break;
                case VSZ:
                        chcnt = sprintf(&line_out[chindx], "%*d", fmt_tbl[i].colsize,
                                PagetoKBLOCKS (PI_DVM));
			chindx += chcnt;
                        break;
                case RSS:
                        chcnt = sprintf(&line_out[chindx], "%*d", fmt_tbl[i].colsize,
                                PagetoKBLOCKS ( PI_RSS));
			chindx += chcnt;
                        break;
                case FLAG:
                        chcnt = sprintf(&line_out[chindx], "%*x", fmt_tbl[i].colsize,
                                PI_FLAG);
			chindx += chcnt;
                        break;
                case STATUS:
                        chcnt = sprintf(&line_out[chindx], "%-*c", fmt_tbl[i].colsize,
                                PI_STAT);
			chindx += chcnt;
                        break;
#ifdef _THREADS
                case ST:
                        chcnt = sprintf(&line_out[chindx], "%-*c", fmt_tbl[i].colsize,
                                PI_STAT);
			chindx += chcnt;
                        break;
#endif /*_THREADS */
                case CP:
                        chcnt = sprintf(&line_out[chindx], "%*ld", fmt_tbl[i].colsize,
                                PI_CPU);
			chindx += chcnt;
                        break;
                case PAGEIN:
                        chcnt = sprintf(&line_out[chindx], "%*d", fmt_tbl[i].colsize,
                                process->pi_majflt);
			chindx += chcnt;
                        break;
#ifndef _THREADS
                case WCHAN:
                        if (PI_WTYPE)
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize,
                                        getwaittype(PI_WTYPE));
                        else
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, " ");
			chindx += chcnt;
                        break;
                case NWCHAN:
                        if (PI_WCHAN)
                                chcnt = sprintf(&line_out[chindx], "%*lx", fmt_tbl[i].colsize,PI_WCHAN);
                        else
                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize, " ");
			chindx += chcnt;
                        break;
#else	/*_THREADS */
		case WCHAN:
			if (PI_WTYPE) {
				if (PI_WTYPE != -1)
                                	chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize,
						getwaittype(PI_WTYPE));
				else {
					prblank(fmt_tbl[i].colsize-1);
					chcnt = sprintf(&line_out[chindx],"*");
				}
			}
			else {
				prblank(fmt_tbl[i].colsize-1);
				chcnt = sprintf(&line_out[chindx], "-");
			}		
			chindx += chcnt;
			break;	
		case NWCHAN:
			if (PI_WCHAN) {
				if (PI_WCHAN != 1)
                                	chcnt = sprintf(&line_out[chindx], "%*lx", fmt_tbl[i].colsize,PI_WCHAN);
				else {
					prblank(fmt_tbl[i].colsize-1);
					chcnt = sprintf(&line_out[chindx], "*");
				}
			}
			else {
				prblank(fmt_tbl[i].colsize-1);
				chcnt = sprintf(&line_out[chindx], "-");
			}		
			chindx += chcnt;
			break;	
#endif /*_THREADS */
                case START:
                        sttime = PI_START;
                        curtime = time((time_t *) 0);
                        sttm = localtime(&sttime);
                        prstime(sttm, curtime, sttime, fmt_tbl[i].colsize);
                        break;
#ifdef _THREADS
		case TID:
			prblank(fmt_tbl[i].colsize-1);
			chcnt = sprintf(&line_out[chindx], "-");
			chindx += chcnt;
                       	break;
		case SCOUNT:
			chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, PI_SCOUNT);
			chindx += chcnt;
                       	break;
		case BIND:
			if (thrd[0].ti_cpuid != NOTBOUND) {
				int j = 1;
				int same = 1;
				while ((same) && (j<(nthread-1))) {
					if (thrd[j].ti_cpuid != thrd[0].ti_cpuid)
						same = 0;
					j++;
				}
				if (same) 
					chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, thrd[0].ti_cpuid);
				else {	
					prblank(fmt_tbl[i].colsize-1);
					chcnt = sprintf(&line_out[chindx], "-");
				}
			}
			else {	
				prblank(fmt_tbl[i].colsize-1);
				chcnt = sprintf(&line_out[chindx], "-");
			}
			chindx += chcnt;
                       	break;
		case SCHED:
			if (nthread > 1) {
				prblank(fmt_tbl[i].colsize-1);
				chcnt = sprintf(&line_out[chindx], "-");
			}
			else
				chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, thrd[0].ti_policy);
			chindx += chcnt;
                       	break;
		case THCOUNT:
			chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, process->pi_thcount);
			chindx += chcnt;
			break;
#endif /*_THREADS */
                case PAD_ID:
                        if ( !((fmt_tbl[i+1].fieldid == ENDID) && !desflg) ) {
                                chcnt = sprintf(&line_out[chindx], "%s", fmt_tbl[i].titlename);
				chindx += chcnt;
			}
                        break;
                case ENDID:
                        break;
                default:
                        break;
                }
        }
        chcnt = sprintf(&line_out[chindx], "\n");
	chindx += chcnt;
}

#ifdef _THREADS
/*
 * NAME: prfmt_thread
 *
 * FUNCTION: Display threads information according to -F/-o flag
 *
 * NOTES: print -o format process
 *        print -F format process
 *
 * RETURNS: void
 */
void prfmt_thread(struct thrdsinfo *thread, long nthread)

{
        int     i, j, len;
        long    tm;
        int     screen;
        int     uzero = 0;
        time_t  sttime, curtime, elapse;
        char    args[AMAX];
        char    *tp, *etime, *specialname;
        struct  tm *sttm;
	long cntr;
	struct thrdsinfo *cur_thread;

	for (cntr=0; cntr<nthread; cntr++){
		cur_thread = &thread[cntr];
		chindx = 0;
	        for (i=0;i<nfmt;i++) {
	                switch (fmt_tbl[i].fieldid) {
	                case ARGS:
	                case COMM:
				screen = print_comm(10, "-");
				if ( ((fmt_tbl[i+2].fieldid != ENDID) || desflg) &&
					(fmt_tbl[i].colsize - screen > 0) ) {
					chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize-screen, " ");
					chindx += chcnt;
				}
	                        break;
	                case F_ETIME:
	                case TIME:
	                case GROUP:
	                case GID:
	                case PCPU:
	                case PMEM:
	                case PGID:
	                case PID:
	                case PPID:
	                case RGROUP:
	                case RGID:
	                case RUSER:
	                case RUID:
	                case TTY:
	                case USER:
	                case UID:
	                case LOGNAME:
	                case VSZ:
	                case RSS:
	                case PAGEIN:
	                case START:
			case NICE :
			case THCOUNT:
				prblank(fmt_tbl[i].colsize-1);
				chcnt = sprintf(&line_out[chindx], "-");
				chindx += chcnt;
	                        break;
	                case PRI:
	                        chcnt = sprintf(&line_out[chindx], "%*ld", fmt_tbl[i].colsize, cur_thread->ti_pri);
				chindx += chcnt;
	                        break;
	                case FLAG:
	                        chcnt = sprintf(&line_out[chindx], "%*x", fmt_tbl[i].colsize,
	                                cur_thread->ti_flag);
				chindx += chcnt;
	                        break;
	                case STATUS:
	                        chcnt = sprintf(&line_out[chindx], "%-*c", fmt_tbl[i].colsize, 
					thread_state(cur_thread));
				chindx += chcnt;
	                        break;
	                case ST:
	                        chcnt = sprintf(&line_out[chindx], "%-*c", fmt_tbl[i].colsize, 
					thread_state(cur_thread));
				chindx += chcnt;
	                        break;
	                case CP:
	                        chcnt = sprintf(&line_out[chindx], "%*ld", fmt_tbl[i].colsize, cur_thread->ti_cpu);
				chindx += chcnt;
	                        break;
	                case WCHAN:
	                        if (cur_thread->ti_wchan)
	                                chcnt = sprintf(&line_out[chindx], "%*s", fmt_tbl[i].colsize,
	                                        getwaittype(cur_thread->ti_wtype));
	                        else {
					prblank(fmt_tbl[i].colsize-1);
					chcnt = sprintf(&line_out[chindx], "-");
				}
				chindx += chcnt;
	                        break;
	                case NWCHAN:
	                        if (cur_thread->ti_wchan)
	                                chcnt = sprintf(&line_out[chindx], "%*lx", fmt_tbl[i].colsize,cur_thread->ti_wchan);
	                        else {
					prblank(fmt_tbl[i].colsize-1);
					chcnt = sprintf(&line_out[chindx], "-");
				}
				chindx += chcnt;
	                        break;
			case TID:
                        	chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, cur_thread->ti_tid);
				chindx += chcnt;
                        	break;
			case SCOUNT:
                        	chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, cur_thread->ti_scount);
				chindx += chcnt;
                        	break;
			case BIND:
				if (cur_thread->ti_cpuid != NOTBOUND)
                        		chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, cur_thread->ti_cpuid);
				else {
					prblank(fmt_tbl[i].colsize-1);
					chcnt = sprintf(&line_out[chindx], "-");
				}
				chindx += chcnt;
                        	break;
			case SCHED:
                        	chcnt = sprintf(&line_out[chindx], "%*lu", fmt_tbl[i].colsize, cur_thread->ti_policy);
				chindx += chcnt;
                        	break;
	                case PAD_ID:
	                        if ( !((fmt_tbl[i+1].fieldid == ENDID) && !desflg) ) {
	                                chcnt = sprintf(&line_out[chindx], "%s", fmt_tbl[i].titlename);
					chindx += chcnt;
				}
	                        break;
	                case ENDID:
	                        break;
	                default:
	                        break;
	                }
	        }
		chcnt = sprintf(&line_out[chindx], "\n");
		print_line();
	}
}
#endif /*_THREADS */

/*
 * NAME: mkwcstr
 *
 * FUNCTION: Convert multibyte string to wchar string.
 *
 * NOTES: If the string includes unprintable character, the character is
 *              converted to "?".
 *
 * RETURNS: Pointer to wchar string.
 */
size_t
mkwcstr(buf, p_pwcs)
char *buf;
wchar_t **p_pwcs;
{
        int     n, next;
        size_t  wcslen=0, area_size;
        wchar_t *pwcs, *ipwcs;          /* work buffer for converting */
        char    *ibuf;

        n = strlen( buf ) + 1;
        area_size = n * sizeof(wchar_t);
        if ( (pwcs = (wchar_t *)malloc( area_size )) == NULL ) {
                fprintf(stderr,MSGSTR(NO_MEM, "ps: no memory\n"));
                done(1);
        }
        memset(pwcs, '\0', area_size);
        ipwcs = pwcs;
        ibuf = buf;
        while (*ibuf != '\0') {
                next = mbtowc(ipwcs, ibuf, MB_CUR_MAX);
                if (next == -1) {
                        mbtowc(ipwcs, "?", MB_CUR_MAX);
                        ibuf++;
                } else if (next == 0) {
                        break;
                } else {
                        if (!iswprint(*ipwcs))
                                mbtowc(ipwcs, "?", MB_CUR_MAX);
                        ibuf += next;
                }
                ipwcs++;
                wcslen++;
        }
        *ipwcs = (wchar_t)0x00;
        *p_pwcs = pwcs;
        return(wcslen);
}

static int	residual = 0;

/*
 * NAME: print_head
 *
 * FUNCTION: Print the header name with specified display width.
 *
 * NOTES: N/A
 *
 * RETURNS: Return the display width that was printed.
 */
static int
print_head(len, buf, mode)
int     len;
char    *buf;
int     mode;
{
        int     i;
        size_t  wcslen, dsplen;
        wchar_t *p_pwcs, *pwcs;   /* work buffer for converting */

        if (len <= 0)
                return (0);
        wcslen = mkwcstr(buf, &p_pwcs);
        pwcs = p_pwcs;

        if ( (dsplen = wcswidth( pwcs, wcslen )) == -1) {
                for (i=0; i<len; i++) {
                        chcnt = sprintf(&line_out[chindx], "?");
			chindx += chcnt;
		}
        } else {
                while(dsplen > len) {
                        wcslen--;
                        dsplen = wcswidth( pwcs, wcslen );
                }
                pwcs[wcslen] = (wchar_t)0x00;

		if (mode == P_PAD) {
			residual += dsplen;
		} else {
			if (residual > 0) {
				chcnt = sprintf(&line_out[chindx], "%*s", residual, " ");
				chindx += chcnt;
				residual = 0;
			}
			if ( mode == P_LEFT ) {
				chcnt = sprintf(&line_out[chindx], "%S", pwcs);
				chindx += chcnt;
				if ( (i = len - dsplen) > 0 )
					residual = i;
			} else {
				if ( (i = len - dsplen) > 0 ) {
					chcnt = sprintf(&line_out[chindx], "%*s", i, " ");
					chindx += chcnt;
				}
				chcnt = sprintf(&line_out[chindx], "%S", pwcs);
				chindx += chcnt;
			}
                }
        }
        free(pwcs);
        return(len);
}

/*
 * NAME: print_comm_special
 *
 * FUNCTION: Print the command name with specified display width.
 *
 * NOTES: N/A
 *
 * RETURNS: Return the display width that was printed.
 */
static int
print_comm_special(len, buf)
int len;        /* MAX display column width that be able to be printed */
char *buf;
{
        int     n, next;
        size_t  wcslen, dsplen;
        wchar_t *p_pwcs, *pwcs, *ipwcs;   /* work buffer for converting */
        char    *ibuf;

        if (len <= 0)
                return (0);
        wcslen = mkwcstr(buf, &p_pwcs);
        pwcs = p_pwcs;

        if ( (dsplen = wcswidth( pwcs, wcslen )) == -1) {
                chcnt = sprintf(&line_out[chindx], "[?]");
		chindx += chcnt;
                free(pwcs);
                return(3);
        }
        while(dsplen > len - 2) {
                wcslen--;
                dsplen = wcswidth( pwcs, wcslen );
        }
        pwcs[wcslen] = (wchar_t)0x00;
        chcnt = sprintf(&line_out[chindx], "[%S]", pwcs);
	chindx += chcnt;
        free(pwcs);
        return (dsplen);
}

/*
 * NAME: print_comm
 *
 * FUNCTION: Print the command name with specified display width.
 *
 * NOTES: N/A
 *
 * RETURNS: Return the display width that was printed.
 */
static int
print_comm(len, buf)
int len;        /* MAX display column width that be able to be printed */
char *buf;
{
        int     n, next;
        size_t  wcslen, dsplen;
        wchar_t *p_pwcs, *pwcs, *ipwcs;   /* work buffer for converting */
        char    *ibuf;

        if (len <= 0)
                return (0);
        wcslen = mkwcstr(buf, &p_pwcs);
        pwcs = p_pwcs;

        if ( (dsplen = wcswidth( pwcs, wcslen )) == -1) {
                chcnt = sprintf(&line_out[chindx], "?");
		chindx += chcnt;
                free(pwcs);
                return(1);
        }
        while(dsplen > len) {
                wcslen--;
                dsplen = wcswidth( pwcs, wcslen );
        }
        pwcs[wcslen] = (wchar_t)0x00;

        chcnt = sprintf(&line_out[chindx], "%S", pwcs);
	chindx += chcnt;
        free(pwcs);
        return (dsplen);
}

/*
 * NAME: print_args
 *
 * FUNCTION: Print the command name and the arguments with specified display
 *              width.
 *
 * NOTES: N/A
 *
 * RETURNS: Return the display width that was printed.
 */
static int
print_args (len, buf)
int len;        /* MAX display column width that be able to be printed */
char *buf;
{
        int     bp=0;

        while ( buf[bp] != '\0' )
        {
                bp++;
                if (buf[bp] == '\0')
                        buf[bp++] = ' ';
        }
        buf[bp] = '\0';
        return ( print_comm(len, buf) );
}

/*
 * NAME: getprocdata
 *
 * FUNCTION: set process table for all processes
 *
 * NOTES: N/A
 *
 * RETURNS: STRUCT_PROCINFO
 */
static STRUCT_PROCINFO *
getprocdata (nproc)
long *nproc;
{
        long temp;
        long multiplier;
        STRUCT_PROCINFO *Proc;

#ifdef _THREADS
	struct procsinfo *P;
	pid_t	index;
	int	count;
	int	ret;
	short	again;

	*nproc = 0;
	again = 1;
        multiplier = 5;
	index = 0;
	count = 1000000;

	Proc = NULL;
	P = NULL;
	do {
		if ((ret = getprocs(P, PROCSIZE, NULL, 0,&index, count)) != -1) {
                	DBIT(printf("inloop:%d %d\n",errno, ret);)
			if (P == NULL) {				/* first call */
				count = ret + (multiplier <<= 1); 	/* Get extra in case busy system*/
                		DBIT(printf("mallocing: %d\n",count);)
                		Proc = (struct procsinfo *) malloc ((size_t)(PROCSIZE * count));
                		DBIT(printf("malloced: %d\n",Proc);)
				P = Proc;
                		if ( Proc == NULL) {
        				/* We ran out of space before we could read */
                               		/* in the entire proc structure.            */
                       			 perror ("malloc: ");
                       			 exit (1);
				}
				index = 0;	/* reset proc slot index */
			}
			else {
				*nproc += ret;
				if (ret >=  count) {			/* Not all entries were retrieved */ 
					count = (multiplier <<= 1);
                			DBIT(printf("reallocing: %d\n",count);)
                			Proc = (struct procsinfo *) realloc ((void *)Proc, (size_t)(PROCSIZE * (*nproc + count)));
                			DBIT(printf("realloced: %d\n",Proc);)
                			if ( Proc == NULL) {
        			                         /* We ran out of space before we could read */
                               				 /* in the entire proc structure.            */
                       				 perror ("realloc: ");
                       				 exit (1);
                			}
					else 
						P = Proc + (*nproc);
				}
				else				/* All entries were retrieved */
					again = 0;
			}
        	}
	} while (again && (ret != -1));
        return (Proc);
#else	/*_THREADS */
        DBIT(printf("malloc data\n");)

        multiplier = 5;
        Proc = (struct procinfo *) malloc ((size_t)sizeof(unsigned long));

        DBIT(printf("malloc done\n");)

        *nproc = 0;

        DBIT(printf("getting data\n");)

        while (((*nproc = getproc (Proc, *nproc, PROCSIZE)) == -1) &&
                errno == ENOSPC)
        {

                DBIT(printf("inloop:%d %d\n",errno, *nproc);)

                *nproc = *(long *) Proc;      /* num of active proc structures*/
                *nproc += (multiplier <<= 1); /* Get extra in case busy system*/

                DBIT(printf("reallocing: %d\n",*nproc);)

                Proc = (struct procinfo *) realloc ((void *)Proc,
                                (size_t)(PROCSIZE * *nproc));
                DBIT(printf("realloced: %d\n",Proc);)

                if ( Proc == NULL)
                {               /* We ran out of space before we could read */
                                /* in the entire proc structure.            */
                        perror ("realloc: ");
                        exit (1);
                }
        }
        return (Proc);
#endif /*_THREADS */
}

#ifdef _THREADS
/*
 * NAME: getthreaddata
 *
 * FUNCTION: fetch information found in the structure thrdsinfo for the threads of a given process
 *
 * NOTES: N/A
 *
 * RETURNS: struct thrdsinfo
 */
static struct thrdsinfo *
getthreaddata (pid,nthread)
pid_t pid;
long *nthread;
{
        long multiplier;
        struct thrdsinfo *Thread;
	struct thrdsinfo *T;
	tid_t	index;
	int	count;
	int	ret;
	short	again;

	*nthread = 0;
	again = 1;
        multiplier = 5;
	index = 0;
	count = 1000000;

	Thread = NULL;
	T = NULL;
	do {
		if ((ret = getthrds(pid, T, THRDSIZE, &index, count)) != -1) {
                	DBIT(printf("inloop:%d %d\n",errno, ret);)
			if (T == NULL) {				/* first call */
				count = ret + (multiplier <<= 1); 	/* Get extra in case busy system*/
                		DBIT(printf("mallocing: %d\n",count);)
                		Thread = (struct thrdsinfo *) malloc ((size_t)(THRDSIZE * count));
                		DBIT(printf("malloced: %d\n",Thread);)
				T = Thread;
                		if ( Thread == NULL) {
        				/* We ran out of space before we could read */
                               		/* in the entire thread structure.          */
                       			 perror ("malloc: ");
                       			 exit (1);
				}
				index = 0;
			}
			else {
				*nthread += ret;
				if (ret >=  count) {			/* Not all entries were retrieved */ 
					count = (multiplier <<= 1);
                			DBIT(printf("reallocing: %d\n",count);)
                			Thread = (struct thrdsinfo *) realloc ((void *)Thread, (size_t)(THRDSIZE * (*nthread + count)));
                			DBIT(printf("realloced: %d\n",Thread);)
                			if ( Thread == NULL) {
        			                         /* We ran out of space before we could read */
                               				 /* in the entire thread structure.            */
                       				 perror ("realloc: ");
                       				 exit (1);
                			}
					else 
						T = Thread + (*nthread);
				}
				else				/* All entries were retrieved */
					again = 0;
			}
        	}
	} while (again && (ret != -1));
        return (Thread);
}

/*
 * NAME: getprocftdata
 *
 * FUNCTION: build process information from thread information 
 *
 * NOTES: N/A
 *
 * RETURNS: struct proc_from_thread *
 */
static struct proc_from_thread *
getprocftdata (struct thrdsinfo *thread, long nthread)

{
        struct proc_from_thread *Pft;
	struct thrdsinfo *th;
	int i;

        DBIT(printf("malloc data\n");)
        Pft = (struct proc_from_thread *) malloc ((size_t)sizeof(struct proc_from_thread));
        if ( Pft == NULL) {
                perror ("malloc: ");
                exit (1);
        }
        DBIT(printf("malloc done\n");)

	Pft->pift_pri    = PIDLE;
	Pft->pift_wchan  = 0;
	Pft->pift_wtype  = 0;
	Pft->pift_cpu    = 0;
	Pft->pift_scount = 0;

	for(i=0;i<nthread;i++){
		th = &thread[i];
		if (th->ti_pri < Pft->pift_pri)
			Pft->pift_pri = th->ti_pri;
		if (th->ti_wchan){
			if (Pft->pift_wchan == 0){
				Pft->pift_wchan = th->ti_wchan;
				Pft->pift_wtype = th->ti_wtype;
			}
		  	else {
				Pft->pift_wchan = 1;
				Pft->pift_wtype = -1;
			}
		}
		Pft->pift_cpu    += th->ti_cpu;		 
   		Pft->pift_scount += th->ti_scount;
	}
        return (Pft);
}

#endif /*_THREADS */	

/*
 * NAME: proc_sort
 *
 * FUNCTION: sort process table according to the %CPU used by each process
 *
 * NOTES:    1: get %CPU value from getuser() for all processes.
 *           2: call p_sort() for sorting.
 *
 * RETURNS:  the pointer to "struct proc_cpu"
 */
#ifndef _THREADS
struct  proc_cpu *proc_sort(Proc, nproc)
struct  procinfo *Proc;
#else	/*_THREADS */
struct  proc_cpu *proc_sort(Proc, nproc)
struct  procsinfo *Proc;
#endif /*_THREADS */
int     nproc;
{
        int i;
#ifndef _THREADS
        struct procinfo *Proc_x;
        int     uzero=0;
#else	/*_THREADS */
        struct procsinfo *process;
#endif /*_THREADS */
        struct proc_cpu *proc_tmp;
        time_t  sttime, curtime, elapse;

        proc_tmp = (struct proc_cpu *)calloc(nproc, sizeof(struct proc_cpu));
        if (proc_tmp == NULL) {
                fprintf(stderr,MSGSTR(NO_MEM, "ps: no memory\n"));
                done(1);
        }

        for (i=0; i<nproc; i++) {
#ifndef _THREADS
                Proc_x = &Proc[i];
                uzero = getuser (Proc_x, PROCSIZE,  &u_info, USERSIZE );
                if (!uzero) {
                        uinfo = &u_info;
#else	/*_THREADS */
			/* for use by PI_START & PI_TIME macros */
                        process = &Proc[i];
#endif /*_THREADS */
                        sttime = PI_START;
                        curtime = time((time_t *) 0);
                        elapse = curtime - sttime;
                        if (elapse)
#ifndef _THREADS
                            proc_tmp[i].pcpu = ((double) PI_TIME / (double)elapse) * (double)100.;
#else /*_THREADS */
                            proc_tmp[i].pcpu = ((double) PI_TIME / (double)elapse) * (double)100. / (double)run_cpus;
#endif /*_THREADS */
                        else
                            proc_tmp[i].pcpu = (double)0;
#ifndef _THREADS
                }
                else {
                        proc_tmp[i].pcpu = (double)(-1);
                }
                proc_tmp[i].process = Proc_x;
#else	/*_THREADS */
		proc_tmp[i].process = &Proc[i];
#endif /*_THREADS */
        }
        p_sort(proc_tmp, 0, (nproc-1));
        return(proc_tmp);
}

/*
 * NAME: p_sort
 *
 * FUNCTION: quick sort function for process table
 *
 * NOTES:    N/A
 *
 * RETURNS:  void
 */
void    p_sort(p_struct, spos, epos)
struct  proc_cpu *p_struct;
int     spos, epos;
        /*
         * sort between p_struct[spos] and p_struct[epos]
         */
{
        int     i = spos;
        int     j = epos;
        double  x, tmp_c;
        STRUCT_PROCINFO *tmp_p;

        x = p_struct[ ( spos + epos ) / 2 ].pcpu;
        do {
                while ( p_struct[i].pcpu > x )
                        ++i;
                while ( x > p_struct[j].pcpu )
                        --j;
                if (i <= j) {
                        tmp_p = p_struct[i].process;
                        tmp_c = p_struct[i].pcpu;
                        p_struct[i].process = p_struct[j].process;
                        p_struct[i].pcpu = p_struct[j].pcpu;
                        p_struct[j].process = tmp_p;
                        p_struct[j].pcpu = tmp_c;
                        ++i;
                        --j;
                }
        } while (i <= j);
        if (spos < j)
                p_sort(p_struct, spos, j);
        if (i < epos)
                p_sort(p_struct, i, epos);
        return;
}
/*
 * NAME: parseberkeley
 *
 * FUNCTION: set global flags assuming Berkeley options is meant
 *
 * NOTES: N/A
 *
 * RETURNS: void
 */
void
parseberkeley(argc,argv)
int argc;
char **argv;
{
char *ptr;
int c, i;
int uniq=0, hftflg=0, ptsflg=0;

        while (--argc)
        {
           ptr = *++argv;
           while (c = *(ptr++))
                switch(c) {
                case 'a':               /* all processes with terminals */
                        berk_flags |= B_AFLAG;
                        aflg++;
                        break;

                case 'c':               /* Print command name from proc */
                        berk_flags |= B_CFLAG;
                        cflg++;
                        break;

                case 'e':               /* Print Environment as well   */
                        berk_flags |= B_EFLAG;
                        break;

                case 'g':               /* All processes                */
                        berk_flags |= B_GFLAG;
                        berk_flags |= B_AFLAG;
                        berk_flags |= B_XFLAG;
                        aflg++;
                        break;

#ifdef DEBUG
                case 'j':
                        jul++;          /* Debugging only */
                        printf ("JULS dbs turned on\n");
                        break;
#endif /* DEBUG */

                case 'l':               /* long listing */
                        berk_flags |= B_LFLAG;
                        uniq++;
                        break;

                case 'n':               /* numerical output for waitchannel */
                        berk_flags |= B_NFLAG;
                        break;

                case 's':               /* add the size SSIZ of kernel stack */
                        berk_flags |= B_SFLAG;
                        uniq++;
                        break;

                case 't':               /* use this controlling tty */
                        berk_flags |= B_TFLAG;
                        if (*ptr == '\0' && argc > 1) {
                                   argc--;
                                   ptr = *++argv;
                        }
                        /* 't' option requires an arg (tty) */
                        if (*ptr == '\0')
                        {
                           errflg++;
                           break;
                        }
                        if (strncmp (ptr,"co",(size_t)2) == 0) {
                            terminal = "console";
                        } else {
                            if (strncmp(ptr,"t",(size_t)1) == 0) {
                                if (strncmp(ptr,"tty",(size_t)3) == 0) {
                                        ptr += 3;
                                }
                                else {
                                        ptr++;
                                }
                            } else if ((strncmp(ptr,"p",(size_t)1) == 0) &&
                                (strncmp(ptr,"pts",(size_t)3) != 0)) {
                                        ptsflg++;
                                        ptr++;
                            } else if ((strncmp(ptr,"h",(size_t)1) == 0) &&
                                (strncmp(ptr,"hft",(size_t)3) != 0)) {
                                        hftflg++;
                                        ptr++;
                            }
                            terminal = (char *)malloc((size_t)strlen(ptr)+5);
                            if (terminal == NULL) {
                                fprintf(stderr,MSGSTR(NO_MEM,
                                        "ps: no memory\n"));
                                done(1);
                            }
                            if (hftflg) {
                                strcpy(terminal,"hft/");
                                strcpy((terminal+4),ptr);
                            } else if (ptsflg) {
                                strcpy(terminal,"pts/");
                                strcpy((terminal+4),ptr);
                            } else {
                                strcpy(terminal,ptr);
                            }
                        }
                        *ptr=0;

                        DBIT(printf ("This is the controlling terminal: %s\n",
                                terminal);)
                        break;

                case 'u':               /* User oriented output */
                        berk_flags |= B_UFLAG;
                        uniq++;
                        break;

                case 'v':               /* add virtual memory stats */
                        berk_flags |= B_VFLAG;
                        uniq++;
                        break;

                case 'w':               /* wide 132-column output */
                        berk_flags |= B_WFLAG;
                        wide_out++;
                        break;

                case 'x':               /*  processes without terminals */
                        berk_flags |= B_XFLAG;
                        break;

                case 'U':               /*  redo database */
                        getdev("/dev");
                        exit(0);

                default:                /* error */
                        if (isdigit((int)c)) {  /* check for process number */
                                number = atoi(--ptr);
                                DBIT(printf ("process number: %d\n",number);)
                                *ptr=0;         /* no more options allowed */
                                argc=1;
                                berk_flags |= B_ONEPROC; /* only one proc */
                        }
                        else
                                errflg++;
                        break;
                }
        }
        if (uniq > 1) {
                printf (MSGSTR(UNIQ,"ps: Specify only one of s,l,v and u\n"));
                exit (1);
        }

        DBIT(printf ("Berk flags are: %x\n",berk_flags);)
}

/*
 * NAME: getwaittype
 *
 * FUNCTION: convert a waitchannel to some sort of symbolic form.
 *              These events are hardcoded from proc.h
 *
 * NOTES: N/A
 *
 * RETURNS: pointer to a symbolic name for the event type.
 */
char *
getwaittype(wtype)
char    wtype;

{
        switch(wtype) {
#ifndef _THREADS
                case SNOWAIT:
                                return ("NOWAIT");

                case SWEVENT:   /* waiting for event(s) signal?     */
                                return ("EVENT");

                case SWLOCK:    /* waiting for serialization lock   */
                                return ("LOCK");

                case SWTIMER:   /* waiting for timer                */
                                return ("TIMER");

                case SWCPU:     /* waiting for CPU (in ready queue) */
                                return ("CPU");

                case SWPGIN:    /* waiting for page in              */
                                return ("PGIN");

                case SWPGOUT:   /* waiting for page out level       */
                                return ("PGOUT");

                case SWPLOCK:   /* waiting for physical lock        */
                                return ("PLOCK");

                case SWFREEF:   /* waiting for a free page frame    */
                                return ("FREEF");
#else	/*_THREADS */
                case TNOWAIT:
                                return ("NOWAIT");

                case TWEVENT:   /* waiting for event(s) signal?     */
                                return ("EVENT");

                case TWLOCK:    /* waiting for serialization lock W */
                                return ("LOCKW");

		case TWLOCKREAD :  /* waiting for serialization lock R */
				return ("LOCKR");

                case TWTIMER:   /* waiting for timer                */
                                return ("TIMER");

                case TWCPU:     /* waiting for CPU (in ready queue) */
                                return ("CPU");

                case TWPGIN:    /* waiting for page in              */
                                return ("PGIN");

                case TWPGOUT:   /* waiting for page out level       */
                                return ("PGOUT");

                case TWPLOCK:   /* waiting for physical lock        */
                                return ("PLOCK");

                case TWFREEF:   /* waiting for a free page frame    */
                                return ("FREEF");

		case TWMEM:     /* waiting for memory */
				return ("MEM");
#endif /*_THREADS */

                default:        /* unknown */
                                return("");
        }

}

/*
 * NAME: tty_hardway
 *
 * FUNCTION: return the name of the controling terminal, by
 *              opening up /dev/tty and doing a ttyname on that
 *              file descriptor.
 * NOTES: N/A
 *
 * RETURNS: returns a pointer to the name of the controling terminal
 */
char *
tty_hardway()
{
        int fd;
        char *name="";

          fd = open ("/dev/tty",0);
          if (fd >= 0){
                name = ttyname(fd);
                close (fd);
          }

          return (name);
}

/*
 * NAME: getmaxpidlen
 *
 * FUNCTION:    walk thru the proc table and find the largest pid
 *              that is active.
 *
 * NOTES: N/A
 *
 * RETURNS:     The number of base ten digits that this maximum pid
 *              will use.
 */
int
getmaxpidlen(process,nproc)
STRUCT_PROCINFO *process;
int             nproc;
{
        int i;
        int len=1;
        uint maxpid=0;

        for (i=nproc-1; i>0; i--)
                if (process[i].PI_STATUS != SNONE && process[i].pi_pid > maxpid)
                        maxpid = process[i].pi_pid;

        while (maxpid > 9) {
                maxpid /= 10;
                len++;
        }
        return(len);
}

#ifdef _THREADS
/*
 * NAME: getmaxtidlen
 *
 * FUNCTION:    walk thru the proc table and find the largest tid
 *              that is active.
 *
 * NOTES: N/A
 *
 * RETURNS:     The number of base ten digits that this maximum tid
 *              will use.
 */
int
getmaxtidlen()
{
        int i;
        int len=1;
        uint maxtid=0;
	struct thrdsinfo *all_threads;
	ulong	thread_count;
	pid_t	all_pids;

	all_pids = -1;
	if ((all_threads = getthreaddata(all_pids, &thread_count)) != (-1)) {
		DBIT(printf("getthreaddata: returned :%d\n",thread_count);)

        	for (i=thread_count-1; i>=0; i--)
			if (all_threads[i].ti_state != TSNONE && all_threads[i].ti_tid > maxtid)
                       		 maxtid = all_threads[i].ti_tid;

        	while (maxtid > 9) {
               		 maxtid /= 10;
               		 len++;
        	}
	}
	len++;  /* 1 digit added for security in case new threads are created before the next 
                   calls to getthreaddata */
        return(len);
}
#endif /*_THREADS */

/*
 * NAME: getmaxuidlen
 *
 * FUNCTION:    walk thru the proc table and find the largest uid
 *              that is active.
 *
 * NOTES: N/A
 *
 * RETURNS:     The number of base ten digits that this maximum uid
 *              will use.
 */
getmaxuidlen(process,nproc)
STRUCT_PROCINFO *process;
int             nproc;
{
        int i;
        int len=1;
        int minlen=3;   /* This is length value for the header name "UID"*/
        int maxlen=8;
        uint maxuid=0;

        for (i=nproc-1; i>0; i--)
                if (process[i].PI_STATUS != SNONE && process[i].pi_uid > maxuid)
                        maxuid = process[i].pi_uid;

        while (maxuid > 9) {
                maxuid /= 10;
                len++;
        }

        if (len < minlen)
                len = minlen;
        else if (len > maxlen)
                len = maxlen;

        return(len);
}

/*
 * NAME: getmaxtimelen
 *
 * FUNCTION:    Determine the maximum date length used in this
 *              language and return it.
 *
 * NOTES: N/A
 *
 * RETURNS: the maximum date length
 */
int
getmaxdatelen()
{
        char a[100];
        int lena,lenb;
        struct tm *sttm;
        long currenttime = time ((time_t *) 0);

        sttm = localtime (&currenttime);

        strftime(a, 100, nl_langinfo(T_FMT), sttm);
        lena = dispwidth(a);

        strftime(a, 100, "%sD", sttm);
        lenb = dispwidth(a);

        if (lena > lenb)
                return (lena);
        else
                return (lenb);
}

/*
 * NAME: mktimestr
 *
 * FUNCTION: Make a string that is displayed as the elapsed time or
 *              the cumulative CPU time.
 *
 * NOTES: The time string is made as follows.
 *              o mode = F_ETIME    [[dd-]hh:]mm:ss
 *              o mode = TIME     [dd-]hh:mm:ss
 *
 * RETURNS: string for displaying the elapsed/CPU time
 */
char *
mktimestr(tmsec, mode)
int     tmsec;  /* second       */
int     mode;   /* F_ETIME/TIME */
{
        int     i;
        char    *time_str;         /* string that is returned           */
        time_t  dd, hh, mm, ss;    /* value for dates/hours/minutes/seconds */
        char    *str_hms[3];       /* string for hours/minutes/seconds  */
        char    *def_str = "--";   /* If error, this string is returned.*/
        char    buff_h[10], buff_m[10], buff_s[10];

        time_str = (char *)malloc((size_t)32);
        if (time_str == NULL) {
                fprintf(stderr,MSGSTR(NO_MEM, "ps: no memory\n"));
                done(1);
        }
        if ( mode != F_ETIME && mode != TIME ) {
                strcpy(time_str, def_str);
                return (time_str);
        }

        str_hms[0] = buff_h;
        str_hms[1] = buff_m;
        str_hms[2] = buff_s;

        hh = tmsec / 3600;
        dd = hh / 24;
        hh = hh % 24;
        mm = (tmsec % 3600) / 60;
        ss = tmsec % 60;

        if (dd > 0)
                sprintf(str_hms[0], "%ld-%02ld", dd, hh);
        else if (hh > 0)
                sprintf(str_hms[0], "%02ld", hh);
        else {
                if (mode == F_ETIME)
                        *str_hms[0] = '\0';
                else    /* mode = TIME */
                        sprintf(str_hms[0], "%02ld", hh);
        }
        sprintf(str_hms[1], "%02ld", mm);
        sprintf(str_hms[2], "%02ld", ss);
        sprintf(time_str, "%s:%s:%s", str_hms[0], str_hms[1], str_hms[2]);
        if (*str_hms[0] == '\0')
                time_str++;     /* The first ":" is skipped.    */
        return (time_str);
}

/*
 * NAME: des_judge
 *
 * FUNCTION: Judge whether Descriptors Format or Specifiers Format.
 *
 * NOTES: If the format list is Descriptors Format, returns 1.
 *       If the format list is Specifiers Format, returns 0.
 *              For example,
 *                      Descriptors Format   ps -F "%u %p %P %t %a"
 *                      Specifiers  Format   ps -F ruser,pid,ppid,etime,args
 *              If the following strings are found in the list, this function
 *              will judge the list as Discriptors Format.
 *                      %a  (except for %arg                            )
 *                      %C  (except for %CPU, %Cpu, %COMMAND, %CMD      )
 *                      %c  (except for %cpu, %com, %command            )
 *                      %G  (except for %GROUP, %Group                  )
 *                      %g  (except for %group, %gname                  )
 *                      %n  (except for %nice                           )
 *                      %P  (except for %PID, %PPID, %PGID              )
 *                      %p  (except for %pid, %ppid, %pgid              )
 *                      %r  (except for %rgname, %runame                )
 *                      %t  (except for %tty, %time                     )
 *                      %U  (except for %USER, %User                    )
 *                      %u  (except for %user, %uname                   )
 *                      %x
 *                      %y
 *                      %z
 *              User may execute ps command as follows. But, des_judge()
 *              will be able to judge the list as Specifiers Format.
 *                      ps -F "runame=%USER pid=%PID etime=%TIME args=%CMD"
 *              (We need to reinfoce this function for judging correctly.)
 *
 * RETURNS: 0 ... Specifiers Format
 *          1 ... Descriptors Format
 */
int
des_judge(listptr)
char    *listptr;
{
        int     fcode, rec=0;
        int     des_fmt=0;

        while((fcode = *listptr++) != '\0') {
                if (fcode == '%') {
                        switch (fcode = *(listptr)) {
                        case 'a':
                                if ( strncmp("arg", listptr, 3) )
                                        des_fmt++;
                                break;
                        case 'C':
                                if ( strncmp("CPU", listptr, 3) &&
                                        strncmp("Cpu", listptr, 3) &&
                                        strncmp("CMD", listptr, 3) &&
                                        strncmp("COMMAND", listptr, 7) )
                                        des_fmt++;
                                break;
                        case 'c':
                                if ( strncmp("cpu", listptr, 3) &&
                                        strncmp("cmd", listptr, 3) &&
                                        strncmp("command", listptr, 7) )
                                        des_fmt++;
                                break;
                        case 'G':
                                if ( strncmp("GROUP", listptr, 5) &&
                                        strncmp("Group", listptr, 5) )
                                        des_fmt++;
                                break;
                        case 'g':
                                if ( strncmp("group", listptr, 5) &&
                                        strncmp("gname", listptr, 5) )
                                        des_fmt++;
                                break;
                        case 'n':
                                if ( strncmp("nice", listptr, 4) )
                                        des_fmt++;
                                break;
                        case 'P':
                                if ( strncmp("PID", listptr, 3) &&
                                        strncmp("PPID", listptr, 4) &&
                                        strncmp("PGID", listptr, 4) )
                                        des_fmt++;
                                break;
                        case 'p':
                                if ( strncmp("pid", listptr, 3) &&
                                        strncmp("ppid", listptr, 4) &&
                                        strncmp("pgid", listptr, 4) )
                                        des_fmt++;
                                break;
                        case 'r':
                                if ( strncmp("rgname", listptr, 6) &&
                                        strncmp("runame", listptr, 6) )
                                        des_fmt++;
                                break;
                        case 't':
                                if ( strncmp("tty", listptr, 3) &&
                                        strncmp("time", listptr, 4) )
                                        des_fmt++;
                                break;
                        case 'U':
                                if ( strncmp("USER", listptr, 4) &&
                                        strncmp("User", listptr, 4) )
                                        des_fmt++;
                                break;
                        case 'u':
                                if ( strncmp("user", listptr, 4) &&
                                        strncmp("uname", listptr, 5) )
                                        des_fmt++;
                                break;
                        case 'x':
                        case 'y':
                        case 'z':
				des_fmt++;
                        default:
                                break;
                        }

                        if (des_fmt) {
                                rec = 1;
                                break;
                        }
                }
        }
        return (rec);
}

/*
 * NAME: des_build
 *
 * FUNCTION: MakeUp Descriptors Format to the structure fmtdata.
 *
 * NOTES: The following is an example process of des_build().
 *      Executed command:  ps -F "%u %p %P %t : %a"
 *      The structure fmtdata:
 *              fmt_tbl[0] = { RUSER, ..... , "RUSER" }
 *              fmt_tbl[1] = { PAD_ID, .... , " " }
 *              fmt_tbl[2] = { PID, ........, "PID" }
 *              fmt_tbl[3] = { PAD_ID, .... , " " }
 *              fmt_tbl[4] = { PPID, ......., "PPID" }
 *              fmt_tbl[5] = { PAD_ID, .... , " " }
 *              fmt_tbl[6] = { F_ETIME, ......, "ELAPSED" }
 *              fmt_tbl[7] = { PAD_ID, .... , " : " }
 *              fmt_tbl[8] = { ARGS, ......., "COMMAND" }
 *
 * RETURNS: the structure fmtdata
 */
void
des_build(listptr)
char    *listptr;
{
        char    *head, *headbk, *t_name, *t_name_hd;
        int     fcode, i, missed;
        int     fieldid;

        if ((head = (char *)malloc((size_t)256)) == NULL) {
                fprintf(stderr,MSGSTR(NO_MEM, "ps: no memory\n"));
                done(1);
        }

        headbk = head;
        while((fcode = *listptr) != '\0') {
            if (fcode == '%') {
                missed = 0;
                switch (fcode = *(++listptr)) {
                case 'u':
                        fieldid = RUSER;
                        break;
                case 'U':
                        fieldid = USER;
                        break;
                case 'g':
                        fieldid = RGROUP;
                        break;
                case 'G':
                        fieldid = GROUP;
                        break;
                case 'p':
                        fieldid = PID;
                        break;
                case 'P':
                        fieldid = PPID;
                        break;
                case 'r':
                        fieldid = PGID;
                        break;
                case 'C':
                        fieldid = PCPU;
                        break;
                case 'z':
                        fieldid = VSZ;
                        break;
                case 'n':
                        fieldid = NICE;
                        break;
                case 't':
                        fieldid = F_ETIME;
                        break;
                case 'x':
                        fieldid = TIME;
                        break;
                case 'y':
                        fieldid = TTY;
                        break;
                case 'c':
                        fieldid = COMM;
                        break;
                case 'a':
                        fieldid = ARGS;
                        break;
                default:
                        /* not descripter       */
                        missed++;
                        break;
                }
                if (!missed) {
                        /* found descriptor     */
                        if ((t_name = (char *)malloc((size_t)256)) == NULL) {
                                fprintf(stderr,MSGSTR(NO_MEM,
                                        "ps: no memory\n"));
                                done(1);
                        }
                        strncpy(t_name, headbk, (head - headbk));
                        t_name_hd = t_name + (head - headbk);
                        *t_name_hd = '\0';
                        fmt_tbl[nfmt].fieldid = PAD_ID;
                        fmt_tbl[nfmt].titlename = t_name;
                        nfmt++;

                        for (i=0; tnames[i].fieldid != ENDID;
                                i++) {
                            if (tnames[i].fieldid == fieldid) {
                                fmt_tbl[nfmt].fieldid = fieldid;
                                if ((t_name = (char *)malloc((size_t)64))
                                        == NULL) {
                                        fprintf(stderr,MSGSTR(NO_MEM,
                                                "ps: no memory\n"));
                                        done(1);
                                }
                                strcpy(t_name, MSGSTR(tnames[i].msg_id,
                                        tnames[i].def_name));
                                fmt_tbl[nfmt].titlename = t_name;
                                fmt_tbl[nfmt].colsize =
                                          dispwidth(fmt_tbl[nfmt].titlename);
                            }
                        }
                        head = headbk;
                        nfmt++;
                }
                else {
                        *head++ = *(listptr--);
                        *head++ = *(listptr++);
                }
                listptr++;
            }
            else {
                *head++ = *listptr++;
            }
        }
        if (head != headbk) {
                fmt_tbl[nfmt].fieldid = PAD_ID;
                *head = '\0';
                fmt_tbl[nfmt].titlename = headbk;
                nfmt++;
        }
        return;
}

/*
 * NAME: dispwidth
 *
 * FUNCTION: Returns the display width
 *
 * NOTES: N/A
 *
 * RETURNS: display width
 */
size_t
dispwidth(mbs_str)
char    *mbs_str;
{
        size_t  n, rec, dsplen;
        wchar_t *pwcs;

        n = strlen( mbs_str ) + 1;
        if ( (pwcs = (wchar_t *)malloc( n * sizeof(wchar_t))) == NULL ) {
                fprintf(stderr,MSGSTR(NO_MEM, "ps: no memory\n"));
                done(1);
        }
        if ( (rec = mbstowcs( pwcs, mbs_str, n )) == -1 ) {
                free(pwcs);
                return ((size_t)0);
        }
        if ( (dsplen = wcswidth( pwcs, rec )) == -1) {
                free(pwcs);
                return ((size_t)0);
        }
        free(pwcs);
        return (dsplen);
}

/*
 * NAME: getfieldlen
 *
 * FUNCTION: Set several field width.
 *
 * NOTES: N/A
 *
 * RETURNS: The following extern variable are set.
 *              PidLen, UidLen, UserLen, GidLen, DateLen
 */
void
getfieldlen(pro, n)
STRUCT_PROCINFO        *pro;
long    n;
{
        PidLen  = getmaxpidlen(pro,n);
        UidLen  = getmaxuidlen(pro,n);
        UserLen = 8;
        GidLen  = 5;
        GroupLen= 8;
        DateLen = getmaxdatelen();
#ifdef _THREADS
	if ((mflg)||(thrflg))
		TidLen  = getmaxtidlen();
#endif /*_THREADS */
        return;
}

/*
 * NAME: print_line
 *
 * FUNCTION: Display line to stdout with wrapping.
 *
 * NOTES: N/A
 *
 * RETURNS: N/A
 */
void
print_line()
{
	int line_size;		/* Size of output line */
	char *line_start;	/* Pointer to position in line */

	/* Set line_size to length of message to be displayed */
	line_size = strlen(line_out);

	/* Set pointer to beginning of message */
	line_start = line_out;

	/*
	 * Print out screen_cols number of characters to screen. If
	 * message is wider than screen_cols, continue doing this
	 * until remainder of message is less than screen_cols
	 * characters.
	 */
	while (line_size > screen_cols) {
		printf("%.*s", screen_cols, line_start);
		line_start += screen_cols;
		line_size -= screen_cols;
		/*
		 * if only one char left, it is probably
		 * a newline, else print a newline.
		 */
		if (line_size > 1)
			printf("\n");
	}

	/*
	 * Print remainder of message to screen
	 */
	if (line_size > 0)
		printf("%s", line_start);
}

/*
 * NAME: prblank
 *
 * FUNCTION: fills the buffer with blanks
 *
 * NOTES: N/A
 *
 * RETURNS: N/A
 */
void
prblank(int n)
{
	int i;
	for(i=0;i<(n);i++) {
                chcnt = sprintf(&line_out[chindx], " ");
		chindx += chcnt;
	}
	chcnt = n;
}


char
thread_state(struct thrdsinfo *cur_thread)
{

	 switch (cur_thread->ti_state) {
		 case TSNONE :
			 return('O');
		 case TSIDL :
			 return('I');
		 case TSRUN :
			 return('R');
		 case TSSLEEP :
			 return('S');
		 case TSSWAP :
			 return('W');
		 case TSSTOP :
			 return('T');
		 case TSZOMB :
			 return('Z');
		 default :
			 return('?');
	 }
}

/*
 * NAME: strpad
 *
 * FUNCTION: Pads input string to specified size.
 *
 * Note: right justification means lined on even on right hand side
 *       left justification means lined on even on left hand side
 *
 * RETURNS: Padded string
 */
char *
strpad(char *str, size_t size, int justify)
{
	static char buf[BUFSIZ];
	static wchar_t wbuf[BUFSIZ], tmpbuf[BUFSIZ];
	size_t curlen, i, curwidth;

	if (MB_CUR_MAX == 1) {
		if (justify == RIGHT)
			sprintf(buf, "%*.*s", size, size, str);
		else
			sprintf(buf, "%-*.*s", size, size, str);
		return buf;
	}

	strncpy(buf, str, size);
	buf[size] = '\0';

	curlen = mbstowcs(tmpbuf, buf, BUFSIZ);
	if (curlen == (size_t) -1)
	    curlen = 0;

	for(i=0; i<curlen; i++)
		if (!iswprint(tmpbuf[i]))
			tmpbuf[i] = L' ';

	tmpbuf[curlen] = L'\0';
	curwidth = wcswidth(tmpbuf, BUFSIZ);
	if (curwidth == -1)
		curwidth = 1;

	i = 0;
	wbuf[0] = L'\0';

	if (justify == LEFT) {
		wcscat(wbuf, tmpbuf);
		i += curlen;
	}
	while (curwidth < size) {
		curwidth++;
		wbuf[i++] = L' ';
	}
	wbuf[i] = L'\0';
	if (justify == RIGHT) {
		wcscat(wbuf, tmpbuf);
		i += curlen;
	}

	while(curwidth > size)
		curwidth--;
	wbuf[curwidth] = L'\0';

	sprintf(buf, "%S", wbuf);
	return buf;
}

