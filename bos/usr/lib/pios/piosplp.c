static char sccsid[] = "@(#)39	1.9.1.4  src/bos/usr/lib/pios/piosplp.c, cmdpios, bos411, 9428A410j 5/12/94 20:14:21";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** piosplp.c ***/

#include <stdio.h>
#include <sys/lpio.h>                           /* changed lprio.h lpio.h JBW */
#include <locale.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>                            /* include for file permissions */
#include <termios.h>
#include "piosplp_msg.h"                         /* include file for messages */

#define DEFFLAGSDIR	"/var/spool/lpd/pio/@local/flags"
#define STRGZARG(s)		#s
#define STRGZ(s)		STRGZARG(s)
#define SETAFLAG(i, f)		(i) |= 0x1U<<(f)
#define ISAFLAGSET(i, f)	((i)&0x1U<<(f))
#define BFLAG			0
#define NFLAG			1
#define PFLAG			2
#define sFLAG			3
#define SFLAG			4

/* include for getmsg routine */
#include <nl_types.h>
char *getmsg();
struct                                                 /* Table of baud rates */
    {
        char    *stringr;
        int     setr;
        int     resetr;
    } rmode[] =
    {
        "50",   B50,    CBAUD,
        "75",   B75,    CBAUD,
        "110",  B110,   CBAUD,
        "134",  B134,   CBAUD,
        "150",  B150,   CBAUD,
        "300",  B300,   CBAUD,
        "600",  B600,   CBAUD,
        "1200", B1200,  CBAUD,
        "1800", B1800,  CBAUD,
        "2400", B2400,  CBAUD,
        "4800", B4800,  CBAUD,
        "9600", B9600,  CBAUD,
        "19200",B19200, CBAUD,
        "19.2", B19200, CBAUD,
        "EXTA", B19200, CBAUD,
        "exta", B19200, CBAUD,
        "38400",B38400, CBAUD,
        "38.4", B38400, CBAUD,
        "EXTB", B38400, CBAUD,
        "extb", B38400, CBAUD,
        0,      0,      0
    };

struct             /*  Table of possible character sizes (bits per character) */
    {
        char    *stringr;
        int     setr;
        int     resetr;
    } rmode2[] =
    {
        "5",  CS5,    CSIZE,
        "6",  CS6,    CSIZE,
        "7",  CS7,    CSIZE,
        "8",  CS8,    CSIZE,
        0,      0,      0
    };


char    *dev = "/dev/lp0";                             /* assumed device name */
struct  lprio   plpst;                                  /* set/get plp buffer */
struct  termios  spio;                                   /* serial parameters */
struct  lptimer plptim;                                        /* timer value */
struct  lprmod  plpmod;                       /* replaced lprmode,oprmode JBW */

int serial;                                            /* serial printer flag */
extern char *CSlocc();

/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME:           main                                                         |
|                                                                              |
| DESCRIPTION:    Accesses the printer device driver for a parallel or serial  |
|                 port and performs one of the following functions, as         |
|                 specified by the caller:                                     |
|                    1)  No flags specified: display all the current values    |
|                    2)  Flag specified: update the specified values           |
|                                                                              |
| PARAMETERS:     argc    argument count                                       |
|                 argv    argument vector                                      |
|                                                                              |
| RETURN VALUES:  0  success                                                   |
|                 1  failure                                                   |
|                                                                              |
+-----------------------------------------------------------------------------*/

