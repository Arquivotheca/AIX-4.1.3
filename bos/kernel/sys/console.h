/* @(#)24	1.3.1.1  src/bos/kernel/sys/console.h, sysio, bos411, 9428A410j 7/21/92 21:39:34 */

#ifndef _H_CONSOLE_
#define _H_CONSOLE_
/*    
 * COMPONENT_NAME: (SYSIO) Console Device Driver Header File
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 */

/* special console config command */
#define CONSOLE_CFG	0x636F6E73	    /* console config command  */

/* special activate/deactivate console tee command */
#define CHG_CONS_TEE	0x636F6E74	    /* activate virtual cons command */

/* Console config commands */
#define CONID           ('Q'<<8)            /* unique ioctl id              */
#define CONSETDFLT      (CONID|1)           /* cmd to set default device */
#define CONSETPRIM	(CONID|2)	    /* cmd to set primary device*/
#define CONS_NULL	(CONID|3)	    /* cmd to change to NULL device*/
#define CONSGETDFLT	(CONID|4)	    /* cmd to get default device  */
#define CONSGETPRIM	(CONID|5)	    /* cmd to get primary device  */
#define CONSGETCURR	(CONID|6)	    /* cmd to get current device  */
#define CONS_ACTDFLT	(CONID|7)	    /* cmd to activate default device*/
#define CONS_ACTPRIM	(CONID|8)	    /* cmd to activate primary device*/
#define CONS_NODEBUG	(CONID|9)	    /* cmd to turn debug off  */
#define CONS_DEBUG	(CONID|10)	    /* cmd to turn debug on  */
#define CONS_CTTY	(CONID|11)	    /* console can be controlling tty */
#define CONS_NCTTY	(CONID|12)	    /* console can not be cont tty */

/* Structure used with the console config command */
struct cons_config
{
	int	cmd;
	char	*path;			    /* pointer to pathname */	
};

/* Structure used with the activate console output tee command */
struct tcons_info
{
	dev_t	tcons_devno;	/* device # for console output tee */
	chan_t	tcons_chan;   /* channel # for console output tee */
};

#endif /* _H_CONSOLE_ */
