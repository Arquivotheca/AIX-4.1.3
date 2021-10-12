#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./lib/nim/methods/Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#  @(#)00 1.68  src/bos/usr/lib/nim/methods/Makefile, cmdnim, bos41J, 9516B_all  4/21/95  15:55:45
#

IPLROM.emulation: IPLROM.image
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

PROGRAMS =	\
		c_ckros_emu \
		c_ch_nfsexp \
		c_errmsg \
		c_mkpfile \
		c_nim_bootinfo32 \
		c_nim_mkboot32 \
		c_stat \
		m_abort \
		m_alloc_boot \
		m_alloc_ns \
		m_alloc_pdir \
		m_alloc_spot \
		m_allocate \
		m_bos_inst \
		m_chattr \
		m_chmac \
		m_chmaster \
		m_chnet \
		m_chres  \
		m_chstate \
		m_ckmac \
		m_ckspot \
		m_ck_lpp_source \
		m_cust \
		m_dealloc_boot \
		m_dealloc_ns \
		m_dealloc_pdir \
		m_dealloc_spot \
		m_deallocate \
		m_destroy_res \
		m_diag \
		m_dkls_inst \
		m_do_sop   \
		m_dtls_inst \
		m_fixquery \
		m_getdate\
		m_instspot \
		m_lslpp \
		m_ls_lpp_source \
		m_maint \
		m_spotmaint \
		m_mkbosi \
		m_mk_lpp_source \
		m_mkmac \
		m_mknet \
		m_mkpdir \
		m_mkres \
		m_mkspot \
		m_reboot \
		m_reset_res \
		m_rmmac \
		m_rmnet \
		m_rmpdir \
		m_rmres \
		m_rmspot \
		m_schedule \
		m_sync_roots
                
SCRIPTS	= 	\
		c_add_routes \
		c_alloc_boot \
		c_at \
		c_bosinst_env \
		c_ch_rhost \
		c_cklevel \
		c_ckspot \
		c_dealloc_boot \
		c_initiate_bootp \
		c_installp \
		c_instspot \
		c_mk_lpp_source \
		c_mk_nimclient \
		c_mkbooti \
		c_mkdir \
		c_mkdump \
		c_mkpaging \
		c_mkroot \
		c_mkspot \
		c_niminfo \
		c_nimpush \
		c_nnc_setup \
		c_popxop \
		c_rmdir \
		c_rmspot \
		c_script \
		c_sh_lib \
		c_sync_root \
		m_unconfig \
		m_nnc_setup \
		m_doreboot

OTHERS = IPLROM.emulation

OPT_LEVEL = -g
CFLAGS	  = -D_BSD -qdbxextra
nim_bootinfo_32_CFLAGS=
LDFLAGS	  += -s



ILIST	= ${PROGRAMS} ${SCRIPTS} IPLROM.emulation

master.example_IDIR = /usr/lpp/bos.sysmgt/nim/
IDIR	= /usr/lpp/bos.sysmgt/nim/methods/

INCFLAGS   += -I${MAKETOP}/bos/usr/sbin/mkboot

# ---------------------------      master methods    ---------------------------

m_alloc_boot_CLEANFILES		= m_alloc_boot ${m_alloc_boot_OFILES}
m_alloc_boot_LIBS				= -lodm -lnim -lmstr
m_alloc_boot_OFILES			= m_alloc_boot.o

m_alloc_ns_CLEANFILES		= m_alloc_ns ${m_alloc_ns_OFILES}
m_alloc_ns_LIBS				= -lodm -lnim -lmstr
m_alloc_ns_OFILES				= m_alloc_ns.o

m_alloc_pdir_CLEANFILES		= m_alloc_pdir ${m_alloc_pdir_OFILES}
m_alloc_pdir_LIBS				= -lodm -lnim -lmstr
m_alloc_pdir_OFILES			= m_alloc_pdir.o

m_alloc_spot_CLEANFILES		= m_alloc_spot ${m_alloc_spot_OFILES}
m_alloc_spot_LIBS				= -lodm -lnim -lmstr
m_alloc_spot_OFILES			= m_alloc_spot.o

