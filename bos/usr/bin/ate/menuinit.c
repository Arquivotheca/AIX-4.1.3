static char sccsid[] = "@(#)16  1.10  src/bos/usr/bin/ate/menuinit.c, cmdate, bos411, 9437A411a 8/26/94 16:04:00";
/* 
 * COMPONENT_NAME: BOS menuinit.c
 * 
 * FUNCTIONS: MSGSTR, menuinit 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * initialization of message strings for menus
 */

#include "menu.h"
#include "modem.h"

#include <nl_types.h>
#include "ate_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ATE,n,s) 

extern struct list *alloc_list();

struct list *mmenu, *altrmenu, *modmenu, *alterdesc, *moddesc; 

char *cline, *dline, *sline, *rline, *bline;
char *xline, *hline, *mline, *aline, *eline, *qline;
char *node, *line, *command, *command2, *enter1;
char *mainh1, *mainh2, *alterh, *modh;
char *off, *on;
char *enter2, *enter3;
char *direntr, *exitstr;

menuinit()
{
    int i;
    char *cptr;

#ifdef DEBUG
    kk = sprintf(ss,"entering menuinit\n");
    write(fe,ss,kk);
#endif

    mmenu = alloc_list(MAIN_LEN);
    altrmenu = alloc_list(ALTER_LEN);
    alterdesc = alloc_list(ALTER_LEN);
    modmenu = alloc_list(MOD_LEN);
    moddesc = alloc_list(MOD_LEN);

    /*
     * connected and unconnected main menus 
     */

    /* titles and headers */

    cptr = MSGSTR(NODE, "\nNode: "); /*MSG*/
    strcpy( node = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(MAINH2, "          CONNECTED MAIN MENU\n"); /*MSG*/
    strcpy( mainh2 = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(MAINH1, "          UNCONNECTED MAIN MENU\n"); /*MSG*/
    strcpy( mainh1 = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(ALTERH, "          ALTER CONNECTION SETTINGS\n"); /*MSG*/
    strcpy( alterh = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(MODH, "          MODIFY LOCAL SETTINGS\n"); /*MSG*/
    strcpy( modh = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(LINE, "-------------------------------------------------------------------------------\n"); /*MSG*/
    strcpy( line = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(CMMD, "     COMMAND      DESCRIPTION\n     -------      -----------------------------------------------\n"); /*MSG*/
    strcpy( command = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(CMMD2, " COMMAND   DESCRIPTION             CURRENT          POSSIBLE CHOICES\n---------  ----------------------  --------  ------------------------------\n"); /*MSG*/
    strcpy( command2 = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(ENTER1, "Type the first letter of the command and press Enter.\n> "); /*MSG*/
    strcpy( enter1 = (char *)malloc(strlen(cptr)+1), cptr);

#ifdef DEBUG
    kk = sprintf(ss,"initializing command names\n");
    write(fe,ss,kk);
#endif

    /* command names */

    cptr = MSGSTR(CONN, "Connect");
    strcpy( mmenu->items[0] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DIR, "Directory");
    strcpy( mmenu->items[1] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(SEND, "Send");
    strcpy( mmenu->items[2] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(RECV, "Receive");
    strcpy( mmenu->items[3] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(BRK, "Break");
    strcpy( mmenu->items[4] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(TERM, "Terminate");
    strcpy( mmenu->items[5] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(HELP, "Help");
    strcpy( mmenu->items[6] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(MOD, "Modify");
    strcpy( mmenu->items[7] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(ALT, "Alter");
    strcpy( mmenu->items[8] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(PERF, "Perform");
    strcpy( mmenu->items[9] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(MQUIT, "Quit");
    strcpy( mmenu->items[10] = (char *)malloc(strlen(cptr)+1), cptr);

    cptr = MSGSTR(LENG, "Length");
    strcpy( altrmenu->items[0] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DLENGTH, "Bits per character      %-8d  7,8\n"); /*MSG*/
    strcpy( alterdesc->items[0] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(STOP, "Stop");
    strcpy( altrmenu->items[1] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DSTOP, "Number of stop bits     %-8d  1,2\n"); /*MSG*/
    strcpy( alterdesc->items[1] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(PARITY, "Parity");
    strcpy( altrmenu->items[2] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DPARITY, "Parity setting          %-8d  0=none, 1=odd, 2=even\n"); /*MSG*/
    strcpy( alterdesc->items[2] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(RATE, "Rate");
    strcpy( altrmenu->items[3] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DRATE, "Number of bits/second   %-8d  50,75,110,134,150,300,600,\n\
					     1200,1800,2400,4800,9600,19200\n\n"); /*MSG*/
    strcpy( alterdesc->items[3] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DEVICE, "Device");
    strcpy( altrmenu->items[4] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DDEVICE, "/dev name of port       %-8.8s  tty0-tty16\n"); /*MSG*/
    strcpy( alterdesc->items[4] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(INITIAL,"Initial");
    strcpy( altrmenu->items[5] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DINITIAL, "Modem dialing prefix    %-8.8s  ATDT, ATDP, etc.\n"); /*MSG*/
    strcpy( alterdesc->items[5] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(FINAL, "Final");
    strcpy( altrmenu->items[6] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DFINAL, "Modem dialing suffix    %-8.8s  0 for none, valid modem suffix\n"); /*MSG*/
    strcpy( alterdesc->items[6] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(WAIT, "Wait");
    strcpy( altrmenu->items[7] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DWAIT, "Wait between redialing  %-8d  seconds between tries\n"); /*MSG*/
    strcpy( alterdesc->items[7] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(ATTEMPTS,"Attempts");
    strcpy( altrmenu->items[8] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DATTEMPTS, "Maximum redial tries    %-8d  0 for none, a positive integer\n\n"); /*MSG*/
    strcpy( alterdesc->items[8] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(TRANSFER,"Transfer");
    strcpy( altrmenu->items[9] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DTRANSFER, "File transfer method    %c         p=pacing, x=xmodem\n"); /*MSG*/
    strcpy( alterdesc->items[9] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(CHARACTER,"Character");
    strcpy( altrmenu->items[10] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DCHARACTER, "Pacing char or number   %c         0 for none, a single char/integer\n"); /*MSG*/
    strcpy( alterdesc->items[10] = (char *)malloc(strlen(cptr)+1), cptr);

    cptr = MSGSTR(NAME, "Name");
    strcpy( modmenu->items[0] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DNAME, "Name the capture file   %-18.18s  Any valid file name\n\n"); /*MSG*/
    strcpy( moddesc->items[0] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(LINEFEEDS, "Linefeeds");
    strcpy( modmenu->items[1] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DLINEFEEDS, "Linefeeds added         %-8s            ON, OFF\n"); /*MSG*/
    strcpy( moddesc->items[1] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(MECHO, "Echo");
    strcpy( modmenu->items[2] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DECHO, "Echo mode               %-8s            ON, OFF\n"); /*MSG*/
    strcpy( moddesc->items[2] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(VT100, "VT100");
    strcpy( modmenu->items[3] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DVT100, "VT100 emulation         %-8s            ON, OFF\n"); /*MSG*/
    strcpy( moddesc->items[3] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(WRITE, "Write");
    strcpy( modmenu->items[4] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR (DWRITE, "Write display data to   %-8s            ON, OFF\n\
             capture file\n");
    strcpy( moddesc->items[4] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(XONOFF, "Xon/Xoff");
    strcpy( modmenu->items[5] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DXONOFF, "Xon/Xoff signals        %-8s            ON, OFF\n"); /*MSG*/
    strcpy( moddesc->items[5] = (char *)malloc(strlen(cptr)+1), cptr);

    cptr = MSGSTR(AUTOWRAP, "AutoWrap");
    strcpy( modmenu->items[6] = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DAUTOWRAP, "VT100 auto wrap margins %-8s            ON, OFF\n"); /*MSG*/
    strcpy( moddesc->items[6] = (char *)malloc(strlen(cptr)+1), cptr);

#ifdef DEBUG
    kk = sprintf(ss,"initializing command descriptions\n");
    write(fe,ss,kk);
#endif

    /* command descriptions */

    cptr = MSGSTR(DCONN, "Make a connection\n"); /*MSG*/
    strcpy( cline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DDIR, "Display a dialing directory\n"); /*MSG*/
    strcpy( dline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DSEND, "Send a file over the current connection\n"); /*MSG*/
    strcpy( sline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DRECV, "Receive a file over the current connection\n"); /*MSG*/
    strcpy( rline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DBRK, "Send a break signal over the current connection\n"); /*MSG*/
    strcpy( bline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DTERM, "Terminate the connection\n"); /*MSG*/
    strcpy( xline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DHELP, "Get help and instructions\n"); /*MSG*/
    strcpy( hline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DMOD, "Modify local settings\n"); /*MSG*/
    strcpy( mline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DALT, "Alter connection settings\n"); /*MSG*/
    strcpy( aline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DPERF, "Perform an Operating System command\n"); /*MSG*/
    strcpy( eline = (char *)malloc(strlen(cptr)+1), cptr);
    cptr =  MSGSTR(DQUIT, "Quit the program\n"); /*MSG*/
    strcpy( qline = (char *)malloc(strlen(cptr)+1), cptr);

    /*
     * other strings
     */

#ifdef DEBUG
    kk = sprintf(ss,"initializing other strings\n");
    write(fe,ss,kk);
#endif

    cptr = MSGSTR(ENTER2, "To change a current choice, type the first letter of the command followed by\nyour new choice (example:  r 300) and press Enter.\n> "); /*MSG*/
    strcpy( enter2 = (char *)malloc(strlen(cptr)+1), cptr);

    cptr = MSGSTR(ENTER3, "To change a value, type the first letter of the command and press Enter.\n> "); /*MSG*/
    strcpy( enter3 = (char *)malloc(strlen(cptr)+1), cptr);

    cptr = MSGSTR(MOFF, "OFF"); /*MSG*/
    strcpy( off = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(MON, "ON"); /*MSG*/
    strcpy( on = (char *)malloc(strlen(cptr)+1), cptr);

    /* directory menu */

    cptr = MSGSTR(EXITSTR, "e(Exit)"); /*MSG*/
    strcpy( exitstr = (char *)malloc(strlen(cptr)+1), cptr);
    cptr = MSGSTR(DIRENTR, "\nEnter directory entry number or "); /*MSG*/
    strcpy( direntr = (char *)malloc(strlen(cptr)+1), cptr);

#ifdef DEBUG
    kk = sprintf(ss,"leaving menuinit\n");
    write(fe,ss,kk);
#endif
}
