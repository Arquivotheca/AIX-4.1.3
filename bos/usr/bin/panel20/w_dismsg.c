#ifdef COMPILE_SCCSID
static char sccsid[] = "@(#)00  1.9  src/bos/usr/bin/panel20/w_dismsg.c, cmdhia, bos411, 9428A410j 11/24/93 12:29:05";
#endif

/*
 * COMPONENT_NAME: (CMDHIA) Messages retrieve/display routines
 *
 * FUNCTIONS: w_init_mess, w_end_mess, w_dismsg, w_disprmt, w_curprmt,
 *            w_dishlp, w_bldpgq, w_iq, w_delpgq, w_tput, w_setvar,
 *            dis_msg, read_timeout, w_newhlp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
/*#include <sys/hft.h> EMR LFT support */
#include <sys/devinfo.h>
#include <termio.h>
#include <locale.h>
#include <cur00.h>
#include "w_dismsg.h"
#include "w_msg.h"

#define OPENFLAGS       (O_WRONLY | O_CREAT | O_APPEND)
#define BUF_SZ          8

extern int mask;

static struct pg *page_ptr;
static int firsttime = 1;
static char outfile[100];
static int out_fd;
static struct termio t;
static char s_clear[32];                /* string to clear the screen */
static char s_kcuu1[8];         /* string returned by up arrow key */
static char s_kcud1[8];         /* string returned by down arrow key */

struct pg {
        struct pg *pg_next;
        struct pg *pg_prev;
        char      *pg_data;
        int        pg_len;
};

#ifdef DEBUGIT
        FILE *fp;
        extern char dbg_str[];
#define dbg_write printf
#endif
char dmsg[200];
char *rmsgptr;
char rmsg[NL_TEXTMAX];
int w_msgfd = -1;               /* for writting message to some other file */
                                /* ... this is set by the calling program */
char *getenv();
/* char *malloc(); */

w_init_mess()
{
  setlocale(LC_ALL, "");

  catd = catopen("panel20.cat",NL_CAT_LOCALE);
  return;

/* change to release 2 catopen
  if ((catd = catopen("panel20.cat",NL_CAT_LOCALE)) == CATD_ERR)
  {
    printf("Cannot open Panel20's message catalog.\n");
    printf("Make sure that the file panel20.cat exists\n");
    printf("in at least one of the directories specified\n");
    printf("by the NLSPATH and LANG environment variables.\n");
    return;
  }
*/
}

w_end_mess()
{
        catclose(catd);
        return;

/* change to release 2 catclose
        if ((catclose(catd)) == -1)
        {
                printf("Cannot close Panel20's message catalog.\n");
                return;
        }
*/
}

w_dismsg(setnum, msgnum, msgarg1)
int setnum;
int msgnum;
int msgarg1;
{
        register char *q;
        int  ttyflag;

#ifdef DEBUGIT
        dbg_write("w_dismsg: begin\n");
#endif

        if (firsttime) {
                firsttime = 0;
                ttyflag = isatty(2);
        }

        w_setvar(setnum, msgnum, &msgarg1);

/* EMR LFT support
        if (strcmp(regmsgnum, "106")) {
#ifdef _POWER
                if (!ttyflag || (ttyflag && ioctl(2, HFTGETID, 0) == 2)) {
#else
                if (!ttyflag || (ttyflag && ioctl(2, HFGCHAN, 0) == 2)) {
#endif

                        if (q = getenv("HOME")) {
                                strcpy(outfile, q);
                                strcat(outfile, "/panel20errors");
                                out_fd = open(outfile, OPENFLAGS, 0644);
                        }
                        else
                                out_fd = -1;
                }
                else
                        out_fd = 1;
        }
        else
EMR LFT support*/
                out_fd = 1;

        sprintf(dmsg, "Cannot retrieve message 0790-%s.\n",regmsgnum);

        if (validmsg)
        {
                rmsgptr = catgets( catd, setnum, msgnum, dmsg );

                switch(count) {
                        case 0:
                                sprintf(rmsg,rmsgptr);
                                break;
                        case 1:
                                sprintf(rmsg,rmsgptr,messargs[0]);
                                break;
                        case 2:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1]);
                                break;
                        case 3:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2]);
                                break;
                        case 4:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3]);
                                break;
                        case 5:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4]);
                                break;
                        case 6:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5]);
                                break;
                        case 7:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5],messargs[6]);
                                break;
                        case 8:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5],messargs[6],messargs[7]);
                                break;
                } /* End of switch */
        }
        else
        {
                rmsgptr = catgets( catd, 1, 50, dmsg );
                sprintf(rmsg, rmsgptr, messargs[0], messargs[1]);
        }

        if (w_msgfd == -1) {    /* if to write to standard place */
                write(out_fd, rmsg, strlen(rmsg));
        }
        else {                  /* write where caller directs */
                write(w_msgfd, rmsg, strlen(rmsg));
        }