main(argc, argv)
int  argc;
char *argv[];
{       register char *arg;
        char *p;
        int  ch, fd, cnt;
        int badarg = 0;                                         /* error flag */
        int noflags = FALSE;            /* assume flags are passed to program */
        int font_init = FALSE;                 /* flag spec'd on command line */
        int font_init_only = FALSE;              /* is it the only flag specd */
        int dev_specd = FALSE;                          /* device specified ? */
        char devname[100];               /* the device name to be manipulated */
        char flagsdir[120];      /* the full path name to the flags directory */
        extern int   errno;
        extern int   optind;
        extern int   optopt;
        extern int   opterr;
        extern char *optarg;
   	uint_t		serial_flags 	= 0U;
	const char	*speed, *penable, *parity = "+", *bpc, *stops;

setlocale(LC_ALL, "");

opterr = 0;                  /* so getopt() won't output error msgs to stderr */

while ((ch =                                      /* Validate the input flags */
           getopt(argc,argv,"T:i:l:w:s:b:C:c:S:e:F:f:n:N:P:p:r:t:W:B:")) != EOF)
    switch(ch)
    {
    case 'F':
    	font_init = TRUE;
	break;
    case 'B':
	speed = optarg;
	SETAFLAG(serial_flags,BFLAG);
	break;
    case 'N':
	penable = optarg;
	SETAFLAG(serial_flags,NFLAG);
	break;
    case 'P':
	parity = optarg;
	SETAFLAG(serial_flags,PFLAG);
	break;
    case 's':
	bpc = optarg;
	SETAFLAG(serial_flags,sFLAG);
	break;
    case 'S':
	stops = optarg;
	SETAFLAG(serial_flags,SFLAG);
	break;
    case '?':
        fprintf(stderr,getmsg(MF_PIOSPLP,1,HELPMSG),optopt);
        exit(1);
    }

if (optind < (argc - 1))     /* Check - is more than one operand is specified */
    {
    fprintf(stderr,getmsg(MF_PIOSPLP,1,ONEOPERAND));
    exit(1);
    }

/* Get the device name */
if (optind == (argc - 1))                    /* there is an arg (device name) */
    {
    dev_specd = TRUE;
    dev = argv[optind];
    if (*dev != '/')
        {
        (void) strcpy(devname, "/dev/");
        (void) strcat(devname, dev);
        dev = devname;
        }
    }

/* If serial flags were specified, display error messages and exit. */
if (serial_flags)
{
   const char	*basednm = strrchr(dev,'/');
   
   basednm++;
   if (ISAFLAGSET(serial_flags,BFLAG))
      (void)fprintf(stderr,getmsg(MF_PIOSPLP,1,FLAGDISCONTD),"-B",basednm,
		    STRGZ(speed),speed);
   if (ISAFLAGSET(serial_flags,NFLAG))
      (void)fprintf(stderr,getmsg(MF_PIOSPLP,1,FLAGDISCONTD),"-N",basednm,
		    STRGZ(parity),*penable=='!'?"none":*parity=='!'?"even":
		    "odd");
   if (ISAFLAGSET(serial_flags,PFLAG))
      (void)fprintf(stderr,getmsg(MF_PIOSPLP,1,FLAGDISCONTD),"-P",basednm,
		    STRGZ(parity),*parity=='!'?"even":"odd");
   if (ISAFLAGSET(serial_flags,sFLAG))
      (void)fprintf(stderr,getmsg(MF_PIOSPLP,1,FLAGDISCONTD),"-s",basednm,
		    STRGZ(bpc),bpc);
   if (ISAFLAGSET(serial_flags,SFLAG))
      (void)fprintf(stderr,getmsg(MF_PIOSPLP,1,FLAGDISCONTD),"-S",basednm,
		    STRGZ(stops),*stops=='!'?"1":"2");
   exit(2);
}

/* Determine if any flags were specified */
if ( argc == (dev_specd + 1) ) noflags = TRUE;

/* Was the Font_Init flag the only flag specified */
if ( font_init && (argc == (dev_specd + 2)) ) font_init_only = TRUE;

optind = 1;                                     /* reset for next getopt loop */


/*-----------------------------------------------------------------------------+
|                                                                              |
|    If the font_init flag is the only one specified on the command line       |
|    then don't fiddle with the device.  This eliminates the permissions       |
|    problems associated with joe user trying to initialize his printer.       |
|                                                                              |
+-----------------------------------------------------------------------------*/

if (!font_init_only)
    {
       /* Open device.  If noflags then open in read only mode.  Add NO_DELAY */
    if (noflags) fd = open(dev, O_RDONLY | O_NDELAY);   /* to open status for */
    else         fd = open(dev, O_RDWR | O_NDELAY);     /* config at ipl time */

    if (fd < 0)                                       /* Was the open successful? */
        {
        switch (errno)
            {
            case ENODEV:
                fprintf(stderr,getmsg(MF_PIOSPLP,1,NOTCNFIGD),dev,errno);
                break;

            case EACCES:
                fprintf(stderr,getmsg(MF_PIOSPLP,1,PERMISSION),dev,errno);
                break;

            case EBUSY:
                fprintf(stderr,getmsg(MF_PIOSPLP,1,DEVBUSY),dev,errno);
                break;

            default:
                fprintf(stderr,getmsg(MF_PIOSPLP,1,DEVNOEXIST),dev,errno);
                break;
            }
        exit(1);
        }

    /* Determine if it is a printer device */
    if (ioctl(fd, LPRMODG, &plpmod) < 0)
        {
        fprintf(stderr,getmsg(MF_PIOSPLP,1,NOTPRINTER),dev, errno);
        exit(1);
        }

    /* Get current printer status */
    if ((ioctl(fd,LPRGET,&plpst) < 0) || (ioctl(fd,LPRMODG,&plpmod) < 0)
                                             || (ioctl(fd,LPRGTOV,&plptim) < 0))
        {
        fprintf(stderr,getmsg(MF_PIOSPLP,1,CANTGETSTAT),dev,errno);
        exit(1);
        }

    /* Determine if it is a serial printer */
    if (tcgetattr(fd, &spio) < 0)        serial = 0;
    else                                 serial = 1;

    }


/*  ---------------------      PROCESS FLAGS     ---------------------------- */


if (!(noflags))               /* don't bother if there weren't any arguements */
    {
    while ((ch = getopt(argc,argv,"T:i:l:w:s:b:C:c:S:e:F:f:n:N:P:p:r:t:W:B:"))
                                                            != EOF && !(badarg))
        {
        switch (ch)
            {
            case 'T':                                              /* timeout */
                plptim.v_timout = atoi(optarg);
                if (plptim.v_timout < 1) badarg = 2 ;
                break;

            case 'i':                                      /* indention value */
                plpst.ind = atoi(optarg);
                break;

            case 'l':                                       /* lines per page */
                plpst.line = atoi(optarg);
                break;

            case 'w':                                     /* width in columns */
                plpst.col = atoi(optarg);
                break;

            case 'b':                          /* send backspaces to printer? */
                plpmod.modes |= NOBS;
                if (*optarg == '+') plpmod.modes ^= NOBS;
                else if (*optarg != '!') badarg = 1;
                break;

            case 'C':                      /* convert lower case to all caps? */
                plpmod.modes |= CAPS;
                if (*optarg == '!') plpmod.modes ^= CAPS;
                else if (*optarg != '+') badarg = 1;
                break;

            case 'c':                               /* send carriage returns? */
                plpmod.modes |= NOCR;
                if (*optarg == '+') plpmod.modes ^= NOCR;
                else if (*optarg != '!') badarg = 1;
                break;

            case 'e':                                     /* return on error? */
                plpmod.modes |= RPTERR;
                if (*optarg == '!') plpmod.modes ^= RPTERR;
                else if (*optarg != '+') badarg = 1;
                break;

            case 'F':                                          /* load fonts? */
                plpmod.modes |= FONTINIT;
                if (*optarg == '!')
                    {
                    font_init = TRUE;
                    plpmod.modes ^= FONTINIT;
                    }
                else if (*optarg != '!') badarg = 4;
                break;

            case 'f':                          /* send form feeds to printer? */
                plpmod.modes |= NOFF;
                if (*optarg == '+') plpmod.modes ^= NOFF;
                else if (*optarg != '!') badarg = 1;
                break;

            case 'n':                           /* send line feeds to printer */
                plpmod.modes |= NONL;
                if (*optarg == '+') plpmod.modes ^= NONL;
                else if (*optarg != '!') badarg = 1;
                break;

            case 'p':                      /* send all characters to printer? */
                plpmod.modes |= PLOT;
                if (*optarg == '!') plpmod.modes ^= PLOT;
                else if (*optarg != '+') badarg = 1;
                break;

            case 'r':              /* insert carriage return after line feed? */
                plpmod.modes |= NOCL;
                if (*optarg == '+') plpmod.modes ^= NOCL;
                else if (*optarg != '!') badarg = 1;
                break;

            case 't':            /* expand tabs on eight position boundaries? */
                plpmod.modes |= NOTB;
                if (*optarg == '!') plpmod.modes ^= NOTB;
                else if (*optarg != '+') badarg = 1;
                break;

            case 'W':                         /* truncate lines or wrap them? */
                plpmod.modes |= WRAP;
                if (*optarg == '!') plpmod.modes ^= WRAP;
                else if (*optarg != '+') badarg = 1;
                break;
            }                                                       /* switch */
        }                                                            /* while */

    if (badarg)
        {
        switch (badarg)
            {
            case 1:
                fprintf(stderr,getmsg(MF_PIOSPLP,1,BADARG_ONOFF));
                exit(3);
                break;

            case 2:
            fprintf(stderr,getmsg(MF_PIOSPLP,1,BAD_TIMEOUT));
                exit(3);
                break;

            case 3:
                fprintf(stderr,getmsg(MF_PIOSPLP,1,HELPMSG));
                exit(1);
                break;

            case 4:
                fprintf(stderr,getmsg(MF_PIOSPLP,1,FONT_PLUS));
                exit(1);
                break;
            }                                                    /* endswitch */
        }                                                      /* if (badarg) */
    }                                                      /* if (!(noflags)) */



if (noflags)
    {
    prstat();
    exit(0);
    }


if (!font_init_only)
    {
    if (ioctl(fd,LPRSET,&plpst) < 0)                /* Set new printer values */
        {
        fprintf(stderr,getmsg(MF_PIOSPLP,1,CANTSETSTAT),errno);
        exit(3);
        }

    if (ioctl(fd,LPRMODS,&plpmod) < 0)
        {
        fprintf(stderr,getmsg(MF_PIOSPLP,1,CANTSETSTAT),errno);
        exit(3);
        }

    if (ioctl(fd,LPRSTOV,&plptim) < 0)
        {
        fprintf(stderr,getmsg(MF_PIOSPLP,1,CANTSETSTAT),errno);
        exit(3);
        }
    }

if (font_init)
    {
    strcpy(flagsdir,DEFFLAGSDIR);
    strcat(flagsdir,strrchr(dev,'/'));
    unlink(flagsdir);
    }
}                                                                 /* end main */

