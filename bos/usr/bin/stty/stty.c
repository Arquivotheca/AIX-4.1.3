static char sccsid[] = "@(#)05	1.23.1.2  src/bos/usr/bin/stty/stty.c, cmdtty, bos41J, 9509A_all 2/27/95 07:34:25";
#ifndef lint
#endif
/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main (stty.c)
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#define _ILS_MACROS
#include <stdio.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <sys/termiox.h>   
#include <stropts.h>   
#include <sys/eucioctl.h>

#include        <locale.h>
#include        <nl_types.h>
#include        "stty_msg.h"

#include <ctype.h>

/* default control chars */
#define CNUL	0
#define CDEL	0177
#define CESC	'\\'
#define CVT	013	/* ASCII VT control */
#define CFORM	014	/* ASCII FF control */
#define CBELL	07	/* ASCII BELL control */

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_STTY,num,str)

extern char *getenv();

#define eq(s) (!strcmp(arg, s))

struct
{
    char *string;
    int speed;
} speeds[] = {
    "0",     B0,
    "50",    B50,
    "75",    B75,
    "110",   B110,
    "134",   B134,
    "150",   B150,
    "200",   B200,
    "300",   B300,
    "600",   B600,
    "1200",  B1200,
    "1800",  B1800,
    "2400",  B2400,
    "4800",  B4800,
    "9600",  B9600,
    "19200", B19200,
    "38400", B38400,
    "134.5", B134,			/* alternates go down here */
    "19.2",  B19200,
    "38.4",  B38400,
    "exta",  B19200,
    "extb",  B38400,
    0, 0,
};

struct mds {
    char *string;
    int set;
    int reset;
};

struct mds cmodes[] = {
    "-parity",	CS8,			PARENB|CSIZE,
    "-evenp",	CS8,			PARENB|CSIZE,
    "-oddp",	CS8,			PARENB|PARODD|CSIZE,
    "parity",	PARENB|CS7,		PARODD|CSIZE,
    "evenp",	PARENB|CS7,		PARODD|CSIZE,
    "even",     PARENB|CS7,		PARODD|CSIZE,        /* bsd */
    "-even",	CS8,			PARENB|CSIZE,        /* bsd */
    "oddp",	PARENB|PARODD|CS7,	CSIZE,
    "odd",      PARENB|PARODD|CS7,	CSIZE,               /* bsd */
    "-odd",	CS8,			PARENB|PARODD|CSIZE, /* bsd */
    "parenb",	PARENB,			0,
    "-parenb",	0,			PARENB,
    "parodd",	PARODD,			0,
    "-parodd",	0,			PARODD,
    "pass8",	CS8, 			PARENB | CSIZE,      /* bsd */
    "-pass8",	PARENB | CSIZE,		CS8,                 /* bsd */
    "cs8",	CS8,			CSIZE,
    "cs7",	CS7,			CSIZE,
    "cs6",	CS6,			CSIZE,
    "cs5",	CS5,			CSIZE,
    "cstopb",	CSTOPB,			0,
    "-cstopb",	0,			CSTOPB,
    "hupcl",	HUPCL,			0,
    "hup",	HUPCL,			0,
    "-hupcl",	0,			HUPCL,
    "-hup",	0,			HUPCL,
    "clocal",	CLOCAL,			0,
    "-clocal",	0,			CLOCAL,
    "nohang",   CLOCAL,			0,         /* bsd */
    "-nohang",  0,                      CLOCAL,    /* bsd */
    "parext",	PAREXT,			0,
    "-parext",	0,			PAREXT,
    "cread",	CREAD,			0,
    "-cread",	0,			CREAD,
    0
    };

struct mds imodes[] = {
    "ignbrk",	IGNBRK,			0,
    "-ignbrk",	0,			IGNBRK,
    "brkint",	BRKINT,			0,
    "-brkint",	0,			BRKINT,
    "ignpar",	IGNPAR,			0,
    "-ignpar",	0,			IGNPAR,
    "parmrk",	PARMRK,			0,
    "-parmrk",	0,			PARMRK,
    "inpck",	INPCK,			0,
    "-inpck",	0,			INPCK,		
    "istrip",	ISTRIP,			0,
    "-istrip",	0,			ISTRIP,
    "inlcr",	INLCR,			0,
    "-inlcr",	0,			INLCR,
    "igncr",	IGNCR,			0,
    "-igncr",	0,			IGNCR,
    "icrnl",	ICRNL,			0,
    "-icrnl",	0,			ICRNL,
    "-nl",	0,			(ICRNL|INLCR|IGNCR),
    "nl",	ICRNL,			0,
    "iuclc",	IUCLC,			0,
    "-iuclc",	0,			IUCLC,
    "lcase",	IUCLC,			0,
    "-lcase",	0,			IUCLC,
    "LCASE",	IUCLC,			0,
    "-LCASE",	0,			IUCLC,
    "tandem",   IXOFF,                  0,               /* bsd */
    "-tandem",  0,                      IXOFF,           /* bsd */
    "decctlq",	0,			IXANY,           /* bsd */
    "-decctlq",	IXANY,			0,               /* bsd */
    "ixon",	IXON,			0,
    "-ixon",	0,			IXON,
    "ixany",	IXANY,			0,
    "-ixany",	0,			IXANY,
    "ixoff",	IXOFF,			0,
    "-ixoff",	0,			IXOFF,
    "dec",      0,                      IXANY,           /* bsd */
    "imaxbel",	IMAXBEL,		0,
    "-imaxbel",	0,			IMAXBEL,
    "raw",	0,			-1,
    "-raw",	(BRKINT|IGNPAR|ICRNL|IXON),	0,
    "cooked",	(BRKINT|IGNPAR|ICRNL|IXON),	0,
    "sane",	(BRKINT|IGNPAR|ICRNL|IXON|IXOFF|IMAXBEL),
			(IGNBRK|PARMRK|INPCK|INLCR|IGNCR|IUCLC),
    0
    };

