/*
 * COMPONENT_NAME: (SYSXHFT) High Function Terminal - hftras.h
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_GEMRAS
#define _H_GEMRAS

#define HOOKUP 0x6969
/*************************************************************************
  ERROR LOGGING
 *************************************************************************/

/*------------
 Unique RAS codes used to identify specific error locations for error logging
  ------------*/
#define UNIQUE_1        "1"
#define UNIQUE_2        "2"
#define UNIQUE_3        "3"
#define UNIQUE_4        "4"
#define UNIQUE_5        "5"
#define UNIQUE_6        "6"
#define UNIQUE_7        "7"
#define UNIQUE_8        "8"
#define UNIQUE_9        "9"
#define UNIQUE_10       "10"
#define UNIQUE_11       "11"
#define UNIQUE_12       "12"
#define UNIQUE_13       "13"
#define UNIQUE_14       "14"
#define UNIQUE_15       "15"
#define UNIQUE_16       "16"
#define UNIQUE_17       "17"
#define UNIQUE_18       "18"
#define UNIQUE_19       "19"
#define UNIQUE_20       "20"

/************************************************************************/
/* Defines unique to HISPD3D (gemini)                                   */
/************************************************************************/
#define  ALREADY_DEFINED      3201      /* Adapter is already defined,  */
                                        /* only one adapter supported   */
#define  INVALID_FONT         3202      /* An invalid font was passed   */
                                        /* in the font list             */
#define  BAD_ID               3203      /* One of the cards in the      */
                                        /* adapter is missing           */
#define  DRP_ERROR            3204      /* DrP FIFO Clear routine failed*/
#define  DRP_TIMEOUT          3205      /* No response from a DrP module*/
#define  SHP_ERROR            3206      /* SHP FIFO Clear routine failed*/
#define  SHP_TIMEOUT          3207      /* No response from a SHP module*/
#define  GCP_ERROR            3208      /* GCP FIFO Clear routine failed*/
#define  GCP_TIMEOUT          3209      /* No response from a GCP module*/
#define  BAD_VECTOR_ID        3210      /* Adapter returned invalid ID  */
#define  VME_BUS_ERROR        3211      /* VME Bus Eerror               */
#define  SPURIOUS_INTR        3212      /* Unable to determine cause of */
                                        /* Interrupt                    */
#define  FAULT_GCP            3213      /* External VME fault - GCP     */
#define  FAULT_DRP            3214      /* External VME fault - DRP     */
#define  FAULT_SHP            3215      /* External VME fault - SHP     */
#define  UC_FAULT_GCP         3216      /* Microcode Fault    - GCP     */
#define  UC_FAULT_DRP         3217      /* Microcode Fault    - DRP     */
#define  UC_FAULT_SHP         3218      /* Microcode Fault    - SHP     */
#define  PARITY_ERR_HI_GCP    3219      /* Parity Error HI    - GCP     */
#define  PARITY_ERR_HI_DRP    3220      /* Parity Error HI    - DRP     */
#define  PARITY_ERR_HI_SHP    3221      /* Parity Error HI    - SHP     */
#define  PARITY_ERR_LO_GCP    3222      /* Parity Error LO    - GCP     */
#define  PARITY_ERR_LO_DRP    3223      /* Parity Error LO    - DRP     */
#define  PARITY_ERR_LO_SHP    3224      /* Parity Error LO    - SHP     */
#define  BAD_REQUEST          3225      /* Illegal Request code         */
#define  BAD_ORDER            3226      /* Illegal Graphic Order        */
#define  BAD_INTR_CODE        3227      /* Unrecognized Interrupt reason*/
#define  DRP_EXCEPT           3228      /* DRP Exception Error          */
#define  BAD_DRP_CODE         3229      /* Invalid DRP Interrupt code   */
#define  BUS_PARITY_ERR       3230      /* VME Bus Parity Error         */
#define  BUS_TIMEOUT          3231      /* VME Bus Timeout    r         */
#define  CPU_BUS_TIMEOUT      3232      /* VME CPU Bus Timeout          */
#define  INVALID_DISPLAY_MODE 3233      
#endif /* _H_GEMRAS */
