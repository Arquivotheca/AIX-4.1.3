static char sccsid[] = "@(#)85	1.1  src/bos/usr/bin/bterm/main.c, bos, bos410 8/26/93 13:35:10";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: killchild
 *		main
 *		reapchild
 *		spawn
 *		window_change
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*	Documentation:
**
** This is the main program file of BTERM.
** Basically a loop on keyboard input, output to pty , pty input
** and then screen output
**
*/
#include <sys/types.h>   
#include <sys/select.h>
#include <sys/devinfo.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <cur00.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <term.h>

#include "global.h"
#include "main.h"
#include "trace.h"

#undef delch()
#undef insch()

static int reapchild();
static int killchild();
static int window_change();

int	CHILD_id;
char	*CMD_to_EXEC = NULL;


/* ====================== main program ============================ */

main(argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
    static char bidi_on[20] = "BIDI=on";
    static char bterm_on[20] = "BTERM=on";
    unsigned char *env_var;

    static int i=-1;
    struct timeval tmout;
    fd_set readfds, writefds, exceptfds;
    int karg, nfds, selstat;
    char *locale_name;
    char *terminal;
    int RC;


/*=========== CHECK IF ALREADY ACTIVE  ===========================*/
        if (strcmp( getenv("BTERM"), "on")== 0)
	{
	   printf ("\nBTERM is already active !\n");
	   exit(0);
	}

      /* check if terminal is supported */
        terminal=malloc(strlen(getenv("TERM")));
        strcpy(terminal,getenv("TERM"));
      /* default is ibm3151 */
        if (strlen(terminal)==0)
         strcpy(terminal,"ibm3151");
        if (!init_term(terminal))
	{
	   printf ("\nBTERM does not support %s terminals !\n",terminal);
           free(terminal);
	   exit(0);
	}
        free(terminal);

 	putenv(bterm_on);
 	putenv(bidi_on);

        /* initialize bidi structure */
        locale_name=malloc(sizeof(setlocale(LC_CTYPE,"")));
        strcpy(locale_name,setlocale(LC_CTYPE,""));
        set_bterm_defaults(locale_name,&argc,argv);

        /* initial command to execute */
 	for(karg = 1; karg < argc; karg++)
	{
		if (strcmp(argv[karg], "-e") == 0)
	 		CMD_to_EXEC = argv[++karg];
	}

	/* Load Conversion Maps */
	if (BDLoadMaps() < 0)
	{
		printf("\nCould not initialize maps......exiting !\n");
		exit (1);
        }

        /* initialize BIDI library */
        layout_object_create(locale_name,&plh);
        free(locale_name);
        /* check active bidi functionality and shaping . */
        BDActive();

/*======================  INITIALIZATION  ============================*/

	tcgetattr(0, &orgtermios);     /* save /dev/tty settings */
        setupterm(0,1,0);              /* initialize terminfo databse */
        /* get window size using 'lines' and 'columns' from terminfo
           database */
        if (lines)     LINES = lines;
        if (columns)   COLUMNS = columns;
        if (LINES>Max_Lines) LINES=Max_Lines;
        if (COLUMNS>Max_Columns) COLUMNS=Max_Columns;
TRACE(("LINES=%d COLUMNS=%d \n",LINES,COLUMNS));

	ptyc_init(0);                  /* initialize PTY         */
	if (ptystatus != 0)
		exit(0);

				   /* set size in the PTY    */
	W_Size.ws_row = LINES;
	W_Size.ws_col = COLUMNS;
	ioctl(ptyfildes,   TIOCSWINSZ, &W_Size);
	ioctl(ttypPfildes, TIOCSWINSZ, &W_Size);


	if((CHILD_id = spawn()) < 0)
		goto stop_program;

	signal(SIGCHLD, reapchild);
	signal(SIGTERM, killchild);
	signal(SIGWINCH,window_change);

	/* This is the parent process: open real terminal
	** and start working
	*/
	physio_init();

        new_jdx = (char *) malloc (1024*sizeof (char));


        /* clear screen and initialize screen buffer before starting */
        SCR = (screen_type *) malloc(sizeof(screen_type));
        if (!SCR) TRACE(("unable to allocate screen buffer \n"));
        SCR->_cur_atrib=active_atrib=ATRIB_NORMAL;
        SCR->_cur_grph=active_grph=0;
        send_to_screen(TERM_INIT,strlen(TERM_INIT)+1);
        Lex_Mode=1;
        (*do_reset_function_keys)();
        do_clear_screen();
        init_v_tabs(); /* initialize horizontal and vertical tasb stops */
        init_h_tabs();

/*====================================================================*/


  wait_for_io:
	FD_ZERO(&readfds);
	FD_SET(0,  &readfds);
	FD_SET(ptyfildes,  &readfds);
	tmout.tv_sec  = 1;
	tmout.tv_usec = 0;
	nfds = ptyfildes + 1;

	/* now wait till some input appears */

	if(Y_Status)
		/* display status once/second   */
		selstat =  select(nfds, &readfds, NULL, NULL, &tmout);
	else
		selstat = select(nfds, &readfds, NULL, NULL, NULL);


/*=============== READ CHARACTER FROM KEYBOARD =======================*/

  get_from_kbd:
               /* Read character from keyboard .
                  If no character found at keyboard, 
                  check if pty has any output . 
                  If character found at keyboard,
                  handle it , then go check if pty
                  has any output. */
    parse_kbd_input();

/*======== RECEIVE FROM OUTSIDE WORLD AND ECHO ON SCREEN ============*/

  get_frm_pty:

	if( (jdx = get_from_pty(new_jdx)) > 0)
	{
             send_to_screen(new_jdx,jdx);
	     goto get_from_kbd;
 	 }


/*=================== TO THE BEGINING AGAIN ==========================*/
  
  flush_to_hft();
  goto wait_for_io;

/*=================== STOP PROGRAM ===================================*/

  stop_program:
   layout_object_free(plh); /* close the BIDI library */
   reset_shell_mode();      /* reset terminfo usage          */
   ptyc_end();              /* Close PTY controller side     */
   physio_end();            /* return tty to original state  */
   free(new_jdx);           /* free all allocated memory */
   free(SCR);
   free(BDCurSeg);
   return;

}

