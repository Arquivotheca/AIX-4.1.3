static char sccsid[] = "@(#)76    1.28  src/bos/usr/ccs/bin/dbx/dpi_main.c, cmddbx, bos411, 9428A410j 6/1/94 16:32:51";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_main
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/


/*
 * Debugger main routine.
 */

#include <stdio.h>
#include <setjmp.h>
#include <termio.h>
#include <locale.h>

#include <signal.h>
#include <fcntl.h>

/* The following defines and declarations are placed here in order to allow 
   dbx to be built as a main program against a debugger library,
   with the header files placed in the build location for the 
   library.  This organization may need to be revamped in the
   future.
*/

typedef enum { false, true } Boolean;
typedef FILE *File;
typedef char *String;
typedef struct {
    struct termio ttyinfo;
    int fcflags;
} Ttyinfo;

#define nil 0
#define MAXLINESIZE 10240
#define DBX_TERMINATED 0x80
#define DETACHED 0x04
#define isterm(file)	(interactive || isatty(fileno(file)))

extern void *process;
extern Boolean interactive;		/* standard input IS a terminal */
/* End of special definitions and declarations */

extern char scanner_linebuf[];
extern File in;
extern String prompt;
extern Boolean eofinput();
extern int *envptr;

extern Ttyinfo ttyinfo;
extern Ttyinfo ttyinfo_in;
extern char *rootdbx_ttyname;

extern int dpi_report_executing();
extern int dpi_report_shell();
extern int dpi_report_trace();
extern int dpi_report_multiprocess();
extern int dpi_ctx_level();
extern int xde_open_windows();

char line[MAXLINESIZE];

#ifdef KDBX
int cur_cpu = -1;
extern int kdbx_multipro;
#endif /* KDBX */

/*
 * Main program.
 */

main(argc, argv)
int argc;
String argv[];
{
    unsigned int dpi_state = 0;
    int pid;
    char *command;
    char eofchar[1];
    jmp_buf main_env;

    /* 
       Setting up new tty for screen and multprocess debugging commands.
       These codes are reached by screen().
    */
    if(*(argv[1]) == '-' && *(argv[1]+1) == 'b' && *(argv[1]+2) == '\0'
      && argc == 3) {
       char *s;
       int fd;
       fd = open(argv[2], O_WRONLY);
       signal(SIGINT, SIG_IGN);
       signal(SIGQUIT, SIG_IGN);
       s= ttyname(0);
       write(fd,s,strlen(s)+1);
       while(1) pause();
       exit(0);
    }

#ifdef KDBXRT
    int fprintf();
#endif

    rootdbx_ttyname = strdup(ttyname(0));  /* save the originating ttyname */
    eofchar[0] = '\0';
    setlocale(LC_CTYPE, ""); /* LC_TIME & LC_MONETARY not needed. */
    setlocale(LC_MESSAGES, ""); /* LC_NUMERIC would cause problems. */
    if (dpi_main(argc, argv, fprintf, fprintf, dpi_report_executing,
        dpi_report_shell, dpi_report_trace, dpi_report_multiprocess,
        dpi_ctx_level, xde_open_windows, &pid, &dpi_state, false, argv[0])
        != nil)
	return -1;
    if (setjmp(main_env)) {
	restoretty(stdout, &ttyinfo);
	restoretty(stdin, &ttyinfo_in);
    } else {
	envptr = main_env;
    }
    while ( !(dpi_state & DBX_TERMINATED) && !(dpi_state & DETACHED) ) {
        if (isterm(in)) {
#ifdef KDBX
	    if (kdbx_multipro && varIsSet("$prompt_cpu"))
		fprintf (stdout, "(%d)%s ", cur_cpu, prompt);
	    else
#endif /* KDBX */
            fprintf (stdout, "%s ", prompt);
        }
        fflush(stdout);
        command = fgets( line, MAXLINESIZE, in );
	if (command == nil) {
	    if (!eofinput()) {
	        command = eofchar;
	    } else {
		dpi_state |= DBX_TERMINATED;
		continue;
	    }
	}
	resetinput();
        dpi_command( command, &dpi_state);
    }
    fflush(stdout );
    exit(0);
}
