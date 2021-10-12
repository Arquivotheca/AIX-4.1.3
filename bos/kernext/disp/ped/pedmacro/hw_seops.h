/* @(#)23       1.3.2.4  src/bos/kernext/disp/ped/pedmacro/hw_seops.h, pedmacro, bos411, 9434B411a 8/3/94 12:36:01 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _HW_SEOPS_H
#define _HW_SEOPS_H

/*---------------------------------------------------------------------------
 * MODULE NAME:  se_ops.h
 *
 * FUNCTION:     Define structure element and other command opcodes
 *               as described in the GT3/4 Software Interface Spec.
 *
 * DEPENDENCIES: None.
 *
 * RESTRICTIONS: Only the lower 10 bits of the SE opcode is used by the
 *               adapter, allowing only 1024 unique opcodes.
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*             THIS FILE CONTAINS THE STRUCTURE ELEMENT OPCODES              */
/*                                                                           */
/*                   !!!!!!!!!!  IMPORTANT  !!!!!!!!!!!                      */
/*                                                                           */
/*  Any time this file is modified, you MUST create new versions of the      */
/*  files "get_opcode.c" and "opc_name.c".  This can be easily done by       */
/*  simply running the scripts "get_opcode.sh" and "opc_name.sh",            */
/*  respectively.  These scripts are available on the library in the         */
/*  se_comp directory.                                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#define OP_INDEX_MASK                       0x03FF

#define OP_NO_OPERATION                     0x0000
#define OP_SET_POLYLINE_INDEX               0x0001
#define OP_SET_POLYMARKER_INDEX             0x0002

#define OP_SET_EDGE_INDEX                   0x0004
#define OP_SET_INTERIOR_INDEX               0x0005
#define OP_SET_DEPTH_CUE_INDEX              0x0006
#define OP_SET_COLOR_PROCESSING_INDEX       0x0007
#define OP_SET_LINETYPE                     0x0008
#define OP_SET_LINEWIDTH_SCALE_FACTOR       0x0009
#define OP_SET_POLYLINE_COLOR_INDEX         0x000A
#define OP_SET_MARKER_TYPE                  0x000B
#define OP_SET_MARKER_SIZE_SCALE_FACTOR     0x000C
#define OP_SET_POLYMARKER_COLOR_INDEX       0x000D

#define OP_SET_TEXT_PRECISION               0x000F
#define OP_SET_CHARACTER_EXPANSION_FACTOR   0x0010
#define OP_SET_CHARACTER_SPACING            0x0011
#define OP_SET_TEXT_COLOR_INDEX             0x0012
#define OP_SET_CHARACTER_HEIGHT             0x0013
#define OP_SET_CHARACTER_UP_VECTOR          0x0014
#define OP_SET_TEXT_PATH                    0x0015
#define OP_SET_CHARACTER_POSITIONING_MODE   0x0016
#define OP_SET_TEXT_ALIGNMENT               0x0017
#define OP_SET_INTERIOR_STYLE               0x0018
#define OP_SET_INTERIOR_STYLE_INDEX         0x0019
#define OP_SET_INTERIOR_COLOR_INDEX         0x001A
#define OP_SET_EDGE_FLAG                    0x001B
#define OP_SET_EDGE_LINETYPE                0x001C
#define OP_SET_EDGE_COLOR_INDEX             0x001D
#define OP_SET_EDGE_SCALE_FACTOR            0x001E
#define OP_SET_POLYLINE_END_TYPE            0x001F
#define OP_SET_ANNOTATION_STYLE             0x0020
#define OP_SET_ANNOTATION_HEIGHT_SCALE_FACT 0x0021
#define OP_SET_ANNOTATION_HEIGHT            0x0022
#define OP_SET_ANNOTATION_UP_VECTOR         0x0023
#define OP_SET_ANNOTATION_PATH              0x0024
#define OP_SET_ANNOTATION_ALIGNMENT         0x0025
#define OP_SET_CHARACTER_UP_AND_BASE_VECTOR 0x0026
#define OP_SET_CHARACTER_LINE_SCALE_FACTOR  0x0027
#define OP_SET_POLYLINE_COLOR_DIRECT        0x0028
#define OP_SET_POLYMARKER_COLOR_DIRECT      0x0029
#define OP_SET_TEXT_COLOR_DIRECT            0x002A
#define OP_SET_INTERIOR_COLOR_DIRECT        0x002B
#define OP_SET_EDGE_COLOR_DIRECT            0x002C

#define OP_SET_FRAME_BUFFER_MASK            0x0031
#define OP_SET_FRAME_BUFFER_COMPARISON      0x0032

#define OP_SET_BACK_INTERIOR_COLOR_INDEX    0x003F
#define OP_SET_BACK_INTERIOR_COLOR_DIRECT   0x0040
#define OP_SET_SPECULAR_COLOR_INDEX         0x0041
#define OP_SET_SPECULAR_COLOR_DIRECT        0x0042
#define OP_SET_BACK_SPECULAR_COLOR_INDEX    0x0043
#define OP_SET_BACK_SPECULAR_COLOR_DIRECT   0x0044
#define OP_SET_POLYGON_CULLING              0x0045
#define OP_SET_SURFACE_PROPERTIES           0x0046
#define OP_SET_BACK_SURFACE_PROPERTIES      0x0047
#define OP_SET_FACE_DISTINGUISH_MODE        0x0048
#define OP_SET_LIGHT_SOURCE_STATE           0x0049
#define OP_SET_HLHSR_IDENTIFIER             0x004A

#define OP_SET_CURVE_APPROXIMATION_CRITERIA 0x004C

#define OP_SET_POLYHEDRON_EDGE_CULLING      0x004E
#define OP_SET_LIGHTING_CALCULATION_MODE    0x004F

#define OP_SET_ANTI_ALIASING_IDENTIFIER     0x0052

#define OP_SET_FACE_LIGHTING_METHOD         0x0054
#define OP_SET_Z_BUFFER_PROTECT_MASK        0x0055

#define OP_SET_POLYLINE_SHADING_METHOD      0x0062

#define OP_SET_MODELLING_TRANSFORMATION_3   0x00D0
#define OP_SET_MODELLING_TRANSFORMATION_2   0x00D1
#define OP_SET_GLOBAL_TRANSFORMATION_3      0x00D2
#define OP_SET_GLOBAL_TRANSFORMATION_2      0x00D3

#define OP_SET_VIEW_INDEX                   0x00D8

#define OP_SET_HIGHLIGHTING_COLOR_INDEX     0x00E0
#define OP_SET_HIGHLIGHTING_COLOR_DIRECT    0x00E1
#define OP_ADD_CLASS_NAME_TO_SET            0x00E2
#define OP_REMOVE_CLASS_NAME_FROM_SET       0x00E3
#define OP_INSERT_APPLICATION_DATA          0x00E4

#define OP_EXECUTE_STRUCTURE                0x00FA
#define OP_INSERT_LABEL                     0x00FB
#define OP_SET_PICK_IDENTIFIER              0x00FC

#define OP_CONDITIONAL_EXECUTE_STRUCTURE    0x00FE
#define OP_NO_OPERATION_II                  0x00FF

#define OP_POLYLINE_3                       0x0101
#define OP_POLYLINE_2                       0x0102
#define OP_DISJOINT_POLYLINE_3              0x0103
#define OP_DISJOINT_POLYLINE_2              0x0104
#define OP_POLYMARKER_3                     0x0105
#define OP_POLYMARKER_2                     0x0106

#define OP_CIRCLE_2                         0x0111
#define OP_CIRCULAR_ARC_2                   0x0112

#define OP_NON_UNIFORM_B_SPLINE_CURVE_3     0x0116
#define OP_NON_UNIFORM_B_SPLINE_CURVE_2     0x0117
#define OP_ELLIPSE_3                        0x0118
#define OP_ELLIPSE_2                        0x0119
#define OP_ELLIPTICAL_ARC_3                 0x011A
#define OP_ELLIPTICAL_ARC_2                 0x011B

#define OP_POLYGON_3                        0x0121
#define OP_POLYGON_2                        0x0122

#define OP_MARKER_GRID_3                    0x0125
#define OP_MARKER_GRID_2                    0x0126
#define OP_LINE_GRID_3                      0x0127
#define OP_LINE_GRID_2                      0x0128

#define OP_POLYGON_WITH_DATA_3              0x012B
#define OP_POLYGON_WITH_DATA_2              0x012C
#define OP_TRIANGLE_STRIP_3                 0x012D

#define OP_COMPOSITE_FILL_AREA_2            0x0134
#define OP_POLYHEDRON_EDGE                  0x0135
#define OP_CELL_ARRAY_3                     0x0136
#define OP_CELL_ARRAY_2                     0x0137
#define OP_POLYSPHERE                       0x0138

#define OP_FILL_AREA_3                      0x013C
#define OP_FILL_AREA_2                      0x013D
#define OP_POLYLINE_SET_3_WITH_DATA         0x013E

#define OP_V3F                              0x0200

#define OP_END_LINE                         0x0203
#define OP_END_POINT                        0x0204

#define OP_COLOR_INDEX                      0x021D
#define OP_C3I                              0x021E
#define OP_C3F                              0x021F

#define OP_N3F                              0x0220
#define OP_C3F_AD                           0x0221
#define OP_C3F_AMBIENT                      0x0222
#define OP_C3F_DIFFUSE                      0x0223
#define OP_C3F_EMISSION                     0x0224
#define OP_C3F_SPECULAR                     0x0225
#define OP_MAT_STATE                        0x0226
#define OP_MAT_PROPERTIES                   0x0227
#define OP_BACK_MATERIAL_STATE_M1M          0x0228
#define OP_BACK_MATERIAL_PROPERTIES_M1M     0x0229

#define OP_V4F                              0x0240

#define OP_V3I                              0x0280

#define OP_SET_LINETYPE_M1M                 0x8008
#define OP_SET_LINEWIDTH_SCALE_FACTOR_M1M   0x8009

#define OP_SET_INTERIOR_STYLE_M1M           0x8018

#define OP_SET_POLYLINE_END_TYPE_M1M        0x801F

#define OP_SET_POLYGON_CULLING_M1M          0x8045

#define OP_SET_CURVE_APPROXIMATION_CRIT_M1M 0x804C
#define OP_SET_SURFACE_APPROXIMATION_CR_M1M 0x804D

#define OP_SET_LIGHTING_CALCULATION_MOD_M1M 0x804F
#define OP_SET_TRIMMING_CURVE_APPROX_CR_M1M 0x8050

#define OP_SET_MODELLING_TRANSFORMATION_M1M 0x80D0

#define OP_NON_UNIFORM_B_SPLINE_CURVE_3_M1M 0x8116

#define OP_NON_UNIFORM_B_SPLINE_SURFACE_M1M 0x8131
#define OP_TRIMMED_NON_UNIFORM_B_SPLINE_M1M 0x8132

#define OP_SET_ACTIVE_FONT                  0x8201
#define OP_REMOVE_ACTIVE_FONT               0x8202
#define OP_DMA_QUEUE_DMA_REQUEST            0x8203

#define OP_SET_WINDOW_PARAMETERS            0x8205
#define OP_ASSOC_WINDOW_WITH_COLOR_PAL_FIFO 0x8206
#define OP_TRANSPARENCY_AND_COLOR_COMPARE   0x8207
#define OP_SYNCHRONIZE_ON_VERTICAL_RETRACE  0x8208
#define OP_SYNCHRONIZATION                  0x8209
#define OP_CLEAR_CONTROL_PLANES_FIFO        0x820A
#define OP_SELECT_DRAWING_BIT_PLANES        0x820B
#define OP_SET_BIT_PLANES_WRITE_MASKS       0x820C
#define OP_SET_BIT_PLANES_DISPLAY_MASKS     0x820D
#define OP_FRAME_BUFFER_CONTROL             0x820E
#define OP_SET_FRAME_BUFFER_SWAP_INTERVAL   0x820F
#define OP_Z_BUFFER_CONTROL                 0x8210
#define OP_CLEAR_BIT_PLANES                 0x8211
#define OP_SET_GPM_COMPARE                  0x8212
#define OP_SET_LOGICAL_OPERATION            0x8213

#define OP_READ_PIXEL_DATA                  0x8215
#define OP_WRITE_PIXEL_DATA                 0x8216
#define OP_PIXEL_BLIT                       0x8217
#define OP_ZOOM                             0x8218

#define OP_SET_FOREGROUND_PIXEL             0x8241
#define OP_SET_BACKGROUND_PIXEL             0x8242
#define OP_SET_LINE_ATTRIBUTES              0x8243
#define OP_SET_FILL_ATTRIBUTES              0x8244
#define OP_DEFINE_LINE_PATTERN              0x8245
#define OP_DEFINE_TILE                      0x8246

#define OP_DEFINE_STIPPLE                   0x8248

#define OP_SET_PATTERN_ORIGIN               0x824A
#define OP_SET_2D_COLOR_MODE                0x824E
#define OP_SET_2D_CLIENT_CLIP               0x824F

#define OP_POLYPOINT                        0x8261
#define OP_POLYLINES                        0x8262
#define OP_POLYSEGMENT                      0x8263
#define OP_POLYRECTANGLE                    0x8264

#define OP_FILL_POLYGON                     0x8266
#define OP_FILL_POLYRECTANGLE               0x8267

#define OP_POLYTEXT_8                       0x8269
#define OP_POLYTEXT_16                      0x826A
#define OP_IMAGETEXT_8                      0x826B
#define OP_IMAGETEXT_16                     0x826C
#define OP_PUSH_PIXELS                      0x826D
#define OP_FILL_SPANS                       0x826E

#define OP_BEGIN_RENDERING                  0x8281
#define OP_END_RENDERING                    0x8282
#define OP_LOAD_RSL_TABLES                  0x8283

#define OP_BEGIN_PICK_M1                    0x8285
#define OP_END_PICK_M1                      0x8286
#define OP_BEGIN_ECHO                       0x8287
#define OP_SET_ECHO_PARAMETERS              0x8288
#define OP_END_ECHO                         0x8289

#define OP_ADJUST_ELEMENT_COUNTER           0x828A
#define OP_INCREMENT_ELEMENT_COUNTER        0x828B

#define OP_LOAD_PDRT_ATTRIBUTES             0x828E
#define OP_LOAD_WORKSTATION_TRANSFORMATION  0x828F
#define OP_LOAD_FILTERS                     0x8290
#define OP_SET_M1_CLIENT_CLIP_REGION        0x8291
#define OP_LOAD_VIEW_TABLE_DATA             0x8292

#define OP_CONDITIONAL_RETURN               0x8294
#define OP_TEST_EXTENT_3                    0x8295
#define OP_TEST_EXTENT_2                    0x8296
#define OP_SET_CONDITION                    0x8297
#define OP_SET_ATTRIBUTE_SOURCE_FLAG_W_CORR 0x8298
#define OP_SET_TEXT_FONT_W_CORR             0x8299
#define OP_SET_TEXT_INDEX_W_CORR            0x829A
#define OP_END_VIEW_W_CORR                  0x829B

#define OP_SAVE_TSL                         0x829D

#define OP_RELEASE_TSL                      0x829F

#define OP_BEGIN_VIEW                       0x82A0

#define OP_RESTORE_TSL_W_CORR               0x82A2

#define OP_SET_HLHSR_PROCESSING_OPTION      0x82A5
#define OP_SURFACE_WITH_LINES_MODE          0x82A6
#define OP_SURFACE_POLYLINE_3               0x82A7

#define OP_ANNOTATION_TEXT_GLYPHS           0x82AA
#define OP_ANNOTATION_TEXT_RELATIVE_GLYPHS  0x82AB
#define OP_GEOMETRIC_TEXT_GLYPHS            0x82AC
#define OP_CHARACTER_LINE_TEXT_GLYPHS       0x82AD
#define OP_FAST_TEXT                        0x82AE
#define OP_FAST_TEXT_RELATIVE               0x82AF

#define OP_MATRIX_MODE                      0x82C1
#define OP_PUSH_MODELLING_MATRIX_STACK      0x82C2
#define OP_POP_MODELLING_MATRIX_STACK       0x82C3
#define OP_SET_MODELLING_MATRIX_INVERSE_TRA 0x82C4
#define OP_GET_CURRENT_CHARACTER_POSITION   0x82C5
#define OP_GET_CURRENT_COLOR                0x82C6
#define OP_GET_CURRENT_MODELLING_MATRIX     0x82C7
#define OP_GET_CURRENT_PROJECTION_MATRIX    0x82C8
#define OP_INITIALIZE_NAME_STACK            0x82C9
#define OP_LOAD_NAME                        0x82CA
#define OP_PUSH_NAME                        0x82CB
#define OP_POP_NAME                         0x82CC
#define OP_BEGIN_PICK_M1M                   0x82CD
#define OP_END_PICK_M1M                     0x82CE

#define OP_SET_3DM1M_COLOR_MODE             0x82D0
#define OP_SET_ANTI_ALIASED_LINE_MODE       0x82D1
#define OP_LOAD_GL_ATTRIBUTE_SETS           0x82D2
#define OP_SET_SCREEN_MASK                  0x82D3

#define OP_LMODEL_STATE                     0x82D5
#define OP_LIGHT_LIST                       0x82D6
#define OP_LIGHTING_MODEL                   0x82D7
#define OP_DEFINE_LINE_STYLE                0x82D8
#define OP_SET_LINE_STYLE_REPEAT_COUNT      0x82D9
#define OP_C2H_PACKET                       0x82DA

#define OP_POLYGON_WITH_DATA_4              0x82E4
#define OP_TRIANGLE_MESH_WITH_DATA_4        0x82E5
#define OP_ELLIPTICAL_ARC_3_M1M             0x82E6
#define OP_ELLIPTICAL_ARC_3_FILLED          0x82E7

#define OP_RASTER_TEXT                      0x82E9
#define OP_DISJOINT_POLYLINE_M1M_2          0x82EA
#define OP_DISJOINT_POLYLINE_M1M_3          0x82EB
#define OP_BEGIN_LINE                       0x82EC

#define OP_BEGIN_POINT                      0x82EE

#define OP_BLAST_READ                       0x82F0
#define OP_BLAST_WRITE                      0x82F1
#define OP_BLAST_LOOP                       0x82F2
#define OP_START_TRACE_FIFO                 0x82F3
#define OP_STOP_TRACE_FIFO                  0x82F4
#define OP_RESET_TRACE_FIFO                 0x82F5
#define OP_START_DMA_TRACE_FIFO             0x82F6
#define OP_STOP_DMA_TRACE_FIFO              0x82F7

#define OP_CDD_CHAR_BLIT                    0x8400
#define OP_CDD_SET_COLORS                   0x8401
#define OP_CDD_CLEAR                        0x8402
#define OP_CDD_CRC                          0x8403

#define OP_SWITCH_CONTEXT                   0x9002
#define OP_START_SOFT_RESET                 0x9003
#define OP_COMPLETE_SOFT_RESET              0x9004
#define OP_ECHO_COMMO                       0x900F

#define OP_SCREEN_BLANKING_CONTROL          0xA001
#define OP_SET_WINDOW_PARAMETERS_PCB        0xA002

#define OP_ASSOC_WINDOW_WITH_COLOR_PALETTE  0xA004
#define OP_REQUEST_TO_DMA_STRUCTURE_ELEMENT 0xA005
#define OP_CRTC_CONTROL                     0xA006
#define OP_DIAGNOSTICS                      0xA007
#define OP_CONTEXT_STATE_UPDATE_COMPLETE    0xA008
#define OP_CONTEXT_MEMORY_PINNED            0xA009

#define OP_DISPLAY_POWER_DOWN  	 	    0xA00C
#define OP_DISPLAY_POWER_UP  	 	    0xA00D

#define OP_CLEAR_CONTROL_PLANES             0xA00E

#define OP_DEFINE_CURSOR                    0xA010
#define OP_SET_ACTIVE_CURSOR                0xA011
#define OP_MOVE_CURSOR                      0xA012
#define OP_HIDE_SHOW_ACTIVE_CURSOR          0xA013
#define OP_BLINK_CURSOR_COLORS              0xA014

#define OP_FONT_PINNED                      0xA018
#define OP_FONT_MUST_BE_UNPINNED            0xA019
#define OP_FONT_REQUEST_RECEIVED            0xA01A

#define OP_LOAD_FRAME_BUFFER_COLOR_TABLE    0xA020
#define OP_BLINK_FRAME_BUFFER_COLOR_TABLE   0xA021
#define OP_LOAD_OVERLAY_PLANES_COLOR_TABLE  0xA022
#define OP_BLINK_OVERLAY_PLANES_COLOR_TABLE 0xA023
#define OP_MULTIMAP                         0xA024
#define OP_ONEMAP                           0xA025
#define OP_SETMAP                           0xA026
#define OP_CYCLE_COLOR_MAPS                 0xA027

#define OP_START_TRACE_PCB                  0xA0F3
#define OP_STOP_TRACE_PCB                   0xA0F4
#define OP_RESET_TRACE_PCB                  0xA0F5
#define OP_START_DMA_TRACE_PCB              0xA0F6
#define OP_STOP_DMA_TRACE_PCB               0xA0F7

/*
 * EVERYTHING BELOW IS TEMPORARY
 */

