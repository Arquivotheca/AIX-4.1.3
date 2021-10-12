/* @(#)68	1.2  src/bos/usr/include/imP.h, libim, bos411, 9428A410j 6/11/91 01:10:58 */
/*
 * COMPONENT_NAME :	LIBIM - AIX Input Method
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_imP_h
#define	_imP_h

#ifndef	True
#define	True	1
#endif
#ifndef	False
#define	False	0
#endif

#define MetaMask	Mod1Mask
#define AltGraphMask	Mod2Mask

#define	REALLOC(bp, size)\
	((bp) ? realloc((bp), (size)) : malloc(size))

#define	GET3DIGITS(ptr)\
	(((ptr)[0] << 16) + ((ptr)[1] << 8) + (ptr)[2])

#define	PUT3BYTES(ptr, c1, c2, c3) \
	((ptr)[0] = (c1), (ptr)[1] = (c2), (ptr)[2] = (c3))

#define	PUT3DIGITS(ptr, val) \
	PUT3BYTES(ptr, ((val) % 0x1000000) / 0x10000, \
	((val) % 0x10000) / 0x100, \
	(val) % 0x100)

#define	PUT3DECIMAL(ptr, val) \
	PUT3BYTES(ptr, \
		 (val) / 100 + '0', ((val) % 100) / 10 + '0', (val) % 10 + '0')

/*
 *	KeyMapElement
 *	There are several types of entries (see the defines below).
 */
typedef struct _KeyMapElement	{
	unsigned char	type;
	unsigned char	data[3];
}	KeyMapElement;

#define	KME_UNBOUND	0		/* unbound */
#define	KME_KEYSYM	1		/* MIT defined keysyms 0x00XXXXXX */
#define	KME_SSTR1	2		/* short string (1 byte) */
#define	KME_SSTR2	3		/* short string (2 byte) */
#define	KME_SSTR3	4		/* short string (3 byte) */
#define KME_LSTR	5		/* long string (< 4 byte) */
#define KME_BSTR	6		/* bound string */
#define KME_QPFK	7		/* ESC [ d d d q */
#define KME_ZPFK	8		/* ESC [ d d d z */
#define KME_ESEQ1	9		/* ESC X */
#define KME_ESEQ2	10		/* ESC X X */
#define KME_ESEQ3	11		/* ESC X X X */
#define KME_PRIVKEYSYM	12		/* private keysyms 0xf0XXXXXX */
#define KME_OTHERKEYSYM	13		/* other keysyms */

/* the top most byte of the aix private keysyms are 0x18 */
#define	AIX_PRIVKEYSYM	(0x18 << 24)

/*
 *	heap
 */
typedef struct _IMBuffer	{
	unsigned char	*data;
	unsigned int	len;
	unsigned int	siz;
}	IMBuffer;

/*
 *	Header of imkeymap.
 */
#define X_KEYMAP_MAGIC		0373	/* UNSUPPORTED (was 0372 in V10) */
#define RT256_KEYMAP_MAGIC	0375
#define RT_KEYMAP_MAGIC		0376	/* (was 0374 in V10) */
#define AIX_KEYMAP_MAGIC	0376	/* new version number - w/ IM */
#define	ILS_KEYMAP_MAGIC	0370	/* This is the one that AIX3.2 uses */

typedef struct _IMKeymapFile	{
	unsigned char	magic;
	unsigned char	dummy[3];
	unsigned int	stat;
	unsigned int	statsiz;
	unsigned int	ksym;
	unsigned int	ksymsiz;
	unsigned int	elmt;
	unsigned int	elmtsiz;
	unsigned int	oksym;		/* "other" keysyms (!0xf0 && !0x00) */
	unsigned int	oksymsiz;
	unsigned int	kstr;
	unsigned int	kstrsiz;
}	IMKeymapFile;

/*
 *	In-Core imkeymap
 */
typedef struct _IMKeymap	{
	IMKeymapFile	*file;
	short	*stat;
	unsigned int	maxstat;	/* # of stat */
	unsigned int	*ksym;
	unsigned int	nksym;		/* # of keysyms */
	KeyMapElement	*elmt;
	unsigned int	nstat;		/* # of mapped stat */
	unsigned int	*oksym;
	unsigned int	oksymsiz;
	IMBuffer	kstr;
	IMBuffer	bstr;
}	IMKeymap;

/*
 *	IMMap
 *	The return value type of IMInitializeKeymap()
 */
typedef struct _IMMapRec	{
	IMKeymap	*immap;
	IMBuffer	output;
	unsigned int	dead_state;
	int	accum_state;
	char	accumulation;
}	IMMapRec;

/*
 *	The following 2 data structures, OldKeyMapElt and OldIMKeymap
 *	were used in AIX V 3.1.5 and
 *	older.   They are converted to the new structures in mapping.c
 *
 *	OldKeyMapElt
 *	contains single-byte character bindings, function binding or
 *	indications that a keycode is actually bound in the string extension
 *	or in the runtime table
 *	OldIMKeymapFile
 *	contains image of old version imkeymap file.
 */

typedef union	{
	int	element;			/* each element is 4 bytes */
	struct	{
		unsigned	type:4;		/* type of key */
		unsigned	stat:4;		/* status of key */
		unsigned char	pad;		/* NOT USED */
		unsigned char	page;		/* code page of key */
		unsigned char	point;		/* code point of key */
	}	key;
	struct	{
		unsigned short	pad;		/* NOT USED */
		unsigned short	id;		/* function id */
	}	func;
	struct	{
		unsigned char	pad;		/* NOT USED */
		unsigned char	page;		/* code page of str */
		unsigned short	offset;		/* offset to str */
	}	str;
}	OldKeyMapElt;

/* key.type field */

#define  GRAPHIC   0                    /* graphic character           */
#define  SGL_CTL   1                    /* Single byte control         */
#define  CHAR_STR  4                    /* Character string ( Xlib fix)*/
#define  ES_FUNC   5                    /* ESC function                */
#define  CTLFUNC   6                    /* CTRL function               */
#define  BIND_STR  7                    /* Rebind'd string             */

/* key.stat field */

#define  NORM      0                    /* NORMAL                      */
#define  CURS      3                    /* cursor key                  */
#define  CPFK      4                    /* PF key                      */
#define  DEAD      5                    /* dead key                    */

/* valid code pages */

#define  P0        0x3c
#define  P1        0x3d
#define  P2        0x3e

#define KEYPADDING      509

typedef struct _OldIMKeymapFile	{
	char	magic;			/* magic number */
	unsigned char	max_state;	/* # of states */
	unsigned short	max_pos;	/* # of key positions */
	unsigned short	min_pos;	/* 1st keycode */
	unsigned char	Max_States;	/* Used only if magic = */
					/*  RT256_KEYMAP_MAGIC */
	char	pad[KEYPADDING];	/* padding for future use */
}	OldIMKeymapFile;

/*
 *	declaration of internal functions.
 */
KeyMapElement	*_IMLookupKeymap(IMKeymap *, unsigned int, unsigned int);
IMKeymap	*_IMOpenKeymap(char *);
IMKeymap	*_IMInitializeKeymap(IMLanguage);
void	_IMMapKeysym(IMKeymap *, unsigned int *, unsigned int *);
void	_IMCloseKeymap(IMKeymap *);
char	*_IMAreYouThere(IMLanguage, char *);

#endif	/* _imP_h */
