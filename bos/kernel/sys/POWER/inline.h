/* @(#)38       1.9.1.2  src/bos/kernel/sys/POWER/inline.h, sysml, bos41J, 9521A_all 5/23/95 13:54:54 */

#ifndef _H_INLINE
#define _H_INLINE
/*
 * COMPONENT_NAME: (SYSML) Kernel Machine Language
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * This module contains in-line assembler functions for use by kernel.
 * These functions are for use by the base kernel only.
 */

#include <sys/types.h>
/*
 * Misc. functions that can be used in any environment
 */
int clz32(int val);
void stbrx(int val, void *addr);
void sthbrx(short val, void *addr);
int lbrx(void *addr);
int lhbrx(void *addr);
void isync(void);
void eieio(void);
void SYNC(void);
void mtear(ulong ear);

#pragma mc_func clz32	{ "7c630034" }          /* cntlz r3,r3 */
#pragma mc_func stbrx	{ "7c60252c" }          /* stbrx r3,0,r4 */
#pragma mc_func sthbrx	{ "7c60272c" }          /* sthbrx r3,0,r4 */
#pragma mc_func lbrx    { "7c601c2c" }          /* lbrx r3,0,r3 */
#pragma mc_func lhbrx   { "7c601e2c" }          /* lhbrx r3,0,r3 */
#pragma mc_func isync	{ "4c00012c" }          /* isync */
#pragma mc_func eieio	{ "7c0006ac" }		/* eieio */
#pragma mc_func SYNC    { "7c0004ac" }          /* sync  */
#pragma mc_func mtear   { "7c7a43a6" }          /* mtspr EAR,r3 */

#pragma reg_killed_by clz32     gr3
#pragma reg_killed_by stbrx     gr3-gr4
#pragma reg_killed_by sthbrx    gr3-gr4
#pragma reg_killed_by lbrx      gr3
#pragma reg_killed_by lhbrx     gr3
#pragma reg_killed_by isync
#pragma reg_killed_by eieio
#pragma reg_killed_by SYNC
#pragma reg_killed_by mtear     gr3

#ifdef _KERNSYS

/*
 * The following functions are used to manipulate the HID0 register 
 * for 603 and 604 processors
 */
int mf_603_hid0();
void mt_603_hid0(ulong value);
#pragma mc_func mf_603_hid0  { "7c70faa6" }          /* mfspr r3,HID0 */
#pragma mc_func mt_603_hid0  { "7c70fba6" }          /* mtspr HID0,r3 */
#pragma reg_killed_by mf_603_hid0     gr3
#pragma reg_killed_by mt_603_hid0     gr3

/*
 * These functions are "bla's" to overlay addresses.  This is done
 * so that the reg_killed pragma can be used.  The overlay address
 * must be kept up to date with overlay.h
 */
#pragma mc_func chgsr	{ "4800a483" }		/* bla 0xa480 */
#pragma mc_func ldsr	{ "4800a49b" }		/* bla 0xa498 */
#pragma mc_func mtsr	{ "4800a49b" }		/* bla 0xa498 */
#pragma mc_func mfsr	{ "4800a4af" }		/* bla 0xa4ac */
#pragma mc_func ldfsr	{ "4800a4af" }		/* bla 0xa4ac */
#pragma mc_func mfsri	{ "4800a4bb" }		/* bla 0xa4b8 */

/*
 * The register kill information must be kept up to date
 * with overlay functions for all platforms
 */
#pragma reg_killed_by chgsr	gr3-gr5,lr
#pragma reg_killed_by ldsr	gr3-gr4,lr
#pragma reg_killed_by mtsr	gr3-gr4,lr
#pragma reg_killed_by mfsr	gr3,lr
#pragma reg_killed_by ldfsr	gr3,lr
#pragma reg_killed_by mfsri	gr3,lr

#ifdef _POWER_MP

#define MAXCPU			8		/* Maximum number of cpus   */
#define NCPUS()			(number_of_cpus)/* Number of running cpus   */
#define GET_PPDA(CPU)		(&ppda[(CPU)])
#define CPUID			((my_ppda())->cpuid)
#define PPDA             	(my_ppda())
#define CSA              	(my_csa())
#define SET_CSA(MST)		(set_csa(MST))
#define CURTHREAD    		(get_curthread())
#define SET_CURTHREAD(T)    	(set_curthread(T))

extern uint number_of_cpus;
extern struct ppda *ppda_p_tab[];
extern struct ppda *my_ppda(void);
extern struct mstsave *my_csa(void);
extern struct thread *get_curthread(void);
extern void set_csa(struct mstsave *);
extern void set_curthread(struct thread *);

/* Pragmas for inlining -- see overlay.h for function addresses */
#pragma mc_func my_ppda		{ "4800b403" }  /* bla 0xb400 */
#pragma mc_func my_csa		{ "4800b40b" }  /* bla 0xb408 */
#pragma mc_func get_curthread	{ "4800b42b" }  /* bla 0xb428 */
#pragma mc_func set_csa		{ "4800b44b" }  /* bla 0xb448 */
#pragma mc_func set_curthread	{ "4800b463" }  /* bla 0xb460 */

