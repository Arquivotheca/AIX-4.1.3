static char sccsid[] = "@(#)44  1.5  src/bos/diag/tu/swmono/Sig.c, tu_swmono, bos411, 9428A410j 1/28/94 13:49:01";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: ARRAY_TO_INT
 *		ARRAY_TO_SHORT
 *		INT_TO_ARRAY
 *		SHORT_TO_ARRAY
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "skytu.h"
#include "exectu.h"
#include <entdisp.h>
#include <errno.h>

extern errno;

#define ERROR      -1

#define INT_TO_ARRAY(a,i)   *((char *) (a)    ) = (i) >> 24;\
	 		    *((char *) (a) + 1) = (i) >> 16;\
		 	    *((char *) (a) + 2) = (i) >>  8;\
	                    *((char *) (a) + 3) = (i);

#define SHORT_TO_ARRAY(a,i) *((char *) (a))     = (i) >>  8;\
			    *((char *) (a) + 1) = (i);

#define ARRAY_TO_INT(i,a)   i = (*((char *) (a)    ) << 24) |    \
	                        (*((char *) (a) + 1) << 16) |    \
	                        (*((char *) (a) + 2) <<  8) |    \
	                        (*((char *) (a) + 3)      ) ;

#define ARRAY_TO_SHORT(i,a) i = (*((char *) (a) + 2) <<  8) |    \
	                        (*((char *) (a) + 3)      ) ;

struct sky_map skydat;
