#ifdef COMPILE_SCCSID
static char sccsid[] = "@(#)95  1.7.1.5  src/bos/usr/bin/panel20/p20_update.c, cmdhia, bos411, 9428A410j 7/21/92 10:26:58";
#endif
/*
 *
 * COMPONENT_NAME: (CMDHIA) Panel20 - utility for HIA device driver
 *
 * FUNCTIONS: update_scrn(), update_one(), update_two(), pr_SNRM(), pr_poll()
 *            pr_page(), pr_p1_line(), update_thr(), pw_msg()
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
 *
 */

#include <ctype.h>
#include <string.h>
#include "panel20.h"
#include "w_msg.h"

#define BEG_OFF 3                        /* beginning offset     */
#define NXT_OFF 9                        /* next position offset */

/*
 *
 *  Function name: update_scrn
 *
 *  Description:  decides which of the update routines needs to be called.
 *                the one for 20 - 01 (update_one), or 20 -02 (update_two).
 *
 *  Inputs/outputs:  none
 *
 *  Externals referenced  - cur_scrn
 *
 *  Externals modified - none
 *
 */

update_scrn( hia )
	char *hia;
{
        switch (cur_scrn) {
                case SCR_ONE:
                        update_one(last_scrn,hia);
                        break;
                case SCR_TWO:
                        update_two(last_scrn);
                        break;
                case SCR_THR:
                        update_thr(last_scrn);
                        break;
                default:
                        break;
        }
        last_scrn = cur_scrn;                   /* update last screen record */
}

/*
 *
 *  Function name: update_one
 *
 *  Description: does the update for the panel 20-01
 *
 *  Inputs/outputs: none
 *
 *  Externals - panel20  used for output
 *  Referenced  cur_page used to determine the screen.
 *
 */

update_one( last,hia )
	char last,
	     *hia;
{
        register int i;                      /* loop counter i               */
        char buf1[21];                       /* buffer 1 for msg services    */
        char buf2[21];                       /* buffer 2 for msg services    */

        if (last != SCR_ONE) {               /* if changing scrns, cl window */
                clear();
        }
        pw_msg(0,1,1);
        pw_msg(1,1,51);                 /* message #2 changed to message #51 */
        sprintf(buf1,"%2d",panel20.link_speed);
        sprintf(buf2,"%4X",panel20.brd_cst_cnt);
        pw_msg(3,1,3,buf1,buf2);
        sprintf(buf1,"%.2X%.2X%.2x%.2x",panel20.u_code_version[0],
        panel20.u_code_version[1],panel20.u_code_version[2],
                                panel20.u_code_version[3]);
        sprintf(buf2,"%4X",panel20.lnk_errors);
        pw_msg(4,1,4,buf1,buf2);
        sprintf(buf1,"%.2X  ",panel20.brd_frame[0]);
        for (i = 1 ; i != 9 ; i++) {                    /* put first 8 bites */
                sprintf(buf1,"%s%.2X",buf1,panel20.brd_frame[i]);
        }
        sprintf(buf2,"    ");
        for (i = 9 ; i != 17 ; i++) {                   /* put last 8 bites  */
                sprintf(buf2,"%s%.2X",buf2,panel20.brd_frame[i]);
        }
        pw_msg(6,1,5,buf1,buf2);
        pw_msg(9,1,52);                  /* message #6 changed to message #52 */
        pw_msg(10,1,53);                 /* message #7 changed to message #53 */
        pw_msg(11,1,54);                 /* message #8 changed to message #54 */
        pr_page(hia);                               /* print the current page */
        pw_msg(21,1,9);
        refresh();
}

/*
 *
 *  Function name: update_two
 *
 *  Description: does the update work for panel 20-02
 *
 *  Inputs/outputs: none.
 *
 *  Externals - panel20.graph_3270_switch
 *  Referenced
 *
 */

