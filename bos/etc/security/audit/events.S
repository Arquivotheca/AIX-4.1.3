# @(#)69	1.35  src/bos/etc/security/audit/events.S, cfgsaud, bos411, 9428A410j 6/3/94 16:04:06
#
#   COMPONENT_NAME: CFGSAUD
#
#   FUNCTIONS:  none
#
#   ORIGINS: 27
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

auditpr:

* kernel proc events 

*	fork()
	PROC_Create = printf "forked child process %d"

*	exit()
	PROC_Delete = printf "exited child process %d"

*	exec()
	PROC_Execute = printf "euid: %d egid: %d epriv: %d name %s"

*	setuidx()
	PROC_RealUID = printf "real uid: %d"
	PROC_AuditID = printf "login uid: %d"

*	setgidx()
	PROC_RealGID = printf "real gid: %d"

* 	usrinfo()
	PROC_Environ = printf "buf: %s"

* 	sigaction()
	PROC_SetSignal = printf ""

* 	setrlimit()
	PROC_Limits = printf ""

* 	nice()
	PROC_SetPri = printf "new priority: %d"

* 	setpri() 
	PROC_Setpri = printf "new priority: %d"

* 	setpriv() 
	PROC_Privilege = printf "cmd: %d privset: %x"

*	settimer()
	PROC_Settimer = printf ""

* file system events 

* 	open() and creat()
	FILE_Open = printf "flags: %d mode: %o fd: %d filename %s"

* 	read() 
	FILE_Read = printf "file descriptor = %d"

* 	write() 
	FILE_Write = printf "file descriptor = %d"

* 	close() 
	FILE_Close = printf "file descriptor = %d"

* 	link() 
	FILE_Link = printf "linkname %s filename %s"

* 	unlink() 
	FILE_Unlink = printf "filename %s"

* 	rename() 
	FILE_Rename = printf "frompath: %s topath: %s"

* 	chown() 
	FILE_Owner = printf "owner: %d group: %d filename %s"

* 	chmod() 
	FILE_Mode = printf "mode: %o filename %s"

* 	mount() 
	FS_Mount = printf "mount: object %s stub %s"

* 	umount() 
	FS_Umount = printf "umount: object %s stub %s"

* 	chacl() 
	FILE_Acl = printf "acl: %d"

* 	chpriv() 
	FILE_Privilege = printf "pcl: %d"

*	chdir()
	FS_Chdir = printf "change current directory to: %s"

*	chroot()
	FS_Chroot = printf "change root directory to: %s"

*	rmdir()
	FS_Rmdir = printf "remove of directory: %s"

*	mkdir()
	FS_Mkdir = printf "make of directory: %s"


* SVIPC system events 

* 	msgget() 
	MSG_Create = printf "key: %d msqid: %d"

* 	msgrcv() 
	MSG_Read = printf "msqid: %d muid: %d mpid: %d"

* 	msgsnd() 
	MSG_Write = printf "msqid: %d"

* 	msgctl() 
	MSG_Delete = printf "msqid: %d"
	MSG_Owner = printf "msqid: %d owner: %d group: %d"
	MSG_Mode = printf "msqid: %d mode: %o"

* 	semget() 
	SEM_Create = printf "key: %d semid: %d"

* 	semop() 
	SEM_Op = printf "semid: %d"

* 	semctl() 
	SEM_Delete = printf "semid: %d"
	SEM_Owner = printf "semid: %d owner: %d group: %d"
	SEM_Mode = printf "semid: %d mode: %o"

* 	shmget() 
	SHM_Create = printf "key: %d shmid: %d"

* 	shmat() 
	SHM_Open = printf "shmid: %d"

* 	shmctl() 
	SHM_Close = printf "shmid: %d"
	SHM_Owner = printf "shmid: %d owner: %d group: %d"
	SHM_Mode = printf "shmid: %d mode: %o"

* TCPIP user level

        TCPIP_config = printf "%s %s %s %s %s"
        TCPIP_host_id = printf "%s %s %s %s"
        TCPIP_route = printf "%s %s %s %s %s"
        TCPIP_connect = printf "%s %s %s %s %s"
        TCPIP_data_out = printf "%s %s %s %s %s"
        TCPIP_data_in = printf "%s %s %s %s %s"
        TCPIP_access = printf "%s %s %s %s %s"
        TCPIP_set_time = printf "%s %s %s %s"

* TCPIP kernel level

        TCPIP_kconfig = /usr/sbin/ip_auditpr -e TCPIP_kconfig
        TCPIP_kroute = /usr/sbin/ip_auditpr -e TCPIP_kroute
        TCPIP_kconnect = /usr/sbin/ip_auditpr -e TCPIP_kconnect
        TCPIP_kdata_out = /usr/sbin/ip_auditpr -e TCPIP_kdata_out
        TCPIP_kdata_in = /usr/sbin/ip_auditpr -e TCPIP_kdata_in
        TCPIP_kcreate = /usr/sbin/ip_auditpr -e TCPIP_kcreate

* commands