#define OP_TEMP_OPCODE_FLAG                 0x40000000
#define OP_OLD_OPCODE_FLAG                  0x20000000
#define OP_UNDEFINED_OPCODE                 0x10000000

/*
 * EVERYTHING BELOW IS FOR THE TEST DRIVER
 */

#define OP_FIXED_TIME_DELAY                 0xF000
#define OP_RANDOM_TIME_DELAY                0xF001
#define OP_SET_TIMER                        0xF002
#define OP_START_TIMER                      0xF003
#define OP_STOP_TIMER                       0xF004
#define OP_PRINT_TIMER                      0xF005
#define OP_PRINT_TIME_AND_DATE              0xF006
#define OP_PRINT_COMMENT                    0xF007
#define OP_WAIT_KEY                         0xF008
#define OP_FETCH_MICROCODE_TRACE            0xF009
#define OP_INVALID_DATA_IGNORED_BY_TED      0xF00F
#define OP_LOAD_DMA_BLIT                    0xF010
#define OP_SAVE_DMA_BLIT                    0xF011
#define OP_LOAD_DMA_SE                      0xF012
#define OP_LOAD_PACKED_DMA_BLIT             0xF013
#define OP_SAVE_PACKED_DMA_BLIT             0xF014
#define OP_CLEAR_DMA_BLIT                   0xF015
#define OP_SAVE_DMA_TRACE_BUFFER            0xF016

