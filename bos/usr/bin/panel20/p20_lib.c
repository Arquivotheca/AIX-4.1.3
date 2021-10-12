#ifdef COMPILE_SCCSID
static char sccsid[] = "@(#)94  1.9.1.4  src/bos/usr/bin/panel20/p20_lib.c, cmdhia, bos411, 9428A410j 7/21/92 10:27:14";
#endif
/*
 *
 * COMPONENT_NAME: (CMDHIA) Panel20 - utility for HIA device driver
 *
 * FUNCTIONS: do_quit(), init_term(), reinit_term(), reset_term(), get_cmd(),
 *            read_hia(), set_conf(), set_lnk_stats()
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

#include <stdio.h>
#include <string.h>
#include <sys/termio.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include "panel20.h"

/*
 *
 *  Function name:  do_quit
 *
 *  Description:  do_quit resets the terminal to the way it was before the
 *                program was run, and then the program exits.
 *
 *  Inputs:  none.
 *  Outputs: none -- when do_quit is called the program is exited.
 *
 */

void do_quit()
{
        reset_term();
        putchar('\n');
        w_end_mess();
        exit(1);
}

/*
 *
 *  Function name:  init_term
 *
 *  Description:   init_term initalizes the terminal to use curses
 *                 these modes are used: crmode, noecho, keypad.
 *
 *  Inputs/outputs: none
 *
 *  Externals -  stdscr is initalized by initscr()
 *  modified
 *
 */

init_term()
{
        initscr();
        nonl();
        noecho();
        crmode();
        keypad(TRUE);
        extended(TRUE);
}

/*
 *
 *  Function name:  reinit_term
 *
 *  Description:   reinit_term reinitalizes the terminal when the SIGCONT
 *                 signal is received.
 *
 *  Inputs/outputs: none
 *
 *  Externals -
 *  modified
 *
 */

void reinit_term()
{
        whp_signal(SIGCONT,reinit_term);
        nonl();
        noecho();
        crmode();
        keypad(TRUE);
        extended(TRUE);
        refresh();
}

/*
 *
 *  Function name: reset_term
 *
 *  Description:   puts the terminal back in the original mode when the
 *                 program was started.
 *
 *  Inputs/outputs: none
 *
 *  Externals - stdscr is deleted by endwin()
 *
 */

reset_term()
{
        refresh();
        keypad(FALSE);
        echo();
        nocrmode();
        endwin();
}

/*
 *
 *  Function name: get_cmd()
 *
 *  Description:  get_cmd reads one command from input.  It is called after
 *                a poll indicating that there was input.
 *
 *  Inputs:  none
 *
 *  Outputs:  1 if quit command is entered, 0 otherwise
 *
 *  Externals - cur_scrn is used to see if DOWN and BACK are valid
 *  Referenced
 *
 *  Externals - cur_scrn (changed to other screen if CH_SCRN command is given)
 *  Modified - trapped keys changed: cursor up & down to page up & down.
 *           - quit is F10
 *           - Space bar to Enter key to toggle between pannels
 *           - Default has key not defined message.
 *
 */

get_cmd( hia )
	char *hia;
{
        int ch;

        ch = getch();              /* try to get a char                  */
        switch( ch ) {             /* the EOF when the signal is trapped */
                case KEY_NEWL:                          /* enter key     */
                        cur_scrn++;                     /* next screen   */
                        move(21,0);
                        pw_msg(21,1,39);
                        refresh();
                        if (cur_scrn > SCR_THR) {             
                                cur_scrn = SCR_ONE;     /* wrap to first */ 
                        }                               /* screen        */  
                        update_scrn(hia);
                        break;
                case QUIT:
                        return(1);                      /* send quit code */
                case P_PAGE:
                        if (cur_scrn == SCR_ONE) {      /* up arrow */
                                move(21,0);
                                pw_msg(21,1,39);
                                refresh();
                                cur_page--;             /* previous screen */
                                if (cur_page <= (char)0) {    
                                        cur_page = num_page;  /* wrap to last */
                                }                             /* screen       */
                        	update_scrn(hia);
                        }
                        else {
                           move( 21,0 );                /* error message */ 
                           pw_msg( 21,1,40 );           /* key not valid */ 
                           refresh();                   /* on scr 2 & 3  */
                        }
                        break;
                case N_PAGE:
                        if (cur_scrn == SCR_ONE) {
                                move(21,0);
                                pw_msg(21,1,39);
                                refresh();
                                cur_page++;            /* next screen */ 
                                if (cur_page > num_page) {   
                                        cur_page = SCR_ONE;  /* wrap to first */
                                }                            /* screen        */
                        	update_scrn(hia);
                        }
                        else {
                           move( 21,0 );               /* error message */
                           pw_msg( 21,1,40 );          /* key not valid */
                           refresh();                  /* on scr 2 & 3  */
                        }
                        break;
                default:
                        move(21,0);
                        if ( cur_scrn == SCR_ONE )  /* display appropriate */
                           pw_msg(21,1,55);         /* error messages if   */
                        else                        /* user hit invalid    */
                           pw_msg(21,1,40);         /* key                 */
                        refresh();
                        break;
        }
        return(0);                                 /* standard exit */
}

