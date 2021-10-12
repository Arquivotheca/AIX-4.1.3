/* @(#)31       1.1  R2/cmd/diag/tu/corv/cfgad.h, tu_corv, bos325 7/22/93 18:57:16 */
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * DEBUGGING AIDS
 */

#ifdef DEBUG
#include <stdio.h>
#define DEBUG_0(A)                      {fprintf(stderr,A);fflush(stderr);}
#define DEBUG_1(A,B)                    {fprintf(stderr,A,B);fflush(stderr);}
#define DEBUG_2(A,B,C)                  {fprintf(stderr,A,B,C);fflush(stderr);}
#define DEBUG_3(A,B,C,D)                {fprintf(stderr,A,B,C,D);fflush(stderr);}
#define DEBUG_4(A,B,C,D,E)              {fprintf(stderr,A,B,C,D,E);fflush(stderr);}
#define DEBUG_5(A,B,C,D,E,F)            {fprintf(stderr,A,B,C,D,E,F);fflush(stderr);}
#define DEBUG_6(A,B,C,D,E,F,G)          {fprintf(stderr,A,B,C,D,E,F,G);fflush(stderr);}
#define DEBUG_7(A)                      {printf(A);}
#define DEBUG_8(A,B)                    {printf(A,B);}
#define DEBUG_9(A,B,C)                  {printf(A,B,C);}
#define DEBUGELSE                       else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUG_7(A)
#define DEBUG_8(A,B)
#define DEBUG_9(A,B,C)
#define DEBUGELSE
#endif

struct attr {
        char *attribute;
        char *value;
};
