/* @(#)15       1.2  src/bos/diag/tu/lega/host_debug.h, tu_lega, bos411, 9428A410j 11/11/91 18:16:04 */
/*
 * COMPONENT_NAME: (tu_lega) Low End Graphics Adapter Test Units
 *
 * FUNCTIONS: Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * MODULE NAME: host_debug.h
 *
 * STATUS: Release 1, EC 00, EVT Version 1
 *
 * DEPENDENCIES: None
 *
 * RESTRICTIONS: None
 *
 * EXTERNAL REFERENCES:
 *
 *      OTHER ROUTINES: None
 *
 *      DATA AREAS: None
 *
 *      TABLES: None
 *
 *      MACROS: None
 *
 * COMPILER/ASSEMBLER
 *
 *      TYPE, VERSION: AIX C Compiler
 *
 *      OPTIONS:
 *
 * NOTES:  This file contains :
 *         1. MACROS used for debug of host code.
 *
 * CHANGE ACTIVITIES:
 *
 *    EC00, VERSION 00 (ORIGINAL), 5/23/91 !KM!
 *
 */


#ifdef DEBUG
#define PRINT(arg) printf arg
#else
#define PRINT
#endif

