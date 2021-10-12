/* @(#)16       1.23  src/bos/usr/lib/pios/virprt.h, cmdpios, bos411, 9428A410j 9/22/93 17:51:21 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** virprt.h ***/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <termio.h>
#include <fcntl.h>
#include <sys/limits.h>

extern int  errno;
extern char *strchr(),
            *strstr(),
            *getmsg(),
            *get_attr_msg(),
            *getval();
extern void readkbd(),
            proceed(),
            goodbye(),
            make_files(),
            interactive(),
            non_interactive();

struct pn_struct *new_pn(),
                *found_pn(),
                *make_list();
struct sn_struct *new_sn();

struct attr {   char *attribute;
        char *value;
        char flag; };

struct attr *att;          /* Attribute-value pairs from command line */

struct sn_struct                      /* secondary values */
       { char *sn;               /* (data stream or queue device) */
     struct sn_struct *sn_next; };         /* ptr to next secondary value */

struct pn_struct                        /* primary values */
       { char *pn;                 /* (printer type or queue) */
     struct sn_struct *sn_head;       /* ptr to first secondary value */
     struct pn_struct *pn_next; };       /* ptr to next primary value */
struct pn_struct *prt_head;

struct termio orig, alterd;             /* orig & modified tty status */

char    *dstype,                        /* Data stream type */
        *dname,                         /* Device name */
        *fname,                         /* file =      */
        *pqname,                        /* Queue name */
        *ptype,                         /* Printer type */
        *vpname,                        /* Queue device name */
        *custom_name,                   /* qname : qdevice name */
        *codeset,                       /* contains default code set */
        tempfile[50],                   /* temporary file name */
        *wr_ptr,                        /* generic pointer for writes */
		*attach_t,						/* attachment type */
#define MAXPRINTERS 1024                                                              /* D45330 */ /* D78838 increased from 500 to 1024 */
        *printers[MAXPRINTERS],         /* ptrs to strings showing printers */       /* D45330 - increased *//* D78838 increased from 500 to 1024 */
        *scr_line[MAXPRINTERS],         /* ptrs to lines displayed on the screen */  /* D45330 - increased */ /* D78838 increased from 500 to 1024 */
        prepath[100],                   /* path name to predefined directory */
        cuspath[100],                   /* path name to custom directory */
        etcpath[100],                   /* path name to etc directory */
        digpath[100],                   /* path name to ddi directory */
		odmpath[PATH_MAX+1],			/* path name to alternate ODM directory */
        digestcmd[100],                 /* path name to digest command */
        backend[200],                   /* path name to printer backend */
        prefile[200],                   /* path name to predefined file */
        cusfile[200],                   /* path name to customized file */
        str[500],                       /* common string; msgs, etc */
        cmd[1000];                      /* comman command string */

int     rc,                             /* return code */
        pr_nums = 0,                    /* number of virtual printers */
        scrn_ht = 0,            /* number of screen lines returned by curses */
        errflag = FALSE;                /* error flag from err_sub*/

#define NAMESIZE        200

#define PIOBASEDIR	"/usr/lib/lpd/pio"
#define PIOVARDIR	"/var/spool/lpd/pio/@local"

#define PIOCNVT         "/usr/sbin/piocnvt"
#define MKVIRPRT        "/usr/sbin/mkvirprt"
#define LSVIRPRT        "/usr/sbin/lsvirprt"
#define CHVIRPRT        "/usr/sbin/chvirprt"
#define RMVIRPRT        "/usr/sbin/rmvirprt"
#define PIOATTRED       "/usr/sbin/pioattred"
#define LSQUECMD        "/usr/bin/lsque"
#define LSQUEDEVCMD     "/usr/bin/lsquedev"
#define LSDEVCMD        "/usr/sbin/lsdev"
#define MKQUECMD        "/usr/bin/mkque"
#define MKQUEDEVCMD     "/usr/bin/mkquedev"
#define RMQUE           "/usr/bin/rmque"
#define RMQUEDEV        "/usr/bin/rmquedev"
#define CHQUEDEV        "/usr/bin/chquedev"
#define PIOUPDATE	"/usr/lib/lpd/pio/etc/pioupdate"

#define  CP		"/usr/bin/cp"
#define	 AWK		"/usr/bin/awk"
#define	 GREP		"/usr/bin/grep"
#define	 FGREP		"/usr/bin/fgrep"
#define	 LS		"/usr/bin/ls"
#define	 LI		"/usr/bin/li"
#define	 TEST		"/usr/bin/test"
#define	 SORT		"/usr/bin/sort"
#define	 MV		"/usr/bin/mv"
#define	 RM		"/usr/bin/rm"
#define	 PG		"/usr/bin/pg"
#define  ECHOCMD	"/usr/bin/echo"
#define  VI		"/usr/bin/vi"

#define CREATETEMP      file = fopen(tempfile,"w")
#define OPENTEMP        file = fopen(tempfile,"r")
#define CLOSETEMP       fclose(file)
#define KILLTEMP        unlink(tempfile)
#define DVNL            "/dev/null"
#define WASTE           ">/dev/null 2>&1"

