/* @(#)21       1.9.2.6  src/bos/kernext/disp/gem/inc/gem_cb.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:41:30 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: 
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



#include "gem_def.h"
 
/************************************************************************/
/*      Subroutine references.                                          */
/************************************************************************/
extern vttinit();
extern vttact();
extern vttclr();
extern vttdact();
extern vttdma();
extern vttrds();
extern vttscr();
extern vttsetm();
extern vttera();
extern vttmovc();
extern vtttext();
extern vttstct();
extern vttupdc();
extern vttckcur();
extern vttcfl();
extern vttcpl();
extern vttdefc();
extern vttterm();
#ifndef _KERNAL
void memcpy();
#endif
void upd_cursor();
 
int gem_define();
int gem_config();
int gem_open();
int gem_close();
int gem_ioctl();
int gem_intr();
int gem_ddf();
 
extern gem_make_gp();
extern gem_unmake_gp();
extern gem_create_rcx();
extern gem_delete_rcx();
extern gem_create_win_attr();
extern gem_delete_win_attr();
extern gem_update_win_attr();
extern gem_bind_window();
extern gem_start_switch();
extern gem_end_switch();
extern gem_check_dev();
extern gem_create_win_geom();
extern gem_delete_win_geom();
extern gem_update_win_geom();
extern gem_create_rcxp();
extern gem_delete_rcxp();
extern gem_associate_rcxp();
extern gem_disassociate_rcxp();
extern gem_async_mask();
extern gem_sync_mask();
extern gem_enable_event();
extern gem_dev_init();
extern gem_dev_term();
 
/************************************************************************/
/* Maps the GCP BIF Communications area                                 */
/************************************************************************/
struct gcp_comm {                       /* GCP BIF communication area   */
        uint g_r_hdbkof;                /* offset to GCP_RIOS head d.b. */
	uint g_r_tdbkof;                /* offset to GCP_RIOS tail d.b. */
        char r_g_dbk[24];               /* RIOS_GCP data block          */
	uint pix_flg;                   /* Pixel Complete flag          */
	char rsrvd1[74];                /* reserved for image card      */
        uint cond_code;                 /* condition code               */
        uint init_lod_f;                /* initialization load flags    */
        uint antxt_loktb;		/* annot text lookup table offst*/
        uint im_se_stp;                 /* immed SE FIFO start ptr      */
        uint im_se_rdp;                 /* immed SE FIFO read ptr       */
        uint tr_se_stp;                 /* trav SE FIFO start ptr       */
        uint tr_se_rdp;                 /* trav SE FIFO read pte        */
	uint instl_ftur;                /* installed features           */
        char rsrvd3[8];
	uint fltr_flgs;                 /* filter flags                 */
	char dblocks[360];              /* RIOS_GCP GCP_RIOS data blocks*/
};
 
/************************************************************************/
/* Maps the DRP BIF Communications area                                 */
/************************************************************************/
struct drp_comm {                       /* DRP BIF communication area   */
	char intr_vect[64];             /* interrupt vector area        */
        struct {
                ushort rsrvd;
		ushort reson;           /* interrupt reason code        */
                uint   dbg_erroff;      /* offset to debugger/error 
					  control block                 */
	} d_r_intblk;                   /* DRP to RIOS intr. request blk*/
	char r_d_intblk[8];             /* RIOS to DRP intr. request blk*/
	char srset_rqst[8];             /* soft reset request block     */
	char rsrvd1[28];                /* rsrvd for cursor contrl blk p*/
	int  fbplans;                   /* 8/24 bit planes installed    */
	int  tag_cntr;
	int   vme_shp_tctr;
        uint im_dt_stp;                 /* Immed Data FIFO start pointer*/
        uint im_dt_rdp;                 /* Immed Data FIFO read pointer */
        uint tr_dt_stp;                 /* Trav Data FIFO start pointer */
        uint tr_dt_rdp;                 /* Trav Data FIFO read pointer  */
	uint vme_pix_flgp;              /* cvme address of the GCP      */
					/* Pixel complete flag          */
	struct {                        /* debugger control block       */
		char rsrvd[76];         /* to be completed later        */
        } dbug_cblk;                        
	struct {                        /* error report control blk     */
		char rsrvd[32];         /* to be completed later        */
        } err_cblk;                
        char rsrvd3[256];
};
 
/************************************************************************/
/* Maps the GCP to System Data Communications block                     */
/************************************************************************/
struct g_r_dblk {                       /* RIOS / GCP data block        */
	short rsrvd1;                   /* reserved                     */
	char  flags;                    /* busy and in-process flag     */
	char  reson;                    /* request code                 */
	uint   dbk_lnkof;               /* data block chain link pointer*/
	uint  data[4];                  /* supplement data              */
};
 
