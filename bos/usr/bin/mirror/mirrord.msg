$ @(#)13        1.3  src/bos/usr/bin/mirror/mirrord.msg, cmdmirror, bos411, 9428A410j 3/16/94 09:40:19
$ COMPONENT_NAME: CMDMIRROR : remote mirrorring
$ 
$ FUNCTIONS: .msg message catalog file for mirror daemon
$ 
$ ORIGINS: 83 
$ 
$
$  LEVEL 1, 5 Years Bull Confidential Information
$


$set 1
$quote "

CAT_ABS_ERROR	"mirrord: Modem dialog error\n"
CAT_ACTIVE	"mirrord: mirror daemon already active \n"
CAT_CANNOT_ACTIVATE	"mirrord: Cannot activate mirror module : %s\n"
CAT_CANNOT_CONNECT	"mirrord: Cannot connect to the key : %s\n"
CAT_CANNOT_GET_KEY	"mirrord: Cannot get the key position : %s\n"
CAT_CANNOT_GET_LIST	"mirrord: Cannot get list of modules : %s\n"
CAT_CANNOT_GET_MODEM	"mirrord: Cannot get information from the modem : %s\n"
CAT_CANNOT_INTERCHANGE	"mirrord: Cannot interchange write queues : %s\n"
CAT_CANNOT_OPEN_CONSOLE	"mirrord: Cannot open %s : %s\n"
CAT_CANNOT_OPEN_LOCK_FILE	"mirrord: Cannot open lock file %s : %s\n"
CAT_CANNOT_OPEN_S2	"mirrord: Cannot open %s : %s\n"
CAT_CANNOT_POP	"mirrord: Cannot pop module : %s\n"
CAT_CANNOT_PUSH_MIRROR	"mirrord: Cannot push mirror module : %s\n"
CAT_CANNOT_PUSH_POPED	"mirrord: Cannot push poped modules : %s\n"
CAT_CANNOT_STRLOAD	"mirrord: Cannot load mirror module \n"
CAT_CANNOT_STRUNLOAD	"mirrord: Cannot unload mirror module\n"
CAT_CANNOT_UNACTIVATE	"mirrord: Cannot unactivate mirror module : %s\n"
CAT_CONTRACT_FLAG	"mirrord: Remote maintenance contract is no more valid, mirrord exited \n"
CAT_DEFAULTTO	"mirrord: DefaultTO variable must be initialized\n"
CAT_FREVOKE	"mirrord: Cannot revoke process on %s : %s\n"
CAT_GETATTR	"mirrord: Cannot get attributs : %s\n"
CAT_GET_FLAGS	"mirrord: Cannot get the value of contract flags : %s\n"
CAT_ICDELAY	"mirrord: ICDelay variable must be initialized\n"
CAT_LOG	"mirrord: Warning, no log during the mirroring \n"
CAT_MANY_PARAMETERS	"mirrord: Too many parameters, you must only specify the name of the modem parameters file\n"
CAT_MIRROR_OFF	"mirrord: Mirroring is stopped.\n"
CAT_MIRROR_ON	"mirrord: Remote user connected, mirroring active.\n"
CAT_MODEM_ERROR	"mirrord: Modem dialog error : %s\n"
CAT_MODEM_TIMEOUT	"mirrord: Modem dialog timeout\n"
CAT_NOT_STREAMS	"mirrord: Console or modem lines are not managed by streams, mirroring failed \n"
CAT_NO_MEMORY	"mirrord: No place for memory\n"
CAT_NO_PARAMETER	"mirrord: You must specify the name of the modem parameters file\n"
CAT_ODM_GET	"mirrord: Cannot get information from ODM : %s\n"
CAT_OPEN_MPF	"mirrord: Cannot open modem parameters file %s : %s\n"
CAT_OPEN_TTYS2_PARAMS	"mirrord: Cannot open the file %s : %s\n"
CAT_PASSWD	"mirrord: Cannot open file /etc/passwd : %s\n"
CAT_POLL	"mirrord: Poll error : %s\n"
CAT_SCRIPT	"mirrord: The script <%s> does not exist\n"
CAT_SETATTR	"mirrord: Cannot set attributs : %s\n"
CAT_SETPGRP	"mirrord: Setpgrp error : %s\n"
CAT_SIGPOLL	"mirrord: Cannot set SIGPOLL signal : %s\n"
CAT_USER_FLAG	"mirrord: Flag user not set, mirroring remote maintenance failed\n"
CAT_WAIT_CALL	"mirrord: Wait connection ...\n"
CAT_WAIT_STOPPED	"mirrord: Connection waiting stopped\n"
CAT_WHO	"mirrord: Cannot execute the who command : %s\n"
CAT_YYERROR	"mirrord: %s: %s: Line %d\n"
