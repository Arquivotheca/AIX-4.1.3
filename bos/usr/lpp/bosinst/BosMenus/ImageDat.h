/*
 * @(#) 93 1.6 src/bos/usr/lpp/bosinst/BosMenus/ImageDat.h, bosinst, bos412, 9446B 94/11/15 16:30:30
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: include file for BosMenus
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* imageData.h
 *    defines a structure for image.data
 */
struct image_data
{
    char IMAGE_TYPE[10];
    char DATE_TIME[70];
    char UNAME_INFO[70];
    char LICENSE_INFO[70];
    char PRODUCT_TAPE[4];
    char USERVG_LIST[70];
};

struct lv_policy
{
    char SHRINK[4];
    char EXACT_FIT[4];
};

struct ils_data
{
    char LANG[20];
};

struct src_disk_data
{
    struct src_disk_data *next;
    char LOCATION[14];
    char SIZE[8];
    char HDISKNAME[256];
};


struct vg_data
{
    char VGNAME[20];
    char PPSIZE[8];
    char VARYON[4];
    char VG_SOURCE_DISK_LIST[170];
};

struct lv_data
{
    struct lv_data *next;
    char	VOLUME_GROUP [20];
    char	LV_SOURCE_DISK_LIST [256]; 
    char	LOGICAL_VOLUME [16];
    char	TYPE [10];
    char	MAX_LPS [10];
    char	COPIES [10];
    char	LPs [10];
    char	BB_POLICY [20];
    char	INTER_POLICY [20];
    char	INTRA_POLICY [20];
    char	WRITE_VERIFY [20];
    char	UPPER_BOUND [20];
    char	SCHED_POLICY [20];
    char	RELOCATABLE [20];
    char	LABEL [20];
    char	MIRROR_WRITE_CONSISTENCY [20];
    char	LV_SEPARATE_PV [20];
    char	MAPFILE [256];
    char	PP_SIZE [20];
};

struct fs_data
{
    struct fs_data *next;
    char FS_NAME[256];
    char FS_SIZE[10];
    char FS_MIN_SIZE[10];
    char FS_LV[21];
    char FS_FS[20];
    char FS_COMPRESS[10];
    char FS_NBPI[20];
};

struct post_install_data
{
    char BOSINST_FILE[80];
};

/* Put the things togeter */
struct imageDat
{
    struct image_data image_data;
    struct lv_policy lv_policy;
    struct ils_data ils_data;
    struct src_disk_data *src_disk_data, *last_sddp;
    struct vg_data vg_data;
    struct lv_data *lv_data, *last_lvdp;
    struct fs_data *fs_data, *last_fsdp;
    struct post_install_data post_install_data;
};
