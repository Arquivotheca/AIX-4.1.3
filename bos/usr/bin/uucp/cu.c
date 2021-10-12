static char sccsid[] = "@(#)87	1.14.1.15  src/bos/usr/bin/uucp/cu.c, cmduucp, bos411, 9428A410j 6/30/94 15:40:06";
/* 
 * COMPONENT_NAME: CMDUUCP cu.c
 * 
 * FUNCTIONS: Mcu, _bye, _dopercen, _flush, _mode, _onintrpt, _quit, 
 *            _rcvdead, _receive, _shell, _w_str, blckcnt, 
 *            cleanup, dofork, logent, r_char, recfork, sysname, 
 *            tdmp, tilda, transmit, w_char, readpipe, givename
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* static char sccsid[] = "cu.c	5.1 -  - "; */
/* cu.c	1.34 */
/********************************************************************
 *cu [-sspeed] [-lline] [-h] [-t] [-d] [-n] [-o|-e] telno | systemname 
 *
 *	legal baud rates: 300, 1200, 2400, 4800, 9600.
 *
 *	-l is for specifying a line unit from the file whose
 *		name is defined in /etc/uucp/Devices.
 *	-h is for half-duplex (local echoing).
 *	-t is for adding CR to LF on output to remote (for terminals).
 *	-d can be used  to get some tracing & diagnostics.
 *	-o or -e is for odd or even parity on transmission to remote.
 *	-m will ignore carrier status on the tty device
 *	-n will request the phone number from the user.
 *	-T changes the default timeout.
 *	Telno is a telephone number with `=' for secondary dial-tone.
 *	If "-l dev" is used, speed is taken from /etc/uucp/Devices.
 *	Only systemnames that are included in /etc/uucp/Systems may
 *	be used.
 *
 *	Escape with `~' at beginning of line.
 *	Silent output diversions are ~>:filename and ~>>:filename.
 *	Terminate output diversion with ~> alone.
 *	~. is quit, and ~![cmd] gives local shell [or command].
 *	Also ~$ for local procedure with stdout to remote.
 *	Both ~%put from [to]  and  ~%take from [to] invoke built-ins.
 *	Also, ~%break or just ~%b will transmit a BREAK to remote.
 *	~%nostop toggles on/off the DC3/DC1 input control from remote,
 *		(certain remote systems cannot cope with DC3 or DC1).
 *	~%debug(~%b) toggles on/off the program tracing and diagnostics output;
 *	cu should no longer be compiled with #ifdef ddt.
 *	Capture to a file is ~o[:filename], where :filename is optional. If
 *	the file name is not present the default capture file is use
 *	(capture.log).
 *
 *	Cu no longer uses dial.c to reach the remote.  Instead, cu places
 *	a telephone call to a remote system through the uucp conn() routine
 *	when the user picks the systemname option or through altconn()--
 *	which bypasses /etc/uucp/Systems -- if a telno or direct
 *	line is chosen. The line termios attributes are set in fixline(),
 *	before the remote connection is made.  As a device-lockout semaphore
 *	mechanism, uucp creates an entry in /usr/spool/locks whose name is
 *	LCK..dev where dev is the device name from the Devices file.
 *	When cu terminates, for whatever reason, cleanup() must be
 *	called to "release" the device, and clean up entries from
 *	the locks directory.  Cu runs with uucp ownership, and thus provides
 *	extra insurance that lock files will not be left around.	
# ******************************************************************/

#include "uucp.h"
#include <string.h>
#include <termios.h>

nl_catd catd;
#define MID	BUFSIZ/2	/* mnemonic */
#define	RUB	'\177'		/* mnemonic */
#define	XON	'\21'		/* mnemonic */
#define	XOFF	'\23'		/* mnemonic */
#define	TTYIN	0		/* mnemonic */
#define	TTYOUT	1		/* mnemonic */
#define	TTYERR	2		/* mnemonic */
#define	YES	1		/* mnemonic */
#define	NO	0		/* mnemonic */
#define IOERR	4		/* exit code */
#define	MAXPATH1	100
#define	NPL	50
#define	CAPTURE_FILE	"capture.log"

int Sflag=0;
int Cn;				/*fd for remote comm line */
static char *_Cnname;		/*to save associated ttyname */
jmp_buf Sjbuf;			/*needed by uucp routines*/

/*      io buffering    */
/*      Wiobuf contains, in effect, 3 write buffers (to remote, to tty  */
/*      stdout, and to tty stderr) and Riobuf contains 2 read buffers   */
/*      (from remote, from tty).  [WR]IOFD decides which one to use.    */
/*      [RW]iop holds current position in each.                         */
#define WIOFD(fd)       (fd == TTYOUT ? 0 : (fd == Cn ? 1 : 2))
#define RIOFD(fd)       (fd == TTYIN ? 0 : 1)
#define WRIOBSZ 256
static char Riobuf[2*WRIOBSZ];
static char Wiobuf[3*WRIOBSZ];
static int Riocnt[2]={0,0};
static char *Riop[2];
static char *Wiop[3];

extern int
	errno,			/* supplied by system interface */
	optind;			/* variable in getopt() */

extern int tflag;
extern int maxexpecttime;

extern char
	*optarg;

static struct call {		/*NOTE-also included in altconn.c--> */
				/*any changes must be made in both places*/
	char *speed;		/* transmission baud rate */
	char *line;		/* device name for outgoing line */
	char *telno;		/* ptr to tel-no digit string */
	char *class;		/* call class */
	}Cucall;
	
static int Saved_tty;		/* was TCGETAW of _Tv0 successful?      */
static struct termios _Tv, _Tv0;/* for saving, changing TTY atributes */
static struct termios _Lv;	/* attributes for the line to remote */
static struct utsname utsn; 

static char
	_Cxc,			/* place into which we do character io*/
	_Tintr,			/* current input INTR */
	_Tquit,			/* current input QUIT */
	_Terase,		/* current input ERASE */
	_Tkill,			/* current input KILL */
	_Teol,			/* current secondary input EOL */
	_Myeof;			/* current input EOF */

int
	Terminal=0,		/* flag; remote is a terminal */
	Oddflag = 0,		/*flag- odd parity option*/
	Evenflag = 0,		/*flag- even parity option*/
	Echoe,			/* save users ECHOE bit */
	Echok,			/* save users ECHOK bit */
	Child,			/* pid for receive process */
	Intrupt=NO,		/* interrupt indicator */
	Duplex=YES,		/* Unix= full duplex=YES; half = NO */ 
	Sstop=YES,		/* NO means remote can't XON/XOFF */
	Rtn_code=0,		/* default return code */
	Takeflag=NO;		/* indicates a ~%take is in progress */


