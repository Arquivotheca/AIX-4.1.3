/* @(#)12       1.1  src/bos/usr/lib/methods/cfgcommon/cfgcommon.h, cfgmethods, bos411, 9428A410j 6/28/94 07:12:49 */
/*
 * COMPONENT_NAME: (CFGMETHODS) cfgcommon.h - header for Generic Config Method Code
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
extern struct	CuDv    cudv;		/* Customized object for device */
extern struct	PdDv    pddv;		/* Predefined object for device */
extern struct	CuDv    pcudv;		/* Customized object for parent */
extern mid_t	kmid;                   /* Device driver kmid */
extern dev_t	devno;                  /* Device driver devno */
extern int	devinst;		/* Device instance number */
extern int	loaded_dvdr;		/* Indicates to unload driver on err */
extern int	inited_dvdr;		/* Indicates CFG_TERM needed on error */
extern char	dvdr[16];		/* Device driver load module name */
