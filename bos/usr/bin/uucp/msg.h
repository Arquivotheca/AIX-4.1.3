/* @(#)96	1.2  src/bos/usr/bin/uucp/msg.h, cmduucp, bos411, 9428A410j 6/16/90 00:00:03 */
/* 
 * COMPONENT_NAME: UUCP msg.h
 * 
 * FUNCTIONS: M_BADFRAME, M_DEAD, M_DOWN, M_DRAINO, M_ENDOFLINE, 
 *            M_INITa, M_INITb, M_LIVE, M_PDEBUG, M_RCLOSE, M_RREJ, 
 *            M_RXMIT, M_Undel_1, M_Undel_2, M_Undel_3, M_Undel_4, 
 *            M_Undel_5, M_Undel_6, M_Undel_7, M_WAITO, M_WR_1, 
 *            M_WR_2, M_WR_3, M_WR_4 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*  The following messages definitions were originally in uucpdefs.c */

#define	 Ct_OPEN  	MSGSTR(MSG_UDEFS_1, "CAN'T OPEN")
#define	 Ct_WRITE  	MSGSTR(MSG_UDEFS_2, "CAN'T WRITE")
#define	 Ct_READ  	MSGSTR(MSG_UDEFS_3, "CAN'T READ")
#define	 Ct_CREATE  	MSGSTR(MSG_UDEFS_4, "CAN'T CREATE")
#define	 Ct_ALLOCATE 	MSGSTR(MSG_UDEFS_5, "CAN'T ALLOCATE")
#define	 Ct_LOCK  	MSGSTR(MSG_UDEFS_6, "CAN'T LOCK")
#define	 Ct_STAT  	MSGSTR(MSG_UDEFS_7, "CAN'T STAT")
#define	 Ct_CHOWN  	MSGSTR(MSG_UDEFS_8, "CAN'T CHOWN")
#define	 Ct_CHMOD  	MSGSTR(MSG_UDEFS_9, "CAN'T CHMOD")
#define	 Ct_LINK  	MSGSTR(MSG_UDEFS_10, "CAN'T LINK")
#define	 Ct_CHDIR  	MSGSTR(MSG_UDEFS_11, "CAN'T CHDIR")
#define	 Ct_UNLINK 	MSGSTR(MSG_UDEFS_12, "CAN'T UNLINK")
#define	 Wr_ROLE  	MSGSTR(MSG_UDEFS_13, "WRONG ROLE")
#define	 Ct_CORRUPT	MSGSTR(MSG_UDEFS_14, "CAN'T MOVE TO CORRUPTDIR")
#define	 Ct_CLOSE  	MSGSTR(MSG_UDEFS_15, "CAN'T CLOSE")
#define	 Ct_FORK  	MSGSTR(MSG_UDEFS_16, "CAN'T FORK")
#define	 Fl_EXISTS 	MSGSTR(MSG_UDEFS_17, "FILE EXISTS")


/*  The following messages definitions were originally in pk1.c */

#define  M_DEAD()	MSGSTR(MSG_PK1_1, "Dead!")
#define  M_INITa()	MSGSTR(MSG_PK1_2, "INIT code a")
#define  M_INITb()	MSGSTR(MSG_PK1_3, "INIT code b")
#define  M_LIVE()	MSGSTR(MSG_PK1_4, "O.K.")
#define  M_RXMIT()	MSGSTR(MSG_PK1_5, "Rcv/Xmit")
#define  M_RREJ()	MSGSTR(MSG_PK1_6, "RREJ?")
#define  M_PDEBUG()	MSGSTR(MSG_PK1_7, "PDEBUG?")
#define  M_DRAINO()	MSGSTR(MSG_PK1_8, "Draino...")
#define  M_WAITO()	MSGSTR(MSG_PK1_9, "Waiting")
#define  M_DOWN()	MSGSTR(MSG_PK1_10, "Link down")
#define  M_RCLOSE()	MSGSTR(MSG_PK1_11, "RCLOSE?")
#define  M_BADFRAME()	MSGSTR(MSG_PK1_12, "Bad frame")
#define  M_ENDOFLINE()	MSGSTR(MSG_PK1_13, "End of the line")

/*  The following messages definitions were originally in uucleanup.c */

#define M_Undel_1() MSGSTR(MSG_UCL_2, "Subject: Undeliverable Mail\n")
#define M_Undel_2() MSGSTR(MSG_UCL_3, "This mail message is undeliverable.\n")
#define M_Undel_3() MSGSTR(MSG_UCL_4, "(Probably to or from system '%s')\n")
#define M_Undel_4() MSGSTR(MSG_UCL_5, "It was sent to you or by you.\n")
#define M_Undel_5() MSGSTR(MSG_UCL_6, "Sorry for the inconvenience.\n")
#define M_Undel_6() MSGSTR(MSG_UCL_25, "Subject: Job Killed By uucp\n")
#define M_Undel_7() MSGSTR(MSG_UCL_26,"We can't contact machine '%s'.\n")
#define M_WR_1()    MSGSTR(MSG_UCL_7, "Subject: Warning From uucp\n")
#define M_WR_2()    MSGSTR(MSG_UCL_8, "We have been unable to contact machine '%s' since you queued your job.\n")
#define M_WR_3()    MSGSTR(MSG_UCL_9, "The job will be deleted in several days if the problem is not corrected.\n")
#define M_WR_4()    MSGSTR(MSG_UCL_10, "If you care to kill the job, execute the following command:\n")