#define OP_LOAD_DMA_FONT                    0xF020

#define OP_LOAD_CONTEXT_SE                  0xF030
#define OP_LOAD_CONTEXT_COMMAND_BLOCK       0xF031
#define OP_BEGIN_CONTEXT_SWITCH             0xF032
#define OP_END_CONTEXT_SWITCH               0xF033

#define OP_LOAD_CURSOR_IMAGE_COMMAND_BLOCK  0xF040

#define OP_LOAD_COLOR_TABLE_COMMAND_BLOCK   0xF050

#define OP_EXECUTE_TED_COMMAND              0xF060

#define OP_SET_SE_CORRELATOR_HW             0xF070

#define OP_SAVE_PICK_BUFFER                 0xF080

#define OP_LOAD_IND_CONTROL_REGISTER        0xF090
#define OP_LOAD_IND_ADDRESS_REGISTER        0xF091
#define OP_LOAD_IND_DATA_BUFFER             0xF092

#define OP_COMPUTE                          0xF0A0
#define OP_MODIFY                           0xF0A1
#define OP_IF                               0xF0A2
#define OP_ELSE                             0xF0A3
#define OP_WHILE                            0xF0A4
#define OP_END                              0xF0A5
#define OP_DO                               0xF0A6
#define OP_UNTIL                            0xF0A7

