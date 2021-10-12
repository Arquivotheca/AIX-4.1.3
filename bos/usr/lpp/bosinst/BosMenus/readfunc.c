static char sccsid[] = "@(#) 02 1.2 src/bos/usr/lpp/bosinst/BosMenus/readfunc.c, bosinst, bos411, 9428A410j 94/01/14 08:38:30";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: BIclear_cmd_line, BIreadtty, BIttytrminate
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*********************************************************************
 * NAME:    BIreadfunc.c                                           
 *                                                                  
 * FUNCTION:     This subroutine reads numeric input and ignores most
 *        non-numeric input.  This is designed to work on all
 *        tty's regardless of the current keyboard   
 *        sofware mapping.                                  
 *                                                               
 * EXECUTION ENVIRONMENT:                                            
 *                                                                 
 *    The readkbd program is a subrouting that can be called from  
 *    a program that requires keyboard input but does not know the 
 *    current keyboard map.                                        
 *                                                                   
 *    Parameters: -- none                                          
 *                                                                   
 *    Syntax:    readkbd();                                           
 *********************************************************************/

#include <string.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/termio.h>
#include <fcntl.h>

int BIttytrminate(int);

/*struct hfunxlate hfunxlate;*/
struct termio setterm, getterm;

char * getenv();
/*********************************************************************
 * BIclear_cmd_line: Clears the command line of previous output before *
 *                      writing out user's response to screen.       *
 *********************************************************************/
BIclear_cmd_line()
{
        write(1, "   ", 3);
        write(1, "\b\b\b", 3);
}

/****************************************************************************
 * readtty: opens tty with NODELAY flag on so that a read will not wait for *
 *    enter.  Then turns off echo and polls tty (using infinite "while")  *
 *    ignoring all but numeric input.                                     *
 ****************************************************************************/
int *BIreadtty(otherchar,bufptr,buflen)
char otherchar;
char *bufptr;
int buflen;
{
  int rc, index, notdone = TRUE;
  char key[80], tmpstr[5];
  static int return_keys[20];
  int numtokens = 0;
  char *ptr;
  int i;

    /*
     * Initialize array which keeps the entered numbers
     */
    key[0] = '\0';

    /*
     * Obtain and save current tty settings.
     */
    rc = ioctl(0, TCGETA, &getterm);
    if (rc < 0)
    {
        printf("TCGETA ioctl system call failed.\n"); 
        return (rc);
    }

    /*
     * Set tty settings to read any character without waiting for <enter>.
     *     setterm = getterm  to get current terminal settings
     *    c_lflag &= ~ICANON turns off canonical processing
     *    c_lflag &= ~ECHO   turns off echo
     *    c_cc[4] = 1        tells read to get only 1 char
     *    c_cc[5] = 0       tells read to not wait for buffer to fill
     */
    setterm = getterm;
    setterm.c_lflag &= ~ICANON;
    setterm.c_lflag &= ~ECHO;
    setterm.c_cc[4] = 1;
    setterm.c_cc[5] = 0;
    rc = ioctl ( 0, TCSETA, &setterm );
    if (rc < 0)
    {
        printf("TCSETA ioctl system call failed.\n");
        BIttytrminate(rc);
        return (rc);
    }

    /*
     * Catch interrupts and call ttytrminate to reset keyboard.
     */
    for (index = SIGHUP; index <= SIGSAK; index++)
        if (index != SIGCHLD)
        signal(index, (void (*)(int))BIttytrminate);

    index = 0;
    /*
     * Read until enter key is pressed.
     */
    while ( notdone )
    {
        tmpstr[0] = '\0';
        rc = read( 0, tmpstr, 1);
        if (rc < 0)
        {
            printf("Read of $CONSOLE failed.\n");
            BIttytrminate(rc);
        } /* if */

        /* do keyboard mapping:
         *  ! == 1
         *  @ == 2 etc
         */
        switch (tmpstr[0])
        {
        case '!':
            tmpstr[0] = '1';
            break;
        case '@':
            tmpstr[0] = '2';
            break;
        case '#':
            tmpstr[0] = '3';
            break;
        case '$':
            tmpstr[0] = '4';
            break;
        case '%':
            tmpstr[0] = '5';
            break;
        case '^':
            tmpstr[0] = '6';
            break;
        case '&':
            tmpstr[0] = '7';
            break;
        case '*':
            tmpstr[0] = '8';
            break;
        case '(':
            tmpstr[0] = '9';
            break;
        case ')':
            tmpstr[0] = '0';
            break;
        }    

        /*
         * Only respond to numeric input, backspace, and enter.
         * Otherwise, beep.
         */
        switch(tmpstr[0])
        {
            case '0' :
            case '1' :
            case '2' :
            case '3' :
            case '4' :
            case '5' :
            case '6' :
            case '7' :
            case '8' :
            case '9' :
            /*
             * clear input line of default info and error msgs
             */
                BIclear_cmd_line();

            /*
             * Concatenate the number to the end of the key array
             * and write out the new number to the screen.
             */
                key[index++] = tmpstr[0];
                write ( 1, tmpstr, 1 );
                break;

            /*
             * On input of backspace, don't decrement key array
             * index below zero and don't back up beyond beginning
             * of input field.
             */
            case '\b' :
                if ( index > 0 )
                {
                    index--;
                    write ( 1, "\b \b", 3 );
                }
                break;

            /*
             * On input of <enter>, set "notdone" to FALSE and write
             * '\n' to screen.
             */
            case '\n' :
		key[index++] = '\0';
                notdone = FALSE;
                write ( 1, "\n", 1 );
                break;

            /*
             * Beep to indicate invalid key was pressed.
             */
            default:
                if((otherchar) && (tmpstr[0] == otherchar))
                {
                    key[index++] = tmpstr[0];
                    write(1, tmpstr, 1);
		    numtokens++;
                }
                else
                    write( 1, "\007", 1);
                break;
        } /* switch */
    } /* while */

    /*
     * Cap off the string with a null charachter.
     */
    key[index] = '\0';

    /*
     * flushing input buffer of tty pointed to by fd
     */
    ioctl ( 0, TCFLSH, 0 );

    /*
     * Reset tty device to previous settings.
     */
    rc = ioctl ( 0, TCSETA, &getterm );
    if (rc < 0)
        printf("TCSETA ioctl system call failed.\n");

    if(bufptr)
        strncpy(bufptr,key,buflen);
    if ( key[0] != '\0' )
    {
	for (ptr = key, i=0; *ptr  ; i++)
	{
	    /* Convert the string to ascii, then scan to next token */
	    return_keys[i] = atoi(ptr);
	    while (!isspace(*ptr) && *ptr )
		ptr++;
	    while (isspace(*ptr) && (*ptr ))
		ptr++;
	}
	return_keys[i] = -1;
    }
    else
        return_keys[0] =  -1 ;
        return return_keys ;
}

/*****************************************************************************
 * BIttytrminate:  called by error routines and upon interrupt in order to clean *
 * up before exiting.                                                        *
 *****************************************************************************/
int
BIttytrminate(int sig)
{
  int  rc;

/* flushing input buffer of tty pointed to by fd */
    ioctl ( 0, TCFLSH, 0 );

/* Reset tty device to previous settings. */
    rc = ioctl ( 0, TCSETA, &getterm );
    if (rc < 0)
        printf("TCSETA ioctl system call failed.\n");

    fprintf(stderr, "\nUnexpected signal:  %d\n", sig);
    exit (-1);
}
