/* @(#)73	1.6.2.5  src/bos/kernel/si/POWER/branch.h, syssi, bos41J, 9521A_all 5/23/95 13:55:52 */
/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * When adding functions to this table the file branch.m4 must also be
 * changed.
 *
 * NOTE: This header file should only be included by hardinit.c.
 *	types.h and overlay.h must be included before this.
 */

/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

extern int brkpoint();
extern int d_slave(), d_slave_rs1(), d_slave_rsc(), d_slave_ppc();
extern int d_kmove(), d_kmove_rs1();
extern int d_move(), d_move_pwr(), d_move_ppc();
extern int d_master(), d_master_pwr(), d_master_ppc();
extern int d_complete(), d_complete_pwr(), d_complete_ppc();
extern int d_init(), d_init_pwr(), d_init_ppc();
extern int d_clear(), d_clear_pwr(), d_clear_ppc();
extern int d_cflush(), d_cflush_pwr(), d_cflush_ppc();
extern int d_bflush(), d_bflush_pwr(), d_bflush_ppc();
extern int d_mask(), d_mask_pwr(), d_mask_ppc();
extern int d_unmask(), d_unmask_pwr(), d_unmask_ppc();
extern int xmemdma(), xmemdma_pwr(), xmemdma_ppc();
extern int state_save(), state_save_rs(), state_save_pc();
extern int resume(), resume_rs(), resume_pc();
extern int mfsri_pwr, mfsri_ppc, mfsr_pwr, mfsr_ppc;
extern int chgsr_pwr, chgsr_601, chgsr_ppc, mtsr_ppc, mtsr_pwr;
extern int vm_cflush(), vm_cflush_pwr(), vm_cflush_ppc_comb();
extern int v_copypage(), v_copypage_pwr(), v_copypage_ppc_comb();
extern int v_zpage(), v_zpage_ppc(), v_zpage_pwr();
extern int mfdec(), mfdec_pwr(), mfdec_ppc();
extern int al_prolog_pwr, al_prolog_ppc;
extern int ds_flih, pr_flih, pr_flih_ppc, mc_flih, sr_flih;
extern int is_flih, fp_flih;
extern int run_flih;
extern int isync_vmatt, isync_vmdet;
extern int noop_inst_pwr, noop_inst_ppc;
extern int isync_dsis;
extern int isync_vcs1, isync_vcs2, isync_vcs3, isync_vcs4,
           isync_vcs5, isync_vcs6, isync_tlb;
extern int isync_disownfp_1, isync_disownfp_2, isync_disownfp_3,
           isync_disownfp_4, isync_disownfp_5, isync_disownfp_6;
extern int isync_swtch_1;
#ifdef _POWER_MP
extern int isync_swtch_2;
#endif
extern int isync_i_dosoft;

extern int isync_copyinstr1, isync_copyinstr2;
extern int mulh_pwr, mull_pwr, divss_pwr, divus_pwr, quoss_pwr, quous_pwr;
extern int mulh_ppc, mull_ppc, divss_ppc, divus_ppc, quoss_ppc, quous_ppc;
extern int mc_prolog_pwr, dse_prolog_ppc;
extern int flih_prolog_pwr;
extern int fpi_flih;
extern int sc_front_pwr, sc_front_ppc, cs_svc_pwr, cs_pwr, cs_ppc;
extern int isync_sc1, isync_sc2, isync_sc3, isync_sc4;
extern int ds_prolog_pwr, ds_prolog_ppc, is_prolog_pwr, is_prolog_ppc;
extern int isync_kgetsig;

/*
 * External interrupt
 */
extern int ex_flih_rs1, ex_flih_rs2, ex_flih_ppc;
extern int i_reset_int(), i_reset_pwr(), i_reset_ppc();
extern int i_unmask_pwr(), i_unmask_ppc();
extern int i_mask_pwr(), i_mask_ppc();
extern int i_genplvl(), i_genplvl_rs1(), i_genplvl_rs2(), i_genplvl_ppc();
extern int i_enableplvl(), i_enableplvl_rs1(), i_enableplvl_rs2(),
					       i_enableplvl_ppc();
extern int i_disableplvl(), i_disableplvl_rs1(), i_disableplvl_rs2(),
						 i_disableplvl_ppc();
extern int i_loginterr(), i_loginterr_pwr(), i_loginterr_ppc();
extern int i_soft(), i_eis_set(), i_peis_set(), i_mfrr_set();
extern int i_reset_soft(), i_eis_reset(), i_peis_reset(), i_mfrr_reset();
extern int i_issue_eoi(), i_issue_eoi_ppc(), i_issue_eoi_rspc();

/* Simple linked list manipulations */
extern void put_onto_list(), put_onto_list_ppc(), put_onto_list_pwr();
extern void get_from_list(), *get_from_list_ppc(), *get_from_list_pwr();

/* Atomic Op with limit check */
extern int fetch_and_limit(), fetch_and_limit_ppc(), fetch_and_limit_pwr();

/* Locks and Atomic Ops--branch table entries */
extern void slock(), slock_pwr(), slock_ppc();
extern void sunlock(), sunlock_pwr(), sunlock_ppc();
extern boolean_t lock_mine(), lock_mine_pwr(), lock_mine_ppc();
extern void lock_write(), lock_write_pwr(), lock_write_ppc();
extern void lock_read(), lock_read_pwr(), lock_read_ppc();
extern void lock_done(), lock_done_pwr(), lock_done_ppc();
extern boolean_t lock_read_to_write(), lock_read_to_write_pwr(),
                 lock_read_to_write_ppc();
extern void lock_write_to_read(), lock_write_to_read_pwr(),
            lock_write_to_read_ppc();
extern boolean_t lock_try_write(), lock_try_write_pwr(), lock_try_write_ppc();
extern boolean_t lock_try_read(), lock_try_read_pwr(), lock_try_read_ppc();
extern boolean_t lock_try_read_to_write(), lock_try_read_to_write_pwr(), 
                 lock_try_read_to_write_ppc();

