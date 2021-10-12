#if !defined(AIX3_2) && !defined(hpux)
#ident "@(#)log.h:3.4  File Date:93/03/25 Last Delta:92/11/22 00:06:31"
#endif
/*=============================================================================
 *(c) Copyright 1988, 1990, 1991 Hewlett-Packard Co.  Unpublished Work.
 * All Rights Reserved.
 *
 *                         RESTRICTED RIGHTS LEGEND
 *
 * Use, duplication or disclosure by the U.S. Government is subject to
 * restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
 * Technical Data and Computer Software clause at DFARS 252.227-7013 for DOD
 * agencies, and subparagraphs (c)(1) and (c)(2) of the Commercial Computer
 * Software Restricted Rights clause at FAR 52.227-19 for other agencies.
 *   
 *                           HEWLETT-PACKARD COMPANY
 *                             3000 Hanover Street
 *                     Palo Alto, California 94304 U.S.A.
 *
 *=============================================================================
 */
/* ==========================================================================
 * Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK LICENSE SERVER) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ==========================================================================
 * log.h
 *
 *    structs for the logging file 
 *
 *    09/10/87    shj      put typedefs in log.idl file
 *    08/18/87    molson   added log_db_change structures
 *    06/26/87    shj      orig coding
 */


/* version of log file */
#define LOG_VRSN -1005
     

/* user stipulation of what goes into log file */
#define LOG_LIC_BIT     0x0001       /* license grant/release */
#define LOG_CHK_BIT     0x0002       /* license check */
#define LOG_WQU_BIT     0x0004       /* wait queue */
#define LOG_VND_BIT     0x0008       /* vendor/db */
#define LOG_PRD_BIT     0x0010       /* product/db */
#define LOG_ERR_BIT     0x0020       /* error */
#define LOG_TIO_BIT     0x0040       /* license timed out */
#define LOG_MSG_BIT     0x0080       /* message */
#define LOG_SVR_BIT     0x0100       /* server start/exit */

/* command line letters for above events */
#define log_stip_events "lcwvpetms"
static short log_bits_array[] = {
    LOG_LIC_BIT,  
    LOG_CHK_BIT,  
    LOG_WQU_BIT,  
    LOG_VND_BIT,  
    LOG_PRD_BIT, 
    LOG_ERR_BIT,  
    LOG_TIO_BIT,  
    LOG_MSG_BIT,  
    LOG_SVR_BIT
};
                 
/* License type names */
/*
static char *license_types[] = {
    "ConcurrentAccess",
    "NodeLocked",
    "UseOnce",
    "Compound",
    "UsageMetering",
    "",
    "",
};
*/

/* ACTIONS for LS */
#define LIC_GRNT       1
#define LIC_RLS        2
#define LIC_CHK        3
#define WAITING        4
#define WAIT_GNT       5
#define WAIT_RMV       6
#define LIC_GRNT_MULTI 7
#define LIC_RLS_MULTI  8
#define LIC_USE_ONCE   9
#define LIC_USE_COMP   10	/* Reserved 10-19 for possible derived types */

/*
static char *action_name[] = {
    " ",
    "G R A N T E D",
    "R E L E A S E D",
    "C H E C K E D",
    "W A I T I N G",
    "G R A N T / W A I T",
    "W A I T   R E M O V E D",
    "M U L T I P L E   G R A N T",
    "M U L T I P L E   R E L E A S E",
    "U S E _ O N C E   U S E D",
    "C O M P O U N D   U S E D",
};
*/
                               
/* ACTIONS for DB */
#define VEND_ADD 1
#define VEND_DEL 2
#define VEND_REN 3
#define PROD_ADD 4
#define PROD_DEL 5
#define PROD_REN 6

/*
static char *db_action_name[] = {
    " ",
    "V E N D O R  A D D E D",
    "V E N D O R  D E L E T E D",
    "V E N D O R  R E N A M E D",
    "P R O D U C T  A D D E D",
    "P R O D U C T  D E L E T E D",
    "P R O D U C T  R E N A M E D"
};
*/

/*********************************************/
/***************** PROTOTYPES ****************/
/*********************************************/

long init_log();

void close_log();

void cleanup_log(
#if defined(__STDC__) && !defined(_NO_PROTO)
long begin_date,
long *status
#endif
);

void log_action(
#if defined(__STDC__) && !defined(_NO_PROTO)
uuid__t *vid,
long prod_id,
char *prod_name,
char u_id[32], 
char n_id[32], 
char g_id[32],
char vrsn[ls_VLEN],
long amt,                             
trans_id_t trans_id,                        
char mach_type[32],                          
ls_job_id_t *job_id,
char multi_use_flag,
long action
#endif
);

void log_db_change(
#ifdef __STDC__
uuid__t *vid,
long    action,
char    *a1,
char    *a2,
char    *a3,
char    *a4,
char    *a5,
char    *a6,
char    *a7,
char    *a8
#endif
);

void log_error(
#ifdef __STDC__
ls_job_id_t *job_id,
uuid__t *vid,
long prod_id,
char *prod_name,
char u_id[32], 
char n_id[32], 
char g_id[32],
char vrsn[ls_VLEN],
long amt,
long status
#endif
);

void log_fatal_error(
#ifdef __STDC__
char *msg
#endif
);

void log_lic_to(
#ifdef __STDC__
uuid__t *vid,
long prod_id,           
char *prod_name,
char u_id[32], 
char n_id[32], 
char g_id[32],
char vrsn[ls_VLEN],
long amt,           
trans_id_t trans_id,            
ls_job_id_t *job_id
#endif
);

void log_msg(
#ifdef __STDC__
char *vname,
char *msg
#endif
);

void log_svr_crsh();

void log_svr_start();
             