#ifdef DEBUGIT
        dbg_write("w_dismsg: end\n");
#endif
        return;
}


w_disprmt(setnum, msgnum, msgarg1)
int setnum;
int msgnum;
int msgarg1;
{
        w_setvar(setnum, msgnum, &msgarg1);
        sprintf(dmsg, "Cannot retrieve message 0790-%s.\n",regmsgnum);

        if (validmsg)
        {
                rmsgptr = catgets( catd, setnum, msgnum, dmsg );
                switch(count) {
                        case 0:
                                sprintf(rmsg,rmsgptr);
                                break;
                        case 1:
                                sprintf(rmsg,rmsgptr,messargs[0]);
                                break;
                        case 2:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1]);
                                break;
                        case 3:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2]);
                                break;
                        case 4:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3]);
                                break;
                        case 5:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4]);
                                break;
                        case 6:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5]);
                                break;
                        case 7:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5],messargs[6]);
                                break;
                        case 8:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5],messargs[6],messargs[7]);
                                break;
                } /* End of switch */
        }
        else
        {
                rmsgptr = catgets( catd, 1, 50, dmsg );
                sprintf(rmsg, rmsgptr, messargs[0], messargs[1]);
        }

        if (w_msgfd == -1) {    /* if to write to standard place */
                fprintf(stderr, "%s", rmsg);
        }
        else {
                write(w_msgfd, rmsg, strlen(rmsg));
        }
        return;
}

char * w_curprmt(buf, setnum, msgnum, msgarg1)
char *buf;
int setnum;
int msgnum;
int msgarg1;
{
        /* Return a pointer to the message to be printed.
         * Assume buf is big enough to hold the buffer.
         * Return NULL if error
         */

#ifdef DEBUGIT
        sprintf(dbg_str, "w_curprmt(%d,%d): begin\n", setnum, msgnum);
        dbg_write(dbg_str);
#endif
        w_setvar(setnum, msgnum, &msgarg1);

        if (validmsg)
        {
                switch(count) {
                        case 0:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ));
                                break;
                        case 1:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0] );
                                break;
                        case 2:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1]);
                                break;
                        case 3:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2] );
                                break;
                        case 4:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3] );
                                break;
                        case 5:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3],
                                         messargs[4] );
                                break;
                        case 6:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3],
                                         messargs[4],messargs[5] );
                                break;
                        case 7:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3], 
                                         messargs[4],messargs[5],messargs[6] );
                                break;
                        case 8:
                                sprintf( buf,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3],
                                         messargs[4],messargs[5],messargs[6],messargs[7] );
                                break;
                } /* End of switch */
        }
        else
                sprintf( buf, catgets( catd, 1, 50, def_msg[ 50 - 1 ] ),
                         messargs[0], messargs[1]);

        if (strcmp(buf, "") == 0)
                return(NULL);

#ifdef DEBUGIT
        dbg_write("w_curprmt: end\n");
#endif
        return(buf);
}