/* Locks and Atomic Ops--overlay table entries */
#ifdef _POWER_MP
extern int rsimple_lock_pwr, rsimple_lock_ppc;
extern int rsimple_lock_try_pwr, rsimple_lock_try_ppc;
extern int rsimple_unlock_pwr, rsimple_unlock_ppc;
#endif /* _POWER_MP */
extern int simple_lock_pwr, simple_lock_ppc;
extern int simple_lock_try_pwr, simple_lock_try_ppc;
extern int simple_unlock_pwr, simple_unlock_ppc;
extern int fetch_and_add_pwr, fetch_and_add_ppc;
extern int fetch_and_add_h_pwr, fetch_and_add_h_ppc;
extern int fetch_and_and_pwr, fetch_and_and_ppc;
extern int fetch_and_or_pwr, fetch_and_or_ppc;
extern int fetch_and_nop_pwr, fetch_and_nop_ppc;
extern int compare_and_swap_pwr, compare_and_swap_ppc;
extern int test_and_set_pwr, test_and_set_ppc;
extern int disable_lock_rs1, disable_lock_rs2, disable_lock_sof;
extern int unlock_enable_rs1, unlock_enable_rs2, unlock_enable_sof;
extern int lockl_pwr, lockl_ppc;
extern int unlockl_pwr, unlockl_ppc;
extern int _check_lock_ppc, _check_lock_pwr, _clear_lock_pwr, _clear_lock_ppc;

#ifdef _POWER_MP
/* ppda/csa/curthread management -- overlay table entries */
extern int my_ppda_pwr, my_ppda_ppc;
extern int my_csa_pwr, my_csa_ppc;
extern int get_curthread_pwr, get_curthread_ppc;
extern int set_csa_pwr, set_csa_ppc;
extern int set_curthread_pwr, set_curthread_ppc;
#endif

extern void vmhwinit(), vmhwinit_pwr(), vmhwinit_ppc();
extern void p_enter(), p_enter_rs1(), p_enter_rs2(), p_enter_ppc();
extern void p_rename(), p_rename_rs1(), p_rename_rs2(), p_rename_ppc();
extern void p_remove(), p_remove_rs1(), p_remove_rs2(), p_remove_ppc();
extern void p_remove_all(), p_remove_all_rs1(), p_remove_all_rs2(),
                p_remove_all_ppc();
extern int p_is_modified(), p_is_modified_rs1(), p_is_modified_rs2(),
                p_is_modified_ppc();
extern int p_is_referenced(), p_is_referenced_rs1(), p_is_referenced_rs2(),
                p_is_referenced_ppc();
extern int p_clear_modify(), p_clear_modify_rs1(), p_clear_modify_rs2(),
                p_clear_modify_ppc();
extern void p_protect(), p_protect_rs1(), p_protect_rs2(), p_protect_ppc();
extern void p_page_protect(), p_page_protect_rs1(), p_page_protect_rs2(),
                p_page_protect_ppc();
extern void p_lookup(), p_lookup_rs1(), p_lookup_rs2(), p_lookup_ppc();
extern void curtime(), curtime_pwr(), curtime_ppc();
extern void update_decrementer(), mtdec(), update_decrementer_ppc();
extern void update_system_time(), update_system_time_pwr(),
            update_system_time_ppc();
extern int i_reset_rspc(), i_unmask_rspc(), i_mask_rspc(), i_genplvl_rspc();
extern int i_enableplvl_rspc(), i_disableplvl_rspc(), i_loginterr_soft();
extern int ex_flih_rspc, dec_flih;
extern int iomem_att_601(), iomem_att_ppc();
extern int iomem_det_601(), iomem_det_ppc();
extern int v_copypage_ppc_splt(), vm_cflush_ppc_splt();
extern int xmemccpy(), xmemccpy_pwr(), xmemccpy_ppc();
extern void invtlb(), invtlb_pwr(), invtlb_ppc();
extern int tlb_flih_ifm, tlb_flih_drm, tlb_flih_dwm;
extern void *copyin();
extern void *copyin_pwr();
extern void *copyin_ppc();
extern void *uiocopyin();
extern void *uiocopyin_pwr();
extern void *uiocopyin_ppc();
extern void *uiocopyin_chksum();
extern void *uiocopyin_chksum_pwr();
extern void *uiocopyin_chksum_ppc();
extern void *copyout();
extern void *copyout_pwr();
extern void *copyout_ppc();
extern void *uiocopyout();
extern void *uiocopyout_pwr();
extern void *uiocopyout_ppc();
extern void *uiocopyout_chksum();
extern void *uiocopyout_chksum_pwr();
extern void *uiocopyout_chksum_ppc();
extern void *exbcopy();
extern void *exbcopy_pwr();
extern void *exbcopy_ppc();

/*
 * These macros are used to enlose model and processor specific functions.
 * The macro generates NULL if the correct preprocessor directive is not
 * set.  Using the macros prevents external references from being
 * genereated when the directive is not on.  NULL allways means no action
 * in the branch tables.
 *
 * PowerRS machines use the same macros for processor and model
 * checks.
 */

/*
 * Power RS1 processor and model architecture
 */
#ifdef _POWER_RS1
#define RS1(name) name
#else
#define RS1(name) NULL
#endif

/*
 * Power RS2 processor and model architecture
 */
#ifdef _POWER_RS2
#define RS2(name) name
#else
#define RS2(name) NULL
#endif

/*
 * Power RSC processor and model architecture
 */
#ifdef _POWER_RSC
#define RSC(name) name
#else
#define RSC(name) NULL
#endif

/*
 * Power PC 601 processor
 */
#ifdef _POWER_601
#define R601(name) name
#else
#define R601(name) NULL
#endif

/*
 * Power PC 603 processor
 */ 
#ifdef _POWER_603
#define R603(name) name
#else
#define R603(name) NULL
#endif

/*
 * Power PC 604 processor
 */
#ifdef _POWER_604
#define R604(name) name
#else
#define R604(name) NULL
#endif

/*
 * Pegasus and Rainbow models
 */
#if defined(_RS6K)
#define RRB(name) name
#else
#define RRB(name) NULL
#endif