/*
 *
 *  Function name: read_hia
 *
 *  Description:  reads the HIA, and fills the panel20 structure with this
 *                data.   This is called by after the poll call that
 *                indicates if there is input to read.
 *
 *  Inputs:  none.
 *
 *  Outputs:  0 for early function quitting on error link status
 *
 *  External - panel20 is brought up to date
 *  modified - config_3270 and config_grph
 */

read_hia( hia )
	char *hia;
{
        register char i;                              /* loop cntr           */
        register int ierror;                          /* error flag for read */

        ierror = read(panel20_fd, &panel20, sizeof(panel20));
        if (ierror < 0x00) {               /* error -- all lnk status goto 2 */
                for (i=0 ; i != NUM_LNKS ; i++) {
                        lnk_stat_3270[i] = 2;
#if GRAPH_ONE
                        lnk_stat_grph[i] = 2;
#endif
                }
        }
        if (lnk_stat_3270[0] != 2) {
                set_lnk_stats();                      /* set link statuses   */
        }
        update_scrn(hia);                                /* update the screen   */
}

/*
 *
 *  Function name: set_conf
 *
 *  Description: retrieves the lower bound and number of sessions values from
 *               Customized or Predefined Attribute Object Class,
 *               converts them from char strings to longs and calculates the
 *               the upper bound.
 *               The name of the HIA file to be opened is put into fname.
 *
 *  Inputs/outputs:  conf - array of flags for configured addresses.
 *                   fname - name of HIA file to be opened.
 *
 * Externals modified - none
 *
 */