w_dishlp(setnum, msgnum, msgarg1)
int     setnum;
int     msgnum;
int     msgarg1;
{
        int num_read;                   /* count of characters read so far */
        int rc;                                 /* return value of read() */
        int esc_len;                    /* length to control esc. sequences */
        int timeout;                    /* read timeout value */
        register struct pg *pe;
        unsigned char   sav_VMIN;
        unsigned char   sav_VTIME;
        unsigned short  sav_c_lflag;
        char    buf[BUF_SZ], *p;
#ifdef DEBUGIT
        dbg_write("w_dishlp: begin\n");
#endif

        w_tput();               /* get strings from terminfo database */

        /* assume the cursor up and cursor down esc. sequences          */
        /* are the same len                                             */
        esc_len = strlen(s_kcuu1);

        /* Get and save tty settings for stdin */
        ioctl(0, TCGETA, &t);
        sav_VMIN = t.c_cc[VMIN];
        sav_VTIME = t.c_cc[VTIME];
        sav_c_lflag = t.c_lflag;

        /* Set stdin for no input processing and no echo */
        t.c_lflag &= ~(ICANON | ECHO);
        ioctl(0, TCSETA, &t);

        /* Get read timeout to be used when reading cursor key esc sequences */
        timeout = read_timeout(&t);
#ifdef DEBUGIT
        sprintf(dbg_str,"timeout=%d\n",timeout);
        dbg_write(dbg_str);
#endif

        /* Get message number */
        w_setvar(setnum, msgnum, &msgarg1);

        /* Get message and page according to keyboard input */
        sprintf(dmsg, "Cannot retrieve message 0790-%s.\n",regmsgnum);
        if (validmsg)
        {
                rmsgptr = catgets( catd, setnum, msgnum, dmsg );
                switch(count) {
                        case 0:
                                sprintf(rmsg,rmsgptr);
                                break;
                        case 1:
                                sprintf(rmsg,rmsgptr,messargs[0]);
                                break;
                        case 2:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1]);
                                break;
                        case 3:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2]);
                                break;
                        case 4:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3]);
                                break;
                        case 5:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4]);
                                break;
                        case 6:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5]);
                                break;
                        case 7:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5],messargs[6]);
                                break;
                        case 8:
                                sprintf(rmsg,rmsgptr,messargs[0],messargs[1],
                                        messargs[2],messargs[3],messargs[4],
                                        messargs[5],messargs[6],messargs[7]);
                                break;
                } /* End of switch */
        }
        else
        {
                rmsgptr = catgets( catd, 1, 50, dmsg );
                sprintf(rmsg, rmsgptr, messargs[0], messargs[1]);
        }

        if (w_bldpgq(rmsg)) {
                pe = page_ptr;
                for (;;) {
                        /* clear display */
                        write(2, s_clear, strlen(s_clear));
                        write(2, pe->pg_data, pe->pg_len);

                        /* set stdin for blocking read (no timeout) */
                        t.c_cc[VMIN] = (char)1;
                        t.c_cc[VTIME] = (char)0;
                        ioctl(0, TCSETA, &t);

                        /* initialize output buffer pointer */
                        p = buf;
                        memset(buf, NULL, BUF_SZ);

                        /* Get input (blocked);             */
                        /* we wait here for kbd input       */
                        if ((rc = read(0, p, esc_len)) < 0) {
                                exit(1);
                        }
                        num_read = rc;
                        p += num_read;

                        /* set timeout for (possible) read  */
                        /* of next character                */
                        t.c_cc[VMIN] = (char)0;
                        t.c_cc[VTIME] = (char)timeout;
                        ioctl(0, TCSETA, &t);

                        /* read (with timeout) rest of esc sequence */
                        while ((rc = read(0,p,esc_len-num_read)) != 0){
                                if (rc < 0) {           /* error */
                                        exit(1);
                                }
                                num_read += rc;
                                p = buf + num_read;
                                if (num_read == esc_len) {
                                        break;
                                }
                        }
#ifdef DEBUGIT
        sprintf(dbg_str,"buf=0x%x 0x%x 0x%x\n", buf[0], buf[1], buf[2]);
        dbg_write(dbg_str);
#endif
                        /* if down arrow */
                        if (strncmp(s_kcud1,buf,strlen(s_kcud1)) == 0){
                                pe = pe->pg_next;
                        }
                        /* else if up arrow */
                        else if (strncmp(s_kcuu1,buf,strlen(s_kcuu1)) == 0){
                                pe = pe->pg_prev;
                        }
                        else {
                                break;
                        }
                }
        }
        write(2, "\n", 1);      /* put cursor at col 1 */
        w_delpgq();

        /* restore org. tty settings on stdin */
        t.c_cc[VMIN] = sav_VMIN;
        t.c_cc[VTIME] = sav_VTIME;
        t.c_lflag = sav_c_lflag;
        ioctl(0, TCSETA, &t);