update_two( last )
	char last;
{
        register char i;                    /* loop counter                 */
        char sp =  2;                       /* number of jump for each loop */
        char num;
        register char offset = BEG_OFF;

        if (last != SCR_TWO) {              /* if changing scrns, cl window */
                clear();
        }
        pw_msg(0,1,10);
        pw_msg(1,1,11);
        if (panel20.graph_3270_switch == 02) {          /* then want spaces */
                sp = 3;
        }
        if (panel20.graph_3270_switch != 01) {          /* not grapics only */
                pw_msg(offset,1,12);
                for (num=i=offset+1 ;num <= offset + 8; i += sp,num += 2) {
                        pr_poll(i,num-offset+1,'t');
                        pr_SNRM(i+1,num-offset+1,'t');
                }
                offset += NXT_OFF;
        }
        if (panel20.graph_3270_switch == 01) {          /* then want spaces */
                sp = 3;
        }
        if (panel20.graph_3270_switch != 02) {             /* not 3270 only */
                pw_msg(offset,1,13);
                for (num=i=offset+1 ;num <= offset + 8; i += sp, num += 2) {
                        pr_poll(i,num-offset+1,'g');
                        pr_SNRM(i+1,num-offset+1,'g');
                }
        }
        pw_msg(21,1,14);
        refresh();
}

/*
 *
 *  Function name:  pr_SNRM
 *
 *  Description:  Outputs a line with the SNRM counts for panel 20 - 02.
 *                Puts 4 out on 1 line.
 *
 *  Inputs - y  =  the y position for the output line
 *           offset  =  the actual # of times thru the calling loop
 *
 *  Outputs:  nothing
 *
 *  Externals    - panel20.m3270[].snrm_cnt
 *  referenced   - panel20.grph[].snrm_cnt
 *
 */

pr_SNRM( y,offset,mode )
	char y,
             offset,
	     mode;
{
        register int i;                                  /* loop counter */

        move(y,2);
        if (mode == 'g') {                                   /* graphics */
                for (i = (2*offset)-4 ; i != (2*offset) ; i++)
                        printw("SNRM0%X = %8X   ",i,panel20.grph[i].snrm_cnt);
        }
        else {                                                   /* 3270 */
                for (i = (2*offset)-4 ; i != (2*offset) ; i++) {
                        printw("SNRM0%X = %8X   ",i,panel20.m3270[i].snrm_cnt);
                }
        }
}

/*
 *
 *  Function name: pr_poll
 *
 *  Description:  Outputs a line with the poll counts for panel 20 - 02.
 *                Puts 4 out on 1 line.
 *
 *  Inputs - y       =  the y position for the output line
 *           offset  =  the actual # of times thru the calling loop
 *           mode    =  'g' graphics mode, 't' terminal mode
 *
 *  Outputs : nothing
 *
 *  Externals    - panel20.m3270[].poll_cnt
 *  referenced   - panel20.grph[].poll_cnt
 *
 */

pr_poll( y,offset,mode )
	char y,
	     offset,
	     mode;
{
        register int i;                                   /* loop counter */

        move(y,2);
        if (mode == 'g') {                                    /* graphics */
                for (i = (2*offset)-4 ; i != (2*offset) ; i++)
                        printw("Poll0%X = %8X   ",i, panel20.grph[i].poll_cnt);
        }
        else {                                                    /* 3270 */
                for (i = (2*offset)-4 ; i != (2*offset) ; i++) {
                        printw("Poll0%X = %8X   ",i,panel20.m3270[i].poll_cnt);
                }
        }
}

/*
 *
 *  Function name: pr_page
 *
 *  Description: prints the current page for panel 20 - 01
 *
 *  Inputs/outputs: none
 *
 *  Externals referenced - cur_page      panel20
 *   config_grph   config_3270
 *
 *  Externals modified - cur_page set to 1 there is not enouf entries to get
 *  to the current page.
 *
 */