set_conf( conf,fname,hname )
        char *conf,      /* array of configured      */
             *fname,     /* place to put hia file name */
             *hname;     /* argv[1]  */
{
        struct CuDv *dev_struct;
        struct CuDv return_data;
        extern struct CuAt *getattr();
        struct CuAt *attr_struct;
        int how_many;
        int t_low, t_high, num_sessions;
        int lbcode, nscode;
        register char i;
        char ttemp[80];

        strcpy(fname,hname);  /* EMR for multi-hia, set proper adapter name */
        /*
         *  Initialize ODM
         */

#ifdef DEBUGIT
printf("Initializing ODM now\n");
#endif

        if ((odm_initialize()) < 0) {
                w_dismsg(1,42);
                w_end_mess();
                exit(1);
        }

        /*
         *  Set ODM path to /etc/objrepos
         */
        if ((int)(odm_set_path("/etc/objrepos")) < 0) {
                w_dismsg(1,43);
                w_end_mess();
                exit(1);
        }

        /*
         *  Get device object from the Customized Device object class
         */
        strcpy(ttemp,"name = '"); /* for multi-hia, */
        strcat(ttemp,fname);      /* set proper     */
        strcat(ttemp,"'");        /* adapter name   */

        if ((int)(dev_struct = odm_get_first(CuDv_CLASS, ttemp,
                                                        &return_data)) < 0) {

                w_dismsg(1,41);
                w_end_mess();
                exit(1);
        }

        /*
         *  Check if the device has been defined
         */
        if (dev_struct == NULL) {

#ifdef DEBUGIT
printf("ODM reports device not defined\n");
#endif

                w_dismsg(1,45);
                w_end_mess();
                exit(1);
        }

        /*
         *  Check if the device has been configured
         */

#ifdef DEBUGIT
printf("ODM reports device status= %x\n",dev_struct->status);
#endif

        if (dev_struct->status != AVAILABLE) {
                w_dismsg(1,46);
                w_end_mess();
                exit(1);
        }

        if (conf == config_3270) {

                /*
                *  Get attribute lower_bond
                */
                if ((attr_struct = getattr(fname, "lower_bond", FALSE, &how_many))
                                                                == NULL) {
                        lbcode = 0;
                }
                else {
                        lbcode = 1;
                        t_low = atoi(attr_struct->value);
                }

                /*
                *  Get attribute num_sessions
                */
                if ((attr_struct = getattr(fname, "num_sessions", FALSE, &how_many))
                                                                 == NULL) {
                        nscode = 0;
                }
                else {
                        nscode = 1;
                        num_sessions = atoi(attr_struct->value);
                        num_page += num_sessions;  /* BAW incr num_page variable */
                }
        }
        else if (conf == config_grph) {

                /*
                *  Get attribute lower_bond
                */
                if ((attr_struct = getattr(fname, "lower_5080_bond", FALSE, &how_many))
                                                                == NULL) {
                        lbcode = 0;
                        }
                else {
                        lbcode = 1;
                        t_low = atoi(attr_struct->value);
                }

                /*
                *  Get attribute num_sessions
                */
                if ((attr_struct = getattr(fname, "num_5080_sess", FALSE, &how_many))
                                                                 == NULL) {
                        nscode = 0;
                }
                else {
                        nscode = 1;
                        num_sessions = atoi(attr_struct->value);
                        num_page += num_sessions;  /* BAW incr num_page variable */
                }

                /*
                *  Get attribute 5080_chan_addr
                */
                if ((attr_struct = getattr(fname, "addr_5080_chan", FALSE, &how_many))
                                                                 == NULL) {
                        chan_addr = 0x0000;
                }
                else {  /* convert to long, and subtract off lower bound to get base */
                        chan_addr = (int)strtol( attr_struct->value,NULL,0 ) - t_low;
                }
        }
        /*
         * Terminate ODM
         */

#ifdef DEBUGIT
printf("Terminating ODM\n");
#endif

        if ((odm_terminate()) < 0) {
                w_dismsg(1,44);
                w_end_mess();
                exit(1);
        }

        /*
         *  If both the lower bound and number of sessions were retrieved,
         *  then calculate upper bound
         *  else display error message and exit
         */
        if ((lbcode == 1) && (nscode == 1)) {
                t_high = t_low + num_sessions - 1;
        }
        else {
                w_dismsg(1,36);
                w_end_mess();
                exit(1);
        }

        if ((t_high > (NUM_LNKS - 1)) || (t_low > (NUM_LNKS - 1))) {
                w_dismsg(1,37);
                w_end_mess();
                exit(1);
        }
        for (i = 0; i != NUM_LNKS; i++) {
                if ((num_sessions > 0) && (i <= t_high) && (i >= t_low)) {
                        conf[i] = 1;
                }
                else {
                        conf[i] = 0;
                }
        }
        strcpy(fname,"/dev/");  /* EMR for multi-hia, set proper adapter name */
        strcat(fname,hname);
        strcat(fname,"/10");
}


/*
 *
 *  Function name: copy_panel20   (copy the panel20 structure)
 *
 *  Description:  Makes a copy of the panel20 data for comparison.
 *
 *  Inputs:  buf  - static target buffer
 *
 *  Outputs: copy of panel20 into buf
 *
 *  Externals Referenced -  panel 20 structure
 *                       -  config_3270, config_grph
 *
 */

copy_panel20( buf )
	struct panel20_data *buf;
{
        register char i;                                /* loop counter */

        buf->graph_3270_switch = panel20.graph_3270_switch;
        buf->link_speed = panel20.link_speed;
        buf->lnk_errors = panel20.lnk_errors;
        buf->brd_cst_cnt = panel20.brd_cst_cnt;
        for (i=0; i < 17 ; i++) {             /* broadcast frame */
                buf->brd_frame[i] = panel20.brd_frame[i];
        }
        for (i=0; i < 16 ; i++) {
                buf->enb_3270[i] = panel20.enb_3270[i];
        }
        for (i=0; i < 16 ; i++) {
                buf->enb_grph[i] = panel20.enb_grph[i];
        }
        for (i=0; i < 16 ; i++) {
                buf->m3270[i].poll_cnt = panel20.m3270[i].poll_cnt;
                buf->grph[i].poll_cnt = panel20.grph[i].poll_cnt;
                buf->m3270[i].snrm_cnt = panel20.m3270[i].snrm_cnt;
                buf->grph[i].snrm_cnt = panel20.grph[i].snrm_cnt;
        }
        for (i=0; i < 16 ; i++) {
                buf->le_count[i] = panel20.le_count[i];
        }
}





