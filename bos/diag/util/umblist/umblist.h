/* @(#)93       1.3.1.4  src/bos/diag/util/umblist/umblist.h, dsaumblist, bos41J, 9510A_all 2/27/95 03:41:36 */
/*
 * COMPONENT_NAME: DUTIL - Diagnostic Utility UMBLIST
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_UMBLIST
#define _H_UMBLIST

/*-------------------------------- codes -------------------------------------*/

#define NVLOAD1                 "nvload1"
#define NVLOAD2                 "nvload2"
#define PVID_LENGTH             17

#define DEVICE_FILENAME         "/tmp/umblist_devicenames"

/*------------------------------ miscellaneous -------------------------------*/
#define UMBLIST_MENU1   0x802020
#define UMBLIST_MENU2   0x802021
#define UMBLIST_MENU3   0x802022
#define UMBLIST_MENU4   0x802023
#define UMBLIST_MENU5   0x802024

#define MIN_STR(x, y)   (strlen(x) < strlen(y)) ? strlen(x) : strlen(y)
#define MAX_CHAR        ASL_DIALOGUE_TEXT_SIZE
#define SSA_SN_SIZE     15

/*----------------------------- NVRAM ADDRESSES -----------------------------*/
#define IPLIST_PREVBOOT         0xA00200
#define IPLIST_NORMAL           0xA00224
#define IPLIST_SERVICE          0xA00278

/*----------------------------- IPLIST SPECIFICS ----------------------------*/
#define IPLIST_LENGTH           84
#define PREVBOOT_LENGTH         18

#define NORMAL_VALIDITY_CODE_1  0x4A
#define NORMAL_VALIDITY_CODE_2  0x4D
#define SERVICE_VALIDITY_CODE_1 0x57
#define SERVICE_VALIDITY_CODE_2 0x52

#define MODE_NORMAL             "normal"
#define MODE_SERVICE            "service"
#define MODE_BOTH               "both"
#define MODE_PREVBOOT           "prevboot"

#define NORMAL_MODE             1
#define SERVICE_MODE            2
#define PREVBOOT_MODE           4


#define NVRAM           "/dev/nvram"

/*-------------------------------- prefix strings ----------------------------*/
#define FD      "fd"
#define SCDISK  "scdisk"
#define BADISK  "badisk"
#define CDROM   "cd"
#define RMT     "rmt"
#define NVLOAD  "nvload"
#define HDISK   "hdisk"
#define ENT     "ent"
#define TOK     "tok"
#define FDDI    "fddi"

/*------------------------------ disk subclasses -----------------------------*/
#define SERDASD_SUBCLASS                "serdasdc"
#define SCSI_SUBCLASS                   "scsi"
#define BADISK_SUBCLASS                 "mca"

/*------------------------------ device types --------------------------------*/
#define SCSI_DISK               1
#define SCSI_DISKETTE           2
#define SCSI_CDROM              3
#define SCSI_RMT                4

/*-------------------------------- codes -------------------------------------*/
#define G_RMT_CODE              'T'
#define G_CDROM_CODE            'C'
#define G_BADISK_CODE           'K'
#define G_SCDISK_CODE           'I'
#define G_FD_CODE               'F'
#define G_FDDI_CODE             'P'
#define GENERIC_CODE            'G'
#define BADISK_CODE             'K'
#define SCSI_CODE               'S'
#define PVID_CODE               'V'
#define NVLOAD_CODE             'R'
#define FD_CODE                 'N'
#define ENT_CODE                'D'
#define TOK_CODE                'O'
#define FDDI_CODE               'P'
#define SSA_CODE                'T'

/*--------------------------- generic device info ---------------------------*/

struct generic_info
{
        char            *name;
        unsigned char   length;
        char            code1;
        char            code2;
};

struct generic_info generic[] = /* device information */
{
        {FD,            2, 'G', 'F'},
        {SCDISK,        2, 'G', 'I'},
        {BADISK,        2, 'G', 'K'},
        {CDROM,         2, 'G', 'C'},

        {RMT,           2, 'G', 'T'},
        {ENT,           2, 'G', 'D'},
        {TOK,           2, 'G', 'O'},
        {FDDI,          2, 'G', 'P'}
};

#define MAX_GENERIC     ((short)(sizeof(generic)/sizeof(generic[0])))

/*--------------------------- specific device info ---------------------------*/

/* This array should always have the same number of "rows" as the specific[]    */

char *specific_str[] =                          /* prefixes for specific */
{                                               /* bootable devices      */
        FD,
        HDISK,
        CDROM,
        RMT,

        NVLOAD,
        ENT,
        TOK,
        FDDI,
};
#define NUM_SPECIFIC    ((short)(sizeof(specific_str)/sizeof(specific_str[0])))

#endif
