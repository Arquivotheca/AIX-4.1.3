$quote "         define quote character.

$   
$ COMPONENT_NAME: UUCP tip.msg  
$   
$ FUNCTIONS:   
$ ORIGINS: 10  26  27   
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989  
$ All Rights Reserved  
$ Licensed Materials - Property of IBM  
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ @(#)69\t1.3  src/bos/usr/bin/tip/tip.msg, cmdtip, bos411, 9428A410j 3/11/94 10:16:10  
$   
$                   The assigned range for this message file is 176-238. 
$ messages for tip  
$len 
$set MS_TIP        Define initial set#

ACCESSDENY           "tip: 0835-176 The access is denied.\r\n"
ACUERR               "tip: 0835-177 There is an ACU open error."
$ Do not translate ACU.  
ALLBUSY              "tip: 0835-178 All the ports are busy.\n"
AWAYFOR              "away for "
BADBAUD              "tip: 0835-179 The baud rate %d is not valid.\n"
BADDIR               "tip: 0835-180 %s is not a valid directory\r\n"
BADENTRY             "tip: 0835-181 The remcap entry is not valid.\n"
$ Do not translate remcap. 
BADFLAG              "tip: 0835-182 %s is not a known flag."
BUFFOFLOW            "tip: 0835-183 %s was too long.\n"
CALLABORT            "\ntip: 0835-234 The call was aborted.\n"
CALLABORT2           "The call was aborted."
CALLCOMPLETE         "The call is completed."
CALLFAIL             "The call failed."
CALLFAILED           "The call failed.\n"
CALLFAILED2          "The call failed."
CALLTERM             "The call was stopped."
CANTALLOCATE         "tip: 0835-184 program stopped: Cannot allocate memory for %s."
CANTCREAT            "\r\ntip: 0835-185 Cannot create the file named %s.\r\n"
CANTCREAT2           "\r\ntip: 0835-186 Cannot create the file named %s.\r\n"
CANTCREAT3           "tip: 0835-187 Cannot create the file named %s.\r\n"
CANTEXECL            "tip: 0835-188 Cannot run the execl system call\r\n"
$ Do not translate execl. 
CANTEXECL2           "\r\ntip: 0835-189 Cannot run the execl system call.\r\n"
$ Do not translate execl. 
CANTFIND             "tip: 0835-190 Cannot find the file %s\r\n"
CANTFORK             "tip: 0835-191 Cannot start.\r\n"
CANTGET              "tip: 0835-192 There is no dial tone."
CANTHANGUP           "tip: 0835-193 Cannot hang up modem.\n\
\r"
CANTINIT             "tip: 0835-194 Cannot initialize the Bizcomp modem."
CANTOPEN             "tip: 0835-195 Cannot open the file %s.\r\n"
CANTOPEN2            "tip: 0835-196 Cannot open the log file.\r\n"
CANTOPEN3            "tip: 0835-197 Cannot open the host description file.\n"
CANTPIPE             "tip: 0835-198 Cannot open pipe.\r\n"
CANTSET              "tip: 0835-199 Cannot set dialing mode."
CANTSYNC             "tip: 0835-200 Cannot synchronize with the Courier modem.\n"
$ Do not translate Courier. 
CANTSYNC2            "Cannot synchronize with the modem."
CANTSYNC3            "tip: 0835-201 Cannot synchronize with the Hayes modem.\n\
\r"
$ Do not translate Hayes. 
CANTSYNC4            "tip: 0835-202 Cannot synchronize with Vadic 3451 modem.\n"
$ Do not translate Vadic 3451. 
CANTSYNC5            "tip: 0835-203 Cannot synchronize with the Ventel modem.\n"
$ Do not translate Ventel. 
CD                   "[cd] "
$ Do not translate this message. 
CHARSXFER            "characters transferred in "
$ Leave a space at the end. This is a constructed sentence. 
CONNECT              "CONNECT"
$ Do not translate this message. 
CONNED               "Connected.\r\n"
CONNECT12            "CONNECT 1200"
$ Do not translate this message. 
CONNECT2             "\07Connected.\r\n"
$ Do not translate \07. 
CONNECTFAIL          "The connect failed.\n"
CONNECTION           "CONNECTION\r\n\07"
$ Do not translate this message. 
DIALINGIT            "\nDialing..."
DIALTIMEDOUT         "%d second dial timeout."
DISCONNECTING        "\r\nDisconnecting..."
DISCONNECTING2       "\rDisconnecting modem...\n\r"
ECHOFAIL             "\"echo\" command failed.\n"
$ Do not translate echo. 
ENTRYTOOLONG         "The remcap entry is too long.\n"
$ Do not translate remcap. 
EOT                  "\r\n[EOT]\r\n"
$ Do not translate this message. 
EOT2                 "\07%s\n[EOT]\n"
$ Do not translate this message. 
ERROR                "tip: 0835-204 The input is not correct."
FAILEDDIAL           "tip: 0835-205 The Vadic modem failed to dial.\n"
$ Do not trabslate Vadic. 
FAILEDDIAL2          "The modem failed to dial."
FLOCK                "tip: flock"
$ Do not translate this message. 
FORK                 "fork"
$ Do not translate this message. 
INFINITE             "tip: 0835-206 There is an infinite tc= loop in /etc/remote.\n"
INTERACTIVE          "tip: 0835-207 Tip must be run in the foreground.\n"
LINEBUSY             "The telephone line is busy."
LINEFAILED           "tip: 0835-208 Failed to open a dialup line."
LINESXFER            " lines transferred in "
$ This is a constructed sentence. 
LINKDOWN             "tip: 0835-209 The link is down.\n"
LISTCOMM             "Type the command for the remote system: "
LOCALCOMM            "Local command? "
LOCALFILE            "Local file name? "
LOSTCARRIER          "tip: 0835-210 The carrier signal was stopped."
NOACCEPT             "tip: 0835-211 The Vadic modem will not accept the dial command.\n"
$ Do not translate Vadic. 
NOACCEPT2            "tip: 0835-212 The modem will not accept the dial."
NOACCEPTPH           "tip: 0835-213 The Vadic modem will not accept phone number.\n"
$ Do not translate Vadic. 
NOACCEPTPH2          "tip: 0835-214 The modem will not accept number."
NOCARRIER            "NO CARRIER"
$ Do not translate this message. 
NOCONNECT            " NO CONNECTION\r\n\007\r\n"
$ Do not translate this message. 
NOCORE               "tip: 0835-215 There is no memory available now.\r\n"
NODEVSPEC            "tip: 0835-216 %s: missing device specification.\n"
NODIALER             "tip: 0835-217 Unable to locate the dialer %s.\n"
NOHOST               "tip: 0835-218 No host is specified.\n"
NOMATCH              "tip: 0835-219 \"%s\": No match is found.\n"
NOPHONE              "tip: 0835-220 %s: The phone number was not specified.\n"
NOPN                 "The phone number is missing."
NOPNFILE             "tip: 0835-221 Cannot open /etc/phones."
NOSUPPORT            "The speed %s is not supported.\n"
NUMTOOLONG           "tip: 0835-222 The phone number is too long. (maximum = %d digits)\n"
OK                   "OK"
$ Do not translate this message. 
OUTOFSYNC            "The modem is out of synchronization."
OUTOFSYNC2           "tip: 0835-223 The Bizcomp modem is out of synchronization.\n"
$ Do not translate Bizcomp. 
PIPE                 "pipe"
$ Do not translate this message. 
PLEASE               "Use 'tip dialer' to make sure the line is disconnected.\n\
\r"
PUT                  "[put] "
$ Do not translate this message. 
READIT               "read"
$ Do not translate this message. 
RING                 "RING"
$ Do not translate this message. 
RINGING              "Ringing..."
RINGING2             "RINGING"
$ Do not translate this message. 
SET                  "[set] "
$ Do not trnaslate this message. 
SET2                 "set %s"
SH                   "[sh]\r\n"
$ Do not translate this message. 
STARTCALL            "\nStarting call..."
TAKE                 "[take] "
$ Do not translate this message. 
TIMEDOUT             "\r\ntip: 0835-224 The remote system is timed out at EOL.\r\n"
$ Do not translate EOL. 
TIMEDOUT2            "The remote system connection timed out.\r\n"
TIMEOUT              "\07The remote system timeout waiting for reply.\n"
TIMEOUT2             "\07The remote system timeout waiting for reply.\n\r"
TIMEOUTERR           "\r\ntip: 0835-225 timeout error (%s)\r\n"
TIPIOCTL             "tip: ioctl"
$ Do not translate this message. 
TOOMANYLOCKS         "TOO MANY LOCKS %d"
$ Do not translate this message. 
UNKERR               "tip: 0835-226 Unknown modem error: %1$c (0x%2$x)"
UNKNOWNACU           "tip: 0835-227 The ACU specified is not a known type."
UNKNOWNHOST          "tip: 0835-228 The host %s is not known.\n"
UNKOPT               "tip: 0835-229 The %s is not a known option.\n"
UNKPARITY            "tip: 0835-230 The parity value %s is not known.\n"
UNKVAR               "tip: 0835-231 The variable %s is not known.\r\n"
UNRECHOST            "tip: 0835-232 The host name is not recognized."
USAGE                "Usage: <take> from [to]\r\n"
USAGE2               "Usage: <put> from [to]\r\n"
USAGE3               "usage: cu telno [-t] [-s Speed] [-a ACU] [-l Line] [-#]\n"
USAGE4               "Usage: tip [-v] [-Speed] [SystemName]\n"
USING                "Using ACU %s.\n"
$ Do not translate ACU. 
WRITEIT              "write"
$ Do not translate this message. 
WRITERR              "\r\ntip: 0835-233 The write system call failed.\r\n"
$ Do not translate write. 
NUMSING              "%1$d %2$s "
$ Do not translate. 
NUMPLURL             "%1$d %2$ss "
$ Do not translate. 
LITSECOND            "second"
$ This is constructed message. 
LITMINUTE            "minute"
$ This is constructed message. 
LITHOUR              "hour"
$ This is constructed message. 
ETSHELL              "shell"
$ This is constructed message. 
ETRCV                "Receive a file from remote host."
ETSND                "Send a file to remote host."
ETTAKE               "Take a file from remote host."
ETPUT                "Put a file to remote host."
ETPIPE               "Pipe command output to a remote host."
ETPIPLOC             "Pipe local command output to a remote host."
ETCONNECT            "Connect to remote host."
ETCD                 "Change directory."
ETEXIT               "Exit from tip."
$ Do not translate tip. 
ETSUSPALL            "Suspend tip on the local and the remote system."
$ Do not translate tip. 
ETSUSPLOC            "Suspend tip on the local system only."
$ Do not translate tip. 
ETSETVAR             "Set a variable."
ETGETSUM             "Display this command summary."
ETSENDBRK            "Send break character to remote system."
$ Do not translate break. 
PLSREMOV             "Please remove any blank lines or lines with one nonblank field\nfrom the phones file.\n"
