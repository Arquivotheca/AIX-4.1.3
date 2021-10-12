static char sccsid[] = "@(#) 46 1.9 src/bos/usr/lpp/bosinst/bi_io/bi_io.c, bosinst, bos41J, 9517A_all 95/04/25 15:57:12";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: bi_io
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:	bi_io.c
 *
 * FUNCTION:  Utility to do various queries to the environment and hardware
 #            during BOS install.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Command line environment.
 *
 *	Flags:	-c, -t
 *
 *	Syntax:	bi_io -c | -t | -k | -f
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/signal.h>
#include <sys/lft_ioctl.h>
#include <sys/termio.h>
#include <fcntl.h>


#define NUMBUFSIZ 80
#define OPTSTRING "cfklt"
#define CLEARLFT "[H[J\n"
#define CLEARTTY "\012\012\012\012\012\012\012\012\012\012\012\012"
#define USAGE "usage: %s -c | -f | -k | -t\n\t-c: bi_io will return the escape sequence which will clear the screen.\n\t-f: bi_io will flush the input buffer.\n\t-k: bi_io will output 101, 102, 106, or ASCII depending on\n\t    the keyboard type.\n\t-t: bi_io will return a non-zero value if 000 is typed within the timeout.\n\n",argv[0]

main(int argc,char *argv[])
{
	int rc;
	int cflag = 0;	/* flag to output 'clear screen' sequence. */
	int fflag = 0;	/* flag to flush the input buffer. */
	int kflag = 0;	/* flag to determine keyboard type (101, 102, or 106). */
	int lflag = 0;	/* Flag to determine if console is an lft or tty. */
	int tflag = 0;	/* flag to catch user input before bypassing menus. */
	char numbuf[NUMBUFSIZ];

	extern int optind;
	extern int opterr;
	extern char *optarg;

	while((rc = getopt(argc,argv,OPTSTRING)) != EOF)
	{
		switch(rc)
		{
		case 'c':
			cflag ++;
			break;

		case 'f':
			fflag ++;
			break;

		case 'k':
			kflag ++;
			break;

		case 'l':
			lflag ++;
			break;

		case 't':
			tflag ++;
			break;

		default:
			fprintf(stderr,USAGE);
			exit(1);
			break;
		}
	}

	if(optind != argc)
	{
		fprintf(stderr,"%s: unknown argument '%s'\n",
				argv[0],argv[optind]);
		fprintf(stderr,USAGE);
		exit(1);
	}

	rc = 0;

	if (cflag)
	{
		/* Output the "clear screen" sequence. */
	   	is_it_lft() ? fprintf(stdout,CLEARLFT) : fprintf(stdout,CLEARTTY);
	}
	else if (fflag)
	{
		/* Flush the input buffer. */
		ioctl ( 0, TCFLSH, 0 );
	}
	else if (kflag)
	{
		/* Output the keyboard type. */
		keyboard_type();
	}
	else if (lflag)
	{
		/* Output 1 if an lft and 0 if not. */
    		fprintf(stdout, "%d\n", is_it_lft());
	}
	else if (tflag)
	{
	    /* Read from standard in for 4 seconds, waiting for special sequence. */
	    rc = readmaster ();
	}
	exit (rc);
}

/*
 * is_it_lft:  Queries console to determine whether or not it is an lft or tty.
 *             Sends clear of tty on non-zero and error return.
 */
int is_it_lft()
{
  int		fd;
  int		rc = 1;
  lft_query_t	argptr;
  char		*console = "/dev/tty";

	/*
	 * Determine if this is an async terminal or an lft.
	 *   Different actions will be taken if this is a tty vs an lft.
	 * ioctl to LFT_QUERY_LFT will fail if this is an async terminal.
	 */
	if( (rc = ioctl(0, LFT_QUERY_LFT, &argptr)) != -1)
	{
#ifdef _DEBUG
	fprintf(stderr,"lft\n");
#endif
		rc = 1;
	}
	else
	{
#ifdef _DEBUG
	fprintf(stderr,"tty\n");
#endif
		rc = 0;
	}

	return (rc);
}


/*
 * Read from standard in for 4 seconds, waiting for special sequence.
 * Calling program will execute a shell or put BOS install into prompted
 * mode if the user types in the special sequence before the read times out.
 * If 333 is typed the calling program will ensure that the install is non-turbo
 */
readmaster ()
{
	int rc;
	char buf[16];
	extern int timeout();

	signal(SIGALRM, timeout);

	alarm(4);

	printf("\n   ");
	gets(buf);

	alarm(0);

	if ( strcmp (buf, "000") == 0 )
	{
		rc = 0;
	}
	else if ( strcmp (buf, "111") == 0 )
	{
		rc = 1;
	}
	else if ( strcmp (buf, "333") == 0 )
	{
		rc = 3;
	}
	else
	{
		rc = 2;
	}

	return (rc);
}

timeout(int sig)
{
	signal(sig, SIG_IGN);
	printf("\b\b\b");
	exit(2);
}

/*
 * keyboard_type: Ouptuts 101, 102, 106, or ASCII, depending on keyboard type.
 */
keyboard_type()
{
    FILE	*odmfp;
    char 	odmget[256];
    char 	line[80];
    char 	*ptr;

    /* is is lft? */
    if (!is_it_lft())
    {
	printf("ASCII\n");
	return;
    }

    /*
     * Query the keyboard to determine its type.
     */
    line[0] = '\0';
    strcpy(odmget, "odmget -q\"name like 'kbd*'\" CuDv");
    odmfp = popen(odmget, "r");

    /* We're only interested int he last line */
    while (1)
    {
	if (!fgets(line, 80, odmfp)) break;
    }
    pclose(odmfp);

    if (line[0] == '\0')
    {
	printf("ASCII\n");
	return;
    }
    ptr = strstr(line, "kb1");
    if (ptr)
	printf("%3.3s\n", ptr+2);
    else
	printf("ASCII\n");

}
