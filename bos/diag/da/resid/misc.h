/* @(#)66	1.1  src/bos/diag/da/resid/misc.h, daresid, bos41J, 9520A_all 5/15/95 17:24:39 */
/*
*   COMPONENT_NAME: DARESID
*
*   FUNCTIONS: *
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1995
*   All Rights Reserved
*   Licensed Materials - Property of IBM
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/


/* TU numbers     */

#define TOD_TU      30
#define VPD_TU      50
#define BATTERY_TU  60

/* TOD TU return codes */
#define TOD_REGA_WR_ERROR       30  /* Register A read/write error      */
#define TOD_REGB_WR_ERROR       31  /* Register A read/write error      */
#define SEC_INCR_ERROR          32  /* seconds didn't increment         */
#define TOD_RAM_ERROR           33  /* TOD NVRAM write/read test failed */
#define TOD_BATT_ERROR          34  /* bad battery bit                  */

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

unsigned int tod_data_reg;
unsigned int tod_idx_reg;


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
/*
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
*/