/************************************************************************/
/* Maps the System to GCP Data Communications block                     */
/************************************************************************/
struct r_g_dblk {                       /* RIOS / GCP data block        */
	short rsrvd1;                   /* reserved                     */
	char  rsrvd2;                   /* busy flag                    */
	char  reqst;                    /* request code                 */
	int   rsrvd3;                   /* data block chain link pointer*/
	uint  data[16];                 /* supplement data              */
};
 
/************************************************************************/
/* Upper Left Pel coordinate data structure                             */
/************************************************************************/
struct ulpel_pos
{
        int x_ul;
        int y_ul;
};
 
/************************************************************************/
/* Maps fields in the GEMINI Control Register                           */
/************************************************************************/
union g_c_r {                           /* gemini control register      */
	uint s_gcr;                     /* no byte swapping             */
	struct {
		unsigned mgcslot     : 4;
		unsigned rsetgem1    : 1;
		unsigned enab_berr   : 1;
		unsigned enab_cvme   : 1;
		unsigned byte_order  : 1;
		unsigned rsvd1       : 1;
		unsigned func_code   : 3;
		unsigned rsvd2       : 2;
		unsigned drp_in_proc : 1;
		unsigned gcp_in_proc : 1;
		unsigned rsvd3       : 12;
		unsigned rsetvme     : 1;
		unsigned rsetgem2    : 1;
		unsigned mode_load   : 1;
		unsigned byte_order2 : 1;
	} b;
};
 
/************************************************************************/
/* Maps fields in the GEMINI Interrupt Pending Register                 */
/************************************************************************/
union ipnd_r {
	ushort s_ipnd_r;
	struct {
		unsigned rsrvd1          : 1;
		unsigned dma_complete    : 1;
		unsigned bus_err_intr    : 1;
		unsigned cvme_bus_intr   : 1;
		unsigned hi_thresh3_intr : 1;
		unsigned hi_thresh2_intr : 1;
		unsigned hi_thresh1_intr : 1;
		unsigned hi_thresh0_intr : 1;
		unsigned lo_thresh3_intr : 1;
		unsigned lo_thresh2_intr : 1;
		unsigned lo_thresh1_intr : 1;
		unsigned lo_thresh0_intr : 1;
		unsigned fifo_sync3_intr : 1;
		unsigned fifo_sync2_intr : 1;
		unsigned fifo_sync1_intr : 1;
		unsigned fifo_sync0_intr : 1;
	} b;
};
 
/************************************************************************/
/* Maps fields in the GEMINI System Control Register                    */
/************************************************************************/
union sys_ctl_r {
	uint s_sys_ctl;
	struct {
		unsigned sys_fail    : 1;
		unsigned ext_falt    : 1;
		unsigned ucod_falt   : 1;
		unsigned prty_err_hi : 1;
		unsigned rsrvd1      : 1;
		unsigned c25_pndg    : 1;
		unsigned cvme_pndg   : 1;
		unsigned reset_hi    : 1;
		unsigned card_id     : 8;
		unsigned rsrvd2      : 3;
		unsigned prty_err_lo : 1;
		unsigned rsrvd3      : 3;
		unsigned reset_lo    : 1;
		unsigned diag_vmeiv  : 8;
	} b;
};
 
/************************************************************************/
/* Maps fields in the GEMINI Master Status Register                     */
/************************************************************************/
union m_stat_r {
	uint s_m_stat;
	struct {
		unsigned sys_fail    : 1;
		unsigned cpu_timout  : 1;
		unsigned cvme_timout : 1;
		unsigned bus_prty    : 1;
		unsigned rsrvd       : 28;
	} b;
};
 
/************************************************************************/
/* Maps the GEMINI DDS data structure. Used by ODM and VDD              */
/************************************************************************/
struct  gem_dds
{
   char     comp_name[16];          /* Component Name                  */
   ulong    display_id;             /* unique display id               */
   ulong    mem_map_start;          /* Memory map start address        */
   ulong    io_bus_mem_start;       /* I/O Bus memory start address    */
				    /* without segment register        */
   ulong    features;               /* features flags                  */
				    /*   001 = 1 = 24 bit planes       */
				    /*   010 = 2 = 4 meg mem installed */
				    /*   100 = 4 = SHP NOT installed   */
				    /*  1000 = 8 = HW Anti-Alias inst  */
				    /* 10000 = 16= ISO Supported       */
				    /*100000 = 32= 77Hz                */
   ulong    dma_channel;            /* DMA Channel Number              */
   short    slot_number;            /* slot number of interface card   */
   short    intr_level;             /* interrupt level                 */
   short    intr_priority;          /* interrupt priority              */
   short    screen_width_mm;        /* screen width in millimeters     */
   short    screen_height_mm;       /* screen heigth in millimeters    */
   short    color_count;            /* Number of Default colors        */
   ulong    color_table[16];        /* Default Color Table             */
				    /*     Microcode Information       */
   union {                          /*  *** GCP Microcode ***          */
	ulong gcpucfd;              /*  GCP ucode file descriptor(odm) */
	char  *gcpucp;              /* pointer to GCP ucode(vdd)       */
   } u1;
   ulong gcpuclen;                  /* length of GCP ucode(odm)        */
 