int     logme=0;                /* boolean, logfile on or off? */
int     tlog=0;                 /* transmit log, on or off? */
int     sockets[2];             /* fd for the parent->child pipe */
char    *pasd;                  /* pointer to ~o:filename stuff */
char    *logf;                  /* pointer to capture filename */
FILE    *fdcap;                 /* FILE pointer to capture fd */
mode_t	omask;			/* Just satisfies link to utility.c */

static void
	_onintrpt(int),		/* interrupt routines */
	_rcvdead(int),
	_quit(int),
	_bye(int);

void readpipe(int);

extern void	cleanup(int);
extern void     tdmp();
extern int	Modemctrl;		/* soft carrier flag */

static int r_char(int), w_char(int), dofork();

static void
	_flush(),
	_shell(),
	_dopercen(),
	_receive(),
	_mode(),
	_w_str();

char *Myline = NULL;  /* flag to force the requested line to be used  */
		      /* by rddev() in uucp conn.c		    */

/* Message translation for the following messages is done in line */
char *P_USAGE= "USAGE:%s [-s speed] [-l line] [-m] [-h] [-n] [-t] [-d] [-T seconds]\\\n\t\t[-o|-e] telno | systemname \n";
char *P_CON_FAILED = "Connect failed: %s\r\n";
char *P_Ct_OPEN = "Cannot open: %s\r\n";
char *P_LINE_GONE = "Remote line gone\r\n";
char *P_Ct_EXSH = "Can't execute shell\r\n";
char *P_Ct_DIVERT = "Can't divert %s\r\n";
char *P_STARTWITH = "Use `~~' to start line with `~'\r\n";
char *P_CNTAFTER = "after %ld bytes\r\n";
char *P_CNTLINES = "%d lines/";
char *P_CNTCHAR = "%ld characters\r\n";
char *P_FILEINTR = "File transmission interrupted\r\n";
char *P_Ct_FK = "Can't fork -- try later\r\n";
char *P_Ct_SPECIAL = "r\nCan't transmit special character `%#o'\r\n";
char *P_TOOLONG = "\nLine too long\r\n";
char *P_IOERR = "r\nIO error\r\n";
char *P_USECMD = "Use `~$'cmd \r\n"; 
char *P_USEPLUSCMD ="Use `~+'cmd \r\n";
char *P_NOTERMSTAT = "Can't get terminal status\r\n";
char *P_3BCONSOLE = "Sorry, you can't cu from a 3B console\r\n";
char *P_PARITY  = "Parity option error\r\n";
char *P_TELLENGTH = "Telno cannot exceed 58 digits!\r\n";

extern struct termios	Savelineb;
/***************************************************************
 *	main: get command line args, establish connection, and fork.
 *	Child invokes "receive" to read from remote & write to TTY.
 *	Main line invokes "transmit" to read TTY & write to remote.
 ***************************************************************/

main(argc, argv)
char *argv[];
{
	extern void setservice();
	extern int sysaccess();
	struct stat buff;
	struct stat bufsave;
	char s[MAXPH];
	char *string;
	int i, j =0;
	int cflag=0;
	int errflag=0;
	int nflag=0;
	int lflag=0;
	int systemname = 0;

        Riop[0] = &Riobuf[0];
        Riop[1] = &Riobuf[WRIOBSZ];
        Wiop[0] = &Wiobuf[0];
        Wiop[1] = &Wiobuf[WRIOBSZ];
        Wiop[2] = &Wiobuf[2*WRIOBSZ];

	Verbose = 1;		/*for uucp callers,  dialers feedback*/
        strcpy(Progname,"cu");
        setservice(Progname);
	if ( sysaccess(EACCESS_SYSTEMS) != 0 ) {
		(void)fprintf(stderr, MSGSTR(MSG_CU23, "cu: cannot read Systems files\n"));
		exit(1);
	}
	if ( sysaccess(EACCESS_DEVICES) != 0 ) {
		(void)fprintf(stderr, MSGSTR(MSG_CU24, "cu: cannot read Devices files\n"));
		exit(1);
	}
	if ( sysaccess(EACCESS_DIALERS) != 0 ) {
		(void)fprintf(stderr, MSGSTR(MSG_CU25, "cu: cannot read Dialers files\n"));
		exit(1);
	}


	Cucall.speed = "Any";       /*default speed*/
	Cucall.line = NULL;
	Cucall.telno = NULL;
	Cucall.class = NULL;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);


		
/* Flags for -h, -t, -e, and -o options set here;     */
/* corresponding line attributes are set in fixline() */
/* in culine.c before remote connection is made       */

	while((i = getopt(argc, argv, "mdhteons:l:c:T:")) != EOF)
		switch(i) {
			case 'm':
				Modemctrl = 1;
				break;
			case 'd':
				Debug = 9; /*turns on uucp debugging-level 9*/
				break;
			case 'h':
				Duplex  = NO;
				Sstop = NO;
				break;
			case 't':
				Terminal = YES;
				break;
			case 'e':
                               if ( Oddflag ) {
                                        (void)fprintf(stderr, MSGSTR(MSG_CU26, "cu: cannot have both even and odd parity\n"));
                                        exit(1);
                                }
				Evenflag = 1;
				break;
			case 'o':
                               if ( Evenflag ) {
                                        (void)fprintf(stderr, MSGSTR(MSG_CU26, "cu: cannot have both even and odd parity\n"));
                                        exit(1);
                                }
				Oddflag = 1;
				break;
			case 's':
				Sflag++;
				Cucall.speed = optarg;
			  	break;
			case 'l':
				lflag++;
				Cucall.line = optarg;
				break;
#ifdef forfutureuse
/* -c specifies the class-selecting an entry type from the Devices file */
			case 'c':
				Cucall.class = optarg;
				break;
#endif
			case 'n':
				nflag++;
				printf(MSGSTR(MSG_CU22,
					"Please enter the number: "));
				gets(s);
				break;
			case 'T':
				maxexpecttime = atoi(optarg);
				tflag = 1;
				break;
			case '?':
				++errflag;
		}


	/*
	** open a pipe to be used for parent->child IPC.
	** a SIGUSR2 tells the child to fetch the command from the pipe
	*/

	if (pipe(sockets) < 0) {
		perror("open pipe");
		exit(10);
	}


