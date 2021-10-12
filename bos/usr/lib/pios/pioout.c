static char sccsid[] = "@(#)35	1.20.1.23  src/bos/usr/lib/pios/pioout.c, cmdpios, bos41J, 9523B_all 6/7/95 16:35:30";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, outflush, outflush2, oct2char, piomsgout,
 *            sigterm_exit, sigusr1_exit, transp_init, rs_setup,
 *            rs_print, rs_cleanup, lion_setup, lion_print, lion_cleanup,
 *            cxma_setup, cxma_print, cxma_cleanup, lookup_TPF_cmds
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

/*** pioout.c - device driver interface program for the printer backend ***/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/lpio.h>
#include <sys/signal.h>
#include <memory.h>
#include <locale.h>
#include <IN/backend.h>
#include <IN/standard.h>
#include <time.h>
#include <piobe_msg.h>
#include <sys/ldr.h>
#include "piostruct.h"
#include "pioout.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <termios.h>
#include <signal.h>
#include <ttyent.h>
#include <curses.h> 
#include <term.h>
#include <string.h>
#include <sys/li.h>    /* 64-port adapter header file */
#include <sys/id.h>
#include <sys/cxma.h>  /* 128-port adapter header file */
#include <sys/mode.h>
#include <stropts.h>

/* include for getmsg routine */
#include <nl_types.h>
char *piogetmsg();

#define PREFLAG 'P'
#define SUFFLAG 'S'
#define OUTFLAG 'O'
#define LOADFLAG 'R'
#define PREFLAGSTR "P"
#define SUFFLAGSTR "S"
#define OUTFLAGSTR "O"
#define LOADFLAGSTR "R"
#define DEFVARDIR "/var/spool/lpd/pio/@local"  /* default base directory */
#define DEVNAME "\001"         /* place holder for device name (replaced
				  by piomsgout() with appropriate string) */

/* Transparent print message set number */
#define TP_MSET	1

/* Default asynchronous adapter parameter values
*/
#define DFLT_RS_BUFSIZ	10
#define DFLT_RS_DELAY	200000L
#define DFLT_LION_PRIORITY	30

#define DFLT_CXMA_MAXCPS	100
#define DFLT_CXMA_MAXCHAR	50
#define DFLT_CXMA_BUFSIZ	100

/* 128-port transparent print device offset
*/
#define CXMA_DEV	0x8000

#define ATTR_TAB_SZ	20

#define OUTC(ch)         {  *nextp++ = (unsigned char) ch;         \
			    if (nextp >= endbuf) {                 \
				    if (printer_dd)                \
					    (void) outflush(0);    \
				    else  if (ttyprint)          \
						(*TPF->print)();  \
					else			\
					    (void) outflush2();    \
			    }                                      \
			 }

#define FLUSH()          {  if (printer_dd)                        \
				    (void) outflush(1);            \
			    else  if (ttyprint)                 \
					(*TPF->print)();    \
				   else                     \
				    (void) outflush2();            \
			 }

#define PIOEXIT(code)    {  if (printer_dd) {                  \
				    (void) ioctl(1, LPRMODS, &lpr_orig); \
			    } else if (ttyprint) {                    \
					      (void) (*TPF->cleanup)();                \
					   }                                 \
			    exit(code);                                  \
			 }

#define MALLOC(memptr, size) \
   {if ((memptr = malloc(size)) == NULL) { \
	(void) fprintf(stderr,(piogetmsg(MF_PIOBE,1,MSG_MALLOC))); \
	PIOEXIT(PIOO_FAIL); \
    }}


char can_str[500];
char ff_str[500];


extern int opterr;
extern char *optarg;
extern int optopt;
extern int errno;

void piomsgout();
int sigterm_exit();
int sigusr1_exit();

char *prefixfile;
char *suffixfile;
char *outputfile;
char *sub_node;
char *readroutine;
char *basedir;

char defbasedir[] = DEFVARDIR;
char nullstring[] = "";
char placeholder[] = DEVNAME;   /* placeholder character for device name */
char msgbuf[1000];
char sub_user[200];             /* user who submitted print job */
char dev_sub[200];              /* device string destined for submitter */
char dev_irq[200];              /* device string destined for int. req'd user*/
char thishost[100];             /* name of this node */
char outbuf[BUFSIZ];            /* output buffer */

FILE *suffile;

int fildes = 1;
int statusfile = 0;
int sigterm, sigusr1;
int off_msgdone, pout_msgdone, tout_msgdone, noslct_msgdone;
int term_on_error;
long len_total;
long len_done;
long len_done_orig;
int numformfeeds;
int numcanstrs;
int len_canstr;
int len_ffstr;
int dothresh;
int lpquery_val;
int mask;
int wait_for_eof;               /* wait for EOF from printer before exiting */
int eof_level;                  /* # EOFs sent to printer - # EOFs returned */
char *nextp = outbuf;           /* where to put next char in output buffer */
char *endbuf = outbuf + BUFSIZ;         /* character after end of buffer */
int special_power=FALSE;        /* Added for A12747 */

uid_t invokers_uid;
uid_t programs_uid;


char *devname;
char *title;
char *qname;
char *qdname;
char *from;
char *ptrtype;
char *jobnum;
int   mailonly;

int rc;
int printer_dd = TRUE;          /* writing to the printer device driver? */
struct lprmod lpr_orig;
struct lprmod lpr_piobe;
struct termios spio;
struct stat statbuf;            /* used with stat system call */

struct piormsg msginfo;         /* interface to exit routine for read msgs */

/* info for chain of users to received "intervention required" messages */
struct irqelem {                /* element in chain of users to receive */
    struct irqelem *next;       /*  "intervention required" messages    */
    char *user;
    char *node;
};
struct irqelem irquser_beg;     /* dummy element at beginning of chain */
struct irqelem *irquser_end = &irquser_beg; /* ptr to last element in chain */
struct irqelem *irquserwp;      /* work pointer */

/* info for chain of strings that identify messages to be discarded */
struct element discard_beg;     /* dummy element at beginning of chain */
struct element *discard_end = &discard_beg; /* ptr to last element in chain */
struct element *discardwp;      /* work pointer */

/* info for chain of strings that identify message */
/* types to send to "intervention required" users  */
struct element irqtype_beg;     /* dummy element at beginning of chain */
struct element *irqtype_end = &irqtype_beg; /* ptr to last element in chain */
struct element *irqtypewp;      /* work pointer */

static union {                  /* used to convert back and forth between  */
    char *charptr;              /* pointer types without upsetting lint    */
    struct irqelem *irqptr;
    struct element *elemptr;
} mp;

int (*loadrc)();                /* return code from load() */

/* Declarations for Streams based serial line printers */
SP_WRITE_DECL

/* Declarations for message ipc (for Palladium) */
extern uchar_t		piomsgipc;	/* flag to indicate if msgs need be
					   sent via ipc */
extern int	pmi_initmsgipc(void);
extern int	pmi_bldhdrfrm(uchar_t,ushort_t,ushort_t,char const *);
extern int	pmi_bldintfrm(int);
extern int	pmi_bldstrfrm(const char *);
extern int	pmi_dsptchmsg(const char *);

/* Declarations for ASCII terminal attached printers */
/*****************************************************/
struct termios back, fore;  /* Terminal mode structures */
struct sigaction action;   /* Action for SIGTTIN and SIGTTOU signals */
static char stpt[100],      /* Start Passthru */
endpt[100];                 /* End Passthru */
static int stptl,          /* Start Passthru command length */
endptl;                    /* End Passthru command length */
char	ttydevname[30];    /* TTY device name. e.g. tty0 */
char	cxmadevname[80];   /* 128-port transparent print device name */
struct ttyent *outty;      /* Pointer to tty definition */
static int backtty;        /* Mode of tty (1: Background; 0: Foreground) */
static int burst_size;     /* Number of characters to burst to tty on native, 8, or 16-port adapter */
static long delay;         /* Delay between bursts to tty on native, 8, or 16-port adapter */
int ttyprint = 0;          /* Flag indicating terminal attached printer */

void transp_init(void);         /* Terminal attached printer initialization routine */

void rs_setup(void);            /* Setup routine for native, 8, and 16-port adapter */
void rs_print(void);            /* Print routine for native, 8, and 16-port adapter */
void rs_cleanup(void);

void lion_setup(void);          /* Setup routine for 64-port adapter */
void lion_print(void);          /* Print routine for 64-port adapter */
void lion_cleanup(void);

void cxma_setup(void);          /* Setup routine for 128-port adapter */
void cxma_print(void);          /* Print routine for 128-port adapter */
void cxma_cleanup(void);

char	*dev_attrtab[ATTR_TAB_SZ]; /* Device driver attribute table.
                                    dev_attrtab[0] is reserved for tty hardware
                                    discipline defined by virtual printer attribute
                                    "y0"
                                 */

/* Indices into device driver attribute table */
/* NOTE: Must start at 1 because dev_attrtab[0] is reserved. */
/*       See comment for dev_attrtab[]                       */
/*************************************************************/
enum	{ RS_BUFSIZ = 1, RS_DELAY };
enum	{ LION_PRIORITY = 1 };
enum	{ CXMA_MAXCPS = 1, CXMA_MAXCHAR, CXMA_BUFSIZ };

struct TPF_cmds {
	char	*hard_disp;  /* Hardware Discipline name */
	void	(*setup)(void);  /* Entry point for setup routine */
	void	(*print)(void);  /* Entry point for transparent print routine */
	void	(*cleanup)(void);/* Entry piont for cleanup routine */
} TPF_cmds_tab[] = {         /* Table of transparent print routines based on hardware discipline */
                             /* Arrange in alphabetical order according to hardware discipline */
	{ "cxia",  rs_setup, rs_print, rs_cleanup },      /* Native, 8, and 16-port routines */
	{ "cxma", cxma_setup, cxma_print, cxma_cleanup },  /* 128-port adapter routines */
	{ "lion", lion_setup, lion_print, lion_cleanup },  /* 64-port adapter routines */
	{ "rs", rs_setup, rs_print, rs_cleanup },         /* Native, 8, and 16-port routines */
	{ "sty",  rs_setup, rs_print, rs_cleanup }      /* Native, 8, and 16-port routines */
};
#define TPF_CMDS (sizeof(TPF_cmds_tab) / sizeof(struct TPF_cmds))
struct TPF_cmds	*TPF;
struct TPF_cmds *lookup_TPF_cmds(register char *name);
struct flock lockdat; /* lockdata for fcntl() D61224 */