m_allocate_CLEANFILES		= m_allocate ${m_allocate_OFILES}
m_allocate_LIBS				= -lodm -lnim -lmstr
m_allocate_OFILES				= m_allocate.o

m_chattr_CLEANFILES			= m_chattr ${m_chattr_OFILES}
m_chattr_LIBS					= -lodm -lnim -lmstr
m_chattr_OFILES				= m_chattr.o

m_chmac_CLEANFILES			= m_chmac ${m_chmac_OFILES}
m_chmac_LIBS					= -lodm -lnim -lmstr
m_chmac_OFILES					= m_chmac.o

m_chmaster_CLEANFILES		= m_chmaster ${m_chmaster_OFILES}
m_chmaster_LIBS				= -lodm -lnim -lmstr
m_chmaster_OFILES				= m_chmaster.o

m_chnet_CLEANFILES			= m_chnet ${m_chnet_OFILES}
m_chnet_LIBS					= -lodm -lnim -lmstr
m_chnet_OFILES					= m_chnet.o

m_chres_CLEANFILES			= m_chres ${m_chres_OFILES}
m_chres_LIBS					= -lodm -lnim -lmstr
m_chres_OFILES					= m_chres.o

m_chstate_CLEANFILES			= m_chstate ${m_chstate_OFILES}
m_chstate_LIBS					= -lodm -lnim -lmstr
m_chstate_OFILES				= m_chstate.o

m_ckmac_CLEANFILES			= m_ckmac ${m_ckmac_OFILES}
m_ckmac_LIBS					= -lodm -lnim -lmstr
m_ckmac_OFILES					= m_ckmac.o

m_ckspot_CLEANFILES			= m_ckspot ${m_ckspot_OFILES}
m_ckspot_LIBS					= -lodm -lnim -lmstr
m_ckspot_OFILES				= m_ckspot.o

m_ck_lpp_source_CLEANFILES	= m_ck_lpp_source ${m_ck_lpp_source_OFILES}
m_ck_lpp_source_LIBS			= -lodm -lnim -lmstr
m_ck_lpp_source_OFILES		= m_ck_lpp_source.o

m_dealloc_boot_CLEANFILES	= m_dealloc_boot ${m_dealloc_boot_OFILES}
m_dealloc_boot_LIBS			= -lodm -lnim -lmstr
m_dealloc_boot_OFILES		= m_dealloc_boot.o

m_dealloc_ns_CLEANFILES		= m_dealloc_ns ${m_dealloc_ns_OFILES}
m_dealloc_ns_LIBS				= -lodm -lnim -lmstr
m_dealloc_ns_OFILES			= m_dealloc_ns.o

m_dealloc_pdir_CLEANFILES	= m_dealloc_pdir ${m_dealloc_pdir_OFILES}
m_dealloc_pdir_LIBS			= -lodm -lnim -lmstr
m_dealloc_pdir_OFILES		= m_dealloc_pdir.o

m_dealloc_spot_CLEANFILES	= m_dealloc_spot ${m_dealloc_spot_OFILES}
m_dealloc_spot_LIBS			= -lodm -lnim -lmstr
m_dealloc_spot_OFILES		= m_dealloc_spot.o

m_deallocate_CLEANFILES		= m_deallocate ${m_deallocate_OFILES}
m_deallocate_LIBS				= -lodm -lnim -lmstr
m_deallocate_OFILES			= m_deallocate.o

m_destroy_res_CLEANFILES	= m_destroy_res ${m_destroy_res_OFILES}
m_destroy_res_LIBS			= -lodm -lnim -lmstr
m_destroy_res_OFILES			= m_destroy_res.o

m_fixquery_CLEANFILES			= m_fixquery ${m_fixquery_OFILES}
m_fixquery_LIBS					= -lodm -lnim -lmstr
m_fixquery_OFILES				= m_fixquery.o

