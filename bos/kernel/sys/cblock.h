/* @(#)24	1.16  src/bos/kernel/sys/cblock.h, sysios, bos411, 9428A410j 10/25/91 09:09:32 */
#ifndef _H_CBLOCK
#define _H_CBLOCK
/*
 * COMPONENT_NAME: (SYSIOS) Character I/O header file
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
   A clist structure is the head of a character list.
   The getc* and putc* services manipulate these structures.
*/
 
struct clist
{
   int		   c_cc;	       /* character count	      */
   struct cblock   *c_cf;	       /* pointer to first	      */
   struct cblock   *c_cl;	       /* pointer to last	      */
};
 
/*
   The structure of a character list buffer.
*/
 
#define CLSIZE	64
 
struct cblock
{
   struct cblock   *c_next;	       /* pointer to next cblock      */
   char 	   c_first;	       /* offset of first character   */
   char 	   c_last;	       /* offset of next character    */
   char 	   c_data[CLSIZE];     /* data (characters)	      */
   char 	   c_flags;	       /* sanity checking */
};
 
extern	struct chead {
   struct cblock   *c_next;	       /* head of cblock free list    */
   int		   c_size;	       /* set to CLSIZE 	      */
   int		   c_flag;	       /* 0 = no free cblock waiters  */
};

#ifdef _KERNEL			/* don't break the build of the libs.	*/

#ifndef _NO_PROTO

extern
int getc(struct clist *header);  /* get character from front of clist */
/* arguments:
 *	register struct	clist	*header		ptr to the character list
 */

extern
int putc(char c, struct clist *p);   /* put character at end of clist */
/* arguments:
 *	register	char	c		character to add to list
 *	register struct	clist	*header		ptr to the character list
 */

extern 
struct cblock *getcf();	/* get a free character buffer		*/
/* arguments:
 *	None
 */

extern 
void putcf(struct cblock *p);		/* frees a character buffer		*/
/* arguments:
 *	register struct	cblock	*p		ptr to character buf to free
 */

extern 
void putcfl(struct clist *header);	/* frees a list of character buffers */
/* arguments:
 *	register struct	clist	*header		list of character bufs to free
 */

extern 
struct cblock *getcb(struct clist *header); /* get char buffer at front of char list*/
/* arguments:
 *	register struct	clist	*header		ptr to character list
 */

/*
 *  put char buffer at end of char list	
 */
extern 
void putcb(struct cblock *p, struct clist *header);
/* arguments:
 *	register struct	cblock	*p		ptr to character buf to free
 */

/* 
 * get n chars from front of char list
 */
extern 
int getcbp(struct clist *header, char *dest, int n);
/* arguments:
 *	register struct	clist	*header		ptr to character list
 *	register 	char	*dest		addr where to place chars
 *	register	int	n		# chars to read from char list
 */

/* 
 * put n chars at end of char list
 */
extern 
int putcbp(struct clist *p, char *source, int n);
/* arguments:
 *	register struct	cblock	*p		ptr to character buf to free
 *	register 	char	*source		addr where to get chars from
 *	register	int	n		# chars to read from char list
 */
extern 
int getcx(struct clist *header); /* get a character from the end of char list */
/* arguments:
 *	register struct	clist	*header		ptr to the character list
 */

/* 
 * put a character at front of the char list
 */
extern
int putcx(char c, struct clist *p);
/* arguments:
 *	register	char	c		character to add to list
 *	register struct	clist	*header		ptr to the character list
 */

extern 
int pincf(int delta);  /* manage the list of free character buffers */
/* arguments:
 *	register	int	delta		# of char buffers to increase/
 *						decrease free list by
 */

extern 
int waitcfree();	/* check the availability of a free char buffer	*/
/* arguments:
 *	None
 */

#else

extern int getc(); 		/* get character from front of clist	*/
/* arguments:
 *	register struct	clist	*header		ptr to the character list
 */

extern int putc();   /* put character at end of clist */
/* arguments:
 *	register	char	c		character to add to list
 *	register struct	clist	*header		ptr to the character list
 */

extern struct cblock *getcf();	/* get a free character buffer		*/
/* arguments:
 *	None
 */

extern void putcf();		/* frees a character buffer		*/
/* arguments:
 *	register struct	cblock	*p		ptr to character buf to free
 */

extern void putcfl();		/* frees a list of character buffers	*/
/* arguments:
 *	register struct	clist	*header		list of character bufs to free
 */

extern struct cblock *getcb();	/* get char buffer at front of char list*/
/* arguments:
 *	register struct	clist	*header		ptr to character list
 */

extern void putcb();		/* put char buffer at end of char list	*/
/* arguments:
 *	register struct	cblock	*p		ptr to character buf to free
 */

extern int getcbp();		/* get n chars from front of char list	*/
/* arguments:
 *	register struct	clist	*header		ptr to character list
 *	register 	char	*dest		addr where to place chars
 *	register	int	n		# chars to read from char list
 */

extern int putcbp();		/* put n chars at end of char list	*/
/* arguments:
 *	register struct	cblock	*p		ptr to character buf to free
 *	register 	char	*source		addr where to get chars from
 *	register	int	n		# chars to read from char list
 */

extern int getcx();	/* get a character from the end of char list	*/
/* arguments:
 *	register struct	clist	*header		ptr to the character list
 */

extern int putcx();	/* put a character at front of the char list	*/
/* arguments:
 *	register	char	c		character to add to list
 *	register struct	clist	*header		ptr to the character list
 */

extern int pincf();	/* manage the list of free character buffers	*/
/* arguments:
 *	register	int	delta		# of char buffers to increase/
 *						decrease free list by
 */

extern int waitcfree();	/* check the availability of a free char buffer	*/
/* arguments:
 *	None
 */

#endif /* not _NO_PROTO */

#endif	/* _KERNEL */
 
#endif /* _H_CBLOCK */