pr_page( hia )
	char *hia;
{
        char page_on=1;                       /* pages counting on           */
        register char entry_on=0;             /* entry terminal that u're on */
        char pr_entry;                        /* the entry getting printed   */
        char found;                           /* num found that are ok.      */
        char scr_fl=0;                        /* true if the screen is full  */

        while (page_on != cur_page) {         /* thumb thru previous pages   */
                for (pr_entry = entry_on,found=scr_fl=0;!scr_fl;pr_entry++) {
                        if (pr_entry >= NUM_LNKS) {
                                if (config_grph[pr_entry - NUM_LNKS]) {
                                        found++;
                                }
                        }
                        else if (config_3270[pr_entry]) {
                                found++;
                        }
                        if (found == ENT_PAGE) {
                                scr_fl = 1;                /* screen is full */
                        }
                        if (pr_entry >= 2 * NUM_LNKS)  {
                                cur_page = 1;          /* cannot get to page */
                                page_on = entry_on = 0;
                                scr_fl = 1;
                                pr_entry = -1;           /* 0 when loop done */
                        }
                }
                entry_on = pr_entry;
                page_on++;
        }
        /*
         * Do the output for the page now
         */
        clrtobot();
        for (found = scr_fl = 0 , pr_entry = entry_on; !scr_fl ;pr_entry++) {
                if (pr_entry >=  (2 * NUM_LNKS))  {      /* cant fill screen */
                        scr_fl = 1;
                        if (!found) {
                                if (cur_page != 1) {
                                        cur_page = 1;     /* go to 1st page */
                                        update_scrn(hia);
                                }
                        }
                }
                if (!scr_fl) {
                        if (pr_entry >= NUM_LNKS) {
                                if (config_grph[pr_entry - NUM_LNKS]) {
                                        pr_p1_line(pr_entry - NUM_LNKS,'g',hia);
                                        found++;
                                }
                        }
                        else if (config_3270[pr_entry]) {
                                pr_p1_line(pr_entry,'t',hia);
                                found++;
                        }
                        if (found == ENT_PAGE) {           /* screen is full */
                                scr_fl = 1;
                        }
                }
        }
}

/*
 *
 *  Function name:  pr_p1_line
 *
 *  Description:  Prints one of the actual terminal lines for the panel 20 - 01
 *
 *  Inputs - numb  - the number of the terminal
 *         - type  - 'g' is graphics, 't' is 3270 terminal
 *
 *  Outputs: none
 *
 *  Externals referenced - panel20 structure
 */

pr_p1_line ( numb,type,hia )
	char numb,
	     type,
	     *hia;
{
        printw("   /dev/");
        printw("%s",hia);
        printw("/");
        if (type == 'g') {                                  /* it's graphics */
                printw("1%X",numb);
                printw("      %4X          %2X    ",
                       chan_addr + numb,
                       numb+panel20.brd_frame[1] );
        }
        else {                                              /* 3270 terminal */
                printw("0%X",numb);
                printw("      %4X          %2X    ",
                       chan_addr + numb + ( panel20.brd_frame[2] - panel20.brd_frame[1] ),
                       numb + panel20.brd_frame[2] );
        }
        if (type == 'g') {                                       /* graphics */
                /*
                 * in order to conform with the version of panel20 run
                 * on the 5085, link status 8 should be outputted
                 * as 7.4
                 */
                if ( lnk_stat_grph[numb] != 8) {
                        printw("       %1d ",lnk_stat_grph[numb]);
                }
                else {
                        printw("      7.4");
                }
        }
        else {                                                       /* 3270 */
                /*
                 * in order to conform with the version of panel20 run
                 * on the 5085, link status 8 should be outputted as 7.3
                 */
                if (lnk_stat_3270[numb] != 8) {
                        printw("       %1d ",lnk_stat_3270[numb]);
                }
                else {
                        printw("      7.3");
                }
        }
        if (type == 'g') {                                       /* graphics */
                printw("      %8X",panel20.grph[numb].poll_cnt);
                printw("     %8X",panel20.grph[numb].snrm_cnt);
        }
        else {                                                       /* 3270 */
                printw("      %8X",panel20.m3270[numb].poll_cnt);
                printw("     %8X",panel20.m3270[numb].snrm_cnt);
        }
        addch('\n');
}

