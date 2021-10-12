/* @(#)19       1.4.2.3  src/bos/kernext/disp/ped/pedmacro/hw_names.h, pedmacro, bos411, 9428A410j 3/23/94 17:07:12 */

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
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_MID_HW_NAMES
#define _H_MID_HW_NAMES

typedef   struct
{
  char    id;
  int     opcode;
} ID_DEF;

typedef   struct
{
  int     adr;
  char    *name;
  int     namelen;
} NAMDEF, *NAMPTR;


/****************************************************************************/
/*                         BIM Host Register Names                          */
/****************************************************************************/

static    NAMDEF   regnames[] = {
	  {MID_ADR_FIFO_DATA    ,"FIFO_DATA"    ,sizeof("FIFO_DATA"    ) - 1},
	  {MID_ADR_FREE_SPACE   ,"FREE_SPACE"   ,sizeof("FREE_SPACE"   ) - 1},

	  {MID_ADR_PCB_DATA     ,"PCB_DATA"     ,sizeof("PCB_DATA"     ) - 1},

	  {MID_ADR_IND_CONTROL  ,"IND_CONTROL"  ,sizeof("IND_CONTROL"  ) - 1},
	  {MID_ADR_IND_ADDRESS  ,"IND_ADDRESS"  ,sizeof("IND_ADDRESS"  ) - 1},
	  {MID_ADR_IND_DATA     ,"IND_DATA"     ,sizeof("IND_DATA"     ) - 1},

	  {MID_ADR_DSP_CONTROL  ,"DSP_CONTROL"  ,sizeof("DSP_CONTROL"  ) - 1},
	  {MID_ADR_CARD_INT_MASK,"CARD_INT_MASK",sizeof("CARD_INT_MASK") - 1},
	  {MID_ADR_HOST_INT_MASK,"HOST_INT_MASK",sizeof("HOST_INT_MASK") - 1},
	  {MID_ADR_HOST_STATUS  ,"HOST_STATUS"  ,sizeof("HOST_STATUS"  ) - 1},
	  {MID_ADR_CARD_COMMO   ,"CARD_COMMO"   ,sizeof("CARD_COMMO"   ) - 1},
	  {MID_ADR_HOST_COMMO   ,"HOST_COMMO"   ,sizeof("HOST_COMMO"   ) - 1}};

static    int     namcnt = sizeof(regnames) / sizeof(NAMDEF);


/****************************************************************************/
/*                          BIM POS Register Names                          */
/****************************************************************************/

static    NAMDEF   posnames[] = {
	  {MID_ADR_POS_REG_0,"POS_REG_0",sizeof("POS_REG_0") - 1},
	  {MID_ADR_POS_REG_1,"POS_REG_1",sizeof("POS_REG_1") - 1},
	  {MID_ADR_POS_REG_2,"POS_REG_2",sizeof("POS_REG_2") - 1},
	  {MID_ADR_POS_REG_3,"POS_REG_3",sizeof("POS_REG_3") - 1},
	  {MID_ADR_POS_REG_4,"POS_REG_4",sizeof("POS_REG_4") - 1},
	  {MID_ADR_POS_REG_5,"POS_REG_5",sizeof("POS_REG_5") - 1},
	  {MID_ADR_POS_REG_6,"POS_REG_6",sizeof("POS_REG_6") - 1},
	  {MID_ADR_POS_REG_7,"POS_REG_7",sizeof("POS_REG_7") - 1}};

static    int     poscnt = sizeof(posnames) / sizeof(NAMDEF);


/****************************************************************************/
/*                       Host Commo Return Code Names                       */
/****************************************************************************/

