/* "@(#)89	1.2  src/bos/sbin/helpers/v3fshelpers/fshargs.h, cmdfs, bos411, 9428A410j 6/15/90 21:20:20" */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* fshargs.h: common header file for helperized ff */
/* 
 * handy functions to print out arguments.
 */

/* for string values in printf's.  returns the string or "(null)" */
#define PR_NULL(val)	((val)==NULL?"(null)":(val))

/* print a boolean value */
#define	PR_BOOL(code,val)	fprintf(stderr,"%s = %s, ", code,val == False ? "FALSE":"TRUE")

/* print a string value */
#define	PR_STR(code,val)	fprintf(stderr,"%s = \"%s\", ", code, \
					PR_NULL(val))
/* print an integer value */
#define	PR_INT(code,val)	fprintf(stderr,"%s = %d, ", code, val)

/* print elements of an integer vector */
#define	PR_INTV(code,vec)\
	if ((vec) != NULL)\
	{\
	    int el; \
	    fprintf(stderr,"%s[%d] = { ",code,\
		((struct fsh_argvector*)(vec))->iv_stride);\
	    for (el=0; el < ((struct fsh_argvector*)(vec))->iv_stride; el++) \
		 fprintf(stderr,"%d ",((struct fsh_argvector*)(vec))->iv_int[el]);\
	    fprintf(stderr," } ");\
	} else fprintf(stderr,"%s = { null vector } \n",code);

/* print elements of a boolean vector */
#define	PR_BOOLV(code,vec)\
	if ((vec) != NULL)\
	{\
	    int el; \
	    fprintf(stderr,"%s[%d] = { ",code,\
		((struct fsh_argvector*)(vec))->iv_stride);\
	    for (el=0; el < ((struct fsh_argvector*)(vec))->iv_stride; el++) \
		fprintf(stderr,"%s ",\
		    ((struct fsh_argvector*)(vec))->iv_bool[el]==FALSE?\
		    "FALSE":"TRUE");\
	    fprintf(stderr," } ");\
	} else fprintf(stderr,"%s = { null vector } \n",code);

/* print elements of a string vector */
#define	PR_STRV(code,vec)\
	if ((vec) != NULL)\
	{\
	    int el; \
	    fprintf(stderr,"%s[%d] = { ",code,\
		((struct fsh_argvector*)(vec))->iv_stride);\
	    for (el=0; el < ((struct fsh_argvector*)(vec))->iv_stride; el++) \
		fprintf(stderr,"\"%s\" ", \
			PR_NULL(((struct fsh_argvector*)(vec))->iv_str[el]));\
	    fprintf(stderr," } ");\
	} else fprintf(stderr,"%s = { null vector } \n",code);

/* EOF fshargs.h */
