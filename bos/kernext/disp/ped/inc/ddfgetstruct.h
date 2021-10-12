/* @(#)55	1.3  src/bos/kernext/disp/ped/inc/ddfgetstruct.h, peddd, bos411, 9428A410j 12/2/93 09:42:35 */
#ifndef _H_DDFGETSTRUCT
#define _H_DDFGETSTRUCT

/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

	/*----------------------------------------------------------
        This is where the DDF arrays are defined.
          The leading defines specify the array sizes. 
	----------------------------------------------------------*/

#	define ARY_SIZE 4

        ddf_data_getcolor_t	ddf_data_getcolor[ARY_SIZE];

        ddf_data_getcpos_t	ddf_data_getcpos[ARY_SIZE];

        ddf_data_get_projection_matrix_t
				ddf_data_get_projection_matrix[ARY_SIZE];
        ddf_data_get_modelling_matrix_t 
				ddf_data_get_modelling_matrix[ARY_SIZE];
        ddf_data_getcondition_t ddf_data_getcondition[ARY_SIZE];

        ddf_data_gettextfontindex_t 
				ddf_data_gettextfontindex[ARY_SIZE];
	ddf_data_t		ddf_data_endrender[ARY_SIZE];

	/*-----------------
        correlator count used to track last value of correlator      
	-----------------*/
        ushort          correlator_count;


#endif /* _H_DDFGETSTRUCT */