m_getdate_LIBS					= -lodm -lnim -lmstr
m_getdate_OFILES 				= m_getdate.o

m_instspot_CLEANFILES		= m_instspot ${m_instspot_OFILES}
m_instspot_LIBS				= -lodm -lnim -lmstr
m_instspot_OFILES				= m_instspot.o

m_lslpp_CLEANFILES			= m_lslpp ${m_lslpp_OFILES}
m_lslpp_LIBS					= -lodm -lnim -lmstr
m_lslpp_OFILES					= m_lslpp.o

m_ls_lpp_source_CLEANFILES	= m_ls_lpp_source ${m_ls_lpp_source_OFILES}
m_ls_lpp_source_LIBS			= -lodm -lnim -lmstr
m_ls_lpp_source_OFILES		= m_ls_lpp_source.o

m_mkbosi_OFILES				= m_mkbosi.o
m_mkbosi_LIBS					= -lodm -lnim -lmstr
m_mkbosi_CLEANFILES			= m_mkbosi ${m_mkbosi_OFILES}

m_mkmac_CLEANFILES			= m_mkmac ${m_mkmac_OFILES}
m_mkmac_LIBS					= -lodm -lnim -lmstr
m_mkmac_OFILES					= m_mkmac.o

m_mknet_CLEANFILES			= m_mknet ${m_mknet_OFILES}
m_mknet_LIBS					= -lodm -lnim -lmstr
m_mknet_OFILES					= m_mknet.o

m_mkpdir_CLEANFILES			= m_mkpdir ${m_mkpdir_OFILES}
m_mkpdir_LIBS					= -lodm -lnim -lmstr
m_mkpdir_OFILES				= m_mkpdir.o

m_mkres_CLEANFILES			= m_mkres ${m_mkres_OFILES}
m_mkres_LIBS					= -lodm -lnim -lmstr
m_mkres_OFILES					= m_mkres.o

m_mk_lpp_source_CLEANFILES	= m_mk_lpp_source ${m_mk_lpp_source_OFILES}
m_mk_lpp_source_LIBS			= -lodm -lnim -lmstr
m_mk_lpp_source_OFILES		= m_mk_lpp_source.o

m_mkspot_CLEANFILES			= m_mkspot ${m_mkspot_OFILES}
m_mkspot_LIBS					= -lodm -lnim -lmstr
m_mkspot_OFILES				= m_mkspot.o

m_reboot_CLEANFILES		= m_reboot ${m_reboot_OFILES}
m_reboot_LIBS			= -lodm -lnim -lmstr
m_reboot_OFILES			= m_reboot.o

m_reset_res_CLEANFILES		= m_reset_res ${m_reset_res_OFILES}
m_reset_res_LIBS				= -lodm -lnim -lmstr
m_reset_res_OFILES			= m_reset_res.o

m_rmmac_CLEANFILES			= m_rmmac ${m_rmmac_OFILES}
m_rmmac_LIBS					= -lodm -lnim -lmstr
m_rmmac_OFILES					= m_rmmac.o

m_rmnet_CLEANFILES			= m_rmnet ${m_rmnet_OFILES}
m_rmnet_LIBS					= -lodm -lnim -lmstr
m_rmnet_OFILES					= m_rmnet.o

m_rmpdir_CLEANFILES			= m_rmpdir ${m_rmpdir_OFILES}
m_rmpdir_LIBS					= -lodm -lnim -lmstr
m_rmpdir_OFILES				= m_rmpdir.o

m_rmres_CLEANFILES			= m_rmres ${m_rmres_OFILES}
m_rmres_LIBS					= -lodm -lnim -lmstr
m_rmres_OFILES					= m_rmres.o

m_rmspot_CLEANFILES			= m_rmspot ${m_rmspot_OFILES}
m_rmspot_LIBS					= -lodm -lnim -lmstr
m_rmspot_OFILES				= m_rmspot.o


# ---------------------------- control methods

m_abort_CLEANFILES			= m_abort ${m_abort_OFILES}
m_abort_LIBS					= -lodm -lnim -lmstr
m_abort_OFILES					= m_abort.o