#ifdef  u3b
	if(fstat(TTYIN, &buff) < 0) {
		VERBOSE(MSGSTR(MSG_CU18, P_NOTERMSTAT),"");
		exit(1);
	} else if(buff.st_rdev == 0) {
		VERBOSE(MSGSTR(MSG_CU19, P_3BCONSOLE),"");
		exit(1);
		}
#endif

	if((optind < argc && optind > 0) || (nflag && optind > 0)) {  
		if(nflag) 
			string=s;
		else
			string = argv[optind];
		if((strlen(string) == strspn(string, "0123456789=-*#,")) ||
		   (Cucall.class != NULL))
			Cucall.telno = string;
		else {			/*if it's not a legitimate telno,  */
					/*then it should be a systemname   */
                        if ( nflag ) {
                                (void)fprintf(stderr, MSGSTR(MSG_CU27,
                                "cu: bad phone number %s\nPhone numbers may contain only the digits 0 through 9 and the special\ncharacters =, -, * and #.\n"), 
				string );
                                exit(1);
                        }
			systemname++;
		}
	} else
		if(Cucall.line == NULL)   /*if none of above, must be direct */
			++errflag;
	
	if(errflag) {
		VERBOSE(MSGSTR(MSG_CU1A, P_USAGE), argv[0]);
		exit(1);
	}

	if(strlen(Cucall.telno) >= (MAXPH - 1)) {
		VERBOSE(MSGSTR(MSG_CU21, P_TELLENGTH),"");
		exit(0);
	}

	/* save initial tty state */
	Saved_tty = ( tcgetattr(TTYIN, &_Tv0) == 0 );
	_Tintr = _Tv0.c_cc[VINTR]? _Tv0.c_cc[VINTR]: '\377';
	_Tquit = _Tv0.c_cc[VQUIT]? _Tv0.c_cc[VQUIT]: '\377';
	_Terase = _Tv0.c_cc[VERASE]? _Tv0.c_cc[VERASE]: '\377';
	_Tkill = _Tv0.c_cc[VKILL]? _Tv0.c_cc[VKILL]: '\377';
	_Teol = _Tv0.c_cc[VEOL]? _Tv0.c_cc[VEOL]: '\377';
	_Myeof = _Tv0.c_cc[VEOF]? _Tv0.c_cc[VEOF]: '\04';
	Echoe = _Tv0.c_lflag & ECHOE;
	Echok = _Tv0.c_lflag & ECHOK;

	(void)signal(SIGHUP, (void(*)(int)) cleanup);
	(void)signal(SIGQUIT, (void(*)(int)) cleanup);
	(void)signal(SIGINT, (void(*)(int)) cleanup);

/* place call to system; if "cu systemname", use conn() from uucp
   directly.  Otherwise, use altconn() which dummies in the
   Systems file line.
*/

	if(systemname) {
                if ( lflag )
                        (void)fprintf(stderr, MSGSTR(MSG_CU28,
                        "cu: warning: -l flag ignored when system name used\n"));
                if ( Sflag )
                        (void)fprintf(stderr, MSGSTR(MSG_CU29,
                        "cu: warning: -s flag ignored when system name used\n"));
		Cn = conn(string);
	} else
		Cn = altconn(&Cucall);

	_Cnname = ttyname(Cn);
	if(_Cnname != NULL) {
                struct stat Cnsbuf;
                if ( fstat(Cn, &Cnsbuf) == 0 )
                        Dev_mode = Cnsbuf.st_mode;
                else
                        Dev_mode = R_DEVICEMODE;
		chown(_Cnname, UUCPUID, UUCPGID);
		chmod(_Cnname, M_DEVICEMODE);
	}

	Euid = geteuid();
	if(seteuid(getuid()) && setegid(getgid()) < 0) {
		VERBOSE(MSGSTR(MSG_CUV1,"Unable to seteuid/gid\n"),"");
		cleanup(0);
		}

	if(Cn < 0) {

		VERBOSE(MSGSTR(MSG_CU2, P_CON_FAILED), UerrorText(Uerror));
		cleanup(-Cn);
	}


	if(Debug) tdmp(Cn); 

	/* At this point succeeded in getting an open communication line */
	/* Conn() takes care of closing the Systems file                 */

	(void)signal(SIGINT,(void(*)(int)) _onintrpt);
	_mode(1);			/* put terminal in `raw' mode */
	VERBOSE(MSGSTR(MSG_CUV2,"Connected\007\r\n"),"");	/*bell!*/

        /* must catch signals before fork.  if not and if _receive()	*/
        /* fails in just the right (wrong?) way, _rcvdead() can be	*/
        /* called and do "kill(getppid(),SIGUSR1);" before parent	*/
        /* has done calls to signal() after recfork().			*/
        (void)signal(SIGUSR1, (void(*) (int))_bye);
        (void)signal(SIGHUP, (void(*) (int))cleanup);
        (void)signal(SIGQUIT, (void(*) (int))_onintrpt);


	recfork();		/* checks for child == 0 */

	if(Child > 0) {  /* parent */
		close(sockets[0]);  /* close 'read' end of pipe */
		Rtn_code = transmit();
		_quit(Rtn_code);
	} else {
		cleanup(Cn);
	}
}

/*
 *	Kill the present child, if it exists, then fork a new one.
 */

recfork()
{
	if (Child)
		kill(Child, SIGKILL);
	Child = dofork();
	if(Child == 0) {
		(void)signal(SIGHUP, (void(*)(int)) _rcvdead);
		(void)signal(SIGQUIT, SIG_IGN);
		(void)signal(SIGINT, SIG_IGN);
		(void)signal(SIGUSR2, (void(*)(int)) readpipe);
		close(sockets[1]); /* close one end of pipe */
		_receive();	/* This should run until killed */
		/*NOTREACHED*/
	}
}

/***************************************************************
 *	transmit: copy stdin to remote fd, except:
 *	~.	terminate
 *	~!	local login-style shell
 *	~!cmd	execute cmd locally
 *	~$proc	execute proc locally, send output to line
 *	~%cmd	execute builtin cmd (put, take, or break)
 ****************************************************************/
#ifdef forfutureuse
 /*****************************************************************
  *	~+proc	execute locally, with stdout to and stdin from line.
  ******************************************************************/