/*
 * Sandalfoot models
 */
#ifdef _RSPC_UP_PCI
#define RSF(name) name
#else
#define RSF(name) NULL
#endif

/*
 * Index defines for processor based branch tables.  For each function
 * an array of MAX_PROC_INDEX choices are available.  Possition determines
 * the appropriate value for each CPU
 */
#define MAX_PROC_INDEX	6
#define RS1_INDEX	0		/* Power RS RS1 */
#define RSC_INDEX	1		/* Power RS RSC */
#define RS2_INDEX 	2		/* Power RS RS2 */
#define R601_INDEX	3		/* Power PC 601 */
#define R603_INDEX	4		/* Power PC 603 */
#define R604_INDEX	5		/* Power PC 604 */

/*
 * Index defines for model based branch tables.  For each function
 * an array of MAX_MODEL_INDEX choices are available.  Possition determines
 * the appropriate value for each model
 */
#define MAX_MODEL_INDEX 5
#define MOD_RS1_INDEX	0		/* Power RS1 based model */
#define MOD_RSC_INDEX	1		/* Power RSC based model */
#define MOD_RS2_INDEX 	2		/* Power RS2 based model */
#define MOD_RB_INDEX	3		/* Rainbow and Pagasus */
#define MOD_SF_INDEX	4		/* Sandal foot */


/* The following tables describes machine dependent functions.
 * This table is used to hide hardware differences in low level routines
 * from the caller.  This table is carefully coded so that it does not
 * reference any function unless its preprocessor directive is defined.
 * 
 * There are three types of tables.  Branch, overlay, and flih vectors.
 * For each type of table a model and processor table is defined.
 *
 * For each entry in the tables, position is important.  Two functions --
 * proc_index() and model_index() are used to return one of the above
 * indexes.  These functions are used for all tables
 */


/* 
 * This is the processor specific branch table
 */
