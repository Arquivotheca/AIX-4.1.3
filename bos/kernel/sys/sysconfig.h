/* @(#)03       1.9  src/bos/kernel/sys/sysconfig.h, sysios, bos411, 9428A410j 3/11/94 15:54:39 */
#ifndef _H_SYSCONFIG
#define _H_SYSCONFIG
/*
 * COMPONENT_NAME: (SYSIOS) Sysconfig system call header file
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

#include <sys/types.h>
#include <sys/lockl.h>

#define CONF_SUCC	0
#define CONF_FAIL	-1

#define SYS_KLOAD	0x01	/* load kernel ext. into kernel	*/
#define SYS_SINGLELOAD	0x02	/* load kernel ext. only once	*/
#define SYS_QUERYLOAD	0x03	/* check if kernel ext. in kernel*/
#define SYS_KULOAD	0x04	/* unload kernel ext.from kernel*/
#define SYS_CFGKMOD	0x05	/* call kernel ext. entry pt.	*/ 
#define SYS_CFGDD	0x06	/* call dev.driver's entry pt.	*/
#define SYS_QDVSW	0x07	/* check dev.switch entry status*/
#define SYS_GETPARMS	0x08	/* get runtime system parameters*/
#define SYS_SETPARMS	0x09	/* set runtime system parameters*/

/*
 * Structure for loading a kernel extension
 */
struct cfg_load
{
	caddr_t	path;		/* ptr to object module pathname	*/
	caddr_t	libpath;	/* ptr to a substitute libpath		*/
	mid_t	kmid;		/* kernel module id (returned)		*/
};

/*
 * Structure for calling a kernel extension's module entry point
 */
struct cfg_kmod
{
	mid_t	kmid;		/* module ID of module to call		*/
	int	cmd;		/* command parameter for module		*/
	caddr_t mdiptr;		/* pointer to module dependent info	*/
	int	mdilen;		/* length of module dependent info	*/
};

/*
 * Structure for calling a device driver's config (module) entry point
 */
struct cfg_dd
{
	mid_t	kmid;		/* module ID of device driver		*/
	dev_t	devno;		/* device major/minor number		*/
	int	cmd;		/* config command code for device driver*/
	caddr_t ddsptr;		/* pointer to device dependent structure*/
	int	ddslen;		/* length of device dependent structure	*/
};

/*
 * Structure for checking the status of a device switch entry
 */
struct qry_devsw
{
	dev_t	devno;		/* device number of entry		*/
	uint	status;		/* returned status information		*/
};

/*
 * Config notification control block
 */
struct cfgncb
{
	struct cfgncb	*cbnext;	/* next control block on chain	*/
	struct cfgncb	*cbprev;	/* previous control block on chain*/
	int		(*func)();	/* notification function	*/
};

/*
 * The config notification control block lock for use with lockl/unlockl.
 * This lock is used to serialize cfgnadd()/cfgndel() kernel services
 * (i.e. updates to the config notification control block chain), as well
 * as the running of this chain by the sysconfig SYS_SETPARMS system call.
 */
extern int cfgcb_lock;

/*
 * The following are the possible values of the cmd parm to the caller's
 * config notification routine.  CFGV_PREPARE is used to check the
 * system parameters in the cfgvar structure for validity.  CFGV_COMMIT
 * is used to indicate that the new values in the cfgvar structure will
 * now be updated (i.e. all values are valid).
 */
#define	CFGV_PREPARE	0x01	/* check parms for validity		*/
#define	CFGV_COMMIT	0x02	/* commit parm changes			*/

#ifndef _NO_PROTO

int sysconfig(
	int cmd,		/* function requested: load, unload, etc*/
	void *parmp,		/* addr of struct containing info for cmd*/
	int parmlen);		/* length of parmp information		*/

#else

int sysconfig();

#endif /* _NO_PROTO */

/* Define offsets for all the RO variables */
#define OFFSET_VLOCK ((int)&((struct var *)0)->v_lock)
#define OFFSET_VFILE ((int)&((struct var *)0)->v_file)
#define OFFSET_VPROC ((int)&((struct var *)0)->v_proc)
#define OFFSET_VCLIST ((int)&((struct var *)0)->v_clist)
#define OFFSET_VTHREAD ((int)&((struct var *)0)->v_thread)
#define OFFSET_VNCPUS ((int)&((struct var *)0)->v_ncpus)
#define OFFSET_VNCPUSCFG ((int)&((struct var *)0)->v_ncpus_cfg)

#define OFFSET_VELOCK ((int)&((struct var *)0)->ve_lock)
#define OFFSET_VEFILE ((int)&((struct var *)0)->ve_file)
#define OFFSET_VEPROC ((int)&((struct var *)0)->ve_proc)
#define OFFSET_VETHREAD ((int)&((struct var *)0)->ve_thread)
#define OFFSET_VBPROC ((int)&((struct var *)0)->vb_proc)
#define OFFSET_VBTHREAD ((int)&((struct var *)0)->vb_thread)

#define NUM_VARS 13  /* size of the ro_vars array */

#endif  /* _H_SYSCONFIG */