#ifdef DEBUG
FILE		*dbg_fp;
#endif /* DEBUG */

/*******************************************************************************
*                                                                              *
* NAME:           pioout                                                       *
*                                                                              *
* DESCRIPTION:    Interface with printer device driver.                        *
*                                                                              *
* PARAMETERS:     argc - argument count;                                       *
*                 argv - argument vector;                                      *
*                                                                              *
* GLOBALS:                                                                     *
*     MODIFIED:                                                                *
*                                                                              *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/
main(argc, argv)
int argc;
char *argv[];
{
int cnt, cnt2, ch, val1, val2;
int bytes;
int threshcnt, done;
FILE *prefile;
char *cp1, *cp2, *p1, *p2;

/* initialize invokers_uid and programs_uid for privilege toggling */
invokers_uid = getuidx(ID_REAL);
programs_uid = getuidx(ID_SAVED);

/* toggle to the privilege domain of the invoker */
seteuid(invokers_uid);


setlocale(LC_ALL, "");
(void) signal(SIGTERM, sigterm_exit);
				/* watch for signal from piobe to terminate */
(void) siginterrupt(SIGTERM, 0);
		       /* make sure write doesn't restart if signal arrives */
(void) signal(SIGUSR1, sigusr1_exit);
				/* watch for signal from filter to terminate */

/* Set up the lockdat for fcntl() replaces lockf() D61224 */
lockdat.l_whence = 0;  /* measure from the beginning of file */
lockdat.l_start = 0;   /* start lock from the beginning */
lockdat.l_len  = 0;    /* Go till end of file */

/* Determine If We're Running Under the Spooler */
if (getenv("PIOSTATUSFILE") != NULL) {
    lockdat.l_type = F_WRLCK;
    (void) fcntl(3, F_SETLKW, &lockdat);
    (void) log_init();
    lockdat.l_type = F_UNLCK;
    (void) fcntl(3, F_SETLKW, &lockdat);
    statusfile = 1;
    devname  = getenv("PIODEVNAME");
    title    = getenv("PIOTITLE");
    qname    = getenv("PIOQNAME");
    qdname   = getenv("PIOQDNAME");
    from     = getenv("PIOFROM");
    ptrtype  = getenv("PIOPTRTYPE");
    jobnum   = getenv("PIOJOBNUM");
    mailonly = atoi(getenv("PIOMAILONLY"));
}

/* Determine Base Directory */
if ((basedir = getenv("PIOVARDIR")) == NULL)
    basedir = defbasedir;                    /* default base directory */

/* Set Up Defaults */
prefixfile = suffixfile = outputfile
      = sub_node = readroutine = nullstring;

len_ffstr = DEFAULT_FF_LEN;
(void) memcpy(ff_str, DEFAULT_FF_STR, len_ffstr + 1);
numformfeeds = DEF_NUM_FF_STRS;

len_canstr = DEFAULT_CAN_LEN;
(void) memcpy(can_str, DEFAULT_CAN_STR, len_canstr + 1);
numcanstrs = DEF_NUM_CAN_STRS;

/* Get ID of Print Job Submitter */
if (statusfile)
    (void) strncpy(sub_user, from, sizeof(sub_user) - 1);

/* Process Flags */
opterr = 0;                     /* suppress error messages to stderr */
while ((ch = getopt(argc, argv, "A:B:C:D:E:F:I:K:L:N:O:P:R:S:W:X")) != EOF) {
    switch (ch) {
    case '?':  /* unrecognized flag */
	(void) fprintf
	  (stderr, piogetmsg(MF_PIOBE, 1, MSG_BADFLAG2));
	PIOEXIT(PIOO_FAIL);
	break;
    case PREFLAG:  /* Prefix File */
	prefixfile = optarg;
	break;
    case SUFFLAG:  /* Suffix File */
	suffixfile = optarg;
	break;
    case OUTFLAG:  /* Output File */
	outputfile = optarg;
	break;
    case 'E':  /* ioctl LPQUERY Mask to Ignore */
        for (p1 = optarg; *p1 != '\0'; p1 = p2 + 1) {
            if ((p2 = strchr(p1, ',')) == NULL)
                p2 = strchr(p1, '\0') - 1;
            else
                *p2 = '\0';
            if (!strcmp(p1, "LPST_ERROR"))
                lpquery_val |= LPST_ERROR;
            else if (!strcmp(p1, "LPST_SOFT"))
                lpquery_val |= LPST_SOFT;
        }
        break;
    case 'F':  /* Form Feed String */
	len_ffstr = oct2char(ff_str, optarg);
	break;
    case 'N':  /* Number of Form Feeds to Output When EOF is Reached */
	numformfeeds = atoi(optarg);
	break;
    case 'B':  /* Total Number of Bytes For Print Job */
	len_total = atoi(optarg);
	break;
    case 'A':  /* Number of Bytes Already Printed */
	len_done = len_done_orig = atoi(optarg);
	break;
    case LOADFLAG:  /* Full Path Name of optional "read printer" Routine */
	readroutine = optarg;
	break;
    case 'D':  /* String to Send to Printer If Print Job Is Cancelled */
	len_canstr = oct2char(can_str, optarg);
	break;
    case 'C':  /* Number of Times to Send String to Printer If Cancelled */
	numcanstrs = atoi(optarg);
	break;
    case 'I':  /* User[@host] For "Intervention Required" Messages */
	       /* User IDs separated by ','; null string represents submitter*/
	done = FALSE;
	p1 = p2 = optarg;
	do {
	    if (*p2 == ',' || *p2 == '\0') {
		if (p2 == p1)
		    cp1 = sub_user;
		else
		    cp1 = p1;
		if (*p2 == '\0')
		    done = TRUE;
		else
		    *p2 = '\0';
		p1 = p2 + 1;
		MALLOC(mp.charptr, sizeof(struct irqelem));
		irquserwp = mp.irqptr; /* convert char * to struct irqelem * */
		irquserwp->next = NULL;
		irquserwp->user = cp1;
		irquserwp->node = nullstring;
		for (cp2 = cp1; *cp2; cp2++)
		    if (*cp2 == '@') {
			*cp2 = '\0';
			irquserwp->node = ++cp2;
			break;
		    }
	        irquser_end->next = irquserwp; /* add element to end of chain*/
		irquser_end = irquserwp;
	    }
	    p2++;
	} while (!done);
	break;
    case 'K':  /* Text string that identifies messages from */
	       /* PostScript printer to be discarded        */
	MALLOC(mp.charptr, sizeof(struct element));
	discardwp = mp.elemptr;  /* convert char * to struct elem * */
	discardwp->next = NULL;         /* initialize the element */
	discardwp->text = optarg;
	discard_end->next = discardwp;  /* add element to end of chain */
	discard_end = discardwp;
	break;
    case 'L':  /* Text string that specifies a PostScript message type */
	       /* to be sent to "intervenion required" users */
	MALLOC(mp.charptr, sizeof(struct element));
	irqtypewp = mp.elemptr;   /* convert char * to struct elem * */
	irqtypewp->next = NULL;         /* initialize the element */
	irqtypewp->text = optarg;
	irqtype_end->next = irqtypewp;  /* add element to end of chain */
	irqtype_end = irqtypewp;
	break;
    case 'W':  /* Wait For EOF (hex 04) From Printer Before Exiting */
	       /* (allows PostScript error messages to be accepted) */
	if (*optarg == '+')
	    wait_for_eof = TRUE;
	break;
    case 'X':  /* Added for A12747. Special handling for power off. */
	special_power = TRUE;
	break;
    } /*switch*/
} /*for*/

/* Message ipc initialization */
pmi_initmsgipc();

/* Ignore -W+ Flag If Not A Serial Device */
if (wait_for_eof && tcgetattr(1, &spio) < 0)
    wait_for_eof = FALSE;

/* Isolate User Name & Node Name of Print Job Submitter */
for (cp1 = sub_user; *cp1 != '\0'; cp1++)
    if (*cp1 == '@') {
	*cp1 = '\0';
	sub_node = ++cp1;
	break;
    }

/* If -I Flag Not Specified, Intervention Req'd User Same As Submitter */
if (irquser_beg.next == NULL) {
    MALLOC(mp.charptr, sizeof(struct irqelem));
    irquserwp = mp.irqptr;   /* convert (char *) to (struct irqelem *) */
    irquserwp->next = NULL;
    irquserwp->user = sub_user;
    irquserwp->node = sub_node;
    irquser_beg.next = irquserwp;  /* create a one-element chain */
}

/* Build Device Names For Messages */
(void) strcpy(dev_sub, devname);   /* device name for msgs to submitter */
if (ptrtype && *ptrtype != '\0') {
    (void) strcat(dev_sub, " (");
    (void) strcat(dev_sub, ptrtype);
    (void) strcat(dev_sub, ")");
}
(void) strcpy(dev_irq, dev_sub); /* device name for "intervention req'd" msgs*/
if (gethostname(thishost, sizeof(thishost)) == 0 && *thishost != '\0') {
    if (*sub_node && strcmp(sub_node, thishost)) {
	(void) strcat(dev_sub, " @ ");
	(void) strcat(dev_sub, thishost);
    }
    (void) strcat(dev_irq, " @ ");
    (void) strcat(dev_irq, thishost);
}


/* Load the Optional "read printer" Routine */
if (*readroutine != '\0') {
    if ((loadrc = (int (*)())load(readroutine, 0, 0)) == NULL) {
	cnt = sprintf(msgbuf, piogetmsg(MF_PIOBE, 1, MSG_LOAD2),
	      LOADFLAG, readroutine, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_LOAD2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(LOADFLAGSTR);
	    (void)pmi_bldstrfrm(readroutine);
	    (void)pmi_bldintfrm(errno);
	}
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    }
    if ((rc = loadbind(0, loadrc, main)) < 0) {
	cnt = sprintf(msgbuf, piogetmsg(MF_PIOBE, 1, MSG_BIND2),
	      LOADFLAG, readroutine, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_BIND2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(LOADFLAGSTR);
	    (void)pmi_bldstrfrm(readroutine);
	    (void)pmi_bldintfrm(errno);
	}
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    }
}

