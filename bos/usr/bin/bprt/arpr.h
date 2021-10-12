/* @(#)15	1.1  src/bos/usr/bin/bprt/arpr.h, libbidi, bos411, 9428A410j 8/27/93 09:56:43 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define   ON                               1
#define   OFF                              0
#define   BLANK                            0x20
#define   NUMBER_OF_SUPPORTED_PRINTERS     16
#define   NUMBER_OF_PRINTER_PORTS          3
#define   DEFAULT_CSD                      0
#define   PASSTHRU                         1
#define   RTL_ORIENTATION                  1
#define   LTR_ORIENTATION                  0
#define   SEGMENT_LENGTH                   128
#define   HEADER_LENGTH                    86
#define   LENGTH_OF_PRINTER_NAME           8
#define   LENGTH_OF_PORT_NAME              13
#define   BYTE_1_FOR_NUMERICS_TO_HINDU     33
#define   BYTE_1_FOR_NUMERICS_TO_ARABIC    17
#define   START_FILE_NAME                  69
#define   START_PAGE_NUMBER                5
#define   START_DAY_NAME                   57
#define   START_DAY_NUMBER                 54
#define   START_YEAR                       42
#define   START_MONTH                      47
#define   START_ARABIC_TEXT                10
#define   MAX_LINES_PER_PAGE               57
#define   LINE_FEED                        10
#define   FORM_FEED                        12
#define   CARRIAGE_RETURN                  13
#define   SWITCH_INDICATOR                 45   
                                        /* - (dash) character to be followed */
                                        /* by a switch to determine a Bidi   */
                                        /* atrribute or a switch to be sent  */
                                        /* to lp or qprt .                   */ 