/*====================================================================*/


/*====== Create Child Process on ttyp side and run sh in it ==========*/

int spawn()
{
	int forkid, uid, gid, fork();
	int x1, x2, x3;
	char *shell;
	void exit();

	uid = getuid();
	gid = getgid();

	forkid = fork();
	if(forkid < 0)
	{
		close(ptyfildes);
		printf("\nFork Failed. -- exiting. \n");
		return(-1);
	}

	else if(forkid == 0)
	{
		/* child process, open ttypxx on fildes = 0, 1, 2
		** before exec sh 
		** remove controll terminal for this process
		*/

		close(0);
		close(1);
		close(2);

		(void) setsid();

		/* Open stdin, stdout, stderr again. open ttyp, it will
		** become the controll terminal for the process
		*/

		if((ttypfildes = open(ttydev,O_RDWR)) < 0)
			 return;

		/* restore /dev/tty sttings               */
		tcsetattr(ttypfildes, TCSANOW, &orgtermios);
		if(ttypfildes != 0)
		{
			dup2(ttypfildes,0);
			close(ttypfildes);
		}
		dup2(0,1);
		dup2(0,2);

		chown(ttypfildes, uid, gid);
		chmod(ttypfildes, 0622);

		if (CMD_to_EXEC)
		{
			execlp(CMD_to_EXEC, CMD_to_EXEC, 0);
		}
		else    /* no command given, use SHELL variable */
		{
			shell = getenv("SHELL");
			if ( (! shell) || *shell == '\0')
				shell = "/bin/sh";

			execlp(shell, shell, 0);
		}

 		exit(0);
	}

	return(forkid);
}

static reapchild()
{
  int pid, status;
  pid = wait(&status);
  if(pid != CHILD_id) {
    (void) signal(SIGCHLD, reapchild);
    return;
  }
  else {
    ptyc_end();
    physio_end();
    exit(0);
  }
}

static killchild()
{
  kill(CHILD_id, 9);

  ptyc_end();
  physio_end();
  exit(0);
}

static window_change()
{
  int i, linx, colx, savNIN[Max_Lines];

  ioctl(0,      TIOCGWINSZ, &W_Size);
  linx = W_Size.ws_row;
  colx = W_Size.ws_col;
/*
  hwin_chng(linx,colx);
*/
  for(i=0; i<LINES; i++) {
    savNIN[i] = 0;
    if(NO_INVERSE[i] == -1) {
      savNIN[i] = -1;
      NO_INVERSE[i] = 0;
      }
    }

  for(i=0; i<LINES; i++) {
    if(savNIN[LINES -1 -i] != 0)
      if((linx - 1 -i) >= 0) NO_INVERSE[linx -1 -i] = -1;
    }

  LINES   = linx;
  COLUMNS = colx;
  if(Y_Status) Y_Status = LINES;
/*
  scr_refresh();
*/
  ioctl(ptyfildes,   TIOCSWINSZ, &W_Size);
  ioctl(ttypPfildes, TIOCSWINSZ, &W_Size);
  kill(0, SIGWINCH);

  signal(SIGWINCH,window_change);
  return;
}

