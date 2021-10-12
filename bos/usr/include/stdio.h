/* @(#)77	1.46.1.10  src/bos/usr/include/stdio.h, libcio, bos411, 9428A410j 6/8/94 18:04:17 */
#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifndef _H_STDIO
#define _H_STDIO

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *
 *      The ANSI and POSIX standards require that certain values be in stdio.h.
 *      It also requires that if _ANSI_C_SOURCE or _POSIX_SOURCE is defined
 *	then ONLY those values are present. This header includes all the ANSI 
 *	and POSIX required entries.
 *      In addition other entries for the AIX system are included.
 *
 */

#ifdef _ANSI_C_SOURCE

/*
 * 	The following definitions are included in <sys/types.h>.  They
 *	are also included here to comply with ANSI standards.
 */

#ifndef NULL
#define NULL	0
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long	size_t;
#endif

#ifndef _FPOS_T
#define	_FPOS_T
typedef long	fpos_t;
#endif

/*
 *	The definition of TMP_MAX is included in <sys/limits.h>.  It is
 *	also defined here to comply with ANSI standards.
 */

#ifndef TMP_MAX
#define	TMP_MAX		16384
#endif


#define FOPEN_MAX 	2000
#define FILENAME_MAX 	255
#define BUFSIZ		4096
#define _P_tmpdir       "/tmp/"
#define L_tmpnam	(sizeof(_P_tmpdir) + 15)

/*
 * _IOLBF means that a file's output will be buffered line by line
 * In addition to being flags, _IONBF, _IOLBF and _IOFBF are possible
 * values for "type" in setvbuf.
 */
#define _IOFBF		0000
#define _IOLBF		0100
#define _IONBF		0004


#ifndef EOF
#define EOF		(-1)
#endif

#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

typedef struct {
	unsigned char	*_ptr;
	int	_cnt;
	unsigned char	*_base;
	unsigned char   *_bufendp;
	short	_flag;
	short	_file;
	int	__stdioid;
	char	*__newbase;
#ifdef _THREAD_SAFE
	void *_lock;
#else
	long	_unused[1];
#endif
} FILE;

#define _IOEOF		0020
#define _IOERR		0040
#define _NIOBRW		16
extern FILE	_iob[_NIOBRW];

#define stdin		(&_iob[0])
#define stdout		(&_iob[1])
#define stderr		(&_iob[2])

#ifdef	_NONSTD_TYPES
extern int 	fread();
extern int	fwrite();
#elif	defined	_NO_PROTO
extern size_t 	fread();
extern size_t	fwrite();
#else	/* _NONSTD_TYPES, _NO_PROTO */
extern size_t 	fread(void *, size_t, size_t, FILE *);
extern size_t	fwrite(const void *, size_t, size_t,FILE *);
#endif	/* _NONSTD_TYPES, _NO_PROTO */

#ifdef _NO_PROTO
extern int	__filbuf();
extern int	__flsbuf();
extern int	ferror();
extern int	feof();
extern void	clearerr();
extern int	putchar();
extern int	getchar();
extern int	putc();
extern int	getc();
extern int	remove();
extern int	rename();
extern FILE 	*tmpfile();
extern char 	*tmpnam();
extern int 	fclose();
extern int 	fflush();
extern FILE	*fopen();
extern FILE 	*freopen();
extern void 	setbuf();
extern int 	setvbuf();
extern int	fprintf(); 
extern int	fscanf();
extern int	printf();
extern int	scanf();
extern int	sprintf(); 
extern int	sscanf(); 
extern int	vfprintf();
extern int	vprintf(); 
extern int	vsprintf();
extern int 	fgetc();
extern char 	*fgets();
extern int 	fputc();
extern int 	fputs();
extern char 	*gets();
extern int 	puts();
extern int	ungetc();
extern int	fgetpos();
extern int 	fseek();
extern int	fsetpos();
extern long	ftell();
extern void	rewind();
extern void 	perror(); 
#ifdef _THREAD_SAFE
	extern void flockfile();
	extern void funlockfile();
#endif

#else			/* use ANSI C required prototypes */