static    NAMDEF   hcrnames[] = {
	  {HARDWARE_ERROR         ,"HARDWARE ERROR"                ,sizeof("HARDWARE ERROR"                ) - 1},
	  {MICROCODE_CRC_FAILED   ,"MICROCODE CRC FAILED"          ,sizeof("MICROCODE CRC FAILED"          ) - 1},
	  {BLAST_ERROR            ,"BLAST ERROR"                   ,sizeof("BLAST ERROR"                   ) - 1},
	  {BLAST_PROC_ERROR       ,"BLAST PROCESSOR ERROR"         ,sizeof("BLAST PROCESSOR ERROR"         ) - 1},
	  {PIPE_PROC1_ERROR       ,"PIPE PROCESSOR 1 ERROR"        ,sizeof("PIPE PROCESSOR 1 ERROR"        ) - 1},
	  {PIPE_PROC2_ERROR       ,"PIPE_PROCESSOR 2 ERROR"        ,sizeof("PIPE_PROCESSOR 2 ERROR"        ) - 1},
	  {PIPE_PROC3_ERROR       ,"PIPE_PROCESSOR 3 ERROR"        ,sizeof("PIPE_PROCESSOR 3 ERROR"        ) - 1},
	  {PIPE_PROC4_ERROR       ,"PIPE_PROCESSOR 4 ERROR"        ,sizeof("PIPE_PROCESSOR 4 ERROR"        ) - 1},

	  {DATA_STREAM_ERROR      ,"DATA STREAM ERROR"             ,sizeof("DATA STREAM ERROR"             ) - 1},
	  {INVALID_SE_LENGTH      ,"INVALID SE LENGTH"             ,sizeof("INVALID SE LENGTH"             ) - 1},
	  {NON_RECOVERABLE_ER     ,"NON RECOVERABLE ERR: OP=%4.4X" ,sizeof("NON RECOVERABLE ERR: OP=XXXX"  ) - 1},
	  {INVALID_CONTEXT_ID     ,"INVALID CONTEXT ID"            ,sizeof("INVALID CONTEXT ID"            ) - 1},
	  {INVALID_REQUEST        ,"INVALID REQUEST"               ,sizeof("INVALID REQUEST"               ) - 1},
	  {CONFIG_NOT_SUPPORTED   ,"CONFIGURATION NOT SUPPORTED"   ,sizeof("CONFIGURATION NOT SUPPORTED"   ) - 1},
	  {DMA_COMPLETE_ERROR     ,"DMA COMPLETE ERROR: CORR=%4.4X",sizeof("DMA COMPLETE ERROR: CORR=XXXX" ) - 1},

	  {RESET_COMPLETE         ,"RESET COMPLETE"                ,sizeof("RESET COMPLETE"                ) - 1},
	  {MICROCODE_CRC_PASSED   ,"MICROCODE CRC PASSED"          ,sizeof("MICROCODE CRC PASSED"          ) - 1},
	  {SOFT_RESET_IN_PROGRESS ,"SOFT_RESET_IN_PROGRESS"        ,sizeof("SOFT_RESET_IN_PROGRESS"        ) - 1},
	  {INQUIRE_DATA_AVAIL     ,"INQUIRE DATA AVAIL: CORR=%4.4X",sizeof("INQUIRE DATA AVAIL: CORR=XXXX" ) - 1},
	  {PICK_COMPLETE          ,"PICK COMPLETE: CORR=%4.4X"     ,sizeof("PICK COMPLETE: CORR=XXXX"      ) - 1},
	  {DMA_COMPLETE           ,"DMA COMPLETE: CORR=%4.4X"      ,sizeof("DMA COMPLETE: CORR=XXXX"       ) - 1},
	  {CTX_SAVE_COMPLETE      ,"CONTEXT SAVE COMPLETE"         ,sizeof("CONTEXT SAVE COMPLETE"         ) - 1},
	  {NEW_CTX_COMPLETE       ,"NEW CONTEXT COMPLETE"          ,sizeof("NEW CONTEXT COMPLETE"          ) - 1},
	  {FONT_REQUEST           ,"FONT REQUEST"                  ,sizeof("FONT REQUEST"                  ) - 1},
	  {PIN_CTX_MEM_REQ        ,"PIN CONTEXT MEMORY REQUEST"    ,sizeof("PIN CONTEXT MEMORY REQUEST"    ) - 1},
	  {SYNC_RESPONSE          ,"SYNC RESPONSE: CORR=%4.4X"     ,sizeof("SYNC RESPONSE: CORR=XXXX"      ) - 1},
	  {FIFO_STALLED           ,"FIFO STALLED: CORR=%4.4X"      ,sizeof("FIFO STALLED: CORR=XXXX"       ) - 1},
	  {FRAME_BUF_SWAPPED      ,"FRAME BUFFER SWAPPED"          ,sizeof("FRAME BUFFER SWAPPED"          ) - 1},
	  {END_RENDERING_3DM1     ,"3DM1 END RENDERING"            ,sizeof("3DM1 END RENDERING"            ) - 1},
	  {HI_CTX_DMA_COMPLETE    ,"HOST CTX DMA COMP: CORR=%4.4X" ,sizeof("HOST CTX DMA COMP: CORR=XXXX"  ) - 1},
	  {AI_CTX_DMA_COMPLETE    ,"CARD CTX DMA COMP: CORR=%4.4X" ,sizeof("CARD CTX DMA COMP: CORR=XXXX"  ) - 1},
	  {CTX_DMA_PARM_ERROR     ,"CONTEXT DMA PARAMETER ERROR"   ,sizeof("CONTEXT DMA PARAMETER ERROR"   ) - 1}};

