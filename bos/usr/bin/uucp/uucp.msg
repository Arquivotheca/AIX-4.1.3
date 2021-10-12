$quote "         define quote character.

$ @(#)84        1.13.4.2  src/bos/usr/bin/uucp/uucp.msg, cmduucp, bos41J, 9515B_all 4/13/95 17:59:33
$  
$ COMPONENT_NAME: CMDUUCP uucp.msg 
$  
$ FUNCTIONS:
$ ORIGINS: 10  27  3  
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1995
$ All Rights Reserved 
$ Licensed Materials - Property of IBM 
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$  
$len 
$ The range of message numbers assigned to this file are 001-175
$set 1        Define initial set#

$ The following messages are used in uutry, Nutry and Uutry     
$ Do not translate %s, %d, %c, %1$s, etc.    
1                    "uutry: 0835-001 %s: Specify a system name."
2                    "uutry: 0835-002 %s is not a valid system name."
$ The following messages are from uulog     
3                    "uulog: 0835-003 %s is not a recognized flag."
4                    "uulog: 0835-004 Specify only one system with the -f flag."
5                    "uulog: 0835-160 The -s option requires an argument.\n"
6                    "uulog: 0835-161 The -f option requires an argument.\n"

$set MS_UUCP        Redefine set#

$ There are no message related defines from the following source files:     
$       
$  \tuucpname.c\tchremdir.c \tcpmv.c     
$  \teio.c\t \tgetargs.c \tgetpwinfo.c     
$  \tgetprm.c\tgio.c \t\tgnamef.c     
$  \tgwd.c\t \timsg.c \t\tpkdefs.c     
$  \tstrpbrk.c \txio.c     
$  \txqt.c\t \tdbg.h \t\tparms.h     
$  \tpk.h\t\tshio.c     
$ Do not translate uucp.    
MSG_UUCHECK_0        "[-v] [-xNumber]"
MSG_UUCHECK_1        "\tUsage: %1$s %2$s\n"
MSG_UUCHECK_2        "uucheck: 0835-005 Check Required Files and Directories.\n"
MSG_UUCHECK_24       "\nCan run the following commands:\n"
MSG_UUCHECK_3        "uucheck: 0835-006 The check of the directories is complete.\n\n"
MSG_UUCHECK_4        "uucheck: 0835-007 Check file %s.\n"
MSG_UUCHECK_5        "uucheck: 0835-008 The %s check is complete.\n\n"
MSG_UUCHECK_6        "Machine phase. (To call or run uux requests.)"
$ Do not translate uux.    
MSG_UUCHECK_7        "Logname phase. (To receive calls.)"
MSG_UUCHECK_8        "uucheck: 0835-009 Cannot open %s.\n"
MSG_UUCHECK_9        "When a system logs in as: "
MSG_UUCHECK_10       "\tWe will call the system back.\n\n"
MSG_UUCHECK_11       "When we call the  system or systems."
MSG_UUCHECK_12       "\tWe allow the system to request files.\n"
MSG_UUCHECK_13       "\tWe do not allow the system to request files.\n"
MSG_UUCHECK_14       "\tWe will send files queued for the system on this call.\n"
MSG_UUCHECK_15       "\tWe will not send files queued for the system on this call.\n"
MSG_UUCHECK_16       "\tThe other system can send files to\n"
MSG_UUCHECK_17       "\t%s    (The default.)\n"
MSG_UUCHECK_18       "\tExcept for\n"
MSG_UUCHECK_19       "\tThe other system can request files from\n"
MSG_UUCHECK_21       "\tMyname for the conversation will be %s.\n"
$ Do not translate Myname.    
MSG_UUCHECK_22       "\tPUBDIR for the conversation will be %s.\n"
$ Do not translate PUBDIR.   
MSG_UUCHECK_23       "Machine or machines: "
MSG_UUCHECK_25       "uucheck: 0835-010 There is a software error in the permission.c file.\n"
$ Do not translate permission.c     
MSG_UUCHECK_26       "Command: %s. Fullname %s.\n"
$ Do not translate Fullname. 
$ These messages are from callers.c     
MSG_CALLERS_L1       "The system is attempting a generic open."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L2       "TIMEOUT"
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L3       "FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L4       "There is a generic call to gdial."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L5       "ACU write"
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L6       "The specified host does not exist."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L7       "There is no available socket."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L8       "The connect failed."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L9       "There is an 801 open."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L10      "There is no available socket."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_A1       "callers: 0835-011 program stopped: There is no uucp server."
$ Do not translate uucp.    
$ The following messages are from conn.c     
MSG_CONN_CD1         "conn(%s)\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD2         "getto returned %d.\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD3         "Call Failed. %s\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD4         "Device Type %s is wanted\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD5         "Requested Device Type Not Found\n"
$ This is debug message. Do not translate. 
MSG_CONN_A1          "BAD LINE"
$ This is debug message. Do not translate. 
MSG_CONN_CD6         "Wrong Time To Call %s\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD7         "expect: ("
$ This is debug message. Do not translate. 
MSG_CONN_CD9         "lost line errno - %d\n"
$ This is debug message. Do not translate. 
MSG_CONN_L1          "FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CONN_L2          "LOGIN"
$ This is a log message. It is better not to translate this message. 
MSG_CONN_L3          "LOST LINE"
$ This is a log message. It is better not to translate this message. 
MSG_CONN_CD10        "enough already\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD8         "got it\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD12        "timed out\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD11        "BREAK\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD13        "EOT\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD14        "Sendthem ("
$ This is debug message. Do not translate. 
MSG_CONN_CD15        "DELAY\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD16        "NO CR"
$ This is debug message. Do not translate. 
MSG_CONN_CD17        "NO CR - MIDDLE IGNORED>\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD18        "PAUSE\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD19        "ECHO CHECK ON\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD20        "ECHO CHECK OFF\n"
$ This is debug message. Do not translate. 
$ The following messages are from ct.c     
MSG_CT1              "ct: 0835-012 Cannot open %s.\n"
$ Do not translate ct.    
MSG_CT2              "\tUsage: %1$s %2$s\n"
MSG_CT3              "ct: 0835-013 The %s parameter to the -w flag must be greater than zero.\n"
MSG_CT5              "\tUsage: %1$s %2$s\n"
MSG_CT6              "\tUsage: %1$s %2$s\n"
MSG_CT7              "ct: 0835-014 Specify a phone number.\n"
MSG_CT20             "ct: 0835-015 The phone number %s is too long.\n"
MSG_CT8              "ct: 0835-016 The phone number %s is not correct.\n"
MSG_CT9              "ct: There is an allocated dialer at %s baud.\n"
$ Do not translate ct. 
MSG_CT10             "ct: There are no %s dialers on this system.\n"
$ Do not translate ct. 
MSG_CTV2             "ct: The dialer is busy %d minutes."
$ Do not translate ct. 
MSG_CTV3             "ct: The dialers are busy %d minutes."
$ Do not translate ct. 
MSG_CT11             "ct: The %1$d %2$s dialer is busy.\n"
$ Do not translate ct. 
MSG_CT12             "ct: The %1$d %2$s dialers are busy.\n"
$ Do not translate ct. 
MSG_CT13             "ct: Waiting for %d minutes.\n"
$ Do not translate ct. 
MSG_CT14             "ct: Waiting for %d minute.\n"
$ Do not translate ct. 
MSG_CT15             "ct: Do you want to wait for a dialer?\nType y for yes: "
$ Do not translate ct. 
$ Replace y for yes to the language you translate. 
MSG_CT16             "ct: Specify the number of minutes to wait: "
$ Do not translate ct. 
MSG_CT17             "ct: Waiting for a dialer.\n"
$ Do not translate ct. 
MSG_CT18             "ct: Time out.\n"
$ Do not translate ct. 
MSG_CTV5             "ct: Connected.\n"
$ Do not translate ct. 
MSG_CT22             "ct: Press the carriage return key."
$ Do not translate ct. 
MSG_CTCD1            "there is a shared or delayed getty; exit\n"
$ This is debug message. Do not translate. 
MSG_CTCD2            "start getty %s "
$ This is debug message. Do not translate. 
MSG_CT19             "ct: 0835-017 Cannot create another process for getty.\n"
$ Do not translate getty.    
MSG_CTV6             "ct: 0835-018 Cannot run getty.\n"
$ Do not translate getty.    
MSG_CT21             "\nTry to connect again?\nType y for yes: "
$ Replace y for yes to the language you translate. 
MSG_CTV7             "The connection is ended.\n"
MSG_CTCD3            "cleanup(%d)\n"
$ This is debug message. Do not translate. 
MSG_CTCD4            "Can't chown/chmod on %s "
$ This is debug message. Do not translate. 
MSG_CTCD5            "to %d\n"
$ This is debug message. Do not translate. 
MSG_CTCD6            "process still active\n"
$ This is debug message. Do not translate. 
$ The following messages are from line.c     
MSG_LINEA1           "program stopped: tty baud rate is not correct."
$ MSG_LINEA2 is an old message and has been commented out since the new
$ message (at the end of this file) has same name.
$ MSG_LINEA2           "program stopped: Error return from ioctl fixline."
$ Do not translate fixline.    
$ Do not translate ioctl.    
MSG_LINEA4           "program stopped: Error return from sytfixline."
$ Do not translate sytfixline.    
MSG_LINEA5           "program stopped: Error return from sytfix2line."
$ Do not translate sytfix2line.    
$ The following messages are from ulockf.c     
MSG_ULOCKFA1         "ulockf: program stopped: There are too many locks."
MSG_ULOCKFA2         "ulockf: program stopped: Cannot allocate for"
$ This message is followed by a filename. 
MSG_ULOCKFL1         "ULOCKC"
$ This is a log message. It is better not to translate this message. 
MSG_ULOCKFL2         "ULOCKLNK"
$ This is a log message. It is better not to translate this message. 
MSG_ULOCKFL3         "ULOCKF"
$ This is a log message. It is better not to translate this message. 
$ The following messages are from line.c     
MSG_ALTC_CD1         "altconn called\r\n"
$ This is debug message. Do not translate. 
MSG_ALTC_CD2         "getto ret %d\n"
$ This is debug message. Do not translate. 
$ The following messages are from altconn.c     
MSG_ALTCONNCD1       "altconn called\r\n"
$ This is debug message. Do not translate. 
MSG_ALTCONNCD2       "getto ret %d\n"
$ This is debug message. Do not translate. 
$ The following messages are from anlwrk.c     
MSG_ANLWRKe1         "anlwrk: 0835-019 Cannot use the old workfile."
$ Do not translate workfile.    
MSG_ANLWRKe2         "anlwrk: 0835-020 A file name in the bldflst subroutine is not valid."
$ Do not translate bldflst.    
$ The following messages are from cico.c     
MSG_CICOA1           "cico: 0835-021 program stopped: The  user number is not correct. "
$ Leave spcae at the end.    
MSG_CICOA2           "cico: 0835-022 program stopped: The  user name is not correct. "
$ Leave space at the end.    
MSG_CICO1            "\tUsage: %1$s %2$s\n"
MSG_CICO6            "here=%s"
$ Do not translate this message. 
MSG_CICOL1           "FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL2           "CALLBACK"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL3           "REQUIRED"
$ This is a log message. It is better not to translate this message. 
MSG_CICO2            "CALL BACK"
$ Do not translate. 
MSG_CICOL4           "PREVIOUS"
$ This is a log message. It is better not to translate this message. 
MSG_CICO3            "BADSEQ"
$ Do not translate. 
MSG_CICO4            "OK"
$ Do not translate. 
MSG_CICOL5           "HANDSHAKE FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CICO5            "BADSEQ"
$ Do not translate. 
MSG_CICOA3           "ULIMIT TOO SMALL"
$ Do not translate. 
MSG_CICOL21          "SYSTEM STATUS"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL6           "CAN NOT CALL"
$ This is a log message. It is better not to translate this message. 
MSG_CICO9            "call to %s "
$ Do not translate. 
MSG_CICOL22          "LOCKED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL7           "CONN FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL8           "SUCCEEDED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOCD1          "Login Successful: System=%s\n"
$ This is debug message. Do not translate. 
MSG_CICOCD2          "HANDSHAKE FAILED: %s\n"
$ This is debug message. Do not translate. 
MSG_CICOL10          "startup"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL11          "TIMEOUT"
$ This is a log message. It is better not to translate this message. 
MSG_CICO7            "conversation complete %s %ld"
$ Do not translate. 
MSG_CICOL17          "conversation complete"
$ This is a log message. It is better not to translate this message. 
MSG_CICOCD3          "send OO %d,"
$ This is debug message. Do not translate. 
$ Do not translate OO.    
MSG_CICOCD4          "Conversation Complete: Status FAILED\n\n"
$ This is debug message. Do not translate. 
MSG_CICOCD5          "Conversation Complete: Status SUCCEEDED\n\n"
$ This is debug message. Do not translate. 
MSG_CICOL19          "CAUGHT"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL20          "INTREXIT"
$ This is a log message. It is better not to translate this message. 
$ Do not translate intrexit.    
MSG_CICO21           "cico: 0835-023 The time out value is not valid.\n"
$ The following messages are from dbg.c     
MSG_DBG1             "dbg: 0835-024 Cannot append to %1$s.\nThe errno is %2$d.\n"
$ Do not translate errno. 
MSG_DBG2             "Opened dbgfile %s."
$ Do not translate dbgfile.    
MSG_DBG3             "dbg: 0835-025 Cannot open /dev/tty because of %1$s %2$s."
$ Do not translate /dev/tty    
$ The following message is from expfile.c     
MSG_EXPFILE1         "expfile: 0835-026 File name %s is not correct.\n"
MSG_EXPFILE2         "expfile: 0835-027 File name %s is not correct. Use ascii characters only.\n"
$ Do not translate ascii.    
$ The following message is from culine.c     
MSG_CULINECD1        "fixline(%d, "
$ This is debug message. Do not translate. 
MSG_CULINEA1         "culine: program stopped: The speed is not correct."
MSG_CULINECD2        "fixline - direct\n"
$ This is debug message. Do not translate. 
$ MSG_CULINEA2 is an old message and has been commented out since the new
$ message (at the end of this file) has same name.
$ MSG_CULINEA2         "program stopped: Error return from ioctl in fixline."
$ Do not translate fixline.    
$ Do not translate ioctl.    
$ The following message is from cu.c     
MSG_CUJNK            "\tUsage: %s [-s Speed] [-l Line] [-dhtu] [-o|-e] telno | systemname\n"
MSG_CU2              "cu: 0835-028 The connection failed. %s\r\n"
MSG_CU3              "cu: 0835-029 Cannot open %s.\r\n"
MSG_CU4              "cu: 0835-030 The remote line is gone.\r\n"
MSG_CU5              "cu: 0835-031 Cannot run a shell.\r\n"
MSG_CU6              "cu: 0835-032 Cannot change direction %s.\r\n"
MSG_CU7              "Use ~~ to start a line with the ~ character.\r\n"
MSG_CU8              "after %ld bytes.\r\n"
$ This line is constructed. 
MSG_CU9              "%d lines/"
$ This line is constructed. 
MSG_CU10             "%ld characters\r\n"
$ This line is constructed. 
MSG_CU11             "The file transmission was interrupted.\r\n"
MSG_CU12             "cu: 0835-033 Cannot create another process at this time.\r\n"
MSG_CU13             "r\ncu: 0835-034 Cannot transmit the special character `%#o`.\r\n"
MSG_CU14             "\nThe line is too long.\r\n"
MSG_CU15             "r\ncu: 0835-035 There is an input or output error.\r\n"
MSG_CU16             "Use the '~$' command.\r\n"
MSG_CU17             "Use '~+' commmand.\r\n"
MSG_CU18             "cu: 0835-036 Cannot get the terminal status.\r\n"
MSG_CU19             "cu: 0835-037 Cannot use the cu from a 3B console.\r\n"
$ Do not translate cu.    
$ Do not translate 3B.    
MSG_CU20             "cu: 0835-038 There is a parity option error.\r\n"
MSG_CU21             "cu: 0835-039 The telephone number cannot exceed 58 digits.\r\n"
MSG_CU22             "Enter the number: "
MSG_CUV1             "cu: 0835-040 Cannot run the seteuid or setegid command.\n"
$ Do not translate setegid.    
$ Do not translate seteuid.    
MSG_CUV2             "Connected\007\r\n"
$ The digits 007 cause a bell to ring.   
MSG_CUCD1            "tranmission started\n\r"
$ This is debug message. Do not translate. 
MSG_CUCD2            "got break in transmit\n\r"
$ This is debug message. Do not translate. 
MSG_CUCD3            "got a tintr\n\r"
$ This is debug message. Do not translate. 
MSG_CUCD4            "call tilda(%s)\r\n"
$ This is debug message. Do not translate. 
MSG_CUV3             "(continue)"
MSG_CUCD5            "call _shell(%s)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD6            "call _dopercen(\"%s\")\r\n"
$ This is debug message. Do not translate. 
$ Do not translate dopercen.    
MSG_CUV4             "\tUsage: ~%%take From [To]\r\n"
MSG_CUV5             "\tUsage: ~%%put From [To]\r\n"
MSG_CUV6             "cu: 0835-041 Cannot change to the %s directory.\r\n"
MSG_CUV7             "cu: 0835-042 Cannot change to the %s directory.\r\n"
MSG_CUV12            "cu: 0835-043 ~%%%s is not known to the cu command.\r\n"
$ Do not translate %%%s.   
MSG_CUCD8            "_receive started\r\n"
$ This is debug message. Do not translate. 
MSG_CUV13            "\r\nLost carrier.\r\n"
MSG_CUCD9            "call _mode(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD10           "got read error. not EINTR\n\r"
$ This is debug message. Do not translate. 
MSG_CUV8             "\n\rcu: 0835-044  The output is blocked.\r\n"
MSG_CUCD11           "call _rcvdead(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD12           "call _quit(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD13           "call _bye(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUV9             "\r\nThe connection is ended.\007\r\n"
$ The digit 007 cause a bell to ring.   
$ Do not translate \007. 
MSG_CUCD14           "call cleanup(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUV10            "\rThe status of device fd is %d.\n"
$ Do not translate fd.    
MSG_CUV11            "\rF_GETFL=%o,"
$ Do not translate. 
$ The following messages is from cntrl.c     
MSG_CNTRL9           "REQUEST"
$ Do not translate this message. 
MSG_CNTRLCD1         "Request: %s\n"
$ This is debug message. Do not translate. 
MSG_CNTRL10          "DENIED"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL11          "ACCESS"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL12          "access denied"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL13          "cntrl: 0835-045 Access denied.\n"
$ This is a log message. It is better not to translate this message. 
$ Do not translate /etc/temp.    
MSG_CNTRL14          "cntrl: 0835-046 Cannot access"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL15          "cntrl: 0835-047 Cannot read %1$s %2$d."
MSG_CNTRL16          "cntrl: 0835-048 Failed."
MSG_CNTRL17          "cntrl: 0835-049 Cannot read %s.\n"
MSG_CNTRLCD2         "Failed: Access Denied - File: %s\n"
$ This is debug message. Do not translate. 
MSG_CNTRL18          "cntrl: 0835-050 Cannot create the temporary file."
MSG_CNTRLCD3         "Failed: No Space!\n"
$ This is debug message. Do not translate. 
MSG_CNTRL20          "Requested"
MSG_CNTRL22          "cntrl: 0835-051 There are not enough parameters in the slave SNDFILE."
$ Do not translate cntrl.    
MSG_CNTRL23          "Remote requested."
MSG_CNTRL24          "Remote requested: %s\n"
MSG_CNTRL26          "Permission"
MSG_CNTRL27          "cntrl: 0835-052 Cannot open."
MSG_CNTRL29          "cntrl: 0835-053 Cannot create a temporary file.\n"
MSG_CNTRL30          "cntrl: 0835-054 There is an input failure."
MSG_CNTRL31          "In send or slave mode."
MSG_CNTRL32          "Copy"
MSG_CNTRL33          "Converting System V multi-hop request."
MSG_CNTRL35          "cntrl: 0835-055 There are not enough parameters in slave RCVFILE."
$ Do not translate RCVFILE.    
MSG_CNTRL37          "The remote has requested %s.\n"
MSG_CNTRL39          "Requesting"
MSG_CNTRL40          "cntrl: 0835-056 Access is denied.\n"
MSG_CNTRL19          "Permission"
MSG_CNTRL21          "cntrl: 0835-057 Access is denied.\n"
MSG_CNTRL41          "Cannot open"
MSG_CNTRL42          "cntrl: 0835-058 Cannot open %s.\n"
MSG_CNTRL43          "BAD READ"
$ Do not translate this message. 
MSG_CNTRL44          "Expected %1$c: Received %2$s."
MSG_CNTRL45          "BAD READ"
$ Do not translate this message. 
MSG_CNTRL55          "The copy succeeded."
MSG_CNTRL46          "Request: %1$s\n(System: %2$s)  %3$s.\n"
MSG_CNTRL47          "Request: %1$s\n(System: %2$s)  %3$s.\n"
MSG_CNTRL48          "The uucp job is: %1$s (%2$s)."
MSG_CNTRL49          "Protocol started %c.\n"
MSG_CNTRL50          "Protocol handshaking failed '%c'.\n"
$ Do not translate handshaking. 
MSG_CNTRL51          "%1$s from %2$s!%3$s has arrived.\n"
MSG_CNTRL52          "The freespace is low."
MSG_CNTRL53          "The remote transfer is denied. "
MSG_CNTRL54          "There are too few inodes."
$ Do not translate inode.    
MSG_CNTRLN0          "cntrl: 0835-059 The copy failed.  The reason for the failure is not given by the remote."
MSG_CNTRLN1          "cntrl: 0835-060 Local access to the file is denied."
MSG_CNTRLN2          "cntrl: 0835-061 Remote access to a path or a file is denied."
MSG_CNTRLN3          "cntrl: 0835-062 There is a system error. A generated uucp command is not valid."
$ Do not translate uucp.    
MSG_CNTRLN4          "cntrl: 0835-063 The remote system cannot create a temporary file."
MSG_CNTRLN5          "cntrl: 0835-064 Cannot copy to a file or a directory.\n\tThe file remains in PUBDIR/user/file."
$ Do not translate PUBDIR/user/file.    
MSG_CNTRLN6          "cntrl: 0835-065 Cannot copy to a file or a directory.\n\tThe file remains in PUBDIR/user/file."
$ Do not translate PUBDIR/user/file.    
MSG_CNTRLN7          "cntrl: 0835-066 There is a forwarding error."
$ The following message is from gnxseq.c     
MSG_GX1              "gnxseq: 0835-067 Cannot lock."
$ The following message is from gtcfile.c     
MSG_GTC1             "gtcfile: 0835-068 There are too many saved C files."
$ The following messages are from gename.c     
MSG_GE1              "null sys -> %s\n"
$ Do not translate this message. 
MSG_GE2              "file - %s\n"
MSG_GE3              "gename: 0835-069 There is a SYSLST overflow."
$ Do not translate SYSLST.    
MSG_GE4              "%s seq "
MSG_GE5              "now %x\n"
$ The following messages are from mailst.c     
MSG_MAILST1          "%s mail %s"
$ Do not translate. 
MSG_MAILST2          "\n\t===== The standard error was =====\n"
MSG_MAILST3          "\n\t===== UUCP general failure =====\n"
$ Do not translate UUCP.  
MSG_MAILST4          "\n\t===== The standard input was ===== \n"
MSG_MAILST5          "0835-156 error running mail command: %m"
$ Do not translate mail.
$ Do not change the %m.
$ Does not require a component ID.
MSG_MAILST6          "0835-157 the mail command returned %#x status"
$ Do not translate mail.
$ Does not require a component ID.
$ The following messages are from pk0.c     
MSG_PK0_1            "not control\n"
MSG_PK0_2            "pk0: 0835-070 The alloc change is not implemented.\n"
$ Do not translate alloc.    
MSG_PK0_4            "pk0: 0835-071 The srj is not implemented.\n"
$ Do not translate srj.    
MSG_PK0_7            "r short %d want %d\n"
$ Do not translate r short. 
MSG_PK0_5            "The rcount is %d.\n"
$ Do not translate rcount.    
MSG_PK0_6            "The xcount is %d.\n"
$ Do not trnaslate xcount.    
$ The following messages are from pk1.c     
MSG_PK1_1            "Dead!"
MSG_PK1_2            "INIT code a."
$ Do not translate INIT.    
MSG_PK1_3            "INIT code b."
$ Do not translate INIT.    
MSG_PK1_4            "O.K."
$ Do not translate this message. 
MSG_PK1_5            "Rcv/Xmit"
$ Do not translate this message. 
MSG_PK1_6            "RREJ?"
$ Do not translate this message. 
MSG_PK1_7            "PDEBUG?"
$ Do not translate this message. 
MSG_PK1_8            "Draino..."
$ Do not translate this message. 
MSG_PK1_9            "Waiting."
MSG_PK1_10           "The link is down."
MSG_PK1_11           "RCLOSE?"
$ Do not translate RCLOSE.    
MSG_PK1_12           "pk1: 0835-072 There is a frame that is not valid."
MSG_PK1_13           "End of line."
MSG_PK1_19           "State -"
$ The following message is from tio.c     
MSG_TIO2             "Sent data: %1$ld bytes in %2$ld.%3$02d seconds."
MSG_TIO5             "Received data: %1$ld bytes in %2$ld.%3$02d seconds."
MSG_TIO6             "Received data: %1$ld bytes in %2$ld.%3$02d seconds."
$ The following message is from permission.c     
MSG_PERM_A1          "program stopped: 0835-073 The following is not a valid option in the permission file :"
MSG_PERM_1           "permission: 0835-074 The following is not a valid option in the permission file:%s\n"
MSG_PERM_2           "permission: 0835-075 There is not enough memory available now.\n"
$ The following messages are from sys5uux.c     
MSG_SYS5_1           "%c return status in event of a failure.\n"
MSG_SYS5_2           "%c the return address for status\n"
MSG_SYS5_3           "QUEUED"
$ Do not translate. 
$ The following messages are from systat.c     
MSG_SYSTAT_A1        "program stopped: 0835-076 Cannot open SYSTAT."
$ Do not translate SYSTAT.   
MSG_SYSTAT_L1        "RETRY TIME NOT REACHED"
$ This is a log message. It is better not to translate this message. 
MSG_SYSTAT_L2        "NO CALL"
$ This is a log message. It is better not to translate this message. 
$ The following messages are from uucleanup.c     
MSG_UCL_1            "\tUsage: [-oDays] [-mString] [-CDays] [-DDays] [-WDays] [-XDays] [-xLevel] [-sSystem]"
MSG_UCL_2            "Subject: Mail that cannot be delivered.\n"
MSG_UCL_3            "This mail message cannot be delivered.\n"
MSG_UCL_4            "(Probably to or from system %s)\n"
MSG_UCL_5            "It was sent to you or sent by you.\n"
MSG_UCL_6            "Sorry for the inconvenience.\n"
MSG_UCL_7            "Subject: Warning from uucp.\n"
$ Do not translate uucp.    
MSG_UCL_8            "uucleanup: Not able to contact machine %s since the job was queued.\n"
MSG_UCL_9            "The job will be deleted in several days if the problem is not corrected.\n"
MSG_UCL_10           "To stop the job, run the following command:\n"
MSG_UCL_11           "\tUsage: %s %s\n"
MSG_UCL_12           "The -C, -D, -W, -X, and -o flags require a value greater than zero.\n"
MSG_UCL_13           "uucleanup: 0835-077 Cannot change to directory %1$s. The errno is %2$d.\n"
MSG_UCL_14           "uucleanup: 0835-078 Cannot open %1$s. The errno is %2$d.\n"
MSG_UCL_15           "uucleanup: 0835-079 dDprocess: Dummy command file %s unlinked."
$ Do not translate dDProcess.    
MSG_UCL_16           "uucleanup: 0835-080 cprocess: Bad command file %1$s, unlink %2$s."
MSG_UCL_17           "uucleanup: 0835-081 The job %s has been deleted.\n"
MSG_UCL_18           "uucleanup: 0835-082 cprocess: C. %1$s, mail returned %2$d, unlink %3$s."
$ Do not translate cprocess.    
MSG_UCL_19           "uucleanup: 0835-083 wprocess: %1$s: %2$s. Warning message sent to %3$s, returned %4$d."
$ Do not translate wprocess.    
MSG_UCL_20           "uucleanup: 0835-084 dMprocess: mail %1$s to %2$s!%3$s, returned %4$d, unlink %5$s."
$ Do not translate dMprocess.    
MSG_UCL_21           "uucleanup: 0835-085 dNprocess: Local news item unlink %s."
$ Do not translate dNprocess.    
MSG_UCL_22           "uucleanup: 0835-086 dNprocess: Remote run of rnews %1$s, returned %2$d, unlink %3$s."
$ Do not translate dNprocess.    
$ Do not translate rnews.    
MSG_UCL_23           "\n\tSincerely,\n\t%s!uucp\n"
$ Do not translate uucp.    
MSG_UCL_24           "##### Data File: ############################\n"
MSG_UCL_25           "Subject: uucp deleted the job.\n"
$ Do not translate uucp.    
MSG_UCL_26           "uucleanup: 0835-087 Cannot contact machine '%s'.\n"
MSG_UUCLEAN1         "Subject: Mail that cannot be delivered.\n"
MSG_UUCLEAN2         "This mail message cannot be delivered.\n"
MSG_UUCLEAN3         "(Probably to or from system %s.\n"
MSG_UUCLEAN4         "It was sent to you or sent by you.\n"
MSG_UUCLEAN5         "Sorry for the inconvenience.\n"
MSG_UUCLEAN6         "Subject: uucp deleted the job.\n"
$ Do not translate uucp.    
MSG_UUCLEAN7         "uucleanup: 0835-088 Cannot contact machine %s.\n"
MSG_UUCLEAN8         "Subject: Warning from uucp.\n"
$ Do not translate uucp.    
MSG_UUCLEAN9         "uucleanup: 0835-089Not able to contact machine %s since job was queued.\n"
MSG_UUCLEAN10        "The job will be deleted in several days if the problem is not corrected.\n"
MSG_UUCLEAN11        "To delete the job, run the following command:\n"
$ The following messages are from uucp.c     
MSG_UUCP1            "uucp: 0835-090 %s is not a known flag.\n"
MSG_UUCP2            "uucp: 0835-091 There is no work directory named %s.\n"
MSG_UUCP3            "\tUsage: uucp from SourceFiles to DestinationFiles\n"
$ Do not translate uucp.    
MSG_UUCP4            "uucp: 0835-092 %s is not a valid system.\n"
MSG_UUCP5            "uucp: 0835-093 %s is not a valid syntax.\n"
MSG_UUCP6            "uucp: 0835-094 ULIMIT %ld is less than MINULIMIT %ld."
$ Do not translate ULIMIT.    
$ Do not translate MINULIMIT.    
MSG_UUCP7            "Low-ULIMIT"
$ Do not translate ULIMIT.    
MSG_UUCP8            "uucp: 0835-095 uucp failed completely. There was %d error .\n"
$ Do not translate uucp.    
MSG_UUCP9            "uucp: 0835-096 uucp failed partially. There were %d errors."
$ Do not translate uucp.    
MSG_UUCP9P           "uucp: 0835-097 uucp failed partially. The error code is %d."
$ Do not translate uucp.    
MSG_UUCP10           "uucp: 0835-098 Cannot get file status %s.\nThe copy failed.\n"
MSG_UUCP11           "uucp: 0835-099 %1$s and %2$s are the same file. Cannot copy.\n"
MSG_UUCP12           "uucp: 0835-100 Permission is denied.\n"
MSG_UUCP13           "uucp: 0835-101 The directory name %s is not valid.\n"
MSG_UUCP14           "uucp: 0835-102 uucp cannot read %s; access permission %o.\n"
$ Do not translate uucp.    
MSG_UUCP14C          "Use the cp command.\n"
$ Do not translate cp.    
MSG_UUCP15           "uucp: 0835-103 Cannot write file %1$s; access permission %2$o.\n"
MSG_UUCP16           "uucp: 0835-104 Cannot copy file %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUCP17           "QUEUED"
$ Do not translate this message. 
MSG_UUCP18           "uucp: 0835-105 Cannot get status for file %s.\n"
MSG_UUCP19           "uucp: 0835-106 The directory name %s is not valid.\n"
$ The following messages are from uucpdefs.c     
MSG_UDEFS_1          "CAN'T OPEN"
$ Do not translate this message. 
MSG_UDEFS_2          "CAN'T WRITE"
$ Do not translate this message. 
MSG_UDEFS_3          "CAN'T READ"
$ Do not translate this message. 
MSG_UDEFS_4          "CAN'T CREATE"
$ Do not translate this message. 
MSG_UDEFS_5          "CAN'T ALLOCATE"
$ Do not translate this message. 
MSG_UDEFS_6          "CAN'T LOCK"
$ Do not translate this message. 
MSG_UDEFS_7          "CAN'T STAT"
$ Do not translate this message. 
MSG_UDEFS_8          "CAN'T CHOWN"
$ Do not translate this message. 
MSG_UDEFS_9          "CAN'T CHMOD"
$ Do not translate this message. 
MSG_UDEFS_10         "CAN'T LINK"
$ Do not translate this message. 
MSG_UDEFS_11         "CAN'T CHDIR"
$ Do not translate this message. 
MSG_UDEFS_12         "CAN'T UNLINK"
$ Do not translate this message. 
MSG_UDEFS_13         "WRONG ROLE"
$ Do not translate this message. 
MSG_UDEFS_14         "CAN'T MOVE TO CORRUPTDIR"
$ Do not translate this message. 
MSG_UDEFS_15         "CAN'T CLOSE"
$ Do not translate this message. 
MSG_UDEFS_16         "CAN'T FORK"
$ Do not translate this message. 
MSG_UDEFS_17         "FILE EXISTS"
$ Do not translate this message. 
$ The following messages come from uuname.c     
MSG_UUNAME1          "\tUsage: uuname [-l | -c]\n"
$ Do not translate uuname.    
MSG_UUNAME2          "uuname: 0835-109 %s is a protected file.\n"
$ The following messages come from uusched.c    
MSG_UUSCHED1         "[-xNumber] [-uNumber]"
MSG_UUSCHED2         "\tUsage: %1$s %2$s\n"
MSG_UUSCHED3         "MACHINE TABLE FULL"
$ Do not translate this message. 
$ The following messages come from uustat.c     
MSG_UUSTAT1          "[-a] [-q|-nNUMBER|-m|-kJOB|-rJob|-p]"
MSG_UUSTAT2          "[-sSystem] [-uUser]"
MSG_UUSTAT3          "\tUsage: %1$s %2$s\n"
MSG_UUSTAT4          "or\n\tUsage: %1$s %2$s\n"
MSG_UUSTAT5          "uustat: 0835-110 Cannot find the job %s.  Not renewed.\n"
MSG_UUSTAT6          "uustat: 0835-111 Cannot get status for %1$s. The errno is%2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT7          "uustat: 0835-112 Cannot get status for %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT8          "uustat: 0835-113 %1$s is not a valid format. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT9          "uustat: 0835-114 %1$s is not a valid format. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT10         "uustat: 0835-115 Only the job owner or root can delete job %s.\n"
MSG_UUSTAT11         "uustat: 0835-116 Only the job owner or root can renew job %s.\n"
MSG_UUSTAT12         "uustat: 0835-117 Cannot delete file %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT13         "uustat: 0835-118 Cannot renew file %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT16         "uustat: 0835-119 The job %s is successfully deleted .\n"
MSG_UUSTAT17         "uustat: 0835-120 The job %s was successfully renewed.\n"
MSG_UUSTAT18         "uustat: 0835-121 Warning: Table overflow. The output is not complete.\n"
MSG_UUSTAT19         "uustat: 0835-122 Cannot find job %s. It was not deleted.\n"
MSG_UUSTAT_E1        "MACHINE TABLE FULL"
$ Do not translate this message. 
MSG_UUSTAT21	     "malloc: Out of memory!!\n"
$ Do not translate ascii.    
$ The following messages come from uux.c     
MSG_UUX_1            "[-aName] [-b] [-c] [-C] [-j] [-gGrade] [-n] [-p] [-r] [-sFile] [-xNumber]"
MSG_UUX_2            "\tUsage: %1$s %2$s\n"
MSG_UUX_3            "uux: 0835-124 There is no spool directory %s.\n"
MSG_UUX_4            "uux: 0835-125 %s is not a valid system name.\n"
MSG_UUX_5            "%c for return status on success.\n"
MSG_UUX_6            "%c to not return status on failure.\n"
MSG_UUX_7            "%c to return status on failure.\n"
MSG_UUX_8            "%c to return input on abnormal exit.\n"
MSG_UUX_9            "%c to return address for status, or input return.\n"
MSG_UUX_11           "uux: 0835-126 %s is not a valid system name.\n"
MSG_UUX_12           "uux: 0835-127 Cannot get status for file %s.\n"
MSG_UUX_13           "uux: 0835-128 Permission is denied %s.\n"
MSG_UUX_14           "uux: 0835-129 uux failed. The errno is %d.\n"
$ Do not translate errno. 
$ Do not translate uux.    
MSG_UUX_15           "uux: 0835-130 Cannot copy %s.\n"
$ The following messages come from uuxqt.c     
MSG_UUXQT_1          "[-xDebug] [-sSystem]"
MSG_UUXQT_2          "\tUsage: %1$s %2$s\n"
MSG_UUXQT_3          "\tUsage: %1$s %2$s\n"
MSG_UUXQT_4          "%s XQT DENIED"
$ Do not translater this message.    
MSG_UUXQT_5          "uuxqt: 0835-134 Permission to run %s is denied."
MSG_UUXQT_6          "uuxqt: 0835-135 %s: XQT- stdin or stdout or file access is denied."
$ Do not translate XQT.   
MSG_UUXQT_7          "uuxqt: 0835-136 File access to %s is denied."
MSG_UUXQT_8          "uuxqt: 0835-137 Exited with status %d."
MSG_UUXQT_9          "Stopped by signal %d."
MSG_UUXQT_10         "COMMAND FAIL"
$ Do not translate this message. 
$ The following message is from versys.c     
MSG_VER_1            "versys: 0835-138 System name %s must contain ascii characteri only.\n"
$ The following messages are from getopt.c     
MSG_GOPT_1           "getopt: 0835-139 The following flag is not valid: "
MSG_GOPT_2           "getopt: 0835-140 The following flag requires a parameter: "
$ The following messages are from uucpd.c     
MSG_UUCPD_1          "user read\n"
MSG_UUCPD_2          "uucpd: 0835-141 The user is not known.\n"
MSG_UUCPD_3          "uucpd: 0835-142 Login is not correct."
MSG_UUCPD_4          "passwd read\n"
$ Do not translate passwd.    
MSG_UUCPD_5          "uucpd: 0835-143 Ther login is not correct."
MSG_UUCPD_6          "%1$s: 0835-144 %2$d, SRC not found, continuing without SRC support.\n"
$ Do not translate SRC. 
MSG_UUCPD_7          "%1$s: 0835-145 %2$d recvfrom.\n"
$ Do not translate recvfrom.    
MSG_UUCPD_8          "uucpd: 0835-146 The command does not support this option.\n"
MSG_UUCPD_9          "setsockopt SO_DEBUG: %m"
$ Do not translate this message. 
$ The following messages are for uuclean.c     
B_CLEAN_1            "Usage:  uuclean [-m] [-n Hours] [-d Subdirectory] [-p Prefix] . . .\n"
B_CLEAN_2            "uuclean: 0835-147 Specify a maximum of 10 prefixes.\n"
B_CLEAN_3            "uuclean: 0835-148 Prefix %s is too long.\n"
B_CLEAN_4            "uuclean: 0835-149 Cannot open the subdirectory %s.\n"
B_CLEAN_5            "uuclean: 0835-150 Cannot get status for %s.\n"
B_CLEAN_6            "uuclean: 0835-151 File %1$s is deleted.\nCannot contact remote. %2$d requests deleted.\n"
B_CLEAN_7            "Request: "
B_CLEAN_8            "First request : "
B_CLEAN_9            "\nLast request : "
B_CLEAN_10           "uuclean deleted file %s.\n"
$ Do not translate uuclean.    
$ The following messages are from uupoll.c     
MSG_UUPOLL_1         "\tUsage: uupoll [-gGrade] [-n] SystemName\n"
MSG_UUPOLL_2         "Current system is %s\n"
MSG_UUPOLL_3         "uupoll: 0835-152 %s is not a known system.\n"
MSG_UUPOLL_4         "uupoll: 0835-153 Cannot create directory %s.\n"
MSG_UUPOLL_5         "uupoll: 0835-154 Cannot write to %s.\n"
MSG_UUPOLL_6         "uupoll: 0835-155 Cannot run the uusched command.\n"
MSG_UUPOLL_7         "uupoll: 0835-158 Cannot run the uucico command.\n"
$ Do not translate uusched.    
MSG_LINEA2           "program stopped: Error return from tcsetattr() in fixline."
$ Do not translate fixline.    
$ Do not translate tcsetattr().    
MSG_CULINEA2         "program stopped: Error return from tcsetattr() in fixline."
$ Do not translate fixline.    
$ Do not translate tcsetattr().    
$ The following message is from cu.c     
MSG_CU1              "\tUsage: %s [-s Speed] [-l Line] [-dhtu] [-m] [-o|-e] telno | systemname\n"
MSG_CICOA4           "cico: cannot access the spool directory\n"
MSG_CU1A             "\tUsage: %s [-s Speed] [-l Line] [-dht] [-m] [-o|-e] telno | systemname\n"
MSG_CU23             "cu: cannot read Systems files\n"
MSG_CU24             "cu: cannot read Devices files\n"
MSG_CU25             "cu: cannot read Dialers files\n"
MSG_CU26             "cu: cannot have both even and odd parity\n"
MSG_CU27             "cu: bad phone number %s\nPhone numbers may contain only the digits 0 through 9 and the special\ncharacters =, -, * and #.\n",
MSG_CU28             "cu: warning: -l flag ignored when system name used\n"
MSG_CU29             "cu: warning: -s flag ignored when system name used\n"
MSG_MAILST7          "empty =====\n"
MSG_MAILST8          "denied read permission =====\n"
MSG_MAILST9          "unreadable =====\n"
MSG_UUCLEAN12        "WARNING: %s: invalid debug level %s ignored, using level %s\n"
MSG_UUCP3A           "\tUsage:  uucp [-c|-C] [-d|-f] [-gGRADE] [-j] [-m] [-nUSER]\\\n [-r] [-sFILE] [-xDEBUG_LEVEL] source-files destination-file\n"
MSG_UUCP9A           "uucp failed partially. %d files were transferred with %d errors."
MSG_UUCP9Q           "uucp failed partially. %d files were transferred. The error code is %d."
MSG_UUNAME3          "uuname: Cannot access protected systems files.\n"
MSG_UUNAME4          "uuname: -l option overrides -c option.  -c ignored.\n"
MSG_UUSCHED4         "WARNING: $s: Invalid debug level %s ignored. Using level 1.\n"
MSG_UUSCHED5         "WARNING: uusched built w/ small flag defined.  Debugging info is not available.\n"
MSG_UUSTAT20         "uustat: System name is invalid\n"
MSG_UUSTAT22         "uustat: The user name you specified is invalid\n"
B_CLEAN_11           "Unknown option %s\n"
MSG_UUX_16           "Usage: uux [-aName] [-b] [-c] [-C] [-j] [-gGrade] [-n] [-p] [-r] [-sFile] [-xNumber] [-z] command_string\n"
MSG_CICO23           "%s: Invalid transfer grade of %c is being ignored.\n"
MSG_CICO24           "Usage: uucico [-xDebuglevel] [-r[0|1]] [-gTransfergrade] -sSYSTEM [-uUSERID] [-dSPOOL]\n"
$ -------------------- Also from uucpdefs.c ---------------------
MSG_OK               "SUCCESSFUL."
MSG_NO_DEVICE        "NO DEVICES AVAILABLE."
MSG_TIME_WRONG       "WRONG TIME TO CALL."
MSG_INPROGRESS       "TALKING."
MSG_CONVERSATION     "CONVERSATION FAILED."
MSG_SEQBAD           "BAD SEQUENCE CHECK."
MSG_LOGIN_FAILED     "LOGIN FAILED."
MSG_DIAL_FAILED      "DIAL FAILED."
MSG_BAD_LOG_MCH      "BAD LOGIN/MACHINE COMBINATION."
MSG_LOCKED_DEVICE    "DEVICE LOCKED."
MSG_ASSERT_ERROR     "ASSERT ERROR."
MSG_BADSYSTEM        "SYSTEM NOT IN Systems FILE(s)."
MSG_CANT_ACCESS      "CAN'T ACCESS DEVICE."
MSG_DEVICE_FAILED    "DEVICE FAILED."
MSG_WRONG_MCH        "WRONG MACHINE NAME."
MSG_CALLBACK         "CALLBACK REQUIRED."
MSG_RLOCKED          "REMOTE HAS A LCK FILE FOR ME."
MSG_RUNKNOWN         "REMOTE DOES NOT KNOW ME."
MSG_RLOGIN           "REMOTE REJECT AFTER LOGIN."
MSG_UNKNOWN_RESPONSE "REMOTE REJECT, UNKNOWN MESSAGE."
MSG_STARTUP          "STARTUP FAILED."
MSG_CHAT_FAILED      "CALLER SCRIPT FAILED."
MSG_NOSUCH_HOST      "HOSTNAME NOT RESOLVED."
$ -------------------- End of uucpdefs.c extractions ---------------------
MSG_UUQ_1            "Usage: uuq [-l] [-h] [-s System] [-u User] [-d jobnumber] [-r spooldir]\n\t[-b baudrate]\n"
MSG_CT23             "Usage: ct [-h] [-v] [-w maximum_wait] [-x debug_level] [-s speed] telephone_number\n"
MSG_CT24             "ct: Cannot open tty stream. error number is %d.\n"
MSG_CT25             "Confirm: Hang up your terminal before dialing? (y/n): "
MSG_CT26             "To dial without closing your terminal, use the -h flag.\n"
MSG_UUX_17           "The operators <<, >>, >|, and >& cannot be used in a command string.\n"