struct mds lmodes[] = {
 "isig",	ISIG,	0,
 "-isig",	0,	ISIG,
 "icanon",	ICANON,	0,
 "-icanon",	0,	ICANON,
 "cbreak",      0,	ICANON,         /* bsd */
 "-cbreak",     ICANON,	0,              /* bsd */
 "xcase",	XCASE,	0,
 "-xcase",	0,	XCASE,
 "lcase",	XCASE,	0,
 "-lcase",	0,	XCASE,
 "LCASE",	XCASE,	0,
 "-LCASE",	0,	XCASE,
 "echo",	ECHO,	0,
 "-echo",	0,	ECHO,
 "echoe",	ECHOE,	0,
 "-echoe",	0,	ECHOE,
 "echok",	ECHOK,	0,
 "-echok",	0,	ECHOK,
 "lfkc",	ECHOK,	0,
 "-lfkc",	0,	ECHOK,
 "echonl",	ECHONL,	0,
 "-echonl",	0,	ECHONL,
 "noflsh",	NOFLSH,	0,
 "-noflsh",	0,	NOFLSH,
 "tostop",	TOSTOP,	0,
 "-tostop",	0,	TOSTOP,
 "echoctl",	ECHOCTL,	0,
 "-echoctl",	0,	ECHOCTL,
 "echoprt",	ECHOPRT,	0,
 "-echoprt",	0,	ECHOPRT,
 "echoke",	ECHOKE,	0,
 "-echoke",	0,	ECHOKE,
 "flusho",	FLUSHO,	0,
 "-flusho",	0,	FLUSHO,
 "pending",	PENDIN,	0,
 "-pending",	0,	PENDIN,
 "pendin",	PENDIN,	0,            /* bsd */
 "-pendin",	0,	PENDIN,       /* bsd */
 "iexten",	IEXTEN,	0,
 "-iexten",	0,	IEXTEN,
 "raw",	0,	(ISIG|ICANON|XCASE),
 "-raw",	(ISIG|ICANON),	0,
 "cooked",	(ISIG|ICANON),	0,
 "sane",	(ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOKE|ECHOCTL),
			(XCASE|ECHONL|ECHOPRT|NOFLSH),
 "crt",		ECHOE|ECHOKE|ECHOCTL,	ECHOK|ECHOPRT,    /* bsd */
 "-crt",	ECHOK,	ECHOE|ECHOKE|ECHOCTL,             /* bsd */
 "crterase",	ECHOE,	0,                                /* bsd */
 "-crterase",	0,	ECHOE,                            /* bsd */
 "crtbs",	ECHOE,	0,                                /* bsd */
 "-crtbs",	0,	ECHOE,                            /* bsd */
 "crtkill",	ECHOKE,	0,                                /* bsd */
 "-crtkill",	0,	ECHOKE,                           /* bsd */
 "ctlecho",	ECHOCTL,	0,                        /* bsd */
 "-ctlecho",	0,	ECHOCTL,                          /* bsd */
 "dec",         ECHOE|ECHOKE|ECHOCTL,	ECHOPRT,          /* bsd */
 "prterase",	ECHOPRT,	0,
 "-prterase",	0,	ECHOPRT,
 0,
};