static    int     hcrcnt = sizeof(hcrnames) / sizeof(NAMDEF);


/****************************************************************************/
/*                     DMA Address Field Identification                     */
/****************************************************************************/

static    NAMDEF   dmanames[] = {
	  {FIFO_DMA_ID         ,"FIFO DMA"         ,sizeof("FIFO DMA"        ) - 1},
	  {READ_BLIT_DMA_ID    ,"READ BLIT DMA"    ,sizeof("READ BLIT DMA"   ) - 1},
	  {READ_BLIT_DMA_ID_2  ,"READ BLIT DMA 2"  ,sizeof("READ BLIT DMA 2" ) - 1},
	  {WRITE_BLIT_DMA_ID   ,"WRITE BLIT DMA"   ,sizeof("WRITE BLIT DMA"  ) - 1},
	  {WRITE_BLIT_DMA_ID_2 ,"WRITE BLIT DMA 2" ,sizeof("WRITE BLIT DMA 2") - 1},
	  {DMA_SE_DMA_ID       ,"DMA SE DMA"       ,sizeof("DMA SE DMA"      ) - 1},
	  {FONT_DMA_ID         ,"FONT DMA"         ,sizeof("FONT DMA"        ) - 1},
	  {PICK_DMA_ID         ,"PICK DMA"         ,sizeof("PICK DMA"        ) - 1},
	  {CONTEXT_RD_DMA_ID   ,"CONTEXT RD DMA"   ,sizeof("CONTEXT RD DMA"  ) - 1},
	  {CONTEXT_WR_DMA_ID   ,"CONTEXT WR DMA"   ,sizeof("CONTEXT WR DMA"  ) - 1},
	  {CONTEXT_MV_DMA_ID   ,"CONTEXT MV DMA"   ,sizeof("CONTEXT MV DMA"  ) - 1},
	  {TRACE_DMA_ID        ,"TRACE DMA"        ,sizeof("TRACE DMA"       ) - 1}};

static    int     dmacnt = sizeof(dmanames) / sizeof(NAMDEF);


/****************************************************************************/
/*                   Adapter Status Control Block Names                     */
/****************************************************************************/

