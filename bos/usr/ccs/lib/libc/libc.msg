$ @(#)16 1.27.2.17 src/bos/usr/ccs/lib/libc/libc.msg, libcmsg, bos411, 9428A410j 3/4/94 10:34:11
$
$ COMPONENT_NAME: (LIBC) commands that deal with the file system
$
$ FUNCTIONS: libc messages
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ LEVEL 1, 5 Years Bull Confidential Information
$
$quote "         define quote character.


$set MS_LIBC

$ $ The following are from libc/gen/errlst.c, for perror
1 "Operation not permitted."

2 "A file or directory in the path name does not exist."

3 "The process does not exist."

4 "A system call received an interrupt."

5 "There is an input or output error."

6 "There is a request to a device or address that does not exist."

7 "The parameter or environment lists are too long."

8 "Cannot run a file that does not have a valid format."

9 "A file descriptor does not refer to an open file."

10 "There are no child processes."

11 "Resource temporarily unavailable"

12 "There is not enough memory available now."

13 "The file access permissions do not allow the specified action."

14 "A memory address is not in the address space for the process."

15 "A specified file must be a block device."

16 "The requested resource is busy."
$ Old message -> "A device is already mounted or cannot be unmounted."
$ Not sure if "mounted" or "unmounted" should be translated.
$ They refer to the results of using the "mount" and "umount" system calls.

17 "Do not specify an existing file."

18 "Cannot link to a file on another device."

19 "The specified device does not exist."

20 "A parameter must be a directory."

21 "Cannot write to a directory."

22 "A system call received a parameter that is not valid."

23 "The system file table is full."

24 "The process file table is full."

25 "A specified file does not support the ioctl system call."
$ Do not translate: ioctl

26 "Cannot open or remove a file containing a running program."

27 "A file cannot be larger than the value set by ulimit."
$ Do not translate: ulimit
$ It is the name of a command and a system call

28 "There is not enough space in the file system."

29 "A file or device is not appropriate for the lseek system call."
$ Do not translate: lseek

30 "The file system has read permission only."

31 "There are too many links to a file."

32 "There is no process to read data written to a pipe."

33 "A parameter to a math subroutine is not in the domain."
$ Domain is a math term for the values for which a function is defined

34 "A return value of a math subroutine is not within machine precision."

35 "The specified message type does not exist on a message queue."

36 "An identifier does not exist."


37 "Channel number out of range."
$ Do not translate.  This one is never seen.

38 "Level 2 not synchronized."
$ Do not translate.  This one is never seen.

39 "Level 3 halted."
$ Do not translate.  This one is never seen.

40 "Level 3 reset."
$ Do not translate.  This one is never seen.

41 "Link number out of range."
$ Do not translate.  This one is never seen.

42 "Protocol driver not attached."
$ Do not translate.  This one is never seen.

43 "No CSI structure available."
$ Do not translate.  This one is never seen.

44 "Level 2 halted."
$ Do not translate.  This one is never seen.

45 "Cannot lock a file because of a potential deadlock condition."

46 "The device is not ready for operation."

47 "The input or output media is write-protected."

48 "The media is not formatted or the format is not correct."

49 "The system lock table is full."

50 "Cannot establish a connection to a remote service."

51 "A connection to a remote service is not available."

52 "A file, file system or message queue is no longer available."

$ The following message is no longer used and should not be translated
53 "System call error number 53."

$ The following message is not seen by the user 
54 "The system call would have caused the process to be suspended."

55 "A connect operation on a socket is not complete."
$ Do not translate: connect

56 "A socket operation is already in progress."

57 "The specified file descriptor is not a socket."

58 "There is no destination address for the socket operation."

59 "A message for a socket data transfer is too long."

60 "The socket type or protocols are not compatible."

61 "The protocol does not support the specified option."

62 "No protocol of the specified type and domain exists."

63 "The specified socket type is not supported."

64 "The socket does not allow the requested operation."

65 "The socket protocol family is not supported."

66 "The type of socket is not supported in this protocol family."

67 "The socket name is already in use."

68 "The socket name is not available on this system."

69 "The network is not currently available."

70 "Cannot reach the destination network."

71 "A socket is connected to a host that is no longer available."

72 "A connection is ended by software."

73 "A connection with a remote socket was reset by that socket."

74 "There is not enough buffer space for the requested socket operation."

75 "Cannot connect to a socket that is already connected."

76 "A socket must be already connected."

77 "Cannot send data after a shutdown operation on a socket."
$ Do not translate: shutdown

78 "A remote host did not respond within the timeout period."

79 "A remote host refused an attempted connect operation."
$ Do not translate: connect

80 "A remote host is not available."

81 "A route to the remote host is not available."

$ The following message is never seen by the user 
82 "The system call will be restarted."

83 "Cannot exceed the user process limit."

84 "There are too many users on the system."

85 "There are too many levels of symbolic links to translate a path name."

86 "A file or path name is too long."

87 "Cannot delete a directory that is not empty."

88 "Cannot exceed the user or group quota."

89 "Reserved errno was encountered"

90 "Reserved errno was encountered"

91 "Reserved errno was encountered"

92 "Reserved errno was encountered"

93  "Cannot mount a file system that is already remotely mounted."

94  "Reserved errno was encountered"

95  "Reserved errno was encountered"

96  "Reserved errno was encountered"

97  "Reserved errno was encountered"

98  "Reserved errno was encountered"

99  "Reserved errno was encountered"

100 "Reserved errno was encountered"

101 "Reserved errno was encountered"

102 "Reserved errno was encountered"

103 "Reserved errno was encountered"

104 "Reserved errno was encountered"

105 "Reserved errno was encountered"

106 "Reserved errno was encountered"

107 "Reserved errno was encountered"

108 "Reserved errno was encountered"

109 "The system call does not exist on this system."

110 "The media surface is damaged."

111 "The operation required corrective action."

112 "Cannot find the requested security attribute."

113 "Security authentication is denied."

114 "Cannot run the specified program in a trusted environment."

115 "Too many references: can't splice."

116 "Invalid wide character."

117 "Asynchronous I/O cancelled."
$ Do not translate "I/O"

118 "Out of STREAMS resources."
$ Do not translate "STREAMS"

119 "System call timed out."

120 "Message type at stream head is wrong."

121 "Error in STREAMS protocol."
$ Do not translate "STREAMS"

122 "No message ready at stream head."

123 "File descriptor (fd) not associated with a stream."
$ Do not translate "fd"

124 "An attempt was made to set an attribute to an unsupported value."

125 "Multihop access to a remote resource not supported."

126 "Communications link to server, for requested resource, has been lost."

127 "The user ID or group ID is to large to fit in the provided structure."

$ $ The following message is for perror
M_PERROR "System call error number %d.\n"

$ $ The following message is for NLinit
M_NLINIT "Cannot get information about the file\n\
\tlisted in the NLFILE environment variable.\n"
$ Do not translate: NLFILE

$ $ The following messages are for assert
M_ASSERT1 "The assert subroutine failed: "
$ Do not translate "assert"
$ M_ASSERT1 is printed on the same line before ASSERT2 and ASSERT3

M_ASSERT2 ", file "
$ Leave the space at the end
$ M_ASSERT1 is printed along with ASSERT2 and ASSERT3

M_ASSERT3 ", line NNNNN\n"
$ This string MUST contain NNNNN. (a literal 5 N's)
$ M_ASSERT{123}  are printed out like:
$ The assert subroutine failed: some_expression, file FileName, line LineNumber
$ There is an "assert" C library subroutine -- do not translate "assert"


$ $ The following message is for initgroups
M_INITG "The initgroups subroutine failed.\n\
\t%s cannot be in more than 32 groups.\n"
$ Do not translate "initgroups"

$ $ The following messages are for getgrent
M_SETGR	"setgrent: getdomainname system call missing\n"
$ Do not translate -- this will never be seen

M_GETGR	"getgrent: getdomainname system call missing\n"
$ Do not translate -- this will never be seen

$ $ The following messages are for getpwent
M_SETPW "setpwent: getdomainname system call missing\n"
$ Do not translate -- this will never be seen

M_GETPW "getpwent: getdomainname system call missing\n"
$ Do not translate -- this will never be seen


$ $ The following messages are for getopt
M_OPTILL ": Not a recognized flag: "
$ Leave the space at the end.  Something follows.

M_OPTARG ": A flag requires a parameter: "
$ Leave the space at the end.  Something follows.

$ $ The following messages are for getwd
M_RSTAT "The getwd subroutine failed.\n\
\tCannot get information about the root directory.\n"
$ Do not translate "getwd"

M_HSTAT "The getwd subroutine failed.\n\
\tCannot get information about the current directory.\n"
$ Do not translate "getwd"

M_OPENPAR "The getwd subroutine failed.\n\
\tCannot open the parent directory.\n"
$ Do not translate "getwd"

M_READPAR "The getwd subroutine failed.\n\
\tCannot read the parent directory.\n"
$ Do not translate "getwd"

$ $ The following messages are for psignal()
$ $ THIS MUST BE KEPT IN_SYNC WITH siglist.c
M_PSIGNAL "Signal 0"

$ This group of messages (65 or so...M_PSIGNAL through M_NO_SIGNAL) are
$ values used by the "signal" system call.  You may not want to
$ translate these at all.  If they are translated, see the documentation
$ for the signal system call and translate them in
$ whatever way the documentation for that system call uses.

M_SIGHUP "Hangup"

M_SIGINT "Interrupt"

M_SIGQUIT "Quit"

M_SIGILL "Illegal instruction"

M_SIGTRAP "Trace/BPT trap"

M_SIGABRT "IOT/Abort trap"

M_SIGEMT "EMT trap"

M_SIGFPE "Floating point exception"

M_SIGKILL "Killed"

M_SIGBUS "Bus error"

M_SIGSEGV "Segmentation fault"

M_SIGSYS "Bad system call"

M_SIGPIPE "Broken pipe"

M_SIGALRM "Alarm clock"

M_SIGTERM "Terminated"

M_SIGURG "Urgent I/O condition"

M_SIGSTOP "Stopped (signal)"

M_SIGSTP "Stopped"

M_SIGCONT "Continued"

M_SIGCHLD "Child exited"

M_SIGTTIN "Stopped (tty input)"

M_SIGTTOU "Stopped (tty output)"

M_SIGIO "I/O possible/complete"

M_SIGXCPU "Cputime limit exceeded"

M_SIGXFSZ "Filesize limit exceeded"

M_SIG26 "Signal 26"

M_SIGMSG "Input device data"

M_SIGWINCH "Window size changes"

M_SIGPWR "Power-failure"

M_SIGUSR1 "User defined signal 1"

M_SIGUSR2 "User defined signal 2"

M_SIGPROF "Profiling timer expired"

M_SIGDANGER "Paging space low"

M_SIGVTALRM "Virtual timer expired"

M_SIG35 "Signal 35"

M_SIG36 "Signal 36"

M_SIG37 "Signal 37"

M_SIG38 "Signal 38"

M_SIG39 "Signal 39"

M_SIG40 "Signal 40"

M_SIG41 "Signal 41"

M_SIG42 "Signal 42"

M_SIG43 "Signal 43"

M_SIG44 "Signal 44"

M_SIG45 "Signal 45"

M_SIG46 "Signal 46"

M_SIG47 "Signal 47"

M_SIG48 "Signal 48"

M_SIG49 "Signal 49"

M_SIG50 "Signal 50"

M_SIG51 "Signal 51"

M_SIG52 "Signal 52"

M_SIG53 "Signal 53"

M_SIG54 "Signal 54"

M_SIG55 "Signal 55"

M_SIG56 "Signal 56"

M_SIG57 "Signal 57"

M_SIG58 "Signal 58"

M_SIGSAK "Secure attention"

M_SIGGRANT "Monitor mode granted"

M_SIGRETRAC "Monitor mode retracted"

M_SIGSOUND "Sound completed"

M_SIG63 "Signal 63"
M_NO_SIGNAL "Unknown signal"

$ $ The following messages are for initstate
M_INITST "The initstate subroutine failed.\n\
\t%d bytes does not provide enough state information.\n"
$ Do not translate: initstate

M_SETST	"The setstate subroutine failed.\n\
\tThe state information is damaged.\n"
$ Do not translate: setstate

$ $ The following messages are for mcount
M_MONCNT "%1$s: The number of %2$s (%3$u)\n\
\texceeds the profiling limit of %4$u.\n"

$ $ The following messages are for monitor
M_MON1 "%s: There is not enough space for the profiling buffers.\n\
\t%u bytes are needed.\n"

M_MON2 "%s: Cannot process output file %s.\n"

M_MON3 "%s: The address range is not correct.\n\
\tThe low value is %1$8.8x. The high value is %2$8.8x.\n"

M_MON4 "%1$s: The _mondata.prof_type variable (%2$d) must be\n\
\teither _PROF_TYPE_IS_P or _PROF_TYPE_IS_PG.\n"
$ Do not translate:   _mondata.prof_type
$ Do not translate:    _PROF_TYPE_IS_P
$ Do not translate:    _PROF_TYPE_IS_PG

M_MON5 "%s: Cannot find a data area.\n"

M_MON6 "%1$s: The buffer range is not contiguous.\n\
\tp[%2$d] is %3$8.8x.\n\
\tsize[%4$d] is %5$8.8x.\n\
\tp[%6$d] is %7$8.8x.\n"
$ Do not translate:  p[]	size[]

M_MON7 "%1$s: The ranges are not in ascending order.\n\
\thi[%2$d] is %3$8.8x.\n\
\tlo[%4$d] is %5$8.8x.\n"
$ Do not translate:  hi[]	lo[]

$ $ The following messages are for crt0/mcrt0/gcrt0
M_CRT0 "%1$s: The profiling setup call to %2$s failed.\n\
\tThe return code is %3$d.\n"

$ $ The following message is for re_comp
M_RETOOLONG "The regular expression is too large.\n"

M_NOPREV "There is no previous regular expression.\n"

M_UNMATCH "There is not a match for a \\( character.\n"

M_MISSING "A ] character is missing.\n"

M_TOOMANY "There are too many \\( \\) pairs.\n"

M_UNMATCHLEFT "There is not a match for a \\) character.\n"

$ $ The following message is for longjmperr
M_SETJMPER "There is no saved context for the longjmp or siglongjmp function.\n"
$ Do not translate:  longjmp  siglongjmp
$ $	Cause:	This is a programming error with the setjmp and longjmp or
$ $		sigsetjmp and siglongjmp functions.  Either the program does
$ $		not use the setjmp or sigsetjmp function to save the program
$ $		context, or the function containing the setjmp or sigsetjmp
$ $		invocation has terminated in the interim.
$ $
$ $ The messages 1201-196 and 1201-197 are used above for new errnos.
$ $ If you are adding please start at 1201-198
$ $

$ The following are Regular Expression errors
$ The default error text is found in regerror.c

$set MS_REG

$ Do not translate: [] () {} *?+ ^ $

1   "Pattern not found.\n"
2   "Invalid pattern.\n"
3   "Invalid character within [] expression.\n"
4   "Invalid character class within [] expression.\n"
5   "Last character cannot be \\.\n"
6   "Invalid subexpression backreference number"
7   "[] imbalance.\n"
8   "\\(\\) or () imbalance.\n"
9   "\\{\\} or {} imbalance.\n"
10  "Invalid \\{\\} repetition.\n"
11  "Invalid range within [] expression.\n"
12  "Out of memory.\n"
13  "?*+ not preceded by valid expression.\n"
14  "Invalid multibyte character.\n"
15  "^ anchor not at beginning of pattern.\n"
16  "$ anchor not at end of pattern.\n"


$set LIBCYP
CYP1 "yp_all - RPC clnt_call (TCP) failure"
CYP2 "yp operation succeeded"
CYP3 "args to yp function are bad"
CYP4 "RPC failure on yp operation"
CYP5 "cannot bind to a server which serves domain"
CYP6 "no such map in server's domain"
CYP7 "no such key in map"
CYP8 "internal yp server or client error"
CYP9 "local resource allocation failure"
CYP10 "no more records in map database"
CYP11 "cannot communicate with portmapper"
CYP12 "cannot communicate with ypbind"
CYP13 "cannot communicate with ypserv"
CYP14 "local domain name not set"
CYP15 "yp map data base is bad"
CYP16 "yp client/server version mismatch"
CYP17 "permission denied"
CYP18 "database is busy"
CYP19 "unknown yp client error code"

$set LIBCRPC
RPC1 "authunix_create: 1832-002 out of memory\n"
RPC2 "auth_none.c 1832-003: Fatal marshalling problem"
RPC3 "RPC: Success"
RPC4 "RPC: 1832-004 Can't encode arguments"
RPC5 "RPC: 1832-005 Can't decode result"
RPC6 "RPC: 1832-006 Unable to send"
RPC7 "RPC: 1832-007 Unable to receive"
RPC8 "RPC: 1832-008 Timed out"
RPC9 "RPC: 1832-009 Incompatible versions of RPC"
RPC10 "RPC: 1832-010 Authentication error"
RPC11 "RPC: 1832-011 Program unavailable"
RPC12 "RPC: 1832-012 Program/version mismatch"
RPC13 "RPC: 1832-013 Procedure unavailable"
RPC14 "RPC: 1832-014 Server can't decode arguments"
RPC15 "RPC: 1832-015 Remote system error"
RPC16 "RPC: 1832-016 Unknown host"
RPC17 "RPC: 1832-017 Unknown protocol"
RPC18 "RPC: 1832-018 Port mapper failure"
RPC19 "RPC: 1832-019 Program not registered"
RPC20 "RPC: 1832-020 Failed (unspecified error)"
RPC21 "RPC: Authentication OK"
RPC22 "RPC: 1832-022 Invalid client credential"
RPC23 "RPC: 1832-023 Server rejected credential"
RPC24 "RPC: 1832-024 Invalid client verifier"
RPC25 "RPC: 1832-025 Server rejected verifier"
RPC26 "RPC: 1832-026 Client credential too weak"
RPC27 "RPC: 1832-027 Invalid server verifier"
RPC28 "RPC: 1832-028 Failed (unspecified error)"
RPC29 "clnttcp_create: 1832-029 out of memory\n"
RPC30 "clntudp_create: 1832-030 out of memory\n"
RPC31 "ku_recvfrom: 1832-031 no body!\n"
RPC32 "ku_recvfrom: 1832-033 len = %1$d\n"
RPC33 "pcbsetaddr failed %1$d\n"
RPC34 "RPC: 1832-034 cannot reassign procedure number %1$d\n"
RPC35 "RPC: 1832-035 could not create and rpc server\n"
RPC36 "RPC: 1832-036 could not register program %1$d version %2$d\n"
RPC37 "registerrpc: 1832-037 out of memory\n"
RPC38 "RPC: 1832-038 trouble replying to program %1$d\n"
RPC39 "RPC: 1832-039 never registered program %1$d\n"
RPC40 "svctcp_create: 1832-040 out of memory\n"
RPC41 "scv_tcp: makefd_xprt: 1832-041 out of memory\n"
RPC42 "svctcp.c - udp socket creation problem"
RPC43 "svctcp.c - cannot getsockname or listen"
RPC44 "svcudp_create: 1832-044 out of memory\n"
RPC45 "enablecache: cache already enabled"
RPC46 "enablecache: could not allocate cache"
RPC47 "enablecache: could not allocate cache data"
RPC48 "enablecache: could not allocate cache fifo"
RPC49 "cache_set: victim not found"
RPC50 "cache_set: victim alloc failed"
RPC51 "cache_set: could not allocate new rpc_buffer"
RPC52 "xdrrec_create: 1832-052 out of memory\n"
RPC53 "clnt_raw.c - Fatal header serialization error"
RPC54 "get_myaddress: socket"
RPC55 "get_myaddress: ioctl (get interface configuration)"
RPC56 "get_myaddress: ioctl"
RPC57 "Cannot register service"
RPC58 "pmap_getmaps rpc problem"
RPC59 "broadcast: ioctl (get interface configuration)"
RPC60 "broadcast: ioctl (get interface flags)"
RPC61 "Cannot create socket for broadcast rpc"
RPC62 "Cannot set socket option SO_BROADCAST"
RPC63 "Cannot send broadcast packet"
RPC64 "Broadcast select problem"
RPC65 "Cannot receive reply to broadcast"
RPC66 "svc_run: - select failed"
RPC67 "svcudp_create: socket creation problem"
RPC68 "svcudp_create - cannot getsockname"
RPC69 "(unknown authentication error - %1$d)"
RPC70 "; low version = %1$lu,  high version = %2$lu"
RPC71 "RPC: (unknown error code)"
RPC72 "Error %1$d"
RPC73 "authunix_create: 1832-053  xdr_authunix_parms failed"
RPC74 "authunix_create_default: 1832-054 gethostname failed:  %1$m"
RPC75 "authunix_create_default: 1832-055  getgroups failed:  %1$m"
RPC76 "get_myaddress: socket: %1$m"
RPC77 "get_myaddress: ioctl (get interface configuration): %1$m"
RPC78 "get_myaddress: ioctl: %1$m"
RPC79 "pmap_getmaps rpc problem"
RPC80 "broadcast: ioctl (get interface configuration) %1$m"
RPC81 "broadcast: ioctl (get interface flags) %1$m"
RPC82 "Cannot create socket for broadcast rpc %1$m"
RPC83 "Cannot set socket option SO_BROADCAST %1$m"
RPC84 "Cannot send broadcast packet %1$m"
RPC85 "Broadcast select problem %1$m"
RPC86 "Cannot receive reply to broadcast %1$m"
RPC87 "bad auth_len gid %1$d str %2$d auth %3$d"
RPC88 "svc_run: select failed: %1$m"
RPC89 "svctcp_create - tcp socket creation problem: %1$m"
RPC90 "svctcp_create - can not getsockname or listen: %1$m"
RPC91 "svctcp_create: out of memory"
RPC92 "svcudp_create: socket creation problem: %1$m"
RPC93 "svcudp_create - cannot getsockname: %1$m"
RPC94 "xdr_bytes: out of memory"
RPC95 "xdr_string: out of memory"
RPC96 "xdr_array: out of memory"
RPC97 "xdr_reference: out of memory"

$set LIBCNET
NET1  "sethostent: getdomainname system call missing\n"
NET2  "gethostent: getdomainname system call missing\n"
NET3  "setnetent: getdomainname system call missing\n"
NET4  "getnetent: getdomainname system call missing\n"
NET5  "setrpcent: getdomainname system call missing\n"
NET6  "getrpcent: getdomainname system call missing\n"
NET7  "setservent: getdomainname system call missing\n"
NET8  "getservent: getdomainname system call missing\n"
NET9  "getnetgrent: getdomainname system call missing\n"
NET10 "Cycle detected in /etc/netgroup: %s.\n"
NET11 "syntax error in /etc/netgroup\n"
NET12 "Unknown Error"
NET13 "Error 0"
NET14 "Unknown host"
NET15 "Host name lookup failure"
NET16 "Unknown server error"
NET17 "No address associated with name"
NET18 "innetgr: getdomainname system call missing\r\n"
NET19 "innetgr: recursive overflow\r\n"
NET20 "innetgr: netgroup %s called recursively\r\n"
NET21 "innetgr: syntax error in /etc/netgroup\r\n"
NET22 "%s: unknown host\n"
NET23 "socket: All ports in use\n"
NET24 "connect to address %s: "
NET25 "Trying %s...\n"
NET26 "write: setting up stderr"
NET27 "socket: protocol failure in circuit setup.\n"
NET28 "rcmd: getdomainname system call missing\n"
NET29 "Error - .netrc file not correct mode.\n"
NET30 "Remove password or correct mode.\n"
NET31 "Unknown .netrc option %s\n"
NET32 "Can't make key\n"
NET33 "Name (%s:%s): "
NET34 "Password (%s:%s): "
NET35 "setprotoent: getdomainname system call missing\n"
NET36 "getprotoent: getdomainname system call missing\n"

$set LIBCTTY
TTY1 "%s: improper file format: file missing %s field\n"
TTY2 "%s: improper file format: unexpected value in %s field\n"
TTY3 "%s: improper file format at line %d\n"