#endif

int
transmit()
{
	char b[BUFSIZ];
	char prompt[sizeof (struct utsname)];
	register char *p;
	register int escape;
	register int id = 0;  /*flag for systemname prompt on tilda escape*/

	CDEBUG(4,MSGSTR(MSG_CUCD1,"transmit started\n\r"),"");
	sysname(prompt);

	/* In main loop, always waiting to read characters from  */
	/* keyboard; writes characters to remote, or to TTYOUT   */
	/* on a tilda escape                                     */

	while(TRUE) {
		p = b;
		while(r_char(TTYIN) == YES) {
			if(p == b)  	/* Escape on leading  ~    */
				escape = (_Cxc == '~');
			if(p == b+1)   	/* But not on leading ~~   */
				escape &= (_Cxc != '~');
			if(escape) {
			     if(_Cxc == '\n' || _Cxc == '\r' || _Cxc == _Teol) {
					*p = '\0';
					if(tilda(b+1) == YES)
						return(0);
					id = 0;
					break;
				}
				if(_Cxc == _Tintr || _Cxc == _Tkill || _Cxc
					 == _Tquit ||
					    (Intrupt && _Cxc == '\0')) {
					if(_Cxc == _Tkill) {
						if(Echok)
							VERBOSE("\r\n","");
					}
					else {
					_Cxc = '\r';
						if( w_char(Cn) == NO) {
							VERBOSE(P_LINE_GONE,"");
							return(IOERR);
						}
					id=0;
					}
					break;
				}
				if((p == b+1) && (_Cxc != _Terase) && (!id)) {
					id = 1;
					VERBOSE("[%s]", prompt);
				}
				if(_Cxc == _Terase) { 
					p = (--p < b)? b:p;
					if(p > b)
						if(Echoe)
							VERBOSE("\b \b", "");
						else 
						 	(void)w_char(TTYOUT);
				} else {
					(void)w_char(TTYOUT);
					if(p-b < BUFSIZ) 
						*p++ = _Cxc;
					else {
						VERBOSE(MSGSTR(MSG_CU14,
							P_TOOLONG),"");
						break;
					}
				}
	/*not a tilda escape command*/
			} else {
				if(Intrupt && _Cxc == '\0') {
					CDEBUG(4,MSGSTR(MSG_CUCD2,
					  "got break in transmit\n\r"),"");
					Intrupt = NO;
					(*genbrk)(Cn);
					_flush();
					break;
				}
				if(w_char(Cn) == NO) {
					VERBOSE(MSGSTR(MSG_CU4,P_LINE_GONE),"");
					return(IOERR);
				}
				if(Duplex == NO)
					if((w_char(TTYERR) == NO) ||
					   (wioflsh(TTYERR) == NO))
						return(IOERR);
				if ((_Cxc == _Tintr) || (_Cxc == _Tquit) ||
					 ( (p==b) && (_Cxc == _Myeof) ) ) {
					CDEBUG(4,MSGSTR(MSG_CUCD3,
						"got a tintr\n\r"),"");
					_flush();
					break;
				}
				if(_Cxc == '\n' || _Cxc == '\r' ||
					_Cxc == _Teol || _Cxc == _Tkill) {
					id=0;
					Takeflag = NO;
					break;
				}
				p = (char*)0;
			}
		}
	}
}

/***************************************************************
 *	routine to halt input from remote and flush buffers
 ***************************************************************/
static void
_flush()
{
	(void)tcflow(TTYOUT, TCOOFF);	/* stop tty output */
	(void)tcflush(Cn, TCIFLUSH);	/* flush remote input */
	(void)tcflush(TTYOUT, TCOFLUSH);	/* flush tty output */
	(void)tcflow(TTYOUT, TCOON);	/* restart tty output */
	if(Takeflag == NO) {
		return;		/* didn't interupt file transmission */
	}
	VERBOSE(MSGSTR(MSG_CU11,P_FILEINTR),"");
	(void)sleep(3);
	_w_str("echo '\n~>\n';mesg y;stty echo\n");
	Takeflag = NO;
}

/**************************************************************
 *	command interpreter for escape lines
 **************************************************************/
int
tilda(cmd)
register char	*cmd;
{

	VERBOSE("\r\n","");
	CDEBUG(4,MSGSTR(MSG_CUCD4, "call tilda(%s)\r\n"), cmd);

	switch(cmd[0]) {
		case '.':
			if (tlog) {
				tlog = 0;
				kill(Child, SIGUSR2);
				VERBOSE("(continue [LOGGING OFF])","");
				sleep(1);
			}
			if(Cucall.telno == NULL)
				if(cmd[1] != '.') {
					_w_str("\04");
					if (Child)
						kill(Child, SIGKILL);
        				/* speed to zero for hangup */ 
					_flush();
					(void) tcgetattr(Cn, &_Lv);
					_Lv.c_cflag |= (HUPCL | B0);
        				(void) tcsetattr(Cn, TCSANOW, &_Lv);
        				(void) sleep (2);
				}
			return(YES);
		case '!':
			_shell(cmd);	/* local shell */
			VERBOSE("\r%c\r\n", *cmd);
			VERBOSE(MSGSTR(MSG_CUV3,"(continue)"),"");
			break;
		case '$':
			if(cmd[1] == '\0') {
				VERBOSE(MSGSTR(MSG_CU16, P_USECMD),"");
				VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			}
			else {
				_shell(cmd);	/*Local shell  */
				VERBOSE("\r%c\r\n", *cmd);
			}
			break;	

#ifdef forfutureuse
		case '+':
			if(cmd[1] == '\0') {
				VERBOSE(MSGSTR(MSG_CU17,P_USEPLUSCMD), "");
				VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			}
			else {
				if (*cmd == '+')
					      /* must suspend receive to give*/
					      /*remote out to stdin of cmd */
					kill(Child, SIGKILL);
					 _shell(cmd);	/* Local shell */
				if (*cmd == '+')
					recfork();
				VERBOSE("\r%c\r\n", *cmd);
			}
			break;
#endif
		case '%':
			_dopercen(++cmd);
			break;
			
		case 't':
			tdmp(TTYIN);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			break;
		case 'l':
			tdmp(Cn);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			break;
		case 'o':
			DEBUG(9,"### passed ~o = |%s|\n", cmd);
			if (tlog) {
				tlog = 0;
				kill(Child, SIGUSR2);
				VERBOSE("(continue [LOGGING OFF])","");
				break;
			}
			pasd = strstr(cmd, ":");
			if (pasd != NULL)
				*pasd++;  /* go past : */
			DEBUG(9,"### filename = |%s|\n", pasd);
			givename(pasd); /* queue write to pipe */
			if (kill(Child, SIGUSR2)) {
				printf("~o failed;  sorry.\n");
				/* break; */
			}
			VERBOSE(MSGSTR(MSG_CUV3, "(continue [LOGGING TO %s])"), (pasd)? pasd : CAPTURE_FILE);
			break;
		default:
			VERBOSE(MSGSTR(MSG_CU7, P_STARTWITH),"");
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			break;
	}
	return(NO);
}