struct mds omodes[] = {
 "opost",	OPOST,	0,
 "-opost",	0,	OPOST,
 "litout",	0,	OPOST,       /* bsd */
 "-litout",	OPOST,	0,           /* bsd */
 "olcuc",	OLCUC,	0,
 "-olcuc",	0,	OLCUC,
 "lcase",	OLCUC,	0,
 "-lcase",	0,	OLCUC,
 "LCASE",	OLCUC,	0,
 "-LCASE",	0,	OLCUC,
 "onlcr",	ONLCR,	0,
 "-onlcr",	0,	ONLCR,
 "-nl",	0,	(ONLCR|OCRNL|ONLRET),
 "nl",	ONLCR,	0,
 "ocrnl",	OCRNL,	0,
 "-ocrnl",0,	OCRNL,
 "onocr",	ONOCR,	0,
 "-onocr",	0,	ONOCR,
 "onlret",	ONLRET,	0,
 "-onlret",	0,	ONLRET,
 "fill",	OFILL,	OFDEL,
 "-fill",	0,	OFILL|OFDEL,
 "nul-fill",	OFILL,	OFDEL,
 "del-fill",	OFILL|OFDEL,	0,
 "ofill",	OFILL,	0,
 "-ofill",	0,	OFILL,
 "ofdel",	OFDEL,	0,
 "-ofdel",	0,	OFDEL,
 "cr0",	CR0,	CRDLY,
 "cr1",	CR1,	CRDLY,
 "cr2",	CR2,	CRDLY,
 "cr3",	CR3,	CRDLY,
 "tab0",	TAB0,	TABDLY,
 "tabs",	TAB0,	TABDLY,
 "tab1",	TAB1,	TABDLY,
 "tab2",	TAB2,	TABDLY,
 "tab3",	TAB3,	TABDLY,
 "-tabs",	TAB3,	TABDLY,
 "tab8",	TAB3,	TABDLY,
 "nl0",	NL0,	NLDLY,
 "nl1",	NL1,	NLDLY,
 "nl2",	NL2,	NLDLY,             /* bsd */
 "nl3",	NL3,	NLDLY,             /* bsd */
 "ff0",	FF0,	FFDLY,
 "ff1",	FF1,	FFDLY,
 "vt0",	VT0,	VTDLY,
 "vt1",	VT1,	VTDLY,
 "bs0",	BS0,	BSDLY,
 "bs1",	BS1,	BSDLY,
 "raw",	0,	OPOST,
 "-raw",	OPOST,	0,
 "cooked",	OPOST,	0,
 "tty33",	CR1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "tn300",	CR1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "ti700",	CR2,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "vt05",	NL1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "tek",	FF1,	(CRDLY|TABDLY|NLDLY|FFDLY|VTDLY|BSDLY),
 "tty37",	(FF1|VT1|CR2|TAB1|NL1),	(NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY),
 "sane",	(OPOST|ONLCR),	(OLCUC|OCRNL|ONOCR|ONLRET|OFILL|OFDEL|
				 NLDLY|CRDLY|TABDLY|BSDLY|VTDLY|FFDLY),
 0,
};


char *arg;
int pitt = 0;
struct termios cb;
struct winsize win;

/* hardware flow control modes */
struct
{
    char *string;
    unsigned short  mode;
} hmodes[] = {
    "rtsxoff",     RTSXOFF,
    "-rtsxoff",    RTSXOFF,
    "ctsxon",      CTSXON,
    "-ctsxon",     CTSXON,
    "dtrxoff",     DTRXOFF,
    "-dtrxoff",    DTRXOFF,    
    "cdxon",       CDXON,
    "-cdxon",      CDXON,
    0,
};

struct termiox hfc;
extern int getopt ();
extern int optind;
extern char *optarg;
#define NORMAL	0	
#define ALL	1
#define GFMT	2	