/* If Output is to File Instead of Printer, Open the File */
if (*outputfile != '\0') {
    (void) unlink(outputfile);
    (void) umask (0777 ^ (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP));
    fildes = open(outputfile, O_CREAT + O_WRONLY + O_TRUNC, 0660);
    if (fildes < 0)  {
	cnt = sprintf(msgbuf, piogetmsg(MF_PIOBE, 1, MSG_FOPEN2),
	      OUTFLAG, outputfile, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_FOPEN2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(OUTFLAGSTR);
	    (void)pmi_bldstrfrm(outputfile);
	    (void)pmi_bldintfrm(errno);
	}
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    }
    printer_dd = FALSE;
} else
    fildes = 1;           /* output is to printer device driver or pipe */

/* Determine If Using the Printer Device Driver; If So, Save Existing Modes */
if (printer_dd) {
    if (ioctl(1, LPRMODG, &lpr_orig) < 0) {
	if (errno == EINTR)
	    (void) sigterm_exit();
	if (errno == EBADF || errno == EFAULT) {
	    cnt = sprintf(msgbuf,
		  piogetmsg(MF_PIOBE, 1, MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
	    piomsgout(msgbuf, cnt, "", 1);
	    PIOEXIT(PIOO_FAIL);
	}
	printer_dd = FALSE;         /* not the printer device driver */
    }
}

/* If Using Printer Device Driver, Set Up Our Printer Modes */
if (printer_dd) {
    lpr_piobe.modes = PLOT + RPTERR;
    if (ioctl(1, LPRMODS, &lpr_piobe) < 0) {
	if (errno == EINTR)
	    (void) sigterm_exit();
	cnt = sprintf
	    (msgbuf, piogetmsg(MF_PIOBE, 1, MSG_IOCTL), DEVNAME, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_IOCTL,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(DEVNAME);
	    (void)pmi_bldintfrm(errno);
	}
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    }
}

	/* If not printer device and not to a file
             check for tty attached printer */
	if (isatty(fildes) && getenv("PIOASCII_ATTR")) {
		(void) transp_init(); /* Run initialization routine */
		ttyprint++; /* set flag for terminal attached printer */
	}

/* Streams initialization */
if (printer_dd) {
   SP_WRITE_INIT
}

/* Decide Whether or Not to Update Percentage Status Here */
dothresh = (statusfile && !printer_dd && !(*outputfile) && len_total != 0);

/* Process Input Data Stream and Prefix File (if any) */
bytes = threshcnt = 0;
while ((val1 = getc(stdin)) != EOF) {
    if (bytes++ == 0 && *prefixfile != '\0') {
	/* Process Prefix File */
	if ((prefile = fopen(prefixfile, "r")) == NULL) {
	    cnt = sprintf(msgbuf, piogetmsg(MF_PIOBE, 1, MSG_FOPEN2),
		  PREFLAG, prefixfile, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_FOPEN2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(PREFLAGSTR);
	    (void)pmi_bldstrfrm(prefixfile);
	    (void)pmi_bldintfrm(errno);
	}
	    piomsgout(msgbuf, cnt, "", 1);
	    PIOEXIT(PIOO_FAIL);
	}
	if (fstat(fileno(prefile), &statbuf) < 0) {
	    cnt = sprintf(msgbuf, piogetmsg(MF_PIOBE, 1, MSG_STAT2),
		  PREFLAG, prefixfile, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_STAT2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(PREFLAGSTR);
	    (void)pmi_bldstrfrm(prefixfile);
	    (void)pmi_bldintfrm(errno);
	}
	    piomsgout(msgbuf, cnt, "", 1);
	    PIOEXIT(PIOO_FAIL);
	}
	len_done -= (int) statbuf.st_size; /* prefix file size not included
					      in "percent done" calculation */
	while ((val2 = getc(prefile)) != EOF)
	    OUTC(val2);
    }
    OUTC(val1);
    if (dothresh && ++threshcnt >= BUFSIZ) {
	len_done += threshcnt;
	cnt = (len_done * 100) / len_total;
	if (cnt > 100)
	    cnt = 100;

        lockdat.l_type = F_WRLCK;
        (void) fcntl(3, F_SETLKW, &lockdat);
	if (log_init() >= 0)
	    (void) log_percent(cnt);
        lockdat.l_type = F_UNLCK;
        (void) fcntl(3, F_SETLKW, &lockdat);

	threshcnt = 0;
    }
}

/* Output Form Feed (if wanted) */
for (cnt = 0; cnt < numformfeeds; cnt++)
    for (cnt2 = 0; cnt2 < len_ffstr; cnt2++)
	OUTC(*(ff_str + cnt2));

/* Process Suffix File (if any) */
if (bytes > 0 && *suffixfile != '\0') {
    if ((suffile = fopen(suffixfile, "r")) == NULL) {
	cnt = sprintf(msgbuf, piogetmsg(MF_PIOBE, 1, MSG_FOPEN2),
	      SUFFLAG, suffixfile, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_FOPEN2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(SUFFLAGSTR);
	    (void)pmi_bldstrfrm(suffixfile);
	    (void)pmi_bldintfrm(errno);
	}
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    }
    (void) setvbuf(suffile, NULL, _IOFBF, BUFSIZ);
    while ((val2 = getc(suffile)) != EOF)
	OUTC(val2);
}

/* Cleanup */
FLUSH();

PIOEXIT(PIOO_GOOD);
return(0);       /* won't get here, but it keeps lint happy */
}

/*******************************************************************************
*                                                                              *
* NAME:           outflush                                                     *
*                                                                              *
* DESCRIPTION:    Flush the output buffer to the printer                       *
*                                                                              *
* PARAMETERS:     cleanup: 0 = called by OUTC(); normal processing             *
*                          1 = called by FLUSH(); cleanup time                 *
*                                                                              *
* GLOBALS:                                                                     *
*     MODIFIED:                                                                *
*                                                                              *
* RETURN VALUES:  0                                                            *
*                                                                              *
* ERROR EXIT:      exit PIOOBAD                                                *
*                                                                              *
*                                                                              *
*******************************************************************************/

outflush(cleanup)
int cleanup;            /* 0 = called by OUTC(); 1 = called by FLUSH() */
{
int bytes_read, bytes_sent, bytes_to_send, cnt, rtnrc;
struct lpquery query_info;
char *readbuf, *cp1, *cp2;
char *sendp;                    /* ptr to where data is to be written from */
struct piormsg *msgp;           /* ptr to info. passed to loadable ddifexit()*/
char wkbuf[200];                /* work buffer */
int	saverrno;

#ifdef DEBUG
FILE *dbg_fp	=	fopen("/dev/console","w");
	(void)setvbuf(dbg_fp,NULL,_IONBF,0);
#endif


sendp = outbuf;
off_msgdone = pout_msgdone = tout_msgdone = noslct_msgdone = FALSE;

/* Loop Till Data Is Output, or We Give Up */
while ((bytes_to_send = nextp - sendp) > 0
      || (cleanup && wait_for_eof && eof_level > 0)) {

    if (bytes_to_send) {
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: bytes_to_send = %ld; sendp = %5.5s;\n",
		      bytes_to_send,sendp);
	errno = 0;
#	endif
	/* Send the Bytes In the Buffer to the Printer */
	bytes_sent = SP_WRITE(1, sendp, bytes_to_send);
	saverrno = errno;
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: bytes_sent = %ld; errno = %ld\n",
		      bytes_sent,saverrno);
#	endif
	if (bytes_sent < 0) {
	    if (term_on_error)
		if (sigterm)
		    PIOEXIT(PIOO_TERMQUIT)  /* no semicolon */
		else if (sigusr1)
		    PIOEXIT(PIOO_USR1QUIT);

	    if (saverrno == EINTR)
		(void) sigterm_exit(SIGTERM);

	    /* Now that write() for the drivers (esp. parallel) returns -1
	       with errno ENOTREADY for printer intervention conditions, skip
	       the exit().
	       Check for ETIME and EBUSY for serial tty (sptr) drivers. */
	    if (isastream(1) && saverrno == EBUSY)
	       bytes_sent = bytes_to_send;
	    else if (saverrno == ENOTREADY || saverrno == EBUSY ||
		     saverrno == ETIME)
	       bytes_sent = 0;
	    else {
	       cnt = sprintf(msgbuf,
		   piogetmsg(MF_PIOBE, 1, MSG_WRITE), DEVNAME, saverrno);
	       if (piomsgipc) {	/* if message is to be sent via ipc */
	          (void)pmi_bldhdrfrm(
		      ID_VAL_EVENT_ABORTED_BY_SERVER,
		      MSG_WRITE,1,MF_PIOBE);
	          (void)pmi_bldstrfrm(DEVNAME);
	          (void)pmi_bldintfrm(saverrno);
	       }
	       piomsgout(msgbuf, cnt, "", 1);
               if (special_power)
                {
                 PIOEXIT(EXITIO);
                }
               else
                {
	         PIOEXIT(PIOO_FAIL);
                }
	    }
	}
	if (wait_for_eof)
	    for (cnt = 0; cnt < bytes_sent; cnt++)
		if (*(sendp + cnt) == '\004')
		    eof_level++;
	sendp += bytes_sent;
	/* Update Percentage Status */
	len_done += bytes_sent;
	if (statusfile && len_total != 0 && len_done > len_done_orig) {
	    cnt = (len_done * 100) / len_total;
	    if (cnt > 100)
		cnt = 100;
	    lockdat.l_type = F_WRLCK;
	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_percent(cnt);
	    lockdat.l_type = F_UNLCK;
	    (void) fcntl(3, F_SETLKW, &lockdat);
	}
    } else
	(void) sleep(1); /* waiting for EOF (hex 04) from PostScript printer */

    /* Query to See What's Happening */
    if (ioctl(1, LPQUERY, &query_info) < 0) {
	cnt = sprintf
	(msgbuf, piogetmsg(MF_PIOBE, 1, MSG_IOCTL), DEVNAME, errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    }

    /* If Print Job Being Cancelled, Don't Wait Around */
    if (sigterm)
	if (query_info.status &
	(LPST_POUT + LPST_TOUT + LPST_ERROR + LPST_NOSLCT + LPST_SOFT)) {
	    (void)ioctl(1,TCFLSH,0);
	    PIOEXIT(PIOO_TERMQUIT);
	}

    /* Check For "Read" Condition */
    if (query_info.reccnt > 0) {
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: query_info.reccnt = %ld\n",
		      query_info.reccnt);
#	endif
	MALLOC(readbuf, query_info.reccnt + 200);
	bytes_read = read(1, readbuf, query_info.reccnt);
	if (bytes_read < 0) {
	    if (errno == EINTR)
		(void) sigterm_exit(SIGTERM);
	    cnt = sprintf(msgbuf,
		  piogetmsg(MF_PIOBE, 1, MSG_READ), DEVNAME, errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_READ,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
	    piomsgout(msgbuf, cnt, "", 1);
	    PIOEXIT(PIOO_FAIL);
	}
	*(readbuf + bytes_read) = '\0';

	/* Look For end-of-file Character Returned by Printer */
	for (cp1 = readbuf; *cp1; cp1++)
	    if (*cp1 == '\004') {
		for (cp2 = cp1; *cp2; cp2++)
		    *cp2 = *(cp2 + 1);
		if (eof_level > 0)
		    eof_level--;
	    }

	/* Set Up Default Values */
	msgp = &msginfo;
	msgp->textptr = readbuf;    /* msg read from printer */
	msgp->sub_predef = 1;       /* assume "Message from printer xxx." */
	MALLOC(msgp->sub_text, bytes_read + 101);
	*(msgp->sub_text) = '\0';   /* buffer for messages to submitter */
	msgp->irq_predef = 1;       /* assume "Message from printer xxx." */
	MALLOC(msgp->irq_text, bytes_read + 101);
	*(msgp->irq_text) = '\0';   /* buffer for msgs to "int. req'd" user */
	msgp->irq_typep = irqtype_beg.next; /* chain of message types for
				       "intervention req'd" user */
	msgp->discardp = discard_beg.next; /* chain of strings identifying
				       messages to be discarded */

	if (*readroutine != NULL) { /* if routine supplied to parse msg */
	    /* Let dynamically loaded parsemsg() parse the message text */
	    rtnrc = parsemsg(msgp);  /* call the loaded subroutine */
	} else {                   /* no read routine, so just send the msg */
	    /* No msg parse subroutine specified, so we'll do it */
	    { /* open block */
	    char *sub_ptr, *irq_ptr, *p1, *p2, *p3, charsave;
	    struct element *ep;

	    sub_ptr = msgp->sub_text;
	    irq_ptr = msgp->irq_text;
	    p1 = msgp->textptr;
	    while (p1 = strstr(p1, BEGSTR)) {
		if (!(p2 = strstr(p1 + BEGLEN, ENDSTR)))
		    break;
		charsave = *(p2 + ENDLEN);
		*(p2 + ENDLEN) = '\0';
		for (ep = msgp->discardp; ep; ep = ep->next)
		    if (strstr(p1, ep->text))
			goto NEXTMSG;
		for (ep = msgp->irq_typep; ep; ep = ep->next)
		    if (p3 = strstr(p1 + BEGLEN, ep->text)) {
			p3 += strlen(ep->text) + 2;
			(void) strncpy(irq_ptr, p3, (p2-p3));
			irq_ptr += (p2-p3);
			*irq_ptr++ = '\n';
			goto NEXTMSG;
		    }
		(void) strncpy(sub_ptr, p1, p2 + ENDLEN - p1);
		sub_ptr += p2 + ENDLEN - p1;
		*sub_ptr++ = '\n';
	      NEXTMSG:
		p1 = p2 + ENDLEN;
		*p1 = charsave;
	    }
	    *sub_ptr = *irq_ptr = '\0';
	    } /* close block */
	    rtnrc = 0;
	}

	/* check for place-holder character */
	if (msgp->sub_text)
	    for (cp1 = msgp->sub_text; *cp1; cp1++)
		if (*cp1 == *placeholder)
		    *cp1 = ' ';
	if (msgp->irq_text)
	    for (cp1 = msgp->irq_text; *cp1; cp1++)
		if (*cp1 == *placeholder)
		    *cp1 = ' ';

	/* Send Message (if any) for Print Job Submitter */
	if (msgp->sub_text && *(msgp->sub_text)) {
	    if (msgp->sub_predef)  /* if "Message from printer xxx:" wanted */
	    {
		(void) sprintf(msgbuf,
		      piogetmsg(MF_PIOBE, 1, MSG_READHDR),DEVNAME);
	        if (piomsgipc) {	/* if message is to be sent via ipc */
	           (void)pmi_bldhdrfrm(
		      ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,
		      MSG_READHDR,1,MF_PIOBE);
		   (void)pmi_bldstrfrm(DEVNAME);
	        }
	    }
	    else
	    {
		*msgbuf = '\0';
	        if (piomsgipc) {	/* if message is to be sent via ipc */
	           (void)pmi_bldhdrfrm(
		      ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,0,0,"");
	        }
	    }
	    piomsgout(msgp->sub_text, strlen(msgp->sub_text), msgbuf, 1);
	}

	/* Send Message (if any) for "Intervention Required" User */
	if (msgp->irq_text && *(msgp->irq_text)) {
	    if (msgp->irq_predef)  /* if "Message from printer xxx:" wanted */
	    {
		(void) sprintf(msgbuf,
		      piogetmsg(MF_PIOBE, 1, MSG_READHDR),DEVNAME);
	        if (piomsgipc) {	/* if message is to be sent via ipc */
	           (void)pmi_bldhdrfrm(
		      ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,
		      MSG_READHDR,1,MF_PIOBE);
		   (void)pmi_bldstrfrm(DEVNAME);
	        }
	    }
	    else
	    {
		*msgbuf = '\0';
	        if (piomsgipc) {	/* if message is to be sent via ipc */
	           (void)pmi_bldhdrfrm(
		      ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,0,0,"");
	        }
	    }
	    piomsgout(msgp->irq_text, strlen(msgp->irq_text), msgbuf, 2);
	}

	/* See If Need to Exit */
	if (rtnrc == PIOR_END)
	    PIOEXIT(PIOO_GOOD)  /* no ; */
	else if (rtnrc == PIOR_BAD)
	    PIOEXIT(PIOO_BAD)   /* no ; */
	else if (rtnrc == PIOR_FAIL)
	    PIOEXIT(PIOO_FAIL);
    }

    /* Check For "Power Off" Condition */
    if (query_info.status & LPST_OFF) {
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: LPST_OFF!!!\n");
#	endif
	if (!off_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(WAITING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    cnt = sprintf(msgbuf,
		piogetmsg(MF_PIOBE, 1, MSG_INTREQ), DEVNAME);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,
		   MSG_PAPER,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
	    }
	    piomsgout(msgbuf, cnt, "", 2);
	    off_msgdone = TRUE;
	}
	continue;
    } else
	if (off_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(RUNNING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    off_msgdone = FALSE;
	}

    /* Check For "Paper Out" Condition */
    if (query_info.status & LPST_POUT) {
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: LPST_POUT!!!\n");
#	endif
	if (!pout_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(WAITING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    cnt = sprintf(msgbuf,
		piogetmsg(MF_PIOBE, 1, MSG_PAPER), DEVNAME);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,
		   MSG_PAPER,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
	    }
	    piomsgout(msgbuf, cnt, "", 2);
	    pout_msgdone = TRUE;
	}
	continue;
    } else
	if (pout_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(RUNNING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    pout_msgdone = FALSE;
	}

    /* Check For "Off line" ("Intervention Required") Condition */
    if (query_info.status & LPST_NOSLCT) {
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: LPST_NOSLCT!!!\n");
#	endif
	if (!noslct_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(WAITING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    cnt = sprintf(msgbuf,
		piogetmsg(MF_PIOBE, 1, MSG_INTREQ), DEVNAME);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,
		   MSG_INTREQ,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
	    }
	    piomsgout(msgbuf, cnt, "", 2);
	    noslct_msgdone = TRUE;
	}
	continue;
    } else
	if (noslct_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(RUNNING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    noslct_msgdone = FALSE;
	}

    /* Check For "Intervention Required" Condition */
    if (query_info.status & LPST_TOUT) {
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: LPST_TOUT!!!\n");
#	endif
	if (!tout_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(WAITING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    cnt = sprintf(msgbuf,
		piogetmsg(MF_PIOBE, 1, MSG_INTREQ), DEVNAME);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_WARNING_RESOURCE_NEEDS_ATTENTION,
		   MSG_INTREQ,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
	    }
	    piomsgout(msgbuf, cnt, "", 2);
	    tout_msgdone = TRUE;
	}
	continue;
    } else
	if (tout_msgdone) {
    	    lockdat.l_type = F_WRLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    if (log_init() >= 0)
		(void) log_status(RUNNING);
    	    lockdat.l_type = F_UNLCK;
    	    (void) fcntl(3, F_SETLKW, &lockdat);
	    tout_msgdone = FALSE;
	}

    /* Check For "Error" Condition */
    if ((mask = query_info.status & (LPST_ERROR + LPST_SOFT))
       && mask != lpquery_val) {

#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: LPST_ERROR or LPST_SOFT!!!\n");
#	endif
        /* Build Error Message */
        *wkbuf = '\0';
        if (query_info.status & LPST_POUT)
            strcat(wkbuf, "LPST_POUT");
        if (query_info.status & LPST_TOUT) {
            if (*wkbuf != '\0')
                strcat(wkbuf, " + ");
            strcat(wkbuf, "LPST_TOUT");
        }
        if (query_info.status & LPST_ERROR) {
            if (*wkbuf != '\0')
                strcat(wkbuf, " + ");
            strcat(wkbuf, "LPST_ERROR");
        }
        if (query_info.status & LPST_BUSY) {
            if (*wkbuf != '\0')
                strcat(wkbuf, " + ");
            strcat(wkbuf, "LPST_BUSY");
        }
        if (query_info.status & LPST_NOSLCT) {
            if (*wkbuf != '\0')
                strcat(wkbuf, " + ");
            strcat(wkbuf, "LPST_NOSLCT");
        }
        if (query_info.status & LPST_SOFT) {
            if (*wkbuf != '\0')
                strcat(wkbuf, " + ");
            strcat(wkbuf, "LPST_SOFT");
        }
        if (query_info.reccnt != 0)
            sprintf(wkbuf + strlen(wkbuf)," (reccnt = %d)", query_info.reccnt);

	cnt = sprintf(msgbuf,
	    piogetmsg(MF_PIOBE, 1, MSG_ERROR), DEVNAME, wkbuf);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_ERROR,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(DEVNAME);
	    (void)pmi_bldstrfrm(wkbuf);
	}
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    }
}

nextp = outbuf;                 /* now have empty buffer */
#ifdef DEBUG
(void)fclose(dbg_fp);
#endif
return(0);
}


/*******************************************************************************
*                                                                              *
* NAME:           outflush2                                                    *
*                                                                              *
* DESCRIPTION:    Flush the output buffer to the output file                   *
*                                                                              *
* PARAMETERS:     (none)                                                       *
*                                                                              *
* GLOBALS:                                                                     *
*     MODIFIED:                                                                *
*                                                                              *
* RETURN VALUES:  0                                                            *
*                                                                              *
* ERROR EXIT:      exit PIOOBAD                                                *
*                                                                              *
*                                                                              *
*******************************************************************************/

outflush2()
{
int bytes_read, bytes_sent, bytes_to_send, cnt;
int fullpipecnt = 0;
char *sendp;                    /* ptr to where data is to be written from */

sendp = outbuf;

/* Loop Till Data Is Output, or We Give Up */
while ((bytes_to_send = nextp - sendp) > 0) {

    /* Send the Bytes In the Buffer to the Printer */
    bytes_sent = write(fildes, sendp, bytes_to_send);
    if (bytes_sent < 0) {
	if (term_on_error)
	    if (sigterm)
		PIOEXIT(PIOO_TERMQUIT)  /* no semicolon */
	    else if (sigusr1)
		PIOEXIT(PIOO_USR1QUIT);

	if (errno == EINTR)
	    (void) sigterm_exit(SIGTERM);
	cnt = sprintf(msgbuf,
	    piogetmsg(MF_PIOBE, 1, MSG_WRITE), DEVNAME, errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_WRITE,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
	piomsgout(msgbuf, cnt, "", 1);
	PIOEXIT(PIOO_FAIL);
    } else if (bytes_sent == 0 && sigterm) {
	    /* if pipe is full and SIGTERM has been received */
	if (++fullpipecnt > 5)          /* if threshold exceeded */
	    PIOEXIT(PIOO_TERMQUIT);         /* give up */
	sleep(1);
	continue;
    }
    fullpipecnt = 0;
    sendp += bytes_sent;
    /* Update Percentage Status */
    len_done += bytes_sent;
    if (statusfile && len_total != 0 && len_done > len_done_orig) {
	cnt = (len_done * 100) / len_total;
	if (cnt > 100)
	    cnt = 100;
    	lockdat.l_type = F_WRLCK;
    	(void) fcntl(3, F_SETLKW, &lockdat);
	if (log_init() >= 0)
	    (void) log_percent(cnt);
    	lockdat.l_type = F_UNLCK;
    	(void) fcntl(3, F_SETLKW, &lockdat);
    }
}

nextp = outbuf;                 /* now have empty buffer */
return(0);
}


/*******************************************************************************
*                                                                              *
* NAME:           oct2char                                                     *
*                                                                              *
* DESCRIPTION:    Converts octal substrings embedded in odm attribute values   *
*                 into 1 byte characters.  For example a substring "\77" or    *
*                 "\077" will be converted to a character '?'.
*                                                                              *
* PARAMETERS:     instr  - input string
*                 outstr - output string
*                                                                              *
* RETURN VALUES:  length of output string
*                                                                              *
*******************************************************************************/

oct2char(outstr, instr)
    char *outstr;               /* output string */
    char *instr;                /* input string */
{
    char *newstring_p;          /* pointer to transformed string */
    char *string_p;             /* pointer to original string string memory */

    int backslash       = 0;    /* '\' flag, 1 if one is found */
    int octcnt          = 0;    /* number of digits following a '\' */
    int octindex        = 0;    /* index into octarray */
    int octvalue        = 0;    /* value of octal string */

    unsigned int octarray[3];   /* array which holds "octal" chars */
    unsigned int newstrsiz = 0; /* size of the transformed string */

    newstring_p = outstr;

    for (string_p = instr; *string_p != '\0'; string_p++)
    {
	if (*string_p == '\\')
	{
	    /* check for an escaped '\', i.e. "\\" */
	    if (backslash = (backslash) ? 0 : 1)
		continue;
	}
	else if (backslash) /* should be some octal digits coming our way */
	{
	    /* copy a max of 3 octal digits to octarray */
	    for (octcnt =0; octcnt <3 && (*string_p >= '0' && *string_p <='7');)
	    {
		octarray[octcnt] = *string_p -'0';  /* convert char to int */
		octcnt++, string_p++;
	    }
	    if (octcnt)
	    {
		/* total up the octal digits */
		for (octindex = --octcnt; octindex >= 0; octindex--)
		    octvalue +=(1<<(3 *(octcnt -octindex)))* octarray[octindex];

		if (octvalue > 255)
		{
		    (void) fprintf(stderr, piogetmsg(MF_PIOBE, 1,
			  MSG_BADSTRING), optopt);
		    PIOEXIT(PIOO_FAIL);
		}
		string_p--;
		*newstring_p = (char) octvalue; /* copy new octval to new str */
		newstring_p++; newstrsiz++;
		backslash = octvalue = 0;
		continue;
	    }
	    else
	    {
		(void) fprintf(stderr, piogetmsg(MF_PIOBE, 1,
		      MSG_BADSTRING), optopt);
		PIOEXIT(PIOO_FAIL);
	    }
	}
	*newstring_p = *string_p;
	newstring_p++;
	newstrsiz++;
    }

    return(newstrsiz);
}


/*******************************************************************************
*                                                                              *
* NAME:           piomsgout                                                    *
*                                                                              *
* DESCRIPTION:    If called by Qdaemon piomsgout locks the statusfile and      *
*                 calls sysnot() and then release the lock otherwise           *
*                 the message goes to standard error.                          *
*                                                                              *
* PARAMETERS:     msgstr - message text                                        *
*                 msglen - length of msgstr                                    *
*                 subhead = string placed after msg header & before message    *
*                 target - 1 submitter                                         *
*                          2 "intervention required" user(s) from si attribute.*
*                                                                              *
* GLOBALS:                                                                     *
*     REFERENCED: statusfile - indicates whether a statusfile exists.          *
*                                                                              *
*                                                                              *
* RETURN VALUES:  void                                                         *
*                                                                              *
*******************************************************************************/

void
piomsgout(msgstr, msglen, subhead, target)
char *msgstr;       /* error message test */
int msglen;         /* length of message */
char *subhead;      /* string placed after msg header & before message */
int target;         /* 1 (submitter), 2 (int req'd user - "si"), 3 (both) */
{
    char *msgptr, *catmsg, *cp1, *cp2, *cp3, *msgtxt;
    char fname[200];
    char *dev_str;
    int  hdrlen, subhdrlen, cnt;
    struct irqelem *irqp;
    FILE *fd;
    char wkbuf[1000];
    char titlebuf[1000];

    if (statusfile)
    {
	subhdrlen = strlen(subhead);
	catmsg = piogetmsg(MF_PIOBE, 1, MSG_SPOOLHDR);
	(void) sprintf(titlebuf, "%s (%s)", jobnum, title);
	hdrlen = sprintf(wkbuf, catmsg, titlebuf);
	MALLOC(msgptr, msglen + hdrlen +subhdrlen + 200);
	(void) memcpy(msgptr, wkbuf, hdrlen);
	msgtxt = msgptr + hdrlen;

	if (target == 2)
	    dev_str = dev_irq;
	else
	    dev_str = dev_sub;
	cp1 = msgtxt;
	/* copy subhead text, replacing place holder with device name */
	for (cp2 = subhead; *cp2 != '\0'; cp2++)
	    if (*cp2 == *placeholder)
		for (cp3 = dev_str; *cp3 != '\0'; cp3++)
		    *cp1++ = *cp3;
	    else
		*cp1++ = *cp2;
	/* copy message text, replacing place holder with device name */
	for (cp2 = msgstr; (cp2 - msgstr) < msglen; cp2++)
	    if (*cp2 == *placeholder)
		for (cp3 = dev_str; *cp3 != '\0'; cp3++)
		    *cp1++ = *cp3;
	    else
		*cp1++ = *cp2;
	*cp1 = '\0';

	(void) freopen("/dev/null", "a", stderr);
    	lockdat.l_type = F_WRLCK;
    	(void) fcntl(3, F_SETLKW, &lockdat);
	if (piomsgipc) {		/* if message is to be sent via ipc */
	   (void)pmi_dsptchmsg(msgptr);
	}
	else {
	if (target == 2) {
	    irqp = irquser_beg.next;
	    /* if submitter is the only user in list */
	    if ((irqp->next == NULL) && (!strcmp(irqp->user,sub_user)))
		(void) sysnot(irqp->user, irqp->node, msgptr,
				mailonly?DOMAIL:DOWRITE);
	    else
		for (; irqp; irqp = irqp->next)
		    (void) sysnot(irqp->user, irqp->node, msgptr,DOWRITE);
	}
	else
	    (void) sysnot(sub_user, sub_node, msgptr, mailonly?DOMAIL:DOWRITE);
	}

	{   /* keep a copy of the message in var dir for autopsies */
	long clock;
	struct tm *tdate;
	char pdate[50];
	clock = time ((long *) 0);  /* get the time */
	tdate = localtime(&clock);
	(void) strftime(pdate,350,"%a %h %d %T %Y",tdate);

        (void) sprintf(fname, "%s/msg2.%s:%s", basedir, qname, qdname);

	(void) unlink(fname);
	(void) umask (0777 ^ (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
	cnt = open(fname, O_CREAT + O_WRONLY + O_TRUNC, 0664);
	if (cnt > 0) {
	    fd = fdopen(cnt, "w");
	    fprintf(fd,
	    "-----------------------------------------------------------\n");
	    fprintf(fd,"%s", pdate);
	    if (target == 2) {
		for (irqp = irquser_beg.next; irqp; irqp = irqp->next) {
		    fprintf(fd, " (%s", irqp->user);
		    if (*(irqp->node))
			fprintf(fd, " @ %s", irqp->node);
		    fprintf(fd, ")");
		}
	    } else {
		fprintf(fd, " (%s", sub_user);
		if (*sub_node)
		    fprintf(fd, " @ %s", sub_node);
		fprintf(fd, ")");
	    }

	    fprintf(fd, "\n");

	    fprintf(fd,
	    "-----------------------------------------------------------\n\n");
	    cnt=fprintf(fd,"%s",msgptr);
	    fflush(fd);
	}
	} /* end block */
    	lockdat.l_type = F_UNLCK;
    	(void) fcntl(3, F_SETLKW, &lockdat);
    }
    else {
	if (piomsgipc) {		/* if message is to be sent via ipc */
	   (void)pmi_dsptchmsg(msgstr);
	}
	else {
	   (void) fprintf(stderr, "%s\n", msgstr);
	   (void) fflush(stderr);
	}
    }
}

/*******************************************************************************
*                                                                              *
* NAME:           sigterm_exit                                                 *
*                                                                              *
* DESCRIPTION:    Exit routine for SIGTERM signal                              *
*                                                                              *
* PARAMETERS:     signal value                                                 *
*                                                                              *
* RETURN VALUES:  (none)                                                       *
*                                                                              *
*******************************************************************************/

sigterm_exit()
{
int cnt, cnt1, cnt2, val2, rc;
int i;
#ifdef DEBUG
FILE *dbg_fp;
#endif

(void) signal(SIGTERM, SIG_IGN);
sigterm = TRUE;
#	ifdef DEBUG
	dbg_fp	=	fopen("/dev/console","w");
	(void)setvbuf(dbg_fp,NULL,_IONBF,0);
	(void)fprintf(dbg_fp,"pioout: sigterm_exit entered\n");
#endif

if (printer_dd) {
	/* For serial printers, flush the data and quit. */
	if (isastream(fildes)) {
		(void)ioctl(1,TCFLSH,TCIOFLUSH);
		PIOEXIT(PIOO_TERMOK);
	}

	/* If Can't Print, Terminate */
	if (off_msgdone || pout_msgdone || tout_msgdone || noslct_msgdone)
		PIOEXIT(PIOO_TERMQUIT);
} else
	rc = fcntl(fildes, F_SETFL, O_NDELAY); /* if pipe, will get return
						  code of 0 from write() when
						  pipe is full */
/* If Encounter Print Problem, Terminate */
term_on_error = TRUE;

/* Send Nulls (or whatever) In Case We're In the Middle of a Command */
nextp = outbuf;
/* Control the number of Nulls on transparent printers */
if (ttyprint)
	i = 1;
else
	i = numcanstrs - len_ffstr;
for (cnt1 = 0; cnt1 < i ; cnt1++)
	for (cnt2 = 0; cnt2 < len_canstr; cnt2++)
		OUTC(*(can_str + cnt2));

for (cnt2 = 0; cnt2 < len_ffstr; cnt2++)
    OUTC(*(ff_str + cnt2));

/* Process Suffix File (if any) */
if (*suffixfile != '\0') {
	if ((suffile = fopen(suffixfile, "r")) == NULL) {
		cnt = sprintf(msgbuf,
			piogetmsg(MF_PIOBE, 1, MSG_FOPEN2),
			SUFFLAG, suffixfile, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_FOPEN2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(SUFFLAGSTR);
	    (void)pmi_bldstrfrm(suffixfile);
	    (void)pmi_bldintfrm(errno);
	}
		piomsgout(msgbuf, cnt, "", 1);
		PIOEXIT(PIOO_FAIL);
	}
	while ((val2 = getc(suffile)) != EOF)
		OUTC(val2);
}

FLUSH();

#	ifdef DEBUG
	(void)fprintf(dbg_fp,"pioout: sigterm_exit exiting\n");
	(void)fclose(dbg_fp);
#	endif
PIOEXIT(PIOO_TERMOK);
}


/*******************************************************************************
*                                                                              *
* NAME:           sigusr1_exit                                                 *
*                                                                              *
* DESCRIPTION:    Exit routine for SIGUSR1 signal                              *
*                                                                              *
* PARAMETERS:     signal value                                                 *
*                                                                              *
* RETURN VALUES:  (none)                                                       *
*                                                                              *
*******************************************************************************/

sigusr1_exit()
{
int val2, cnt;

(void) signal(SIGUSR1, SIG_IGN);
sigusr1 = TRUE;

if (printer_dd) {
	/* For serial printers, flush the data and quit. */
	if (isastream(fildes)) {
		(void)ioctl(1,TCFLSH,TCIOFLUSH);
		PIOEXIT(PIOO_USR1OK);
	}

	/* If Can't Print, Terminate */
	if (off_msgdone || pout_msgdone || tout_msgdone || noslct_msgdone)
		PIOEXIT(PIOO_USR1QUIT);
	/* If Encounter Print Problem, Terminate */
	term_on_error = TRUE;
}

if (*suffixfile != '\0') {
	if ((suffile = fopen(suffixfile, "r")) == NULL) {
		cnt = sprintf(msgbuf,
			piogetmsg(MF_PIOBE, 1, MSG_FOPEN2),
			SUFFLAG, suffixfile, errno);
	if (piomsgipc) {	/* if message is to be sent via ipc */
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_FOPEN2,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(SUFFLAGSTR);
	    (void)pmi_bldstrfrm(suffixfile);
	    (void)pmi_bldintfrm(errno);
	}
		piomsgout(msgbuf, cnt, "", 1);
		PIOEXIT(PIOO_FAIL);
	}
	while ((val2 = getc(suffile)) != EOF)
		OUTC(val2);
}

FLUSH();

PIOEXIT(PIOO_USR1OK);
}

/*
*******************************************************************************
*******************************************************************************
** NAME:        piogetmsg()
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
static char *msgbuffer = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

char *piogetmsg(CatName, set, num)
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
		if (!msgbuffer)
			msgbuffer = malloc(4001);
		if (msgbuffer)
			strncpy(msgbuffer, ptr, 4000);
		if (strcmp(ptr,"dummy") != 0)  /* did catgets fail? */
			return(msgbuffer);
		}
	
	if (def_catd == 0)
		{
		sprintf(defpath,"/usr/lib/lpd/pio/etc/%s",CatName);
		def_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	sprintf(default_msg,"Cannot access message catalog %s.\n",CatName);
	ptr = catgets(def_catd,set,num, default_msg);
	if (!msgbuffer)
		msgbuffer = malloc(4001);
	if (msgbuffer)
		strncpy(msgbuffer, ptr, 4000);

	free(defpath);
	return(msgbuffer);
}

/*******************************************************************************
* FUNCTIONS TO SUPPORT ASCII TERMINAL ATTACHED PRINTERS
********************************************************************************
*/

/*******************************************************************************
*                                                                              
* NAME: 	transp_init                                                        
*                                                                              
* DESCRIPTION:  Initialization routine for terminal connected printers.        
*               1. Get tty device name.                                        
*               2. Get device driver attributes from environment, if any       
*               3. Get terminal printer access cmds from terminfo DB           
*               4. Get hardware discipline based on asynchronous adapter type.
*               5. Populate TPF_cmds struct                                   
*               6. Setup signal handlers                                      
*
* PARAMETERS: NONE
*
* GLOBALS:
*     MODIFIED: ttydevname, dev_attrtab, stpt, stptl, endpt, endptl
*                                                                              
* RETURN VALUES:
*
********************************************************************************
*/
void
transp_init(void)
{
	struct str_list	strlst;
	char	*termtype;  /* stores termname found in PIOTERM  or TTY ODM */
	char *tmp;
	char *p;
	char *sp, *ep;
	int stat;
	int i;
	int cnt;

#	ifdef DEBUG
	dbg_fp = fopen("/dev/console","w");
	(void)setvbuf(dbg_fp,NULL,_IONBF,0);
#	endif /* DEBUG */

	/* Get TTY device name */
	if (ioctl(1,TXTTYNAME, ttydevname) == -1) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* if multiplexed, remove channel indicator */
	if ((tmp = strrchr(ttydevname,'/')) != NULL)
		strcpy(tmp,"\0");
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"TXTTYNAME: ttydevname = %s\n",ttydevname);
#	endif /* DEBUG */

	/* Get device driver attributes from environment, if any 
	   then load device driver attribute table.
	   */
	bzero(dev_attrtab,sizeof(dev_attrtab));
	if ((tmp = getenv("PIOASCII_ATTR")) != NULL) {
		i=0;
		while ((i < ATTR_TAB_SZ) && ((p = strtok(tmp,":")) != NULL))	{
			if (strcmp(p,"<na>") == 0) { /* null attribute accepted */
				tmp = NULL;
				i++;
				continue;
			}
			if ((tmp = malloc(strlen(p)+1)) != NULL) {
				strcpy(tmp,p);
				dev_attrtab[i] = tmp; /* load table */
			}
			tmp = NULL;
			i++;
		}
	}
#	ifdef DEBUG
	for (i = 0; i < ATTR_TAB_SZ; i++)
	   (void)fprintf(dbg_fp,"PIOASCII_ATTR: dev_attrtab[%d] = %s\n",
			 i,dev_attrtab[i]);
#	endif /* DEBUG */

	/* Get terminal type.
	   1. Check PIOTERM environment variable.
	   2. Check tty description file.
	   */
	termtype = NULL;
	if ((termtype = getenv("PIOTERM")) == (char *)NULL) {
		/* raise to program's privilege */
		seteuid(programs_uid);
		if ((outty = getttynam(ttydevname)) != NULL) 
			termtype = outty->ty_type;
		/* lower to invoker's privilege */
		seteuid(invokers_uid);
	}
	if (termtype == (char *) NULL) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_UNKNOWN_TERM));
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_UNKNOWN_TERM,TP_MSET,MF_PIOBE);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"getttynam: termtype = %s\n",termtype);
#	endif /* DEBUG */

	/* Read in terminfo entry for terminal */
	setupterm(termtype,1,&stat);
	if (stat != 1) {
		cnt=sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_TERMINFO1),termtype);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_TERMINFO1,TP_MSET,MF_PIOBE);
		(void)pmi_bldstrfrm(termtype);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* Get start passthru command from terminfo database */
	if ((sp = tparm(prtr_on)) != NULL) {
		strcpy(stpt,sp);
		stptl = strlen(stpt);
	} else {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_TERMINFO2),"mc5",termtype);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_TERMINFO2,TP_MSET,MF_PIOBE);
		(void)pmi_bldstrfrm("mc5");
		(void)pmi_bldstrfrm(termtype);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"tparm: prtr_on = %s\n",stpt);
