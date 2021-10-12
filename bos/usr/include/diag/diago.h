/* @(#)27	1.5  src/bos/usr/include/diag/diago.h, cmddiag, bos41J, 9509A_all 2/21/95 09:15:13 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: DIAG_ITEM_SELECTED
 *		DIAG_NUM_ENTRIES
 *		asl_beep
 *		asl_init
 *		asl_quit
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_DIAGO
#define _H_DIAGO

#include "asl.h"
#ifndef NULL
#define NULL 0
#endif
/*------------ ASL names renamed --------------*/
#define	DIAG_ASL_CANCEL		ASL_CANCEL
#define	DIAG_ASL_HELP		ASL_HELP
#define	DIAG_ASL_REDRAW		ASL_REDRAW
#define	DIAG_ASL_EXIT		ASL_EXIT
#define	DIAG_ASL_LIST		ASL_LIST
#define	DIAG_ASL_DEFAULT	ASL_DEFAULT
#define	DIAG_ASL_COMMAND	ASL_COMMAND
#define	DIAG_ASL_COMMIT		ASL_COMMIT
#define	DIAG_ASL_PRINT		ASL_PRINT
#define	DIAG_ASL_SHELL		ASL_SHELL
#define	DIAG_ASL_EDIT		ASL_EDIT
#define	DIAG_ASL_ENTER		ASL_ENTER
#define	DIAG_ASL_OK		ASL_OK
#define	DIAG_ASL_FAIL		ASL_FAIL

#define	DIAG_ASL_ERR_NO_SUCH_TERM	ASL_ERR_NO_SUCH_TERM
#define	DIAG_ASL_ERR_TERMINFO_GET	ASL_ERR_TERMINFO_GET
#define	DIAG_ASL_ERR_NO_TERM		ASL_ERR_NO_TERM
#define	DIAG_ASL_ERR_INITSCR		ASL_ERR_INITSCR
#define	DIAG_ASL_ERR_SCREEN_SIZE	ASL_ERR_SCREEN_SIZE

#define	DIAG_ASL_RC	ASL_RC

#define	DIAG_ASL_YES	ASL_YES
#define	DIAG_ASL_NO	ASL_NO

#define	DIAG_ASL_SCREEN_CODE	ASL_SCREEN_CODE
#define	DIAG_ASL_SCR_INFO	ASL_SCR_INFO
#define	DIAG_ASL_SCR_TYPE	ASL_SCR_TYPE
#define	DIAG_ASL_LIST_ENTRY	ASL_LIST_ENTRY
/*---------------------------------------------*/

#define DIAG_ASL_ARGS1	55
#define DIAG_ASL_ARGS2	56
#define DIAG_ASL_ARGS3	58
#define DIAG_MALLOCFAILED	67

#define DIAG_MSGONLY	4
#define DIAG_IO		5

#define DIAG_ENDLIST	0

#define DIAG_ITEM_SELECTED(A) ((A).cur_index)

#define DM_TYPE_DEFAULTS \
		{ASL_DIAG_ENTER_SC,0,1}
extern	ASL_SCR_TYPE dm_menutype;

struct msglist {
	unsigned short setid;
	unsigned short msgid;
};
#define	DIAG_NUM_ENTRIES(MSGLIST)	(sizeof((MSGLIST))/sizeof((MSGLIST)[0]))
#ifndef asl_init
#define asl_init(a) diag_asl_init("default")
#endif

#ifndef asl_beep
#define asl_beep() diag_asl_beep()
#endif

#ifndef asl_quit
#define asl_quit(a) diag_asl_quit("default")
#endif

#ifndef asl_msg
#define asl_msg diag_asl_msg
#endif

#define ASL_INIT_DEFAULT	11


/*--Screen Codes for Generic Menus (diag_display_menu)--*/
#define CUSTOMER_TESTING_MENU 1
#define ADVANCED_TESTING_MENU 2
#define LOOPMODE_TESTING_MENU 3
#define NO_MICROCODE_MENU 4
#define NO_DIAGMICROCODE_MENU 5
#define NO_DDFILE_MENU 6
#define NO_HOT_KEY 7
#define	DEVICE_INITIAL_STATE_FAILURE 8

#endif