main(argc, argv)
char *argv[];
{
    register i;
    int x_hflag_set = 0;
    int tcgetx_flag = 0; 
    int winset = 0;
    int ok = 1;
    int ch;
    int fmt = NORMAL;
    
    setlocale(LC_ALL,"") ;
    catd = catopen(MF_STTY, NL_CAT_LOCALE);
    opterr = 0;
    while (ok && (ch = getopt(argc, argv, "ga")) != EOF)
	    switch((char)ch) {
	    case 'a':
		    fmt = ALL;
		    break;
	    case 'g':
		    fmt = GFMT;
		    break;
	    case '?':
	    default:
		    ok = 0;	/* Any more args are turning off options */
		    break;
		    
	    }

    argc -= optind;
    argv += optind;

    if (argc == 0 && !ok) {
      fprintf(stderr, MSGSTR(USAGE, "usage: stty [-ag] [modes]\n"));
      exit(2);
    }
    gttyinfo();

    if (argc == 0) {
	    switch(fmt)
	    {
	    case NORMAL:
		    prmodes();
		    exit(0);
	    case ALL:
		    pramodes();
		    exit(0);
	    case GFMT:
		    prencode();
		    exit(0);
	    }
    }
    
    while(argc-- > 0) {
	int flag = 0;
	

	arg = *argv++;
	if (eq("intr") && argc--) {
	    cb.c_cc[VINTR] = gct(*argv++);
	    continue;
	} else if (eq("quit") && argc--){
	    cb.c_cc[VQUIT] = gct(*argv++);
	    continue;
	} else if (eq("erase") && argc--) {
	    cb.c_cc[VERASE] = gct(*argv++);
	    continue;
	} else if (eq("kill") && argc--) {
	    cb.c_cc[VKILL] = gct(*argv++);
	    continue;
	} else if (eq("eof") && argc--) {
	    cb.c_cc[VEOF] = gct(*argv++);
	    continue;
	} else if (eq("eol") && argc--) {
	    cb.c_cc[VEOL] = gct(*argv++);
	    continue;
	} else if (eq("brk") && argc--) {  /* bsd */
	    cb.c_cc[VEOL] = gct(*argv++);
	    continue;
	} else if (eq("eol2") && argc--) {
	    cb.c_cc[VEOL2] = gct(*argv++);
	    continue;
	} else if (eq("start") && argc--) {
	    cb.c_cc[VSTART] = gct(*argv++);
	    continue;
	} else if (eq("stop") && argc--) {
	    cb.c_cc[VSTOP] = gct(*argv++);
	    continue;
	} else if (eq("susp") && argc--) {
	    cb.c_cc[VSUSP] = gct(*argv++);
	    continue;
	} else if (eq("dsusp") && argc--) {
	    cb.c_cc[VDSUSP] = gct(*argv++);
	    continue;
	} else if (eq("reprint") && argc--) {
	    cb.c_cc[VREPRINT] = gct(*argv++);
	    continue;
	} else if (eq("rprnt") && argc--) {      /* bsd */
	    cb.c_cc[VREPRINT] = gct(*argv++);
	    continue;   
	} else if (eq("discard") && argc--) {
	    cb.c_cc[VDISCRD] = gct(*argv++);
	    continue;
        } else if (eq("flush") && argc--) {      /* bsd */
	    cb.c_cc[VDISCRD] = gct(*argv++);
	    continue;
	} else if (eq("werase") && argc--) {
	    cb.c_cc[VWERSE] = gct(*argv++);
	    continue;
	} else if (eq("lnext") && argc--) {
	    cb.c_cc[VLNEXT] = gct(*argv++);
	    continue;
	} else if (eq("min") && argc--) {
	    cb.c_cc[VMIN] = atoi(*argv++);
	    continue;
	} else if (eq("time") && argc--) {
	    cb.c_cc[VTIME] = atoi(*argv++);
	    continue;
	} else if (eq("ek")) {
	    cb.c_cc[VERASE] = CERASE;
	    cb.c_cc[VKILL] = CKILL;
	    continue;
	} else if (eq("raw")) {
	    cb.c_cc[VMIN] = 1;
	    cb.c_cc[VTIME] = 1;
	} else if (eq("-raw") | eq("cooked")) {
	    cb.c_cc[VEOF] = CEOF;
	    cb.c_cc[VEOL] = CNUL;
	} else if(eq("sane")) {
	    cb.c_cc[VERASE] = CERASE;
	    cb.c_cc[VKILL] = CKILL;
	    cb.c_cc[VQUIT] = CQUIT;
	    cb.c_cc[VINTR] = CINTR;
	    cb.c_cc[VEOF] = CEOF;
	    cb.c_cc[VEOL] = CNUL;
	    cb.c_cc[VEOL2] = CNUL;
	    cb.c_cc[VSTART] = '\21';
	    cb.c_cc[VSTOP] = '\23';
	    cb.c_cc[VSUSP] = '\32';
	    cb.c_cc[VDSUSP] = '\31';
	    cb.c_cc[VREPRINT] = '\22';
	    cb.c_cc[VDISCRD] = '\17';
	    cb.c_cc[VWERSE] = '\27';
	    cb.c_cc[VLNEXT] = '\26';
        } else if (eq("rows") && argc--) {
	    win.ws_row = atoi(*argv++);
	    winset++;
	    continue;
        } else if ((eq("cols") || eq("columns")) && argc--) {
	    win.ws_col = atoi(*argv++);
	    winset++;
	    continue;
        } else if (eq("size")) {
            printf("%d %d\n", win.ws_row, win.ws_col); 
	    exit(0);
	} else if (eq("all")) {    /* bsd */
	    bsd_prmodes(1);
	    exit(0);
        } else if (eq("everything")) {      /* bsd */ 
            bsd_prmodes(2);
	    exit(0); 
        } else if (eq("dec")) {  /* bsd */
	    cb.c_cc[VERASE] = 0177;
	    cb.c_cc[VKILL] = 'u' & 037;
	    cb.c_cc[VINTR]  ='c' & 037;
        } else if (eq("speed")) {  /* bsd */
	    prspeed();
	    printf("\n");
	    exit(0);
	/* ispeed/ospeed - posix control modes */
	} 
	else if (eq("ispeed")) {
	       if (argc-- <= 0) {
		 fprintf(stderr, "stty: ");
		 fprintf(stderr, "missing ispeed\n");
		 exit(2);
	       }
	       arg = *argv++;
	       for (i=0; speeds[i].string; i++)
		  if (eq(speeds[i].string)) {
		    cfsetispeed(&cb, speeds[i].speed);
		    break;
	       }
	       continue;
	}
	else if (eq("ospeed")) {
		if (argc-- <= 0) {
		  fprintf(stderr, "stty: ");
		  fprintf(stderr, "missing ospeed\n");
		  exit(2);
		}
		arg = *argv++;
		for (i=0; speeds[i].string; i++)
		  if (eq(speeds[i].string)) {
		    cfsetospeed(&cb, speeds[i].speed);
		    break;
	        }
		continue;
	}

	/* hardware flow control modes - new options - new ioctl's */
	/* just set the x_hflag here - do the ioctl last */
	for(i=0; hmodes[i].string; i++)
	    if (eq(hmodes[i].string)) {
	      if (!tcgetx_flag) { 
                if (ioctl(0, TCGETX, &hfc) < 0) {
	          fprintf(stderr, "stty: ");
	          perror("TCGETX");
	          exit(2);
                }
	        tcgetx_flag = 1;
	      }

	      if (arg[0] == '-') {
	        hfc.x_hflag &= ~hmodes[i].mode;
	      }
	      else {
	        hfc.x_hflag |= hmodes[i].mode;
	      }
	      x_hflag_set = 1;
	      break;
            }
	if (hmodes[i].string) 
            continue;
        
	if (isdigit(*arg)) {
	    char *o;
	    speed_t in = 0, out = 0;

	    if (o = strchr(arg, '/')) {
		*o = '\0';
		for(i=0; speeds[i].string; i++)
		    if(eq(speeds[i].string)) {
			in = speeds[i].speed;
			break;
		    }
		arg = o+1;
	    }

	    for(i=0; speeds[i].string; i++)
		if(eq(speeds[i].string)) {
		    out = speeds[i].speed;
		    cfsetospeed(&cb, out);
		    cfsetispeed(&cb, in);
		    break;
		}
	    if (speeds[i].string)
		continue;
	}

	for(i=0; imodes[i].string; i++)
	    if(eq(imodes[i].string)) {
		cb.c_iflag &= ~imodes[i].reset;
		cb.c_iflag |= imodes[i].set;
		flag = 1;
	    }
	for(i=0; omodes[i].string; i++)
	    if(eq(omodes[i].string)) {
		cb.c_oflag &= ~omodes[i].reset;
		cb.c_oflag |= omodes[i].set;
		flag = 1;
	    }
	for(i=0; cmodes[i].string; i++)
	    if(eq(cmodes[i].string)) {
		cb.c_cflag &= ~cmodes[i].reset;
		cb.c_cflag |= cmodes[i].set;
		flag = 1;
	    }
	for(i=0; lmodes[i].string; i++)
	    if(eq(lmodes[i].string)) {
		cb.c_lflag &= ~lmodes[i].reset;
		cb.c_lflag |= lmodes[i].set;
		flag = 1;
	    }
	
	if(!flag && !encode(arg)) {
	    fprintf(stderr,"stty: ");
	    fprintf(stderr, MSGSTR(UNKNOWN, "unknown mode: %s\n"), arg);
	    exit(2);
	}
    }

    if (tcsetattr(0, TCSADRAIN, &cb) == -1) {
	fprintf(stderr,"stty: ");    
	perror("setattr");
	exit(2);
    }

    if (winset && (ioctl(0,TIOCSWINSZ, &win) < 0)) {
	fprintf(stderr,"stty: ");    
	perror("TIOCSWINSZ");
	exit(2);
     }
    
    if (x_hflag_set &&
	(ioctl(0,TCSETXW, &hfc) < 0)) {
	fprintf(stderr,"stty: ");    
	perror("TCSETXW");
	exit(2);  
     }  

    exit(0);
}

