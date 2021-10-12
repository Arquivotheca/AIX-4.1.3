/* @(#)61  1.7  src/bos/diag/da/artic/dartic.h, daartic, bos41J, 9511A_all 3/3/95 11:13:36 */
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "dartic_msg.h"

#define                 WHICH_INTERFACE_PORT_0          100
#define                 WHICH_INTERFACE_PORT_1          101
#define                 WHICH_INTERFACE_PORT_2          102

#define                 PORT_0_MENU                     103
#define                 INSERT_V35_WRAP_PLUG_PORT_0     104
#define                 INSERT_232_WRAP_PLUG_PORT_0     105
#define                 INSERT_X21_WRAP_PLUG_PORT_0     106
#define                 INSERT_422_WRAP_PLUG_PORT_0     107
#define                 REMOVE_V35_WRAP_PLUG_PORT_0     108
#define                 REMOVE_232_WRAP_PLUG_PORT_0     109
#define                 REMOVE_X21_WRAP_PLUG_PORT_0     110
#define                 REMOVE_422_WRAP_PLUG_PORT_0     111

#define                 PORT_1_MENU                     112
#define                 INSERT_V35_WRAP_PLUG_PORT_1     113
#define                 INSERT_232_WRAP_PLUG_PORT_1     114
#define                 REMOVE_V35_WRAP_PLUG_PORT_1     115
#define                 REMOVE_232_WRAP_PLUG_PORT_1     116

#define                 PORT_2_MENU                     117
#define                 REMOVE_232_WRAP_PLUG_PORT_2     118
#define                 REMOVE_422_WRAP_PLUG_PORT_2     119
#define                 INSERT_232_WRAP_PLUG_PORT_2     120
#define                 INSERT_422_WRAP_PLUG_PORT_2     121

#define                 PORT_3_MENU                     122
#define                 REMOVE_232_WRAP_PLUG_PORT_3     123
#define                 INSERT_232_WRAP_PLUG_PORT_3     124

#define                 PORT_0_MENU_CABLE_TESTING       125
#define                 PORT_1_MENU_CABLE_TESTING       126
#define                 PORT_2_MENU_CABLE_TESTING       127
#define                 PORT_3_MENU_CABLE_TESTING       128

#define                 INSERT_V35_CABLE_PORT_0         129
#define                 INSERT_232_CABLE_PORT_0         130
#define                 INSERT_X21_CABLE_PORT_0         131
#define                 INSERT_422_CABLE_PORT_0         132
#define                 REMOVE_WRAP_PLUG_FROM_CABLE_PORT_0              133

#define                 INSERT_V35_CABLE_PORT_1                 134
#define                 INSERT_232_CABLE_PORT_1                 135
#define                 REMOVE_WRAP_PLUG_FROM_CABLE_PORT_1      136

#define                 INSERT_232_CABLE_PORT_2                 137
#define                 INSERT_422_CABLE_PORT_2                 138
#define                 REMOVE_WRAP_PLUG_FROM_CABLE_PORT_2      139

#define                 INSERT_232_CABLE_PORT_3                 140
#define                 REMOVE_WRAP_PLUG_FROM_CABLE_PORT_3      141

/** Cable isolate                       ***/

#define         CABLE_ISOLATE_PORT_0_V35                        150
#define         CABLE_ISOLATE_PORT_0_232                        151
#define         CABLE_ISOLATE_PORT_0_X21                        152
#define         CABLE_ISOLATE_PORT_0_422                        153
#define         CABLE_ISOLATE_PORT_1_232                        154
#define         CABLE_ISOLATE_PORT_1_V35                        155
#define         CABLE_ISOLATE_PORT_2_232                        156
#define         CABLE_ISOLATE_PORT_2_422                        157
#define         CABLE_ISOLATE_PORT_3_232                        158


#define         CABLE_MENU_TESTING                              160
#define         PORTS_MENU_TESTING                              161
#define         TESTING_INDIVIDUAL_PORT_CONNECTOR               162


#define                 NEW_MSG                         1
#define                 COMMON_GROUP_TEST               0
#define                 MPQP_4PORT                      1
#define                 X_25                            2
#define                 PM                              3
#define                 MP_2                            4
#define                 T1E1J1                          5
#define                 MPQP_4PORT_ADVANCED             6
#define                 PORTMASTER_ADVANCED             7

#define         MAX_NUM_CARDS           16

#define         NO_ERROR                0
#define         NOT_GOOD                -1
#define         CATALOG_NAME            "MF_DARTIC"
#define         YES                     1
#define         NO                      2

#define         REPORT_FRU              -2
#define         NO_REPORT_FRU           -1


#define         NOT_CONFIG              -1
#define         QUIT                     9999
#define         BAD_CABLE_RC            1234


/*
*       FRU tables of Artic Message
*       Need to fill in rmsg, fmsg field later
*/
/* Common test units for Artic family   */


#define FRU_101         0
#define FRU_118         1
#define FRU_119         2
#define FRU_103         3
#define FRU_104         4
#define FRU_105         5

/* Non interactive tests for MPQP and MP        */

#define FRU_152         0
#define FRU_151         1
#define FRU_8           2

/* Non interactive tests for MP/2 and X.25      */

#define FRU_6           0
#define FRU_7           1
#define FRU_8           2

/* Non interactive tests for T1E1J1             */
/* Test 52, 51 and 2                            */
#define FRU_102         2



#define FRU_120         9
#define FRU_121         10
#define FRU_112         11
#define FRU_150         12
#define FRU_OPEN        13
#define FRU_ERROR_VPD   14


/* For getting the device purpose */

struct device_name_t
{
        char    rsv[33];
        char    device_name[45];
};