extern int	__flsbuf(unsigned char, FILE *);
extern int	__filbuf(FILE *);
extern int 	ferror(FILE *);
extern int 	feof(FILE *);
extern void 	clearerr(FILE *);
extern int 	putchar(int);
extern int 	getchar(void);
extern int 	putc(int, FILE *);
extern int 	getc(FILE *);
extern int	remove(const char *);
extern int	rename(const char *, const char *);
extern FILE 	*tmpfile(void);
extern char 	*tmpnam(char *);
extern int 	fclose(FILE *);
extern int 	fflush(FILE *);
extern FILE	*fopen(const char *, const char *);
extern FILE 	*freopen(const char *, const char *, FILE *);
extern void 	setbuf(FILE *, char *);
extern int 	setvbuf(FILE *, char *, int, size_t);
extern int	fprintf(FILE *, const char *, ...); 
extern int	fscanf(FILE *, const char *, ...);
extern int	printf(const char *, ...); 
extern int	scanf(const char *, ...); 
extern int	sprintf(char *, const char *, ...); 
extern int	sscanf(const char *, const char *, ...); 

#ifdef _VA_LIST
extern int	vfprintf(FILE *, const char *, va_list);
extern int	vprintf(const char *, va_list); 
extern int	vsprintf(char *, const char *, va_list);
#else
#define _HIDDEN_VA_LIST         /* define a type not in the namespace */
#include <va_list.h>
extern int	vfprintf(FILE *, const char *, __va_list);
extern int	vprintf(const char *, __va_list); 
extern int	vsprintf(char *, const char *, __va_list);
#endif /* _VA_LIST */

extern int 	fgetc(FILE *);
extern char 	*fgets(char *, int, FILE *);
extern int 	fputc(int, FILE *);
extern int 	fputs(const char *, FILE *);
extern char 	*gets(char *);
extern int 	puts(const char *);
extern int	ungetc(int, FILE *);
extern int	fgetpos(FILE *, fpos_t *);
extern int 	fseek(FILE *, long int, int);
extern int	fsetpos(FILE *, const fpos_t *);
extern long	ftell(FILE *);
extern void	rewind(FILE *);
extern void 	perror(const char *); 
#ifdef _THREAD_SAFE
	extern void flockfile(FILE *stream);
	extern void funlockfile(FILE *stream);
#endif

#endif /* _NO_PROTO */

#ifdef _THREAD_SAFE
/*
 * The default for getc and putc are locked for compatibility with 
 * Posix P1003.4a
 * By defining _STDIO_UNLOCK_CHAR_IO before including this
 * file, the default action is changed to unlocked putc and getc.
 * A file lock can still be placed around a block of putc's or getc's
 * regardless of the locking mode, and invoking the locked or
 * unlocked version directly always overrides the default action.
 */

#define getc_unlocked(p)	(--(p)->_cnt < 0 ? __filbuf(p) : (int) *(p)->_ptr++)
#define getchar_unlocked()	getc_unlocked(stdin)
#define getc_locked(p)		fgetc(p)
#define getchar_locked()	getc_locked(stdin)
#define putc_unlocked(x, p)	(--(p)->_cnt < 0 ? \
				__flsbuf((unsigned char) (x), (p)) : \
				(int) (*(p)->_ptr++ = (unsigned char) (x)))
#define putchar_unlocked(x)	putc_unlocked(x,stdout)
#define putc_locked(x, p)	fputc(x, p)
#define putchar_locked(x)	putc_locked(x,stdout)

#ifndef _STDIO_UNLOCK_CHAR_IO
#define getc(p)			getc_locked(p)
#define putc(x, p)		putc_locked(x, p)
#else	/* _STDIO_UNLOCK_CHAR_IO */
#define getc(p)			getc_unlocked(p)
#define putc(x, p)		putc_unlocked(x, p)

/*
 * if _STDIO_UNLOCK_CHAR_IO is not defined, these macros will not be defined
 * and become functions.
 */
#define clearerr(p)		((void) ((p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof(p)			((p)->_flag & _IOEOF)
#define ferror(p)		((p)->_flag & _IOERR)
#endif /* _STDIO_UNLOCK_CHAR_IO */

#define clearerr_unlocked(p)	((void) ((p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof_unlocked(p)	((p)->_flag & _IOEOF)
#define ferror_unlocked(p)	((p)->_flag & _IOERR)
#define fileno_unlocked(p)	((p)->_file)