gttyinfo()
{
    if (tcgetattr(0, &cb) == -1) {
	fprintf(stderr,"stty: ");    
	perror("tcgetattr");
	exit(2);
    }

    if (ioctl(0, TIOCGWINSZ, &win)) {
	fprintf(stderr,"stty: ");
	perror("TIOCGWINSZ");
	exit(2);
    }

}

prmodes()
{
    register m;

    m = cb.c_cflag;
    
    prspeed();
    if (m&PARENB)
	if (m&PARODD)
	    printf("oddp ");
	else
	    printf("evenp ");
    else
	printf("-parity ");
    if(((m&PARENB) && !(m&CS7)) || (!(m&PARENB) && !(m&CS8)))
	printf("cs%c ",'5'+(m&CSIZE)/CS6);
    if (m&CSTOPB)
	printf("cstopb ");
    if (m&HUPCL)
	printf("hupcl ");
    if (!(m&CREAD))
	printf("cread ");
    if (m&CLOCAL)
	printf("clocal ");
    if (m&PAREXT)
	printf("parext ");
    printf("\n");
    pitt = 0;
    if(cb.c_cc[VINTR] != CINTR)
	pit(cb.c_cc[VINTR], "intr");
    if(cb.c_cc[VQUIT] != CQUIT)
	pit(cb.c_cc[VQUIT], "quit");
    if(cb.c_cc[VERASE] != CERASE)
	pit(cb.c_cc[VERASE], "erase");
    if(cb.c_cc[VKILL] != CKILL)
	pit(cb.c_cc[VKILL], "kill");
    if(cb.c_cc[VEOF] != CEOF)
	pit(cb.c_cc[VEOF], "eof");
    if(cb.c_cc[VEOL] != CNUL)
	pit(cb.c_cc[VEOL], "eol");
    if(cb.c_cc[VEOL2] != CNUL)
	pit(cb.c_cc[VEOL2], "eol2");
    if(cb.c_cc[VSTART] != '\21')
	pit(cb.c_cc[VSTART], "start");
    if(cb.c_cc[VSTOP] != '\23')
	pit(cb.c_cc[VSTOP], "stop");
    if(cb.c_cc[VSUSP] != '\32')
	pit(cb.c_cc[VSUSP], "susp");
    if(cb.c_cc[VDSUSP] != '\31')
	pit(cb.c_cc[VDSUSP], "dsusp");
    if(cb.c_cc[VREPRINT] != '\22')
	pit(cb.c_cc[VREPRINT], "reprint");
    if(cb.c_cc[VDISCRD] != '\17')
	pit(cb.c_cc[VDISCRD], "discrd");
    if(cb.c_cc[VWERSE] != '\27')
	pit(cb.c_cc[VWERSE], "werse");
    if(cb.c_cc[VLNEXT] != '\26')
	pit(cb.c_cc[VLNEXT], "lnext");
    if (pitt)
	printf("\n");
    m = cb.c_iflag;
    if (m&IGNBRK)
	printf("ignbrk ");
    else if (m&BRKINT)
	printf("brkint ");
    if (!(m&INPCK))
	printf("-inpck ");
    else if (m&IGNPAR)
	printf("ignpar ");
    if (m&PARMRK)
	printf("parmrk ");
    if (!(m&ISTRIP))
	printf("-istrip ");
    if (m&INLCR)
	printf("inlcr ");
    if (m&IGNCR)
	printf("igncr ");
    if (m&ICRNL)
	printf("icrnl ");
    if (m&IUCLC)
	printf("iuclc ");
    if (!(m&IXON))
	printf("-ixon ");
    else if (!(m&IXANY))
	printf("-ixany ");
    if (m&IXOFF)
	printf("ixoff ");
    m = cb.c_oflag;
    if (!(m&OPOST))
	printf("-opost ");
    else {
	if (m&OLCUC)
	    printf("olcuc ");
	if (m&ONLCR)
	    printf("onlcr ");
	if (m&OCRNL)
	    printf("ocrnl ");
	if (m&ONOCR)
	    printf("onocr ");
	if (m&ONLRET)
	    printf("onlret ");
	if (m&OFILL)
	    if (m&OFDEL)
		printf("del-fill ");
	    else
		printf("nul-fill ");
	delay((m&CRDLY)/CR1, "cr");
	delay((m&NLDLY)/NL1, "nl");
	delay((m&TABDLY)/TAB1, "tab");
	delay((m&BSDLY)/BS1, "bs");
	delay((m&VTDLY)/VT1, "vt");
	delay((m&FFDLY)/FF1, "ff");
    }
    printf("\n");
    m = cb.c_lflag;
    if (!(m&ISIG))
	printf("-isig ");
    if (!(m&ICANON))
	printf("-icanon ");
    if (m&XCASE)
	printf("xcase ");
    printf("-echo "+((m&ECHO)!=0));
    printf("-echoe "+((m&ECHOE)!=0));
    printf("-echok "+((m&ECHOK)!=0));
    if (m&ECHONL)
	printf("echonl ");
    if (m&NOFLSH)
	printf("noflsh ");
    printf("\n");
}

