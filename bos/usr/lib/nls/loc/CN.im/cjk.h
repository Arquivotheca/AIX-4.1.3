/* @(#)95	1.1  src/bos/usr/lib/nls/loc/CN.im/cjk.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:02  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: HIBYTE
 *		HIWORD
 *		LOBYTE
 *		LOWORD
 *		TO_HI_BYTE
 *		TO_LOW_BYTE
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
typedef int		    BOOL;
#define FALSE		    0
#define TRUE		    1


typedef unsigned char	    BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned int	    UINT;

#ifdef STRICT
typedef signed long	    LONG;
#else
#define LONG long
#endif


#define LOBYTE(w)	    ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((UINT)(w) >> 8) & 0xFF))

#define LOWORD(l)           ((WORD)(DWORD)(l))
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))


typedef char *              LPSTR;
typedef unsigned int 	    HWND;

#ifdef STRICT
typedef const void NEAR*        HANDLE;
#else   /* STRICT */
typedef UINT                    HANDLE;
#endif  /* !STRICT */



/* OpenFile() Flags */
#define OF_READ 	    0x0000
#define OF_WRITE	    0x0001
#define OF_READWRITE	    0x0002
#define OF_SHARE_COMPAT	    0x0000
#define OF_SHARE_EXCLUSIVE  0x0010
#define OF_SHARE_DENY_WRITE 0x0020
#define OF_SHARE_DENY_READ  0x0030
#define OF_SHARE_DENY_NONE  0x0040
#define OF_PARSE	    0x0100
#define OF_DELETE	    0x0200
#define OF_VERIFY	    0x0400      /* Used with OF_REOPEN */
#define OF_SEARCH	    0x0400	/* Used without OF_REOPEN */
#define OF_CANCEL	    0x0800
#define OF_CREATE	    0x1000
#define OF_PROMPT	    0x2000
#define OF_EXIST	    0x4000
#define OF_REOPEN	    0x8000


#define TO_LOW_BYTE(x,y)   x=((x&0xff00)|(y))
#define TO_HI_BYTE(x,y)    x=((x&0xff)|((y)<<8))



