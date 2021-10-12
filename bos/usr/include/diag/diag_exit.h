/* @(#)26	1.3  src/bos/usr/include/diag/diag_exit.h, cmddiag, bos411, 9428A410j 12/8/92 08:57:25 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: DA_CHECKRC_ERROR
 *		DA_CHECKRC_MORE
 *		DA_CHECKRC_STATUS
 *		DA_CHECKRC_TESTS
 *		DA_CHECKRC_USER
 *		DA_EXIT
 *		DA_SETRC_ERROR
 *		DA_SETRC_MORE
 *		DA_SETRC_STATUS
 *		DA_SETRC_TESTS
 *		DA_SETRC_USER
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_DA_EXIT
#define _H_DA_EXIT

#define	DA_SETRC_STATUS(VAL)	da_exit_code.status = (VAL)
#define	DA_SETRC_USER(VAL)	da_exit_code.user = (VAL)
#define	DA_SETRC_ERROR(VAL)	da_exit_code.error = (VAL)
#define	DA_SETRC_TESTS(VAL)	da_exit_code.tests = (VAL)
#define	DA_SETRC_MORE(VAL)	da_exit_code.more = (VAL)

#define	DA_CHECKRC_STATUS()	da_exit_code.status
#define	DA_CHECKRC_USER()	da_exit_code.user
#define	DA_CHECKRC_ERROR()	da_exit_code.error
#define	DA_CHECKRC_TESTS()	da_exit_code.tests
#define	DA_CHECKRC_MORE()	da_exit_code.more

#define	DA_EXIT()	exit( *( (char*)(&da_exit_code) )  )

enum diag_enum_status
{
	DA_STATUS_GOOD,
	DA_STATUS_BAD
};
enum diag_enum_user {
	DA_USER_NOKEY,
	DA_USER_EXIT,
	DA_USER_QUIT
};
enum diag_enum_error
{
	DA_ERROR_NONE,
	DA_ERROR_OPEN,
	DA_ERROR_OTHER
};
enum diag_enum_tests
{
	DA_TEST_NOTEST,
	DA_TEST_FULL,
	DA_TEST_SUB,
	DA_TEST_SHR
};
enum diag_enum_more
{
	DA_MORE_NOCONT,
	DA_MORE_CONT
};

typedef struct {
		unsigned status : 1;	/* enum diag_enum_status */
		unsigned user   : 2;	/* enum diag_enum_user */
		unsigned error  : 2;	/* enum diag_enum_error */
		unsigned tests  : 2;	/* enum diag_enum_tests */
		unsigned more   : 1;	/* enum diag_enum_more */
} da_returncode_t;

extern	da_returncode_t da_exit_code;

#endif