#	endif /* DEBUG */

	/* Get stop passthru command from terminfo database */
	if ((ep = tparm(prtr_off)) != NULL) {
		strcpy(endpt,ep);
		endptl = strlen(endpt);
	} else {
		cnt=sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_TERMINFO2),"mc4",termtype);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_TERMINFO2,TP_MSET,MF_PIOBE);
		(void)pmi_bldstrfrm("mc4");
		(void)pmi_bldstrfrm(termtype);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"tparm: prtr_off = %s\n",endpt);
#	endif /* DEBUG */

	/* Determine Hardware Discipline */
	if ((strlst.sl_nmods = ioctl(1,I_LIST,NULL)) == -1 || !strlst.sl_nmods)
	{
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
	MALLOC(strlst.sl_modlist,strlst.sl_nmods*sizeof(*strlst.sl_modlist));
	if ((i = ioctl(1,I_LIST,&strlst)) == -1 || !i) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	for (i = 0; i < strlst.sl_nmods; i++)
	   (void)fprintf(dbg_fp,"I_LIST: strlst.sl_modlist[%d].l_name = %s\n",
			 i,strlst.sl_modlist[i].l_name);
#	endif /* DEBUG */

	/* Check tty harware discipline against queue setup */
	if ((dev_attrtab[0] == NULL) ||
	    (strcmp(dev_attrtab[0],
		    (strlst.sl_modlist+strlst.sl_nmods-1)->l_name) != 0)) 
	{ /* TTY hardware discipline error */ 
		cnt = sprintf(msgbuf, piogetmsg(MF_PIOBE, TP_MSET, TP_BADHD), ttydevname, dev_attrtab[0]);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_BADHD,TP_MSET,MF_PIOBE);
		(void)pmi_bldstrfrm(ttydevname);
		(void)pmi_bldstrfrm(dev_attrtab[0]);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* Get adapter specific routines */
	if ((TPF = lookup_TPF_cmds(
		(strlst.sl_modlist+strlst.sl_nmods-1)->l_name)) != NULL) {
		(*TPF->setup)();  /* Execute setup routine */
	} else {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_UNKNOWN_ASYNC));
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_UNKNOWN_ASYNC,TP_MSET,MF_PIOBE);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* Setup signal handlers */
	action.sa_handler = SIG_IGN;
	sigaction(SIGTTIN, &action, NULL);
	sigaction(SIGTTOU, &action, NULL);
	signal(SIGHUP, sigterm_exit);

}
/*******************************************************************************
*                                                                              
* NAME:           rs_setup                                                     
*                                                                              
* DESCRIPTION:    Transparent Printing setup routine for native, 8, and 16-port
*                 adapters.                                                    
*                                                                              
* PARAMETERS:     NONE                                                         
*                                                                              
* GLOBALS:                                                                     
*     MODIFIED: burst_size, delay                                              
*                                                                              
* RETURN VALUES:                                                               
*                                                                             
********************************************************************************
*/
void
rs_setup(void)
{
	int	i;

	/* Check for user defined buffer size */
	if ((i = atoi(dev_attrtab[RS_BUFSIZ])) > 0)
		burst_size = i;
	else
		burst_size = DFLT_RS_BUFSIZ; /* if not in environment, use default */
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"rs_setup: burst_size = %d\n",burst_size);
#	endif /* DEBUG */

	/* Check for user defined delay value */
	if ((i = atoi(dev_attrtab[RS_DELAY])) > 0)
		delay = i;
	else
		delay = DFLT_RS_DELAY; /* if not in environment, use default */
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"rs_setup: delay = %d\n",delay);
#	endif /* DEBUG */

}