m_bos_inst_CLEANFILES		= m_bos_inst ${m_bos_inst_OFILES}
m_bos_inst_LIBS				= -lodm -lnim -lmstr
m_bos_inst_OFILES				= m_bos_inst.o

m_cust_CLEANFILES				= m_cust ${m_cust_OFILES}
m_cust_LIBS						= -lodm -lnim -lmstr
m_cust_OFILES					= m_cust.o

m_diag_CLEANFILES				= m_diag ${m_diag_OFILES}
m_diag_LIBS						= -lodm -lnim -lmstr
m_diag_OFILES					= m_diag.o

m_dkls_inst_CLEANFILES		= m_dkls_inst ${m_dkls_inst_OFILES}
m_dkls_inst_LIBS				= -lodm -lnim -lmstr
m_dkls_inst_OFILES			= m_dkls_inst.o dd_funcs.o

m_do_sop_CLEANFILES			= m_do_sop ${m_do_sop_OFILES}
m_do_sop_LIBS					= -lodm -lnim -lmstr
m_do_sop_OFILES				= m_do_sop.o

m_dtls_inst_CLEANFILES		= m_dtls_inst ${m_dtls_inst_OFILES}
m_dtls_inst_LIBS				= -lodm -lnim -lmstr
m_dtls_inst_OFILES			= m_dtls_inst.o dd_funcs.o

m_maint_CLEANFILES			= m_maint ${m_maint_OFILES}
m_maint_LIBS					= -lodm -lnim -lmstr
m_maint_OFILES					= m_maint.o

m_spotmaint_CLEANFILES		= m_spotmaint ${m_spotmaint_OFILES}
m_spotmaint_LIBS				= -lodm -lnim -lmstr
m_spotmaint_OFILES			= m_spotmaint.o

m_sync_roots_CLEANFILES		= m_sync_roots ${m_sync_roots_OFILES}
m_sync_roots_LIBS				= -lodm -lnim -lmstr
m_sync_roots_OFILES			= m_sync_roots.o

m_schedule_CLEANFILES		= m_schedule ${m_schedule_OFILES}
m_schedule_LIBS				= -lodm -lnim -lmstr
m_schedule_OFILES				= m_schedule.o


# ----------------------------      client methods    --------------------------

c_ckros_emu_OFILES			= c_ckros_emu.o
c_ckros_emu_LIBS				= -lnim
c_ckros_emu_CLEANFILES		= c_ckros_emu ${c_ckros_emu_OFILES}

c_ch_nfsexp_OFILES 			= c_ch_nfsexp.o
c_ch_nfsexp_LIBS 				= -lodm -lnim
c_ch_nfsexp_CLEANFILES		= c_ch_nfsexp ${c_ch_nfsexp_OFILES}

c_errmsg_OFILES 				= c_errmsg.o
c_errmsg_LIBS 					= -lodm -lnim
c_errmsg_CLEANFILES			= c_errmsg ${c_errmsg_OFILES}

c_mkpfile_OFILES 				= c_mkpfile.o
c_mkpfile_LIBS 				= -lodm -lnim
c_mkpfile_CLEANFILES			= c_mkpfile ${c_mkpfile_OFILES}

c_stat_OFILES 					= c_stat.o
c_stat_LIBS 					= -lodm -lnim
c_stat_CLEANFILES				= c_stat ${c_stat_OFILES}

c_nim_bootinfo32_OFILES 			= c_nim_bootinfo32.o
c_nim_bootinfo32_LIBS 				= -lodm -lcfg
c_nim_bootinfo32_CLEANFILES	= c_nim_bootinfo32 ${c_nim_bootinfo32_OFILES}

c_nim_mkboot32_OFILES 			= c_nim_mkboot32.o
c_nim_mkboot32_LIBS 				= -lodm -lcfg
c_nim_mkboot32_CLEANFILES	= c_nim_mkbootfo32 ${c_nim_mkboot32_OFILES}

.include <${RULES_MK}>

