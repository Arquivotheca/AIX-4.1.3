/* src/bos/diag/tu/370pca/extern.h, tu_370pca, bos411, 9428A410j 6/25/91 10:51:35 */
/*************************************************************************
* The following externals are declared above main in hxeca.c		 *
*************************************************************************/
extern int		device;		/* The fildes for the PSCA board */

#ifdef HTX
extern struct ruleinfo pr;		/* The info associated with the current rule */
extern struct htx_data ps;		/* The line of communication to HTX */
#endif

/*************************************************************************
* These externals are related to Test Units.  Declared in pscatu.c	 *
*************************************************************************/
extern int		max_tu;		/* max TU number, declared in pscatu.c */
extern struct tu_type	tu[];		/* TU functions, declared in pscatu.c */
extern int		ucode;		/* The currently loaded microcode */
extern int		max_errors;	/* The maximum # of errors reported per TU */

#ifdef DIAGS
extern char		msg_buf[];	/* An area to build messages */
#endif
#ifdef BOXMFG	
extern TUTYPE *tucb_ptr;		/* external ref to global test unit control block */
#endif	
#ifdef MTEX
extern char		msg_buf[];	/* An area to build messages */
extern int		dump;		/* file to dump to disk (or 0 if not) */
#endif
#ifdef TUMAIN
extern char		msg_buf[];	/* An area to build messages */
extern FILE		*dump;		/* file to dump to disk (or 0 if not) */
#endif

/*************************************************************************
* These return strings for enumerated status words. Declared in pscatu.c *
*************************************************************************/
extern char		*operlvl_string();	/* operlvl names, declared in pscatu.c */
extern char		*ready_string();	/* ready names, declared in pscatu.c */

/*************************************************************************
* These are from libc and are used here and there			 *
*************************************************************************/
#ifdef AIX
extern char		*sys_errlist[]; /* system defined err msgs, indexed by errno */
extern int		sys_nerr;	/* max value for errno */
extern int		errno;		/* error returned by system call */
extern char		*malloc();	/* memory allocation, libc */
#endif