static    NAMDEF   ascbnames[] = {
	  {MID_ASCB_BASE + MID_ASCB_MICROCODE_LEVEL                ,"MICROCODE LEVEL"              ,sizeof("MICROCODE LEVEL"              ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_EXPECTED_CRC_VALUE             ,"EXPECTED CRC VALUE"           ,sizeof("EXPECTED CRC VALUE"           ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_ACTUAL_CRC_VALUE               ,"ACTUAL CRC VALUE"             ,sizeof("ACTUAL CRC VALUE"             ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_LAST_FIFO_DATA_LENGTH          ,"LAST FIFO DATA LENGTH"        ,sizeof("LAST FIFO DATA LENGTH"        ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SE_LENGTH                      ,"SE FIELD LENGTH"              ,sizeof("SE FIELD LENGTH"              ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_FBCSB                      ,"FRAME BUFFER CONTROL STAT BLK",sizeof("FRAME BUFFER CONTROL STAT BLK") - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB                        ,"CURRENT CONTEXT STATUS BLOCK" ,sizeof("CURRENT CONTEXT STATUS BLOCK" ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 0                  ,"CONTEXT STATUS BLOCK 0"       ,sizeof("CONTEXT STATUS BLOCK 0"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 1                  ,"CONTEXT STATUS BLOCK 1"       ,sizeof("CONTEXT STATUS BLOCK 1"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 2                  ,"CONTEXT STATUS BLOCK 2"       ,sizeof("CONTEXT STATUS BLOCK 2"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 3                  ,"CONTEXT STATUS BLOCK 3"       ,sizeof("CONTEXT STATUS BLOCK 3"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 4                  ,"CONTEXT STATUS BLOCK 4"       ,sizeof("CONTEXT STATUS BLOCK 4"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 5                  ,"CONTEXT STATUS BLOCK 5"       ,sizeof("CONTEXT STATUS BLOCK 5"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 6                  ,"CONTEXT STATUS BLOCK 6"       ,sizeof("CONTEXT STATUS BLOCK 6"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 7                  ,"CONTEXT STATUS BLOCK 7"       ,sizeof("CONTEXT STATUS BLOCK 7"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 8                  ,"CONTEXT STATUS BLOCK 8"       ,sizeof("CONTEXT STATUS BLOCK 8"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 9                  ,"CONTEXT STATUS BLOCK 9"       ,sizeof("CONTEXT STATUS BLOCK 9"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 10                 ,"CONTEXT STATUS BLOCK 10"      ,sizeof("CONTEXT STATUS BLOCK 10"      ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 11                 ,"CONTEXT STATUS BLOCK 11"      ,sizeof("CONTEXT STATUS BLOCK 11"      ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 12                 ,"CONTEXT STATUS BLOCK 12"      ,sizeof("CONTEXT STATUS BLOCK 12"      ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 13                 ,"CONTEXT STATUS BLOCK 13"      ,sizeof("CONTEXT STATUS BLOCK 13"      ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 14                 ,"CONTEXT STATUS BLOCK 14"      ,sizeof("CONTEXT STATUS BLOCK 14"      ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + 15                 ,"CONTEXT STATUS BLOCK 15"      ,sizeof("CONTEXT STATUS BLOCK 15"      ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CMRB                       ,"CONTEXT MEMORY REQUEST BLOCK" ,sizeof("CONTEXT MEMORY REQUEST BLOCK" ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_FRB                        ,"FONT REQUEST BLOCK"           ,sizeof("FONT REQUEST BLOCK"           ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CCB                        ,"CONTEXT COMMAND BLOCK"        ,sizeof("CONTEXT COMMAND BLOCK"        ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CTCB_X + 0                 ,"COLOR TABLE 0 COMMAND BLOCK"  ,sizeof("COLOR TABLE 0 COMMAND BLOCK"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CTCB_X + 1                 ,"COLOR TABLE 1 COMMAND BLOCK"  ,sizeof("COLOR TABLE 1 COMMAND BLOCK"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CTCB_X + 2                 ,"COLOR TABLE 2 COMMAND BLOCK"  ,sizeof("COLOR TABLE 2 COMMAND BLOCK"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CTCB_X + 3                 ,"COLOR TABLE 3 COMMAND BLOCK"  ,sizeof("COLOR TABLE 3 COMMAND BLOCK"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CTCB_X + 4                 ,"COLOR TABLE 4 COMMAND BLOCK"  ,sizeof("COLOR TABLE 4 COMMAND BLOCK"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_CICB                       ,"CURSOR IMAGE COMMAND BLOCK"   ,sizeof("CURSOR IMAGE COMMAND BLOCK"   ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_TRACE_BEGIN                ,"MICROCODE TRACE BEGIN POINTER",sizeof("MICROCODE TRACE BEGIN POINTER") - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_TRACE_END                  ,"MICROCODE TRACE END POINTER"  ,sizeof("MICROCODE TRACE END POINTER"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_TRACE_NEXT                 ,"MICROCODE TRACE NEXT POINTER" ,sizeof("MICROCODE TRACE NEXT POINTER" ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_FUNCTION_TABLE             ,"MICROCODE FUNCTION TABLE PTR" ,sizeof("MICROCODE FUNCTION TABLE PTR" ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_LOADED_PIPES               ,"MICROCODE LOADED PIPES PTR"   ,sizeof("MICROCODE LOADED PIPES PTR"   ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PTR_PIPE_TABLE                 ,"MICROCODE PIPE TABLE PTR"     ,sizeof("MICROCODE PIPE TABLE PTR"     ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_VERSION                        ,"MICROCODE VERSION"            ,sizeof("MICROCODE VERSION"            ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_ROM_REVISION                   ,"MICROCODE ROM REVISION"       ,sizeof("MICROCODE ROM REVISION"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_HOST_DMA_BUSY                  ,"MICROCODE HOST DMA BUSY"      ,sizeof("MICROCODE HOST DMA BUSY"      ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_CURRENT_DMA_TYPE               ,"MICROCODE CURRENT DMA TYPE"   ,sizeof("MICROCODE CURRENT DMA TYPE"   ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_CORRELATOR_1                   ,"MICROCODE CORRELATOR 1"       ,sizeof("MICROCODE CORRELATOR 1"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_CORRELATOR_2                   ,"MICROCODE CORRELATOR 2"       ,sizeof("MICROCODE CORRELATOR 2"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_ERROR_CODE                     ,"MICROCODE ERROR CODE"         ,sizeof("MICROCODE ERROR CODE"         ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_PICK_ADDRESS                   ,"MICROCODE PICK ADDRESS"       ,sizeof("MICROCODE PICK ADDRESS"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECOND_BLAST                   ,"MICROCODE SECOND BLAST"       ,sizeof("MICROCODE SECOND BLAST"       ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_ELEMENT_COUNTER                ,"MICROCODE ELEMENT COUNTER"    ,sizeof("MICROCODE ELEMENT COUNTER"    ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_LOAD_SECTION_COUNT             ,"MICROCODE LOAD SECTION COUNT" ,sizeof("MICROCODE LOAD SECTION COUNT" ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_0                      ,"MICROCODE SECTION 0 ADDRESS"  ,sizeof("MICROCODE SECTION 0 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_1                      ,"MICROCODE SECTION 1 ADDRESS"  ,sizeof("MICROCODE SECTION 1 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_2                      ,"MICROCODE SECTION 2 ADDRESS"  ,sizeof("MICROCODE SECTION 2 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_3                      ,"MICROCODE SECTION 3 ADDRESS"  ,sizeof("MICROCODE SECTION 3 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_4                      ,"MICROCODE SECTION 4 ADDRESS"  ,sizeof("MICROCODE SECTION 4 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_5                      ,"MICROCODE SECTION 5 ADDRESS"  ,sizeof("MICROCODE SECTION 5 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_6                      ,"MICROCODE SECTION 6 ADDRESS"  ,sizeof("MICROCODE SECTION 6 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_7                      ,"MICROCODE SECTION 7 ADDRESS"  ,sizeof("MICROCODE SECTION 7 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_8                      ,"MICROCODE SECTION 8 ADDRESS"  ,sizeof("MICROCODE SECTION 8 ADDRESS"  ) - 1},
	  {MID_ASCB_BASE + MID_ASCB_SECTION_9                      ,"MICROCODE SECTION 9 ADDRESS"  ,sizeof("MICROCODE SECTION 9 ADDRESS"  ) - 1}};

static    int     ascbcnt = sizeof(ascbnames) / sizeof(NAMDEF);


/****************************************************************************/
/*                         System Trace Hook ID's                           */
/****************************************************************************/

static  ID_DEF   hookids[] = {
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
	  {MID_TRACE_ID_POLL_COUNT  , MID_TRACE_HOOK_POLL_COUNT  },
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

static  int     hookcnt = sizeof(hookids) / sizeof(ID_DEF);


/****************************************************************************/
/*                         System Trace Opcode ID's                         */
/****************************************************************************/

static  ID_DEF   opcids[] = {
	  {MID_TRACE_ID_TIME_STAMP  , MID_TRACE_OP_TIME_STAMP  },
	  {MID_TRACE_ID_FIFO_BUFFER , MID_TRACE_OP_FIFO_BUFFER },
	  {MID_TRACE_ID_PCB_BUFFER  , MID_TRACE_OP_PCB_BUFFER  },
	  {MID_TRACE_ID_IRD_BUFFER  , MID_TRACE_OP_IRD_BUFFER  },
	  {MID_TRACE_ID_IWR_BUFFER  , MID_TRACE_OP_IWR_BUFFER  },
	  {MID_TRACE_ID_TED_BUFFER  , MID_TRACE_OP_TED_BUFFER  },
	  {MID_TRACE_ID_HCR_BUFFER  , MID_TRACE_OP_HCR_BUFFER  },
	  {MID_TRACE_ID_READ_REG    , MID_TRACE_OP_READ_REG    },
	  {MID_TRACE_ID_WRITE_REG   , MID_TRACE_OP_WRITE_REG   },
	  {MID_TRACE_ID_FREE_SPACE  , MID_TRACE_OP_FREE_SPACE  },
	  {MID_TRACE_ID_PCB_FREE    , MID_TRACE_OP_PCB_FREE    },
	  {MID_TRACE_ID_POLL_COUNT  , MID_TRACE_OP_POLL_COUNT  },
	  {MID_TRACE_ID_DELAY       , MID_TRACE_OP_DELAY       },
	  {MID_TRACE_ID_ECHO_COMMENT, MID_TRACE_OP_ECHO_COMMENT},
	  {MID_TRACE_ID_COMMENT     , MID_TRACE_OP_COMMENT     },

	  {MID_TRACE_ID_GET_POS     , MID_TRACE_OP_GET_POS     },
	  {MID_TRACE_ID_PUT_POS     , MID_TRACE_OP_PUT_POS     },

	  {MID_TRACE_ID_SYSTEM_FILE , MID_TRACE_OP_SYSTEM_FILE },
	  {MID_TRACE_ID_ASCII_FILE  , MID_TRACE_OP_ASCII_FILE  },
	  {MID_TRACE_ID_BINARY_FILE , MID_TRACE_OP_BINARY_FILE },
	  {MID_TRACE_ID_ASCII_SCREEN, MID_TRACE_OP_ASCII_SCREEN},
	  {MID_TRACE_ID_SINGLE_STEP , MID_TRACE_OP_SINGLE_STEP },
	  {MID_TRACE_ID_RING_BELL   , MID_TRACE_OP_RING_BELL   },
	  {MID_TRACE_ID_NAMES       , MID_TRACE_OP_NAMES       },

	  {MID_TRACE_ID_VALUE       , MID_TRACE_OP_VALUE       },
	  {MID_TRACE_ID_EQUAL       , MID_TRACE_OP_EQUAL       },
	  {MID_TRACE_ID_NOT_EQUAL   , MID_TRACE_OP_NOT_EQUAL   },
	  {MID_TRACE_ID_LESS_EQUAL  , MID_TRACE_OP_LESS_EQUAL  },
	  {MID_TRACE_ID_MORE_EQUAL  , MID_TRACE_OP_MORE_EQUAL  }};

static  int     opccnt = sizeof(opcids) / sizeof(ID_DEF);


/****************************************************************************/
/*                        System Trace User Names                           */
/****************************************************************************/

static    ID_DEF   userids[] = {
	  {MID_TRACE_USER_ID_OTHER,MID_TRACE_USER_OP_OTHER},
	  {MID_TRACE_USER_ID_TED  ,MID_TRACE_USER_OP_TED  },
	  {MID_TRACE_USER_ID_CDD  ,MID_TRACE_USER_OP_CDD  },
	  {MID_TRACE_USER_ID_DD   ,MID_TRACE_USER_OP_DD   },
	  {MID_TRACE_USER_ID_3DM2 ,MID_TRACE_USER_OP_3DM2 },
	  {MID_TRACE_USER_ID_3DM1 ,MID_TRACE_USER_OP_3DM1 },
	  {MID_TRACE_USER_ID_2D   ,MID_TRACE_USER_OP_2D   },
	  {MID_TRACE_USER_ID_RMS  ,MID_TRACE_USER_OP_RMS  }};

static    int     usercnt = sizeof(userids) / sizeof(ID_DEF);


/****************************************************************************/
/*                  System Trace Indirect Access Block Names                */
/****************************************************************************/

static    NAMDEF   blknames[] = {
	  {BLK_NONE   ,"Indirect Access"         ,sizeof("Indirect Access"        ) - 1},
	  {BLK_ASCB   ,"Adapter Status Ctl Blk"  ,sizeof("Adapter Status Ctl Blk" ) - 1},
	  {BLK_FBCSB  ,"Frame Bufr Ctl Stat Blk" ,sizeof("Frame Bufr Ctl Stat Blk") - 1},
	  {BLK_CSB    ,"Current Context Stat Bl" ,sizeof("Current Context Stat Bl") - 1},
	  {BLK_CSB_X  ,"Context Status Block %d" ,sizeof("Context Status Block X" ) - 1},
	  {BLK_CMRB   ,"Context Memory Req Blk"  ,sizeof("Context Memory Req Blk" ) - 1},
	  {BLK_FRB    ,"Font Request Block"      ,sizeof("Font Request Block"     ) - 1},
	  {BLK_CCB    ,"Context Command Block"   ,sizeof("Context Command Block"  ) - 1},
	  {BLK_CTCB_X ,"Color Table Cmd Blk %d"  ,sizeof("Color Table Cmd Blk X"  ) - 1},
	  {BLK_CICB   ,"Cursor Image Cmd Block"  ,sizeof("Cursor Image Cmd Block" ) - 1},
	  {BLK_M1SB   ,"Current 3DM1 Status Blk" ,sizeof("Current 3DM1 Status Blk") - 1},
	  {BLK_M1SB_X ,"3DM1 Status Block %d"    ,sizeof("3DM1 Status Block X"    ) - 1},
	  {BLK_M1MSB  ,"Current 3DM1M Stat Blk"  ,sizeof("Current 3DM1M Stat Blk" ) - 1},
	  {BLK_M1MSB_X,"3DM1M Status Block %d"   ,sizeof("3DM1M Status Block X"   ) - 1}};

static    int     blkcnt = sizeof(blknames) / sizeof(NAMDEF);

#endif /* _H_MID_HW_NAMES */
