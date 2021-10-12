/* static char sccsid[] = "@(#)79  1.3  src/bos/diag/tu/mouse/mou_io.h, tu_mouse, bos411, 9428A410j 1/19/94 11:01:52"; */
/*
 *   COMPONENT_NAME: tu_mouse
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


extern void clean();
extern int non_block( int );
extern int re_block(int);
extern int rd_byte(int , unsigned char *, unsigned int );
extern int rd_word(int, unsigned int *, unsigned int );
extern int wr_byte(int , unsigned char *, unsigned int );
extern int wr_word(int , unsigned int *, unsigned int );
extern int wr_2byte(int  , ushort *, unsigned int );
extern int init_dev(int);
extern int rd_iocc(int , unsigned char *, unsigned int );
extern int wr_iocc(int , unsigned char *, unsigned int );
extern int enable_mou(int);
extern int set_sem(int);
extern int rel_sem();