pramodes()
{
    register int m;

    prspeed();
    prsize();
    printf("\n");
    prcswidth();  
    printf("\n");
    pitt = 0;
    pit(cb.c_cc[VINTR], "intr");
    pit(cb.c_cc[VQUIT], "quit");
    pit(cb.c_cc[VERASE], "erase");
    pit(cb.c_cc[VKILL], "kill");
    pit(cb.c_cc[VEOF], "eof");
    pit(cb.c_cc[VEOL], "eol");
    pit(cb.c_cc[VEOL2], "eol2");
    pit(cb.c_cc[VSTART], "start");
    pit(cb.c_cc[VSTOP], "stop");
    pit(cb.c_cc[VSUSP], "susp");
    pit(cb.c_cc[VDSUSP], "dsusp");
    pit(cb.c_cc[VREPRINT], "reprint");
    pit(cb.c_cc[VDISCRD], "discard");
    pit(cb.c_cc[VWERSE], "werase");
    pit(cb.c_cc[VLNEXT], "lnext");
    if (pitt)
	printf("\n");

    m = cb.c_cflag;
    printf("-parenb "+((m&PARENB)!=0));
    printf("-parodd "+((m&PARODD)!=0));
    printf("cs%c ",'5'+(m&CSIZE)/CS6);
    printf("-cstopb "+((m&CSTOPB)!=0));
    printf("-hupcl "+((m&HUPCL)!=0));
    printf("-cread "+((m&CREAD)!=0));
    printf("-clocal "+((m&CLOCAL)!=0));
    printf("-parext "+((m&PAREXT)!=0));
    printf("\n");
    m = cb.c_iflag;
    printf("-ignbrk "+((m&IGNBRK)!=0));
    printf("-brkint "+((m&BRKINT)!=0));
    printf("-ignpar "+((m&IGNPAR)!=0));
    printf("-parmrk "+((m&PARMRK)!=0));
    printf("-inpck "+((m&INPCK)!=0));
    printf("-istrip "+((m&ISTRIP)!=0));
    printf("-inlcr "+((m&INLCR)!=0));
    printf("-igncr "+((m&IGNCR)!=0));
    printf("-icrnl "+((m&ICRNL)!=0));
    printf("-iuclc "+((m&IUCLC)!=0));
    printf("\n");
    printf("-ixon "+((m&IXON)!=0));
    printf("-ixany "+((m&IXANY)!=0));
    printf("-ixoff "+((m&IXOFF)!=0));
    printf("-imaxbel "+((m&IMAXBEL)!=0));
    prhfcntl();
    printf("\n");
    m = cb.c_lflag;
    printf("-isig "+((m&ISIG)!=0));
    printf("-icanon "+((m&ICANON)!=0));
    printf("-xcase "+((m&XCASE)!=0));
    printf("-echo "+((m&ECHO)!=0));
    printf("-echoe "+((m&ECHOE)!=0));
    printf("-echok "+((m&ECHOK)!=0));
    printf("-echonl "+((m&ECHONL)!=0));
    printf("-noflsh "+((m&NOFLSH)!=0));
    printf("\n");
    printf("-tostop "+((m&TOSTOP)!=0));
    printf("-echoctl "+((m&ECHOCTL)!=0));
    printf("-echoprt "+((m&ECHOPRT)!=0));
    printf("-echoke "+((m&ECHOKE)!=0));
    printf("-flusho "+((m&FLUSHO)!=0));
    printf("-pending "+((m&PENDIN)!=0));
    printf("-iexten "+((m&IEXTEN)!=0));
    printf("\n");
    m = cb.c_oflag;
    printf("-opost "+((m&OPOST)!=0));
    printf("-olcuc "+((m&OLCUC)!=0));
    printf("-onlcr "+((m&ONLCR)!=0));
    printf("-ocrnl "+((m&OCRNL)!=0));
    printf("-onocr "+((m&ONOCR)!=0));
    printf("-onlret "+((m&ONLRET)!=0));
    printf("-ofill "+((m&OFILL)!=0));
    printf("-ofdel "+((m&OFDEL)!=0));
    delay((m&CRDLY)/CR1, "cr");
    delay((m&NLDLY)/NL1, "nl");
    delay((m&TABDLY)/TAB1, "tab");
    delay((m&BSDLY)/BS1, "bs");
    delay((m&VTDLY)/VT1, "vt");
    delay((m&FFDLY)/FF1, "ff");
    printf("\n");
}

