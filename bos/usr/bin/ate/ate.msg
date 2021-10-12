$ @(#)28        1.6.2.5  src/bos/usr/bin/ate/ate.msg, cmdate, bos411, 9437A411a 8/30/94 13:51:00
$
$ COMPONENT_NAME: BOS ate.msg
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for ate
$
$ Do not translate the word message.c.
$
$quote "

$set MS_ATE

CONN            "Connect"
$      Do not translate. This is a command (from main menu).
DIR             "Directory"
$      Do not translate. This is a command (from main menu).
SEND            "Send"
$      Do not translate. This is a command (from main menu).
RECV            "Receive"
$      Do not translate. This is a command (from main menu).
BRK             "Break"
$      Do not translate. This is a command (from main menu).
TERM            "Terminate"
$      Do not translate. This is a command (from main menu).
HELP            "Help"
$      Do not translate. This is a command (from main menu).
MOD             "Modify"
$      Do not translate. This is a command (from main menu).
ALT             "Alter"
$      Do not translate. This is a command (from main menu).
PERF            "Perform"
$      Do not translate. This is a command (from main menu).
MQUIT           "Quit"
$      Do not translate. This is a command (from main menu).


DCONN           "Make a connection\n"
$  This and the next 10 messages are the description of the previous
$  commands. They may be translated.
DDIR            "Display a dialing directory.\n"
DSEND           "Send a file over the current connection.\n"
DRECV           "Receive a file over the current connection.\n"
DBRK            "Send a break signal over the current connection.\n"
DTERM           "Terminate the connection.\n"
DHELP           "Get help and instructions.\n"
DMOD            "Modify local settings.\n"
DALT            "Alter connection settings.\n"
DPERF           "Perform an Operating System command.\n"
DQUIT           "Quit the program.\n"


PHNUM           "Type the phone number of the connection for auto dialing, or the name\n\
of the port for direct connect, and press Enter.  To manually dial a\n\
number, just press Enter.  To redial the last number (%s), type 'r'\n\
and press Enter.\n"
$ This and the next 9 messages are messages for prompts.
DIRNAME         "Type the file name of the directory you want to display and press Enter.\n"
DIRNAME2        "To use the current directory (%s), just press Enter.\n"
OSCMD           "Type an operating system command and press Enter.\n"
FNAME           "Type the name of the file you wish to send and press Enter.  To use\n\
the last file name (%s), just press Enter.\n"
FNAMES          "Type the name of the file you wish to store the received data in and\n\
press Enter.  To use the last file name (%s), just\n\
press Enter.\n"
KFILE           "Type the name of the file you wish to store the captured data in and\n\
press Enter.  To use the last file name (%s), just\n\
press Enter.\n"
INVPR           "The prompt number %d passed to the prompt routine is not valid.\n"
KEYS            "     The following keys can be used during a connection:\n\
         ctrl %c Start or stop recording display output.\n\
         ctrl %c Display main menu to issue a command.\n\
         ctrl %c Return to a previous screen at any time.\n"
$ Leave 5 blank spaces at the start of the message.
$ Do not translate ctrl %c.
$ The text following %c must start in the 18th column.
NODE            "\nNode: "

$               menu header information

MAINH2          "          CONNECTED MAIN MENU\n"
$ Start in Column 11.
MAINH1          "          UNCONNECTED MAIN MENU\n"
$ Start in Column 11.
ALTERH          "          ALTER CONNECTION SETTINGS\n"
$ Start in Column 11.
MODH            "          MODIFY LOCAL SETTINGS\n"
$ Start in Column 11.
LINE            "-------------------------------------------------------------------------------\n"
$ Do not translate.
CMMD            "     COMMAND      DESCRIPTION\n     -------      -----------------------------------------------\n"
$ Retain the ---- in the exact same columns.  The translation for
$ DESCRIPTION must start in the 19th column.
$ Center your translation for COMMAND in the first 18 columns.
$ Increase or decrease your dashes in the 2nd line to correspond.
ENTER1          "Type the first letter of the command and press Enter.\n> "
ENTER2          "To change a current choice, type the first letter of the command followed by\nyour new choice (example:  r 300) and press Enter.\n> "
ENTER3          "To change a value, type the first letter of the command and press Enter.\n> "

$ $             messages in message.c file; the number following MSG
$ $             indicates the message number in the call to message,
$ $             e.g. MSG37 is the identifier for the message printed
$ $             by the call 'message(37)'

MSG1            "ate: 0828-001 The Name command cannot create or open the\n\
              file %s\n\
              because of the error shown below.\n"
$ Do not translate Name.
NAME            "Name"
$ Do not translate.
MSG2            "ate: 0828-002 The Receive command is ending because the sending site\n\
              has not sent anything for %d seconds.\n"
$ Do not translate Receive
MSG5            "ate: 0828-005 The system is ready to receive file %s.\r\n\
              Use ctrl-X to stop xmodem.\r\n"
$ Do not translate ctrl-X, xmodem.
MSG6            "ate: 0828-006 The Connect command cannot start the process to read the port\n\
              because of the error shown below.\n"
$ Do not translate Connect
MSG7            "ate: 0828-007 Please dial a telephone number to make the requested connection.\n"
MSG10           "ate: 0828-010 The Connect command has made a connection through port %s.\n"
$ Do not translate Connect
MSG11           "ate: 0828-011 The default file ate.def cannot be created or opened because\n\
              of the error shown below.  The program is continuing with standard\n\
              defaults.\n"
$ Do not translate ate.def
ATE             "ate"
$ Do not translate ate
MSG12           "ate: 0828-012 The Directory command cannot open the\n\
              file %s\n\
              because of the error shown below.\n"
$ Do not translate Directory
MSG13           "ate: 0828-013 The number %s that you entered\n\
              is not a valid directory entry number.  Enter a number \n\
              from the directory list.\n"
MSG14           "ate: 0828-014 The letter %s that you entered is\n\
              not a valid Help option.  Enter 'Help' followed by the\n\
              first letter of a valid command.\n"
$ Do not translate Help
MSG15           "ate: 0828-015 The file transfer is complete.\n"
MSG16           "ate: 0828-016 The system is ready to send file %s.\r\n\
              Use ctrl-X to stop xmodem.\r\n"
$ Do not translate ctrl-X, xmodem
MSG17           "ate: 0828-017 The Receive command cannot create or open the\r\n\
              file %s\r\n\
              because of the error shown below.\r\n"
$ Do not translate Receive
MSG18           "ate: 0828-018 The Receive command cannot complete because you did not\r\n\
              specify an input file name.\r\n"
$ Do not translate Receive
MSG20           "ate: 0828-020 The program is ready to receive file %s.\n\
              You will receive another message when the file transfer is complete.\n"
MSG22           "ate: 0828-022 The Send command cannot open the\r\n\
              file %s\r\n\
              because of the error shown below.\r\n"
$ Do not translate Send
MSG23           "ate: 0828-023 The Send command cannot complete because you did not\r\n\
              specify an output file name.\r\n"
$ Do not translate Send
MSG24           "ate: 0828-024 The program is ready to send file %s.\n\
              You will receive another message when the file transfer is complete.\n"
MSG25           "ate: 0828-025 The system is sending block %d.\n"
WRITE           "Write"
$ Do not translate
MSG27           "ate: 0828-027 The Receive command cannot write to the\n\
              file %s\n\
              because of the error shown below.\n"
$ Do not translate Receive
MSG28           "ate: 0828-028 The system is receiving block %d.\n"
MSG29           "ate: 0828-029 The file transfer is active, but a checksum error occurred on\n\
              sector %d.  You do not need to take any action at this time.\n"
MSG30           "ate: 0828-030 The file transfer is active, but sector %s was received twice.\n\
              The duplicate sector has been discarded.\n"
MSG31           "ate: 0828-031 The file transfer is active, but sector %1$s was received when\n\
              sector %2$d was expected.  The sector has been requested again.\n"
MSG32           "ate: 0828-032 The file transfer is active, but sector number %d could not be\n\
              verified as being correct.  The sector has been requested again.\n"
MSG36           "ate: 0828-036 The Send command cannot read the\n\
              file %s\n\
              because of the error shown below.\n"
$ Do not translate Send
MSG41           "ate: 0828-041 More.  Press Enter.\n"
MSG42           "ate: 0828-042 The value %s specified for the Attempts command is not valid.\n\
              Possible choices include any integer greater than zero.\n"
$ Do not translate Attempts
MSG44           "ate: 0828-044 The value %s specified for the Length command is not valid.\n\
              Possible choices are 7 and 8.\n"
$ Do not translate Length
MSG45           "ate: 0828-045 The value %s specified for the Stop command is not valid.\n\
              Possible choices are 1 and 2.\n"
$ Do not translate Stop
MSG46           "ate: 0828-046 The value %s specified for the Parity command is not valid.\n\
              Possible choices are 0 for none, 1 for odd, and 2 for even.\n"
$ Do not translate Parity
MSG47           "ate: 0828-047 The value %s specified for the Echo command is not valid.\n\
              Possible choices are 0 for OFF and 1 for ON.\n"
$ Do not translate Echo
MSG48           "ate: 0828-048 The value %s specified for the Linefeeds command is not valid.\n\
              Possible choices are 0 for OFF and 1 for ON.\n"
$ Do not translate Linefeeds
MSG49           "ate: 0828-049 Correct the entry in your dialing directory and try again,\n\
              or use the Connect command to make your connection.\n"
$ Do not translate Connect
MSG50           "ate: 0828-050 The value %s you specified for the Wait command is not valid.\n\
              Possible choices include any integer greater than or equal to zero.\n"
$ Do not translate Wait
MSG51           "ate: 0828-051 The value %s you specified for the Transfer command is not valid.\n\
              Possible choices are p for pacing and x for xmodem.\n"
$ Do not translate Transfer, p, pacing, x, xmodem
MSG52           "ate: 0828-052 The value %s you specified for the Character command is not valid.\n\
              Possible choices include any single character or integer.\n"
$ Do not translate Character
MSG53           "ate: 0828-053 The value %s specified for the VT100 command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"
$ Do not translate VT100
MSG54           "ate: 0828-054 The system will use a parity of 0 because a length of 8 was\n\
              specified.\n"
MSG55           "ate: 0828-055 The value %s specified for the Write command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"
$ Do not translate Write
MSG56           "ate: 0828-056 The session has been disconnected because the system can no\n\
              longer detect a carrier signal.\n"
MSG57           "ate: 0828-057 The dialing directory %s \n\
              has more than 20 entries.  Only the first 20 entries can\n\
              be used.\n"
MSG58           "ate: 0828-058 The value %s specified for the Xon/Xoff command is not valid.\n\
              Possible choices are 0 for ON and 1 for OFF.\n"
$ Do not translate Xon, Xoff
MSG60           "ate: 0828-060 The system cannot open port %s because it is\n\
              enabled or in use.\n"
MSG61           "ate: 0828-061 The system cannot open port %s because the port \n\
is busy.\n"
MSG62           "ate: 0828-062 The system cannot complete the Perform command requested\n\
              because of the error shown below.\n"
$ Do not translate Perform
MSGUNK          "Message number %d is not in messages.c\r\n"
$ Do not translate the word messages.c.  This refers to a source file in which
$ the hard coded messages reside.
MSG40           "ate: 0828-040 Press Enter.\n"
MSG3            "ate: 0828-003 The %s command is not valid.\n\
              Enter the first letter of a command from the list on the menu.\n"
MSG4            "ate: 0828-004 A required temporary file cannot be opened because of the\n\
              error shown below.  Use the Quit command to end the program,\n\
              then start the program again.\n"
$ Do not translate Quit
MSG8            "ate: 0828-008 The system tried to open port %s\n\
              but failed.  If the port name is not correct, change it using the\n\
              Alter menu.  Or, take the action indicated by the system message\n\
              shown below.\n"
$ Do not translate Alter
MSG9            "ate: 0828-009 The Connect command cannot complete because the line was busy,\n\
              or the modem did not detect a carrier signal.  Make sure the number\n\
              is correct and try again, or try the same number later.\n"
$ Do not translate Connect
MSG19           "ate: 0828-019 The file transfer cannot begin because no pacing character\n\
              has been received for 100 seconds.  Verify that both the sending\n\
              and receiving user specified the same pacing character in the\n\
              Character command.\n"
$ Do not translate Character
MSG21           "ate: 0828-021 The file transfer cannot continue because no pacing character\n\
              has been received for 30 seconds.  Verify that both the sending\n\
              and receiving user specified the same pacing character in the\n\
              Character command.\n"
$ Do not translate Character
MSG26           "ate: 0828-026 Data is no longer being captured, and the current capture\n\
              data has been lost.  The capture buffer is full and cannot be written\n\
              to the file %s\n\
              because of the reason shown below.\n"
MSG33           "ate: 0828-033 Data is no longer being captured, and the current capture\n\
              data has been lost.  The process to write the capture buffer to the\n\
              capture file cannot be started because of the reason shown below.\n"
MSG34           "ate: 0828-034 File transfer cannot complete because too many transmission\n\
              errors have occurred. Use Connect command to re-establish the\n\
              connection.  Then try to transfer the file again.\n"
$ Do not translate Terminate, Connect
MSG35           "ate: 0828-035 The Send command cannot complete because the receiving site\n\
              has not indicated it is ready to receive.  Make sure the receiving\n\
              site is using the same communication protocol, then send the file\n\
              again.\n"
$ Do not translate Send
MSG37           "ate: 0828-037 The file transfer cannot complete because the receiving site\n\
              did not acknowledge receipt of sector %d.  Make sure the\n\
              receiving site is using the same communication protocol, and send\n\
              the file again.\n"
MSG38           "ate: 0828-038 The file transfer cannot complete because the receiving site\n\
              did not acknowledge end of transmission.  Make sure the receiving site\n\
              is using the same communication protocol, and send the file again.\n"
MSG39           "ate: 0828-039 The xmodem command cannot complete because a flag is not valid.\r\n\
              Valid flags are -r for receive, -s for send, or -p for passthrough.\r\n\
              Enter the command again using one of these flags.\r\n"
$ Do not translate xmodem, r, receive, s, send, p, passthrough
MSG43           "ate: 0828-043 The value %s specified for the Rate command is not valid.\n\
              Possible choices are 50, 75, 110, 134, 150, 300, 600,\n\
              1200, 1800, 2400, 4800, 9600 and 19200.\n"
$ Do not translate Rate
MSG59           "ate: 0828-059 The value %s specified for a control key is not valid.\n\
              Possible choices include any integer greater than or equal to zero\n\
              and less than 32 decimal (040 octal, 0x20 hex).\n"

$ $              messages printed by help; all these message identifiers
$ $              begin with the letter 'H'
$ $              when a help subject has more than one message string
$ $              associated with it, the continuations have 2, 3, etc.
$ $              appended to the end of the message identifier, e.g.
$ $              HMOD, HMOD2, HMOD3 ...

HDEFLT          "No such help as %d.\n\n"
HHELP                   "\
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
   unconnected main menu (for example, 'h m' to get  help  on  modifying  local\n\
   values).  Valid command letters are:\n\
            c (connect)    h (help)     a (alter)    q (quit)\n\
            d (directory)  s (send)     m (modify)   b (break)\n\
            t (terminate)  r (receive)  p (perform)\n\n"
$ Do not translate ctrl-r, HELP, help
$ Do not translate the last three lines. These are all the commands.
HCONN                   "\
                                 CONNECT COMMAND\n\
\n\
The connect command allows you to establish a  connection  either  manually  or\n\
modem-dialed.  It also allows you to open a locally-attached port.\n\
\n"
HCONN2                  "\
Before you try to make a connection, your modem must be set up properly.   Read\n\
your  modem book to determine how to set its switches.  Some suggested settings\n\
follow:\n\
   The computer supports RS-232 DTR (pin 20).\n\
   No result codes are sent to the computer by the modem.\n\
   Allow the computer to detect carrier by reading the RS-232C pin.\n\
\n"
$ Do not translate RS-232 DTR, RS-232C, CONNECT, connect
HCONN3                  "\
If you type 'c' alone, you will be prompted for a phone number  (auto  dialing)\n\
and/or  a port name (direct connect).  If you enter both a telephone number and\n\
a port name, the telephone number must be first.\n\
\n\
MANUAL DIALING:  If you just press Enter after the prompt,  you  will  have  90\n\
   seconds  to  make a manual connection.  If no connection is established, you\n\
   will be returned to the main menu.\n\n"
HCONN4                  "\
AUTO DIALING:  If you enter a number after the prompt, it will be auto  dialed.\n\
   If no connection has been established in 45 seconds (the line is busy, there\n\
   is no answer, or a wrong number was dialed), you will  be  returned  to  the\n\
   main menu.  Telephone numbers are limited to 40 characters.  They must begin\n\
   with a number, and they may not include spaces.\n\
\n\
OPEN A PORT:  If you enter a port name after  the  prompt,  the  port  will  be\n\
   opened.\n\
\n"
HCONN5                  "\
NO PROMPT:  You may avoid prompting by entering a  'c',  a  space,  and  (1)  a\n\
   telephone  number or (2) a port name or (3) a telephone number, a space, and\n\
   a port name.  (Example: c 9,555-1234 tty1).\n\
\n\
CONTROL KEYS:  Once connected, the following control keys may be used:\n\
           ctrl b  Start or stop recording display output.\n\
           ctrl v  Display main menu to issue a command.\n\
           ctrl r  Return to previous screen at any time.\n\n"
$ Do not translate the example.
$ Do not translate ctrl b, ctrl v, ctrl r
HDIR                    "\
                                DIRECTORY COMMAND\n\
\n\
An already-existing dialing directory file can be  read  by  entering  'd'  and\n\
pressing  Enter.   You will be prompted for a dialing directory file name.  The\n\
default dialing directory is the last one  specified.   If  no  name  has  been\n\
specified,  the  default  directory  name is  /usr/lib/dir .  You may avoid the\n\
prompt by entering the first letter of the command followed by a space and  the\n\
directory name (example: d mydirectory).\n\
\n"
$ Do not translate /usr/lib/dir, DIRECTORY
HDIR2                   "\
CREATING A DIRECTORY:  Directories can be created  using  the  editor  of  your\n\
choice.   Items  in  the  directory  are  space separated (1 or more), and must\n\
appear on the line in the following order:\n\
    name             (up to 20 chars; no embedded blanks permitted)\n\
    telephone number (up to 40 chars; no embedded blanks permitted)\n\
    rate             (50,75,110,134,150,300,600,1200,1800,2400,4800,9600,19200)\n"
HDIR3                   "\
    length           (7,8)\n\
    stop bits        (1,2)\n\
    parity           (0=none, 1=odd, 2=even; Parity must be 0 if Length=8)\n\
    echo             (0=off, 1=on)\n\
    linefeeds        (0=off, 1=on)\n\n"
HDIR4                   "\
A maximum of 20 entries may be placed in a single directory.  An example  of  a\n\
directory with 3 numbers follows.  The header is provided for this example only\n\
and should not appear in a normal directory file.\n\
\n\
         NAME            NUMBER    RATE LENGTH STOP PARITY ECHO LF's\n\
\n\
         Contest_BBS   9,555-1669  1200    8     1     0     0   1\n\
         Matchmaker    9,555-8747  1200    7     1     1     1   0\n\
         Utility_Board 9,555-3470  1200    7     2     2     0   0\n\n"
HMOD                    "\
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
\n"
$ Do not translate MODIFY, modify, kapture, Write, LINEFEEDS, linefeeds
HMOD2                   "\
ECHO CHARACTERS:  Have the program echo characters typed at the keyboard to the\n\
   display.  Use this option if you see no characters or double characters.\n\
\n\
VT100:  Emulate a Digital Equipment Corporation VT100 terminal.\n\
\n\
WRITE TO CAPTURE FILE:  Write display data to the capture file.  This value may\n\
   also be set during a connection by using ctrl-b.\n\n"
$ Do not translate ECHO, echo, VT100, Digital Equipment Corporation, ctrl b
HMOD3                   "\
XON/XOFF:  Turn Xon/Xoff signals on  or  off.   If  on,  signals  indicating  a\n\
   communications  buffer overflow will be sent to and received from the remote\n\
   computer.  The signals are ascii 17 (Xon) and ascii 19 (Xoff).\n\
\n\
IF YOU USE A DIRECTORY FILE:  The values specified in the  directory  file  for\n\
   echo  and  line  feeds will be used during the connection, and remain as the\n\
   current values after the connection.\n\
\n"
$ Do not translate XON, XOFF, Xon, Xoff, ascii
HMOD4                   "\
MODIFY COMMANDS:  The modify menu is reached by typing the first letter of  the\n\
   Modify command  'm'  at the main menu.  The modify menu may be avoided (fast\n\
   path) by specifying values after the 'm' command.  For example, to name  the\n\
   capture file 'junk' and turn on the write value, type:  m n junk w.\n\n"
$ Do not translate MODIFY, Modify, modify, m,  m n junk w
HALTER                  "\
                                  ALTER COMMAND\n\
\n\
The alter command allows you to specify the:\n\
LENGTH:  Set the character length in bits.  The default is 8.\n\
STOP BITS:  Set the number of stop bits sent.  The default is 1.\n\
PARITY:  Set the parity to none, odd or even.  The default is none.\n\
RATE:  Set the bits per second (also called the speed or the baud  rate).   The\n\
   default setting is 1200.\n"
$ Do not translate ALTER, alter, LENGTH, PARITY, RATE, STOP
HALTER2                 "\
PORT:  Specify the tty device.  The default device is tty0.\n\
PREFIX:  Specify the modem  prefix.   The  default  is  ATDT  (ATtention,  Dial\n\
   Touchtone).  Another common value is ATDP (ATtention, Dial Pulse).  See your\n\
   modem manual to determine what prefix is appropriate.\n\
SUFFIX: Specify the modem suffix.  The default is none.  To reset to no  suffix\n\
   from another string, set the suffix value to 0.\n"
$ Do not translate tty, tty0, ATDT, ATDP
HALTER3                 "\
REDIAL WAIT:  Specify the seconds to  wait  between  redialing  attempts.   The\n\
   default is 0 (no delay).\n\
REDIAL ATTEMPTS:  Specify the maximum number of times to redial.   The  default\n\
   is 0 (no redialing will be done).\n\
PACING CHARACTER:  Set the pacing character or time delay.  The default  is  0,\n\
   no delay between sending lines.\n\n"
HALTER4                 "\
FILE TRANSFER METHOD:  Choose the file transfer method.   This  may  either  be\n\
   pacing or xmodem:\n\
\n\
   PACING operates by sending or receiving a file one line at a time.   If  you\n\
      enter  an  integer  for  the  pacing  character,  it will pause that many\n\
      seconds between transmitting lines.  If you enter a  character,  it  will\n\
      wait  for  the  receiving computer to send this prompt before sending the\n\
      next line.  In the receive mode, pacing will send the prompt character to\n\
      the remote computer after receiving a carriage return.\n\
   XMODEM is a  protocol that  sends  or receives  data a block  at a time, and\n\
      provides for error checking.\n\
\n"
$ Do not translate xmodem, pacing
HALTER5                 ""
$ This is a null statement. Do not translate.
HALTER6                 "\
IF YOU USE A DIRECTORY FILE:  The values specified in the  directory  file  for\n\
   length,  stop  bits, parity and rate will be used during the connection, and\n\
   remain as the current values after the connection.\n\
\n\
ALTER COMMANDS: The alter menu is reached by typing the  first  letter  of  the\n\
   Alter  command  'a'  at the main menu.  The alter menu may  be avoided (fast\n\
   path) by specifying values after the 'a' command.  For example, to  set  the\n\
   rate to 300 and the parity to even, type:  a r 300 p 2.\n\n"
$ Do not translate ALTER, alter, a, a r 300 p 2
HPERF                   "\
                                 PERFORM COMMAND\n\
\n\
The Perform command is issued from the menu by typing the first letter  of  the\n\
command,  'p'.  You will be prompted to enter an operating system command.  You\n\
may avoid the prompt by entering the first letter of the command followed by  a\n\
space and the operating system command.  For example, to see a list of files in\n\
the current directory, enter 'p li'.  After the operating  system  command  has\n\
been  executed,  'Press Enter' will be displayed.  Enter will return you to the\n\
program.\n\n"
$ Do not translate PERFORM, Perform, p, p li
HPERF2                  ""
$ This is a null message. Do not translate.
HQUIT                   "\
                                 QUIT COMMAND\n\
\n\
The quit command exits the program and returns you to the operating system.\n\n"
$ Do not translate QUIT, quit
HSEND                   "\
                                 SEND COMMAND\n\
\n\
The Send command is available only when a connection has been established.   It\n\
is  used  to  send  a  file to a remote computer.  Two methods are provided for\n\
sending a file:  pacing and xmodem protocol.  You specify which method you wish\n\
to use by using the Transfer and Character commands on the Alter menu.\n\
\n"
$ Do not translate SEND, Send, xmodem, Transfer, Character, Alter, pacing
HSEND2                  "\
PACING operates by sending a file to the remote computer one line  at  a  time.\n\
   If  you  enter  an integer for the pacing character, it will pause that many\n\
   seconds between transmitting lines.  If you enter a character, it will  wait\n\
   for the receiving computer to send this prompt before sending the next line.\n\
XMODEM is a  protocol  that  sends or  receives  data  a block  at a  time, and\n\
   provides  for  error  checking.  The  remote  computer  must  also  have the\n\
   capability to use xmodem.\n\
\n"
$ Do not translate xmodem, XMODEM, pacing
HSEND3                  "\
SEND COMMAND: To use the Send command, enter the first letter  of  the  command\n\
   ('s')  and  press  Enter.   You will be prompted for the name of the file to\n\
   send.  The file must exist.  You may avoid the prompt by entering the  first\n\
   letter  of  the command followed by a space and the name of the file to send\n\
   (example:  s send_file).\n\n"
$ Do not translate Send, s send_file
HRECV                   "\
                                RECEIVE COMMAND\n\
\n\
The Receive command is available only when a connection has  been  established.\n\
It  is used to receive a file from a remote computer.  Two methods are provided\n\
for receiving a file:  pacing and xmodem protocol.  You  specify  which  method\n\
you wish to use by using the Transfer and Character commands on the Alter menu.\n\
\n"
$ Do not translate RECEIVE, Receive, pacing, xmodem, Transfer, Character, Alter
HRECV2                  "\
PACING operates by receiving a file from a  remote computer one line at a time.\n\
   If  the pacing character is not an integer, the pacing routine will send the\n\
   character to the remote computer after each carriage return it receives.\n\
XMODEM is a  protocol  that  sends or  receives  data  a block  at a  time, and\n\
   provides  for  error  checking.  The  remote  computer  must  also  have the\n\
   capability to use xmodem.\n\
\n"
$ Do not translate pacing, XMODEM, xmodem
HRECV3                  "\
RECEIVE COMMAND:  To use the Receive command, enter the  first  letter  of  the\n\
   command  'r'  and  press  Enter.  You  will be  prompted for the name of the\n\
   file to receive.  You may avoid the prompt by entering the first  letter  of\n\
   the  command  followed  by  a  space  and  the  name  of the file to receive\n\
   (example: r receive_file).  If the file does not exist, it will be  created.\n\
   If it does exist, the received data will be appended to the bottom of it.\n\n"
$ Do not translate RECEIVE, Receive, r
HTERM                   "\
                               TERMINATE COMMAND\n\
\n\
The Terminate command is used to end a connection.  It is available only when a\n\
connection has been established.  It is issued from the main menu (type  ctrl-v\n\
while  connected  to display the main  menu, enter 't' and  press  Enter).   It\n\
disconnects  the modem, closes the port, and returns you to the main menu or to\n\
the directory menu, depending on how you established the connection.\n\n"
$ Do not translate TERMINATE, Terminate, t, ctrl-v
HBRK                    "\
                                 BREAK COMMAND\n\
\n\
The Break command is used to send a break signal to the remote computer.  It is\n\
available  only  when a connection has been established, and is issued from the\n\
main menu (type  ctrl-v  while connected to display the main  menu,  enter  'b'\n\
and press Enter).\n\n"
$ Do not translate BREAK, Break, ctrl-v

$ $              messages for the default file; all message identifiers
$ $              begin with 'DF'

DFLEN           "LENGTH"
$ Do not translate
DFSTOP          "STOP"
$ Do not translate
DFPARITY        "PARITY"
$ Do not translate
DFRATE          "RATE"
$ Do not translate
DFDEVICE        "DEVICE"
$ Do not translate
DFINITIAL       "INITIAL"
$ Do not translate
DFFINAL         "FINAL"
$ Do not translate
DFWAIT          "WAIT"
$ Do not translate
DFATTEMPTS      "ATTEMPTS"
$ Do not translate
DFXFER          "TRANSFER"
$ Do not translate
DFCHAR          "CHARACTER"
$ Do not translate
DFNAME          "NAME"
$ Do not translate
DFLFS           "LINEFEEDS"
$ Do not translate
DFECHO          "ECHO"
$ Do not translate
DFVT100         "VT100"
$ Do not translate
DFWRITE         "WRITE"
$ Do not translate
DFXONOFF        "XON/XOFF"
$ Do not translate
DFDIR           "DIRECTORY"
$ Do not translate
DFCAPKEY        "CAPTURE_KEY"
$ Do not translate
DFMMENUKEY      "MAINMENU_KEY"
$ Do not translate
DFPREVKEY       "PREVIOUS_KEY"
$ Do not translate

PORTERR         "Error opening /etc/inittab:\n"
$ Do not translate /etc/inittab:
$ $ PORTERR                 This message is issued by ckenabled (called from
$ $ PORTERR                 connect.c) when the open of /etc/portstatus fails
NOMEM           "No memory available for screen buffer.\n"
$ $ NOMEM                   This message is issued by portinit (called from
$ $ NOMEM                   portrw.c) when the malloc fails

$       messages used by menuinit.c
CMMD2           " COMMAND   DESCRIPTION             CURRENT          POSSIBLE CHOICES\n---------  ----------------------  --------  ------------------------------\n"
$ This is a header file. Start all items in the same columns
LENG            "Length"
$ Do not translate
DLENGTH         "Bits per character      %-8d  7,8\n"
$ Do not translate anything after the %. The % must start in column 25
STOP            "Stop"
$ Do not translate
DSTOP           "Number of stop bits     %-8d  1,2\n"
$ Do not translate anything after the %. The % must start in column 25
PARITY          "Parity"
$ Do not translate
DPARITY         "Parity setting          %-8d  0=none, 1=odd, 2=even\n"
$ Do not translate Parity.
$ Do not translate anything after the %. The % must start in column 25
RATE            "Rate"
$ Do not translate
DRATE           "Number of bits/second   %-8d  50,75,110,134,150,300,600,\n\
                                             1200,1800,2400,4800,9600,19200\n\n"
$ Do not translate anything after the %. The % must start in column 25
DEVICE          "Device"
$ Do not translate
DDEVICE         "/dev name of port       %-8.8s  tty0-tty16\n"
$ Do not translate /dev
$ Do not translate anything after the %. The % must start in column 25
INITIAL         "Initial"
$ Do not translate
DINITIAL        "Modem dialing prefix    %-8.8s  ATDT, ATDP, etc.\n"
$ Do not translate anything after the %. The % must start in column 25
FINAL           "Final"
$ Do not translate
DFINAL          "Modem dialing suffix    %-8.8s  0 for none, valid modem suffix\n"
$ Do not translate %-8.8s. The % must start in column 25
WAIT            "Wait"
$ Do not translate
DWAIT           "Wait between redialing  %-8d  seconds between tries\n"
$ Do not translate Wait.
$ Do not translate %-8d. The % must start in column 25
ATTEMPTS        "Attempts"
$ Do not translate
DATTEMPTS       "Maximum redial tries    %-8d  0 for none, a positive integer\n\n"
$ Do not translate %-8d. The % must start in column 25
TRANSFER        "Transfer"
$ Do not translate
DTRANSFER        "File transfer method    %c         p=pacing, x=xmodem\n"
$ Do not translate anything after the %. The % must start in column 25
CHARACTER       "Character"
$ Do not translate
DCHARACTER      "Pacing char or number   %c         0 for none, a single char/integer\n"
$ Do not translate Pacing
$ Do not translate %c. The % must start in column 25. 0 must start in col. 36.
DNAME           "Name the capture file   %-18.18s  Any valid file name\n\n"
$ Do not translate %-18.18s. The % must start in column 25
LINEFEEDS       "Linefeeds"
$ Do not translate
DLINEFEEDS      "Linefeeds added         %-8s            ON, OFF\n"
$ Do not translate Linefeeds.
$ Do not translate anything after the %. The % must start in column 25
MECHO           "Echo"
$ Do not translate
DECHO           "Echo mode               %-8s            ON, OFF\n"
$ Do not translate Echo
$ Do not translate anything after the %. The % must start in column 25
VT100           "VT100"
$ Do not translate
DVT100          "VT100 emulation         %-8s            ON, OFF\n"
$ Do not translate VT100
$ Do not translate anything after the %. The % must start in column 25
DWRITE          "Write display data to   %-8s            ON, OFF\n\
             capture file\n"
$ Do not translate anything after the %. The % must start in column 25
$ The second line must start in the same column.
XONOFF          "Xon/Xoff"
$ Do not translate
DXONOFF         "Xon/Xoff signals        %-8s            ON, OFF\n"
$ Do not translate anything after the %. The % must start in column 25
MOFF            "OFF"
MON             "ON"

$ $              messages from signal.c

DISCONN         "Disconnect (%s or %s)?  "
NOSUCH          "No such state as %d.\r\n"

$ $             directory message strings

DIRHDR          " #        NAME           TELEPHONE (first digits)  RATE  LEN STOP PAR ECHO LF's\n"
$ This is a header do not change the columns. See next message to see where
$ The columns actually start.
DIRDASH         "-- -------------------- -------------------------- ----- --- ---- --- ---- ----\n"
$ Do not translate
DIRENTR         "\nEnter directory entry number or "
EXITSTR         "e(Exit)  "
$ Do not translate.
$ Do not translate the word "widechar".
DIR_MB		"direc: Error in converting to widechar.\n"
LK_MB		"item_cmp: Error in converting to widechar.\n"
MENU_MB		"menu: Error in converting to widechar.\n"
MAIN_MB		"main: Error in converting to widechar.\n"
MAIN_WC		"main: Error in converting to multibyte.\n"
AUTOWRAP	"AutoWrap"
DAUTOWRAP       "VT100 auto wrap margins %-8s            ON, OFF\n"