#ifdef DEBUGIT
        dbg_write("w_dishlp: end\n");
#endif
        return;
}

w_bldpgq(text)                  /* build circular queue of page info */
        char    *text;
{
        extern char *strchr();
        register char *np;                      /* pointer to next page */
        register char *pp;                      /* pointer to previous page */
        np = pp = text;
        while (np = strchr(np, '@')) {          /* look for page delimiter */
                *np++ = 0;                      /* replace '@' by string   */
                                                /* terminator              */
                if (!w_iq(pp)) {                /* insert page into queue  */
                        return(0);
                }
                pp = ++np;                      /* skip newline after page */
                                                /* delimiter               */
        }
        if (!w_iq(pp)) {                        /* insert page into queue */
                return(0);
        }
        return(1);
}

w_iq(p)
        char *p;
{
        register struct pg *pe;                 /* pointer to page queue  */
                                                /* element                */

        if ((pe = (struct pg *)malloc(sizeof(struct pg))) == NULL) {
                return(0);
        }
        pe->pg_data = p;                        /* save pointer to page data */
        pe->pg_len = strlen(p) - 1;             /* ... and its length     */
                                /* ... subtract 1 to get rid of final '\n' */
        if (page_ptr == (struct pg *)0) {       /* if queue empty */
                page_ptr = pe;
                pe->pg_next = pe;
                pe->pg_prev = pe;
        }
        else {
                pe->pg_next = page_ptr;
                pe->pg_prev = page_ptr->pg_prev;
                page_ptr->pg_prev->pg_next = pe;
                page_ptr->pg_prev = pe;
        }
        return(1);
}

w_delpgq()
{
        register struct pg *pe;

        if (page_ptr) {                 /* if something in queue */
                page_ptr->pg_prev->pg_next = 0; /* terminate forward chain */
                do {
                        pe = page_ptr->pg_next; /* get next in forward chain */
                        free(page_ptr);
                } while (page_ptr = pe);        /* until no more in chain */
        }
        return;
}

w_tput()
{
        FILE    *fp;

        if ((fp = popen("tput clear", "r")) != NULL) {
                fgets(s_clear, (int)sizeof(s_clear), fp);
                pclose(fp);
        }
        if ((fp = popen("tput kcuu1", "r")) != NULL) {
                fgets(s_kcuu1, (int)sizeof(s_kcuu1), fp);
                pclose(fp);
        }
        if ((fp = popen("tput kcud1", "r")) != NULL) {
                fgets(s_kcud1, (int)sizeof(s_kcud1), fp);
                pclose(fp);
        }
}

w_setvar(snum, mnum, marg1)
int snum;
int mnum;
int *marg1;
{
        register int *p;
        FILE *fp;
        int i = 0;

#ifdef DEBUGIT
fp = fopen("debugit","a");
fprintf(fp,"w_setvar\n");
fclose(fp);
#endif
        initialize(snum, mnum);
        if (validmsg)
        {
                p = marg1;
                if (mask & W_MSGVI1)
                        messargs[i++] = (int *)*p++;
                if (mask & W_MSGVI2)
                        messargs[i++] = (int *)*p++;
                if (mask & W_MSGVL1)
                        messargs[i++] = (int *)*p++;
                if (mask & W_MSGVL2)
                        messargs[i++] = (int *)*p++;
                if (mask & W_MSGVC1)
                        messargs[i++] = (int *)*p++;
                if (mask & W_MSGVC2)
                        messargs[i++] = (int *)*p++;
                if (mask & W_MSGVC3)
                        messargs[i++] = (int *)*p++;
                if (mask & W_MSGVCH1)
                        messargs[i++] = (int *)*p++;
        }
        else
        {
                messargs[0] = (int *)snum;
                messargs[1] = (int *)mnum;
        }
#ifdef DEBUGIT
fp = fopen("debugit","a");
fprintf(fp,"w_setvar: snum = %d mnum = %d\n", snum, mnum);
fclose(fp);
#endif
        return;
}

