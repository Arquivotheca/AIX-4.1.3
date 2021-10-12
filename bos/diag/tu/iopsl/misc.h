/* @(#)33       1.3  src/bos/diag/tu/iopsl/misc.h, tu_iopsl, bos411, 9428A410j 7/8/93 08:14:01 */
/*
 *   COMPONENT_NAME: TU_IOPSL
 *
 *   FUNCTIONS: ERR_DESC
 *              PRINT
 *              PRINTERR
 *              PRINTSYS
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef BOOL
#define BOOL uchar_t
#endif

/* TU numbers     */

#define TOD_TU      30
#define VPD_TU      50
#define BATTERY_TU  60

/* Define error codes for Salmon system I/O and adapter TU's */

#define IO_ERROR                (-1)    /* returned by ioctl()  */

#define NVRAM_ERROR             10
#define LED_NVRAM_ERROR         20

/* TOD TU return codes */
#define TOD_REGA_WR_ERROR       30  /* Register A read/write error      */
#define TOD_REGB_WR_ERROR       31  /* Register A read/write error      */
#define SEC_INCR_ERROR          32  /* seconds didn't increment         */
#define TOD_RAM_ERROR           33  /* TOD NVRAM write/read test failed */
#define TOD_BATT_ERROR          34  /* bad battery bit                  */

#define POW_KEY_ERROR           40

/* VPD TU return codes */
#define VPD_NOT_FOUND_ERROR     50
#define DELIM_NOT_FOUND_ERROR   50
#define TM_NOT_FOUND_ERROR      51
#define TM_ERROR                51
#define PPN_NOT_FOUND_ERROR     52
#define EC_NOT_FOUND_ERROR      53
#define PI_NOT_FOUND_ERROR      54
#define FN_NOT_FOUND_ERROR      55
#define MF_NOT_FOUND_ERROR      56
#define RPN_NOT_FOUND_ERROR     57
#define VPD_LENGTH_ERROR        58
#define CRC_ERROR               59

#define BATTERY_ERROR           60

#define WRONG_TU_NUMBER        256

#define FOREVER while(1)

#define BLANK   ' '
#define TAB             '\t'
#define EOS             '\0'

#define SUCCESS          0
#define FAIL             1

/* Initialize 'POWER' and 'RSC' flags */
enum machines { RSC = 1, POWER };

/* Variables for system registers, NVRAM and LED locations */

unsigned int power_stat_reg; 
unsigned int nvram_addr;
unsigned int led_reg;
unsigned int tod_data_reg;
unsigned int tod_idx_reg;

/* Flag for determining machine type */
int power_flag;

/* Define print macros */

#ifdef LOGMSG
#define PRINT(msg)       logmsg(msg)
#define PRINTERR(msg)    logerror(msg)
#define PRINTSYS(msg)    log_syserr(msg)
#define ERR_DESC(rc)     get_err_desc(rc)
#else
#define PRINT(msg)
#define PRINTERR(msg)
#define PRINTSYS(msg)
#define ERR_DESC(rc)
#endif

/* Set up structures for planar testing */

typedef struct tucb_t TUCB;

/************************  Time Of Day chip defines  ************/

#define TOD_SIZE       64  /* bytes */
#define TOD_NVRAM_SIZE 50  /* bytes */

/* register offsets */
enum { SECONDS = 0, SECONDS_ALARM, MINUTES, MINUTES_ALARM,
       HOURS, HOURS_ALARM, DAY_OF_WEEK, DAY_OF_MONTH,
       MONTH, YEAR, REG_A, REG_B, REG_C, REG_D,
       NVRAM
     };

/****************************************************************/

/* function prototypes */

ushort_t crc_gen(uchar_t [], int);
int get_tod_data(int, ulong_t, uchar_t *);
int put_tod_data(int, ulong_t, uchar_t);
int getbyte(int, uchar_t *, ulong_t, int);
int put_byte(int, uchar_t, ulong_t, int);
int get_word(int, ulong_t *, ulong_t, int);
int put_word(int, ulong_t, ulong_t, int);
int battery_tu(int, TUCB *);
int tod_tu(int, TUCB *);
int vpd_tu(int, TUCB *);