static struct {
	int (*ext_name)();			/* real function name */
	int (*proc_name[MAX_PROC_INDEX])();	/* processor specific choices */
}branch_data[] = {

	{xmemdma, RS1(xmemdma_pwr), RSC(xmemdma_pwr), RS2(xmemdma_pwr),
		R601(xmemdma_ppc), R603(xmemdma_ppc), R604(xmemdma_ppc)},
	{resume, {RS1(resume_rs), RSC(resume_rs), RS2(resume_rs),
		R601(resume_pc), R603(resume_pc), R604(resume_pc)}},
	{state_save, RS1(state_save_rs), RSC(state_save_rs), RS2(state_save_rs),
		R601(state_save_pc), R603(state_save_pc), R604(state_save_pc)},
	{vm_cflush, RS1(vm_cflush_pwr), RSC(vm_cflush_pwr), RS2(vm_cflush_pwr),
		R601(vm_cflush_ppc_comb), R603(vm_cflush_ppc_splt),
		R604(vm_cflush_ppc_splt)},
	{v_copypage, RS1(v_copypage_pwr), RSC(v_copypage_pwr),
		RS2(v_copypage_pwr), R601(v_copypage_ppc_comb),
		R603(v_copypage_ppc_splt), R604(v_copypage_ppc_splt)},
	{v_zpage, RS1(v_zpage_pwr), RSC(v_zpage_pwr),
		RS2(v_zpage_pwr), R601(v_zpage_ppc), R603(v_zpage_ppc),
		R604(v_zpage_ppc)},
	{mfdec, RS1(mfdec_pwr), RSC(mfdec_pwr), RS2(mfdec_pwr),
		R601(mfdec_ppc), R603(mfdec_ppc), R604(mfdec_ppc)},
	{iomem_att, RS1(brkpoint), RSC(brkpoint), RS2(brkpoint),
		R601(iomem_att_601), R603(iomem_att_ppc), R604(iomem_att_ppc)},
	{iomem_det, RS1(brkpoint), RSC(brkpoint), RS2(brkpoint),
		R601(iomem_det_601), R603(iomem_det_ppc), R604(iomem_det_ppc)},
	{io_att, RS1(vm_att), RSC(vm_att), RS2(vm_att),
		R601(vm_att), R603(vm_att), R604(vm_att)},
	{io_det, RS1(vm_det), RSC(vm_det), RS2(vm_det),
		R601(vm_det), R603(vm_det), R604(vm_det)},
	{xmemccpy, RS1(xmemccpy_pwr), RSC(xmemccpy_pwr), RS2(xmemccpy_pwr),
		R601(xmemccpy_pwr), R603(xmemccpy_ppc), R604(xmemccpy_ppc) },
	{invtlb, RS1(invtlb_pwr), RSC(invtlb_pwr), RS2(invtlb_pwr),
		R601(invtlb_ppc), R603(invtlb_ppc), R604(invtlb_ppc) },

/* Simple list manipulations */
	{get_from_list, RS1(get_from_list_pwr), RSC(get_from_list_pwr),
		RS2(get_from_list_pwr), R601(get_from_list_ppc),
	        R603(get_from_list_ppc), R604(get_from_list_ppc)},

	{put_onto_list, RS1(put_onto_list_pwr), RSC(put_onto_list_pwr),
		RS2(put_onto_list_pwr), R601(put_onto_list_ppc),
	        R603(put_onto_list_ppc), R604(put_onto_list_ppc)},

/* Atomic Op with limit check */
	{fetch_and_limit, RS1(fetch_and_limit_pwr), RSC(fetch_and_limit_pwr),
		RS2(fetch_and_limit_pwr), R601(fetch_and_limit_ppc),
	        R603(fetch_and_limit_ppc), R604(fetch_and_limit_ppc)},

/* Locks and Atomic Ops--branch table entries */
	{slock, RS1(slock_pwr), RSC(slock_pwr),
		RS2(slock_pwr), R601(slock_ppc), R603(slock_ppc),
		R604(slock_ppc)},
	{sunlock, RS1(sunlock_pwr), RSC(sunlock_pwr),
		RS2(sunlock_pwr), R601(sunlock_ppc), R603(sunlock_ppc),
		R604(sunlock_ppc)},
	{lock_mine, RS1(lock_mine_pwr), RSC(lock_mine_pwr),
		RS2(lock_mine_pwr), R601(lock_mine_ppc), R603(lock_mine_ppc),
		R604(lock_mine_ppc)},
	{lock_write, RS1(lock_write_pwr), RSC(lock_write_pwr),
		RS2(lock_write_pwr), R601(lock_write_ppc), R603(lock_write_ppc),
		R604(lock_write_ppc)},
	{lock_read, RS1(lock_read_pwr), RSC(lock_read_pwr),
		RS2(lock_read_pwr), R601(lock_read_ppc), R603(lock_read_ppc),
		R604(lock_read_ppc)},
	{lock_done, RS1(lock_done_pwr), RSC(lock_done_pwr),
		RS2(lock_done_pwr), R601(lock_done_ppc), R603(lock_done_ppc),
		R604(lock_done_ppc)},
	{lock_read_to_write, RS1(lock_read_to_write_pwr),
		RSC(lock_read_to_write_pwr),RS2(lock_read_to_write_pwr),
		R601(lock_read_to_write_ppc), R603(lock_read_to_write_ppc),
		R604(lock_read_to_write_ppc)},
	{lock_write_to_read, RS1(lock_write_to_read_pwr),
		RSC(lock_write_to_read_pwr), RS2(lock_write_to_read_pwr),
		R601(lock_write_to_read_ppc), R603(lock_write_to_read_ppc),
		R604(lock_write_to_read_ppc)},
	{lock_try_write, RS1(lock_try_write_pwr), RSC(lock_try_write_pwr),
		RS2(lock_try_write_pwr), R601(lock_try_write_ppc),
		R603(lock_try_write_ppc), R604(lock_try_write_ppc)},
	{lock_try_read, RS1(lock_try_read_pwr), RSC(lock_try_read_pwr),
		RS2(lock_try_read_pwr), R601(lock_try_read_ppc),
		R603(lock_try_read_ppc), R604(lock_try_read_ppc)},
	{lock_try_read_to_write, RS1(lock_try_read_to_write_pwr),
		RSC(lock_try_read_to_write_pwr),
		RS2(lock_try_read_to_write_pwr),
		R601(lock_try_read_to_write_ppc),
		R603(lock_try_read_to_write_ppc),
		R604(lock_try_read_to_write_ppc)},
        {vmhwinit, RS1(vmhwinit_pwr), RSC(vmhwinit_pwr), RS2(vmhwinit_pwr),
                R601(vmhwinit_ppc), R603(vmhwinit_ppc), R604(vmhwinit_ppc)},
        {p_enter, RS1(p_enter_rs1), RSC(p_enter_rs1),
                RS2(p_enter_rs2), R601(p_enter_ppc), R603(p_enter_ppc),
		R604(p_enter_ppc)},
        {p_rename, RS1(p_rename_rs1), RSC(p_rename_rs1),
                RS2(p_rename_rs2), R601(p_rename_ppc), R603(p_rename_ppc),
		R604(p_rename_ppc)},
        {p_remove, RS1(p_remove_rs1), RSC(p_remove_rs1),
                RS2(p_remove_rs2), R601(p_remove_ppc), R603(p_remove_ppc),
		R604(p_remove_ppc)},
        {p_remove_all, RS1(p_remove_all_rs1), RSC(p_remove_all_rs1),
                RS2(p_remove_all_rs2), R601(p_remove_all_ppc),
		R603(p_remove_all_ppc), R604(p_remove_all_ppc)},
        {p_is_modified, RS1(p_is_modified_rs1), RSC(p_is_modified_rs1),
                RS2(p_is_modified_rs2), R601(p_is_modified_ppc),
		R603(p_is_modified_ppc), R604(p_is_modified_ppc)},
        {p_is_referenced, RS1(p_is_referenced_rs1), RSC(p_is_referenced_rs1),
                RS2(p_is_referenced_rs2), R601(p_is_referenced_ppc),
		R603(p_is_referenced_ppc), R604(p_is_referenced_ppc)},
        {p_clear_modify, RS1(p_clear_modify_rs1), RSC(p_clear_modify_rs1),
                RS2(p_clear_modify_rs2), R601(p_clear_modify_ppc),
		R603(p_clear_modify_ppc), R604(p_clear_modify_ppc)},
        {p_protect, RS1(p_protect_rs1), RSC(p_protect_rs1),
                RS2(p_protect_rs2), R601(p_protect_ppc), R603(p_protect_ppc),
		R604(p_protect_ppc)},
        {p_page_protect, RS1(p_page_protect_rs1), RSC(p_page_protect_rs1),
                RS2(p_page_protect_rs2), R601(p_page_protect_ppc),
		R603(p_page_protect_ppc), R604(p_page_protect_ppc)},
        {p_lookup, RS1(p_lookup_rs1), RSC(p_lookup_rs1),
                RS2(p_lookup_rs2), R601(p_lookup_ppc), R603(p_lookup_ppc),
		R604(p_lookup_ppc)},
        {curtime, RS1(curtime_pwr), RSC(curtime_pwr),
                RS2(curtime_pwr), R601(curtime_pwr), R603(curtime_ppc),
		R604(curtime_ppc)},
        {update_decrementer, RS1(mtdec), RSC(mtdec),
                RS2(mtdec), R601(mtdec), R603(update_decrementer_ppc),
		R604(update_decrementer_ppc)},
        {update_system_time, RS1(update_system_time_pwr),
		RSC(update_system_time_pwr), RS2(update_system_time_pwr),
		R601(update_system_time_pwr), R603(update_system_time_ppc),
		R604(update_system_time_ppc)},
	{copyin,  RS1(copyin_pwr),  RSC(copyin_pwr),
		RS2(copyin_pwr),  R601(copyin_pwr), 
		R603(copyin_ppc),  R604(copyin_ppc)},
	{copyout,  RS1(copyout_pwr),  RSC(copyout_pwr),
 		RS2(copyout_pwr),  R601(copyout_pwr), 
 		R603(copyout_ppc),  R604(copyout_ppc)},
	{uiocopyin,  RS1(uiocopyin_pwr),  RSC(uiocopyin_pwr),
		RS2(uiocopyin_pwr),  R601(uiocopyin_pwr), 
		R603(uiocopyin_ppc),  R604(uiocopyin_ppc)},
	{uiocopyout,  RS1(uiocopyout_pwr),  RSC(uiocopyout_pwr),
 		RS2(uiocopyout_pwr),  R601(uiocopyout_pwr), 
 		R603(uiocopyout_ppc),  R604(uiocopyout_ppc)},
	{uiocopyin_chksum,  RS1(uiocopyin_chksum_pwr),  RSC(uiocopyin_chksum_pwr),
		RS2(uiocopyin_chksum_pwr),  R601(uiocopyin_chksum_pwr), 
		R603(uiocopyin_chksum_ppc),  R604(uiocopyin_chksum_ppc)},
	{uiocopyout_chksum,  RS1(uiocopyout_chksum_pwr),  RSC(uiocopyout_chksum_pwr),
		RS2(uiocopyout_chksum_pwr),  R601(uiocopyout_chksum_pwr), 
		R603(uiocopyout_chksum_ppc),  R604(uiocopyout_chksum_ppc)},
	{exbcopy,  RS1(exbcopy_pwr),  RSC(exbcopy_pwr),
		RS2(exbcopy_pwr),  R601(exbcopy_pwr), 
		R603(exbcopy_ppc),  R604(exbcopy_ppc)}
};

