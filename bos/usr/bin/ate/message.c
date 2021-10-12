static char sccsid[] = "@(#)40	1.11  src/bos/usr/bin/ate/message.c, cmdate, bos411, 9435B411a 8/30/94 11:29:37";
/* 
 * COMPONENT_NAME: BOS message.c
 * 
 * FUNCTIONS: MSGSTR, message 
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


#include <nl_types.h>
#include "ate_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ATE,n,s) 

/****************************************************************************/
/*                                                                          */
/* Module name:      message.c                                              */
/* Description:      print user messages, record RAS errors in error log    */
/* Functions:        message - display user messages                        */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      alter in alter.c                                       */
/*                   ckentry, direc, readfile in directory.c                */
/*                   ckkey, cklength, ckpacing, ckparity, ckrate, ckredial, */
/*                     ckstop, cktoggle, cktransfer in check.c              */
/*                   cmd in command.c                                       */
/*                   conn, digits in connect.c                              */
/*                   getdef, opendf, openfk in main.c                       */
/*                   hangup in hangup.c                                     */
/*                   help in help.c                                         */
/*                   modify in modify.c                                     */
/*                   portinit, portread in portrw.c                         */
/*                   prcv, psend in pacing.c                                */
/*                   sigrout in signal.c                                    */
/*                   vt2 in portvt.c                                        */
/*                   xrcv in xmodem.c                                       */
/*                   portrw.c and xshell.c each have their own message      */
/*                     functions internal to the modules to save space,     */
/*                     although the messages are identical among all 3      */
/*                   two message cross-reference lists (by number and by    */
/*                     module) are available in the documentation text      */
/*   Receives:       integer number of message to print                     */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          message services in the run time library               */
/*                   errlog RAS error logging in the run time library      */
/*                   cls in cls.c (clear screen)                            */
/*   Modifies:       nothing                                                */
/*                                                                          */
/****************************************************************************/
#include <sys/erec.h>
#include <sys/errids.h>
#include "modem.h"

static char cret = '\r';                     /* carriage return */

int msgvi1;
char *msgvc1;

/* --------------------------------------------------------------------------
   messages:  Receive message number to be printed from calling routine; call
   message services to display message.  If necessary, also log an error into
   the error log.
   
   Print=1 causes "Press Enter" to be displayed to give the user time to read
   the message.  Print=0 simply displays the message and returns to the calling
   routine.

   msgvc1 and msgvi1 are used to pass variables to the message services
   routine.  msgvc1 (msg variable character one) is initialized to argp[25],
   a global variable which is usually used by the calling routine to pass
   a value to message().  msgvi1 (msg variable integer one) is initialized
   to sectnum.  These two variables are reset only when the default values
   can't be used.
   
   Error logging is done only during file transfers (pacing and xmodem).

   Messages 19, 25 and 33, still in the code, are never called.
   -------------------------------------------------------------------------- */
