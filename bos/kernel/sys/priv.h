/* @(#)36	1.23  src/bos/kernel/sys/priv.h, sysspriv, bos411, 9428A410j 6/16/90 00:34:06 */

/*
 * COMPONENT_NAME: SYSSEC - Security Component
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_PRIV
#define _H_PRIV

#include <sys/mode.h>	/* definition of statpriv() bits		*/
#include <sys/tcb.h>	/* definition of TCB flags */

struct priv
{
	unsigned long	pv_priv[2];  /* privilege bits */
};
typedef struct priv	priv_t;

#include <sys/pcl.h>	/* definition of PCL structure */

/* commands for getpriv and setpriv */
#define PRIV_COMMANDS	0xFFFF0000	/* mask for commands */
#define PRIV_ADD	0x00010000	/* add privileges */
#define PRIV_SUB	0x00020000	/* subtract privileges */
#define PRIV_SET	0x00030000	/* set privileges */

/* selector part of command word for getpriv and setpriv */
#define	PRIV_EFFECTIVE	0x00000001	/* effective privilege set */
#define	PRIV_INHERITED	0x00000002	/* inherited privilege set */
#define	PRIV_BEQUEATH	0x00000004	/* bequeath privilege set */
#define	PRIV_MAXIMUM	0x00000008	/* maximum privilege set */

/* commands for privilege() library routine  */
#define	PRIV_LAPSE	0x30001		/* lapse privilege */
#define	PRIV_ACQUIRE	0x30002		/* acquire privilege */
#define	PRIV_DROP	0x30003		/* drop privilege */

/* 
 * privilege definitions 
 */
 
/* object privilege (10-19) */
#define SET_OBJ_DAC	10	/* setting object owner, group, mode, ACL */
#define SET_OBJ_RAC	11	/* not used */
#define SET_OBJ_MAC	12	/* setting object MAC sensitivity label */
#define SET_OBJ_INFO	13	/* setting object MAC information label */
#define SET_OBJ_STAT	14	/* setting misc. attributes */
#define SET_OBJ_PRIV	15	/* setting object PCL and TP, TCB attributes */

/* subject privileges (20-29) */
#define SET_PROC_DAC	20	/* setting procs real uid, gid and group set */
#define SET_PROC_RAC	21	/* setting procs resource limits, quotas */
#define SET_PROC_MAC	22	/* setting procs MAC sensitivity label */
#define SET_PROC_INFO	23	/* setting procs MAC information label */
#define SET_PROC_ENV	24	/* setting procs protected environment */
#define SET_PROC_ACCT	25	/* not used */
#define SET_PROC_AUDIT	26	/* setting procs audit classes and ID */

/* system configuration privileges (40-64) */
#define AUDIT_CONFIG	40	/* config auditing, bin and stream modes */
#define ACCT_CONFIG	41	/* enabling accounting */
#define DEV_CONFIG	42	/* configuring hardware */
#define FS_CONFIG	43	/* mounting filesystems, chroots */
#define	GSS_CONFIG	44	/* configuring X, graphics subsystems */
#define	LVM_CONFIG	45	/* configuring the Logical Volume Manager */
#define NET_CONFIG	46	/* network (SNA, TCP/IP, OSI) configuration */
#define RAS_CONFIG	47	/* configuring and writing RAS records */
				/* error logging, tracing, dumps */
#define RAC_CONFIG	48	/* not used */
#define SYS_CONFIG	49	/* adding/removing kernel extensions */
#define SYS_OPER	50	/* setting time, system naming info */
#define TPATH_CONFIG	51	/* setting terminal SAK, Trusted state */
#define VMM_CONFIG	52	/* defining paging space */

/* policy bypass privileges (0-9) */
#define BYPASS_DAC_WRITE	1	/* write all objects */
#define BYPASS_DAC_READ		2	/* read all objects (inc dir search */
#define BYPASS_DAC_EXEC		3	/* execute all programs */
#define BYPASS_DAC_KILL		4	/* signal all processes */
#define BYPASS_RAC		5	/* consume all resources */
#define BYPASS_MAC_WRITE	6	/* write all objects */
#define BYPASS_MAC_READ		7	/* read all objects */
#define BYPASS_TPATH		8	/* do actions where tpath is required */
#define BYPASS_DAC		9

/* pseudo privileges, used only by privcheck() */
#define TRUSTED_PATH	910


#endif /* _H_PRIV */