   union {                          /*  *** GCP Table ***              */
	ulong gcptblfd;             /* GCP Table file descriptor(odm)  */
	char  *gcptblp;             /* pointer to GCP Table(vdd)       */
   } u2;
   ulong gcptbllen;                 /* length of GCP Table(odm)        */
 
   union {                          /*  *** C25 Microcode ***          */
	ulong c25ucfd;              /* C25 ucode file descriptor(odm)  */
	char  *c25ucp;              /* pointer to C25 ucode(vdd)       */
   } u3;
   ulong c25uclen;                  /* length of C25 Table(odm)        */
 
   union {                          /*  *** SHP Table ***              */
	ulong shptblfd;             /* SHP Table file descriptor(odm)  */
	char  *shptblp;             /* pointer to SHP Table(vdd)       */
     } u4;
   ulong shptbllen;                 /* length of SHP Table(odm)        */
 
   union {                          /*  *** GCP GVP Table ***          */
	ulong gvp5afd;              /* GVP Table file descriptor(odm)  */
	char  *gvp5ap;              /* pointer to GVP Table(vdd)       */
     } u5;
   ulong gvp5alen;                  /* length of GVP Table(odm)        */
 
   union {                          /*  *** DRP/SHP VPD ucode ***      */
	ulong c25vpdfd;             /* VPD file descriptor             */
	char  *c25vpd;              /* pointer to VPD ucode            */
     } u6;
   ulong c25vpdlen;                 /* length of VPD ucode             */
   ulong uchannel_id;               /* BUS ID for accessing adapter    */
 };
 
 
 typedef struct anno_text {
      uint fnt_lookup_off0;
      uint fnt_lookup_off1;
      uint fnt_lookup_off2;
      uint fnt_lookup_off3;
      uint fnt_lookup_off4;
      uint fnt_lookup_off5;
      uint fnt_lookup_kata;
      uint fnt_lookup_off7;
      uint fnt_lookup_off8;
      uint fnt_lookup_csid9;
      uint fnt_lookup_csid10;
      uint rsv3[117];
      uint fnt_lookup_off128;
      char rsv4[TEXT_TBL_LEN-516];
      uint rsv5;
      uint fnt_locate_off;
      char rsv6[LOOKUP_TBL_LEN-8];
      uint fnt_loc_rcd1[7];
      char rsv7[LOC_TBL_RCD_LEN-28];
      uint kata_rsv5;
      uint kata_fnt_locate_off;
      char kata_rsv6[LOOKUP_TBL_LEN-8];
      uint kata_fnt_loc_rcd1[7];
      char kata_rsv7[LOC_TBL_RCD_LEN-28];
      uint csid10_rsv5;
      uint csid10_fnt_locate_off;
      char csid10_rsv6[LOOKUP_TBL_LEN-8];
      uint csid10_fnt_loc_rcd1[7];
      char csid10_rsv7[LOC_TBL_RCD_LEN-28];
      uint csid9_rsv5;
      uint csid9_fnt_locate_off;
      char csid9_rsv6[LOOKUP_TBL_LEN-8];
      uint csid9_fnt_loc_rcd1[7];
      char csid9_rsv7[LOC_TBL_RCD_LEN-28];
    } anno_text;
 
    typedef struct dflt_asl {
      uint rcm_mgt_off;
      uint rsv1;
      uint view_tbl_num;
      uint view_tbl_off;
      uint rsv2[51];
      uint display_mask;
      uint rsv3[3];
      uint line_width_ratio;
      uint poly_mark_size;
      uint anno_text_ratio;
      uint geo_text_cull;
      uint geo_text_cull_ht;
    } dflt_asl;
 
    typedef struct dflt_view_tbl {
      uint index;
      uint flags;
      uint z_clipp_bdy[2];
      uint true_view_port[6];
      uint trans_matrix[16];
      uint p_ref_point[3];
      uint view_pl_dist;
      uint true_win_to_view_ratio[3];
      uint true_inv_win_to_view_ratio[3];
      uint true_win_vc[6];
      uint shear_x;
      uint shear_y;
      uint shear_z;
    } dflt_view_tbl;
 
     typedef struct dflt_rcm {
      uint ch_ind;
      uint frame_buff_mask;
      uint compare_mask;
      uint compare_value;
      uint line_color;
      uint interior_color;
      uint logical_op;
      uint win_x_origin;
      uint win_y_origin;
      uint win_width;
      uint win_height;
      uint flags;
      uint num_pick_attr;
    } dflt_rcm;
 