/***************************************************************
 *	The routine "shell" takes an argument starting with
 *	either "!" or "$", and terminated with '\0'.
 *	If $arg, arg is the name of a local shell file which
 *	is executed and its output is passed to the remote.
 *	If !arg, we escape to a local shell to execute arg
 *	with output to TTY, and if arg is null, escape to
 *	a local shell and blind the remote line.  In either
 *	case, '^D' will kill the escape status.
 **************************************************************/

#ifdef forfutureuse
/***************************************************************
 *	Another argument to the routine "shell" may be +.  If +arg,
 *	arg is the name of a local shell file which is executed with
 *	stdin from and stdout to the remote.
 **************************************************************/
#endif

static void
_shell(str)
char	*str;
{
	int	fk;
	void	(*xx)(int), (*yy)(int);
       

	CDEBUG(4,MSGSTR(MSG_CUCD5, "call _shell(%s)\r\n"), str);
	fk = dofork();
	if(fk < 0)
		return;
	_mode(0);	/* restore normal tty attributes */
	xx = signal(SIGINT, SIG_IGN);
	yy = signal(SIGQUIT, SIG_IGN);
	if(fk == 0) {
		char *shell, *getenv();
		shell = getenv("SHELL");

		if(shell == NULL)
			shell = "/bin/bsh";
					   /*user's shell is set if*/
					   /*different from default*/
		(void)close(TTYOUT);

		/***********************************************
		 * Hook-up our "standard output"
		 * to either the tty for '!' or the line
		 * for '$'  as appropriate
		 ***********************************************/
#ifdef forfutureuse

		/************************************************
		 * Or to the line for '+'.
		 **********************************************/
#endif

		(void)fcntl((*str == '!')? TTYERR:Cn,F_DUPFD,TTYOUT);

#ifdef forfutureuse
		/*************************************************
		 * Hook-up "standard input" to the line for '+'.
		 * **********************************************/
		if (*str == '+')
			{
			(void)close(TTYIN);
			(void)fcntl(Cn,F_DUPFD,TTYIN);
			}
#endif

		/***********************************************
		 * Hook-up our "standard input"
		 * to the tty for '!' and '$'.
		 ***********************************************/

		(void)close(Cn);   	/*parent still has Cn*/
		(void)signal(SIGINT, SIG_DFL);
		(void)signal(SIGHUP, SIG_DFL);
		(void)signal(SIGQUIT, SIG_DFL);
		(void)signal(SIGUSR1, SIG_DFL);
		if(*++str == '\0')
			(void)execl(shell,shell,(char*)0,(char*)0,0);
		else
			(void)execl(shell,"bsh","-c",str,0);
		VERBOSE(MSGSTR(MSG_CU5, P_Ct_EXSH), "");
		exit(0);
	}
	while(wait((int*)0) != fk);
	(void)signal(SIGINT, xx);
	(void)signal(SIGQUIT, yy);
	_mode(1);
}


/***************************************************************
 *	This function implements the 'put', 'take', 'break', and
 *	'nostop' commands which are internal to cu.
 ***************************************************************/

