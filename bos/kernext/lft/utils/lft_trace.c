static char sccsid[] = "@(#)82	1.1  src/bos/kernext/lft/utils/lft_trace.c, lftdd, bos411, 9428A410j 10/20/93 17:08:50";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
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

#define GS_DEFINE_MACRO_EXTERNS

#include <graphics/gs_trace.h>
#include <lft_debug.h>

GS_TRC_GLB(0,99);

GS_TRC_MODULE(lftconfig, 2);
GS_TRC_MODULE(lftinit, 2);
GS_TRC_MODULE(lftterm, 2);
GS_TRC_MODULE(lftfonts, 2);
GS_TRC_MODULE(lftioctl, 2);
GS_TRC_MODULE(lftsi, 2);
GS_TRC_MODULE(lftki, 2);
GS_TRC_MODULE(lftsicfg, 2);
GS_TRC_MODULE(lftst, 2);
GS_TRC_MODULE(lftte, 2);
GS_TRC_MODULE(lftvi, 2);
GS_TRC_MODULE(fkproc, 2);
GS_TRC_MODULE(fsqueue, 2);
GS_TRC_MODULE(kernel_ftok, 2);

