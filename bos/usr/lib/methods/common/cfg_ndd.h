/* @(#)18  1.1  src/bos/usr/lib/methods/common/cfg_ndd.h, cfgmethods, bos411, 9428A410j 8/5/93 10:24:36 */
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   cfg_ndd.h - Generic Config Include Code (for NDD Devices)
 */

/* defines for commands to method for additional drivers */
#define ADDL_CFG        0
#define ADDL_UCFG       1

/* structure for passing odm pointers to subroutines */
	struct  ndd_cfg_odm {
	    struct Class *cusdev;       /* customized devices class ptr */
	    struct Class *predev;       /* predefined devices class ptr */
	    struct Class *cusatt;       /* customized attributes class ptr */
	    struct Class *preatt;       /* predefined attributes class ptr */

	    struct CuDv cusobj;         /* device customized object storate */
	    struct PdDv preobj;         /* device predefined object storage */
	    struct CuDv parobj;         /* parents customized object storage*/
	} ;

typedef struct  ndd_cfg_odm    ndd_cfg_odm_t;