static void
_dopercen(cmd)
register char *cmd;
{
	char	*arg[5];
	char	*getpath, *getenv();
	char	mypath[MAXPATH1];
	int	narg;

	blckcnt((long)(-1));

	CDEBUG(4,MSGSTR(MSG_CUCD6, "call _dopercen(\"%s\")\r\n"), cmd);

	arg[narg=0] = strtok(cmd, " \t\n");

		/* following loop breaks out the command and args */
	while((arg[++narg] = strtok((char*) NULL, " \t\n")) != NULL) {
		if(narg < 4)
			continue;
		else
			break;
	}

	/* ~%take file option */
	if(EQUALS(arg[0], "take")) {
		if(narg < 2 || narg > 3) {
			VERBOSE(MSGSTR(MSG_CUV4,"usage: ~%%take from [to]\r\n"),
				"");
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
		if(narg == 2)
			arg[2] = arg[1];

		/*
 		* be sure that the remote file (arg[1]) exists before
 		* you try to take it.   otherwise, the error message from
 		* cat will wind up in the local file (arg[2])
 		*
 		* what we're doing is:
 		*      stty -echo; \
 		*      if test -r arg1
 		*      then (echo '~'>:arg2; cat arg1; echo '~'>)
 		*      else echo can't open: arg1
 		*      fi; \
 		*      stty echo
 		*
 		*/
		_w_str("stty -echo;if test -r ");
		_w_str(arg[1]);
		_w_str("; then (echo '~>':");
		_w_str(arg[2]);
		_w_str(";cat ");
		_w_str(arg[1]);
		_w_str(";echo '~>'); else echo cant\\'t open: ");
		_w_str(arg[1]);
		_w_str("; fi;stty echo\n");
		Takeflag = YES;
		return;
	}
	/* ~%put file option*/
	if(EQUALS(arg[0], "put")) {
		FILE	*file;
		char	ch, buf[BUFSIZ], spec[NCCS+1], *b, *p, *q;
		int	i, j, len, tc=0, lines=0;
		long	chars=0L;

		if(narg < 2 || narg > 3) {
			VERBOSE(MSGSTR(MSG_CUV5, "usage: ~%%put from [to]\r\n"),
				"");
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
		if(narg == 2)
			arg[2] = arg[1];

		if((file = fopen(arg[1], "r")) == NULL) {
			VERBOSE(MSGSTR(MSG_CU3, P_Ct_OPEN), arg[1]);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
                /*
                 * if cannot write into file on remote machine, write into
                 * /dev/null
                 *
                 * what we're doing is:
                 *      stty -echo
                 *      (cat - > arg2) || cat - > /dev/null
                 *      stty echo
                 */
		_w_str("stty -echo; (cat - > ");
		_w_str(arg[2]);
                _w_str(")||cat - >/dev/null;stty echo\n");
		Intrupt = NO;
		for(i=0,j=0; i < NCCS; ++i)
			if((ch=_Tv0.c_cc[i]) != '\0')
				spec[j++] = ch;
		spec[j] = '\0';
		_mode(2);	/*accept interrupts from keyboard*/
		(void)sleep(5);	/*hope that w_str info digested*/

/* Read characters line by line into buf to write to remote with character*/
/*and line count for blckcnt                                              */
		while(Intrupt == NO &&
				fgets(b= &buf[MID],MID,file) != NULL) {
/*worse case= each*/
/*char must be escaped*/
			len = strlen(b);
			chars += len;		/* character count */
			p = b;
			while(q = strpbrk(p, spec)) {
				if(*q == _Tintr || *q == _Tquit ||
							*q == _Teol) {
					VERBOSE(MSGSTR(MSG_CU13, P_Ct_SPECIAL),
						 *q);
					(void)strcpy(q, q+1);
					Intrupt = YES;
				}
				else {
				b = strncpy(b-1, b, q-b);
				*(q-1) = '\\';
				}
			p = q+1;
			}
			if((tc += len) >= MID) {
				(void)sleep(1);
				tc = len;
			}
			if(write(Cn, b, (unsigned)strlen(b)) < 0) {
				VERBOSE(MSGSTR(MSG_CU15, P_IOERR),"");
				Intrupt = YES;
				break;
			}
			++lines;		/* line count */
			blckcnt((long)chars);
		}
		_mode(1);
		blckcnt((long)(-2));		/* close */
		(void)fclose(file);
		if(Intrupt == YES) {
			Intrupt = NO;
			VERBOSE(MSGSTR(MSG_CU11,P_FILEINTR),"");
			_w_str("\n");
			VERBOSE(MSGSTR(MSG_CU8, P_CNTAFTER), ++chars);
		} else
			VERBOSE(MSGSTR(MSG_CU9, P_CNTLINES), lines);
			VERBOSE(MSGSTR(MSG_CU10, P_CNTCHAR), chars);
		_w_str("\04");
		(void)sleep(3);
		return;
	}

		/*  ~%b or ~%break  */
	if(EQUALS(arg[0], "b") || EQUALS(arg[0], "break")) {
                (*genbrk)(Cn);
		return;
	}
		/*  ~%d or ~%debug toggle  */
	if(EQUALS(arg[0], "d") || EQUALS(arg[0], "debug")) {
		if(Debug == 0)
			Debug = 9;
		else
			Debug = 0;
		VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
		return;
	}
		/*  ~%nostop  toggles start/stop input control  */
	if(EQUALS(arg[0], "nostop")) {
		(void)tcgetattr(Cn, &_Tv);
		if(Sstop == NO)
			_Tv.c_iflag |= IXOFF;
		else
			_Tv.c_iflag &= ~IXOFF;
		(void)tcsetattr(Cn, TCSADRAIN, &_Tv);
		Sstop = !Sstop;
		_mode(1);
		VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
		return;
	}
		/* Change local current directory */
	if(EQUALS(arg[0], "cd")) {
		if (narg < 2) {
			getpath = getenv("HOME");
			strcpy(mypath, getpath);
			if(chdir(mypath) < 0) {
				VERBOSE(MSGSTR(MSG_CUV6, 
				  "Cannot change to %s\r\n"), mypath);
				VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
				return;
			}
		}
		else if (chdir(arg[1]) < 0) {
			VERBOSE(MSGSTR(MSG_CUV7,"Cannot change to %s\r\n"), 
				arg[1]);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
		recfork();	/* fork a new child so it know about change */
		VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
		return;
	}
	VERBOSE(MSGSTR(MSG_CUV12,"~%%%s unknown to cu\r\n"), arg[0]);
	VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
}

/***************************************************************
 *	receive: read from remote line, write to fd=1 (TTYOUT)
 *	catch:
 *	~>[>]:file
 *	.
 *	. stuff for file
 *	.
 *	~>	(ends diversion)
 ***************************************************************/

static void
_receive()
{
	register silent=NO, file;
	register char *p;
	int	tic;
	char	b[BUFSIZ];
        clock_t lseek();
	long	count;


	CDEBUG(4,MSGSTR(MSG_CUCD8, "_receive started\r\n"),"");

	b[0] = '\0';
	file = -1;
	p = b;

	while(r_char(Cn) == YES) {
		if (logme)
			fputc(_Cxc, fdcap);

		if(silent == NO)	/* ie., if not redirecting to file*/
			if(w_char(TTYOUT) == NO) 
				_rcvdead(IOERR);	/* this will exit */
		/* remove CR's and fill inserted by remote */
		if(_Cxc == '\0' || _Cxc == RUB || _Cxc == '\r')
			continue;
		*p++ = _Cxc;
		if(_Cxc != '\n' && (p-b) < BUFSIZ)
			continue;
		/***********************************************
		 * The rest of this code is to deal with what
		 * happens at the beginning, middle or end of
		 * a diversion to a file.
		 ************************************************/
		if(b[0] == '~' && b[1] == '>') {
			/****************************************
			 * The line is the beginning or
			 * end of a diversion to a file.
			 ****************************************/
			if((file < 0) && (b[2] == ':' || b[2] == '>')) {
				/**********************************
				 * Beginning of a diversion
				 *********************************/
				int	append;

				*(p-1) = (int) NULL; /* terminate file name */
				append = (b[2] == '>')? 1:0;
				p = b + 3 + append;
				if(append && (file=open(p,O_WRONLY))>0)
					(void)lseek(file, 0L, 2);
				else
					file = creat(p, 0666);
				if(file < 0) {
					VERBOSE(MSGSTR(MSG_CU6, P_Ct_DIVERT),p);
					perror("cu: open|creat failed");
					(void)sleep(5); /* 10 seemed too long*/
				} else {
					silent = YES; 
					count = tic = 0;
				}
			} else {
				/*******************************
				 * End of a diversion (or queer data)
				 *******************************/
				if(b[2] != '\n')
					goto D;		/* queer data */
				if(silent = close(file)) {
					VERBOSE(MSGSTR(MSG_CU6, P_Ct_DIVERT),b);
					perror("cu: close failed");
					silent = NO;
				}
				blckcnt((long)(-2));
				VERBOSE("~>\r\n","");
				VERBOSE(MSGSTR(MSG_CU9, P_CNTLINES), tic);
				VERBOSE(MSGSTR(MSG_CU10,P_CNTCHAR), count);
				file = -1;
			}
		} else {
			/***************************************
			 * This line is not an escape line.
			 * Either no diversion; or else yes, and
			 * we've got to divert the line to the file.
			 ***************************************/
D:
			if(file > 0) {
				(void)write(file, b, (unsigned)(p-b));
				count += p-b;	/* tally char count */
				++tic;		/* tally lines */
				blckcnt((long)count);
			}
		}
		p = b;
	}
	VERBOSE(MSGSTR(MSG_CUV13, "\r\nLost Carrier\r\n"),"");
	_rcvdead(IOERR);
}

/***************************************************************
 *	change the TTY attributes of the users terminal:
 *	0 means restore attributes to pre-cu status.
 *	1 means set `raw' mode for use during cu session.
 *	2 means like 1 but accept interrupts from the keyboard.
 ***************************************************************/
static void
_mode(arg)
{
	CDEBUG(4,MSGSTR(MSG_CUCD9, "call _mode(%d)\r\n"), arg);
	if(arg == 0) {
		if ( Saved_tty ) {
			if (tcsetattr(TTYIN, TCSADRAIN, &_Tv0) < 0)
				CDEBUG(4, "tcsetattr(TTYIN) restore failed. errno = %d\r\n", errno);
		}
	} else {
		(void)tcgetattr(TTYIN, &_Tv);
		if(arg == 1) {
			_Tv.c_iflag &= ~(INLCR | ICRNL | IGNCR |
						IXOFF | IUCLC);
			_Tv.c_oflag |= OPOST;
			_Tv.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR
						| ONLRET);
			_Tv.c_lflag &= ~(ICANON | ISIG | ECHO);
			if(Sstop == NO)
				_Tv.c_iflag &= ~IXON;
			else
				_Tv.c_iflag |= IXON;
			if(Terminal) {
				_Tv.c_oflag |= ONLCR;
				_Tv.c_iflag |= ICRNL;
			}
			_Tv.c_cc[VEOF] = '\01';
			_Tv.c_cc[VEOL] = '\0';
		}
		if(arg == 2) {
			_Tv.c_iflag |= IXON;
			_Tv.c_lflag |= ISIG;
		}
		(void)tcsetattr(TTYIN, TCSADRAIN, &_Tv);
	}
}


static int
dofork()
{
	register int x,i;

	for(i = 0; i < 6; ++i) {
		if((x = fork()) >= 0) {
			return(x);
		}
	}

	if(Debug) perror("dofork");

	VERBOSE(MSGSTR(MSG_CU12, P_Ct_FK),"");
	return(x);
}

static int
r_char(int fd)
{
	int rtn = 1, rfd;
	char *riobuf;

	/* find starting pos in correct buffer in Riobuf        */
	rfd = RIOFD(fd);
	riobuf = &Riobuf[rfd*WRIOBSZ];

        if (Riop[rfd] >= &riobuf[Riocnt[rfd]]) {
                /* empty read buffer - refill it        */

                /*      flush any waiting output        */
                if ( (wioflsh(Cn) == NO ) || (wioflsh(TTYOUT) == NO) )
                        return(NO);

                while((rtn = read(fd, riobuf, WRIOBSZ)) < 0){
			if(errno == EINTR) {
	/* onintrpt() called asynchronously before this line */
				if(Intrupt == YES) {
					/* got a BREAK */
					_Cxc = '\0';
					return(YES);
				} else {
					/*a signal other than interrupt */ 
					/*received during read*/
					continue;	
				}
			} else {
				CDEBUG(4,MSGSTR(MSG_CUCD10, 
					"got read error, not EINTR\n\r"),"");
				break;			/* something wrong */
			}
		}
                if (rtn > 0) {
                        /* reset current position in buffer     */
                        /* and count of available chars         */
                        Riop[rfd] = riobuf;
                        Riocnt[rfd] = rtn;
                }
	}

        if ( rtn > 0 ) {
/* Don't mask out MSB.  We're in the 8 bit realm.
                _Cxc = *(Riop[rfd]++) & 0177;   /%must mask off parity bit%/
*/
                _Cxc = *(Riop[rfd]++);   
                return(YES);
        } else {
                _Cxc = '\0';
                return(NO);
        }
}

static int
w_char(fd)
{
	int wfd;
	char *wiobuf;

	/* find starting pos in correct buffer in Wiobuf        */
	wfd = WIOFD(fd);
	wiobuf = &Wiobuf[wfd*WRIOBSZ];

	if (Wiop[wfd] >= &wiobuf[WRIOBSZ]) {
		/* full output buffer - flush it */
		if ( wioflsh(fd) == NO )
			return(NO);
	}
	*(Wiop[wfd]++) = _Cxc;
	return(YES);
}

/* wioflsh      flush output buffer     */
static int
wioflsh(fd)
int fd;
{
        int rtn, wfd;
        char *wiobuf;

        /* find starting pos in correct buffer in Wiobuf        */
        wfd = WIOFD(fd);
        wiobuf = &Wiobuf[wfd*WRIOBSZ];

        if (Wiop[wfd] > wiobuf) {
                /* there's something in the buffer */
                while((rtn = write(fd, wiobuf, (Wiop[wfd] - wiobuf))) < 0) {
                        if(errno == EINTR)
                                if(Intrupt == YES) {
                                        VERBOSE("\ncu: Output blocked\r\n","");
                                        _quit(IOERR);
                                } else
                                        continue;       /* alarm went off */
                        else {
                                Wiop[wfd] = wiobuf;
                                return(NO);                     /* bad news */
                        }
                }
        }
        Wiop[wfd] = wiobuf;
        return(YES);
}


static void
_w_str(string)
register char *string;
{
	int len;

	len = strlen(string);
	if(write(Cn, string, (unsigned)len) != len)
		VERBOSE(MSGSTR(MSG_CU4, P_LINE_GONE), "");
}

static void
_onintrpt(int s)
{
	(void)signal(SIGINT, (void(*)(int)) _onintrpt);
	(void)signal(SIGQUIT, (void(*)(int)) _onintrpt);
	Intrupt = YES;
}

static void
_rcvdead(int arg)	/* this is executed only in the receive process */
{
	CDEBUG(4,MSGSTR(MSG_CUCD11, "call _rcvdead(%d)\r\n"), arg);
	(void)kill(getppid(), SIGUSR1);
	exit((arg == SIGHUP)? SIGHUP: arg);
	/*NOTREACHED*/
}

static void
_quit(int arg)	/* this is executed only in the parent process */
{
	CDEBUG(4,MSGSTR(MSG_CUCD12, "call _quit(%d)\r\n"), arg);
	(void)kill(Child, SIGKILL);
	_bye(arg);
	/*NOTREACHED*/
}

static void
_bye(int arg)	/* this is executed only in the parent proccess */
{
	int status;

	CDEBUG(4,MSGSTR(MSG_CUCD13, "call _bye(%d)\r\n"), arg);

	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)wait(&status);
	VERBOSE(MSGSTR(MSG_CUV9,"\r\nDisconnected\007\r\n"),"");
	cleanup((arg == SIGUSR1)? (status >>= 8): arg);
	/*NOTREACHED*/
}



void
cleanup(int code) 	/*this is executed only in the parent process*/
			/*Closes device; removes lock files          */
{

	CDEBUG(4,MSGSTR(MSG_CUCD14, "call cleanup(%d)\r\n"), code);

	if (Child)                 /* shut down logfile garbage */
		close(sockets[0]);
	else
		close(sockets[1]);

	if (logme)
		fclose(fdcap);

	(void) seteuid(Euid);
	/* Because the ioctl in tilda() doesn't work in AIX v2,
	   we set HUPCL here.  */
	(void) sethup(Cn);
	if(Cn > 0) {
		tcsetattr(Cn, TCSANOW, &Savelineb);
		chown(_Cnname, 0, 0);       /* give the TTY back to root */
		chmod(_Cnname, Dev_mode);
		(void)close(Cn);
	}


	clrlock((char*) NULL);	/*uucp routine in ulockf.c*/	
	_mode(0);		/*which removes lock files*/
	catclose(catd);
	exit(code);		/* code=negative for signal causing disconnect*/
}



void
tdmp(arg)
{

	struct termios xv;
	int i;

	VERBOSE(MSGSTR(MSG_CUV10, "\rdevice status for fd=%d\n"), arg);
	VERBOSE(MSGSTR(MSG_CUV11, "\rF_GETFL=%o,"), fcntl(arg, F_GETFL,1));
	if(tcgetattr(arg, &xv) < 0) {
		char	buf[100];
		i = errno;
		(void)sprintf(buf, "\rtdmp for fd=%d", arg);
		errno = i;
		perror(buf);
		return;
	}
	VERBOSE("iflag=`%o',", xv.c_iflag);
	VERBOSE("oflag=`%o',", xv.c_oflag);
	VERBOSE("cflag=`%o',", xv.c_cflag);
	VERBOSE("lflag=`%o',", xv.c_lflag);
	VERBOSE("cc[0]=`%o',",  xv.c_cc[0]);
	for(i=1; i<NCCS; ++i) {
		VERBOSE("[%d]=", i);
		VERBOSE("`%o' , ",xv.c_cc[i]);
	}
	VERBOSE("\r\n","");
}



sysname(name)
char * name;
{

	register char *s;

	if(uname(&utsn) < 0)
		s = "Local";
	else
		s = utsn.nodename;

	strncpy(name,s,strlen(s) + 1);
	*(name + strlen(s) + 1) = '\0';
	return;
}


blckcnt(count)
long count;
{
	static long lcharcnt = 0;
	register long c1, c2;
	register int i;
	char c;

	if(count == (long) (-1)) {       /* initialization call */
		lcharcnt = 0;
		return;
	}
	c1 = lcharcnt/BUFSIZ;
	if(count != (long)(-2)) {	/* regular call */
		c2 = count/BUFSIZ;
		for(i = c1; i++ < c2;) {
			c = '0' + i%10;
			write(2, &c, 1);
			if(i%NPL == 0)
				write(2, "\n\r", 2);
		}
		lcharcnt = count;
	}
	else {
		c2 = (lcharcnt + BUFSIZ -1)/BUFSIZ;
		if(c1 != c2)
			write(2, "+\n\r", 3);
		else if(c2%NPL != 0)
			write(2, "\n\r", 2);
		lcharcnt = 0;
	}
}

void logent(){}		/* so we can load ulockf() */





/*
** if the child receives a SIGUSR2, he'll end up here where we will
** attempt to read a filename from the pipe, open the file, and then
** return to _receive().
*/

void
readpipe(int s)
{
	char buf[1024];

	DEBUG(9, "readpipe()\n", "");
	if (logme) {     /* toggle, so close capture file */
		fclose(fdcap);
		logme = 0;
		goto rcv;
	}


	/*
	** read the filename from the parent via the pipe
	** (using sockets[0] to read from)
	*/

	if (read(sockets[0], buf, 1024) < 0) {
		perror("reading from transmit()");
		logme = 0;
		goto rcv;
	}
	DEBUG(9, "filename received is '%d' long.\n", strlen(buf));

	if (strlen(buf) == 0)		/* no name given so use the default */
		(void) strcpy(buf, CAPTURE_FILE);
	logf = buf;
	DEBUG(9, "Logfile received from transmit() = |%s|\n", logf);
	if ((fdcap = fopen(logf, "a")) == NULL) {
		VERBOSE("error opening logfile '%s' ... aborted.\n", logf);
		goto rcv;
	}
	logme = 1;
 rcv:
	/* reset signal for the next time */
	(void)signal(SIGUSR2, (void(*)(int)) readpipe);
	_receive();         /* this will run until killed */
	/*NOTREACHED*/
}

/*
** givename(cmd)
** 
** Used by parent to transmit the filename to the child;  child
** reads the data via readpipe().  It is assumed that the pipe
** is already opened.
*/

givename(filenm)
char *filenm;
{
	tlog = 1;                    /* we're on the air */

	DEBUG(9, "Sending the filename '%s' to _receive process\n",
	      filenm);

	/* use sockets[1] for writing */
	DEBUG(9, "filename is '%d' long.\n", strlen(filenm));

	if (write(sockets[1], filenm, strlen(filenm)) < 0) {
		perror("writing to _receive");
		return(-1);
	}

	if (write(sockets[1], NULL, 1) < 0) {
		perror("writing to _receive");
		return(-1);
	}

	DEBUG(9, "Filename sent.\n", "");
	return(0);
}
