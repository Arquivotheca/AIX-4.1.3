/*  @(#)38     1.5  src/bos/diag/tu/bad/badatu.h, tu_bad, bos411, 9428A410j 6/11/91 15:06:20 */
/*
 * COMPONENT_NAME: tu_bad - Direct Bus Attached header file.
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* #include <atu.h> */
typedef struct tucb_t {
        long tu,mfg,loop;
        long r1,r2;
};
/******************************************************************************/
/* tu is the test unit number.                                                */
/* mfg is manufacturing mode.                                                 */
/* loop is number of times to repeat the test unit. If an error occurs        */
/* the application test unit exits.                                           */
/* r1 and r2 is reserved.                                                     */
/******************************************************************************/
#define BAD_TUTYPE bad_tucb_t
typedef struct bad_rd_verify_t {
        unsigned int soft_error;
        unsigned int seek_error;
        unsigned int eqp_check_error;
};
typedef struct bad_tucb_t {
        struct tucb_t tucb;
        long ip1,ip2;
        struct devinfo bad_devinfo;
        struct badisk_dstat_blk bad_diag_stat;
        struct bad_rd_verify_t bad_rd_verify_stat;
};
/******************************************************************************/
/* ip1,ip2 is required for buffer write read compare test                     */
/******************************************************************************/
#define SELF_TEST_TIME     1000*3
#define DIAG_SELF_TEST     0
#define WR_BUF_TIME        1000*7
#define RD_BUF_TIME        1000*7
#define BUFF_WR_CMP_TEST   1
#define RW_TIME            1000*25
#define DIAG_RW_TEST       2
#define SEEK_TIME          1000*45
#define DIAG_SEEK_TEST     3
#define RD_VERIFY_TIME     1000*200
#define DIAG_RD_VERIFY_TEST 4
#define BUFF_SIZE          64*512
#define BAD_INFO_TEST      5
#define MFG_RD_VERIFY_TEST 6
#define MFG_RD_VERIFY_TIME 1000*200
#define FORMAT_TEST        7
#define FORMAT_TIME        1000*10*60
#define RBA_OFFSET         1024*64