/*******************************************************************************
*                                                                              
* NAME:           rs_print                                                     
*                                                                              
* DESCRIPTION:    Transparent Printing routine for native, 8, and 16-port      
*                 adapters.                                                   
*                                                                              
* PARAMETERS:     NONE                                                        
*                                                                            
* GLOBALS:                                                                  
*     MODIFIED:  len_done, nextp, backtty
*                                                                          
* RETURN VALUES:                                                         
*                                                                       
********************************************************************************
*/
void
rs_print(void)
{
	int bytes_read, bytes_sent, bytes_to_send, cnt;
	int fullpipecnt = 0;
	char	*sendp;
	char	*buf;
	char	*bufinp;
	int	saverrno;

	sendp = outbuf;
	backtty=FALSE;
	MALLOC(buf, burst_size+stptl+endptl);  /* Allocate space for temp out buffer */
	memcpy(buf, stpt, stptl);              /* Copy start passthru command in buf */
	bufinp=buf+stptl;                      /* Point just beyond the start passthru command */
	while ((bytes_to_send = nextp - sendp) > 0) {

		/* Copy print data to temp buffer */
		memcpy(bufinp, sendp, bytes_read = ((bytes_to_send > burst_size) ? burst_size : bytes_to_send));

		/* Tag data with end passthru command */
		memcpy(bufinp+bytes_read, endpt, endptl);

		/* Get ttymode of output (Foreground) */
		if (tcgetattr(fildes, &fore) == -1)  {
			cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
			piomsgout(msgbuf,cnt,"",1);
			PIOEXIT(PIOO_FAIL);
		}
#		ifdef DEBUG
		(void)fprintf(dbg_fp,"tcgetattr: fore.c_oflag = %d\n",
			      fore.c_oflag);
#		endif /* DEBUG */

		back = fore;
		back.c_oflag = 0x0; /* Turn off output processing */

		/* Set ttymode of output (Background) */
		if (tcsetattr(fildes, TCSANOW, &back) == -1)  {
			cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
			piomsgout(msgbuf,cnt,"",1);
			PIOEXIT(PIOO_FAIL);
		}
		backtty=TRUE;
#		ifdef DEBUG
		(void)fprintf(dbg_fp,"tcsetattr: back.c_oflag = %d\n",
			      back.c_oflag);
#		endif /* DEBUG */

		/* Send data to printer */
		bytes_sent = write(fildes, buf, bytes_read+stptl+endptl);
		saverrno = errno;
#		ifdef DEBUG
		(void)fprintf(dbg_fp,"write: bytes_sent = %d; saverrno = %d\n",
			      bytes_sent,saverrno);
#		endif /* DEBUG */

		/* Set ttymode of output (Foreground) */
		if (tcsetattr(fildes, TCSANOW, &fore) == -1)  {
			cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
			piomsgout(msgbuf,cnt,"",1);
			PIOEXIT(PIOO_FAIL);
		}
		backtty=FALSE;
#		ifdef DEBUG
		(void)fprintf(dbg_fp,"tcsetattr: fore.c_oflag = %d\n",
			      fore.c_oflag);
#		endif /* DEBUG */


		if (bytes_sent < 0) {
			if (term_on_error)
				if (sigterm)
					PIOEXIT(PIOO_TERMQUIT) /* no semicolon */
				    else if (sigusr1)
					PIOEXIT(PIOO_USR1QUIT);

			if (saverrno == EINTR)
				(void) sigterm_exit(SIGTERM);
			cnt = sprintf(msgbuf,
			    piogetmsg(MF_PIOBE, 1, MSG_WRITE), DEVNAME, saverrno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_WRITE,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(saverrno);
	    }
			piomsgout(msgbuf,cnt,"",1);
			PIOEXIT(PIOO_FAIL);
		} else if (bytes_sent == 0 && sigterm) {
			/* if pipe is full and SIGTERM has been received */
			if (++fullpipecnt > 5)          /* if threshold exceeded */
				PIOEXIT(PIOO_TERMQUIT);         /* give up */
			sleep(1);
			continue;
		}
		fullpipecnt = 0;
		sendp += bytes_read;
		/* Update Percentage Status */
		len_done += bytes_sent;
		if (statusfile && len_total != 0 && len_done > len_done_orig) {
			cnt = (len_done * 100) / len_total;
			if (cnt > 100)
				cnt = 100;
    			lockdat.l_type = F_WRLCK;
    			(void) fcntl(3, F_SETLKW, &lockdat);
			if (log_init() >= 0)
				(void) log_percent(cnt);
    			lockdat.l_type = F_UNLCK;
    			(void) fcntl(3, F_SETLKW, &lockdat);
		}

		/* Pause between data packets */
		usleep((unsigned int) delay);
	}

	nextp = outbuf;                 /* now have empty buffer */
	free(buf);

}
/******************************************************************
* 
* NAME:			rs_cleanup
*
* DESCRIPTION:	Function to restore previous state of native, 8, 16-port
*               driver.
*
* PARAMETERS:   NONE
*
* GLOBALS:
*   MODIFIED:
*
* RETURN VALUES:
*
*******************************************************************
*/
void
rs_cleanup()
{
	/* Restore Terminal (If necessary) */
	if (backtty) {
		(void) tcsetattr(fildes, TCSANOW, &fore);
#		ifdef DEBUG
		(void)fprintf(dbg_fp,"rs_cleanup-tcsetattr:fore.c_oflag = %d\n",
			      fore.c_oflag);
#		endif /* DEBUG */
	}
}

