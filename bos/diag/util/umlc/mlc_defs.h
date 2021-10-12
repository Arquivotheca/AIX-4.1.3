/* @(#)44       1.14.3.1  src/bos/diag/util/umlc/mlc_defs.h, dsaumlc, bos41J, 9520A_all 5/16/95 11:35:55 */
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY - header file
 *
 * FUNCTIONS:
 *
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <asl.h>

#define         TRUE            1
#define         FALSE           0

#define         FIELD_SIZE      4096
#define         MAX_FLOPS       16

/* Minimum free blocks needed to write Topology files */
#define         MIN_FREE        5

/* Flags for write_disk */
#define         WRITE_SDDB      0
#define         WRITE_OUTFILE   1


#define         ILVL            0
#define         ULVL            1
#define         RLVL            2

/* Indexes into char *keywrds[] string */
#define         FEATURE         15
#define         FB              19
#define         PART            21
#define         DATE            35
#define         DSMSG           36
#define         END_OF_LIST     37

/* Menu numbers */
#define         PROCESS_DATA    0x802600
#define         SYS_RECORD      0x802601
#define         TERMINATE       0x802602
#define         NEW_VITEM       0x802603
#define         LOAD_SYS        0x802604
#define         RETRY_FORMAT    0x802605
#define         COMPLETE        0x802606


#define         NOFUNC          ((void (*)(void)) 0)
#define         VP_NULL         ((struct VP_PTR *) 0)
#define         FC_NULL         ((struct FC_PTR *) 0)
#define         FB_NULL         ((struct FB_PTR *) 0)
#define         PN_NULL         ((struct PN_PTR *) 0)

/* various bit mapped flags */

#define         RMV_TMP         0x01
#define         EM_DSK          0x20

#define         RSC_MODEL       0x02000000

typedef struct {
        char            vers[FIELD_SIZE],
                        serial[FIELD_SIZE],
                        type[FIELD_SIZE],
                        name[FIELD_SIZE],
                        street[FIELD_SIZE],
                        citystate[FIELD_SIZE],
                        zip[FIELD_SIZE],
                        contact[FIELD_SIZE],
                        phone[FIELD_SIZE],
                        sysno[FIELD_SIZE],
                        cust[FIELD_SIZE],
                        adrs[FIELD_SIZE],
                        build[FIELD_SIZE],
                        install[FIELD_SIZE],
                        pid[FIELD_SIZE],
                        seq_no[FIELD_SIZE],
                        d1[FIELD_SIZE],
                        d2[FIELD_SIZE],
                        d3[FIELD_SIZE],
                        d4[FIELD_SIZE],
                        d5[FIELD_SIZE],
                        d6[FIELD_SIZE],
                        d7[FIELD_SIZE],
                        d8[FIELD_SIZE],
                        d9[FIELD_SIZE],
                        d10[FIELD_SIZE],
                        d11[FIELD_SIZE],
                        d12[FIELD_SIZE],
                        d13[FIELD_SIZE],
                        d14[FIELD_SIZE],
                        d15[FIELD_SIZE];

        } MACH_REC;

typedef struct {
        char            dat[FIELD_SIZE];
        } LBUFF;

typedef struct {
        short   dummy;
        short   dflgs;
        LBUFF   dat[80];
        } SCRATCH;

typedef struct {
        char    fcode[FIELD_SIZE];
        char    p_num[FIELD_SIZE];
        char    eclvl[FIELD_SIZE];
        char    s_num[FIELD_SIZE];
        char    bar_c[FIELD_SIZE];
        LBUFF   chang[60];
        char    danda[FIELD_SIZE];
        } HIST_REC;

typedef struct {
        int     *baseptr;
        short   type;
        short   action;
        } MD_2LINES;

typedef struct {
        int     fcs;
        int     fbs;
        int     parts;
        } ITEM_CNT;

typedef struct {
        short   rec_size;
        short   flags;
        char    pn[FIELD_SIZE],
                pl[FIELD_SIZE],
                ax[FIELD_SIZE],
                ec[FIELD_SIZE],
                sn[FIELD_SIZE],
                bc[FIELD_SIZE],
                si[FIELD_SIZE],
                cd[FIELD_SIZE],
                lo[FIELD_SIZE],
                rl[FIELD_SIZE],
                ll[FIELD_SIZE],
                dd[FIELD_SIZE],
                dg[FIELD_SIZE],
                fn[FIELD_SIZE],
                dc[FIELD_SIZE],
                ds[FIELD_SIZE];
        } P_REC, V_REC;

typedef struct {
        short   rec_size;
        short   flags;
        char    fc[FIELD_SIZE],
                t1[FIELD_SIZE],
                s1[FIELD_SIZE],
                ms[FIELD_SIZE],
                dc[FIELD_SIZE],
                ds[FIELD_SIZE];
        } F_REC;

typedef struct {
        short   rec_size;
        short   flags;
        char    fb[FIELD_SIZE],
                fd[FIELD_SIZE],
                ds[FIELD_SIZE];
        } B_REC;

#define         V_LINES         (sizeof(V_REC)/FIELD_SIZE)
#define         P_LINES         (sizeof(P_REC)/FIELD_SIZE)
#define         F_LINES         (sizeof(F_REC)/FIELD_SIZE)
#define         B_LINES         (sizeof(B_REC)/FIELD_SIZE)
#define         H_LINES         (sizeof(HIST_REC)/FIELD_SIZE)
#define         M_LINES         (sizeof(MACH_REC)/FIELD_SIZE)

struct PN_PTR {
        P_REC           *data;
        P_REC           *mod_dat;
        P_REC           *tmp_dat;
        struct FC_PTR   *FCown;
        struct FB_PTR   *FBown;
        struct PN_PTR   *nextPN;
        struct PN_PTR   *prevPN;
        struct VP_PTR   *v_mate;
        int             action;
        } PN_PTR;

struct FB_PTR {
        B_REC           *data;
        B_REC           *mod_dat;
        B_REC           *tmp_dat;
        struct FC_PTR   *FCown;
        struct PN_PTR   *FstPN;
        struct FB_PTR   *nextFB;
        struct FB_PTR   *prevFB;
        int             action;
        } FB_PTR;

struct FC_PTR {
        F_REC           *data;
        F_REC           *mod_dat;
        F_REC           *tmp_dat;
        struct FB_PTR   *FstFB;
        struct PN_PTR   *FstPN;
        struct FC_PTR   *nextFC;
        struct FC_PTR   *prevFC;
        int             action;
        };

struct VP_PTR {
        V_REC           *data;
        struct PN_PTR   *p_mate;
        struct VP_PTR   *nextVP;
        };

typedef struct {
        MACH_REC        mr;
        struct VP_PTR   *vpd;
        } VPD_REC;

typedef struct {
        int             dir;
        void            (*func1)(void);
        void            (*func2)(void);
        void            (*func3)(void);
        char            *str1;
        char            *str2;
        char            *str3;
        } pd_struct;                    /* used as argument to parse_data() */
