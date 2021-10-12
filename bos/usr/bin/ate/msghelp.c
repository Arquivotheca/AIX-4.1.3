static char sccsid[] = "@(#)20	1.6  src/bos/usr/bin/ate/msghelp.c, cmdate, bos411, 9428A410j 4/18/91 10:57:50";
/* 
 * COMPONENT_NAME: BOS msghelp.c
 * 
 * FUNCTIONS: MSGSTR, msghelp 
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

msghelp(num)
int num;
{

switch(num)
{
case 1 : printf(MSGSTR(HHELP, "\
                                 HELP COMMAND\n\
\n\
TO ENTER A COMMAND: Commands are entered from the  menu  by  typing  the  first\n\
   letter of the command and pressing Enter.  Control keys are typed by holding\n\
   down the control key and typing the indicated letter while the  control  key\n\
   is  still depressed.  You may use ctrl-r at any time to return to a previous\n\
   screen.\n\
\n\
FOR FURTHER HELP INSTRUCTIONS:  For further help  instructions,  use  the  help\n\
   command  followed  by  the  first  letter  of  a command on the connected or\n\
   unconnected main menu (for example, `h m` to get  help  on  modifying  local\n\
   values).  Valid command letters are:\n\
            c (connect)    h (help)     a (alter)    q (quit)\n\
            d (directory)  s (send)     m (modify)   b (break)\n\
            t (terminate)  r (receive)  p (perform)\n\n")); /*MSG*/
break;

case 2 : printf(MSGSTR(HCONN, "\
                                 CONNECT COMMAND\n\
\n\
The connect command allows you to establish a  connection  either  manually  or\n\
modem-dialed.  It also allows you to open a locally-attached port.\n\
\n")); /*MSG*/

printf(MSGSTR(HCONN2, "\
Before you try to make a connection, your modem must be set up properly.   Read\n\
your  modem book to determine how to set its switches.  Some suggested settings\n\
follow:\n\
   The computer supports RS-232 DTR (pin 20)\n\
   No result codes are sent to the computer by the modem\n\
   Allow the computer to detect carrier by reading the RS-232C pin\n\
\n")); /*MSG*/

printf(MSGSTR(HCONN3, "\
If you type 'c' alone, you will be prompted for a phone number  (auto  dialing)\n\
and/or  a port name (direct connect).  If you enter both a telephone number and\n\
a port name, the telephone number must be first.\n\
\n\
MANUAL DIALING:  If you just press Enter after the prompt,  you  will  have  90\n\
   seconds  to  make a manual connection.  If no connection is established, you\n\
   will be returned to the main menu.\n\n")); /*MSG*/
break;

case 3 : printf(MSGSTR(HCONN4, "\
AUTO DIALING:  If you enter a number after the prompt, it will be auto  dialed.\n\
   If no connection has been established in 45 seconds (the line is busy, there\n\
   is no answer, or a wrong number was dialed), you will  be  returned  to  the\n\
   main menu.  Telephone numbers are limited to 40 characters.  They must begin\n\
   with a number, and they may not include spaces.\n\
\n\
OPEN A PORT:  If you enter a port name after  the  prompt,  the  port  will  be\n\
   opened.\n\
\n")); /*MSG*/

printf(MSGSTR(HCONN5, "\
NO PROMPT:  You may avoid prompting by entering a  'c',  a  space,  and  (1)  a\n\
   telephone  number or (2) a port name or (3) a telephone number, a space, and\n\
   a port name.  (Example: c 9,555-1234 tty1).\n\
\n\
CONTROL KEYS:  Once connected, the following control keys may be used:\n\
           ctrl b  start or stop recording display output\n\
           ctrl v  display main menu to issue a command\n\
           ctrl r  return to previous screen at any time\n\n")); /*MSG*/
break;

