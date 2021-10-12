/* @(#)49	1.8.1.2  src/bos/kernel/proc/exec.h, sysproc, bos411, 9428A410j 4/1/93 13:40:29 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: XA_ARGSIZE
 *		XA_ENVSIZE
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* user stack area sizes */
#define	XA_ARGSIZE(xp)	(NBPW*((xp)->na+1))
#define	XA_ENVSIZE(xp)	(NBPW*((xp)->ne+1))

#define INTERPRET    256

/*
 * exec argument structure
 * (used to pass argument information between internal subroutines)
 */
struct xargs {
	char	*fname;			/* name of file to exec */
	char	**argp;			/* array of ptrs to arguments */
	char	**envp;			/* array of ptrs to env variables */
	char	*libpath;		/* address of copy of LIBPATH environment value */
	int 	indir;			/* TRUE if indirect shell file */
	int 	loaderror;		/* TRUE if execload failed once */
	int	(*ep)();		/* entry point function pointer */
	struct top_of_stack *ucp;	/* user stack context pointer */
	char	*buf;			/* argument buffer address */
	char	*ebuf;			/* loader error messages */
	caddr_t	ap;			/* user argument area pointer */
	int	nc;			/* number of characters in strings */
	int	na;			/* number of arg strings */
	int	ne;			/* number of env strings */
	char	cfname[MAXCOMLEN+1];	/* base file name */
	char	cfarg[INTERPRET];	/* indirect file argument */
};


/* current exec file information */
union execunionX {			/* file header union */
	struct xcoffhdr u_xcoffhdr;	/* xcoff header */
	char u_exshell[INTERPRET]; 	/* #! + name of interpreter */
};