/*******************************************************************************
* 
* NAME:           lion_setup                                              
*
* DESCRIPTION:    Transparent Printing setup routine for 64-port adapter.  
*
* PARAMETERS:     NONE                                                      
* 
* GLOBALS:                                                                   
*     MODIFIED:                                                               
*
* RETURN VALUES:                                                               
* 
********************************************************************************
*/
void
lion_setup(void)
{
	char lionfname[80];
	struct xpar_parms	xpar;
	int i;
	int cnt;


	/* Get current device driver settings */
	if (ioctl(1, LI_GETXP, &xpar)) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* Set Start Passthru */
	memset(xpar.in_xpar, 0xff, sizeof(xpar.in_xpar));
	memcpy(xpar.in_xpar, stpt, stptl);

	/* Set End Passthru */
	memset(xpar.lv_xpar, 0xff, sizeof(xpar.lv_xpar));
	memcpy(xpar.lv_xpar, endpt, endptl);

	/* Set Priority */
	if (((i = atoi(dev_attrtab[LION_PRIORITY])) > 0) && (i <= 60))
		xpar.priority = i;
	else
		xpar.priority = DFLT_LION_PRIORITY;

	/* Make settings known to 64-port adapter */
	if (ioctl(1, LI_SETXP, &xpar)) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* Construct transparent printer device name */
	sprintf(lionfname,"/dev/x%s", ttydevname);

	/* Open file on transparent printer channel */
	if ((fildes = open(lionfname, O_WRONLY)) == -1) {
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"open: %s FAILED! errno = %d\n",lionfname,errno);
#	endif /* DEBUG */
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_FOPEN1),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_FOPEN1,TP_MSET,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"open: lionfname = %s\n",lionfname);
#	endif /* DEBUG */
}