/*
 * This is the model specific branch table
 */
static struct {
	int (*ext_name)();			/* function name */
	int (*model_name[MAX_MODEL_INDEX])();	/* model specific choices */
}model_branch_data[] = {

	{d_slave, RS1(d_slave_rs1), RSC(d_slave_rsc), RS2(d_slave_rs1),
		RRB(d_slave_ppc), RSF(brkpoint)},
	{d_kmove, RS1(d_kmove_rs1), RSC(d_kmove_rs1), RS2(d_kmove_rs1),
		RRB(d_move_ppc), RSF(brkpoint)},
	{d_move, RS1(d_move_pwr), RSC(d_move_pwr), RS2(d_move_pwr),
		RRB(d_move_ppc), RSF(brkpoint)},
	{d_master, RS1(d_master_pwr), RSC(d_master_pwr), RS2(d_master_pwr),
		RRB(d_master_ppc), RSF(brkpoint)},
	{d_complete, RS1(d_complete_pwr), RSC(d_complete_pwr), 
		RS2(d_complete_pwr), RRB(d_complete_ppc), RSF(brkpoint)},
	{d_init, RS1(d_init_pwr), RSC(d_init_pwr), RS2(d_init_pwr),
		RRB(d_init_ppc), RSF(brkpoint)},
	{d_clear, RS1(d_clear_pwr), RSC(d_clear_pwr), RS2(d_clear_pwr),
		RRB(d_clear_ppc), RSF(brkpoint)},
	{d_cflush, RS1(d_cflush_pwr), RSC(d_cflush_pwr), RS2(d_cflush_pwr),
		RRB(d_cflush_ppc), RSF(brkpoint)},
	{d_bflush, RS1(d_bflush_pwr), RSC(d_bflush_pwr), RS2(d_bflush_pwr),
		RRB(d_bflush_ppc), RSF(brkpoint)},
	{d_mask, RS1(d_mask_pwr), RSC(d_mask_pwr), RS2(d_mask_pwr),
		RRB(d_mask_ppc), RSF(brkpoint)},
	{d_unmask, RS1(d_unmask_pwr), RSC(d_unmask_pwr), RS2(d_unmask_pwr),
		RRB(d_unmask_ppc), RSF(brkpoint)},

/*
 * External interrupts
 */
        {i_reset_int, RS1(i_reset_pwr), RSC(i_reset_pwr), RS2(i_reset_pwr),
		RRB(i_reset_ppc), RSF(i_reset_rspc)},
	{i_unmask, RS1(i_unmask_pwr), RSC(i_unmask_pwr), RS2(i_unmask_pwr),
		RRB(i_unmask_ppc), RSF(i_unmask_rspc)},
	{i_mask, RS1(i_mask_pwr), RSC(i_mask_pwr), RS2(i_mask_pwr),
		RRB(i_mask_ppc), RSF(i_mask_rspc)},
	{i_genplvl, RS1(i_genplvl_rs1), RSC(i_genplvl_rs1), RS2(i_genplvl_rs2),
		RRB(i_genplvl_ppc), RSF(i_genplvl_rspc)},
	{i_enableplvl, RS1(i_enableplvl_rs1), RSC(i_enableplvl_rs1),
		RS2(i_enableplvl_rs2), RRB(i_enableplvl_ppc),
		RSF(i_enableplvl_rspc)},
	{i_disableplvl, RS1(i_disableplvl_rs1), RSC(i_disableplvl_rs1),
		RS2(i_disableplvl_rs2), RRB(i_disableplvl_ppc),
		RSF(i_disableplvl_rspc)},
	{i_loginterr, RS1(i_loginterr_pwr), RSC(i_loginterr_pwr),
		RS2(i_loginterr_pwr), RRB(i_loginterr_ppc),
		RSF(i_loginterr_soft)},
	{i_soft, RS1(i_eis_set), RSC(i_eis_set),
		RS2(i_peis_set), RRB(i_mfrr_set), RSF(brkpoint)},
	{i_reset_soft, RS1(i_eis_reset), RSC(i_eis_reset),
		RS2(i_peis_reset), RRB(i_mfrr_reset), RSF(brkpoint)},
	{i_issue_eoi, RS1(brkpoint), RSC(brkpoint),
		RS2(brkpoint), RRB(i_issue_eoi_ppc), RSF(i_issue_eoi_rspc)}
};

/* These tables defines functions that are overlays.  Only functions
 * that are performance critical are reached with this mechanism.
 * The files overlay.h and overlay.m4 must be carefully
 * updated when adding functions to this table.  A value of NULL indicates
 * that the overlay is in place or not required for the the particular
 * implementation.
 *
 * This is the processor overlay table
 */