case 4 : printf(MSGSTR(HDIR, "\
                                DIRECTORY COMMAND\n\
\n\
An already-existing dialing directory file can be  read  by  entering  'd'  and\n\
pressing  Enter.   You will be prompted for a dialing directory file name.  The\n\
default dialing directory is the last one  specified.   If  no  name  has  been\n\
specified,  the  default  directory  name is '/usr/lib/dir'.  You may avoid the\n\
prompt by entering the first letter of the command followed by a space and  the\n\
directory name (example: d mydirectory).\n\
\n")); /*MSG*/

printf(MSGSTR(HDIR2, "\
CREATING A DIRECTORY:  Directories can be created  using  the  editor  of  your\n\
choice.   Items  in  the  directory  are  space separated (1 or more), and must\n\
appear on the line in the following order:\n\
    name             (up to 20 chars; no embedded blanks permitted)\n\
    telephone number (up to 40 chars; no embedded blanks permitted)\n\
    rate             (50,75,110,134,150,300,600,1200,1800,2400,4800,9600,\n\
	 	      19200)\n")); /*MSG*/

printf(MSGSTR(HDIR3, "\
    length           (7,8)\n\
    stop bits        (1,2)\n\
    parity           (0=none, 1=odd, 2=even; Parity must be 0 if Length=8)\n\
    echo             (0=off, 1=on)\n\
    linefeeds        (0=off, 1=on)\n\n")); /*MSG*/
break;

case 5 : printf(MSGSTR(HDIR4, "\
A maximum of 20 entries may be placed in a single directory.  An example  of  a\n\
directory with 3 numbers follows.  The header is provided for this example only\n\
and should not appear in a normal directory file.\n\
\n\
         NAME            NUMBER    RATE LENGTH STOP PARITY ECHO LF's\n\
\n\
         Contest_BBS   9,555-1669  1200    8     1     0     0   1\n\
         Matchmaker    9,555-8747  1200    7     1     1     1   0\n\
         Utility_Board 9,555-3470  1200    7     2     2     0   0\n\n")); /*MSG*/
break;

case 6 : printf(MSGSTR(HMOD, "\
                                MODIFY COMMAND\n\
\n\
The modify command allows you to:\n\
\n\
SPECIFY A CAPTURE FILE:  This is a  file  in  which  to  save  a  copy  of  the\n\
   displayed  output.   If no name is specified, the default name is 'kapture'.\n\
   If the capture file doesn't exist, it will be created.  If  it  does  exist,\n\
   captured  data  will  be appended to the bottom of it.  No capturing is done\n\
   unless the 'Write' value is ON (see below).\n\
\n\
ADD LINEFEEDS:  Have the program add linefeeds  whenever  it  sees  a  carriage\n\
   return.\n\
\n")); /*MSG*/

printf(MSGSTR(HMOD2, "\
ECHO CHARACTERS:  Have the program echo characters typed at the keyboard to the\n\
   display.  Use this option if you see no characters or double characters.\n\
\n\
VT100:  Emulate a Digital Equipment Corporation VT100 terminal.\n\
\n\
WRITE TO CAPTURE FILE:  Write display data to the capture file.  This value may\n\
   also be set during a connection by using 'ctrl b'.\n\n")); /*MSG*/
break;

case 7 : printf(MSGSTR(HMOD3, "\
XON/XOFF:  Turn Xon/Xoff signals on  or  off.   If  on,  signals  indicating  a\n\
   communications  buffer overflow will be sent to and received from the remote\n\
   computer.  The signals are ascii 17 (xon) and ascii 19 (xoff).\n\
\n\
IF YOU USE A DIRECTORY FILE:  The values specified in the  directory  file  for\n\
   echo  and  line  feeds will be used during the connection, and remain as the\n\
   current values after the connection.\n\
\n")); /*MSG*/