gct(cp)
register char *cp;
{
  register c;
  char *arg = cp;
  int undef = 0;
  
  c = *cp++;
  if (c == '^') {
    c = *cp;
    if (c == '?')
	    c = 0177;
    else if (c == '-') 
	    undef = 1;
    else
	    c &= 037;
  }
  else if (!strcmp(arg, "undef")) 
	  undef = 1;
  if (undef) {
    if (fpathconf(0,_PC_VDISABLE) < 0) {
      fprintf(stderr, 
	      MSGSTR(PVDISABLE,
		     "_POSIX_VDISABLE not in effect for the device\n"));
      exit(2);
    }
    else
	    c = _POSIX_VDISABLE;
  }
  return(c);
}

pit(what, itsname)
unsigned char what;
char *itsname;
{
    if (pitt > 60) {
	printf("\n");
	pitt = 0;
    }
    if (pitt) {
	printf("; ");
	pitt += 2;
    }

    printf("%s", itsname);
    pitt += strlen(itsname);
    if (what == 0377) {
	printf(" = <undef>");
	pitt += 8;
	return;
    }
    printf(" = ");
    pitt += 3;
    if (iscntrl(what)) {
	printf("^");
	++pitt;
	what ^= '@';
    }
    printf("%c", what);
    ++pitt;
}

delay(m, s)
char *s;
{
    if(m)
	printf("%s%d ", s, m);
}

prspeed()
{
    speed_t is = cfgetispeed(&cb);
    speed_t os = cfgetospeed(&cb);
    
    if (is && is != os)
	printf(MSGSTR(SPLBAUD, "ispeed %s baud; ospeed %s baud; "),
	       speeds[is].string, speeds[os].string);
    else
	printf(MSGSTR(BAUD, "speed %s baud; "), speeds[os].string);
}

prsize()
{
    printf(MSGSTR(ROWS, "%d rows; %d columns"), win.ws_row,
	    win.ws_col);
}

prencode()
{
    int i;

    for (i = 0; i < NCCS; ++i)
	printf("%x:", cb.c_cc[i]);
    printf("%x:%x:%x:%x:\n", cb.c_iflag, cb.c_oflag, cb.c_cflag, cb.c_lflag);
}

encode(argptr)
char *argptr;
{
    int i, c;

    for (i = 0; i < NCCS; ++i, ++argptr) {
	if (sscanf(argptr, "%x", &c) != 1 ||
	    !(argptr = strchr(argptr, ':')))
	    return 0;
	cb.c_cc[i] = c;
    }

    if (sscanf(argptr, "%x:%x:%x:%x", &cb.c_iflag, &cb.c_oflag, &cb.c_cflag,
	       &cb.c_lflag) != 4)
	return 0;
    return(1);
}