/*******************************************************************************
*                       
* NAME:           lion_print   
*                        
* DESCRIPTION:    Transparent Printing routine for 64-port adapter.           
*                
* PARAMETERS:     NONE    
*                        
* GLOBALS: 
*     MODIFIED:  len_done, nextp
*         
*        
* RETURN VALUES:     
*                   
*                  
*                 
********************************************************************************
*/
void
lion_print(void)
{
	int bytes_read, bytes_sent, bytes_to_send, cnt;
	int fullpipecnt = 0;
	char	*sendp;

	sendp = outbuf;
	while ((bytes_to_send = nextp - sendp) > 0) {

		/* Write buffer to output */
		errno = 0;
		bytes_sent = write(fildes, sendp, bytes_to_send); 
#		ifdef DEBUG
		(void)fprintf(dbg_fp,"write: bytes_sent = %d; errno = %d\n",
			      bytes_sent,errno);
#		endif /* DEBUG */

		if (bytes_sent < 0) {
			if (term_on_error)
				if (sigterm)
					PIOEXIT(PIOO_TERMQUIT) /* no semicolon */
				else if (sigusr1)
					PIOEXIT(PIOO_USR1QUIT);

			if (errno == EINTR)
				(void) sigterm_exit(SIGTERM);
			cnt = sprintf(msgbuf,
			    piogetmsg(MF_PIOBE, 1, MSG_WRITE), DEVNAME, errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_WRITE,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
			piomsgout(msgbuf,cnt,"",1);
			PIOEXIT(PIOO_FAIL);
		} else if (bytes_sent == 0 && sigterm) {
			/* if pipe is full and SIGTERM has been received */
			if (++fullpipecnt > 5)          /* if threshold exceeded */
				PIOEXIT(PIOO_TERMQUIT);         /* give up */
			sleep(1);
			continue;
		}
		fullpipecnt = 0;
		sendp += bytes_sent;
		/* Update Percentage Status */
		len_done += bytes_sent;
		if (statusfile && len_total != 0 && len_done > len_done_orig) {
			cnt = (len_done * 100) / len_total;
			if (cnt > 100)
				cnt = 100;
    			lockdat.l_type = F_WRLCK;
    			(void) fcntl(3, F_SETLKW, &lockdat);
			if (log_init() >= 0)
				(void) log_percent(cnt);
    			lockdat.l_type = F_UNLCK;
    			(void) fcntl(3, F_SETLKW, &lockdat);
		}
	}

	nextp = outbuf;                 /* now have empty buffer */
}

/******************************************************************
* 
* NAME:			lion_cleanup
*
* DESCRIPTION:	Function to restore previous state of 64-Port device
*               driver.
*
* PARAMETERS:   NONE
*
* GLOBALS:
*   MODIFIED:
*
* RETURN VALUES:
*
*******************************************************************
*/
void
lion_cleanup(void)
{
	close(fildes); /* Turn off transparent print dev. */
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"lion_cleanup: fildes closed\n");
#	endif /* DEBUG */
}
/******************************************************************
* 
* NAME:			cxma_setup
*
* DESCRIPTION:	Function to setup 128-Port device driver paramters
*               for transparent printing.
*
* PARAMETERS:   NONE
* 
* GLOBALS:
*    MODIFIED:
*
* RETURN VALUES:
*
*******************************************************************
*/
void
cxma_setup(void)
{
	char ttyname[30];
	struct stat fstat;
	cxma_t	xpar;         /* ioctl structure */
	int	i;
	int cnt;

	/* Get current settings */
	if (ioctl(1, CXMA_GETA, &xpar)) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* Set start passthrough command and length */
	memcpy(xpar.cxma_onstr, stpt, stptl);
	xpar.cxma_onlen = (unsigned char) stptl;

	/* Set end passthrough command and length */
	memcpy(xpar.cxma_offstr, endpt, endptl);
	xpar.cxma_offlen = (unsigned char) endptl;

	/* Set MAXCPS */
	if ((i = atoi(dev_attrtab[CXMA_MAXCPS])) > 0)
		xpar.cxma_maxcps = (unsigned short) i;
	else
		xpar.cxma_maxcps = DFLT_CXMA_MAXCPS;

	/* Set MAXCHAR */
	if ((i = atoi(dev_attrtab[CXMA_MAXCHAR])) > 0)
		xpar.cxma_maxchar = (unsigned short) i;
	else
		xpar.cxma_maxchar = DFLT_CXMA_MAXCHAR;
		
	/* Set BUFSIZE */
	if ((i = atoi(dev_attrtab[CXMA_BUFSIZ])) > 0)
		xpar.cxma_bufsize = (unsigned short) i;
	else
		xpar.cxma_bufsize = DFLT_CXMA_BUFSIZ;

	/* Make settings known to 128-port adapter */
	if (ioctl(1, CXMA_SETA, &xpar)) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_IOCTL),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_IOCTL,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}

	/* Get terminal device status */
	sprintf(ttyname, "/dev/%s", ttydevname);
	if (statx(ttyname, &fstat, STATXSIZE, STX_NORMAL) != 0) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,1,MSG_STAT2),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   MSG_STAT2,1,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"statx: ttyname = %s\n",ttyname);