printf(MSGSTR(HMOD4, "\
MODIFY COMMANDS:  The modify menu is reached by typing the first letter of  the\n\
   Modify command ('m') at the main menu.  The modify menu may be avoided (fast\n\
   path) by specifying values after the 'm' command.  For example, to name  the\n\
   capture file 'junk' and turn on the write value, type:  m n junk w.\n\n")); /*MSG*/
break;

case 8 : printf(MSGSTR(HALTER, "\
                                  ALTER COMMAND\n\
\n\
The alter command allows you to specify the:\n\
LENGTH:  Set the character length in bits.  The default is 8.\n\
STOP BITS:  Set the number of stop bits sent.  The default is 1.\n\
PARITY:  Set the parity to none, odd or even.  The default is none.\n\
RATE:  Set the bits per second (also called the speed or the baud  rate).   The\n\
   default setting is 1200.\n")); /*MSG*/

printf(MSGSTR(HALTER2, "\
PORT:  Specify the tty device.  The default device is tty0.\n\
PREFIX:  Specify the modem  prefix.   The  default  is  ATDT  (ATtention,  Dial\n\
   Touchtone).  Another common value is ATDP (ATtention, Dial Pulse).  See your\n\
   modem manual to determine what prefix is appropriate.\n\
SUFFIX: Specify the modem suffix.  The default is none.  To reset to no  suffix\n\
   from another string, set the suffix value to 0.\n")); /*MSG*/

printf(MSGSTR(HALTER3, "\
REDIAL WAIT:  Specify the seconds to  wait  between  redialing  attempts.   The\n\
   default is 0 (no delay).\n\
REDIAL ATTEMPTS:  Specify the maximum number of times to redial.   The  default\n\
   is 0 (no redialing will be done).\n\
PACING CHARACTER:  Set the pacing character or time delay.  The default  is  0,\n\
   no delay between sending lines.\n\n")); /*MSG*/
break;

case 9 : printf(MSGSTR(HALTER4, "\
FILE TRANSFER METHOD:  Choose the file transfer method.   This  may  either  be\n\
   pacing or xmodem:\n\
\n\
   PACING operates by sending or receiving a file one line at a time.   If  you\n\
      enter  an  integer  for  the  pacing  character,  it will pause that many\n\
      seconds between transmitting lines.  If you enter a  character,  it  will\n\
      wait  for  the  receiving computer to send this prompt before sending the\n\
      next line.  In the receive mode, pacing will send the prompt character to\n\
      the remote computer after receiving a carriage return.\n\
   XMODEM is a protocol that  sends  or  receives data a block at a time, and\n\
      provides for error checking.\n\
\n")); /*MSG*/

printf(MSGSTR(HALTER6, "\
IF YOU USE A DIRECTORY FILE:  The values specified in the  directory  file  for\n\
   length,  stop  bits, parity and rate will be used during the connection, and\n\
   remain as the current values after the connection.\n\
\n\
ALTER COMMANDS: The alter menu is reached by typing the  first  letter  of  the\n\
   Alter  command  ('a') at the main menu.  The alter menu may be avoided (fast\n\
   path) by specifying values after the 'a' command.  For example, to  set  the\n\
   rate to 300 and the parity to even, type:  a r 300 p 2.\n\n")); /*MSG*/
break;

case 10 : printf(MSGSTR(HPERF, "\
                                 PERFORM COMMAND\n\
\n\
The Perform command is issued from the menu by typing the first letter  of  the\n\
command,  'p'.  You will be prompted to enter an operating system command.  You\n\
may avoid the prompt by entering the first letter of the command followed by  a\n")); /*MSG*/

printf(MSGSTR(HPERF2, "\
space and the operating system command.  For example, to see a list of files in\n\
the current directory, enter 'p li'.  After the operating  system  command  has\n\
been  executed,  'Press Enter' will be displayed.  Enter will return you to the\n\
program.\n\n")); /*MSG*/
break;

case 11 : printf(MSGSTR(HQUIT, "\
                                 QUIT COMMAND\n\
\n\
The quit command exits the program and returns you to the operating system.\n\n")); /*MSG*/
break;