#define OP_CURRENT_USER_ID_IS_OTHER         0xF0B0
#define OP_CURRENT_USER_ID_IS_TED           0xF0B1
#define OP_CURRENT_USER_ID_IS_CDD           0xF0B2
#define OP_CURRENT_USER_ID_IS_DD            0xF0B3
#define OP_CURRENT_USER_ID_IS_3DM2          0xF0B4
#define OP_CURRENT_USER_ID_IS_3DM1          0xF0B5
#define OP_CURRENT_USER_ID_IS_X             0xF0B6
#define OP_CURRENT_USER_ID_IS_RMS           0xF0B7

#define OP_READ_IND_DATA_BUFFER             0xF0C0
#define OP_READ_ADAPTER_STATUS_CONTROL_BLK  0xF0C1
#define OP_READ_FRAME_BUFFER_CTRL_STAT_BLK  0xF0C2
#define OP_READ_CURRENT_CONTEXT_STATUS_BLK  0xF0C3
#define OP_READ_CONTEXT_STATUS_BLOCK_X      0xF0C4
#define OP_READ_CONTEXT_MEMORY_REQUEST_BLK  0xF0C5
#define OP_READ_FONT_REQUEST_BLOCK          0xF0C6
#define OP_READ_CONTEXT_COMMAND_BLOCK       0xF0C7
#define OP_READ_COLOR_TABLE_COMMAND_BLOCK   0xF0C8
#define OP_READ_CURSOR_IMAGE_COMMAND_BLOCK  0xF0C9
#define OP_READ_CURRENT_3DM1_STATUS_BLOCK   0xF0CA
#define OP_READ_3DM1_STATUS_BLOCK           0xF0CB
#define OP_READ_CURRENT_3DM1M_STATUS_BLOCK  0xF0CC
#define OP_READ_3DM1M_STATUS_BLOCK          0xF0CD