dis_msg(setnum, msgnum, msgarg1)
int     setnum;
int     msgnum;
int     msgarg1;
{
        w_setvar(setnum, msgnum, &msgarg1);
        sprintf(dmsg, "Cannot retrieve message 0790-%s.\n",regmsgnum);
        if (validmsg)
        {
                switch(count) {
                        case 0:
                                printf( catgets( catd, setnum, msgnum, dmsg ));
                                break;
                        case 1:
                                printf( catgets( catd, setnum, msgnum, dmsg ),
                                       messargs[0]);
                                break;
                        case 2:
                                printf(catgets( catd, setnum, msgnum, dmsg),
                                       messargs[0], messargs[1]);
                                break;
                        case 3:
                                printf(catgets(catd, setnum, msgnum, dmsg),
                                       messargs[0], messargs[1], messargs[2]);
                                break;
                        case 4:
                                printf(catgets(catd, setnum, msgnum, dmsg),
                                       messargs[0], messargs[1], messargs[2],
                                       messargs[3]);
                                break;
                        case 5:
                                printf(catgets(catd, setnum, msgnum, dmsg),
                                       messargs[0], messargs[1], messargs[2],
                                       messargs[3], messargs[4]);
                                break;
                        case 6:
                                printf(catgets(catd, setnum, msgnum, dmsg),
                                       messargs[0], messargs[1], messargs[2],
                                       messargs[3], messargs[4], messargs[5]);
                                break;
                        case 7:
                                printf(catgets(catd, setnum, msgnum, dmsg),
                                       messargs[0], messargs[1], messargs[2],
                                       messargs[3], messargs[4], messargs[5],
                                       messargs[6]);
                                break;
                        case 8:
                                printf(catgets(catd, setnum, msgnum, dmsg),
                                       messargs[0], messargs[1], messargs[2],
                                       messargs[3], messargs[4], messargs[5],
                                       messargs[6], messargs[7]);
                                break;
                } /* End of switch */
        }
        else
                printf(catgets(catd, 1, 50, dmsg),
                       messargs[0], messargs[1]);

        return;
}


/****************************************************************************/
/**          read_timeout                                                   */
/**          returns how long to block on read based on baud rate           */
/****************************************************************************/
read_timeout(t)
        struct termio *t;
{
        unsigned char remote;   /* scale timeout if remote */

        if ( ioctl( 0, IOCTYPE, 0 ) != ( DD_PSEU << 8 ) ){ /* is it a pty ?  */
#ifdef DEBUGIT
        sprintf(dbg_str,"local\n");
        dbg_write(dbg_str);
#endif
                remote = FALSE;
        }
        else {
#ifdef DEBUGIT
        sprintf(dbg_str,"remote\n");
        dbg_write(dbg_str);
#endif
                remote = TRUE;
        }

        switch(t->c_cflag & 0x0f){   /* mask off low nibble */
                case B300:                  /* 300 baud */
                        return(12);         /* set 1.2 seconds */
                case B1200:                  /* 1200 baud */
                        return(8);           /* set 800 msec */
                case B2400:                  /* 2400 baud */
                        return(6);           /* set 600 msec */
                case B4800:                  /* 4800 baud */
                        return(4);           /* set 200 msec */
                case B9600:                  /* 9600 baud */
                        if (remote)
                                return(4);   /* set 300 msec */
                        else
                                return(2);   /* set 200 msec */
                default:
                        if (remote)
                                return(4);   /* set 300 msec */
                        else
                                return(2);   /* set 100 msec */
        }
}

/*
 *
 * This message routine is set up to handle a maximum of
 * five message screens.  So if you plan on having more
 * than five message screens increase the case statements
 * using the same procedure as the one before it.
 *
 */
