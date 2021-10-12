/*
static char sccsid[] = "@(#)77  1.5  src/bos/usr/lib/asw/mpqp/mpqp.pro, ucodmpqp, bos411, 9428A410j 8/23/93 13:22:10";
*/

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	mpqp.pro 		: Function prototypes for commonly used
 *				  functions.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
   The function prototypes.  Notice that these also appear as external
   declarations.
*/

/* The following four externs define the parameter interface to the IN and
   OUT I/O interface instructions implemented. */

extern unsigned char	in08 ( unsigned short );
extern unsigned short	in16 ( unsigned short );

extern void		out08 ( unsigned short, unsigned char );
extern void		out16 ( unsigned short, unsigned short );

/* All commands with high nibble = 0xF are adapter, vs. port, commands.
   this extern is to the adapter command dispatch routine. */

extern void		acmdvec ( unsigned char );

/* C language interrupt handlers cannot do IRET machine instructions, as
   required to restore the flags for the interrupted procedure.  This
   extern declares the call point for an assembler stub which DOES NOT
   RETURN to the caller, but directly to the caller's caller via IRET. */

extern void			*IRET();

extern void			enable ( void );
extern void			disable ( void );
extern void			log_error ();

extern sch_lev			portbit ( unsigned char );
extern unsigned char		port ( sch_lev );

extern unsigned long		queue_readl ();
extern int			queue_writel();
extern int			queue_insertl ();
extern unsigned long 		queue_previewl ();

extern unsigned short		queue_readw ();
extern int			queue_writew();
extern int			queue_insertw ();
extern unsigned short 		queue_previeww ();

extern unsigned char		queue_readb ();
extern int			queue_writeb();
extern int 			queue_insertb ();
extern unsigned char 		queue_previewb ();

extern int			queue_init  ();

/* 0xF? Vectors */
extern int	ramtst(), chktst(), cputst(), ciotst(), scctst(), ssttst();
extern int	adreset();

/* 0xE? Vectors */
extern unsigned far	*memtest();
extern int		dmatest();

/* 0xD? Vectors */
extern int	gmsize(), getiid(), geteid(), setcio(), setscc(), setdma();
extern int	settmr(), intwdt(), priswc();