/*
 *
 *  Function name: set_lnk_stat   (get link status)
 *
 *  Description:  Returns the link status (integer).
 *
 *  Inputs:  numb - number of the input link
 *           type - type of the input link ('g' graphics, 't' terminal)
 *
 *  Outputs:
 *           0  device enabled
 *           2  probable hardware error
 *           4  link not operational
 *           5  5088 reported system error
 *           6  Device conflict
 *           7  Device address not within 5088 Address Range (!done)
 *           8  5088 is not Enabled for 3274 display address
 *           8  5088 is not Enabled for Graphic address
 *           9  device not enabled
 *
 *  Externals Referenced -  panel 20 structure
 *                       -  config_3270, config_grph
 *
 */

set_lnk_stats()
{
        static struct panel20_data p20_buf;     /* save poll counters   */
        static int broc_cnt=0;                  /* broad cast cnt timer */
        register char i;                                /* loop counter */
        char low_conf;
        char high_conf;

        if (panel20.brd_frame[0]) {              /* 5088 reported error */
                for (i = 0 ; i != NUM_LNKS ; i++) {
                        lnk_stat_3270[i] = 5;
#if GRAPH_ONE
                        lnk_stat_grph[i] = 5;
#endif
                }
                copy_panel20(&p20_buf);
                return(0);
        }
        broc_cnt++;
        if (p20_buf.brd_cst_cnt != panel20.brd_cst_cnt) { /* is stepping */
                broc_cnt=0;                                   /* reset count */
        }
        if (broc_cnt > 3) {                           /* appx 12 seconds */
                for (i = 0; i != NUM_LNKS ; i++) {
                        lnk_stat_3270[i] = 4;
#if GRAPH_ONE
                        lnk_stat_grph[i] = 4;
#endif
                }
                copy_panel20(&p20_buf);
                return(0);
        }
        low_conf = panel20.brd_frame[2] & 0x0f;            /* low nibble */
        low_conf = 0;     /* i is a device number, independent of lower 5088 address */
        high_conf= low_conf + panel20.brd_frame[4];
        for (i=0; i != NUM_LNKS ; i++) {
                /*
                 * 3270 connection
                 */
                if (config_3270[i]) {                      /* its configured */

                        if (panel20.brd_frame[6] == 00) {
                                lnk_stat_3270[i] = 8; /* displayed as 7.3 in p20_update */
                        }
                        else if ((i<low_conf) || (i >= high_conf)) {
                                lnk_stat_3270[i] = 7;
                        }
                        else if (!panel20.enb_3270[i]) {
                                if (panel20.m3270[i].poll_cnt !=
                                        p20_buf.m3270[i].poll_cnt) {
                                        lnk_stat_3270[i] = 6;
                                }
                                else {
                                        lnk_stat_3270[i] = 9;
                                }
                        }
                        else {                          /* device enabled */
                                if (panel20.m3270[i].poll_cnt !=
                                        p20_buf.m3270[i].poll_cnt) {
                                                lnk_stat_3270[i] = 0;
                                }
                                else {  /* device enabled but not polling */
                                                lnk_stat_3270[i] = 3;
                                }
                        }
                }
        }
#if GRAPH_ONE
        for (i=0; i != NUM_LNKS ; i++) {
                low_conf = panel20.brd_frame[1] & 0x0f;        /* low nibble */
                low_conf = 0;     /* i is a device number, independent of lower 5088 address */
                high_conf= panel20.brd_frame[3];
                /*
                 * Graphic connection
                 */
                if (config_grph[i]) {                      /* its configured */
                        if (panel20.brd_frame[5] == 00) {
                                lnk_stat_grph[i] = 8;  /* displayed as 7.4 in p20_update */
                        }
                        else if ((i<low_conf) || (i >= high_conf)) {
                                lnk_stat_grph[i] = 7;
                        }
                        else if (!panel20.enb_grph[i]) {
                                if (panel20.grph[i].poll_cnt !=
                                        p20_buf.grph[i].poll_cnt) {
                                                lnk_stat_grph[i] = 6;
                                }
                                else {
                                        lnk_stat_grph[i] = 9;
                                }
                        }
                        else {                           /* device enabled */
                                if (panel20.grph[i].poll_cnt !=
                                        p20_buf.grph[i].poll_cnt) {
                                                lnk_stat_grph[i] = 0;
                                }
                                else {  /* device enabled but not polling */
                                                lnk_stat_grph[i] = 3;
                                }
                        }
                }
        }
        copy_panel20(&p20_buf);

#endif
}
