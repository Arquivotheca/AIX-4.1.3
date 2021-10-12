static char sccsid[] = "@(#)12  1.1  src/bos/usr/lib/piosCN/lib/shrink.c, ils-zh_CN, bos41J, 9516A_all 4/17/95 15:56:49";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: getmargin
 *              shrink
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include "lc.h"
#include "ffaccess.h"
#include "shrink.h"

void getmargin(Fontimg f, Margin margin)
{
	int h, w, bytes_in_line, real_bytes_in_line, bit_off, leftmargin, rightmargin, blanklines;
	char c, *p, *ep;

	bit_off = f->w & 7;
	real_bytes_in_line = (f->w >> 3) + (bit_off > 0);

	margin->top = 0;
	margin->bottom = 0;
	margin->left = f->w;
	margin->right = f->w;

	ep = f->img;
	for(h = 0; h < f->h; ++h){
		leftmargin = 0;
		rightmargin = 0;
		p = ep;
		ep += real_bytes_in_line;
		for(bytes_in_line = 0; bytes_in_line < real_bytes_in_line; ++bytes_in_line){
			leftmargin += 8;
			c = *p;
			if(c){
				while(c){
					--leftmargin;
					c >>= 1;
				}
				margin->left = margin->left < leftmargin? margin->left: leftmargin;
				p = ep;
				do rightmargin += 8;
				while(*--p == 0);
				if(bit_off) rightmargin -= 8 - bit_off;
				c = *p;
				while(c){
					--rightmargin;
					c <<= 1;
				}
				margin->right = margin->right < rightmargin? margin->right: rightmargin;
				break;
			}else	++p;
		}
		if(p == ep){
			++blanklines;
			if(margin->top == h) ++margin->top;
		}else	blanklines = 0;
		ep += f->bytes_in_line - real_bytes_in_line;
	}

	if(margin->top == f->h){
		margin->top = (f->h + 1) >> 1;
		margin->bottom = f->h - margin->top;
		margin->left = (f->w + 1) >> 1;
		margin->right = f->w - margin->left;
	}else	margin->bottom = blanklines;
}

void shrink(Fontimg f, int high, int wide)
{
	int h, w, bytes_in_line, old_bytes_in_line, thigh, lwide, byteoffset, bitoffset, i, j;
	char *fp, *tp;
	MarginRec marginrec;

	getmargin(f, &marginrec);

	f->h -= high;
	f->w -= wide;
	old_bytes_in_line = f->bytes_in_line;
	f->bytes_in_line = (f->w + 7) >> 3;

	if(marginrec.bottom - marginrec.top >= high) thigh = 0;
	else if(marginrec.top - marginrec.bottom >= high) thigh = high;
	else thigh = (high + marginrec.top - marginrec.bottom + 1) >> 1;

	if(marginrec.right - marginrec.left >= wide) lwide = 0;
	else if(marginrec.left - marginrec.right >= wide) lwide = wide;
	else lwide = (wide + marginrec.left - marginrec.right + 1) >> 1;

	byteoffset = lwide >> 3;
	bitoffset = lwide & 7;

	tp = f->img;
	fp = tp + old_bytes_in_line * thigh + byteoffset;

	for(h = 0; h < f->h; ++h){
		for(bytes_in_line = 0; bytes_in_line < f->bytes_in_line; ++bytes_in_line){
			*tp++ = (*fp << bitoffset) | (*++fp >> (8 - bitoffset));
		}
		fp += old_bytes_in_line - bytes_in_line;
	}
}