#pragma reg_killed_by my_ppda		gr3,lr
#ifndef _SLICER
#pragma reg_killed_by my_csa		gr3,lr
#pragma reg_killed_by get_curthread	gr3,lr
#else
#pragma reg_killed_by my_csa		gr3-gr4,lr
#pragma reg_killed_by get_curthread	gr3-gr4,lr
#endif
#pragma reg_killed_by set_csa		gr4,lr
#pragma reg_killed_by set_curthread	gr4,lr

/* Bits in MFRR flags */
#define HARD_MFRR_INT	1

#define csa			CSA
#define curthread		CURTHREAD
#define curproc			(curthread->t_procp)

#else /* _POWER_MP */

#define MAXCPU			1		/* Maximum number of cpus   */
#define NCPUS()			1		/* Number of running cpus   */
#define GET_PPDA(CPU)		(&ppda[0])
#define CPUID			(MP_MASTER)
#define PPDA             	(&ppda[0])
#define CSA              	(ppda[0]._csa)
#define SET_CSA(MST)		(ppda[0].mstack->prev=ppda[0]._csa=(MST))
#define CURTHREAD    		(ppda[0]._curthread)
#define SET_CURTHREAD(T)    	(ppda[0]._curthread=(T))

extern struct mstsave *g_mstack;        /* next mstsave stack pointer 	*/
extern struct mststack mststack1;      	/* first mstsave stack entry 	*/
extern struct mstsave *csa;             /* current save area @  	*/
extern struct mstsave *mstack;          /* Next save area ptr   	*/
extern struct thread *curthread;        /* current thread blk. @	*/

#define curproc 		(curthread->t_procp)

#endif /* _POWER_MP */

/*
 * Misc base kernel only assembler functions
 */
int mfdbat0u(void);
int mfdbat1u(void);
int mfdbat2u(void);
int mfdbat0l(void);
int mfdbat1l(void);
int mfdbat2l(void);
void mtdbat0u(int batu);
void mtdbat1u(int batu);
void mtdbat2u(int batu);
void mtdbat0l(int batl);
void mtdbat1l(int batl);
void mtdbat2l(int batl);
void tlbsync(void);
void tlbie(caddr_t eaddr);
int mfpvr(void);


#pragma mc_func mfdbat0u	{ "7c7882a6" }		/* mfdbatu r3, 0 */
#pragma mc_func mfdbat1u	{ "7c7a82a6" }		/* mfdbatu r3, 1 */
#pragma mc_func mfdbat2u	{ "7c7c82a6" }		/* mfdbatu r3, 2 */
#pragma mc_func mfdbat0l	{ "7c7982a6" }		/* mfdbatl r3, 0 */
#pragma mc_func mfdbat1l	{ "7c7b82a6" }		/* mfdbatl r3, 1 */
#pragma mc_func mfdbat2l	{ "7c7d82a6" }		/* mfdbatl r3, 2 */
#pragma mc_func mtdbat0u	{ "7c7883a6" }		/* mtdbatu 0, r3 */
#pragma mc_func mtdbat1u	{ "7c7a83a6" }		/* mtdbatu 1, r3 */
#pragma mc_func mtdbat2u	{ "7c7c83a6" }		/* mtdbatu 2, r3 */
#pragma mc_func mtdbat0l	{ "7c7983a6" }		/* mtdbatl 0, r3 */
#pragma mc_func mtdbat1l	{ "7c7b83a6" }		/* mtdbatl 1, r3 */
#pragma mc_func mtdbat2l	{ "7c7d83a6" }		/* mtdbatl 2, r3 */
#pragma mc_func tlbsync		{ "7c00046c" }		/* tlbsync */
#pragma mc_func tlbie		{ "7c001a64" }		/* tlbie r3 */
#pragma mc_func mfpvr		{ "7c7f42a6" }		/* mfpvr r3 */

#pragma reg_killed_by mfdbat0u	gr3
#pragma reg_killed_by mfdbat1u	gr3
#pragma reg_killed_by mfdbat2u	gr3
#pragma reg_killed_by mfdbat0l	gr3
#pragma reg_killed_by mfdbat1l	gr3
#pragma reg_killed_by mfdbat2l	gr3
#pragma reg_killed_by mtdbat0u	gr3
#pragma reg_killed_by mtdbat1u	gr3
#pragma reg_killed_by mtdbat2u	gr3
#pragma reg_killed_by mtdbat0l	gr3
#pragma reg_killed_by mtdbat1l	gr3
#pragma reg_killed_by mtdbat2l	gr3
#pragma reg_killed_by tlbsync
#pragma reg_killed_by tlbie     gr3
#pragma reg_killed_by mfpvr	gr3

#endif /* _KERNSYS */

#endif /* _H_INLINE */