w_newhlp(setnum,msgnum,no_screens)
int setnum;
int msgnum;
int no_screens;
{
        int num_read;                   /* count of characters read so far */
        int rc;                                 /* return value of read() */
        int esc_len;                    /* length to control esc. sequences */
        int timeout;                    /* read timeout value */
        int help_scr=1;                 /* read timeout value */
        unsigned char   sav_VMIN;
        unsigned char   sav_VTIME;
        unsigned short  sav_c_lflag;
        char    buf[BUF_SZ], *p;

        w_tput();               /* get strings from terminfo database */
        /* assume the cursor up and cursor down esc. sequences          */
        /* are the same len                                             */
        esc_len = strlen(s_kcuu1);
        /* Get and save tty settings for stdin */
        ioctl(0, TCGETA, &t);
        sav_VMIN = t.c_cc[VMIN];
        sav_VTIME = t.c_cc[VTIME];
        sav_c_lflag = t.c_lflag;
        /* Set stdin for no input processing and no echo */
        t.c_lflag &= ~(ICANON | ECHO);
        ioctl(0, TCSETA, &t);
        /* Get read timeout to be used when reading cursor key esc sequences */
        timeout = read_timeout(&t);
        /* clear display */
        write(2,s_clear,strlen(s_clear));
        dis_msg(setnum,msgnum);
        for (;;) {
                /* set stdin for blocking read (no timeout) */
                t.c_cc[VMIN] = (char) 1;
                t.c_cc[VTIME] = (char) 0;
                ioctl(0,TCSETA,&t);
                /* initialize output buffer pointer */
                p = buf;
                memset(buf,NULL,BUF_SZ);
                /* Get input (blocked);             */
                /* we wait here for kbd input       */
                if ((rc = read(0,p,esc_len)) < 0) {
                        exit(1);
                }
                num_read = rc;
                p += num_read;
                /* set timeout for (possible) read  */
                /* of next character                */
                t.c_cc[VMIN] = (char) 0;
                t.c_cc[VTIME] = (char) timeout;
                ioctl(0,TCSETA,&t);
                /* read (with timeout) rest of esc sequence */
                while ((rc = read(0,p,esc_len-num_read)) != 0){
                        if (rc < 0) {           /* error */
                                exit(1);
                        }
                        num_read += rc;
                        p = buf + num_read;
                        if (num_read == esc_len) {
                                break;
                        }
                }
                /* if down arrow */
                if (strncmp(s_kcud1,buf,strlen(s_kcud1)) == 0) {
                        write(2,s_clear,strlen(s_clear));
                        switch(help_scr) {
                                case 1:
                                        if (no_screens == 1) {
                                                dis_msg(setnum,msgnum);
                                                help_scr = 1;
                                                break;
                                        }
                                        dis_msg(setnum,msgnum+1);
                                        help_scr = 2;
                                        break;
                                case 2:
                                        if (no_screens == 2) {
                                                dis_msg(setnum,msgnum);
                                                help_scr = 1;
                                                break;
                                        }
                                        dis_msg(setnum,msgnum+2);
                                        help_scr = 3;
                                        break;
                                case 3:
                                        if (no_screens == 3) {
                                                dis_msg(setnum,msgnum);
                                                help_scr = 1;
                                                break;
                                        }
                                        dis_msg(setnum,msgnum+3);
                                        help_scr = 4;
                                        break;
                                case 4:
                                        if (no_screens == 4) {
                                                dis_msg(setnum,msgnum);
                                                help_scr = 1;
                                                break;
                                        }
                                        dis_msg(setnum,msgnum+4);
                                        help_scr = 5;
                                        break;
                                case 5:
                                        dis_msg(setnum,msgnum);
                                        help_scr = 1;
                                        break;
                        }
                }
                /* else if up arrow */
                else if (strncmp(s_kcuu1,buf,strlen(s_kcuu1)) == 0) {
                        write(2,s_clear,strlen(s_clear));
                        switch(help_scr) {
                                case 1:
                                        dis_msg(setnum,
                                        (msgnum + no_screens - 1));
                                        help_scr = no_screens;
                                        break;
                                case 2:
                                        dis_msg(setnum,msgnum);
                                        help_scr = 1;
                                        break;
                                case 3:
                                        dis_msg(setnum,msgnum+1);
                                        help_scr = 2;
                                        break;
                                case 4:
                                        dis_msg(setnum,msgnum+2);
                                        help_scr = 3;
                                        break;
                                case 5:
                                        dis_msg(setnum,msgnum+3);
                                        help_scr = 4;
                                        break;
                        }
                }
                else {
                        break;
                }
        }
        write(2,"\n",1);        /* put cursor at col 1 */
        w_delpgq();
        /* restore org. tty settings on stdin */
        t.c_cc[VMIN] = sav_VMIN;
        t.c_cc[VTIME] = sav_VTIME;
        t.c_lflag = sav_c_lflag;
        ioctl(0,TCSETA,&t);
        return;
}