/*******************************************************************************
*
* NAME:           prstat
*
* DESCRIPTION:    Outputs the current values to stdout
*
* PARAMETERS:     (none)
*
* RETURN VALUE:   0
*
*******************************************************************************/

prstat()                                            /* display printer values */
    {
    register int m = plpmod.modes;
    register int r = spio.c_cflag;

    char *point = (char *)getmsg(MF_PIOSPLP,1,SPLP_PARMS);

    char *hdg[4];
    char *msg[22];
    int baud, bits, p;

    hdg[0] = point;
    for (p = 1 ; p < 4 ;)
        {
        if (*point == '|')
            {
            *point++ = '\0';
            hdg[p++] = point;
            }
        else *point++;
        }

    p = 0;
    msg[0] = point;
    while(*point)
        {
        if (*point == '|')
            {
            *point++ = '\0';
            msg[p++] = point;
            }
        else *point++;
        }

    fprintf(stdout,"\n%s = %s      (+ %s     ! %s)\n",msg[2],dev,msg[0],msg[1]);
    fprintf(stdout,"%s\n%s\n",hdg[0],hdg[1]);
    fprintf(stdout,"-p %c    %-30.30s  -c %c    %-30.30s\n",
        (m & PLOT)?'+':'!',msg[3],(m & NOCR)?'!':'+',msg[4]);
    fprintf(stdout,"-l %-2d   %-30.30s  -n %c    %-30.30s\n",
        plpst.line,msg[5],(m & NONL)?'!':'+',msg[6]);
    fprintf(stdout,"-w %-3d  %-30.30s  -r %c    %-30.30s\n",
        plpst.col,msg[7],(m & NOCL)?'!':'+',msg[8]);
    fprintf(stdout,"-i %-2d   %-30.30s  -t %c    %-30.30s\n",
        plpst.ind,msg[9],(m & NOTB)?'+':'!',msg[10]);
    fprintf(stdout,"-W %c    %-30.30s  -b %c    %-30.30s\n",
        (m & WRAP)?'+':'!',msg[11],(m & NOBS)?'!':'+',msg[12]);
    fprintf(stdout,"-C %c    %-30.30s  -f %c    %-30.30s\n",
        (m & CAPS)?'+':'!',msg[13],(m & NOFF)?'!':'+',msg[14]);

    fprintf(stdout,"%s\n",hdg[2]);
    fprintf(stdout,"-T %-3d  %-30.30s  -e %c    %-30.30s\n",
        plptim.v_timout,msg[15],(m & RPTERR)?'+':'!',msg[16]);


    if (serial)
        {
        fprintf(stdout,"%s\n",hdg[3]);
        switch (r & CBAUD)
            {
            case    B50: baud =    50 ; break;
            case    B75: baud =    75 ; break;
            case   B110: baud =   110 ; break;
            case   B134: baud =   134 ; break;
            case   B150: baud =   150 ; break;
            case   B300: baud =   300 ; break;
            case   B600: baud =   600 ; break;
            case  B1200: baud =  1200 ; break;
            case  B1800: baud =  1800 ; break;
            case  B2400: baud =  2400 ; break;
            case  B4800: baud =  4800 ; break;
            case  B9600: baud =  9600 ; break;
            case B19200: baud = 19200 ; break;
            case B38400: baud = 38400 ; break;
            default:     baud =     0 ; break;
            }

        switch (r & CSIZE)
            {
            case CS5: bits = 5 ; break;
            case CS6: bits = 6 ; break;
            case CS7: bits = 7 ; break;
            case CS8: bits = 8 ; break;
            default:  bits = 0 ; break;
            }

        fprintf(stdout,"   %-5d%-30.30s     %d    %-30.30s\n",
            baud,msg[17],bits,msg[18]);
        fprintf(stdout,"   %c    %-30.30s     %c    %-30.30s\n",
            (r & PARENB)?'+':'!',msg[19],(r & CSTOPB)?'+':'!',msg[20]);
        fprintf(stdout,"   %c    %-30.30s\n",
            (r & PARODD)?'+':'!',msg[21]);
        }                                                        /* if serial */
    }                                                             /* prstat() */

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
		sprintf(defpath,"/usr/lib/lpd/pio/etc/%s",CatName);
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
