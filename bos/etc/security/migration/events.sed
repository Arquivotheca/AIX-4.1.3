# @(#)81	1.4  src/bos/etc/security/migration/events.sed, cfgsauth, bos411, 9428A410j 6/8/94 16:45:14
#
#   COMPONENT_NAME: CFGSAUTH
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Add setgidx() comment line
/PROC_AuditID/a\
\
*	setgidx()

# Change PROC_Privilege line
s/PROC_Privilege.*$/PROC_Privilege = printf "cmd: %d privset: %x"/

# Add PROC_Settimer line
/PROC_Privilege/a\
\
*	settimer()\
	PROC_Settimer = printf ""

# Change open() line
s/* 	open.*$/* 	open() and creat()/

# Change FILE_Close line
s/FILE_Close.*$/FILE_Close = printf "file descriptor = %d"/

# Change FILE_Rename line
s/FILE_Rename.*$/FILE_Rename = printf "frompath: %s topath: %s"/

# Change FILE_Owner line
s/FILE_Owner.*$/FILE_Owner = printf "owner: %d group: %d filename %s"/

# Change MSG_Owner line
s/MSG_Owner.*$/MSG_Owner = printf "msqid: %d owner: %d group: %d"/

# Change SEM_Owner line
s/SEM_Owner.*$/SEM_Owner = printf "semid: %d owner: %d group: %d"/

# Change SHM_Owner line
s/SHM_Owner.*$/SHM_Owner = printf "shmid: %d owner: %d group: %d"/

# Add PORT_Locked line
/USER_Login/a\
	PORT_Locked = printf "Port %s locked due to invalid login attempts"

# Add PORT_Change line
/USER_Logout/a\
\
*	chsec\
	PORT_Change = printf "Changed attributes of port %s; new values: %s"

# Change USER_Check line
s/USER_Check.*$/USER_Check = printf "%s %s %s"/

# Add CRON_Start and CRON_Finish lines
/CRON_JobAdd/a\
	CRON_Start = printf "event = %s cmd = %s time = %s"\
	CRON_Finish = printf "user = %s pid = %s time = %s"

# Change chdev line
s/*	chdev.*$/*	chdev and mkdev/

# Change lchangepv line
s/*	lchangepv.*$/*	lchangepv, ldeletepv, linstallpv/

# Add FS_chdir, FS_chroot, FS_rmdir, and FS_mkdir lines
/FILE_Privilege/a\
\
*	chdir()\
	FS_Chdir = printf "change current directory to: %s"\
\
*	chroot()\
	FS_Chroot = printf "change root directory to: %s"\
\
*       rmdir()\
        FS_Rmdir = printf "remove of directory: %s"\
\
*       mkdir()\
        FS_Mkdir = printf "make of directory: %s"
