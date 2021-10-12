/* @(#)37       1.6  src/bos/kernel/sys/strconf.h, sysxpse, bos412, 9446B 11/9/94 16:46:06 */
/*
 *   COMPONENT_NAME: SYSXPSE strconf.h
 *
 *   ORIGINS: 27 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef	_STRCONF_H_
#define	_STRCONF_H_

/*
 * kernel configuration parameters
 */

typedef struct {
	char *sc_name;		/* null-terminated module/device name	*/
	struct streamtab *sc_str;	/* pointer to streamtab		*/
	int sc_flags;		/* which style open to use		*/
	int sc_major;		/* major number for device		*/
	int sc_sqlevel;		/* Synchronization level mode		*/
	caddr_t sc_sqinfo;	/* pointer for SQLVL_ELSEWHERE case	*/
} kstrconf_t;
#define	strconf_t kstrconf_t

#define sc_open_style   sc_flags

#define	STR_NEW_OPEN	0	/* SVR4 style opens ("default")		*/
#define	STR_OLD_OPEN	1	/* SVR3 style opens			*/
#define STR_MPSAFE	2	/* must be treated as MP safe or efficient */
#define STR_Q_SAFETY	4	/* Module needs safe callbacks */
#define STR_Q_NOTTOSPEC	8	/* Module may sleep or fault in service routine. Needs schedule. */

#define	STR_LOAD_DEV		1
#define	STR_LOAD_MOD		2
#define	STR_UNLOAD_DEV		3
#define	STR_UNLOAD_MOD		4

/*
 * Synchronization level codes.
 * These are supplied to fmodsw_install and dmodsw_install and stored
 * in the appropriate tables.  sth_osr_open and sth_ipush use these to
 * set up synch queue subordination for new devices and modules.
 */

#define SQLVL_DEFAULT           0
#define SQLVL_GLOBAL            1
#define SQLVL_ELSEWHERE         2
#define SQLVL_MODULE            3
#define SQLVL_QUEUEPAIR         4
#define SQLVL_QUEUE             5

extern  int             str_install(int , strconf_t *);

#ifndef	_KERNEL

/*
 * user configuration parameters
 */

typedef struct node {
	dev_t	dev;		/* device major,minor pair		*/
	char	*name;		/* node name in filesystem		*/
} node_t;

typedef struct ustrconf {
	int	version;	/* data structure version		*/
	char	*extname;	/* kernel extension to operate upon	*/
	int	flags;		/* generic flags			*/
	int	maj;		/* device major				*/
	void	*dds;		/* DDS/MDI to give to extension		*/
	int	ddslen;		/* length of DDS/MDI			*/
	char	*odmkey;	/* key to gen a unique device major	*/
	node_t	*nodes;		/* nodes for this device		*/
} ustrconf_t;

/* ustrconf.version */
#define	STRCONF_V1	1	/* version of data structure		*/

/* ustrconf.flags */
#define	SF_MODULE	0x0000	/* describes a module			*/
#define	SF_DEVICE	0x0001	/* describes a device			*/
#define	SF_DUP		0x0002	/* can be loaded several times		*/
#define	SF_KEEPNODES	0x0004	/* create/remove nodes on first/last load */

/* cmds */
#define	STR_LOAD	1	/* load extension			*/
#define	STR_UNLOAD	2	/* unload extension			*/
#define	STR_QUERY	3	/* query extension's existance		*/
#define	STR_INIT	4	/* init extension			*/
#define	STR_TERM	5	/* terminate extension			*/

#endif	/* _KERNEL */
#endif	/* _STRCONF_H_ */
