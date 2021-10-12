/* @(#)90	1.1  src/bos/usr/ccs/lib/libi18n/defaults.h, libi18n, bos411, 9428A410j 8/24/93 11:01:38 */
/*
 *   COMPONENT_NAME: LIBI18N
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
typedef struct
              {
                LayoutTextDescriptor Orient;
                char *temp_buf;
                size_t temp_count;
                size_t temp_index;
              } DefaultValuesCoreRec, *DefaultValuesRec;