static struct {
	int *ov_addr;		/* target address of overlay */
	int size;		/* maximum size of overlay */
	int *proc_addr[MAX_PROC_INDEX]; /* processor address */
}overlay_data[] = {
	/* overlays for no-op'ing isyncs on Power machines
	 */
	{&isync_vmatt, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_vmdet, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_dsis, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_vcs1, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_vcs2, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_vcs3, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_vcs4, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_vcs5, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_vcs6, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_sc1, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_sc2, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_sc3, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_sc4, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_kgetsig, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_disownfp_1, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_disownfp_2, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_disownfp_3, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_disownfp_4, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_disownfp_5, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_disownfp_6, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_copyinstr1, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_copyinstr2, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
	{&isync_swtch_1, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
#ifdef _POWER_MP
	{&isync_swtch_2, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},
#endif
	{&isync_i_dosoft, sizeof(int), RS1(&noop_inst_pwr),
		RSC(&noop_inst_pwr), RS2(&noop_inst_pwr), R601(&noop_inst_ppc),
		R603(NULL), R604(NULL)},

	/* PPC prolog is in flih table
	 */
	{(int *)MC_FLIH_ADDR, MC_FLIH_SIZE, RS1(&mc_prolog_pwr),
		RSC(&mc_prolog_pwr), RS2(&mc_prolog_pwr), R601(NULL),
		R603(NULL), R604(NULL)},
	{(int *)DS_FLIH_ADDR, DS_FLIH_SIZE, RS1(&ds_prolog_pwr),
		RSC(&ds_prolog_pwr), RS2(&ds_prolog_pwr), R601(&ds_prolog_ppc),
		R603(&ds_prolog_ppc), R604(&ds_prolog_ppc)},
	{(int *)IS_FLIH_ADDR, IS_FLIH_SIZE, RS1(&is_prolog_pwr),
		RSC(&is_prolog_pwr), RS2(&is_prolog_pwr), R601(&is_prolog_ppc),
		R603(&is_prolog_ppc), R604(&is_prolog_ppc)},
	{(int *)AL_FLIH_ADDR, AL_FLIH_SIZE, RS1(&al_prolog_pwr),
		RSC(&al_prolog_pwr), RS2(&al_prolog_pwr), R601(&al_prolog_ppc),
		R603(&al_prolog_ppc), R604(&al_prolog_ppc)},
	{(int *)DSE_FLIH_ADDR, DSE_FLIH_SIZE, RS1(NULL),
		RSC(NULL), RS2(NULL), R601(&dse_prolog_ppc),
		R603(NULL), R604(NULL)},
	{(int *)SC_PPC_ADDR, SC_PPC_SIZE, RS1(NULL),
		RSC(NULL), RS2(NULL), R601(&sc_front_ppc),
		R603(&sc_front_ppc), R604(&sc_front_ppc)},
	{(int *)TLB_FLIH_IFM_ADDR, TLB_FLIH_SIZE, RS1(NULL),
		RSC(NULL), RS2(NULL), R601(NULL),
		R603(&tlb_flih_ifm), R604(NULL)},    
	{(int *)TLB_FLIH_DRM_ADDR, TLB_FLIH_SIZE, RS1(NULL),
		RSC(NULL), RS2(NULL), R601(NULL),
		R603(&tlb_flih_drm), R604(NULL)},    
	{(int *)TLB_FLIH_DWM_ADDR, TLB_FLIH_SIZE, RS1(NULL),
		RSC(NULL), RS2(NULL), R601(NULL),
		R603(&tlb_flih_dwm), R604(NULL)},    
	{(int *)CS_PWR_ADDR, CS_PWR_SIZE, RS1(&cs_svc_pwr),
		RSC(&cs_svc_pwr), RS2(&cs_svc_pwr), R601(NULL),
		R603(NULL), R604(NULL)},
	{(int *)SC_PWR_ADDR, SC_PWR_SIZE, RS1(&sc_front_pwr),
		RSC(&sc_front_pwr), RS2(&sc_front_pwr), R601(NULL),
		R603(NULL), R604(NULL)},

	{(int *)MULH_ADDR, MULH_SIZE, RS1(&mulh_pwr),
		RSC(&mulh_pwr), RS2(&mulh_pwr), R601(&mulh_ppc),
		R603(&mulh_ppc), R604(&mulh_ppc)},
	{(int *)MULL_ADDR, MULL_SIZE, RS1(&mull_pwr),
		RSC(&mull_pwr), RS2(&mull_pwr), R601(&mull_pwr),
		R603(&mull_ppc), R604(&mull_ppc)},
	{(int *)DIVSS_ADDR, DIVSS_SIZE, RS1(&divss_pwr),
		RSC(&divss_pwr), RS2(&divss_pwr), R601(&divss_pwr),
		R603(&divss_ppc), R604(&divss_ppc)},
	{(int *)DIVUS_ADDR, DIVUS_SIZE, RS1(&divus_pwr),
		RSC(&divus_pwr), RS2(&divus_pwr), R601(&divus_ppc),
		R603(&divus_ppc), R604(&divus_ppc)},
	{(int *)QUOSS_ADDR, QUOSS_SIZE, RS1(&quoss_pwr),
		RSC(&quoss_pwr), RS2(&quoss_pwr), R601(&quoss_ppc),
		R603(&quoss_ppc), R604(&quoss_ppc)},
	{(int *)QUOUS_ADDR, QUOUS_SIZE, RS1(&quous_pwr),
		RSC(&quous_pwr), RS2(&quous_pwr), R601(&quous_ppc),
		R603(&quous_ppc), R604(&quous_ppc)},

	{(int *)_CHECK_LOCK_ADDR, _CHECK_LOCK_SIZE, RS1(&cs_pwr),
		RSC(&cs_pwr), RS2(&cs_pwr), R601(&_check_lock_ppc),
		R603(&_check_lock_ppc), R604(&_check_lock_ppc)},
	{(int *)_CLEAR_LOCK_ADDR, _CLEAR_LOCK_SIZE, RS1(&_clear_lock_pwr),
		RSC(&_clear_lock_pwr), RS2(&_clear_lock_pwr), R601(&_clear_lock_ppc),
		R603(&_clear_lock_ppc), R604(&_clear_lock_ppc)},

	{(int *)CS_ADDR, CS_SIZE, RS1(&cs_pwr),
		RSC(&cs_pwr), RS2(&cs_pwr), R601(&cs_ppc),
		R603(&cs_ppc), R604(&cs_ppc)},

	{(int *)CHGSR_ADDR, CHGSR_SIZE, RS1(&chgsr_pwr),
		RSC(&chgsr_pwr), RS2(&chgsr_pwr), R601(&chgsr_601),
		R603(&chgsr_ppc), R604(&chgsr_ppc)},
	/* Use PowerRS version -- no isyncs -- for 601
	 */
	{(int *)MTSR_ADDR, MTSR_SIZE, RS1(&mtsr_pwr),
		RSC(&mtsr_pwr), RS2(&mtsr_pwr), R601(&mtsr_pwr),
		R603(&mtsr_ppc), R604(&mtsr_ppc)},
	{(int *)MFSR_ADDR, MFSR_SIZE, RS1(&mfsr_pwr),
		RSC(&mfsr_pwr), RS2(&mfsr_pwr), R601(&mfsr_ppc),
		R603(&mfsr_ppc), R604(&mfsr_ppc)},
	{(int *)MFSRI_ADDR, MFSRI_SIZE, RS1(&mfsri_pwr),
		RSC(&mfsri_pwr), RS2(&mfsri_pwr), R601(&mfsri_ppc),
		R603(&mfsri_ppc), R604(&mfsri_ppc)},

/* Locks and Atomic Ops-overlay table entries */
	{(int *)LOCKL_ADDR, LOCKL_SIZE, RS1(&lockl_pwr),
		RSC(&lockl_pwr), RS2(&lockl_pwr), R601(&lockl_ppc),
		/* replaced R603(&lockl_ppc) with R603(&lockl_pwr)
		       for 603e errata 15 workaround */
		R603(&lockl_pwr), R604(&lockl_ppc)},
	{(int *)UNLOCKL_ADDR, UNLOCKL_SIZE, RS1(&unlockl_pwr),
		RSC(&unlockl_pwr), RS2(&unlockl_pwr), R601(&unlockl_ppc),
		R603(&unlockl_ppc), R604(&unlockl_ppc)},
#ifdef _POWER_MP
	{(int *)RSIMPLE_LOCK_ADDR, RSIMPLE_LOCK_SIZE, RS1(&rsimple_lock_pwr),
		RSC(&rsimple_lock_pwr), RS2(&rsimple_lock_pwr),
		R601(&rsimple_lock_ppc), R603(&rsimple_lock_ppc), 
		R604(&rsimple_lock_ppc)},
	{(int *)RSIMPLE_LOCK_TRY_ADDR, RSIMPLE_LOCK_TRY_SIZE,
		RS1(&rsimple_lock_try_pwr), RSC(&rsimple_lock_try_pwr),
		RS2(&rsimple_lock_try_pwr), R601(&rsimple_lock_try_ppc),
		R603(&rsimple_lock_try_ppc), R604(&rsimple_lock_try_ppc)},
	{(int *)RSIMPLE_UNLOCK_ADDR, RSIMPLE_UNLOCK_SIZE,
		RS1(&rsimple_unlock_pwr), RSC(&rsimple_unlock_pwr),
		RS2(&rsimple_unlock_pwr), R601(&rsimple_unlock_ppc),
		R603(&rsimple_unlock_ppc), R604(&rsimple_unlock_ppc)},
#endif /* _POWER_MP */
	{(int *)SIMPLE_LOCK_ADDR, SIMPLE_LOCK_SIZE, RS1(&simple_lock_pwr),
		RSC(&simple_lock_pwr), RS2(&simple_lock_pwr),
		R601(&simple_lock_ppc), R603(&simple_lock_ppc),
		R604(&simple_lock_ppc)},
	{(int *)SIMPLE_UNLOCK_ADDR, SIMPLE_UNLOCK_SIZE, RS1(&simple_unlock_pwr),
		RSC(&simple_unlock_pwr), RS2(&simple_unlock_pwr),
		R601(&simple_unlock_ppc), R603(&simple_unlock_ppc),
		R604(&simple_unlock_ppc)},
	{(int *)SIMPLE_LOCK_TRY_ADDR, SIMPLE_LOCK_TRY_SIZE,
		RS1(&simple_lock_try_pwr), RSC(&simple_lock_try_pwr),
		RS2(&simple_lock_try_pwr), R601(&simple_lock_try_ppc),
		R603(&simple_lock_try_ppc), R604(&simple_lock_try_ppc)},
	{(int *)FETCH_AND_ADD_ADDR, FETCH_AND_ADD_SIZE, RS1(&fetch_and_add_pwr),
		RSC(&fetch_and_add_pwr), RS2(&fetch_and_add_pwr),
		R601(&fetch_and_add_ppc), R603(&fetch_and_add_ppc),
		R604(&fetch_and_add_ppc)},
	{(int *)FETCH_AND_ADD_H_ADDR, FETCH_AND_ADD_H_SIZE,
		RS1(&fetch_and_add_h_pwr), RSC(&fetch_and_add_h_pwr),
		RS2(&fetch_and_add_h_pwr), R601(&fetch_and_add_h_ppc),
		R603(&fetch_and_add_h_ppc), R604(&fetch_and_add_h_ppc)},
	{(int *)FETCH_AND_AND_ADDR, FETCH_AND_AND_SIZE, RS1(&fetch_and_and_pwr),
		RSC(&fetch_and_and_pwr), RS2(&fetch_and_and_pwr),
		R601(&fetch_and_and_ppc), R603(&fetch_and_and_ppc),
		R604(&fetch_and_and_ppc)},
	{(int *)FETCH_AND_OR_ADDR, FETCH_AND_OR_SIZE, RS1(&fetch_and_or_pwr),
		RSC(&fetch_and_or_pwr), RS2(&fetch_and_or_pwr),
		R601(&fetch_and_or_ppc), R603(&fetch_and_or_ppc),
		R604(&fetch_and_or_ppc)},
	{(int *)FETCH_AND_NOP_ADDR, FETCH_AND_NOP_SIZE, RS1(&fetch_and_nop_pwr),
		RSC(&fetch_and_nop_pwr), RS2(&fetch_and_nop_pwr),
		R601(&fetch_and_nop_ppc), R603(&fetch_and_nop_ppc),
		R604(&fetch_and_nop_ppc)},
	{(int *)COMPARE_AND_SWAP_ADDR, COMPARE_AND_SWAP_SIZE,
		RS1(&compare_and_swap_pwr), RSC(&compare_and_swap_pwr),
		RS2(&compare_and_swap_pwr), R601(&compare_and_swap_ppc),
		R603(&compare_and_swap_ppc), R604(&compare_and_swap_ppc)},
	{(int *)TEST_AND_SET_ADDR, TEST_AND_SET_SIZE, RS1(&test_and_set_pwr),
		RSC(&test_and_set_pwr), RS2(&test_and_set_pwr),
		R601(&test_and_set_ppc), R603(&test_and_set_ppc),
		R604(&test_and_set_ppc)},
#ifdef _POWER_MP
	{(int *)MY_PPDA_ADDR, MY_PPDA_SIZE, RS1(&my_ppda_pwr),
		RSC(&my_ppda_pwr), RS2(&my_ppda_pwr), R601(&my_ppda_ppc),
		R603(&my_ppda_ppc), R604(&my_ppda_ppc)},
	{(int *)MY_CSA_ADDR, MY_CSA_SIZE, RS1(&my_csa_pwr), RSC(&my_csa_pwr),
		RS2(&my_csa_pwr), R601(&my_csa_ppc), R603(&my_csa_ppc),
		R604(&my_csa_ppc)},
	{(int *)GET_CURTHREAD_ADDR, GET_CURTHREAD_SIZE, RS1(&get_curthread_pwr),
		RSC(&get_curthread_pwr), RS2(&get_curthread_pwr),
		R601(&get_curthread_ppc), R603(&get_curthread_ppc),
		R604(&get_curthread_ppc)},
	{(int *)SET_CSA_ADDR, SET_CSA_SIZE, RS1(&set_csa_pwr),
		RSC(&set_csa_pwr), RS2(&set_csa_pwr), R601(&set_csa_ppc),
		R603(&set_csa_ppc), R604(&set_csa_ppc)},
	{(int *)SET_CURTHREAD_ADDR, SET_CURTHREAD_SIZE, RS1(&set_curthread_pwr),
		RSC(&set_curthread_pwr), RS2(&set_curthread_pwr),
		R601(&set_curthread_ppc), R603(&set_curthread_ppc),
		R604(&set_curthread_ppc)},
#endif
};

/*
 * This is the model specific overlay table
 */

static struct {
	int *ov_addr;		/* target address of overlay */
	int size;		/* maximum size of overlay */
	int *model_addr[MAX_MODEL_INDEX]; /* processor address */
}model_overlay_data[] = {

	{(int *)I_DISABLE_ADDR, I_DISABLE_SIZE+DISABLE_LOCK_SIZE,
		RS1(&disable_lock_rs1), RSC(&disable_lock_rs1),
		RS2(&disable_lock_rs2), RRB(&disable_lock_sof),
		RSF(&disable_lock_sof)},

	{(int *)I_ENABLE_ADDR, I_ENABLE_SIZE+UNLOCK_ENABLE_SIZE,
		RS1(&unlock_enable_rs1), RSC(&unlock_enable_rs1),
		RS2(&unlock_enable_rs2), RRB(&unlock_enable_sof),
		RSF(&unlock_enable_sof)}
};

/* This table defines interrupt vectors to be initialized at system
 * initialization.  A value of NULL indicates that this vector is not
 * initialized on the particular implementation.  Only the address
 * of the flih is in the table.  The si code determines the appropriate
 * flih prolog to copy into place with a run time check.
 */
static struct {
	int *vector;		/* address of interrupt vector */
	int *proc_addr[MAX_PROC_INDEX];
}flih_data[] = {
	
	{(int *)0x100, RS1(&sr_flih), RSC(&sr_flih),
		RS2(&sr_flih), R601(&sr_flih), R603(&sr_flih), R604(&sr_flih)},
	{(int *)0x200, RS1(NULL), RSC(NULL),	/* Power Prolog is Overlay */
		RS2(NULL), R601(&mc_flih), R603(&mc_flih), R604(&mc_flih)},
#ifndef DBI_FLIH
	{(int *)0x700, RS1(&pr_flih), RSC(&pr_flih),
		RS2(&pr_flih), R601(&pr_flih_ppc),
		R603(&pr_flih_ppc), R604(&pr_flih_ppc)},
#endif
	{(int *)0x800, RS1(&fp_flih), RSC(&fp_flih),
		RS2(&fp_flih), R601(&fp_flih), R603(&fp_flih), R604(&fp_flih)},
	{(int *)0xa00, RS1(NULL), RSC(NULL),
		RS2(&fpi_flih), R601(NULL), R603(NULL), R604(NULL)},
	{(int *)0x2000, RS1(NULL), RSC(NULL),
		RS2(NULL), R601(&run_flih), R603(NULL), R604(NULL)}

};

static struct {
	int *vector;		/* address of interrupt vector */
	int *model_addr[MAX_MODEL_INDEX];
}model_flih_data[] = {
	{(int *)0x500, RS1(&ex_flih_rs1), RSC(&ex_flih_rs1),
		RS2(&ex_flih_rs2), RRB(&ex_flih_ppc), RSF(&ex_flih_rspc)},
	{(int *)0x900, RS1(NULL), RSC(NULL),
		RS2(NULL), RRB(&dec_flih), RSF(&dec_flih)}
};