case 12 : printf(MSGSTR(HSEND, "\
                                 SEND COMMAND\n\
\n\
The Send command is available only when a connection has been established.   It\n\
is  used  to  send  a  file to a remote computer.  Two methods are provided for\n\
sending a file:  pacing and xmodem protocol.  You specify which method you wish\n\
to use by using the Transfer and Character commands on the Alter menu.\n\
\n")); /*MSG*/

printf(MSGSTR(HSEND2, "\
PACING operates by sending a file to the remote computer one line  at  a  time.\n\
   If  you  enter  an integer for the pacing character, it will pause that many\n\
   seconds between transmitting lines.  If you enter a character, it will  wait\n\
   for the receiving computer to send this prompt before sending the next line.\n\
XMODEM is a protocol developed by Ward Christensen.  It sends or receives  data\n\
   a  block  at  a  time, and provides for error checking.  The remote computer\n\
   must also have the capability to use xmodem.\n\
\n")); /*MSG*/

printf(MSGSTR(HSEND3, "\
SEND COMMAND: To use the Send command, enter the first letter  of  the  command\n\
   ('s')  and  press  Enter.   You will be prompted for the name of the file to\n\
   send.  The file must exist.  You may avoid the prompt by entering the  first\n\
   letter  of  the command followed by a space and the name of the file to send\n\
   (example:  s send_file).\n\n")); /*MSG*/
break;

case 13 : printf(MSGSTR(HRECV, "\
                                RECEIVE COMMAND\n\
\n\
The Receive command is available only when a connection has  been  established.\n\
It  is used to receive a file from a remote computer.  Two methods are provided\n\
for receiving a file:  pacing and xmodem protocol.  You  specify  which  method\n\
you wish to use by using the Transfer and Character commands on the Alter menu.\n\
\n")); /*MSG*/

printf(MSGSTR(HRECV2, "\
PACING operates by receiving a file from a  remote computer one line at a time.\n\
   If  the pacing character is not an integer, the pacing routine will send the\n\
   character to the remote computer after each carriage return it receives.\n\
XMODEM is a protocol developed by Ward Christensen.  It sends or receives  data\n\
   a  block  at  a  time, and provides for error checking.  The remote computer\n\
   must also have the capability to use xmodem.\n\
\n")); /*MSG*/

printf(MSGSTR(HRECV3, "\
RECEIVE COMMAND:  To use the Receive command, enter the  first  letter  of  the\n\
   command  ('r')  and  press  Enter.  You will be prompted for the name of the\n\
   file to receive.  You may avoid the prompt by entering the first  letter  of\n\
   the  command  followed  by  a  space  and  the  name  of the file to receive\n\
   (example: r receive_file).  If the file does not exist, it will be  created.\n\
   If it does exist, the received data will be appended to the bottom of it.\n\n")); /*MSG*/
break;

case 14 : printf(MSGSTR(HTERM, "\
                               TERMINATE COMMAND\n\
\n\
The Terminate command is used to end a connection.  It is available only when a\n\
connection  has  been established.  It is issued from the main menu (type 'ctrl\n\
v' while connected to display the main menu, enter 't' and  press  Enter).   It\n\
disconnects  the modem, closes the port, and returns you to the main menu or to\n\
the directory menu, depending on how you established the connection.\n\n")); /*MSG*/
break;

case 15 : printf(MSGSTR(HBRK, "\
                                 BREAK COMMAND\n\
\n\
The Break command is used to send a break signal to the remote computer.  It is\n\
available  only  when a connection has been established, and is issued from the\n\
main menu (type 'ctrl v' while connected to display the main  menu,  enter  'b'\n\
and press Enter).\n\n")); /*MSG*/
break;

default:  printf(MSGSTR(HDEFLT, "No such help as %d\n\n"),num); /*MSG*/
          break;

}  /* end of switch */
}  /* end of program */