message(num)
int num;
{
int print=1;
int errlen, rc;
int errlog();                               /* error logging routine */
struct termios oldmode;                     /* saved terminal mode */
struct termios newmode;                     /* changed terminal mode */
struct errbuf                               /* ras buffer */
  {
	struct err_rec0 err_ate;
	char	err_str[ERR_REC_MAX] ;
  } ras;
extern void sigrout();
 
#ifdef DEBUG
kk = sprintf(ss,"Entering msg: num=%d\n",num);
write(fe,ss,kk);
#endif DEBUG
 strcpy(ras.err_ate.resource_name,"ATE");
 errlen = sizeof(struct err_rec0) ; 
 
 msgvc1 = argp[25];                          /* pointer to msg char variable */
 msgvi1 = sectnum;                           /* pointer to msg int variable */
 if ((rc = tcgetattr (2, &oldmode)) != 0)    /* save current tty mode */
	exit(1);
 if ((rc = tcgetattr (2, &newmode)) != 0)
	exit(1);
 newmode.c_oflag |= OPOST;                   /* turn on output post-proc */
 signal(SIGHUP,(void(*)(int)) SIG_IGN);        /* ignore hangup */
 if ((rc = tcsetattr (2, TCSANOW, &newmode)) != 0)
	exit(1);
 signal(SIGHUP,(void(*)(int)) sigrout);        /* enable hangup */

 switch (num)
   {
    case  1 : /* This message is in portrw.c only--placed here for reference */
              cls();                         /* clear screen */
              msgvc1 = kapfile;              /* pass capture file name */
			  printf(MSGSTR(MSG1, "ate: 0828-001 The Name command cannot create or open the\n             %s file\n             because of the error shown below.\n"), msgvc1); /*MSG*/
              perror(MSGSTR(NAME, "Name"));               /* print system error */ /*MSG*/
              break;
              
    case  2 : printf(MSGSTR(MSG2, "ate: 0828-002 The Receive command is ending because the sending site\n             has not sent anything for %d seconds.\n"), msgvi1); /*MSG*/
              ras.err_ate.error_id = ERRID_ATE_ERR5 ;       /* ate error # */
	      sprintf(ras.err_str,"The sending site has not sent anything for %d seconds. \n",msgvi1);
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              break;
              
    case  3 : cls();
			  printf(MSGSTR(MSG3, "ate: 0828-003 The %s command is not valid.\n\
              Enter the first letter of a command from the list on the menu.\n"),
			  msgvc1);
              break;
              
    case  4 : cls();
			  printf(MSGSTR(MSG4, "ate: 0828-004 A required temporary file cannot be opened because of the\n\
              error shown below.  Use the Quit command to end the program,\n\
              then start the program again.\n"));
              perror(MSGSTR(CONN, "Connect")); /*MSG*/
              break;

    case  5 : /* This message is only in xshell.c only--placed here for reference */
              msgvc1 = rcvfile;
			  printf(MSGSTR(MSG5, "ate: 0828-005 The system is ready to receive file %s.\r\n\
              Use Ctrl-X to stop xmodem.\r\n"), msgvc1); /*MSG*/
              sleep(5);
              break;
              
    case  6 : cls();
			  printf(MSGSTR(MSG6, "ate: 0828-006 The Connect command cannot start the process to read the port\n\
              because of the error shown below.\n")); /*MSG*/
              perror(MSGSTR(CONN, "Connect")); /*MSG*/
              break;
              
    case  7 : cls();
			  printf(MSGSTR(MSG7, "ate: 0828-007 Please dial a telephone number to make the requested connection.\n")); /*MSG*/
              print=0;
              break;
              
    case  8 : cls();
			  printf(MSGSTR(MSG8, "ate: 0828-008 The system tried to open port %s\n\
              but failed.  If the port name is not correct, change it using the\n\
              Alter menu.  Or, take the action indicated by the system message\n\
              shown below.\n"), msgvc1);
              perror(MSGSTR(CONN, "Connect")); /*MSG*/
              break;
              
    case  9 : cls();
			  printf(MSGSTR(MSG9, "ate: 0828-009 The Connect command cannot complete because the line was busy,\n\
              or the modem did not detect a carrier signal.  Make sure the number\n\
              is correct and try again, or try the same number later.\n"));
              break;
              
    case 10 : cls();
              msgvc1 = port;
			  printf(MSGSTR(MSG10, "ate: 0828-010 The Connect command has made a connection through port %s.\n"), msgvc1); /*MSG*/
              print=0;
              break;

    case 11 : cls();
			  printf(MSGSTR(MSG11, "ate: 0828-011 The default file ate.def cannot be created or opened because\n\
              of the error shown below.  The program is continuing with standard\n\
              defaults.\n")); /*MSG*/
              perror(MSGSTR(ATE, "ate")); /*MSG*/
              break;
              
    case 12 : cls();
              msgvc1 = dirfile;
			  printf(MSGSTR(MSG12, "ate: 0828-012 The Directory command cannot open the\n\
              %s file\n\
              because of the error shown below:\n"), msgvc1); /*MSG*/
              perror(MSGSTR(DIR, "Directory")); /*MSG*/
              break;
              
    case 13 : cls();
              msgvc1 = userin;
			  printf(MSGSTR(MSG13, "ate: 0828-013 The number %s that you entered\n\
              is not a valid directory entry number.  Enter a number \n\
              from the directory list.\n"), msgvc1); /*MSG*/
              break;
              
    case 14 : cls();
			  printf(MSGSTR(MSG14, "ate: 0828-014 The letter %s that you entered is\n\
              not a valid Help option.  Enter Help followed by the\n\
              first letter of a valid command.\n"), msgvc1); /*MSG*/
              break;
              
    case 15 : printf(MSGSTR(MSG15, "ate: 0828-015 The file transfer is complete.\n")); /*MSG*/
              break;
              
    case 16 : /* This message is only in xshell.c only--placed here for reference */
              msgvc1 = sndfile;
			  printf(MSGSTR(MSG16, "ate: 0828-016 The system is ready to send file %s.\r\n\
              Use Ctrl-X to stop xmodem.\r\n"), msgvc1); /*MSG*/
              sleep(5);
              break;
              
    case 17 : cls();
              msgvc1 = rcvfile;
			  printf(MSGSTR(MSG17, "ate: 0828-017 The Receive command cannot create or open the\r\n\
              %s file\r\n\
              because of the error shown below.\r\n"), msgvc1); /*MSG*/
              perror(MSGSTR(RECV, "Receive")); /*MSG*/
              break;
              
    case 18 : cls();
			  printf(MSGSTR(MSG18, "ate: 0828-018 The Receive command cannot complete because you did not\r\n\
              specify an input file name.\r\n")); /*MSG*/
              break;
              
    case 19 : /* This message is never called */
			  printf(MSGSTR(MSG19, "ate: 0828-019 The file transfer cannot begin because no pacing character\n\
              has been received for 100 seconds.  Verify that both the sending\n\
              and receiving user specified the same pacing character in the\n\
              Character command.\n"));
              ras.err_ate.error_id = ERRID_ATE_ERR1;
	      sprintf(ras.err_str,"No pacing character has been received for 100 seconds \n");
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              break;
              
    case 20 : cls();
              msgvc1 = rcvfile;
			  printf(MSGSTR(MSG20, "ate: 0828-020 The program is ready to receive file %s.\n\
              You will receive another message when the file transfer is complete.\n"),msgvc1);
              print=0;
              break;
              
    case 21 : printf(MSGSTR(MSG21, "ate: 0828-021 The file transfer cannot continue because no pacing character\n\
              has been received for 30 seconds.  Verify that both the sending\n\
              and receiving user specified the same pacing character in the\n\
              Character command.\n"));
              ras.err_ate.error_id = ERRID_ATE_ERR1 ;
	      sprintf(ras.err_str,"No pacing character has been received for 30 seconds \n");
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              break;
              
    case 22 : cls();
              msgvc1 = sndfile;
			  printf(MSGSTR(MSG22, "ate: 0828-022 The Send command cannot open the\r\n\
              %s file\r\n\
              because of the error shown below.\r\n"), msgvc1); /*MSG*/
              perror(MSGSTR(SEND, "Send")); /*MSG*/
              break;
              
    case 23 : cls();
			  printf(MSGSTR(MSG23, "ate: 0828-023 The Send command cannot complete because you did not\r\n\
              specify an output file name.\r\n")); /*MSG*/
              break;
              
    case 24 : cls();
              msgvc1 = sndfile;
			  printf(MSGSTR(MSG24, "ate: 0828-024 The program is ready to send file %s.\n\
              You will receive another message when the file transfer is complete.\n"), msgvc1); /*MSG*/
              print=0;
              break;
              
    case 25 : /* This message is never called */
			  printf(MSGSTR(MSG25, "ate: 0828-025 The system is sending block %d.\n"), msgvi1); /*MSG*/
              print=0;
              break;
              
    case 26 : /* This message is in portrw.c only--placed here for reference */
              cls();
              msgvc1 = kapfile;
			  printf(MSGSTR(MSG26, "ate: 0828-026 Data is no longer being captured, and the current capture data\n\
              has been lost.  The capture buffer is full and cannot be written\n\
              to the %s file\n\
              because of the reason shown below.\n"), msgvc1);
              perror(MSGSTR(WRITE, "Write")); /*MSG*/
              break;
              
    case 27 : msgvc1 = rcvfile;
	    		  printf(MSGSTR(MSG27, "ate: 0828-027 The Receive command cannot write to the\n\
              %s file\n\
              because of the error shown below.\n"), msgvc1); /*MSG*/
              perror(MSGSTR(RECV, "Receive")); /*MSG*/
              break;
              
    case 28 : printf(MSGSTR(MSG28, "ate: 0828-028 The system is receiving block %d.\n"), msgvi1); /*MSG*/
              write(1,&cret,1);
              print=0;
              break;
              
    case 29 : printf(MSGSTR(MSG29, "ate: 0828-029 The file transfer is active, but a checksum error occurred on\n\
              sector %d.  You do not need to take any action at this time.\n"), msgvi1); /*MSG*/
              ras.err_ate.error_id = ERRID_ATE_ERR7 ;
	      sprintf(ras.err_str,"Checksum error occurred on sector %d.\n",msgvi1);
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              print=0;
              break;
              
    case 30 : printf(MSGSTR(MSG30, "ate: 0828-030 The file transfer is active, but sector %s was received twice.\n\
              The duplicate sector has been discarded.\n"), msgvc1); /*MSG*/
              ras.err_ate.error_id = ERRID_ATE_ERR8 ;
	      sprintf(ras.err_str,"Sector %s was received twice, the duplicate sector has been discarded\n",msgvc1);
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              print=0;
              break;
              
    case 31 : printf(MSGSTR(MSG31, "ate: 0828-031 The file transfer is active, but sector %s was received when\n\
              sector %d was expected.  The sector has been requested again.\n"), msgvc1, msgvi1); /*MSG*/
              ras.err_ate.error_id = ERRID_ATE_ERR9 ;
	      sprintf(ras.err_str,"Sector %s was received when sector %d was expected\n",msgvc1,msgvi1);
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              print=0;
              break;
              
    case 32 : printf(MSGSTR(MSG32, "ate: 0828-032 The file transfer is active, but sector number %d could not be\n\
              verified as being correct.  The sector has been requested again.\n"), msgvi1); /*MSG*/
              ras.err_ate.error_id = ERRID_ATE_ERR10 ;
	      sprintf(ras.err_str,"Sector number %d could not be verified as being correct, The sector has been requested again.\n",msgvi1);
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              print=0;
              break;
              
    case 33 : /* This message is never called */
              cls();
			  printf(MSGSTR(MSG33, "ate: 0828-033 Data is no longer being captured, and the current capture data\n\
              has been lost.  The process to write the capture buffer to the\n\
              capture file cannot be started because of the reason shown below.\n"));
              perror(MSGSTR(WRITE, "Write")); /*MSG*/
              break;
              
    case 34 : printf(MSGSTR(MSG34, "ate: 0828-034 The file transfer cannot complete because too many transmission\n\
              errors have occurred. Use Connect command to re-establish the\n\
              connection.  Then try to transfer the file again.\n"));
              ras.err_ate.error_id = ERRID_ATE_ERR2 ;
	      sprintf(ras.err_str,"Too many transmission errors.\n");
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              break;
              
    case 35 : printf(MSGSTR(MSG35, "ate: 0828-035 The Send command cannot complete because the receiving site\n\
              has not indicated it is ready to receive.  Make sure the receiving\n\
              site is using the same communication protocol, then send the file\n\
              again.\n"));
              ras.err_ate.error_id = ERRID_ATE_ERR4 ;
	      sprintf(ras.err_str,"Receiving site has not indicated it is ready to receive\n");
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              break;
              
    case 36 : msgvc1 = sndfile;
			  printf(MSGSTR(MSG36, "ate: 0828-036 The Send command cannot read the\n\
              %s file\n\
              because of the error shown below.\n"), msgvc1); /*MSG*/
              perror(MSGSTR(SEND, "Send")); /*MSG*/
              break;
              
    case 37 : msgvi1 = sectnum - 1;
			  printf(MSGSTR(MSG37, "ate: 0828-037 The file transfer cannot complete because the receiving site did\n\
              not acknowledge receipt of sector %d.  Make sure the receiving\n\
              site is using the same communication protocol, and send the file\n\
              again.\n"), msgvi1);
              ras.err_ate.error_id = ERRID_ATE_ERR3 ;
	      strcpy(ras.err_str,"Receving  site did not acknowledge");
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              break;
              
    case 38 : printf(MSGSTR(MSG38, "ate: 0828-038 The file transfer cannot complete because the receiving site did not\n\
              acknowledge end of transmission.  Make sure the receiving site is\n\
              using the same communication protocol, and send the file again.\n"));
              ras.err_ate.error_id = ERRID_ATE_ERR3 ;
	      sprintf(ras.err_str,"Receiving site did not acknowledge end of transmission.\n");
	      errlen = errlen + strlen(ras.err_str);
              errlog((char *)&ras,errlen);
              break;
              
    case 39 : /* This message is only in xshell.c only--placed here for reference */
			  printf(MSGSTR(MSG39, "ate: 0828-039 The xmodem command cannot complete because a flag is not valid.\r\n\
              Valid flags are -r for receive, -s for send, or -p for pass-\r\n\
              through.  Enter the command again using one of these flags.\r\n"));
              break;
	
	case 40 : break;
              
    case 41 : printf(MSGSTR(MSG41, "ate: 0828-041 More.  Press Enter.\n")); /*MSG*/
              print=0;
		      read(0,&kbdata,1);
              break;
              
    case 42 : cls();
			  printf(MSGSTR(MSG42, "ate: 0828-042 The value %s specified for the Attempts command is not valid.\n\
              Possible choices include any integer greater than zero.\n"), msgvc1); /*MSG*/
              break;
              
    case 43 : cls();
			  printf(MSGSTR(MSG43, "ate: 0828-043 The value %s specified for the Rate command is not valid.\n\
              Possible choices are 50, 75, 110, 134, 150, 300, 600,\n\
              1200, 1800, 2400, 4800, 9600 and  19200.\n"), msgvc1);
              break;
              
    case 44 : cls();
			  printf(MSGSTR(MSG44, "ate: 0828-044 The value %s specified for the Length command is not valid.\n\
              Possible choices are 7 and 8.\n"), msgvc1); /*MSG*/
              break;
              
    case 45 : cls();
			  printf(MSGSTR(MSG45, "ate: 0828-045 The value %s specified for the Stop command is not valid.\n\
              Possible choices are 1 and 2.\n"), msgvc1); /*MSG*/
              break;
              
    case 46 : cls();
			  printf(MSGSTR(MSG46, "ate: 0828-046 The value %s specified for the Parity command is not valid.\n\
              Possible choices are 0 for none, 1 for odd, and 2 for even.\n"), msgvc1); /*MSG*/
			  break;
              
    case 47 : cls();
			  printf(MSGSTR(MSG47, "ate: 0828-047 The value %s specified for the Echo command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"), msgvc1); /*MSG*/
              break;
              
    case 48 : cls();
			  printf(MSGSTR(MSG48, "ate: 0828-048 The value %s specified for the Linefeeds command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"), msgvc1); /*MSG*/
              break;
              
    case 49 : cls();
			  printf(MSGSTR(MSG49, "ate: 0828-049 Correct the entry in your dialing directory and try again,\n\
              or use the Connect command to make your connection.\n")); /*MSG*/
              break;
              
    case 50 : cls();
			  printf(MSGSTR(MSG50, "ate: 0828-050 The value %s you specified for the Wait command is not valid.\n\
              Possible choices include any integer greater than or equal to zero.\n"), msgvc1); /*MSG*/
              break;
              
    case 51 : cls();
			  printf(MSGSTR(MSG51, "ate: 0828-051 The value %s you specified for the Transfer command is not valid.\n\
              Possible choices are p for pacing and x for xmodem.\n"), msgvc1); /*MSG*/
              break;
              
    case 52 : cls();
			  printf(MSGSTR(MSG52, "ate: 0828-052 The value %s you specified for the Character command is not valid.\n\
              Possible choices include any single character or integer.\n"), msgvc1); /*MSG*/
              break;
              
    case 53 : cls();
			  printf(MSGSTR(MSG53, "ate: 0828-053 The value %s specified for the VT100 command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"), msgvc1); /*MSG*/
              break;

    case 54 : cls();
			  printf(MSGSTR(MSG54, "ate: 0828-054 The system will use a parity of 0 because a length of 8 was\n\
              specified.\n")); /*MSG*/
              break;

    case 55 : cls();
			  printf(MSGSTR(MSG55, "ate: 0828-055 The value %s specified for the Write command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"), msgvc1); /*MSG*/
              break;

    case 56 : cls();
			  printf(MSGSTR(MSG56, "ate: 0828-056 The session has been disconnected because the system can no longer\n\
              detect a carrier signal.\n")); /*MSG*/
              break;
              
    case 57 : cls();
              msgvc1 = dirfile;
			  printf(MSGSTR(MSG57, "ate: 0828-057 The dialing directory %s \n\
              has more than 20 entries.  Only the first 20 entries can\n\
              be used.\n"), msgvc1); /*MSG*/
              break;

    case 58 : cls();
			  printf(MSGSTR(MSG58, "ate: 0828-058 The value %s specified for the Xon/Xoff command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"), msgvc1); /*MSG*/
              break;

    case 59 : cls();
			  printf(MSGSTR(MSG59, "ate: 0828-059 The value %s specified for a control key is not valid.\n\
              Possible choices include any integer greater than or equal to zero\n\
              and less than 32 decimal (040 octal, 0x20 hex).\n"), msgvc1);
              break;

    case 60 : cls();
			  printf(MSGSTR(MSG60, "ate: 0828-060 The system cannot open port %s because it is\n\
              enabled or in use.\n"), msgvc1); /*MSG*/
              break;

    case 61 : cls();
			  printf(MSGSTR(MSG61, "ate: 0828-061 The system cannot open port %s because the port \n\
              is busy.\n"), msgvc1); /*MSG*/
              break;
              
    case 62 : cls();
			  printf(MSGSTR(MSG62, "ate: 0828-062 The system cannot complete the Perform command requested\n\
              because of the error shown below.\n")); /*MSG*/
              perror(MSGSTR(PERF, "Perform"));               /* print system error */
              break;
              
    default : printf(MSGSTR(MSGUNK, "Message number %d is not in messages.c\r\n"),num); /*MSG*/
              break;
   }
   
 if (print) 
   {
   printf(MSGSTR(MSG40, "ate: 0828-040 Press Enter.\n"));		 /* "Press Enter" */ /*MSG*/
   read(0,&kbdata,1);
   }

 tcsetattr (2, TCSANOW, &oldmode);           /* restore entry-time TTY mode */
 retcode=(int)NULL;                          /* In case user types ctrl-r */
                                             /*   instead of "Press Enter" */
 
#ifdef DEBUG
kk = sprintf(ss,"Leaving message\n");
write(fe,ss,kk);
#endif DEBUG

}
