/* static char sccsid[] = "@(#)07  1.2  src/bos/usr/lib/methods/common/rds_common.h, cfgmethods, bos412, 9446B 11/16/94 19:03:32"; */
/*
 * COMPONENT_NAME: (CFGMETHODS) Configuration Functions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#ifndef RDS_H_FILE
#define RDS_H_FILE
#define NONE            0
#define OK              1
#define HINT            2
#define FOUND           1
#define NOT_FOUND       -1
#define PRESENT         0
#define NOT_PRESENT     2
#define NB_MAX_CABINETS 8
#define MAX_BUS_SCSI    2

/* Structure corresponding to the possible bus_loc attribute values */
struct entries_list {
    char   *value;
    int     cnt;
    struct entries_list *next;
};
#endif
