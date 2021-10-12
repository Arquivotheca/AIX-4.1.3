#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)93	1.16  src/bos/usr/ccs/bin/ld/bind/global.c, cmdld, bos41B, 9504A 12/7/94 15:44:53")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/shm.h>
#include <aouthdr.h>

#include "bind.h"
#include "global.h"

#ifdef XLC_TYPCHK_BUG
#include "error.h"
#endif

#if DEBUG || XLC_TYPCHK_BUG
#include "strs.h"
#endif

/* global state information */
struct bind_state	Bind_state = {
#ifdef STATS
    {{0},},				/* Memory */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Files statistics */
#endif
    0, 0, STATE_RESOLVE_NEEDED,		/* Flags loadmap_err state */
    0,					/* sections_seen */
    0, 0,				/* ever_interactive interactive */
    0, 1,				/* stdin_is_tty stdout_is_tty */
    0,					/* cmd_err_lev */
    4, 4,				/* err_exit_lev(halt) err_show_lev */
    0, "(ld): ",			/* Max return code, prompt */
    NULL,				/* tty_file */
    NULL				/* Initialized to STR for "__start" */,
    NULL,				/* entrypoint symbol, if found */
    NULL, 0,				/* libpath, libpath_len */
    0,					/* num_data_in_toc_fixups */
    0, 0, 0,				/* long_strings typchks C_FILE_strs */
    0,					/* num_glue_symbols */
    0, 0, 0,				/* Alignments (.text .data .loader) */
    NULL, 0,				/* TOC anchor TOC size */
    NULL, NULL,				/* ldrinfo order_file */
    0, 0,				/* generated_symbols, num_exports */
    0,					/* files_kept */
    0, NULL, 0,				/* out_Fd out_name temp_out_Fd */
    NULL, 0,				/* out_tmp_name out_writing */
    NULL,				/* loadmap_fn */
    					/* machine instructions: */
    0, 0, NULL,				/* num_nops     max_nops     nops */
    0, 0, NULL				/* num_loadtocs max_loadtocs loadtocs*/
#ifdef DEBUG
	, 'a'				/* Any value */
#endif
};

AOUTHDR	temp_aout_hdr = {
    0x10b,				/* Magic number */
    1,					/* version */
    0, 0, 0,				/* Section sizes */
    -1,					/* Entry point address */
    0, 0,				/* Base section addresses */
    -1,					/* TOC address */
    0, 0, 0, 0, 0, 0,			/* Section #s */
    0, 0,				/* Alignments */
    {' ', ' '},				/* Module type */
#ifdef _CPUTYPE_FEATURE
    TOBJ_SAME, '\0',			/* cpuflag, cputype */
#else
    '\0', '\0',				/* cpuflag, cputype */
#endif
    0, 0};				/* Max stack/data */

struct switches	Switches = {
    0, 0, 1, 0, 1,			/* quiet loadmap ASIS verbose ERRMSG */
    0,					/* skip_if_shared */

#if CENTERLINE && I_ACCESS_MALLOC
    I_ACCESS_MALLOC,			/* input_opt */
#else
    I_ACCESS_SHMAT,			/* input_opt */
#endif

    1, 0, 0,				/* AUTOIMP del_csect keepall */
    1, 0, 0, 1,				/* HASH_CHK loadabs member EXECUTE */
    0, 0, 0, 0,				/* strip local ro_text bigtoc */
    REORDER_DFS,			/* reorder */
    0, MAPPING_NORMAL,			/* fold mapping */
    STABCMPCT_NODUPS, POWER_TARGET,	/* stabcmpct, target */
    0,0,0,0, 0,0,0,0, 0,		/* debug options */
#ifdef DEBUG
    1,					/* dbg_opt9 */
#else
    0,					/* dbg_opt9 */
#endif
    0, 0				/* dbg_opt10, dbg_opt11 */
#ifdef DEBUG
	, &NULL_STR			/* Any STR value */
#endif
};
