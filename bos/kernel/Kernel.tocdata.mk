# @(#)45        1.3  src/bos/kernel/Kernel.tocdata.mk, kernel, bos41J, 9517A_all 4/25/95 17:46:50
# 
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

DATA_IN_TOC = -qdebug=tocdata -qdatalocal \
-qdataimported=sys_resource:TCE_space:proc:thread:lock_pinned:lock_pageable \
-qdataimported=family_lock_statistics:lockl_hstat:problem_lock:vmmdseg:ptaseg \
-qdataimported=__ublock:environ:_environ:errno:_errno:errnop:_errnop:csa:cs \
-qdataimported=curthread:g_kxsrval:runrun:key_value:pwr_obj_end:pin_dbg_end \
-qdataimported=ipl_cb:g_ksrval:svc_instruction:svc_table:svc_table_entries \
-qdataimported=init_obj_end:pg_obj_end:g_toc:pin_com_start:proc_arr_addr \
-qdataimported=ipl_cb_ext:nonpriv_page:pin_obj_end:pin_obj_start:pg_com_end \
-qdataimported=dbg_pinned:endcomm:pwr_com_end:pin_dbgcom_end:pg_com_end \
-qdataimported=ram_disk_end:ram_disk_start:base_conf_end:base_conf_start \
-qdataimported=pin_com_end:cmp_swap_index:Trconflag:_system_configuration \
-qdataimported=ppda:fp_owner:alignlo:alignhi:endtoc:tb_Xmask:fetchprot_page \
-qdataimported=intctl_pri:intctl_sec:mpic_base