#	endif /* DEBUG */

	/* Construct transparent printer device name */
	sprintf(cxmadevname, "%s/dev/%s", defbasedir, ttydevname);

	/* Create transparent print device */
	seteuid(programs_uid);
	unlink(cxmadevname);
	i = mknod(cxmadevname, S_IFCHR|S_IRWXU, fstat.st_rdev + CXMA_DEV) ||
	    chown(cxmadevname, invokers_uid, -1);
	seteuid(invokers_uid);
	if (i) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_FOPEN1),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_FOPEN1,TP_MSET,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"mknod: cxmadevname = %s\n",cxmadevname);
#	endif /* DEBUG */

	/* Open transparent printer device */
	if ((fildes = open(cxmadevname, O_WRONLY)) == -1) {
		cnt = sprintf(msgbuf,piogetmsg(MF_PIOBE,TP_MSET,TP_FOPEN1),DEVNAME,errno);
	    if (piomsgipc) {	/* if message is to be sent via ipc */
	        (void)pmi_bldhdrfrm(
		   ID_VAL_EVENT_ABORTED_BY_SERVER,
		   TP_FOPEN1,TP_MSET,MF_PIOBE);
		(void)pmi_bldstrfrm(DEVNAME);
		(void)pmi_bldintfrm(errno);
	    }
		piomsgout(msgbuf,cnt,"",1);
		PIOEXIT(PIOO_FAIL);
	}
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"open: cxmadevname = %s\n",cxmadevname);
#	endif /* DEBUG */

}

/******************************************************************
* 
* NAME:			cxma_print
*
* DESCRIPTION:	Function to send output to 128-Port device driver 
*               for transparent printing.
*
* PARAMETERS:   NONE
*
* GLOBALS:
*   MODIFIED:
*
* RETURN VALUES:
*
*******************************************************************
*/
void
cxma_print(void)
{
	lion_print(); /* same operation as 64-port */
}

/******************************************************************
* 
* NAME:			cxma_cleanup
*
* DESCRIPTION:	Function to restore previous state of 128-Port device
*               driver.
*
* PARAMETERS:   NONE
*
* GLOBALS:
*   MODIFIED:
*
* RETURN VALUES:
*
*******************************************************************
*/
void
cxma_cleanup(void)
{

	close(fildes);
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"cxma_cleanup: %s closed\n",cxmadevname);
#	endif /* DEBUG */
	unlink(cxmadevname);
#	ifdef DEBUG
	(void)fprintf(dbg_fp,"cxma_cleanup: %s unlinked\n",cxmadevname);
#	endif /* DEBUG */
}
/*******************************************************************************
*
* NAME:           lookup_TPF_cmds                                       
* 
* DESCRIPTION:    Function to search Transparent Printing Function Table 
*                 for entry based on the hardware discipline.             
*
* PARAMETERS:     name - hardware discipline name;                         
*
* GLOBALS:                                                                  
*     MODIFIED:                                                              
*
*
* RETURN VALUES: Pointer to TPF_cmds structure                                
* 
*
* 
********************************************************************************
*/
struct TPF_cmds *lookup_TPF_cmds(register char *name)
{
	register int lo = 0, hi = TPF_CMDS - 2;
	register int mid;
	register struct TPF_cmds	*p;

	if (name == (char *) NULL)
		return ((struct TPF_cmds *)NULL);
	while (lo <= hi) {
		if (strcmp(name, (p = TPF_cmds_tab + (mid = (lo + hi) >> 1))->hard_disp) <= 0)
			hi = mid - 1;
		else
			lo = mid + 1;
	}
	p = TPF_cmds_tab + lo;
	if (strcmp(name,p->hard_disp) == 0)
		return(p);
	else
		return(NULL);
}
