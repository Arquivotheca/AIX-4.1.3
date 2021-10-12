$ @(#)68	1.14.3.1  src/bos/usr/bin/src/cmds/src.msg, cmdsrc, bos411, 9428A410j 9/8/93 13:38:27
$
$ COMPONENT_NAME: (cmdsrc) System Resource Controller
$
$ FUNCTIONS:
$       none
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1988,1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregate modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$              src.msg

$quote "         define quote character.

$delset 1
$set 1

$ This first set in the catalog are the character representation
$ of the internal SRC status
1               "active"
2               "inactive"
3               "stopping"
4               ""
5               "warned to stop"
6               "starting"
7               "opened"
8               "closed"
9               "open pending"
10              "close pending"
11              "inoperative"
12              "open but inactive"
13              ""
14              "test pending"
15              "testing"

$delset 2
$set 2

$ status header to be placed on status from subsystem/SRC
$ these headers must be exactly the length they are now.
$	 placed in objname  placed in objtext
$	 ------------------ -------------------------------
1	"Subsystem         :Group            PID     Status"
$	 ------------------ -------------------------------


$ SRC error and message set
$delset 3
$set 3

001 "0513-001 \
The System Resource Controller daemon is not active.\n"

002 "0513-002 \
The System Resource Controller has received a request that\n\
it does not understand and could not service.\n"

003 "0513-003 \
A possible systems administration problem has been found.\n\
The System Resource Controller is not running as superuser.\n"

004 "0513-004 \
The Subsystem or Group, %1$s, is currently inoperative.\n"

005 "0513-005 \
The Subsystem, %1$s, only supports signal communication.\n"

006 "0513-006 \
The Subsystem, %1$s, is currently stopping its execution.\n"

007 "0513-007 \
The options specified are not valid for this command.\n\
Please reference your manual for valid options.\n"

008 "0513-008 \
The subserver type, %1$s, is not on file.\n"

$ This message should never be displayied but?
$ getting this error message means that the calling process did not
$ set continue to NEWREQUEST or they called scstat or srcsrqt after
$ getting an error or and END packet.
009 "0513-009 \
You must set the parameter typerequest to NEWREQUEST to send a new\n\
request and receive your first reply.  You will only receive continued\n\
replys while typerequest is not set to END and an error is not returned.\n\
Refer to the typerequest parameter to srcsrqt or 'srcstat' in your\n\
programmers manual.\n"

010 "0513-010 \
The System Resource Controller is experiencing problems with its\n\
message queue communications.\n"

$ Note this error should never be printed since subsystems should
$ not try and send replys of invalid lengths.
011 "0513-011 \
The subsystem reply length is invalid. The valid lengths for the reply \
size must be between the size of a srchdr structure and SRCPKTMAX, or \
the size of a short."

012 "0513-012 \
The %1$s Subsystem could not be started.\n\
The Subsystem's user id could not be established.\n\
Please check the Subsystem's user id and try again.\n"

013 "0513-013 \
The %1$s Subsystem could not be started.\n\
The Subsystem's audit id could not be established.\n\
Please check the Subsystem's audit id and try again.\n"

014 "0513-014 \
The request could not be passed to the %1$s subsystem.\n\
The System Resource Controller is experiencing problems with\n\
the subsystem's communication socket.\n"

015 "0513-015 \
The %1$s Subsystem could not be started.\n\
Please check subsystem resources and try again later.\n"

016 "0513-016 \
The System Resource Controller is experiencing problems in acquiring\n\
the memory that it needs. Please try the command again later.\n"

017 "0513-017 \
The %1$s Subsystem ended abnormally. You must manually restart it.\n"

018 "0513-018 \
System Resource Controller requires that you be running as root\n\
when a remote host is specified for a request.\n"

019 "0513-019 \
System Resource Controller requires that you be running as \n\
root or be part of the group system for local requests.\n"

020 "0513-020 \
The %1$s Subsystem did not end normally.\n\
The subsystem respawn limit has been exceeded.\n\
Check the Subsystem and restart it manually.\n"

021 "0513-021 \
The subserver is not on file or subserver is not currently active.\n\
Check your subserver entries and try again.\n"

022 "0513-022 \
Subserver currently under command processing.\n\
The Subserver you are referencing is currently busy.\n\
Wait until the Subserver is finished and try again.\n"

023 "0513-023 \
The Command has already been processed.\n"

024 "0513-024 \
The command cannot be performed because a System resource limit\n\
has been exceeded.\n\
Contact your System Administrator to correct the problem.\n"

025 "0513-025 \
The Subserver exists under another name. Duplicates are not allowed.\n"

026 "0513-026 \
The remote host cannot determine your host's name. The remote host\n\
must be able to determine your host's name for remote System Resource\n\
Controller requests.\n"

027 "0513-027 \
Your host is not specified in the /etc/hosts.equiv file on the remote\n\
host. Your host may only execute remote System Resource Controller\n\
requests if your host name appears in the remote host's\n\
/etc/hosts.equiv file.\n"

028 "0513-028 \
A privileged internet port could not be allocated. \n\
Remote system Resource Controller requests require a privileged \n\
internet port. Try again later when a port become available.\n"

029 "0513-029 \
The %1$s Subsystem is already active.\n\
Multiple instances are not supported.\n"

030 "0513-030 \
The System Resource Controller daemon has stopped.\n"

031 "0513-031 \
Multiple instances of the Subsystem exist.\n\
Specify the Subsystem Process ID and try your command again.\n"

032 "0513-032 \
Invalid SRC return AF_UNIX address. You must use the System Resource\n\
Controller commands or library calls to use the System Resource Controller.\n"

033 "0513-033 \
The %1$s Subsystem could not be started. \n\
The Subsystem's communication queue could not be established.\n"

034 "0513-034 The System Resource Controller was unable to\n\
establish a pipe with which to communicate with the %1$s Subsystem.\n"

035 "0513-035 \
The %1$s Subsystem ended abnormally. SRC will try and restart it.\n"

036 "0513-036 \
The request could not be passed to the %1$s subsystem.\n\
Start the subsystem and try your command again.\n"

037 "0513-037 \
The System Resource Controller is already active.\n"

038 "0513-038 \
Usage is one of:\n\
startsrc [-h host] [-e environment] [-a argument] -s subsystem_name\n\
startsrc [-h host] [-e environment] [-a argument] -g group_name\n\
startsrc [-h host] -t subserver_type [-o subserver_object] [-p subsystem_pid]\n"

039 "0513-039 \
Usage is one of:\n\
lssrc [-h host] -a \n\
lssrc [-h host] -g group_name\n\
lssrc [-h host] [-l] -s subsystem_name\n\
lssrc [-h host] [-l] -p subsystem_pid\n\
lssrc [-h host] [-l] -t subserver_type [-o subserver_object] [-p subsystem_pid]\n\
lssrc [-h host] [-l] -t subserver_type [-P subserver_pid] [-p subsystem_pid]\n\
lssrc -S [-s subsystem_name | -d ]\n\
lssrc -T [-t subserver_type]\n\
lssrc -N [-n notify_name]\n"

040 "0513-040 \
Usage is one of:\n\
stopsrc [-h host] [-f | -c] -a \n\
stopsrc [-h host] [-f | -c] -g group_name\n\
stopsrc [-h host] [-f | -c] -s subsystem_name\n\
stopsrc [-h host] [-f | -c] -p subsystem_pid\n\
stopsrc [-h host] [-f] -t subserver_type [-o subserver_object] [-p subsystem_pid]\n\
stopsrc [-h host] [-f] -t subserver_type [-P subserver_pid] [-p subsystem_pid]\n"

$ Unused place holder
$ NO_MSG_041
$ NO_MSG_042

043 "0513-043 \
The stop of the %1$s Subsystem failed.\n"

044 "0513-044 \
The stop of the %1$s Subsystem was completed successfully.\n"

$ Unused place holder
$ NO_MSG_045

046 "0513-046 \
The %1$s Subsystem could not be started. \n\
The Subsystem's standard in could not be established.\n\
Please check the Subsystem's standard in path and try again.\n"

047 "0513-047 \
The %1$s Subsystem could not be started. \n\
The Subsystem's standard out could not be established.\n\
Please check the Subsystem's standard out path and try again.\n"

048 "0513-048 \
The %1$s Subsystem could not be started. \n\
The Subsystem's standard error could not be established.\n\
Please check the Subsystem's standard error path and try again.\n"

$ Unused place holder
$ NO_MSG_049

050 "0513-050 \
Usage is one of:\n\
traceson [-h host] [-l] -g group_name\n\
traceson [-h host] [-l] -s subsystem_name\n\
traceson [-h host] [-l] -p subsystem_pid\n\
traceson [-h host] [-l] -t subserver_type [-o subserver_object] [-p subsystem_pid]\n\
traceson [-h host] [-l] -t subserver_type [-P subserver_pid] [-p subsystem_pid]\n"

051 "0513-051 \
Usage is one of:\n\
tracesoff [-h host] -g group_name\n\
tracesoff [-h host] -s subsystem_name\n\
tracesoff [-h host] -p subsystem_pid\n\
tracesoff [-h host] -t subserver_type [-o subserver_object] [-p subsystem_pid]\n\
tracesoff [-h host] -t subserver_type [-P subserver_pid] [-p subsystem_pid]\n"

052 "0513-052 \
Usage is one of:\n\
refresh [-h host] -g group_name\n\
refresh [-h host] -s subsystem_name\n\
refresh [-h host] -p subsystem_pid\n"

053 "0513-053 \
The System Resource Controller is experiencing problems with its\n\
socket communications.\n\
If you specified a foreign host, try configuring Internet sockets and\n\
try your command again, otherwise contact System Administration.\n"

054 "0513-054 \
The host, %1$s, that you wish to send this command to is not a known host.\n"

055 "0513-055 \
SRC's internet service is not defined.\n\
You must define SRC's internet service to be able to use remote SRC.\n"

056 "0513-056 \
Timeout waiting for command response.\n"

057 "0513-057 \
The %1$s Subsystem could not be started. \n\
The Subsystem's communication socket could not be established.\n"

058 "0513-058 \
SRC has received a request with a version identifier that\n\
it could not service.\n"

059 "0513-059 \
The %1$s Subsystem has been started. Subsystem PID is %2$s.\n"

$ Unused place holder
$ NO_MSG_060

061 "0513-061 \
Usage: mkserver -s subsystem_name -t subserver_type -c subserver_code.\n"

062 "0513-062 \
The %1$s Subsystem must exist to complete this command.\n"

063 "0513-063 \
Subserver type already exists on file.\n"

064 "0513-064 \
The Subsystem name and Subserver code point are already on file.\n"

065 "0513-065 \
The %1$s Subserver has been added.\n"

066 "0513-066 \
Usage: mknotify -n notify_name -m notify_method\n"

067 "0513-067 \
Notify method already exists with subsystem or group name.\n"

068 "0513-068 \
The %1$s Notify method has been added.\n"

069 "0513-069 Usage: mkssys -s subsystem_name \n\
\t-p subsystem_path \n\
\t-u userid \n\
\t[-t synonym_name] \n\
\t[-a command_arguments] \n\
\t[-i stdin] \n\
\t[-o stdout] \n\
\t[-e stderr] \n\
\t[-R | -O] \n\
\t[-d | -D] \n\
\t[-q | -Q] \n\
\t[{-I message_queue_key -m subsystem_mtype}\n\n\
\t\t| {-S -n signorm -f sigforce} | -K] \n\
\t[-E priority] \n\
\t[-G group_name] \n\
\t[-w wait_time]\n"

070 "0513-070 \
Subsystem or synonym_name is already on file.\n"

071 "0513-071 \
The %1$s Subsystem has been added.\n"

072 "0513-072 \
Usage: chserver  -t subserver_type {[-t subserver_type] [-s subsystem_name] [-c subserver_code]}\n"

073 "0513-073 The %1$s Subserver has been changed.\n"

074 "0513-074 Usage: chssys -s old_subsystem_name \n\
\t{[-s new_subsystem_name] \n\
\t[-t synonym_name] \n\
\t[-a command_arguments] \n\
\t[-p subsystem_path] \n\
\t[-u userid] \n\
\t[-i stdin] \n\
\t[-o stdout] \n\
\t[-e stderr] \n\
\t[-R | -O] \n\
\t[-d | -D] \n\
\t[-q | -Q] \n\
\t[{-I message_queue_key -m subsystem_mtype} \n\
\t\t| {-S -n signorm -f sigforce} | -K] \n\
\t[-E priority] \n\
\t[-G group_name] \n\
\t[-w wait_time] }\n"

075 "0513-075 The new subsystem name is already on file.\n"

076 "0513-076 The new synonym_name is already on file.\n"

077 "0513-077 Subsystem has been changed.\n"

078 "0513-078 \
Usage: rmnotify -n subsystem_name or group_name\n"

079 "0513-079 \
Notify method has been removed.\n"

080 "0513-080 Usage: rmserver -t subserver_type\n"

081 "0513-081 Subserver type has been removed.\n"

082 "0513-082 Usage: rmssys -s subsystem_name\n"

083 "0513-083 Subsystem has been Deleted.\n"

084 "0513-084 There were no records that matched your request.\n"

085 "0513-085 The %1$s Subsystem is not on file.\n"

086 "0513-086 The %1$s Group is not on file.\n"

087 "0513-087 \
The %1$s Subsystem has received a request that\n\
it does not understand and could not service.\n\
Contact System Administration.\n"

088 "0513-088 \
The stop force signal specified is an invalid signal.\n\
Change the signal number to a valid one and try again.\n"

089 "0513-089 \
The stop normal signal specified is an invalid signal.\n\
Change the signal number to a valid one and try again.\n"

090 "0513-090 \
There has been an unexpected database error with ODM.\n\
Please record this number '%1$s' and contact system administration.\n"

091 "0513-091 \
The request to turn on tracing was completed successfully.\n"

092 "0513-092 \
The request to turn on tracing failed.\n"

093 "0513-093 \
The request to turn off tracing was completed successfully.\n"

094 "0513-094 \
The request to turn off tracing failed.\n"

095 "0513-095 \
The request for subsystem refresh was completed successfully.\n"

096 "0513-096 \
The request for subsystem refresh failed.\n"

097 "0513-097 \
The System Resource Controller objects have been created.\n"

098 "0513-098 \
The notify_name must exist as a subsystem name or a group name.\n"

099 "0513-099 \
The notify_name is too long. Only 29 bytes are allowed.\n"

100 "0513-100 \
The notify_method is too long. Only 255 bytes are allowed.\n"

101 "0513-101 \
The subserver_type is too long. Only 29 bytes are allowed.\n"

102 "0513-102 \
The subsystem_name is too long. Only 29 bytes are allowed.\n"

103 "0513-103 \
The synonym_name is too long. Only 29 bytes are allowed.\n"

$ This is for mkssys and chssys
104 "0513-104 \
The argument too the subsystem is too long. Only 199 bytes are allowed.\n"

105 "0513-105 \
The subsystem_path is too long. Only 199 bytes are allowed.\n"

$ Unused, Place holder
$ NO_MSG_106
$ NO_MSG_107

108 "0513-108 \
The stdin path is too long. Only 199 bytes are allowed.\n"

109 "0513-109 \
The stdout path is too long. Only 199 bytes are allowed.\n"

110 "0513-110 \
The stderr path is too long. Only 199 bytes are allowed.\n"

111 "0513-111 \
The group_name is too long. Only 29 bytes are allowed.\n"

$ This is for the startsrc command
112 "0513-112 \
The argument too the subsystem is too long. Only 2400 bytes are allowed.\n"

113 "0513-113 \
The environment is too long. Only 2400 bytes are allowed.\n"

114 "0513-114 \
The subserver_object is too long. Only 29 bytes are allowed.\n"

115 "0513-115 \
The host name is too long. Only 255 bytes are allowed.\n"

$ Unused, Place holder
$ NO_MSG_116

117 "0513-117 \
Invalid Subsystem priority. Priority must be in the range 0-39.\n"

118 "0513-118 \
Invalid Subsystem message type. Message type must be greater than zero.\n"

119 "0513-119 \
You must specify the subsystem name.\n"

120 "0513-120 \
You must specify the path to the subsystem executable.\n"

121 "0513-121 \
You must specify the subsystem communication method.\n"

122 "0513-122 \
AF_UNIX Socket family has not been configured.\n\
The System Resource Controller is experiencing problems with its.\n\
local socket communications.  Contact Systems Administration.\n"

123 "0513-123 \
You may only pass 2000 bytes of information to the subsystem.\n"

124 "0513-124 The %1$s subserver has been started.\n"

125 "0513-125 The %1$s subserver could not be started.\n"

126 "0513-126 The %1$s subserver is already active.\n"

127 "0513-127 The %1$s subserver was stopped successfully.\n"

128 "0513-128 The %1$s subserver was not stopped successfully.\n"

129 "0513-129 The %1$s subsystem does not support the requested action.\n"
