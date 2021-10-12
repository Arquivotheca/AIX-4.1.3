/* @(#)25       1.14.1.5  src/bos/kernel/sys/var.h, sysios, bos411, 9428A410j 3/11/94 15:50:12 */
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystems
 *
 * FUNCTIONS: var.h 
 *
 * ORIGINS: 3, 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_VAR
#define _H_VAR

/*
 * Header structure for var structure.
 */
struct var_hdr
{
	long	var_vers;	/* version number of var structure	*/
	long	var_gen;	/* generation number of var structure	*/
	long	var_size;	/* size of the of the var structure 
				   returned by the sysconfig system call
				 */
	
};

/*
 * Structure for base kernel parameters
 * RW in comment indicates a Read-Write variable alterable by sysconfig
 * RO in comment indicates a Read-Only variable not alterable by sysconfig
 */
struct var {
	struct var_hdr	var_hdr;	/* cfgvar header information	    */

	int		v_bufhw;	/* RW: buffer pool high-water mark  */
	int     	v_mbufhw;	/* RW: max. mbufs high water mark   */
	int		v_maxup;	/* RW: max. # of user processes     */
	int		v_iostrun;	/* RW: enable disk i/o history      */
	int		v_leastpriv;	/* RW: least privilege enablement   */
	int		v_autost;	/* RW: automatic boot after halt    */
	int		v_maxpout;	/* RW: # of file pageouts at which  */
					/*     waiting occurs		    */
	int		v_minpout;	/* RW: # of file pageout at which   */
					/*     ready occurs		    */
	int		v_memscrub;	/* RW: memory scrubbing enabled	    */


	int		v_lock;		/* RO: # entries in record lock table*/
	char 		*ve_lock;	/* RO: ptr to end of recordlock table*/
	int		v_file;		/* RO: # entries in open file table  */
	char 		*ve_file;	/* RO: ptr to end of open file table */
	int		v_proc;		/* RO: max # of system processes     */
	char 		*ve_proc; 	/* RO: process table high water mark */
	int		v_clist;	/* RO: # of cblocks in cblock array  */
	int		v_thread;	/* RO: max # of system threads	     */
	char		*ve_thread;	/* RO: thread table high water mark  */
	char		*vb_proc;	/* RO: beginning of process table    */
	char		*vb_thread;	/* RO: beginning of thread table     */
	int		v_ncpus;	/* RO: number of active CPUs	     */
	int		v_ncpus_cfg;	/* RO: number of processor configured*/
	int		v_fullcore;	/* RW: full core enabled	     */
	char		v_initlvl[4];	/* RW: init level		     */
};


extern struct var v;

#define	SYSCONFIG_VARSIZE  ((int)&((struct var *)0)->v_lock)  

#endif	/* _H_VAR */