#else /* Not _THREAD_SAFE */

#define getc(__p)		(--(__p)->_cnt < 0 ? __filbuf(__p) : (int) *(__p)->_ptr++)
#define putc(__x, __p)	(--(__p)->_cnt < 0 ? \
			__flsbuf((unsigned char) (__x), (__p)) : \
			(int) (*(__p)->_ptr++ = (unsigned char) (__x)))
#define clearerr(__p)	((void) ((__p)->_flag &= ~(_IOERR | _IOEOF)))
#define feof(__p)		((__p)->_flag & _IOEOF)
#define ferror(__p)	((__p)->_flag & _IOERR)
#endif /* _THREAD_SAFE */

#define getchar()	getc(stdin)
#define putchar(__x)	putc((__x), stdout)

#endif /*_ANSI_C_SOURCE */


#ifdef _POSIX_SOURCE
/*
 *   The following are values that have historically been in stdio.h.
 *
 *   They are a part of the POSIX defined stdio.h and therefore are
 *   included when _POSIX_SOURCE and _XOPEN_SOURCE are defined.
 *
 */

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#define L_ctermid	9
#define L_cuserid	9

#ifdef _NO_PROTO
extern int 	fileno();
extern FILE 	*fdopen();
extern char	*ctermid();
#else
extern int 	fileno(FILE *);
extern FILE 	*fdopen(int,const char *);
extern char	*ctermid(char *);
#endif /* _NO_PROTO */

#if !defined(_THREAD_SAFE) || !defined(_STDIO_UNLOCK_CHAR_IO)
#define fileno(__p)	((__p)->_file)     
#endif

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE

#include <va_list.h>	/* va_list must be define in XPG4 */

extern	char	*optarg;
extern	int	opterr;
extern	int	optind;
extern	int	optopt;

#define P_tmpdir	_P_tmpdir

#ifdef _NO_PROTO
extern int 	getw();
extern int 	putw();
extern char 	*tempnam();
extern FILE 	*popen();
extern int 	pclose();
extern int	getopt();
extern char 	*cuserid();
#else
extern int 	getw(FILE *);
extern int 	putw(int, FILE *);
extern char 	*tempnam(const char*, const char*);
extern FILE 	*popen(const char *, const char *);
extern int 	pclose(FILE *);
extern int	getopt(int, char * const [], const char*);
extern char 	*cuserid(char *);

#endif /* _NO_PROTO */
#endif /*_XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#ifndef _H_LIMITS
#include <sys/limits.h>   /* limits.h not allowed by Posix.1a.  Must be in _ALL_SOURCE */
#endif

#ifdef _NO_PROTO
extern void setbuffer();
extern void setlinebuf();
#else /* _NO_PROTO */
extern void setbuffer(FILE *, char *, size_t);
extern void setlinebuf(FILE *);
#endif /* _NO_PROTO */

#ifndef WEOF
#define WEOF		(-1)
#endif

#ifdef OPEN_MAX
#define _NFILE		OPEN_MAX
#else
#define _NFILE		2000
#endif
			/* For Dynamic iob's. Make sure _NFILE is       */
  			/* a multiple of _NIOBRW (IOB Row).   		*/

#define _NRWS		(_NFILE / _NIOBRW )+1
			/* Number of iob rows should be equal 		*/
			/* to (_NFILE / _NIOBRW ) + 1.  The extra 	*/
			/* row ensures insures that the _NFILE+1th 	*/
			/* fopen fails because of open(). 		*/

/* buffer size for multi-character output to unbuffered files */
#define _SBFSIZ 8

#define _IOREAD		0001
#define _IOWRT		0002
#define _IOMYBUF	0010
#define _IORW		0200
#define	_IONOFD		0400
#define	_IOUNGETC	01000
#define	_IOCLOSE	02000
#ifdef _THREAD_SAFE
#define _IOINUSE	02000
#endif /* _THREAD_SAFE */
#define	_IONONSTD	04000
#define _IOISTTY        010000

#define _bufend(__p)	((__p)->_bufendp)
#define _bufsiz(__p)	(_bufend(__p) - (__p)->_base)

#endif /* _ALL_SOURCE */
#endif /* _H_STDIO */

