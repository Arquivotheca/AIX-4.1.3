/* @(#)81       1.2  src/bos/kernext/disp/ped/pedmacro/hw_hooks.h, pedmacro, bos411, 9428A410j 3/17/93 19:27:10 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_MID_HOOKS
#define _H_MID_HOOKS

typedef   struct
{
  char    id;
  int     hook;
} HOOKDEF;

/****************************************************************************/
/*                         BIM Host Register Names                          */
/****************************************************************************/
static  HOOKDEF   hookdefs[] = {
          {MID_TRACE_ID_TIME_STAMP  , MID_TRACE_HOOK_TIME_STAMP  },
          {MID_TRACE_ID_FIFO_BUFFER , MID_TRACE_HOOK_FIFO_BUFFER },
          {MID_TRACE_ID_PCB_BUFFER  , MID_TRACE_HOOK_PCB_BUFFER  },
          {MID_TRACE_ID_IRD_BUFFER  , MID_TRACE_HOOK_IRD_BUFFER  },
          {MID_TRACE_ID_IWR_BUFFER  , MID_TRACE_HOOK_IWR_BUFFER  },
          {MID_TRACE_ID_TED_BUFFER  , MID_TRACE_HOOK_TED_BUFFER  },
          {MID_TRACE_ID_HCR_BUFFER  , MID_TRACE_HOOK_HCR_BUFFER  },
          {MID_TRACE_ID_READ_REG    , MID_TRACE_HOOK_READ_REG    },
          {MID_TRACE_ID_WRITE_REG   , MID_TRACE_HOOK_WRITE_REG   },
          {MID_TRACE_ID_FREE_SPACE  , MID_TRACE_HOOK_FREE_SPACE  },
          {MID_TRACE_ID_PCB_FREE    , MID_TRACE_HOOK_PCB_FREE    },
          {MID_TRACE_ID_LOAD_ADDRESS, MID_TRACE_HOOK_LOAD_ADDRESS},
          {MID_TRACE_ID_DELAY       , MID_TRACE_HOOK_DELAY       },
          {MID_TRACE_ID_ECHO_COMMENT, MID_TRACE_HOOK_ECHO_COMMENT},
          {MID_TRACE_ID_COMMENT     , MID_TRACE_HOOK_COMMENT     },

          {MID_TRACE_ID_GET_POS     , MID_TRACE_HOOK_GET_POS     },
          {MID_TRACE_ID_PUT_POS     , MID_TRACE_HOOK_PUT_POS     },

          {MID_TRACE_ID_SYSTEM_FILE , MID_TRACE_HOOK_SYSTEM_FILE },
          {MID_TRACE_ID_ASCII_FILE  , MID_TRACE_HOOK_ASCII_FILE  },
          {MID_TRACE_ID_BINARY_FILE , MID_TRACE_HOOK_BINARY_FILE },
          {MID_TRACE_ID_ASCII_SCREEN, MID_TRACE_HOOK_ASCII_SCREEN},
          {MID_TRACE_ID_SINGLE_STEP , MID_TRACE_HOOK_SINGLE_STEP },
          {MID_TRACE_ID_RING_BELL   , MID_TRACE_HOOK_RING_BELL   },
          {MID_TRACE_ID_NAMES       , MID_TRACE_HOOK_NAMES       },

          {MID_TRACE_ID_VALUE       , MID_TRACE_HOOK_VALUE       },
          {MID_TRACE_ID_EQUAL       , MID_TRACE_HOOK_EQUAL       },
          {MID_TRACE_ID_NOT_EQUAL   , MID_TRACE_HOOK_NOT_EQUAL   },
          {MID_TRACE_ID_LESS_EQUAL  , MID_TRACE_HOOK_LESS_EQUAL  },
          {MID_TRACE_ID_MORE_EQUAL  , MID_TRACE_HOOK_MORE_EQUAL  }};

static  int     hookcnt = sizeof(hookdefs) / sizeof(HOOKDEF);

#endif /* _H_MID_HOOKS */
