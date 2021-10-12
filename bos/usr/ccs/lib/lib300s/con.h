/* @(#)57	1.1  src/bos/usr/ccs/lib/lib300s/con.h, libt300s, bos411, 9428A410j 9/30/89 15:44:09 */
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: con.h
 *
 * ORIGINS: 4,10,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <termio.h>
/* gsi plotting output routines */
# define DOUBLE 010
#define ADDR 0100
#define COM 060
#define PENUP 04
#define MAXX 070
#define MAXY 07
#define SPACES 7
# define DOWN 012
# define UP 013
# define LEFT 010
# define RIGHT 040
# define BEL 007
# define ESC 033
# define ACK 006
# define INPLOT 'P'
# define CR 015
# define FF 014
# define VERTRESP 48
# define HORZRESP 60.
# define VERTRES 8.
# define HORZRES 6.
/* down is line feed, up is reverse line feed,
   left is backspace, right is space.  48 points per inch
   vertically, 60 horizontally */

extern int xnow, ynow;
extern struct termio ITTY, PTTY;
extern int OUTF;
extern float HEIGHT, WIDTH, OFFSET;
extern int xscale, xoffset, yscale;
extern float botx, boty, obotx, oboty, scalex,scaley;