/* for bsd compatibility - use the bsd print function to print 
   out bsd all/everything info 
*/
bsd_prmodes(int all)
{
    register m;
    int any;
    int nothing = 1;

    struct tchars tc;
    struct ltchars ltc;
    struct sgttyb mode;
    int lmode;
    int ldisc =0;
    speed_t is = cfgetispeed(&cb);
    speed_t os = cfgetospeed(&cb); 

    /* get necessary info using BSD ioctl's */
    if (ioctl(0, TIOCGETP, &mode) < 0) {
	    fprintf(stderr, "stty: ");
	    perror("ioctl TIOCGETP");
	    return;
    }
    if (ioctl(0, TIOCGETC, &tc) < 0) {
	    fprintf(stderr, "stty: ");
	    perror("ioctl TIOCGETC");
	    return;
    }
    if (ioctl(0, TIOCLGET, &lmode) < 0) {
	    fprintf(stderr, "stty: ");
	    perror("ioctl TIOCLGET");
	    return;
    }
    if (ioctl(0, TIOCGLTC, &ltc) < 0) {
	    fprintf(stderr, "stty: ");
	    perror("ioctl TIOCGLT");
	    return;
    }

    if (mode.sg_ispeed != mode.sg_ospeed) {
	printf("input speed %s", speeds[is].string);
	printf("output speed %s", speeds[os].string);
    } else
	printf("speed %s baud, ", speeds[os].string);
    if (all)
        printf("%d rows, %d columns", win.ws_row,
	    win.ws_col);    
    printf( all==2 ? "\n" : "; ");
    m = mode.sg_flags;

    if (all==2 || (m&(EVENP|ODDP))!=(EVENP|ODDP)) {
	if (m & EVENP)
	    printf("even ");
	if (m & ODDP)
	    printf("odd ");
    }

    if (all==2 || m&RAW)
	printf("-raw "+((m&RAW)!=0));
    if (all==2 || (m&CRMOD)==0)
	printf("-nl "+((m&CRMOD)==0));
    if (all==2 || (m&ECHO)==0)
	printf("-echo "+((m&ECHO)!=0));
    if (all==2 || (m&LCASE))
	printf("-lcase "+((m&LCASE)!=0));
    if (all==2 || (m&TANDEM))
	printf("-tandem "+((m&TANDEM)!=0));
    printf("-tabs "+((m&XTABS)!=XTABS));
    if (all==2 || (m&CBREAK))
	printf("-cbreak "+((m&CBREAK)!=0));
    if (all==2 || (m&NLDELAY))
	delay((m&NLDELAY)/NL1,	"nl");
    if ((m&TBDELAY)!=XTABS)
	delay((m&TBDELAY)/TAB1,	"tab");
    if (all==2 || (m&CRDELAY))
	delay((m&CRDELAY)/CR1,	"cr");
    if (all==2 || (m&VTDELAY))
	delay((m&VTDELAY)/FF1,	"ff");
    if (all==2 || (m&BSDELAY))
	delay((m&BSDELAY)/BS1,	"bs");
    if (all)
	printf("\n");
#define	lpit(what,str) \
    if (all==2||(lmode&what)) { \
	printf(str+((lmode&what)!=0)); \
	any++; \
    }
	
    lpit(LCRTBS, "-crtbs ");
    lpit(LCRTERA, "-crterase ");
    lpit(LCRTKIL, "-crtkill ");
    lpit(LCTLECH, "-ctlecho ");
    lpit(LPRTERA, "-prterase ");

    lpit(LTOSTOP, "-tostop ");
    if (all==2) {
	    printf( "\n");
	    any = 0;
	    nothing = 0;
    }
    lpit(LFLUSHO, "-flusho ");
    lpit(LLITOUT, "-litout ");
    lpit(LPASS8, "-pass8 ");
    lpit(LNOHANG, "-nohang ");
    if (any) {
	    printf("\n");
	    any = 0;
	    nothing = 0;
    }
    lpit(LPENDIN, "-pendin ");
    lpit(LDECCTQ, "-decctlq ");
    lpit(LNOFLSH, "-noflsh ");
    if (any || nothing)
	    printf("\n");
    if (all) {
	    printf("\
erase  kill   werase rprnt  flush  lnext  susp   intr   quit   stop   eof\
\n"); 
	    pcol(mode.sg_erase, -1);
	    pcol(mode.sg_kill, -1);
	    pcol(ltc.t_werasc, -1);
	    pcol(ltc.t_rprntc, -1);
	    pcol(ltc.t_flushc, -1);
	    pcol(ltc.t_lnextc, -1);
	    pcol(ltc.t_suspc, ltc.t_dsuspc);
	    pcol(tc.t_intrc, -1);
	    pcol(tc.t_quitc, -1);
	    pcol(tc.t_stopc, tc.t_startc);
	    pcol(tc.t_eofc, tc.t_brkc);
	    printf("\n");

    } 
    printf( "\n");
}



pcol(ch1, ch2)
int ch1, ch2;
{
    int nout = 0;

    ch1 &= 0377;
    ch2 &= 0377;
    if (ch1 == ch2)
	ch2 = 0377;
    for (; ch1 != 0377 || ch2 != 0377; ch1 = ch2, ch2 = 0377) {
	if (ch1 == 0377)
	    continue;
	if (ch1 & 0200) {
	    printf( "M-");
	    nout += 2;
	    ch1 &= ~ 0200;
	}
	if (ch1 == 0177) {
	    printf( "^");
	    nout++;
	    ch1 = '?';
	} else if (ch1 < ' ') {
	    printf( "^");
	    nout++;
	    ch1 += '@';
	}
	printf( "%c", ch1);
	nout++;
	if (ch2 != 0377) {
	    printf( "/");
	    nout++;
	}
    }
    while (nout < 7) {
	printf( " ");
	nout++;
    }
}



prcswidth()
{

  struct strioctl i_str;
  struct eucioc wp;

  bzero(&wp, sizeof(struct eucioc));
  i_str.ic_cmd = EUC_WGET;
  i_str.ic_timout = 0;
  i_str.ic_len = sizeof(struct eucioc);
  i_str.ic_dp = (char *)&wp;
  if (ioctl(0, I_STR, &i_str) < 0) {
	  fprintf(stderr,"stty: ");
	  perror("ioctl I_STR");
  }
  printf("eucw %d:%d:%d:%d, scrw %d:%d:%d:%d:",
	 wp.eucw[0],wp.eucw[1], wp.eucw[2],wp.eucw[3],
	 wp.scrw[0],wp.scrw[1], wp.scrw[2],wp.scrw[3]);
}


prhfcntl()
{
  if (ioctl(0, TCGETX, &hfc) < 0) {
    fprintf(stderr, "stty: ");
    perror("TCGETX");
    exit(2);
  }

  if (!hfc.x_hflag)
	  return;
  
  if (hfc.x_hflag & RTSXOFF)
	  printf("rtsxoff ");
  if (hfc.x_hflag & CTSXON)
	  printf("ctsxon ");
  if (hfc.x_hflag & DTRXOFF)
	  printf("dtrxoff ");
  if (hfc.x_hflag & CDXON)
	  printf("cdxon ");
}
