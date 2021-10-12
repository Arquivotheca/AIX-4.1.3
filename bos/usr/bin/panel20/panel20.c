#ifdef COMPILE_SCCSID
static char sccsid[] = "@(#)97  1.6.1.2  src/bos/usr/bin/panel20/panel20.c, cmdhia, bos411, 9428A410j 7/21/92 10:26:27";
#endif
/*
 *
 * COMPONENT_NAME: (CMDHIA) Panel20 - utility for HIA device driver
 *
 * FUNCTIONS: main()
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

#define DEFINE_SYMS                /* MUST BE BEFORE #include "panel20.h" */

#include "panel20.h"
#include "w_msg.h"
#include <fcntl.h>
#include <sys/poll.h>
#include <errno.h>

/*
 *
 *  Function name: main
 *
 *  Description:  This is a program to work with the hia card.
 *                It does diagnostics on the card via 2 different
 *                formats, called panel 20 - 01, and panel 20 - 02.
 *
 *                The main opens the special hia card file and runs until
 *                a quit command is done to set done to true, as well as
 *                some other initalization and closing.
 *
 *                This program uses poll to detect keyboard input and
 *                hia input (appx every 3 seconds hia is updated).
 *
 *  Externals -  panel20
 *  Referenced
 *
 *  Externals -  panel20
 *  Modified
 *
 */

main(int argc, char *argv[])
{
        char done = 0;                       /* true if the program is done */
        register char i;                     /* loop counter                */
        void do_quit();                      /* defined for use with signal */
        void reinit_term();                  /* defined for use with signal */
        struct pollfd fdlist[2];             /* array of fds for poll       */
        unsigned long nfds;                  /* number of fds for poll      */
        int poll_ret;                        /* poll return code            */

#if DEBUGIT
printf("Initializing sig handlers\n");
#endif


        whp_signal(SIGINT,do_quit);          /* fix term b4 quitting        */
        whp_signal(SIGHUP,do_quit);          /* fix term b4 quitting        */
        whp_signal(SIGQUIT,do_quit);         /* fix term b4 quitting        */
        if (signal(SIGCONT,SIG_IGN) == SIG_DFL)
                whp_signal(SIGCONT,reinit_term); /* reinitialize terminal   */

#if DEBUGIT
printf("Initializing messages\n");
#endif

        w_init_mess();

        /*
         * initalization
         */

        if (( argc > 2 ) || 
           (( argc == 2 ) && ( argv[1][0] == '?' )) ||
           (( argc == 2 ) && ( argv[1][0] == '-' ) && ( argv[1][1] == 'h' ))) {
           printf( "%s",catgets( catd,1,56,def_msg[ 56 - 1 ] ));
           w_end_mess();
           exit(1);
        }

        /* if 2 args, second is adapter name   */
        /*  (i.e. hia0,hia1), else default is hia0 */
        if ( argc == 2 )
           strcpy( adapter_arg,argv[1] );
        else
           strcpy( adapter_arg,"hia0" );

        num_page = 0;  /* BAW init num_page variable */
        set_conf( config_3270,HIA_FILE,adapter_arg );

#if GRAPH_ONE
        set_conf( config_grph,HIA_FILE,adapter_arg );
#endif
        num_page = ( num_page - 1 ) / 8 + 1;  /* BAW sessions/8 lines per page */
        init_term();
        panel20_fd = open(HIA_FILE, O_RDWR);

#if DEBUGIT
printf("Opening Device Driver rc= %x\n",panel20_fd);
printf("Press any key to continue\n");
getchar();
#endif

        if (panel20_fd < 0) {                /* cant open set l.s. to 2     */
                for (i = 0; i != NUM_LNKS; i++) {
                        lnk_stat_3270[i] = 2;
                        lnk_stat_grph[i] = 2;
                }
        }
        else {
                for (i = 0; i != NUM_LNKS; i++) {
                        lnk_stat_3270[i] = 4;
                        lnk_stat_grph[i] = 4;
                 }


        }
        update_scrn( adapter_arg );

        while (!done) {
                fdlist[0].fd = 0;             /* assign fd for stdin         */
                fdlist[0].reqevents = POLLIN; /* only check for input (read) */
                if (panel20_fd >= 0) {
                        fdlist[1].fd = panel20_fd; /* set fd for HIA_FILE   */
                        fdlist[1].reqevents = POLLIN;
                                                   /* only check for input   */
                        nfds = 2;                  /* stdin + HIA_FILE      */
                }
                else {
                        nfds = 1;                  /* only stdin             */
                }
                poll_ret = poll(fdlist, nfds, (long)-1);
                if (poll_ret > 0) {
                        if (panel20_fd >= 0) {            /* was opened    */
                           if (fdlist[1].rtnevents == POLLIN) {
                              read_hia( adapter_arg );
                           }
                        }
                        if (fdlist[0].rtnevents == POLLIN) { /* stdin waiting */
                           done = get_cmd( adapter_arg );
                        }
                }
                else if (errno != EINTR) {
                        w_dismsg(1,38);
                        reset_term();
                        w_end_mess();
                        exit(1);
                }
        }
        close(panel20_fd);
        reset_term();
        putchar('\n');
        w_end_mess();
        exit(0);
}