* 	tsm
	USER_Login = printf "%s"
	PORT_Locked = printf "Port %s locked due to invalid login attempts"

* 	sysck
	SYSCK_Check = printf "%s"
	SYSCK_Update = printf "%s"
	SYSCK_Install = printf "%s"

* 	usrck
	USER_Check = printf "%s %s %s"

* 	logout
	USER_Logout = printf "%s"

*	chsec
	PORT_Change = printf "Changed attributes of port %s; new values: %s"

* 	chuser
	USER_Change = printf "%s %s"

* 	rmuser
	USER_Remove = printf "%s"

* 	mkuser
	USER_Create = printf "%s"

* 	setgroups
	USER_SetGroups = printf "%s %s"

* 	setsenv
	USER_SetEnv = printf "%s %s"

* 	su
	USER_SU = printf "%s"

* 	grpck
        GROUP_User = printf "grpck: removed user %s from %s in /etc/group"

* 	grpck
        GROUP_Adms = printf "grpck: removed admin user %s from %s in /etc/security/group"

* 	chgroup
	GROUP_Change = printf "%s %s"

* 	mkgroup
	GROUP_Create = printf "%s"

* 	rmgroup
	GROUP_Remove = printf "%s"

* 	passwd
	PASSWORD_Change = printf "%s"

* 	pwdadm
	PASSWORD_Flags = printf "%s"

* 	pwdck
	PASSWORD_Check = printf "User = %s Error/Fix = %s Status = %s"
	PASSWORD_Ckerr = printf "User/File = %s Error = %s Status = %s"

* 	startsrc
	SRC_Start = printf "%s"

* 	stopsrc
	SRC_Stop = printf "%s"

* 	addssys
	SRC_Addssys = printf "%s"

* 	chssys
	SRC_Chssys = printf "%s"

* 	addserver
	SRC_Addserver = printf "%s"

* 	chserver
	SRC_Chserver = printf "%s"

* 	rmssys
	SRC_Delssys = printf "%s"

* 	rmserver
	SRC_Delserver = printf "%s"

* 	enq
	ENQUE_admin = printf "queue = %s device = %s request = %s to: %s op = %s"

* 	qdaemon
	ENQUE_exec = printf "queue = %s request = %s host = %s file = %s to: %s op = %s"

* 	sendmail
	SENDMAIL_Config = printf "%s"
	SENDMAIL_ToFile = printf "Mail from user %s to file %s"

* 	at
	AT_JobAdd = printf "file name = %s User = %s time = %s"
	AT_JobRemove = printf "file name = %s User = %s"

* 	cron
	CRON_JobRemove = printf "file name = %s User = %s time = %s"
	CRON_JobAdd = printf "file name = %s User = %s time = %s"
	CRON_Start = printf "event = %s cmd = %s time = %s"
	CRON_Finish = printf "user = %s pid = %s time = %s"

*	nvload
	NVRAM_Config = printf " %s"

*	cfgmgr
	DEV_Configure = printf " device %s"

*	chdev and mkdev
	DEV_Change = printf " params = %s"

*	mkdev
	DEV_Create = printf "mode: %o dev: %d filename %s"
	DEV_Start = printf " %s "

*	installp
	INSTALLP_Inst = printf "Option Name: %s Level: %s  Installation %s"
	INSTALLP_Exec = printf "Option Name: %s Level: %s  Executed Program %s"

*	updatep
	UPDATEP_Name = printf " %s "

*	rmdev
	DEV_Stop = printf " device %s"
	DEV_UnConfigure = printf " device %s"
	DEV_Remove = printf " device %s"

*	lchangelv, lextendlv, lreducelv
	LVM_ChangeLV= printf " %s "

*	lchangepv, ldeletepv, linstallpv
	LVM_ChangeVG = printf " %s "

*	lcreatelv
	LVM_CreateLV = printf " %s "

*	lcreatevg
	LVM_CreateVG = printf " %s "

*	ldeletepv
	LVM_DeleteVG = printf " %s "

*	rmlv
	LVM_DeleteLV = printf " %s "

*	lvaryoffvg
	LVM_VaryoffVG = printf " %s "

*	lvaryonvg
	LVM_VaryonVG = printf " %s "

* 	backup, restore
	BACKUP_Export = printf " %s "
	BACKUP_Priv = printf " %s "
	RESTORE_Import = printf " %s "

* 	shell
	USER_Shell = printf " %s "

* objects (files)

*	/etc/security/environ
	S_ENVIRON_WRITE = printf "%s"

*	/etc/group
	S_GROUP_WRITE = printf "%s"

*	/etc/security/limits
	S_LIMITS_WRITE = printf "%s"

*	/etc/security/login.cfg
	S_LOGIN_WRITE = printf "%s"

*	/etc/security/passwd
	S_PASSWD_READ = printf "%s"

*	/etc/security/passwd
	S_PASSWD_WRITE = printf "%s"

*	/etc/security/user
	S_USER_WRITE = printf "%s"

*	/etc/security/audit/config
	AUD_CONFIG_WR = printf "%s"