#define CAT_CMMT        "if [ -f %s ] ; \
                        then /usr/bin/echo \"\\n#\\n#  %s\\n#\" >>%s ; \
                        fi"

#define CAT_CMD         "if [ -f %s ] ; \
                        then /usr/bin/echo \"%s\" >>%s ; \
                        fi"

#define  CONT           0            /* return to caller on error */
#define  ABORT          1              /* exit on error condition */

#define  GOOD           0                  /* exit code - success */
#define  BAD            1                  /* exit code - failure */
#define  TERMINATE      2               /* exit code - terminated */

#define  GETNUM         1     /* tells resp_menu what kind of response to */
#define  GETTEXT        2    /* expect: 1; line number  2; arbitrary text */

#define  DZNTXST        1            /* colon file does not exist */
#define  PERM_OK        2   /* colon file exists and has good permissions */
#define  PERM_BAD       3   /* colon file exists, bus has bad permissions */

#define  CLR_SCR        0                     /* clear screen */
#define  CLR_EOL        1                 /* clear to end of line */
#define  STANDOUT       2                    /* standout mode */
#define  NORMAL         3                      /* normal mode */
#define  PUT_CUR        4                /* put cursor at x,y */

#define  SAVE_TTY       ioctl(0,TCGETA,&orig)
#define  CHANGE_TTY     ioctl(0,TCSETAW,&alterd)
#define  RESTORE_TTY    ioctl(0,TCSETAW,&orig)

#define  WRITE(x,y)     wr_ptr = y;  \
                        do { if (x==2) rc = write(x,wr_ptr,strlen(wr_ptr));  \
			     else { putp(wr_ptr); rc = strlen(wr_ptr);       \
			            fflush(stdout); }                        \
                        while ( rc-- ) wr_ptr++; } while ( *wr_ptr )

#define  NULN           write(1,"\n",1)
#define  PUT(x,y)       strcpy(cmd,tparm(screen[PUT_CUR],x,y)); WRITE(1,cmd)

#define  BREAK          0                    /* break key */
#define  QUIT           1                    /* quit key */
#define  ENTER          2                    /* enter key */
#define  KEYBS          3                    /* backspace */
#define  KEYUP          4                    /* up arrow */
#define  KEYUP_ALT      5               /* alternate up arrow '-' */
#define  KEYDN          6                   /* down arrow */
#define  KEYDN_ALT      7               /* alternate down arrow '+' */

#define  ISATTY         if ( !isatty(0) )  \
                        {  \
                        pqname = getenv("PIOSMIT");  \
                        if ( *pqname == '1' ) goodbye(GOOD);  \
                        else err_sub(ABORT,TTY);  \
                        }

#define  INIT(x)        x = malloc(NAMESIZE); *x = '\0'

#include <piobe_msg.h>

#define DEFMSG "0782-000 Cannot access message catalog file piobe.cat.\n\
\t Use local problem reporting procedures.\n"
#define  ERRSETNUM      2
#define  ATTSETNUM      1

#define  TEXT(str,num) strcpy(str,getmsg(MF_PIOBE,ERRSETNUM,num,DEFMSG))

#define  NO_MSG_CAT     "Cannot access attribute description catalog."

#define ETCPATH "/usr/lib/lpd/pio/etc/"
#define NUMFIELDS 10                 /* # fields on line in *.config file */
#define HOSTATTACH 0
#define NOTHOSTATTACH 1

#define  CNVTFLAGS      int c, newflags = 1;  \
            for (c = 1; c < argc; c++)  \
              if (*argv[c] == '-' && *(argv[c] + 1) == 'v')  \
                  { newflags = 0; break; }  \
            if (newflags)  \
              for (c = 1; c < argc; c++)  \
                if (*argv[c] == '-' && *(argv[c]+1) == 'd')  \
                        *(argv[c] + 1) = 'v';  \
                else if (*argv[c] == '-' && *(argv[c]+1) == 's')  \
                        *(argv[c] + 1) = 'd';


/* Define macros for non-modifiable attributes */
#define ATTR_DEF_STATE	"zD"
#define ATTR_CUR_STATE	"zS"
#define PROHIBIT_ATTRLIST_DEFN	char *prohibit_attrs[] = \
			{ \
			   ATTR_DEF_STATE, \
			   ATTR_CUR_STATE, \
			   NULL \
			}

#define DEFMC_PREFIXPATH        "/usr/lib/lpd/pio/etc/"
#define MF_PIOATTR1SETNO	(1)
#define SETBEGINCHR		'['
#define SETENDCHR		']'
#define MSGSEPCHR		';'
#define FLDSEPCHR		','
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#define MALLOC(p,sz)		do \
				{ \
				   if (!((p) = malloc((size_t)(sz)))) \
				      err_sub(ABORT,MSG_MALLOC); \
				} while (0)
#define MAXARGLEN		(1006)  /* 1000+2+3+1 (-aci=...\0) */

