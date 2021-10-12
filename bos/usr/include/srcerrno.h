/* @(#)45	1.12  src/bos/usr/include/srcerrno.h, cmdsrc, bos411, 9428A410j 4/24/91 08:46:19 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989,1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SRCERRNO
#define _H_SRCERRNO


#define SRC_OK    0	/* SRC commmand successful */

#define SRC_SUBMSG -1	/* subsys unique message text returned
			** no corresponding message for this
			** number in the SRC message catalog
			**/

#define SRC_DMNA -9001	/* SRC daemon not active */
#define SRC_ICMD -9002	/* Invalid command */
#define SRC_NOTROOT -9003 /* sys adm problem. SRC not running as root */
#define SRC_SVND -9004	/* Unknown subsystem */
#define SRC_CONT -9005	/* Status/Request cannot be done with signals */
#define SRC_STPG -9006	/* Subsystem stopping. Command rejected. */
#define SRC_PARM -9007	/* Parameters incorrect */
#define SRC_TYPE -9008	/* Unknown type */
#define SRC_NOCONTINUE -9009	/* NO active Continuation on communitions */
#define SRC_MSGQ -9010  /* Invalid message queue id/name              */
#define SRC_REPLYSZ -9011	/* Subsystem reply is of invalid size */
#define SRC_SUBSYSID -9012 	/* Subsystem id is invalid */
#define SRC_AUDITID -9013 	/* Subsystem audit id is invalid */
#define SRC_BADSOCK -9014 	/* Failed to contact subsystem by socket */
#define SRC_FEXE -9015	/* Error doing fork or exec of subsystem */
#define SRC_MMRY -9016	/* Insufficient memory */
#define SRC_SVKO -9017	/* Subsystem ended */
#define SRC_INVALID_USER_ROOT -9018 /* not root */
#define SRC_INVALID_USER -9019	/* not root or system */
#define SRC_TRYX -9020	/* Retry limit exceeded restarting subsystem */
#define SRC_RES  -9021	/* Resource not found */
#define SRC_CURR -9022	/* Resource currently under command processing */
#define SRC_PROC -9023	/* Command already processed */
#define SRC_IRES -9024	/* Insufficient resources */
#define SRC_DUP  -9025	/* Resource exists under another name */
#define SRC_INET_INVALID_HOST -9026	/* invalid host name received */
#define SRC_INET_AUTHORIZED_HOST -9027	/* host not in /etc/host.equiv */
#define SRC_NO_RESV_PORT -9028	/* not a reserved port not root */
#define SRC_MULT -9029	/* MULTIPLE instances not supported */
#define SRC_DIED -9030	/* SRC master says GOODBYE! */
#define SRC_WICH -9031	/* Multi instances -- Which one? */
#define SRC_NOT_SRC_SOCKADDR -9032	/* not valid af_unix socket */
#define SRC_SVRQ -9033	/* Unable to create subsystem's queue */
#define SRC_PIPE -9034	/* Unable to create or communication pipe */
#define SRC_RSTRT -9035	/* Subsystem ended -- will be restarted */
#define SRC_NSVR  -9036 /* request can't be passed to subsys. its inactive */
#define SRC_ACTV  -9037	/* SRCMSTR already active */
#define SRC_STRT  -9038	/* usage start */
#define SRC_STAT  -9039	/* usage status */
#define SRC_STOP  -9040	/* usage stop */
#define SRC_STPF  -9043	/* STOP command Failed */
#define SRC_STPOK -9044	/* STOP command OK */
#define SRC_INPT  -9046	/* Can't open subsystem's standard input */
#define SRC_OUT   -9047	/* Can't open subsystem's standard output */
#define SRC_SERR  -9048	/* Can't open subsystem's standard error */
#define SRC_TRACEON  -9050 	/* usage traceson */
#define SRC_TRACEOFF -9051 	/* usage tracesoff */
#define SRC_REFRESH  -9052 	/* usage refresh */
#define SRC_SOCK  -9053		/* socket communication problem */
#define SRC_UHOST  -9054	/* unknown host */
#define SRC_UDP -9055		/* no entry for src in services file */
#define SRC_NORPLY -9056	/* src/subsystem didnot reply */
#define SRC_SUBSOCK -9057	/* could not establish subsystems socket */
#define SRC_VERSION -9058	/* SRC msg version incompatable */
#define SRC_STRTOK -9059	/* START command successful */
#define SRC_MKSERVER -9061	/* mkserver usage */
#define SRC_SSME -9062		/* Subsystem must exist */
#define SRC_SVREXIST -9063	/* subserver type already exists */
#define SRC_SUBCODE -9064	/* code point already used by subsystem */
#define SRC_SVRADD -9065	/* subserver added successfuly */
#define SRC_MKNOTIFY -9066	/* mknotify usage */
#define SRC_NOTEXIST -9067	/* notify exists */
#define SRC_NOTADD -9068	/* notify added */
#define SRC_MKSSYS -9069	/* usage mksssy */
#define SRC_SUBSYN -9070	/* subsystem or synonyn already on file */
#define SRC_SUBADD -9071	/* subsystem added */
#define SRC_CHSERVER -9072	/* usage chsserver */
#define SRC_SERVERCH -9073	/* subserver updated */
#define SRC_CHSSYS -9074	/* usage chssys */
#define SRC_SUBEXIST -9075	/* subststem already exists */
#define SRC_SYNEXIST -9076	/* synonym already exists */
#define SRC_SUBCH -9077		/* subsystem updated */
#define SRC_RMNOTIFY -9078	/* usage rmnotify */
#define SRC_NOTRM -9079		/* notify method removed */
#define SRC_RMSERVER -9080	/* usage rmserver */
#define SRC_SERVERRM -9081	/* subserver removed */
#define SRC_RMSSYS -9082	/* usage rmssys */
#define SRC_SSYSRM -9083	/* subsys deleted */
#define SRC_NOREC -9084		/* no record with key */
#define SRC_SUBSYS -9085	/* subsys not on file */
#define SRC_GROUP -9086		/* group not on file */
#define SRC_SUBICMD -9087	/* subsystem recieved invalid command */
#define SRC_BADFSIG -9088	/* invalid force signal specified */
#define SRC_BADNSIG -9089	/* invalid norm signal specified */
#define SRC_ODMERR -9090	/* catostropic odm error */
#define SRC_TRACONOK -9091	/* trace on ok */
#define SRC_TRACONFAIL -9092	/* trace on fail */
#define SRC_TRACOFFOK -9093	/* trace off ok */
#define SRC_TRACOFFFAIL -9094	/* trace off fail */
#define SRC_REFOK -9095		/* trace refresh ok */
#define SRC_REFFAIL -9096	/* trace refresh fail */
#define SRC_OBJOK -9097		/* SRC objects created */
#define SRC_NOSUBSYS -9098	/* no subsystem or group exist */
#define SRC_NOTENAME2BIG -9099	/* notify name to long */
#define SRC_METHOD2BIG -9100	/* notify method to long */
#define SRC_SUBTYP2BIG -9101	/* sub_type to long */
#define SRC_SUBSYS2BIG -9102	/* subsystem to long */
#define SRC_SYN2BIG -9103	/* synonym name to long */	
#define SRC_CMDARG2BIG -9104	/* command args to long */
#define SRC_PATH2BIG -9105	/* subsys path to long */
#define SRC_STDIN2BIG -9108	/* stdin path to long */
#define SRC_STDOUT2BIG -9109	/* stdout path to long */
#define SRC_STDERR2BIG -9110	/* stderr path to long */
#define SRC_GRPNAM2BIG -9111	/* group name to long */
#define SRC_ARG2BIG -9112	/* start argument to long */
#define SRC_ENV2BIG -9113	/* start env to long */
#define SRC_SUBOBJ2BIG -9114	/* subsever object to long */
#define SRC_HOST2BIG -9115	/* host name to long */
#define SRC_PRIORITY -9117 	/* Invalid subsystem priority. must be 0-39 */
#define SRC_MTYPE -9118 	/* Invalid subsystem mtype. must be > 0 */
#define SRC_NONAME -9119 	/* Invalid subsystem name */
#define SRC_NOPATH -9120 	/* Invalid subsystem path */
#define SRC_NOCONTACT -9121 	/* Invalid contact type */
#define SRC_NOINET -9122 	/* AF_INET sockets not available */
#define SRC_REQLEN2BIG -9123 	/* Subsystem request to many bytes */

#define SRC_STRTSVROK -9124	/* start successful */
#define SRC_STRTSVRBAD -9125	/* start unsuccessful */
#define SRC_STRTSVRDUP -9126	/* start unsuccessful, already active */
#define SRC_STOPSVROK -9127	/* stop subserver successful */
#define SRC_STOPSVRBAD -9128	/* stop unsuccessful */
#define SRC_NOSUPPORT -9129	/* stop unsuccessful */

#define FIRST_SRC_ERROR SRC_DMNA 
#define LAST_SRC_ERROR SRC_NOSUPPORT 


#endif
