/* @(#)58       1.2  src/bos/usr/ccs/lib/libc/xputc.h, libcprnt, bos41J, 9518A_all 4/27/95 11:55:04 */
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Maximum number of digits in any integer representation */
/* With support for 64-bit long long integers, the largest */
/* unsigned number converted to octal is 22 characters long */
#ifndef _H_XPUTC
#define _HXPUTC
/*
 * Fast putc macros. 
 *
 * To use these: 
 *
 * Doeclare the required automatic variables used by the macros by inserting
 * xputc_decl 
 *
 * just after the declarations of the procedure. 
 *
 * after performing fp=fopen(filename,mode) to open the file for writing, or at
 * the beginning of the procedure, in case fp is already open, insert: 
 *
 * XPUTC_INIT(fp); 
 *
 * use XPUTC(c,fp) instead of putc(c,fp) as you normally would in the body of
 * the procedure 
 *
 * At the end of the procedure, before returning or performing fclose(fp) to
 * close the file, call XPUTC_FINISH(fp); 
 *
 * These macros should preferably not be mixed with other stdio calls to the
 * same file (except for fopen and fclose, as described) 
 */
/*
 * We are assuming that putc is the same as putc_unlocked for making these
 * optimizations. For thread-safe implementation, the file must already be
 * locked, during the printf or fprintf call. 
 */

/* xputc_decl declares the required automatic (register) variables */

#define XPUTC_DECL  int _cnt; char *_ptr; int _n_put; 

/*
 * xputc_init loads the buffer byte count and pointer into registers 
 */

#define XPUTC_INIT(p) (_cnt=(p)->_cnt,_ptr=(p)->_ptr)

/* xputc_finish stores the registers into their home locations */

#define XPUTC_FINISH(p) ((p)->_cnt=_cnt,(p)->_ptr = _ptr)

/*
 * xputc macro keeps _cnt and _ptr values in registers, except when calling
 * __flsbuf, it stores them in  their home locations in memory before the
 * call and loads them back after the call 
 */
#define XPUTC(x,p)    \
    if(--_cnt>=0) {   \
           *_ptr++ = (x); \
    }   \
    else  {                       \
       XPUTC_FINISH(p);           \
       if(_flsbuf((x),p) == EOF) goto error_return;       \
       XPUTC_INIT(p);                         \
    }

#define PUT1(x,n,iop)  \
if((n) <= _cnt) {   \
for(_n_put=0;_n_put<(n);_n_put++) (void)(*_ptr++ = (x)); \
_cnt -= (n);  \
}\
else { \
 for(_n_put=0;_n_put<(n);_n_put++) XPUTC((x),iop); \ 
}

/* faster PUT macro to write n characters from buffer s to file iop */
/*
 * this macro also assumes that _ptr and _cnt fields are already in
 * registers. Uses memcpy for performing the buffer copy where possible 
 */

#define PUT0(s,n,iop) \
if((n)<=_cnt) { \
for(_n_put=0;_n_put<(n);_n_put++) (void)(*_ptr++ = s[_n_put]); \
_cnt -=(n); \
} \
else { \
 XPUTC_FINISH(iop); \
 if(fwrite(s,1,n,iop) < (n) ) goto error_return; \
 XPUTC_INIT(iop); \
}
#endif
