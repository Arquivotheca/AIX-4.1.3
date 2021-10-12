# @(#)67	1.19  src/bos/etc/security/audit/config.S, cfgsaud, bos411, 9435A411a 8/29/94 16:03:19
#  COMPONENT_NAME: CFGSAUD
#
#  FUNCTIONS:  none
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

start:
	binmode = on 
	streammode = off 

bin:
	trail = /audit/trail
	bin1 = /audit/bin1
	bin2 = /audit/bin2
	binsize = 10240
	cmds = /etc/security/audit/bincmds

stream:
	cmds = /etc/security/audit/streamcmds

classes:
	general = USER_SU,PASSWORD_Change,FILE_Unlink,FILE_Link,FILE_Rename,FS_Chdir,FS_Chroot,PORT_Locked,PORT_Change,FS_Mkdir,FS_Rmdir
	objects = S_ENVIRON_WRITE,S_GROUP_WRITE,S_LIMITS_WRITE,S_LOGIN_WRITE,S_PASSWD_READ,S_PASSWD_WRITE,S_USER_WRITE,AUD_CONFIG_WR
	SRC = SRC_Start,SRC_Stop,SRC_Addssys,SRC_Chssys,SRC_Delssys,SRC_Addserver,SRC_Chserver,SRC_Delserver
	kernel = PROC_Create,PROC_Delete,PROC_Execute,PROC_RealUID,PROC_AuditID,PROC_RealGID,PROC_Environ,PROC_SetSignal,PROC_Limits,PROC_SetPri,PROC_Setpri,PROC_Privilege,PROC_Settimer
	files = FILE_Open,FILE_Read,FILE_Write,FILE_Close,FILE_Link,FILE_Unlink,FILE_Rename,FILE_Owner,FILE_Mode,FILE_Acl,FILE_Privilege,DEV_Create
	svipc = MSG_Create,MSG_Read,MSG_Write,MSG_Delete,MSG_Owner,MSG_Mode,SEM_Create,SEM_Op,SEM_Delete,SEM_Owner,SEM_Mode,SHM_Create,SHM_Open,SHM_Close,SHM_Owner,SHM_Mode
	mail = SENDMAIL_Config,SENDMAIL_ToFile
	cron = AT_JobAdd,AT_JobRemove,CRON_JobAdd,CRON_JobRemove,CRON_Start,CRON_Finish
	tcpip = TCPIP_config,TCPIP_host_id,TCPIP_route,TCPIP_connect,TCPIP_data_out,TCPIP_data_in,TCPIP_access,TCPIP_set_time,TCPIP_kconfig,TCPIP_kroute,TCPIP_kconnect,TCPIP_kdata_out,TCPIP_kdata_in,TCPIP_kcreate

users:
	root = general