/*
 *
 * Function name: update_thr
 *
 * Description: print panel 20 - 03
 *
 * Externals Referenced: panel20 structure.
 *
 */

update_thr( last )
        char last;
{
        char buf[9];                                    /* buffer for status */

        if (last != SCR_THR) {
                clear();
        }
        pw_msg(0,1,15);
        pw_msg(1,1,16);
        pw_msg(2,1,17);
        sprintf(buf,"%8X",panel20.le_count[0]); pw_msg(3,1,18,buf);
        sprintf(buf,"%8X",panel20.le_count[1]); pw_msg(4,1,19,buf);
        sprintf(buf,"%8X",panel20.le_count[2]); pw_msg(5,1,20,buf);
        sprintf(buf,"%8X",panel20.le_count[3]); pw_msg(6,1,21,buf);
        sprintf(buf,"%8X",panel20.le_count[4]); pw_msg(7,1,22,buf);
        sprintf(buf,"%8X",panel20.le_count[5]); pw_msg(8,1,23,buf);
        sprintf(buf,"%8X",panel20.le_count[6]); pw_msg(9,1,24,buf);
        sprintf(buf,"%8X",panel20.le_count[7]); pw_msg(10,1,25,buf);
        sprintf(buf,"%8X",panel20.le_count[8]); pw_msg(11,1,26,buf);
        sprintf(buf,"%8X",panel20.le_count[9]); pw_msg(12,1,27,buf);
        sprintf(buf,"%8X",panel20.le_count[10]); pw_msg(13,1,28,buf);
        sprintf(buf,"%8X",panel20.le_count[11]); pw_msg(14,1,29,buf);
        sprintf(buf,"%8X",panel20.le_count[12]); pw_msg(15,1,30,buf);
        sprintf(buf,"%8X",panel20.le_count[13]); pw_msg(16,1,31,buf);
        sprintf(buf,"%8X",panel20.le_count[14]); pw_msg(17,1,32,buf);
        sprintf(buf,"%8X",panel20.le_count[15]); pw_msg(18,1,33,buf);
        pw_msg(20,1,34);
        pw_msg(21,1,35);
        refresh();
}

/*
 *
 * Name: pw_msg
 *
 * Purpose: pw_msg gets the specified message, and prints it to the curses
 *          screen with a 'printw' call.  This routine is based on 'w_disprmt'
 *
 *          y_pos is the y positon to move to.  if y = -1 then the move
 *          is not made.
 *
 *          calls to catgets (to get the message from the catalog) use 
 *          a pointer to the def_msg array (w_msg.h) indexed by the 
 *          message number (- 1) as the default string
 */

pw_msg( y_pos,setnum,msgnum,msgarg1 )
        int y_pos;
        int setnum;
        int msgnum;
        int msgarg1;
{
        char rmsg[NL_TEXTMAX];

        if (y_pos != -1) {
                move(y_pos,0);
        }
        w_setvar(setnum,msgnum,&msgarg1);
        if (validmsg)
        {
                switch(count) {
                        case 0:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ));
                                break;
                        case 1:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0] );
                                break;
                        case 2:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1] );
                                break;
                        case 3:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2] );
                                break;
                        case 4:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3] );
                                break;
                        case 5:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3],
                                         messargs[4] );
                                break;
                        case 6:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3],
                                         messargs[4], messargs[5] );
                                break;
                        case 7:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3],
                                         messargs[4],messargs[5],messargs[6] );
                                break;
                        case 8:
                                sprintf( rmsg,catgets( catd,setnum,msgnum,def_msg[ msgnum - 1 ] ),
                                         messargs[0],messargs[1],messargs[2],messargs[3],
                                         messargs[4],messargs[5],messargs[6],messargs[7]);
                                break;
                }               /* End of switch */
        }
        else
                sprintf( rmsg,catgets( catd,1,50,def_msg[ 50 - 1 ] ),messargs[0],messargs[1] );

        if (strcmp(rmsg, "") == 0)
                printw("Cannot retrieve a message.\n");
        else
                printw("%s",rmsg);
}