#define OP_WRITE_IND_DATA_BUFFER            0xF0D0
#define OP_WRITE_ADAPTER_STATUS_CONTROL_BLK 0xF0D1
#define OP_WRITE_FRAME_BUFFER_CTRL_STAT_BLK 0xF0D2
#define OP_WRITE_CURRENT_CONTEXT_STATUS_BLK 0xF0D3
#define OP_WRITE_CONTEXT_STATUS_BLOCK_X     0xF0D4
#define OP_WRITE_CONTEXT_MEMORY_REQUEST_BLK 0xF0D5
#define OP_WRITE_FONT_REQUEST_BLOCK         0xF0D6
#define OP_WRITE_CONTEXT_COMMAND_BLOCK      0xF0D7
#define OP_WRITE_COLOR_TABLE_COMMAND_BLOCK  0xF0D8
#define OP_WRITE_CURSOR_IMAGE_COMMAND_BLOCK 0xF0D9
#define OP_WRITE_CURRENT_3DM1_STATUS_BLOCK  0xF0DA
#define OP_WRITE_3DM1_STATUS_BLOCK          0xF0DB
#define OP_WRITE_CURRENT_3DM1M_STATUS_BLOCK 0xF0DC
#define OP_WRITE_3DM1M_STATUS_BLOCK         0xF0DD

#define OP_HARDWARE_RESET                   0xFFFF

#endif /* _HW_SEOPS_H not defined */

